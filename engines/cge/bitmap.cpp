#include	"bitmap.h"
#include	<cfile.h>

#ifdef	VOL
  #include	"vol.h"
#endif

#ifdef	DROP_H
  #include	"drop.h"
#endif


#include	<alloc.h>
#include	<dos.h>
#include	<dir.h>
#include	<mem.h>


//--------------------------------------------------------------------------




DAC far *	BITMAP::Pal = NULL;



#pragma argsused
BITMAP::BITMAP (const char * fname, Boolean rem)
: M(NULL), V(NULL)
{
  char pat[MAXPATH];

  ForceExt(pat, fname, ".VBM");

  #if (BMP_MODE < 2)
  if (rem && PIC_FILE::Exist(pat))
    {
      PIC_FILE file(pat);
      if (file.Error == 0)
	if (! VBMLoad(&file))
	  DROP("Bad VBM", fname);
    }
  else
  #endif
    {
      #if (BMP_MODE)
      ForceExt(pat, fname, ".BMP");
      PIC_FILE file(pat);
      if (file.Error == 0)
	{
	  if (BMPLoad(&file))
	    {
	      Code();
	      if (rem)
		{
		  farfree(M);
		  M = NULL;
		}
	    }
	  else DROP("Bad BMP", fname);
	}
      #else
      DROP("Bad VBM", fname);
      #endif
    }
}





BITMAP::BITMAP (word w, word h, byte far * map)
: W(w), H(h), M(map), V(NULL)
{
  if (map) Code();
}




// following routine creates filled rectangle
// immediately as VGA video chunks, in near memory as fast as possible,
// especially for text line real time display

BITMAP::BITMAP (word w, word h, byte fill)
: W((w + 3) & ~3),		// only full dwords allowed!
  H(h),
  M(NULL)
{
  word dsiz = W >> 2;		// data size (1 plane line size)
  word lsiz = 2 + dsiz + 2;	// word for line header, word for gap
  word psiz = H * lsiz;		// - last gape, but + plane trailer
  byte * v = new byte[4 * psiz	// the same for 4 planes
	+ H * sizeof(*B)];	// + room for wash table
  if (v == NULL) DROP("No core", NULL);

  * (word *) v = CPY | dsiz;		// data chunk hader
  memset(v+2, fill, dsiz);		// data bytes
  * (word *) (v + lsiz - 2) = SKP | ((SCR_WID / 4) - dsiz); // gap
  memcpy(v + lsiz, v, psiz - lsiz);	// tricky replicate lines
  * (word *) (v + psiz - 2) = EOI;	// plane trailer word
  memcpy(v + psiz, v, 3 * psiz);	// tricky replicate planes
  HideDesc * b = (HideDesc *) (v + 4 * psiz);
  b->skip = (SCR_WID - W) >> 2;
  b->hide = W >> 2;
  memcpy(b+1, b, (H-1) * sizeof(*b));	// tricky fill entire table
  b->skip = 0;				// fix the first entry
  V = v;
  B = b;
}







BITMAP::BITMAP (const BITMAP& bmp)
: W(bmp.W), H(bmp.H),
  M(NULL), V(NULL)
{
  byte far * v0 = bmp.V;
  if (v0)
    {
      word vsiz = FP_OFF(bmp.B) - FP_OFF(v0);
      word siz = vsiz + H * sizeof(HideDesc);
      byte far * v1 = farnew(byte, siz);
      if (v1 == NULL) DROP("No core", NULL);
      _fmemcpy(v1, v0, siz);
      B = (HideDesc far *) ((V = v1) + vsiz);
    }
}





BITMAP::~BITMAP (void)
{
  switch (MemType(M))
    {
      case FAR_MEM  : farfree(M); break;
    }
  switch (MemType(V))
    {
      case NEAR_MEM : delete[] (byte *) V; break;
      case FAR_MEM  : farfree(V); break;
    }
}



BITMAP& BITMAP::operator = (const BITMAP& bmp)
{
  byte far * v0 = bmp.V;
  W = bmp.W;
  H = bmp.H;
  M = NULL;
  if (MemType(V) == FAR_MEM) farfree(V);
  if (v0 == NULL) V = NULL;
  else
    {
      word vsiz = FP_OFF(bmp.B) - FP_OFF(v0);
      word siz = vsiz + H * sizeof(HideDesc);
      byte far * v1 = farnew(byte, siz);
      if (v1 == NULL) DROP("No core", NULL);
      _fmemcpy(v1, v0, siz);
      B = (HideDesc far *) ((V = v1) + vsiz);
    }
  return *this;
}





word BITMAP::MoveVmap (byte far * buf)
{
  if (V)
    {
      word vsiz = FP_OFF(B) - FP_OFF(V);
      word siz = vsiz + H * sizeof(HideDesc);
      _fmemcpy(buf, V, siz);
      if (MemType(V) == FAR_MEM) farfree(V);
      B = (HideDesc far *) ((V = buf) + vsiz);
      return siz;
    }
  return 0;
}







BMP_PTR BITMAP::Code (void)
{
  if (M)
    {
      word i, cnt;

      if (V) // old X-map exists, so remove it
	{
	  switch (MemType(V))
	    {
	      case NEAR_MEM : delete[] (byte *) V; break;
	      case FAR_MEM  : farfree(V); break;
	    }
	  V = NULL;
	}

      while (TRUE) // at most 2 times: for (V == NULL) & for allocated block;
	{
	  byte far * im = V+2;
	  word far * cp = (word far *) V;
	  int bpl;

	  if (V) // 2nd pass - fill the hide table
	    {
	      for (i = 0; i < H; i ++)
		{
		  B[i].skip = 0xFFFF;
		  B[i].hide = 0x0000;
		}
	    }
	  for (bpl = 0; bpl < 4; bpl ++) // once per each bitplane
	    {
	      byte far * bm = M;
	      Boolean skip = (bm[bpl] == TRANS);
	      word j;

	      cnt = 0;
	      for (i = 0; i < H; i ++) // once per each line
		{
		  byte pix;
		  for (j = bpl; j < W; j += 4)
		    {
		      pix = bm[j];
		      if (V && pix != TRANS)
			{
			  if (j < B[i].skip) B[i].skip = j;
			  if (j >= B[i].hide) B[i].hide = j+1;
			}
		      if ((pix == TRANS) != skip || cnt >= 0x3FF0) // end of block
			{
			  cnt |= (skip) ? SKP : CPY;
			  if (V)
			    {
			      *cp = cnt;  // store block description word
			    }
			  cp = (word far *) im;
			  im += 2;
			  skip = (pix == TRANS);
			  cnt = 0;
			}
		      if (! skip)
			{
			  if (V) * im = pix;
			  ++ im;
			}
		      ++ cnt;
		    }

		  bm += W;
		  if (W < SCR_WID)
		    {
		      if (skip)
			{
			  cnt += (SCR_WID - j + 3) / 4;
			}
		      else
			{
			  cnt |= CPY;
			  if (V)
			    {
			      *cp = cnt;
			    }
			  cp = (word far *) im;
			  im += 2;
			  skip = TRUE;
			  cnt = (SCR_WID - j + 3) / 4;
			}
		    }
		}
	      if (cnt && ! skip)
		{
		  cnt |= CPY;
		  if (V)
		    {
		      *cp = cnt;
		    }
		  cp = (word far *) im;
		  im += 2;
		}
	      if (V) *cp = EOI;
	      cp = (word far *) im;
	      im += 2;
	    }
	  if (V) break;
	  word sizV = (word) (im - 2 - V);
	  V = farnew(byte, sizV + H * sizeof(*B));
	  if (! V)
	    {
	      DROP("No core", NULL);
	    }
	  B = (HideDesc far *) (V + sizV);
	}
      cnt = 0;
      for (i = 0; i < H; i ++)
	{
	  if (B[i].skip == 0xFFFF) // whole line is skipped
	    {
	      B[i].skip = (cnt + SCR_WID) >> 2;
	      cnt = 0;
	    }
	  else
	    {
	      word s = B[i].skip & ~3;
	      word h = (B[i].hide + 3) & ~3;
	      B[i].skip = (cnt + s) >> 2;
	      B[i].hide = (h - s) >> 2;
	      cnt = SCR_WID - h;
	    }
	}
    }
  return this;
}






Boolean BITMAP::SolidAt (int x, int y)
{
  byte far * m;
  word r, n, n0;

  if (x >= W || y >= H) return FALSE;

  m = V;
  r = x % 4;
  n0 = (SCR_WID * y + x) / 4, n = 0;

  while (r)
    {
      word w, t;

      w = * (word far *) m;
      m += 2;
      t = w & 0xC000;
      w &= 0x3FFF;

      switch (t)
	{
	  case EOI : -- r;
	  case SKP : w = 0; break;
	  case REP : w = 1; break;
	}
      m += w;
    }

  while (TRUE)
    {
      word w, t;

      w = * (word far *) m;
      m += 2;
      t = w & 0xC000;
      w &= 0x3FFF;

      if (n > n0) return FALSE;
      n += w;
      switch (t)
	{
	  case EOI : return FALSE;
	  case SKP : w = 0; break;
	  case REP :
	  case CPY : if (n-w <= n0 && n > n0) return TRUE; break;
	}
      m += (t == REP) ? 1 : w;
    }
}






Boolean BITMAP::VBMSave (XFILE * f)
{
  word p = (Pal != NULL),
       n = ((word) (((byte far *)B) - V)) + H * sizeof(HideDesc);
  if (f->Error == 0) f->Write((byte far *)&p, sizeof(p));
  if (f->Error == 0) f->Write((byte far *)&n, sizeof(n));
  if (f->Error == 0) f->Write((byte far *)&W, sizeof(W));
  if (f->Error == 0) f->Write((byte far *)&H, sizeof(H));
  if (f->Error == 0) if (p) f->Write((byte far *)Pal, 256 * sizeof(DAC));
  if (f->Error == 0) f->Write(V, n);
  return (f->Error == 0);
}





Boolean BITMAP::VBMLoad (XFILE * f)
{
  word p, n;
  if (f->Error == 0) f->Read((byte far *)&p, sizeof(p));
  if (f->Error == 0) f->Read((byte far *)&n, sizeof(n));
  if (f->Error == 0) f->Read((byte far *)&W, sizeof(W));
  if (f->Error == 0) f->Read((byte far *)&H, sizeof(H));
  if (f->Error == 0)
    {
      if (p)
	{
	  if (Pal) f->Read((byte far *)Pal, 256 * sizeof(DAC));
	  else f->Seek(f->Mark() + 256 * sizeof(DAC));
	}
    }
  if ((V = farnew(byte, n)) == NULL) return FALSE;
  if (f->Error == 0) f->Read(V, n);
  B = (HideDesc far *) (V + n - H * sizeof(HideDesc));
  return (f->Error == 0);
}





