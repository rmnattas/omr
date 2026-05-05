/*******************************************************************************
 * Copyright IBM Corp. and others 2024
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#include "JitTest.hpp"
#include "compile/Method.hpp"
#include "compile/ResolvedMethod.hpp"
#include "control/CompileMethod.hpp"
#include "compile/CompilationTypes.hpp"
#include "il/DataTypes.hpp"
#include "ilgen/IlGeneratorMethodDetails_inlines.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/MethodBuilder.hpp"

class BobathonRefCountTest : public TRTest::JitTest {};

template <typename S>
class StaticSignatureMethodBuilder : public TR::MethodBuilder {
public:
    typedef S* FunctionPtr;

    StaticSignatureMethodBuilder(TR::TypeDictionary* types)
        : TR::MethodBuilder(types) {}

    FunctionPtr Compile() {
        TR::ResolvedMethod resolvedMethod(this);
        TR::IlGeneratorMethodDetails details(&resolvedMethod);
        int32_t rc = 0;
        FunctionPtr entry = (FunctionPtr)(reinterpret_cast<void *>(compileMethodFromDetails(NULL, details, warm, rc)));
        return entry;
    }
};

// Test 1: True commoned node - same computation used twice
class CommonedMulMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    CommonedMulMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

CommonedMulMethod::CommonedMulMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("CommonedMul");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool CommonedMulMethod::buildIL() {
    // Create commoned computation: x * 3
    // This will have refCount=2 when used twice
    TR::IlValue* commonedMul = Mul(Load("x"), ConstInt32(3));
    
    // Use it twice: (x*3) + (x*3)
    // Tests register reuse when getReferenceCount() > 1
    Return(Add(commonedMul, commonedMul));
    return true;
}

TEST_F(BobathonRefCountTest, CommonedNode_RegisterReuse) {
    TR::TypeDictionary types;
    CommonedMulMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(0, entry(0));      // (0*3) + (0*3) = 0
    ASSERT_EQ(60, entry(10));    // (10*3) + (10*3) = 60
    ASSERT_EQ(-60, entry(-10));  // (-10*3) + (-10*3) = -60
}

// Test 2: Commoned node with constant boundary value
class CommonedBoundaryMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    CommonedBoundaryMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

CommonedBoundaryMethod::CommonedBoundaryMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("CommonedBoundary");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool CommonedBoundaryMethod::buildIL() {
    // Commoned add with boundary immediate (4095 = max 12-bit)
    TR::IlValue* commonedAdd = Add(Load("x"), ConstInt32(4095));
    
    // Use twice in subtraction: (x+4095) - (x+4095) = 0
    Return(Sub(commonedAdd, commonedAdd));
    return true;
}

TEST_F(BobathonRefCountTest, CommonedNode_ImmediateBoundary) {
    TR::TypeDictionary types;
    CommonedBoundaryMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    // Should always return 0 regardless of input
    ASSERT_EQ(0, entry(0));
    ASSERT_EQ(0, entry(100));
    ASSERT_EQ(0, entry(-100));
}

// Test 3: Triple commoning - node used 3 times
class TripleCommonedMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    TripleCommonedMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

TripleCommonedMethod::TripleCommonedMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("TripleCommoned");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool TripleCommonedMethod::buildIL() {
    // Create computation used 3 times (refCount=3)
    TR::IlValue* squared = Mul(Load("x"), Load("x"));
    
    // Use it 3 times: x^2 + x^2 + x^2 = 3*x^2
    Return(Add(Add(squared, squared), squared));
    return true;
}

TEST_F(BobathonRefCountTest, TripleCommoned_HighRefCount) {
    TR::TypeDictionary types;
    TripleCommonedMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(0, entry(0));       // 3*0^2 = 0
    ASSERT_EQ(3, entry(1));       // 3*1^2 = 3
    ASSERT_EQ(12, entry(2));      // 3*2^2 = 12
    ASSERT_EQ(75, entry(5));      // 3*5^2 = 75
    ASSERT_EQ(300, entry(10));    // 3*10^2 = 300
}

// Test 4: Commoned shift operation
class CommonedShiftMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    CommonedShiftMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

CommonedShiftMethod::CommonedShiftMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("CommonedShift");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool CommonedShiftMethod::buildIL() {
    // Commoned shift: x << 3 (multiply by 8)
    TR::IlValue* shifted = ShiftL(Load("x"), ConstInt32(3));
    
    // Use twice: (x<<3) + (x<<3) = 2*(x<<3) = x*16
    Return(Add(shifted, shifted));
    return true;
}

TEST_F(BobathonRefCountTest, CommonedShift_RegisterReuse) {
    TR::TypeDictionary types;
    CommonedShiftMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(0, entry(0));       // 0*16 = 0
    ASSERT_EQ(16, entry(1));      // 1*16 = 16
    ASSERT_EQ(160, entry(10));    // 10*16 = 160
    ASSERT_EQ(-160, entry(-10));  // -10*16 = -160
}

// Test 5: Nested commoning - commoned node contains commoned child
class NestedCommonedMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    NestedCommonedMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

NestedCommonedMethod::NestedCommonedMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("NestedCommoned");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool NestedCommonedMethod::buildIL() {
    // Inner commoned: x + 5
    TR::IlValue* innerCommoned = Add(Load("x"), ConstInt32(5));
    
    // Outer commoned: (x+5) * 2
    TR::IlValue* outerCommoned = Mul(innerCommoned, ConstInt32(2));
    
    // Use outer twice: ((x+5)*2) + ((x+5)*2) = 4*(x+5)
    Return(Add(outerCommoned, outerCommoned));
    return true;
}

TEST_F(BobathonRefCountTest, NestedCommoned_ComplexRefCounts) {
    TR::TypeDictionary types;
    NestedCommonedMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(20, entry(0));      // 4*(0+5) = 20
    ASSERT_EQ(40, entry(5));      // 4*(5+5) = 40
    ASSERT_EQ(60, entry(10));     // 4*(10+5) = 60
}

// Test 6: Commoned with negative constant that benefits from negation
class CommonedNegationMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    CommonedNegationMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

CommonedNegationMethod::CommonedNegationMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("CommonedNegation");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool CommonedNegationMethod::buildIL() {
    // Commoned add with -65536 (tests negation logic)
    TR::IlValue* commonedAdd = Add(Load("x"), ConstInt32(-65536));
    
    // Use twice: (x-65536) + (x-65536) = 2x - 131072
    Return(Add(commonedAdd, commonedAdd));
    return true;
}

TEST_F(BobathonRefCountTest, CommonedNegation_ConstantMaterialization) {
    TR::TypeDictionary types;
    CommonedNegationMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(-131072, entry(0));      // 2*0 - 131072 = -131072
    ASSERT_EQ(0, entry(65536));        // 2*65536 - 131072 = 0
    ASSERT_EQ(131072, entry(131072));  // 2*131072 - 131072 = 131072
}

// Test 7: Commoned 64-bit operation
class Commoned64BitMethod : public StaticSignatureMethodBuilder<int64_t(int64_t)> {
public:
    Commoned64BitMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

Commoned64BitMethod::Commoned64BitMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int64_t(int64_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("Commoned64Bit");
    DefineParameter("x", Int64);
    DefineReturnType(Int64);
}

bool Commoned64BitMethod::buildIL() {
    // Commoned 64-bit multiply
    TR::IlValue* commonedMul = Mul(Load("x"), ConstInt64(1000000LL));
    
    // Use twice
    Return(Add(commonedMul, commonedMul));
    return true;
}

TEST_F(BobathonRefCountTest, Commoned64Bit_LargeConstants) {
    TR::TypeDictionary types;
    Commoned64BitMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(0LL, entry(0LL));
    ASSERT_EQ(2000000LL, entry(1LL));
    ASSERT_EQ(2000000000LL, entry(1000LL));
}

// Test 8: Commoned division by power of 2
class CommonedDivisionMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    CommonedDivisionMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

CommonedDivisionMethod::CommonedDivisionMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("CommonedDivision");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool CommonedDivisionMethod::buildIL() {
    // Commoned division: x / 8
    TR::IlValue* commonedDiv = Div(Load("x"), ConstInt32(8));
    
    // Use twice: (x/8) + (x/8) = 2*(x/8)
    Return(Add(commonedDiv, commonedDiv));
    return true;
}

TEST_F(BobathonRefCountTest, CommonedDivision_PowerOf2) {
    TR::TypeDictionary types;
    CommonedDivisionMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(0, entry(0));       // 2*(0/8) = 0
    ASSERT_EQ(2, entry(8));       // 2*(8/8) = 2
    ASSERT_EQ(10, entry(40));     // 2*(40/8) = 10
    ASSERT_EQ(20, entry(80));     // 2*(80/8) = 20
}
