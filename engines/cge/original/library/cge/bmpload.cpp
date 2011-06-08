#include	"bitmap.h"
#include	<alloc.h>
#include	<dos.h>
#include	<mem.h>


//--------------------------------------------------------------------------




Boolean BITMAP::BMPLoad (XFILE * f)
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

  f->Read((byte far *)&hea, sizeof(hea));
  if (f->Error == 0)
    {
      if (hea.hdr == 0x436L)
	{
	  word i = (hea.hdr - sizeof(hea)) / sizeof(BGR4);
	  f->Read((byte far *)&bpal, sizeof(bpal));
	  if (f->Error == 0)
	    {
	      if (Pal)
		{
		  for (i = 0; i < 256; i ++)
		    {
		      Pal[i].R = bpal[i].R;
		      Pal[i].G = bpal[i].G;
		      Pal[i].B = bpal[i].B;
		    }
		  Pal = NULL;
		}
	      H = hea.hig;
	      W = hea.wid;
	      if ((M = farnew(byte, H * W)) != NULL)
		{
		  word r = (4 - (hea.wid & 3)) % 4;
		  byte buf[3]; int i;
		  for (i = H-1; i >= 0; i --)
		    {
		      f->Read(M + (W * i), W);
		      if (r && f->Error == 0) f->Read(buf, r);
		      if (f->Error) break;
		    }
		  if (i < 0) return TRUE;
		}
	    }
	}
    }
  return FALSE;
}

