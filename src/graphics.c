#include "graphics.h"
#pragma code-name (push, "BANKRAM06")

//#define VERBOSE_CHAR_SET_LOAD

#ifdef VERBOSE_CHAR_SET_LOAD
byte printOn = TRUE;
int byteCounter = 0;
#endif
void b6ConvertsOneBitPerPixCharToTwoBitPerPixelChars()
{
    int i;
    byte j; //Must be int because it needs to be unsigned
    byte romPixel, output = 0, romRow;
    byte resultByteShift = 6;

    for (i = 0; i < NO_CHARS * SIZE_PER_CHAR_CHAR_SET_ROM; i++)
    {
        READ_BYTE_VAR_FROM_ASSM(romRow, VERA_data0);

        for (j = 7; j != 255; j--) //Overflow means the loop is done
        {

            romPixel = romRow >> j & 1;
           
            if (i >= TRANSPARENT_CHAR_BYTE && i <= LAST_BYTE_TRANSPARENT_CHAR)
            {
                romPixel = 0;
            }
            else if (!romPixel)
            {
                romPixel = 2; //Note: We have four colors trans:0,b:1,w:2,red:4. Therefore a value of 0 (white in the ROM needs to become 2)
            }

            output  |= (romPixel << resultByteShift);
            
            resultByteShift -= 2;
            if (resultByteShift == 254) //Underflow means we are ready to start again
            {
                resultByteShift = 6;
               
                WRITE_BYTE_VAR_TO_ASSM(output, VERA_data1);
 
                output = 0;
            }
        }
    }
    asm("stp");
}

#define SET_VERA_ADDRESS(VeraAddress, AddressSel) \
    do {                                           \
        asm("lda #%w", AddressSel);                  \
        asm("sta %w", VERA_ctrl);                  \
        asm("lda #$10");                           \
        asm("ora #^%l", VeraAddress);           \
        asm("sta %w", VERA_addr_bank);             \
        asm("lda #< %l", VeraAddress);          \
        asm("sta %w", VERA_addr_low);             \
        asm("lda #> %l", VeraAddress);          \
        asm("sta %w", VERA_addr_high);              \
    } while(0)

void b6InitCharset()
{
#define ORIGINAL_CHARSET_ADDRESS 0x1f000

    int i;
    byte* veraData0, *veraData1;
    
    veraData0 = (byte*)VERA_data0;
    veraData1 = (byte*)VERA_data1;

    //byte nonSequencedCharsToGet[9] = {31, 32, 38, 39, 40};

    printf("Initializing CharSet. . .\n");

#ifdef VERBOSE_CHAR_SET_LOAD
    PRINTF("The address of new charset buffer is %p\n", buffer);
#endif // VERBOSE_CHAR_SET_LOAD

    SET_VERA_ADDRESS(ORIGINAL_CHARSET_ADDRESS, ADDRESSSEL0);
    SET_VERA_ADDRESS(TILEBASE, ADDRESSSEL1);
    
    b6ConvertsOneBitPerPixCharToTwoBitPerPixelChars();

#ifdef VERBOSE_CHAR_SET_LOAD
    printf("returning : %p. The byte counter is %d\n.", buffer, byteCounter);
#endif // VERBOSE_CHAR_SET_LOAD

}

void b6InitLayer1Mapbase()
{
    int i;
#define BYTE1 TRANSPARENT_CHAR
#define BYTE2 8 //1 Offset 0 v flip 0 h flip 0 tile index bit 8 and 9

    SET_VERA_ADDRESS(MAPBASE, ADDRESSSEL0);

    for (i = 0; i < TILE_LAYER_NO_TILES; i++)
    {
        WRITE_BYTE_DEF_TO_ASSM(BYTE1, VERA_data0);
        WRITE_BYTE_DEF_TO_ASSM(BYTE2, VERA_data0);
    }

}

#pragma code-name (pop)
