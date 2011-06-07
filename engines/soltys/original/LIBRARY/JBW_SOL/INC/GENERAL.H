#ifndef		__GENERAL__
#define		__GENERAL__

#include	<jbw.h>
#include	<io.h>

#define		SEED		0xA5

#define		SCR_WID_	320
#define		SCR_HIG_	200
#define		SCR_WID		((word)SCR_WID_)
#define		SCR_HIG		((word)SCR_HIG_)
#define		SCR_SEG		0xA000
#define		SCR_ADR		((byte far *) MK_FP(SCR_SEG, 0))



enum	CPU		{ _8086, _80186, _80286, _80386, _80486 };
enum	MEM_TYPE	{ BAD_MEM, EMS_MEM, NEAR_MEM, FAR_MEM };
enum	ALLOC_MODE	{ FIRST_FIT, BEST_FIT, LAST_FIT };
enum	IOMODE		{ REA, WRI, UPD };

typedef	struct	{
		  byte R, G, B;
		} DAC;

typedef	word	CRYPT	(void far * buf, word siz, word seed);






class COUPLE
{
protected:
  signed char A;
  signed char B;
public:
  COUPLE (void) { }
  COUPLE (const signed char a, const signed char b) : A(a), B(b) { }
  COUPLE operator + (COUPLE c) { return COUPLE(A+c.A, B+c.B); }
  void operator += (COUPLE c) { A += c.A; B += c.B; }
  COUPLE operator - (COUPLE c) { return COUPLE(A-c.A, B-c.B); }
  void operator -= (COUPLE c) { A -= c.A; B -= c.B; }
  Boolean operator == (COUPLE c) { return ((A - c.A) | (B - c.B)) == 0; }
  Boolean operator != (COUPLE c) { return ! (operator == (c)); }
  void Split (signed char& a, signed char& b) { a = A; b = B; }
};


//-------------------------------------------------------------------------



class ENGINE
{
protected:
  static void interrupt (far * OldTimer) (...);
  static void interrupt NewTimer (...);
public:
  ENGINE (word tdiv);
  ~ENGINE (void);
};




//-------------------------------------------------------------------------


class EMS;



class EMM
{
  friend EMS;
  Boolean Test (void);
  long Top, Lim;
  EMS * List;
  int Han;
  static void _seg * Frame;
public:
  EMM::EMM (long size = 0);
  EMM::~EMM (void);
  EMS * Alloc (word siz);
  void Release (void);
};





class EMS
{
  friend EMM;
  EMM * Emm;
  long Ptr;
  word Siz;
  EMS * Nxt;
public:
  EMS (void);
  void far * operator & () const;
  word Size (void);
};



//-------------------------------------------------------------------------




template <class T>
void Swap (T& A, T& B)
{
  T a = A;
    A = B;
    B = a;
};





#ifdef __cplusplus


template <class T>
T max (T A, T B)
{
  return (A > B) ? A : B;
};



template <class T>
T min (T A, T B)
{
  return (A < B) ? A : B;
};


#endif







class XFILE
{
public:
  IOMODE Mode;
  word Error;
  XFILE (void) : Mode(REA), Error(0) { }
  XFILE (IOMODE mode) : Mode(mode), Error(0) { }
  virtual word Read (void far * buf, word len) = 0;
  virtual word Write (void far * buf, word len) = 0;
  virtual long Mark (void) = 0;
  virtual long Size (void) = 0;
  virtual long Seek (long pos) = 0;
};





template <class T>
inline word XRead (XFILE * xf, T * t)
{
  return xf->Read((byte far *) t, sizeof(*t));
};





class IOHAND : public XFILE
{
protected:
  int Handle;
  word Seed;
  CRYPT * Crypt;
public:
  IOHAND (const char near * name, IOMODE mode = REA, CRYPT crypt = NULL);
  IOHAND (IOMODE mode = REA, CRYPT * crpt = NULL);
  virtual ~IOHAND (void);
  static Boolean Exist (const char * name);
  word Read (void far * buf, word len);
  word Write (void far * buf, word len);
  long Mark (void);
  long Size (void);
  long Seek (long pos);
  ftime Time (void);
  void SetTime (ftime t);
};





CRYPT		XCrypt;
CRYPT		RXCrypt;
CRYPT		RCrypt;

MEM_TYPE	MemType		(void far * mem);
unsigned	FastRand	(void);
unsigned	FastRand	(unsigned s);
CPU		Cpu		(void);
ALLOC_MODE	SetAllocMode	(ALLOC_MODE am);
word		atow		(const char * a);
word		xtow		(const char * x);
char *		wtom		(word val, char * str, int radix, int len);
char *		dwtom		(dword val, char * str, int radix, int len);
char *		DateTimeString	(void);
void		StdLog		(const char *msg, const char *nam = NULL);
void		StdLog		(const char *msg, word w);
void		StdLog		(const char *msg, dword d);
int		TakeEnum	(const char ** tab, const char * txt);
word		ChkSum		(void far * m, word n);
long		Timer		(void);
long		TimerLimit	(word t);
Boolean		TimerLimitGone	(long t);
char *		MergeExt	(char * buf, const char * nam, const char * ext);
char *		ForceExt	(char * buf, const char * nam, const char * ext);
inline const char * ProgPath	(void);
const char *	ProgName	(const char * ext = NULL);
int		DriveFixed	(unsigned drv);
int 		DriveRemote	(unsigned drv);
int 		DriveCD		(unsigned drv);
Boolean		IsVga		(void);

EC void		_fqsort		(void far *base, word nelem, word width,
				int (*fcmp)(const void far *, const void far *));



#endif
