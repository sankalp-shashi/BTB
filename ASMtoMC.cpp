#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<algorithm>
#include<bitset>
#include<unordered_map>
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
string addBinary(string A, string B)
{
        if (A.length() > B.length())
        return addBinary(B, A);
    int diff = B.length() - A.length();
 
    string padding;
    for (int i = 0; i < diff; i++)
        padding.push_back('0');
 
    A = padding + A;
    string res;
    char carry = '0';
 
    for (int i = A.length() - 1; i >= 0; i--) {
        if (A[i] == '1' && B[i] == '1') {
if (carry == '1')
                res.push_back('1'), carry = '1';
            else
                res.push_back('0'), carry = '1';
        }
        else if (A[i] == '0' && B[i] == '0') {
            if (carry == '1')
                res.push_back('1'), carry = '0';
            else
                res.push_back('0'), carry = '0';
        }
        else if (A[i] != B[i]) {
            if (carry == '1')
                res.push_back('0'), carry = '1';
            else
                res.push_back('1'), carry = '0';
        }
    }
if (carry == '1')
        res.push_back(carry);
    // reverse the result
    reverse(res.begin(), res.end());
 
    // To remove leading zeroes
    int index = 0;
    while (index + 1 < res.length() && res[index] == '0')
        index++;
    return (res.substr(index));
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
// cout << mc + " " + pc << endl;
string mcb=hexToBinary(mc);
string pcb=hexToBinary(pc);
// cout << mcb  + " " + pcb << endl;
reverse(mcb.begin(),mcb.end());
string offset="0"+mcb.substr(8,4)+mcb.substr(25,6)+mcb.substr(7,1)+mcb.substr(31,1);
// cout << offset << endl;
reverse(offset.begin(),offset.end());
string fin_=addBinary(offset,pcb);
// cout << fin_ << endl;
return bin_to_hex(fin_);
}

string alwaysNotTaken(string mc,string pc)
{
string pcb=hexToBinary(pc);
string offset="100";
string fin_=addBinary(offset,pcb);
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
else
{
return alwaysNotTaken(mc,pc);
}
}
int main()
{
createHexToDec();
cout << "it works" << endl;
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
string alwaysTakenPred=alwaysTaken(mc,pc);
string alwaysNotTakenPred=alwaysNotTaken(mc,pc);
string _1bitPred=_1bit(mc,pc);
string _2bitPred=_2bit(mc,pc);

string nextpc = getPC(trace);
//nextpc.erase(0,2);
cout << alwaysTakenPred + " " + alwaysNotTakenPred + " " + _1bitPred + " " + _2bitPred + " " + nextpc.substr(2,8) << endl;
if(alwaysNotTakenPred==nextpc.substr(2,8))alwaysNotTakencount++;
if(alwaysTakenPred==nextpc.substr(2,8))alwaysTakencount++;
if(_1bitPred==nextpc.substr(2,8))oneBitcount++;
if(_2bitPred==nextpc.substr(2,8))twoBitcount++;
cout << alwaysNotTakencount << endl;
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
