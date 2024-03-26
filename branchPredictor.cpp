#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<algorithm>
#include<bitset>
#include<unordered_map>
#define INPUTFILE "Fac_test_Lab"
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
int alwaysTakencount = 0; //Number of correct predictions made by branch predictor that says "Always Taken".
int alwaysNotTakencount = 0; //Number of correct predictions made by branch predictor that says "Always Not Taken".

int oneBitcount = 0; //Number of correct predictions made by branch predictor that predicts the according to the previous time same branch was encountered.
// T => T and N => N.
// For first encounter, it predicts N.

int twoBitcount = 0; // Number of correct predictions made by branch predictor that takes into account previous two times same branch was encountered.
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
	if (temp=="") return temp;
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


long int pow2(long int x)
{
	long int res = 1;
	while(x--) res *= 2;
	return res;
}

string addBinary(string offset, string pc)
{
        long int pc_int=0, offset_int=0;
        for (int i = 0; i < pc.size(); i++)
        {
        	if (pc[i] == '1') pc_int += pow2((long)(pc.size()-i-1));
        }
        
        for (int i = 1; i < offset.size(); i++)
        {
        	if (offset[i] == '1') offset_int += pow2((long)(offset.size()-i-1));
        }
        if (offset[0] == '1') offset_int -= pow2((long)(offset.size()-1));
        
        string res;
        long int res_int = pc_int + offset_int;
        
        for (int i = 0; i < 32; i++)
       	{
       		if (res_int&1) res += '1';
       		else res += '0';
       		res_int /= 2;
       	}
       	reverse(res.begin(),res.end());
//       	cout << pc_int << "," + pc + " + " << offset_int << "," + offset + " = " << res_int << "," + res << endl;
       	return res;
}


string hexToBinary(string input)
{
	input.erase(0,2);
	unsigned int x =  stoll(input, nullptr, 16) ;
	string result = bitset<32>(x).to_string();
	return result;
}

string bin_to_hex(string binary) {
	binary = string(binary.length() % 4 ? 4 - binary.length() % 4 : 0, '0') + binary;
	unordered_map<string, char> hex_dict = {
	{"0000", '0'}, {"0001", '1'}, {"0010", '2'}, {"0011", '3'},
	{"0100", '4'}, {"0101", '5'}, {"0110", '6'}, {"0111", '7'},
	{"1000", '8'}, {"1001", '9'}, {"1010", 'a'}, {"1011", 'b'},
	{"1100", 'c'}, {"1101", 'd'}, {"1110", 'e'}, {"1111", 'f'}
	};

	string hexadecimal;
	for (size_t i = 0; i < binary.length(); i += 4) {
		string group = binary.substr(i, 4);
		hexadecimal += hex_dict[group];
	}
	return hexadecimal;
}


string alwaysTaken(string mc,string pc)
{
	string mcb=hexToBinary(mc);
	string pcb=hexToBinary(pc);
	// cout << mcb  + " " + pcb << endl;
	reverse(mcb.begin(),mcb.end());
	string offset="0"+mcb.substr(8,4)+mcb.substr(25,6)+mcb.substr(7,1)+mcb.substr(31,1);
	reverse(mcb.begin(),mcb.end());
	reverse(offset.begin(),offset.end());
//	cout << offset << " " << mcb << endl;
	// cout << offset << endl;
	string fin_=addBinary(offset,pcb);
	while (fin_.size() < 32) fin_ = '0' + fin_;
	// cout << fin_ << endl;
	return bin_to_hex(fin_);
}

string alwaysNotTaken(string mc,string pc)
{
	string pcb=hexToBinary(pc);
	long int pc_int = 0;
        for (int i = 0; i < pcb.size(); i++)
        {
        	if (pcb[i] == '1') pc_int += pow2((long)(pcb.size()-i-1));
        }

	pc_int += 4;
	string fin_ = "";
	for (int i = 0; i < 32; i++)
	{
		if (pc_int&1) fin_ += '1';
		else fin_ += '0';
		pc_int /= 2;
	}
	reverse(fin_.begin(), fin_.end());
	return bin_to_hex(fin_);
}


string _1bit(string mc,string pc)
{
	string pred=BranchHistoryTable[pc];
	if(pred.size() >= 1)
	{
		if(pred[pred.size()-1]=='T')
		return alwaysTaken(mc,pc);
		else
		return alwaysNotTaken(mc,pc);
	}
	else
	{
		BranchHistoryTable.erase(pc);
		return alwaysNotTaken(mc,pc);
	}
}


string _2bit(string mc,string pc)
{
	string pred=BranchHistoryTable[pc];
	if(pred.size() > 1)
	{
		if(pred[pred.size()-2]=='T')
		return alwaysTaken(mc,pc);
		else
		return alwaysNotTaken(mc,pc);
	}
	else if (pred.size() == 1)
	{
		return alwaysNotTaken(mc,pc);
	}
	else
	{
		BranchHistoryTable.erase(pc);
		return alwaysNotTaken(mc,pc);
	}
}


int main()
{
	createHexToDec();
	//cout << "it works" << endl;
	ifstream trace(INPUTFILE); //opening file with trace.
	bool getNextInstruction = true;
	string pc = "", mc = "";
	int mycount = 0;
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
			++mycount;
			string alwaysTakenPred=alwaysTaken(mc,pc);
			string alwaysNotTakenPred=alwaysNotTaken(mc,pc);
			string _1bitPred=_1bit(mc,pc);
			string _2bitPred=_2bit(mc,pc);

			string nextpc = getPC(trace);
//			cout << pc.substr(2,8) + " " + alwaysTakenPred + " " + alwaysNotTakenPred + " " + _1bitPred + " " + _2bitPred + " " + nextpc.substr(2,8) << endl;
			if(alwaysNotTakenPred==nextpc.substr(2,8)){
		//		cout << " N";
				alwaysNotTakencount++;
				BranchTargetBuffer[pc]=nextpc;
				auto it=BranchHistoryTable.find(pc);
				if(it!=BranchHistoryTable.end()){
				BranchHistoryTable[pc]=BranchHistoryTable[pc]+"N";
				}
				else{
				BranchHistoryTable[pc]="N";
				}
			}
			if(alwaysTakenPred==nextpc.substr(2,8)){
		//		cout << " T";
				alwaysTakencount++;
				BranchTargetBuffer[pc]=nextpc;
				auto it=BranchHistoryTable.find(pc);
				if(it!=BranchHistoryTable.end()){
					BranchHistoryTable[pc]=BranchHistoryTable[pc]+"T";
				}
				else{
					BranchHistoryTable[pc]="T";
				}
			}
			if(_1bitPred==nextpc.substr(2,8))oneBitcount++;
			if(_2bitPred==nextpc.substr(2,8))twoBitcount++;
			//cout << alwaysNotTakencount << endl;
			pc = nextpc;
			mc = getMC(trace);
			getNextInstruction = false; // In next iteration, don't use getPC() and getMC() as pc and mc already correspond to next instruction.
		}
	//	cout << pc << " " << mc << endl;
	}
	cout<<"Accuracy on "<<INPUTFILE<<endl;
	cout<<endl;
	float temporary=0;
	temporary=((float)alwaysTakencount/(alwaysNotTakencount+alwaysTakencount))*100;
	cout<<"The accuracy of the alwaysTaken Predictor is "<<temporary<<"%"<<endl;
	temporary=((float)alwaysNotTakencount/(alwaysNotTakencount+alwaysTakencount))*100;
	cout<<"The accuracy of the alwaysNotTaken Predictor is "<<temporary<<"%"<<endl;
	temporary=((float)oneBitcount/(alwaysNotTakencount+alwaysTakencount))*100;
	cout<<"The accuracy of the oneBit Predictor is "<<temporary<<"%"<<endl;
	temporary=((float)twoBitcount/(alwaysNotTakencount+alwaysTakencount))*100;
	cout<<"The accuracy of the twoBit Predictor is "<<temporary<<"%"<<endl;
	cout<<"BranchTargetBuffer"<<endl;
	cout<<endl;
	for(const auto& pair : BranchTargetBuffer){
		cout<<"pc: "<<pair.first<<", nextpc: "<<pair.second<<endl;
	}
	cout<<"BranchHistoryTable"<<endl;
	cout<<endl;
	for(const auto& pair : BranchHistoryTable){
		cout<<"pc: "<<pair.first<<", history: "<<pair.second<<endl;
	}

	return 0;
}
