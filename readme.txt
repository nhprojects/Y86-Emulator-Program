Nile Henry Readme PA3

The goal of this assignment was to understand how the fetch, decode, execute process works.

It was a really difficult assignment but also very rewarding once all the 
separate pieces of the code acted together.


1. I implemented registers as an int[8] with each index of the array representing one of the 8 registers
2. I had a variable called memorysize which stored the memorysize
3. I stored the memory in a char pointer which I malloc enough size for in my decode step
4. I had two boolean variables, hasSizeDir and hasTextDir which would be checked before executing 
the program. If these two directives: .size and .textare not in the y86 program, we can't run the program.
5. I have a program counter which is an int. Which is the instruction pointer

Fetch: The file that the user indicates in the cmd line arguments is read in and put into a string. It	
	    is split by line into a char * lineArray[] which holds each line of the y86 program in a separate
		index
		
Decode: The program goes through each line and using the interpretline function I made with helper 
	    functions (interpretSize(),interpretString(),interpretLong(),interpretByte(), and interpretText(),
		it places each of those data types into memory at the specified address. It also takes note of		
		where the instructions start.
		
Execute: executeProgram() holds a state variable (state is an enum I created), ~status~ and a while loop 
		and while that status = AOK (good to continue). The program will get the instruction at the current
		program counter (using hex value equality if statements) and execute it. I have methods 
		implemented for each instruction. These methods return each return a state. Through each call 
		of the while, we change the state to be the return value of each of these instruction's functions.
		
		The instruction functions I implemented are:
			. rrmovl()
			. irmovl();
			. rmmovl();
			. mrmovl();
			. addl();
			. subl();
			. andl();
			. xorl();
			. mull();
			. cmpl();
			. jmp();
			. jle();
			. jl();
			. je();
			. jne();
			. jge();
			. jg();
			. call();
			. ret();
			. pushl();
			. popl();
			. readb();
			. readl();
			. writeb();
			. writel();
			. movsbl();
			
		At the end of the program, the program says how it ended. 

Some helper functions I implemented for the program were 
	. bool equals(char * str1, char * str2) - which indicates if two strings are equal
	. int * getIntAt(int loc) - which returns an int pointer to memory+loc	
	. getCharBits(int nums[2],char c) - which places the left 4 bits of c into nums[0]
	  and right 4 bits of c into nums[1]
	.linesplitwhitespace(char line[],char * lineArray[3])- which takes a line in original file and 
	 places each of the three parts: directive, address, value into the corresponding index of lineArray
	 
Challenges: My main challenge was that it was difficult to check code since all the parts were dependent
	       on each other. I just had to hope the code would work and reread my functions over and over to
		   spot errors. 
	 

		