#include	<wind.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>




extern	Boolean SaveAll;




Boolean SaveIni (const char *fname, IniItem *tab)
{
  FILE *hf = NULL; char *p, line[128];
  int n;


  for ( ; tab->Addr != NULL; tab ++) if (SaveAll || tab->Save)
    {
      void * x = tab->Addr;

      if (hf == NULL) if ((hf = fopen(fname, "wt")) == NULL) return FALSE;
      strcpy(line, tab->Name);
      p = line + strlen(line);
      switch (tab->Type)
	{
	  case TypeBool  : itoa(* (Boolean *) x, p, 10); break;
	  case TypeByte  : itoa(* (byte *)    x, p, 10); break;
	  case TypeInt   : itoa(* (int *)     x, p, 10); break;
	  case TypeLong  : ltoa(* (long *)    x, p, 10); break;
	  case TypeString: n = * (int *) (tab->Max);
			   strncpy(p, (char *) x, n);
			   p[n] = '\0'; break;
	}

      strcat(line, "\n");
      if (fputs(line, hf) == EOF)
	{
	  fclose(hf);
	  unlink(fname);
	  return FALSE;
	}
    }
  fclose(hf);
  return TRUE;
}
