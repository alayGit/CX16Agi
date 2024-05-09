/**************************************************************************
** AGIFILES.H
**************************************************************************/

#ifndef _AGIFILES_H_
#define _AGIFILES_H_

#include "memoryManager.h"
#include "helpers.h"
#include "general.h"
#include <stdint.h>

#ifdef  __CX16__
#include <cx16.h>
#include "helpers.h"
#endif

#define  LOGIC    0
#define  PICTURE  1
#define  VIEW     2
#define  SOUND    3

#define FILE_OPEN_ADDRESS 2
#define NO_BYTES_PER_MESSAGE 2
#define FILE_NAME_SIZE 10

#define  EMPTY  0xFFFFF   /* Empty DIR entry */

typedef struct {          /* AGI data file structure */
   unsigned int totalSize;
   unsigned int codeSize;
   byte* code;
   byte codeBank;
   byte messageBank;
   unsigned int noMessages;
   byte** messagePointers;
   byte* messageData;
} AGIFile;

extern AGIFilePosType* logdir;
extern AGIFilePosType* picdir; 
extern AGIFilePosType* viewdir;
extern AGIFilePosType* snddir;

extern int numLogics, numPictures, numViews, numSounds;

#pragma wrapped-call (push, trampoline, FILE_LOADER_HELPERS)
extern void b6InitFiles();
extern void b6LoadAGIDirs();
extern void b6LoadAGIFile(int resType, AGIFilePosType* location, AGIFile *AGIData);
extern byte b6Cbm_openForSeeking(char* fileName);
extern int8_t b6Cx16_fseek(uint8_t channel, uint32_t offset);
#pragma wrapped-call (pop)

#endif  /* _AGIFILES_H_ */
