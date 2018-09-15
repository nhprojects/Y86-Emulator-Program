#ifndef Y86EMUL_Y86EMUL_H
#define Y86EMUL_Y86EMUL_H

enum state{
    //AOK means everything is fine
    //HLT means a halt instruction has occured, which is how y86 program normally ends
    //ADR means some sort of invalid address has been encountered which also stops y86 instruction
    //INS is set for invalid instruction which also stops y86 instruction
    //program should print how program executione ends
    AOK,HLT,ADR,INS
};
void interpretLine(char line[]);
void interpretSize(char * lineArray[3]);
void interpretString(char * lineArray[3]);
void interpretLong(char * lineArray[3]);
void interpretByte(char * lineArray[3]);
void interpretText(char * lineArray[3]);
void executeProgram();

void getCharBits(int nums[2], char c);
int* getIntAt(int loc);

enum state rrmovl();
enum state irmovl();
enum state rmmovl();
enum state mrmovl();
enum state addl();
enum state subl();
enum state andl();
enum state xorl();
enum state mull();
enum state cmpl();
enum state jmp();
enum state jle();
enum state jl();
enum state je();
enum state jne();
enum state jge();
enum state jg();
enum state call();
enum state ret();
enum state pushl();
enum state popl();
enum state readb();
enum state readl();
enum state writeb();
enum state writel();
enum state movsbl();

void lineSplitWhiteSpace(char line[],char * lineArray[3]);
bool equals(char * str1, char * str2);

#endif //Y86EMUL_Y86EMUL_H