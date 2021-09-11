#include "common.h"
#include "resdata.h"
#include "cga.h"
#include "room.h"
#include "sound.h"

unsigned char far *anima_end_ofs;

unsigned char last_anim_y = 0;
unsigned char last_anim_x = 0;
unsigned char anim_shift_y = 0;
unsigned char anim_shift_x = 0;
unsigned char last_anim_height;
unsigned char last_anim_width;
unsigned char anim_cycle;
unsigned char anim_flags;
unsigned char anim_use_dot_effect;
unsigned int anim_draw_delay;
unsigned char dot_effect_step;
unsigned int dot_effect_delay;

extern unsigned short cpu_speed_delay;

extern unsigned char * SeekToEntry(unsigned char far *bank, unsigned int num, unsigned char far **end);
extern void LoadLutinSprite(unsigned int lutidx);

void GetScratchBuffer(unsigned char mode)
{
	unsigned char *buffer = scratch_mem2;
	unsigned int offs = 0;
	if(mode & 0x80)
		offs += 3200;
	if(mode & 0x40)
		offs += 1600;
	lutin_mem = buffer + offs;
}

void AnimLoadSprite(unsigned char **panim)
{
	unsigned char mode;
	unsigned char index;
	mode = *((*panim)++);
	index = *((*panim)++);
	GetScratchBuffer(mode);
	LoadLutinSprite(index);
}

void ClipSprite(unsigned char *x, unsigned char *y, unsigned char *sprw, unsigned char *sprh, unsigned char **sprite, signed char dx, signed char dy)
{
	if(anim_flags == 7)
		return;
	if(anim_flags & 4)
	{
		if(anim_cycle == 0)
			return;
		if(anim_flags & 2)
		{
			*sprh = anim_cycle;
			if(anim_cycle >= dy)
				anim_cycle -= dy;
			else
				anim_cycle = 0;
		}
		else if(anim_flags & 1)
		{
			*sprw = anim_cycle;
			anim_cycle--;
		}
		else
		{
			*x -= dx;
			*sprite += (*sprw - anim_cycle) * 2;
			*sprw = anim_cycle;
			anim_cycle--;
		}
	}
	else if(anim_flags & 2)
	{
		if(*sprw == anim_cycle)
		{
			anim_cycle = 0;
		}
		else if(anim_flags & 1)
		{
			*sprw = anim_cycle;
			anim_cycle++;
		}
		else
		{
			*x -= dx;
			*sprite += (*sprw - anim_cycle) * 2;
			*sprw = anim_cycle;
			anim_cycle++;
		}
	}
}

void CopyScreenBlockWithDotEffect(unsigned char *source, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *target)
{
	unsigned int offs;
	unsigned int xx = x * 4;
	unsigned int ww = width * 4;
	unsigned int cur_image_end = ww * height;

	for(offs = 0;offs != cur_image_end;)
	{
		unsigned char mask = 0xC0 >> (((xx + offs % ww) % 4) * 2);
		unsigned int ofs = CGA_CalcXY(xx + offs % ww, y + offs / ww);

		target[ofs] = (target[ofs] & ~mask) | (source[ofs] & mask);

		if(dot_effect_delay / 4 != 0)
		{
			unsigned int i;
			for(i = 0;i < dot_effect_delay / 4;i++) ;	/*TODO: weak delay*/
		}
		
		offs += dot_effect_step;
		if(offs > cur_image_end)
			offs -= cur_image_end;
	}
}

void AnimDrawSprite(unsigned char x, unsigned char y, unsigned char sprw, unsigned char sprh, unsigned char *pixels, unsigned int pitch)
{
	unsigned int delay, delay2;
	unsigned char ex, ey, updx, updy, updw, updh;
	unsigned int ofs = CGA_CalcXY_p(x, y);
	CGA_BackupImage(backbuffer, ofs, sprw, sprh, sprit_load_buffer);
	CGA_BlitSprite(pixels, pitch, sprw, sprh, backbuffer, ofs);
	ex = x + sprw;
	ey = y + sprh;
	if(last_anim_height != 0)
	{
		if(last_anim_x + last_anim_width > ex)
			ex = last_anim_x + last_anim_width;

		if(last_anim_y + last_anim_height > ey)
			ey = last_anim_y + last_anim_height;

		updx = (x > last_anim_x) ? last_anim_x : x;
		updy = (y > last_anim_y) ? last_anim_y : y;
	}
	else
	{
		updx = x;
		updy = y;
	}
	updw = ex - updx;
	updh = ey - updy;
	ofs = CGA_CalcXY_p(updx, updy);
	/*TODO looks like here was some code before*/
	for(delay = 0;delay < anim_draw_delay;delay++)
	{
		for(delay2 = 0;delay2 < cpu_speed_delay;delay2++) ;	/*TODO FIXME weak delay*/
	}
	WaitVBlank();

	if(anim_use_dot_effect)
		CopyScreenBlockWithDotEffect(backbuffer, updx, updy, updw, updh, frontbuffer);
	else
	{
		CGA_CopyScreenBlock(backbuffer, updw, updh, frontbuffer, ofs);
	}
	CGA_RestoreImage(sprit_load_buffer, backbuffer);

	last_anim_x = x;
	last_anim_y = y;
	last_anim_width = sprw;
	last_anim_height = sprh;

	anim_shift_x = anim_shift_y = 0;
}

void AnimUndrawSprite(void)
{
	CGA_CopyScreenBlock(backbuffer, last_anim_width, last_anim_height, CGA_SCREENBUFFER, CGA_CalcXY_p(last_anim_x, last_anim_y));
	last_anim_height = 0;
}

void PlayAnimCore(unsigned char **panim)
{
	unsigned char mode;
	unsigned int count, count2;
	unsigned char *pframe;
	mode = *((*panim)++);
	anim_flags = mode & 7;
	count = mode >> 3;

	while(count--)
	{
		pframe = *panim;	
		mode = *pframe++;
		anim_draw_delay = ((mode & ~7) >> 3) << 1;
		dot_effect_step = (mode & ~7) >> 3;
		dot_effect_delay = 500;
		count2 = mode & 7;
		while(count2--)
		{
			unsigned char far *sprite;
			unsigned char sprw, sprh;
			unsigned char x, y;
			signed char dx, dy;
			unsigned int pitch;
			mode = *pframe++;
			GetScratchBuffer(mode);
			dy = mode & 7;
			dx = (mode >> 3) & 7;

			dx = (dx & 1) ? -(dx & ~1) : dx;
			dx /= 2;
			dy = (dy & 1) ? -(dy & ~1) : dy;

			x = last_anim_x + dx + anim_shift_x;
			y = last_anim_y + dy + anim_shift_y;

			sprite = lutin_mem;
			sprw = *sprite++;
			sprh = *sprite++;

			pitch = sprw * 2;			
			ClipSprite(&x, &y, &sprw, &sprh, &sprite, dx, dy);
			AnimDrawSprite(x, y, sprw, sprh, sprite, pitch);

			if(anim_flags & 4)
			{
				if(anim_cycle == 0)
				{
					AnimUndrawSprite();
					goto end;
				}
			}
			else if(anim_flags & 2)
			{
				if(anim_cycle == 0)
				{
					goto end;
				}
			}
		}
	}
end:;
	mode = *((*panim)++);
	*panim += mode & 7;
}

void Anim1(unsigned char **panim)
{
	anim_cycle = 0xFF;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim2(unsigned char **panim)
{
	anim_cycle = 1;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim3(unsigned char **panim)
{
	anim_cycle = 1;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim4(unsigned char **panim)
{
	anim_cycle = last_anim_width - 1;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim5(unsigned char **panim)
{
	anim_cycle = last_anim_width - 1;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim6(unsigned char **panim)
{
	anim_cycle = last_anim_height;
	anim_use_dot_effect = 0;
	PlayAnimCore(panim);
}

void Anim7(unsigned char **panim)
{
	anim_cycle = 0xFF;
	anim_use_dot_effect = 1;
	PlayAnimCore(panim);
}

typedef void (*animhandler_t)(unsigned char **panim);

animhandler_t anim_handlers[] = {
	AnimLoadSprite,
	Anim1,
	Anim2,
	Anim3,
	Anim4,
	Anim5,
	Anim6,
	Anim7
};

void PlayAnim(unsigned char index, unsigned char x, unsigned char y)
{
	unsigned char sound;
	unsigned char *panim;

	last_anim_width = 0;
	last_anim_height = 0;
	last_anim_x = x;
	last_anim_y = y;

	panim = SeekToEntry(anima_data, index - 1, &anima_end_ofs);
	while(panim != anima_end_ofs)
	{
		unsigned char mode = *panim;
		switch(mode)
		{
		case 0xFE:	/*set shift*/
			panim++;
			anim_shift_x = *panim++;
			anim_shift_y = *panim++;
			break;
		case 0xFD:	/*play sfx*/
			panim++;
			sound = *panim++;
			panim++;	/*unused*/
			PlaySound(sound);
			break;
		case 0xFC:	/*nothing*/
			panim++;
			break;
		default:
			anim_handlers[mode & 7](&panim);
		}
	}
}
