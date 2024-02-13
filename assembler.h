/*
General header file for the assembly.
Contains macros, data structures and methods declaration.
===============
By Michael Bistritzki
ID 203439385
and Noa Minsker
ID 308147131
===============
*/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <string.h>

/* ========== Macros ========== */
/* Utilities */
#define FOREVER				for(;;)
#define BYTE_SIZE			8
#define FALSE				0
#define TRUE				1

/* Given Constants */
#define MAX_DATA_NUM		4096
#define FIRST_ADDRESS		100 
#define MAX_LINE_LENGTH		80
#define MAX_LABEL_LENGTH	30
#define MEMORY_WORD_LENGTH	14
#define MAX_REGISTER_DIGIT	7
#define MACRO_COMMAND		"define"
/* Defining Constants */
#define MAX_LINES_NUM		700
#define MAX_LABELS_NUM		MAX_LINES_NUM 

/* ========== Data Structures ========== */
typedef unsigned int bool; /* Only get TRUE or FALSE values */

/* === First Read  related === */

/* Labels Management */
typedef struct
{
	int address;					/* The address it contains */
	char name[MAX_LABEL_LENGTH];	/* The name of the label */
	bool isExtern;					/* Extern flag */
	bool isData;					/* Data flag (.data or .string) */
} labelInfo;

/* Directive, Macro And Commands */
typedef struct
{
	char *name;
	void(*parseFunc)();
} directive;

typedef struct
{
	char name[MAX_LABEL_LENGTH];
	int lineNum;
	int value;

} macro;

typedef struct
{
	char *name;
	unsigned int opcode : 4;
	int numOfParams;
} command;

/* Operands */
typedef enum { NUMBER = 0, LABEL = 1, INDEX = 2,REGISTER = 3, INVALID = -1 } opType; /* Addressing methods of the operands as described*/

typedef struct
{
	int indexVal;			/* Index value in case of Index opType */
	int value;				/* Value */
	char *str;				/* String */
	opType type;			/* Type of operands */
	int address;			/* The address of the operand in the memory */
} operandInfo;

/* Line */
typedef struct
{
	int lineNum;				/* The number of the line in the file */
	int address;				/* The address of the first word in the line */
	char *originalString;		/* The original pointer, allocated by malloc */
	char *lineStr;				/* The text it contains (changed while using parseLine) */
	bool isError;				/* Represent whether there is an error or not*/
	labelInfo *label;			/* A poniter to the lines label in labelArr */
	char *commandStr;			/* The string of the command or directive */
	macro *mac;					/* A pointer to macro in macroArr */
	char *tempStr;				/* Temporary text of the line (use to adjust parsing in some cases */
	/* Command line */
	const command *cmd;			/* A pointer to the command in g_cmdArr */
	operandInfo op1;			/* The 1st operand */
	operandInfo op2;			/* The 2nd operand */
	labelInfo *jumpLabel;		/* A poniter to the lines label in labelArr */
	char jumpLabelName[MAX_LABEL_LENGTH]; /* the name of label in labelArr */
} lineInfo;

/* === Second Read  === */

typedef enum { ABSOLUTE = 0, EXTENAL = 1, RELOCATABLE = 2 } eraType;

/* Memory Word */

typedef struct /* 14 bits */
{
	unsigned int era : 2;

	union /* 12 bits */
	{
		/* Commands (only 12 bits) */
		struct
		{
			unsigned int dest : 2;		/* Destination op addressing method ID */
			unsigned int src : 2;		/* Source op addressing method ID */
			unsigned int opcode : 4;	/* Command ID */
			unsigned int unUsed : 4;  	/* Bits have no use */
		} cmdBits;	/*Refering to the first 'word' in the translation */

		/* Registers (only 12 bits) */
		struct
		{
			unsigned int destBits : 3;
			unsigned int srcBits : 3;
		} regBits;

		/* Other operands */
		int value : 12; /* (12 bits) */

	} valueBits; /* End of 12 bits union */

} memoryWord;


/* ======== Methods Declaration ======== */

/* utility.c methods */
int getCmdId(char *cmdName);
labelInfo *getLabel(char *labelName);
void trimLeftStr(char **ptStr);
void trimStr(char **ptStr);
char *getFirstTok(char *str, char **endOfTok);
bool isOneWord(char *str);
bool isWhiteSpaces(char *str);
bool isLegalLabel(char *label, int lineNum, bool printErrors);
bool isExistingLabel(char *label);
bool isExistingEntryLabel(char *labelName);
bool isRegister(char *str, int *value);
bool isCommentOrEmpty(lineInfo *line);
char *getFirstOperand(char *line, char **endOfOp, bool *foundComma);
bool isDirective(char *cmd);
bool isMacro(char *cmd);
bool isLegalStringParam(char **strParam, int lineNum);
int getCmdOpCode(char *cmdName);
bool isLegalNum(char *numStr, int numOfBits, int lineNum, int *value);
macro *getMacro(char *macroName);
bool isExistingMacro(char *macro);
int *getMacroValue(macro *mac,int *val);
int getIndexValue(operandInfo *operand);
int getAddressValue(operandInfo *operand);

/* firstRead.c methods */
int firstFileRead(FILE *file, lineInfo *linesArr, int *linesFound, int *IC, int *DC);
void findMacroName(lineInfo *line);
bool areLegalOpTypes(const command *cmd, operandInfo op1, operandInfo op2, int lineNum);
bool addNumberToData(int num, int *IC, int *DC, int lineNum);
/* secondRead.c methods */
int secondFileRead(int *memoryArr, lineInfo *linesArr, int lineNum, int IC, int DC);

/* main.c methods */
void printError(int lineNum, const char *format, ...);

#endif
