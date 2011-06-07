#include	<general.h>
#include	"cfile.h"
#include	"vol.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<alloc.h>

#define		IsFile(s)	(access(s,0)==0)
#define		IsWrit(s)	(access(s,2)==0)

#define		DROP(m,n)	{ printf("%s [%s]\n", (m), (n)); _exit(1); }

	word	_stklen	=	K(16);



void main (int argc, char **argv)
{
  if (argc < 2)
    {
      char nam[MAXPATH];
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is    %s [-]filename\n", strupr(nam));
      exit(1);
    }

  char pat[128];
  strupr(strcpy(pat, argv[1]));

  if (*pat == '-')
    {
      char * p = pat+1;
      VFILE vf(p);
      if (vf.Error) DROP("Unable to access", p);
      if (CFILE::Exist(p)) DROP("File exists", p)
      else
	{
	  CFILE f(p, WRI);
	  if (! f.Error)
	    {
	      byte far * buf = farnew(byte, K(48));
	      if (buf == NULL) DROP("No core", NULL);
	      while (TRUE)
		{
		  word n = vf.Read(buf, K(48));
		  if (n == 0) break;
		  f.Write(buf, n);
		}
	      farfree(buf);
	    }
	}
    }
  else
    {
      VFILE vf(pat, WRI);
    }
  printf("\nFinished.\n");
}
