#ifndef		__MFILE__
#define		__MFILE__

#include	"xfile.h"





class MFILE : public XFILE
{
  byte far * Adr, far * Ptr, far * Lim;
public:
  MFILE (byte far * adr, long siz, MODE mode = REA);
  virtual ~MFILE (void);
  virtual word Read (byte far * buf, word len);
  virtual word Write (byte far * buf, word len);
  virtual long Mark (void);
  virtual long Size (void);
  virtual long Seek (long pos);
};

#endif
