#include	<general.h>

#define		EMS_INT		0x67
#define		PAGE_MASK	0x3FFF
#define		SIZ(n)		((n) ? ((long)n) : (0x10000L))


enum EMM_FUN { GET_STATUS = 0x40,
	       GET_FRAME,
	       GET_SIZE,
	       OPEN_HANDLE,
	       MAP_PAGE,
	       CLOSE_HANDLE,
	       GET_VER,
	       SAVE_CONTEXT,
	       REST_CONTEXT,
	       GET_PAGES = 0x4B,
	       GET_HANDLES,
	       GET_INFO,
	       CONTROL };








void	_seg *	EMM::Frame = NULL;






EMM::EMM (long size)
: Han(-1), Top(0), Lim(0), List(NULL)
{
  if (Test())
    {
      asm	mov	ah,GET_FRAME	// get EMS frame segment
      asm	int	EMS_INT		// do it!
      asm	or	ah,ah		// see status
      asm	jnz	xit		// abort on error
      Frame = (void _seg *) _BX;	// save frame segment

      if (size == 0)
	{
	  asm	mov	ah,GET_SIZE	// get EMS memory size
	  asm	int	EMS_INT		// do it!
	  asm	or	ah,ah		// see status
	  asm	jnz	xit		// abort on error
	  asm	or	bx,bx		// test page count
	  asm	jz	xit		// abort if no free pages
	  // number of available pages in BX is ready to use by OPEN
	}
      else _BX = (word) ((size + PAGE_MASK) >> 14);
      asm	mov	ah,OPEN_HANDLE	// open EMM handle
      asm	int	EMS_INT		// do it!
      asm	or	ah,ah		// see status
      asm	jnz	xit		// abort on error
      Han = _DX;
      Lim = _BX;
      Lim <<= 14;
      _DX = Han;
      asm	mov	ah,SAVE_CONTEXT	// save mapping context
      asm	int	EMS_INT		// do it!
    }
  xit:
}







EMM::~EMM (void)
{
  Release();
  if (Han >= 0)
    {
      _DX = Han;
      asm	mov	ah,REST_CONTEXT
      asm	int	EMS_INT
      asm	mov	ah,CLOSE_HANDLE
      asm	int	EMS_INT
    }
}



Boolean EMM::Test (void)
{
  static char e[] = "EMMXXXX0";

  asm	mov	ax,0x3D40
  asm	mov	dx,offset e
  asm	int	0x21
  asm	jc	fail

  asm	push	ax
  asm	mov	bx,ax
  asm	mov	ax,0x4407
  asm	int	0x21

  asm	pop	bx
  asm	push	ax
  asm	mov	ax,0x3E00
  asm	int	0x21
  asm	pop	ax

  asm	cmp	al,0x00
  asm	je	fail

  success:
  return TRUE;
  fail:
  return FALSE;
}





EMS * EMM::Alloc (word siz)
{
  long size = SIZ(siz),
       top = Top;

  word pgn = (word) (top >> 14),
       cnt = (word) ((top + size + PAGE_MASK) >> 14) - pgn;

  if (cnt > 4)
    {
      top = (top + PAGE_MASK) & 0xFFFFC000L;
      ++ pgn;
      -- cnt;
    }

  if (size <= Lim - top)
    {
      EMS * e = new EMS, * f;

      if (e)
	{
	  Top = (e->Ptr = top) + (e->Siz = siz);
	  e->Emm = this;

	  if (List)
	    {
	      for (f = List; f->Nxt; f = f->Nxt);
	      return (f->Nxt = e);		// existing list: link to the end
	    }
	  else
	    {
	      return (List = e);		// empty list: link to the head
	    }
	}
    }
  fail: return NULL;
}






void EMM::Release (void)
{
  while (List)
    {
      EMS * e = List;
      List = e->Nxt;
      delete e;
    }
  Top = 0;
}




EMS::EMS (void)
: Ptr(0), Siz(0), Nxt(NULL)
{
}




void far * EMS::operator & () const
{
  word pgn = (word) (Ptr >> 14),
       off = (word) Ptr & PAGE_MASK,
       cnt = (word) ((Ptr + SIZ(Siz) + PAGE_MASK) >> 14) - pgn,
       cmd = MAP_PAGE << 8;

  _DX = Emm->Han;		// take EMM handle
  asm	dec	cnt		// prapare for deferred checking
  asm	or	dx,dx		// see if valid
  asm	jns	more		// negative handle = unavailable

  fail: return NULL;

  more:
  asm	mov	ax,cmd		// command + page frame index
  asm	mov	bx,pgn		// logical page number
  asm	int	EMS_INT		// do it!
  asm	or	ah,ah		// check error code
  asm	jnz	fail		// exit on error
  asm	inc	cmd		// advance page frame index
  asm	inc	pgn		// advance logical page number
  asm	cmp	al,byte ptr cnt	// all pages?
  asm	jb	more

  return (void far *) (EMM::Frame + (void near *) off);
}








word EMS::Size (void)
{
  return Siz;
}

