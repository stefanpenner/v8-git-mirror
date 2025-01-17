// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/v8.h"

#include "src/interpreter/bytecode-array-builder.h"
#include "test/unittests/test-utils.h"

namespace v8 {
namespace internal {
namespace interpreter {

class BytecodeArrayBuilderTest : public TestWithIsolateAndZone {
 public:
  BytecodeArrayBuilderTest() {}
  ~BytecodeArrayBuilderTest() override {}
};


TEST_F(BytecodeArrayBuilderTest, AllBytecodesGenerated) {
  BytecodeArrayBuilder builder(isolate(), zone());

  builder.set_locals_count(1);
  builder.set_parameter_count(0);
  CHECK_EQ(builder.locals_count(), 1);

  // Emit constant loads.
  builder.LoadLiteral(Smi::FromInt(0))
      .LoadLiteral(Smi::FromInt(8))
      .LoadLiteral(Smi::FromInt(10000000))
      .LoadUndefined()
      .LoadNull()
      .LoadTheHole()
      .LoadTrue()
      .LoadFalse();

  // Emit accumulator transfers.
  Register reg(0);
  builder.LoadAccumulatorWithRegister(reg).StoreAccumulatorInRegister(reg);

  // Emit load / store property operations.
  builder.LoadNamedProperty(reg, 0, LanguageMode::SLOPPY)
      .LoadKeyedProperty(reg, 0, LanguageMode::SLOPPY)
      .StoreNamedProperty(reg, reg, 0, LanguageMode::SLOPPY)
      .StoreKeyedProperty(reg, reg, 0, LanguageMode::SLOPPY);

  // Emit binary operators invocations.
  builder.BinaryOperation(Token::Value::ADD, reg)
      .BinaryOperation(Token::Value::SUB, reg)
      .BinaryOperation(Token::Value::MUL, reg)
      .BinaryOperation(Token::Value::DIV, reg)
      .BinaryOperation(Token::Value::MOD, reg);

  // Emit control flow. Return must be the last instruction.
  builder.Return();

  // Generate BytecodeArray.
  Handle<BytecodeArray> the_array = builder.ToBytecodeArray();
  CHECK_EQ(the_array->frame_size(), builder.locals_count() * kPointerSize);

  // Build scorecard of bytecodes encountered in the BytecodeArray.
  std::vector<int> scorecard(Bytecodes::ToByte(Bytecode::kLast) + 1);
  Bytecode final_bytecode = Bytecode::kLdaZero;
  for (int i = 0; i < the_array->length(); i++) {
    uint8_t code = the_array->get(i);
    scorecard[code] += 1;
    int operands = Bytecodes::NumberOfOperands(Bytecodes::FromByte(code));
    CHECK_LE(operands, Bytecodes::MaximumNumberOfOperands());
    final_bytecode = Bytecodes::FromByte(code);
    i += operands;
  }

  // Check return occurs at the end and only once in the BytecodeArray.
  CHECK_EQ(final_bytecode, Bytecode::kReturn);
  CHECK_EQ(scorecard[Bytecodes::ToByte(final_bytecode)], 1);

#define CHECK_BYTECODE_PRESENT(Name, ...)     \
  /* Check Bytecode is marked in scorecard */ \
  CHECK_GE(scorecard[Bytecodes::ToByte(Bytecode::k##Name)], 1);
  BYTECODE_LIST(CHECK_BYTECODE_PRESENT)
#undef CHECK_BYTECODE_PRESENT
}


TEST_F(BytecodeArrayBuilderTest, FrameSizesLookGood) {
  for (int locals = 0; locals < 5; locals++) {
    for (int temps = 0; temps < 3; temps++) {
      BytecodeArrayBuilder builder(isolate(), zone());
      builder.set_parameter_count(0);
      builder.set_locals_count(locals);
      builder.Return();

      TemporaryRegisterScope temporaries(&builder);
      for (int i = 0; i < temps; i++) {
        temporaries.NewRegister();
      }

      Handle<BytecodeArray> the_array = builder.ToBytecodeArray();
      int total_registers = locals + temps;
      CHECK_EQ(the_array->frame_size(), total_registers * kPointerSize);
    }
  }
}


TEST_F(BytecodeArrayBuilderTest, TemporariesRecycled) {
  BytecodeArrayBuilder builder(isolate(), zone());
  builder.set_parameter_count(0);
  builder.set_locals_count(0);
  builder.Return();

  int first;
  {
    TemporaryRegisterScope temporaries(&builder);
    first = temporaries.NewRegister().index();
    temporaries.NewRegister();
    temporaries.NewRegister();
    temporaries.NewRegister();
  }

  int second;
  {
    TemporaryRegisterScope temporaries(&builder);
    second = temporaries.NewRegister().index();
  }

  CHECK_EQ(first, second);
}


TEST_F(BytecodeArrayBuilderTest, RegisterValues) {
  int index = 1;
  uint8_t operand = static_cast<uint8_t>(-index);

  Register the_register(index);
  CHECK_EQ(the_register.index(), index);

  int actual_operand = the_register.ToOperand();
  CHECK_EQ(actual_operand, operand);

  int actual_index = Register::FromOperand(actual_operand).index();
  CHECK_EQ(actual_index, index);
}


TEST_F(BytecodeArrayBuilderTest, Parameters) {
  BytecodeArrayBuilder builder(isolate(), zone());
  builder.set_parameter_count(10);
  builder.set_locals_count(0);

  Register param0(builder.Parameter(0));
  Register param9(builder.Parameter(9));
  CHECK_EQ(param9.index() - param0.index(), 9);
}


TEST_F(BytecodeArrayBuilderTest, Constants) {
  BytecodeArrayBuilder builder(isolate(), zone());
  builder.set_parameter_count(0);
  builder.set_locals_count(0);

  Factory* factory = isolate()->factory();
  Handle<HeapObject> heap_num_1 = factory->NewHeapNumber(3.14);
  Handle<HeapObject> heap_num_2 = factory->NewHeapNumber(5.2);
  Handle<Object> large_smi(Smi::FromInt(0x12345678), isolate());
  Handle<HeapObject> heap_num_2_copy(*heap_num_2);
  builder.LoadLiteral(heap_num_1)
      .LoadLiteral(heap_num_2)
      .LoadLiteral(large_smi)
      .LoadLiteral(heap_num_1)
      .LoadLiteral(heap_num_1)
      .LoadLiteral(heap_num_2_copy);

  Handle<BytecodeArray> array = builder.ToBytecodeArray();
  // Should only have one entry for each identical constant.
  CHECK_EQ(array->constant_pool()->length(), 3);
}

}  // namespace interpreter
}  // namespace internal
}  // namespace v8
