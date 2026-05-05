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
#include "default_compiler.hpp"

// Test indirect call (icall) nodes - tests call evaluator codegen
class BobathonCallTest : public TRTest::JitTest {};

// Helper functions for testing
static int32_t add_helper(int32_t a, int32_t b) { return a + b; }
static int32_t mul_helper(int32_t a, int32_t b) { return a * b; }
static int32_t sub_helper(int32_t a, int32_t b) { return a - b; }
static int64_t add64_helper(int64_t a, int64_t b) { return a + b; }
static int64_t mul64_helper(int64_t a, int64_t b) { return a * b; }

TEST_F(BobathonCallTest, ICall_Simple_TwoArgs) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32, Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (iload parm=0)"
        "        (iload parm=1)))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t, int32_t)>();
    
    ASSERT_EQ(5, entry_point(2, 3));
    ASSERT_EQ(0, entry_point(0, 0));
    ASSERT_EQ(-5, entry_point(-2, -3));
    ASSERT_EQ(100, entry_point(42, 58));
}

TEST_F(BobathonCallTest, ICall_WithConstantArg) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (iload parm=0)"
        "        (iconst 100)))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    
    ASSERT_EQ(100, entry_point(0));
    ASSERT_EQ(101, entry_point(1));
    ASSERT_EQ(150, entry_point(50));
    ASSERT_EQ(50, entry_point(-50));
}

TEST_F(BobathonCallTest, ICall_BoundaryImmediate) {
    // Test with 12-bit boundary immediate (4095)
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (iload parm=0)"
        "        (iconst 4095)))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    
    ASSERT_EQ(4095, entry_point(0));
    ASSERT_EQ(4096, entry_point(1));
    ASSERT_EQ(8190, entry_point(4095));
}

TEST_F(BobathonCallTest, ICall_NegativeConstant) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (iload parm=0)"
        "        (iconst -65536)))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    
    ASSERT_EQ(-65536, entry_point(0));
    ASSERT_EQ(-65535, entry_point(1));
    ASSERT_EQ(0, entry_point(65536));
}

TEST_F(BobathonCallTest, ICall_MultipleOperations) {
    // Test call with computed arguments
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32, Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (imul (iload parm=0) (iconst 2))"
        "        (iadd (iload parm=1) (iconst 5))))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t, int32_t)>();
    
    // (2*2) + (3+5) = 4 + 8 = 12
    ASSERT_EQ(12, entry_point(2, 3));
    // (10*2) + (0+5) = 20 + 5 = 25
    ASSERT_EQ(25, entry_point(10, 0));
}

TEST_F(BobathonCallTest, LCall_64Bit_Simple) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64, Int64]"
        "  (block"
        "    (lreturn"
        "      (lcall address=0x%jX args=[Int64, Int64]"
        "        (lload parm=0)"
        "        (lload parm=1)))))",
        reinterpret_cast<uintmax_t>(&add64_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t, int64_t)>();
    
    ASSERT_EQ(5LL, entry_point(2LL, 3LL));
    ASSERT_EQ(0LL, entry_point(0LL, 0LL));
    ASSERT_EQ(2000000000LL, entry_point(1000000000LL, 1000000000LL));
}

TEST_F(BobathonCallTest, LCall_LargeConstant) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64]"
        "  (block"
        "    (lreturn"
        "      (lcall address=0x%jX args=[Int64, Int64]"
        "        (lload parm=0)"
        "        (lconst 1000000000)))))",
        reinterpret_cast<uintmax_t>(&add64_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t)>();
    
    ASSERT_EQ(1000000000LL, entry_point(0LL));
    ASSERT_EQ(1000000001LL, entry_point(1LL));
    ASSERT_EQ(2000000000LL, entry_point(1000000000LL));
}

TEST_F(BobathonCallTest, ICall_ChainedCalls) {
    // Test multiple calls in sequence
    char inputTrees[512];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32, Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (icall address=0x%jX args=[Int32, Int32]"
        "          (iload parm=0)"
        "          (iload parm=1))"
        "        (iconst 10)))))",
        reinterpret_cast<uintmax_t>(&add_helper),
        reinterpret_cast<uintmax_t>(&mul_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t, int32_t)>();
    
    // (2*3) + 10 = 6 + 10 = 16
    ASSERT_EQ(16, entry_point(2, 3));
    // (5*4) + 10 = 20 + 10 = 30
    ASSERT_EQ(30, entry_point(5, 4));
}

TEST_F(BobathonCallTest, ICall_WithShift) {
    // Test call with shifted argument
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (ishl (iload parm=0) (iconst 3))"
        "        (iconst 5)))))",
        reinterpret_cast<uintmax_t>(&add_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    
    // (2<<3) + 5 = 16 + 5 = 21
    ASSERT_EQ(21, entry_point(2));
    // (10<<3) + 5 = 80 + 5 = 85
    ASSERT_EQ(85, entry_point(10));
}

TEST_F(BobathonCallTest, LCall_WithMultiplication) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64, Int64]"
        "  (block"
        "    (lreturn"
        "      (lcall address=0x%jX args=[Int64, Int64]"
        "        (lload parm=0)"
        "        (lload parm=1)))))",
        reinterpret_cast<uintmax_t>(&mul64_helper));

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t, int64_t)>();
    
    ASSERT_EQ(6LL, entry_point(2LL, 3LL));
    ASSERT_EQ(0LL, entry_point(0LL, 100LL));
    ASSERT_EQ(1000000LL, entry_point(1000LL, 1000LL));
}