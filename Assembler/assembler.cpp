/* Assembler code fragment */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace std;

#define MAXLINELENGTH 1000
#define MAXLABELS 1000
#define MAXLABELLEN 7 // Label max length 6 char

typedef struct { 
    char label[MAXLABELLEN+1]; // array of each label (+1 for \0)
    int address;
} LabelEntry;

LabelEntry labelTable[MAXLABELS];
int labelCount = 0;

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int findLabelAddr(const char* );

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        /* reached end of file */
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    // -------- Phase 1 ----------
    // รับ label ทั้งหมด (อาจมี label ที่ reference อยู่ข้างหน้า)

    labelCount = 0;
    int address = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        // DEBUG ทุกบรรทัดที่มีคำสั่ง
        // cout << "DEBUG [" << address << "] label:'" << label << "' opcode:'" << opcode << "'" << endl;

        // มี label
        if (label[0] != '\0') {
            strcpy(labelTable[labelCount].label, label);
            labelTable[labelCount].address = address;
            labelCount++;
        }
        address++;  // address ต้องเพิ่มทุกบรรทัดที่มีคำสั่ง/ข้อมูล
    }

    //แสดง Label + address
    // cout << "Label Table:" << endl;
    // for (int i = 0; i < labelCount; ++i) {
    //     cout << "  " << labelTable[i].label << " -> " << labelTable[i].address << endl;
    // }

    // -------- Phase 2 ----------
    // แปลงแต่ละคำสั่งเป็น machine code

    /*reading from the beginning of the file(round 2)*/
    rewind(inFilePtr);

    int pc = 0; // Program Counter แทน address ของแต่ละคำสั่ง

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if (opcode[0] == '\0') {
            pc++; // ทุกบรรทัดในไฟล์ถือเป็น address แม้ opcode จะว่าง
            continue; // ข้ามบรรทัดว่าง
        }

        int machineCode = 0;

        // ======== R-type instructions =========
        if (!strcmp(opcode, "add")) {
            int regA = atoi(arg0);
            int regB = atoi(arg1);
            int destReg = atoi(arg2);
            machineCode = (0 << 22) | (regA << 19) | (regB << 16) | destReg;
        }
        else if (!strcmp(opcode, "nand")) {
            int regA = atoi(arg0);
            int regB = atoi(arg1);
            int destReg = atoi(arg2);
            machineCode = (1 << 22) | (regA << 19) | (regB << 16) | destReg;
        }
        // ======== I-type instructions =========
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) {
            int opNum = (!strcmp(opcode, "lw")) ? 2 : 3;
            int regA = atoi(arg0);
            int regB = atoi(arg1);
            int offset;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            } else {
                offset = findLabelAddr(arg2); // offsetField
            }
            // รองรับ negative และ mask 16 บิต
            machineCode = (opNum << 22) | (regA << 19) | (regB << 16) | (offset & 0xFFFF);
        }
        else if (!strcmp(opcode, "beq")) {
            int regA = atoi(arg0);
            int regB = atoi(arg1);
            int offset;
            if (isNumber(arg2)) {
                offset = atoi(arg2);
            } else {
                offset = findLabelAddr(arg2) - (pc + 1); // offsetField = labelAddr - (pc+1)
            }
            // รองรับ negative และ mask 16 บิต
            machineCode = (4 << 22) | (regA << 19) | (regB << 16) | (offset & 0xFFFF);
        }
        // ======== J-type instruction =========
        else if (!strcmp(opcode, "jalr")) {
            int regA = atoi(arg0);
            int regB = atoi(arg1);
            machineCode = (5 << 22) | (regA << 19) | (regB << 16);
        }
        // ======== O-type instructions =========
        else if (!strcmp(opcode, "halt")) {
            machineCode = (6 << 22);
        }
        else if (!strcmp(opcode, "noop")) {
            machineCode = (7 << 22);
        }
        // ======== .fill Pseudo-instruction =========
        else if (!strcmp(opcode, ".fill")) {
            if (isNumber(arg0)) {
                machineCode = atoi(arg0);
            } else {
                machineCode = findLabelAddr(arg0);
            }
        }
        // ========= ไม่ตรง opcode อื่น =========
        else {
            printf("error: unrecognized opcode: %s at address %d\n", opcode, pc);
            exit(2);
        }

        // เขียน machine code ลงไฟล์ output
        fprintf(outFilePtr, "%d\n", machineCode);

        pc++; // address +1 ทุกครั้ง
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    

    /* after doing a readAndParse, you may want to do the following to test the
        opcode */
    if (!strcmp(opcode, "add")) {
        /* do whatever you need to do for opcode "add" */
    }

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    bool foundLine = false;
    while (fgets(line, MAXLINELENGTH, inFilePtr) != NULL) {
        // ตรวจสอบว่าเป็นบรรทัดว่างจริงๆ (มีแต่ whitespace)
        bool isEmpty = true;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') {
                isEmpty = false;
                break;
            }
        }
        if (!isEmpty)   
        {
            foundLine = true;
            break;  // ถ้าไม่ว่างให้หยุด loop
        }
    }

    if (!foundLine) return 0;

    // ถ้าอ่านจนจบไฟล์
    if (feof(inFilePtr)) return 0;

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
	printf("error: line too long\n");
	exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int findLabelAddr(const char* target) {
    for (int i = 0; i < labelCount; ++i) {
        if (!strcmp(labelTable[i].label, target)) {
            return labelTable[i].address;
        }
    }
    printf("error: undefined label: %s\n", target);
    exit(1);
}

