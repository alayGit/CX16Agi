.ifndef  COMMAND_LOOP_INC
COMMAND_LOOP_INC = 1
LOGIC_ENTRY_PARAMETERS_OFFSET =  0

.include "global.s"

ZP_PTR_LE = $02
ZP_PTR_LF = $04
NO_ZERO_PAGE_VALUES = $4

oldZPValues: .byte $0,$0,$0,$0
codePointer: .byte $0,$0
codeBank: .byte $0
startPos: .byte $0,$0
entryPoint: .byte $0,$0
endPos:  .byte $0,$0
codeSize: .byte $0,$0
_commandLoop:
    .export _commandLoop

         SAVE_ZERO_PAGE ZP_PTR_LE,oldZPValues,NO_ZERO_PAGE_VALUES

         STA   ZP_PTR_LE
         STX   ZP_PTR_LE  + 1

         LDA   GOLDEN_RAM + PARAMETERS_WORK_AREA_GOLDEN_OFFSET + LOGIC_ENTRY_PARAMETERS_OFFSET
         LDX   GOLDEN_RAM + PARAMETERS_WORK_AREA_GOLDEN_OFFSET + LOGIC_ENTRY_PARAMETERS_OFFSET + 1
         STA   ZP_PTR_LF
         STX   ZP_PTR_LF  + 1

         GET_STRUCT_16 LOGIC_FILE_LOGIC_CODE_OFFSET, ZP_PTR_LE, startPos
         GET_STRUCT_16 LOGIC_FILE_LOGIC_CODE_OFFSET, ZP_PTR_LE, codeSize
         GET_STRUCT_16 LOGIC_ENTRY_POINT_OFFSET, ZP_PTR_LF, entryPoint

         RESTORE_ZERO_PAGE ZP_PTR_LE,oldZPValues,NO_ZERO_PAGE_VALUES

         rts
.endif