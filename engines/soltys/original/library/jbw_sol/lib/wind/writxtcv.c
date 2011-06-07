#include	<wind.h>
#include	<string.h>





void WriteWindTextConvr (Wind *W, int x, int y, const char *txt)
{
  WriteWindMemConvr(W, x, y, txt, strlen(txt));
}

