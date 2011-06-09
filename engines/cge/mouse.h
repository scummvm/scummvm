#ifndef		__MOUSE__
#define		__MOUSE__

#include	"game.h"
#include	"talk.h"

#define		EVT_MAX		256

#define		ROLL		0x01
#define		L_DN		0x02
#define		L_UP		0x04
#define		R_DN		0x08
#define		R_UP		0x10
#define		ATTN		0x20
//				0x40
#define		KEYB		0x80


extern	TALK *	Talk;

struct	EVENT	{ word Msk;
		  word X, Y;
		  SPRITE * Ptr;
		};
extern	EVENT	Evt[EVT_MAX];
extern	word	EvtHead, EvtTail;
typedef	void	(far MOUSE_FUN)		(void);





class MOUSE : public SPRITE
{
  static MOUSE_FUN * OldMouseFun;
  static MOUSE_FUN NewMouseFun;
  static word OldMouseMask;
  SPRITE * Hold;
  int hx, hy;
  //void SetFun (void);
  //void ResetFun (void);
public:
  Boolean Exist;
  int Buttons;
  SPRITE * Busy;
  //SPRITE * Touched;
  MOUSE (BITMAP ** shpl = MC);
  ~MOUSE (void);
  void On (void);
  void Off (void);
  static void ClrEvt (SPRITE * spr = NULL);
  void Tick (void);
};




#endif
