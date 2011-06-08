#ifndef	__WIND__
#define	__WIND__

#include	<jbw.h>
#include	<mouse.h>
#include	<stddef.h>

#ifndef		EC
  #define	EC
#endif

#define		CurOvr		((Mono) ? 0x000C : 0x0007)
#define		CurIns		((Mono) ? 0x0B0C : 0x0607)
#define		CurShp(i)	((i)    ? CurIns : CurOvr)



// Some old functions stealed from FASTIO

#define		WhereX		(GetXY() & 0xFF)
#define		WhereY		(GetXY() >> 8)

EC void	Video		(void);
EC Boolean	IsMono		(void);
EC word far *ScrAdr	(int x, int y);
EC int	MaxRows		(void);
EC int	MaxColumns	(void);
EC void	GetScrSiz	(void);
EC word	SetCursor	(word lines);
EC void	SetXY		(int x, int y);
EC word	GetXY		(void);
EC char	*Stuff		(char *str, int where, int delc, const char *repl);
EC char	*Normalize	(char *dest, const char *srce, const char *ext);
EC Boolean	IsFile		(const char *fname);
EC Boolean	IsWritable	(const char *fname);



/// debug window refresh cycle length in timer units
#ifndef DEBUG_DELAY
#define	DEBUG_DELAY	3
#endif

/// Hardware error routine return codes
#define		ABORT		2
#define		RETRY		1
#define		IGNORE		0

/// Mouse routine flags
#define		MouseMovFlag	0x01
#define		MouseLPrFlag    0x02
#define		MouseLReFlag    0x04
#define		MouseRPrFlag    0x08
#define		MouseRReFlag    0x10
#define		MouseAllFlags   0x1F

#define		AppearMask	0x0001
#define		AllocatedMask	0x0002
#define		FrameMask	0x0004
#define		ZoomedMask	0x0008

#define		ZoomableMask	0x0010
#define		MovableMask	0x0020
#define		VSizeableMask	0x0040
#define		HSizeableMask	0x0080
#define		KeyChoiceMask	0x0100
#define		CapitalMask	0x0200
#define		TCenterMask	0x0400
#define		EditEnableMask	0x1000

#define		Spare1Mask	0x2000
#define		Spare2Mask	0x4000
#define		Spare3Mask	0x8000

#define		MaxDirWid	14

#define		Image(w)	((char near *) (w->Body.Near))
#define		FarImage(w)	((char far *) (w->Body.Far))


typedef	enum	{ STD, MNU, HLP, WAR, DSK, ColorBanks } ColorBank;
typedef	enum	{ FRM_N, FRM_H, FLD_N, FLD_H, SEL_N, SEL_H, ColorRegs } ColorReg;

typedef	struct	{
		  char *Txt;
		  void *Ptr;
		  int  Type;
		} DebugEntry;

	struct WindStruct;
typedef	struct	{
		  Keys		Key;
		  Boolean	(*Proc)(struct WindStruct *);
		} KeyEntry;

typedef struct WindStruct
	  {
	    int  Lft, Top, Rgt, Bot;   // window coordinates
	    int  Wid, Hig;             // size of image
	    int  X, Y;                 // relative position in window
	    int  Hp, Vp;               // numbers used by Menus
	    int	Help;
	    long Time;                 // timer count register
	    word Delay;                // timer delay
	    struct
	      {
		int X, Y;              // cursor's relative position
		word Shape;            // cursor's shape
	      } Cursor;
	    union
	      {
		void near * Near;
		void far  * Far;
	      } Body;                  // pointer to the Wind body buffer
	    int  ColBank;              // color bank number
	    byte Color[ColorRegs];     // copy of color bank
	    struct
	      {
		unsigned Appear    : 1;
		unsigned Allocated : 1;
		unsigned Frame     : 1;
		unsigned Zoomed    : 1;

		unsigned Zoomable  : 1;
		unsigned Movable   : 1;
		unsigned VSizeable : 1;
		unsigned HSizeable : 1;

		unsigned KeyChoice : 1;
		unsigned Capital   : 1;
		unsigned TCenter   : 1;
		unsigned Repaint   : 1;

		unsigned EditEnable: 1;
		unsigned Touched   : 1;
		unsigned Switchable: 1;
		unsigned _         : 1;

		unsigned Flag0     : 1;
		unsigned Flag1     : 1;
		unsigned Flag2     : 1;
		unsigned Flag3     : 1;

		unsigned Flag4     : 1;
		unsigned Flag5     : 1;
		unsigned Flag6     : 1;
		unsigned Flag7     : 1;

		unsigned Flag8     : 1;
		unsigned Flag9     : 1;
		unsigned FlagA     : 1;
		unsigned FlagB     : 1;

		unsigned FlagC     : 1;
		unsigned FlagD     : 1;
		unsigned FlagE     : 1;
		unsigned FlagF     : 1;
	      } Flags;                 // wind flags
	    struct
	      {
		char *Text;            // window heading text
		int  X1, X2;           // title bounds
	      } Title;
	    KeyEntry *KeyTab;          // key & action table
	    char *TxtPtr;              // text pointer
	    void *AuxPtr;              // auxiliary pointer
	    Boolean (*KeybProc)(struct WindStruct *); // keyboard routine
	    void (*ReptProc)(struct WindStruct *); // repainting
	    void (*KillProc)(struct WindStruct *); // cleanup routine
	    word (*ShowProc)(void);    // 1 char display routine
	    struct WindStruct *Next;   // pointer to the next Wind structure
	  } Wind;

typedef	void	(*MenuFunType)(Wind *);

typedef	enum { TypeBool,TypeByte,TypeInt,TypeLong,TypeString,TypeProc } IniType;

typedef	struct	{ Boolean Save;
		  IniType Type;
		  char *Name;
		  void *Addr, *Min, *Max;
		} IniItem;

extern	Boolean	Mono;			// video mode flag
extern	Boolean	InsMode;		// insert mode flag for edit
extern	Boolean	Status;			// status line visibility flag
extern	word far * Screen;		// video memory address
extern	int	ProtectDelay;		// screen protect delay
extern	long	ProtectTime;		// in timer ticks
extern	int	ProtectSpeed;		// animation speed in [1/s]
extern	int	MaxScrHig, MaxScrWid;	// screen size
extern	int	ZoomTop, ZoomBot;	// vertical zoom bounds
extern	int	StdX, StdY;		// standard window's place
extern	int	HelpNo;			// actual help number
extern	int	ScrollLockHelpNo;	// scroll mode help number
extern	Keys	LastKey;		// the recent key code
extern	Keys	NextKey;		// the "stuff" buffer
extern	Keys *	KeyPipe;		// key list for above
extern	Boolean	Critic;			// Critical error service flag
extern	Boolean SaveAll;		// forces saving all of .INI items
extern	Wind	*KeyBarWind;		// key bar window
extern	Wind	*BigClockWind;		// BigClock window
extern	byte	Colors[2][ColorBanks][ColorRegs]; // colors
extern	char	FrDes[];		// frame design
extern	char	HrdErrDevTxt[];		// "Device" text
extern	char	HrdErrDrvTxt[];		// "Drive" text
extern	char	HrdErrQueTxt[];		// "Retry Cancel" text
extern	char	*HrdErrInfo[];		// hard error message table
extern	char	Polskie[18+1];		// polish national characters
#define		Male	Polskie
#define		Duze	(Polskie+9)
EC extern	void	(*Log) (const char *, const char *); // log user's routine


//----- standard Show functions ----
EC word	ShowFrameChar	(void); /* WindHit result in AX */
EC word	ShowWindChar	(void);
EC word	ShowPureChar	(void);
EC word	ShowFieldChar	(void);
EC word	ShowScreenChar	(void);
EC word	ShowMenuChar	(void);
EC word	ShowHMChar	(void);

//---- window creation ----
EC Wind *	CreateWind	(int l, int t, int r, int b, ColorBank c, Boolean room, Boolean frm);
EC Wind *	MakeWind	(int l, int t, int r, int b, /*char *tit,*/ ColorBank c);
EC Wind *	MakeField	(int l, int t, int r, int b, ColorBank c, char ch);
EC Wind *	MakeMenu	(int l, int t, /*char *t,*/ ColorBank c, char *mnu, int max, int pos);
EC Wind *	MakeHMnu	(int l, int t, int ColorBank, char *mnu);
EC Wind *	FormatMenu	(Wind *W);
EC Wind *	MakeClock	(int l, int t, ColorBank c, Boolean day);
EC Wind *	MakeBigClock	(int l, int t, ColorBank c);
EC Wind *	MakeDebug	(DebugEntry *tab, void (*fun)(void));
EC Wind *	MakeProgress	(int l, int t, ColorBank cb, char *msg, long total);
EC Wind *	MakeWorm	(void);

//---- idle processes ----
EC void	Idle		(void);
EC void	BigClock	(Boolean appear);
EC void	ShowBigClock	(Wind *W, int x, int y);
EC void	ShowDebug	(Wind *W);
EC Boolean	Progress	(void);

//---- window query ----
EC Wind *	NumToWind	(int num);
EC Wind *	PosToWind	(int x, int y);
EC Wind *	TopWind		(void);
EC int	RelX		(Wind *W, int x);
EC int	RelY		(Wind *W, int y);
EC int	AbsX		(Wind *W, int x);
EC int	AbsY		(Wind *W, int y);
EC int	WindWid		(Wind *W);
EC int	WindHig		(Wind *W);
EC int	WindBodyWid	(Wind *W);
EC int	WindBodyHig	(Wind *W);
EC int	WindHit		(void);

//---- window output ----
EC void	WriteWindMem	(Wind *W, int x, int y, const char *txt, int len);
EC void	WriteWindText	(Wind *W, int x, int y, const char *txt);
EC void	WriteWindInt	(Wind *W, int x, int y, int n);
EC void	WriteWindLong	(Wind *W, int x, int y, long n);
EC void	WriteWindWord	(Wind *W, int x, int y, word n);
EC void	WriteWindDword	(Wind *W, int x, int y, dword n);
EC void	DrawHLine	(Wind *W, int row, char c);
EC void WriteWindMemConvr  (Wind *W, int x, int y, const char *txt, int len);
EC void WriteWindTextConvr (Wind *W, int x, int y, const char *txt);

//---- basic window services ----
EC void	SetRefresh	(int l, int t, int r, int b); // absolute l,t,r,b
EC void	ResetRefresh	(void);
EC void	SetRefreshChar	(Wind *W, int x, int y); // relative x,y
EC void	SetRefreshWind	(Wind *W);
EC void	SetRefreshTitle	(Wind *W);
EC void	RepaintMenuWind	(Wind *W);
EC void	SetRefreshDesk	(void);
EC void	Refresh		(void);
EC void	CursorGoto	(Wind *W, int x, int y);
EC void	ShowWind	(Wind *W);
EC void   	HideWind	(Wind *W);
EC Boolean	CloseWind	(Wind *W);
EC Boolean	ZoomWind	(Wind *W);
EC void	DragWind	(Wind *W);
EC void	ResizeWind	(Wind *W);
EC void	MoveWindHorz	(Wind *W, int x);
EC void	MoveWindVert	(Wind *W, int y);
EC void	CenterWind	(Wind *W);
EC void	MoveWind	(Wind *W, int x, int y);
EC void	SetTitle	(Wind *W, const char *txt, int x1);
EC void	CenterTitle	(Wind *W);
EC void	StdMove		(Wind *W);
EC void	BringToFront	(Wind *W);
EC void	PushDown	(Wind *W);

//---- new style key response ----
EC extern	KeyEntry	HMKeys[];
EC Boolean	HMKey		(Wind *W);
EC void	HMRepaint	(Wind *W);

//---- higher level window routines ----
EC Boolean	ColorChoice	(int x, int y, char *names);
EC int	MenuChoice	(Wind *W);
EC int	HorzChoice	(Wind *W);
EC int	Question	(ColorBank c, const char *m, const char *q);
EC void	Message		(ColorBank c, const char *m);
EC int	CriticalError	(int err, int dev, word dhs, word dho);
EC void	DisableBreak	(void);
EC void	RestoreBreak	(void);
EC Wind *	SetDesk		(Boolean on);
EC void	SetDeskFill	(char fill);
EC Boolean	WindHelp	(const char *filename, const char *title);
EC Boolean	LoadIni		(const char *fname, IniItem *tab, Boolean infmode);
EC Boolean	SaveIni		(const char *fname, IniItem *tab);
EC IniItem * IniEntry	(IniItem *tab, void * adr);
EC char *	GetString	(char * str, int len, int caps,
			 const char * title, ColorBank c);

//---- menu keyboard support ----
EC Boolean	MenuUp		(Wind *W);
EC Boolean	MenuDown	(Wind *W);
EC Boolean	MenuPgUp	(Wind *W);
EC Boolean	MenuPgDn	(Wind *W);
EC Boolean	MenuHome	(Wind *W);
EC Boolean	MenuEnd		(Wind *W);
EC Boolean	MenuMouse	(Wind *W);

//---- key bar functions ----
EC Wind *	MakeKeyBar	(const char *bar);
EC void	SetKeyBar	(const char *bar);
EC void	SetKeyLabel	(Keys fn, const char *lab);
EC void	CloseKeyBar	(void);

//---- status line functions ----
EC Wind *	MakeStatusWind	(void);
EC const char * SetStatus	(const char * s);
EC void	FlipStatus	(void);

//---- timer support ----
#ifndef __GENERAL__
EC char *	DateTimeString	(void);
EC long	TimerLimit	(word t);
EC long	TimerLimitS	(word sec);
EC Boolean	TimerLimitGone	(long t);
#endif
EC void	ProtectSuppress	(void);
EC void	Protect		(void);

//---- keyboard ----
EC void	ClearKeyboard	(void);
EC Keys	GetKey		(void);
EC Keys	WindKeyProc	(Keys k);
EC Boolean	KeyExec		(Wind * W, Keys key, KeyEntry * tab);
EC Boolean	KeyService	(Wind *W);

//---- conversions ----
EC int	Upper		(int);
EC int	UpperPL		(int);
EC int	UpperCmp	(char * m1, char * m2, int len);
EC char * StrConvr		(char * str);
EC char * MemConvr		(char * mem, int len);
EC int	MemTrmLen	(const char * m, int len);
EC int	mtoi		(const char * m, int radix, int len);
EC long	mtol		(const char * m, int radix, int len);
#ifndef __GENERAL__
EC char *	itom		(int val, char * str, int radix, int len);
EC char *	ltom		(long val, char * str, int radix, int len);
#endif

//---- user function support ----
EC void	(*GetIdleProc(void))();
EC void	(*GetHelpProc(void))();
EC Keys	(*GetKeyProc(void))();
EC void	SetIdleProc	(void (*ip)(void));
EC void	SetHelpProc	(void (*h0)(void));
EC void	SetSaveProc	(Wind * (*sp)(void));
EC void	SetKeyProc	(Keys (*kp)(Keys));
EC void	SetMenuProc	(void (*mp)(int));
EC void	SetUpperProc	(int (*up)(int));
EC void	SetConvrProc	(char (*cp)(char));
EC MouseFunType *	SetMouseFun(MouseFunType *p, word f);
EC MouseFunType	StdMouseFun;


//---- system ----
EC void	interrupt IretCode	(void);
EC int	_fastcall FileFlush	(int handle);
EC int	_fastcall IsDrvFixed	(int drv); // drv=0 for default, 1 for A:, ...
EC int	_fastcall IsDrvRemote	(int drv); // drv=0 for default, 1 for A:, ...
EC long	Ticks		(word sec);
EC dword	DwordFileTime	(const char *fname);
EC dword	GetTime		(void);
EC void	Move		(void far * dst, void far * src, unsigned len);
EC word	*SaveScreen	(void);
EC void	RestoreScreen	(word *ssav);

//---- DBF support functions ----
EC Boolean	WindDbfSay	(Wind *W, int x, int y, int fld);
//void	WindDbfGet	(Wind *W);


#include	<edit.h>

#endif
