#include	<wind.h>
#include	<string.h>



extern	Wind	*KeyBarWind;




void SetKeyBar (const char *bar)
{
  if (KeyBarWind != NULL && KeyBarWind->Body.Near != bar)
    {
      KeyBarWind->Body.Near = bar;
      SetRefreshWind(KeyBarWind);
    }
}
