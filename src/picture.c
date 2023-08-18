/**************************************************************************
** picture.c
**
** Routines to load AGI picture resources, discard them, draw them and
** show the various screens.
**
** (c) Lance Ewing, 1997 - Modified (6 Jan 98)
**************************************************************************/

#include "picture.h"

#define PIC_DEFAULT 15
#define PRI_DEFAULT 4
//#define VERBOSE
//#define VERBOSE_REL_DRAW
//#define TEST_QUEUE
//#define VERBOSE_FLOOD
//#define TEST_DIVISION 
//#define TEST_ROUND
#define VERBOSE_DRAW_LINE

boolean okToShowPic = FALSE;
PictureFile* loadedPictures = (PictureFile*)&BANK_RAM[PICTURE_START];
int screenMode;
int min_print_line = 1, user_input_line = 23, status_line_num = 0;
boolean statusLineDisplayed = FALSE, inputLineDisplayed = FALSE;

BITMAP* picture;
BITMAP* priority;
BITMAP* control;
BITMAP* agi_screen;      /* This is a subbitmap of the screen */

boolean picDrawEnabled = FALSE, priDrawEnabled = FALSE;
/* Not sure what the default patCode value is */
byte picColour = 0, priColour = 0, patCode, patNum;
word rpos = FLOOD_QUEUE_START, spos = FLOOD_QUEUE_START;
byte rposBank = FIRST_FLOOD_BANK, sposBank = FIRST_FLOOD_BANK;


/* QUEUE DEFINITIONS */
#define QEMPTY 0xFF
int* bitmapWidthPreMult = &BANK_RAM[BITMAP_WIDTH_PREMULT_START];

extern void b11PSet(byte x, byte y);
extern fix32 floatDivision(byte numerator, byte denominator);

#ifdef VERBOSE_FLOOD
extern long pixelCounter;
extern long pixelStartPrintingAt;
#endif // VERBOSE_FLOOD

void getLoadedPicture(PictureFile* returnedloadedPicture, byte loadedPictureNumber)
{
	byte previousRamBank = RAM_BANK;

	RAM_BANK = PICTURE_BANK;

	*returnedloadedPicture = loadedPictures[loadedPictureNumber];

	RAM_BANK = previousRamBank;
}

void setLoadedPicture(PictureFile* loadedPicture, byte loadedPictureNumber)
{
	byte previousRamBank = RAM_BANK;

	RAM_BANK = PICTURE_BANK;

	loadedPictures[loadedPictureNumber] = *loadedPicture;

	RAM_BANK = previousRamBank;
}

#pragma code-name (push, "BANKRAMFLOOD")

extern byte bFloodQretrieve();
extern byte bFloodQstore(byte q);

/**************************************************************************
** pset
**
** Draws a pixel in each screen depending on whether drawing in that
** screen is enabled or not.
**************************************************************************/
#define PSETFLOOD(x, y) \
    if (picDrawEnabled) { \
if ((x) <= 159 && (y) <= 167) {  \
            trampoline_2Byte(&b11PSet,x, y, PICTURE_CODE_BANK); \
           } \
    } 


extern byte bFloodPicGetPixel(word x, word y);

///**************************************************************************
//** picGetPixel
//**
//** Get colour at x,y on the picture page.
//**************************************************************************/
//byte bFloodPicGetPixel(word x, word y)
//{
//    if (x > 159) return(PIC_DEFAULT);
//    if (y > 167) return(PIC_DEFAULT);
//return (picture->line[y][x]);
//}

/**************************************************************************
** priGetPixel
**
** Get colour at x,y on the priority page.
**************************************************************************/
byte bFloodPriGetPixel(word x, word y)
{
	if (x > 159) return(PRI_DEFAULT);
	if (y > 167) return(PRI_DEFAULT);
	return (priority->line[y][x]);
}


//boolean bFloodEmpty()
//{
//    return (rpos == spos);
//}

//void bFloodQstore(word q)
//{
//    if (spos + 1 == rpos || (spos + 1 == QMAX && !rpos)) {
//        nosound();
//        return;
//    }
//    buf[spos] = q;
//    spos++;
//    if (spos == QMAX) spos = 0;  /* loop back */
//}
//
//word bFloodQretrieve()
//{
//    if (rpos == QMAX) rpos = 0;  /* loop back */
//    if (rpos == spos) {
//        return QEMPTY;
//    }
//    rpos++;
//    return buf[rpos - 1];
//}

/**************************************************************************
** okToFill
**************************************************************************/
boolean bFloodOkToFill(byte x, byte y)
{
	boolean getPicResult;

#ifdef VERBOSE_FLOOD
	if (pixelCounter >= pixelStartPrintingAt)
	{
		printf("State: pic: %d, pri %d, color: %d\n", picDrawEnabled, priDrawEnabled, picColour);
	}
#endif

	if (!picDrawEnabled && !priDrawEnabled) return FALSE;
	if (picColour == PIC_DEFAULT) return FALSE;
	if (!priDrawEnabled)
	{
		getPicResult = bFloodPicGetPixel(x, y);
#ifdef VERBOSE_FLOOD
		if (pixelCounter >= pixelStartPrintingAt)
		{
			printf("result %d,%d %d \n", x, y, getPicResult);
		}
#endif
		return (getPicResult == PIC_DEFAULT);
	}
	if (priDrawEnabled && !picDrawEnabled) return (bFloodPriGetPixel(x, y) == PRI_DEFAULT);
	return (bFloodPicGetPixel(x, y) == PIC_DEFAULT);
}

#ifdef TEST_QUEUE
void testQueue()
{
	int testAmount = 40103;
	byte testVal;
	unsigned int i;
	printf("The address of i %p and the address of testVal is %p\n", &i, &testVal);

	for (i = 0; i <= testAmount; i++)
	{
		bFloodQstore(i);
	}


	for (i = 0; i <= testAmount; i++)
	{
		testVal = bFloodQretrieve();
		if ((byte)i != testVal)
		{
			asm("stp");
			asm("lda #4"); //This is a deliberately pointless instruction. It is there so we can see where we have stopped in the debugger
		}
	}

	asm("stp");
	asm("lda #$19");
}
#endif // TEST_QUEUE


/**************************************************************************
** agiFill
**************************************************************************/
void bFloodAgiFill(word x, word y)
{
	byte x1, y1;
#ifdef TEST_QUEUE
	testQueue();
#endif // TEST_QUEUE

#ifdef VERBOSE_FLOOD_FILL
	if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
		printf("bl\n");
	}
#endif

	bFloodQstore(x);
	bFloodQstore(y);

	for (;;) {
#ifdef VERBOSE_FLOOD_FILL
		if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
			printf("al\n");
		}
#endif
		x1 = bFloodQretrieve();
		y1 = bFloodQretrieve();

#ifdef VERBOSE_FLOOD_FILL
		if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
			printf("R %d,%d\n", x1, y1);
		}
#endif // VERBOSE_FLOOD



		if ((x1 == QEMPTY) || (y1 == QEMPTY))
			break;
		else {

			if (bFloodOkToFill(x1, y1)) {

				PSETFLOOD(x1, y1);

				if (bFloodOkToFill(x1, y1 - 1) && (y1 != 0)) {
#ifdef VERBOSE_FLOOD_FILL
					if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
						printf("1\n");
					}
#endif
					bFloodQstore(x1);
					bFloodQstore(y1 - 1);
				}
				if (bFloodOkToFill(x1 - 1, y1) && (x1 != 0)) {
#ifdef VERBOSE_FLOOD_FILL
					if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
						printf("2\n");
					}
#endif
					bFloodQstore(x1 - 1);
					bFloodQstore(y1);
				}
				if (bFloodOkToFill(x1 + 1, y1) && (x1 != 159)) {
#ifdef VERBOSE_FLOOD_FILL
					if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
						printf("3\n");
					}
#endif
					bFloodQstore(x1 + 1);
					bFloodQstore(y1);
				}
				if (bFloodOkToFill(x1, y1 + 1) && (y1 != 167)) {
#ifdef VERBOSE_FLOOD_FILL
					if (pixelCounter >= pixelStartPrintingAt && pixelStartPrintingAt != 1) {
						printf("4\n");
					}
#endif
					bFloodQstore(x1);
					bFloodQstore(y1 + 1);
				}

			}

		}

	}

}
#pragma code-name (pop)
#pragma code-name (push, "BANKRAM11")

fix32 DIV(int numerator, int denominator) {
	if (denominator == 0 || numerator == 0) {
		return (fix32)0;
	}
	else if (denominator == 1) {
		return (fix32)(numerator) << 16;
	}
	else if (numerator == denominator) {
		return (fix32)1 << 16;
	}
	else {
		return floatDivision(numerator, denominator);
	}
}

#define ROUND_THRESHOLD_POS 499 
#define ROUND_THRESHOLD_NEG 501
int round(fix32 aNumber, boolean isPos)
{
	if(isPos)
	{
#ifdef TEST_ROUND
	printf("%lu Pos True %d result %p. %d < %d\n", aNumber, getMantissa(aNumber), getMantissa(aNumber) < ROUND_THRESHOLD_POS ? floor_fix_32(aNumber) : ceil_fix_32(aNumber), getMantissa(aNumber), ROUND_THRESHOLD_POS);
#endif
		return getMantissa(aNumber) < ROUND_THRESHOLD_POS ? floor_fix_32(aNumber) : ceil_fix_32(aNumber);
	}
	else
	{
#ifdef TEST_ROUND
		printf("%lu Neg True %d result %p %d < %d\n", aNumber, getMantissa(aNumber), getMantissa(aNumber) <= ROUND_THRESHOLD_NEG ? floor_fix_32(aNumber) : ceil_fix_32(aNumber), getMantissa(aNumber), ROUND_THRESHOLD_POS);
#endif
		return getMantissa(aNumber) <= ROUND_THRESHOLD_NEG ? floor_fix_32(aNumber) : ceil_fix_32(aNumber);
	}
}

#ifdef TEST_ROUND
void testRound()
{
	int result;
	result = round(0x4301F3, TRUE);

	if (result != 0x44)
	{
		printf("Fail Round 1 Pos (Equal). Expected %p got %p \n", 0x44, result);
	}

	result = round(0x4301F2, TRUE);

	if (result != 0x43)
	{
		printf("Fail Round 2 Pos (Less). Expected %p got %p \n", 0x43, result);
	}

	result = round(0x4301F4, TRUE);

	if (result != 0x44)
	{
		printf("Fail Round 2 Pos (Less). Expected %p got %p \n", 0x44, result);
	}

	result = round(0x4301F5, FALSE);

	if (result != 0x43)
	{
		printf("Fail Round 1 Neg (Equal). Expected %p got %p \n", 0x43, result);
	}

	result = round(0x4301F4, FALSE);

	if (result != 0x43)
	{
		printf("Fail Round 2 Neg (Less). Expected %p got %p \n", 0x43, result);
	}

	result = round(0x4301F6, FALSE);

	if (result != 0x44)
	{
		printf("Fail Round 3 Neg (Greater). Expected %p got %p \n", 0x44, result);
	}
}
#endif // TEST_ROUND


#ifdef TEST_DIVISION
void testDivision()
{
	fix32 result;

	result = DIV(0x1, 0x2); //1 and 167

	if (result != 0x1388l)
	{
		printf("Fail Division 1. Expected %lx got %lx\n", 0x1388l, result);
	}

	result = DIV(0x1, 0xA7); //1 and 167

	if (result != 0x3Cl)
	{
		printf("Fail Division 2. Expected %lx got %lx\n", 0x3Cl, result);
	}

	result = DIV(0x23, 0x40); //35 and 64

	if (result != 0x155Dl)
	{
		printf("Fail Division 3. Expected %lx got %lx\n", 0x155Dl, result);
	}

	result = DIV(0xA7, 0x2); //167 and 2

	if (result != 0x531388)
	{
		printf("Fail Division 4. Expected %lx got %lx\n", 0xCBDB8l, result);
	}


	result = DIV(0, 0xA7); //1 and 167

	if (result != 0)
	{
		printf("Fail Division 5. Expected %lx got %lx\n", 0, result);
	}

	result = DIV(0xA7, 0); //1 and 167

	if (result != 0)
	{
		printf("Fail Division 6. Expected %lx got %lx\n", 0, result);
	}


	result = DIV(0xA7, 1); //1 and 167

	if (result != int_to_fix32(0xA7))
	{
		printf("Fail Division 7. Expected %lx got %lx\n", int_to_fix32(0xA7), result);
	}


	result = DIV(0xA7, 0xA7); //1 and 167

	if (result != int_to_fix32(0x1))
	{
		printf("Fail Division 8. Expected %lx got %lx\n", int_to_fix32(0x1), result);
	}
}
#endif // TEST_DIVISION


void b11FloodBankFull() //Is on this bank to save room on flood bank 
{
	printf("warning Flood Bank Full\n");
}


/**************************************************************************
** fill
**
** Agi flood fill.  (drawing action 0xF8)
**************************************************************************/
void b11FloodFill(byte** data)
{
	byte x1, y1;
	byte picColorOld = picColour;
	picColour = 0xE;

	//b11PSet(90, 43);
	picColour = picColorOld;

	for (;;) {
		if ((x1 = *((*data)++)) >= 0xF0) break;
		if ((y1 = *((*data)++)) >= 0xF0) break;
		trampoline_2Int(&bFloodAgiFill, x1, y1, FIRST_FLOOD_BANK);
	}

	(*data)--;
}

void b11LoadDivisionMetadata(const char* fileName, int metadataSize, byte* metadataLocation)
{
	FILE* fp;
	char fileNameBuffer[30];
	size_t bytesRead;

#ifdef VERBOSE
	printf("The filename is %s and the metadata size is %d\n", fileName, metadataSize);
#endif // VERBOSE


	if ((fp = fopen(fileName, "rb")) != NULL) {
     	bytesRead = fread(&GOLDEN_RAM_WORK_AREA[0], 1, metadataSize, fp);
		
#ifdef VERBOSE
		printf("Read %d bytes. The first byte is %p\n", bytesRead);
#endif // VERBOSE
		
		memCpyBanked(metadataLocation, &GOLDEN_RAM_WORK_AREA[0], DIV_METADATA_BANK, metadataSize);
#ifdef VERBOSE
		printf("Copy %d bytes to location %p \n", bytesRead, metadataLocation);
#endif // VERBOSE

		fclose(fp);
	}
	else {
		printf("Failed to open %s\n", fileName);
	}
}


#define SWIDTH   640  /* Screen resolution */
#define SHEIGHT  480
#define PWIDTH   160  /* Picture resolution */
#define PHEIGHT  168
#define VWIDTH   640  /* Viewport size */
#define VHEIGHT  336

void b11LoadDivisionTables()
{
	FILE* fp;
	int bank, i;
	char fileNameBuffer[30];
	const char* divFileName = "div%x.bin";
	const char* bankfileName = "divb.bin";
	const char* addressfileName = "diva.bin";

	for (bank = FIRST_DIVISION_BANK; bank <= LAST_DIVISION_BANK; bank++)
	{
		sprintf(&fileNameBuffer[0], divFileName, bank);

		printf("Loading division tables %d of %d \n", bank - FIRST_DIVISION_BANK + 1, LAST_DIVISION_BANK - FIRST_DIVISION_BANK + 1);
		if ((fp = fopen(&fileNameBuffer[0], "rb")) != NULL) {
			size_t bytesRead;
			i = 0;
			while ((bytesRead = fread(&GOLDEN_RAM_WORK_AREA[0], 1, LOCAL_WORK_AREA_SIZE, fp)) > 0) {
				memCpyBanked(DIVISION_AREA + LOCAL_WORK_AREA_SIZE * i++, &GOLDEN_RAM_WORK_AREA[0], bank, bytesRead);
			}

			fclose(fp);
		}
		else {
			printf("failed to division table file %s\n", &fileNameBuffer[0]);
		}
	}
	printf("Loading Division Metdata 1 of 2\n");
	b11LoadDivisionMetadata(bankfileName, DIV_BANK_METADATA_SIZE, &divBankMetadata[0]);
	printf("Loading Division Metdata 2 of 2\n");
	b11LoadDivisionMetadata(addressfileName, DIV_ADDRESS_METADATA_SIZE, &divAddressMetadata[0]);
}

/**************************************************************************
** initPicture
**
** Purpose: To initialize allegro and create the picture and priority
** bitmaps. This function gets called once at the start of an AGI program.
**************************************************************************/
void b11InitPicture()
{
	int i;
	int* tempbitmapWidthPreMult = (int*)GOLDEN_RAM_WORK_AREA;

	for (i = 0; i < PICTURE_HEIGHT; i++)
	{
		tempbitmapWidthPreMult[i] = i * BYTES_PER_ROW;
	}

	memcpy(&bitmapWidthPreMult[0], &tempbitmapWidthPreMult[0], PICTURE_HEIGHT * 2);

	for (i = FIRST_FLOOD_BANK; i <= LAST_FLOOD_BANK; i++)
	{
		memCpyBanked(&bitmapWidthPreMult[0], &tempbitmapWidthPreMult[0], i, PICTURE_HEIGHT * 2);
	}

	b11LoadDivisionTables();
}
/**************************************************************************
** initAGIScreen
**
** Purpose: Sets the screen mode to 640x480x256. This can be called a
** number of times during the program, e.g. when switching back from a
** text mode.
**************************************************************************/
void b11InitAGIScreen()
{
}

/**************************************************************************
** clearPicture
**
** Purpose: To clear the picture and priority bitmaps so that they are
** ready for drawing another PICTURE.
**************************************************************************/
void b11ClearPicture()
{
	trampoline_0(&b6ClearBackground, IRQ_BANK);
	clear_to_color(priority, PRI_DEFAULT);
	clear_to_color(control, PRI_DEFAULT);
}

/**************************************************************************
** priPSet
**
** Draws a pixel in the priority screen.
**************************************************************************/
void b11PriPSet(word x, word y)
{
	if (x > 159) return;
	if (y > 167) return;
	priority->line[y][x] = priColour;
}

/**************************************************************************
** pset
**
** Draws a pixel in each screen depending on whether drawing in that
** screen is enabled or not.
**************************************************************************/
#define PSET(x, y) \
    if (picDrawEnabled) { \
if ((x) <= 159 && (y) <= 167) {  \
             b11PSet(x, y); \
           } \
    }


void b11Drawline(byte x1, byte y1, byte x2, byte y2)
{
	int height, width, startX, startY;
	boolean xIsPos = TRUE, yIsPos = TRUE;
	fix32 x, y, addX, addY;
	word temp;
	
#ifdef VERBOSE_DRAW_LINE
	printf("drawing %d:%d %d:%d \n", x1, y1, x2, y2);
#endif // VERBOSE_DRAW_LINE

	asm("stp");

	if (x1 > x2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
		xIsPos = FALSE;

#ifdef VERBOSE_DRAW_LINE
		printf("swap x drawing %d:%d %d:%d \n", x1, y1, x2, y2);
#endif
	}

	if (y1 > y2)
	{
		temp = y1;
		y1 = y2;
		y2 = temp;
		yIsPos = FALSE;

#ifdef VERBOSE_DRAW_LINE
	printf("swap y drawing %d:%d %d:%d \n", x1, y1, x2, y2);
#endif
	}

	height = (y2 - y1);
	printf("Height %d - %d = %d\n", y2, y1, height);

	width = (x2 - x1);
	printf("Width %d - %d = %d \n", x2, x1, width);

	printf("****%d == %d (%d) %lx \n", width, height, width == height , (fix32)1 << 16);
	addX = height == 0 ? height : DIV(width, height);

#ifdef VERBOSE_DRAW_LINE
	printf("divide addx %d / %d result: %lx  Address %p\n", width, height, addX, &addX);
#endif // VERBOSE

	addY = width == 0 ? width : DIV(height, width);


#ifdef VERBOSE_DRAW_LINE
	printf("divide addy %d / %d result: %lx. Address %p\n ", height, width, addY, &addY);
#endif // VERBOSE

	if (width > height) {
		y = int_to_fix32(y1);


#ifdef VERBOSE_DRAW_LINE
		printf("convert top y %d to fix32 %lx\n ", y1, y);
#endif // VERBOSE

		addX = (width == 0 ? 0 : int_to_fix32(1));

#ifdef VERBOSE_DRAW_LINE
		printf("convert top width (%d) to fix32 %lx\n ", width, addX);
#endif // VERBOSE

		for (x = int_to_fix32(x1); x != int_to_fix32(x2); x += addX) {
#ifdef VERBOSE_DRAW_LINE
			printf("psettop in loop %lx, %d (isPos), %lx, %d (isPos)  round %d %d\n", x, xIsPos, y, yIsPos, round(x, xIsPos), round(y, yIsPos));
#endif // VERBOSE

			PSET(round(x, xIsPos), round(y, yIsPos));

#ifdef VERBOSE_DRAW_LINE
			printf("add y top %lx + %lx = %lx\n", y, addY, y + addY);
#endif
			y += addY;

#ifdef VERBOSE_DRAW_LINE
			printf("add x top %lx + %lx = %lx, %lx != %lx (%d)\n", x, addX, x + addX, x + addX, int_to_fix32(x2), x + addX != int_to_fix32(x2));
#endif
		}

#ifdef VERBOSE_DRAW_LINE
		printf("pset top out of loop %d,%d\n", x2, y2);
#endif
		PSET(x2, y2);
	}
	else {
		x = int_to_fix32(x1);
#ifdef VERBOSE_DRAW_LINE
		printf("%d convert bottom x to fix32 %d\n ",x1, x);
#endif // VERBOSE


		addY = (height == 0 ? 0 : int_to_fix32(1));


#ifdef VERBOSE_DRAW_LINE
		printf("convert top height (%d) to fix32 %lx\n ", height, addY);
#endif // VERBOSE



		for (y = int_to_fix32(y1); y != int_to_fix32(y2); y += addY) {

#ifdef VERBOSE_DRAW_LINE
			printf("pset bottom in loop %lx, %d (isPos), %lx, %d (isPos)  round %d %d\n", x, xIsPos, y, yIsPos, round(x, xIsPos), round(y, yIsPos));
#endif // VERBOSE
			PSET(round(x, xIsPos), round(y, yIsPos));


#ifdef VERBOSE_DRAW_LINE
			printf("add x bottom %lx + %lx = %lx\n", x, addX, x + addX);
#endif
			x += addX;

#ifdef VERBOSE_DRAW_LINE
			printf("add y bottom %lx + %lx = %lx, %lx != %lx (%d)\n", y, addY, y + addY, y + addY, int_to_fix32(y2), y + addY != int_to_fix32(y2));
#endif
		}

#ifdef VERBOSE_DRAW_LINE
		printf("pset bottom out loop %d,%d\n", x2, y2);
#endif
		PSET(x2, y2);
	}

}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
void b11XCorner(byte** data)
{
	byte x1, x2, y1, y2;

	x1 = *((*data)++);
	y1 = *((*data)++);

	PSET(x1, y1);

	for (;;) {
		x2 = *((*data)++);
		if (x2 >= 0xF0) break;


#ifdef VERBOSE
		printf("x corner line 1: %d,%d : %d,%d\n", x1, y1, x2, y2);
#endif
		b11Drawline(x1, y1, x2, y2);
		x1 = x2;
		y2 = *((*data)++);
		if (y2 >= 0xF0) break;

#ifdef VERBOSE
		printf("x corner line 2: %d,%d : %d,%d\n", x1, y1, x2, y2);
#endif
		b11Drawline(x1, y1, x2, y2);
		y1 = y2;
	}

	(*data)--;
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void b11YCorner(byte** data)
{
	byte x1, x2, y1, y2;

	x1 = *((*data)++);
	y1 = *((*data)++);

	PSET(x1, y1);

	for (;;) {
		y2 = *((*data)++);
		if (y2 >= 0xF0) break;



#ifdef VERBOSE
		printf("y corner line 1: %d,%d : %d,%d\n", x1, y1, x2, y2);
#endif

		b11Drawline(x1, y1, x2, y2);
		y1 = y2;
		x2 = *((*data)++);
		if (x2 >= 0xF0) break;

#ifdef VERBOSE
		printf("y Corner line 2: %d,%d : %d,%d\n", x1, y1, x2, y2);
#endif
		b11Drawline(x1, y1, x2, y2);
		x1 = x2;
	}

	(*data)--;
}

/**************************************************************************
** relativeDraw
**
** Draws short lines relative to last position.  (drawing action 0xF7)
**************************************************************************/
void b11RelativeDraw(byte** data)
{
	byte x1, y1, disp;
	signed char dx, dy;

	x1 = *((*data)++);
	y1 = *((*data)++);

	PSET(x1, y1);

	for (;;) {
		disp = *((*data)++);

#ifdef VERBOSE_REL_DRAW
		printf("Disp %u \n", disp);
#endif // VERBOSE

		if (disp >= 0xF0) break;
		dx = ((disp & 0xF0) >> 4) & 0x0F;
#ifdef VERBOSE_REL_DRAW
		printf("Prior dx: ((%u & 0xF0) >> 4) & 0x0f : %d\n", disp, dx);
#endif


		dy = (disp & 0x0F);

#ifdef VERBOSE_REL_DRAW
		printf("Prior dy: ( %u & 0x0f): %d\n", disp, dy);
#endif

		if (dx & 0x08)
		{
#ifdef VERBOSE_REL_DRAW
			printf("x neg -1 * (%d & 07) :  %d \n", dx, (-1) * (dx & 0x07));
#endif
			dx = (-1) * (dx & 0x07);
#ifdef VERBOSE_REL_DRAW
			printf("dy is %d\n", dy);
#endif
		}
#ifdef  VERBOSE_REL_DRAW
		else
		{
			printf("x not neg %d \n", dx & 0x08);
		}
#endif //  VERBOSE

		if (dy & 0x08)
		{
#ifdef VERBOSE_REL_DRAW
			printf("y neg -1 * (%d & 07) :  %d \n", dy, (-1) * (dy & 0x07));
#endif
			dy = (-1) * (dy & 0x07);

#ifdef VERBOSE_REL_DRAW
			printf("dy is %d\n", dy);
#endif
		}
#ifdef  VERBOSE_REL_DRAW
		else
		{
			printf("y not neg %d \n", dy & 0x08);
		}
#endif //  VERBOSE

#ifdef VERBOSE_REL_DRAW
		printf("Rel Draw  x1 %u, y1 %u dx: %d, dy: %d, x1 + dx: %u, y1 + dy %u \n", x1, y1, dx, dy, x1 + dx, y1 + dy);
#endif // VERBOSE


#ifdef VERBOSE
		printf("rel line: %d,%d : %d,%d\n", x1, y1, x1 + dx, y1 + dy);
#endif

		b11Drawline(x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}

	(*data)--;
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void b11AbsoluteLine(byte** data)
{
	byte x1, y1, x2, y2;

	x1 = *((*data)++);
	y1 = *((*data)++);

	PSET(x1, y1);

	for (;;) {
		if ((x2 = *((*data)++)) >= 0xF0)
		{
			//#ifdef VERBOSE
			//            printf("Absolute Line Break\n");
			//#endif // VERBOSE

			break;
		}
		if ((y2 = *((*data)++)) >= 0xF0)
		{
			//#ifdef VERBOSE
			//            printf("Absolute Line Break\n");
			//#endif // VERBOSE
			break;
		}
#ifdef VERBOSE
		printf("abs line: %d,%d : %d,%d\n", x1, y1, x2, y2);
#endif
		b11Drawline(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}

	(*data)--;
}


#define plotPatternPoint() \
   if (patCode & 0x20) { \
      if ((splatterMap[bitPos>>3] >> (7-(bitPos&7))) & 1) PSET(x1, y1); \
      bitPos++; \
      if (bitPos == 0xff) bitPos=0; \
   } else PSET(x1, y1)

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
void b11PlotPattern(byte x, byte y)
{
	static char circles[][15] = { /* agi circle bitmaps */
	  {0x80},
	  {0xfc},
	  {0x5f, 0xf4},
	  {0x66, 0xff, 0xf6, 0x60},
	  {0x23, 0xbf, 0xff, 0xff, 0xee, 0x20},
	  {0x31, 0xe7, 0x9e, 0xff, 0xff, 0xde, 0x79, 0xe3, 0x00},
	  {0x38, 0xf9, 0xf3, 0xef, 0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xe3, 0x80},
	  {0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e,
	   0x7e, 0x3c, 0x18}
	};

	static byte splatterMap[32] = { /* splatter brush bitmaps */
	  0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2,
	  0x82, 0x09, 0x0a, 0x22, 0x12, 0x10, 0x42, 0x14,
	  0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
	  0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04
	};

	static byte splatterStart[128] = { /* starting bit position */
	  0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	  0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	  0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
	  0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	  0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
	  0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	  0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
	  0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	  0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
	  0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	  0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
	  0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	  0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
	  0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	  0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
	};

	int circlePos = 0;
	byte x1, y1, penSize, bitPos = splatterStart[patNum];

	penSize = (patCode & 7);

	/* Don't know exactly what happens at the borders, but it can definitely
	** cause problems if it isn't right. */
	/*
	if (x<((penSize/2)+1)) x=((penSize/2)+1);
	else if (x>160-((penSize/2)+1)) x=160-((penSize/2)+1);
	if (y<penSize) y = penSize;
	else if (y>=168-penSize) y=167-penSize;
	*/
	if (x < penSize) x = penSize - 1;
	if (y < penSize) y = penSize;
	//else if (y>=168-penSize) y=167-penSize;

	//TODO: Fix Float
	//for (y1 = y - penSize; y1 <= y + penSize; y1++) {
	//    for (x1 = x - (ceil((float)penSize / 2)); x1 <= x + (floor((float)penSize / 2)); x1++) {
	//        if (patCode & 0x10) { /* Square */
	//            plotPatternPoint();
	//        }
	//        else { /* Circle */
	//            if ((circles[patCode & 7][circlePos >> 3] >> (7 - (circlePos & 7))) & 1) {
	//                plotPatternPoint();
	//            }
	//            circlePos++;
	//        }
	//    }
	//}

}


/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void b11PlotBrush(byte** data)
{
	byte x1, y1, store;

	for (;;) {
		if (patCode & 0x20) {
			if ((patNum = *((*data)++)) >= 0xF0) break;
			patNum = ((patNum >> 1) & 0x7f);
		}
		if ((x1 = *((*data)++)) >= 0xF0) break;
		if ((y1 = *((*data)++)) >= 0xF0) break;
		b11PlotPattern(x1, y1);
	}

	(*data)--;
}

/**************************************************************************
** splitPriority
**
** Purpose: To split the priority colours from the control colours. It
** determines the priority information for pixels that are overdrawn by
** control lines by the same method used in Sierras interpreter (at this
** stage). This could change later on.
**************************************************************************/
void b11SplitPriority()
{
	int x, y, dy;
	byte data;
	boolean priFound;

	//for (x = 0; x < 160; x++) {
	//    for (y = 0; y < 168; y++) {
	//        data = priority->line[y][x];
	//        if (data == 3) {
	//            priority->line[y][x] = 4;
	//            control->line[y][x] = data;
	//        }
	//        if (data < 3) {
	//            control->line[y][x] = data;
	//            dy = y + 1;
	//            priFound = FALSE;
	//            while (!priFound && (dy < 168)) {
	//                data = priority->line[dy][x];
	//                /* The following if statement is a hack to fix a problem
	//                ** in KQ1 room 1.
	//                */
	//                if (data == 3) {
	//                    priFound = TRUE;
	//                    priority->line[y][x] = 4;
	//                }
	//                else if (data > 3) {
	//                    priFound = TRUE;
	//                    priority->line[y][x] = data;
	//                }
	//                else
	//                    dy++;
	//            }
	//        }
	//    }
	//}
}

int picFNum = 0;

/**************************************************************************
** drawPic
**
** Purpose: To draw an AGI picture from the given PICTURE resource data.
**
**  pLen = length of PICTURE data
**************************************************************************/
void b11DrawPic(byte* bankedData, int pLen, boolean okToClearScreen, byte picNum)
{
	unsigned long i;
	byte action;
	boolean stillDrawing = TRUE;
	PictureFile loadedPicture;
	byte* data;
	int** zpTemp = (int**)ZP_PTR_TEMP;
	byte** zpTemp2 = (int**)ZP_PTR_TEMP_2;
	int** zpB1 = (int**)ZP_PTR_B1;
	int** zpB2 = (int**)ZP_PTR_B2;
	byte** zpCh = (byte**)ZP_PTR_CH;
	byte** zpDisp = (byte**)ZP_PTR_DISP;

	*zpTemp = &bitmapWidthPreMult[0];
	*zpTemp2 = &DIVISION_AREA[0];
	*zpB1 = (int*)FLOOD_QUEUE_START;
	*zpB2 = (int*)FLOOD_QUEUE_START;
	*zpCh = divBankMetadata;
	*zpDisp = divAddressMetadata;
	rpos = FLOOD_QUEUE_START;
	spos = FLOOD_QUEUE_START;
	rposBank = FIRST_FLOOD_BANK;
	sposBank = FIRST_FLOOD_BANK;

#ifdef TEST_DIVISION
	testDivision();
#endif

#ifdef TEST_ROUND
	testRound();
#endif

	getLoadedPicture(&loadedPicture, picNum);

#ifdef VERBOSE
	printf("Preparing To Draw %d of size %d\n", picNum, loadedPicture.size);
#endif // VERBOSE


	data = (byte*)malloc(loadedPicture.size);

	if (!data)
	{
		printf("Out of memory in picture code");
	}

	memCpyBanked(&data[0], (byte*)loadedPicture.data, loadedPicture.bank, loadedPicture.size);

	//asm("sei");

	if (okToClearScreen) b11ClearPicture();

	//asm("cli");

	//trampoline_0(&b6DisableAndWaitForVsync, IRQ_BANK);

	//asm("sei");

	patCode = 0x00;

#ifdef VERBOSE
	printf("Plotting. . .\n");
#endif // VERBOSE

	do {
		action = *(data++);
#ifdef VERBOSE
		printf("Action: %p \n", action);
#endif // VERBOSE
		switch (action) {
		case 0xFF:
			stillDrawing = 0;
			break;
		case 0xF0: picColour = *(data++);
			picDrawEnabled = TRUE;
			break;
		case 0xF1: picDrawEnabled = FALSE; break;
		case 0xF2: priColour = *(data++);
			priDrawEnabled = TRUE;
			break;
		case 0xF3: priDrawEnabled = FALSE; break;
		case 0xF4: b11YCorner(&data); break;
		case 0xF5: b11XCorner(&data); break;
		case 0xF6: b11AbsoluteLine(&data); break;
		case 0xF7: b11RelativeDraw(&data); break;
		case 0xF8: b11FloodFill(&data); break;
		case 0xF9: patCode = *(data++); break;
		case 0xFA: b11PlotBrush(&data); break;
		default: printf("Unknown picture code : %X\n", action); exit(0);
		}

		//if (picFNum == 3) {
		//   showPicture();
		//   if ((readkey() >> 8) == KEY_ESC) closedown();
		//}
#ifdef VERBOSE
		printf(" data %p pLen %d data + pLen %p stillDrawing %d \n", data, pLen, data + pLen, stillDrawing);
#endif
	} while ((data < (data + pLen)) && stillDrawing);

	b11SplitPriority();

	//asm("cli");

	//trampoline_0(&b6DisableAndWaitForVsync, IRQ_BANK);



	free(data);

	for (;;);
}

void b11InitPictures()
{
	int i;
	PictureFile loadedPicture;
	for (i = 0; i < 256; i++) {
		getLoadedPicture(&loadedPicture, i);
		loadedPicture.loaded = FALSE;
		setLoadedPicture(&loadedPicture, i);
	}
}

/***************************************************************************
** loadPictureFile
**
** Purpose: To load the picture resource with the given number.
***************************************************************************/
void b11LoadPictureFile(int picFileNum)
{
	AGIFile tempAGI;
	AGIFilePosType agiFilePosType;
	PictureFile loadedPicture;

	getLoadedPicture(&loadedPicture, picFileNum);

	getLogicDirectory(&agiFilePosType, &picdir[picFileNum]);


#ifdef VERBOSE
	printf("The address of picdir is %p\n", &picdir[picFileNum]);
	printf("The picture number is %d \n", picFileNum);
	printf("Loading Picture file %d, position %d\n", agiFilePosType.fileNum, agiFilePosType.filePos);
#endif
	loadAGIFileTrampoline(PICTURE, &agiFilePosType, &tempAGI);


	loadedPicture.size = tempAGI.totalSize;
	loadedPicture.data = tempAGI.code;
	loadedPicture.bank = tempAGI.codeBank;
	loadedPicture.loaded = TRUE;

	setLoadedPicture(&loadedPicture, picFileNum);

#ifdef VERBOSE
	printf("Loaded Picture %d, data %p, bank %d, loaded %d\n", loadedPicture.size, loadedPicture.data, loadedPicture.bank, loadedPicture.loaded);
#endif // VERBOSE
}

void b11DiscardPictureFile(int picFileNum)
{
	PictureFile loadedPicture;

	getLoadedPicture(&loadedPicture, picFileNum);

	if (loadedPicture.loaded) {
		loadedPicture.loaded = FALSE;
		banked_deallocTrampoline(loadedPicture.data, loadedPicture.bank);
	}

	setLoadedPicture(&loadedPicture, picFileNum);
}

void b11ShowPicture()
{
	//Doesn't need to do much since picture is stored straight in VRAM. Need to investigate whether we need to do this
}

#pragma code-name (pop)

void drawPicTrampoline(byte* bankedData, int pLen, boolean okToClearScreen, byte picNum)
{
	byte previousBank = RAM_BANK;
	RAM_BANK = PICTURE_CODE_BANK;

	b11DrawPic(bankedData, pLen, okToClearScreen, picNum);

	RAM_BANK = previousBank;
}

