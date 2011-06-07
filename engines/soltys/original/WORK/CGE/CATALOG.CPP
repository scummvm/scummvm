#include	"catalog.h"
#include	"mouse.h"
#include	<string.h>







CATALOG *	CATALOG::Ptr		= NULL;
char *		CATALOG::FileName	= NULL;




CATALOG::CATALOG (const char * wild, void (*proc)(void))
: Proc(proc), Shift(0), WinPos(0), Scroll(0)
{
  int i, n;
  ffblk fb;

  FileName = NULL;
  Mode = RECT;
  //Delay = 5;
  TS[0] = Box(2*TEXT_HM + 8*8, 2*TEXT_VM+CAT_HIG*(FONT_HIG+TEXT_LS)-TEXT_LS);
  SetShapeList(TS);
  Ptr = this;
  Flags.Kill = TRUE;
  Flags.BDel = TRUE;
  fnsplit(wild, NULL, NULL, NULL, Ext);

  // count up files
  n = 0;
  for (i = findfirst(wild, &fb, 0); i == 0; i = findnext(&fb)) ++ n;

  // allocate space
  Cat = new FNAME[n];
  if (Cat == NULL)
    VGA::Exit("CATALOG::CATALOG - No room for filenames", wild);

  // fill table with names
  CatMax = 0;
  for (i = findfirst(wild, &fb, 0); i == 0 && CatMax < n; i = findnext(&fb))
    {
      char * p = strchr(fb.ff_name, '.');
      int l = (p) ? (p - fb.ff_name) : strlen(fb.ff_name);
      _fmemcpy(Cat[CatMax], fb.ff_name, l);
      Cat[CatMax][l] = '\0';
      ++ CatMax;
    }
  Paint();
  VGA::ShowQ.Insert(this, VGA::ShowQ.Last());
  Bar = new MENU_BAR(W - 2 * TEXT_HM);
  Bar->Goto(X + TEXT_HM - MB_HM, Y + TEXT_VM - MB_VM);
  VGA::ShowQ.Insert(Bar, VGA::ShowQ.Last());
}





CATALOG::~CATALOG (void)
{
  delete[] Cat;
  Ptr = NULL;
}






void CATALOG::Paint (void)
{
  int i;
  for (i = 0; i < CAT_HIG; i ++)
    {
      PutLine(i, (Shift+i < CatMax) ? Cat[Shift+i] : NULL);
    }
}




void CATALOG::Tick (void)
{
  if (Scroll)
    {
      if (Scroll < 0) { if (Shift) -- Shift; }
      else { if (Shift+CAT_HIG < CatMax) ++ Shift; }
      Paint();
    }
}





void CATALOG::Touch (word mask, int x, int y)
{
#define h (FONT_HIG+TEXT_LS)
  Boolean ok = FALSE;

  SPRITE::Touch(mask, x, y);

  if (y < 0 && Shift) Scroll = -1;
  else
    if (y > (int) H && Shift+CAT_HIG < CatMax) Scroll = 1;
    else Scroll = 0;

  y -= TEXT_VM;

  if (y > 0)
    {
      WinPos = y / h;
      if (WinPos < CAT_HIG) ok = (x >= TEXT_HM && x < W - TEXT_HM && y % h < FONT_HIG);
      else WinPos = CAT_HIG-1;
    }

  Bar->Goto(X + TEXT_HM - MB_HM, Y + TEXT_VM + WinPos * h - MB_VM);

  if (ok && (mask & L_UP))
    {
      SNPOST_(SNKILL, -1, 0, this);
      if (Shift+WinPos < CatMax)
	{
	  static char s[MAXFILE-1+MAXEXT];
	  _fstrcpy(s, Cat[Shift+WinPos]);
	  strcat(s, Ext);
	  FileName = s;
	  Proc();
	}
    }
#undef h
}
