#include	<cfile.h>
#include	<string.h>
#include	<stdio.h>




/*
int main (int argc, char *argv[])
{
  if (argc < 2) printf("Syntax is:    %s file\n", strupr((char *) ProgName()));
  else
    {
      CFILE source(argv[1], DFILE::REA, (argv[1][0] & 0x20) ? NULL : RCrypt);
      if (OK(source))
	{
	  CFILE target((argc > 2) ? argv[2] : "con", DFILE::WRI, (argv[2][0] & 0x20) ? NULL : RCrypt);
	  if (OK(target))
	    {
	      char line[LINE_MAX+1];
	      while(source.Read(line)) target.Write(line);
	    }
	  else printf("I/O error\n");
	}
      else printf("Bad source file\n");
    }
  return 0;
}
*/



int main (int argc, char *argv[])
{
  if (argc < 2) printf("Syntax is:    %s file\n", strupr((char *) ProgName()));
  else
    {
      CFILE source(argv[1], REA, (argv[1][0] & 0x20) ? NULL : RCrypt);
      if (source.Error == 0)
	{
	  CFILE target((argc > 2) ? argv[2] : "con", WRI, (argv[2][0] & 0x20) ? NULL : RCrypt);
	  if (target.Error == 0)
	    {
	      int x;
	      while ((x = source.Read()) != -1) target.Write((char far *)&x, 1);
	    }
	  else printf("I/O error\n");
	}
      else printf("Bad source file\n");
    }
  return 0;
}
