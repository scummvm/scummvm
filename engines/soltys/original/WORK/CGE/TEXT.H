#ifndef		__TEXT__
#define		__TEXT__

#include	"talk.h"
#include	<jbw.h>
#include	<dir.h>




#ifndef		SYSTXT_MAX
  #define	SYSTXT_MAX	1000
#endif

#define		SAY_EXT		".SAY"

#define		NOT_VGA_TEXT	90
#define		BAD_CHIP_TEXT	91
#define		BAD_DOS_TEXT	92
#define		NO_CORE_TEXT	93
#define		BAD_MIPS_TEXT	94
#define		NO_MOUSE_TEXT	95


#define		INF_NAME	101
#define		SAY_NAME	102

#define		INF_REF		301
#define		SAY_REF		302


class TEXT
{
  struct HAN { int Ref; char * Txt; } * Cache;
  int Size;
  char FileName[MAXPATH];
  char * Load (int idx, int ref);
  int Find (int ref);
public:
  TEXT (const char * fname, int size = 128);
  ~TEXT (void);
  void Clear (int from = 1, int upto = 0x7FFF);
  void Preload (int from = 1, int upto = 0x7FFF);
  char * operator[] (int ref);
};



extern		TALK *		Talk;
extern		TEXT		Text;


void		Say		(const char * txt, SPRITE * spr);
void		SayTime		(SPRITE * spr);
void		Inf		(const char * txt);
void		KillText	(void);



#endif