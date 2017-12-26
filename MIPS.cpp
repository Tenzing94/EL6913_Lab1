/* Name: Tenzing Rabgyal
 * Net ID: tr1440
 * Course: EL-GY 6913-A “Computing Systems Architecture”
 * Instructor: Dr. Garg
 * Due Date: 10/29/2017
 * Lab1: Pipelined MIPS
 */

#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define ADDU 1         /* Function Bits (Hex): 21 */
#define SUBU 3         /* Function Bits (Hex): 23 */
#define AND 4          /* Function Bits (Hex): 24 */
#define OR  5          /* Function Bits (Hex): 25 */
#define NOR 7          /* Function Bits (Hex): 27 /*/
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

/* FUNCTION PROTOTYPES */
bitset<6> getOpcode (bitset<32>);
bitset<5> getRsBits (bitset<32>);
bitset<5> getRtBits (bitset<32>);
bitset<5> getRdBits (bitset<32>);
bitset<6> getFuncBits (bitset<32>);
bitset<3> getRtypeALUop (bitset<32>);
bitset<28> getJmpAddress (bitset<32>);
bitset<4> getJmpPCValue (bitset<32>);
bitset<32> combineJmpPCAndJmpAddr(bitset<4>, bitset<28>);
bitset<32> getITypeImmSignExt(bitset<32>);
bitset<32> getBranchAddr (bitset<32>, bitset<32>);

/* CLASSES */
class RF
{
public:
	bitset<32> ReadData1, ReadData2;
	RF() /* constructor */
	{
		Registers.resize(32);  /* 32 registers with each holding 32 bits */
		Registers[0] = bitset<32> (0);  /* the value of Register Zero is fixed to 0. <32> refers to the size of the binary bits. (0) is the value that the 32 binary bits represent */
	}

	void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable) // returns nothing
	{
		// implement the function by you.

		ReadData1 = Registers[RdReg1.to_ulong()]; // Convert the 5 bit RdReg1 value into unsigned long and then access the Register of that index. Then put the data from that register into ReadData1
		ReadData2 = Registers[RdReg2.to_ulong()];

		if (WrtEnable == 1)
		{
			Registers[WrtReg.to_ulong()] = WrtData; // if WrtEnable == 1, then place the 32 bit value of WrtData into the register file of the index WrtReg
		}

	}

	void OutputRF() /* This function has already been implemented for us */
	{
		ofstream rfout;
		rfout.open("RFresult.txt",std::ios_base::app);
		if (rfout.is_open())
		{
			rfout<<"A state of RF:"<<endl;
			for (int j = 0; j<32; j++)
			{
				rfout << Registers[j]<<endl;
			}

		}
		else cout<<"Unable to open file";
		rfout.close();

	}
private:
	vector<bitset<32> >Registers; /* Vector Format: vector<datatype> variablename */

};

class ALU
{
public:
	bitset<32> ALUresult;
	bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    		{
		// implement the ALU operations by you.

		if (ALUOP.to_ulong() == ADDU)
			ALUresult = oprand1.to_ulong() + oprand2.to_ulong(); // Since bitsets cannot be directly added, I used a bitset function named "to_ulong" to convert the values of the two operands into unsigned longs, and then add them
		else if (ALUOP.to_ulong() == SUBU)
			ALUresult = oprand1.to_ulong() - oprand2.to_ulong(); // Same as with addition
		else if (ALUOP.to_ulong() == AND)
			ALUresult = oprand1 & oprand2;
		else if (ALUOP.to_ulong() == OR)
			ALUresult = oprand1 | oprand2;
		else if (ALUOP.to_ulong() == NOR)
			ALUresult = ~(oprand1 | oprand2);

		return ALUresult;
    		}
};

class INSMem
{
public:
	bitset<32> Instruction;
	INSMem() /* constructor */
	{
		IMem.resize(MemSize); /* MemSize is 65536 */
		ifstream imem;
		string line;
		int i=0;
		imem.open("imem.txt");
		if (imem.is_open())
		{
			while (getline(imem,line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout<<"Unable to open file";
		imem.close();

	}

	bitset<32> ReadMemory (bitset<32> ReadAddress)
    		{
		vector<bitset<8> > currIns(4);// I will use currIns vector to temporarily hold four 8 bit values of the word
		string stringWord = "";
		unsigned long insIncrementer;

		for (int i = 0; i <=3; i++) // Loop four times to get one byte of data in each loop (four bytes in total)
		{
			insIncrementer = ReadAddress.to_ulong() + i; // This variable will be used to increment over 4 bytes of data to get a word

			currIns[i] = IMem[insIncrementer]; // In currIns vector, put the 8 bit data in the location insIncrementer
		}

		stringWord = currIns[0].to_string() + currIns[1].to_string() + currIns[2].to_string() + currIns[3].to_string(); // the string of the 32 bit word we need

		Instruction = bitset<32>(stringWord);
		/*
         for (int i = 0; i < 32; i++) // this loop will check every character in the string and if the character is '1', it will set the corresponding bit of the Instruction. Else if the character is '0', it will reset the corresponding bit of the Instruction.
         {
         if (stringWord.at(i) == '1') // index i will go from left to right
         Instruction.set(31 - i); // but instruction bitset is indexed from right to left, therefore I used 31 - i

         else if (stringWord.at(i) == '0')
         Instruction.reset(31 - i);
         }
		 */
		return Instruction;
    		}

private:
	vector<bitset<8> > IMem; /* Vector Format: vector<datatype> variablename */

};

class DataMem
{
public:
	bitset<32> readdata;
	DataMem() /* constructor */
	{
		DMem.resize(MemSize); /* resize is the member function of vector class */
		ifstream dmem;
		string line;
		int i=0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem,line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout<<"Unable to open file";
		dmem.close();

	}

	bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
    		{

		string stringWrtData = "", stringByte1 = "", stringByte2 = "", stringByte3 = "", stringByte4 = "";
		bitset<8> byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0; // I will use these to store the bitset value of the four substrings
		vector<bitset<8> > currData(4);
		string stringDataWord = "";
		unsigned long dataIncrementer;

		if (writemem.to_ulong() == 1)
		{
			dataIncrementer = Address.to_ulong();
			stringWrtData = WriteData.to_string();     // Convert the 32 bit data into a string
			stringByte1 = stringWrtData.substr(0,8);   // make a substring with bits 0 to 7
			stringByte2 = stringWrtData.substr(8,8);   // make a substring with bits 8 to 15
			stringByte3 = stringWrtData.substr(16,8);  // make a substring with bits 16 to 23
			stringByte4 = stringWrtData.substr(24,8);  // make a substring with bits 24 to 31

			byte1 = bitset<8>(stringByte1);
			byte2 = bitset<8>(stringByte2);
			byte3 = bitset<8>(stringByte3);
			byte4 = bitset<8>(stringByte4);

			/*
             for (int i = 0; i < 8; i++)
             {
             if (stringByte1.at(i) == '1')
             byte1.set(7 - i);

             else if (stringByte1.at(i) == '0')
             byte1.reset(7 - i);
             }
             for (int i = 0; i < 8; i++)
             {
             if (stringByte2.at(i) == '1')
             byte2.set(7 - i);

             else if (stringByte2.at(i) == '0')
             byte2.reset(7 - i);
             }
             for (int i = 0; i < 8; i++)
             {
             if (stringByte3.at(i) == '1')
             byte3.set(7 - i);

             else if (stringByte3.at(i) == '0')
             byte3.reset(7 - i);
             }
             for (int i = 0; i < 8; i++)
             {
             if (stringByte4.at(i) == '1')
             byte4.set(7 - i);

             else if (stringByte4.at(i) == '0')
             byte4.reset(7 - i);
             }
			 */

			DMem[dataIncrementer] = byte1;      // dataIncrementer is a ulong of ReadAddress
			DMem[dataIncrementer + 1] = byte2;  // This is used to access the address next to ReadAddress
			DMem[dataIncrementer + 2] = byte3;  // This is used to access the address two spots away from ReadAddress
			DMem[dataIncrementer + 3] = byte4;  // This is used to access the address three spots away from ReadAddress
		}

		else if (readmem.to_ulong() == 1)
		{
			for (int i = 0; i <=3; i++) // Loop four times to get one byte of data in each loop (four bytes in total)
			{
				dataIncrementer = Address.to_ulong() + i; // This variable will be used to increment over 4 bytes of data to get a word

				currData[i] = DMem[dataIncrementer];
			}

			stringDataWord = currData[0].to_string() + currData[1].to_string() + currData[2].to_string() + currData[3].to_string();

			readdata = bitset<32>(stringDataWord);
			/*
             for (int i = 0; i < 32; i++) // this loop will check every character in the string and if the character is '1', it will set the corresponding bit of the Instruction. Else if the character is '0', it will reset the corresponding bit of the Instruction.
             {
             if (stringDataWord.at(i) == '1') // index i will go from left to right
             readdata.set(31 - i); // but readdata bitset is indexed from right to left, therefore I used 31 - i

             else if (stringDataWord.at(i) == '0')
             readdata.reset(31 - i);
             }
			 */
		}

		return readdata;
    		}

	void OutputDataMem() /* This function has already been implemented for us */
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j< 1000; j++)
			{
				dmemout << DMem[j]<<endl;
			}

		}
		else cout<<"Unable to open file";
		dmemout.close();

	}

private:
	vector<bitset<8> > DMem; /* Vector Format: vector<datatype> variablename */

};

void dumpResults(bitset<32> pc, bitset<5> WrRFAdd, bitset<32> WrRFData, bitset<1> RFWrEn, bitset<32> WrMemAdd, bitset<32> WrMemData, bitset<1> WrMemEn) /* This function has already been implemented for us */
{
	ofstream fileout;

	fileout.open("Results.txt",std::ios_base::app);
	if (fileout.is_open())
	{

		fileout <<pc<<' '<<WrRFAdd<<' '<<WrRFData<<' '<<RFWrEn<<' '<<WrMemAdd<<' '<<WrMemData<<' '<<WrMemEn << endl;

	}
	else cout<<"Unable to open file";
	fileout.close();

}

/* MAIN */
int main()
{
	bitset<32> pc = 0;
	RF myRF; /* myRF is an object of RF class */
	ALU myALU;
	INSMem myInsMem;
	DataMem myDataMem;

	bitset<32> curIns; // Current Instruction
	bitset<6> opcode; // This is the 6 bit opcode that defines the instruction's purpose
	bitset<6> copyOpcode;/////////////////
	bitset<5> regRs, regRt, regRd; // Registers rs, rt, and rd. All are 5 bits
	bitset<6> func; // last 6 bits of an R-type instruction
	bitset<3> aluOpcode; // 3 bit opcode used by the ALU
	bitset<28> jmpAddress; // 26 bits from the J-Type instruction and it needs to be multiplied by 4;
	bitset<4> jmpPCValue; // the 4 MSB bits of the PC+4 value;
	bitset<32> realJmpAddress;
	bitset<32> immSignExt; // 16 bits of immediate for I-Type Instructions
	bitset<32> aluResult;
	bitset<32> copyAluResult;
	bitset<32> branchAddress;
	bitset<32> readData; // 32-bit coming from the DMem during lw instructions
	bool jmpFlag = false; // used for stopping Stage 2 after Jump Instruction
    bool jmpStopStg3 = false;
    bool jmpStopStg2 = false;

	// Variables made specifically for dumpResults
	bitset<5> writeReg; // In R-Type, this is rd. In I Type, this is rt
	bitset<5> copyWriteReg, copyRegRs, copyRegRt;
	bitset<32> writeRFData;
	bitset<1> rfWriteEnable = 0;
	bitset<32> writeMemAddr;
	bitset<32> writeMemData;
	bitset<1> writeMemEnable = 0;
    
	bool enableStage3 = false;
	bool enableStage2 = false;

	bool stg3rtype = false, stg3addiu = false, stg3lw = false, stg3sw = false;

    
	while (1) //each loop body corresponds to one clock cycle.
	{
		copyWriteReg = writeReg;
		copyAluResult = aluResult;
		copyRegRs = regRs;
		copyRegRt = regRt;
        
        if (jmpFlag == true) // If there was a jump instruction decoded in the previous, jmpFlag was set to "true"
        {
            jmpStopStg2 = true; // That means that in this clock cycle, we need to stop stage 2 in order to squash instruction following the jump instruction
        }


		// ******************************(Stage 3) Memory and Writeback [MEM and WB]*********************************************
		if (enableStage3 == true && jmpStopStg3 == false)
        cout << "**Just Entered Stage 3**" << endl;
		{
			//*MEM*

			if (stg3lw == true)
			{
				writeMemEnable = 0; //variable for dumpResults
				readData = myDataMem.MemoryAccess(copyAluResult, myRF.ReadData2, 1, writeMemEnable); // aluResult = Rs + imm, ReadData2 = value at Rt [this is the location], readmem = 1, writemem = 0 [because we are not writing to DMem
				cout << "Inside Stage 3 MEM lw" << endl;
			}

			else if (stg3sw == true)
			{
				writeMemEnable = 1; //variable for dumpResults
				myDataMem.MemoryAccess(copyAluResult, myRF.ReadData2, 0, writeMemEnable);
				writeMemAddr = aluResult; //variable for dumpResults
				writeMemData = myRF.ReadData2; //variable for dumpResults
				cout << "Inside Stage 3 MEM sw" << endl;
			}


			//*WB*

			if (stg3lw == true)
			{
				rfWriteEnable = 1; //variable for dumpResults
				myRF.ReadWrite(copyRegRs, copyWriteReg, copyWriteReg, readData, rfWriteEnable);
				writeRFData = readData; //variable for dumpResults
				cout << "Inside Stage 3 WB lw" << endl;
			}

			else if (stg3addiu == true)
			{
				rfWriteEnable = 1; //variable for dumpResults
				myRF.ReadWrite(copyRegRs, copyWriteReg, copyWriteReg, readData, rfWriteEnable);
				writeRFData = readData; //variable for dumpResults
				cout << "Inside Stage 3 WB addiu" << endl;
			}

			else if (stg3rtype == true)
			{
				rfWriteEnable = 1; //variable for dumpResults
				myRF.ReadWrite(copyRegRs, copyRegRt, copyWriteReg, copyAluResult, rfWriteEnable);
				writeRFData = readData; //variable for dumpResults
				cout << "Inside Stage 3 WB rtype" << endl;
			}

		}
        jmpStopStg3 = false;
		stg3rtype = false;
		stg3addiu = false;
		stg3lw = false;
		stg3sw = false;
        
        if (jmpFlag == true)
        {
            jmpStopStg3 = true; // If a jump instruction was fetched in the previous clock cycle , this will stop stage 3 in the next clock cycle.
        }
        
        jmpFlag = false; // Once we have set jmpStopStg2 to stop Stage 2 in this clock cycle) and jmpStopStg3 (to stop Stage 3 in the next clock cycle), we can now set the jmpFlag to false

		// ******************************(Stage 2) Instruction Decode/Register File and Execute [ID/RF and EX]*******************
		if (enableStage2 == true && jmpStopStg2 == false) // skips Stage 2 in first clock cycle && Whenever there is a Jump Instruction decoded in the previous clock cycle
		{
            cout << "**Just Entered Stage 2**" << endl;
			// *ID/RF*
			opcode = getOpcode(curIns); // getOpcode is a function that I made

			if (opcode == 0x00) // R-Type Instructions. For addu, subu, and, or , nor. all have opcode 0x00
			{
				stg3rtype = true; // In stage 3, make the boolean variable tg3rtype == true
				regRs = getRsBits(curIns); // Fetches the 5 bits from the Rs part of the instruction
				regRt = getRtBits(curIns); // Fetches the 5 bits from the Rt part of the instruction
				regRd = getRdBits(curIns); // Fetches the 5 bits from the Rd part of the instruction
				writeReg = regRd;
				func = getFuncBits(curIns); // Fetches the 6 bits from the func part of the instruction
				aluOpcode = getRtypeALUop(curIns); // Fetches the last 3 bits of an R-Type instruction
				myRF.ReadWrite(regRs, regRt, writeReg, 0, 0); // Reading here, so last two values are zero
				cout << "Inside Stage 2 IDRF rtype" << endl;
			}
			else if (opcode == 0x02) // MIPS's J-Type has two opcodes. But since this program doesn't use "jal" instruction, we only have to worry about implementing the "j" instruction (opcode: 0x02)
			{
				jmpAddress = getJmpAddress(curIns);
				jmpPCValue = getJmpPCValue(pc); // The 4 MSBs of PC+4
				realJmpAddress = combineJmpPCAndJmpAddr(jmpPCValue, jmpAddress);
				jmpFlag = true;
				pc = realJmpAddress;
				cout << "Inside Stage 2 IDRF jtype" << endl;
			}

			else if (opcode == 0x09 || opcode == 0x04 || opcode == 0x23 || opcode == 0x2b) // I-Type Instructions. For addiu (0x09), beq (0x04), lw (0x23), and sw (0x2b)
			{
				if (opcode == 0x09)
					stg3addiu = true;
				else if (opcode == 0x23)
					stg3lw = true;
				else if (opcode == 0x2b)
					stg3sw = true;

				regRs = getRsBits(curIns); // Fetches the 5 bits from the Rs part of the instruction
				regRt = getRtBits(curIns); // Fetches the 5 bits from the Rt part of the instruction
				writeReg = regRt;
				immSignExt = getITypeImmSignExt(curIns); // Fetches the 16 lower bits of the I-Type instruction
				cout << "Inside Stage 2 IDRF itype" << endl;


			}

			// *EX*
			if (opcode == 0x00) // R-Type Instructions. For addu, subu, and, or , nor. all have opcode 0x00
			{
				aluResult = myALU.ALUOperation(aluOpcode, myRF.ReadData1, myRF.ReadData2);
				cout << "Inside Stage 2 EX rtype" << endl;
			}

			// J-Type instructions do not use ALU so we don't have to worry about that here


			else if (opcode == 0x04) // beq (opcode: 0x04)
			{
				if (myRF.ReadData1 == myRF.ReadData2) // same as subtracting the two values in ALU and checking if the result is zero
				{
					branchAddress = getBranchAddr(pc, immSignExt);
					pc = pc.to_ulong() + branchAddress.to_ulong() + 4;
				}
				cout << "Inside Stage 2 EX beq" << endl;
			}
			else if (opcode == 0x09 || opcode == 0x23 || opcode == 0x2b) // addiu, lw, sw
			{
				aluOpcode = 0x01; // addu, addiu, lw, sw all have the same ALU opcode
				aluResult = myALU.ALUOperation(aluOpcode, myRF.ReadData1, immSignExt);
				cout << "Inside Stage 2 EX addiu, lw, sw" << endl;
			}

		enableStage3 = true;

		}
        jmpStopStg2 = false;



		// ******************************(Stage 1) Instruction Fetch [IF]********************************************************
        cout << "**Just Entered Stage 1**" << endl;
		curIns = myInsMem.ReadMemory(pc.to_ulong()); // fetch the current instruction, which is located in memory location 0
		cout << "Inside Stage 1 IF" << endl;
		if (curIns.to_ulong() == 4294967295) // binary "11111111111111111111111111111111"
		{
			cout << "HALT! Instruction 0xFFFFFFFF was just fetched. Terminate the Program." << endl;
			break; // this line will force the while loop to stop
		}


		// At the end of each cycle, fill in the corresponding data into "dumpResults" function to output files.
		// The first dumped pc value should be 0.
		dumpResults(pc, copyWriteReg, writeRFData, rfWriteEnable, writeMemAddr, writeMemData, writeMemEnable); // (bitset<32> pc, bitset<5> WrRFAdd, bitset<32> WrRFData, bitset<1> RFWrEn, bitset<32> WrMemAdd, bitset<32> WrMemData, bitset<1> WrMemEn)

		rfWriteEnable = 0;
		writeMemEnable = 0;
		copyWriteReg = 0;
        writeRFData = 0;
        writeMemAddr = 0;
        writeMemData = 0;
        

		pc = pc.to_ulong() + 4;

		enableStage2 = true;


	}
    cout << "PC value at the end of the program: " << pc.to_ulong() << endl;

	myRF.OutputRF(); // dump RF into a textfile named "RFresult.txt"
	myDataMem.OutputDataMem(); // dump data mem

	return 0;

}


/* FUNCTIONS */
bitset<6> getOpcode (bitset<32> ins)
{
	string insString = ins.to_string();
	string opcodeString = insString.substr(0,6);
	bitset<6> opcodeBits = bitset<6>(opcodeString);
	return opcodeBits;
}

bitset<5> getRsBits (bitset<32> ins)
{
	string insString = ins.to_string();
	string rsString = insString.substr(6,5);
	bitset<5> rsBits = bitset<5>(rsString);
	return rsBits;
}
bitset<5> getRtBits (bitset<32> ins)
{
	string insString = ins.to_string();
	string rtString = insString.substr(11,5);
	bitset<5> rtBits = bitset<5>(rtString);
	return rtBits;
}
bitset<5> getRdBits (bitset<32> ins)
{
	string insString = ins.to_string();
	string rdString = insString.substr(16,5);
	bitset<5> rdBits = bitset<5>(rdString);
	return rdBits;
}

bitset<6> getFuncBits (bitset<32> ins) // For R-Type only
{
	string insString = ins.to_string();
	string funcString = insString.substr(26,6);
	bitset<6> funcBits = bitset<6>(funcString);
	return funcBits;
}

bitset<3> getRtypeALUop (bitset<32> ins)
{
	string insString = ins.to_string();
	string aluString = insString.substr(29,3);
	bitset<3> aluBits = bitset<3>(aluString);
	return aluBits;
}

bitset<28> getJmpAddress (bitset<32> ins)
{
	string insString = ins.to_string();
	string jmpAddressString = insString.substr(6,26);
	bitset<26> jmpAddrBits = bitset<26>(jmpAddressString);
	bitset<28> realJmpAddrBits = jmpAddrBits.to_ulong() * 4; // add two bits to the end of the 26 bit value
	return realJmpAddrBits;
}

bitset<4> getJmpPCValue (bitset<32> pc)
{
	pc = pc.to_ulong() + 4; // PC + 4
	string pcString = pc.to_string();
	string msbPcValue = pcString.substr(0,4);
	bitset<4> pcBits = bitset<4>(msbPcValue);
	return pcBits;
}

bitset<32> combineJmpPCAndJmpAddr(bitset<4> jmpPC, bitset<28> jmpAddress)
{
	string realJmpAddressString = jmpPC.to_string() + jmpAddress.to_string();
	bitset<32> realJmpAddressBits = bitset<32>(realJmpAddressString);
	return realJmpAddressBits;
}

bitset<32> getITypeImmSignExt(bitset<32> ins)
{
	bitset<32> finalImmBits;
	string extend;
	string insString = ins.to_string();
	string immString = insString.substr(16,16);

	if (immString.at(0) == '0')
		extend = "0000000000000000";
	else
		extend = "1111111111111111";

	string finalImmString = extend + immString;
	finalImmBits = bitset<32>(finalImmString);
	return finalImmBits;
}

bitset<32> getBranchAddr (bitset<32> pc, bitset<32> imm)
{
	pc = pc.to_ulong() + 4; // PC + 4
	imm = imm.to_ulong() * 4; // two 0 bits added to the right hand side of imm
	bitset<32> branchAddrBits = pc.to_ulong() + imm.to_ulong();
	return branchAddrBits;
}

