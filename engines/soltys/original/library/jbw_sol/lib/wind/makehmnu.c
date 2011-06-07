#include	<wind.h>
#include	<string.h>




extern	Wind	*WindStack;




Wind * MakeHMnu (int l, int t, int c, char *mnu)
{
  int n = strlen(mnu);
  if (CreateWind(l, t, l+n-1, t, c, FALSE, FALSE) == NULL) return NULL;
  WindStack->Body.Near = mnu;
  WindStack->Vp = 0;
  WindStack->ShowProc = ShowHMChar;
  return WindStack;
}
