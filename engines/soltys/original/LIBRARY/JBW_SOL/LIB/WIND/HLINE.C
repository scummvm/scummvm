#include	<wind.h>
#include	<mem.h>




void DrawHLine (Wind *W, int row, char c)
{
  memset(((char *) W->Body.Near) + row * W->Wid, c, W->Wid);
}





