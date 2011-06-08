#ifndef		__VOL__
#define		__VOL__


#include	<dir.h>
#include	"btfile.h"
#include	"cfile.h"

#define		CAT_NAME	"VOL.CAT"
#define		DAT_NAME	"VOL.DAT"

#ifndef	CRP
  #define	CRP		XCrypt
#endif

#define		XMASK		0xA5

#ifdef	VOL_UPD
#define		VOLBASE		IOHAND
#else
#define		VOLBASE		CFILE
#endif



class DAT
{
  friend VFILE;
  static VOLBASE File;
public:
  static Boolean Append (byte far * buf, word len);
  static Boolean Write (CFILE& f);
  static Boolean Read (long org, word len, byte far * buf);
};







class	VFILE : public IOBUF
{
  static DAT Dat;
  static BTFILE Cat;
  static VFILE * Recent;
  long BegMark, EndMark;
  void ReadBuff (void);
  void WriteBuff (void) { }
  void Make(const char * fspec);
public:
  VFILE (const char * name, IOMODE mode = REA);
  ~VFILE (void);
  static Boolean Exist (const char * name);
  static const char * Next (void);
  long Mark (void) { return (BufMark+Ptr) - BegMark; }
  long Size (void) { return EndMark - BegMark; }
  long Seek (long pos) { Recent = NULL; Lim = 0; return (BufMark = BegMark+pos); }
};




#endif
