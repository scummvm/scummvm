#include	<general.h>
#include	"vga13h.h"
#include	"snail.h"
#include	"cfile.h"
#include	"vbmview.h"
#include	<alloc.h>
#include	<conio.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dos.h>
#include	<dir.h>
#include	<fcntl.h>
#include	<bios.h>
#include	<io.h>
#include	"..\stdpal.cpp"

extern	char	Copr[] = "VBM file viewer 1.00 by JBW";

//--------------------------------------------------------------------------

	DAC	SysPal[PAL_CNT];

//-------------------------------------------------------------------------



static void SetPal (void)
{
  int i;
  DAC * p = SysPal + 256-ArrayCount(StdPal);
  for (i = 0; i < ArrayCount(StdPal); i ++)
    {
      if (p[i].R) return;
      if (p[i].G) return;
      if (p[i].B) return;
    }
  for (i = 0; i < ArrayCount(StdPal); i ++)
    {
      p[i].R = StdPal[i].R >> 2;
      p[i].G = StdPal[i].G >> 2;
      p[i].B = StdPal[i].B >> 2;
    }
}




/*
static void SwitchColorMode (void)
{
  SNPOST(SNBACKPT, 121, VGA::Mono = ! VGA::Mono, NULL);
  VGA::SetColors(SysPal, 64);
}
*/




void ShowVBM (const char * name)
{
  VGA Vga = M13H;
  memset(SysPal, 0, sizeof(SysPal));
  BITMAP::Pal = SysPal;
  BMP_PTR LB[] =  { new BITMAP(name), NULL };
  SPRITE D(LB);
  SetPal();
  D.Flags.BDel = TRUE;
  D.Center();
  D.Show();
  Vga.Update();
  Vga.Sunrise(SysPal);
  getch();
  Vga.Sunset();
}





void main (int argc, char **argv)
{
  if (argc < 2) printf("Syntax is:    %s vbm_file\n", strupr((char *)ProgName()));
  else ShowVBM(argv[1]);
}
