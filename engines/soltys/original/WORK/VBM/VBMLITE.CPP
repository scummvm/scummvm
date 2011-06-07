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


static	DAC	Pal0[256] = { {0, 0, 0} }, Palette[256];



void Mov (word far *& h1, word far *& h0, int n)
{
  _fmemcpy(h1, h0, 2+n);
  h0 = (word far *) (((byte far *) (h0+1)) + n);
  h1 = (word far *) (((byte far *) (h1+1)) + n);
}



void Compr (word far *& h1, word far *& h0, word n)
{
  byte far * d0 = (byte far *) (h0+1), far * d1 = (byte far *) (h1+1);
  int i = 0, rep = 0;
  byte recent = *d0 ^ 0xFF;

  while (i <= n)
    {
      byte current = d0[i];
      if (current == recent && i < n) ++ rep;
      else
	{
	  Boolean repblk = rep > ((rep == i || i == n) ? 3 : 5);
	  if (i == n || repblk)
	    {
	      int k = (repblk) ? (i - rep) : i;
	      if (k)
		{
		  *h1 = CPY | k;
		  _fmemcpy(d1, d0, k);
		  d0 += k;
		  h1 = (word far *) (d1+k);
		  d1 = (byte far *) (h1+1);
		  n -= k;
		}
	      if (repblk)
		{
		  *h1 = REP | rep;
		  *d1 = recent;
		  d0 += rep;
		  h1 = (word far *) (d1+1);
		  d1 = (byte far *) (h1+1);
		  n -= rep;
		}
	      i = 0;
	    }
	  recent = current;
	  rep = 1;
	}
      ++ i;
    }
  h0 = (word far *) d0;
}



static void Compress (BITMAP& bmp)
{
  word far * hea0 = (word far *) (bmp.V), far * hea1 = hea0;
  int bpl;
  for (bpl = 0; bpl < 4; )
    {
      word siz0 = *hea0 & 0x3FFF;
      switch (*hea0 & 0xC000)
	{
	  case EOI : ++ bpl;
	  case SKP : Mov(hea1, hea0, 0); break;
	  case REP : Mov(hea1, hea0, 1); break;
	  case CPY : Compr(hea1, hea0, siz0); break;
	}
    }
  _fmemcpy(hea1, bmp.B, bmp.H * sizeof(HideDesc));
  bmp.B = (HideDesc far *) hea1;
}



void main (int argc, char **argv)
{
  if (argc < 2)
    {
      char nam[MAXFILE];
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is:    %s vbm_file\n", strupr(nam));
    }
  else
    {
      char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR];
      ffblk fb;
      int total = 0, i;

      strupr(MergeExt(pat, argv[1], ".VBM"));
      fnsplit(pat, drv, dir, NULL, NULL);

      for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
	{
	  char iname[MAXPATH], oname[MAXPATH], nam[MAXFILE], ext[MAXEXT];

	  fnsplit(fb.ff_name, NULL, NULL, nam, ext);
	  fnmerge(iname, drv, dir, nam, ext);
	  fnmerge(oname, NULL, NULL, nam, ".VBM");

	  printf("%s\n", nam);
	  memcpy(Palette, Pal0, sizeof(Palette));
	  BITMAP::Pal = Palette;
	  BITMAP bmp(iname);
	  Compress(bmp);
	  CFILE f(oname, CFILE::WRI);
	  BITMAP::Pal = (memcmp(Palette, Pal0, sizeof(Palette))) ? (DAC far *) Palette
								 : (DAC far *) NULL;
	  bmp.VBMSave(&f);

	  ++ total;
	}
      printf("\nTotal %d file(s) processed.\n", total);
    }
}
