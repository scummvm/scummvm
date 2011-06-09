#ifndef	__STARTUP__
#define	__STARTUP__


#include	<general.h>

#define		GAME_ID		45
#define		CDINI_FNAME	46

#define		NOT_VGA_TEXT	90
#define		BAD_CHIP_TEXT	91
#define		BAD_DOS_TEXT	92
#define		NO_CORE_TEXT	93
#define		BAD_MIPS_TEXT	94
#define		NO_MOUSE_TEXT	95
#define		BAD_ARG_TEXT	96
#define		BADCD_TEXT	97

#define		CFG_EXT		".CFG"

#if defined(DEMO)
  #define	MINI_EMM_SIZE	0x00004000L
  #define	CORE_HIG	400
#else
  #define	MINI_EMM_SIZE	0x00010000L
  #define	CORE_HIG	450
#endif

#define		CORE_MID	(CORE_HIG-20)
#define		CORE_LOW	(CORE_MID-20)


class STARTUP
{
  static Boolean get_parms (void);
public:
  static int Mode;
  static int Core;
  static int SoundOk;
  static word Summa;
  STARTUP (void);
};




extern	EMM	MiniEmm;

const char *UsrPath (const char *nam);


#endif
