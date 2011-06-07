#include	"btfile.h"
#include	<string.h>
#include	<stdio.h>
#include	<dir.h>

#define		CAT_MAX		3000


void main (void)
{
  static BT_KEYPACK tab[CAT_MAX];
  static char fn[] = "BTF.CAT";
  if (!BTFILE::Exist(fn))
    {
      ffblk ffb;
      int i, n = 0;
      for (i = findfirst("*.*", &ffb, 0); i == 0; i = findnext(&ffb))
	{
	  printf("adding file %s\n", ffb.ff_name);
	  memset(tab[n].Key, 0, BT_KEYLEN);
	  strupr(strcpy(tab[n].Key, ffb.ff_name));
	  tab[n].Mark = n;
	  tab[n].Size = 0;
	  if (++n >= CAT_MAX) break;
	}
      BTFILE btf(fn, WRI);
      if (n) btf.Make(tab, n);
    }

  BTFILE btf = fn;
  BT_KEYPACK far * btp;

  while ((btp = btf.Next()) != NULL)
    {
      char name[80];
      _fstrcpy(name, btp->Key);
      printf("%s\n", name);
    }
}
