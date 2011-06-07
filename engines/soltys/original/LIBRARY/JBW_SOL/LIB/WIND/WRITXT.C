#include	<wind.h>
#include	<string.h>





void WriteWindText (Wind *W, int x, int y, const char *txt)
{
  WriteWindMem(W, x, y, txt, strlen(txt));
}
