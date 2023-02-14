#include <mem.h>
#include <stdio.h>
#include "common.h"
#include "portrait.h"
#include "resdata.h"
#include "room.h"
#include "cga.h"
#include "script.h"
#include "dialog.h"
#include "input.h"
#include "sound.h"

extern unsigned short cpu_speed_delay;

unsigned char *cur_image_pixels;
unsigned char cur_image_size_w;
unsigned char cur_image_size_h;
unsigned char cur_image_coords_x;
unsigned char cur_image_coords_y;
unsigned int cur_image_offs;
unsigned int cur_image_end;
unsigned char cur_image_idx;
unsigned char cur_image_anim1;
unsigned char cur_image_anim2;
unsigned int cur_frame_width;

typedef struct persframe_t {
unsigned char height;
unsigned char width;
unsigned char topbot;	/*border and fill colors*/
unsigned char fill;
unsigned char left;
unsigned char right;
} persframe_t;

persframe_t pers_frames[] = {
	{65, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{70, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{65, 17, 0xFF, 0xAA, 0xEA, 0xAB},
	{75, 17, 0xFF, 0xAA, 0xEA, 0xAB},
	{85, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{47, 13, 0xFF,    0, 0xC0,    3},
	{65, 18, 0xFF, 0xAA, 0xEA, 0xAB},
	{38, 11, 0xFF,    0, 0xC0,    3},
	{27, 34,    0,    0,    0,    0}
};

void MakePortraitFrame(unsigned char index, unsigned char *target)
{
	unsigned int i;
	persframe_t *pframe = &pers_frames[index];
	*target++ = pframe->height;
	*target++ = pframe->width;
	cur_frame_width = pframe->width;
	memset(target, pframe->topbot, pframe->width); target += pframe->width;
	for(i = 0;i < pframe->height - 2;i++)
	{
		*target++ = pframe->left;
		memset(target, pframe->fill, pframe->width - 2); target += pframe->width - 2;
		*target++ = pframe->right;
	}
	memset(target, pframe->topbot, pframe->width);
}

/*TODO: move this to CGA ?*/
/*
Superimpose source sprite data over target image data
*/
void MergeImageAndSpriteData(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h)
{
	unsigned int x;
	while(h--)
	{
		for(x = 0;x < w;x++)
		{
			unsigned char m = *source++;
			*target &= m;
			*target++ |= *source++;
		}
		target -= w;
		target += pitch;
	}
}

/*
Superimpose horizontally-flipped source sprite data over target image data
*/
void MergeImageAndSpriteDataFlip(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h)
{
	unsigned int x;
	target += w - 1;
	while(h--)
	{
		for(x = 0;x < w;x++)
		{
			unsigned char m = cga_pixel_flip[*source++];
			*target &= m;
			*target |= cga_pixel_flip[*source++];
			target -= 1;
		}
		target += w;
		target += pitch;
	}
}

/*
Build portrait from multiple pers sprites
*/
unsigned char *LoadPortrait(unsigned char **pinfo, unsigned char *end)
{
	while(*pinfo != end)
	{
		unsigned char index;
		unsigned int flags;
		signed int pitch;
		unsigned char *buffer, *sprite;
		unsigned char sprw, sprh;

		index = *((*pinfo)++);
		flags = *((*pinfo)++);
		flags |= (*((*pinfo)++)) << 8;
		buffer = sprit_load_buffer + 2 + 2 + (flags & 0x3FFF);
		pitch = cur_frame_width;

		sprite = LoadPersSprit(index);
		sprw = *sprite++;
		sprh = *sprite++;

		if(flags & 0x8000)	/*vertical flip*/
		{
			buffer += pitch * (sprh - 1);
			pitch = -pitch;
		}
		if(flags & 0x4000)	/*horizontal flip*/
			MergeImageAndSpriteDataFlip(buffer, pitch, sprite, sprw, sprh);
		else
			MergeImageAndSpriteData(buffer, pitch, sprite, sprw, sprh);
	}
	return sprit_load_buffer + 2;
}

unsigned char *LoadPortraitWithFrame(unsigned char index)
{
	unsigned char *pinfo, *end;
	pinfo = SeekToEntry(icone_data, index, &end);
	MakePortraitFrame(*pinfo++, sprit_load_buffer + 2);
	return LoadPortrait(&pinfo, end);
}


#define STATIC_ANIMS_MAX 24

struct {
unsigned char	index;
unsigned char	image;
unsigned char	x;
unsigned char	y;
unsigned char	anim1;
unsigned char	anim2;
} static_anims[] = {
	{ 24,  13, 35, 10,  4,  5},
	{ 88,  42, 35, 10, 11, 12},
	{152,  50, 35, 10, 13, 14},
	{216,  58, 35, 10, 15, 16},
	{ 40,   9, 30, 20,  3,  3},
	{ 48,   1, 35, 20,  1,  2},
	{ 32,  66, 35, 20, 17, 18},
	{128,  21, 20, 10,  6,  6},
	{192,  25,  2, 70,  7,  7},
	{ 56,  85, 25, 20, 26, 27},
	{ 64,  74, 56, 85, 23, 23},
	{ 72,  74, 56, 85, 23, 23},
	{ 80,  78, 27, 20, 24, 24},
	{144,  80, 27, 20, 25, 25},
	{ 96, 100, 27, 20, 29, 29},
	{104,  92, 27, 20, 28, 28},
	{112, 100, 27, 20, 29, 53},
	{224,  96, 27, 20, 48, 48},
	{232,  92, 27, 20, 47, 47},
	{184, 160, 27, 20, 50, 52},
	{200,  78, 27, 20, 24, 24},
	{160, 106, 33,  2, 49, 49},
	{168, 147, 16,  2, 32, 32},
	{248, 117, 16,  2, 33, 33}
};

unsigned char SelectCurrentAnim(unsigned char *x, unsigned char *y, unsigned char *index)
{
	int i;
	unsigned char aniidx = ((pers_t*)(script_vars[ScrPool8_CurrentPers]))->index & ~7;
	for(i = 0;i < STATIC_ANIMS_MAX;i++)
	{
		if(static_anims[i].index == aniidx)
		{
			*x = static_anims[i].x;
			*y = static_anims[i].y;
			*index = static_anims[i].image;
			cur_image_anim1 = static_anims[i].anim1;
			cur_image_anim2 = static_anims[i].anim2;
			return 1;
		}
	}
printf("SelectCurrentAnim: not found for %d\n", aniidx);
	return 0;
}

void DrawBoxAroundSpot(void)
{
	unsigned char *buffer;
	unsigned int w, h;
	unsigned int ofs;
	unsigned int x, y;

	if(*spot_sprite == 0)
		return;
	zone_spots_cur = found_spot;
	zone_spr_index = script_byte_vars.cur_spot_idx - 1;

	buffer = *spot_sprite;

	h = *(unsigned char*)(buffer + 0);
	w = *(unsigned char*)(buffer + 1);
	ofs = *(unsigned int*)(buffer + 2);

	/*decode ofs back to x:y*/
	/*TODO: this is CGA-only!*/
	y = (ofs & CGA_ODD_LINES_OFS) ? 1 : 0;
	ofs &= ~CGA_ODD_LINES_OFS;
	x = (ofs % CGA_BYTES_PER_LINE) * CGA_PIXELS_PER_BYTE;
	y += (ofs / CGA_BYTES_PER_LINE) * 2;
	w *= CGA_PIXELS_PER_BYTE;	/*TODO: this will overflow on large sprite*/

	CGA_DrawVLine(x, y, h - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawHLine(x, y, w - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawVLine(x + w - 1, y, h - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawHLine(x, y + h - 1, w - 1, 0, CGA_SCREENBUFFER);

	CGA_RefreshImageData(*spot_sprite);	
}

/*Get on-screen image as specified by script to temp buffer and register it with dirty rect of kind 2
If rmb is pressed, draw it immediately and return 0
*/
int DrawPortrait(unsigned char **desc, unsigned char *x, unsigned char *y, unsigned char *width, unsigned char *height)
{
	unsigned char index;
	unsigned char xx, yy;
	unsigned char *image;

	index = *((*desc)++);
	if(index == 0xFF)
	{
		if(script_byte_vars.dirty_rect_kind != 0)
			return 0;
		DrawBoxAroundSpot();
		if(!SelectCurrentAnim(&xx, &yy, &index))
			return 0;
	}
	else
	{
		xx = *((*desc)++);
		yy = *((*desc)++);
	}
	cur_image_coords_x = xx;
	cur_image_coords_y = yy;
	cur_image_idx = index;

	image = LoadPortraitWithFrame(index - 1);
	cur_image_size_h = *image++;
	cur_image_size_w = *image++;
	cur_image_pixels = image;
	cur_image_offs = CGA_CalcXY_p(cur_image_coords_x, cur_image_coords_y);
	AddDirtyRect(DirtyRectSprite, cur_image_coords_x, cur_image_coords_y, cur_image_size_w, cur_image_size_h, cur_image_offs);

	/*TODO: remove and use only globals?*/
	*x = cur_image_coords_x;
	*y = cur_image_coords_y;
	*width = cur_image_size_w;
	*height = cur_image_size_h;

	if(right_button)
	{
		CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
		return 0;
	}

	return 1;
}

void BlinkWithSound(unsigned char color)
{
	CGA_ColorSelect(color);
	PlaySound(144);
	SelectPalette();
}

void BlinkToRed(void)
{
	BlinkWithSound(0x3C);	
}

void BlinkToWhite(void)
{
	BlinkWithSound(0x3F);	
}

void AnimPortrait(unsigned char layer, unsigned char index, unsigned char delay)
{
	unsigned char *ani, *ani_end;
	unsigned char temp;

	SelectCurrentAnim(&temp, &temp, &temp);

	if(index == 0xFF)
		index = cur_image_anim1;
	if(index == 0xFE)
		index = cur_image_anim2;

	ani = SeekToEntry(anico_data, index - 1, &ani_end);
	cur_image_pixels = sprit_load_buffer + 2 + 2;

	while(ani != ani_end)
	{
		unsigned char kind;
        unsigned char x, y;
        unsigned char width, height;
        unsigned int offs;

		unsigned char portrait = *ani++;
		LoadPortraitWithFrame(portrait - 1);
		if(*ani == 0xFF)
		{
			ani++;
			LoadPortrait(&ani, ani + 3);
		}
		GetDirtyRectAndSetSprite(layer, &kind, &x, &y, &width, &height, &offs);
		WaitVBlank();
		CGA_BlitAndWait(cur_image_pixels, width, width, height, CGA_SCREENBUFFER, offs);		
		if(delay)
		{
			if(ani[-1] == 37)	/*TODO: what is it?*/
			{
				if(script_byte_vars.byte_179F3)
					BlinkToRed();
				else
					BlinkToWhite();
			}
			else
			{
				int i;
				while(delay--) for(i = 0;i < cpu_speed_delay;i++) ;	/*TODO: FIXME weak delay*/
			}
		}
	}
}
