/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHAMBER_RENDERER_H
#define CHAMBER_RENDERER_H

#include "chamber/common.h"

namespace Chamber {

class Renderer {
public:
	virtual ~Renderer() {}

	virtual void switchToGraphicsMode() = 0;
	virtual void colorSelect(byte csel) = 0;
	virtual void selectCursor(uint16 num) = 0;
	virtual void blitToScreen(int16 x, int16 y, int16 w, int16 h) = 0;
	virtual void blitToScreen(int16 ofs, int16 w, int16 h) = 0;

	virtual uint16 calcXY(uint16 x, uint16 y) = 0;
	virtual uint16 calcXY_p(uint16 x, uint16 y) = 0;

	virtual void backBufferToRealFull() = 0;
	virtual void realBufferToBackFull() = 0;
	virtual void swapRealBackBuffer() = 0;
	virtual void copyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;
	virtual void swapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;

	virtual byte *backupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) = 0;
	virtual void restoreImage(byte *buffer, byte *target) = 0;
	virtual void refreshImageData(byte *buffer) = 0;
	virtual void restoreBackupImage(byte *target) = 0;

	virtual void blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void blitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void fillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;

	virtual void blitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void blitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void blitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) = 0;
	virtual void blitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void blitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) = 0;
	virtual void drawSprite(byte *sprite, byte *screen, uint16 ofs) = 0;
	virtual void drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs) = 0;
	virtual byte *loadSprit(byte index) = 0;
	virtual byte *loadPersSprit(byte index) = 0;
	virtual void backupAndShowSprite(byte index, byte x, byte y) = 0;
	virtual byte *loadPortraitWithFrame(byte index) = 0;

	virtual void drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) = 0;
	virtual void drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) = 0;
	virtual uint16 drawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) = 0;
	virtual void printChar(byte c, byte *target) = 0;

	virtual void animLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void animLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void animLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) = 0;
	virtual void animLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y) = 0;

	virtual void hideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;
	virtual void hideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;
	virtual void hideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;
	virtual void hideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;
	virtual void hideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) = 0;

	virtual void traceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target) = 0;

	virtual void zoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) = 0;
	virtual void animZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) = 0;
	virtual void zoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target) = 0;
};

class CGARenderer : public Renderer {
public:
	void switchToGraphicsMode() override;
	void colorSelect(byte csel) override;
	void selectCursor(uint16 num) override;
	void blitToScreen(int16 x, int16 y, int16 w, int16 h) override;
	void blitToScreen(int16 ofs, int16 w, int16 h) override;
	uint16 calcXY(uint16 x, uint16 y) override;
	uint16 calcXY_p(uint16 x, uint16 y) override;
	void backBufferToRealFull() override;
	void realBufferToBackFull() override;
	void swapRealBackBuffer() override;
	void copyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void swapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	byte *backupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) override;
	void restoreImage(byte *buffer, byte *target) override;
	void refreshImageData(byte *buffer) override;
	void restoreBackupImage(byte *target) override;
	void blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void fillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) override;
	void blitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) override;
	void drawSprite(byte *sprite, byte *screen, uint16 ofs) override;
	void drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs) override;
	byte *loadSprit(byte index) override;
	byte *loadPersSprit(byte index) override;
	void backupAndShowSprite(byte index, byte x, byte y) override;
	byte *loadPortraitWithFrame(byte index) override;
	void drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
	void drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
	uint16 drawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) override;
	void printChar(byte c, byte *target) override;
	void animLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y) override;
	void hideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void traceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target) override;
	void zoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) override;
	void animZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) override;
	void zoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target) override;
};

class EGARenderer : public Renderer {
public:
	void switchToGraphicsMode() override;
	void colorSelect(byte csel) override;
	void selectCursor(uint16 num) override;
	void blitToScreen(int16 x, int16 y, int16 w, int16 h) override;
	void blitToScreen(int16 ofs, int16 w, int16 h) override;
	uint16 calcXY(uint16 x, uint16 y) override;
	uint16 calcXY_p(uint16 x, uint16 y) override;
	void backBufferToRealFull() override;
	void realBufferToBackFull() override;
	void swapRealBackBuffer() override;
	void copyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void swapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	byte *backupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) override;
	void restoreImage(byte *buffer, byte *target) override;
	void refreshImageData(byte *buffer) override;
	void restoreBackupImage(byte *target) override;
	void blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void fillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) override;
	void blitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void blitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) override;
	void drawSprite(byte *sprite, byte *screen, uint16 ofs) override;
	void drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs) override;
	byte *loadSprit(byte index) override;
	byte *loadPersSprit(byte index) override;
	void backupAndShowSprite(byte index, byte x, byte y) override;
	byte *loadPortraitWithFrame(byte index) override;
	void drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
	void drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) override;
	uint16 drawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) override;
	void printChar(byte c, byte *target) override;
	void animLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) override;
	void animLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y) override;
	void hideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void hideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) override;
	void traceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target) override;
	void zoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) override;
	void animZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) override;
	void zoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target) override;
};

} // End of namespace Chamber

#endif // CHAMBER_RENDERER_H
