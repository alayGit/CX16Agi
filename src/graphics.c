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
        asm("lda %w", VERA_data0);
        asm("sta %v", _assm);
        romRow = _assm;

        for (j = 7; j != 255; j--) //Overflow means the loop is done
        {

            romPixel = romRow >> j & 1;

            if (!romPixel)
            {
                romPixel = 2; //Note: We have four colors trans:0,b:1,w:2,red:4. Therefore a value of 0 (white in the ROM needs to become 2)
            }

            output  |= (romPixel << resultByteShift);
            
            resultByteShift -= 2;
            if (resultByteShift == 254) //Underflow means we are ready to start again
            {
                resultByteShift = 6;
               
                _assm = output;        
                asm("lda %v", _assm);
                asm("sta %w", VERA_data1);

                output = 0;
            }
        }
    }
}

#define AddressSel0 0 
#define AddressSel1 1

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
#define OriginalCharsetAddress 0x1f000

    int i;
    byte* veraData0, *veraData1;
    
    veraData0 = (byte*)VERA_data0;
    veraData1 = (byte*)VERA_data1;

    //byte nonSequencedCharsToGet[9] = {31, 32, 38, 39, 40};

    printf("Initializing CharSet. . .\n");

#ifdef VERBOSE_CHAR_SET_LOAD
    PRINTF("The address of new charset buffer is %p\n", buffer);
#endif // VERBOSE_CHAR_SET_LOAD

    SET_VERA_ADDRESS(OriginalCharsetAddress, AddressSel0);
    SET_VERA_ADDRESS(MapBase, AddressSel1);
    
    asm("stp");
    b6ConvertsOneBitPerPixCharToTwoBitPerPixelChars();
    asm("stp");
#ifdef VERBOSE_CHAR_SET_LOAD
    printf("returning : %p. The byte counter is %d\n.", buffer, byteCounter);
#endif // VERBOSE_CHAR_SET_LOAD

}
#pragma code-name (pop)
