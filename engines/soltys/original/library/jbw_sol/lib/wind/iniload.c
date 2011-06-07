#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>




extern	Boolean SaveAll = FALSE;




Boolean LoadIni (const char *fname, IniItem *tab, Boolean infmode)
{
  FILE *hf;

  hf = fopen(fname, "rt");
  if (hf == NULL) return FALSE;
  while (TRUE)
    {
      char *p, *q, line[128];
      IniItem * t, * tn;
      int l = 0, n;
      long ln;

      if (fgets(line, sizeof(line), hf) == NULL) break;
      for (t = tab; t->Addr != NULL; t ++)
	{
	  char *s = t->Name; int l1 = strlen(s);
	  if (l1 > l && memicmp(s, line, l1) == 0)
	    {
	      tn = t;
	      l = l1;
	    }
	}
      if (l > 0)
	{
	  IniType it = tn->Type;
	  void * a = tn->Min, * b = tn->Max, * x = tn->Addr;
	  if (! infmode) tn->Save = TRUE;
	  p = line + l;
	  if ((it == TypeProc) || (*p != '\n' && *p != '\0')) switch(it)
	    {
	      case TypeBool  : n = atoi(p);
			       * (Boolean *) x = (n != 0);
			       break;
	      case TypeByte  :
	      case TypeInt   : n = atoi(p);
			       if (a) n = max(n, * (int *) a);
			       if (b) n = min(n, * (int *) b - 1);
			       if (it == TypeInt) * (int *) x = n;
			       else               * (byte*) x = n;
			       break;
	      case TypeLong  : ln = atol(p);
			       if (a) ln = max(ln, * (long *) a);
			       if (b) ln = min(ln, * (long *) b - 1);
			       * (long *) x = ln;
			       break;
	      case TypeString: if (a) memset(x, '\0', * (int *) a);
			       if (b) n = * (int *) b;
			       q = strchr(p, '\n');
			       if (q != NULL) *q = '\0';
			       l = strlen(p)+1;
			       memcpy(x, p, min(l, n));
			       break;
	      case TypeProc  : if (infmode) ((void(*)(void))(x))();
			       break;
	    }
	}
    }
  fclose(hf);
  return TRUE;
}
