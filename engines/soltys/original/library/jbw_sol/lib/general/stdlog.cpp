#include	<general.h>
#include	<string.h>
#include	<fcntl.h>
#include	<sys\stat.h>
#include	<dos.h>
#include	<io.h>


#define		LOG_EXT		".LOG"


extern	Boolean	LogMode		= TRUE;
static	int	File;



static Boolean wr (const char * s)
{
  word n;
  return (*s) ? (_dos_write(File, s, strlen(s), &n) == 0) : TRUE;
}


void StdLog (const char *msg, const char *nam)
{
  if (LogMode)
    {
      const char * fn = ProgName(LOG_EXT);
      int i;

      i = (access(fn, 0) == 0) ? _dos_open(fn , O_RDWR | O_DENYALL, &File)
			       : _dos_creat(fn , FA_ARCH, &File);
      if (i == 0)
	{
	  char * dts = DateTimeString();
	  if (! msg) msg = "*NULL*";
	  lseek(File, 0, SEEK_END);
	  wr(dts);
	  wr("  ");
	  wr(msg);
	  if (nam)
	    {
	      wr("  ");
	      wr(nam);
	    }
	  wr("\r\n");
	}
      _dos_close(File);
    }
}
