#include	"startup.h"
#include	"text.h"
#include	"sound.h"
#include	"ident.h"
#include	<cfile.h>
#include	<snddrv.h>
#include	<stdio.h>
#include	<process.h>
#include	<dos.h>
#include	<alloc.h>
#include	<string.h>

#ifdef	DEBUG
  #include	<stdlib.h>
#endif

extern	char	Copr[];

#define	id	(*(IDENT*)Copr)


	EMM	MiniEmm		= MINI_EMM_SIZE;

static	STARTUP	StartUp;


	int	STARTUP::Mode = 0;
	int	STARTUP::Core;
	int	STARTUP::SoundOk = 0;
	word	STARTUP::Summa;





Boolean STARTUP::get_parms (void)
{
  int i = _argc;
  while (i > 1)
    {
      static char *PrmTab[] = { "NEW", "MK0SVG", "QUIET", "SB", "GUS", "MIDI",
				"P", "D", "I", "M" };
      int n = TakeEnum(PrmTab, strtok(_argv[--i], " =:("));
      word p = xtow(strtok(NULL, " h,)"));
      switch (n)
	{
	  case 0 : if (Mode != 2) Mode = 1; break;
	  case 1 : Mode = 2; break;
	  case 2 : SNDDrvInfo.DDEV = DEV_QUIET; break;
	  case 3 : SNDDrvInfo.DDEV = DEV_SB; break;
	  case 4 : SNDDrvInfo.DDEV = DEV_GUS; break;
	  case 5 : SNDDrvInfo.MDEV = DEV_GM; break;
	  case 6 : SNDDrvInfo.DBASE = p; break;
	  case 7 : SNDDrvInfo.DDMA = p; break;
	  case 8 : SNDDrvInfo.DIRQ = p; break;
	  case 9 : SNDDrvInfo.MBASE = p;
		   SNDDrvInfo.MDEV = DEV_GM; break;
	  default: return FALSE;
	}
      if (n >= 2) SoundOk = 2;
    }
  #ifdef DEMO
      // protection disabled
      Summa =  0;
  #else
    #ifdef EVA
      {
	union { dosdate_t d; dword n; } today;
	_dos_getdate(&today.d);
	id.disk += (id.disk < today.n);
      }
    #endif
      // disk signature checksum
      Summa = ChkSum(Copr, sizeof(IDENT));
  #endif
  if (SNDDrvInfo.MDEV != DEV_GM) SNDDrvInfo.MDEV = SNDDrvInfo.DDEV;
  return TRUE;
}




STARTUP::STARTUP (void)
{
  const char * fn = ProgName(CFG_EXT);
  dword m = farcoreleft() >> 10;
  if (m < 0x7FFF) Core = (int) m; else Core = 0x7FFF;

  if (! IsVga())
    {
      fputs(Text[NOT_VGA_TEXT], stderr);
      fputc('\n', stderr);
      _exit(1);
    }
  if (Cpu() < _80286)
    {
      fputs(Text[BAD_CHIP_TEXT], stderr);
      fputc('\n', stderr);
      _exit(1);
    }
  if (100 * _osmajor + _osminor < 330)
    {
      fputs(Text[BAD_DOS_TEXT], stderr);
      fputc('\n', stderr);
      _exit(1);
    }

  #ifndef DEBUG
  if (Core < CORE_LOW)
    {
      fputs(Text[NO_CORE_TEXT], stderr);
      fputc('\n', stderr);
      _exit(1);
    }
  if (Core < CORE_HIG)
    {
      SNDDrvInfo.MDEV = DEV_QUIET;
      Music = FALSE;
    }
  #endif
  if (! get_parms())
    {
      fputs(Text[BAD_ARG_TEXT], stderr);
      fputc('\n', stderr);
      #ifdef DEBUG
	char t[] = "000K FREE";
	wtom(Core, t, 10, 3);
	fputs(t, stderr);
	fputc('\n', stderr);
      #endif
      _exit(1);
    }
  //--- load sound configuration
  if (! SoundOk && CFILE::Exist(fn))
    {
      CFILE cfg(fn, REA);
      if (! cfg.Error)
	{
	  cfg.Read(&SNDDrvInfo, sizeof(SNDDrvInfo)-sizeof(SNDDrvInfo.VOL2));
	  if (! cfg.Error) SoundOk = 1;
	}
    }
}

