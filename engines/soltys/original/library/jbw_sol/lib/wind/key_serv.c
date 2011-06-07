#include	<wind.h>



Boolean KeyService (Wind *W)
{
  return KeyExec(W, LastKey, W->KeyTab);
}
