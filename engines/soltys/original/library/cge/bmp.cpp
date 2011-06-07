#include	<general.h>
#include	"cfile.h"
#include	<dir.h>
#include	<dos.h>
#include	<stdio.h>
#include	<conio.h>
#include	<stdlib.h>
#include	<alloc.h>
#include	<string.h>



#define		IsFile(s)	(access(s,0)==0)
#define		IsWrit(s)	(access(s,2)==0)
#define		OK(f)		(((f).Error)==0)


typedef	struct	{
		  byte R, G, B;
		} DAC;

typedef	struct	{ word r : 2; word R : 6;
		  word g : 2; word G : 6;
		  word b : 2; word B : 6;
		} RGB;

typedef	struct	{ word b : 2;
		  word B : 6;
		  word g : 2;
		  word G : 6;
		  word r : 2;
		  word R : 6;
		  word Z : 8;
		} BGR4;

typedef	union	{
		  DAC dac;
		  RGB rgb;
		} TRGB;



#define		PAL_SIZ		(256*sizeof(DAC))



class BITMAP
{
  struct {
	   char BM[2];
	   union { word len; dword len_; };
	   union { word _06; dword _06_; };
	   union { word hdr; dword hdr_; };
	   union { word _0E; dword _0E_; };
	   union { word wid; dword wid_; };
	   union { word hig; dword hig_; };
	   union { word _1A; dword _1A_; };
	   union { word _1E; dword _1E_; };
	   union { word _22; dword _22_; };
	   union { word _26; dword _26_; };
	   union { word _2A; dword _2A_; };
	   union { word _2E; dword _2E_; };
	   union { word _32; dword _32_; };
	 } hea;
  BGR4 pal[256];
  char pat[MAXPATH];
public:
  Boolean Ok;
  word W, H;
  byte far * M;
  BITMAP (const char * fname);
  ~BITMAP (void);
  Boolean Save (void);
};






BITMAP::BITMAP (const char * fname) : M(NULL), Ok(FALSE)
{
  MergeExt(pat, fname, ".BMP");

  CFILE file(pat);

  if (OK(file))
    {
      file.Read(&hea, sizeof(hea));
      if OK(file)
	{
	  if (hea.hdr == 0x436L)
	    {
	      //word i = (hea.hdr - sizeof(hea)) / sizeof(BGR4);
	      file.Read(pal, sizeof(pal));
	      if (OK(file))
		{
		  H = hea.hig;
		  W = hea.wid;
		  if (W <= 320 && H <= 200)
		    {
		      M = farnew(byte, H * W);
		      if (M)
			{
			  word r = (4 - (hea.wid & 3)) % 4;
			  byte buf[3]; int i;
			  for (i = H-1; i >= 0; i --)
			    {
			      file.Read(M + (W * i), W);
			      if (r && OK(file)) file.Read(buf, r);
			      if (! OK(file)) break;
			    }
			  if (i < 0) Ok = TRUE;
			}
		    }
		}
	    }
	}
    }
}






BITMAP::~BITMAP (void)
{
  if (M) farfree(M);
}








Boolean BITMAP::Save (void)
{
  Boolean ok = FALSE;
  CFILE file(pat, CFW);

  if (OK(file))
    {
      file.Write(&hea, sizeof(hea));
      if OK(file)
	{
	  file.Write(pal, sizeof(pal));
	  if (OK(file))
	    {
	      word r = (4 - (hea.wid & 3)) % 4;
	      byte buf[3] = { 0, 0, 0 }; int i;
	      for (i = H-1; i >= 0; i --)
		{
		  file.Write(M + (W * i), W);
		  if (r && OK(file)) file.Write(buf, r);
		  if (! OK(file)) break;
		}
	      if (i < 0) ok = TRUE;
	    }
	}
    }
  return ok;
}






void ListColors (char * mask)
{
  int i;
  struct ffblk fb;
  char pat[MAXPATH];

  printf("Scanning file(s) for all used colors...\n\n");

  MergeExt(pat, mask, ".BMP");
  int total = 0;

  for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
    {
      BITMAP bmp = fb.ff_name;
      if (bmp.Ok)
	{
	  word n = bmp.H * bmp.W, i;
	  int count = 0;
	  byte use[256];

	  printf("%-16s", fb.ff_name);
	  memset(use, 0, sizeof(use));
	  for (i = 0; i < n; i ++) use[bmp.M[i]] = TRUE;
	  for (i = 0; i < ArrayCount(use); i ++) count += use[i];
	  printf("- %3d colors used\n", count);

	  i = fnsplit(pat,NULL,NULL,NULL,NULL);
	  if ((i & WILDCARDS) == 0)
	    {
	      n = 0;
	      Boolean first = TRUE;

	      while (n < ArrayCount(use))
		{
		  for (i = n+1; i < ArrayCount(use) && use[i] == use[n]; i ++) ;
		  if (use[n])
		    {
		      if (first)
			{
			  printf("    ");
			  count = 4;
			  first = FALSE;
			}
		      else
			{
			  printf(", ");
			  count += 2;
			  if (count > 70)
			    {
			      printf("\n    ");
			      count = 4;
			    }
			}

		      if (i == n+1)		// one color
			{
			  printf("%d", n);
			  count += 1 + (n > 9) + (n > 99);
			}
		      else			// range of colors
			{
			  printf("%d\-%d", n, i-1);
			  count += 3 + (n > 9) + (n > 99) + (i > 10) + (i > 100);
			}
		    }
		  n = i;
		}
	      if (count > 4) printf("\n");
	    }
	  ++ total;
	}
      else printf("*** Warning: unable to read %s\n", fb.ff_name);
    }
  printf("\n%d file(s) listed.\n", total);
}







void CheckColor (char * mask, int color)
{
  int i;
  struct ffblk fb;
  char pat[MAXPATH];

  printf("Scanning file(s) for color #%d...\n\n", color);

  MergeExt(pat, mask, ".BMP");
  int count = 0, total = 0;

  for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
    {
      BITMAP bmp = fb.ff_name;
      if (bmp.Ok)
	{
	  if (_fmemchr(bmp.M, color, bmp.H * bmp.W))
	    {
	      printf("%s\n", fb.ff_name);
	      ++ count;
	    }
	  ++ total;
	}
      else printf("*** Warning: unable to read %s\n", fb.ff_name);
    }
  printf("\n%d file(s) found of total %d checked.\n", count, total);
}






#pragma argsused
void RemapColor (char * mask, int col0, int col1)
{
  int i;
  struct ffblk fb;
  char pat[MAXPATH];
  int count = 0, total = 0;

  printf("Do you wish to remap color %d to color %d in all that files? ", col1, col0);
  i = getch();
  if (i == 'Y' || i == 'y')
    {
      printf("Yes\n");

      MergeExt(pat, mask, ".BMP");

      for (i = findfirst(pat, &fb, 0); i == 0; i = findnext(&fb))
	{
	  BITMAP bmp = fb.ff_name;
	  if (bmp.Ok)
	    {
	      word n = bmp.H * bmp.W, i;

	      if (_fmemchr(bmp.M, col1, n))
		{
		  printf("%s\n", fb.ff_name);
		  for (i = 0; i < n; i ++)
		    {
		      if (bmp.M[i] == col1) bmp.M[i] = col0;
		    }
		  if (bmp.Save()) ++ count;
		  else printf("*** Warning: unable to write changes");
		}
	      ++ total;
	    }
	  else printf("*** Warning: unable to read %s\n", fb.ff_name);
	}
    }
  else printf("No\n");
  printf("\n%d file(s) remapped of total %d checked.\n", count, total);
}







void main (int argc, char **argv)
{
  switch (argc)
    {
      case 0 :
      case 1 : char nam[MAXPATH];
	       fnsplit(argv[0], NULL, NULL, nam, NULL);
	       printf("Syntax is    %s filespec [color] [color]\n", strupr(nam));
	       exit(1);
      case 2 : ListColors(argv[1]); break;
      case 3 : CheckColor(argv[1], atoi(argv[2])); break;
      case 4 : RemapColor(argv[1], atoi(argv[2]), atoi(argv[3])); break;
    }
}
