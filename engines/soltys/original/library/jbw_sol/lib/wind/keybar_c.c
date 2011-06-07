#include	<wind.h>
#include	<string.h>



extern	Wind	*KeyBarWind;






void CloseKeyBar (void)
{
  CloseWind(KeyBarWind);
  KeyBarWind = NULL;
  ++ ZoomBot;
}
