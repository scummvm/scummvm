#include <dos.h>
#include "common.h"
#include "cga.h"
#include "script.h"
#include "resdata.h"

void AnimateGauss(unsigned char *target)
{
	unsigned char *sprite;
	unsigned char phase = Rand() % 4;
	if(phase == script_byte_vars.gauss_phase)
		phase = (phase + 1) % 4;
	script_byte_vars.gauss_phase = phase;
	sprite = gauss_data + 8 + phase * (8 * 30);
	CGA_Blit(sprite, 8, 8, 30, target, 80);	/*draw to 0:4*/
}

void interrupt (*old_timer_isr)();

void interrupt TimerIsr()
{
	disable();
	script_byte_vars.timer_ticks++;
	if(!script_byte_vars.game_paused)
	{
		if(script_byte_vars.timer_ticks % 16 == 0)
		{
			script_word_vars.timer_ticks2 = Swap16(Swap16(script_word_vars.timer_ticks2) + 1);
#if 1
			AnimateGauss(frontbuffer);	
#endif
		}
	}
	enable();
}

void InitTimer(void)
{
	disable();
	old_timer_isr = getvect(0x1C);
	setvect(0x1C, TimerIsr);
	enable();
}

void UninitTimer(void)
{
	disable();
	setvect(0x1C, old_timer_isr);
	enable();
}
