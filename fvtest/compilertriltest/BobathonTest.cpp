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

#include "OpCodeTest.hpp"
#include "default_compiler.hpp"

// Bobathon Test Suite - Custom IL tree tests

class BobathonTest : public TRTest::JitTest {};
// Phase 1: Immediate Boundary Tests
TEST_F(BobathonTest, ImmediateBoundary_Max12Bit) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 4095)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(4095, entry_point(0));
    ASSERT_EQ(4096, entry_point(1));
    ASSERT_EQ(8190, entry_point(4095));
}

TEST_F(BobathonTest, ImmediateBoundary_JustOver12Bit) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 4096)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(4096, entry_point(0));
    ASSERT_EQ(4097, entry_point(1));
    ASSERT_EQ(8192, entry_point(4096));
}

TEST_F(BobathonTest, ImmediateBoundary_ShiftedMax) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 16773120)))))");  // 0xFFF000 = 4095 << 12

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(16773120, entry_point(0));
    ASSERT_EQ(16773121, entry_point(1));
}

TEST_F(BobathonTest, ImmediateBoundary_NegativeMax12Bit) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst -4095)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-4095, entry_point(0));
    ASSERT_EQ(-4094, entry_point(1));
    ASSERT_EQ(0, entry_point(4095));
}

TEST_F(BobathonTest, ImmediateBoundary_NegativeJustOver) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst -4096)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-4096, entry_point(0));
    ASSERT_EQ(-4095, entry_point(1));
    ASSERT_EQ(0, entry_point(4096));
}

// Phase 2: Overflow and Edge Value Tests
TEST_F(BobathonTest, EdgeValue_AddInt32Min) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst -2147483648)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-2147483648, entry_point(0));
    ASSERT_EQ(-2147483647, entry_point(1));
    ASSERT_EQ(0, entry_point(-2147483648));  // Overflow wraps
}

TEST_F(BobathonTest, EdgeValue_SubInt32Min) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (isub"
        "        (iload parm=0)"
        "        (iconst -2147483648)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-2147483648, entry_point(0));  // 0 - INT_MIN = INT_MIN (overflow)
    ASSERT_EQ(-2147483647, entry_point(1));
}

TEST_F(BobathonTest, EdgeValue_NegateInt32Min) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32"
        "  (block"
        "    (ireturn"
        "      (ineg"
        "        (iconst -2147483648)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)()>();
    ASSERT_EQ(-2147483648, entry_point());  // -INT_MIN = INT_MIN (overflow)
}

TEST_F(BobathonTest, EdgeValue_MulInt32Max) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (imul"
        "        (iload parm=0)"
        "        (iconst 2147483647)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(0));
    ASSERT_EQ(2147483647, entry_point(1));
    ASSERT_EQ(-2147483647, entry_point(-1));
    ASSERT_EQ(-2, entry_point(2));  // Overflow
}

// Phase 3: Shift Edge Cases
TEST_F(BobathonTest, Shift_ByZero) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (ishl"
        "        (iload parm=0)"
        "        (iconst 0)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(42, entry_point(42));
    ASSERT_EQ(-1, entry_point(-1));
}

TEST_F(BobathonTest, Shift_By31) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (ishl"
        "        (iload parm=0)"
        "        (iconst 31)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(0));
    ASSERT_EQ(-2147483648, entry_point(1));  // 1 << 31 = INT_MIN
    ASSERT_EQ(0, entry_point(2));  // Overflow
}

TEST_F(BobathonTest, Shift_ArithmeticRightNegative) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32"
        "  (block"
        "    (ireturn"
        "      (ishr"
        "        (iconst -1)"
        "        (iconst 31)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)()>();
    ASSERT_EQ(-1, entry_point());  // -1 >> 31 = -1 (sign extension)
}

TEST_F(BobathonTest, Shift_LogicalRightNegative) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32"
        "  (block"
        "    (ireturn"
        "      (iushr"
        "        (iconst -1)"
        "        (iconst 31)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)()>();
    ASSERT_EQ(1, entry_point());  // -1 >>> 31 = 1 (logical shift)
}

// Phase 4: Division Edge Cases
TEST_F(BobathonTest, Division_ByPowerOf2) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (idiv"
        "        (iload parm=0)"
        "        (iconst 8)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(5, entry_point(40));
    ASSERT_EQ(-5, entry_point(-40));
    ASSERT_EQ(0, entry_point(7));
    ASSERT_EQ(0, entry_point(-7));
}

TEST_F(BobathonTest, Division_MinByNegOne) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32"
        "  (block"
        "    (ireturn"
        "      (idiv"
        "        (iconst -2147483648)"
        "        (iconst -1)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)()>();
    ASSERT_EQ(-2147483648, entry_point());  // INT_MIN / -1 = INT_MIN (overflow)
}

TEST_F(BobathonTest, Division_64BitMinByNegOne) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64"
        "  (block"
        "    (lreturn"
        "      (ldiv"
        "        (lconst -9223372036854775808)"
        "        (lconst -1)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)()>();
    ASSERT_EQ(-9223372036854775808LL, entry_point());  // LONG_MIN / -1 = LONG_MIN (overflow)
}

TEST_F(BobathonTest, Remainder_ByPowerOf2) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (irem"
        "        (iload parm=0)"
        "        (iconst 8)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(40));
    ASSERT_EQ(7, entry_point(47));
    ASSERT_EQ(0, entry_point(-40));
    ASSERT_EQ(-7, entry_point(-47));
}

// Phase 5: Complex Expression Tests
TEST_F(BobathonTest, Complex_ChainedAdditions) {
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iadd"
        "          (iadd"
        "            (iload parm=0)"
        "            (iconst 4095))"
        "          (iconst 4096))"
        "        (iconst 16773120)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(16781311, entry_point(0));  // 4095 + 4096 + 16773120
    ASSERT_EQ(16781312, entry_point(1));
}

TEST_F(BobathonTest, Complex_MixedOperations) {
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32, Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (imul"
        "          (ishl"
        "            (iload parm=0)"
        "            (iconst 2))"
        "          (iconst 3))"
        "        (isub"
        "          (iload parm=1)"
        "          (iconst 100))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t, int32_t)>();
    // (10 << 2) * 3 + (200 - 100) = 40 * 3 + 100 = 220
    ASSERT_EQ(220, entry_point(10, 200));
    // (5 << 2) * 3 + (50 - 100) = 20 * 3 + (-50) = 10
    ASSERT_EQ(10, entry_point(5, 50));
}

TEST_F(BobathonTest, Complex_NestedShifts) {
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (ishr"
        "        (ishl"
        "          (iload parm=0)"
        "          (iconst 16))"
        "        (iconst 16)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(42, entry_point(42));  // Sign extend lower 16 bits
    ASSERT_EQ(-1, entry_point(65535));  // 0xFFFF -> -1
    ASSERT_EQ(32767, entry_point(32767));  // 0x7FFF
}

// Phase 6: 64-bit Edge Cases
TEST_F(BobathonTest, Int64_ImmediateBoundaries) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64]"
        "  (block"
        "    (lreturn"
        "      (ladd"
        "        (lload parm=0)"
        "        (lconst 4095)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t)>();
    ASSERT_EQ(4095LL, entry_point(0LL));
    ASSERT_EQ(4096LL, entry_point(1LL));
}

TEST_F(BobathonTest, Int64_LargeConstant) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64]"
        "  (block"
        "    (lreturn"
        "      (ladd"
        "        (lload parm=0)"
        "        (lconst 1099511627776)))))");  // 2^40

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t)>();
    ASSERT_EQ(1099511627776LL, entry_point(0LL));
    ASSERT_EQ(1099511627777LL, entry_point(1LL));
}

TEST_F(BobathonTest, Int64_ShiftBy63) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64"
        "  (block"
        "    (lreturn"
        "      (lshl"
        "        (lconst 1)"
        "        (iconst 63)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)()>();
    ASSERT_EQ(-9223372036854775808LL, entry_point());  // 1 << 63 = LONG_MIN
}

TEST_F(BobathonTest, Int64_MultiplyOverflow) {
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64, Int64]"
        "  (block"
        "    (lreturn"
        "      (lmul"
        "        (lload parm=0)"
        "        (lload parm=1)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t, int64_t)>();
    ASSERT_EQ(0LL, entry_point(0LL, 1000000LL));
    ASSERT_EQ(1000000000000LL, entry_point(1000000LL, 1000000LL));
    ASSERT_EQ(-1000000000000LL, entry_point(-1000000LL, 1000000LL));
    // Overflow case: LONG_MAX * 2 = 0xFFFFFFFFFFFFFFFE = -2
    ASSERT_EQ(-2LL, entry_point(9223372036854775807LL, 2LL));
}

// Niche Edge Cases: Constant Materialization
TEST_F(BobathonTest, Niche_ConstantMaterialization_0xFFFF0000) {
    // Value: 0xFFFF0000 = -65536
    // Negation: 0x00010000 = 65536
    // Tests if negation logic correctly handles 16-bit chunk boundaries
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst -65536)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-65536, entry_point(0));
    ASSERT_EQ(0, entry_point(65536));
}

TEST_F(BobathonTest, Niche_ConstantMaterialization_0x0000FFFF) {
    // Value: 0x0000FFFF = 65535
    // Negation: 0xFFFF0001 = -65535
    // Opposite pattern from previous test
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 65535)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(65535, entry_point(0));
    ASSERT_EQ(131070, entry_point(65535));
}

TEST_F(BobathonTest, Niche_ConstantMaterialization_MultiChunk) {
    // Value: 0x12340000 - requires multiple chunks
    // Tests if multi-instruction constant loading works correctly
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 305397760)))))");  // 0x12340000

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(305397760, entry_point(0));
    ASSERT_EQ(305397761, entry_point(1));
}

TEST_F(BobathonTest, Niche_64BitConstant_ThreeChunks) {
    // 64-bit value with exactly 3 non-zero 16-bit chunks
    // Tests boundary between movz and movn selection
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64]"
        "  (block"
        "    (lreturn"
        "      (ladd"
        "        (lload parm=0)"
        "        (lconst 281474976710656)))))");  // 0x0001000000000000

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t)>();
    ASSERT_EQ(281474976710656LL, entry_point(0LL));
}

TEST_F(BobathonTest, Niche_64BitConstant_AlternatingPattern) {
    // Pattern: 0x00FF00FF00FF00FF
    // Tests if logical immediate encoding is considered
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int64]"
        "  (block"
        "    (lreturn"
        "      (land"
        "        (lload parm=0)"
        "        (lconst 71777214294589695)))))");  // 0x00FF00FF00FF00FF

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t)>();
    ASSERT_EQ(0LL, entry_point(0LL));
    ASSERT_EQ(71777214294589695LL, entry_point(-1LL));
}

TEST_F(BobathonTest, Niche_SubtractWithNegation) {
    // Subtract a value that benefits from negation
    // Tests if sub(x, const) correctly becomes add(x, -const)
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (isub"
        "        (iload parm=0)"
        "        (iconst 65536)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-65536, entry_point(0));
    ASSERT_EQ(0, entry_point(65536));
    ASSERT_EQ(-131072, entry_point(-65536));
}

TEST_F(BobathonTest, Niche_LogicalImmediate_Boundary) {
    // Test value that is a valid logical immediate
    // Pattern: 0x7FFFFFFF (all 1s except MSB)
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iand"
        "        (iload parm=0)"
        "        (iconst 2147483647)))))");  // INT32_MAX

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(0));
    ASSERT_EQ(2147483647, entry_point(-1));
    ASSERT_EQ(42, entry_point(42));
}

TEST_F(BobathonTest, Niche_RepeatingPattern_32Bit) {
    // Pattern: 0x01010101 - repeating byte
    // Tests if pattern recognition works
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (ior"
        "        (iload parm=0)"
        "        (iconst 16843009)))))");  // 0x01010101

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(16843009, entry_point(0));
    ASSERT_EQ(-1, entry_point(-1));
}

// Ultra-Niche: Register Reuse Conflicts
TEST_F(BobathonTest, UltraNiche_CommonedNode_BothSides) {
    // Use same computation on both sides of operation
    // Tests if register reuse handles commoned nodes correctly
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (imul"
        "          (iload parm=0)"
        "          (iconst 3))"
        "        (imul"
        "          (iload parm=0)"
        "          (iconst 3))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(0));
    ASSERT_EQ(60, entry_point(10));  // (10*3) + (10*3) = 60
    ASSERT_EQ(-60, entry_point(-10));
}

TEST_F(BobathonTest, UltraNiche_NestedRefCount1_Chain) {
    // Chain of operations where each intermediate has refcount=1
    // Tests if register reuse doesn't corrupt intermediate values
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iadd"
        "          (imul"
        "            (iload parm=0)"
        "            (iconst 2))"
        "          (iconst 5))"
        "        (isub"
        "          (imul"
        "            (iload parm=0)"
        "            (iconst 3))"
        "          (iconst 7))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    // (10*2 + 5) + (10*3 - 7) = 25 + 23 = 48
    ASSERT_EQ(48, entry_point(10));
    // (5*2 + 5) + (5*3 - 7) = 15 + 8 = 23
    ASSERT_EQ(23, entry_point(5));
}

TEST_F(BobathonTest, UltraNiche_ShiftedImmediate_Boundary_Add) {
    // Test add with value that's exactly at shifted immediate boundary
    // 0xFFF000 is max shifted immediate, 0x1000000 requires different encoding
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 16777216)))))");  // 0x1000000 = 4096 << 12

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(16777216, entry_point(0));
    ASSERT_EQ(16777217, entry_point(1));
}

TEST_F(BobathonTest, UltraNiche_ShiftedImmediate_JustUnder) {
    // 0xFFE000 = 4094 << 12 (just under max shifted)
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst 16769024)))))");  // 0xFFE000

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(16769024, entry_point(0));
}

TEST_F(BobathonTest, UltraNiche_NegatedConstant_EdgeCase) {
    // Value where negation changes instruction count
    // 0xFFFF0001 = -65535, negation = 0x0000FFFF = 65535
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (iadd"
        "        (iload parm=0)"
        "        (iconst -65535)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(-65535, entry_point(0));
    ASSERT_EQ(0, entry_point(65535));
}

TEST_F(BobathonTest, UltraNiche_MixedWidth_Conversion) {
    // Test where 32-bit operation feeds into 64-bit
    // Potential sign extension issues
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int32]"
        "  (block"
        "    (lreturn"
        "      (i2l"
        "        (iadd"
        "          (iload parm=0)"
        "          (iconst -1))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int32_t)>();
    ASSERT_EQ(-1LL, entry_point(0));
    ASSERT_EQ(0LL, entry_point(1));
    ASSERT_EQ(-2147483648LL, entry_point(-2147483647));  // Sign extend INT_MIN+1
}

TEST_F(BobathonTest, UltraNiche_DivisionRemainder_Negative) {
    // Division and remainder with negative numbers
    // Tests if sign handling is correct in optimized paths
    char inputTrees[512] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (irem"
        "        (iload parm=0)"
        "        (iconst -8)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(0, entry_point(0));
    ASSERT_EQ(3, entry_point(11));   // 11 % -8 = 3
    ASSERT_EQ(-3, entry_point(-11)); // -11 % -8 = -3
}

// Extreme Niche: Memory Reference Address Calculation Edge Cases
TEST_F(BobathonTest, ExtremeNiche_ArrayIndex_I2L_MaxShift) {
    // Array indexing with i2l conversion and maximum shift (scale=8 for 64-bit)
    // Tests sign extension in scaled index addressing
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Address, Int32]"
        "  (block"
        "    (lreturn"
        "      (lloadi"
        "        (aladd"
        "          (aload parm=0)"
        "          (lshl"
        "            (i2l (iload parm=1))"
        "            (iconst 3)))))))");  // scale=8 for 64-bit elements

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    int64_t array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t*, int32_t)>();
    ASSERT_EQ(0LL, entry_point(array, 0));
    ASSERT_EQ(5LL, entry_point(array, 5));
    ASSERT_EQ(9LL, entry_point(array, 9));
}

TEST_F(BobathonTest, ExtremeNiche_ArrayIndex_NegativeI2L) {
    // Negative 32-bit index converted to 64-bit with sign extension
    // Tests if sign extension is handled correctly in address calculation
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Address, Int32]"
        "  (block"
        "    (ireturn"
        "      (iloadi"
        "        (aladd"
        "          (aload parm=0)"
        "          (lshl"
        "            (i2l (iload parm=1))"
        "            (iconst 2)))))))");  // scale=4 for 32-bit elements

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    // Create array with guard values before and after
    int32_t buffer[20];
    for (int i = 0; i < 20; i++) buffer[i] = i * 100;
    int32_t *array = &buffer[10];  // Point to middle

    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t*, int32_t)>();
    ASSERT_EQ(1000, entry_point(array, 0));
    ASSERT_EQ(900, entry_point(array, -1));   // Negative index
    ASSERT_EQ(800, entry_point(array, -2));
}

TEST_F(BobathonTest, ExtremeNiche_ConstantFold_ShiftBothConst) {
    // Both operands of shift are constants - should fold into offset
    // Tests constant folding in memory reference
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Address]"
        "  (block"
        "    (ireturn"
        "      (iloadi"
        "        (aladd"
        "          (aload parm=0)"
        "          (lshl"
        "            (lconst 5)"
        "            (iconst 2)))))))");  // 5 << 2 = 20

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    int32_t array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t*)>();
    ASSERT_EQ(5, entry_point(array));  // array[5] because offset is 20 bytes = 5 ints
}

TEST_F(BobathonTest, ExtremeNiche_MaxScaleWithBoundary) {
    // Maximum scale (8) with index at boundary
    // Tests if scaled addressing handles large offsets correctly
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Address, Int32]"
        "  (block"
        "    (lreturn"
        "      (lloadi"
        "        (aladd"
        "          (aload parm=0)"
        "          (lshl"
        "            (i2l (iload parm=1))"
        "            (iconst 3)))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    int64_t array[512];
    for (int i = 0; i < 512; i++) array[i] = i * 1000;
    
    auto entry_point = compiler.getEntryPoint<int64_t (*)(int64_t*, int32_t)>();
    ASSERT_EQ(0LL, entry_point(array, 0));
    ASSERT_EQ(255000LL, entry_point(array, 255));
    ASSERT_EQ(511000LL, entry_point(array, 511));
}

TEST_F(BobathonTest, ExtremeNiche_I2L_SignExtend_Boundary) {
    // Test i2l with value at INT32_MAX boundary
    // Ensures sign extension doesn't corrupt address
    char inputTrees[1024] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int64 args=[Int32]"
        "  (block"
        "    (lreturn"
        "      (ladd"
        "        (i2l (iload parm=0))"
        "        (lconst 1000)))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    auto entry_point = compiler.getEntryPoint<int64_t (*)(int32_t)>();
    ASSERT_EQ(1000LL, entry_point(0));
    ASSERT_EQ(2147484647LL, entry_point(2147483647));  // INT32_MAX + 1000
    ASSERT_EQ(-2147482648LL, entry_point(-2147483648)); // INT32_MIN + 1000
}

TEST_F(BobathonTest, ExtremeNiche_ComplexAddressCalc_MultiLevel) {
    // Multi-level address calculation with multiple operations
    // Tests if complex address expressions are handled correctly
    char inputTrees[1536] = {0};
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Address, Int32, Int32]"
        "  (block"
        "    (ireturn"
        "      (iloadi"
        "        (aladd"
        "          (aladd"
        "            (aload parm=0)"
        "            (lshl"
        "              (i2l (iload parm=1))"
        "              (iconst 2)))"
        "          (lshl"
        "            (i2l (iload parm=2))"
        "            (iconst 2)))))))");

    auto trees = parseString(inputTrees);
    ASSERT_NOTNULL(trees);

    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly";

    int32_t array[100];
    for (int i = 0; i < 100; i++) array[i] = i;
    
    auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t*, int32_t, int32_t)>();
    ASSERT_EQ(0, entry_point(array, 0, 0));
    ASSERT_EQ(10, entry_point(array, 5, 5));  // array[5+5]
    ASSERT_EQ(15, entry_point(array, 10, 5)); // array[10+5]
}
