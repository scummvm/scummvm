#include	<wind.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#define		MaxHelpWid	48
#define		MaxHelpHig	12
#define		HelpSepChr	'#'

extern	int	HelpNo = 0;



Boolean WindHelp (const char *fname, const char *title)
{
  static Wind *w = NULL;
  FILE *hf; char line[MaxHelpWid+5];
  int i = -1, l, n = HelpNo;

  if (w != NULL) return TRUE;
  w = MakeWind(0, 0, MaxHelpWid + 3, MaxHelpHig + 1, HLP);
  if (w != NULL)
    {
      w->Flags.Movable = TRUE;
      CenterWind(w);
      if (title != NULL) SetTitle(w, title, -1);
      ShowWind(w);
      if (n > 999) n += w->Next->Vp;
      hf = fopen(fname, "rt");
      if (hf != NULL)
	{
	  while (TRUE)
	    {
	      if (fgets(line, MaxHelpWid+1, hf) == NULL) break;
	      if (*line == HelpSepChr) if ((i=atoi(line+1)) == n) break;
	    }
	  if (i == n)
	    {
	      Boolean eol = TRUE;
	      for (i = 0; i < MaxHelpHig; i ++)
		{
		  again:
		  if (fgets(line, MaxHelpWid+1, hf) == NULL) break;
		  if (*line == HelpSepChr) break;
		  l = strlen(line);
		  if (l < 2 && ! eol) goto again;
		  eol = line[l-1] == '\n';
		  if (eol) line[--l] = '\0';
		  WriteWindText(w, 1, i, line);
		}
	    }
	  else i = -1;
	  fclose(hf);
	}
      if (i != -1) GetKey();
      CloseWind(w);
      w = NULL;
    }
  return (i != -1);
}
