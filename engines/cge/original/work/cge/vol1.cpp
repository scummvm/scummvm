#include	"vol.h"
#include	<alloc.h>
#include	<stdlib.h>
#include	<string.h>



#if VOL_MODE == CFU
CAT::REC far *	CAT::Buf			= NULL;
word		CAT::Max			= 0;
#endif
CAT::REC	CAT::Rec;
word		CAT::Cnt			= 0;
CFILE		CAT::File(CAT_NAME, VOL_MODE);
CFILE		DAT::File(DAT_NAME, VOL_MODE);

CAT		VFILE::Cat;
DAT		VFILE::Dat;
VFILE *		VFILE::Recent			= NULL;


CAT::CAT (void)
{
  Cnt = (word) (File.Size() / sizeof(REC));
  #if VOL_MODE
  if (Buf == NULL)
    {
      dword m = farcoreleft();
      if (m > 4 * IOBUF_SIZE + K(1))
	{
	  Max = (word) ((m - 4 * IOBUF_SIZE) / sizeof(REC));
	  Buf = farnew(REC, Max);
	  if (Max < Cnt) exit(1);
	  File.Read((byte far *) Buf, Cnt * sizeof(REC));
	}
    }
  #endif
}






CAT::~CAT (void)
{
  #if VOL_MODE == CFU
  if (Buf)
    {
      File.Seek(0);
      File.Write((byte far *) Buf, Cnt * sizeof(REC));
    }
  #endif
}






int CAT::Cmp(REC& r1, REC& r2)
{
  int d = stricmp(r1.Nam, r2.Nam);
  return (d) ? d : stricmp(r1.Ext, r2.Ext);
}





int CAT::Search (REC& r, int a, int b)
{
  int i, d;
  i = (a + b) / 2;
  TakeRec(Rec, i);
  d = Cmp(r, Rec);
  if (d == 0 || a >= b) return a;
  if (d < 0) return Search(r, a, i-1);
  else       return Search(r, i+1, b);
}







Boolean CAT::Find (const char * name, long& org, long& end)
{
  if (File.Error == 0)
    {
      REC r;
      fnsplit(name, NULL, NULL, r.Nam, r.Ext);

      if (Cnt == 0) return FALSE;
      Search(r, 0, Cnt-1);
      if (Cmp(r, Rec) == 0)
	{
	  org = Rec.Org;
	  end = org + Rec.Len;
	  return TRUE;
	}
    }
  return FALSE;
}






#if VOL_MODE == CFU
Boolean CAT::Add (const char * name, long org, long len)
{
  if (File.Error == 0)
    {
      word n;
      REC r;
      memset(&r, '\0', sizeof(r));
      fnsplit(name, NULL, NULL, r.Nam, r.Ext);
      r.Org = org;
      r.Len = len;

      if (Cnt >= Max) DROP("Too many files", "");
      if (Cnt == 0) n = 0;
      else
	{
	  n = Search(r, 0, Cnt-1);
	  int d = Cmp(r, Rec);
	  if (d == 0) DROP("File exists", name);
	  if (d > 0) ++ n;
	  if (n > Cnt) DROP("Internal error", "#6591");	///////////////
	  if (n < Cnt)
	    {
	      _fmemmove(Buf+n+1, Buf+n, (Cnt-n) * sizeof(*Buf));
	    }
	}
      Buf[n] = r;
      ++ Cnt;
      return TRUE;
    }
  return FALSE;
}






Boolean DAT::Append (CFILE& cf)
{
  File.Append(cf);
  return File.Error == 0;
}








VFILE::VFILE (CFILE& file, const char * name)
 : Error(0)
{
  Pos = Dat.File.Size();
  Dat.Append(file);
  End = Dat.File.Size();
  Cat.Add(name, Pos, End-Pos);
  Dat.File.Seek(Pos);
}

#endif





VFILE::VFILE (const char * name)
 : Error(0)
{
  if (Cat.Find(name, Pos, End))
    {
      Dat.File.Seek(Pos);
      Recent = this;
    }
  else DROP("Unable to locate file", name);
}







VFILE::Exist (const char * name)
{
  long a, b;
  return Cat.Find(name, a, b);
}





word VFILE::Read (byte far * buf, word len)
{
  if (Recent != this)
    {
      Dat.File.Flush();
      Dat.File.Seek(Pos);
      Recent = this;
    }
  word n = (word) (End-Pos);
  if (len > n) len = n;
  n = Dat.File.Read(buf, len);
  if (Dat.File.Error) n = 0;
  Pos += n;
  return n;
}








/*
word VFILE::Read (byte far * buf)
{
  if (Pos >= End) return 0;
  Dat.File.Flush();
  Dat.File.Seek(Pos);
  word n = Dat.File.Read(buf);
  if (n > (word) (End-Pos))
    {
      n = (word) (End-Pos);
      buf[n] = '\0';
    }
  if (Dat.File.Error) n = 0;
  if (n) Pos += n+1;
  return n;
}
*/





word VFILE::Read (byte far * buf)
{
  byte far * p, far * q;
  word n = (word) (End - Pos);

  if (n > LINE_MAX) n = LINE_MAX;
  Dat.File.Seek(Pos);
  n = Dat.File.Read(buf, n);
  if (Dat.File.Error || n == 0) return 0;

  for (p = (byte far *) _fmemchr(buf, '\r', n);
       p;
       p = (byte far *) _fmemchr(p, '\r', n - (word) (p - buf)))
    {
      if (p[1] == '\n')
	{
	  n = (word) (p - buf);
	  break;
	}
    }
  q = (byte far *) _fmemchr(buf, 26, n);
  if (q)
    {
      n = (word) (q - buf);
      Pos += n;
    }
  else
    {
      if (p)
	{
	  buf[n ++] = '\n';
	  Pos += n+1;
	}
      else Pos += n;
    }
  buf[n] = '\0';
  return n;
}




/*
word VFILE::Read (byte far * buf)
{
  if (Recent != this)
    {
      Dat.File.Flush();
      Dat.File.Seek(Pos);
      Recent = this;
    }
  word n = Dat.File.Read(buf);
  if (Pos + n > End)
    {
      n = (word) (End - Pos);
      buf[n] = '\0';
    }
  Pos += n;
  if (buf[n-1] == '\n') ++ Pos;
  return n;
}
*/