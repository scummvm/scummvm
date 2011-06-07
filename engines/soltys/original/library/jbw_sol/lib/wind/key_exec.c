#include	<wind.h>



Boolean KeyExec (Wind * W, Keys key, KeyEntry * tab)
{
  if (tab != NULL)
    while (tab->Key != NoKey)
      if (tab->Key == key) return tab->Proc(W); else ++ tab;
  return FALSE;
}
