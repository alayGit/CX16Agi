    .importzp sreg
    .include "init.s"
    .include "commandLoop.s"
    .include "logicCommands.s"
    .include "codeWindow.s"
    .include "global.s"
    .include "graphicsAsm.s"
    .include "helpersAsm.s"
    .include "splitView.s"
    .include "spriteMemoryManagerAsm.s"
    .include "wordsAsm.s"
    .include "debug.s"
    .include "fillAsm.s"
    .include "floatDivisionAsm.s"
    .include "priority.s"
    .include "movement.s"
    .include "randomAsm.s"
    .include "modulus.s"
    .export _executeLogic
    .export _b6InitInterpreter
    .export _loadAndIncWinCode
    .export _incCodeBy
    .export codeWindow
    .export _codeBank
    .export _logDebugVal1
    .export _logDebugVal2
    .export _logDebugVal3
    .export _logDebugVal4
    .export _logDebugVal5
    .export _logDebugVal6
    .export _floatDivision
    .export _b6SetAndWaitForIrqStateAsm
    .export _displayTextAddressToCopyTo
    .export _vSyncCounter
    .export _lastBoxLines
    .export _lastBoxStartLine
    .export _textBuffer1
    .export _textBuffer2
    .export _currentTextBuffer
    .export _trampoline
    .export _viewHeaderBuffer
    .export _loopHeaderBuffer
    .export _b9CelToVera
    .export _b6InitGraphics
    .export _b6InitIrq
    .export _b6TellMeTheAddressPlease
    .export _bESpritesUpdatedBuffer
    .export _bESpriteAddressTableMiddle
    .export _spriteAllocTable
    .export _bEAllocateSpriteMemory32
    .export _bEAllocateSpriteMemory64
    .export _bEAllocateSpriteMemoryBulk
    .export _bEBulkAllocatedAddresses
    .export _bECellToVeraBulk
    .export _bEToBlitCelArray
    .export _bESpritesUpdatedBufferPointer
    .export _bEClearSpriteAttributes
    .export _bESplitCel
    .export _b3PaletteAddress
    .export _b3PaletteRows
    .export _b3PaletteNumber
    .export _b3InitLayer1Mapbase
    .export _b4ClearPicture
    .export _b12FindSynonymNumSearch
    .export _b6Clear
    .export _b5IsDebuggingEnabled
    .export _b8DrawPixel
    .export _trampolineDebug
    .export _b8AsmPlotVisHLineFast
    .export _b8AsmFloodFill
    .export _b8AsmFloodFill
    .export _b8AsmFloodFillSections
    .export _b8AsmFloodFillSectionsVisOnly
    .export _b8GetVeraPictureAddress
    .export _b8GetControl
    .export _celToVera
    .export _bECelToVeraBackwards
    .export _bEClearVeraSprite
    .export _bECalculateBytesPerRow
    .export _bASpriteAddressReverseHighNotSet
    .export _bASpriteAddressReverseHighSet
    .export _runSpriteGarbageCollector
    .export _runIncrementalGarbageCollector
    .export _bARunSpriteGarbageCollectorAll
    .export _SGC_LAST_LOCATION_GC_CHECKED
    .export _bAFollowEgoAsmSec
    .export _rand8Bit
    .export _randBetween