#include	"gettext.h"
#include	"keybd.h"
#include	"mouse.h"
#include	<string.h>





GET_TEXT *	GET_TEXT::Ptr	= NULL;



GET_TEXT::GET_TEXT (const char * info, char * text, int size, void (*click)(void))
: Text(text), Size(min(size, GTMAX)), Len(min(Size, strlen(text))),
  Cntr(GTBLINK), Click(click), OldKeybClient(KEYBOARD::SetClient(this))
{
  int i = 2 * TEXT_HM + Font.Width(info);
  Ptr = this;
  Mode = RECT;
  TS[0] = Box((i + 3) & ~3, 2 * TEXT_VM + 2 * FONT_HIG + TEXT_LS);
  SetShapeList(TS);
  Flags.BDel = TRUE;
  Flags.Kill = TRUE;
  memcpy(Buff, text, Len);
  Buff[Len] = ' ';
  Buff[Len+1] = '\0';
  PutLine(0, info);
  Tick();
}






GET_TEXT::~GET_TEXT (void)
{
  KEYBOARD::SetClient(OldKeybClient);
  Ptr = NULL;
}






void GET_TEXT::Tick (void)
{
  if (++ Cntr >= GTBLINK)
    {
      Buff[Len] ^= (' ' ^ '_');
      Cntr = 0;
    }
  PutLine(1, Buff);
  Time = GTTIME;
}




void GET_TEXT::Touch (word mask, int x, int y)
{
  static char ogon[] = "èïêú•£ò†°";
  static char bezo[] = "ACELNOSXZ";
  char * p;

  if (mask & KEYB)
    {
      if (Click) Click();
      switch (x)
	{
	  case Enter : Buff[Len] = '\0'; strcpy(Text, Buff);
		       for (p = Text; *p; p ++)
			 {
			   char * q = strchr(ogon, *p);
			   if (q) *p = bezo[q-ogon];
			 }
	  case Esc   : SNPOST_(SNKILL, -1, 0, this); break;
	  case BSp   : if (Len)
			 {
			   -- Len;
			   Buff[Len] = Buff[Len+1];
			   Buff[Len+1] = Buff[Len+2];
			 }
		       break;
	  default    : if (x < 'A' || x > 'Z')
			 {
			   if (OldKeybClient)
			     OldKeybClient->Touch(mask, x, y);
			 }
		       else
			 {
			   if (KEYBOARD::Key[ALT])
			     {
			       p = strchr(bezo, x);
			       if (p) x = ogon[p-bezo];
			     }
			   if (Len < Size && 2 * TEXT_HM + Font.Width(Buff) + Font.Wid[x] <= W)
			     {
			       Buff[Len+2] = Buff[Len+1];
			       Buff[Len+1] = Buff[Len];
			       Buff[Len ++] = x;
			     }
			 }
		       break;
	}
    }
  else SPRITE::Touch(mask, x, y);
}
