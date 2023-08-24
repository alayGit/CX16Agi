; Check if global definitions are included, if not, include them
.ifndef  PICTURE_INC
PICTURE_INC = 1

.include "x16.inc"
.include "global.s"
.include "graphics.s"


.import _picColour
.import _picDrawEnabled
.import _priDrawEnabled
.import _noSound

.import _rpos
.import _spos
.import _rposBank
.import _sposBank

.import _trampoline_0
.import _b11FloodBankFull

.import popa
.import popax
.import pushax

.import picDrawEnabled
.import priDrawEnabled

.segment "CODE"

DEBUG_PIXEL_DRAW = 1

.ifdef DEBUG_PIXEL_DRAW
.import _b5DebugPixelDraw
.import _b5DebugPrePixelDraw
.import _b5CheckPixelDrawn
.import _stopAtPixel
.endif

.ifdef DEBUG_PIXEL_DRAW
.import _b5DebugFloodQueueRetrieve
.import _b5DebugFloodQueueStore
.import _pixelCounter
.import _pixelStartPrintingAt
.import _pixelStopAt
.import _queueAction
.import _pixelFreezeAt
.endif

.ifdef DEBUG_CHECK_LINE_DRAWN
.import _b5LineDrawDebug
.endif

MAX_X = 160
MAX_Y = 168

.macro PRINT_PIXEL_MESSAGE printFunc
.local @end
EQ_32_LONG_TO_LITERAL _pixelCounter, NEG_1_16, NEG_1_16, @end
LESS_THAN_32 _pixelCounter, _pixelStartPrintingAt, @end
JSRFAR printFunc, DEBUG_BANK
@end:
.endmacro

.macro DEBUG_PIXEL_DRAW var1, var2
.ifdef DEBUG_PIXEL_DRAW
lda var1
sta _logDebugVal1
lda var2
sta _logDebugVal2
PRINT_PIXEL_MESSAGE _b5DebugPixelDraw
.endif
.endmacro

.macro DEBUG_PREPIXEL_DRAW var1, var2
.local @stop
.local @end
.local @freeze
.local @checkFreeze


.ifdef DEBUG_PIXEL_DRAW
lda var1
sta _logDebugVal1
lda var2
sta _logDebugVal2

PRINT_PIXEL_MESSAGE _b5DebugPrePixelDraw

clc
lda #$1
adc _pixelCounter
sta _pixelCounter
lda #$0
adc _pixelCounter + 1
sta _pixelCounter + 1
lda #$0
adc _pixelCounter + 2
sta _pixelCounter + 2
lda #$0
adc _pixelCounter + 3
sta _pixelCounter + 3

EQ_32_LONG_TO_LITERAL _pixelStopAt, NEG_1_16, NEG_1_16, @checkFreeze
LESS_THAN_32 _pixelCounter, _pixelStopAt, @checkFreeze, @stop
@stop:
stp
nop ;There to make it clearer where we have stopped
@checkFreeze:

EQ_32_LONG_TO_LITERAL _pixelFreezeAt, NEG_1_16, NEG_1_16, @end
LESS_THAN_32 _pixelCounter, _pixelFreezeAt, @end, @freeze
@freeze:
lda var1 ;Pointless instruction to make it clearer what the points are on freeze
lda var2
bra @freeze
@end:

.endif
.endmacro

.macro DEBUG_PIXEL_DRAWN var1, var2
.ifdef DEBUG_PIXEL_DRAW
lda var1
sta _logDebugVal1
lda var2
sta _logDebugVal2
PRINT_PIXEL_MESSAGE _b5CheckPixelDrawn
.endif
.endmacro

.macro DEBUG_LINE_DRAW var1, var2, var3, var4
.ifdef DEBUG_CHECK_LINE_DRAWN
.local @shouldPrint
lda var1
sta _logDebugVal1
lda var2
sta _logDebugVal2
lda var3
sta _logDebugVal3
lda var4
sta _logDebugVal4
JSRFAR _b5LineDrawDebug, DEBUG_BANK
.endif
.endmacro


.macro DEBUG_FLOOD_QUEUE_RETRIEVE
.local @end

.ifdef DEBUG_PIXEL_DRAW


sta _logDebugVal1
EQ_32_LONG_TO_LITERAL _pixelCounter, NEG_1_16, NEG_1_16, @end
LESS_THAN_32 _pixelCounter, _pixelStartPrintingAt, @end
JSRFAR _b5DebugFloodQueueRetrieve, DEBUG_BANK
@end:
INC_32 _queueAction
lda _logDebugVal1
.endif
.endmacro

.macro DEBUG_FLOOD_QUEUE_STORE var1, var2
.local @end
.ifdef DEBUG_PIXEL_DRAW
EQ_32_LONG_TO_LITERAL _pixelCounter, NEG_1_16, NEG_1_16, @end
LESS_THAN_32 _pixelCounter, _pixelStartPrintingAt, @end
lda var1
sta _logDebugVal1
JSRFAR _b5DebugFloodQueueStore, DEBUG_BANK
@end:
INC_32 _queueAction
.endif
.endmacro


_drawWhere: .word $0
_toDraw: .byte $0
.macro PSET coX, coY
.local @endPSet
.local @start
.local @checkYBounds

lda coX
cmp #MAX_X       ; if x > 159
bcc @checkYBounds         ; then @end
lda #$1
jmp @endPSet

@checkYBounds:
lda coY
cmp #MAX_Y        ; if y > 167
bcc @start         ; then @end
lda #$2
jmp @endPSet

@start:
lda _picDrawEnabled
bne @drawPictureScreen         ; If picDrawEnabled == 0, skip to the end
jmp @endPSet

@drawPictureScreen:
lda _picColour
asl a           ; Shift left 4 times to multiply by 16
asl a  
asl a  
asl a  
ora _picColour
sta _toDraw     ; toDraw = picColour << 4 | picColour

SET_VERA_ADDRESS coX, coY

lda _toDraw
sta VERA_data0

DEBUG_PIXEL_DRAW coX, coY

@endPSet:
    ; Continue with the rest of the code

.endmacro

.macro SET_VERA_ADDRESS coX, coY
.local @start
.local @originalZPTMP
bra @start
@originalZPTMP: .word $0

@start:
lda ZP_TMP 
sta @originalZPTMP  ; Save ZP_TMP
lda ZP_TMP+1
sta @originalZPTMP+1

lda coX
clc
adc #<STARTING_BYTE
sta _drawWhere
lda #$0
adc #>STARTING_BYTE
sta _drawWhere+1

clc
lda coY
adc ZP_TMP ;  Loading from preMultiplyTable populated in C (picture.c). Add twice as each entry is two bytes
sta ZP_TMP
lda #$0
adc ZP_TMP+1 
sta ZP_TMP+1 
lda coY
adc ZP_TMP
sta ZP_TMP
lda #$0
adc ZP_TMP+1
sta ZP_TMP+1 

clc
lda (ZP_TMP)  ; y * BYTES_PER_ROW.
adc _drawWhere
sta _drawWhere
ldy #$1
lda (ZP_TMP),y
adc _drawWhere+1
sta _drawWhere+1

lda @originalZPTMP
sta ZP_TMP
lda @originalZPTMP+1
sta ZP_TMP+1

lda #$10
sta VERA_addr_bank ; Stride 1. High byte of address will always be 0


lda _drawWhere + 1
sta VERA_addr_high

lda _drawWhere
sta VERA_addr_low

.endmacro

;boolean bFloodOkToFill(byte x, byte y)
;{
;	boolean getPicResult;
;
;#ifdef VERBOSE_FLOOD
;	if (pixelCounter >= pixelStartPrintingAt)
;	{
;		printf("State: pic: %d, pri %d, color: %d\n", picDrawEnabled, priDrawEnabled, picColour);
;	}
;#endif
;
;	if (!picDrawEnabled && !priDrawEnabled) return FALSE;
;	if (picColour == PIC_DEFAULT) return FALSE;
;	if (!priDrawEnabled)
;	{
;		getPicResult = bFloodPicGetPixel(x, y);
;#ifdef VERBOSE_FLOOD
;		if (pixelCounter >= pixelStartPrintingAt)
;		{
;			printf("result %d,%d %d \n", x, y, getPicResult);
;		}
;#endif
;		return (getPicResult == PIC_DEFAULT);
;	}
;	if (priDrawEnabled && !picDrawEnabled) return (bFloodPriGetPixel(x, y) == PRI_DEFAULT);
;	return (bFloodPicGetPixel(x, y) == PIC_DEFAULT);
;}

.macro OK_TO_FILL
.local @start
.local @priDisabledPicDisabled
.local @temp
.local @colorDefault
.local @checkColorDefault
.local @checkPriDisable
.local @checkPriEnabledPicDisabled
.local @returnGetPixelResult
.local @priDisablePicEnabled
.local @priEnabledPicEnabled
.local @priEnabledPicDisabled
.local @returnZero
.local @end

lda _okFillX
cmp #MAX_X       ; if x > 159
bcc @checkYBounds         ; then @returnZero
lda #$1
jmp @returnZero

@checkYBounds:
lda _okFillY
cmp #MAX_Y        ; if y > 167
bcc @start         ; then @returnZero
lda #$2
jmp @returnZero

@start:
lda _picDrawEnabled
eor #$1
tax
lda _priDrawEnabled
eor #$1
sta @temp
txa
and @temp
beq @checkColorDefault
@priDisabledPicDisabled:
jmp @returnZero

@checkColorDefault:
lda _picColour
cmp #PIC_DEFAULT
bne @checkPriDisable

@colorDefault:
jmp @returnZero

@checkPriDisable:
lda _priDrawEnabled
bne @checkPriEnabledPicDisabled
@priDisablePicEnabled:
bra @returnGetPixelResult

@checkPriEnabledPicDisabled:
lda _picDrawEnabled
eor #$1
and _priDrawEnabled
@priEnabledPicEnabled:
beq @returnGetPixelResult

@priEnabledPicDisabled: ;ToDO
jmp @returnZero

@returnGetPixelResult:
PIC_GET_PIXEL _okFillX, _okFillY
cmp #PIC_DEFAULT
bne @returnZero
lda #$1
bra @end
@returnZero:
lda #$0
bra @end
@temp: .byte $0
@end:
.endmacro


_floatDivision:
bra @start
@numerator: .word $0 ; Even though numerator is only one byte we double it for address looked up
@denominator: .word $0 ; Even though denominator is only one byte we double it for address looked up
@originalZPCh: .word $0 ;For Division Bank Table
@originalZPDisp: .word $0 ;For Division Address Table
@previousRamBank: .byte $0
@start:
dec
dec
sta @denominator

jsr popa
dec
sta @numerator

lda RAM_BANK
sta @previousRamBank

lda ZP_PTR_CH
sta @originalZPCh
lda ZP_PTR_CH+1
sta @originalZPCh+1

lda ZP_PTR_DISP
sta @originalZPDisp
lda ZP_PTR_DISP+1
sta @originalZPDisp+1

lda @numerator
clc
adc ZP_PTR_CH
sta ZP_PTR_CH
lda #$0
adc ZP_PTR_CH + 1
sta ZP_PTR_CH + 1

lda #DIVISION_METADATA_BANK
sta RAM_BANK

lda (ZP_PTR_CH)
tax

lda @originalZPCh
sta ZP_PTR_CH

lda @originalZPCh + 1
sta ZP_PTR_CH + 1


lda @numerator
clc
asl 
sta @numerator
lda #$0 ; always zero
rol
sta @numerator+1

lda @numerator
clc
adc ZP_PTR_DISP
sta ZP_PTR_DISP
lda @numerator+1
adc ZP_PTR_DISP + 1
sta ZP_PTR_DISP + 1

lda (ZP_PTR_DISP)
sta ZP_TMP_2
ldy #$1
lda (ZP_PTR_DISP),y
sta ZP_TMP_2+1

lda @originalZPDisp
sta ZP_PTR_DISP
lda @originalZPDisp + 1
sta ZP_PTR_DISP + 1

lda @denominator
pha
clc
asl 
sta @denominator
lda #$0 ; always zero
rol
sta @denominator+1

pla
clc
adc @denominator
sta @denominator
lda #$0
adc @denominator+1
sta @denominator+1

lda @denominator
clc
adc ZP_TMP_2
sta ZP_TMP_2
lda @denominator+1
adc ZP_TMP_2+1
sta ZP_TMP_2+1

stx RAM_BANK
stz sreg + 1
ldy #$1
lda (ZP_TMP_2),y
tax
ldy #$2
lda (ZP_TMP_2),y
sta sreg
lda (ZP_TMP_2)

ldy @previousRamBank
sty RAM_BANK

rts

.segment "BANKRAM11"
_b11PSet:
sta @coY
jsr popa
sta @coX
DEBUG_PREPIXEL_DRAW @coX, @coY
PSET @coX, @coY
DEBUG_PIXEL_DRAWN @coX, @coY
rts
@coX: .byte $0
@coY: .byte $0

.segment "BANKRAMFLOOD"

PIC_DEFAULT = $F
PRI_DEFAULT = 4
.macro PIC_GET_PIXEL coX, coY
.local @end
.local @returnDefault

lda coX
cmp #MAX_X
bcs @returnDefault

lda coY
cmp #MAX_Y
bcs @returnDefault

SET_VERA_ADDRESS coX, coY
lda VERA_data0
lsr
lsr
lsr
lsr
bra @end
@returnDefault:
lda #PIC_DEFAULT
@end:
ldx #$0
.endmacro

FLOOD_QUEUE_START = $A84F
FLOOD_QUEUE_END = $BEB0

_bFloodPicGetPixel:
bra @start
@coX: .byte $0
@coY: .byte $0
@start:
sta @coY
jsr popax
sta @coX

PIC_GET_PIXEL @coX, @coY
rts

_bFloodOkToFill:
OK_TO_FILL
rts
@temp: .byte $0

.segment "CODE"
_okFillX: .byte $0
_okFillY: .byte $0
.segment "BANKRAMFLOOD"

; void FLOOD_Q_STORE(unsigned short* ZP_PTR_B1) {
;     unsigned char q = 0;
;     unsigned short floodQueueEnd = 0;
;     unsigned short RAM_BANK = _sposBank;

;     *ZP_PTR_B1 = q;
;     (*ZP_PTR_B1)++; // Increment the queue pointer

;     if (*ZP_PTR_B1 == FLOOD_QUEUE_END) {
;         *ZP_PTR_B1 = FLOOD_QUEUE_START; // Reset the queue pointer to the start

;         if (RAM_BANK == LAST_FLOOD_BANK) {
;             RAM_BANK = FIRST_FLOOD_BANK;
;             _sposBank = FIRST_FLOOD_BANK;
;         } else {
;             RAM_BANK++;
;             _sposBank++;
;         }
;     }
; }
.macro FLOOD_Q_STORE
.local @start
.local @q
.local @end
.local @incBank
sta _logDebugVal1
sta @q
DEBUG_FLOOD_QUEUE_STORE @q
bra @start
.segment "CODE"
@q: .byte $0
.segment "BANKRAMFLOOD"
@floodQueueEnd: .word $0
@start:
lda _sposBank
sta RAM_BANK

lda @q
sta (ZP_PTR_B1)

clc
lda #$1
adc ZP_PTR_B1
sta ZP_PTR_B1

lda #$0
adc ZP_PTR_B1 + 1
sta ZP_PTR_B1 + 1
NEQ_16_WORD_TO_LITERAL ZP_PTR_B1, (FLOOD_QUEUE_END + 1), @end

lda #< FLOOD_QUEUE_START
sta ZP_PTR_B1
lda #> FLOOD_QUEUE_START
sta ZP_PTR_B1 + 1

lda #LAST_FLOOD_BANK
cmp RAM_BANK
bne @incBank

lda #FIRST_FLOOD_BANK
sta RAM_BANK
sta _sposBank

ldx #> _b11FloodBankFull
lda #< _b11FloodBankFull
jsr pushax
lda #PICTURE_BANK
ldx #$0
jsr _trampoline_0

bra @end

@incBank:
inc RAM_BANK ; The next flood bank will have identical code, so we can just increment the bank
inc _sposBank
@end:
.endmacro

_bFloodQstore:
FLOOD_Q_STORE
rts

QEMPTY = $FF

; void FLOOD_Q_RETRIEVE() {
;     unsigned short RAM_BANK = _rposBank;
;     unsigned char X; // Using 'X' to represent 6502's X register

;     if (*ZP_PTR_B2 == FLOOD_QUEUE_END + 1) {
;         *ZP_PTR_B2 = FLOOD_QUEUE_START;

;         _rposBank++;
;         if (_rposBank == LAST_FLOOD_BANK + 1) {
;             _rposBank = FIRST_FLOOD_BANK;
;             return QEMPTY; // Assuming QEMPTY is a status indicating the queue is empty
;         }
;     }

;     X = *ZP_PTR_B2;
;     (*ZP_PTR_B2)++;

;     return X; // Assuming this function returns the value from the queue
; }

.macro FLOOD_Q_RETRIEVE
.local @end
.local @serve
.local @resetBank
.local @returnResult
.local @serve

lda _rposBank
sta RAM_BANK

lda _sposBank
cmp _rposBank
bne @serve

lda ZP_PTR_B1
cmp ZP_PTR_B2
bne @serve

lda ZP_PTR_B1 + 1
cmp ZP_PTR_B2 + 1
beq @returnEmpty

@serve:
lda (ZP_PTR_B2)
tay

clc
lda #$1
adc ZP_PTR_B2
sta ZP_PTR_B2

lda #$0
adc ZP_PTR_B2 + 1
sta ZP_PTR_B2 + 1

NEQ_16_WORD_TO_LITERAL ZP_PTR_B2, (FLOOD_QUEUE_END + 1), @returnResult

lda #< FLOOD_QUEUE_START
sta ZP_PTR_B2
lda #> FLOOD_QUEUE_START
sta ZP_PTR_B2 + 1

lda _rposBank
cmp #LAST_FLOOD_BANK
beq @resetBank

inc RAM_BANK
inc _rposBank
bra @returnResult

@resetBank:
lda #FIRST_FLOOD_BANK
sta _rposBank
bra @returnResult

@returnEmpty:
lda #QEMPTY
jmp @end

@returnResult:
tya
DEBUG_FLOOD_QUEUE_RETRIEVE 
@end:
ldx #$0
.endmacro

_bFloodQretrieve:
FLOOD_Q_RETRIEVE
rts
.endif