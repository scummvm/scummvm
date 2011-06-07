#ifndef	__BASE__
#define	__BASE__

#include	<wind.h>
#include	<dbf.h>

typedef	enum { NUL, GET, BRW, TXT } FmtType;

typedef	struct	{
		  int	Field;
		  char	*(*Proc)(int);
		} BrwDef;

typedef struct  {
		  int	  	X, Y;
		  char *	Nam;
		  int		Fld;
		  Boolean	(*Keyb)(Wind *);
		  void		(*Show)(Wind *, int);
		  void		(*Edit)(void);
		  int		Base;
		} EdtDef;

typedef	struct	{
		  char *	Name;
		  FldDef *	Format;
		  int		Orders;
		  int		Share;
		  long		Current;
		  IndexProcType * IdxProc;
		  DbfFilter *	Filter;
		} BaseHan;

typedef	struct	{
		  char *	Nam;
		  FmtType	Tpe;
		  int		Sel;
		  int		Lnk;
		  void *	Fmt;
		  Wind *	Wnd;
		  int		Mem;
		  int		X, Y, W, H;
		  word		F;
		} WindHan;

typedef	struct	{
		  char *	Ptr;
		  int		Len;
		  int		Dec;
		  int		Wid;
		  int		X, Y;
		  Boolean	Cap;
//		  Boolean	Ins;
		  char		Tpe;
		} GetHan;


extern	BaseHan		Base[];
extern	WindHan		WHan[];
extern	GetHan		DbfGetHan;
extern	char		MaleSign[];
extern	char		FemaleSign[];
extern	char *		OpenMsg;
extern	int		MaxSaySize;
extern	int		NrRadix;
extern	Boolean		ReindexMode;
extern	void		(*DictBrowseProc)(void);
extern	IndexProcType *	IdxPrc;

void	NoCore		(void);

long	NrVal		(int fld);
void	NrStr		(int fld, long num);


IndexProcType
	BaseIdxProc	;
void	BasePush	(int B);
void	BasePop		(void);
void	BaseReset	(Wind * W);
Boolean	FieldKeyb	(Wind * W);
void	FieldShow	(Wind * W, int n);
Boolean	DbfGetIns	(Wind * W);
Boolean	DbfGetUp	(Wind * W);
Boolean	DbfGetDown	(Wind * W);

Boolean WindDbfKey	(Wind * W);
char *	GetFldC		(int fld, const char * s, const char * t, int hlp);

void	BaseSelect	(int B);
Boolean	SelectPrevWH	(Wind * W);
Boolean	SelectNextWH	(Wind * W);
Boolean	CloseWH		(Wind * W);
Boolean	DictKey		(Wind * W);
Boolean	DictKeyb	(Wind * W);
void	DictShow	(Wind * W, int n);
Boolean	DictNew		(char * d0);
Boolean	SexKey		(char * s);
Boolean	SexKeyb		(Wind * W);
void	SexShow		(Wind * W, int n);
void	EditShut	(Wind * W);
Wind *	OpenEditWind	(WindHan * wh);
Wind *	OpenBrowseWind	(WindHan * wh);
Wind *	OpenBaseWind	(WindHan * wh);
void	BrowseGoto	(Wind * W, RecPos rn);
int	FormatFieldLen	(WindHan * wh, int F);
void	NewRec		(int B);
void	BaseRepaint	(Wind * W);
Boolean	BaseKey		(Wind * W);
void	BaseReset	(Wind * W);
void	UndoRec		(void);
void	GoCurrent	(int B);
void	BaseSkip	(int B);
void	BrowseRepaint	(Wind * W);
Boolean	BrowseKey	(Wind * W);
char *	BaseNrPtr	(int B);
char *	CharStr		(int c);
char *	FieldStr	(int f);
char *	DictStr		(int cpl);
char *	DictStrTrim	(int cpl);
char *	BrowseLine	(BrwDef * tab);
Boolean	BasePrev	(Wind * W);
Boolean	BaseNext	(Wind * W);
Boolean	BaseEnd		(Wind * W);
Boolean	BaseSave	(Wind * W);
Boolean	BaseUndo	(Wind * W);
Boolean	BaseMouse	(Wind * W);
Boolean	BrowseUp	(Wind * W);
Boolean	BrowseDown	(Wind * W);
Boolean	BrowsePgUp	(Wind * W);
Boolean	BrowsePgDn	(Wind * W);
Boolean	BrowseHome	(Wind * W);
Boolean	BrowseEnd	(Wind * W);
Boolean	BrowseMouse	(Wind * W);
Boolean	BrowseLeft	(Wind * W);
Boolean	BrowseRight	(Wind * W);
Boolean	BrowseClear	(Wind * W);
Boolean	BrowseSearch	(Wind * W);

Wind *	OpenTextWind	(WindHan * wh);
void	KillTextWind	(Wind * W);
void	WriteText	(Wind * W);
void	AppendText	(Wind * W, const char * header);
void	ReadText	(Wind * W);

#endif
