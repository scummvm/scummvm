/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

//##############################################################################
#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "SonyClie.h"
#include "VFSMgr.h"
#include "mathlib.h"
#include "vibrate.h"

#include <stdarg.h>
//##############################################################################
//##############################################################################
int main(int argc, char **argv);
MemPtr realloc(MemPtr, UInt32);
MemPtr calloc(UInt32, UInt32);
void exit(int);

//UInt16 parseCards(Boolean forceDisplay);

Err CheckMathLib();
//Err CheckHRmode(void);
Err SonySoundLib(UInt16 *refNumP);
UInt16 StrReplace(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, const Char *fndParamStr);
Int16 vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam);
Int16 sprintf(Char* s, const Char* formatStr, ...);
Int16 printf(const Char* formatStr, ...);
//void xsprintf(const Char* formatStr, ...);
void PalmFatalError(const Char *err);

typedef FileRef FILE;
UInt16 FindFileSlot();
FILE *fopen(const char *filename, const char *type);
UInt16 fprintf(FILE *stream, const Char *format, ...);
int feof(FILE *stream);
void clearerr(FILE *stream);
UInt32 fread(void *ptr, UInt32 size, UInt32 nitems, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fclose(FILE *stream);
char *fgets(char *s, UInt32 n, FILE *stream);
int free(MemPtr ptr);
UInt32 fwrite(void *ptr, UInt32 size, UInt32 nitems, FILE *stream);
int ftell(FILE *stream);
//int getc(FILE *stream);

void *memchr(const void *s, int c, UInt32 n);
Char *StrTok(Char *str, const Char *sep);
void WinDrawWarpChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth);

Err HwrDisplayPalette(UInt8 operation, Int16 startIndex, 
			 	  			 UInt16 paletteEntries, RGBColorType *tableP)
							SYS_TRAP(sysTrapHwrDisplayPalette);
							
//#define memNewChunkFlagNonMovable    0x0200 
#define memNewChunkFlagAllowLarge    0x1000  // this is not in the sdk *g* 


SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP) 
							SYS_TRAP(sysTrapSysGetAppInfo); 

//##############################################################################
//#define DEBUG
#define	MAXPATHLEN	255
#define NONSTANDARD_SAVE // scumm.h
////////////////////////////////////////////////////////////////////////////////
/*
extern UInt16 firstCard;
extern UInt16 HRrefNum;
extern UInt16 SndRefNum;

extern FileRef logfile;
extern Boolean _flipping;
extern Boolean _vibrator;
*/

//extern UInt8 *screen_1;
//extern UInt8 *screen_2;

//extern Char **argvP;

//#define assert(a)
//#define fflush(a)

//#define OutputDebugString	PalmFatalError

//#define	SEEK_SET			vfsOriginBeginning 
//#define	SEEK_CUR			vfsOriginCurrent  
//#define	SEEK_END			vfsOriginEnd

//#define strtol(a,b,c)		StrAToI(a)

//#define abs			fabs
/*
//#define	malloc		MemPtrNew
//#define	calloc(a,b)	MemPtrNew(((UInt32)a)*((UInt32)b))
#define memcpy		MemMove
#define memmove		MemMove
//#define free		MemPtrFree
#define strcmp		StrCompare
#define strncmp		StrNCompare 
#define stricmp		StrCaselessCompare 
#define	atoi		StrAToI
#define tolower		TxtGlueLowerChar
#define strchr		StrChr
#define strcat		StrCat
//#define vsprintf	StrVPrintF
//#define sprintf		StrPrintF
#define strtok		StrTok
#define rand()		SysRandom(0)
#define strncpy		MemMove


#define stdin		0
#define stdout		&logfile
#define stderr		&logfile

#define memset(ptr,value,numBytes)		MemSet(ptr,numBytes,value)
*/
//typedef UInt32 time_t;
//UInt32 time(UInt32 *cur);

#define bool		Boolean
typedef unsigned int uint;
typedef unsigned long uint32;
// scumm protos
///////////////
struct AkosRenderer;
// akos.cpp
void akos_c1_0y_decode(AkosRenderer * ar);
void akos_generic_decode(AkosRenderer * ar);
void akos_c1_spec1(AkosRenderer * ar);
// sound.cpp
int CompDecode(unsigned char *src, unsigned char *dst);
// simon.cpp
void palette_fadeout(uint32 *pal_values,uint num);
uint fileReadItemID(FILE *in);
