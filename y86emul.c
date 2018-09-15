#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "y86emul.h"

char * memory; 

int memorySize;
int esp;
bool hasSizeDir;
bool hasTextDir;
int registers[8];  //holds the 8 registers
bool overflowflag=0; 
bool negativeflag=0;
bool zeroflag=1;
int programcounter=0; //instruction pointer. holds next machine instruction to execute

int main(int argc, char * argv[])
{	
	char * filearg;
	if (argc!=2 && argc!=3){
		printf("ERROR: Invalid amount of arguments, must have y86emul [-h] <y86 inputfile>\n");
		return 0;
	}
	
	if (argc==3){
		if (equals(argv[1],"-h")){
			printf("Usage: y86emul [-h] <y86 inputfile>\n");
		}
		else{
			printf("ERROR: invalid syntax, must be: y86emul [-h] <y86 inputfile>\n");
		}
		return 0;
	}
	else{ //argc=2;
		if (equals(argv[1],"-h")){
			printf("Usage: y86emul [-h] <y86 inputfile>\n");
			return 0;
		}
		else{
			filearg= argv[1];
		}
	}
    
	FILE * fp;
	fp=fopen(filearg,"r");
	int n=0;
	int c;
	if (fp==NULL){
		printf("File is null\n");
		return 0;
	}
	fseek(fp,0,SEEK_END);
	long fsize=ftell(fp);
	fseek(fp,0,SEEK_SET);
	char code[fsize];
	
	while ((c=fgetc(fp))!=EOF){
		code[n]=(char) c;
		n=n+1;
	}
	code[n]='\0';
	

	//read file to a new array with each index of array as a line of the file
	char * splitter= "\n";

    char * lineArray[500];
    int index=0;
    char * line= strtok(code,splitter);
    while (line!=0){
        lineArray[index]=line;
        line= strtok(NULL,splitter); 
        index=index+1;
    }
    
	//go through each line and interpret its meaning
    int sizeLineArray=index;
    index=0;
    for (index=0; index<sizeLineArray;index=index+1){
        interpretLine(lineArray[index]);
    };
    
	if (!hasSizeDir || !hasTextDir){
		printf("ERROR: Program must have a size directive and text directive\n");
	}
    executeProgram();
  
    return 0;
}


void interpretLine(char line[]){
    char * lineArray[3];  //split into the lines three parts 1. directive  2. address 3. value
    lineSplitWhiteSpace(line,lineArray);;
    
    if (equals(lineArray[0],".size")){
		hasSizeDir=1;
        interpretSize(lineArray);
    }
    else if (equals(lineArray[0],".string")){
        interpretString(lineArray);
    }
   else if (equals(lineArray[0],".long")){
        interpretLong(lineArray);
    }
    else if (equals(lineArray[0],".byte")){
        interpretByte(lineArray);
    }
    else if (equals(lineArray[0],".text")){
		hasTextDir=1;
        interpretText(lineArray);
    }
    else{
        printf("ERROR: Invalid y86 directive. Program will now exit");
    }
}
void interpretSize(char * lineArray[3]){  //sets memory size of program
    long size= strtol(lineArray[1],NULL,16);
    memorySize=size;
    memory= malloc(size);
}
void interpretString(char * lineArray[3]){  //puts a string into memory
    int address= strtol(lineArray[1],NULL,16);
    
    char * string= lineArray[2];
    int i=0;
    for (i=0; i<strlen(string);i=i+1){
        //printf("%c",string[i]);
        memory[address+i]= string[i];
    }
}
void interpretLong(char * lineArray[3]){  //puts a long into memory
    int address= strtol(lineArray[1],NULL,16);  
    int value= strtol(lineArray[2],NULL,10);  
	*getIntAt(address)=value;
}
void interpretByte(char * lineArray[3]){
    int address= strtol(lineArray[1],NULL,16);
    char byte=strtol(lineArray[2],NULL,16);
    memory[address]=byte;
}
void interpretText(char * lineArray[3]){
    int address= strtol(lineArray[1],NULL,16);
    programcounter=address;
    char * instructions= lineArray[2];
    
    int i=0;  //index tracker
    int j=0; 
    for (j=0;j<strlen(instructions);j=j+2){ //gets each byte split into high 4 bits, low 4 bits
        char * currentByte=malloc(2);
        strncpy(currentByte,instructions+j,2);
        memory[address+i]=(unsigned char) strtol(currentByte,NULL,16);//&0xff;
        free(currentByte);
        i=i+1;
    }
    
}

void executeProgram(){
   enum state status=AOK;
    while (status==AOK){
       unsigned char c=  memory[programcounter];
 
        //printf("program counter: %i, value:  %x\n",programcounter,c);
        if (c==0x00){
            programcounter=programcounter+1;
        }
        else if (c==0x10){
            printf("Program halted\n");
            status=HLT;
        }
        else if (c==0x20){status=rrmovl();}
        else if (c==0x30){status=irmovl();} 
        else if (c==0x40){status=rmmovl();} 
        else if (c==0x50){status=mrmovl();}  
        else if (c==0x60){status=addl();}
        else if (c==0x61){status=subl();}
        else if (c==0x62){status=andl();}
        else if (c==0x63){status=xorl();}
        else if (c==0x64){status=mull();}
        else if (c==0x65){status=cmpl();}
        else if (c==0x70){status=jmp();}
        else if (c==0x71){status=jle();}
        else if (c==0x72){status=jl();}
        else if (c==0x73){status=je();}
        else if (c==0x74){status=jne();}
        else if (c==0x75){status=jge();}
        else if (c==0x76){status=jg();}
        else if (c==0x80){status=call();}
        else if (c==0x90){status=ret();}
        else if (c==0xa0){status=pushl();}
        else if (c==0xb0){status=popl();}
        else if (c==0xc0){status=readb();}
        else if (c==0xc1){status=readl();}
        else if (c==0xd0){status=writeb();}
        else if (c==0xd1){status=writel();}
        else if (c==0xe0){status=movsbl();}
        else{ //not a valid opcode
            printf("ERROR: Invalid instruction, program will now exit\n");
            status=INS;
        }
 
    }
	if (status==INS){
		printf("ERROR: Invalid instruction, program will now exit\n");
	}
	else if (status==ADR){
		printf("ERROR: Invalid address, program will now exit\n");
	}
	else if (status==HLT){
		printf("Program Halted\n");
	}
	
    printf("Program done\n");
}

enum state rrmovl(){ //performs a register to register move
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    
    if (nums[0]<0 || nums[0]>7 || nums[0]<0 || nums[0]>7){
        return INS;
    }
    int ra= registers[nums[0]]; //register [left 4 bits]
    //int rb= registers[nums[1]]; //register [right 4 bits]
    registers[nums[1]]=ra; //move contents of ra into rb
    programcounter=programcounter+2;
    return AOK;
}
enum state irmovl(){ //working on not complete
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]!=15 || nums[1]<0 || nums[1]>7){
        return INS;
    }
    
	if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
    int immediate= *getIntAt(programcounter+2); 
  
    registers[nums[1]]= immediate; //place immediate in rb
    programcounter=programcounter+6;
    return AOK;
}
enum state rmmovl(){
    int nums[2];
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]<0 || nums[1]>7){
        return INS;
    }
	
	if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}

    int displacement=*getIntAt(programcounter+2); 
	
	
	if (((registers[nums[1]]+displacement)>memorySize) || ((registers[nums[1]]+displacement)<0)){
		return ADR;
	}
    *getIntAt(registers[nums[1]]+displacement)=registers[nums[0]]; //place value of register in memory
    
    programcounter=programcounter+6;
    return AOK;
}

enum state mrmovl(){
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    
    if (nums[0]<0 || nums[0]>7 || nums[1]<0 || nums[1]>7){
        return INS;
    }
    int rb= registers[nums[1]];
	
	if (((programcounter+2)>memorySize)|| ((programcounter+2)<0)){
		return ADR;
	}
    
    int displacement= *getIntAt(programcounter+2); 
	
	if (((rb+displacement)>memorySize)|| ((rb+displacement)<0)){
		return ADR;
	}
                
    int number= *getIntAt(rb+displacement);
    
    
    registers[nums[0]]=number; //place number in ra
    programcounter=programcounter+6;
    return AOK;
}
enum state addl(){
    overflowflag=0;
    zeroflag=0;
    negativeflag=0;
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
	
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0x0f;
    
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    int prera= registers[ra];
    int prerb= registers[rb];
    int sum= prerb+prera;
    registers[rb]=sum; //put result in rb
    if ((prera>0 && prerb>0 && sum<0)||(prera<0 && prerb<0 && sum>0)){ //overflow check
        overflowflag=1;
    }
    if (sum==0){
        zeroflag=1;
    }
    if (sum<0){
        negativeflag=1;
    }
    
    
    programcounter=programcounter+2;
    return AOK;
}
enum state subl(){
    overflowflag=0;
    negativeflag=0;
    zeroflag=0;
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0x0f;
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    int prera= registers[ra];
    int prerb= registers[rb];
    int result= prerb-prera; //put result in rb
    registers[rb]=result;
    if ((prera<0 && prerb>0 && result<0)||(prera>0 && prerb<0 && result>0)){ //overflow check
        overflowflag=1;
    }
    if (result==0){
        zeroflag=1;
    }
    if (result<0){
        negativeflag=1;
    }

    programcounter=programcounter+2;
    return AOK;
}
enum state andl(){
    negativeflag=0;
   // overflowflag=0;
    zeroflag=0;
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0xf;
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    int prera= registers[ra];
    int prerb= registers[rb];
    int result=prerb&prera;
    registers[rb]=result;
    if (result==0){
        zeroflag=1;
    }
    if (result<0){
        negativeflag=1;
    }
    
    programcounter=programcounter+2;
    return AOK;
}
enum state xorl(){
    zeroflag=0;
    negativeflag=0;
    //overflowflag=0;
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0xf;
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    int prera= registers[ra];
    int prerb=registers[rb];
    int result=prerb^prera;
    registers[rb]=result;
    
    if (result==0){
        zeroflag=1;
    }
    if (result<0){
        negativeflag=1;
    }
    programcounter=programcounter+2;
    return AOK;
}
enum state mull(){
    zeroflag=0;
    overflowflag=0;
    negativeflag=0;
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0xf;
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    int prera= registers[ra];
    int prerb= registers[rb];
    int result=prerb*prera;
    registers[rb]=result;
     if ((prera>0 && prerb>0 && result<0)||(prera<0 && prerb<0 && result<0)||(prera<0 && prerb>0 && result>0)||(prera>0 && prerb<0 && result>0)){
        overflowflag=1;
    }
    if (result==0){
        zeroflag=1;
    }
    if (result<0){
        negativeflag=1;
    }
    programcounter=programcounter+2;
    return AOK;
}
enum state cmpl(){
    zeroflag=0;
    /*overflowflag=0;
    negativeflag=0;*/
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int ra= memory[programcounter+1]>>4;
    int rb= memory[programcounter+1]&0xf;
    
    if (ra<0 || ra>7 || rb<0 || rb>7){
        return INS;
    }
    
    if (rb==ra){
        zeroflag=1;
    }
    programcounter=programcounter+2;
    return AOK;
}
enum state jmp(){
	
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    int displacement= *getIntAt(programcounter+1); 
    
    programcounter=displacement;
    return AOK;
}
enum state jle(){
    if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if ((negativeflag && overflowflag) || zeroflag ){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state jl(){
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if (negativeflag && overflowflag){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state je(){
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if (zeroflag){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state jne(){
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if (!zeroflag){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state jge(){
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if (!(negativeflag && zeroflag)){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state jg(){
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    if ((!(negativeflag && zeroflag)) & (!zeroflag)){
        programcounter=displacement;
        return AOK;
    }
    else{
        programcounter=programcounter+5;
    }
    return AOK;
}
enum state call(){
    if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     int displacement= *getIntAt(programcounter+1); 
    
    registers[4]=registers[4]-4;

	if (((registers[4])>memorySize) || ((registers[4])<0)){
		return ADR;
	}
    *getIntAt(registers[4])=programcounter+5;
	if (((displacement)>memorySize) || ((displacement)<0)){
		return ADR;
	}
     programcounter=displacement;
     return AOK;
}
enum state ret(){
	 if (((registers[4])>memorySize) || ((registers[4])<0)){
		return ADR;
	}
    int displacement= *getIntAt(registers[4]);
	 if (((displacement)>memorySize) || ((displacement)<0)){
		return ADR;
	}
    programcounter=displacement;
    registers[4]=registers[4]+4;
    return AOK;
}
enum state pushl(){
    int nums[2];
	 if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
        return INS;
    }

    registers[4]=registers[4]-4;
	 if (((registers[4])>memorySize) || ((registers[4])<0)){
		return ADR;
	}
    *getIntAt(registers[4])=registers[nums[0]];
    
    programcounter=programcounter+2;
    
    return AOK;
    
}
enum state popl(){
    int nums[2];
	 if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
        return INS;
    }
	if (((registers[4])>memorySize) || ((registers[4])<0)){
		return ADR;
	}
    registers[nums[0]]=*getIntAt(registers[4]);
    registers[4]=registers[4]+4;
    programcounter=programcounter+2;
    return AOK;
}
enum state readb(){
	zeroflag=0;
   negativeflag=0;
    overflowflag=0;
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c=memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
        return INS;
    }
    if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
	
    int displacement= *getIntAt(programcounter+2);
    //read character
    unsigned char readchar;
	scanf("%c",&readchar);
    if(readchar==EOF){
        zeroflag=1;
    }
	else if(readchar==27){
		return HLT;
	}
    else{
        zeroflag=0;
    }
	if (((registers[nums[0]]+displacement)>memorySize) || ((registers[nums[0]]+displacement)<0)){
		return ADR;
	}
    memory[registers[nums[0]]+displacement]=readchar;
    programcounter=programcounter+6;
    return AOK;
}
enum state readl(){
    zeroflag=0;
   negativeflag=0;
    overflowflag=0;
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c=memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
        return INS;
    }
	if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
    int displacement= *getIntAt(programcounter+2);
   
    //read int
    int readint;
	
	scanf("%i",&readint);
    if(readint==EOF){
	
        zeroflag=1;
    }
    else{	
		zeroflag=0;   
    }
	if (((registers[nums[0]]+displacement)>memorySize) || ((registers[nums[0]]+displacement)<0)){
		return ADR;
	}
	*getIntAt(registers[nums[0]]+displacement)=readint;
    programcounter=programcounter+6;
    return AOK;
}
enum state writeb(){
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     unsigned char c=memory[programcounter+1];
     getCharBits(nums,c);
     if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
         return INS;
     }
	 
	 if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
    
    int displacement= *getIntAt(programcounter+2);
	if (((registers[nums[0]]+displacement)>memorySize) || ((registers[nums[0]]+displacement)<0)){
		return ADR;
	}
    unsigned char writebyte=memory[registers[nums[0]]+displacement];
    printf("%c",writebyte);
    programcounter=programcounter+6;
    return AOK;
}
enum state writel(){
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
     unsigned char c=memory[programcounter+1];
     getCharBits(nums,c);
     if (nums[0]<0 || nums[0]>7 || nums[1]!=15){
         return INS;
     }
	 
	 if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
   
    int displacement= *getIntAt(programcounter+2);
 
	if (((registers[nums[0]]+displacement)>memorySize) || ((registers[nums[0]]+displacement)<0)){
		return ADR;
	}
    int write4byte= *getIntAt(registers[nums[0]]+displacement);
    
    printf("%i",write4byte);
    programcounter=programcounter+6;
    return AOK;
}
enum state movsbl(){
    int nums[2];
	if (((programcounter+1)>memorySize) || ((programcounter+1)<0)){
		return ADR;
	}
    unsigned char c= memory[programcounter+1];
    getCharBits(nums,c);
    if (nums[0]<0 || nums[0]>7 || nums[1]<0 || nums[1]>7){
        return INS;
    }
    
	if (((programcounter+2)>memorySize) || ((programcounter+2)<0)){
		return ADR;
	}
	int displacement= *getIntAt(programcounter+2);
	char charToMove;
	int x;
	
	if (((registers[nums[1]]+displacement)>memorySize) || ((registers[nums[1]]+displacement)<0)){
		return ADR;
	}
	charToMove=memory[registers[nums[1]+displacement]]	;
	if (charToMove&0x80){ //if negative
		x=0xffffff00|charToMove;
	}
	else{
		x=0x000000ff&charToMove;
	}
	registers[nums[0]]=x;
	
    /*int displacement= *getIntAt(programcounter+2);
    
    int ra= nums[0];
    int rb= nums[1];
    
    
    unsigned char current= memory[registers[rb]+displacement];
    registers[ra]=current;*/
	
	
	//moving char at memory[rb+displacement] to an integer and placing it in ra
	
	/*int ra= nums[0];
	int rb= nums[1];
	
	int displacement= *getIntAt(programcounter+2);
	printf("ra: %i, rb: %i, displacement: %i\n",ra,rb,displacement);
	//printf("address: %i\n",registers[rb]+displacement);
	char charToMove= memory[registers[rb]+displacement];
//	printf("made it past\n");
	//printf("char to move: %i\n",charToMove);
	registers[ra]= (int) charToMove;*/
	
    programcounter=programcounter+6;
    return AOK;
}
void lineSplitWhiteSpace(char line[],char * lineArray[3]){
    int i=0;
    char * splitter= "\t";
    char * portionline= strtok(line,splitter);
    while (portionline!=0){
        lineArray[i]=portionline;
        portionline= strtok(NULL,splitter); 
        i=i+1;
    }
}
bool equals(char * str1, char * str2){
    return strcmp(str1,str2)==0;
}
void getCharBits(int nums[2],char c){ 
    int cint= (unsigned char) c;
    int left= (cint>>4)&0xf;  
    int right= cint&0xf;

    nums[0]=left;
    nums[1]=right;
    
}
int * getIntAt(int loc){
    return (int*)(memory+loc);
}


















