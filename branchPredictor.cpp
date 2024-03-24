#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<algorithm>
#define INPUTFILE "test_input"
using namespace std;


// N stands for NotTaken and T stands for Taken.
const string conv = "0123456789abcdef";
map<char, int> hexTodec;
map<string,string> BranchTargetBuffer; // Map from pc string of a branch instruction,
					//to the pc string of next instruction (correct next address last time branch was encountered)

					//Basically the same as a 1-bit branch predictor, with the only difference being that it stores the
					//address we jumped to last time, instead of storing only T or N.

map<string,string> BranchHistoryTable; // Map from pc string of a branch instruction to a history string.
					//history string should look something like this: "TTTTNNT"
					//result of each time the branch is encountered should be appended to its history string.


int instructionCount = 1e6;
int predictionCount = 0; //Total number of predictions.
int alwaysTaken = 0; //Number of correct predictions made by branch predictor that says "Always Taken".
int alwaysNotTaken = 0; //Number of correct predictions made by branch predictor that says "Always Not Taken".

int oneBit = 0; //Number of correct predictions made by branch predictor that predicts the according to the previous time same branch was encountered.
		// T => T and N => N.
		// For first encounter, it predicts N.
		
int twoBit = 0; // Number of correct predictions made by branch predictor that takes into account previous two times same branch was encountered.
		// TT => T, TN => T, NT => N, NN => N. (basically, prediction is same as second-to-last time branch was encountered)
		// For first and second encounter, it predicts N.


void createHexToDec()
{
	for (int i = 0; i < 16; i++) hexTodec[conv[i]] = i;
}


string getPC(ifstream& trace)
{
	string temp = "";
	if (trace.eof()) return temp;
	trace >> temp;
	trace >> temp;
	trace >> temp;
	return temp; // pc is a string of the form: 0x80005b98
}


int pow16(int x)
{
	int res = 1;
	while(x--) res *= 16;
	return res;	
}


string getMC(ifstream& trace)
{
	string temp = "";
	if (trace.eof()) return temp;
	trace >> temp;
	string mc = temp.substr(1,10);
	getline(trace,temp);
	return mc; // mc is a string of the form: 0x02051663
}


bool isBranch(string mc)
{
	if (mc.size() > 1) mc.erase(0,2); // getting rid of the 0x.

	int opcodeFinder = 0x0000007F;
	int machineCode = 0;

	for (int i = 7; i >= 0; i--) machineCode += hexTodec[mc[7-i]]*(pow16(i)); //Converting machine code to integer.
	
	int opcode = machineCode&opcodeFinder;
	
	return opcode == 0x00000063;
}


int main()
{
	createHexToDec();
	ifstream trace(INPUTFILE); //opening file with trace.
	bool getNextInstruction = true;
	string pc = "", mc = "";
	while(instructionCount-- && !trace.eof())
	{
		if (getNextInstruction)
		{
			pc = getPC(trace);
			mc = getMC(trace);
		}
		else getNextInstruction = true;
		if(isBranch(mc))
		{
			/* Calculate target pc using current pc and offset from mc */
			/* Make predictions */
			
			string nextpc = getPC(trace);
			
			/* Check if prediction was correct */
			/* Update counters for each kind of branch predictor */
			/* Append result to branch history table */
			/* Update BTB if needed */

			pc = nextpc;
			mc = getMC(trace);
			getNextInstruction = false; // In next iteration, don't use getPC() and getMC() as pc and mc already correspond to next instruction.
		}
	}
	
	
	/* Print accuracy for each kind of branch predictor */
	
	/* Print the branch target buffer */
	return 0;
}
