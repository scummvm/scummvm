#include	"ident.h"
#include	<security.h>
#include	<general.h>
#include	<boot.h>
#include	<alloc.h>
#include	<dir.h>
#include	<dos.h>
#include	<stdio.h>


#define		dif	0x13
#define		cpt	(sizeof(id)-sizeof(id.cork))
#define		dpt	(cpt-sizeof(id.disk))
#define		dat	(*(dosdate_t *) &id.disk)



static	IDENT	id = { "V‚€€‚3Zt€x3Xz|x", "", 0, 0 };
static	Boolean	ok;




#pragma argsused
int InitHook (int drv)
{
  ok =
  #ifdef EVA
       TRUE;
  #else
       DskChk(drv);
  #endif
  return 1;
}





int CopyHook (char c)
{
  static int ptr = 0;
  if (ptr >= 0)
    {
      static int match = 0;
      if (match)
	{
	  if (ptr == cpt)
	    {
	       id.cork = c = dif * match - ChkSum(&id, sizeof(id));
	       ptr = -1;
	    }
	  else
	    {
	      if (ptr >= dpt) c = id.copr[ptr];
	      else id.copr[ptr] = c;
	      ++ ptr;
	    }
	}
      else
	{
	  if (id.copr[ptr] == 0)
	    {
	      #ifdef EVA
	      _dos_getdate(&dat);
	      if ((dat.month += 2) > 12)
		{
		  dat.month -= 12;
		  ++dat.year;
		}
	      #else
	      Boot * b = ReadBoot(getdisk());
	      id.disk = (b->XSign == 0x29) ? b->Serial : b->lTotSecs;
	      free(b);
	      #endif
	      id.cork = 0;
	      id.copr[match = ptr++] = c;
	      if (! ok) ptr = -1;
	    }
	  else
	    if (c+dif == id.copr[ptr]) ++ ptr; else ptr = 0;
	}
    }
  return c;
}