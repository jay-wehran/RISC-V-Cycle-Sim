#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
#include <stdint.h>
#include "type_SE.h"


using namespace std;

#define MemSize 1000 // Memory Size

struct IFStruct
{
    bitset<32> PC;
    bool nop;
};

struct IDStruct
{
    bitset<32> Instr;
    bool nop = true;
};

struct EXStruct
{
    bitset<32> Read_data1;
    bitset<32> Read_data2;
    bitset<12> Imm_I;
    bitset<12> Imm_S;
    bitset<12> Imm_B;
    bitset<20> Imm_U;
    bitset<20> Imm_J;
    bitset<32> Imm;
    bitset<5> rs1;
    bitset<5> rs2;
    bitset<5> rd;
    bitset<3> func3;
    bitset<7> func7;
    bitset<32> Instr;
    bool rd_mem = false;
    bool wrt_mem = false;
    bool alu_op = false;
    bool wrt_enable = false;
    bool nop = true;
    bool is_I_type = false;
};

struct MEMStruct
{
    bitset<32> ALUresult;
    bitset<32> Store_data;
    bitset<5> rs1;
    bitset<5> rs2;
    bitset<5> rd;
    bool rd_mem = false;
    bool wrt_mem = false;
    bool wrt_enable = false;
    bool nop = true;
};

struct WBStruct
{
    bitset<32> Wrt_data;
    bitset<5> rs1;
    bitset<5> rs2;
    bitset<5> rd;
    bool wrt_enable;
    bool nop = true;
};

struct stateStruct
{
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

class InsMem
{
public:
    string id, ioDir;
    InsMem(string name, string ioDir) // Reads instructions from a file, storing each byte as an 8-bit bitset. Formatting.
    {
        id = name;
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open(ioDir + "\\imem.txt");
        if (imem.is_open())
        {
            while (getline(imem, line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open IMEM input file.";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress) // combines 4 8-bit entries (models fetching), bounds protect against out of range
    {

        int start_address;         // for identifying the starting address of the 32 bit bitset
        bitset<32> instruction(0); // the final combined bitset when the 4 bytes are combined

        start_address = ReadAddress.to_ulong(); // converting the hex start address to a long, (i.e. 0x00000000 --> 0, 0x00000004 --> 4)

        if (start_address + 3 < IMem.size()) // error checking to make sure we dont go out of bounds with MemSize
        {
            instruction = (IMem[start_address].to_ulong() << 24 |     // taking the 8 bits at starting address, shifting them 24 left				--> MSB
                           IMem[start_address + 1].to_ulong() << 16 | // taking the next 8 bits at starting address + 1, shifting them 16 left
                           IMem[start_address + 2].to_ulong() << 8 |  // taking the next 8 bits at starting address + 2, shifting them 8 left
                           IMem[start_address + 3].to_ulong());       // taking the last 8 bits at starting address + 3, maintining position		--> LSB
        }
        else
        {
            printf("Error(ri): Address is out of range."); // error checking feedback
            cout << "Accessing IMem at address: " << start_address << endl;

            return bitset<32>(0); // returning empty 32 bit bitset if OAB
        }

        return instruction; // returning the combined 32 bit bitset
    }

private:
    vector<bitset<8>> IMem; // Streams of 8 bits, to then be used by readInstr
};

class DataMem // Used to access and update data that has already been stored
{
public:
    string id, opFilePath, ioDir;

    DataMem(string name, string ioDir) : id{name}, ioDir{ioDir}
    {
        DMem.resize(MemSize);
        opFilePath = ioDir + "\\" + name + "_DMEMResult.txt";
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open(ioDir + "\\dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line) && i < MemSize)
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
            dmem.close();
        }
        else
            cout << "Unable to open DMEM input file.";
    }

    void printDMemState(int start = 0, int end = 16) const
    { // debug function to help see state of dmem
        cout << "(Inside printDMemState) Current state of DMem:" << endl;
        for (int i = start; i < end && i < DMem.size(); ++i)
        {
            cout << "DMem[" << i << "]: " << DMem[i] << endl;
        }
    }

    bitset<32> readDataMem(bitset<32> Address) // fetches data from memory
    {

        int start_address;      // for identifying the starting address of the 32 bit bitset
        bitset<32> data_val(0); // the final combined bitset when the 4 bytes are combined

        start_address = Address.to_ulong(); // converting the hex start address to a long, (i.e. 0x00000000 --> 0, 0x00000004 --> 4)

        if (start_address + 3 < DMem.size()) // error checking to make sure we dont go out of bounds with MemSize
        {
            data_val = (DMem[start_address].to_ulong() << 24 |     // taking the 8 bits at starting address, shifting them 24 left				--> MSB
                        DMem[start_address + 1].to_ulong() << 16 | // taking the next 8 bits at starting address + 1, shifting them 16 left
                        DMem[start_address + 2].to_ulong() << 8 |  // taking the next 8 bits at starting address + 2, shifting them 8 left
                        DMem[start_address + 3].to_ulong());       // taking the last 8 bits at starting address + 3, maintining position		--> LSB
        }
        else
        {
            printf("Error(rdm): Address is out of range."); // error checking feedback
            cout << "Accessing IMem at address: " << start_address << endl;

            return bitset<32>(0); // returning empty 32 bit bitset if OAB
        }

        return data_val; // returning the combined 32 bit bitset
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        int start_address = Address.to_ulong();

        if (start_address + 3 < DMem.size())
        {
            // Writing each byte of the 32-bit data into consecutive line, using mshifts and masks to extract
            DMem[start_address] = bitset<8>((WriteData.to_ulong() >> 24) & 0xFF);
            DMem[start_address + 1] = bitset<8>((WriteData.to_ulong() >> 16) & 0xFF);
            DMem[start_address + 2] = bitset<8>((WriteData.to_ulong() >> 8) & 0xFF);
            DMem[start_address + 3] = bitset<8>(WriteData.to_ulong() & 0xFF);
        }
        else
        {
            cout << "Error(wdm): Address is out of range. Accessing IMem at address: " << start_address << endl;
        }
    }

    void outputDataMem()
    {

        ofstream dmemout(opFilePath, std::ios_base::trunc);
        if (dmemout.is_open())
        {
            for (int j = 0; j < DMem.size(); j++)
            {
                dmemout << DMem[j].to_string() << endl;
                // if (j < 16) {  // debug
                //     cout << "Memory[" << j << "]: " << DMem[j] << endl;
                // }
            }
            // cout << "Debug - Data memory dump successful." << endl;
            dmemout.close();
        }
        else
        {
            cout << "Unable to open " << opFilePath << " for writing." << endl;
        }
    }

private:
    vector<bitset<8>> DMem;
};

class RegisterFile
{
public:
    string outputFile;
    RegisterFile(string ioDir) : outputFile{ioDir + "RFResult.txt"}
    {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr) // reads data from a specified register
    {

        if (Reg_addr.to_ulong() < Registers.size()) // checking if Reg_addr param is with in the valid register range of 0-31
        {
            return Registers[Reg_addr.to_ulong()]; // returning the contents of the register specified
        }
        else
        {
            printf("Error: Register address is out of range."); // giving error if the register given is out of range

            return bitset<32>(0); // returning empty bitset as feedback error value
        }
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) // writes data to a specified register
    {

        if (Reg_addr.to_ulong() < Registers.size()) // checking if Reg_addr param is with in the valid register range of 0-31
        {
            if (Reg_addr.to_ulong() != 0) // Checking if intended register is 0,
            {
                Registers[Reg_addr.to_ulong()] = Wrt_reg_data; // if not 0, writing data to specified register
            }
            else
            {
                printf("Warming: Attempt to write to register 0."); // printing warning when attempting to write to register 0
            }
        }
        else
        {
            printf("Error: Register address out of range."); // giving error feedback if given invalid register #
        }
    }

    void outputRF(int cycle) // writes the state of registers to output file
    {
        ofstream rfout;
        if (cycle == 0)
            rfout.open(outputFile, std::ios_base::trunc);
        else
            rfout.open(outputFile, std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF after executing cycle:\t" << cycle << endl;
            for (int j = 0; j < 32; j++)
            {
                rfout << Registers[j] << endl;
            }
        }
        else
            cout << "Unable to open RF output file." << endl;
        rfout.close();
    }

private:
    vector<bitset<32>> Registers;
};

class Core
{

public:
    RegisterFile myRF;
    uint32_t cycle = 0;
    bool halted = false;
    string ioDir;
    struct stateStruct state, nextState;
    InsMem ext_imem;
    DataMem ext_dmem;

    Core(string ioDir, InsMem &imem, DataMem &dmem) : myRF(ioDir), ioDir{ioDir}, ext_imem{imem}, ext_dmem{dmem} {}

    virtual void step() {}

    virtual void printState() {}
};

class SingleStageCore : public Core
{
public:
	SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem) : Core(ioDir + "\\SS_", imem, dmem), opFilePath(ioDir + "\\StateResult_SS.txt"), perfFilePath(ioDir + "\\PerformanceMetrics_SS.txt") {} //! __________________

	DataMem &getDataMem()
	{ // helper function to get the final Dmem values
		return ext_dmem;
	}

	// Immidiate Sign Extensions
	int32_t sign_extend_imm(int16_t imm)
	{
		return static_cast<int32_t>(imm); // Sign-extend 12-bit to 32-bit
	}

	int32_t sign_extend_imm_I(int32_t imm)
	{
		if (imm & (1 << 11))
		{					   // Check if the 12th bit (sign bit) is set
			imm |= 0xFFFFF000; // Fill the upper 20 bits with 1's for sign extension
		}
		return imm;
	}

	int32_t sign_extend_imm_B(int32_t imm)
	{
		if (imm & (1 << 12))
		{					   // Check if the 13th bit (sign bit) is set
			imm |= 0xFFFFE000; // Sign-extend to 32-bits for 13-bit immediate
		}
		return imm;
	}

	int32_t sign_extend_imm_J(int32_t imm)
	{
		// Check if the 20th bit (sign bit for 21-bit immediate) is set
		if (imm & (1 << 20))
		{
			imm |= 0xFFE00000; // Fill the upper bits with 1's
		}
		return imm;
	}

	int32_t extract_jal_imm(const bitset<32> &instruction)
	{	// helper function to extra i-type 21 bit immediate
		int32_t imm = 0;

		// Assemble the immediate value from instruction bits as described above
		imm |= ((instruction[31] ? 1 : 0) << 20);			  // Bit 20 (sign bit)
		imm |= ((instruction.to_ulong() >> 12) & 0xFF) << 12; // Bits 19-12
		imm |= ((instruction[20] ? 1 : 0) << 11);			  // Bit 11
		imm |= ((instruction.to_ulong() >> 21) & 0x3FF) << 1; // Bits 10-1

		// Sign-extend to 32 bits if the sign bit is set
		if (imm & (1 << 20))
		{
			imm |= 0xFFE00000; // Extend the upper bits with 1's
		}

		return imm;
	}

	void step()
	{
		// Instruction Fetch (IF)
		bitset<32> current_instruction = ext_imem.readInstr(state.IF.PC);
		// cout << "Cycle: " << cycle << endl; 	//* debug

		if (current_instruction == bitset<32>(0) || current_instruction == bitset<32>(0xFFFFFFFF) || nextState.IF.PC.to_ulong() >= MemSize)
		{	// checking for halt conditions
			totalInstructions++; 	// keeping track of instruction count
			halted = true;
			nextState.IF.nop = true;
			nextState.IF.PC = state.IF.PC;
			myRF.outputRF(cycle);
			printState(nextState, cycle);
			cycle++;

			// extra cycles to confirm state
			int extra_Cycles = 0;
			if (halted)
			{
				if (extra_Cycles < 2)
				{
					myRF.outputRF(cycle);
					printState(nextState, cycle);
					cycle++;
				}
				return;
			}

			return;
		}
		else
		{
			nextState.IF.nop = false;
		}

		totalInstructions++;

		nextState.IF.PC = state.IF.PC.to_ulong() + 4;

		// Decode Instruction
		bitset<7> opcode = bitset<7>((current_instruction.to_ulong()) & 0x7F);
		bitset<5> rs1 = bitset<5>((current_instruction >> 15).to_ulong() & 0x1F);
		bitset<5> rs2 = bitset<5>((current_instruction >> 20).to_ulong() & 0x1F);
		bitset<5> rd = bitset<5>((current_instruction >> 7).to_ulong() & 0x1F);
		bitset<3> func3 = bitset<3>((current_instruction >> 12).to_ulong() & 0x07);
		bitset<7> func7 = bitset<7>((current_instruction >> 25).to_ulong());

		// Immediate Extraction
		int32_t imm_I = sign_extend_imm_I(static_cast<int16_t>((current_instruction.to_ulong() >> 20) & 0xFFF));
		int32_t imm_S = sign_extend_imm(static_cast<int16_t>(((current_instruction >> 20).to_ulong() & 0xFE0) |
															 ((current_instruction >> 7).to_ulong() & 0x1F)));
		// Correct B-Type immediate calculation
		int32_t imm_B = ((current_instruction[31] << 12) |
						 ((current_instruction.to_ulong() >> 25) & 0x3F) << 5 |
						 ((current_instruction.to_ulong() >> 8) & 0xF) << 1 |
						 ((current_instruction.to_ulong() >> 7) & 0x1) << 11);

		imm_B = sign_extend_imm_B(imm_B); // Apply sign extension
		int32_t imm_J = extract_jal_imm(current_instruction);

		// extracting source registers
		bitset<32> read_data1 = myRF.readRF(rs1);
		bitset<32> read_data2 = myRF.readRF(rs2);
		bitset<32> alu_result(0);

		if (opcode == bitset<7>("0110011")) // R-Type Instructions
		{
			// cout << "R-type called" << endl;		//* debug
			if (func3 == bitset<3>("000") && func7 == bitset<7>("0000000")) // ADD
			{
				// cout << "ADD called" << endl;	//* debug
				alu_result = read_data1.to_ulong() + read_data2.to_ulong();
			}
			else if (func3 == bitset<3>("000") && func7 == bitset<7>("0100000")) // SUB
			{
				// cout << "SUB called" << endl;	//* debug
				alu_result = read_data1.to_ulong() - read_data2.to_ulong();
			}
			else if (func3 == bitset<3>("100")) // XOR
			{
				// cout << "XOR called" << endl;	//* debug
				alu_result = read_data1 ^ read_data2;
			}
			else if (func3 == bitset<3>("110")) // OR
			{
				// cout << "OR called" << endl;		//* debug
				alu_result = read_data1 | read_data2;
			}
			else if (func3 == bitset<3>("111")) // AND
			{
				// cout << "AND called" << endl;	//* debug
				alu_result = read_data1 & read_data2;
			}
		}

		else if (opcode == bitset<7>("0010011")) // I-Type Instructions
		{
			// cout << "I-Type called" << endl;		//* debug
			if (func3 == bitset<3>("000")) // ADDi
			{
				// cout << "ADDi called" << endl;	//* debug

				// Convert read_data1 to signed integer
				int32_t data1_signed = static_cast<int32_t>(read_data1.to_ulong());

				// Addition
				int32_t result = data1_signed + imm_I;
				alu_result = bitset<32>(result); // Convert back to bitset for storing in alu_result
			}
			else if (func3 == bitset<3>("100")) // XORi
			{
				// cout << "XORi called" << endl;	//* debug
				alu_result = read_data1.to_ulong() ^ imm_I;
			}
			else if (func3 == bitset<3>("110")) // ORi
			{
				// cout << "ORi called" << endl;	//* debug
				alu_result = read_data1.to_ulong() | imm_I;
			}
			else if (func3 == bitset<3>("111")) // ANDi
			{
				// cout << "ANDi called" << endl;	//* debug
				alu_result = read_data1.to_ulong() & imm_I;
			}
		}

		else if (opcode == bitset<7>("0000011")) // LW
		{
			// cout << "LW called" << endl;		//* debug
			alu_result = read_data1.to_ulong() + imm_I;
		}

		else if (opcode == bitset<7>("0100011")) // SW
		{
			// cout << "SW called" << endl;		//* debug
			alu_result = read_data1.to_ulong() + imm_S;
			ext_dmem.writeDataMem(bitset<32>(alu_result), read_data2);
		}
		else if (opcode == bitset<7>("1100011")) // BEQ & BNE
		{
			// cout << "BEQ or BNE called" << endl;		//* debug
			bool branch_taken = false;

			if ((func3 == bitset<3>("000") && read_data1 == read_data2) || // BEQ
				(func3 == bitset<3>("001") && read_data1 != read_data2))   // BNE
			{
				branch_taken = true;
				int32_t branch_target = state.IF.PC.to_ulong() + imm_B; // Calculating branch target

				// Set the new PC to target if branching
				nextState.IF.PC = bitset<32>(branch_target);
				// cout << "Branch taken, PC updated to: " << branch_target << ", imm_B: " << imm_B << endl;	//* debug
			}

			if (!branch_taken)
			{
				// No branch taken; proceed to next instruction
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
				// cout << "Branch not taken, PC updated to: " << nextState.IF.PC.to_ulong() << endl;	//* debug
			}
		}

		else if (opcode == bitset<7>("1101111")) // JAL
		{
			// cout << "JAL called" << endl;	//* debug

			// Calculate the link address
			int32_t link_address = state.IF.PC.to_ulong() + 4;

			// Calculate the target jump
			int32_t jump_target = static_cast<int32_t>(state.IF.PC.to_ulong()) + imm_J;

			// Write the link address to rd register
			if (rd.to_ulong() != 0)
			{
				myRF.writeRF(rd, bitset<32>(link_address)); // Store PC + 4 in the destination register
				// cout << "JAL link address (PC + 4) written to rd: " << link_address << endl;		//* debug
			}

			// Update PC to jump
			nextState.IF.PC = bitset<32>(jump_target);
			// cout << "JAL executed, New PC set to: " << nextState.IF.PC.to_ulong() << ", imm_J: " << imm_J << endl;	//* debug
		}

		if (nextState.IF.PC.to_ulong() >= MemSize || nextState.IF.PC.to_ulong() < 0)
		{
			// cout << "Error: PC out of range - accessing IMem at address: " << nextState.IF.PC.to_ulong() << endl;	//* debug
			halted = true;
			return;
		}

		bitset<32> mem_data(0);

		if (opcode == bitset<7>("0000011")) // SS_RFResult
		{
			mem_data = ext_dmem.readDataMem(alu_result);
		}

		if (opcode == bitset<7>("0110011") || opcode == bitset<7>("0010011"))
		{
			myRF.writeRF(rd, alu_result);
		}
		else if (opcode == bitset<7>("0000011"))
		{
			myRF.writeRF(rd, mem_data);
		}

		myRF.outputRF(cycle);
		printState(nextState, cycle);

		state = nextState;
		cycle++;
	}

	void printState(stateStruct state, int cycle)
	{	// output for StateResult
		ofstream printstate(opFilePath, cycle == 0 ? std::ios_base::trunc : std::ios_base::app);
		if (printstate.is_open())
		{
			printstate << "----------------------------------------------------------------------\n";
			printstate << "State after executing cycle: " << cycle << "\n";
			printstate << "IF.PC: " << state.IF.PC.to_ulong() << "\n";
			printstate << "IF.nop: " << (state.IF.nop ? "True" : "False") << "\n";
			printstate << "----------------------------------------------------------------------\n";
		}
		printstate.close();
	}

	void outputPerformanceMetrics()
	{	// output for PerformanceMetrics
		ofstream metricsOut(perfFilePath);
		if (metricsOut.is_open())
		{
			float cpi = static_cast<float>(cycle) / totalInstructions;
			float ipc = static_cast<float>(totalInstructions) / cycle;

			metricsOut << "-----------------------------Single Stage Core Performance Metrics-----------------------------" << endl;
			metricsOut << "Number of cycles taken: " << cycle << endl;
			metricsOut << "Total Number of Instructions: " << totalInstructions << endl;
			metricsOut << "Cycles per instruction (CPI): " << cpi << endl;
			metricsOut << "Instructions per cycle (IPC): " << ipc << endl;

			metricsOut.close();
		}
		else
		{
			cout << "Unable to open performance metrics output file." << endl;
		}
	}

private:
	string opFilePath;
	string perfFilePath;
	int totalInstructions = 0;
};

class FiveStageCore : public Core
{
public:
    FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem) : Core(ioDir + "\\FS_", imem, dmem), opFilePath(ioDir + "\\StateResult_FS.txt"), perfFilePath(ioDir + "\\PerformanceMetrics_SS.txt") {} //! __________________

    void step()
    {
        if (cycle == 0)
        {
            state.EX.nop = true;
            state.MEM.nop = true;
            state.WB.nop = true;

            nextState.EX.nop = true;
            nextState.MEM.nop = true;
            nextState.WB.nop = true;
        }

        cout << "---------------- Cycle: " << cycle << " ----------------" << endl;

        /* --------------------- WB stage --------------------- */
        if (!state.WB.nop)
        {
            totalInstructions++;
            if (state.WB.wrt_enable && state.WB.rd.to_ulong() != 0)
            {
                myRF.writeRF(state.WB.rd, state.WB.Wrt_data);
            }
        }

        /* --------------------- MEM stage --------------------- */
        if (!state.MEM.nop)
        {
            if (state.MEM.rd_mem)
            {
                nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
            }
            else if (state.MEM.wrt_mem)
            {
                ext_dmem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
            }
            else
            {
                nextState.WB.Wrt_data = state.MEM.ALUresult;
            }

            nextState.WB.rd = state.MEM.rd;
            nextState.WB.wrt_enable = state.MEM.wrt_enable;
            nextState.WB.nop = false;
        }
        else
        {
            nextState.WB.nop = true;
        }

        // /* --------------------- EX stage --------------------- */
        if (!state.EX.nop)
        {
            // Propagate the current instruction to the MEM stage
            nextState.MEM.ALUresult = bitset<32>(0);
            nextState.MEM.Store_data = state.EX.Read_data2;
            nextState.MEM.rd = state.EX.rd;
            nextState.MEM.rd_mem = state.EX.rd_mem;
            nextState.MEM.wrt_mem = state.EX.wrt_mem;
            nextState.MEM.wrt_enable = state.EX.wrt_enable;

            // Process instruction types
            if (state.EX.is_I_type)
            {
                nextState.MEM.ALUresult = handleIType(state.EX.rs1, state.EX.func3, state.EX.Imm_I, myRF);
            }
            else if (state.EX.rd_mem)
            {
                nextState.MEM.ALUresult = handleLoad(state.EX.rs1, state.EX.Imm_I, myRF);
            }
            else if (state.EX.wrt_mem)
            {
                auto storeResult = handleStore(state.EX.rs1, state.EX.rs2, state.EX.Imm_S, myRF);
                nextState.MEM.ALUresult = storeResult.first;
                nextState.MEM.Store_data = storeResult.second;
            }
            else
            {
                nextState.MEM.ALUresult = handleRType(state.EX.rd, state.EX.rs1, state.EX.rs2, state.EX.func3, state.EX.func7, myRF);
            }

            nextState.MEM.nop = false;
        }
        else
        {
            nextState.MEM.nop = true;
        }

        /* --------------------- ID stage --------------------- */

        int stallCounter = 0;

        if (!state.ID.nop)
        {
            bitset<32> instruction = state.ID.Instr;
            InstructionFields fields = checkInstr(instruction);

            bool hazard = (state.EX.rd.to_ulong() != 0 &&
                           (state.EX.rd == fields.rs1 || state.EX.rd == fields.rs2)) &&
                          state.EX.wrt_enable;

            if (hazard)
            {
                cout << "Hazard detected. Stalling pipeline." << endl;

                cout << "Hazard detected in cycle: " << cycle << endl;
                cout << "EX.rd: " << state.EX.rd << " Fields.rs1: " << fields.rs1 << " Fields.rs2: " << fields.rs2 << endl;
                cout << "EX.wrt_enable: " << state.EX.wrt_enable << endl;

                nextState.ID = state.ID; // Keep ID stage instruction the same
                nextState.EX.nop = true; // Insert bubble in EX stage

                // Freeze PC in IF stage
                nextState.IF = state.IF;
                stallCounter++;

                return; 
            }
            else
            {
                // Reset stall counter once hazard clears
                if (stallCounter > 0)
                {
                    cout << "Hazard cleared after " << stallCounter << " cycles." << endl;
                    stallCounter = 0;
                }
            }

            // Normal decoding if no hazard
            bitset<7> opcode = bitset<7>(instruction.to_ulong() & 0x7F);

            if (opcode == bitset<7>("0110011")) // R-Type
            {
                nextState.EX.rd = fields.rd;
                nextState.EX.rs1 = fields.rs1;
                nextState.EX.rs2 = fields.rs2;
                nextState.EX.func3 = fields.funct3;
                nextState.EX.func7 = fields.funct7;
                nextState.EX.is_I_type = false;
                nextState.EX.rd_mem = false;
                nextState.EX.wrt_mem = false;
                nextState.EX.wrt_enable = true;
            }
            else if (opcode == bitset<7>("0010011")) // I-Type
            {
                nextState.EX.rd = fields.rd;
                nextState.EX.rs1 = fields.rs1;
                nextState.EX.func3 = fields.funct3;
                nextState.EX.Imm_I = fields.imm_I;
                nextState.EX.is_I_type = true;
                nextState.EX.rd_mem = false;
                nextState.EX.wrt_mem = false;
                nextState.EX.wrt_enable = true;
            }
            else if (opcode == bitset<7>("0000011")) // Load
            {
                nextState.EX.rd = fields.rd;
                nextState.EX.rs1 = fields.rs1;
                nextState.EX.func3 = fields.funct3;
                nextState.EX.Imm_I = fields.imm_I;
                nextState.EX.is_I_type = true;
                nextState.EX.rd_mem = true;
                nextState.EX.wrt_mem = false;
                nextState.EX.wrt_enable = true;
            }
            else if (opcode == bitset<7>("0100011")) // Store
            {
                nextState.EX.rs1 = fields.rs1;
                nextState.EX.rs2 = fields.rs2;
                nextState.EX.func3 = fields.funct3;
                nextState.EX.Imm_S = fields.imm_S;
                nextState.EX.is_I_type = false;
                nextState.EX.rd_mem = false;
                nextState.EX.wrt_mem = true;
                nextState.EX.wrt_enable = false;
            }
            else if (opcode == bitset<7>("1100011")) // Branch
            {
                nextState.EX.rs1 = fields.rs1;
                nextState.EX.rs2 = fields.rs2;
                nextState.EX.func3 = fields.funct3;
                nextState.EX.Imm_B = fields.imm_B;
                nextState.EX.is_I_type = false;
                nextState.EX.rd_mem = false;
                nextState.EX.wrt_mem = false;
                nextState.EX.wrt_enable = false;
            }
            else if (opcode == bitset<7>("1101111")) // Jump
            {
                nextState.EX.rd = fields.rd;
                nextState.EX.Imm_J = fields.imm_J;
                nextState.EX.is_I_type = false;
                nextState.EX.rd_mem = false;
                nextState.EX.wrt_mem = false;
                nextState.EX.wrt_enable = true;
            }
            else
            {
                nextState.EX.nop = true; 
            }

            nextState.EX.nop = false;
        }
        else
        {
            nextState.EX.nop = true;
        }

        /* --------------------- IF stage --------------------- */
        if (!state.IF.nop)
        {
            if (stallCounter > 0)
            {
                // Stall the PC and instruction fetch during hazard
                nextState.IF = state.IF;
            }
            else
            {
                // Normal instruction fetch and PC increment
                bitset<32> instruction = ext_imem.readInstr(state.IF.PC);
                if (instruction.to_ulong() == 0xFFFFFFFF) // HALT instruction
                {
                    nextState.IF.nop = true;
                    nextState.ID.nop = true;
                    halt = true;
                }
                else
                {
                    nextState.ID.Instr = instruction;
                    nextState.ID.nop = false;
                    nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
                }
            }
        }
        else
        {
            nextState.ID.nop = true;
        }

        // Check if pipeline is halted
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
        {
            halted = true;
            cout << "Program halted." << endl;
            return;
        }

        // Update pipeline state
        myRF.outputRF(cycle);
        printState(nextState, cycle);
        state = nextState;
        cycle++;
    }

    //! HELPERS

    bool canForward(stateStruct &state)
    {
        return (state.MEM.wrt_enable && state.MEM.rd.to_ulong() != 0) ||
               (state.WB.wrt_enable && state.WB.rd.to_ulong() != 0);
    }

    bitset<32> handleRType(bitset<5> rd, bitset<5> rs1, bitset<5> rs2, bitset<3> func3, bitset<7> func7, RegisterFile &myRF)
    {
        int32_t rs1_val = static_cast<int32_t>(myRF.readRF(rs1).to_ulong());
        int32_t rs2_val = static_cast<int32_t>(myRF.readRF(rs2).to_ulong());
        int32_t result = 0;

        if (func3 == bitset<3>("000")) // ADD or SUB
        {
            if (func7 == bitset<7>("0000000")) // ADD
                result = rs1_val + rs2_val;
            else if (func7 == bitset<7>("0100000")) // SUB
                result = rs1_val - rs2_val;
        }
        else if (func3 == bitset<3>("100")) // XOR
            result = rs1_val ^ rs2_val;
        else if (func3 == bitset<3>("110")) // OR
            result = rs1_val | rs2_val;
        else if (func3 == bitset<3>("111")) // AND
            result = rs1_val & rs2_val;

        return bitset<32>(result); // Return the ALU result
    }

    bitset<32> handleIType(bitset<5> rs1, bitset<3> func3, bitset<12> Imm_I, RegisterFile &myRF)
    {
        int32_t rs1_val = static_cast<int32_t>(myRF.readRF(rs1).to_ulong());
        int32_t imm_val = signExtend(Imm_I, 12); // Convert to signed 32-bit
        int32_t result = 0;

        if (func3 == bitset<3>("000")) // ADDI
            result = rs1_val + imm_val;
        else if (func3 == bitset<3>("100")) // XORI
            result = rs1_val ^ imm_val;
        else if (func3 == bitset<3>("110")) // ORI
            result = rs1_val | imm_val;
        else if (func3 == bitset<3>("111")) // ANDI
            result = rs1_val & imm_val;

        return bitset<32>(result); // Return the ALU result
    }

    bitset<32> handleLoad(bitset<5> rs1, bitset<12> Imm_I, RegisterFile &myRF)
    {
        int32_t rs1_val = static_cast<int32_t>(myRF.readRF(rs1).to_ulong());
        int32_t imm_val = signExtend(Imm_I, 12); // Convert to signed 32-bit
        int32_t address = rs1_val + imm_val;
        return bitset<32>(address); // Return the memory address
    }

    pair<bitset<32>, bitset<32>> handleStore(bitset<5> rs1, bitset<5> rs2, bitset<12> Imm_S, RegisterFile &myRF)
    {
        int32_t rs1_val = static_cast<int32_t>(myRF.readRF(rs1).to_ulong());
        int32_t rs2_val = static_cast<int32_t>(myRF.readRF(rs2).to_ulong());
        int32_t imm_val = signExtend(Imm_S, 12); // Convert to signed 32-bit
        int32_t address = rs1_val + imm_val;
        return {bitset<32>(address), bitset<32>(rs2_val)}; // Return address and store data
    }

    void handleBranch(bitset<5> rs1, bitset<5> rs2, bitset<3> func3, bitset<12> Imm_B, bitset<32> PC, stateStruct &nextState, RegisterFile &myRF)
    {
        int32_t rs1_val = static_cast<int32_t>(myRF.readRF(rs1).to_ulong());
        int32_t rs2_val = static_cast<int32_t>(myRF.readRF(rs2).to_ulong());
        int32_t imm_val = signExtend(Imm_B, 13); // Convert to signed 32-bit
        bool branchTaken = false;

        if (func3 == bitset<3>("000")) // BEQ
            branchTaken = (rs1_val == rs2_val);
        else if (func3 == bitset<3>("001")) // BNE
            branchTaken = (rs1_val != rs2_val);

        // Update the next PC based on whether the branch is taken
        nextState.IF.PC = branchTaken
                              ? bitset<32>(PC.to_ulong() + imm_val)
                              : bitset<32>(PC.to_ulong() + 4);

        cout << "Branch " << (branchTaken ? "Taken" : "Not Taken") << ", New PC=" << nextState.IF.PC << endl;
    }

    void handleJump(bitset<5> rd, bitset<20> Imm_J, bitset<32> PC, stateStruct &nextState, RegisterFile &myRF)
    {
        int32_t imm_val = signExtend(Imm_J, 20); // Convert to signed 32-bit
        bitset<32> targetPC = bitset<32>(PC.to_ulong() + imm_val);

        // Write the return address (PC + 4) to the destination register
        if (rd.to_ulong() != 0) // Avoid writing to x0
        {
            bitset<32> returnAddress = bitset<32>(PC.to_ulong() + 4);
            myRF.writeRF(rd, returnAddress);
            cout << "Jump: Writing return address to Register[" << rd << "] = " << returnAddress << endl;
        }

        // Update PC for the next instruction
        nextState.IF.PC = targetPC;
        cout << "Jump to PC=" << targetPC << endl;
    }

    void handleHalt(stateStruct &nextState)
    {
        nextState.IF.nop = true;
        nextState.ID.nop = true;
        nextState.EX.nop = true;
        nextState.MEM.nop = true;
        nextState.WB.nop = true;

        cout << "HALT: Pipeline stopped." << endl;
    }

    void printState(stateStruct state, int cycle)
    { // output for StateResult
        ofstream printstate(opFilePath, cycle == 0 ? std::ios_base::trunc : std::ios_base::app);
        if (printstate.is_open())
        {
            printstate << "----------------------------------------------------------------------\n";
            printstate << "State after executing cycle: " << cycle << "\n";
            printstate << "IF.nop: " << (state.IF.nop ? "True" : "False") << "\n";
            printstate << "IF.PC: " << state.IF.PC.to_ulong() << "\n";
            printstate << "ID.nop: " << (state.ID.nop ? "True" : "False") << "\n";
            printstate << "ID.Instr: " << state.ID.Instr << "\n";
            printstate << "EX.nop: " << (state.EX.nop ? "True" : "False") << "\n";
            printstate << "EX.Instr: " << state.EX.Instr << "\n";
            printstate << "EX.Read_data1: " << state.EX.Read_data1 << "\n";
            printstate << "EX.Read_data2: " << state.EX.Read_data2 << "\n";
            printstate << "EX.Imm: " << state.EX.Imm << "\n";
            printstate << "EX.Rs1: " << state.EX.rs1 << "\n";
            printstate << "EX.Rs2: " << state.EX.rs2 << "\n";
            printstate << "EX.Rd: " << state.EX.rd << "\n";
            printstate << "EX.is_I_type: " << state.EX.is_I_type << "\n";
            printstate << "EX.rd_mem: " << state.EX.rd_mem << "\n";
            printstate << "EX.wrt_mem: " << state.EX.wrt_mem << "\n";
            printstate << "EX.alu_op: " << (state.EX.alu_op ? "01" : "00") << "\n";
            printstate << "EX.wrt_enable: " << state.EX.wrt_enable << "\n";
            printstate << "MEM.nop: " << (state.MEM.nop ? "True" : "False") << "\n";
            printstate << "MEM.ALUresult: " << state.MEM.ALUresult << "\n";
            printstate << "MEM.Store_data: " << state.MEM.Store_data << "\n";
            printstate << "MEM.Rs1: " << state.MEM.rs1 << "\n";
            printstate << "MEM.Rs2: " << state.MEM.rs2 << "\n";
            printstate << "MEM.Rd: " << state.MEM.rd << "\n";
            printstate << "MEM.rd_mem: " << state.MEM.rd_mem << "\n";
            printstate << "MEM.wrt_mem: " << state.MEM.wrt_mem << "\n";
            printstate << "MEM.wrt_enable: " << state.MEM.wrt_enable << "\n";
            printstate << "WB.nop: " << (state.WB.nop ? "True" : "False") << "\n";
            printstate << "WB.Wrt_data: " << state.WB.Wrt_data << "\n";
            printstate << "WB.Rs1: " << state.WB.rs1 << "\n";
            printstate << "WB.Rs2: " << state.WB.rs2 << "\n";
            printstate << "WB.rd: " << state.WB.rd << "\n";
            printstate << "WB.wrt_enable: " << state.WB.wrt_enable << "\n";
        }

        printstate.close();
    }

    void outputPerformanceMetrics()
    { // output for PerformanceMetrics
        ofstream metricsOut(perfFilePath);
        if (metricsOut.is_open())
        {
            float cpi = static_cast<float>(cycle) / totalInstructions;
            float ipc = static_cast<float>(totalInstructions) / cycle;

            metricsOut << "-----------------------------Performace of Five Stage-----------------------------" << endl;
            metricsOut << "#Cycles -> " << cycle << endl;
            metricsOut << "#Instructions -> " << totalInstructions << endl;
            metricsOut << "CPI -> " << cpi << endl;
            metricsOut << "IPC -> " << ipc << endl;

            metricsOut.close();
        }
        else
        {
            cout << "Unable to open performance metrics output file." << endl;
        }
    }

private:
    string opFilePath;
    string perfFilePath;
    int totalInstructions = 0;
    bool halt = false; // Global halt flag to signal termination
};

int main(int argc, char *argv[])
{
    string ioDir = "";

    // Command-line argument handling
    if (argc == 3 && string(argv[1]) == "--iodir")
    {
        ioDir = argv[2];
        cout << "IO Directory: " << ioDir << endl;
    }
    else if (argc == 1)
    {
        cout << "Enter path containing the memory files: ";
        cin >> ioDir;
    }
    else
    {
        cout << "Invalid number of arguments. Usage: ./main --iodir <path_to_directory>" << endl;
        return -1;
    }

    InsMem imem = InsMem("Imem", ioDir);
    // DataMem dmem_ss = DataMem("SS", ioDir);
    DataMem dmem_fs = DataMem("FS", ioDir);

    // SingleStageCore SSCore(ioDir, imem, dmem_ss);
    FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (!FSCore.halted) // Exit loop if halt flag is true
    {
        FSCore.step();
    }

    dmem_fs.outputDataMem();

    FSCore.outputPerformanceMetrics();

    // Use outputDataMem to output data memory
    // SSCore.getDataMem().outputDataMem();

    // SSCore.outputPerformanceMetrics();

    return 0;
}
