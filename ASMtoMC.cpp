#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<algorithm>
#define INPUTFILE "demoInput.asm"
#define OUTPUTFILE "demoOutput.mc"
using namespace std;
map<string,string> dataInc, opcode, f3, f7, formatType;
map<string,int> labelAddress;
// dataInc : maps directive name to number of bytes to skip.
// opcode : maps instruction name to string corresponding to opcode.
// f3 : maps instruction name to string corresponding to func3.
// f7 : maps instruction name to string corresponding to func7.
// formatType : maps instruction name to its format (R,I,S,SB,U,UJ).
// labelAddress : maps label names to addresses.

string inst, opname, dirname, mc, conv = "0123456789ABCDEF", dataSegment;
// inst : instruction currently being parsed.
// opname : name of the operation currently being parsed.
// dirname : name of the assembler directive currently being parsed.
// mc : the machine code string.
//conv : string to aid conversion to hexadecimal.
// dataSegment : the entire data segment (because it has to be printed after the text segment).


int pc = 0, dc = 0x10000000;
// pc : program counter
// dc : data counter

//Function List
//1. Preprocessing Functions.
void buildMaps(); //Calls all the other map building functions.
void buildRMaps(); //Creates and initializes maps related to R-type instructions.
void buildIMaps(); //Creates and initializes maps related to I-type instructions.
void buildSMaps(); //Creates and initializes maps related to S-type instructions.
void buildSBMaps(); //Creates and initializes maps related to SB-type instructions.
void buildUMaps(); //Creates and initializes maps related to U-type instructions.
void buildUJMaps(); //Creates and initializes maps related to UJ-type instructions.
void buildDirectiveMaps(); //Creates and initializes the dataInc map.
void prescan(); //Initially scans the input file to look for labels.


//2. Parsing Functions.
string getName(); //Returns the name of the instruction.
string getReg(); //Returns the next register in the instruction.
string getImm(); //Returns the immediate value that is part of the instruction. (I,S type).
string getImm1(); //Returns the immediate value that is part of the instruction. (SB type).
string getImm2(); //Returns the immediate value that is part of the instruction. (U type).
string getImm3(); //Returns the immediate value that is part of the instruction. (UJ type).
string getDirectiveName(); //Returns the name of assembler directive.
string getString(); //Reads next string from instruction.
void dataSegmentReader(istream& asmFile); //Reads the data segment of the program.


//3. Machine Code creating functions.
void formatMatcher(); //Calls the machine code creating functions of different formats.
void typeRmc();  //Creates the machine code for an R-type instruction.
void typeImc();  //Creates the machine code for an I-type instruction.(to be created)
void typeSmc(); //Creates the machine code for an S-type instruction.
void typeSBmc(); //Creates the machine code for an SB-type instruction.
void typeUmc(); //Creates the machine code for an U-type instruction.
void typeUJmc(); //Creates the machine code for an UJ-type instruction.


//4. Utility Functions.
string counterToHex(int counter); //Returns counter in hexadecimal.
string bin2hex(string s); //Converts binary machine code string to hexadecimal string.
int bitCount(unsigned long long int x); //Counts the number of bits in x.
string invalidString(string temp); //Informs user that the string entered is incorrect.
void invalidInstruction(); //Informs user that the instruction does not exist in the database.
void invalidDirective(); //Informs user that the directive does not exist in the database.
void skipDelimiters(); //Skips any spaces, commas, and closing parantheses before the next characters in the instruction. -- ' ' or ',' or ')'.
void outOfRange(long long int x); //Informs user that the number entered is out of range.


//Driver function.
int main()
{
	buildMaps();
	ifstream asmFile(INPUTFILE); //Opening input file.
	ofstream temp(OUTPUTFILE); temp.close(); //Clearing output file.
	ofstream mcFile(OUTPUTFILE, ios::app); //Opening output file.
	getline(asmFile,inst);
	skipDelimiters();
	while(!asmFile.eof())
	{
		if (inst[0] == '.') //If instruction might be a directive,
		{
			dirname = getDirectiveName();
			skipDelimiters();
			if (dataInc[dirname] != "") dataSegmentReader(asmFile);
			else invalidDirective(); //If directive is not valid.
			dirname = "";
		}
		else
		{
			opname = getName();
			skipDelimiters();
			if (formatType[opname] != "") formatMatcher();
			else invalidInstruction();			
			pc += 4;
			opname = "";
		}
		inst = "";
		getline(asmFile,inst); //Get next line from assembly code file.
	}
	mcFile << counterToHex(pc) << " " << "0x00000000" << endl; //Marking end of code segment. 
	mcFile << dataSegment;
	mcFile.close();
	asmFile.close();

	return 0;
}



/*-------------------------------------------------------------- PREPROCESSING FUNCTIONS --------------------------------------------------------------*/
void buildDirectiveMaps()
{
	ifstream iFile("InstructionSet/directives.txt");
	string increment, name;
	while(1)
	{
		iFile >> name; //Reading operation name.
		if (name == "end" || iFile.eof()) break;
		iFile >> increment; //Reading increment.
		dataInc[name] = increment;
	}
	iFile.close();
}

void buildMaps()
{
	buildDirectiveMaps();
	buildRMaps();
	buildIMaps();
	buildSMaps();
	buildSBMaps();
	buildUMaps();
	buildUJMaps();
	prescan();
}

//Function that reads RtypeInstructions.txt and stores opcode, func3, func7 values into maps.
void buildRMaps()
{
	ifstream iFile("InstructionSet/RtypeInstructions.txt");
	string temp, name;
	while(1)
	{
		iFile >> name; //Reading operation name.
		if (name == "end" || iFile.eof()) break;
		iFile >> temp; //Reading opcode.
		opcode[name] = temp;
		
		iFile >> temp; //Reading func3.
		f3[name] = temp;
		
		iFile >> temp; //Reading func7.
		f7[name] = temp;
		
		iFile >> temp; //Reading formatType.
		formatType[name] = temp;
	}
	iFile.close();
}

//Function that reads ItypeInstructions.txt and stores opcode, func3 values into maps.
void buildIMaps()
{
	ifstream iFile("InstructionSet/ItypeInstructions.txt");
	string temp, name;
	while(1)
	{
		iFile >> name; //Reading operation name.
		if (name == "end" || iFile.eof()) break;
		iFile >> temp; //Reading opcode.
		opcode[name] = temp;

		iFile >> temp; //Reading func3.
		f3[name] = temp;

		iFile >> temp; //Reading formatType.
		formatType[name] = temp;
	}
	iFile.close();
}

//Function that reads StypeInstructions.txt and stores opcode, func3 values into maps.
void buildSMaps()
{
	ifstream iFile("InstructionSet/StypeInstructions.txt");
	string temp,name;
	while(1)
	{
		iFile >> name; //Reading operation name.
		if (name == "end" || iFile.eof()) break;
		iFile >> temp; //Reading opcode.
		opcode[name] = temp;

		iFile >> temp; //Reading func3.
		f3[name] = temp;

		iFile >> temp; //Reading formatType.
		formatType[name] = temp;
	}
	iFile.close();
}

//Function that reads SBtypeInstructions.txt and stores opcode, func3 values into maps.
void buildSBMaps()
{
    ifstream iFile("InstructionSet/SBtypeInstructions.txt");
    string temp,name;
    while(1)
    {
        iFile >> name;//reading operation name
        if (name == "end" || iFile.eof()) break;
        iFile >> temp;//reading opcode
        opcode[name]=temp;

        iFile >> temp;//reading funct3
        f3[name]=temp;

        iFile >> temp;//reading format type
        formatType[name]=temp;
    }
    iFile.close();
}

//Function that reads UtypeInstructions.txt and stores opcode values into maps.
void buildUMaps()
{
    ifstream iFile("InstructionSet/UtypeInstructions.txt");
    string temp,name;
    while(1)
    {
        iFile >> name;//reading operation name
        if (name == "end" || iFile.eof()) break;
        
        iFile >> temp;//reading opcode
        opcode[name]=temp;

        iFile >> temp;//reading format type
        formatType[name]=temp;
    }
    iFile.close();
}

//Function that reads UJtypeInstructions.txt and stores opcode values into maps.
void buildUJMaps()
{
    ifstream iFile("InstructionSet/UJtypeInstructions.txt");
    string temp,name;
    while(1)
    {
        iFile >> name;//reading operation name
        if (name == "end" || iFile.eof()) break;
        
        iFile >> temp;//reading opcode
        opcode[name]=temp;

        iFile >> temp;//reading format type
        formatType[name]=temp;
    }
    iFile.close();
}

void prescan()
{
	ifstream asmFile(INPUTFILE); //Opening input file.
	getline(asmFile,inst);
	skipDelimiters();
	while(!asmFile.eof())
	{
		if (inst[0] == '.') //If instruction might be a directive,
		{
			dirname = getDirectiveName();
			skipDelimiters();
			int increment = stoi(dataInc[dirname]);
			if (increment > 0) dc += increment;
			dirname = "";
		}
		else
		{
			opname = getName();
			skipDelimiters();
			pc += 4;
			opname = "";
		}
		inst = "";
		getline(asmFile,inst); //Get next line from assembly code file.
	}
	asmFile.close();
	pc = 0;
	dc = 0x10000000;
}


/*============================================================ PARSING FUNCTIONS ============================================================*/
void dataSegmentReader(istream& asmFile)
{
	int increment = stoi(dataInc[dirname]); //Getting increment correspponding to directive.
	string num = "";
	if (increment < 0) //Increment is set as -1 for .text, .data, .string, and .asciiz
	{
		if (dirname == "text") return; //If .text, return back and continue with next instruction.
		else if (dirname == "data") //If .data, get the next directive.
		{
			getline(asmFile,inst);
			dirname = getDirectiveName();
			increment = stoi(dataInc[dirname]);
		}
		else if (dirname == "string" || dirname == "asciiz" || dirname == "asciz")
		{
			inst.erase(0,1); //Getting rid of the opening double quote ".
			num += getString();
			
			if (num != "") dataSegment += counterToHex(dc) + " " + num + '\n';
			dc += num.length();
			return;
		}
		else return;
	}
	if (increment == 0) invalidDirective(); //If .data is followed by an invalid directive.
	while (inst.length() && increment > 0)
	{
		skipDelimiters();
		//Read number to be stored.
		while(inst.length() && inst[0] != ' ')
		{
			num += inst[0];
			inst.erase(0,1);
		}
		
		//Convert to integer
		long long int x = stoi(num);
		num = "";
		
		//Confirm number is in range.
		if (bitCount(abs(x)) > 8*increment)
		{
			outOfRange(x);
			continue;
		}
		
		//Convert number to hex string.
		for (int i = 0; i < 2*increment; i++)
		{
			num += conv[((x%16)+16)%16];
			x = x >> 4;
		}
		reverse(num.begin(), num.end());

		//Append to data segment.
		dataSegment += counterToHex(dc) + " 0x" + num + "\n";
		dc += increment;
		num = "";
	}
}


//Getting the name of the instruction.
string getName()
{
	string name;
	while(inst.length() && inst[0] != ' ' && inst[0] != ',' && inst[0] != ':')
	{
		name += inst[0];
		inst.erase(0,1); //erasing the character read.
	}
	skipDelimiters();
	if (inst[0] == ':') //name was actually a label!
	{
		labelAddress[name] = pc;
		inst.erase(0,1);
		skipDelimiters();
		return getName();
	}
	return name;
}


//Getting the register name.
string getReg()
{
	string reg;
	if (inst[0] == '(') inst.erase(0,1); //getting rid of the potential '(' for load-store type of instructions.
	if (inst[0] == 'x') inst.erase(0,1); //getting rid of the 'x'.
	else return "";	
	
	//obtaining the register number as a string.
	while (inst.length() && inst[0] != ' ' && inst[0] != ',' && inst[0] != 'x' && inst[0] != '\n')
	{
		reg += inst[0];
		inst.erase(0,1);
	}
	skipDelimiters();

	//converting register number to int.
	int x = stoi(reg);
	reg = "";
	
	//converting the int to a 5 bit binary string.
	for (int i = 0; i < 5; i++)
	{
		if (x&1) reg += '1';
		else reg += '0';
		x /= 2;
	}
	reverse(reg.begin(),reg.end()); 

	return reg;
}

//Getting the immediate value.(to be debugged) for sb we need 13 bit so new func for that
string getImm()
{	
	string imm="";//initialising the immediate string.	
	while(inst.length() && inst[0] !=' ' && inst[0] !=',' && inst[0] != '\n' && inst[0] !='(' ){ //extracting the entire immediate string from the instruction
	
		imm=imm+inst[0];
		inst.erase(0,1);
		
	}
	
	int base=10;//as a default taking base 10
	
	if (imm[0]!='-'){ //if the given input is not a negative value
		if ( imm[0] == '0' && imm[1] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(0,2);
		}
		
		else if( imm[0] == '0' && imm[1] == 'x' ){ //checking if the immediate if hexadecimal.
			base=16;
		}
	}
	
	else if (imm[0] == '-'){ //if the given input is a negative number.
		if ( imm[1] == '0' && imm[2] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(1,2);
		}
		
		else if( imm[1] == '0' && imm[2] =='x'){ //checking if the immediate is hexadecimal.
			base=16;
		}
	}	
	long long int immediate;
//	if(all_of(imm.begin(),imm.end(),::isdigit)==false)
//	{
//		immediate=labelAddress[imm]-pc;
//	cout << imm << " " << immediate << endl;
//	}
//	else
	immediate = stoll( imm , 0 , base );
	if (immediate > 2047 ){
		outOfRange(immediate);
	}
	else if (immediate< -2047){
		outOfRange(immediate);
	}
	else{
		string binary_representation="";
		
		if (immediate>=0){
		
		//converting the int to a 12 bit binary string.
		for (int i = 0; i < 12; i++)
		{
			if (immediate&1) binary_representation += '1';
			else binary_representation += '0';
			immediate /= 2;
		}
		reverse(binary_representation.begin(),binary_representation.end()); 
		skipDelimiters();
		return binary_representation;
		}
		
		else if(immediate<0){
			immediate=-(immediate);
			
			//converting the absolute value of the int to 12 bit binary string.
			for ( int i=0; i<12; i++){
				if(immediate&1) binary_representation +='1';
				else binary_representation +='0';
				immediate /=2;
			}
			reverse(binary_representation.begin(), binary_representation.end());
			for ( int i=0; i<12 ; i++){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
				}
				else if ( binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
			}
			int i=11;
			while(1){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
					break;
				}
				else if (binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
				i=i-1;
				
			}
			skipDelimiters();
			return binary_representation;
		}
	}
	
        skipDelimiters();
	return 0;	
}

//for sb type instruction

string getImm1()
{	
	string imm="";//initialising the immediate string.	
	while(inst.length() && inst[0] !=' ' && inst[0] !=',' && inst[0] != '\n' && inst[0] !='(' ){ //extracting the entire immediate string from the instruction
	
		imm=imm+inst[0];
		inst.erase(0,1);
		
	}
	
	int base=10;//as a default taking base 10
	
	if (imm[0]!='-'){ //if the given input is not a negative value
		if ( imm[0] == '0' && imm[1] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(0,2);
		}
		
		else if( imm[0] == '0' && imm[1] == 'x' ){ //checking if the immediate if hexadecimal.
			base=16;
		}
	}
	
	else if (imm[0] == '-'){ //if the given input is a negative number.
		if ( imm[1] == '0' && imm[2] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(1,2);
		}
		
		else if( imm[1] == '0' && imm[2] =='x'){ //checking if the immediate is hexadecimal.
			base=16;
		}
	}	
	long long int immediate;
	if(all_of(imm.begin(),imm.end(),::isdigit)==false)
	{
		immediate=labelAddress[imm]-pc;
	}
	else
	immediate = stoll( imm , 0 , base );
	if (immediate > 4095 ){
		outOfRange(immediate);
	}
	else if (immediate< -4095){
		outOfRange(immediate);
	}
	else{
		string binary_representation="";
		
		if (immediate>=0){
		
		//converting the int to a 12 bit binary string.
		for (int i = 0; i < 13; i++)
		{
			if (immediate&1) binary_representation += '1';
			else binary_representation += '0';
			immediate /= 2;
		}
		reverse(binary_representation.begin(),binary_representation.end()); 
		skipDelimiters();
		return binary_representation;
		}
		
		else if(immediate<0){
			immediate=-(immediate);
			
			//converting the absolute value of the int to 12 bit binary string.
			for ( int i=0; i<13; i++){
				if(immediate&1) binary_representation +='1';
				else binary_representation +='0';
				immediate /=2;
			}
			reverse(binary_representation.begin(), binary_representation.end());
			for ( int i=0; i<13 ; i++){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
				}
				else if ( binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
			}
			int i=12;
			while(1){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
					break;
				}
				else if (binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
				i=i-1;
				
			}
			skipDelimiters();
			return binary_representation;
		}
	}
	
        skipDelimiters();
	return 0;	
}

// imm for u type instructions.
string getImm2()
{	
	string imm="";//initialising the immediate string.	
	while(inst.length() && inst[0] !=' ' && inst[0] !=',' && inst[0] != '\n' && inst[0] !='(' ){ //extracting the entire immediate string from the instruction
	
		imm=imm+inst[0];
		inst.erase(0,1);
		
	}
	
	long long int immediate;
	if(labelAddress[imm]==0)
	{
		int base=10;//as a default taking base 10
	
	if (imm[0]!='-'){ //if the given input is not a negative value
		if ( imm[0] == '0' && imm[1] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(0,2);
		}
		
		else if( imm[0] == '0' && imm[1] == 'x' ){ //checking if the immediate if hexadecimal.
			base=16;
		}
	}
	
	else if (imm[0] == '-'){ //if the given input is a negative number.
		if ( imm[1] == '0' && imm[2] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(1,2);
		}
		
		else if( imm[1] == '0' && imm[2] =='x'){ //checking if the immediate is hexadecimal.
			base=16;
		}
	}	
		immediate = stoll( imm , 0 , base );
	}
	else
	{
		immediate=labelAddress[imm]-pc;
	}
	//cout << immediate << endl;
	if (immediate > 1048575 ){
		outOfRange(immediate);
	}
	else if (immediate< 0){
		outOfRange(immediate);
	}
	else{
		string binary_representation="";
		
		if (immediate>=0){
		
		//converting the int to a 12 bit binary string.
		for (int i = 0; i < 20; i++)
		{
			if (immediate&1) binary_representation += '1';
			else binary_representation += '0';
			immediate /= 2;
		}
		reverse(binary_representation.begin(),binary_representation.end()); 
		skipDelimiters();
		return binary_representation;
		}
		
		else if(immediate<0){
			immediate=-(immediate);
			
			//converting the absolute value of the int to 12 bit binary string.
			for ( int i=0; i<20; i++){
				if(immediate&1) binary_representation +='1';
				else binary_representation +='0';
				immediate /=2;
			}
			reverse(binary_representation.begin(), binary_representation.end());
			for ( int i=0; i<20 ; i++){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
				}
				else if ( binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
			}
			int i=19;
			while(1){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
					break;
				}
				else if (binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
				i=i-1;
				
			}
			skipDelimiters();
			return binary_representation;
		}
	}
	
        skipDelimiters();
	return 0;	
}
// get imm for uj inst as it needs 21 bits
string getImm3()
{	
	string imm="";//initialising the immediate string.	
	while(inst.length() && inst[0] !=' ' && inst[0] !=',' && inst[0] != '\n' && inst[0] !='(' ){ //extracting the entire immediate string from the instruction
	
		imm=imm+inst[0];
		inst.erase(0,1);
		
	}
	
	long long int immediate;
	if(labelAddress[imm]==0)
	{
		int base=10;//as a default taking base 10
	
	if (imm[0]!='-'){ //if the given input is not a negative value
		if ( imm[0] == '0' && imm[1] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(0,2);
		}
		
		else if( imm[0] == '0' && imm[1] == 'x' ){ //checking if the immediate if hexadecimal.
			base=16;
		}
	}
	
	else if (imm[0] == '-'){ //if the given input is a negative number.
		if ( imm[1] == '0' && imm[2] == 'b'){ //checking if the immediate is binary.
			base=2;
			imm.erase(1,2);
		}
		
		else if( imm[1] == '0' && imm[2] =='x'){ //checking if the immediate is hexadecimal.
			base=16;
		}
	}	
		immediate = stoll( imm , 0 , base );
	}
	else
	{
		immediate=labelAddress[imm]-pc;
	}
	//cout << immediate << endl;
	if (immediate > 1048575 ){
		outOfRange(immediate);
	}
	else if (immediate< -1048575){
		outOfRange(immediate);
	}
	else{
		string binary_representation="";
		
		if (immediate>=0){
		
		//converting the int to a 12 bit binary string.
		for (int i = 0; i < 21; i++)
		{
			if (immediate&1) binary_representation += '1';
			else binary_representation += '0';
			immediate /= 2;
		}
		reverse(binary_representation.begin(),binary_representation.end()); 
		skipDelimiters();
		return binary_representation;
		}
		
		else if(immediate<0){
			immediate=-(immediate);
			
			//converting the absolute value of the int to 12 bit binary string.
			for ( int i=0; i<21; i++){
				if(immediate&1) binary_representation +='1';
				else binary_representation +='0';
				immediate /=2;
			}
			reverse(binary_representation.begin(), binary_representation.end());
			for ( int i=0; i<21 ; i++){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
				}
				else if ( binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
			}
			int i=20;
			while(1){
				if(binary_representation[i] == '0'){
					binary_representation[i] = '1';
					break;
				}
				else if (binary_representation[i] == '1'){
					binary_representation[i] = '0';
				}
				i=i-1;
				
			}
			skipDelimiters();
			return binary_representation;
		}
	}
	
        skipDelimiters();
	return 0;	
}


//Extract name of directive from instruction
string getDirectiveName()
{
	if(inst[0] == '.') inst.erase(0,1); //Remove the '.'
	else
	{
		string label = "";
		while(inst.length() && inst[0] != ':' && inst[0] != ' ') //while you don't hit a delimiter,
		{
			label += inst[0]; //append to directive
			inst.erase(0,1);
		}
		if (inst.length() && inst[0] == ':')
		{
			labelAddress[label] = dc; //adding the label to the map.
			inst.erase(0,1);
		}
		skipDelimiters();
		return getDirectiveName();
	}
	string directive = "";
	while(inst.length() && inst[0] != ' ') //while you don't hit a delimiter,
	{
		directive += inst[0]; //append to directive
		inst.erase(0,1);
	}
	return directive; //return the name of the directive.
}


//Reading next string from instruction.
string getString()
{
	string temp = "";
	while (inst.length() && inst[0] != '"')
	{
		temp += inst[0];
		inst.erase(0,1);
	}
	if (inst.length()) inst.erase(0,1); //Checking if the string ended with a double quote.
	else return invalidString(temp);
	return temp;
}


/*============================================================ MACHINE CODE CREATING FUNCTIONS ============================================================*/
//Getting all the fields and creating the machine code from them, for R type instruction.
void typeRmc()
{
	//Extracting the registers as binary strings.
	string rd,r1,r2;
	rd = getReg();
	r1 = getReg();
	r2 = getReg();
	ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file

	mc = f7[opname] + r2 + r1 + f3[opname] + rd + opcode[opname]; //getting the binary machine code.
	mc = bin2hex(mc); //converting the machine code to hex.

	mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
	mcFile.close();
}

//Getting all the fields and creating the machine code from them, for I type instruction.
void typeImc() 
{	
	if (opname=="addi" || opname=="andi" || opname=="ori")
	{
		//Extracting the registers and the immediate as binary strings.
		string rd,r1,imm;
		rd = getReg();
		r1 = getReg(); //after the execution of this I should  be at the start of the immediate value.
		imm = getImm(); //to be created
		ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file
		
		mc = imm + r1 + f3[opname] + rd + opcode[opname]; //getting the binary machine code.
		mc = bin2hex(mc); //converting the machine code to hex.
		
		mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
		mcFile.close();
	}
	
	else if (opname == "lb" || opname == "lw" || opname == "lh" || opname == "ld"){
		// these instructions follow the format eg: lw x5,100(x4)
		
		//Extracting the registers and the immediate as binary strings.
		string rd,r1,imm;
		rd = getReg();  
		imm = getImm(); 
		r1 = getReg(); 
		ofstream mcFile(OUTPUTFILE, ios::app); // appending to the output file.
	
		mc = imm + r1 + f3[opname] + rd + opcode[opname]; //getting the binary machine code.
		mc = bin2hex(mc); //converting the machine code to hex.
		
		mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
		mcFile.close();
	}
	
	else if (opname == "jalr"){
		//this instruction follows the format eg: sw x0,x1,0
		
		//Extracting the registers and the immediate as binary strings.
		string rd,r1,imm;
		rd = getReg();
		r1 = getReg();
		imm = getImm();
		ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file.
		
		mc = imm + r1 + f3[opname] + rd + opcode[opname]; //getting the binary machine code.
		mc = bin2hex(mc); //converting the machine code to hex.
		
		mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
		mcFile.close();
	}
}


//Getting all the fields and creating the machine code from them, for S type instruction.
void typeSmc() //complete this.
{
	//Extracting the registers and the immediate as binary strings.
	string r2,r1,imm;
	r2 = getReg();
	imm = getImm();
	r1 = getReg();
	ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file.
	
	mc = imm.substr(0,7) + r2 + r1 + f3[opname] + imm.substr(7,5) + opcode[opname]; //getting the binary machine code.
	mc = bin2hex(mc); //converting the machine code to hex.

	mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
	mcFile.close();
}
//Getting all the fields and creating the machine code from them, for U type instruction.
void typeUmc()
{
	string rd,imm;
	rd = getReg();
	imm = getImm2();
	ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file.
	
	mc = imm + rd + opcode[opname];//getting bin mc
	mc= bin2hex(mc);//converting mc to hex

	mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
	mcFile.close();
}
//Getting all the fields and creating the machine code from them, for UJ type instruction.
void typeUJmc()
{
	string rd,imm;
	rd = getReg();
	imm = getImm3();
	ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file.
	//cout << imm << endl;
	string imm110=imm.substr(10,10);
	string imm11=imm.substr(9,1);
	string imm1219=imm.substr(1,8);
	string imm20=imm.substr(0,1);
	
	mc = imm20 + imm110 + imm11 + imm1219 + rd + opcode[opname];//getting bin mc
	mc= bin2hex(mc);//converting mc to hex

	mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
	mcFile.close();
}
//Getting all the fields and creating the machine code from them, for SB type instruction.
void typeSBmc()
{
	string r2,r1,imm;
	r2=getReg();
	r1=getReg();
	imm=getImm1();
	ofstream mcFile(OUTPUTFILE, ios::app); //appending to the output file.

	string imm12=imm.substr(0,1);
	string imm11=imm.substr(1,1);
	string imm105=imm.substr(2,6);
	string imm41=imm.substr(8,4);

	mc= imm12 + imm105 + r2 + r1 + f3[opname] + imm41 + imm11 + opcode[opname];//getting bin mc
	mc= bin2hex(mc);//converting mc to hex

	mcFile << counterToHex(pc) << " " << "0x" << mc << endl;
	mcFile.close();

}


void formatMatcher()
{
	if (formatType[opname] == "R") typeRmc(); //Creating machine code for type R.
	else if (formatType[opname]=="I") typeImc(); //Creating machine code for type I
	else if (formatType[opname]=="S") typeSmc(); //Creating machine code for type S
	else if (formatType[opname]=="SB") typeSBmc();// Creating mc for type SB -vtgg :)
	else if (formatType[opname]=="U")typeUmc();// Creating mc for type U
	else if (formatType[opname]=="UJ") typeUJmc();// Creating mc for type UJ
}

/*============================================================ UTILITY FUNCTIONS ============================================================*/

//Function to convert integer pc to hexadecimal.
string counterToHex(int counter)
{
	string counterInHex;	
	int temp = counter;
	if (temp == 0) counterInHex = '0';
	
	//Converting to hex
	while (temp)
	{
		counterInHex += conv[temp%16];
		temp /= 16;
	}
	reverse(counterInHex.begin(), counterInHex.end());
	
	return "0x" + counterInHex;
}


//Function to convert binary string of machine code into hexadecimal.
string bin2hex(string s)
{
	string hex;
	for (int i = 0; i < s.size();)
	{
		int num = 0;
		if (s[i++] == '1') num += 8;
		if (s[i++] == '1') num += 4;
		if (s[i++] == '1') num += 2;
		if (s[i++] == '1') num += 1;
		hex += conv[num];
	}
	return hex;
}


int bitCount(unsigned long long int x)
{
	int count = 0;
	while(x /= 2) count++;
	return count+1;
}


void skipDelimiters()
{
	while (inst.length() && (inst[0] == ' ' || inst[0] == ',' || inst[0] == ')' || inst[0] == '(')) inst.erase(0,1);
}


void invalidInstruction()
{
	ofstream mcFile(OUTPUTFILE, ios::app);
	mcFile << "Invalid instruction: " << opname << endl;
	mcFile.close();
	pc -= 4; //decrementing pc to account for the automatic increment.
}



void invalidDirective()
{
	ofstream mcFile(OUTPUTFILE, ios::app);
	mcFile << "Invalid directive: ." << dirname << endl;
	mcFile.close();
}


void outOfRange(long long int x)
{
	ofstream mcFile(OUTPUTFILE, ios::app);
	mcFile << "Number " << x << " is out of range for " << dirname << endl;
	mcFile.close();
}


string invalidString(string temp)
{
	ofstream mcFile(OUTPUTFILE, ios::app);
	mcFile << "Invalid string " << '"' << temp << endl;
	mcFile.close();
	return "";
}


