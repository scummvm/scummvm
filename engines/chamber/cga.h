#ifndef _CGA_H_
#define _CGA_H_

#include <dos.h>

#define CGA_WIDTH 320
#define CGA_HEIGHT 200
#define CGA_BASE_SEG 0xB800
#define CGA_ODD_LINES_OFS 0x2000
#define CGA_BITS_PER_PIXEL 2
#define CGA_PIXELS_PER_BYTE (8 / CGA_BITS_PER_PIXEL)
#define CGA_BYTES_PER_LINE (CGA_WIDTH / CGA_PIXELS_PER_BYTE)

#define CGA_SCREENBUFFER ((unsigned char*)MK_FP(CGA_BASE_SEG, 0))

#define CGA_FONT_HEIGHT 6

#define CGA_NEXT_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) + (((offs) & CGA_ODD_LINES_OFS) ? 0 : CGA_BYTES_PER_LINE))
#define CGA_PREV_LINE(offs) ((CGA_ODD_LINES_OFS ^ (offs)) - (((offs) & CGA_ODD_LINES_OFS) ? CGA_BYTES_PER_LINE : 0))

#define frontbuffer CGA_SCREENBUFFER
extern unsigned char backbuffer[0x4000];

extern unsigned char sprit_load_buffer[1290];

extern unsigned char cga_pixel_flip[256];

extern unsigned char char_draw_coords_x;
extern unsigned char char_draw_coords_y;
extern unsigned char *char_xlat_table;
extern unsigned char string_ended;
extern unsigned char char_draw_max_width;
extern unsigned char char_draw_max_height;

void SwitchToGraphicsMode(void);
void SwitchToTextMode(void);

void WaitVBlank(void);

void CGA_ColorSelect(unsigned char csel);
void CGA_BackBufferToRealFull(void);
void CGA_RealBufferToBackFull(void);
void CGA_SwapRealBackBuffer(void);

void CGA_SwapScreenRect(unsigned char *pixels, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);

unsigned int CGA_CalcXY(unsigned int x, unsigned int y);
unsigned int CGA_CalcXY_p(unsigned int x, unsigned int y);

void CGA_CopyScreenBlock(unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);

unsigned char * CGA_BackupImage(unsigned char *source, unsigned int ofs, unsigned int w, unsigned int h, unsigned char *buffer);
unsigned char * CGA_BackupImageReal(unsigned int ofs, unsigned int w, unsigned int h);

void CGA_RestoreImage(unsigned char *buffer, unsigned char *target);
void CGA_RefreshImageData(unsigned char *buffer);
void CGA_RestoreBackupImage(unsigned char *target);

void CGA_Blit(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_BlitAndWait(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_FillAndWait(unsigned char pixel, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);

void CGA_DrawVLine(unsigned int x, unsigned int y, unsigned int l, unsigned char color, unsigned char *target);
void CGA_DrawHLine(unsigned int x, unsigned int y, unsigned int l, unsigned char color, unsigned char *target);
unsigned int CGA_DrawHLineWithEnds(unsigned int bmask, unsigned int bpix, unsigned char color, unsigned int l, unsigned char *target, unsigned int ofs);

void CGA_PrintChar(unsigned char c, unsigned char *target);

void CGA_BlitScratchBackSprite(unsigned int sprofs, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_BlitFromBackBuffer(unsigned char w, unsigned char h, unsigned char *screen, unsigned int ofs);

void CGA_BlitSprite(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_BlitSpriteFlip(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);

void CGA_BlitSpriteBak(unsigned char *pixels, signed int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs, unsigned char *backup, unsigned char mask);

void DrawSprite(unsigned char *sprite, unsigned char *screen, unsigned int ofs);
void DrawSpriteFlip(unsigned char *sprite, unsigned char *screen, unsigned int ofs);

void DrawSpriteN(unsigned char index, unsigned int x, unsigned int y, unsigned char *target);
void DrawSpriteNFlip(unsigned char index, unsigned int x, unsigned int y, unsigned char *target);

void BackupAndShowSprite(unsigned char index, unsigned char x, unsigned char y);

unsigned char * LoadSprite(unsigned char index, unsigned char *bank, unsigned char *buffer, unsigned char header_only);

unsigned char *LoadSprit(unsigned char index);
unsigned char *LoadPersSprit(unsigned char index);

void CGA_AnimLiftToUp(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int x, unsigned int y);
void CGA_AnimLiftToDown(unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_AnimLiftToLeft(unsigned int n, unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);
void CGA_AnimLiftToRight(unsigned int n, unsigned char *pixels, unsigned int pw, unsigned int w, unsigned int h, unsigned char *screen, unsigned int ofs);

void CGA_HideScreenBlockLiftToUp(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToDown(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToLeft(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);
void CGA_HideScreenBlockLiftToRight(unsigned int n, unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);

void CGA_HideShatterFall(unsigned char *screen, unsigned char *source, unsigned int w, unsigned int h, unsigned char *target, unsigned int ofs);

void CGA_TraceLine(unsigned int sx, unsigned int ex, unsigned int sy, unsigned int ey, unsigned char *source, unsigned char *target);

#endif
