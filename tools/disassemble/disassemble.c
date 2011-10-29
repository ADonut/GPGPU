#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct ABOpInfo
{
	int isInfix;		// If 1, the format is R op R, otherwise it is op(R)
	int numArgs;
	int isFloat;
	const char *name;
} abOpcodeTable[] = {
	{ 1, 2, 0, "|" },	// 0
	{ 1, 2, 0, "&" }, 	// 1
	{ 1, 2, 0, "&~" }, // 2
	{ 1, 2, 0, "^" },	// 3
	{ 1, 1, 0, "~" },	// 4
	{ 1, 2, 0, "+" }, 	// 5
	{ 1, 2, 0, "-" },	// 6
	{ 1, 2, 0, "*" },	// 7
	{ 1, 2, 0, "/" },	// 8
	{ 1, 2, 0, ">>>" },	// 9
	{ 1, 2, 0, ">>" },	// 10
	{ 1, 2, 0, "<<" }, 	// 11
	{ 0, 1, 0, "clz" },	// 12
	{ 1, 2, 0, "==" },	// 13
	{ 1, 2, 0, "<>" },	// 14
	{ 1, 2, 0, ">" },	// 15
	{ 1, 2, 0, ">=" },	// 16
	{ 1, 2, 0, "<" },	// 17
	{ 1, 2, 0, "<=" },	// 18
	{ 0, 2, 0, "sftoi" },// 19
	{ 0, 2, 1, "sitof" },// 20
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 1, 2, 1, "+" },	// 32
	{ 1, 2, 1, "-" },	// 33
	{ 1, 2, 1, "*" },	// 34
	{ 1, 2, 1, "/" },	// 35
	{ 0, 0, 0, "" },
	{ 0, 0, 0, "" },
	{ 0, 1, 1, "floor" },// 38
	{ 0, 1, 1, "frac" },// 39
	{ 0, 1, 1, "reciprocal" },// 40
	{ 0, 1, 1, "abs" },	// 41
	{ 1, 2, 1, ">" },	// 42
	{ 1, 2, 1, ">=" },	// 43
	{ 1, 2, 1, "<" },	// 44
	{ 1, 2, 1, "<=" },	// 45
	{ 0, 1, 1, "sqrt" },// 46
};

struct AFmtInfo
{
	int op1IsScalar;
	int op2IsScalar;
	int masked;
	int invertMask;
} aFormatTab[] = {
	{ 1, 1, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 1, 1, 0 },
	{ 0, 1, 1, 1 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 1, 1 },
	{ 0, 0, 0, 0 }
};

struct BFmtInfo
{
	int op1IsScalar;
	int masked;
	int invertMask;
} bFormatTab[] = {
	{ 1, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 1, 1 }
};

int isCompareInstruction(int opcode)
{
	return (opcode >= 13 && opcode <= 18)
		|| (opcode >= 42 && opcode <= 45);
}

void disassembleAOp(unsigned int instr)
{
	int opcode = (instr >> 23) & 0x3f;
	const struct ABOpInfo *opInfo = &abOpcodeTable[opcode];
	const struct AFmtInfo *fmtInfo = &aFormatTab[(instr >> 20) & 7];
	char vecSpec;
	char typeSpec;

	if (isCompareInstruction((instr >> 23) & 0x3f))
		vecSpec = 's';
	else
		vecSpec = fmtInfo->op1IsScalar ? 's' : 'v';
	
	if (isCompareInstruction((instr >> 23) & 0x3f))
		typeSpec = 'i';
	else
		typeSpec = opInfo->isFloat ? 'f' : 'i';

	printf("%c%c%d", vecSpec, typeSpec, (instr >> 5) & 0x1f);

	if (fmtInfo->masked)
	{
		printf("{");
		if (fmtInfo->invertMask)
			printf("~");
			
		printf("si%d}", (instr >> 10) & 0x1f);
	}
	
	printf(" = ");
	if (opInfo->isInfix)
	{
		if (opInfo->numArgs == 1)
		{
			printf("%s %c%c%d\n", opInfo->name, 
				fmtInfo->op2IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				(instr >> 15) & 0x1f);
		}
		else
		{
			printf("%c%c%d %s %c%c%d\n", 
				fmtInfo->op1IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				instr & 0x1f,
				opInfo->name, 
				fmtInfo->op2IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				(instr >> 15) & 0x1f);
		}
	}
	else
	{
		if (opInfo->numArgs == 1)
		{
			printf("%s(%c%c%d)\n", opInfo->name, 
				fmtInfo->op2IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				(instr >> 15) & 0x1f);
		}
		else
		{
			// NOTE: we explicitly check for sftoi and sitof, which
			// have odd parameter types (since they are type conversions)
			printf("%s(%c%c%d, %c%c%d)\n", 
				opInfo->name, 
				fmtInfo->op1IsScalar ? 's' : 'v',
				(opcode != 20 && (opcode == 19 || opInfo->isFloat)) ? 'f' : 'i',
				instr & 0x1f,
				fmtInfo->op2IsScalar ? 's' : 'v',
				(opcode != 20 && opInfo->isFloat) ? 'f' : 'i',
				(instr >> 15) & 0x1f);
		}
	}
}

void disassembleBOp(unsigned int instr)
{
	int opcode = (instr >> 26) & 0x1f;
	const struct ABOpInfo *opInfo = &abOpcodeTable[opcode];
	const struct BFmtInfo *fmtInfo = &bFormatTab[(instr >> 24) & 3];
	char vecSpec;
	int immValue = (instr >> 15) & 0x1ff;

	if (isCompareInstruction((instr >> 26) & 0x1f))
		vecSpec = 's';
	else
		vecSpec = fmtInfo->op1IsScalar ? 's' : 'v';
	
	printf("%c%c%d", vecSpec, opcode == 20 ? 'f' : 'i', (instr >> 5) & 0x1f);

	if (fmtInfo->masked)
	{
		printf("{");
		if (fmtInfo->invertMask)
			printf("~");
			
		printf("si%d}", (instr >> 10) & 0x1f);
	}
	
	printf(" = ");

	// Assume two ops: one op B instructions are not allowed
	if (opInfo->isInfix)
	{
		if (opcode == 5 && immValue == 0)
		{
			// An assignment is emulated by just adding 0.  Disassemble
			// that explicitly here.
			printf("%c%c%d\n", 
				fmtInfo->op1IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				instr & 0x1f);
		}
		else
		{
			printf("%c%c%d %s %d\n", 
				fmtInfo->op1IsScalar ? 's' : 'v',
				opInfo->isFloat ? 'f' : 'i',
				instr & 0x1f,
				opInfo->name,
				immValue);
		}
	}
	else
	{
		printf("%s(%c%c%d, %d)\n", 
			opInfo->name, 
			fmtInfo->op1IsScalar ? 's' : 'v',
			(opcode != 20 && (opcode == 19 || opInfo->isFloat)) ? 'f' : 'i',
			instr & 0x1f,
			immValue);
	}
}

const char *memSuffixes[] = {
	"b",
	"bx",
	"s",
	"sx",
	"l",
	"linked"
};

struct CFmtInfo
{
	enum
	{
		SCALAR,
		BLOCK,
		STRIDED,
		SCATTER_GATHER
	} accessType;
	int masked;
	int invertMask;
} cFormatTab[] = {
	{ SCALAR, 0, 0 },
	{ SCALAR, 0, 0 },
	{ SCALAR, 0, 0 },
	{ SCALAR, 0, 0 },
	{ SCALAR, 0, 0 },
	{ SCALAR, 0, 0 },
	{ BLOCK, 0, 0 },
	{ BLOCK, 1, 0 },
	{ BLOCK, 1, 1 },
	{ STRIDED, 0, 0 },
	{ STRIDED, 1, 0 },
	{ STRIDED, 1, 1 },
	{ SCATTER_GATHER, 0, 0 },
	{ SCATTER_GATHER, 1, 0 },
	{ SCATTER_GATHER, 1, 1 },
	{ 0, 0, 0 }		// reserved
};

void printMemRef(const struct CFmtInfo *fmtInfo,
	int offset,
	int ptrReg,
	int op)
{
	switch (fmtInfo->accessType)
	{
		case SCALAR:
			// Scalar
			printf("mem_%s[si%d", memSuffixes[op], ptrReg);
			if (offset != 0)
				printf(" + %d", offset);
				
			break;

		case BLOCK:
			printf("mem_l[si%d", ptrReg);
			if (offset != 0)
				printf(" + %d", offset);
			
			break;
		
		case STRIDED:
			printf("mem_l[si%d", ptrReg);
			if (offset != 0)
				printf(", %d", offset);
				
			break;
		
		case SCATTER_GATHER:
			printf("mem_l[vi%d", ptrReg);
			if (offset != 0)
				printf(" + %d", offset);

			break;
			
		default:
			break;
	}
	
	printf("]");
}

void printMemMask(const struct CFmtInfo *fmtInfo, int instr)
{
	if (fmtInfo->masked)
	{
		printf("{");
		if (fmtInfo->invertMask)
			printf("~");
			
		printf("si%d}", (instr >> 10) & 0x1f);
	}
}

void disassembleCOp(unsigned int instr)
{
	const struct CFmtInfo *fmtInfo = &cFormatTab[(instr >> 25) & 0xf];
	int op = (instr >> 25) & 0xf;
	int offset = (instr >> 15) & 0x3ff;
	int ptrReg = instr & 0x1f;
	int srcDest = (instr >> 5) & 0x1f;

	if (offset & 0x200)
		offset |= 0xfffffc00;	//  Sign extend

	if ((instr >> 29) & 1)
	{
		// Load
		if (fmtInfo->accessType == SCALAR)
			printf("s%d = ", srcDest);
		else
		{
			printf("v%d", srcDest);
			printMemMask(fmtInfo, instr);
			printf(" = ");
		}

		printMemRef(fmtInfo, offset, ptrReg, op);
	}
	else
	{
		// Store
		printMemRef(fmtInfo, offset, ptrReg, op);
		if (fmtInfo->accessType == SCALAR)
			printf(" = s%d", srcDest);
		else
		{
			printMemMask(fmtInfo, instr);
			printf(" = v%d", srcDest);
		}
	}
	
	printf("\n");
}

void disassembleEOp(unsigned int instr)
{
	int target = (instr >> 5) & 0x1fffff;
	int sourceReg = instr & 0x1f;
	if (target & 0x100000)
		target |= 0xffe00000;

	switch ((instr >> 26) & 3)
	{
		case 0:
			printf("ball si%d, %d\n", sourceReg, target);
			break;
		case 1:
			printf("bzero si%d, %d\n", sourceReg, target);
			break;
		case 2:
			printf("bnzero si%d, %d\n", sourceReg, target);
			break;
		case 3:
			printf("goto %d\n", target);
			break;
	}
}

void disassemble(unsigned int instr)
{
	if ((instr & 0xe0000000) == 0xc0000000)
		disassembleAOp(instr);
	else if ((instr & 0x80000000) == 0)
		disassembleBOp(instr);
	else if ((instr & 0xc0000000) == 0x80000000)
		disassembleCOp(instr);
	else if ((instr & 0xf0000000) == 0xf0000000)
		disassembleEOp(instr);
	else
		printf("Unknown instruction\n");
}

int main(int argc, const char *argv[])
{
	FILE *file;
	char line[64];
	unsigned int instr;
	
	if (argc != 2)
	{
		fprintf(stderr, "Enter filename\n");
		return 1;
	}

	file = fopen(argv[1], "r");
	if (file == NULL)
	{
		perror("Error opening file");
		return 1;
	}

	while (fgets(line, sizeof(line), file))
	{
		instr = strtoul(line, NULL, 16);
		disassemble(instr);	
	}
	
	fclose(file);
	return 0;
}