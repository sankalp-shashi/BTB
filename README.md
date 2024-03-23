# BTB
Branch Target Buffer and Branch History Table of a RISC-V assembler

1) Read the file to get pc address, machine code and instruction name. Check opcode to see if instruction is branch.
2) Have a map that acts as the branch target buffer. (pc to pc)
3) Have a map that acts as the branch history table. (pc to string of 'N' and 'T')
4) Get the offset value from machine code, and make prediction for each type of predictor.
5) For each predictor, keep a count of how many predictions were made, and how many of them were correct.
6) Finally print the accuracy of each kind of predictor, along with the BTB and branch history table.
