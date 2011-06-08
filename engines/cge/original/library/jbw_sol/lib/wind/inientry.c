#include	<wind.h>





IniItem * IniEntry (IniItem *tab, void * adr)
{
  while (tab->Addr != NULL) if (tab->Addr == adr) break; else ++ tab;
  return tab;
}
