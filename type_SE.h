#ifndef TYPE_SE_H
#define TYPE_SE_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
#include <cstdint>


using namespace std;

struct InstructionFields
{
    bitset<5> rd;
    bitset<3> funct3;
    bitset<5> rs1;
    bitset<5> rs2;
    bitset<7> funct7;
    bitset<12> imm_I;
    bitset<12> imm_S;
    bitset<12> imm_B;
    bitset<20> imm_U;
    bitset<20> imm_J;
};

// Helper function for sign extension
template <typename T>
int32_t signExtend(T imm, size_t bitWidth)
{
    int32_t mask = 1 << (bitWidth - 1); // Mask to check the sign bit
    int32_t extended = imm.to_ulong();  // Convert to unsigned long
    if (extended & mask)
    {
        // If sign bit is 1, extend with 1s
        extended |= ~((1 << bitWidth) - 1);
    }
    return extended;
}

InstructionFields checkInstr(bitset<32> instruction)
{

    InstructionFields fields;

    bitset<7> opcode = bitset<7>((instruction.to_ulong()) & 0x7F);

    // I-Type - works
    if (opcode == bitset<7>("0000011") || opcode == bitset<7>("0010011"))
    {
        fields.rd = bitset<5>((instruction.to_ulong() >> 7) & 0x1F);
        fields.funct3 = bitset<3>((instruction.to_ulong() >> 12) & 0x7);
        fields.rs1 = bitset<5>((instruction.to_ulong() >> 15) & 0x1F);
        fields.imm_I = bitset<12>((instruction.to_ullong() >> 20) & 0xFFF);

        cout << "Pre-SignExtended: " << fields.imm_I << endl;
        fields.imm_I = signExtend(bitset<12>((instruction.to_ullong() >> 20) & 0xFFF), 12);

        cout << "I-Type detected" << endl;
        cout << "opcode: " << opcode << endl;
        cout << "rd : " << fields.rd << endl;
        cout << "funct3 : " << fields.funct3 << endl;
        cout << "rs1 : " << fields.rs1 << endl;
        cout << "imm_I (sign-extended): " << fields.imm_I << endl;

    // R-Type - works
    }
    else if (opcode == bitset<7>("0110011"))
    {
        fields.rd = bitset<5>((instruction.to_ulong() >> 7) & 0x1F);
        fields.funct3 = bitset<3>((instruction.to_ulong() >> 12) & 0x7);
        fields.rs1 = bitset<5>((instruction.to_ulong() >> 15) & 0x1F);
        fields.rs2 = bitset<5>((instruction.to_ulong() >> 20) & 0x1F);
        fields.funct7 = bitset<7>((instruction.to_ulong() >> 25) & 0x7F);

        cout << "R-Type detected" << endl;
        cout << "rd : " << fields.rd << endl;
        cout << "funct3 : " << fields.funct3 << endl;
        cout << "rs1 : " << fields.rs1 << endl;
        cout << "rs2 : " << fields.rs2 << endl;
        cout << "funct7 : " << fields.funct7 << endl;

    // S-Type - works
    }
    else if (opcode == bitset<7>("0100011"))
    {
        fields.imm_S = signExtend(
            bitset<12>(
                ((instruction.to_ulong() >> 7) & 0x1F) |       // Bits 4:0
                ((instruction.to_ulong() >> 25) & 0x7F) << 5), // Bits 11:5
            12);
        fields.funct3 = bitset<3>((instruction.to_ulong() >> 12) & 0x7);
        fields.rs1 = bitset<5>((instruction.to_ulong() >> 15) & 0x1F);
        fields.rs2 = bitset<5>((instruction.to_ulong() >> 20) & 0x1F);

        cout << "S-Type detected" << endl;
        cout << "funct3 : " << fields.funct3 << endl;
        cout << "rs1 : " << fields.rs1 << endl;
        cout << "rs2 : " << fields.rs2 << endl;
        cout << "imm_S (sign-extended): " << fields.imm_S << endl;

    // B-Type - works
    }
    else if (opcode == bitset<7>("1100011"))
    {
        fields.imm_B = signExtend(
            bitset<13>(
                ((instruction.to_ulong() >> 31) & 0x1) << 12 | // imm[12]
                ((instruction.to_ulong() >> 7) & 0x1) << 11 |  // imm[11]
                ((instruction.to_ulong() >> 25) & 0x3F) << 5 | // imm[10:5]
                ((instruction.to_ulong() >> 8) & 0xF) << 1),   // imm[4:1]
            13);
        fields.funct3 = bitset<3>((instruction.to_ulong() >> 12) & 0x7);
        fields.rs1 = bitset<5>((instruction.to_ulong() >> 15) & 0x1F);
        fields.rs2 = bitset<5>((instruction.to_ulong() >> 20) & 0x1F);

        cout << "B-Type detected" << endl;
        cout << "imm_B (sign-extended): " << fields.imm_B << endl;
        cout << "funct3: " << fields.funct3 << endl;
        cout << "rs1: " << fields.rs1 << endl;
        cout << "rs2: " << fields.rs2 << endl;
    }

    // J-type
    else if (opcode == bitset<7>("1101111"))
    {
        fields.rd = bitset<5>((instruction.to_ulong() >> 7) & 0x1F);

        // Combine the parts directly into a 20-bit immediate in the correct order
        fields.imm_J = signExtend(
            bitset<20>(
                ((instruction.to_ulong() >> 31) & 0x1) << 19 |  // imm[20]
                ((instruction.to_ulong() >> 21) & 0x3FF) << 9 | // imm[10:1]
                ((instruction.to_ulong() >> 20) & 0x1) << 8 |   // imm[11]
                ((instruction.to_ulong() >> 12) & 0xFF)),       // imm[19:12]
            20);

        cout << "J-Type detected" << endl;
        cout << "opcode: " << opcode << endl;
        cout << "rd: " << fields.rd << endl;
        cout << "imm_J (sign-extended): " << fields.imm_J << endl;
    }

    return fields;
}

#endif
