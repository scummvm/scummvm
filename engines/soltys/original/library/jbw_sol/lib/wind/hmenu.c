#include	<wind.h>
#include	<mem.h>
#include	<dos.h>



extern	void	(*MenuProc)	(int);




static Boolean FindHorItem (Wind *W, int n)
{
  int p, i = 0, l = W->Wid;
  char *s = Image(W) + (W->Y * l);
  while (i < l)
    {
      while (IsWhite(s[i]) && i < l) i ++;
      p = i;
      while (!IsWhite(s[i]) && i < l) i ++;
      if (n == 0 || p >= l) break;
      -- n;
    }
  if (p < l)
    {
      W->Hp = p;
      W->X = i;
      return TRUE;
    }
  return FALSE;
}





static int PointSeek (Wind * W, int x)
{
  register int i;
  for (i = 0; FindHorItem(W, i); i ++) if (W->X > x) break;
  return i;
}




Boolean HMLeft (Wind *W)
{
  if (W->Vp)
    {
      -- W->Vp;
      return TRUE;
    }
  return FALSE;
}






Boolean HMRight (Wind *W)
{
  if (FindHorItem(W, ++ (W->Vp))) return TRUE;
  -- W->Vp;
  return FALSE;
}






Boolean HMExec (Wind *W)
{
  MenuFunType *mf = W->AuxPtr;
  LastKey = Enter;
  if (mf == NULL) return FALSE;
  SetRefreshWind(W);
  Refresh();
  mf[W->Vp](W);
  return TRUE;
}






Boolean HMMouse (Wind *W)
{
  if (PosToWind(MX, MY) == W)
    {
      int x = RelX(W, MX), y = RelY(W, MY);
      if (y == W->Y && x >= 0 && x < WindBodyWid(W))
	{
	  if (!IsWhite(Image(W)[W->Wid * y + x]))
	    {
	      W->Vp = PointSeek(W, x);
	      SetRefreshWind(W);
	      if (LastKey == MouseLeft && MousePressed(1)) return TRUE;
	      //W->Vp = i;
	      return HMExec(W);
	    }
	}
    }
  return TRUE;
}






void HMRepaint (Wind *W)
{
  if (MenuProc != NULL) MenuProc(W->Vp);
  FindHorItem(W, W->Vp);
  // if (W != TopWind()) W->X = W->Hp;
  SetRefreshWind(W);
}





Boolean HMKey (Wind *W)
{
  Keys k = Upper(LastKey);

  W->Flags.Repaint = TRUE;
  if (KeyService(W)) return TRUE;
  if (IsAlpha(k))
    {
      char *p = Image(W) + W->Wid * W->Y;
      char *q =  memchr(p, k, W->Wid);
      if (q != NULL)
	{
	  W->Vp = PointSeek(W, q - p);
	  SetRefreshWind(W);
	  return HMExec(W);
	}
    }
  return FALSE;
}





extern	KeyEntry	HMKeys[] =  { { Left,		HMLeft    },
				      { Right,		HMRight   },
				      { Enter,		HMExec    },
				      { TwiceLeft,	HMMouse   },
				      { MouseLeft,	HMMouse   },
				      { NoKey,		NULL      } };
