#include	<boot.h>
#include	<dos.h>


byte CheckBoot (Boot * boot)
{
  int i; byte n = 0;
  for (i = 0; i < BOOTSECT_SIZ; i ++) n += ((byte *) boot)[i];
  return n;
}
