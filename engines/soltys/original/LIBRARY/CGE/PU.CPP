#include	"general.h"
#include	<stdio.h>
#include	<dir.h>
#include	<io.h>
#include	<fcntl.h>


typedef	struct	{
		  byte R, G, B;
		} DAC;

typedef	struct	{
		  byte B;
		  byte G;
		  byte R;
		  byte Z;
		} BGR4;


#include	"stdpal.cpp"

#define		NN		ArrayCount(StdPal)


void UpdatePal (const char * fname)
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
  BGR4 bpal[256];
  char pat[MAXPATH];
  Boolean ok = FALSE;

  MergeExt(pat, fname, ".BMP");

  int f = _open(pat, O_RDWR | O_DENYALL);

  if (f >= 0)
    {
      int n = _read(f, &hea, sizeof(hea));
      if (n == sizeof(hea))
	{
	  if (hea.hdr == 0x436L)
	    {
	      _read(f, bpal, sizeof(bpal) - NN * sizeof(*bpal));
	      BGR4 * p = bpal + 256 - NN;
	      int i;
	      for (i = 0; i < NN; i ++)
		{
		  p[i].R = StdPal[i].R;
		  p[i].G = StdPal[i].G;
		  p[i].B = StdPal[i].B;
		  p[i].Z = 0;
		}
	      _write(f, p, NN * sizeof(*bpal));
	      ok = TRUE;
	    }
	}
      _close(f);
    }
  if (! ok)
    {
      puts("I/O error!");
    }
}




int main (int argc, char ** argv)
{
  if (argc > 1) UpdatePal(argv[1]);
  return 0;
}