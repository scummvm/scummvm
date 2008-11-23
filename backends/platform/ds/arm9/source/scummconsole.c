//////////////////////////////////////////////////////////////////////
//
// console.cpp -- provides basic print functionality
//
// version 0.1, February 14, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//	 0.2: Fixed sprite mapping bug.  1D mapping should work now.  
//			Changed some register defines for consistency.
//
//////////////////////////////////////////////////////////////////////


#include <nds.h>
#include "scummconsole.h"

#include <stdarg.h>

#include <default_font_bin.h>

/////////////////////////////////////////
//global console variables

#define CONSOLE_WIDTH 32
#define CONSOLE_HEIGHT 24
#define TAB_SIZE 3

//map to print to
u16* fontMap;

//location of cursor
u8 row, col;

//font may not start on a character base boundry
u16 fontOffset; 

//the first character in the set (0 if you have a full set)
u16 fontStart;

//the 16-color palette to use
u16 fontPal;




///////////////////////////////////////////////////////////
//consoleInit
// param: 
//		font: 16 color font
//		charBase: the location the font data will be loaded to
//		numCharacters: count of characters in the font
//		charStart: The ascii number of the first character in the font set
//					if you have a full set this will be zero
//		map: pointer to the map you will be printing to.
//		pal: specifies the 16 color palette to use, if > 15 it will change all non-zero
//			entries in the font to use palette index 255
void consoleInit(u16* font, u16* charBase, u16 numCharacters, u8 charStart, u16* map, u8 pal, u8 bitDepth)
{
	int i;

	row = col = 0;
	
	fontStart = charStart;

	fontOffset = 0;

	fontMap = map;

	if(bitDepth == 16)
	{
		if(pal < 16)
		{
			fontPal = pal << 12;

			for (i = 0; i < numCharacters * 16; i++)
				charBase[i] = font[i];
		}
		else
		{
			fontPal = 15 << 12;

			for (i = 0; i < numCharacters * 16; i++)
			{
				u16 temp = 0;

				if(font[i] & 0xF)
					temp |= 0xF;
				if(font[i] & 0xF0)
					temp |= 0xF0;
				if(font[i] & 0xF00)
					temp |= 0xF00;
				if(font[i] & 0xF000)
					temp |= 0xF000;
	
				charBase[i] = temp;
			}	
		}
	}//end if bitdepth
	else
	{
		fontPal = 0;
		for(i = 0; i < numCharacters * 16; i++)
		{
			u32 temp = 0;

			if(font[i] & 0xF)
				temp = 255;
			if(font[i] & 0xF0)
				temp |= 255 << 8;
			if(font[i] & 0xF00)
				temp |= 255 << 16;
			if(font[i] & 0xF000)
				temp |= 255 << 24;

			((u32*)charBase)[i] = temp;

		}
	}
}

void consoleInitDefault(u16* map, u16* charBase, u8 bitDepth)
{
	consoleInit((u16 *) default_font_bin, charBase, 128, 0, map, CONSOLE_USE_COLOR255, bitDepth); 	
}

void consolePrintSet(int x, int y)
{
	if(y < CONSOLE_HEIGHT)
		row = y;
	else
		row = CONSOLE_HEIGHT - 1;

	if(x < CONSOLE_WIDTH)
		col = x;
	else
		col = CONSOLE_WIDTH - 1;
}

void consolePrintChar(char c)
{
	int i;

	if(col >= CONSOLE_WIDTH)
	{
		col = 0;

		row++;		
	}
	
	if(row >= CONSOLE_HEIGHT)
	{
		row--;

		for(i = CONSOLE_WIDTH; i < CONSOLE_HEIGHT * CONSOLE_WIDTH; i++)
			fontMap[i - CONSOLE_WIDTH] = fontMap[i];
		for(i = 0; i < CONSOLE_WIDTH; i++)
			fontMap[i + (CONSOLE_HEIGHT-1)*CONSOLE_WIDTH] = fontPal | (u16)(' ' + fontOffset - fontStart);


	}
	
	switch(c)
	{

	case 10:
	case 11:
	case 12:
	case 13:
		row++;
		col = 0;
		break;
	case 9:
		col += TAB_SIZE;
		break;
	default:
		fontMap[col + row * CONSOLE_WIDTH] = fontPal | (u16)(c + fontOffset - fontStart);
		col++;
		break;

	}

	
}


void printX(int w, unsigned d)
{
	int loop = 0;
	int i = 0;
	
	char buf[20] = {0};


	while(d > 0)
	{
		buf[loop++] =  d & 0xF;
		d = d>>4; 
	}
	
	for (i = 7; i >= 0; i--)
	{
		if(buf[i] || i < loop)
		{
			if(buf[i] < 10)
				consolePrintChar(buf[i] + '0');
			else
				consolePrintChar(buf[i] + 'A' - 10);
		}
		else if(i < w)
			consolePrintChar(' ');
	}
}

void printx(int w, unsigned int d)
{
	int loop = 0;
	int i = 0;
	
	char buf[20] = {0};

	while(d > 0)
	{
		buf[loop++] =  d & 0xF;
		d = d>>4; 
	}
	
	for (i = 7; i >= 0; i--)
	{
		if(buf[i] || i < loop)
		{
			if(buf[i] < 10)
				consolePrintChar(buf[i] + '0');
			else
				consolePrintChar(buf[i] + 'a' - 10);
		}
		else if(i < w)
			consolePrintChar(' ');
	}
}

void printInt(int w, int d)
{
	int loop = 0;
	int i = 0;
	
	char buf[20] = {0}; 

	if(d < 0)
	{
		consolePrintChar('-');
		d *= -1;
	}

	if (d == 0)
		buf[loop++] = 0;
	else while (d > 0)
	{
		buf[loop++] =  d % 10;
		d /= 10; 
	}
	
	for (i = 7; i >= 0; i--)
	{
		if(buf[i] || i < loop)
			consolePrintChar(buf[i] + '0');
		else if(i < w)
			consolePrintChar(' ');
	}
}

void printBin(int w, int d)
{
	int i;
	int first = 0;
	for (i = 31; i >= 0; i--)
	{
		if(d & BIT(i))
		{
			first = 1;
			consolePrintChar('1');
		}
		else if (first  || i == 0)
			consolePrintChar('0');
		else if (i < w)
			consolePrintChar(' ');
	}
}

void print0X(int w, unsigned d)
{
	int loop = 0;
	int i = 0;
	
	char buf[] = {0,0,0,0,0,0,0,0}; //set to zero cause I may add formatted output someday
	

	while(d > 0)
	{
		buf[loop++] =  d & 0xF;
		d = d>>4; 
	}
	
	for (i = 7; i >= 0; i--)
	{
		if(buf[i] || i < w  || i < loop)
		{
			if(buf[i] < 10)
				consolePrintChar(buf[i] + '0');
			else
				consolePrintChar(buf[i] + 'A' - 10);
		}
	}
}

void print0x(int w, unsigned int d)
{
	int loop = 0;
	int i = 0;
	
	char buf[] = {0,0,0,0,0,0,0,0}; //set to zero cause I may add formatted output someday
	

	while(d > 0)
	{
		buf[loop++] =  d & 0xF;
		d = d>>4; 
	}
	
	for (i = 7; i >= 0; i--)
	{
		if(buf[i] || i < w  || i < loop)
		{
			if(buf[i] < 10)
				consolePrintChar(buf[i] + '0');
			else
				consolePrintChar(buf[i] + 'a' - 10);
		}
	}
}

void print0Int(int w, int d)
{
	int loop = 0;
	int i = 0;
	
	char buf[] = {0,0,0,0,0,0,0,0,0,0,0,0,0}; //set to zero cause I may add formatted output someday
	
	if(d < 0)
	{
		consolePrintChar('-');
		d *= -1;
	}

	while(d > 0)
	{
		buf[loop++] =  d % 10;
		d /= 10;  
	}
	
	for (i = 15; i >= 0; i--)
		if(buf[i] || i < w  || i < loop)
			consolePrintChar(buf[i] + '0');

}

void print0Bin(int w, int d)
{
	int i;
	int first = 0;
	for (i = 31; i >= 0; i--)
	{
		if(d & BIT(i))
		{
			first = 1;
			consolePrintChar('1');
		}
		else if (first  || i == 0)
			consolePrintChar('0');
		else if (i < w)
			consolePrintChar('0');
	}
}

void print(const char* s)
{
	for(; *s; s++) consolePrintChar(*s);
}

void printF(int w, float f)
{
	unsigned int* t = (unsigned int*)&f;
	unsigned int fraction = (*t) & 0x007FFFFF;
	int exp = ((*t) >> 23) & 0xFF;

	if(*t & BIT(31))
		consolePrintChar('-');
	

	print0Bin(32, fraction);
	
	printInt(1, fraction);
	consolePrintChar('e');
	printInt(1, exp - 127);
	
	/*
	if(exp == 0 && fraction == 0)
	{
		printInt(1,0);
	}
	else if(exp == 0xFF && fraction == 0)
	{
		print("Inifinite");
	}
	else
	{
		printInt(w,fraction);
		consolePrintChar('e');
		printInt(1,exp - 127);
	}
	*/
}

int consolePrintf(const char* s, ...)
{
	int w = 1, z = 0;

	va_list argp;

	va_start(argp, s);
	

	while(*s)
	{
		w = 1;
		z = 0;

		switch(*s)
		{
		case '%':
			s++;
			if(*s == '0')
			{
				z = 1;
				s++;
			}
			if(*s > '0' && *s <= '9')
			{
				w = *s - '0';
				s++;
			}
			switch (*s)
			{
			case 'i':
			case 'I':
			case 'd':
			case 'D':
				if(z)print0Int(w, va_arg(argp, int)); 
				else printInt(w, va_arg(argp, int));
				s++;
				break;
			case 'X':
				if(z)print0X(w, va_arg(argp, int));
				else printX(w, va_arg(argp, int));
				s++;
				break;

			case 'x':
				if(z)print0x(w, va_arg(argp, int));
				else printx(w, va_arg(argp, int));
				s++;
				break;

			case 'b':
			case 'B':
				if(z)print0Bin(w, va_arg(argp, int));
				else printBin(w, va_arg(argp, int));
				s++;
				break;
			case 'f':
			case 'F':
/* Need to undo our 'all doubles are floats' definition */
#define TEMP_DEF double
#undef double
			        printF(w,va_arg(argp, double));
#define double TEMP_DEF
#undef TEMP_DEF
				s++;
				break;
			case 's':
			case 'S':
				print(va_arg(argp, char*));
				s++;
				break;
			default:
				consolePrintChar('%');
				break;
			}
		default:
			consolePrintChar(*s);
			break;
		}

		s++;
	}
	va_end(argp);
	
	return 0;
}

void consolePutString(int x, int y, char* s)
{
	consolePrintSet(x, y);
	consolePrintf(s);
}

void consolePutInt(int x, int y, int d)
{
	consolePrintSet(x,y);
	printInt(1,d);
}

void consolePutX(int x, int y, int d)
{
	consolePrintSet(x, y);
	printX(1,d);
}

void consolePutChar(int x, int y, char c)
{
	consolePrintSet(x, y);
	consolePrintChar(c);
}

void consolePutBin(int x, int y, int b)
{
	consolePrintSet(x, y);
	printBin(1,b);
}
void consoleClear(void)
{
	int i = 0;
	consolePrintSet(0,0);

	while(i++ < CONSOLE_HEIGHT * CONSOLE_WIDTH)
		consolePrintChar(' ');

	consolePrintSet(0,0);
}
