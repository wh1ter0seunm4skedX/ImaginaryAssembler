/*
The main file.
This file manages the assembling process.
It calls the first and second read methods, and then creates the output files.

*/

/* ======== Includes ======== */
#include "assembler.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

/* ====== Global Data Structures ====== */
/* Labels */
labelInfo g_labelArr[MAX_LABELS_NUM];
int g_labelNum = 0;
/* Entry Lines */
lineInfo *g_entryLines[MAX_LABELS_NUM]; 
int g_entryLabelsNum = 0;
/* Data */
int g_dataArr[MAX_DATA_NUM];
/* Macro */
macro g_macroArr[MAX_LABELS_NUM];
int macroArrInd;

/* ====== Methods ====== */

/* Prints an error with the line number. */
void printError(int lineNum, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[Error] At line %d: ", lineNum);
	vprintf(format, args);
	printf("\n");
	va_end(args);
}

/*Returns a pointer to a string representing the binary translation of a memory word*/
char *Comp2Binary(int num, char *str)
{
	char bin_str[MEMORY_WORD_LENGTH + 1];
	unsigned int i;
	for (i = 0; i < MEMORY_WORD_LENGTH; i++)
	{
		unsigned int mask = 1u << (MEMORY_WORD_LENGTH - 1 - i);
		bin_str[i] = (num & mask) ? '1' : '0';
	}
	bin_str[MEMORY_WORD_LENGTH] = '\0';
	strcpy(str, bin_str);
	return str;
}

/* Prints a number(a memory word) in base 4 special in the file. */
void fprintfBase4Spcl(FILE *file, int num)
{
	int i;
	char buffer[MEMORY_WORD_LENGTH + 1];
	char *p = buffer;
	p = Comp2Binary(num, p); /* translation from base 2 to base 4 spcl char's */
	
	for (i = 0; i < strlen(p)-1; i+=2)
  	{
		if(p[i] == '0' && p[i+1] == '0')
			fprintf(file,"*");
		else if(p[i] == '0' && p[i+1] == '1')
			fprintf(file,"#");
		else if(p[i] == '1' && p[i+1] == '0')
			fprintf(file,"%%");
		else if(p[i] == '1' && p[i+1] == '1')
			fprintf(file,"!");
	}
}

/* Creates a file (for writing) from a given name and ending, and returns a pointer to it. */
FILE *openFile(char *name, char *ending, const char *mode)
{
	FILE *file;
	char *mallocStr = (char *)malloc(strlen(name) + strlen(ending) + 1), *fileName = mallocStr;
	sprintf(fileName, "%s%s", name, ending);

	file = fopen(fileName, mode);
	free(mallocStr);

	return file;
}

/* Creates the .obj file, which contains the assembled lines in base 2 wird. */
void createObjectFile(char *name, int IC, int DC, int *memoryArr)
{
	int i;

	FILE *file;
	file = openFile(name, ".ob", "w");
	/* Print IC and DC */
	fprintf(file, "\t\t");
	fprintf(file, "%d", IC);
	fprintf(file, "  ");
	fprintf(file, "%d", DC);

	/* Print all of memoryArr */
	for (i = 0; i < IC + DC; i++)
	{
		fprintf(file, "\n");
		fprintf(file, "%d", FIRST_ADDRESS + i);
		fprintf(file, "\t\t");
		fprintfBase4Spcl(file, memoryArr[i]);
	}
	fclose(file);
}

/* Creates the .ent file, which contains the addresses for the .entry labels. */
void createEntriesFile(char *name)
{
	int i;
	FILE *file;

	/* Don't create the entries file if there aren't entry lines */
	if (!g_entryLabelsNum)
	{
		return;
	}

	file = openFile(name, ".ent", "w");

	for (i = 0; i < g_entryLabelsNum; i++)
	{
		fprintf(file, "%s\t\t", g_entryLines[i]->lineStr);
		fprintf(file,"%d", getLabel(g_entryLines[i]->lineStr)->address);

		if (i != g_entryLabelsNum - 1)
		{
			fprintf(file, "\n");
		}
	}

	fclose(file);
}

/* Creates the .ext file, which contains the addresses for the extern labels operands. */
void createExternFile(char *name, lineInfo *linesArr, int linesFound)
{
	int i;
	labelInfo *label;
	bool firstPrint = TRUE; /* This bool meant to prevent the creation of the file if there aren't any externs */
	FILE *file = NULL;

	for (i = 0; i < linesFound; i++)
	{

		
		/* Check if the 1st operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 2 && linesArr[i].op1.type == LABEL)
		{
			label = getLabel(linesArr[i].op1.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintf(file, "%d", linesArr[i].op1.address);
				
				firstPrint = FALSE;
			}
		}

		/* Check if the 2nd operand is extern label, and print it. */
		if (linesArr[i].cmd && linesArr[i].cmd->numOfParams >= 1 && linesArr[i].op2.type == LABEL)
		{
			label = getLabel(linesArr[i].op2.str);
			if (label && label->isExtern)
			{
				if (firstPrint)
				{
					/* Create the file only if there is at least 1 extern */
					file = openFile(name, ".ext", "w");
				}
				else
				{
					fprintf(file, "\n");
				}

				fprintf(file, "%s\t\t", label->name);
				fprintf(file, "%d",linesArr[i].op2.address);
				firstPrint = FALSE;
			}
		}
	}

	if (file)
	{
		fclose(file);
	}
}

/* Resets all the globals and free all the malloc blocks. */
void clearData(lineInfo *linesArr, int linesFound, int dataCount)
{
	int i;

	/* --- Reset Globals --- */

	/* Reset global labels */
	for (i = 0; i < g_labelNum; i++)
	{
		g_labelArr[i].address = 0;
		g_labelArr[i].isData = 0;
		g_labelArr[i].isExtern = 0;
	}
	g_labelNum = 0;
	
	/* Reset global macro */
	for (i = 0; i < macroArrInd; i++)
	{
		g_macroArr[i].lineNum = 0;
		g_macroArr[i].value = 0;
	}
	macroArrInd = 0;
	
	/* Reset global entry lines */
	for (i = 0; i < g_entryLabelsNum; i++)
	{
		g_entryLines[i] = NULL;
	}
	g_entryLabelsNum = 0;

	/* Reset global data */
	for (i = 0; i < dataCount; i++)
	{
		g_dataArr[i] = 0;
	}

	/* Free malloc blocks */
	for (i = 0; i < linesFound; i++)
	{
		free(linesArr[i].originalString);
	}
}

/* Parsing a file, and creating the output files. */
void parseFile(char *fileName)
{
	FILE *file = openFile(fileName, ".as", "r");
	lineInfo linesArr[MAX_LINES_NUM];
	int memoryArr[MAX_DATA_NUM] = { 0 }, IC = 0, DC = 0, numOfErrors = 0, linesFound = 0;

	/* Open File */
	if (file == NULL)
	{
		printf("[Info] Can't open the file \"%s.as\".\n", fileName);
		return;
	}
	printf("[Info] Successfully opened the file \"%s.as\".\n", fileName);

	/* First Read */
	numOfErrors += firstFileRead(file, linesArr, &linesFound, &IC, &DC);
	/* Second Read */
	numOfErrors += secondFileRead(memoryArr, linesArr, linesFound, IC, DC);

	/* Create Output Files */
	if (numOfErrors == 0)
	{
		/* Create all the output files */
		createObjectFile(fileName, IC, DC, memoryArr);
		createExternFile(fileName, linesArr, linesFound);
		createEntriesFile(fileName);
		printf("[Info] Created output files for the file \"%s.as\".\n", fileName);
	}
	else
	{
		/* print the number of errors. */
		printf("[Info] A total of %d error%s found throughout \"%s.as\".\n", numOfErrors, (numOfErrors > 1) ? "s were" : " was", fileName);
	}

	/* Free all malloc pointers, and reset the globals. */
	clearData(linesArr, linesFound, IC + DC);

	/* Close File */
	fclose(file);
}

/* Main method. Calls the "parsefile" method for each file name in argv. */
int main(int argc, char *argv[])
{
	int i;

	if (argc < 2)
	{
		printf("[Info] no file names were observed.\n");
		return 1;
	}

	/* initialize random seed for later use */
	srand((unsigned)time(NULL));

	for (i = 1; i < argc; i++)
	{
		parseFile(argv[i]);
		printf("\n");
	}

	return 0;
}
