/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CINE_GFX_H
#define CINE_GFX_H

#include "common/noncopyable.h"
#include "cine/object.h"

namespace Cine {

/*! \brief Background with palette
 */
struct palBg {
	byte *bg; ///< Background data
	byte *hiPal; ///< 256 color palette
	uint16 *lowPal; ///< 16 color palette
	char name[15]; ///< Background filename
};

/*! \brief Future Wars renderer
 *
 * Screen backbuffer is not cleared between frames, you can draw menus etc.
 * without calling drawFrame() all the time
 */
class FWRenderer : public Common::NonCopyable {
private:
	byte *_background; ///< Current background
	char _bgName[13]; ///< Background filename
	uint16 *_palette; ///< 16 color backup palette

	Common::String _cmd; ///< Player command string

protected:
	static const int _screenSize = 320 * 200; ///< Screen size
	static const int _screenWidth = 320; ///< Screen width
	static const int _screenHeight = 200; ///< Screen height
	static const int _lowPalSize = 16; ///< 16 color palette size

	byte *_backBuffer; ///< Screen backbuffer
	uint16 *_activeLowPal; ///< Active 16 color palette
	int _changePal; ///< Load active palette to video backend on next frame

	void fillSprite(const objectStruct &obj, uint8 color = 0);
	void drawMaskedSprite(const objectStruct &obj, const byte *mask);
	virtual void drawSprite(const objectStruct &obj);

	void drawCommand();
	void drawMessage(const char *str, int x, int y, int width, byte color);
	void drawPlainBox(int x, int y, int width, int height, byte color);
	void drawBorder(int x, int y, int width, int height, byte color);
	void drawDoubleBorder(int x, int y, int width, int height, byte color);
	virtual int drawChar(char character, int x, int y);
	void drawLine(int x, int y, int width, int height, byte color);
	void remaskSprite(byte *mask, Common::List<overlay>::iterator it);
	virtual void drawBackground();

	virtual void renderOverlay(const Common::List<overlay>::iterator &it);
	void drawOverlays();

public:
	uint16 _messageBg; ///< Message box background color
	uint16 _cmdY; ///< Player command string position on screen

	FWRenderer();
	virtual ~FWRenderer();

	/*! \brief Test if renderer is ready to draw */
	virtual bool ready() { return _background != NULL; }

	virtual void clear();

	void drawFrame();
	void blit();
	void setCommand(const char *cmd);

	virtual void incrustMask(const objectStruct &obj, uint8 color = 0);
	virtual void incrustSprite(const objectStruct &obj);

	virtual void loadBg16(const byte *bg, const char *name);
	virtual void loadBg16(const byte *bg, const char *name, unsigned int idx);
	virtual void loadCt16(const byte *ct, const char *name);
	virtual void loadBg256(const byte *bg, const char *name);
	virtual void loadBg256(const byte *bg, const char *name, unsigned int idx);
	virtual void loadCt256(const byte *ct, const char *name);
	virtual void selectBg(unsigned int idx);
	virtual void selectScrollBg(unsigned int idx);
	virtual void setScroll(unsigned int shift);
	virtual void removeBg(unsigned int idx);
	void saveBg(Common::OutSaveFile &fHandle);

	virtual void refreshPalette();
	virtual void reloadPalette();
	void restorePalette(Common::SeekableReadStream &fHandle);
	void savePalette(Common::OutSaveFile &fHandle);
	virtual void rotatePalette(int a, int b, int c);
	virtual void transformPalette(int first, int last, int r, int g, int b);

	void drawMenu(const CommandeType *items, unsigned int height, int x, int y, int width, int selected);
	void drawInputBox(const char *info, const char *input, int cursor, int x, int y, int width);

	virtual void fadeToBlack();
};

/*! \brief Operation Stealth renderer
 */
class OSRenderer : public FWRenderer {
private:
	palBg _bgTable[9]; ///< Table of backgrounds loaded into renderer
	byte *_activeHiPal; ///< Active 256 color palette
	unsigned int _currentBg; ///< Current background
	unsigned int _scrollBg; ///< Current scroll background
	unsigned int _bgShift; ///< Background shift

protected:
	static const int _hiPalSize = 256 * 3; ///< 256 color palette size

	void drawSprite(const objectStruct &obj);
	int drawChar(char character, int x, int y);
	void drawBackground();
	void renderOverlay(const Common::List<overlay>::iterator &it);

public:
	OSRenderer();
	~OSRenderer();

	/*! \brief Test if renderer is ready to draw */
	bool ready() { return _bgTable[_currentBg].bg != NULL; }

	void clear();

	void incrustMask(const objectStruct &obj, uint8 color = 0);
	void incrustSprite(const objectStruct &obj);

	void loadBg16(const byte *bg, const char *name);
	void loadBg16(const byte *bg, const char *name, unsigned int idx);
	void loadCt16(const byte *ct, const char *name);
	void loadBg256(const byte *bg, const char *name);
	void loadBg256(const byte *bg, const char *name, unsigned int idx);
	void loadCt256(const byte *ct, const char *name);
	void selectBg(unsigned int idx);
	void selectScrollBg(unsigned int idx);
	void setScroll(unsigned int shift);
	void removeBg(unsigned int idx);

	void refreshPalette();
	void reloadPalette();
	void rotatePalette(int a, int b, int c);
	void transformPalette(int first, int last, int r, int g, int b);

	void fadeToBlack();
};

void gfxDrawSprite(byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy);

extern byte *page3Raw;
extern FWRenderer *renderer;

void setMouseCursor(int cursor);
void gfxCopyPage(byte *source, byte *dest);

void transformPaletteRange(byte startColor, byte numColor, int8 r, int8 g, int8 b);
void gfxFlipPage(void);

void gfxDrawMaskedSprite(const byte *ptr, const byte *msk, uint16 width, uint16 height, byte *page, int16 x, int16 y);
void gfxFillSprite(const byte *src4, uint16 sw, uint16 sh, byte *dst4, int16 sx, int16 sy, uint8 fillColor = 0);

void gfxUpdateSpriteMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *maskPtr, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight);

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void gfxDrawPlainBox(int16 x1, int16 y1, int16 x2, int16 y2, byte color);

void gfxResetPage(byte *pagePtr);

int16 gfxGetBit(int16 x, int16 y, const byte *ptr, int16 width);
byte gfxGetColor(int16 x, int16 y, const byte *ptr, int16 width);

void gfxResetRawPage(byte *pageRaw);
void gfxConvertSpriteToRaw(byte *dst, const byte *src, uint16 w, uint16 h);
void gfxCopyRawPage(byte *source, byte * dest);
void gfxFlipRawPage(byte *frontBuffer);
void drawSpriteRaw(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y);
void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page);
void drawSpriteRaw2(const byte *spritePtr, byte transColor, int16 width, int16 height, byte *page, int16 x, int16 y);
void maskBgOverlay(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y);

void fadeFromBlack(void);
void fadeToBlack(void);

// wtf?!
//void gfxDrawMaskedSprite(byte *param1, byte *param2, byte *param3, byte *param4, int16 param5);
void gfxWaitVBL(void);
void gfxRedrawMouseCursor(void);

void blitScreen(byte *frontBuffer, byte *backbuffer);
void blitRawScreen(byte *frontBuffer);
void flip(void);

} // End of namespace Cine

#endif
