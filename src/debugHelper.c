#pragma once
#include "debugHelper.h"

extern boolean* flag;
extern byte* var;
extern byte codeBank;
extern byte newRoomNum;
extern boolean hasEnteredNewRoom, exitAllLogics;

long opCounter = 1;
long stopAt = -1;
long exitAt = -1;
long startPrintingAt = -1;
boolean stopEvery = FALSE;
int _clockBefore = 0;

//#define CHECK_MEM;

void stopAtFunc()
{
	if (opCounter >= 197)
	{
		asm("stp");
		asm("nop");
		asm("nop");
	}
}


#pragma code-name (push, "BANKRAM05");
void b5CheckMemory()
{
#ifdef CHECK_MEM
	int i;
	byte* mem = (byte*) 1;
	for (i = 10; i < 100000 && i >= 0 && mem; i = i + 100)
	{
		mem = (byte*)malloc(i);
		if (!mem)
		{
			printf("Your remaining memory is approx: %d \n", i);
			i = -1;
		}
		else
		{
			free((byte*)mem);
		}
	}

#endif // CHECK_MEM
}

void debugPrint(byte toPrint)
{
	int time;
	int clockVal = (int)clock();
	if (opCounter >= startPrintingAt && startPrintingAt != -1 && startPrintingAt != -1)
	{
		if (clockVal > _clockBefore)
		{
			time = clockVal - _clockBefore;
		}
		else
		{
			time = _clockBefore - clockVal;
		}



		printf("op %lu, %d, var 0 is %d. Time taken %d\n", opCounter, toPrint, var[0], time);
		_clockBefore = clockVal;
#ifdef CHECK_MEM
		b5CheckMemory();
#endif
	}
	if (stopEvery)
	{
		asm("stp");
	}

	if (opCounter == stopAt)
	{
		asm("stp");
		asm("nop");
		return;
	}

	if (opCounter == exitAt)
	{
		exit(0);
	}

	opCounter++;
}

void debugPrintFalse()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("the result is false\n");
	}
}

void debugPrintTrue()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("the result is true\n");
	}
}

void debugPrintNot()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("the result is inverted by not\n");
	}
}

void debugPrintOrMode()
{
	//printf("or mode started\n");
}


extern byte logDebugVal1;
extern byte logDebugVal2;

void debugIsSet()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d is set and it %d\n", logDebugVal1, flag[logDebugVal1]);
	}
}

void debugGreaterThan_8N()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d (%d) is > %d and the result should be %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal1] > logDebugVal2);
	}
}

void debugLessThan_8N()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("Checking that %d is < %d and the result should be %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal1] < logDebugVal2);
	}
}

void debugGreaterThan_8V()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d (%d) is > %d and the result should be %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], var[logDebugVal1] > var[logDebugVal2]);
	}
}

void debugLessThan_8V()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d is < %d and the result should be %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], var[logDebugVal1] < var[logDebugVal2]);
	}
}

void debugEqualN()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d (%d) is equal to %d and it %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal1] == logDebugVal2);
	}
}

void debugEqualV()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("checking that %d (%d) is equal to %d (%d) and it %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], var[logDebugVal1] == var[logDebugVal2]);
	}
}

void debugInc()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("incrementing var %d(%d) to %d\n", logDebugVal1, var[logDebugVal1], var[logDebugVal1] + 1);
	}
}

void debugDec()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("decrementing var %d(%d) to %d\n", logDebugVal1, var[logDebugVal1], var[logDebugVal1] - 1);
	}
}

void debugAddN()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("add var %d (%d) to %d which is %d", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal1] + logDebugVal2);
	}
}

void debugAddV()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("add var %d (%d) to %d (%d) which is\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], var[logDebugVal1] + var[logDebugVal2]);
	}
}

void debugSubN()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("sub var %d (%d) to %d which is\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal1] - logDebugVal2);
	}
}

void debugSubV()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1) {
		printf("sub var %d (%d) to %d (%d) which is\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], var[logDebugVal1] - var[logDebugVal2]);
	}
}

void debugAssignN()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("assign var %d (%d) to %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2);
	}
}

void debugAssignV()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("assign var %d (%d) to %d (%d) which is\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2], logDebugVal2);
	}
}

void debugIndirect()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("indir %d (%d) value %d\n", logDebugVal1, var[logDebugVal1], logDebugVal2);
	}
}

void debugIndirectV()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("indir V %d (%d) value %d (%d)\n", logDebugVal1, var[logDebugVal1], logDebugVal2, var[logDebugVal2]);
	}
}


void debugPostCheckVar()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("post check var %d (%d)\n", logDebugVal1, var[logDebugVal1]);
	}
}

void debugPostCheckFlag()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("post check flag %d (%d)\n", logDebugVal1, flag[logDebugVal1]);
	}
}

void codeJumpDebug()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("b1 is %d b2 is %d, Shift %u and the jump result is %u.\n", logDebugVal1, logDebugVal2, (logDebugVal2 << 8), (logDebugVal2 << 8) | logDebugVal1);
	}
}

void debugNewRoom()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("Attempting to enter new room %d\n", logDebugVal1);
	}
}

void debugExitAllLogics()
{
	if (opCounter >= startPrintingAt && startPrintingAt != -1)
	{
		printf("----------Exit Debug: Attempting to enter new room %d. Has entered new Room: %d. Has exited all logics %d\n", newRoomNum, hasEnteredNewRoom, exitAllLogics);
	}
}

void debugPrintCurrentCodeState(byte* code)
{
	byte codeValue;
	memCpyBanked(&codeValue, code, codeBank, 1);
	if (opCounter >= startPrintingAt && startPrintingAt != -1) {
		printf("the code is now %u and the address is %p\n", codeValue, code);
	}
}

#pragma code-name (pop);





