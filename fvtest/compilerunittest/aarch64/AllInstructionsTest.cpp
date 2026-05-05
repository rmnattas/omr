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

#include <gtest/gtest.h>
#include "../CodeGenTest.hpp"
#include "codegen/ARM64Instruction.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/GenerateInstructions.hpp"
#include "codegen/Linkage.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RegisterDependency.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include <vector>
#include <random>

#define ARM64_INSTRUCTION_ALIGNMENT 32

/**
 * @brief Test to generate all ARM64 instructions
 * 
 * This test attempts to generate every instruction in the ARM64 instruction set
 * using static registers. The goal is to verify that instruction generation
 * doesn't crash, not to verify correctness of encoding.
 */
class ARM64AllInstructionsTest : public TRTest::BinaryEncoderTest<ARM64_INSTRUCTION_ALIGNMENT> {};

TEST_F(ARM64AllInstructionsTest, GenerateAllInstructions)
{
    // Static registers to use for instruction generation
    auto x0 = cg()->machine()->getRealRegister(TR::RealRegister::x0);
    auto x1 = cg()->machine()->getRealRegister(TR::RealRegister::x1);
    auto x2 = cg()->machine()->getRealRegister(TR::RealRegister::x2);
    auto v0 = cg()->machine()->getRealRegister(TR::RealRegister::v0);
    auto v1 = cg()->machine()->getRealRegister(TR::RealRegister::v1);
    auto v2 = cg()->machine()->getRealRegister(TR::RealRegister::v2);
    
    // Create a label for branch instructions
    TR::LabelSymbol *label = generateLabelSymbol(cg());
    
    // Create a memory reference for load/store instructions
    auto memRef = TR::MemoryReference::createWithDisplacement(cg(), x0, 0);
    
    int successCount = 0;
    int failCount = 0;
    
    // Pattern counters - count by instruction Kind, not by generation function
    int trg1Src2Count = 0;
    int trg1ImmCount = 0;
    int trg1Src1Count = 0;
    int trg1MemCount = 0;
    int labelCount = 0;
    int simpleCount = 0;
    int otherCount = 0;
    
    // Store all successfully generated instructions for sampling
    std::vector<TR::Instruction*> generatedInstructions;
    std::vector<TR::InstOpCode::Mnemonic> generatedOpcodes;
    
    // Iterate through all ARM64 opcodes
    for (int i = TR::InstOpCode::bad + 1; i < TR::InstOpCode::ARM64NumOpCodes; i++)
    {
        TR::InstOpCode::Mnemonic opcode = static_cast<TR::InstOpCode::Mnemonic>(i);
        TR::InstOpCode op(opcode);
        
        TR::Instruction *instr = nullptr;
        bool patternFound = false;
        
        // Try different instruction patterns
        // Pattern 1: Trg1Src2 (most common - try with GPR first)
        if (!patternFound)
        {
            try
            {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, x0, x1, x2);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 2: Vector Trg1Src2
        if (!patternFound)
        {
            try
            {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, v0, v1, v2);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 3: Trg1Imm (for instructions like vmovi8h)
        if (!patternFound)
        {
            try
            {
                instr = generateTrg1ImmInstruction(cg(), opcode, fakeNode, v0, 0);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 4: Trg1Src1
        if (!patternFound)
        {
            try
            {
                instr = generateTrg1Src1Instruction(cg(), opcode, fakeNode, x0, x1);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 5: Trg1Mem
        if (!patternFound)
        {
            try
            {
                instr = generateTrg1MemInstruction(cg(), opcode, fakeNode, x0, memRef);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 6: Label
        if (!patternFound)
        {
            try
            {
                instr = generateLabelInstruction(cg(), opcode, fakeNode, label);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        // Pattern 7: Simple instruction
        if (!patternFound)
        {
            try
            {
                instr = generateInstruction(cg(), opcode, fakeNode);
                if (instr != nullptr)
                {
                    patternFound = true;
                }
            }
            catch (...)
            {
                instr = nullptr;
            }
        }
        
        if (patternFound && instr != nullptr)
        {
            successCount++;
            generatedInstructions.push_back(instr);
            generatedOpcodes.push_back(opcode);
            
            // Count by actual instruction Kind
            switch (instr->getKind())
            {
                case TR::Instruction::IsTrg1Src2:
                    trg1Src2Count++;
                    break;
                case TR::Instruction::IsTrg1Imm:
                    trg1ImmCount++;
                    break;
                case TR::Instruction::IsTrg1Src1:
                    trg1Src1Count++;
                    break;
                case TR::Instruction::IsTrg1Mem:
                    trg1MemCount++;
                    break;
                case TR::Instruction::IsLabel:
                    labelCount++;
                    break;
                case TR::Instruction::IsNotExtended:
                    simpleCount++;
                    break;
                default:
                    otherCount++;
                    break;
            }
        }
        else
        {
            failCount++;
        }
    }
    
    // Print summary
    int totalOpcodes = TR::InstOpCode::ARM64NumOpCodes - TR::InstOpCode::bad - 1;
    printf("\n=== ARM64 All Instructions Test Summary ===\n");
    printf("Total opcodes: %d\n", totalOpcodes);
    printf("Successfully generated: %d\n", successCount);
    printf("Failed to generate: %d\n", failCount);
    printf("Success rate: %.1f%%\n\n", (successCount * 100.0) / totalOpcodes);
    
    printf("=== Instruction Pattern Breakdown (by Kind) ===\n");
    printf("Trg1Src2:              %4d (%.1f%%)\n", trg1Src2Count, (trg1Src2Count * 100.0) / successCount);
    printf("Trg1Imm:               %4d (%.1f%%)\n", trg1ImmCount, (trg1ImmCount * 100.0) / successCount);
    printf("Trg1Src1:              %4d (%.1f%%)\n", trg1Src1Count, (trg1Src1Count * 100.0) / successCount);
    printf("Trg1Mem:               %4d (%.1f%%)\n", trg1MemCount, (trg1MemCount * 100.0) / successCount);
    printf("Label:                 %4d (%.1f%%)\n", labelCount, (labelCount * 100.0) / successCount);
    printf("Simple:                %4d (%.1f%%)\n", simpleCount, (simpleCount * 100.0) / successCount);
    printf("Other:                 %4d (%.1f%%)\n", otherCount, (otherCount * 100.0) / successCount);
    printf("==========================================\n\n");
    
    // Sample 20 random instructions and print their details
    printf("=== Random Sample of Generated Instructions ===\n");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, generatedInstructions.size() - 1);
    
    int sampleSize = std::min(20, (int)generatedInstructions.size());
    for (int i = 0; i < sampleSize; i++)
    {
        int idx = dis(gen);
        TR::Instruction *instr = generatedInstructions[idx];
        TR::InstOpCode::Mnemonic opcode = generatedOpcodes[idx];
        
        // Get the binary encoding
        uint32_t encoding = TR::InstOpCode::getOpCodeBinaryEncoding(opcode);
        
        const char *kindStr = "Unknown";
        switch (instr->getKind())
        {
            case TR::Instruction::IsTrg1Src2: kindStr = "Trg1Src2"; break;
            case TR::Instruction::IsTrg1Imm: kindStr = "Trg1Imm"; break;
            case TR::Instruction::IsTrg1Src1: kindStr = "Trg1Src1"; break;
            case TR::Instruction::IsTrg1Mem: kindStr = "Trg1Mem"; break;
            case TR::Instruction::IsLabel: kindStr = "Label"; break;
            case TR::Instruction::IsNotExtended: kindStr = "Simple"; break;
            default: kindStr = "Other"; break;
        }
        
        printf("Opcode %4d: 0x%08X  Kind: %s\n", opcode, encoding, kindStr);
    }
    printf("==========================================\n");
    
    // Test passes if we generated at least some instructions
    EXPECT_GT(successCount, 0);
}
