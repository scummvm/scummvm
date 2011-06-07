#ifndef __DBF__
#define __DBF__

#include	<jbw.h>

#define	Areas		16
#define	MaxIxFiles	64
#define	DCONVR_MAX	10

#ifndef		MaxKeyLen
  #define	MaxKeyLen 	10
#endif

typedef
  #ifdef DEMO
    signed char
  #else
    long
  #endif
	RecPos;

typedef enum { 	Ok,
		NoBaseErr,
		RangeErr,
		NoCoreErr,
		NoWorkErr,
		NoFileErr,
		BadFileErr,
		FRdOnlyErr,
		FOpenErr,
		FSeekErr,
		FReadErr,
		FWriteErr,
		FCloseErr,

		NoIxErr,
		NoIxFileErr,
		BadIxFileErr,
		IxRdOnlyErr,
		IxOpenErr,
		IxSeekErr,
		IxReadErr,
		IxWriteErr,
		IxCloseErr,

		ShareErr,
		LockErr
	     } Errors;

typedef struct {
		char	Nme[11];
		char	Tpe;
		byte	Len;
		byte	Dec;
		int	Inf;
	       } FldDef;

typedef	struct {
		 Boolean (*Bof)		(void);
		 Boolean (*GoTop)	(void);
		 Boolean (*Skip)	(int);
		 Boolean (*GoBottom)	(void);
		 Boolean (*Eof)		(void);
	       } DbfFilter;

typedef	char *	IndexProcType (const char * key);



extern	Boolean	Network;
extern	int	Share;
extern	int	DbfRetryTime;
extern	char	DateConvr[DCONVR_MAX+1];
extern	DbfFilter DbfStdFilter;
extern	Boolean	(*Timeout)(void);			// TRUE = RETRY
extern	void	(*DbfError)(Errors, const char *);

#ifdef __FASTIO__
  extern	int	MaxSaySize;
#endif


//EC void	SetHelpProc	(void hp());
EC void		SetEchoProc	(void (*ep)());
EC Errors	LastError	(void);
EC int		DbfLastKey	(void);
EC char	*	ErrMsg		(Errors err);
EC char	*	PolErrMsg	(Errors err);
EC char	*	Stuff		(char *s1, int p, int n, const char *s2);
EC char	*	LFill		(const char *text, int fill_ch, int size);
EC char	*	RFill		(const char *text, int fill_ch, int size);
EC char	*	DbfD2C		(const char *date_str);
EC void		DbfStr		(int field, long n);
EC long		DbfVal		(int field);
EC int		DbfSelected	(void);
EC int		DbfLocked	(void);
EC Boolean	DbfUpdated	(void);
EC Boolean	DbfSelect	(int work);
EC Boolean 	DbfOpen		(const char *fname);
EC Boolean 	DbfOpenToWrite	(const char *fname);
EC Boolean 	DbfOpenToRead	(const char *fname);
EC Boolean	DbfCreate	(const char *fname, FldDef *fd);
EC Boolean	DbfClose	(void);
EC Boolean	DbfFlush	(void);
EC Boolean	DbfCommit	(void);
EC Boolean	CloseAll	(void);
EC Boolean	DbfZap		(void);
EC Boolean	DbfLock		(void);
EC Boolean	DbfUnlock	(void);
EC Boolean	DbfHardUnlock	(void);
EC Boolean	DbfFldSet	(int field, const char *s);
EC Boolean	DbfReplace	(int field, const char *s);
EC Boolean	DbfString	(int field,       char *s);
EC Boolean	DbfMatch	(int field, const char *s);
EC Boolean	DbfUndo		(void);
EC Boolean 	DbfUsed		(void);
EC Boolean	DbfRdOnly	(void);
EC char	*	DbfName		(void);
EC RecPos	DbfRCount	(void);
EC int		DbfFCount	(void);
EC char	*	DbfRecPtr	(void);
EC int		DbfRecLen	(void);
EC char	*	DbfFldNam	(int field);
EC int		DbfFldTpe	(int field);
EC int		DbfFldLen	(int field);
EC int		DbfTrmLen	(int field);
EC int		DbfFldDec	(int field);
EC char	*	DbfFldPtr	(int field);
EC char	*	_DbfFldPtr	(int work, int field);
EC char	*	DbfUpdat	(void);
EC char	*	DbfToday	(char *d);
EC Boolean	DbfRemove	(void);
EC RecPos	DbfRecNo	(void);
EC Boolean	DbfGoto		(RecPos recno);
EC void		DbfSetWriteHook	(void (*wh)(void));

EC DbfFilter *	DbfSetFilter	(DbfFilter * f);

EC Boolean	_DbfBof		(void);
EC Boolean	DbfBof		(void);
EC Boolean	_DbfGoTop	(void);
EC Boolean	DbfGoTop	(void);
EC Boolean	_DbfSkip	(int amount);
EC Boolean	DbfSkip		(int amount);
EC Boolean	_DbfGoBottom	(void);
EC Boolean	DbfGoBottom	(void);
EC Boolean 	_DbfEof		(void);
EC Boolean 	DbfEof		(void);

EC Boolean	DbfDelete	(Boolean on);
EC Boolean	DbfIsDeleted	(void);
EC Boolean	DbfClone	(void);
EC void		SayMemD		(int x, int y, const char *text, int l);
EC void		SayC		(int x, int y, const char *text, int l);
EC Boolean	DbfSay		(int x, int y, int field);
EC Boolean	GetC		(int x, int y, char *text, int l);
EC int 		DbfGet		(int x, int y, int field);


EC Boolean	IxClose		(void);
EC Boolean	IxCreat		(const char *fname, int field, IndexProcType * ix_proc);
EC Boolean	IxOpen		(const char *fname, IndexProcType * ix_proc);
EC Boolean	SetOrder	(int ixnum);
EC Boolean	DbfFind		(const char *key);
EC Boolean	DbfIsIndexed	(void);
EC int		DbfOrder	(void);

#endif
