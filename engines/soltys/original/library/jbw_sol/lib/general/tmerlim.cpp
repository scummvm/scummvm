#include	<general.h>




long TimerLimit (word t)
{
  return (Timer() + t) % MAX_TIMER;
}






Boolean TimerLimitGone (long t)
{
  long d;
  if (t < 0) return TRUE;
  d = Timer() - t;

  asm	mov	ax,word ptr d+2
  asm	or	ax,ax
  asm	js	neg

  // positive number: up to 16hrs gone
  asm	test	ax,0xFFF0
  asm	jz	ret_TRUE

  ret_FALSE:
  return FALSE;

  // negative number: up to 8hrs not gone
  neg:
  asm	cmp	ax,-8
  asm	jge	ret_FALSE

  ret_TRUE:
  return TRUE;
}






