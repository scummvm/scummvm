
#ifndef __GPFONT_H__
#define	__GPFONT_H__

#include "gpdef.h" 
#include "gpgraphic.h"

typedef struct tagBGFONTINFO{
	int kor_w;
	int kor_h;
	int eng_w;
	int eng_h;
	int chargap;	
	int linegap;
}BGFONTINFO;

void GpFontInit (BGFONTINFO * ptr);

void GpFontResSet (unsigned char * p_kor, unsigned char * p_eng);
unsigned char * GpKorFontResGet(void);
unsigned char * GpEngFontResGet(void);

void GpSysFontGet (BGFONTINFO *fInfo);
int GpTextWidthGet (const char * lpsz);
int GpTextHeightGet (const char * lpsz);
int GpTextLenGet (const char * str);

void GpTextOut (GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y,
              char * source, unsigned char color);
void GpCharOut (GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, 
              char * source, unsigned char color);
void GpTextNOut (GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, 
		      char * source, int nStart, int nString, unsigned char color);
		      
#define GPC_GT_BOTTOM		0x2
#define GPC_GT_HCENTER		0x8
#define GPC_GT_LEFT		0x10
#define GPC_GT_RIGHT		0x20
#define GPC_GT_TOP		0x40
#define GPC_GT_WORDBREAK	0x80
#define GPC_GT_VCENTER		0x100
		      
void GpTextDraw (GPDRAWSURFACE * ptgpds, GPRECT * cRect, unsigned int uFormat, 
		      char * source, int nStart, int nCount, unsigned char color);

typedef struct tagEXT_FONT{
	int fx_flag;
	unsigned char * lpsz;
	unsigned char * pBmFont;
	int ex_font_w;
	int ex_font_h;
	int ex_chargap;
	int ex_linegap;
	int color1;
	int color2;
} EXT_FONT;

int GpCustTextOut (GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, EXT_FONT * y_font);

void GpHAutomatonInit (void);
int GpHAutomatonInput (const char * p_input, char ** p_working);
int GpHAutomatonDelete (char ** p_working);
char * GpHAutomatonBuffered (void);

#endif


