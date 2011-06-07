#ifndef		__FEDIT__
#define		__FEDIT__

#include	"talk.h"
#include	"game.h"


#define		FEDIT_VM	3
#define		FEDIT_HM	3
#define		FEDIT_PT	7


class FEDIT : public TALK
{
  static char * BN[][2];
  BUTTON * Btn[4];
  BITMAP * Panel;
  byte ChrBuf[8];
  byte CurChr;
public:
  static FEDIT * Ptr;
  FEDIT (void);
  ~FEDIT (void);
  void Touch (word mask, int x, int y);
  void Tick (void);
  void GetChr (byte);
  void PutChr (void);
  void Plot (byte colr, word x, word y);
  void Update (void);
};





#endif