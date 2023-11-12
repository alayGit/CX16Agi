.ifndef SPRITE_MEMORY_MANAGER_INC
SPRITE_MEMORY_MANAGER_INC = 1

.include "globalGraphics.s"

.importzp sreg

.segment "BANKRAM0E"
SPRITE_ALLOC_TABLE_SIZE = (SPRITE_END - SPRITE_START) / SEGMENT_SMALL
RESULT_SIZE = 20

_bESpriteAllocTable: .res SPRITE_ALLOC_TABLE_SIZE, $0

_bESpriteAddressTableMiddle: .res SPRITE_ALLOC_TABLE_SIZE, $0 ; Low will always be zero, hence no need for a table

;low a middle x high y
.macro ALLOCATE_SPRITE_MEMORY_32
.local @loop
.local @found
.local @prepareResult
.local @increaseWall
.local @greater
.local @lesser
.local @nonEmpty
.local @return
.local @returnFail

ldx #$0 ;Indicates never reset to zero
ldy ZP_PTR_SEG_32
lda ZP_PTR_WALL_32

@loop:
cpy ZP_PTR_WALL_64
beq @resetAtZero
lda _bESpriteAllocTable, y
bne @nonEmpty

@found:
lda ZP_PTR_WALL_32
cmp ZP_PTR_SEG_32 ;If the segment is equal to the wall then that mean we have never reached the end before, or extra space has since become available, so we should just increase it
bne @prepareResult

@increaseWall:
inc ZP_PTR_WALL_32

@prepareResult:
lda #$1
sta _bESpriteAllocTable, y

ldx _bESpriteAddressTableMiddle,y

tya 
iny 
sty ZP_PTR_SEG_32

cmp ZP_PTR_HIGH_BYTE_START
bcs @greater

@lesser:
ldy #$0
bra @return

@greater:
ldy #$1
bra @return


@nonEmpty:
iny 
cpy ZP_PTR_SEG_32
beq @returnFail

bra @loop

@resetAtZero:
cpx #$0
bne @returnFail ;We have also reset to zero before if this branch is followed
ldx #$1

ldy #$0
bra @loop

@returnFail:
sty ZP_PTR_SEG_32
ldy #0
ldx #0

@return:
lda #$0 ;Low byte is always zero
.endmacro

;low a middle x high y
.macro ALLOCATE_SPRITE_MEMORY_64
.local @loop
.local @checkResultTable
.local @found
.local @prepareResult
.local @increaseWall
.local @greater
.local @lesser
.local @nonEmpty
.local @return
.local @returnFail
.local @resetToEnd
ldx #$0 ;Indicates never reset to zero
ldy ZP_PTR_SEG_64

@loop:
cpy ZP_PTR_WALL_32
beq @resetToEnd

bcs @checkResultTable ;As 64 allocator is going in twos, we could jump over
dey ;We have jumped over go back two bytes and reset to end
dey
bra @resetToEnd

@checkResultTable:
lda _bESpriteAllocTable, y
bne @nonEmpty

@found:
lda ZP_PTR_WALL_64
cmp ZP_PTR_SEG_64 ;If the segment is equal to the wall then that mean we have never reached the end before, or extra space has since become available, so we should just increase it
bne @prepareResult

@increaseWall:
inc ZP_PTR_WALL_64

@prepareResult:

lda #$1
sta _bESpriteAllocTable, y

ldx _bESpriteAddressTableMiddle,y

tya 
dey
dey
sty ZP_PTR_SEG_64

cmp ZP_PTR_HIGH_BYTE_START
bcs @greater

@lesser:
ldy #$0
bra @return

@greater:
ldy #$1
bra @return


@nonEmpty:
dey
dey
cpy ZP_PTR_SEG_64
beq @returnFail

bra @loop

@resetToEnd:
cpx #$0
bne @returnFail ;We have also reset to end before if this branch is followed
ldx #$1

ldy #SPRITE_ALLOC_TABLE_SIZE - 2
bra @loop

@returnFail:
stp
ldy #SPRITE_ALLOC_TABLE_SIZE - 2
sty ZP_PTR_SEG_64
ldy #0
ldx #0

@return:
lda #$0 ;Low byte is always zero
.endmacro





_bEAllocateSpriteMemory32:
ALLOCATE_SPRITE_MEMORY_32
sty sreg
stz sreg + 1
rts

_bEAllocateSpriteMemory64:
ALLOCATE_SPRITE_MEMORY_64
sty sreg
stz sreg + 1
rts

.endif