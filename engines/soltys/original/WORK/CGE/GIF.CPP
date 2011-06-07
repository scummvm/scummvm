#include	"vga13h.h"
#include	"keybd.h"
#include	"lz.h"
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


class GIF
{
  char Sig[6];
  struct { word ScrWid;
	   word ScrHig;
	   struct { word Bit : 3;
		    word _0_ : 1;
		    word Res : 3;
		    word Map : 1;
		  } Param;
	   byte Back;
	   byte Zero;
	 } ScrDesc;
  DAC * Pal;
  struct { char Sep;
	   word Lft;
	   word Top;
	   word Wid;
	   word Hig;
	   struct { word Bit : 3;
		    word _0_ : 3;
		    word Int : 1;
		    word Map : 1;
		  } Param;
	 } ImgDesc;

  public:
  BITMAP Pic;
  GIF (const char * fname);
  ~GIF (void);
//  byte * Map (void);
  DAC * TakePal (void);
};


//--------------------------------------------------------------------------



class LZW_ : public LZW
{
  word GetByte (void);

  public:
  LZW_::LZW_ (CFILE * input, CFILE * output);
};




LZW_::LZW_ (CFILE * input, CFILE * output)
{
  Input = input;
  Output = output;
  Clear();
}



word LZW_::GetByte (void)
{
  static byte buf[256];
  static byte ptr = 0, lim = 0;

  if (ptr >= lim)
    {
      lim = 0;
      Input->Read(&lim, 1);
      if (lim == 0) return NOTHING;
      Input->Read(buf, lim);
      ptr = 0;
    }
  return buf[ptr ++];
}





//--------------------------------------------------------------------------





static void Quit (void)
{
  Vga.Sunset();
  while (Scene)
    {
      SPRITE * s = Scene;
      Scene = Scene->Next;
      delete s;
    }
  exit(0);
}




//--------------------------------------------------------------------------






GIF::GIF (const char * fname)
{
  word n;
  Boolean ok = FALSE;
  char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE], ext[MAXEXT];

  Pal = NULL;
  Pic.M = NULL;
  n = fnsplit(fname, drv, dir, nam, ext);
  fnmerge(pat, drv, dir, nam, (n & EXTENSION) ? ext : "GIF");

  CFILE file(pat);
  if (! file.Error)
    {
      file.Read(Sig, sizeof(Sig));
      if (! file.Error && memcmp(Sig, "GIF", 3) == 0)
	{
	  file.Read(&ScrDesc, sizeof(ScrDesc));
	  if (! file.Error)
	    {
	      if (ScrDesc.Param.Map)
		{
		  n = 2 << ScrDesc.Param.Bit;
		  if (n != 256) goto xit;
		  Pal = new DAC[n];
		  if (Pal == NULL) goto xit;
		  file.Read((char *) Pal, n * sizeof(Pal[0]));
		}
	      if (! file.Error)
		{
		  file.Read(&ImgDesc, sizeof(ImgDesc));
		  if (! file.Error        &&
		      ! ImgDesc.Param.Map &&
		      ! ImgDesc.Param.Int    )
		    {
		      word psiz = (Pic.H=ImgDesc.Hig) * (Pic.W=ImgDesc.Wid);
		      Pic.M = new byte[psiz];
		      if (Pic.M != NULL)
			{
			  CFILE picture(Pic.M, psiz);
			  LZW_ lzw(&file, &picture);
			  byte codesiz;

			  file.Read(&codesiz, 1);
			  lzw.Unpack();
			  ok = TRUE;
			}
		    }
		}
	    }
	}
    }
  xit: if (! ok) exit(1);
}





GIF::~GIF (void)
{
  if (Pal) delete[] Pal;
}







DAC * GIF::TakePal (void)
{
  DAC * p;

  for (p = Pal; p < Pal+256; p ++)
    {
      p->R = p->R >> 2;
      p->G = p->G >> 2;
      p->B = p->B >> 2;
    }
  p = Pal;
  Pal = NULL;
  return p;
}










#pragma argsused
int main (int argc, char **argv)
{
  if (argc < 2) Quit();
  GIF gif(argv[1]);
  DAC * pal = gif.TakePal();
  SHP * shp[] = { new SHP(&gif.Pic), NULL };
  SPRITE spr(shp);

  Vga.Sunrise(pal);
  //spr->Append(Scene);
  spr.Show();
  Vga.Update();
  while (InKey() == 0);
  Vga.Sunset();

  return 0;
}