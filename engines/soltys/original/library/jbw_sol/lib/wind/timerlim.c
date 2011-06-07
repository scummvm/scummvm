#include	<wind.h>






long TimerLimitS (word sec)
{
  return TimerLimit((word)(Ticks((sec > 3600) ? 3600 : sec)));
}






long TimerLimit (word t)
{
  return (TimerCount + t) % 0x1800B0L;
}






Boolean TimerLimitGone (long t)
{
  long d;
  if (t < 0) return TRUE;
  d = TimerCount - t;

  asm	mov	ax,word ptr d+2
  asm	or	ax,ax
  asm	js	neg

  // positive number: up to 16hrs gone
  asm	test	ax,0FFF0h
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
