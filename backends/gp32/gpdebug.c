//////////////////////////////////////////////////////////////////////////////
// debug_emu.cpp                                                            //
//////////////////////////////////////////////////////////////////////////////
/*
	debug support for EmuDebug console v1.2+
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include "gpdebug.h"

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
#define debugBufferData			((volatile char *)debugBufferAddr + 8)
#define debugBufferBegin		REG4((int)debugBufferAddr + 0)	// read
#define debugBufferEnd			REG4((int)debugBufferAddr + 4)	// write
#define debugBufferDataSize		256
#define debugBufferSize			(8 + debugBufferDataSize)

//////////////////////////////////////////////////////////////////////////////
// Variables                                                                //
//////////////////////////////////////////////////////////////////////////////

static int debugging = 0;
static void * volatile debugBufferAddr;
char debugBuffer[debugBufferDataSize];	// instead of malloc

//////////////////////////////////////////////////////////////////////////////
// __putchar                                                                //
//////////////////////////////////////////////////////////////////////////////
void __putchar(int c)
{
	unsigned int nextEnd;
	if (!debugging) return;
	do
	{
		nextEnd = debugBufferEnd + 1;
		if (nextEnd >= debugBufferDataSize) nextEnd = 0;
	} while (nextEnd == debugBufferBegin);	// full?
	debugBufferData[debugBufferEnd] = c;
	debugBufferEnd = nextEnd;
}

//////////////////////////////////////////////////////////////////////////////
// __getchar                                                                //
//////////////////////////////////////////////////////////////////////////////
int __getchar()
{
	//if (!debugging) return -1;
	return -1;
}

//////////////////////////////////////////////////////////////////////////////
// __kbhit                                                                  //
//////////////////////////////////////////////////////////////////////////////
int __kbhit()
{
	//if (!debugging) return false;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// __gets                                                                   //
//////////////////////////////////////////////////////////////////////////////
char * __gets(char *s)
{
	char *p = s;
	if (!debugging) return 0;
	while (1)
	{
		int c = getchar();
		if (c >= 0) *p++ = c;
		if (c == 0) return s;
	}
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// __puts                                                                   //
//////////////////////////////////////////////////////////////////////////////
int __puts(const char *s)
{
	if (!debugging) return 0;
	while (*s) putchar(*s++);
	return 0;
/*
	while (debugBufferAddr[0]) {}	// wait until buffer is clear
	int r = sprintf(debugBufferAddr+1, "%s", s);
	debugBufferAddr[0] = r;
	return r;
*/
}

//////////////////////////////////////////////////////////////////////////////
// __printf                                                                 //
//////////////////////////////////////////////////////////////////////////////
int __printf(char *fmt, ...)
{
	char s[256];
	int r;
	va_list marker;
	
	if (!debugging) return 0;
	va_start(marker, fmt);
	r = vsprintf(s, fmt, marker);
	va_end(marker);
	puts(s);
	return r;
}

//////////////////////////////////////////////////////////////////////////////
// InitDebug                                                                //
//////////////////////////////////////////////////////////////////////////////
void InitDebug()
{
	debugBufferAddr = debugBuffer;
	//debugBufferAddr = malloc(debugBufferSize);
	debugBufferBegin = debugBufferEnd = 0xEDEBEDEB;
	memset((void *)debugBufferData, ' ', debugBufferDataSize);
	debugBufferData[0] = '{'; debugBufferData[debugBufferDataSize - 1] = '}';
	while (debugBufferBegin && debugBufferEnd) { }		// wait for debugger
	debugging = 1;
}
