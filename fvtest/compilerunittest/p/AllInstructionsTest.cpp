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
#include "codegen/PPCInstruction.hpp"
#include "codegen/CodeGenerator.hpp"
#include "codegen/GenerateInstructions.hpp"
#include "codegen/Linkage.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RegisterDependency.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "ras/Debug.hpp"
#include <vector>
#include <random>
#include <map>
#include <fstream>
#include <iomanip>

#define PPC_INSTRUCTION_ALIGNMENT 32

/**
 * @brief Test to generate all Power/PPC instructions
 * 
 * This test attempts to generate every instruction in the Power instruction set
 * and categorizes them by their instruction format metadata.
 */
class PowerAllInstructionsTest : public TRTest::BinaryEncoderTest<PPC_INSTRUCTION_ALIGNMENT> {};

TEST_F(PowerAllInstructionsTest, GenerateAllInstructions)
{
    // Static registers to use for instruction generation
    auto r0 = cg()->machine()->getRealRegister(TR::RealRegister::gr0);
    auto r1 = cg()->machine()->getRealRegister(TR::RealRegister::gr1);
    auto r2 = cg()->machine()->getRealRegister(TR::RealRegister::gr2);
    auto r3 = cg()->machine()->getRealRegister(TR::RealRegister::gr3);
    auto fp0 = cg()->machine()->getRealRegister(TR::RealRegister::fp0);
    auto fp1 = cg()->machine()->getRealRegister(TR::RealRegister::fp1);
    auto fp2 = cg()->machine()->getRealRegister(TR::RealRegister::fp2);
    auto vr0 = cg()->machine()->getRealRegister(TR::RealRegister::vr0);
    auto vr1 = cg()->machine()->getRealRegister(TR::RealRegister::vr1);
    auto vr2 = cg()->machine()->getRealRegister(TR::RealRegister::vr2);
    
    // Create a label for branch instructions
    TR::LabelSymbol *label = generateLabelSymbol(cg());
    
    // Create a memory reference for load/store instructions
    auto memRef = TR::MemoryReference::createWithDisplacement(cg(), r1, 0, 4);
    
    int successCount = 0;
    int failCount = 0;
    
    // Format counters - track by actual instruction format from metadata
    std::map<PPCInstructionFormat, int> formatCounts;
    
    // Helper function to get format name
    auto getFormatName = [](PPCInstructionFormat format) -> std::string {
        switch(format) {
            case FORMAT_UNKNOWN: return "FORMAT_UNKNOWN";
            case FORMAT_NONE: return "FORMAT_NONE";
            case FORMAT_DIRECT: return "FORMAT_DIRECT";
            case FORMAT_DIRECT_PREFIXED: return "FORMAT_DIRECT_PREFIXED";
            case FORMAT_DD: return "FORMAT_DD";
            case FORMAT_I_FORM: return "FORMAT_I_FORM";
            case FORMAT_B_FORM: return "FORMAT_B_FORM";
            case FORMAT_XL_FORM_BRANCH: return "FORMAT_XL_FORM_BRANCH";
            case FORMAT_MTFSFI: return "FORMAT_MTFSFI";
            case FORMAT_MTFSF: return "FORMAT_MTFSF";
            case FORMAT_RS: return "FORMAT_RS";
            case FORMAT_RA_SI16: return "FORMAT_RA_SI16";
            case FORMAT_RA_SI5: return "FORMAT_RA_SI5";
            case FORMAT_RS_FXM: return "FORMAT_RS_FXM";
            case FORMAT_RS_FXM1: return "FORMAT_RS_FXM1";
            case FORMAT_RT: return "FORMAT_RT";
            case FORMAT_RA_RS: return "FORMAT_RA_RS";
            case FORMAT_RT_RA: return "FORMAT_RT_RA";
            case FORMAT_FRT_FRB: return "FORMAT_FRT_FRB";
            case FORMAT_BF_BFA: return "FORMAT_BF_BFA";
            case FORMAT_RA_XS: return "FORMAT_RA_XS";
            case FORMAT_XT_RA: return "FORMAT_XT_RA";
            case FORMAT_RT_BFA: return "FORMAT_RT_BFA";
            case FORMAT_VRT_VRB: return "FORMAT_VRT_VRB";
            case FORMAT_RT_VRB: return "FORMAT_RT_VRB";
            case FORMAT_XT_XB: return "FORMAT_XT_XB";
            case FORMAT_RT_RA_RB: return "FORMAT_RT_RA_RB";
            case FORMAT_RT_RA_RB_MEM: return "FORMAT_RT_RA_RB_MEM";
            case FORMAT_RA_RS_RB: return "FORMAT_RA_RS_RB";
            case FORMAT_BF_RA_RB: return "FORMAT_BF_RA_RB";
            case FORMAT_BF_FRA_FRB: return "FORMAT_BF_FRA_FRB";
            case FORMAT_FRT_RA_RB_MEM: return "FORMAT_FRT_RA_RB_MEM";
            case FORMAT_FRT_FRA_FRB: return "FORMAT_FRT_FRA_FRB";
            case FORMAT_VRT_RA_RB_MEM: return "FORMAT_VRT_RA_RB_MEM";
            case FORMAT_VRT_VRA_VRB: return "FORMAT_VRT_VRA_VRB";
            case FORMAT_XT_RA_RB: return "FORMAT_XT_RA_RB";
            case FORMAT_XT_RA_RB_MEM: return "FORMAT_XT_RA_RB_MEM";
            case FORMAT_XT_XA_XB: return "FORMAT_XT_XA_XB";
            case FORMAT_FRT_FRA_FRC: return "FORMAT_FRT_FRA_FRC";
            case FORMAT_RT_RA_SI16: return "FORMAT_RT_RA_SI16";
            case FORMAT_RA_RS_UI16: return "FORMAT_RA_RS_UI16";
            case FORMAT_BF_RA_SI16: return "FORMAT_BF_RA_SI16";
            case FORMAT_BF_RA_UI16: return "FORMAT_BF_RA_UI16";
            case FORMAT_BF_FRA_DM: return "FORMAT_BF_FRA_DM";
            case FORMAT_RA_RS_SH5: return "FORMAT_RA_RS_SH5";
            case FORMAT_RA_RS_SH6: return "FORMAT_RA_RS_SH6";
            case FORMAT_VRT_VRB_UIM4: return "FORMAT_VRT_VRB_UIM4";
            case FORMAT_VRT_VRB_UIM3: return "FORMAT_VRT_VRB_UIM3";
            case FORMAT_VRT_VRB_UIM2: return "FORMAT_VRT_VRB_UIM2";
            case FORMAT_XT_XB_UIM2: return "FORMAT_XT_XB_UIM2";
            case FORMAT_RT_BI: return "FORMAT_RT_BI";
            case FORMAT_RT_SI16: return "FORMAT_RT_SI16";
            case FORMAT_BF_BFAI: return "FORMAT_BF_BFAI";
            case FORMAT_RT_FXM: return "FORMAT_RT_FXM";
            case FORMAT_RT_FXM1: return "FORMAT_RT_FXM1";
            case FORMAT_VRT_SIM: return "FORMAT_VRT_SIM";
            case FORMAT_RLDIC: return "FORMAT_RLDIC";
            case FORMAT_RLDICL: return "FORMAT_RLDICL";
            case FORMAT_RLDICR: return "FORMAT_RLDICR";
            case FORMAT_RLWINM: return "FORMAT_RLWINM";
            case FORMAT_BF_RA_RB_L: return "FORMAT_BF_RA_RB_L";
            case FORMAT_BT_BA_BB: return "FORMAT_BT_BA_BB";
            case FORMAT_FRT_FRA_FRB_RMC: return "FORMAT_FRT_FRA_FRB_RMC";
            case FORMAT_RLDCL: return "FORMAT_RLDCL";
            case FORMAT_RLWNM: return "FORMAT_RLWNM";
            case FORMAT_VRT_VRA_VRB_SHB: return "FORMAT_VRT_VRA_VRB_SHB";
            case FORMAT_XT_XA_XB_DM: return "FORMAT_XT_XA_XB_DM";
            case FORMAT_XT_XA_XB_SHW: return "FORMAT_XT_XA_XB_SHW";
            case FORMAT_RT_RA_RB_RC: return "FORMAT_RT_RA_RB_RC";
            case FORMAT_FRT_FRA_FRC_FRB: return "FORMAT_FRT_FRA_FRC_FRB";
            case FORMAT_VRT_VRA_VRB_VRC: return "FORMAT_VRT_VRA_VRB_VRC";
            case FORMAT_XT_XA_XB_XC: return "FORMAT_XT_XA_XB_XC";
            case FORMAT_RA_RB: return "FORMAT_RA_RB";
            case FORMAT_RA_RB_MEM: return "FORMAT_RA_RB_MEM";
            case FORMAT_RT_D16_RA: return "FORMAT_RT_D16_RA";
            case FORMAT_FRT_D16_RA: return "FORMAT_FRT_D16_RA";
            case FORMAT_RT_DS_RA: return "FORMAT_RT_DS_RA";
            case FORMAT_XT28_DQ_RA: return "FORMAT_XT28_DQ_RA";
            case FORMAT_RS_D16_RA: return "FORMAT_RS_D16_RA";
            case FORMAT_FRS_D16_RA: return "FORMAT_FRS_D16_RA";
            case FORMAT_RS_DS_RA: return "FORMAT_RS_DS_RA";
            case FORMAT_XS28_DQ_RA: return "FORMAT_XS28_DQ_RA";
            case FORMAT_RS_RA_RB: return "FORMAT_RS_RA_RB";
            case FORMAT_RS_RA_RB_MEM: return "FORMAT_RS_RA_RB_MEM";
            case FORMAT_FRS_RA_RB_MEM: return "FORMAT_FRS_RA_RB_MEM";
            case FORMAT_VRS_RA_RB_MEM: return "FORMAT_VRS_RA_RB_MEM";
            case FORMAT_XS_RA_RB: return "FORMAT_XS_RA_RB";
            case FORMAT_XS_RA_RB_MEM: return "FORMAT_XS_RA_RB_MEM";
            case FORMAT_RT_RA_RB_BFC: return "FORMAT_RT_RA_RB_BFC";
            case FORMAT_RT_D34_RA_R: return "FORMAT_RT_D34_RA_R";
            case FORMAT_RT_D32_RA_R: return "FORMAT_RT_D32_RA_R";
            case FORMAT_RTP_D34_RA_R: return "FORMAT_RTP_D34_RA_R";
            case FORMAT_FRT_D34_RA_R: return "FORMAT_FRT_D34_RA_R";
            case FORMAT_VRT_D34_RA_R: return "FORMAT_VRT_D34_RA_R";
            case FORMAT_XT5_D34_RA_R: return "FORMAT_XT5_D34_RA_R";
            case FORMAT_RS_D34_RA_R: return "FORMAT_RS_D34_RA_R";
            case FORMAT_RSP_D34_RA_R: return "FORMAT_RSP_D34_RA_R";
            case FORMAT_FRS_D34_RA_R: return "FORMAT_FRS_D34_RA_R";
            case FORMAT_VRS_D34_RA_R: return "FORMAT_VRS_D34_RA_R";
            case FORMAT_XS5_D34_RA_R: return "FORMAT_XS5_D34_RA_R";
            case FORMAT_XT_IMM8: return "FORMAT_XT_IMM8";
            default: return "UNKNOWN_FORMAT_" + std::to_string(static_cast<int>(format));
        }
    };
    
    // Store all successfully generated instructions for sampling
    std::vector<TR::Instruction*> generatedInstructions;
    std::vector<TR::InstOpCode::Mnemonic> generatedOpcodes;
    
    // Use real registers directly for proper symbolic name printing
    TR::Register *dummyGPR1 = cg()->machine()->getRealRegister(TR::RealRegister::gr1);
    TR::Register *dummyGPR2 = cg()->machine()->getRealRegister(TR::RealRegister::gr2);
    TR::Register *dummyGPR3 = cg()->machine()->getRealRegister(TR::RealRegister::gr3);
    TR::Register *dummyFPR1 = cg()->machine()->getRealRegister(TR::RealRegister::fp1);
    TR::Register *dummyFPR2 = cg()->machine()->getRealRegister(TR::RealRegister::fp2);
    TR::Register *dummyVRF1 = cg()->machine()->getRealRegister(TR::RealRegister::vr1);
    TR::Register *dummyVRF2 = cg()->machine()->getRealRegister(TR::RealRegister::vr2);
    TR::Register *dummyCCR = cg()->machine()->getRealRegister(TR::RealRegister::cr0);
    
    // Iterate through all Power opcodes
    for (int i = TR::InstOpCode::bad + 1; i < TR::InstOpCode::NumOpCodes; i++)
    {
        TR::InstOpCode::Mnemonic opcode = static_cast<TR::InstOpCode::Mnemonic>(i);
        TR::InstOpCode op(opcode);
        
        TR::Instruction *instr = nullptr;
        
        // Get the format from metadata
        PPCInstructionFormat format = op.getFormat();
        
        // Create instruction with appropriate registers based on format
        try
        {
            // Use format-specific generation for better operand display
            if (format == FORMAT_RT_RA_RB || format == FORMAT_RA_RS_RB) {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, dummyGPR1, dummyGPR2, dummyGPR3);
            } else if (format == FORMAT_FRT_FRA_FRB) {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, dummyFPR1, dummyFPR2, dummyFPR2);
            } else if (format == FORMAT_VRT_VRA_VRB) {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, dummyVRF1, dummyVRF2, dummyVRF2);
            } else if (format == FORMAT_RT_RA || format == FORMAT_RA_RS) {
                instr = generateTrg1Src1Instruction(cg(), opcode, fakeNode, dummyGPR1, dummyGPR2);
            } else if (format == FORMAT_BF_RA_RB) {
                instr = generateTrg1Src2Instruction(cg(), opcode, fakeNode, dummyCCR, dummyGPR1, dummyGPR2);
            } else {
                // Fallback to general generation
                instr = generateInstruction(cg(), opcode, fakeNode);
            }
            
            if (instr != nullptr)
            {
                successCount++;
                generatedInstructions.push_back(instr);
                generatedOpcodes.push_back(opcode);
                
                // Count by format from metadata
                formatCounts[format]++;
            }
            else
            {
                failCount++;
            }
        }
        catch (...)
        {
            failCount++;
        }
    }
    
    // Print summary
    int totalOpcodes = TR::InstOpCode::NumOpCodes - TR::InstOpCode::bad - 1;
    printf("\n=== Power/PPC All Instructions Test Summary ===\n");
    printf("Total opcodes: %d\n", totalOpcodes);
    printf("Successfully generated: %d\n", successCount);
    printf("Failed to generate: %d\n", failCount);
    printf("Success rate: %.1f%%\n\n", (successCount * 100.0) / totalOpcodes);
    
    printf("=== Instruction Format Breakdown (from metadata) ===\n");
    for (const auto& pair : formatCounts)
    {
        std::string name = getFormatName(pair.first);
        printf("%-25s: %4d (%.1f%%)\n", name.c_str(), pair.second, (pair.second * 100.0) / successCount);
    }
    printf("==========================================\n\n");
    
    // Write all instructions to files
    std::ofstream binaryFile("all_instructions.bin", std::ios::binary);
    FILE* textFileHandle = fopen("all_instructions.txt", "w");
    
    if (binaryFile.is_open() && textFileHandle != nullptr)
    {
        printf("\n=== Writing all instructions to files ===\n");
        printf("Binary file: all_instructions.bin\n");
        printf("Text file: all_instructions.txt\n");
        
        // Create a logger for the text file using static create method
        auto textLogger = OMR::CStdIOStreamLogger::create(PERSISTENT_NEW, textFileHandle);
        
        // Create debug object if it doesn't exist
        TR_Debug *debug = cg()->comp()->getDebug();
        if (debug == nullptr) {
            debug = createDebugObject(cg()->comp());
            cg()->comp()->setDebug(debug);
        }
        
        for (size_t i = 0; i < generatedInstructions.size(); i++)
        {
            TR::InstOpCode::Mnemonic opcode = generatedOpcodes[i];
            TR::InstOpCode op(opcode);
            TR::Instruction *instr = generatedInstructions[i];
            
            // Get the binary encoding
            uint32_t prefix = op.getMetaData().prefix;
            uint32_t encoding = op.getOpCodeBinaryEncoding();
            
            // Write binary encoding (big-endian format for Power)
            if (prefix != 0) {
                // Prefixed instruction: write prefix first, then encoding
                uint32_t prefixBE = __builtin_bswap32(prefix);
                uint32_t encodingBE = __builtin_bswap32(encoding);
                binaryFile.write(reinterpret_cast<const char*>(&prefixBE), sizeof(prefixBE));
                binaryFile.write(reinterpret_cast<const char*>(&encodingBE), sizeof(encodingBE));
            } else {
                // Regular 4-byte instruction
                uint32_t encodingBE = __builtin_bswap32(encoding);
                binaryFile.write(reinterpret_cast<const char*>(&encodingBE), sizeof(encodingBE));
            }
            
            // Write human-readable instruction using TR_Debug
            if (debug) {
                debug->print(textLogger, instr);
                textLogger->println();
                textLogger->flush();
            }
        }
        
        // Ensure all data is written
        textLogger->flush();
        fflush(textFileHandle);
        
        binaryFile.close();
        fclose(textFileHandle);
        printf("Successfully wrote %zu instructions to files\n", generatedInstructions.size());
    }
    else
    {
        printf("ERROR: Failed to open output files\n");
    }
    
    // Sample 10 random instructions and print their details
    printf("\n=== Random Sample of Generated Instructions ===\n");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, generatedInstructions.size() - 1);
    
    int sampleSize = std::min(10, (int)generatedInstructions.size());
    for (int i = 0; i < sampleSize; i++)
    {
        int idx = dis(gen);
        TR::InstOpCode::Mnemonic opcode = generatedOpcodes[idx];
        TR::InstOpCode op(opcode);
        TR::Instruction *instr = generatedInstructions[idx];
        
        // Get the binary encoding and format
        uint32_t prefix = op.getMetaData().prefix;
        uint32_t encoding = op.getOpCodeBinaryEncoding();
        PPCInstructionFormat format = op.getFormat();
        std::string formatName = getFormatName(format);
        
        // Print binary encoding only
        if (prefix != 0) {
            printf("Opcode %4d (%s): 0x%08X%08X  Format: %s (Prefixed)\n", 
                   opcode, op.getMnemonicName(), prefix, encoding, formatName.c_str());
        } else {
            printf("Opcode %4d (%s): 0x%08X  Format: %s\n", 
                   opcode, op.getMnemonicName(), encoding, formatName.c_str());
        }
    }
    printf("==========================================\n");
    
    // Test passes if we generated at least some instructions
    EXPECT_GT(successCount, 0);
}
