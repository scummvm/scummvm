#include	"vga13h.h"
#include	"timer.h"
#include	"cfile.h"
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


#define		OK(f)		((f).Error==0)

//--------------------------------------------------------------------------





static	VgaRegBlk VideoMode[] = {

		    { 0x04, VGASEQ, 0x08, 0x04 },	// memory mode

		    { 0x03, VGAGRA, 0xFF, 0x00 },	// data rotate = 0
		    { 0x05, VGAGRA, 0x03, 0x00 },	// R/W mode = 0
		    { 0x06, VGAGRA, 0x02, 0x00 },	// misc

		    { 0x14, VGACRT, 0x40, 0x00 },	// underline
		    { 0x13, VGACRT, 0xFF, 0x28 },	// screen width
		    { 0x17, VGACRT, 0xFF, 0xC3 },	// mode control

		    { 0x11, VGACRT, 0x80, 0x00 },	// vert retrace end
		    { 0x09, VGACRT, 0xEF, 0x01 },	// max scan line

		    { 0x30, VGAATR, 0x00, 0x20 },	// 256 color mode

//		    { 0x12, VGACRT, 0xFF, 0x6E },	// vert display end
//		    { 0x15, VGACRT, 0xFF, 0x7F },	// start vb
//		    { 0x10, VGACRT, 0xFF, 0x94 },	// start vr

		    { 0x00                     } };


extern	Boolean		SpeedTest	= FALSE;




static void Video (void)
{
  static word SP_S;

  asm	push	bx
  asm	push	bp
  asm	push	si
  asm	push	di
  asm	push	es
  asm	xor	bx,bx		// video page #0
  SP_S = _SP;
  asm	int	VIDEO
  _SP = SP_S;
  asm	pop	es
  asm	pop	di
  asm	pop	si
  asm	pop	bp
  asm	pop	bx
}






word far * SaveScreen (void)
{
  word cxy, cur, siz, far * scr = NULL, far * sav;

  // horizontal size of text mode screen
  asm	mov	ah,0x0F		// get current video mode
  Video();			// BIOS video service
  asm	xchg	ah,al		// answer in ah
  asm	push	ax		// preserve width

  // vertical size of text mode screen
  asm	mov	dl,24		// last row on std screen
  asm	xor	bx,bx		// valid request in BH
  asm	mov	ax,0x1130	// get EGA's last row #
  Video();			// BIOS video service
  asm	inc	dl		// # of rows = last+1

  // compute screen size in words
  asm	pop	ax		// restore width
  asm	mul	dl		// width * height

  siz = _AX;

  asm	mov	ax,0x40		// system data segment
  asm	mov	es,ax
  asm	mov	ax,0B000H	// Mono
  asm	cmp	byte ptr es:[0x49],0x07
  asm	je	sto
  asm	mov	ax,0B800H	// Color
  sto:				// store screen address
  asm	mov	word ptr scr+2,ax

  _AH = 0x0F; Video();		// active page

  // take cursor shape
  _AH = 0x03; Video();		// get cursor size
  cur = _CX;

  // take cursor position
  _DH = 0;
  _AH = 0x03; Video();		// get cursor
  cxy = _DX;

  sav = farnew(word, siz+3);	// +3 extra words for size and cursor
  if (sav)
    {
      sav[0] = siz;
      sav[1] = cur;
      sav[2] = cxy;
      _fmemcpy(sav+3, scr, siz * 2);
    }
  return sav;
}





void RestoreScreen (word far * &sav)
{
  word far * scr = NULL;

  asm	mov	ax,0x40		// system data segment
  asm	mov	es,ax
  asm	mov	ax,0B000H	// Mono
  asm	cmp	byte ptr es:[0x49],0x07
  asm	je	sto
  asm	mov	ax,0B800H	// Color
  sto:				// store screen address
  asm	mov	word ptr scr+2,ax

  _fmemcpy(scr, sav+3, sav[0] * 2);

  _AH = 0x0F; Video();		// active page

  // set cursor shape
  _CX = sav[1];
  _AH = 0x01; Video();		// set cursor size

  // set cursor position
  _DX = sav[2];
  _AH = 0x02; Video();		// set cursor

  farfree(sav);
  sav = NULL;
}






DAC MkDAC (byte r, byte g, byte b)
{
  static DAC x;
  x.R = r;
  x.G = g;
  x.B = b;
  return x;
}




RGB MkRGB (byte r, byte g, byte b)
{
  static TRGB x;
  x.dac.R = r;
  x.dac.G = g;
  x.dac.B = b;
  return x.rgb;
}






void Unlink (SPRITE * spr)
{
  if (! spr->Flags.Remo)
    {
      SPRITE * next = spr->Next;
      if (next) if (next->Flags.Slav) Unlink(next);
      spr->Delay = 2;
      spr->Repaint = 2;
      spr->Flags.Remo = TRUE;
    }
}




//--------------------------------------------------------------------------




SPRITE *	SPRITE::Head = NULL;
SPRITE *	SPRITE::Tail = NULL;




SPRITE::SPRITE (BITMAP ** shp)
: ShpList(shp), X(0), Y(0), Repaint(2),
  Next(NULL), Prev(NULL), Phase(0), Time(0), Delay(0),
  lA(0), lB(0), lC(0), lD(0), lT(0),
  x0(0), x1(0), y0(0), y1(0)
{
  *((word *)&Flags) = 0;
  *Name = '\0';
  CountUp();
}



BITMAP ** SPRITE::SetShapeList (BITMAP ** shp)
{
  BITMAP ** r = ShpList;
  ShpList = shp;
  CountUp();
  return r;
}




void SPRITE::SetName (char * n)
{
  memcpy(Name, n, MAX_NAME);
  Name[MAX_NAME] = '\0';
}




void SPRITE::Tick (void)
{
  if (Flags.Auto) Step();
}



void SPRITE::CountUp (void)
{
  W = 0;
  H = 0;
  ShpCnt = 0;
  if (ShpList)
    {
      while (ShpList[ShpCnt])
	{
	  BITMAP * s = ShpList[ShpCnt]->Code(Flags.Keep);
	  if (s->W > W) W = s->W;
	  if (s->H > H) H = s->H;
	  ++ ShpCnt;
	}
    }
}





BITMAP * SPRITE::Shp (void)
{
  return ShpList[Phase];
}





void SPRITE::MakeXlat (byte far * x)
{
  if (ShpList)
    {
      int i;
      for (i = 0; i < ShpCnt; i ++)
	{
	  ShpList[i]->M = x;
	}
      Flags.Xlat = TRUE;
      Repaint = 2;
    }
}





void SPRITE::KillXlat (void)
{
  if (ShpList)
    {
      word i;
      if (ShpCnt) delete[] ShpList[0]->M;
      for (i = 0; i < ShpCnt; i ++)
	{
	  ShpList[i]->M = NULL;
	}
      Flags.Xlat = FALSE;
      Repaint = 2;
    }
}





void SPRITE::Goto (int x, int y)
{
  int x0 = X, y0 = Y;
  if (W < SCR_WID)
    {
      if (x < 0) x = 0;
      if (x + W > SCR_WID) x = (SCR_WID - W);
      X = x;
    }
  if (H < SCR_HIG)
    {
      if (y < 0) y = 0;
      if (y + H > SCR_HIG) y = (SCR_HIG - H);
      Y = y;
    }
  Repaint = 2;
  if (Next) if (Next->Flags.Slav) Next->Goto(Next->X-x0+X, Next->Y-y0+Y);
}












void SPRITE::Center (void)
{
  Goto((SCR_WID - W) / 2, (SCR_HIG - H) / 2);
}








void SPRITE::Show (void)
{
  if (Repaint)
    {
      BITMAP * shp = Shp();
      asm	cli		// critic section...
      x0 = x1;
      y0 = y1;
      x1 = X;
      y1 = Y;
      asm	sti		// ...done!
      if (Flags.Remo)
	{
	  if (Delay) -- Delay;
	}
      else
	{
	  if (Flags.Xlat) shp->XShow(x1, y1);
	  else shp->Show(x1, y1);
	}
      -- Repaint;
    }
}







void SPRITE::Show (word pg)
{
  byte far * a = VGA::Page[1];
  VGA::Page[1] = VGA::Page[pg & 3];
  Shp()->Show(X, Y);
  VGA::Page[1] = a;
}





void SPRITE::Hide (void)
{
  if (Repaint)
    {
      word p = x0 % 4,
	   o = y0 * (SCR_WID / 4) + x0 / 4,
	   w = (p + W + 3) / 4,
	   h = H;
      byte far * scr = VGA::Page[1] + o,
	   far * bak = VGA::Page[2] + o;

	    asm	push	ds

	    asm	cld
	    asm	les	di,scr
	    asm	lds	si,bak

	    asm	mov	bx,SCR_WID_/4	// screen width
	    asm	sub	bx,w		// from end of row to next

	    asm	mov	dx,VGAGRA_
	    asm	mov	al,0x05		// R/W mode
	    asm	out	dx,al
	    asm	inc	dx
	    asm	in	al,dx
	    asm	and	al,0xF4
	    asm	push	ax
	    asm	push	dx
	    asm	or	al,0x01
	    asm	out	dx,al

	    asm	mov	dx,VGASEQ_
	    asm	mov	ax,0x0F02	// enable all planes
	    asm	out	dx,ax

      // copy background

	    asm	mov	cx,h
      row:
	    asm	push	cx
	    asm	mov	cx,w
	    asm	rep movsb
	    asm	pop	cx
	    asm	add	si,bx
	    asm	add	di,bx
	    asm	loop	row

	    asm	pop	dx
	    asm	pop	ax
	    asm	out	dx,al		// end of copy mode

	    asm	pop	ds
    }
}







void SPRITE::Append (void)
{
  if (Tail)
    {
      Prev = Tail;
      Tail->Next = this;
    }
  else Head = this;
  Tail = this;
  Repaint = 2;
}





void SPRITE::Insert (SPRITE * spr)
{
  if (spr == Head)
    {
      Next = Head;
      Head = this;
      if (! Tail) Tail = this;
    }
  else
    {
      Next = spr;
      Prev = spr->Prev;
      if (Prev) Prev->Next = this;
    }
  if (Next) Next->Prev = this;
  Repaint = 2;
}





SPRITE * SPRITE::Remove (void)
{
  if (this == Head) Head = Next;
  if (this == Tail) Tail = Prev;
  if (Next) Next->Prev = Prev;
  if (Prev) Prev->Next = Next;
  Prev = NULL;
  Next = NULL;
  return this;
}





SPRITE * SPRITE::At (int x, int y)
{
  SPRITE * spr;
  for (spr = SPRITE::Tail; spr; spr = spr->Prev)
    {
      if (spr->Flags.Mice) continue;
      if (spr->Shp()->SolidAt(x-spr->X, y-spr->Y)) break;
    }
  return spr;
}




//--------------------------------------------------------------------------





const char *	VGA::Msg = "Unrecognized exception ";
const char *	VGA::Nam = NULL;
DAC		VGA::OldColors[256], VGA::NewColors[256];
Boolean		VGA::SetPal = FALSE;
int		VGA::Mono = 0;
byte far *	VGA::Page[4] = { (byte far *) MK_FP(SCR_SEG, 0x0000),
				 (byte far *) MK_FP(SCR_SEG, 0x4000),
				 (byte far *) MK_FP(SCR_SEG, 0x8000),
				 (byte far *) MK_FP(SCR_SEG, 0xC000) };




VGA::VGA (int mode)
: FrmCnt(0)
{
  extern const char Copr[];
  puts(Copr);
  GetColors(OldColors);
  OldScreen = SaveScreen();
  Sunset();
  OldMode = SetMode(mode);
  SetColors();
  Setup(VideoMode);
  Clear();
}





VGA::~VGA (void)
{
  Clear();
  VGA::Mono = 0;
  SetMode(OldMode);
  SetColors();
  RestoreScreen(OldScreen);
  Sunrise(OldColors);
  if (Msg) fputs(Msg, stderr);
  if (Nam)
    {
      fputs(" [", stderr);
      fputs(Nam, stderr);
      fputc(']', stderr);
    }
  if (Msg || Nam) fputc('\n', stderr);
}






#pragma argsused
void VGA::WaitVR (Boolean on)
{
  _AH = (on) ? 0x00 : 0x08;

  asm	mov	dx,VGAST1_
  asm	mov	cx,2
  // wait for vertical retrace on (off)
  wait:
  asm	in	al,dx
  asm	xor	al,ah
  asm	test	al,0x08
  asm	jnz	wait
  asm	xor	ah,0x08
  asm	loop	wait
}






void VGA::Setup (VgaRegBlk * vrb)
{
  WaitVR();			// *--LOOK!--* resets VGAATR logic
  asm	cld
  asm	mov	si, vrb		// take address of parameter table
  asm	mov	dh,0x03		// higher byte of I/O address is always 3

  s:
  asm	lodsw			// take lower byte of I/O address and index
  asm	or	ah,ah		// 0 = end of table
  asm	jz	xit		// no more: exit
  asm	or	al,al		// indexed register?
  asm	js	single		// 7th bit set means single register
  asm	mov	dl,ah		// complete I/O address
  asm	out	dx,al		// put index into control register
  asm	inc	dx		// data register is next to control
  asm	in	al,dx		// take old data

  write:
  asm	mov	cl,al		// preserve old data
  asm	lodsw			// take 2 masks from table
  asm	xor	al,0xFF		// invert mask bits
  asm	and	al,cl		// clear bits with "clr" mask
  asm	or	al,ah		// set bits with "set" mask
  asm	cmp	dl,0xC1		// special case?
  asm	jne	std2		// no: standard job, otherwise...
  asm	dec	dx		// data out reg shares address with index
  std2:
  asm	out	dx,al		// write new value to register
  asm	jmp	s

  single:			// read address in al, write address in ah
  asm	mov	dl,al		// complete I/O read address
  asm	in	al,dx		// take old data
  asm	mov	dl,ah		// complete I/O write address
  asm	jmp	write		// continue standard routine

  xit:
}






int VGA::SetMode (int mode)
{
  Clear();
  // get current mode
  asm	mov	ah,0x0F
  Video();			// BIOS video service
  asm	xor	ah,ah
  asm	push	ax

  // wait for v-retrace
  WaitVR();

  // set mode
  asm	xor	ah,ah
  asm	mov	al,byte ptr mode
  Video();			// BIOS video service
  // return previous mode
  asm	pop	ax
  return _AX;
}






void VGA::GetColors (DAC * tab)
{
  asm	cld
  asm	mov	di,ds
  asm	mov	es,di
  asm	mov	di,tab		// color table
  asm	mov	dx,0x3C7	// PEL address read mode register
  asm	xor	al,al		// start from address 0
  asm	out	dx,al		// put address
  asm	mov	cx,256*3	// # of colors
  asm	mov	dl,0xC9		// PEL data register
  gc:
  asm	in	al,dx		// take 1 color
  asm	jmp	sto		// little delay
  sto:
  asm	stosb			// store 1 color
  asm	loop	gc		// next one?
}




void VGA::SetColors (DAC * tab, int lum)
{
  int i;
  for (i = 0; i < ArrayCount(NewColors); i ++)
    {
      NewColors[i].R = (tab[i].R * lum) >> 6;
      NewColors[i].G = (tab[i].G * lum) >> 6;
      NewColors[i].B = (tab[i].B * lum) >> 6;
      if (Mono)
	{
	  word n;
	  if (Mono == 1)
	    {
	      n = (NewColors[i].R*30 + NewColors[i].G*59 + NewColors[i].B*11) / 100;
	    }
	  else
	    {
	      n = (NewColors[i].R + NewColors[i].G + NewColors[i].B) / 3;
	    }
	  NewColors[i].R = (byte) n;
	  NewColors[i].G = (byte) n;
	  NewColors[i].B = (byte) n;
	}
    }
  SetPal = TRUE;
}






void VGA::SetColors (void)
{
  memset(NewColors, 0, sizeof(NewColors));
  UpdateColors();
}






void VGA::Sunrise (DAC * tab)
{
  int i;
  for (i = 1; i <= 64; i ++)
    {
      SetColors(tab, i);
      WaitVR();
      UpdateColors();
    }
}






void VGA::Sunset (void)
{
  DAC tab[256];
  int i;
  GetColors(tab);
  for (i = 63; i >= 0; i --)
    {
      SetColors(tab, i);
      WaitVR();
      UpdateColors();
    }
}









void VGA::Show (void)
{
  SPRITE * spr = SPRITE::First();

  while (spr)
    {
      SPRITE * s = spr;
      spr = s->Next;
      if (s->Flags.Back) { s->Show(2); s->Flags.Back = FALSE; }
      s->Show();
      if (s->Flags.Remo)
	{
	  if (s->Delay == 0)
	    {
	      s->Remove();
	      s->Flags.Remo = FALSE;
	      if (s->Flags.Kill) delete s;
	    }
	}
    }
  Update();
  for (spr = SPRITE::First(); spr; spr = spr->Next) spr->Hide();
  ++ FrmCnt;
}




void VGA::UpdateColors (void)
{
  DAC * tab = NewColors;

  asm	cld
  asm	mov	si,tab		// color table
  asm	mov	dx,0x3C8	// PEL address write mode register
  asm	xor	al,al		// start from address 0
  asm	out	dx,al		// put address
  asm	mov	cx,256*3	// # of colors
  asm	mov	dl,0xC9		// PEL data register
  sc:
  asm	lodsb			// take 1/3 color
  asm	out	dx,al		// put 1/3 color
  asm	jmp	nxt		// little delay
  nxt:
  asm	loop	sc		// next one?
}







void VGA::Update (void)
{
  byte far * p = Page[1];
  Page[1] = Page[0];
  Page[0] = p;

  asm	mov	dx,VGACRT_
  asm	mov	al,0x0D
  asm	mov	ah,byte ptr p
  asm	out	dx,ax
  asm	dec	al
  asm	mov	ah,byte ptr p+1
  asm	out	dx,ax

  if (! SpeedTest) WaitVR();

  if (SetPal)
    {
      UpdateColors();
      SetPal = FALSE;
    }
}








void VGA::Clear (byte color)
{
  byte far * a = (byte far *) MK_FP(SCR_SEG, 0);

  asm	mov	dx,VGASEQ_
  asm	mov	ax,0x0F02	// map mask register - enable all planes
  asm	out	dx,ax
  asm	les	di,a
  asm	cld

  //asm	mov	cx,0xFFFF
  asm	mov	cx,0xC000
  asm	mov	al,color
  asm	rep stosb
  //asm	stosb
}






void VGA::CopyPage (word d, word s)
{
  byte far * S = Page[s & 3], far * D = Page[d & 3];

  asm	mov	dx,VGAGRA_
  asm	mov	al,0x05		// R/W mode
  asm	out	dx,al
  asm	inc	dx
  asm	in	al,dx
  asm	and	al,0xF4
  asm	push	ax
  asm	push	dx
  asm	or	al,0x01
  asm	out	dx,al

  asm	mov	dx,VGASEQ_
  asm	mov	ax,0x0F02	// map mask register - enable all planes
  asm	out	dx,ax

  asm	push	ds

  asm	lds	si,S
  asm	les	di,D
  asm	cld
  asm	mov	cx,0x4000
  asm	rep movsb

  asm	pop	ds

  asm	pop	dx
  asm	pop	ax
  asm	out	dx,al		// end of copy mode
}







void VGA::Exit (const char * txt, const char * name)
{
  Msg = txt;
  Nam = name;
  exit(0);
}




//--------------------------------------------------------------------------



DAC	BITMAP::Pal[256];




BITMAP::BITMAP (const char * fname, Boolean mirr)
: V(NULL)
{
  Boolean ok = FALSE;
  char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE], ext[MAXEXT];
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

  fnmerge(pat, drv, dir, nam, ((fnsplit(fname, drv, dir, nam, ext)) & EXTENSION) ? ext : "BMP");

  CFILE file(pat);

  if OK(file)
    {
      file.Read(&hea, sizeof(hea));
      if OK(file)
	{
	  if (hea.hdr == 0x436L)
	    {
	      word i = (hea.hdr - sizeof(hea)) / sizeof(BGR4);
	      file.Read(bpal, sizeof(bpal));
	      if (OK(file))
		{
		  for (i = 0; i < 256; i ++)
		    {
		      Pal[i].R = bpal[i].R;
		      Pal[i].G = bpal[i].G;
		      Pal[i].B = bpal[i].B;
		    }
		  M = farnew(byte, (H=hea.hig) * (W=hea.wid));
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
		      if (i < 0) ok = TRUE;
		    }
		}
	    }

	}
    }
  if (! ok) VGA::Exit("BITMAP::BITMAP - File error", fname);
  if (mirr) FlipH();
}





BITMAP::BITMAP (word w, word h, byte far * map, Boolean mirr)
: W(w), H(h), M(map), V(NULL)
{
  if (mirr) FlipH();
}




// following routine creates filled rectangle
// immediately as VGA video chunks, in near memory as fast as I can,
// especially for text line real time display

BITMAP::BITMAP (word w, word h, byte fill)
: W((w + 3) & ~3),		// only full dwords allowed!
  H(h),
  M(NULL)
{
  word dsiz = W >> 2;		// data size (1 plane line size)
  word lsiz = 2 + dsiz + 2;	// word for line header, word for gap
  word psiz = H * lsiz;		// - last gape, but + plane trailer
  byte * v = new byte[4 * psiz];// the same for 4 planes

  if (v == NULL) VGA::Exit("BITMAP::BITMAP - No near core");

  * (word *) v = CPY | dsiz;	// data chunk hader
  memset(v+2, fill, dsiz);	// data bytes
  * (word *) (v + lsiz - 2) = SKP | ((SCR_WID / 4) - dsiz); // gap
  memcpy(v + lsiz, v, psiz - lsiz); // tricky replicate lines
  * (word *) (v + psiz - 2) = EOI; // plane trailer word
  memcpy(v + psiz, v, 3 * psiz);// tricky replicate planes
  V = v;
}






BITMAP::~BITMAP (void)
{
  if (M && FP_SEG(M) != _DS) farfree(M);
  if (V)
    {
      if (FP_SEG(V) == _DS) delete[] V;
      else farfree(V);
    }
}



BITMAP * BITMAP::FlipH (void)
{
  byte far * m = M;
  word i;

  for (i = 0; i < H; i ++)
    {
      byte far * p = m, far * q = m + W - 1;
      while (p < q)
	{
	  byte x = *p;
	  *p = *q;
	  *q = x;
	  ++ p;
	  -- q;
	}
      m += W;
    }
  return this;
}






BITMAP * BITMAP::Code (Boolean keep)
{
  if (M)
    {
      if (! keep)
	{
	  if (V && FP_SEG(V) != _DS) farfree(V);
	  V = NULL;
	}

      while (TRUE)
	{
	  byte far * im = V+2;
	  word far * cp = (word far *) V;
	  int bpl;

	  for (bpl = 0; bpl < 4; bpl ++)
	    {
	      byte far * bm = M;
	      Boolean skip = (bm[bpl] == TRANS);
	      word i, j, cnt;

	      cnt = 0;
	      for (i = 0; i < H; i ++)
		{
		  byte pix;
		  for (j = bpl; j < W; j += 4)
		    {
		      pix = bm[j];
		      if ((pix == TRANS) != skip || cnt >= 0x3FF0) // end of block
			{
			  cnt |= (skip) ? SKP : CPY;
			  if (V) *cp = cnt;
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
			  if (V) *cp = cnt;
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
		  if (V) *cp = cnt;
		  cp = (word far *) im;
		  im += 2;
		}
	      if (V) *cp = EOI;
	      cp = (word far *) im;
	      im += 2;
	    }
	  if (V) break;
	  V = farnew(byte, (word) (im - 2 - V));
	  if (! V) VGA::Exit("BITMAP::Code - No core");
	}
      if (! keep && FP_SEG(M) != _DS)
	{
	  farfree(M);
	  M = NULL;
	}
    }
  return this;
}




void BITMAP::XShow (int x, int y)
{
  byte rmsk = x % 4,
       mask = 1 << rmsk,
       far * scr = VGA::Page[1] + y * (SCR_WID / 4) + x / 4;
  byte near * m = (char *) M;
  byte far  * v = V;

	asm	push	bx
	asm	push	si
	asm	push	ds

	asm	cld
	asm	les	di,scr
	asm	lds	si,v
	asm	mov	bx,m



	asm	mov	al,0x02		// map mask register
	asm	mov	ah,mask

  plane:
	// enable output plane
	asm	mov	dx,VGASEQ_
	asm	out	dx,ax
	asm	push	ax

	// select input plane
	asm	mov	dx,VGAGRA_
	asm	mov	al,0x04		// read map select register
	asm	mov	ah,rmsk
	asm	out	dx,ax

	asm	push	di

  block:
	asm	lodsw
	asm	mov	cx,ax
	asm	and	ch,0x3F
	asm	test	ah,0xC0
	asm	jz	endpl
	asm	jns	skip
	asm	jnp	incsi		// replicate?
	asm	add	si,cx		// skip over data block
	asm	dec	si		// fix it before following inc

  incsi:
	asm	inc	si
  tint:
	asm	mov	al,es:[di]
	//-----------------------------------------------
	// asm	xlat	ss:0	// unsupported with BASM!
	__emit__(0x36, 0xD7);	// this stands for above!
	//-----------------------------------------------
	asm	stosb
	asm	loop	tint
	asm	jmp	block

  skip:
	asm	add	di,cx
	asm	jmp	block

  endpl:
	asm	pop	di
	asm	pop	ax
	asm	inc	rmsk
	asm	shl	ah,1
	asm	test	ah,0x10
	asm	jz	x_chk
	asm	mov	ah,0x01
	asm	mov	rmsk,0
	asm	inc	di
  x_chk:
	asm	cmp	ah,mask
	asm	jne	plane
	asm	pop	ds
	asm	pop	si
	asm	pop	bx
}






void BITMAP::Show (int x, int y)
{
  byte mask = 1 << (x % 4),
       far * scr = VGA::Page[1] + y * (SCR_WID / 4) + x / 4;
  byte far * v = V;

	asm	push	si
	asm	push	ds

	asm	cld
	asm	les	di,scr
	asm	lds	si,v
	asm	mov	dx,VGASEQ_
	asm	mov	al,0x02
	asm	mov	ah,mask

  plane:
	asm	out	dx,ax
	asm	push	ax
	asm	push	di

  block:
	asm	mov	cx,[si]		// with ADD faster then LODSW
	asm	add	si,2
	asm	test	ch,0xC0
	asm	jns	skip		// 1 (SKP) or 0 (EOI)
	asm	jpo	repeat

  copy:
	asm	and	ch,0x3F
	asm	shr	cx,1
	asm	rep	movsw
	asm	jnc	block
	asm	movsb
	asm	jmp	block

  repeat:
	asm	and	ch,0x3F
	asm	mov	al,[si]
	asm	inc	si
	asm	mov	ah,al
	asm	shr	cx,1
	asm	rep stosw
	asm	jnc	block
	asm	mov	es:[di],al
	asm	inc	di
	asm	jmp	block

  skip:
	asm	jz	endpl
	asm	and	ch,0x3F
	asm	add	di,cx
	asm	jmp	block

  endpl:
	asm	pop	di
	asm	pop	ax
	asm	shl	ah,1
	asm	test	ah,0x10
	asm	jz	x_chk
	asm	mov	ah,0x01
	asm	inc	di
  x_chk:
	asm	cmp	ah,mask
	asm	jne	plane
	asm	pop	ds
	asm	pop	si
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





//--------------------------------------------------------------------------



