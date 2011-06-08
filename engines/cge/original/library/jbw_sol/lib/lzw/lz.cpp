#include	<lz.h>
#include	<conio.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dos.h>
#include	<ctype.h>


#define	CLEAR		(1<<DATA_LEN)
#define	FINIS		(CLEAR+1)
#define	FIRST		(FINIS+1)
#define	SPECIAL		0xFFFE
#define	ERROR		0xFFFD





//--------------------------------------------------------------------------





LZW::LZW (XFILE * input, XFILE * output, void (* echo)(void))
: Input(input), Output(output), Echo(echo),
  gbuf(0), grem(0), pbuf(0), prem(0),
  gptr(0), glim(0), pptr(0)
{
}



void LZW::Clear (word org)
{
  entry * e = Tab;
  word i;

  for (i = 0; i < TAB_SIZ; i ++)
    {
      e->pfx = NOTHING;
      e->sfx = (i < CLEAR) ? (org + i) : NOTHING;
      e->nxt = NULL;
      ++ e;
    }
  for (i = 0, e = Tab; i < ArrayCount(end); i ++) end[i] = e ++;
  for (Avail = CLEAR; Avail < FIRST; Avail ++) Tab[Avail].sfx = SPECIAL;
  CodeLen = MIN_LEN;
}





word _fastcall LZW::Find (word pfx, word sfx)
{
  register entry * e = &Tab[sfx];
  while (e->pfx != pfx) if ((e = e->nxt) == NULL) return Avail;
  return (word) (e - Tab);
}





Boolean _fastcall LZW::Store (word pfx, word sfx)
{
  register entry * n, * p;
  if (Avail == TAB_SIZ) return FALSE;
  n = &Tab[Avail++];
  n->pfx = pfx;
  n->sfx = sfx;

  // link to sfx chain
  p = end[sfx];
  end[sfx] = n;
  p->nxt = n;
  return TRUE;
}





Boolean LZW::Unpack (void)
{
  static byte stack[1024], * sptr;
  word code, incode, oldcode;
  word finchr, lastchr;
  Boolean unknown = FALSE;

  start:
  if (Echo) Echo();
  Clear();
  sptr = stack;
  if ((code = oldcode = GetCode()) == ERROR) return FALSE;
  if (code == CLEAR)
    {
      Debug( printf("\n*** CLEAR code received\n"); )
      goto start;
    }
  finchr = Tab[code].sfx;
  if (! PutByte(finchr)) return FALSE;

  while (TRUE)
    {
      if ((code = incode = GetCode()) == ERROR) return FALSE;
      if (code == FINIS)
	{
	  Debug( printf("\n*** FINIS code received\n"); )
	  break;
	}
      if (code == CLEAR)
	{
	  Debug( printf("\n*** CLEAR code received\n"); )
	  goto start;
	}
      if (code >= Avail) // unknown
	{
	  lastchr = finchr;
	  code = oldcode;
	  unknown = TRUE;
	}
      while (Tab[code].pfx != NOTHING) // not atomic?
	{
	  * (sptr ++) = Tab[code].sfx;
	  code = Tab[code].pfx;
	}
      finchr = Tab[code].sfx;
      if (! PutByte(finchr)) return FALSE;
      while (sptr != stack) if (! PutByte(* (-- sptr))) return FALSE;
      if (unknown)
	{
	  finchr = lastchr;
	  if (! PutByte(finchr)) return FALSE;
	  unknown = FALSE;
	}

      if (Avail >= TAB_SIZ) return FALSE;
      if (! Store(oldcode, finchr)) return FALSE;
      if (Avail >= (1 << CodeLen))
	{
	  ++ CodeLen;
	  Debug( printf("\n*** increase code length to %u\n", CodeLen); )
	}
      oldcode = incode;
    }
  if (! PutByte(NOTHING)) return FALSE; // flush output
  Debug( printf("\n*** final table usage = %u\n", Avail); )
  return TRUE;
}





Boolean LZW::Pack (void)
{
  word tmp;

  if (Echo) Echo();
  Clear();
  tmp = NOTHING;

  while (TRUE)
    {
      word chr, idx;

      if ((chr = GetByte()) == ERROR) return FALSE;

      if (chr == NOTHING)
	{
	  if (tmp != NOTHING) if (! PutCode(tmp)) return FALSE;
	  break;
	}
      if ((idx = Find(tmp, chr)) < Avail) tmp = idx;
      else
	{
	  if (! PutCode(tmp)) return FALSE;
	  if (Avail >= (1 << CodeLen))
	    {
	      ++ CodeLen;
	      Debug( printf("\n*** increase code length to %u\n", CodeLen); )
	    }
	  Store(tmp, chr);
	  if (Avail >= TAB_SIZ)
	    {
	      Debug( printf("\n*** send CLEAR code\n"); )
	      if (Echo) Echo();
	      if (! PutCode(CLEAR)) return FALSE;
	      Clear();
	    }
	  tmp = Find(NOTHING, chr);
	}
    }
  if (! PutCode(FINIS)) return FALSE;
  Debug( printf("\n*** final table usage = %u\n", Avail); )
  return TRUE;
}






word _fastcall LZW::GetCode (void)
{
  word code = 0;
  word bits = 0;

  while (bits < CodeLen)
    {
      if (grem == 0)
	{
	  if ((gbuf = GetByte()) == ERROR) return ERROR;
	  grem = 8;
	}
      code |= gbuf << bits;
      bits += grem;
      if (bits > CodeLen)
	{
	  gbuf >>= CodeLen - (bits-grem);
	  grem = bits - CodeLen;
	  bits = CodeLen;
	}
      else grem = 0;
    }
  return code & MASK(CodeLen);
}





Boolean _fastcall LZW::PutCode (word code)
{
  word len = CodeLen;
  Boolean fl = code == FINIS;

  while (len)
    {
      pbuf |= (code << prem);
      prem += len;
      if (prem >= 16)
	{
	  len = prem - 16;
	  prem = 16;
	  code >>= CodeLen - len;
	}
      else
	{
	  code = 0;
	  len = 0;
	}
      if (prem >= 8)
	{
	  if (! PutByte(pbuf & 0xFF)) return FALSE;
	  pbuf >>= 8;
	  prem -= 8;
	}
    }
  while (fl && prem)
    {
      if (! PutByte(pbuf & 0xFF)) return FALSE;
      pbuf >>= 8;
      prem = (prem > 8) ? (prem - 8) : 0;
    }
  if (fl) if (! PutByte(NOTHING)) return FALSE; // flush output
  return TRUE;
}






word _fastcall LZW::GetByte (void)
{
  static byte buf[IOBUF_SIZ];

  if (gptr >= glim)
    {
      glim = Input->Read(buf, IOBUF_SIZ);
      if (Input->Error) return ERROR;
      if (glim == 0) return NOTHING;
      gptr = 0;
    }
  return buf[gptr ++];
}




Boolean _fastcall LZW::PutByte (word w)
{
  static byte buf[IOBUF_SIZ];

  if (w == NOTHING || pptr >= IOBUF_SIZ)
    {
      if (pptr) Output->Write(buf, pptr);
      if (Output->Error) return FALSE;
      pptr = 0;
    }
  if (w != NOTHING)
    {
      buf[pptr ++] = w;
    }
  return TRUE;
}
