#include	<general.h>
#include	<cfile.h>
#include	<wav.h>
#include	<snddrv.h>

#include	<string.h>
#include	<stdlib.h>
#include	<alloc.h>
#include	<stdio.h>
#include	<conio.h>
#include	<dos.h>
#include	<dir.h>
#include	<io.h>
#include	<fcntl.h>

static	SMPINFO		smpinf;

//------------------------------------------------------------------------


static void test (const char * name)
{
  DATACK * data = LoadWave(&CFILE(name));
  if (data)
    {
      SNDDrvInfo.DDEV = DEV_SB;
      SNDInit();

      smpinf.saddr = data->Addr();
      smpinf.slen = (word)data->Size();
      smpinf.span = 8;
      smpinf.sflag = 1;
      SNDDigiStart(&smpinf);
      while (smpinf.sflag)
	{
	  if (smpinf.sflag == -1) if (kbhit()) break;
	}
      SNDDigiStop(&smpinf);
      SNDDone();
    }
}




#pragma argsused
int main (int argc, char ** argv)
{
  test(argv[1]);
  return 0;
}
