#include	<general.h>
#include	<cfile.h>
#include	<alloc.h>
#include	<conio.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<dos.h>
#include	<dir.h>
#include	<fcntl.h>
#include	<io.h>
#include	<ctype.h>


#define		VER		0x10
#define		LIM		100


//-------------------------------------------------------------------------

	char		Signature[] = "File Splitter by JBW";

//--------------------------------------------------------------------------





static Boolean CheckLim (word lim)
{
  if (lim >= LIM)
    {
      printf("Error: unable to create more then %u pieces!", LIM-1);
      return FALSE;
    }
  return TRUE;
}




static Boolean CopyFile (CFILE * source, char * name, long size)
{
  if (CFILE::Exist(name))
    {
      printf("Warning: %s exists. Overwrite it? ", name);
      if (toupper(getch()) == 'Y') printf("Yes\n");
      else
	{
	  printf("No\n");
	  return FALSE;
	}
    }
  CFILE target(name, WRI);
  if (target.Error)
    {
      printf("Unable to write: %s\a\n", name);
      return FALSE;
    }
  else
    {
      byte far * buff = farnew(byte, 0x8000);
      if (buff == NULL)
	{
	  printf("Unable to allocate memory\n");
	  return FALSE;
	}
      printf("Writing %s ...\n", name);
      while (size)
	{
	  long n = min(size, 0x8000L);
	  source->Read(buff, (word) n);
	  if (source->Error)
	    {
	      printf("Unable to read file\n");
	      farfree(buff);
	      return FALSE;
	    }
	  target.Write(buff, (word) n);
	  if (target.Error)
	    {
	      printf("Unable to write file\n");
	      farfree(buff);
	      return FALSE;
	    }
	  size -= n;
	}
      farfree(buff);
    }
  return TRUE;
}




int main (int argc, char **argv)
{
  static char Ext[] = ".#00";
  dword size[LIM];
  word n, lim = 0, retcode = 1;

  char drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE], ext[MAXEXT];

  printf("%s version %d.%d\n", Signature, VER / 16, VER % 16);
  if (argc < 2)
    {
      printf("Syntax is:  %s file [size|*count ...]\n", ProgName(""));
    }
  else
    {
      char IName[MAXPATH], OName[MAXPATH];
      fnsplit(strupr(strcpy(IName, argv[1])), drv, dir, nam, ext);
      CFILE IFile(IName);

      if (IFile.Error) printf("Unable to read: %s\a\n", IName);
      else
	{
	  dword siz0 = size[0] = IFile.Size();
	  lim = 1;
	  if (argc < 3)
	    {
	      size[lim ++] = siz0/2 + (siz0 & 1);
	      siz0 -= size[1];
	    }
	  else
	    {
	      int i;
	      for (i = 1; siz0 && i < argc-1; i ++)
		{
		  char * p = argv[i+1];
		  if (*p == '*') // count of pieces
		    {
		      word cnt = atoi(++p);
		      dword siz = siz0/cnt + (siz0%cnt != 0);
		      word k;
		      for (k = 0; siz0 && k < cnt; k ++)
			{
			  if (! CheckLim(lim)) goto xit;
			  if (siz > siz0) siz = siz0;
			  size[lim ++] = siz;
			  siz0 -= siz;
			}
		    }
		  else // single piece
		    {
		      dword siz = atol(p);
		      if (! CheckLim(lim)) goto xit;
		      if (siz > siz0) siz = siz0;
		      size[lim ++] = siz;
		      siz0 -= siz;
		    }
		}
	    }
	  if (siz0)
	    {
	      if (! CheckLim(lim)) goto xit;
	      size[lim ++] = siz0;
	      siz0 = 0;
	    }
	  for (n = 1; n < lim; n ++)
	    {
	      wtom(n, Ext+2, 10, 2);
	      fnmerge(OName, NULL, NULL, nam, Ext);
	      if (! CopyFile(&IFile, OName, size[n])) break;
	    }
	  if (n == lim)
	    {
	      retcode = 0;
	      printf("Well done.\n");
	    }
	}
      xit:
    }
  return retcode;
}
