#include	"btfile.h"
#include	<alloc.h>
#include	<string.h>



#ifdef	DROP_H
  #include	"drop.h"
#else
  #include	<stdio.h>
  #include	<stdlib.h>
  #define	DROP(m,n)	{ printf("%s [%s]\n", m, n); _exit(1); }
#endif


#ifndef	BT_SIZE
  #define	BT_SIZE		K(1)
#endif

#ifndef	BT_KEYLEN
  #define	BT_KEYLEN	13
#endif





BTFILE::BTFILE (const char * name, IOMODE mode, CRYPT * crpt)
: IOHAND(name, mode, crpt)
{
  int i;
  for (i = 0; i < BT_LEVELS; i ++)
    {
      Buff[i].Page = new far BT_PAGE;
      Buff[i].PgNo = BT_NONE;
      Buff[i].Indx = -1;
      Buff[i].Updt = FALSE;
      if (Buff[i].Page == NULL) DROP("No core", NULL);
    }
}









BTFILE::~BTFILE (void)
{
  int i;
  for (i = 0; i < BT_LEVELS; i ++)
    {
      PutPage(i);
      delete Buff[i].Page;
    }
}






void BTFILE::PutPage (int lev, Boolean hard)
{
  if (hard || Buff[lev].Updt)
    {
      Seek(Buff[lev].PgNo * sizeof(BT_PAGE));
      Write((byte far *) Buff[lev].Page, sizeof(BT_PAGE));
      Buff[lev].Updt = FALSE;
    }
}






BT_PAGE far * BTFILE::GetPage (int lev, word pgn)
{
  if (Buff[lev].PgNo != pgn)
    {
      dword pos = pgn * sizeof(BT_PAGE);
      PutPage(lev);
      Buff[lev].PgNo = pgn;
      if (Size() > pos)
	{
	  Seek((dword) pgn * sizeof(BT_PAGE));
	  Read((byte far *) Buff[lev].Page, sizeof(BT_PAGE));
	  Buff[lev].Updt = FALSE;
	}
      else
	{
	  Buff[lev].Page->Hea.Count = 0;
	  Buff[lev].Page->Hea.Down = BT_NONE;
	  _fmemset(Buff[lev].Page->Data, '\0', sizeof(Buff[lev].Page->Data));
	  Buff[lev].Updt = TRUE;
	}
      Buff[lev].Indx = -1;
    }
  return Buff[lev].Page;
}





BT_KEYPACK far * BTFILE::Find (const byte * key)
{
  int lev = 0;
  word nxt = BT_ROOT;
  while (! Error)
    {
      BT_PAGE far * pg = GetPage(lev, nxt);
      // search
      if (pg->Hea.Down != BT_NONE)
	{
	  int i;
	  for (i = 0; i < pg->Hea.Count; i ++)
	    if (_fmemicmp(key, pg->Inn[i].Key, BT_KEYLEN) < 0)
	      break;
	  nxt = (i) ? pg->Inn[i-1].Down : pg->Hea.Down;
	  Buff[lev].Indx = i-1;
	  ++ lev;
	}
      else
	{
	  int i;
	  for (i = 0; i < pg->Hea.Count-1; i ++)
	    if (_fstricmp(key, pg->Lea[i].Key) <= 0)
	      break;
	  Buff[lev].Indx = i;
	  return &pg->Lea[i];
	}
    }
  return NULL;
}




int keycomp (const void far * k1, const void far * k2)
{
  return _fmemicmp(k1, k2, BT_KEYLEN);
}



void BTFILE::Make(BT_KEYPACK far * keypack, word count)
{
  #if BT_LEVELS != 2
    #error This tiny BTREE implementation works with exactly 2 levels!
  #endif
  _fqsort(keypack, count, sizeof(*keypack), keycomp);
  word n = 0;
  BT_PAGE far * Root = GetPage(0, n ++),
	  far * Leaf = GetPage(1, n);
  Root->Hea.Down = n;
  PutPage(0, TRUE);
  while (count --)
    {
      if (Leaf->Hea.Count >= ArrayCount(Leaf->Lea))
	{
	  PutPage(1, TRUE);		// save filled page
	  Leaf = GetPage(1, ++n);	// take empty page
	  _fmemcpy(Root->Inn[Root->Hea.Count].Key, keypack->Key, BT_KEYLEN);
	  Root->Inn[Root->Hea.Count ++].Down = n;
	  Buff[0].Updt = TRUE;
	}
      Leaf->Lea[Leaf->Hea.Count ++] = * (keypack ++);
      Buff[1].Updt = TRUE;
    }
}
