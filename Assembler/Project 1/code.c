/*
CS 4200 Group 12: Aubrey Church, Cole Hulick, Colin Elison
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "parser.h"
#include "writer.h"

// Struct that holds data for each instruction
/* [form_type]
	0 --> R type
	1 --> I
	2 --> S
	3 --> SB
	4 --> U
	5 --> UJ
*/
struct instInfo{
	int opcode, form_type, funct3, funct7;
};

struct labelInfo{
	char *label;
	int memAddr;
};


/* Prototypes for functions */
unsigned long int rform_convert(int funct7, int funct3, int opcode, struct token_node* listHead);
unsigned long int iform_convert(int funct3, int opcode, struct token_node* listHead);
unsigned long int sform_convert(int funct3, int opcode, struct token_node* listHead);
unsigned long int sbform_convert(int funct3, int opcode, int pc, struct token_node* listHead);
unsigned long int uform_convert(int opcode, struct token_node* listHead);
unsigned long int ujform_convert(int opcode, int pc, struct token_node* listHead);
int hexToDec(char *hexValue);
int binToDec(char *binValue);
int numToDec(char *val);
int dissasembleNum(int val, int *bounds, int bitSize);
int lookupReg(char *regString);
int lookupLabel(char *label);
void addLabel(char *label, int memAddress);
void convertText(uint32_t *data_segment, uint32_t *text_segment, struct line *llh);
void storeLabels(struct line *llh, int currPC_counter);
void encodeTextSeg(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyTextArrayIndex, int *pc_counter);
void encodeDataSeg(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, int *data_counter, int *unallocIndex);
void directiveWord(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead);
void directiveSpace(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead, int *unallocIndex);
void directiveAsciiz(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead);
void directiveAlign(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead, int *unallocIndex);

// Declared rray to hold data for each label
struct labelInfo *labelArr;

int labelArrEmptyIndex = 0;

// Register array to search for value
char *regArr[32][3] = {
	{"x0","zero"},
	{"x1","ra"},
	{"x2","sp"},
	{"x3","gp"},
	{"x4","tp"},
	{"x5","t0"},
	{"x6","t1"},
	{"x7","t2"},
	{"x8","s0","fp"},
	{"x9","s1"},
	{"x10","a0"},
	{"x11","a1"},
	{"x12","a2"},
	{"x13","a3"},
	{"x14","a4"},
	{"x15","a5"},
	{"x16","a6"},
	{"x17","a7"},
	{"x18","s2"},
	{"x19","s3"},
	{"x20","s4"},
	{"x21","s5"},
	{"x22","s6"},
	{"x23","s7"},
	{"x24","s8"},
	{"x25","s9"},
	{"x26","s10"},
	{"x27","s11"},
	{"x28","t3"},
	{"x29","t4"},
	{"x30","t5"},
	{"x31","t6"}
};

// Array representing data for each instruction
struct instInfo instArr[] = {
	[0] = {.form_type=0, .opcode=51, .funct3=0, .funct7=0}, //add
	[1] = {.form_type=1, .opcode=19, .funct3=0}, //addi
	[2] = {.form_type=0, .opcode=51, .funct3=7, .funct7=0}, //and		
	[3] = {.form_type=1, .opcode=19, .funct3=7}, //andi
	[4] = {.form_type=4, .opcode=23}, //auipc
	[5] = {.form_type=3, .opcode=99, .funct3=0}, //beq
	[6] = {.form_type=3, .opcode=99, .funct3=1}, //bne
	[7] = {.form_type=5, .opcode=111}, //jal
	[8] = {.form_type=1, .opcode=103, .funct3=0}, //jalr
	[9] = {.form_type=4, .opcode=55}, //lui
	[10] = {.form_type=1, .opcode=3, .funct3=2}, //lw
	[11] = {.form_type=0, .opcode=51, .funct3=6, .funct7=0}, //or
	[12] = {.form_type=1, .opcode=19, .funct3=6}, //ori
	[13] = {.form_type=0, .opcode=51, .funct3=2, .funct7=0}, //slt
	[14] = {.form_type=1, .opcode=19, .funct3=2}, //slti
	[15] = {.form_type=0, .opcode=51, .funct3=1, .funct7=0}, //sll
	[16] = {.form_type=1, .opcode=19, .funct3=1, .funct7=0}, //sli
	[17] = {.form_type=0, .opcode=51, .funct3=5, .funct7=32}, //sra
	[18] = {.form_type=1, .opcode=19, .funct3=5, .funct7=32}, //srai 
	[19] = {.form_type=0, .opcode=51, .funct3=5, .funct7=0}, //srl
	[20] = {.form_type=1, .opcode=19, .funct3=5, .funct7=0}, //srli
	[21] = {.form_type=0, .opcode=51, .funct3=0, .funct7=32}, //sub
	[22] = {.form_type=2, .opcode=35, .funct3=2}, //sw
	[23] = {.form_type=0, .opcode=51, .funct3=4, .funct7=0}, //xor
	[24] = {.form_type=1, .opcode=19, .funct3=4}, //xori
		//pseudoinstructions as their replacements
	[25] = {.form_type=5, .opcode=111}, //j as jal
	[26] = {.form_type=4, .opcode=23}, //la as auipc
	[27] = {.form_type=1, .opcode=19, .funct3=0}, //li as addi
	[28] = {.form_type=1, .opcode=19, .funct3=0}, //mv as addi
	[29] = {.form_type=0, .opcode=51, .funct3=0, .funct7=32}, //neg as sub
	[30] = {.form_type=1, .opcode=19, .funct3=0}, //nop as addi
	[31] = {.form_type=1, .opcode=19, .funct3=4}, //not as xori
	[32] = {.form_type=1, .opcode=103, .funct3=0} //ret as jalr
};	

// Set memaddr of data section
int data_counter = 10000000;

// Takes in two pointers to binary array, and pointer to struct of type line
void convertText(uint32_t *data_segment, uint32_t *text_segment, struct line *llh){

	struct line *currentLine = NULL;

	// If the lines don't exist, just exit the program
	if (!llh){
		exit(-1);
	}
 
	// Set initial counters
	int pc_counter = 4194304;
	int emptyTextArrayIndex = 0;
	int unallocIndex = 0;
	int emptyDataArrayIndex = 0;
	int labelCount = 0;
	
	int funct3, funct7, opcode, form_type, finalInstructionValue;
	
	// Iterate through to find how many labels there are in entire program //
	for (currentLine = llh; currentLine != NULL; currentLine = currentLine->next){

		// If label exists, increase label count
		if (currentLine->label) labelCount++;
	}

	// Allocate size of array for appropriate number of labels
	labelArr = malloc(sizeof(struct labelInfo)*labelCount);

	currentLine = llh;
	// Data segment
	if (currentLine->type == 2){

		currentLine = currentLine->next;

		// While the line exists AND is not a text segment AND is a directive...
		while ((currentLine != NULL) && (currentLine->type != 4) && (currentLine->type < 6)){
			
			encodeDataSeg(data_segment,text_segment,currentLine,&emptyTextArrayIndex,&pc_counter,&unallocIndex);
			currentLine = currentLine->next;
		}
	}

	// Text segment
	if(currentLine->type == 4){

		currentLine = currentLine->next;

		struct line *tempLine = currentLine;

		// Goes through and stores labels and respective addresses
		storeLabels(tempLine, pc_counter);

		// // While the line exists AND is an instruction...
		while ((currentLine != NULL) && (currentLine->type >= 6)){

			encodeTextSeg(data_segment,text_segment,currentLine,&emptyTextArrayIndex,&pc_counter);

			// Go to next line
			currentLine = currentLine->next;
		}
	}

	free(labelArr);
}

void encodeTextSeg(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyTextArrayIndex, int *pc_counter){

	int opcode, form_type, funct3, funct7;
	unsigned long int finalInstructionValue;
	
	// Get type of assembly line
	linetype type = currentLine->type;

	// Get pointer to the head of linked list of assembly tokens
	struct token_node *listHead = currentLine->token_listhead;

	// If assembly line type is an instruction...
	if (type >= 6){

		// Set index as type number minus 6
		int instArrIndex = type - 6;

		// Get opcode from struct element inside instArr
		opcode = instArr[instArrIndex].opcode;

		// Get form_type from struct element inside instArr
		form_type = instArr[instArrIndex].form_type;

		// Get funct3 and funct7 from struct element inside instArr
		// These values default to 0 if struct element doesn't have them initialized
		funct3 = instArr[instArrIndex].funct3;
		funct7 = instArr[instArrIndex].funct7;
		
		// Depending on which format type the instruction belongs to, call relevant convert function with appropriate parameters

		// If load address instruction, call uform_convert() and iform_convert() and increment pc_counter and emptyTextArrayIndex twice
		if (type == 32){
			finalInstructionValue = uform_convert(23,listHead);

			text_segment[*emptyTextArrayIndex] = finalInstructionValue;

			*emptyTextArrayIndex = *emptyTextArrayIndex + 1;
			*pc_counter = *pc_counter + 4;

			finalInstructionValue = iform_convert(0,19,listHead);

			text_segment[*emptyTextArrayIndex] = finalInstructionValue;

			*emptyTextArrayIndex = *emptyTextArrayIndex + 1;
			*pc_counter = *pc_counter + 4;
		}
		else{
			// Call the respective convert() function based on what instruction it is
			switch (form_type){
				// R-type
				case 0:
					finalInstructionValue = rform_convert(funct7,funct3,opcode,listHead);
					break;
				// I-type
				case 1:
					finalInstructionValue = iform_convert(funct3,opcode,listHead);
					break;
				// S-type
				case 2:
					finalInstructionValue = sform_convert(funct3,opcode,listHead);
					break;
				// SB-type
				case 3:
					finalInstructionValue = sbform_convert(funct3,opcode,*pc_counter,listHead);
					break;
				// U-type
				case 4:
					finalInstructionValue = uform_convert(opcode,listHead);
					break;
				// UJ-type
				case 5:
					finalInstructionValue = ujform_convert(opcode,*pc_counter,listHead);
					break;
			}
			text_segment[*emptyTextArrayIndex] = finalInstructionValue; // Store value representing instruction in text array at next available slot
			*emptyTextArrayIndex = *emptyTextArrayIndex + 1; // Increment emptyTextArrayIndex counter
			*pc_counter = *pc_counter + 4; // Increment pc counter by 4
		}
	}
}

// Carries out the data segment of the program
void encodeDataSeg(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, int *data_counter, int *unallocIndex){

	linetype type = currentLine->type;

	struct token_node *listHead = currentLine->token_listhead;

	/*
	.align --> 0
	.asciiz --> 1
	.data --> 2
	.space --> 3
	.text --> 4
	.word --> 5
	*/

	// Based on the type of directive, call their respective function
	switch(type){

		// .align
		case 0:
			directiveAlign(data_segment,text_segment,currentLine,emptyDataArrayIndex,listHead,unallocIndex);
			break;
		// .asciiz
		case 1:
			directiveAsciiz(data_segment,text_segment,currentLine,emptyDataArrayIndex,listHead);
			break;
		// .space
		case 3:
			directiveSpace(data_segment,text_segment,currentLine,emptyDataArrayIndex,listHead,unallocIndex);
			break;
		// .word
		case 5:
			directiveWord(data_segment,text_segment,currentLine,emptyDataArrayIndex,listHead);
			break;
	}
}

// Carries out the .space directive
void directiveSpace(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead, int *unallocIndex){
	
	int numAllocSlots = 0;

	int numBytes = numToDec(listHead->next->token);

	numAllocSlots = (int) floor(numBytes/4);

	// If numAllocSlots == 0 and numBytes != 0, set numAllocSlots to 1
	if ((numAllocSlots == 0) && (numBytes != 0)) numAllocSlots = 1;

	if (currentLine->label){
		if (numAllocSlots == 0){
			addLabel(currentLine->label,0); // If no bytes allocated, associate 0 value with label
		}
		else{
			addLabel(currentLine->label,(data_counter + (4*(*unallocIndex)))); // Else add label with mem address 
		}
	}

	*unallocIndex = *unallocIndex + numAllocSlots;
}

// Stores word values in consecutive memory locations
void directiveWord(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead){
	
	// Get memory address
	int memAddress = data_counter + (4*(*emptyDataArrayIndex));

	// label exists, add it with correct memory address
	if (currentLine->label) addLabel(currentLine->label,memAddress);

	struct token_node *args = listHead->next;

	// Iterate through each value and store in consecutive memory locations
	while ((args != NULL)){
		data_segment[*emptyDataArrayIndex] = (unsigned long int) numToDec(args->token);
		*emptyDataArrayIndex = *emptyDataArrayIndex + 1;
		args = args->next;
	}
}

// Carry out .asciiz directive
void directiveAsciiz(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead){

	char *str = listHead->next->token;

	// Set to 2 second character since quotation marks are the first char
	str = str+1;

	// Get rest of string up to qoutation marks
	str = strtok(strdup(str),"\"");

	int memAddress = data_counter + (4*(*emptyDataArrayIndex));

	if (currentLine->label) addLabel(currentLine->label,memAddress);

	int byteIndexCounter = 0;
	int wordVal = 0;

	// Iterate through each character in the string and concatenate them in binary together, with no more than 4 characters per mem slot
	for (int i = 0; i < strlen(str); i++){

		if (byteIndexCounter > 3){

			data_segment[*emptyDataArrayIndex] = wordVal; // Place in memory

			wordVal = 0;
			byteIndexCounter = 0;
			*emptyDataArrayIndex = *emptyDataArrayIndex + 1;
		}

		int asciiVal = str[i];

		wordVal = wordVal | (asciiVal << (8*byteIndexCounter));

		byteIndexCounter++;
	}

	// Store null terminator in last slot to end the string
	int nullTerminator = '\0';

	wordVal = wordVal | (nullTerminator << (8*byteIndexCounter));

	data_segment[*emptyDataArrayIndex] = wordVal;

	*emptyDataArrayIndex = *emptyDataArrayIndex + 1;
}

// Carries out the .align directive
void directiveAlign(uint32_t *data_segment, uint32_t *text_segment, struct line *currentLine, int *emptyDataArrayIndex, struct token_node *listHead, int *unallocIndex){
	char *valString = listHead->next->token;

	int alignVal = numToDec(valString);
}

// Iterates through list and stores labels
void storeLabels(struct line *llh, int currPC_counter){

	int pc_counter = currPC_counter;

	struct line *currentLine = NULL;
	currentLine = llh;

	// While the current line exists and its an instruction
	while ((currentLine != NULL) && (currentLine->type >= 6)){
		
		if (currentLine->label){
			addLabel(currentLine->label,pc_counter);
		}
		// If the instruction is a la address instruction, in which case PC gets incremented twice (+8)
		if (currentLine->type == 32){
			pc_counter += 8;
		}
		else{
			pc_counter += 4;
		}

		currentLine = currentLine->next;
	}
}

// Adds label to labelArr
void addLabel(char *label, int memAddress){
	
	// Includes every character up to the ":"
	char *newLabel = strtok(strdup(label),":");

	struct labelInfo tempStruct;
	tempStruct.label = newLabel;
	tempStruct.memAddr = memAddress;

	// Add to array at empty slot
	labelArr[labelArrEmptyIndex] = tempStruct;

	// Increment counter
	labelArrEmptyIndex++;
}

// Function which lookups the value of a given label and returns it's store memory address
int lookupLabel(char *label){
	for (int i = 0; i < labelArrEmptyIndex; i++){
		if (strcmp(label,labelArr[i].label) == 0){
			return labelArr[i].memAddr;
		}
	}
	return -1;
}

// Function which takes in string value, checks if it corresponds to a register, and if it does, returns the decimal value represented by that register
int lookupReg(char *val){
	for (int row = 0; row < 32; row++){
		for (int column = 0; column < 3; column++){
			if (regArr[row][column]){
				if (strcmp(val,regArr[row][column]) == 0){
					return row;
				}
			}
		}
	}
	return -1;
}

// Converts R-Form instructions
unsigned long int rform_convert(int funct7, int funct3, int opcode, struct token_node* listHead){

	int rd, rs1, rs2;
	unsigned long int funct7L;

	// Parsing through listhead - first element of linked list
	// Linked list used for recieving token
	// Getting string rep of token 
	char* instName = listHead->token;
	char* destReg = listHead->next->token;
	char* source1Reg = listHead->next->next->token;

	// looks for the decimal value associated by the register string both literal and menunonic
	rd = lookupReg(destReg);
	rs1 = lookupReg(source1Reg);
	
	if (strcmp(instName, "neg")) {
		char* source2Reg = listHead->next->next->next->token;
		rs2 = lookupReg(source2Reg);
	}

	//pseudoinstruction
	if (strcmp(instName, "neg") ==0) { //no rs2
		rs2 = rs1;
		rs1 = 0;
	}

	// Shifting left amount of bits
	funct7L = funct7 << 25;
	rs2 = rs2 << 20;
	rs1 = rs1 << 15;
	funct3 = funct3 << 12;
	rd = rd << 7;

	// Builds final decimal value and makes sure not to exclude bits
	return (unsigned long int) (funct7L|rs2|rs1|funct3|rd|opcode); // Stored in text segment array
}

// Converts I-Form instructions
unsigned long int iform_convert(int funct3, int opcode, struct token_node* listHead) {
	//lw inst is exception 

	int rd, rs1, imm;
	unsigned long int immL;
	char *destReg, *source1Reg, *immediateString;

	char *instName = listHead->token;

	//to seperate imm from reg
	if (strcmp(instName,"lw") == 0){ //rd, imm(rs1)

		destReg = listHead->next->token;

		char* str = strdup(listHead->next->next->token);

		// Returns first token
		char *token = strtok(str, "("); // stops at (

		// Keep printing tokens while one of the
		// delimiter "()" is present in str[].
		if (token != NULL){
			immediateString = token;
		}

		if (token = ")"){
			token = strtok(NULL, ")"); 
			source1Reg = token;
		}
		rd = lookupReg(destReg);
		rs1 = lookupReg(source1Reg);
		imm = numToDec(immediateString);
	}
	
	// else if instName does not equal 
	else if (strcmp(instName,"ret") && strcmp(instName,"nop") && strcmp(instName,"li") && strcmp(instName,"mv") && (instName, "not")){
		destReg = listHead->next->token;
		source1Reg = listHead->next->next->token;
		immediateString = listHead->next->next->next->token;
		rd = lookupReg(destReg);
		rs1 = lookupReg(source1Reg);
		imm = numToDec(immediateString);
	}
//pseudoinstructions
	//ret or nop (one token)
	if (strcmp(instName, "nop")== 0){
		rd=0;
		rs1 = 0;
		imm = 0;
	}
	if(strcmp(instName, "ret") ==0) {
		return (32871);
	}
	//li, mv, not
	if ((strcmp(instName,"li") == 0) || (strcmp(instName,"mv") == 0) || (strcmp(instName,"not") == 0)){
		destReg = listHead->next->token;
		source1Reg = listHead->next->next->token;
		rd = lookupReg(destReg);
		rs1 = lookupReg(source1Reg);

		if(strcmp(instName, "li") ==0) { //3 (no rs1)
			imm = rs1;
			rs1 = 0;
		}
		if(strcmp(instName, "mv") ==0) { //3 (imm=0)
			imm=0;
		}
		if(strcmp(instName, "not") ==0) { //3 (no imm, it = 4095)
			imm = 4095;
		}
	}

	int bound [] = {11,10,9,8,7,6,5,4,3,2,1,0};
	int sizeBound = sizeof(bound)/sizeof(int);
	
	// //usage: dissasembleNum(immediate, bounds determined by format, sizeof(bounds)/sizeof(int))
	// //int dissasembleNum(int var, int *bound, int bitSize)
	int immDis = dissasembleNum(imm, bound, sizeBound);

	//shifting left bits to match format
	immL = immDis << 20;
	rs1 = rs1 << 15;
	funct3 = funct3 << 12;
	rd = rd << 7;

	//final decimal value
	return (unsigned long int) (immL|rs1|funct3|rd|opcode);
}

// Converts S-Form instructions
unsigned long int sform_convert(int funct3, int opcode, struct token_node* listHead){
	//the only s type is SW, form is rs2, imm(rs1)

	("\nTHIS IS AN S-TYPE INSTRUCTION\n");

	int rs1, rs2, imm;
	unsigned long int imm1L;
	char *rs1Temp, *source2Reg, *immediateString, *str;

	source2Reg = listHead->next->token;
	str = strdup(listHead->next->next->token);
	     
   	// Returns first token
   	char *token = strtok(str, "("); // stops at (
   
   	// Keep printing tokens while one of the
   	// delimiter "()" is present in str[].
   	if (token != NULL){
       	immediateString = token;
    }

    if (token = ")"){
	    token = strtok(NULL, ")"); 
       	rs1Temp = token;
    }

	rs1 = lookupReg(rs1Temp);
	rs2 = lookupReg(source2Reg);
	imm = numToDec(immediateString);

	int bound1 [] = {11,10,9,8,7,6,5};
	int sizeBound1 = sizeof(bound1) / sizeof(int);
	int bound2 [] = {4,3,2,1,0};
	int sizeBound2 = sizeof(bound2) / sizeof(int);

	int immDis1 = dissasembleNum(imm, bound1, sizeBound1);
	int immDis2 = dissasembleNum(imm, bound2, sizeBound2);

	//shifting left bits to match format
	imm1L = immDis1 << 25;
	rs2 = rs2 << 20;
	rs1 = rs1 << 15;
	funct3 = funct3 << 12;
	immDis2 = immDis2 << 7;

	return (unsigned long int) (imm1L|rs2|rs1|funct3|immDis2|opcode);
}

// Converts SB-Form instructions
unsigned long int sbform_convert(int funct3, int opcode, int pc, struct token_node* listHead){

	//form: rs1, rs2, label (where imm is stored as addr-PC)
	int rs1, rs2, imm;
	unsigned long int imm1L;
	char *source1Reg, *source2Reg, *label;

	source1Reg = listHead->next->token;
	source2Reg = listHead->next->next->token;
	label = listHead->next->next->next->token;

	rs1 = lookupReg(source1Reg);
	rs2 - lookupReg(source2Reg);
	imm = lookupLabel(label);

	imm = imm - pc; /* immediate is stored as addr - PC */

	int bound1 [] = {12,10,9,8,7,6,5};
	int sizeBound1 = sizeof(bound1)/sizeof(int);
	int bound2 [] = {4,3,2,1,11};
	int sizeBound2 = sizeof(bound2) / sizeof(int);

	int immDis1 = dissasembleNum(imm, bound1, sizeBound1);
	int immDis2 = dissasembleNum(imm, bound2, sizeBound2);

	//shifting left bits to match format
	imm1L = immDis1 << 25;
	rs2 = rs2 << 20;
	rs1 = rs1 << 15;
	funct3 = funct3 << 12;
	immDis2 = immDis2 << 7;

	return (unsigned long int) (imm1L|rs2|rs1|funct3|immDis2|opcode);	
}

// Converts U-Form instructions
unsigned long int uform_convert(int opcode, struct token_node* listHead){

	int rd, imm;
	unsigned long int immL;
	char *destReg, *immediateString, *instName;

	instName = listHead->token;
	destReg = listHead->next->token;
	immediateString = listHead->next->next->token;

	rd = lookupReg(destReg);
	
	if(strcmp(instName, "la")) {
		imm = numToDec(immediateString);
	}

	if(strcmp(instName, "la") ==0) {
		imm = lookupLabel(immediateString);
	}

	int bound [] = {31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12};
	int sizeBound = sizeof(bound)/sizeof(int);

	//left shift to fit format
	immL = imm << 12;
	rd = rd << 7;

	return (unsigned long int) (immL|rd|opcode);
}

// Converts UJ-Form instructions
unsigned long int ujform_convert(int opcode, int pc, struct token_node* listHead){

	//rd, imm where imm is stored as addr-pc
	int rd, imm;
	unsigned long int immL;
	char *destReg, *label, *instName;
	
	if(strcmp(instName, "j")) {
		instName = listHead->token; //j
		destReg = listHead->next->token; //label
		label = listHead->next->next->token;

		rd = lookupReg(destReg);
		imm = lookupLabel(label);
		imm = imm - pc;
	}

	if(strcmp(instName, "j") ==0) {
		instName = listHead->token; //j
		label = listHead->next->token; //label
		imm = lookupLabel(label);
		imm = imm - pc;
	}

	int bound [] = {20,10,9,8,7,6,5,4,3,2,1,11,19,18,17,16,15,14,13,12};
	int sizeBound = sizeof(bound)/sizeof(int);

	int immDis = dissasembleNum(imm, bound, sizeBound);

	//left shift to fit format
	immL = immDis << 12;
	rd = rd << 7;

	return (unsigned long int) (immL|rd|opcode);
}

// Takes in string binary value and returns decimal value
int binToDec(char *val){
	int unsignedDecVal = strtol(val,NULL,2);

	int bitLength = strlen(val);

	int isItNegative = unsignedDecVal & (int)pow(2,bitLength-1);

	if (isItNegative){
		return ((unsignedDecVal ^ ((int)pow(2,bitLength)-1))+1)*-1;
	}
	else{
		return unsignedDecVal;
	}
}

// Takes in string hexadecimal value and returns decimal value
int hexToDec(char *val){
	int unsignedDecVal = strtol(val,NULL,16);

	int bitLength = strlen(val)*4;

	int isItNegative = unsignedDecVal & (int)pow(2,bitLength-1);

	if (isItNegative){
		return ((unsignedDecVal ^ ((int)pow(2,bitLength)-1))+1)*-1;
	}
	else{
		return unsignedDecVal;
	}
}

// Function which decides if string represents a decimal number, a binary number, or a hexadecimal number and returns the value represented by it
/* EXAMPLE of 'val':
	-56 --> Decimal value
	0xf12b911 --> Hexadecimal value
	0b1010001011 --> Binary value
*/
int numToDec(char *val){

	int strLength = strlen(val);

	char *pureNum;

	int retVal;

	// If binary value...
	if (strncmp(val,"0b",2) == 0){
		pureNum = malloc(strLength);
		strncpy(pureNum,val+2,strLength-1);
		retVal = binToDec(pureNum);
		free(pureNum);
		return retVal;
	}
	// If hex value...
	else if(strncmp(val,"0x",2) == 0){
		pureNum = malloc(strLength);
		strncpy(pureNum,val+2,strLength-1);
		retVal = hexToDec(pureNum);
		free(pureNum);
		return retVal;
	}
	// Else if decimal value...
	else{
		// Return integer represented by string, NOT ASCII value
		return atoi(val);
	}
}

// Function which takes in value and picks and chooses bits from this value to assemble a new value
/* 
   var --> value to be changed
   bound[] --> array representing the bitmapping of new value
   bitSize --> size of the new value in bits (i.e. the number of elements in bound[])
*/
int dissasembleNum(int var, int *bound, int bitSize){
	//usage: dissasembleNum(immediate, bounds determined by format, sizeof(bounds)/sizeof(int))
	int bitIndex = bitSize-1;

	int assembledNum = 0;

	// Returns a new concatenated binary value based on the bit locations sent in through bound
	for (int i = 0; i < bitSize; i++, bitIndex--){

		int extractBitIndex = bound[i];
		int extractedShift;

		int tempVal = var & ((int) pow(2,extractBitIndex));

		int shiftAmount = abs(extractBitIndex - bitIndex);

		if (extractBitIndex > bitIndex){
			extractedShift = tempVal >> shiftAmount;
		}
		else if (extractBitIndex < bitIndex){
			extractedShift = tempVal << shiftAmount;
		}
		else{
			extractedShift = tempVal;
		}

		assembledNum = assembledNum | extractedShift;

	}
	return assembledNum;
}