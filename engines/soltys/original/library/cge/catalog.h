#ifndef		__CATALOG__
#define		__CATALOG__

#include	"talk.h"
#include	"vmenu.h"
#include	<dir.h>



#define		CAT_HIG		8



typedef	char	FNAME[MAXFILE];




class CATALOG : public TALK
{
  int Shift, WinPos, CatMax;
  int Scroll;
  char Ext[MAXEXT];
  void (*Proc)(void);
  MENU_BAR * Bar;
  FNAME * Cat;
public:
  static CATALOG * Ptr;
  static char * FileName;
  CATALOG (const char * wild, void (*proc)(void));
  ~CATALOG (void);
  void Paint (void);
  void Touch (word mask, int x, int y);
  void Tick (void);
};




#endif