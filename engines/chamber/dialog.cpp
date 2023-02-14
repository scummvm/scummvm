#include "common.h"
#include "dialog.h"
#include "print.h"
#include "cga.h"
#include "script.h"
#include "cursor.h"
#include "input.h"

unsigned int cur_str_index;
unsigned int cur_dlg_index;

#define MAX_DIRTY_RECT 10
dirty_rect_t dirty_rects[MAX_DIRTY_RECT];

void AddDirtyRect(unsigned char kind, unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned int offs)
{
	int i;
	dirty_rect_t *r = dirty_rects;
	for(i = 0;i < MAX_DIRTY_RECT;i++, r++)	/*TODO: may go oob*/
		if(r->kind == DirtyRectFree)
			break;
	r->kind = kind;
	r->offs = offs;
	r->width = w;
	r->height = h;
	r->y = y;
	r->x = x;
	script_byte_vars.dirty_rect_kind = dirty_rects[0].kind;
}

void GetDirtyRect(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs, unsigned char newkind)
{
	*kind = dirty_rects[index].kind;
	*offs = dirty_rects[index].offs;
	*w = dirty_rects[index].width;
	*h = dirty_rects[index].height;
	*y = dirty_rects[index].y;
	*x = dirty_rects[index].x;

	dirty_rects[index].kind = newkind;
	script_byte_vars.dirty_rect_kind = dirty_rects[0].kind;
}

void GetDirtyRectAndFree(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs)
{
	GetDirtyRect(index - 1, kind, x, y, w, h, offs, DirtyRectFree);
}

void GetDirtyRectAndSetSprite(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs)
{
	GetDirtyRect(index - 1, kind, x, y, w, h, offs, DirtyRectSprite);
}

int FindDirtyRectAndFree(unsigned char kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs)
{
	int i;
	for(i = 0;i < MAX_DIRTY_RECT;i++)
	{
		if(dirty_rects[i].kind == kind)
		{
			GetDirtyRect(i, &kind, x, y, w, h, offs, DirtyRectFree);
			return 1;
		}
	}
	return 0;
}

/*Restore screen data from back buffer as specified by dirty rects of kind*/
void PopDirtyRects(unsigned char kind)
{
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;
	while(FindDirtyRectAndFree(kind, &x, &y, &width, &height, &offs))
	{
		CGA_CopyScreenBlock(backbuffer, width, height, frontbuffer, offs);
		if(kind == DirtyRectBubble)
		{
			/*pop bubble's spike*/
			CGA_CopyScreenBlock(backbuffer, 2, 21, frontbuffer, offs = (x << 8) | y);
		}
	}
}

void DesciTextBox(unsigned int x, unsigned int y, unsigned int width, unsigned char *msg)
{
	draw_x = x;
	draw_y = y;
	char_draw_max_width = width;
	CGA_DrawTextBox(msg, frontbuffer);
	AddDirtyRect(DirtyRectText, draw_x, draw_y, char_draw_max_width + 2, char_draw_coords_y - draw_y + 8, CGA_CalcXY_p(draw_x, draw_y));
}

/*Draw dialog bubble with text and spike*/
void DrawPersonBubble(unsigned char x, unsigned char y, unsigned char flags, unsigned char *msg)
{
	unsigned int ofs;
	unsigned char w, h;

	char_draw_max_width = flags & 0x1F;
	char_xlat_table = chars_color_bonw;

	/*upper border*/
	ofs = CGA_CalcXY_p(x, y);
	ofs = CGA_DrawHLineWithEnds(0xF00F,      0,    0, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = CGA_DrawHLineWithEnds(0xC003, 0x0FF0, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = CGA_DrawHLineWithEnds(     0, 0x3FFC, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = CGA_DrawHLineWithEnds(     0, 0x3FFC, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);

	/*body*/
	char_draw_coords_x = x;
	char_draw_coords_y = y + 4;

	for(string_ended = 0;!string_ended;char_draw_coords_y += 6)
	{
		CGA_PrintChar(0x3B, CGA_SCREENBUFFER);
		msg = PrintStringPadded(msg, CGA_SCREENBUFFER);
		CGA_PrintChar(0x3C, CGA_SCREENBUFFER);
		char_draw_coords_x = x;
	}
	
	ofs = CGA_CalcXY_p(x, char_draw_coords_y);
	ofs = CGA_DrawHLineWithEnds(0xC003, 0x0FF0, 0xFF, char_draw_max_width, CGA_SCREENBUFFER, ofs);
	ofs = CGA_DrawHLineWithEnds(0xF00F,      0,    0, char_draw_max_width, CGA_SCREENBUFFER, ofs);

	w = char_draw_max_width + 2;
	h = char_draw_coords_y - y + 2;

	/*draw spike*/
	switch(flags & SPIKE_MASK)
	{
	case SPIKE_UPLEFT:	/*upper-left spike*/
		ofs = CGA_CalcXY_p(x + 1, y - 7);
		DrawSpriteN(18, x + 1, y - 7, CGA_SCREENBUFFER);
		break;
	case SPIKE_UPRIGHT:	/*upper-right spike*/
		ofs = CGA_CalcXY_p(x + char_draw_max_width, y - 7) - 1;
		DrawSpriteNFlip(18, x + char_draw_max_width, y - 7, CGA_SCREENBUFFER);
		break;
	case SPIKE_DNRIGHT:	/*lower-right spike*/
		ofs = CGA_CalcXY_p(x + char_draw_max_width, char_draw_coords_y + 1) - 1;
		DrawSpriteNFlip(21, x + char_draw_max_width, char_draw_coords_y + 1, CGA_SCREENBUFFER);
		break;
	case SPIKE_DNLEFT:	/*lower-left spike*/
		ofs = CGA_CalcXY_p(x + 1, char_draw_coords_y + 1);
		DrawSpriteN(21, x + 1, char_draw_coords_y + 1, CGA_SCREENBUFFER);
		break;
	case SPIKE_BUBRIGHT:	/*lower-right bubbles*/
		ofs = CGA_CalcXY_p(x + char_draw_max_width, char_draw_coords_y + 4);
		DrawSpriteN(20, x + char_draw_max_width, char_draw_coords_y + 4, CGA_SCREENBUFFER);
		break;
	case SPIKE_BUBLEFT:	/*lower-left bubbles*/
		ofs = CGA_CalcXY_p(x + 1, char_draw_coords_y + 4);
		DrawSpriteN(19, x + 1, char_draw_coords_y + 4, CGA_SCREENBUFFER);
		break;
	}

	AddDirtyRect(DirtyRectBubble, ofs >> 8, ofs & 255, w, h, CGA_CalcXY_p(x, y));
}

void ShowPromptAnim(void)
{
	if(script_byte_vars.zone_index == 135)
		return;
	WaitVBlank();
	DrawSpriteN(cursor_anim_phase ? 23 : 22, 300 / 4, 155, frontbuffer);
	cursor_anim_phase = ~cursor_anim_phase;
}

void PromptWait(void)
{
	cursor_anim_phase = 0;

	do
	{
		unsigned char ticks = script_byte_vars.timer_ticks;
		if((ticks % 8) == 0 && ticks != cursor_anim_ticks)
		{
			cursor_anim_ticks = ticks;
			ShowPromptAnim();
		}
		PollInput();
	}
	while(!buttons);

	if(cursor_anim_phase)
		ShowPromptAnim();
}

/*
Get string with index num from strings bank
*/
unsigned char * SeekToString(unsigned char *bank, unsigned int num)
{
	unsigned char len;
	unsigned char *p = bank;

	cur_str_index = num;

	num -= 4;
	while(num--)
	{
		len = *p;
		p += len;
	}
	len = *p;
	cur_str_end = p + len;
	return p + 1;
}

/*
Get string with index num from strings bank, with large string index support for scripts
*/
unsigned char * SeekToStringScr(unsigned char *bank, unsigned int num, unsigned char **ptr)
{
	unsigned char len;
	unsigned char *p = bank;

	if(num < 4)
	{
		num = (num << 8) | *(++(*ptr));
	}
	cur_str_index = num;

	num -= 4;
	while(num--)
	{
		len = *p;
		p += len;
	}
	len = *p;
	cur_str_end = p + len;
	return p + 1;
}
