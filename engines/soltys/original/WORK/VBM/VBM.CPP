#include	<general.h>
#include	"bitmap.h"
#include	<cfile.h>
#include	<dir.h>
#include	<dos.h>
#include	<stdio.h>
#include	<conio.h>
#include	<stdlib.h>
#include	<alloc.h>
#include	<string.h>



typedef	struct	{ word r : 2; word R : 6;
		  word g : 2; word G : 6;
		  word b : 2; word B : 6;
		} RGB;

typedef	union	{
		  DAC dac;
		  RGB rgb;
		} TRGB;



#define		PAL_SIZ		(256*sizeof(DAC))


static	DAC	Palette[256];



void main (int argc, char **argv)
{
  if (argc < 2)
    {
      char nam[MAXFILE];
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is:    %s bmp_file [/p]\n", strupr(nam));
    }
  else
    {
      char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR];
      ffblk fb;
      int total = 0, i;

      strupr(MergeExt(pat, argv[1], ".BMP"));
      fnsplit(pat, drv, dir, NULL, NULL);

      for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
	{
	  char iname[MAXPATH], oname[MAXPATH], nam[MAXFILE], ext[MAXEXT];

	  fnsplit(fb.ff_name, NULL, NULL, nam, ext);
	  fnmerge(iname, drv, dir, nam, ext);
	  fnmerge(oname, NULL, NULL, nam, ".VBM");

	  printf("%s\n", nam);
	  if (argc > 2) BITMAP::Pal = Palette;
	  BITMAP bmp(iname);
	  CFILE f(oname, CFILE::WRI);
	  if (argc > 2) BITMAP::Pal = Palette;
	  bmp.VBMSave(&f);

	  ++ total;
	}
      printf("\nTotal %d file(s) processed.\n", total);
    }
}
