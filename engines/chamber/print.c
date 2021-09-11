#include "common.h"
#include "cga.h"
#include "cursor.h"
#include "dialog.h"

unsigned char *cur_str_end;

unsigned char draw_x;
unsigned char draw_y;

/*
Calculate number of string's character until whitespace
Return current word's characters count and the next word ptr
*/
unsigned char * CalcStringWordWidth(unsigned char *str, unsigned int *w)
{
	unsigned int ww = 0;
	unsigned char c;

	if((*str & 0x3F) == 0)
	{
		str++;
		ww++;
	}

	while(str != cur_str_end)
	{
		if((*str & 0x3F) == 0)
			break;
		ww++;
		c = *str & 0xC0;
		if(c != 0)
		{
			if(c == 0x40)	/*space?*/
			{
				str++;
				break;
			}
			ww++;
		}
		str++;
	}

	*w = ww;
	return str;
}

/*
Calculate number of text's words and max word width (in chars)
*/
void CalcStringSize(unsigned char *str, unsigned int *w, unsigned int *n)
{
	unsigned int ww = 0, nw = 0, lw;
	unsigned char *s = str;
	do
	{
		s = CalcStringWordWidth(s, &lw);
		if(lw > ww)
			ww = lw;
		nw += 1;
	}
	while(s != cur_str_end);
	*w = ww;
	*n = nw;
}

/*
Calculate number of text's lines with respect to set max width
If a line is longer, wrap it to the next line
*/
unsigned int CalcTextLines(unsigned char *str)
{
	unsigned int lines = 1;
	unsigned int w, left = char_draw_max_width;
	while(str != cur_str_end)
	{
		str = CalcStringWordWidth(str, &w);
		if(left > w)
		{
			left = left - w - 1;
		}
		else
		{
			lines++;
			left = char_draw_max_width - w - 1;
		}
	}
	return lines;
} 

/*; translate 1-bit raster (4 columns per byte) to 4 2-bit color pixels*/
unsigned char chars_color_bonw[] = {0xFF, 0xFC, 0xF3, 0xF0, 0xCF, 0xCC, 0xC3, 0xC0, 0x3F, 0x3C, 0x33, 0x30, 0x0F, 0x0C,    3,    0}; /*black on white*/
unsigned char chars_color_bonc[] = {0x55, 0x54, 0x51, 0x50, 0x45, 0x44, 0x41, 0x40, 0x15, 0x14, 0x11, 0x10,    5,    4,    1,    0}; /*black on cyan*/
unsigned char chars_color_wonb[] = {   0,    3, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, 0xC0, 0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF}; /*white on black*/
unsigned char chars_color_wonc[] = {0x55, 0x57, 0x5D, 0x5F, 0x75, 0xF7, 0x7D, 0x7F, 0xD5, 0xD7, 0xDD, 0xDF, 0xF5, 0xF7, 0xFD, 0xFF}; /*white on cyan*/

void PrintStringPad(unsigned int w, unsigned char *target)
{
	while(w--)
		CGA_PrintChar(0, target);
}

unsigned char * PrintWord(unsigned char *str, unsigned char *target)
{
	unsigned char c, f;
	if((*str & 0x3F) == 0)
		goto skip_1st;
	while(str != cur_str_end)
	{
		f = *str;
		c = f & 0x3F;
		if(c == 0)
		{
			if((f & 0xC0) == 0)
				str++;
			return str;
		}
		CGA_PrintChar(c, target);

skip_1st:;
		f = *str & 0xC0;
		str++;
		if(f)
		{
			if(f == 0x80)
				CGA_PrintChar(0x25, target);
			else if(f != 0x40)
				CGA_PrintChar(0x21, target);
			else
				return str;
		}
	}
	string_ended = 1;
	return str;
}

unsigned char * PrintStringLine(unsigned char *str, unsigned int *left, unsigned char *target)
{
	unsigned int mw = char_draw_max_width;
	for(;;)
	{
		unsigned int w;
		CalcStringWordWidth(str, &w);
		if(mw < w)
			break;
		mw -= w;
		str = PrintWord(str, target);
		if(string_ended || (mw == 0))
			break;
		mw--;
		CGA_PrintChar(0, target);
	}
	*left = mw;
	return str;
}

unsigned char * PrintStringPadded(unsigned char *str, unsigned char *target)
{
	unsigned int w, n;
	CalcStringSize(str, &w, &n);
	if(w + 2 >= char_draw_max_width)
		char_draw_max_width = w + 2;
	str = PrintStringLine(str, &w, target);
	if(w != 0)
		PrintStringPad(w, target);
	return str;
}

void PrintStringCentered(unsigned char *str, unsigned char *target)
{
	unsigned char pad = 0;
	unsigned int ww = 0, lw;
	unsigned char *s = str;
	do
	{
		s = CalcStringWordWidth(s, &lw);
		ww += lw;
	}
	while(s != cur_str_end);

	pad = (char_draw_max_width - ww) / 2;
	if(pad)
	{
		char_draw_max_width -= pad;
		PrintStringPad(pad, target);
	}
	string_ended = 0;	/*TODO: move me elsewhere*/
	PrintStringPadded(str, target);
}

void CGA_DrawTextBox(unsigned char *msg, unsigned char *target)
{
	unsigned int x, y, w, i;

	char_xlat_table = chars_color_bonc;

	x = draw_x * 4;
	y = draw_y;
	w = (char_draw_max_width + 2) * 4 - 2;

	CGA_DrawHLine(x + 2, y, w - 2, 0, target);			/*box top*/
	for(i = 0;i < 3;i++)
		CGA_DrawHLine(x + 1, y + 1 + i, w, 1, target);	/*top margin*/
	CGA_DrawVLine(x,     y + 2, 2, 0, target);			/*left top corner 1*/
	CGA_DrawVLine(x + 1, y + 1, 1, 0, target);			/*left top corner 2*/
	CGA_DrawVLine(x + w, y + 1, 1, 0, target);		/*right top corner 1*/
	CGA_DrawVLine(x + w + 1, y + 2, 2, 0, target);			/*right top corner 2*/

	char_draw_coords_y = draw_y + 4;
	string_ended = 0;
	do
	{
		char_draw_coords_x = draw_x;
		CGA_PrintChar(0x3B, target);
		msg = PrintStringPadded(msg, target);
		CGA_PrintChar(0x3C, target);
		char_draw_coords_y += 6;
	}
	while(!string_ended);

	x = draw_x * 4;
	y = char_draw_coords_y;
    CGA_DrawHLine(x + 1, y, w, 1, target);				/*bottom margin*/
	CGA_DrawVLine(x + 1, y, 1, 0, target);				/*bottom left corner 1*/
	CGA_DrawHLine(x + 2, y + 1, w - 2, 0, target);		/*box bottom*/
	CGA_DrawVLine(x + 1, y, 1, 0, target);				/*TODO: duplicated?*/
	CGA_DrawVLine(x + w, y, 1, 0, target);				/*bottom right corner*/
}

void DrawMessage(unsigned char *msg, unsigned char *target)
{
	unsigned int x, y;
	unsigned int w, h;
	CalcStringSize(msg, &w, &h);
	char_draw_max_width = (h < 5) ? (w + 2) : 20;
	char_draw_max_height = CalcTextLines(msg) * 6 + 7;

	x = cursor_x / 4;
	if(x < 9)
		x = 9;
	if(x + char_draw_max_width + 2 >= 73)
		x = 73 - (char_draw_max_width + 2);

	y = cursor_y;
	if(y + char_draw_max_height >= 200)
		y = 200 - char_draw_max_height;

	draw_x = x;
	draw_y = y;

	CGA_BackupImageReal(CGA_CalcXY_p(x, y), char_draw_max_width + 2, char_draw_max_height);	/*backup orig. screen data*/
	CGA_DrawTextBox(msg, target);					/*draw box with text*/
	PromptWait();									/*wait keypress*/
	CGA_RestoreBackupImage(target);					/*restore screen data*/
}

#if 1
#include <stdio.h>
void DebugMessage(char *msg, ...)
{
	int i;
	unsigned char c;
	unsigned char m[256];
	va_list	ap;

	va_start(ap,msg);
	vsprintf((char*)m,msg,ap);
	va_end(ap);

	for(i = 0;m[i];i++)
	{
		c = m[i];
		if(c >= 'A' && c <= 'Z')
			c = 0x21 + (c - 'A');
		else if(c >= 'a' && c <= 'z')
			c = 0x21 + (c - 'a');
		else if(c >= '0' && c <= '9')
			c = 0x10 + (c - '0');
		else if(c == ' ')
			c = 0x20;
		else if(c == '\n')
			c = 0x00;
		else
			c = 0x1F;
		m[i] = c;
	}

	cur_str_end = m + i;

	DrawMessage(m, frontbuffer);
}
#endif
