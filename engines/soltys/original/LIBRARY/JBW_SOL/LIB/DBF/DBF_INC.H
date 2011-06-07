#ifndef	__FASTIO__
#include	<wind.h>
#endif

#define		CLEAR_PAGE	1
#define		SHARE_MAX	4

#include	<dbf.h>
#include	<string.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<io.h>
#include	<sys\stat.h>
#include	<dos.h>
#include	<dir.h>

#define		dB2		0x02
#define		dB3		0x03
#define		dB3M		0x83

#define		PageSize	1024
#define		MaxLevels	4

#define		Work		(*WorkPtr)
#define		Err(e)		return Error(e)
#define		UseChk		{ if (! Work.Used) Err(NoBaseErr); }
#define		IxChk		{ if (IxPkP() == NULL) Err(NoIxErr); }

#define		NoRec		0xFFFF
#define		IsLeaf(n)       (IxPkP()->PageBoxTab[n].Page.LessPage == NoRec)
#define		NodePos(n)	(((long) (n).FilePosHB << 16) + (n).FilePos)


#define		PageRoom	(PageSize-3*sizeof(word)-sizeof(int))
#define		MaxNodes	((PageRoom/sizeof(IxNode))/2)

typedef struct
	  {	word		NextPage;
		word		FilePos;
		byte		FilePosHB;
		char		Item[MaxKeyLen];
	  } IxNode;

typedef struct /* adjust PageRoom after changing this struct !!! */
	  { 	word		LeftPage, RightPage, LessPage;
		int		Count;
		IxNode		Node[MaxNodes*2];
		#if		(PageRoom-sizeof(IxNode)*MaxNodes*2 > 0)
		byte		Unused[PageRoom-sizeof(IxNode)*MaxNodes*2];
		#endif
	  } IxPage;

typedef struct
	  {	word		Root;
		word		Free;
		word		New;
		int		Field;
		int		KeyLen;
	  } IxFileDesc;

typedef struct
	  {	word		PgPos;
		Boolean		Updated;
		int		CurOff;
		IxPage  	Page;
	  } PageBox;

typedef struct
	 {      int		Shared;
		int		IFile;
		Boolean		IUpdt;
		IxFileDesc	IDesc;
		char *		(*IProc)(const char *);
		PageBox *	PageBoxTab;
	 } IndexPack;

typedef	IndexPack *IPP;

typedef	struct
	  {	byte		Signature;
		byte		LastUpdt[3];
		RecPos		RecCount;
		#if (sizeof(RecPos) < 4)
		byte		__[4-sizeof(RecPos)];
		#endif
		word		HdrLength;
		word		RecLength;
		byte		_[20];
	  } Header;

typedef struct
	  { 	char		FldNme[11];
		char		FldTpe;
		char *		FldPtr;
		#if (sizeof(void *) < 4)
		byte		__[4-sizeof(void *)];
		#endif
		byte		FldLen;
		byte		FldDec;
		byte		_[14];
	  } FieldDesc;

typedef struct
	  { 	Boolean		Used,
				AnyUpdat,
				RdOnly;
		int		Locked;
		word		Fields;
		char		dBPath[MAXPATH];
		int		FileHan;
		int		Order;
		Header *	HeadPtr;
		FieldDesc *	DescPtr;
		char *		RecPtr;
		DbfFilter *	Filter;
		void		(*WriteHook) (void);
		RecPos		RecNo;
		IPP		IxPk[MaxIxFiles];
	  } WorkArea;



#ifdef		HUNT
  extern int	Hunt;
  #define	Hnt(n)	{ Hunt = n; }
#else
  #define	Hnt(n)	{ }
#endif

extern	WorkArea	WorkTab[Areas], *WorkPtr;
extern	Errors		dBError;
extern	int		CurLev;



Boolean	Error		(Errors err);
void	ClrBuff		(void);
void	PutMirror	(void);
int	CmpMirror	(void);
IPP	IxPkP		(void);
PageBox * IBoxP		(int n);
void	InitPageBoxTab	(PageBox * pb);
Boolean	Flush		(void);
Boolean	IxFlush		(void);
Boolean	PutAllIx	(void);
Boolean	DelAllIx	(void);
Boolean	DbfNetLock	(void);
//int _fastcall FileFlush	(int handle);
