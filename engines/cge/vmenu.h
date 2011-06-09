#ifndef		__VMENU__
#define		__VMENU__

#include	"talk.h"

#define		MB_VM		1
#define		MB_HM		3



typedef	struct	{ char * Text; void (* Proc)(void); } CHOICE;






class MENU_BAR : public TALK
{
public:
  MENU_BAR (word w);
};







class VMENU : public TALK
{
  word Items;
  CHOICE * Menu;
public:
  static VMENU * Addr;
  static int Recent;
  MENU_BAR * Bar;
  VMENU (CHOICE * list, int x, int y);
  ~VMENU (void);
  void Touch (word mask, int x, int y);
};



#endif
