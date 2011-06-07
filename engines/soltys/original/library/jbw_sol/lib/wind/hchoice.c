#include	<wind.h>

extern	void	(*MenuProc)	(int);

int HorzChoice (Wind * W)
{
  W->ShowProc = ShowHMChar;
  W->ReptProc = HMRepaint;
  W->KeyTab = HMKeys;
  while (TRUE)
    {
      W->Flags.Repaint = TRUE;
      GetKey();
      if (! HMKey(W)) switch (LastKey)
	{
	  case Enter	: return (W->Vp);
	  case MouseRight:
	  case Esc	: return -1;
	}
    }
}
