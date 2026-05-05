# Bobathon Codegen Bug Hunting Plan

## Objective
Create Tril tests that expose potential bugs in the OMR compiler's AArch64 code generation by targeting edge cases in evaluator implementations.

## Target Areas Identified

### 1. Immediate Value Boundary Cases (BinaryEvaluator.cpp)

**Code Location**: `genericBinaryEvaluator` function (lines 44-150)

**Vulnerability**: The code checks if constants fit in 12-bit unsigned immediate or 12-bit shifted immediate:
```cpp
if ((constantIsUnsignedImm12(value) || constantIsUnsignedImm12Shifted(value)) && regOp != regOpImm)
```

**Test Strategy**:
- Values at boundary: 4095 (0xFFF - max 12-bit), 4096 (just over)
- Shifted boundary: 4095 << 12 = 16773120 (0xFFF000)
- Values that look like they fit but don't: 4097, 8191
- Negative values near boundaries: -4096, -4095

**Expected Bugs**:
- Incorrect immediate encoding
- Wrong instruction selection (immediate vs register)
- Sign extension issues

### 2. Constant Negation Logic

**Code Location**: Lines 100-140 in `genericBinaryEvaluator`

**Vulnerability**: Code tries to negate constants to use opposite operations:
```cpp
if ((secondChild->getReferenceCount() == 1)
    && (regOp == TR::InstOpCode::addw || regOp == TR::InstOpCode::addx || ...)
    && (is64Bit ? shouldLoadNegatedConstant64(value) : shouldLoadNegatedConstant32(value)))
```

**Test Strategy**:
- INT32_MIN negation (overflow case)
- INT64_MIN negation (overflow case)
- Values where negation changes immediate eligibility
- Combinations: (x + INT32_MIN), (x - INT32_MIN)

**Expected Bugs**:
- Overflow in negation
- Wrong operation after negation
- Incorrect constant materialization

### 3. Internal Pointer Handling (aladd)

**Code Location**: Lines 65-75 in `genericBinaryEvaluator`

**Vulnerability**: Special logic for internal pointers:
```cpp
if ((1 == firstChild->getReferenceCount()) && node->isInternalPointer() && src1Reg->containsInternalPointer()
    && (node->getPinningArrayPointer() == src1Reg->getPinningArrayPointer()))
```

**Test Strategy**:
- aladd with various offset patterns
- Chained aladd operations
- aladd with large offsets
- aladd with negative offsets

**Expected Bugs**:
- Register reuse issues
- Incorrect pointer arithmetic
- GC map corruption

### 4. 64-bit Vector Multiplication

**Code Location**: `vmulInt64Helper` (lines 535-580)

**Vulnerability**: Complex multi-step algorithm:
```cpp
// AB * CD = B*D + 2^32*(A*D + B*C)
// Uses: vrev64, vmul, vushr, vadd, vshl, vuzp1, vumlal
```

**Test Strategy**:
- Edge values: 0, 1, -1, MAX, MIN
- Values that cause overflow in intermediate steps
- Patterns that might expose instruction ordering bugs
- Values where upper/lower 32-bit split matters

**Expected Bugs**:
- Incorrect intermediate results
- Wrong element ordering after shuffle
- Overflow in accumulation

### 5. Shift Operations with Edge Cases

**Code Location**: Various shift evaluators (lines 1470-1752)

**Vulnerability**: Shift amount validation and handling:
```cpp
// Shifts by constant vs variable
// Shifts by 0, 31, 32, 63, 64
```

**Test Strategy**:
- Shift by 0 (identity)
- Shift by width-1 (31 for int32, 63 for int64)
- Shift by width (32, 64 - undefined behavior)
- Shift by negative amount
- Shift by very large amount (> 64)

**Expected Bugs**:
- Undefined behavior not handled
- Wrong masking of shift amount
- Incorrect instruction selection

### 6. Division by Powers of 2

**Code Location**: Division evaluators (lines 1179-1241)

**Vulnerability**: Optimization of division by constant powers of 2:

**Test Strategy**:
- Division by 1, 2, 4, 8, ..., 2^31
- Division by -1, -2, -4, ...
- Division of MIN values by -1 (overflow)
- Remainder operations with same values

**Expected Bugs**:
- Wrong optimization applied
- Sign handling in right shift
- Overflow in special cases

## Implementation Plan

### Phase 1: Immediate Boundary Tests
```tril
(method return=Int32 args=[Int32]
  (block
    (ireturn
      (iadd
        (iload parm=0)
        (iconst 4095)))))  // Max 12-bit immediate

(method return=Int32 args=[Int32]
  (block
    (ireturn
      (iadd
        (iload parm=0)
        (iconst 4096)))))  // Just over - should use register
```

### Phase 2: Overflow and Edge Value Tests
```tril
(method return=Int32 args=[Int32]
  (block
    (ireturn
      (iadd
        (iload parm=0)
        (iconst -2147483648)))))  // INT32_MIN

(method return=Int32
  (block
    (ireturn
      (ineg
        (iconst -2147483648)))))  // Negate INT32_MIN
```

### Phase 3: Complex Expression Tests
```tril
(method return=Int32 args=[Int32, Int32]
  (block
    (ireturn
      (iadd
        (imul
          (iload parm=0)
          (iconst 2147483647))  // INT32_MAX
        (iload parm=1)))))
```

### Phase 4: Shift Edge Cases
```tril
(method return=Int32 args=[Int32]
  (block
    (ireturn
      (ishl
        (iload parm=0)
        (iconst 32)))))  // Shift by width

(method return=Int32 args=[Int32]
  (block
    (ireturn
      (ishr
        (iconst -1)
        (iconst 31)))))  // Arithmetic shift of -1
```

### Phase 5: Vector Operation Edge Cases
```tril
(method return=Int64 args=[Int64, Int64]
  (block
    (lreturn
      (lmul
        (lload parm=0)
        (lconst -1)))))  // Multiply by -1

// Vector version
(method return=VectorInt64 args=[VectorInt64]
  (block
    (vreturn
      (vmul
        (vload parm=0)
        (vsplats (lconst -1))))))
```

## Test Execution Strategy

1. **Baseline Tests**: Run with known-good values to verify test infrastructure
2. **Edge Case Tests**: Run boundary values and document failures
3. **Stress Tests**: Combine multiple edge cases in single expression
4. **Optimization Level Tests**: Run same tests at different opt levels (cold/warm/hot)
5. **Cross-validation**: Compare results with reference implementation or interpreter

## Success Criteria

A test successfully exposes a bug if:
1. The IL tree is valid and well-formed
2. Compilation succeeds without assertion
3. Execution produces incorrect result compared to expected value
4. The bug is reproducible
5. The bug can be minimized to a simple test case

## Documentation Template

For each bug found:
```markdown
## Bug: [Short Description]

**Evaluator**: [e.g., BinaryEvaluator::iaddEvaluator]
**IL Pattern**: [Tril code]
**Expected Result**: [value]
**Actual Result**: [value]
**Root Cause**: [hypothesis]
**Minimal Reproduction**: [simplified test]
```

## Next Steps

1. Implement Phase 1 tests in BobathonTest.cpp
2. Run and document results
3. Iterate through remaining phases
4. Create bug reports for confirmed issues
5. Propose fixes or workarounds

---

## Lessons Learned: Writing Tril Tests

### Type System Rules (Critical!)

**Address vs Int64 Distinction**:
- `aladd` requires **Address** type for both operands
- Use `aload` for address parameters, not `lload`
- Shifts in address calculations must use `lshl` (64-bit), not `ishl` (32-bit)
- Example of CORRECT address calculation:
  ```tril
  (aladd
    (aload parm=0)
    (lshl
      (i2l (iload parm=1))
      (iconst 3)))
  ```

**Common Type Errors**:
- ❌ `(aladd (aload parm=0) (ishl ...))` - Wrong! ishl returns Int32
- ✅ `(aladd (aload parm=0) (lshl ...))` - Correct! lshl returns Int64
- ❌ `(iloadi (lsub (aload parm=0) ...))` - Wrong! lsub returns Int64, not Address
- ✅ `(iloadi (aladd (aload parm=0) ...))` - Correct! Use aladd for addresses

**Sign Extension with i2l**:
- `i2l` performs **sign extension** from 32-bit to 64-bit
- Negative 32-bit values become negative 64-bit values
- Use in scaled index addressing: `(lshl (i2l (iload parm=1)) (iconst 3))`
- Tests should verify sign extension works correctly with negative indices

### Constant Materialization Edge Cases

**16-bit Chunk Boundaries**:
- AArch64 loads constants in 16-bit chunks using MOVZ/MOVN/MOVK
- Critical values to test:
  - `0xFFFF0000` (-65536): Tests lower chunk = 0x0000
  - `0x0000FFFF` (65535): Tests lower chunk = 0xFFFF
  - `0x12340000`: Tests multi-chunk with zero lower chunk
  
**Negation Logic**:
- Compiler may negate constants to reduce instruction count
- `shouldLoadNegatedConstant32/64()` decides whether to use negation
- Test values where negation changes encoding:
  - `-65535` vs `65535`
  - `-65536` vs `65536`
  - Values with different chunk patterns when negated

**Shifted Immediate Boundaries**:
- 12-bit immediate: 0-4095 (0x000-0xFFF)
- 12-bit shifted: (0-4095) << 12 = 0x000000-0xFFF000
- Test boundaries:
  - `4095` (max unshifted)
  - `4096` (requires different encoding)
  - `16773120` (0xFFF000 - max shifted)
  - `16777216` (0x1000000 - over max shifted)

### Register Reuse Patterns

**Reference Count = 1 Optimization**:
- When `node->getReferenceCount() == 1`, compiler may reuse register
- Test scenarios:
  - Commoned nodes (same computation used multiple times)
  - Chains where each intermediate has refcount=1
  - Mixed refcounts in expression tree

**Internal Pointer Handling**:
- `aladd` with internal pointers has special register allocation
- Must preserve GC map correctness
- Test with chained address calculations

### Memory Reference Address Calculation

**Scaled Index Addressing**:
- Scale factors: 1, 2, 4, 8 (for byte, short, int, long)
- Represented as shift: scale=8 → `(lshl index (iconst 3))`
- Test with:
  - Maximum scale (8 for 64-bit elements)
  - Negative indices (with sign extension)
  - Large indices (boundary of addressing mode)

**Constant Folding in Addresses**:
- Both operands constant: `(lshl (lconst 5) (iconst 2))` → offset 20
- Should fold into immediate offset, not generate shift instruction
- Test to verify optimization happens

**Complex Address Expressions**:
- Multi-level: `(aladd (aladd base idx1) idx2)`
- Compiler must consolidate into valid addressing mode
- Test register allocation doesn't corrupt intermediate values

### Test Design Best Practices

**1. Start Simple, Add Complexity**:
```tril
// Simple: Single operation
(iadd (iload parm=0) (iconst 4095))

// Medium: Two operations
(iadd (imul (iload parm=0) (iconst 3)) (iconst 5))

// Complex: Nested with multiple paths
(iadd
  (iadd (imul (iload parm=0) (iconst 2)) (iconst 5))
  (isub (imul (iload parm=0) (iconst 3)) (iconst 7)))
```

**2. Test Boundary Values Systematically**:
- Zero, One, Minus One
- MIN, MAX for each type
- Powers of 2: 1, 2, 4, 8, ..., 2^31, 2^63
- Boundary ± 1: 4095, 4096, 4097

**3. Verify Expected Behavior**:
```cpp
auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();
ASSERT_EQ(expected, entry_point(input));
// Test multiple inputs to verify pattern
ASSERT_EQ(expected2, entry_point(input2));
ASSERT_EQ(expected3, entry_point(input3));
```

**4. Use Descriptive Test Names**:
- `Niche_ConstantMaterialization_0xFFFF0000` - Clear what's being tested
- `UltraNiche_CommonedNode_BothSides` - Indicates complexity level
- `ExtremeNiche_ArrayIndex_I2L_MaxShift` - Multiple aspects in name

### Common Pitfalls to Avoid

1. **Type Mismatches**: Always verify IL tree types match operation requirements
2. **Undefined Behavior**: Don't assume compiler handles undefined cases (e.g., shift by 32)
3. **Platform Assumptions**: Test values may behave differently on different architectures
4. **Optimization Levels**: Same IL may generate different code at different opt levels
5. **Array Bounds**: When testing with arrays, ensure indices are in bounds

### Debugging Failed Tests

**Compilation Errors**:
```
*** VALIDATION ERROR ***
Node: aladd n5n
Child 1 has unexpected type Int32 (expected Int64)
```
→ Fix: Change `ishl` to `lshl` or add `i2l` conversion

**Wrong Results**:
1. Print generated assembly: Use compiler debug flags
2. Verify IL tree is what you intended: Check parseString output
3. Test with simpler inputs: Reduce to minimal failing case
4. Compare with interpreter: Run same IL in interpreter mode

### Reference Count Testing: Critical Distinction

**The Problem with parseString() for Commoning**:

When using Tril string parsing, identical subtrees create **separate nodes**:
```tril
(iadd
  (imul (iload parm=0) (iconst 3))
  (imul (iload parm=0) (iconst 3)))
```
- Parser creates **TWO** `imul` nodes
- Each has `refCount=1`
- **NOT** testing true commoned node behavior
- Does **NOT** exercise `getReferenceCount() > 1` code paths

**The Solution: Programmatic IL Generation**:

Use IlBuilder API to create truly commoned nodes:
```cpp
class CommonedMulMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
    bool buildIL() {
        // Create computation ONCE
        TR::IlValue* commonedMul = Mul(Load("x"), ConstInt32(3));
        
        // Use it TWICE - creates refCount=2
        Return(Add(commonedMul, commonedMul));
        return true;
    }
};
```

**Why This Matters**:

Evaluator code frequently checks reference counts:
```cpp
// From BinaryEvaluator.cpp
if (firstChild->getReferenceCount() == 1) {
    trgReg = src1Reg;  // Reuse register
} else {
    trgReg = cg->allocateRegister();  // Allocate new
}
```

**String-based tests NEVER exercise the `else` branch!**

**Test Categories for Reference Count Testing**:

1. **Simple Commoning** (refCount=2):
   - Same computation used twice
   - Tests basic register reuse logic

2. **High Reference Counts** (refCount=3+):
   - Node used 3+ times
   - Tests register allocation under pressure

3. **Nested Commoning**:
   - Commoned node contains commoned child
   - Tests complex reference count scenarios

4. **Commoning with Edge Values**:
   - Boundary immediates (4095, 65536)
   - Negation-eligible constants
   - Tests interaction with constant materialization

5. **Commoning with Operations**:
   - Shifts, divisions, multiplications
   - Tests operation-specific register reuse

**Implementation Template**:

```cpp
// In test file (e.g., BobathonRefCountTest.cpp)
class MyCommonedMethod : public StaticSignatureMethodBuilder<int32_t(int32_t)> {
public:
    MyCommonedMethod(TR::TypeDictionary *types);
    virtual bool buildIL();
};

MyCommonedMethod::MyCommonedMethod(TR::TypeDictionary *types)
    : StaticSignatureMethodBuilder<int32_t(int32_t)>(types) {
    DefineLine(LINETOSTR(__LINE__));
    DefineFile(__FILE__);
    DefineName("MyCommoned");
    DefineParameter("x", Int32);
    DefineReturnType(Int32);
}

bool MyCommonedMethod::buildIL() {
    TR::IlValue* commoned = /* your computation */;
    Return(/* use commoned multiple times */);
    return true;
}

TEST_F(MyTest, TestName) {
    TR::TypeDictionary types;
    MyCommonedMethod method(&types);
    auto entry = method.Compile();
    ASSERT_NOTNULL(entry);
    
    ASSERT_EQ(expected, entry(input));
}
```

**Key Differences Summary**:

| Aspect | parseString() | IlBuilder API |
|--------|---------------|---------------|
| Node Creation | Separate nodes per occurrence | Single node, multiple uses |
| Reference Count | Always 1 per node | Accurate (2, 3, 4+) |
| Register Reuse | Never tested | Properly tested |
| Use Case | General codegen validation | RefCount-specific testing |
| Complexity | Simple string syntax | More verbose C++ code |

**When to Use Each**:

- **parseString()**: General codegen, immediate boundaries, type conversions, simple operations
- **IlBuilder API**: Reference count testing, register allocation, commoned node scenarios

**Example: BobathonRefCountTest.cpp**:

Created 8 tests using IlBuilder API:
1. CommonedNode_RegisterReuse - Basic refCount=2
2. CommonedNode_ImmediateBoundary - With boundary constant
3. TripleCommoned_HighRefCount - refCount=3
4. CommonedShift_RegisterReuse - Shift operation
5. NestedCommoned_ComplexRefCounts - Nested commoning
6. CommonedNegation_ConstantMaterialization - With negation
7. Commoned64Bit_LargeConstants - 64-bit operations
8. CommonedDivision_PowerOf2 - Division optimization

All tests validate true commoned node behavior that parseString() cannot test.

### Call Node Testing: Indirect Calls (icall/lcall)

**Purpose**: Test call evaluator codegen, argument passing, and register allocation around calls.

**Key Insights**:

1. **Call Syntax in Tril**:
   ```tril
   (icall address=0x%jX args=[Int32, Int32]
     (iload parm=0)
     (iload parm=1))
   ```
   - Must provide actual function address via `snprintf`
   - `args=[...]` specifies parameter types
   - Children are the actual arguments

2. **Helper Functions**:
   - Define C++ helper functions to call
   - Use `reinterpret_cast<uintmax_t>(&function)` for address
   - Helpers must match signature specified in `args=[...]`

3. **Test Categories**:
   - **Simple calls**: Basic argument passing
   - **Constant arguments**: Test immediate materialization before call
   - **Computed arguments**: Test expression evaluation before call
   - **Chained calls**: Test register allocation across multiple calls
   - **64-bit calls**: Test lcall with Int64 types

4. **Edge Cases to Test**:
   - Boundary immediates (4095, 65536) as arguments
   - Negative constants as arguments
   - Shifted/multiplied values as arguments
   - Multiple operations feeding into call arguments

5. **What Gets Tested**:
   - Argument register allocation (x0-x7 on AArch64)
   - Caller-saved register preservation
   - Stack alignment for calls
   - Return value handling
   - Interaction with constant materialization

**Example Test Pattern**:
```cpp
static int32_t helper(int32_t a, int32_t b) { return a + b; }

TEST_F(Test, CallWithBoundary) {
    char inputTrees[256];
    std::snprintf(inputTrees, sizeof(inputTrees),
        "(method return=Int32 args=[Int32]"
        "  (block"
        "    (ireturn"
        "      (icall address=0x%jX args=[Int32, Int32]"
        "        (iload parm=0)"
        "        (iconst 4095)))))",
        reinterpret_cast<uintmax_t>(&helper));
    
    auto trees = parseString(inputTrees);
    Tril::DefaultCompiler compiler(trees);
    ASSERT_EQ(0, compiler.compile());
    
    auto entry = compiler.getEntryPoint<int32_t (*)(int32_t)>();
    ASSERT_EQ(4095, entry(0));
}
```

**Implementation: BobathonCallTest.cpp**

Created 10 tests covering:
1. `ICall_Simple_TwoArgs` - Basic two-argument call
2. `ICall_WithConstantArg` - Constant immediate as argument
3. `ICall_BoundaryImmediate` - 12-bit boundary (4095)
4. `ICall_NegativeConstant` - Negative constant (-65536)
5. `ICall_MultipleOperations` - Computed arguments (mul, add)
6. `LCall_64Bit_Simple` - 64-bit call with two args
7. `LCall_LargeConstant` - Large 64-bit constant (1000000000)
8. `ICall_ChainedCalls` - Multiple calls in sequence
9. `ICall_WithShift` - Shifted value as argument
10. `LCall_WithMultiplication` - 64-bit multiplication call

**All 10 tests PASS** ✅

**Key Differences from arraycopy**:
- `arraycopy` is a special IL node with fastpath codegen
- `icall/lcall` are general indirect calls through evaluator
- `arraycopy` not supported in OMR (Java-specific)
- Use `icall/lcall` for testing general call mechanics

### Future Test Ideas

**Floating Point Edge Cases**:
- NaN, Infinity, -Infinity
- Denormalized numbers
- Rounding modes

**Vector Operations**:
- Lane-wise operations with edge values
- Shuffle patterns
- Reduction operations

**Optimization Levels**:
- Run all tests at cold/warm/hot
- Verify optimizations don't break correctness

**Cross-Platform**:
- Test on x86, ARM, Power
- Verify portable IL generates correct code everywhere
