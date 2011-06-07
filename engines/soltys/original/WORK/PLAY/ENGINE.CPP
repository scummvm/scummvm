#include	"engine.h"
#include	<stdlib.h>
#include	<dos.h>

#define		TIMER_INT	0x08


void interrupt	(far * ENGINE::OldTimer) (...) = NULL;






ENGINE::ENGINE (void)
{
  // steal timer interrupt
  OldTimer = getvect(TIMER_INT);
  setvect(TIMER_INT, NewTimer);

  // set turbo-timer mode
  asm	mov	al,0x36
  asm	out	0x43,al
  asm	mov	ax,TMR_DIV
  asm	out	0x40,al
  asm	mov	al,ah
  asm	out	0x40,al
}




ENGINE::~ENGINE (void)
{
  // reset timer
  asm	mov	al,0x36
  asm	out	0x43,al
  asm	xor	al,al
  asm	out	0x40,al
  asm	out	0x40,al
  // bring back timer interrupt
  setvect(TIMER_INT, OldTimer);
}

