// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ic/ic-compiler.h"

#include "src/cpu-profiler.h"
#include "src/ic/handler-compiler.h"
#include "src/ic/ic-inl.h"


namespace v8 {
namespace internal {


Handle<Code> PropertyICCompiler::Find(Handle<Name> name,
                                      Handle<Map> stub_holder, Code::Kind kind,
                                      ExtraICState extra_state,
                                      CacheHolderFlag cache_holder) {
  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, extra_state, cache_holder);
  Object* probe = stub_holder->FindInCodeCache(*name, flags);
  if (probe->IsCode()) return handle(Code::cast(probe));
  return Handle<Code>::null();
}


bool PropertyICCompiler::IncludesNumberMap(MapHandleList* maps) {
  for (int i = 0; i < maps->length(); ++i) {
    if (maps->at(i)->instance_type() == HEAP_NUMBER_TYPE) return true;
  }
  return false;
}


Handle<Code> PropertyICCompiler::CompileMonomorphic(Handle<Map> map,
                                                    Handle<Code> handler,
                                                    Handle<Name> name,
                                                    IcCheckType check) {
  MapHandleList maps(1);
  CodeHandleList handlers(1);
  maps.Add(map);
  handlers.Add(handler);
  Code::StubType stub_type = handler->type();
  return CompilePolymorphic(&maps, &handlers, name, stub_type, check);
}


Handle<Code> PropertyICCompiler::ComputeMonomorphic(
    Code::Kind kind, Handle<Name> name, Handle<Map> map, Handle<Code> handler,
    ExtraICState extra_ic_state) {
  Isolate* isolate = name->GetIsolate();
  if (handler.is_identical_to(isolate->builtins()->LoadIC_Normal()) ||
      handler.is_identical_to(isolate->builtins()->LoadIC_Normal_Strong()) ||
      handler.is_identical_to(isolate->builtins()->StoreIC_Normal())) {
    name = isolate->factory()->normal_ic_symbol();
  }

  CacheHolderFlag flag;
  Handle<Map> stub_holder = IC::GetICCacheHolder(map, isolate, &flag);
  if (kind == Code::KEYED_STORE_IC) {
    // Always set the "property" bit.
    extra_ic_state =
        KeyedStoreIC::IcCheckTypeField::update(extra_ic_state, PROPERTY);
    DCHECK(STANDARD_STORE ==
           KeyedStoreIC::GetKeyedAccessStoreMode(extra_ic_state));
  } else if (kind == Code::KEYED_LOAD_IC) {
    extra_ic_state = KeyedLoadIC::IcCheckTypeField::update(extra_ic_state,
                                                           PROPERTY);
  }

  Handle<Code> ic;
  // There are multiple string maps that all use the same prototype. That
  // prototype cannot hold multiple handlers, one for each of the string maps,
  // for a single name. Hence, turn off caching of the IC.
  bool can_be_cached = map->instance_type() >= FIRST_NONSTRING_TYPE;
  if (can_be_cached) {
    ic = Find(name, stub_holder, kind, extra_ic_state, flag);
    if (!ic.is_null()) return ic;
  }

  PropertyICCompiler ic_compiler(isolate, kind, extra_ic_state, flag);
  ic = ic_compiler.CompileMonomorphic(map, handler, name, PROPERTY);

  if (can_be_cached) Map::UpdateCodeCache(stub_holder, name, ic);
  return ic;
}


Handle<Code> PropertyICCompiler::ComputeKeyedLoadMonomorphicHandler(
    Handle<Map> receiver_map, ExtraICState extra_ic_state) {
  Isolate* isolate = receiver_map->GetIsolate();
  bool is_js_array = receiver_map->instance_type() == JS_ARRAY_TYPE;
  ElementsKind elements_kind = receiver_map->elements_kind();

  // No need to check for an elements-free prototype chain here, the generated
  // stub code needs to check that dynamically anyway.
  bool convert_hole_to_undefined =
      is_js_array && elements_kind == FAST_HOLEY_ELEMENTS &&
      *receiver_map == isolate->get_initial_js_array_map(elements_kind) &&
      !(is_strong(LoadICState::GetLanguageMode(extra_ic_state)));
  Handle<Code> stub;
  if (receiver_map->has_indexed_interceptor()) {
    stub = LoadIndexedInterceptorStub(isolate).GetCode();
  } else if (receiver_map->IsStringMap()) {
    // We have a string.
    stub = LoadIndexedStringStub(isolate).GetCode();
  } else if (receiver_map->has_sloppy_arguments_elements()) {
    stub = KeyedLoadSloppyArgumentsStub(isolate).GetCode();
  } else if (receiver_map->has_fast_elements() ||
             receiver_map->has_fixed_typed_array_elements()) {
    stub = LoadFastElementStub(isolate, is_js_array, elements_kind,
                               convert_hole_to_undefined).GetCode();
  } else {
    stub = LoadDictionaryElementStub(isolate, LoadICState(extra_ic_state))
               .GetCode();
  }
  return stub;
}


Handle<Code> PropertyICCompiler::ComputeKeyedStoreMonomorphicHandler(
    Handle<Map> receiver_map, LanguageMode language_mode,
    KeyedAccessStoreMode store_mode) {
  Isolate* isolate = receiver_map->GetIsolate();
  ExtraICState extra_state =
      KeyedStoreIC::ComputeExtraICState(language_mode, store_mode);

  DCHECK(store_mode == STANDARD_STORE ||
         store_mode == STORE_AND_GROW_NO_TRANSITION ||
         store_mode == STORE_NO_TRANSITION_IGNORE_OUT_OF_BOUNDS ||
         store_mode == STORE_NO_TRANSITION_HANDLE_COW);

  PropertyICCompiler compiler(isolate, Code::KEYED_STORE_IC, extra_state);
  Handle<Code> code =
      compiler.CompileKeyedStoreMonomorphicHandler(receiver_map, store_mode);
  return code;
}


Handle<Code> PropertyICCompiler::ComputeKeyedStoreMonomorphic(
    Handle<Map> receiver_map, LanguageMode language_mode,
    KeyedAccessStoreMode store_mode) {
  Isolate* isolate = receiver_map->GetIsolate();
  ExtraICState extra_state =
      KeyedStoreIC::ComputeExtraICState(language_mode, store_mode);
  Code::Flags flags =
      Code::ComputeMonomorphicFlags(Code::KEYED_STORE_IC, extra_state);

  DCHECK(store_mode == STANDARD_STORE ||
         store_mode == STORE_AND_GROW_NO_TRANSITION ||
         store_mode == STORE_NO_TRANSITION_IGNORE_OUT_OF_BOUNDS ||
         store_mode == STORE_NO_TRANSITION_HANDLE_COW);

  Handle<String> name = isolate->factory()->KeyedStoreMonomorphic_string();
  Handle<Object> probe(receiver_map->FindInCodeCache(*name, flags), isolate);
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  PropertyICCompiler compiler(isolate, Code::KEYED_STORE_IC, extra_state);
  Handle<Code> code =
      compiler.CompileKeyedStoreMonomorphic(receiver_map, store_mode);

  Map::UpdateCodeCache(receiver_map, name, code);
  DCHECK(KeyedStoreIC::GetKeyedAccessStoreMode(code->extra_ic_state()) ==
         store_mode);
  return code;
}


Code* PropertyICCompiler::FindPreMonomorphic(Isolate* isolate, Code::Kind kind,
                                             ExtraICState state) {
  Code::Flags flags = Code::ComputeFlags(kind, PREMONOMORPHIC, state);
  UnseededNumberDictionary* dictionary =
      isolate->heap()->non_monomorphic_cache();
  int entry = dictionary->FindEntry(isolate, flags);
  DCHECK(entry != -1);
  Object* code = dictionary->ValueAt(entry);
  // This might be called during the marking phase of the collector
  // hence the unchecked cast.
  return reinterpret_cast<Code*>(code);
}


static void FillCache(Isolate* isolate, Handle<Code> code) {
  Handle<UnseededNumberDictionary> dictionary = UnseededNumberDictionary::Set(
      isolate->factory()->non_monomorphic_cache(), code->flags(), code);
  isolate->heap()->SetRootNonMonomorphicCache(*dictionary);
}


Handle<Code> PropertyICCompiler::ComputeStore(Isolate* isolate,
                                              InlineCacheState ic_state,
                                              ExtraICState extra_state) {
  Code::Flags flags = Code::ComputeFlags(Code::STORE_IC, ic_state, extra_state);
  Handle<UnseededNumberDictionary> cache =
      isolate->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  PropertyICCompiler compiler(isolate, Code::STORE_IC);
  Handle<Code> code;
  if (ic_state == UNINITIALIZED) {
    code = compiler.CompileStoreInitialize(flags);
  } else if (ic_state == PREMONOMORPHIC) {
    code = compiler.CompileStorePreMonomorphic(flags);
  } else if (ic_state == GENERIC) {
    code = compiler.CompileStoreGeneric(flags);
  } else if (ic_state == MEGAMORPHIC) {
    code = compiler.CompileStoreMegamorphic(flags);
  } else {
    UNREACHABLE();
  }

  FillCache(isolate, code);
  return code;
}


Handle<Code> PropertyICCompiler::ComputeCompareNil(Handle<Map> receiver_map,
                                                   CompareNilICStub* stub) {
  Isolate* isolate = receiver_map->GetIsolate();
  Handle<String> name(isolate->heap()->empty_string());
  if (!receiver_map->is_dictionary_map()) {
    Handle<Code> cached_ic =
        Find(name, receiver_map, Code::COMPARE_NIL_IC, stub->GetExtraICState());
    if (!cached_ic.is_null()) return cached_ic;
  }

  Code::FindAndReplacePattern pattern;
  Handle<WeakCell> cell = Map::WeakCellForMap(receiver_map);
  pattern.Add(isolate->factory()->meta_map(), cell);
  Handle<Code> ic = stub->GetCodeCopy(pattern);

  if (!receiver_map->is_dictionary_map()) {
    Map::UpdateCodeCache(receiver_map, name, ic);
  }

  return ic;
}


Handle<Code> PropertyICCompiler::ComputePolymorphic(
    Code::Kind kind, MapHandleList* maps, CodeHandleList* handlers,
    int valid_maps, Handle<Name> name, ExtraICState extra_ic_state) {
  Handle<Code> handler = handlers->at(0);
  Code::StubType type = valid_maps == 1 ? handler->type() : Code::NORMAL;
  DCHECK(kind == Code::LOAD_IC || kind == Code::STORE_IC);
  PropertyICCompiler ic_compiler(name->GetIsolate(), kind, extra_ic_state);
  return ic_compiler.CompilePolymorphic(maps, handlers, name, type, PROPERTY);
}


void PropertyICCompiler::ComputeKeyedStorePolymorphicHandlers(
    MapHandleList* receiver_maps, MapHandleList* transitioned_maps,
    CodeHandleList* handlers, KeyedAccessStoreMode store_mode,
    LanguageMode language_mode) {
  Isolate* isolate = receiver_maps->at(0)->GetIsolate();
  DCHECK(store_mode == STANDARD_STORE ||
         store_mode == STORE_AND_GROW_NO_TRANSITION ||
         store_mode == STORE_NO_TRANSITION_IGNORE_OUT_OF_BOUNDS ||
         store_mode == STORE_NO_TRANSITION_HANDLE_COW);
  ExtraICState extra_state =
      KeyedStoreIC::ComputeExtraICState(language_mode, store_mode);
  PropertyICCompiler compiler(isolate, Code::KEYED_STORE_IC, extra_state);
  compiler.CompileKeyedStorePolymorphicHandlers(
      receiver_maps, transitioned_maps, handlers, store_mode);
}


Handle<Code> PropertyICCompiler::ComputeKeyedStorePolymorphic(
    MapHandleList* receiver_maps, KeyedAccessStoreMode store_mode,
    LanguageMode language_mode) {
  Isolate* isolate = receiver_maps->at(0)->GetIsolate();
  DCHECK(store_mode == STANDARD_STORE ||
         store_mode == STORE_AND_GROW_NO_TRANSITION ||
         store_mode == STORE_NO_TRANSITION_IGNORE_OUT_OF_BOUNDS ||
         store_mode == STORE_NO_TRANSITION_HANDLE_COW);
  Handle<PolymorphicCodeCache> cache =
      isolate->factory()->polymorphic_code_cache();
  ExtraICState extra_state =
      KeyedStoreIC::ComputeExtraICState(language_mode, store_mode);
  Code::Flags flags =
      Code::ComputeFlags(Code::KEYED_STORE_IC, POLYMORPHIC, extra_state);
  Handle<Object> probe = cache->Lookup(receiver_maps, flags);
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  PropertyICCompiler compiler(isolate, Code::KEYED_STORE_IC, extra_state);
  Handle<Code> code =
      compiler.CompileKeyedStorePolymorphic(receiver_maps, store_mode);
  PolymorphicCodeCache::Update(cache, receiver_maps, flags, code);
  return code;
}


Handle<Code> PropertyICCompiler::CompileLoadInitialize(Code::Flags flags) {
  LoadIC::GenerateInitialize(masm());
  Handle<Code> code = GetCodeWithFlags(flags, "CompileLoadInitialize");
  PROFILE(isolate(), CodeCreateEvent(Logger::LOAD_INITIALIZE_TAG, *code, 0));
  return code;
}


Handle<Code> PropertyICCompiler::CompileStoreInitialize(Code::Flags flags) {
  StoreIC::GenerateInitialize(masm());
  Handle<Code> code = GetCodeWithFlags(flags, "CompileStoreInitialize");
  PROFILE(isolate(), CodeCreateEvent(Logger::STORE_INITIALIZE_TAG, *code, 0));
  return code;
}


Handle<Code> PropertyICCompiler::CompileStorePreMonomorphic(Code::Flags flags) {
  StoreIC::GeneratePreMonomorphic(masm());
  Handle<Code> code = GetCodeWithFlags(flags, "CompileStorePreMonomorphic");
  PROFILE(isolate(),
          CodeCreateEvent(Logger::STORE_PREMONOMORPHIC_TAG, *code, 0));
  return code;
}


Handle<Code> PropertyICCompiler::CompileStoreGeneric(Code::Flags flags) {
  ExtraICState extra_state = Code::ExtractExtraICStateFromFlags(flags);
  LanguageMode language_mode = StoreICState::GetLanguageMode(extra_state);
  GenerateRuntimeSetProperty(masm(), language_mode);
  Handle<Code> code = GetCodeWithFlags(flags, "CompileStoreGeneric");
  PROFILE(isolate(), CodeCreateEvent(Logger::STORE_GENERIC_TAG, *code, 0));
  return code;
}


Handle<Code> PropertyICCompiler::CompileStoreMegamorphic(Code::Flags flags) {
  StoreIC::GenerateMegamorphic(masm());
  Handle<Code> code = GetCodeWithFlags(flags, "CompileStoreMegamorphic");
  PROFILE(isolate(), CodeCreateEvent(Logger::STORE_MEGAMORPHIC_TAG, *code, 0));
  return code;
}


Handle<Code> PropertyICCompiler::GetCode(Code::Kind kind, Code::StubType type,
                                         Handle<Name> name,
                                         InlineCacheState state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, state, extra_ic_state_, type, cache_holder());
  Handle<Code> code = GetCodeWithFlags(flags, name);
  PROFILE(isolate(), CodeCreateEvent(log_kind(code), *code, *name));
#ifdef DEBUG
  code->VerifyEmbeddedObjects();
#endif
  return code;
}


void PropertyICCompiler::CompileKeyedStorePolymorphicHandlers(
    MapHandleList* receiver_maps, MapHandleList* transitioned_maps,
    CodeHandleList* handlers, KeyedAccessStoreMode store_mode) {
  for (int i = 0; i < receiver_maps->length(); ++i) {
    Handle<Map> receiver_map(receiver_maps->at(i));
    Handle<Code> cached_stub;
    Handle<Map> transitioned_map =
        Map::FindTransitionedMap(receiver_map, receiver_maps);

    // TODO(mvstanton): The code below is doing pessimistic elements
    // transitions. I would like to stop doing that and rely on Allocation Site
    // Tracking to do a better job of ensuring the data types are what they need
    // to be. Not all the elements are in place yet, pessimistic elements
    // transitions are still important for performance.
    bool is_js_array = receiver_map->instance_type() == JS_ARRAY_TYPE;
    ElementsKind elements_kind = receiver_map->elements_kind();
    if (!transitioned_map.is_null()) {
      cached_stub =
          ElementsTransitionAndStoreStub(isolate(), elements_kind,
                                         transitioned_map->elements_kind(),
                                         is_js_array, store_mode).GetCode();
    } else if (receiver_map->instance_type() < FIRST_JS_RECEIVER_TYPE) {
      // TODO(mvstanton): Consider embedding store_mode in the state of the slow
      // keyed store ic for uniformity.
      cached_stub = isolate()->builtins()->KeyedStoreIC_Slow();
    } else {
      if (IsSloppyArgumentsElements(elements_kind)) {
        cached_stub =
            KeyedStoreSloppyArgumentsStub(isolate(), store_mode).GetCode();
      } else if (receiver_map->has_fast_elements() ||
                 receiver_map->has_fixed_typed_array_elements()) {
        cached_stub = StoreFastElementStub(isolate(), is_js_array,
                                           elements_kind, store_mode).GetCode();
      } else {
        cached_stub =
            StoreElementStub(isolate(), elements_kind, store_mode).GetCode();
      }
    }
    DCHECK(!cached_stub.is_null());
    handlers->Add(cached_stub);
    transitioned_maps->Add(transitioned_map);
  }
}


Handle<Code> PropertyICCompiler::CompileKeyedStorePolymorphic(
    MapHandleList* receiver_maps, KeyedAccessStoreMode store_mode) {
  // Collect MONOMORPHIC stubs for all |receiver_maps|.
  CodeHandleList handlers(receiver_maps->length());
  MapHandleList transitioned_maps(receiver_maps->length());
  CompileKeyedStorePolymorphicHandlers(receiver_maps, &transitioned_maps,
                                       &handlers, store_mode);

  Handle<Code> code = CompileKeyedStorePolymorphic(receiver_maps, &handlers,
                                                   &transitioned_maps);
  isolate()->counters()->keyed_store_polymorphic_stubs()->Increment();
  PROFILE(isolate(), CodeCreateEvent(log_kind(code), *code, 0));
  return code;
}


#define __ ACCESS_MASM(masm())


Handle<Code> PropertyICCompiler::CompileKeyedStoreMonomorphicHandler(
    Handle<Map> receiver_map, KeyedAccessStoreMode store_mode) {
  ElementsKind elements_kind = receiver_map->elements_kind();
  bool is_jsarray = receiver_map->instance_type() == JS_ARRAY_TYPE;
  Handle<Code> stub;
  if (receiver_map->has_sloppy_arguments_elements()) {
    stub = KeyedStoreSloppyArgumentsStub(isolate(), store_mode).GetCode();
  } else if (receiver_map->has_fast_elements() ||
             receiver_map->has_fixed_typed_array_elements()) {
    stub = StoreFastElementStub(isolate(), is_jsarray, elements_kind,
                                store_mode).GetCode();
  } else {
    stub = StoreElementStub(isolate(), elements_kind, store_mode).GetCode();
  }
  return stub;
}


Handle<Code> PropertyICCompiler::CompileKeyedStoreMonomorphic(
    Handle<Map> receiver_map, KeyedAccessStoreMode store_mode) {
  Handle<Code> stub =
      CompileKeyedStoreMonomorphicHandler(receiver_map, store_mode);

  Handle<WeakCell> cell = Map::WeakCellForMap(receiver_map);

  __ DispatchWeakMap(receiver(), scratch1(), scratch2(), cell, stub,
                     DO_SMI_CHECK);

  TailCallBuiltin(masm(), Builtins::kKeyedStoreIC_Miss);

  return GetCode(kind(), Code::NORMAL, factory()->empty_string());
}


#undef __
}  // namespace internal
}  // namespace v8
