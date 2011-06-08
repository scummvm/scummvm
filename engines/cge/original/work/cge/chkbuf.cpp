#include	"ident.h"
#include	<security.h>
#include	<general.h>
#include	<boot.h>
#include	<alloc.h>
#include	<dir.h>

#define		dif	0x13
#define		cpt	(sizeof(id)-sizeof(id.cork))
#define		dpt	(cpt-sizeof(id.disk))


static	IDENT	id = { "V‚€€‚3Zt€x3Xz|x", "", 0, 0 };
static	Boolean	ok;





int InitHook (int drv)
{
  ok = DskChk(drv);
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
	      Boot * b = ReadBoot(getdisk());
	      id.disk = b->Serial;
	      free(b);
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