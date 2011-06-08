#include	"vol.h"
#include	<alloc.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#ifdef	DROP_H
  #include	"drop.h"
#else
  #include	<stdio.h>
  #define	DROP(m,n)	{ printf("%s [%s]\n", (m), (n)); _exit(1); }
#endif




#ifdef VOL_UPD
BTFILE		VFILE::Cat(CAT_NAME, UPD, CRP);
VOLBASE		DAT::File(DAT_NAME, UPD, CRP);
#else
BTFILE		VFILE::Cat(CAT_NAME, REA, CRP);
VOLBASE		DAT::File(DAT_NAME, REA, CRP);
#endif
DAT		VFILE::Dat;
VFILE *		VFILE::Recent			= NULL;





VFILE::VFILE (const char * name, IOMODE mode)
: IOBUF(mode)
{
  if (mode == REA)
    {
      if (Dat.File.Error || Cat.Error) DROP("Bad volume data", NULL);
      BT_KEYPACK far * kp = Cat.Find(name);
      if (_fstricmp(kp->Key, name) != 0) Error = ENOFILE;
      EndMark = (BufMark = BegMark = kp->Mark) + kp->Size;
    }
  #ifdef VOL_UPD
  else Make(name);
  #endif
}





VFILE::~VFILE (void)
{
  if (Recent == this) Recent = NULL;
}





Boolean VFILE::Exist (const char * name)
{
  return _fstricmp(Cat.Find(name)->Key, name) == 0;
}




void VFILE::ReadBuff (void)
{
  if (Recent != this)
    {
      Dat.File.Seek(BufMark + Lim);
      Recent = this;
    }
  BufMark = Dat.File.Mark();
  long n = EndMark - BufMark;
  if (n > IOBUF_SIZE) n = IOBUF_SIZE;
  Lim = Dat.File.Read(Buff, (word) n);
  Ptr = 0;
}
