#include	"general.h"
#include	"cfile.h"

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<dir.h>


#define		VER		((1<<8)+10)



//--------------------------------------------------------------------------




static	char	Signature[]	= "JBW batch generator for export CGE files";



static	CFILE *	Batch = NULL;




static void Gen (const char * name)
{
  char n[128], line[256];
  printf("%s\n", strupr(strcpy(n, name)));
  strcpy(line, "COPY ");
  strcat(line, n);
  strcat(line, " %1");
  Batch->Write(line);
}






void ScanSprite (const char * fname)
{
  int lcnt = 0;
  char line[LINE_MAX]; int len;
  static char * Comd[] = { "Name", "Type", "Phase", "East",
			   "Left", "Right", "Top", "Bottom",
			   "Seq", "Near", "Take",
			   "Portable", "Transparent",
			   NULL };

  if (CFILE::Exist(MergeExt(line, fname, ".SPR")))	// sprite description exist
    {
      CFILE sprf(line);
      if (sprf.Error)
	{
	  printf("Unable to read %s\n", line);
	  exit(1);
	}
      else Gen(line);

      while ((len = sprf.Read(line)) != 0)
	{
	  int i;
	  ++ lcnt;
	  if (len)
	    if (line[len-1] == '\n' || line[len-1] == '\32')
	      line[--len] = '\0';
	  if (len == 0 || line[0] == '.') continue;

	  if ((i = TakeEnum(Comd, strtok(line, " =\t"))) < 0)
	    {
	      printf("Unrecognized keyword in line %d file %s\n", lcnt, fname);
	      exit(1);
	    }

	  switch (i)
	    {
	      case  2 : // Phase
			Gen(ForceExt(line, strtok(NULL, " \t,;/"), ".VBM"));
			break;
	    }
	}
    }
  else	// no sprite description: mono-shaped sprite with only .VBM file
    {
      Gen(ForceExt(line, fname, ".VBM"));
    }
}





static void ScanScript (const char *fname)
{
  char line[LINE_MAX];
  CFILE script(fname);

  int lcnt = 0;
  Boolean ok = TRUE;

  if (script.Error) return;
  else Gen(fname);

  while (script.Read(line) != 0)
    {
      char *n;
      int len = strlen(line);
      ++ lcnt;

      if (len)
	if (line[len-1] == '\n' || line[len-1] == '\32')
	  line[--len] = '\0';
      if (len == 0 || line[0] == '.') continue;

      ok = FALSE;	// not OK if break
      // sprite ident number
      if (strtok(line, " \t\n") == NULL) break;
      // sprite file name
      if ((n = strtok(NULL, " ,;/\t\n")) == NULL) break;
      ScanSprite(n);
      ok = TRUE;	// no break: OK
    }
  if (! ok)
    {
      printf("Error in main script line %d\n", lcnt);
      exit(1);
    }
}






#pragma argsused
void main (int argc, char **argv)
{
  char nam[MAXPATH];
  printf("\n%s   version %d\.%d\n", Signature, VER >> 8, VER & 0xFF);
  printf("Copyright (c) 1994 Janusz B. Wi$niewski\n");

  if (argc < 2)
    {
      fnsplit(argv[0], NULL, NULL, nam, NULL);
      printf("Syntax is:    %s ini_file\n", strupr(nam));
      exit(1);
    }
  else printf("\n");

  CFILE	bat("EXPORT.BAT", WRI);

  if (bat.Error)
    {
      printf("Unable to create batch file\n");
      exit(1);
    }
  else Batch = &bat;

  bat.Write("@ECHO OFF");
  bat.Write("IF NOT [%1]==[] GOTO RUN");
  bat.Write("ECHO No destination given\a!");
  bat.Write("GOTO XIT");
  bat.Write(":RUN");
  bat.Write("ECHO Copying files...");
  Gen(ForceExt(nam, argv[1], ".EXE"));
  Gen(ForceExt(nam, argv[1], ".CFT"));
  Gen(ForceExt(nam, argv[1], ".HXY"));
  Gen(ForceExt(nam, argv[1], ".TAB"));
  Gen(ForceExt(nam, argv[1], ".SAY"));
  Gen("WELCOME.VBM");
  ScanSprite("MINI.SPR");
  ScanSprite("00SHADOW.SPR");
  ScanScript(ForceExt(nam, argv[1], ".IN0"));
  ScanScript(ForceExt(nam, argv[1], ".INI"));
  bat.Write(":XIT ");
  bat.Write("\32", 1);
  printf("\n<End of job>\n");
}
