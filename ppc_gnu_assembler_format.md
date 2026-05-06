# PowerPC GNU Assembler Format Reference

This document describes the assembly syntax required by the GNU `as` assembler for PowerPC instructions.

## Register Naming Convention

**Use bare register numbers (no prefix):**
- ✅ Correct: `1`, `2`, `3`, `31`
- ❌ Wrong: `r1`, `gr1`, `%r1`

**Examples:**
```asm
addc 1,2,3      # Add with carry: r1 = r2 + r3 + CA
ld 5,0(1)       # Load doubleword: r5 = mem[r1 + 0]
```

## Instruction Variants

### Record Bit (Rc) - Dot Suffix
Append `.` to instruction name to set the record bit (Rc=1), which updates condition register CR0.

```asm
addc 1,2,3      # Rc=0, no CR update
addc. 1,2,3     # Rc=1, updates CR0
```

### Overflow Enable (OE) Bit
Some instructions have `o` suffix for overflow enable:
```asm
addc 1,2,3      # OE=0
addco 1,2,3     # OE=1
addco. 1,2,3    # OE=1, Rc=1
```

## Operand Formats

### 1. Three-Register Format (X-form, XO-form)
```asm
instruction target,source1,source2
```
**Examples:**
```asm
add 1,2,3       # r1 = r2 + r3
subf 4,5,6      # r4 = r6 - r5 (note: reversed operands)
and 7,8,9       # r7 = r8 & r9
```

### 2. Two-Register + Immediate (D-form)
```asm
instruction target,source,immediate
```
**Examples:**
```asm
addi 1,2,100        # r1 = r2 + 100
addis 1,2,0x1234    # r1 = r2 + (0x1234 << 16)
ori 3,4,0xFF        # r3 = r4 | 0xFF
```

**Immediate Value Formats:**
- Decimal: `100`, `-50`
- Hexadecimal: `0x1234`, `0xFFFF`
- Binary: `0b1010` (if supported)

### 3. Memory Reference Format (D-form Load/Store)
```asm
instruction register,offset(base_register)
```
**Examples:**
```asm
ld 1,16(2)      # r1 = mem64[r2 + 16]
std 1,32(2)     # mem64[r2 + 32] = r1
lbz 3,0(4)      # r3 = mem8[r4 + 0] (zero-extended)
stw 5,8(6)      # mem32[r6 + 8] = r5
```

**Offset Range:**
- Signed 16-bit: -32768 to 32767
- Can use hex: `ld 1,0x10(2)`

### 4. Rotate/Shift Instructions (M-form)
```asm
rlwinm target,source,shift,mask_begin,mask_end
```
**Examples:**
```asm
rlwinm 1,2,5,0,31       # Rotate r2 left by 5, mask bits 0-31 → r1
rlwinm. 1,2,5,0,31      # Same with Rc=1
slwi 1,2,3              # Shift left word immediate (pseudo-op)
```

**Parameters:**
- `shift`: Rotation amount (0-31)
- `mask_begin`: Start bit of mask (0-31)
- `mask_end`: End bit of mask (0-31)

### 5. Compare Instructions
```asm
cmpw crfield,reg1,reg2          # Compare word
cmpwi crfield,register,immediate # Compare word immediate
```
**Examples:**
```asm
cmpwi 0,1,100       # Compare r1 with 100, result in CR0
cmpw 1,2,3          # Compare r2 with r3, result in CR1
cmpdi 0,4,-1        # Compare doubleword r4 with -1
```

**CR Field:**
- 0-7: Condition register field number
- Field 0 is default for most operations

### 6. Branch Instructions
```asm
b target            # Unconditional branch
bc BO,BI,target     # Conditional branch
```
**Examples:**
```asm
b loop_start        # Branch to label
bc 12,2,target      # Branch if CR bit 2 is set
beq target          # Branch if equal (pseudo-op)
bne target          # Branch if not equal (pseudo-op)
```

**Branch Operands:**
- `BO`: Branch options (5-bit field)
- `BI`: Condition register bit (0-31)
- `target`: Label or address

### 7. Special Instructions
```asm
nop                 # No operation
mtctr 1             # Move to count register
mflr 2              # Move from link register
```

## Condition Register Fields

The condition register (CR) has 8 fields (CR0-CR7), each 4 bits:
- Bit 0: Less than (LT)
- Bit 1: Greater than (GT)
- Bit 2: Equal (EQ)
- Bit 3: Summary overflow (SO)

**Example:**
```asm
cmpwi 0,1,100       # Compare r1 with 100
blt target          # Branch if less than (checks CR0[LT])
```

## Labels and Comments

### Labels
```asm
label_name:         # Label definition (colon required)
    instruction
```

### Comments
```asm
# This is a comment
addi 1,2,100        # Inline comment
```

## Complete Example

```asm
.text
.globl _start

_start:
    # Load immediate
    li 3,100            # r3 = 100 (pseudo-op: addi 3,0,100)
    
    # Arithmetic
    addi 4,3,50         # r4 = r3 + 50 = 150
    add 5,3,4           # r5 = r3 + r4 = 250
    
    # Memory operations
    std 5,0(1)          # Store r5 to stack
    ld 6,0(1)           # Load back to r6
    
    # Compare and branch
    cmpwi 0,6,250       # Compare r6 with 250
    beq equal_label     # Branch if equal
    
    # Rotate
    rlwinm 7,6,4,0,27   # Rotate and mask
    
equal_label:
    nop                 # Do nothing
    blr                 # Return
```

## Common Pseudo-Instructions

The assembler provides convenient pseudo-instructions:

| Pseudo-op | Expands to | Description |
|-----------|------------|-------------|
| `li r,imm` | `addi r,0,imm` | Load immediate |
| `mr r1,r2` | `or r1,r2,r2` | Move register |
| `nop` | `ori 0,0,0` | No operation |
| `slwi r1,r2,n` | `rlwinm r1,r2,n,0,31-n` | Shift left word |
| `srwi r1,r2,n` | `rlwinm r1,r2,32-n,n,31` | Shift right word |

## Key Differences from Other Assemblers

1. **No register prefix**: Use `1` not `r1` or `%r1`
2. **Comma separation**: `addc 1,2,3` not `addc 1 2 3`
3. **Memory syntax**: `offset(base)` not `[base+offset]`
4. **Dot suffix**: `.` for record bit, not separate instruction
5. **Comments**: `#` not `;` or `//`

## Validation

To verify assembly syntax:
```bash
as input.s -o output.o          # Assemble
objdump -d output.o             # Disassemble to verify
```

## References

- GNU Binutils documentation: https://sourceware.org/binutils/docs/as/
- PowerPC ISA: Power ISA Version 3.1
- OMR Compiler: `omr/compiler/p/codegen/PPCInstruction.hpp`
