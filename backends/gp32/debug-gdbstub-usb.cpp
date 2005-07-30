/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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

/*
 *
 * Basic GP32 USB GDB Debug Stub - Use with Multi-firmware that supports GDB
 * Mithris kindly gave me the basic stub code.
 *
 */
#include <sys/types.h>
#include <gpcomm.h>
#include <gpos_def.h>
#include <gpdef.h>
#include <gpmain.h>
#include <gpos_def.h>
#include <gpstdio.h>
#include <gpgraphic.h>
#include <gpfont.h>
#include <gpstdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//#define USE_PRINTF // If there is a Printf(const char *pFormat,...) implemented

///////////////////////////////////////////////////////////////
// Externs
extern int __text_start;
extern int __data_start;
extern int __bss_start;
#ifdef USE_PRINTF
extern void Printf(char *pFormat, ...);
#endif
///////////////////////////////////////////////////////////////

extern "C" {
	int OpenUSB();
	void InstallISR();
}

struct g_Namedregisters {
	unsigned int	r0;
	unsigned int	r1;
	unsigned int	r2;
	unsigned int	r3;
	unsigned int	r4;
	unsigned int	r5;
	unsigned int	r6;
	unsigned int	r7;
	unsigned int	r8;
	unsigned int	r9;
	unsigned int	r10;
	unsigned int	r11;
	unsigned int	r12;
	unsigned int	sp;
	unsigned int	lr;
	unsigned int	pc;
	unsigned int	fps;
	unsigned int	fcpsr;
};

///////////////////////////////////////////////////////////////
// Defines


#define _REG_R1		0
#define _REG_R2		2
#define _REG_R3		3
#define _REG_R4		4
#define _REG_R5		5
#define _REG_R6		6
#define _REG_R7		7
#define _REG_R8		8
#define _REG_R9		9
#define _REG_R10	10
#define _REG_R11	11
#define _REG_R12	12
#define _REG_FP		11
#define _REG_IP		12
#define _REG_SL		10
#define _REG_SP		13
#define _REG_LR		14
#define _REG_PC		15

#define PACKET_SIZE 0x100

#define MODE_USER		0
#define MODE_FIQ		1
#define MODE_IRQ		2
#define MODE_SUPERVISOR	3
#define MODE_ABORT		4
#define MODE_UNDEF		5
#define MODE_SYSTEM		6
#define MODE_DUNNO		-1

///////////////////////////////////////////////////////////////
// Global stuff

// Register array.
int g_Registers[50] = {1, 2, 3, 4 ,5, 6, 7, 8, 9, 10,
					11, 12, 13, 14, 15, 16, 17, 18, 19,
					21, 22, 23, 24, 25, 26, 27, 28, 29,
					31, 32, 33, 34, 35, 36, 37, 38, 39,
					41, 42, 43, 44, 45, 46, 47, 48, 49};

// Register name strings, not used right now.
static char * arm_register_name_strings[] =
{"r0",  "r1",  "r2",  "r3",     /*  0  1  2  3 */
 "r4",  "r5",  "r6",  "r7",     /*  4  5  6  7 */
 "r8",  "r9",  "r10", "r11",    /*  8  9 10 11 */
 "r12", "sp",  "lr",  "pc",     /* 12 13 14 15 */
 "f0",  "f1",  "f2",  "f3",     /* 16 17 18 19 */
 "f4",  "f5",  "f6",  "f7",     /* 20 21 22 23 */
 "fps", "cpsr" };               /* 24 25       */


// Some USB stuff
GPN_DESC		g_CommDesc;
GPN_COMM		g_Comm;
const char 		HexDigits[17] = "0123456789abcdef";
char			g_SendBuffer[256];
char			g_TempBuffer[256];
char			g_ReadBuffer[0x100];
unsigned int	g_CurrentCSPR = 0;
unsigned int	g_SavedStepInstruction = 0;
unsigned int	g_StepAddress = 0;
bool			g_LastWasStep = false;
int				g_iTrap = 0;
bool			g_GDBConnected = false;
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// Prototypes
void BreakPoint();
int GetException(unsigned int CSPR);
unsigned int *GetNextInstruction(unsigned int *pAddr);
bool CondWillExecute(unsigned int uiCond, unsigned int CSPR);
unsigned int DecodeInstruction(unsigned int uiInstruction, unsigned int PC);

///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// Code Begins here
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Open usb comms stuff
int OpenUSB()
{
	int	iResult;

	g_CommDesc.port_kind = COMM_USB_D;
	g_CommDesc.tr_mode = 1;
	g_CommDesc.tr_buf_size = PACKET_SIZE;
	g_CommDesc.sz_pkt = PACKET_SIZE;
	g_CommDesc.isr_comm_ram = 0;


	iResult = GpCommCreate(&g_CommDesc, &g_Comm);
	return iResult;
}

///////////////////////////////////////////////////////////////
// No need for explanation
void CloseUSB()
{
	GpCommDelete(&g_CommDesc);
}

///////////////////////////////////////////////////////////////
// No need for explanation
int SendUSB(const void *pData, unsigned int uiSize)
{
	return g_Comm.comm_send((unsigned char *)pData, uiSize);
}


///////////////////////////////////////////////////////////////
// No need for explanation
int RecvUSB(char *pBuffer, unsigned int uiSize)
{
		return g_Comm.comm_recv((unsigned char *)pBuffer, uiSize);
}

///////////////////////////////////////////////////////////////
// Waits for an acknowledge from the server
void WaitACK()
{
	bool bBreak = false;
	int iResult;

	while(!bBreak) {
		iResult = g_Comm.comm_recv((unsigned char *)g_SendBuffer, 0x100);
		if (iResult > 0) {
			bBreak = true;
		}
	}
}
///////////////////////////////////////////////////////////////
// Formats and sends a command to the server,
// it also calculates checksum
void SendCommand(unsigned char *pCommand)
{
	int		iOffset;
	unsigned int		iCheckSum;

	iOffset = 4;
	g_SendBuffer[iOffset++] = '$';

	iCheckSum = 0;
	while(*pCommand != 0) {
		g_SendBuffer[iOffset++] = *pCommand;
		iCheckSum += *pCommand++;
	}

	g_SendBuffer[iOffset++] = '#';
	iCheckSum = iCheckSum & 0xff;
	g_SendBuffer[iOffset++] = HexDigits[(iCheckSum & 0xf0) >> 4];
	g_SendBuffer[iOffset++] = HexDigits[(iCheckSum & 0x0f)];
	g_SendBuffer[iOffset] = 0;

	(*(int *)&g_SendBuffer[0]) = (iOffset - 4);
	SendUSB(g_SendBuffer, 0x100);
	WaitACK();
}

///////////////////////////////////////////////////////////////
// This function creates and sends a package which tells gdb that
// the last command was unsupported
void UnSupportedCommand()
{
	(*(int *)&g_SendBuffer[0]) = 4;
	g_SendBuffer[4] = '$';
	g_SendBuffer[5] = '#';
	g_SendBuffer[6] = '0';
	g_SendBuffer[7] = '0';
	g_SendBuffer[8] = '0';
	SendUSB(g_SendBuffer, 0x100);

	WaitACK();

}


///////////////////////////////////////////////////////////////
// This function is quite similar to the SendCommand above
// But it allows the user to set the package length.
// If the length of a package exceeds 256bytes including
// the protocol stuff and the package length you can split
// the packages by sending the package size * -1.
// The server will then merge all packages until a package
// with a length >0 is recieved.
void SendCommandSize(unsigned char *pCommand, int iSize)
{
	int		iOffset;
	unsigned int		iCheckSum;

	iOffset = 4;
	g_SendBuffer[iOffset++] = '$';

	iCheckSum = 0;
	while(*pCommand != 0) {
		g_SendBuffer[iOffset++] = *pCommand;
		iCheckSum += *pCommand++;
	}

	g_SendBuffer[iOffset++] = '#';
	iCheckSum = iCheckSum & 0xff;
	g_SendBuffer[iOffset++] = HexDigits[(iCheckSum & 0xf0) >> 4];
	g_SendBuffer[iOffset++] = HexDigits[(iCheckSum & 0x0f)];
	g_SendBuffer[iOffset] = 0;

	(*(int *)&g_SendBuffer[0]) = iSize;
	SendUSB(g_SendBuffer, 0x100);

	WaitACK();
}


///////////////////////////////////////////////////////////////
// Writes a 32bit hexadeciman number in ascii to the string.
void HexToString(char *pString, int iNumber)
{
	pString[0] = HexDigits[(iNumber >> 28) & 0x0f];
	pString[1] = HexDigits[(iNumber >> 24) & 0x0f];
	pString[2] = HexDigits[(iNumber >> 20) & 0x0f];
	pString[3] = HexDigits[(iNumber >> 16) & 0x0f];
	pString[4] = HexDigits[(iNumber >> 12) & 0x0f];
	pString[5] = HexDigits[(iNumber >> 8) & 0x0f];
	pString[6] = HexDigits[(iNumber >> 4) & 0x0f];
	pString[7] = HexDigits[(iNumber) & 0x0f];

}

///////////////////////////////////////////////////////////////
// This does the same, but in a differend endian.
void HexToStringBW(char *pString, int iNumber)
{
	pString[6] = HexDigits[(iNumber >> 28) & 0x0f];
	pString[7] = HexDigits[(iNumber >> 24) & 0x0f];
	pString[4] = HexDigits[(iNumber >> 20) & 0x0f];
	pString[5] = HexDigits[(iNumber >> 16) & 0x0f];
	pString[2] = HexDigits[(iNumber >> 12) & 0x0f];
	pString[3] = HexDigits[(iNumber >> 8) & 0x0f];
	pString[0] = HexDigits[(iNumber >> 4) & 0x0f];
	pString[1] = HexDigits[(iNumber) & 0x0f];

	pString[8] = 0;

}


///////////////////////////////////////////////////////////////
// equiv to strcat i imagine
void PrintToString(char *pString, char *pOtherString)
{
	while(*pOtherString != 0) {
		*pString = *pOtherString;
		*pString++;
		*pOtherString++;
	}
}

///////////////////////////////////////////////////////////////
// converts "ff" -> 0xff
unsigned char StringToByte(char *pString)
{
	unsigned char	ucValue = 0;

	for (int i = 0; i < 2; i++) {
		ucValue  = ucValue << 4;
		switch(pString[i]) {
			case '0':
				break;
			case '1':
				ucValue |= (0x01);
				break;
			case '2':
				ucValue |= (0x02);
				break;
			case '3':
				ucValue |= (0x03);
				break;
			case '4':
				ucValue |= (0x04);
				break;
			case '5':
				ucValue |= (0x05);
				break;
			case '6':
				ucValue |= (0x06);
				break;
			case '7':
				ucValue |= (0x07);
				break;
			case '8':
				ucValue |= (0x08);
				break;
			case '9':
				ucValue |= (0x09);
				break;
			case 'a':
				ucValue |= (0x0a);
				break;
			case 'b':
				ucValue |= (0x0b);
				break;
			case 'c':
				ucValue |= (0x0c);
				break;
			case 'd':
				ucValue |= (0x0d);
				break;
			case 'e':
				ucValue |= (0x0e);
				break;
			case 'f':
				ucValue |= (0x0f);
				break;

		}
	}

	return ucValue;
}

///////////////////////////////////////////////////////////////
// Sends a package with the program offsets to GDB
// Of some reason all offsets should be NULL.
void SendOffsets()
{
	char    String[255];
	int a = 0;//(int)&__text_start;
	int b = 0;//(int)&__data_start;
	int c = 0;//(int)&__bss_start;

	PrintToString(String, "Text=");
	HexToString(&String[5], a);
	PrintToString(&String[5+8], ";Data=");
	HexToString(&String[5+8+6], b);
	PrintToString(&String[5+8+6+8], ";Bss=");
	HexToString(&String[5+8+6+8+5], c);
	String[5+8+6+8+5+8] = 0;

	SendCommand((unsigned char *)String);
}

///////////////////////////////////////////////////////////////
// This function dumps all registers to GDB
// It utilizes the SendCommandSize function to split the package.
void DumpRegisters()
{
	char	Buffer[0x100];
	// Ugly hack so far.

	for (int i = 0; i < 21; i++) {
		//g_Registers[i] = i;
		HexToStringBW(&Buffer[i * 8], g_Registers[i]);
	}
	Buffer[21*8] = 0;
	SendCommandSize((unsigned char *)Buffer, -1 * (21 * 8 + 4));
	for (int i = 0; i < 21; i++) {
		//g_Registers[20 + i] = i;
		HexToStringBW(&Buffer[i * 8], g_Registers[21 + i]);
	}
	Buffer[21*8] = 0;

	SendCommandSize((unsigned char *)Buffer, (21 * 8) + 4);
}

///////////////////////////////////////////////////////////////
// This function extracts an address from a string.
void *GetAddr(char *pBuffer)
{
	int iAddr;
	int i = 0;
	iAddr = 0;

	while (pBuffer[i] != ',') {
		iAddr = iAddr << 4;
		switch(pBuffer[i]) {
		case '0':
			//iAddr |=
			break;
		case '1':
			iAddr |= (0x01);
			break;
		case '2':
			iAddr |= (0x02);
			break;
		case '3':
			iAddr |= (0x03);
			break;
		case '4':
			iAddr |= (0x04);
			break;
		case '5':
			iAddr |= (0x05);
			break;
		case '6':
			iAddr |= (0x06);
			break;
		case '7':
			iAddr |= (0x07);
			break;
		case '8':
			iAddr |= (0x08);
			break;
		case '9':
			iAddr |= (0x09);
			break;
		case 'a':
			iAddr |= (0x0a);
			break;
		case 'b':
			iAddr |= (0x0b);
			break;
		case 'c':
			iAddr |= (0x0c);
			break;
		case 'd':
			iAddr |= (0x0d);
			break;
		case 'e':
			iAddr |= (0x0e);
			break;
		case 'f':
			iAddr |= (0x0f);
			break;

		}
		i++;
	}
	return (void *)iAddr;
}
///////////////////////////////////////////////////////////////
// This function does pretty much the same, but returns an int
// I know, some redundant code.
int	GetBytes(char *pBuffer)
{
	int iBytes = 0;
	int i = 0;


	while ((pBuffer[i] != '#') && (pBuffer[i] != ':')) {
		iBytes = iBytes << 4;
		switch(pBuffer[i]) {
		case '0':
			//iAddr |=
			break;
		case '1':
			iBytes |= 0x01;
			break;
		case '2':
			iBytes |= 0x02;
			break;
		case '3':
			iBytes |= 0x03;
			break;
		case '4':
			iBytes |= 0x04;
			break;
		case '5':
			iBytes |= 0x05;
			break;
		case '6':
			iBytes |= 0x06;
			break;
		case '7':
			iBytes |= 0x07;
			break;
		case '8':
			iBytes |= 0x08;
			break;
		case '9':
			iBytes |= 0x09;
			break;
		case 'a':
			iBytes |= 0x0a;
			break;
		case 'b':
			iBytes |= 0x0b;
			break;
		case 'c':
			iBytes |= 0x0c;
			break;
		case 'd':
			iBytes |= 0x0d;
			break;
		case 'e':
			iBytes |= 0x0e;
			break;
		case 'f':
			iBytes |= 0x0f;
			break;

		}

		i++;
	}
	return iBytes;
}

///////////////////////////////////////////////////////////////
// This function reads memory and sends it back to GDB.
void SendMemory(void *pAddr, int iBytes)
{
	unsigned char	*pData;
	unsigned char 	iData;
	int				iBufferPos = 0;
	int				iBytesToSend;
	char			Byte;
	int				i;

	pData = (unsigned char *)pAddr;


	do {
		if (iBytes > 100) {
			iBytesToSend = 100;
			iBytes -= 100;
		} else {
			iBytesToSend = iBytes;
			iBytes = 0;
		}
		iBufferPos = 0;
		for (i = 0; i < iBytesToSend; i+=1) {
			iData = *pData++;
			g_TempBuffer[iBufferPos++] = HexDigits[(iData & 0xf0) >> 4];
			g_TempBuffer[iBufferPos++] = HexDigits[(iData & 0x0f)];

		}
		if (iBytes > 0) {
			// This mean that we have not yet sent our last command.
			g_TempBuffer[iBufferPos] = 0;
			SendCommandSize((unsigned char *)g_TempBuffer, -1 * (i + 1 + 4));
		}
	} while (iBytes > 0 );
	g_TempBuffer[iBufferPos] = 0;
	SendCommand((unsigned char *)g_TempBuffer);
}



///////////////////////////////////////////////////////////////
// Does pretty much what it says.
void WriteMemory(void *pAddr, unsigned int uiBytes, char *pHexString)
{
	unsigned char	*pData = ((unsigned char *)pAddr);
	unsigned int	uiOffset = 0;
	unsigned char	ucByte;

	//Printf("0x%x 0x%x", pAddr, uiBytes);
	for (unsigned int i = 0; i < uiBytes ; i++) {
		ucByte = StringToByte(&pHexString[uiOffset]);
		//Printf("0x%x", ucByte);
		*pData++ = ucByte;
		uiOffset += 2;
	}
	/*
	while(1);
	unsigned int *piData = (unsigned int *)pAddr;
	*piData = 0xe7ffdefe;//0xfedeffe7;
	*/

}


///////////////////////////////////////////////////////////////
// Sends the required information about a trap
// TODO: correct numbers need to be placed there.
void SendBreakPoint()
{
	int iOffset = 0;

	g_TempBuffer[iOffset++] = 'T';
	g_TempBuffer[iOffset++] = '0';
	g_TempBuffer[iOffset++] = '5';
	g_TempBuffer[iOffset++] = '0';
	g_TempBuffer[iOffset++] = 'd';
	g_TempBuffer[iOffset++] = ':';
	HexToStringBW(&g_TempBuffer[iOffset], g_Registers[_REG_SP]);
	iOffset += 8;
	g_TempBuffer[iOffset++] = ';';
	g_TempBuffer[iOffset++] = '0';
	g_TempBuffer[iOffset++] = 'b';
	g_TempBuffer[iOffset++] = ':';
	HexToStringBW(&g_TempBuffer[iOffset], g_Registers[_REG_FP]);
	iOffset += 8;
	g_TempBuffer[iOffset++] = ';';
	g_TempBuffer[iOffset++] = '0';
	g_TempBuffer[iOffset++] = 'f';
	g_TempBuffer[iOffset++] = ':';
	HexToStringBW(&g_TempBuffer[iOffset], g_Registers[_REG_PC]);
	iOffset += 8;
	g_TempBuffer[iOffset++] = ';';
	g_TempBuffer[iOffset] = 0;

	SendCommand((unsigned char *)g_TempBuffer);




}


///////////////////////////////////////////////////////////////
// Finds a character in a string and returns the offset.
int FindChar(char *pBuffer, char sign)
{
	int iRetVal = 0;

	while(*pBuffer != sign) {
		iRetVal++;
		*pBuffer++;
	}
	return iRetVal;
}


// Attibute naked.
///////////////////////////////////////////////////////////////
// This is the ISR(Interrupt Service Routine) which handles
// All traps, it's basically a context switcher.
void ISR() __attribute__ ((naked));
void ISR()
{
	// Lets snatch the registers!

	asm volatile(" \n"
		" str		r4, [%0, #0x10] \n"
		" str		r5, [%0, #0x14] \n"
		" str		r6, [%0, #0x18] \n"
		" str		r7, [%0, #0x1C] \n"
		" str		r8, [%0, #0x20] \n"
		" str		r9, [%0, #0x24] \n"
		" str		r10, [%0, #0x28] \n"
		" str		r11, [%0, #0x2C] \n"
		" str		r12, [%0, #0x30] \n"

		" str		r14, [%0, #0x3C] \n"

		" @// skip 8 * 12byte(96bit) = 0x60 \n"

		" mov		r4, %0 \n"
		" ldmia	sp!, {r0, r1, r2, r3} \n"
		" str		r0, [r4, #0x00] \n"
		" str		r1, [r4, #0x04] \n"
		" str		r2, [r4, #0x08] \n"
		" str		r3, [r4,#0x0C] \n"

		" mrs		r1, SPSR \n"
		" str		r1, [r4, #0x48] \n"
		" str		r1, [r4,#0xA0] \n"
		" str		r1, [r4,#0xA4] \n"

		" mrs		r1, CPSR \n"
		" mov		r2, r1 \n"


		" @// Let us set the mode to supervisor so we can get r13 and r14 \n"
		" bic		r1, r1, #0x1f \n"
		" orr		r1, r1, #0x13 \n"
		" msr		CPSR_c, r1 \n"
		" @// Just remember that we're in the supervisor stack aswell now \n"
		" str		r13, [r4,#0x34] \n"
		" str		r14, [r4,#0x38] \n"
		" @// Lets move back to whatever mode we was in. \n"
		" @// Make sure that IRQ's are turned on \n"
		" bic		r2, r2, #0x80 \n"
		" msr		CPSR_fsxc, r2 \n"




		" \n"
		:
		:  "r" (g_Registers)
		: "%0", "r1", "r2", "r4");


	// Get Current CSPR and save LR
	asm volatile(" \n"
		" mrs		r0, CPSR \n"
		" str		r0, [%0] \n"

		" str		r14, [%1, #0x40] \n"
		" str		r0, [%1, #0x44] \n"
		" str		r13, [%1, #0x4C] \n"
		" \n"
		:
		: "r" (&g_CurrentCSPR), "r" (g_Registers)
		: "r0");


	switch(g_CurrentCSPR & 0x1f) {
		case 0x10: // USER
			g_iTrap = 0;
			break;
		case 0x11: // FIQ
			g_iTrap = 1;
			break;
		case 0x12: // IRQ
			g_iTrap = 2;
			break;
		case 0x13: // Supervisor
			g_iTrap = 3;
			break;
		case 0x17: // Abort
			g_iTrap = 4;
			break;
		case 0x1B: // Undefined/Breakpoint
			// We cannot continue like this!
			g_Registers[15] -= 4;
			g_Registers[16] -= 4;
			g_iTrap = 5;
			break;
		case 0x1F: // System
			g_iTrap = 6;
			break;
		default:
			g_iTrap = -1;
	}


	#ifdef USE_PRINTF
	Printf("Trap@0x%x:%d", g_Registers[15], g_iTrap);
	#endif
	/*
	switch (g_iTrap) {
		case MODE_USER:
			break;
		case MODE_FIQ:
			break;
		case MODE_IRQ:
			break;
		case MODE_SUPERVISOR:
			break;
		case MODE_ABORT:
			break;
		case MODE_UNDEF:
			break;
		case MODE_SYSTEM:
			break;
		case MODE_DUNNO:
		default:
			while(1);
	}
	*/
	SendBreakPoint();
	BreakPoint();

		//Printf("0x%x 0x%x", g_Registers[15], g_Registers[16]);
	// Okay, it's time to continue.


	switch (g_iTrap) {
		case MODE_USER:
			//Printf("Dunno!!\n");
			break;
		case MODE_FIQ:
			//Printf("Dunno!!\n");
			break;
		case MODE_IRQ:
			//Printf("Dunno!!\n");
			break;
		case MODE_SUPERVISOR:
			//Printf("Dunno!!\n");
			break;
		case MODE_ABORT:
			asm volatile(" \n"
				" mov r10, #0 \n"
				" @Invalidate I&D cache \n"
				" mcr	p15, 0, r10, c7, c7 \n"
				" @ restore the registers \n"
				" ldr	r1,[%0, #0x04] \n"
				" ldr	r2,[%0, #0x08] \n"
				" ldr	r4,[%0, #0x10] \n"
				" ldr	r5,[%0, #0x14] \n"
				" ldr	r6,[%0, #0x18] \n"
				" ldr	r7,[%0, #0x1C] \n"
				" ldr	r8,[%0, #0x20] \n"
				" ldr	r9,[%0, #0x24] \n"
				" ldr	r10,[%0, #0x28] \n"
				" ldr	r11,[%0, #0x2C] \n"
				" ldr	r12,[%0, #0x30] \n"
				" ldr	r14,[%0, #0x40] \n"
				" ldr	r0, [%0, #0x44] \n"
				" msr	CPSR_fsxc, r0 \n"
				" ldr	r0, [%0, #0x48] \n"
				" msr	SPSR_fsxc, r0 \n"

				" ldr	r0,[%0, #0x00] \n"
				" ldr	r3,[%0, #0x0C] \n"

				" subs	pc, lr, #0x04 \n"

				" \n"
				:
				:"r" (g_Registers)
				:"r0");
			break;
		case MODE_UNDEF:
			// This will be the breakpoint i'm gonna test with.
			asm volatile(" \n"
				" mov r10, #0 \n"
				" @Invalidate I&D cache \n"
				" mcr	p15, 0, r10, c7, c7 \n"
				" @ restore the registers \n"
				" ldr	r1,[%0, #0x04] \n"
				" ldr	r2,[%0, #0x08] \n"
				" ldr	r4,[%0, #0x10] \n"
				" ldr	r5,[%0, #0x14] \n"
				" ldr	r6,[%0, #0x18] \n"
				" ldr	r7,[%0, #0x1C] \n"
				" ldr	r8,[%0, #0x20] \n"
				" ldr	r9,[%0, #0x24] \n"
				" ldr	r10,[%0, #0x28] \n"
				" ldr	r11,[%0, #0x2C] \n"
				" ldr	r12,[%0, #0x30] \n"
				" ldr	r14,[%0, #0x40] \n"
				" ldr	r0, [%0, #0x44] \n"
				" msr	CPSR_fsxc, r0 \n"
				" ldr	r0, [%0, #0x48] \n"
				" msr	SPSR_fsxc, r0 \n"

				" ldr	r0,[%0, #0x00] \n"
				" ldr	r3,[%0, #0x0C] \n"
				" @ The subbing has already been done! \n"
				" @subs	pc, lr, #0x04 \n"
				" movs	pc, lr \n"
				" \n"
				:
				:"r" (g_Registers)
				:"r0");


			break;
		case MODE_SYSTEM:
			//Printf("Dunno!!\n");
			break;
		case MODE_DUNNO:
		default:
			//Printf("Dunno!!\n");
			while(1);
	}




}

///////////////////////////////////////////////////////////////
// Returns which exception occured based on CSPR
int GetException(unsigned int CSPR)
{
	switch(CSPR & 0x1f) {
		case 0x10: // USER
			return 0;
			break;
		case 0x11: // FIQ
			return 1;
			break;
		case 0x12: // IRQ
			return 2;
			break;
		case 0x13: // Supervisor
			return 3;
			break;
		case 0x17: // Abort
			return 4;
			break;
		case 0x1B: // Undefined/Breakpoint
			return 5;
			break;
		case 0x1F: // System
			return 6;
			break;
		default:
			return -1;
	}
}


///////////////////////////////////////////////////////////////
// Installs the ISR address into the system RAM
void InstallISR()
{
	int		*pPointer = (int *)(0x0c7ac040);
	int		*pPointer2 = (int *)(0x0c7ac018);

	void (*pISR)();

	pISR = ISR;

	*pPointer = (int)pISR;
	*pPointer2 = (int)pISR;

}

void DEBUG_Print(char *pFormat, ...)
{
	char            Temp[0x100];
	va_list         VaList;
	int				iLength;
	int				iOffset;
	unsigned char	MyChar;

	if (!g_GDBConnected) return;

	va_start(VaList , pFormat);
	vsnprintf(Temp, 0x100, pFormat , VaList);
	va_end(VaList);

	iLength = strlen(Temp);
	if (iLength > 100) iLength = 100;

	g_TempBuffer[0] = 'O';
	iOffset = 1;
	for (int i = 0; i < iLength; i++) {
		MyChar = (unsigned char)Temp[i];
		g_TempBuffer[iOffset++] = HexDigits[(MyChar & 0xf0) >> 4];
		g_TempBuffer[iOffset++] = HexDigits[(MyChar & 0x0f)];
	}
	g_TempBuffer[iOffset] = 0;
	SendCommand((unsigned char *)g_TempBuffer);

}


///////////////////////////////////////////////////////////////
// The main thread when the GDB thread has control
void BreakPoint()
{
	unsigned int *pNextInstruction;
	bool	bBreakLoop = false;
	int		iResult;
	int		iMessageLength;
	int		iOffsetAdd;
	int 	iNullVal = 0;
	void	*pAddr;
	int		iOffset;
	int		iBytes;


	// Find out if we got here through a STEP command
	if (g_LastWasStep) {
		#ifdef USE_PRINTF
		Printf("I:0x%x 0x%x", *((unsigned int *)(g_Registers[15] + 4)), *((unsigned int *)(g_Registers[15])));
		Printf("S: 0x%x", g_StepAddress);
		#endif
		if ((unsigned int)g_Registers[15] == g_StepAddress) {
			// Yes it was, Lets restore the instruction.
			*((unsigned int *)g_StepAddress) = g_SavedStepInstruction;
			#ifdef USE_PRINTF
			Printf("Restore: 0x%x", g_SavedStepInstruction);
			#endif
		} else {
			while(1);
		}
		g_LastWasStep = false;
	}


	while(!bBreakLoop) {
		iResult = RecvUSB(g_ReadBuffer, 0x100);
		//Printf("%d\n", iResult);

		if (iResult > 0) {
			// If we recieve a package we can assume that GDB is connected.. or smth..:D
			g_GDBConnected = true;
			// Well, we have recieved a package, lets print the contents.
			iMessageLength = *(int *)&g_ReadBuffer[0];
			g_ReadBuffer[4 + iMessageLength] = 0;
			//Printf("%s\n %d", &g_ReadBuffer[4], iMessageLength);

			// Let us also send an ACK '+'
			(*(int *)&g_SendBuffer[0]) = 1;
			g_SendBuffer[4] = '+';
			SendUSB((const void *)g_SendBuffer, 0x100);
			WaitACK();

			// I can see that i get a bunch of '+' and '-' in the messages.. lets remove them.
			iOffsetAdd = 4;
			while((g_ReadBuffer[iOffsetAdd] == '+') || (g_ReadBuffer[iOffsetAdd] == '-')) iOffsetAdd++;

			// Check whether it's legimit command
			if (g_ReadBuffer[iOffsetAdd] == '$') {
				// Well it is!
				switch(g_ReadBuffer[iOffsetAdd + 1]) {
					case 'H': // Set thread, we're not having any threads.. so.. just return OK
						SendCommand((unsigned char *)"OK");
						break;
					case 'q': // Query, there are some queries, but GDB first asks for Offsets
						switch(g_ReadBuffer[iOffsetAdd + 2]) {
							case 'O': // Offsets
								SendOffsets();
							break;
							case 'C':
								SendCommand((unsigned char *)"QC0000");
								//SendBreakPoint();
								break;

						}
						break;
					case '?':
						// This will have to be modified later to send the correct signal.
						SendBreakPoint();
						break;
					case 'g':
						DumpRegisters();
						break;
					case 'm':
							pAddr = GetAddr(&g_ReadBuffer[iOffsetAdd + 2]);
							iOffset = FindChar(&g_ReadBuffer[iOffsetAdd + 2], ',');
							iBytes = GetBytes(&g_ReadBuffer[iOffsetAdd + 2 + iOffset]);
							SendMemory(pAddr, iBytes);
							break;
					case 'X':	//Write memory binary, which we DON't support.. ofcourse.
						UnSupportedCommand();
						break;
					case 'P':	// Write register
						{
							SendCommand((unsigned char *)"OK");

						}
						break;
					case 'M':	// Write memory not binary
						{
							pAddr = GetAddr(&g_ReadBuffer[iOffsetAdd + 2]);
							iOffset = FindChar(&g_ReadBuffer[iOffsetAdd + 2], ',');
							iBytes = GetBytes(&g_ReadBuffer[iOffsetAdd + 2 + iOffset]);
							iOffset = FindChar(&g_ReadBuffer[iOffsetAdd + 2], ':');
							WriteMemory(pAddr, iBytes, &g_ReadBuffer[iOffsetAdd + 2 + iOffset + 1]);
							SendCommand((unsigned char *)"OK");

						}
						break;
					case 'c':	// continue
						{
							return;

						}
						break;

					case 's': // Stepping.
						{
							// Get the address of the next instruction.
							pNextInstruction = GetNextInstruction((unsigned int *)g_Registers[15]);

							// Read whatsever there.
							g_SavedStepInstruction = *pNextInstruction;
							g_StepAddress = (unsigned int)pNextInstruction;
							g_LastWasStep = true;

							//Printf("Curr: 0x%x", g_Registers[15]);
							#ifdef USE_PRINTF
							Printf("Next: 0x%x->0x%x", g_Registers[15], pNextInstruction);
							#endif
							//Printf("Trap: 0x%x", GetException((unsigned int)g_Registers[40]));
							// Write a breakpoint instruction to the address.
							*pNextInstruction = 0xe7ffdefe;
							return;

						}
						break;
					case 'Z':	// BreakPoint.
						{
							switch(g_ReadBuffer[iOffsetAdd + 2]) {
								case '0':
									// Software breakpoint, i think it's up to me to add, it, lets send OK for now.
									UnSupportedCommand();
									break;
								default:
									// We only support software breakpoints for now, lets return unsupported.
									// Actually we don't even support SW breakpoints now
									UnSupportedCommand();
									break;
							}
						}
						break;

					default:
						UnSupportedCommand();
						break;

				}
			}

		}

	}

}

///////////////////////////////////////////////////////////////
// Tries to find the next instruction to be executed after a break
unsigned int *GetNextInstruction(unsigned int *pAddr)
{
	unsigned int	uiInstruction	= *pAddr;
	unsigned int	uiAndVal		= 0;
	unsigned int	iNewPC			= 0;
	int				iNewAddr		= 0;
	int				iRegsbeforePC	= 0;
	unsigned int	uiBaseRegister	= 0;
	unsigned int	uiRegVal		= 0;
	unsigned int	uiData			= 0;
	unsigned int	uiCondMask		= 0;
	int				iPCOffset		= 0;

	unsigned int uiNewPC = DecodeInstruction(uiInstruction, (unsigned int)pAddr);
	return (unsigned int *)uiNewPC;


	// Set new PC to pAddr + 4, because we really hope that is the case...:D
	iNewPC = (unsigned int)pAddr;
	iNewPC += 4; // Next instruction (atleast in ARM mode, we don't support thumb yet)

	// Now it's a good point to find out if the instruction would be executed anyway.
	uiCondMask = (uiInstruction & 0xf0000000) >> 28;

	if (CondWillExecute(uiCondMask, (unsigned int)g_Registers[18])) {
		//Printf("Condition will execute");
		// Find out if it's a B or BL instruction. (This is the easy one)
		if ((uiInstruction & 0xE000000 ) ==  0xA000000) {
			#ifdef USE_PRINTF
			Printf("0x%x", uiInstruction);
			#endif
			// Okay, it's a branch instruction, lets get the address it's for
			iNewAddr = uiInstruction & 0xffffff;
			// We might need to sign extend this instruction.
			if ((iNewAddr & 0x00800000) != 0) {
				#ifdef USE_PRINTF
				printf("Sign extending");
				#endif
				//iNewAddr *= -1;
				iNewAddr |= 0xff000000;
			}
			#ifdef USE_PRINTF
			Printf("0x%x", iNewAddr);
			#endif
			iNewAddr *= 4; // Instruction size.
			iNewPC = ((int)pAddr + iNewAddr + 8);
		}

		// Well, it might be a ldm(ea)?

		if ((uiInstruction & 0xE000000) == 0x8000000) {
			#ifdef USE_PRINTF
			Printf("LDM");
			#endif
			// this is a LDM/STM alright.
			if ((uiInstruction & 0x100000) != 0) {
				// This is a LDM instruction
				// Lets see if the PC is ever loaded.
				#ifdef USE_PRINTF
				Printf("includes PC");
				#endif
				if ((uiInstruction & 0x8000) != 0) {
					// Well (damn the PC is loaded)
					for (int i = 0; i < 15; i++) {
						uiAndVal = 1 << i;
						if ((uiInstruction & uiAndVal) != 0) iRegsbeforePC++;
					}
					#ifdef USE_PRINTF
					Printf("%d regs", iRegsbeforePC);
					#endif
					/*
					<mr_spiv> da = fa
					<mr_spiv> ia = fd
					<mr_spiv> db = ea
					<mr_spiv> ib = ed
					*/
					// Lets find out which register is used as base for this operation.
					uiBaseRegister = (uiInstruction & 0xF0000) >> 16;
					uiRegVal = ((unsigned int *)g_Registers)[uiBaseRegister];
					// First, have a look at the U bit.
					if ((uiInstruction & (1 << 23)) != 0) {

						// Transfer is made descending
						// Which also means that the PC is closest to the base register i just found out.
						// Lets check the P bit (If i'm supposed to increment before or after.

						iPCOffset = iRegsbeforePC * 4;
						if (((uiInstruction) & (1 << 24)) != 0) iPCOffset += 4;

					} else {
						// Transfer is done ascending
						// Lets check the P bit (If i'm supposed to decrement before or after.
						if (((uiInstruction) & (1 << 24)) != 0) iPCOffset = -4;
					}


					iNewPC = *(unsigned int *)((((int)uiRegVal) + iPCOffset) & ~0x03);
				}
			}
		}

		// Check if it's a mov pc, Rn

	}

	return (unsigned int *)iNewPC;
}

///////////////////////////////////////////////////////////////
// Determines if uiCond will be true with this CSPR
bool CondWillExecute(unsigned int uiCond, unsigned int CSPR)
{

	switch(uiCond) {
		case 0: // EQ
			// This is true if Z is set in CSPR
			if ((CSPR & (1 << 30)) != 0) return true;
			else return false;

			break;
		case 1: // NE
			// This should be true if Z is not set.
			if ((CSPR & (1 << 30)) == 0) return true;
			else return false;
			break;
		case 2:	// CS/HS
			//  this one should be true if C is set.
			if ((CSPR & (1 << 29)) != 0) return true;
			else return false;
			break;
		case 3:	// CC/LO
			//  this one should be true if C is clear.
			if ((CSPR & (1 << 29)) == 0) return true;
			else return false;
			break;
		case 4: // MI
			//  this one should be true if N is set
			if ((CSPR & (1 << 31)) != 0) return true;
			else return false;
			break;
		case 5: // PL
			//  this one should be true if N is clear.
			if ((CSPR & (1 << 31)) == 0) return true;
			else return false;
			break;
		case 6: // VS
			//  this one should be true if V is set
			if ((CSPR & (1 << 28)) != 0) return true;
			else return false;
			break;
		case 7:	// VC
			//  this one should be true if V is clear.
			if ((CSPR & (1 << 28)) == 0) return true;
			else return false;
			break;
		case 8: // HI
			// This is true if C and Z is clear
			if (((CSPR & (1 << 30)) == 0) && ((CSPR & (1 << 29)) == 0)) return true;
			else return false;
			break;
		case 9:	// LS
			// C clear OR Z set
			if (((CSPR & (1 << 29)) == 0) || ((CSPR & (1 << 30)) != 0)) return true;
			else return false;
			break;
		case 10: // GE
			// N set AND V set || N clear and V clear
			if ((CSPR & (1 << 31)) == (CSPR & (1 << 28))) return true;
			else return false;
			break;
		case 11: // LT
			// N != V
			if ((CSPR & (1 << 31)) != (CSPR & (1 << 28))) return true;
			else return false;
			break;
		case 12: // GT
			// Z == 0, N == V
			if (((CSPR & (1 << 30)) == 0) && ((CSPR & (1 << 31)) == (CSPR & (1 << 28)))) return true;
			else return false;
			break;
		case 13: // LE
			if (((CSPR & (1 << 30)) == 1) && ((CSPR & (1 << 31)) != (CSPR & (1 << 28)))) return true;
			else return false;
			break;
		case 14: // AL
			return true;
			break;
		default:
			break;
	}


}
// I got the idea for this layout from the singlestep.c (found in eCos)
// But i thought the code was a bit tricky to port, and i wanna learn more about this anyway so, i'll just do smth similar to that



typedef struct
{
	unsigned Rm		: 4; // Rm
	unsigned resv2	: 1; // Reserved 2 (0)
	unsigned shift	: 2; // hmm.. dunno actually but probably (LSL, LSR, ASR, ROR )
	unsigned amount	: 5; // Shift amount 0-31
	unsigned Rd		: 4; // Rd
	unsigned Rn		: 4; // Rn
	unsigned s		: 1; //  S-flag
	unsigned opcode	: 4; // Opcode (Mov etc)
	unsigned resv1	: 3; // Reserved 1 (000)
	unsigned cond	: 4; // Condition

} dpisr; // Data Processing Immediate Register Shift
#define DPISR_R1 0
#define DPISR_R2 0
// Example <opcode> Rd, Rm, <shift> amount
typedef struct
{
	unsigned Rm		: 4; // Rm
	unsigned resv3	: 1; // Reserved 3 (1)
	unsigned shift	: 2; // (LSL, LSR, ASR, ROR )
	unsigned resv2	: 1; // Reserved 2 (0)
	unsigned Rs		: 4; // Rs
	unsigned Rd		: 4; // Rd
	unsigned Rn		: 4; // Rn
	unsigned s		: 1; // S-flag
	unsigned opcode	: 4; // Opcode
	unsigned resv1	: 3; // Reserved 1 (000)
	unsigned cond	: 4; // Condition
} dprrs; // Data Processing Register Register Shift
#define DPRRS_R1	0
#define DPRRS_R2	0
#define DPRRS_R3	1
// Example <opcode> Rd, Rn, Rm <shift> Rs
// This intruction is unpredictable if R15 is one of the used registers anyway.
typedef struct
{
	unsigned immed	: 8; // Immediate value
	unsigned rotate	: 4; // rotate
	unsigned Rd		: 4; // Rd
	unsigned Rn		: 4; // Rn
	unsigned s		: 1; // S-flag
	unsigned opcode	: 4; // Opcode
	unsigned resv1	: 3; // Reserved 1 (001)
	unsigned cond	: 4; // Condition
} dpi; // Data processing immediate
// example add r0, r1, (ror <immed>, <rotate * 2>)
#define DPI_R1	1

typedef struct
{
	unsigned immed	: 12; // Immediate
	unsigned Rd		: 4; // Rd
	unsigned Rn		: 4; // Rn
	unsigned L		: 1; // L-bit (Load/Store)?
	unsigned W		: 1; // W-bit
	unsigned B		: 1; // B-bit
	unsigned U		: 1; // U-bit
	unsigned p		: 1; // P-bit
	unsigned resv1	: 3; // Reserved 1 (010)
	unsigned cond	: 4; // Condition
} lsio; // Load/store immediate offset
// Example ldr Rd, [Rn, #<immed>]
#define LSIO_R1	2


typedef struct
{
	unsigned Rm		: 4; // Rm
	unsigned resv2	: 1; // Reserved 2 (0)
	unsigned shift	: 2; // Shit type (LSL, LSR, ASR, ROR )
	unsigned amount	: 5; // Shift amount (0-31)
	unsigned Rd		: 4; // Rd
	unsigned Rn		: 4; // Rn
	unsigned L		: 1; // L-bit (Load/Store)?
	unsigned W		: 1; // W-bit
	unsigned B		: 1; // B-bit
	unsigned U		: 1; // U-bit
	unsigned p		: 1; // P-bit
	unsigned resv1	: 3; // Reserved 1 (011)
	unsigned cond	: 4; // Condition
} lsro; // Load/Store register offset
// Example ldr Rd, [Rn + Rm lsl 5]
#define LSRO_R1 3
#define LSRO_R2 0


typedef struct
{
	unsigned regs	: 16; // Register mask
	unsigned Rn		: 4; // Rn
	unsigned L		: 1; // L-bit (Load/Store)?
	unsigned W		: 1; // W-bit
	unsigned S		: 1; // B-bit
	unsigned U		: 1; // U-bit
	unsigned p		: 1; // P-bit

	unsigned resv1	: 3; // Reserved 1 (100)
	unsigned cond	: 4; // Condition
} lsm; // Load store multiple
// Example: ldm r0, {r1, r2, r3}
#define LSM_R1  4

typedef struct
{
	unsigned offset	: 24; // Branch offset
	unsigned link	: 1; // Link flag
	unsigned resv1	: 3; // Reserved 1 (101)
	unsigned cond	: 4; // Condition
} bl; // Branch with link(optional)
#define BL_R1 5

typedef union {
	dpisr	DPISR;
	dprrs	DPRRS;
	dpi		DPI;
	lsio	LSIO;
	lsro	LSRO;
	lsm		LSM;
	bl		BL;
	unsigned int uiInstruction;
} Instruction;

/*
#define DPISR_R1 0
#define DPISR_R2 0


#define DPRRS_R1	0
#define DPRRS_R2	0
#define DPRRS_R3	1

#define DPI_R1	1

#define LSIO_R1	2

#define LSRO_R1 3
#define LSRO_R2 0

#define LSM_R1  4

#define BL_R1 5
*/

/*
 * Data Processiong Opcode field values
 */
#define OPCODE_MOV  0xD
#define OPCODE_MVN  0xF
#define OPCODE_ADD  0x4
#define OPCODE_ADC  0x5
#define OPCODE_SUB  0x2
#define OPCODE_SBC  0x6
#define OPCODE_RSB  0x3
#define OPCODE_RSC  0x7
#define OPCODE_AND  0x0
#define OPCODE_EOR  0x1
#define OPCODE_ORR  0xC
#define OPCODE_BIC  0xE
#define OPCODE_CMP  0xA
#define OPCODE_CMN  0xB
#define OPCODE_TST  0x8
#define OPCODE_TEQ  0x9

/*
 * Shift field values
 */
#define SHIFT_LSL   0x0
#define SHIFT_LSR   0x1
#define SHIFT_ASR   0x2
#define SHIFT_ROR   0x3
#define SHIFT_RRX   0x3    /* Special case: ROR(0) implies RRX */


unsigned int DecodeDPISR(dpisr Instr, unsigned int PC);
unsigned int DecodeDPRRS(dprrs Instr, unsigned int PC);
unsigned int DecodeDPI(dpi Instr,  unsigned int PC);
unsigned int DecodeLSIO(lsio Instr,  unsigned int PC);
unsigned int DecodeLSRO(lsro Instr,  unsigned int PC);
unsigned int DecodeLSM(lsm Instr,  unsigned int PC);
unsigned int DecodeBL(bl Instr,  unsigned int PC);


///////////////////////////////////////////////////////////////
//
unsigned int DecodeInstruction(unsigned int uiInstruction, unsigned int PC)
{
	Instruction myInstruction;
	unsigned int uiCondMask;

	uiCondMask = (uiInstruction & 0xf0000000) >> 28;

	// This instruction can do whatever it wants, but if it doesn't execute we don't give a shit.
	if (!CondWillExecute(uiCondMask, (unsigned int)g_Registers[18])) return PC + 4;
	//Printf("CondWillExec");

	myInstruction.uiInstruction = uiInstruction;

	// Start decoding.. phuu

	if ((myInstruction.DPISR.resv1 == DPISR_R1) && (myInstruction.DPISR.resv2 == DPISR_R2)) return DecodeDPISR(myInstruction.DPISR, PC);
	else if ((myInstruction.DPRRS.resv1 == DPRRS_R1) &&
			 (myInstruction.DPRRS.resv2 == DPRRS_R2) &&
			 (myInstruction.DPRRS.resv3 == DPRRS_R3)) return DecodeDPRRS(myInstruction.DPRRS, PC);
	else if ((myInstruction.DPI.resv1 == DPI_R1)) return DecodeDPI(myInstruction.DPI, PC);
	else if ((myInstruction.LSIO.resv1 == LSIO_R1)) return DecodeLSIO(myInstruction.LSIO, PC);

	else if ((myInstruction.LSRO.resv1 == LSRO_R1) &&
			 (myInstruction.LSRO.resv2 == LSRO_R2)) return DecodeLSRO(myInstruction.LSRO, PC);
	else if (myInstruction.LSM.resv1 == LSM_R1) return DecodeLSM(myInstruction.LSM, PC);
	else if (myInstruction.BL.resv1 == BL_R1) return  DecodeBL(myInstruction.BL, PC);

	return 0;
}

///////////////////////////////////////////////////////////////
//
unsigned int LSL(unsigned int uiValue, unsigned int uiSteps)
{
	return uiValue << uiSteps;
}

///////////////////////////////////////////////////////////////
//
unsigned int LSR(unsigned int uiValue, unsigned int uiSteps)
{
	return uiValue >> uiSteps;
}

///////////////////////////////////////////////////////////////
//
// This one could be trickier since, i'm nor sure if a signed shift really is a signed shift.
unsigned int ASR(unsigned int uiValue, unsigned int uiSteps)
{
	unsigned int	uiSignMask = 0;

	// Check if it's a negative number
	if (uiValue & 0x80000000) {
		// Yes, damn
		uiSignMask = ((~0) << (32 - uiSteps));
	}

	return ((uiValue >> uiSteps) | uiSignMask);
}

///////////////////////////////////////////////////////////////
//
unsigned int ROR(unsigned int uiValue, unsigned int uiSteps)
{
	unsigned int uiRetval;

	while(uiSteps-- > 0) {
		if (uiValue & 0x01) {
			uiValue = (uiValue >> 1) | 0x80000000;
		} else {
			uiValue = uiValue >> 1;
		}
	}
	return uiValue;
}


///////////////////////////////////////////////////////////////
//
unsigned int Shift_Operand(unsigned int Rm, unsigned int amount, unsigned int shift)
{
	unsigned int	uiRegisterValue;




	uiRegisterValue = g_Registers[Rm];
	if (Rm == 0x0f) {
		// Rm is PC, and PC is offseted by 8.
		uiRegisterValue += 8;
	}

	// Determine the shift mode.
	//(LSL, LSR, ASR, ROR )
	switch (shift) {
		case 0: // LSL
			return LSL(uiRegisterValue, amount);
			break;
		case 1: // LSR
			return LSR(uiRegisterValue, amount);
			break;
		case 2: // ASR
			return ASR(uiRegisterValue, amount);
			break;
		case 3: // ROR
			return ROR(uiRegisterValue, amount);
			break;

		default:
			break;

	}
	return 0;
}

///////////////////////////////////////////////////////////////
//
// Example <opcode> Rd, Rm, <shift> amount
unsigned int DecodeDPISR(dpisr Instr, unsigned int uiPC)
{
	unsigned int uiOperand = Shift_Operand(Instr.Rm, Instr.amount, Instr.shift);
	unsigned int uiRnVal = g_Registers[Instr.Rn];



	// Only do this i Pc is Rd
	if (Instr.Rd != 0x0f) return uiPC + 4;

	// The actual value that PC contains when executing this instruction is the instruction address+8
	if (Instr.Rn == 0x0f) uiRnVal += 8;

	// Check what opcode it is!
	switch (Instr.opcode) {
		case OPCODE_MOV:
			return uiOperand;
		case OPCODE_MVN:
			return ~uiOperand;
		case OPCODE_ADD:
			return uiRnVal + uiOperand;
		case OPCODE_ADC:
			return uiRnVal + uiOperand + (((g_Registers[18] & (1 << 29))) == 0?0:1);
		case OPCODE_SUB:
			return uiRnVal - uiOperand;
		case OPCODE_SBC:
			return uiRnVal - uiOperand - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_RSB:
			return uiOperand - uiRnVal;
		case OPCODE_RSC:
			return uiOperand - uiRnVal - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_AND:
			return (uiRnVal & uiOperand);
		case OPCODE_EOR:
			return (uiRnVal^uiOperand);
		case OPCODE_ORR:
			return (uiRnVal | uiOperand);
		case OPCODE_BIC:
			return (uiRnVal & ~uiOperand);
		default:
			return 0;
	}
}


///////////////////////////////////////////////////////////////
//
//dprrs; // Data Processing Register Register Shift
// Example <opcode> Rd, Rn, Rm <shift> Rs
unsigned int DecodeDPRRS(dprrs Instr, unsigned int uiPC)
{
	unsigned int uiRmValue = g_Registers[Instr.Rm];
	unsigned int uiRsValue = g_Registers[Instr.Rs];
	unsigned int uiRnVal = g_Registers[Instr.Rn];
	if ((Instr.Rm = 0x0f)) uiRmValue += 8;


	unsigned int uiOperand = Shift_Operand(uiRmValue, uiRsValue, Instr.shift);

	// Check if destination is PC
	if (Instr.Rd != 0x0f) return uiPC + 4;
	if ((Instr.Rn = 0x0f)) uiRnVal += 8;

	// Check what opcode it is!
	switch (Instr.opcode) {
		case OPCODE_MOV:
			return uiOperand;
		case OPCODE_MVN:
			return ~uiOperand;
		case OPCODE_ADD:
			return uiRnVal + uiOperand;
		case OPCODE_ADC:
			return uiRnVal + uiOperand + (((g_Registers[18] & (1 << 29))) == 0?0:1);
		case OPCODE_SUB:
			return uiRnVal - uiOperand;
		case OPCODE_SBC:
			return uiRnVal - uiOperand - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_RSB:
			return uiOperand - uiRnVal;
		case OPCODE_RSC:
			return uiOperand - uiRnVal - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_AND:
			return (uiRnVal & uiOperand);
		case OPCODE_EOR:
			return (uiRnVal^uiOperand);
		case OPCODE_ORR:
			return (uiRnVal | uiOperand);
		case OPCODE_BIC:
			return (uiRnVal & ~uiOperand);
		default:
			return 0;
	}

}

///////////////////////////////////////////////////////////////
//
// dpi; // Data processing immediate
// example add r0, r1, (ror <immed>, <rotate * 2>)
unsigned int DecodeDPI(dpi Instr, unsigned int uiPC)
{
	unsigned int uiOperand = (ROR(Instr.immed, Instr.rotate << 1));
	unsigned int uiRnVal = g_Registers[Instr.Rn];

	// Check if PC is destination
	if (Instr.Rd != 0x0f) return uiPC + 4; // Next instruction
	if ((Instr.Rn = 0x0f)) uiRnVal += 8;

	// Check what opcode it is!
	switch ((Instr.opcode)) {
		case OPCODE_MOV:
			return uiOperand;
		case OPCODE_MVN:
			return ~uiOperand;
		case OPCODE_ADD:
			return uiRnVal + uiOperand;
		case OPCODE_ADC:
			return uiRnVal + uiOperand + (((g_Registers[18] & (1 << 29))) == 0?0:1);
		case OPCODE_SUB:
			return uiRnVal - uiOperand;
		case OPCODE_SBC:
			return uiRnVal - uiOperand - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_RSB:
			return uiOperand - uiRnVal;
		case OPCODE_RSC:
			return uiOperand - uiRnVal - (((g_Registers[18] & (1 << 29))) == 0?1:0);
		case OPCODE_AND:
			return (uiRnVal & uiOperand);
		case OPCODE_EOR:
			return (uiRnVal^uiOperand);
		case OPCODE_ORR:
			return (uiRnVal | uiOperand);
		case OPCODE_BIC:
			return (uiRnVal & ~uiOperand);
		default:
			return 0;
	}

}

///////////////////////////////////////////////////////////////
//
// lsio; // Load/store immediate offset
// Example ldr Rd, [Rn, #<immed>]
unsigned int DecodeLSIO(lsio Instr, unsigned int uiPC)
{
	unsigned int uiRnValue = g_Registers[Instr.Rn];
	unsigned int uiMemValue;

	// Check if destination is PC
	if (Instr.Rd != 0x0f) return uiPC + 4;
	// Check if it's a LDR instruction
	if (Instr.L != 1) return uiPC + 4;

	if (Instr.Rn == 0x0f) uiRnValue += 8;

	// Check if it's pre-indexed
	if (Instr.p == 1){

		if (Instr.U == 1) {
			// Add offset
			uiRnValue += Instr.immed;
		} else {
			// Sub offset
			uiRnValue -= Instr.immed;
		}
	}

	uiMemValue = *(unsigned int *)(uiRnValue);

	return uiMemValue;

}

///////////////////////////////////////////////////////////////
//
// lsro; // Load/Store register offset
// Example ldr Rd, [Rn + Rm lsl 5]
unsigned int DecodeLSRO(lsro Instr, unsigned int uiPC)
{
	unsigned int uiRnValue = g_Registers[Instr.Rn];
	unsigned int uiRmValue = g_Registers[Instr.Rm];
	unsigned int uiIndex;
	unsigned int uiMemValue;

	if (Instr.Rm == 0x0f) uiRmValue += 8;
	if (Instr.Rn == 0x0f) uiRnValue += 8;

	// Check if destination is PC and that it's LDR instruction
	if ((Instr.Rd != 0x0f) || (Instr.L != 1)) return uiPC + 4;

	uiIndex = Shift_Operand(Instr.Rm, Instr.amount, Instr.shift);

	if (Instr.p == 1){

		if (Instr.U == 1) {
			// Add offset
			uiRnValue += uiIndex;
		} else {
			// Sub offset
			uiRnValue -= uiIndex;
		}
	}

	uiMemValue = *(unsigned int *)(uiRnValue);

	return uiMemValue;


}
///////////////////////////////////////////////////////////////
//
// lsm; // Load store multiple
// Example: ldm r0, {r1, r2, r3}
unsigned int DecodeLSM(lsm Instr, unsigned int uiPC)
{
	unsigned int	uiRnValue = g_Registers[Instr.Rn];
	unsigned int  	uiOffsetToPC = 0;
	unsigned int	uiMemValue;

	// Make sure PC is destination and it's Load instruction
	if (((Instr.regs & (1 << 15)) == 0) || (Instr.L != 1)) return uiPC + 4;


	// Check if U bit it set
	if (Instr.U == 0) {
		// This means that it's ascending
		// Also means that the PC is closest to Rn
		if (Instr.p == 1) {
			// Pre decrement.
			uiOffsetToPC -= 4;
		} else {
			uiOffsetToPC = 0;
		}
	} else {
		// The stack is descending, that means that the PC is as far away as possible.
		// Lets find out how many registers before it.
		for (int i = 0; i < 15; i++) {
			if ((Instr.regs & (1 << i)) != 0) uiOffsetToPC += 4;
		}

		// If the P bit is set, it uses pre increment
		if (Instr.p == 1) uiOffsetToPC += 4;
	}

	// read from out calculated address.
	uiMemValue = *(unsigned int *)((uiRnValue + uiOffsetToPC) & ~0x03);

	return uiMemValue;

}

///////////////////////////////////////////////////////////////
//
// bl; // Branch with link(optional)
unsigned int DecodeBL(bl Instr, unsigned int uiPC)
{
	//Printf("Decode BL");
	unsigned int uiAddress;

	uiAddress = Instr.offset;
	if (uiAddress & 0x00800000) {
		uiAddress |= 0xff000000;
	}

	uiAddress <<= 2;
	uiAddress += 8;

	return uiPC + uiAddress;

}
