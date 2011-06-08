#include	<general.h>
#include	"cfile.h"
#include	"vol.h"
#include	<conio.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<alloc.h>
#include	<ctype.h>

#define		IsFile(s)	(access(s,0)==0)
#define		IsWrit(s)	(access(s,2)==0)

#define		DROP(m,n)	{ printf("%s [%s]\n", (m), (n)); _exit(1); }

	word	_stklen	=	K(16);





static char * ExpandName (char * buff, const char * name)
{
  char c = ' ', e[MAXEXT], *p, *q = buff+8;

  fnsplit(name, NULL, NULL, buff, e);
  p = strchr(buff, '*');
  if (p) c = '?';
  else p = buff + strlen(buff);
  while (p < q) *(p ++) = c;
  *p = '\0';

  c = ' ';
  q = e+4;
  p = strchr(e, '*');
  if (p) c = '?';
  else p = e + strlen(e);
  while (p < q) *(p ++) = c;
  *p = '\0';

  return strcat(buff, e);
}



static int WildMatch (const char * name, const char * mask)
{
  char n[MAXFILE-1+MAXEXT], m[MAXFILE-1+MAXEXT];
  int i, d;

  ExpandName(n, name);
  ExpandName(m, mask);
  for (i = 0; i < 13; i ++)
    {
      d = (m[i] == '?') ? 0 : (n[i] - m[i]);
      if (d) break;
    }
  return d;
}



static Boolean Write (const char * fn)
{
  static Boolean ask = TRUE;
  VFILE vf(fn);
  if (vf.Error) DROP("Unable to access", fn);
  if (ask) if (CFILE::Exist(fn))
    {
      printf("File exists! Overwrite? ");
      switch (toupper(getch()))
	{
	  case 'Y' : puts("Yes"); break;
	  case 'A' : puts("All"); ask = FALSE; break;
	  case 'Q' : puts("Quit"); return FALSE;
	  default  : puts("No"); return TRUE;
	}
    }
  CFILE f(fn, WRI);
  if (! f.Error)
    {
      byte far * buf = new far byte[IOBUF_SIZE];
      if (buf == NULL) DROP("No core", NULL);
      while (TRUE)
	{
	  word n = vf.Read(buf, IOBUF_SIZE);
	  if (n == 0) break;
	  f.Write(buf, n);
	}
      delete[] buf;
    }
  return TRUE;
}






int main (int argc, char **argv)
{
  coreleft();
  farcoreleft();
  if (argc < 2)
    {
      char nam[MAXPATH];
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is    %s [-]filename\n", strupr(nam));
      return 1;
    }

  char pat[128];
  strupr(strcpy(pat, argv[1]));

  if (*pat == '+')
    {
      VFILE vf(pat+1, WRI);	// add all files
    }
  else
    {
      Boolean extract = (*pat == '=');
      char * p = pat+extract;
      if ((fnsplit(p, NULL, NULL, NULL, NULL) & WILDCARDS) == 0)
	{
	  if (extract) Write(p);
	  else puts("Found.");
	}
      else
	{
	  int count = 0;
	  const char * fn;

	  //VFILE::Exist("");
	  while ((fn = VFILE::Next()) != NULL)
	    {
	      if (WildMatch(fn, p) == 0)
		{
		  ++ count;
		  puts(fn);
		  if (extract) if (! Write(fn)) break;
		}
	    }
	  printf("\n%d file(s) found\n", count);
	}
    }
  return 0;
}
