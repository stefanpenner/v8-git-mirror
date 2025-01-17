// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/opcodes.h"
#include "src/compiler/operator.h"
#include "src/compiler/operator-properties.h"
#include "src/compiler/simplified-operator.h"
#include "src/types-inl.h"
#include "test/unittests/test-utils.h"

namespace v8 {
namespace internal {
namespace compiler {

// -----------------------------------------------------------------------------
// Pure operators.


namespace {

struct PureOperator {
  const Operator* (SimplifiedOperatorBuilder::*constructor)();
  IrOpcode::Value opcode;
  Operator::Properties properties;
  int value_input_count;
};


std::ostream& operator<<(std::ostream& os, const PureOperator& pop) {
  return os << IrOpcode::Mnemonic(pop.opcode);
}


const PureOperator kPureOperators[] = {
#define PURE(Name, properties, input_count)              \
  {                                                      \
    &SimplifiedOperatorBuilder::Name, IrOpcode::k##Name, \
        Operator::kPure | properties, input_count        \
  }
    PURE(BooleanNot, Operator::kNoProperties, 1),
    PURE(BooleanToNumber, Operator::kNoProperties, 1),
    PURE(NumberEqual, Operator::kCommutative, 2),
    PURE(NumberLessThan, Operator::kNoProperties, 2),
    PURE(NumberLessThanOrEqual, Operator::kNoProperties, 2),
    PURE(NumberAdd, Operator::kCommutative, 2),
    PURE(NumberSubtract, Operator::kNoProperties, 2),
    PURE(NumberMultiply, Operator::kCommutative, 2),
    PURE(NumberDivide, Operator::kNoProperties, 2),
    PURE(NumberModulus, Operator::kNoProperties, 2),
    PURE(NumberToInt32, Operator::kNoProperties, 1),
    PURE(NumberToUint32, Operator::kNoProperties, 1),
    PURE(PlainPrimitiveToNumber, Operator::kNoProperties, 1),
    PURE(StringEqual, Operator::kCommutative, 2),
    PURE(StringLessThan, Operator::kNoProperties, 2),
    PURE(StringLessThanOrEqual, Operator::kNoProperties, 2),
    PURE(ChangeTaggedToInt32, Operator::kNoProperties, 1),
    PURE(ChangeTaggedToUint32, Operator::kNoProperties, 1),
    PURE(ChangeTaggedToFloat64, Operator::kNoProperties, 1),
    PURE(ChangeInt32ToTagged, Operator::kNoProperties, 1),
    PURE(ChangeUint32ToTagged, Operator::kNoProperties, 1),
    PURE(ChangeFloat64ToTagged, Operator::kNoProperties, 1),
    PURE(ChangeBoolToBit, Operator::kNoProperties, 1),
    PURE(ChangeBitToBool, Operator::kNoProperties, 1),
    PURE(ObjectIsSmi, Operator::kNoProperties, 1)
#undef PURE
};

}  // namespace


class SimplifiedPureOperatorTest
    : public TestWithZone,
      public ::testing::WithParamInterface<PureOperator> {};


TEST_P(SimplifiedPureOperatorTest, InstancesAreGloballyShared) {
  const PureOperator& pop = GetParam();
  SimplifiedOperatorBuilder simplified1(zone());
  SimplifiedOperatorBuilder simplified2(zone());
  EXPECT_EQ((simplified1.*pop.constructor)(), (simplified2.*pop.constructor)());
}


TEST_P(SimplifiedPureOperatorTest, NumberOfInputsAndOutputs) {
  SimplifiedOperatorBuilder simplified(zone());
  const PureOperator& pop = GetParam();
  const Operator* op = (simplified.*pop.constructor)();

  EXPECT_EQ(pop.value_input_count, op->ValueInputCount());
  EXPECT_EQ(0, op->EffectInputCount());
  EXPECT_EQ(0, op->ControlInputCount());
  EXPECT_EQ(pop.value_input_count, OperatorProperties::GetTotalInputCount(op));

  EXPECT_EQ(1, op->ValueOutputCount());
  EXPECT_EQ(0, op->EffectOutputCount());
  EXPECT_EQ(0, op->ControlOutputCount());
}


TEST_P(SimplifiedPureOperatorTest, OpcodeIsCorrect) {
  SimplifiedOperatorBuilder simplified(zone());
  const PureOperator& pop = GetParam();
  const Operator* op = (simplified.*pop.constructor)();
  EXPECT_EQ(pop.opcode, op->opcode());
}


TEST_P(SimplifiedPureOperatorTest, Properties) {
  SimplifiedOperatorBuilder simplified(zone());
  const PureOperator& pop = GetParam();
  const Operator* op = (simplified.*pop.constructor)();
  EXPECT_EQ(pop.properties, op->properties() & pop.properties);
}

INSTANTIATE_TEST_CASE_P(SimplifiedOperatorTest, SimplifiedPureOperatorTest,
                        ::testing::ValuesIn(kPureOperators));


// -----------------------------------------------------------------------------
// Buffer access operators.


namespace {

const ExternalArrayType kExternalArrayTypes[] = {
    kExternalUint8Array,   kExternalInt8Array,   kExternalUint16Array,
    kExternalInt16Array,   kExternalUint32Array, kExternalInt32Array,
    kExternalFloat32Array, kExternalFloat64Array};

}  // namespace


class SimplifiedBufferAccessOperatorTest
    : public TestWithZone,
      public ::testing::WithParamInterface<ExternalArrayType> {};


TEST_P(SimplifiedBufferAccessOperatorTest, InstancesAreGloballyShared) {
  BufferAccess const access(GetParam());
  SimplifiedOperatorBuilder simplified1(zone());
  SimplifiedOperatorBuilder simplified2(zone());
  EXPECT_EQ(simplified1.LoadBuffer(access), simplified2.LoadBuffer(access));
  EXPECT_EQ(simplified1.StoreBuffer(access), simplified2.StoreBuffer(access));
}


TEST_P(SimplifiedBufferAccessOperatorTest, LoadBuffer) {
  SimplifiedOperatorBuilder simplified(zone());
  BufferAccess const access(GetParam());
  const Operator* op = simplified.LoadBuffer(access);

  EXPECT_EQ(IrOpcode::kLoadBuffer, op->opcode());
  EXPECT_EQ(Operator::kNoThrow | Operator::kNoWrite, op->properties());
  EXPECT_EQ(access, BufferAccessOf(op));

  EXPECT_EQ(3, op->ValueInputCount());
  EXPECT_EQ(1, op->EffectInputCount());
  EXPECT_EQ(1, op->ControlInputCount());
  EXPECT_EQ(5, OperatorProperties::GetTotalInputCount(op));

  EXPECT_EQ(1, op->ValueOutputCount());
  EXPECT_EQ(1, op->EffectOutputCount());
  EXPECT_EQ(0, op->ControlOutputCount());
}


TEST_P(SimplifiedBufferAccessOperatorTest, StoreBuffer) {
  SimplifiedOperatorBuilder simplified(zone());
  BufferAccess const access(GetParam());
  const Operator* op = simplified.StoreBuffer(access);

  EXPECT_EQ(IrOpcode::kStoreBuffer, op->opcode());
  EXPECT_EQ(Operator::kNoRead | Operator::kNoThrow, op->properties());
  EXPECT_EQ(access, BufferAccessOf(op));

  EXPECT_EQ(4, op->ValueInputCount());
  EXPECT_EQ(1, op->EffectInputCount());
  EXPECT_EQ(1, op->ControlInputCount());
  EXPECT_EQ(6, OperatorProperties::GetTotalInputCount(op));

  EXPECT_EQ(0, op->ValueOutputCount());
  EXPECT_EQ(1, op->EffectOutputCount());
  EXPECT_EQ(0, op->ControlOutputCount());
}


INSTANTIATE_TEST_CASE_P(SimplifiedOperatorTest,
                        SimplifiedBufferAccessOperatorTest,
                        ::testing::ValuesIn(kExternalArrayTypes));


// -----------------------------------------------------------------------------
// Element access operators.


namespace {

const ElementAccess kElementAccesses[] = {
    {kTaggedBase, FixedArray::kHeaderSize, Type::Any(), kMachAnyTagged},
    {kUntaggedBase, 0, Type::Any(), kMachInt8},
    {kUntaggedBase, 0, Type::Any(), kMachInt16},
    {kUntaggedBase, 0, Type::Any(), kMachInt32},
    {kUntaggedBase, 0, Type::Any(), kMachUint8},
    {kUntaggedBase, 0, Type::Any(), kMachUint16},
    {kUntaggedBase, 0, Type::Any(), kMachUint32},
    {kUntaggedBase, 0, Type::Signed32(), kMachInt8},
    {kUntaggedBase, 0, Type::Unsigned32(), kMachUint8},
    {kUntaggedBase, 0, Type::Signed32(), kMachInt16},
    {kUntaggedBase, 0, Type::Unsigned32(), kMachUint16},
    {kUntaggedBase, 0, Type::Signed32(), kMachInt32},
    {kUntaggedBase, 0, Type::Unsigned32(), kMachUint32},
    {kUntaggedBase, 0, Type::Number(), kRepFloat32},
    {kUntaggedBase, 0, Type::Number(), kRepFloat64},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Signed32(),
     kMachInt8},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Unsigned32(),
     kMachUint8},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Signed32(),
     kMachInt16},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Unsigned32(),
     kMachUint16},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Signed32(),
     kMachInt32},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Unsigned32(),
     kMachUint32},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Number(),
     kRepFloat32},
    {kTaggedBase, FixedTypedArrayBase::kDataOffset, Type::Number(),
     kRepFloat64}};

}  // namespace


class SimplifiedElementAccessOperatorTest
    : public TestWithZone,
      public ::testing::WithParamInterface<ElementAccess> {};


TEST_P(SimplifiedElementAccessOperatorTest, LoadElement) {
  SimplifiedOperatorBuilder simplified(zone());
  const ElementAccess& access = GetParam();
  const Operator* op = simplified.LoadElement(access);

  EXPECT_EQ(IrOpcode::kLoadElement, op->opcode());
  EXPECT_EQ(Operator::kNoThrow | Operator::kNoWrite, op->properties());
  EXPECT_EQ(access, ElementAccessOf(op));

  EXPECT_EQ(2, op->ValueInputCount());
  EXPECT_EQ(1, op->EffectInputCount());
  EXPECT_EQ(1, op->ControlInputCount());
  EXPECT_EQ(4, OperatorProperties::GetTotalInputCount(op));

  EXPECT_EQ(1, op->ValueOutputCount());
  EXPECT_EQ(1, op->EffectOutputCount());
  EXPECT_EQ(0, op->ControlOutputCount());
}


TEST_P(SimplifiedElementAccessOperatorTest, StoreElement) {
  SimplifiedOperatorBuilder simplified(zone());
  const ElementAccess& access = GetParam();
  const Operator* op = simplified.StoreElement(access);

  EXPECT_EQ(IrOpcode::kStoreElement, op->opcode());
  EXPECT_EQ(Operator::kNoRead | Operator::kNoThrow, op->properties());
  EXPECT_EQ(access, ElementAccessOf(op));

  EXPECT_EQ(3, op->ValueInputCount());
  EXPECT_EQ(1, op->EffectInputCount());
  EXPECT_EQ(1, op->ControlInputCount());
  EXPECT_EQ(5, OperatorProperties::GetTotalInputCount(op));

  EXPECT_EQ(0, op->ValueOutputCount());
  EXPECT_EQ(1, op->EffectOutputCount());
  EXPECT_EQ(0, op->ControlOutputCount());
}


INSTANTIATE_TEST_CASE_P(SimplifiedOperatorTest,
                        SimplifiedElementAccessOperatorTest,
                        ::testing::ValuesIn(kElementAccesses));

}  // namespace compiler
}  // namespace internal
}  // namespace v8
