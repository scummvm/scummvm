#include	<wind.h>
#include	<string.h>


void SetTitle (Wind *w, const char *txt, int x1)
{
  w->Title.Text = (char *) txt;
  if (x1 < 0)
    {
      w->Flags.TCenter = TRUE;
      CenterTitle(w);
    }
  else
    {
      SetRefreshTitle(w);
      w->Title.X1 = x1;
      w->Title.X2 = x1+((txt == NULL) ? 0 : strlen(txt))-1;
      SetRefreshTitle(w);
    }
}






