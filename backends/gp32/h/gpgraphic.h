
#ifndef 	__GRAPHIC_H__
#define		__GRAPHIC_H__

#define GPC_DRAW_OK				0
#define	GPC_DRAW_ERR_FULL		1
#define GPC_DRAW_ERR_PARAM		2
#define GPC_DRAW_ERR_UNKNOWN	3
#define GPC_DRAW_ERR_OUTOFMEM	4

#define GPC_LCD_ON_BIT			0x80
#define GPC_LCD_VACTIVE			0x40
#define GPC_LCD_HACTIVE			0x20

typedef struct tagGPLCDINFO{
	union{
		int U32_lcd;
		struct{
			char clk_value;
			char lcd_buf_count;
			char bpp;
			char b_lcd_on;
		}U8_lcd;
	}lcd_global;
	unsigned int buffer_size;
	unsigned int * buf_addr[4];
	unsigned int * r_palette;
	unsigned int * m_palette;
}GPLCDINFO;

typedef struct tagGPDRAWTAG{
	int restoreflag;
	short clip_x;
	short clip_y;
	short clip_w;
	short clip_h;
}GPDRAWTAG;

typedef struct tagGPDRAWSURFACE{
	unsigned char * ptbuffer;
	int bpp;	/*reserved*/
	int buf_w;
	int buf_h;
	int ox;
	int oy;
	unsigned char * o_buffer;
} GPDRAWSURFACE;

#define GPC_DFLAG_1BPP	1
#define GPC_DFLAG_2BPP	2
#define GPC_DFLAG_4BPP	4
#define GPC_DFLAG_8BPP	8
#define GPC_DFLAG_16BPP	16

int GpGraphicModeSet(int gd_bpp, int * gp_pal);
void GpLcdInfoGet(GPLCDINFO * p_info);
void GpLcdEnable(void);
void GpLcdDisable(void);
int GpLcdStatusGet(void);
int GpLcdSurfaceGet(GPDRAWSURFACE * ptgpds, int idx);
int GpMemSurfaceGet(GPDRAWSURFACE * ptgpds);
extern void (*GpSurfaceSet)(GPDRAWSURFACE * ptgpds);
extern void (*GpSurfaceFlip)(GPDRAWSURFACE * ptgpds);
unsigned char * GpLcdLock(void);
void GpLcdUnlock(void);

#define GPC_PAL_RED_MASK	0xf800
#define GPC_PAL_GREEN_MASK	0x07c0
#define GPC_PAL_BLUE_MASK	0x003e
#define GPC_PALCHAN_RED_BIT		0x4
#define GPC_PALCHAN_GREEN_BIT	0x2
#define GPC_PALCHAN_BLUE_BIT	0x1

typedef unsigned short GP_PALETTEENTRY;
typedef struct tag_GP_LOGPALENTRY{
	unsigned char peRed;
	unsigned char peGreen;
	unsigned char peBlue;
	unsigned char peFlags;
}GP_LOGPALENTRY;

typedef unsigned long * GP_HPALETTE;
GP_HPALETTE GpPaletteCreate (int entry_num, GP_PALETTEENTRY * pal_entry);
GP_HPALETTE GpPaletteCreateEx (int entry_num, GP_LOGPALENTRY * pal_entry);

GP_HPALETTE GpPaletteSelect(GP_HPALETTE h_new);

unsigned int GpPaletteRealize (void);
int GpPaletteDelete (GP_HPALETTE hPal);
#define GPC_PAL_SYNC_REALIZE	0
#define GPC_PAL_ASYNC_REALIZE	1
#define GPC_PAL_NO_REALIZE		2
int GpPaletteEntryChange (int pal_offset, int entry_num, GP_PALETTEENTRY * pal_entry, int flag);
int GpPaletteEntryChangeEx (int pal_offset, int entry_num, GP_LOGPALENTRY * pal_entry, int flag);

GP_HPALETTE GpRegPalGet (void);
GP_HPALETTE GpLogPalGet (void);

int GpLcdFade (int fade_step, GP_HPALETTE old_pal);
void GpLcdNoFade (GP_HPALETTE old_pal);
int GpLcdChanFade (int fade_step, int chan, GP_HPALETTE old_pal);
int GpLcdFadeNormalize (GP_HPALETTE basic_pal);

int GpBitBlt (GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char * src, int sx,int sy,int imgw,int imgh);
int GpTransBlt (GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char *src,int sx,int sy,int imgw,int imgh,unsigned char color);
int GpBitLRBlt (GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char * src,int sx,int sy,int imgw,int imgh);
int GpTransLRBlt (GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char* src,int sx,int sy,int imgw,int imgh,unsigned char color);
int GpBitUDBlt (GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char * src,int sx,int sy,int imgw,int imgh);
int GpTransUDBlt(GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char* src,int sx,int sy,int imgw,int imgh,unsigned char color);
int GpRectFill(GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int dx,int dy,int width,int height,unsigned char color);

void GpPointSet(GPDRAWSURFACE * ptgpds,int x, int y, unsigned char color);
void GpLineDraw(GPDRAWSURFACE * ptgpds,int xS,int yS,int xE,int yE,unsigned char color);
void GpRectDraw(GPDRAWSURFACE * ptgpds,int left,int top,int right,int bottom,unsigned char color);
void GpEllipseDraw(GPDRAWSURFACE * ptgpds,int xS,int yS,int w,int h,unsigned char color);

/*************************************************************************************************

GPGRAPHIC Effect API

	You must import libgpg_ex??.a for using the following:


CURRENTLY, THE FOLLOWING ARE ALLOWED

  GPC_GDFX_COPY
  GPC_GDFX_TRANS
  GPC_GDFX_EASYBLEND
  GPC_GDFX_TRANS | GPC_GDFX_EASYBLEND
  GPC_GDFX_BLEND
  GPC_GDFX_TRANS | GPC_GDFX_BLEND
  GPC_GDFX_INVERT
  GPC_GDFX_TRANS | GPC_GDFX_INVERT
  GPC_GDFX_LRFLIP
  GPC_GDFX_UDFLIP
  GPC_GDFX_TRANS | GPC_GDFX_LRFLIP
  GPC_GDFX_TRANS | GPC_GDFX_UDFLIP
  GPC_GDFX_RAYPLUS
  GPC_GDFX_TRANS | GPC_GDFX_RAYPLUS
  GPC_GDFX_TRANS | GPC_GDFX_MASKING
  GPC_GDFX_MASKING | GPC_GDFX_RAYPLUS
  GPC_GDFX_TRANS | GPC_GDFX_CHRAYPLUS
  GPC_GDFX_TRANS | GPC_GDFX_PART_RAY
  GPC_GDFX_RAYMINUS
  GPC_GDFX_TRANS | GPC_GDFX_RAYMINUS
  GPC_GDFX_MASKING | GPC_GDFX_RAYMINUS
*****************************************************************************************************/

#define GPC_GDFX_NOEFFECT	0x00
#define GPC_GDFX_COPY		0x01
#define GPC_GDFX_TRANS		0x02
#define GPC_GDFX_EASYBLEND	0x04
#define GPC_GDFX_BLEND		0x08
#define GPC_GDFX_EXCHANGE	0x10
#define GPC_GDFX_INVERT		0x20
#define GPC_GDFX_LRFLIP		0x40
#define GPC_GDFX_UDFLIP		0x80
#define GPC_GDFX_RAYPLUS	0x100
#define GPC_GDFX_MASKING	0x200
#define GPC_GDFX_CHRAYPLUS	0x400
#define GPC_GDFX_PART_RAY	0x800
#define GPC_GDFX_RAYMINUS	0x1000
#define GPC_GDFX_SRC_RPLUS	0x2000

typedef struct tagGPGD_FX{
	int fx_flag;		/*effect option*/
	int v_default;		/*default value*/
	int v_tmp;		/*temporary value*/
	int * v_list;		/*value list (not yet used)*/
}GPGD_FX;

int GpFxBlt(GPDRAWTAG *gptag, GPDRAWSURFACE *tggpds, int dx, int dy, int width, int height, GPDRAWSURFACE *srcgpds, GPGD_FX *gpfx);

#endif /*__gpgraphic_h__*/


