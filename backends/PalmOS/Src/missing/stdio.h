#include <PalmOS.h>
#include <VFSMgr.h>
#include <stdarg.h>
#include "globals.h"

//extern UInt16	gVolRefNum;
//extern FileRef	gLogFile;

typedef FileRef FILE;

#define stdin		0
#define stdout		(&gVars->logFile)
#define stderr		(&gVars->logFile)

#define clearerr(a)
#define fflush(a)
#define vsnprintf(a,b,c,d)	vsprintf(a,c,d)

#define	SEEK_SET			vfsOriginBeginning 
#define	SEEK_CUR			vfsOriginCurrent  
#define	SEEK_END			vfsOriginEnd

UInt16 fclose(FileRef *stream);
UInt16 feof(FileRef *stream);
Char *fgets(Char *s, UInt32 n, FileRef *stream);
FileRef *fopen(const Char *filename, const Char *type);
UInt32 fread(void *ptr, UInt32 size, UInt32 nitems, FileRef *stream);
UInt32 fwrite(const void *ptr, UInt32 size, UInt32 nitems, FileRef *stream);
Int32 fseek(FileRef *stream, Int32 offset, Int32 whence);
UInt32 ftell(FileRef *stream);
UInt16 fprintf(FileRef *stream, const Char *format, ...);
Int16 printf(const Char* formatStr, ...);
Int16 sprintf(Char* s, const Char* formatStr, ...);
Int16 vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam);
