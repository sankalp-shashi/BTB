# CS204-Project

To give assembly code as input either:
	1. Replace the code in demoInput.asm with your assembly code.
	OR
	2. In line 6 of ASMtoMC.cpp replace demoInput.asm with the name of (or path to) your assembly file.

To compile and run:
	g++ ASMtoMC.cpp
	./a.out

By default the output machine code will appear in demoOutput.mc .
To get the output into another file, in line 7 of ASMtoMC.cpp replace demoOutput.mc with the name of (or path to) your target file.


The InstructionSet folder contains all the information regarding the instructions that have to be supported, like:
name, opcode, funct3, format type, etc. ASMtoMC.cpp reads this information in the preprocessing stage and stores it into maps for use while parsing.

Whenever an unknown instruction occurs, or if an instruction occurs in an unexpected format, the error message will be displayed in the corresponding location
inside the machine code output file. The conversion doesn't halt when an error is encountered (though this behaviour can be changed easily).
PC will also not be incremented in case of an error.
This means, the incorrect instruction will simply be skipped.
