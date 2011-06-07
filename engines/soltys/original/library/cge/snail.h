#ifndef		__SNAIL__
#define		__SNAIL__

#include	<jbw.h>

#define		POCKET_X	174
#define		POCKET_Y	176
#define		POCKET_DX	18
#define		POCKET_DY	22
#define		POCKET_NX	8
#define		POCKET_NY	1

#define		POCKET_SX	8
#define		POCKET_SY	3

#define		SNINSERT(c,r,v,p)	Snail.InsCom(c,r,v,p)
#define		SNPOST(c,r,v,p)		Snail.AddCom(c,r,v,p)
#define		SNPOST_(c,r,v,p)	Snail_.AddCom(c,r,v,p)



typedef	struct	{ byte Horz, Vert; } BAR;



struct SCB
{
  byte far *	Ptr;
  word		Siz;
  SCB *		Nxt;
};



enum	SNCOM	{ SNLABEL, SNPAUSE, SNWAIT, SNLEVEL,
		  SNHIDE, SNSAY, SNINF, SNTIME,
		  SNCAVE, SNKILL, SNRSEQ,
		  SNSEQ, SNSEND, SNSWAP, SNKEEP, SNGIVE,
		  SNIF, SNGAME, SNSETX0, SNSETY0, SNSLAVE,
		  SNSETXY, SNRELX, SNRELY, SNRELZ,
		  SNSETX, SNSETY, SNSETZ, SNTRANS, SNPORT,
		  SNNEXT, SNNNEXT, SNTNEXT, SNRNNEXT, SNRTNEXT,
		  SNRMNEAR, SNRMTAKE, SNFLAG, SNSETREF,
		  SNBACKPT, SNFLASH, SNLIGHT,
		  SNSETHB, SNSETVB,
		  SNWALK, SNREACH, SNCOVER, SNUNCOVER,
		  SNCLEAR, SNTALK, SNMOUSE,
		  SNSOUND, SNCOUNT,
		  SNEXEC, SNSTEP, SNZTRIM,
		  SNGHOST
		};

enum	SNLIST	{ NEAR, TAKE };

class SNAIL
{
  struct COM { SNCOM Com; int Ref; int Val; void * Ptr; } far * SNList;
  byte Head, Tail;
  Boolean Turbo, Busy, TextDelay;
  word Pause;
public:
  static char * ComTxt[];
  Boolean TalkEnable;
  SNAIL (Boolean turbo = FALSE);
  ~SNAIL (void);
  void RunCom (void);
  void AddCom (SNCOM com, int ref = 0, int val = 0, void * ptr = NULL);
  void InsCom (SNCOM com, int ref = 0, int val = 0, void * ptr = NULL);
  Boolean Idle (void);
};





void	SelectPocket	(int n);
void	PocFul		(void);







extern	SCB		Scb;
extern	Boolean		Flag[4];
extern	Boolean		Game;
extern	Boolean		Dark;
extern	SNAIL		Snail;
extern	SNAIL		Snail_;
extern	int		Now;
extern	int		Lev;
extern	int		MaxCave;
extern	int		PocPtr;
extern	BAR		Barriers[];
extern	struct HXY { int X; int Y; } HeroXY[];

#endif
