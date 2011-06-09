#ifndef		__CFILE__
#define		__CFILE__

#include	<general.h>
#include	<io.h>


#define		LINE_MAX	512

#ifndef		IOBUF_SIZE
  #define	IOBUF_SIZE	K(2)
#endif

#define		CFREAD(x)	Read((byte far *)(x),sizeof(*(x)))




class IOBUF : public IOHAND
{
protected:
  byte far * Buff;
  word Ptr, Lim;
  long BufMark;
  word Seed;
  CRYPT * Crypt;
  virtual void ReadBuff (void);
  virtual void WriteBuff (void);
public:
  IOBUF (IOMODE mode, CRYPT * crpt = NULL);
  IOBUF (const char * name, IOMODE mode, CRYPT * crpt = NULL);
  virtual ~IOBUF (void);
  word Read (void far * buf, word len);
  word Read (char far * buf);
  int Read (void);
  word Write (void far * buf, word len);
  word Write (byte far * buf);
  void Write (byte b);
};



class CFILE : public IOBUF
{
public:
  static word MaxLineLen;
  CFILE (const char near * name, IOMODE mode = REA, CRYPT * crpt = NULL);
  virtual ~CFILE (void);
  void Flush (void);
  long Mark (void);
  long Seek (long pos);
  void Append (CFILE& f);
};


#endif
