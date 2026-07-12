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

#include "common/system.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/rebel/smush_player_ra1.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

inline int ra1OverlayViewOffsetX(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	// Opcode 0x0B target/GOST markers are handled as raw/projected coordinates
	// in their callers. Keep this helper scoped to that legacy path.
	return (rebel1->getEffectiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveX() : 0;
}

inline int ra1OverlayViewOffsetY(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	return (rebel1->getEffectiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveY() : 0;
}

int ra1GameplayWindowOffsetX(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	switch (rebel1->getEffectiveGameOpcode()) {
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
		return rebel1->getPerspectiveX();
	default:
		return 0;
	}
}

int ra1GameplayWindowOffsetY(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	switch (rebel1->getEffectiveGameOpcode()) {
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
		return rebel1->getPerspectiveY();
	default:
		return 0;
	}
}

void drawBankString(const RA1SpriteBank &bank, byte *dst, int pitch, int width, int height,
	int x, int y, const char *text) {
	if (!dst || !text || bank.numSprites <= 0)
		return;

	for (int i = 0; text[i] != '\0'; i++) {
		const byte ch = (byte)text[i];

		if (ch == ' ') {
			x += 6;
			continue;
		}

		// RA1 font renderer indexes printable characters from '!' (0x21), not raw ASCII.
		if (ch < 0x21) {
			x += 4;
			continue;
		}
		const int fontIdx = (int)ch - 0x21;
		if (fontIdx < 0 || fontIdx >= bank.numSprites) {
			x += 4;
			continue;
		}

		const RA1Sprite &glyph = bank.sprites[fontIdx];
		const int gw = glyph.width;
		const int gh = glyph.height;
		const int gx = x + glyph.xoffs;
		const int gy = y + glyph.yoffs;
		const uint64 glyphPixels = (uint64)gw * (uint64)gh;
		if (!glyph.data || gw <= 0 || gh <= 0 || glyphPixels == 0 || glyphPixels > 0x10000) {
			x += 4;
			continue;
		}
		if (!(bank.decodedData && bank.decodedSize > 0)) {
			x += 4;
			continue;
		}
		const byte *bankStart = bank.decodedData;
		const byte *bankEnd = bank.decodedData + bank.decodedSize;
		if (glyph.data < bankStart || glyph.data >= bankEnd || glyph.data + glyphPixels > bankEnd) {
			x += 4;
			continue;
		}

		for (int py = 0; py < gh; py++) {
			const int sy = gy + py;
			if (sy < 0 || sy >= height)
				continue;
			for (int px = 0; px < gw; px++) {
				const int sx = gx + px;
				if (sx < 0 || sx >= width)
					continue;
				const byte pixel = glyph.data[py * gw + px];
				if (pixel != 0)
					dst[sy * pitch + sx] = pixel;
			}
		}

		x += gw > 0 ? gw : 4;
	}
}

const RA1Sprite *lookupBankGlyph(const RA1SpriteBank &bank, char ch) {
	if (bank.numSprites <= 0)
		return nullptr;
	if ((byte)ch < 0x21)
		return nullptr;

	const int fontIdx = (int)(byte)ch - 0x21;
	if (fontIdx < 0 || fontIdx >= bank.numSprites)
		return nullptr;

	const RA1Sprite &glyph = bank.sprites[fontIdx];
	if (!glyph.data || glyph.width <= 0 || glyph.height <= 0)
		return nullptr;

	return &glyph;
}

// which centers the glyph and ignores the NUT x/y offsets. Use the same anchor
// rules here instead of the generic left-anchored text path.
void drawCenteredBankGlyph(const RA1SpriteBank &bank, byte *dst, int pitch, int width, int height,
	int centerX, int centerY, char ch) {
	char glyphStr[2] = { ch, '\0' };
	const RA1Sprite *glyph = lookupBankGlyph(bank, ch);
	if (!glyph) {
		drawBankString(bank, dst, pitch, width, height, centerX, centerY, glyphStr);
		return;
	}

	const int drawX = centerX - glyph->xoffs - (int)glyph->width / 2;
	const int drawY = centerY - glyph->yoffs - (int)glyph->height / 2;
	drawBankString(bank, dst, pitch, width, height, drawX, drawY, glyphStr);
}

int getBankStringWidth(const RA1SpriteBank &bank, const char *text) {
	if (!text || bank.numSprites <= 0)
		return 0;

	int w = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		const byte ch = (byte)text[i];
		if (ch == ' ') {
			w += 6;
			continue;
		}
		if (ch < 0x21) {
			w += 4;
			continue;
		}
		const int fontIdx = (int)ch - 0x21;
		if (fontIdx < 0 || fontIdx >= bank.numSprites) {
			w += 4;
			continue;
		}
		const RA1Sprite &glyph = bank.sprites[fontIdx];
		w += glyph.width > 0 ? glyph.width : 4;
	}
	return w;
}

int getBankSpaceAdvance(const RA1SpriteBank &bank) {
	const int exclWidth = getBankStringWidth(bank, "!");
	if (exclWidth <= 0)
		return 6;
	return MIN(exclWidth, 8);
}

const RA1SpriteBank &selectLayerBank(const RA1SpriteBank &titleBank,
		const RA1SpriteBank &hudBank, const RA1SpriteBank &techBank, int layer) {
	const bool techLayer = (layer >= 2);
	const bool talkLayer = (layer == 1);
	if (techLayer)
		return (techBank.numSprites > 0) ? techBank : hudBank;
	if (talkLayer)
		return hudBank;
	return (titleBank.numSprites > 0) ? titleBank : hudBank;
}

void drawCenteredRebel1String(InsaneRebel1 *rebel1, byte *dst, int pitch, int width, int height,
		int centerX, int y, const char *text, int maxChars = -1) {
	if (!rebel1 || !text)
		return;

	const char *drawText = text;
	char clipped[96];
	if (maxChars >= 0) {
		int len = 0;
		while (text[len] != '\0' && len < maxChars && len < (int)sizeof(clipped) - 1) {
			clipped[len] = text[len];
			len++;
		}
		clipped[len] = '\0';
		drawText = clipped;
	}

	if (drawText[0] == '\0')
		return;

	const int textW = rebel1->getFontBankStringWidth(drawText);
	rebel1->drawFontBankString(dst, pitch, width, height, centerX - textW / 2, y, drawText);
}

int getBankSpaceHeight(const RA1SpriteBank &bank) {
	// With current NUT decoding we approximate that using the '!' glyph (index 0).
	if (bank.numSprites > 0) {
		const RA1Sprite &glyph = bank.sprites[0];
		if (glyph.height > 0)
			return glyph.height;
	}
	return 8;
}

int ra1ShotDirection(int16 x1, int16 y1, int16 x2, int16 y2) {
	int dx = x2 - x1;
	int dy = y1 - y2;
	if (dy < 0) {
		dy = -dy;
		dx = -dx;
	}

	if (dx >= 0) {
		if (dy > dx * 5)
			return 0;
		if (dx * 3 < dy * 2)
			return 1;
		if (dx * 2 < dy * 3)
			return 2;
		if (dx * 2 < dy * 9)
			return 3;
		return 4;
	}

	const int adx = -dx;
	if (dy > adx * 5)
		return 0;
	if (adx * 3 < dy * 2)
		return -1;
	if (adx * 2 < dy * 3)
		return -2;
	if (adx * 2 < dy * 9)
		return -3;
	return -4;
}

int ra1ShotDirectionBucket(int dir) {
	const int absDir = ABS(dir);
	if (absDir <= 1)
		return 0;
	if (absDir == 2)
		return 5;
	return 10;
}

// Return one of nine direction sprites and publish glyph flip flags.
int ra1ProjectileDirectionWithFlags(int16 srcX, int16 srcY, int16 dstX, int16 dstY, uint16 &flags) {
	int dx = srcX - dstX;
	int dy = dstY - srcY;
	flags = 0;

	if (dy < 0) {
		dy = -dy;
		flags |= 0x4000;
	}
	if (dx < 0) {
		dx = -dx;
		flags |= 0x2000;
	}

	if (dx * 10 < dy)
		return 0;
	if (dx * 10 < dy * 3)
		return 1;
	if (dx * 2 < dy)
		return 2;
	if (dx * 5 < dy * 4)
		return 3;
	if (dx * 4 < dy * 5)
		return 4;
	if (dx < dy * 2)
		return 5;
	if (dx * 3 < dy * 10)
		return 6;
	if (dx < dy * 10)
		return 7;
	return 8;
}

struct RA1ShotEmitterPair {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
};

const RA1ShotEmitterPair kRA1ShotEmitters244A[27] = {
	{ 11, -11, -11, 0 }, { 16, -9, -16, -1 }, { 20, -6, -19, -3 }, { 20, -5, -21, -4 }, { -20, -6, 20, -5 },
	{ -18, -9, 16, -1 }, { -13, -11, 13, 0 }, { -7, -13, 8, 2 }, { 1, -10, 3, 2 }, { 11, -16, -11, 4 },
	{ 16, -14, -15, 1 }, { 19, -10, -19, -2 }, { 20, -5, -20, -4 }, { -20, -8, 19, -2 }, { -17, -11, 17, 1 },
	{ -12, -15, 14, 3 }, { -7, -17, 8, 3 }, { 0, -18, 3, 0 }, { 10, -17, -10, 8 }, { 15, -14, -15, 5 },
	{ 18, -10, -19, 1 }, { 20, -8, -19, -3 }, { -19, -8, 18, -6 }, { -16, -12, 17, 1 }, { -12, -16, 12, 3 },
	{ -5, -18, 9, 6 }, { -1, -11, -3, -6 }
};

const RA1ShotEmitterPair kRA1ShotEmitters251A[27] = {
	{ -1, -11, -3, -6 }, { 7, -12, -8, 1 }, { 14, -11, -12, 0 }, { 18, -9, -17, -1 }, { 21, -7, -19, -4 },
	{ -20, -6, 21, -5 }, { -18, -8, 19, -2 }, { -16, -10, 16, -1 }, { -11, -12, 11, 0 }, { 1, -18, -2, -1 },
	{ 8, -17, -5, 1 }, { 13, -15, -12, 2 }, { 17, -13, -15, 0 }, { 21, -8, -19, -2 }, { -19, -6, 21, -4 },
	{ -18, -10, 19, -3 }, { -15, -14, 17, 1 }, { -10, -15, 11, 4 }, { 1, -19, -2, 6 }, { 7, -18, -7, 8 },
	{ 13, -16, -11, 5 }, { 18, -12, -14, 3 }, { 19, -8, -18, -2 }, { -17, -7, 20, -3 }, { -17, -10, 19, 1 },
	{ -15, -14, 16, 5 }, { 0, -38, -14, 37 }
};

// emitter offsets instead of the generic edge-beam fallback.
const RA1ShotEmitterPair kRA1FlightShotEmitters25EC[45] = {
	{ -38, -14, 37, 6 }, { 37, -14, 37, 7 }, { 42, -11, -40, 11 }, { -37, -6, 38, 14 }, { -37, -5, 38, 15 },
	{ -35, -19, 36, 11 }, { -35, -18, 35, 12 }, { -37, -15, 36, 16 }, { -37, -11, 34, 19 }, { -37, -10, 34, 20 },
	{ -31, -24, 33, 16 }, { -31, -23, 33, 17 }, { -32, -19, 33, 22 }, { -34, -17, 29, 24 }, { -35, -15, 28, 25 },
	{ -25, -28, 29, 21 }, { -25, -28, 29, 20 }, { -30, -25, 28, 27 }, { -30, -20, 24, 28 }, { -31, -19, 23, 29 },
	{ -18, -31, 26, 25 }, { -18, -31, 25, 25 }, { -23, -28, 22, 30 }, { -25, -24, 18, 32 }, { -26, -24, 18, 32 },
	{ 35, -19, -35, 12 }, { 36, -19, -35, 11 }, { 39, -16, -38, 16 }, { 37, -12, -35, 19 }, { 37, -11, -34, 20 },
	{ 30, -23, -33, 17 }, { 31, -23, -33, 17 }, { 33, -20, -32, 21 }, { 35, -17, -29, 23 }, { 34, -17, -30, 24 },
	{ 25, -28, -30, 21 }, { 26, -27, -28, 23 }, { 27, -25, -28, 25 }, { 29, -20, -24, 27 }, { 30, -22, -24, 28 },
	{ 18, -32, -26, 25 }, { 19, -31, -25, 26 }, { 22, -28, -22, 29 }, { 25, -24, -19, 31 }, { 25, -24, -17, 31 }
};

const RA1ShotEmitterPair kRA1FlightShotEmitters25F0[45] = {
	{ 37, -14, -37, 6 }, { -38, -12, -36, 7 }, { -41, -10, 41, 10 }, { 39, -6, -36, 13 }, { 38, -5, -36, 15 },
	{ 41, -8, -39, 1 }, { 40, -7, -38, 1 }, { 41, -4, -40, 5 }, { 39, -1, -40, 9 }, { 39, 1, -40, 10 },
	{ -38, -5, 42, -3 }, { -39, -4, 42, -1 }, { -43, 0, 40, 2 }, { -41, 2, 39, 5 }, { -42, 4, 37, 6 },
	{ -36, -10, 42, 4 }, { -36, -10, 42, 4 }, { -42, -6, 41, 9 }, { -41, -3, 37, 11 }, { -42, -2, 36, 12 },
	{ -33, -15, 42, 10 }, { -33, -15, 42, 10 }, { -39, -12, 39, 14 }, { -39, -8, 36, 17 }, { -41, -8, 33, 17 },
	{ -41, -8, 39, 1 }, { -40, -8, 40, 1 }, { -43, -4, 42, 4 }, { -39, 0, 41, 7 }, { -39, 1, 41, 9 },
	{ 37, -4, -43, -1 }, { 39, -4, -42, -1 }, { 42, -1, -43, 3 }, { 42, 1, -39, 5 }, { 42, 2, -38, 6 },
	{ 36, -10, -43, 5 }, { 38, -8, -41, 5 }, { 38, -7, -42, 8 }, { 41, -3, -37, 11 }, { 41, -4, -37, 11 },
	{ 32, -15, -42, 10 }, { 34, -14, -40, 11 }, { 36, -12, -38, 13 }, { 39, -10, -35, 17 }, { 41, -9, -33, 17 }
};

const RA1ShotEmitterPair kRA1FlightShotEmitters28BC[45] = {
	{ -18, 0, 18, 0 }, { -18, -1, 18, 0 }, { -18, 0, 18, 0 }, { -18, 0, 17, 0 }, { -18, -1, 18, -1 },
	{ -14, -3, 19, 3 }, { -15, -5, 20, 0 }, { -17, -4, 18, 1 }, { -15, -4, 18, 1 }, { -19, -4, 19, 2 },
	{ -13, -9, 20, 2 }, { -16, -8, 19, 3 }, { -15, -9, 21, 3 }, { -14, -4, 18, 5 }, { -14, -2, 17, 6 },
	{ -9, -11, 19, 1 }, { -8, -10, 21, 2 }, { -11, -11, 18, 3 }, { -13, -11, 20, 4 }, { -14, -9, 16, 4 },
	{ -7, -13, 20, 4 }, { -10, -14, 19, 5 }, { -11, -14, 19, 5 }, { -10, -13, 18, 6 }, { -11, -11, 16, 6 },
	{ 14, -5, -18, 0 }, { 16, -6, -19, -1 }, { 17, -5, -20, 1 }, { 17, -5, -19, 2 }, { 17, -2, -16, 2 },
	{ 12, -8, -19, 3 }, { 13, -8, -19, 3 }, { 11, -7, -19, 3 }, { 14, -8, -17, 2 }, { 14, -7, -17, 3 },
	{ -16, 3, 10, -12 }, { -20, 3, 11, -11 }, { -20, 4, 10, -10 }, { -17, 4, 12, -11 }, { -19, 3, 13, -11 },
	{ -18, 3, 5, -13 }, { -17, 3, 7, -14 }, { -18, 3, 8, -15 }, { -19, 3, 9, -11 }, { -16, 5, 11, -11 }
};

void renderSpriteWithFlags(byte *dst, int pitch, int width, int height,
	int x, int y, const RA1Sprite &spr, uint32 flags) {
	if (!spr.data || spr.width <= 0 || spr.height <= 0)
		return;

	int drawX = x;
	int drawY = y;
	if ((flags & 0x1) == 0) {
		drawX += spr.xoffs;
		drawY += spr.yoffs;
	}
	if (flags & 0x2) {
		drawX -= spr.width / 2;
		drawY -= spr.height / 2;
	}

	const bool flipX = (flags & 0x2000) != 0;
	const bool flipY = (flags & 0x4000) != 0;

	int srcOffsetX = 0;
	int srcOffsetY = 0;
	int drawW = spr.width;
	int drawH = spr.height;

	if (drawX < 0) {
		srcOffsetX = -drawX;
		drawW += drawX;
		drawX = 0;
	}
	if (drawY < 0) {
		srcOffsetY = -drawY;
		drawH += drawY;
		drawY = 0;
	}
	if (drawX + drawW > width)
		drawW = width - drawX;
	if (drawY + drawH > height)
		drawH = height - drawY;
	if (drawW <= 0 || drawH <= 0)
		return;

	for (int iy = 0; iy < drawH; iy++) {
		const int srcY = flipY ? (spr.height - 1 - (srcOffsetY + iy)) : (srcOffsetY + iy);
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			const int srcX = flipX ? (spr.width - 1 - (srcOffsetX + ix)) : (srcOffsetX + ix);
			const byte px = spr.data[srcY * spr.width + srcX];
			if (px != 0)
				d[ix] = px;
		}
	}
}

// several GAME handlers. It is collapsed here because the direction/lerp/render
// sequence is identical for one-beam shot sprites.
void renderAimedShotSprite(byte *dst, int pitch, int width, int height,
		const RA1SpriteBank &laserBank, int startX, int startY, int targetX, int targetY,
		int lerp) {
	if (laserBank.numSprites <= 0)
		return;

	const int dir = ra1ShotDirection((int16)startX, (int16)startY, (int16)targetX, (int16)targetY);
	const int sprIdx = MIN<int>(ABS(dir), laserBank.numSprites - 1);
	const uint32 flags = 0x83 | ((dir < 0) ? 0x2000 : 0);
	const int interpX = startX + (((targetX - startX) * lerp) >> 3);
	const int interpY = startY + (((targetY - startY) * lerp) >> 3);

	renderSpriteWithFlags(dst, pitch, width, height,
		interpX, interpY, laserBank.sprites[sprIdx], flags);
}

void renderAimedShotPair(byte *dst, int pitch, int width, int height,
		const RA1SpriteBank &laserBank, int start1X, int start1Y, int start2X, int start2Y,
		int targetX, int targetY, int lerp) {
	renderAimedShotSprite(dst, pitch, width, height, laserBank,
		start1X, start1Y, targetX, targetY, lerp);
	renderAimedShotSprite(dst, pitch, width, height, laserBank,
		start2X, start2Y, targetX, targetY, lerp);
}

// Beam emitters share the same bucketed shot sprite lookup.
void renderBucketedShotSprite(byte *dst, int pitch, int width, int height,
		const RA1SpriteBank &laserBank, int startX, int startY, int targetX, int targetY,
		int lerp, int frame, uint32 flags) {
	if (laserBank.numSprites <= 0)
		return;

	const int dir = ra1ShotDirection((int16)startX, (int16)startY, (int16)targetX, (int16)targetY);
	const int sprIdx = frame + ra1ShotDirectionBucket(dir);
	if (sprIdx < 0 || sprIdx >= laserBank.numSprites)
		return;

	const int interpX = startX + (((targetX - startX) * lerp) >> 3);
	const int interpY = startY + (((targetY - startY) * lerp) >> 3);

	renderSpriteWithFlags(dst, pitch, width, height,
		interpX, interpY, laserBank.sprites[sprIdx], flags);
}

void InsaneRebel1::getTurretShipCenter(int16 &x, int16 &y) const {
	// Recover the ship center from the movement accumulators.
	if (_turretFrameShipCenterValid) {
		x = _turretFrameShipCenterX;
		y = _turretFrameShipCenterY;
		return;
	}

	x = (int16)(kRA1CenterX + (int16)(_posAccumX >> 8));
	y = (int16)(kRA1CenterY + (int16)(_posAccumY >> 8));
}

// window inside the 384x242 framebuffer.
void InsaneRebel1::procPreRendering(byte *renderBitmap) {
	_frameGameOpcodeMask = 0;
	_frameDispatchFlags = 0;
	_hudRenderFlag = 0;
	_gameOp0BPhysicsUpdatedThisFrame = false;
	_gameOp0BOverlayRenderedThisFrame = false;
	_turretFrameShipCenterValid = false;
	_frameObjectHitRevealPending = false;

	if (_interactiveVideoActive && _player) {
		const bool usePerspectiveViewport =
			_frameHasGameChunk &&
			(_activeGameOpcode == 0x07 || _activeGameOpcode == 0x08 ||
			 _activeGameOpcode == 0x09 || _activeGameOpcode == 0x0A ||
			 _activeGameOpcode == 0x0B);
		// GAME stream (for example LVL4/L4PLAY2.ANM) keep a static camera.
		if (usePerspectiveViewport) {
			ra1Player()->_ra1ViewportOffsetX = _perspectiveX;
			ra1Player()->_ra1ViewportOffsetY = _perspectiveY;
		} else {
			ra1Player()->_ra1ViewportOffsetX = 0;
			ra1Player()->_ra1ViewportOffsetY = 0;
		}
	} else if (_player) {
		ra1Player()->_ra1ViewportOffsetX = 0;
		ra1Player()->_ra1ViewportOffsetY = 0;
	}
}

void InsaneRebel1::syncViewportOffset(bool usePerspectiveViewport) {
	if (!_player)
		return;

	if (!usePerspectiveViewport) {
		ra1Player()->_ra1ViewportOffsetX = 0;
		ra1Player()->_ra1ViewportOffsetY = 0;
		return;
	}

	ra1Player()->_ra1ViewportOffsetX = _perspectiveX;
	ra1Player()->_ra1ViewportOffsetY = _perspectiveY;

	// Store shake-adjusted viewport once so overlays and crop share one origin.
	if (_screenFlash > 0) {
		ra1Player()->_ra1ViewportOffsetX += (int16)(_vm->_rnd.getRandomNumber(4) - 2);
		ra1Player()->_ra1ViewportOffsetY += (int16)(_vm->_rnd.getRandomNumber(4) - 2);
	}
}

void InsaneRebel1::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, int32 curFrame, int32 maxFrame) {
	if (_menuActive && renderBitmap) {
		int width = _player ? _player->_width : 0;
		int height = _player ? _player->_height : 0;
		if (width == 0) width = _screenWidth;
		if (height == 0) height = _screenHeight;
		int pitch = width;
		renderMainMenuOverlay(renderBitmap, pitch, width, height);
	}

	// Intro title overlay.
	if (_introTextActive && renderBitmap) {
		int w = _player ? _player->_width : _screenWidth;
		int h = _player ? _player->_height : _screenHeight;
		if (w == 0) w = _screenWidth;
		if (h == 0) h = _screenHeight;
		drawLevelTitleOverlay(renderBitmap, w, w, h, curFrame, maxFrame);
	}

	if (_chapterSummary.active && renderBitmap) {
		int w = _player ? _player->_width : _screenWidth;
		int h = _player ? _player->_height : _screenHeight;
		if (w == 0) w = _screenWidth;
		if (h == 0) h = _screenHeight;
		drawChapterSummaryOverlay(renderBitmap, w, w, h, curFrame, maxFrame);
	}

	if (!_interactiveVideoActive || !renderBitmap)
		return;

	int width = _player ? _player->_width : 0;
	int height = _player ? _player->_height : 0;
	if (width == 0) width = _screenWidth;
	if (height == 0) height = _screenHeight;
	int pitch = width;

	if (!_frameHasGameChunk) {
		syncViewportOffset(false);
		_fireCooldown = _playerFired ? 1 : 0;
		return;
	}

	const bool haveFrameGameOpcodes = (_frameGameOpcodeMask != 0);
	const bool allowImplicitGameplayMode = _frameHasGameChunk && !haveFrameGameOpcodes;

	const bool gameOp0BMode = hasFrameGameOpcode(0x0B) ||
		(allowImplicitGameplayMode && _activeGameOpcode == 0x0B);
	const bool onFootSequenceMode = hasFrameGameOpcode(0x19) ||
		(allowImplicitGameplayMode && _activeGameOpcode == 0x19);
	const bool onFootAimMode = hasFrameGameOpcode(0x1A) ||
		(allowImplicitGameplayMode && _activeGameOpcode == 0x1A);
	const bool onFootMode = onFootSequenceMode || onFootAimMode;
	const bool turretMode = hasFrameGameOpcode(0x08) || hasFrameGameOpcode(0x0A) ||
		(allowImplicitGameplayMode && (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A));
	const bool flightMode = hasFrameGameOpcode(0x07) || hasFrameGameOpcode(0x09) ||
		(allowImplicitGameplayMode &&
			(_activeGameOpcode == 0x07 || _activeGameOpcode == 0x09));
	bool shotOverlayHandled = false;
	bool drawShipAfterGameplayOverlays = false;
	bool drawGameOp0BTargetingAfterFetch = false;
	if (gameOp0BMode) {
		if (!_gameOp0BPhysicsUpdatedThisFrame) {
			updateGameOp0BPhysics();
			_gameOp0BPhysicsUpdatedThisFrame = true;
			syncViewportOffset(true);
		}

		// 0x0B chapters transition to retry/death clips as soon as health drops.
		if (_health < 0) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}

		shotOverlayHandled = _gameOp0BOverlayRenderedThisFrame;
		drawGameOp0BTargetingAfterFetch = _gameOp0BOverlayRenderedThisFrame;
	} else if (onFootMode) {
		if (_currentLevel == 8 && onFootAimMode && !onFootSequenceMode && _killCount > 0) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}

		if (onFootSequenceMode)
			initOnFootSequence();

		// shot overlay uses the current crosshair/pose and is then covered by the
		// character DrawFobjGlyph from 0x19; the 0x19 pose update is for the next
		// frame. Keep that ordering explicit here.
		if (onFootAimMode) {
			shotOverlayHandled = true;
			if (_health >= 0) {
				const bool preserveTargetSnap = (_targetProximity == 2 && _tuning.snap > 0);
				const int16 snappedTargetX = _shipPosX;
				const int16 snappedTargetY = _shipPosY;

				updateOnFootAimVariant();
				if (preserveTargetSnap) {
					_shipPosX = snappedTargetX;
					_shipPosY = snappedTargetY;
				}
				renderShotOverlayPipeline(renderBitmap, pitch, width, height, false);
			}
		}

		// GAME 0x1A-only selector clips reuse the targeting handler but do not
		// draw the walking character.
		if (onFootSequenceMode && _shipBank.numSprites > 0 && _shipDirIndex >= 0 &&
			_shipDirIndex < _shipBank.numSprites) {
			const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];
			int drawX = _onFootCharX + spr.xoffs;
			int drawY = _onFootCharY + spr.yoffs;
			renderSprite(renderBitmap, pitch, width, height, drawX, drawY, spr);
		}

		if (onFootSequenceMode)
			updateOnFootSequence();
		finishOnFootFrame();

		if (_health < 0 && _deathTimer < 2) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}
	} else {
		// Dispatch movement path by GAME handler family:
		//   0x07/0x09 -> flight-family handlers
		if (turretMode) {
			updateTurretPhysics();
		} else if (flightMode) {
			updateShipPhysics();
		}

		// Most RA1 gameplay loops exit the current interactive movie as soon as
		// health drops below 0, then dispatch to the level-specific retry/death
		// clip.
		if (_health < 0) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}

		// The paired GAME 0x09/0x0A handlers draw target boxes, shots and
		// reticles before the 0x07/0x08 ship/cockpit pass.
		drawShipAfterGameplayOverlays = (flightMode || turretMode);
	}

		// Sync viewport before HUD/screen copy so overlays and crop share the camera.
		// On-foot mode uses no viewport crop.
	if (_player) {
		if (onFootMode || (!gameOp0BMode && !turretMode && !flightMode)) {
			syncViewportOffset(false);
		} else if (!gameOp0BMode) {
			syncViewportOffset(true);
		}
	}

	// Targeting overlay is owned by handlers 0x09/0x0A/0x0B/0x1A.
	const bool hasTargetingPipeline =
		!shotOverlayHandled &&
		(hasFrameGameOpcode(0x09) || hasFrameGameOpcode(0x0A) ||
		 hasFrameGameOpcode(0x0B) || hasFrameGameOpcode(0x1A) ||
		 (allowImplicitGameplayMode &&
			(_activeGameOpcode == 0x09 || _activeGameOpcode == 0x0A ||
			 _activeGameOpcode == 0x0B || _activeGameOpcode == 0x1A)));
	if (hasTargetingPipeline) {
		const bool flightVariantTargetingMode =
			hasFrameGameOpcode(0x09) ||
			(allowImplicitGameplayMode && _activeGameOpcode == 0x09);
		const bool turretTargetingMode =
			hasFrameGameOpcode(0x0A) ||
			(allowImplicitGameplayMode && _activeGameOpcode == 0x0A);
		renderShotOverlayPipeline(renderBitmap, pitch, width, height, true);

			// The preceding overlay/shot pass uses the previous frame's cursor; the
			// handler then publishes the next cursor position from the current
			// pre-physics ship offset. The following 0x08 handler updates the camera
			// afterward, so overlay anchors and the viewport crop intentionally
			// observe different moments in the frame.
		if (turretTargetingMode) {
			const int16 shipOffsetX = _turretFrameShipCenterValid ?
				_turretFrameShipOffsetX : (int16)(_posAccumX >> 8);
			const int16 shipOffsetY = _turretFrameShipCenterValid ?
				_turretFrameShipOffsetY : (int16)(_posAccumY >> 8);
			_shipPosX = (int16)(kRA1CenterX + shipOffsetX);
			_shipPosY = (int16)((kRA1CenterY + shipOffsetY - 0x23) - (shipOffsetY >> 3));
		} else if (flightVariantTargetingMode) {
			updateFlightVariantCursor();
		}
	} else if (!shotOverlayHandled) {
		_targetProximity = 0;
		_prevTargetProx = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
	}

	// Route branches use the decoded timeline and updated 0x09 cursor state.
	if (_currentLevel == 6)
		checkDynamicLevelBranch(curFrame);

	if (_currentLevel == 12)
		renderLevel13EnemyShots(renderBitmap, pitch, width, height);

	if (_currentLevel == 6)
		renderLevel7RouteOverlays(renderBitmap, pitch, width, height);

	renderExplosions(renderBitmap, pitch, width, height);

	if (drawShipAfterGameplayOverlays)
		renderShip(renderBitmap, pitch, width, height);

	if (drawGameOp0BTargetingAfterFetch)
		renderTargeting(renderBitmap, pitch, width, height);

	handleLevel14Play2BSplice(curFrame, maxFrame);

	if (_currentLevel == 4 && _levelGameplayPhase == 2)
		renderLevel5Part2Overlay(renderBitmap, pitch, width, height, curFrame);

	if (_currentLevel == 0 && _flyControlMode == 2)
		renderLevelHitsOverlay(renderBitmap, pitch, width, height, 0x04, true);

	if (_currentLevel == 10)
		renderLevelHitsOverlay(renderBitmap, pitch, width, height, 0x16, true);

	if (_currentLevel == 7) {
		updateLevel8WalkerState();
		const int viewportX = _player ? ra1Player()->_ra1ViewportOffsetX : 0;
		const int viewportY = _player ? ra1Player()->_ra1ViewportOffsetY : 0;
		renderLevel8Overlay(renderBitmap, pitch, width, height, viewportX, viewportY);
	}

	if (gameOp0BMode || onFootMode || turretMode || flightMode || _hudRenderFlag != 0)
		renderHUD(renderBitmap, pitch, width, height);
	_fireCooldown = _playerFired ? 1 : 0;
}

void InsaneRebel1::renderShotOverlayPipeline(byte *dst, int pitch, int width, int height,
		bool drawTargetBoxes, bool drawTargeting) {
	if (drawTargetBoxes) {
		renderTargetBoxes(dst, pitch, width, height);
	} else {
		_prevTargetCount = _targetCount;
		_targetCount = 0;
	}

	processShot();
	renderLaserShots(dst, pitch, width, height);
	// This ensures timer==5 first frame is rendered with gun barrel offset and lerp=1.
	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer > 0)
			_shotSlots[i].timer--;
	}
	renderGostSlots(dst, pitch, width, height);
	if (drawTargeting)
		renderTargeting(dst, pitch, width, height);
}

void InsaneRebel1::renderGameOp0BOverlayDuringChunk(byte *dst, int pitch, int width, int height) {
	if (_gameOp0BOverlayRenderedThisFrame || !dst || width <= 0 || height <= 0 || _health < 0)
		return;

	renderShotOverlayPipeline(dst, pitch, width, height, true, false);
	_gameOp0BOverlayRenderedThisFrame = true;
}

void InsaneRebel1::renderTargetBoxes(byte *dst, int pitch, int width, int height) {
	if (_gameplayFlags75fe & 2) {
		_prevTargetCount = _targetCount;
		_targetCount = 0;
		return;
	}
	const int overlayX = ra1OverlayViewOffsetX(this);
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	const bool projectTargetMarkers = (getEffectiveGameOpcode() == 0x0B);

	for (int i = _targetCount - 1; i >= 0; --i) {
		if (i >= kMaxTargetBoxes)
			continue;

		int16 drawX = _targetBoxX[i];
		int16 drawY = _targetBoxY[i];
		if (projectTargetMarkers)
			drawX = (int16)(drawX - _perspectiveX);

		const char boxGlyph = (char)('i' + CLIP<int16>(_targetBoxVariant[i], 0, 5));
		drawCenteredBankGlyph(markerBank, dst, pitch, width, height,
			overlayX + drawX, drawY, boxGlyph);
	}

	_prevTargetCount = _targetCount;
	_targetCount = 0;
}

// whose state depends on _targetProximity.
void InsaneRebel1::renderTargeting(byte *dst, int pitch, int width, int height) {
	const char kRA1TorpedoIndicator[] = "<<d";
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	const int overlayX = ra1GameplayWindowOffsetX(this);
	const int overlayY = ra1GameplayWindowOffsetY(this);

	if (_targetProximity > 0 && _prevTargetProx == 0)
		playSfx(kSfxLockOn, 127, 0);
	else if (_targetProximity == 0 && _prevTargetProx != 0)
		stopSfx(kSfxLockOn);

	if (markerBank.numSprites > 0) {
		// Baseline RA1 targeting uses '^' and animation e..h.
		const bool altMarkerSet = isTorpedoModeActive();

		if ((_gameplayFlags75ff & 0x4) == 0) {
			// Lock indicator at fixed center positions:
			if (_targetProximity > 0) {
				drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x78, ']');
				if (_targetProximity > 1)
					drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x7E, 'a');
			}
		}

		if ((_gameplayFlags75fe & 0x4) == 0) {
			// default marker ('^' or 'x') and animated lock marker (e..h or y..|).
			char marker[2] = { (char)(altMarkerSet ? 'x' : '^'), '\0' };
			if (_targetProximity > 1) {
				_targetAnimCounter++;
				marker[0] = (char)((altMarkerSet ? 'y' : 'e') + (_targetAnimCounter & 3));
			}

			int cursorX = CLIP<int>(overlayX + getGameplayCursorX(), 0, width - 1);
			int cursorY = CLIP<int>(overlayY + getGameplayCursorY(), 0, height - 1);
			drawCenteredBankGlyph(markerBank, dst, pitch, width, height, cursorX, cursorY, marker[0]);

			if (altMarkerSet) {
				const int indicatorWidth = getFontBankStringWidth(kRA1TorpedoIndicator);
				drawFontBankString(dst, pitch, width, height,
					overlayX + 0xA0 - indicatorWidth / 2, overlayY + 0x6E, kRA1TorpedoIndicator);
			}
		}
	}

	_prevTargetProx = _targetProximity;
	_targetProximity = 0;
	_lastHitTarget = 0;
}

void InsaneRebel1::handleLevel14Play2BSplice(int32 curFrame, int32 maxFrame) {
	if (_currentLevel != 13 || _levelGameplayPhase != 2 || _level14Play2BSpliced ||
			_level14Play2BSplicePending || maxFrame < 0x0F)
		return;

	if (curFrame != maxFrame - 0x0F)
		return;

	_level14Play2BSpliced = true;
	_level14Play2BSplicePending = true;
	_level14Play2BSpliceFrame = curFrame;

	if (_player)
		_player->setPreserveGameVideoStateOnRelease(true);

	clearFrameObjectPrimaryBits(1, 0x05);
	clearFrameObjectPrimaryBits(2, 0x40);
	_vm->_smushVideoShouldFinish = true;
}

void InsaneRebel1::renderGostScorePopup(byte *dst, int pitch, int width, int height,
										int16 centerX, int16 centerY, int16 frame) {
	char glyphChar = '\0';
	uint16 scoreValue = (uint16)_tuning.kill;
	if (scoreValue == 10)       glyphChar = 0x72;  // 'r'
	else if (scoreValue == 25)  glyphChar = 0x74;  // 't'
	else if (scoreValue == 50)  glyphChar = 0x73;  // 's'
	else if (scoreValue == 100) glyphChar = 0x6F;  // 'o'
	else if (scoreValue == 200) glyphChar = 0x70;  // 'p'
	else if (scoreValue == 500) glyphChar = 0x71;  // 'q'

	if (glyphChar == '\0')
		return;

	char scoreText[4] = { '<', '<', glyphChar, '\0' };
	drawFontBankString(dst, pitch, width, height,
					   centerX - 4, centerY - frame, scoreText);
}

void InsaneRebel1::renderGostSlots(byte *dst, int pitch, int width, int height) {
	if ((_gameplayFlags75fe & 0x10) != 0)
		return;

	const int overlayX = ra1OverlayViewOffsetX(this);
	const bool projectGostMarkers = (getEffectiveGameOpcode() == 0x0B);
	for (int i = 0; i < kMaxGostSlots; i++) {
		if (_gostSlots[i].targetId != 0 && _gostSlots[i].frame < 10) {
			int16 centerX = _gostSlots[i].posX;
			int16 centerY = _gostSlots[i].posY;
			if (projectGostMarkers)
				centerX = (int16)(centerX - _perspectiveX);

			if (_bangBank.numSprites > 0) {
				int sprIdx = _gostSlots[i].frame;
				if (sprIdx >= _bangBank.numSprites)
					sprIdx = _bangBank.numSprites - 1;

				const RA1Sprite &spr = _bangBank.sprites[sprIdx];
				int drawX = overlayX + centerX - spr.width / 2;
				int drawY = centerY - spr.height / 2;
				renderSprite(dst, pitch, width, height, drawX, drawY, spr);
			}

			if ((_gameplayFlags75ff & 8) == 0) {
				renderGostScorePopup(dst, pitch, width, height,
									overlayX + centerX, centerY, _gostSlots[i].frame);
			}

			_gostSlots[i].frame++;
		}
	}
}

void InsaneRebel1::renderLevelHitsOverlay(byte *dst, int pitch, int width, int height, int y,
		bool screenSpace) {
	if (_hudFontBank.numSprites <= 0 && _techFontBank.numSprites <= 0)
		return;

	int drawX = 0x119;
	int drawY = y;
	if (screenSpace && _player) {
		drawX += ra1Player()->_ra1ViewportOffsetX;
		drawY += ra1Player()->_ra1ViewportOffsetY;
	}

	char hitsStr[16];
	Common::sprintf_s(hitsStr, "<<HITS %02d", (int)_killCount);
	drawFontBankString(dst, pitch, width, height, drawX, drawY, hitsStr);
}

// part-2 instruction reveal, then switches to the live target count.
void InsaneRebel1::renderLevel5Part2Overlay(byte *dst, int pitch, int width, int height, int32 curFrame) {
	if (_hudFontBank.numSprites <= 0 && _techFontBank.numSprites <= 0)
		return;

	const int viewportX = _player ? ra1Player()->_ra1ViewportOffsetX : 0;
	const int viewportY = _player ? ra1Player()->_ra1ViewportOffsetY : 0;
	int16 overlayX = 0x88;
	int16 overlayY = 0x8C;
	projectGameplayPoint(overlayX, overlayY);
	overlayX = (int16)(0x88 - ((overlayX - 0x88) >> 2));
	overlayY = (int16)(0x8C - ((overlayY - 0x8C) >> 2));

	if (curFrame < 0x32) {
		const char instructionText[] = "<<SHOOT TARGETS FOR BONUS";
		const int revealChars = CLIP<int>((int)curFrame, 0, (int)sizeof(instructionText) - 1);
		char revealedText[sizeof(instructionText)];
		Common::strlcpy(revealedText, instructionText, revealChars + 1);
		drawFontBankString(dst, pitch, width, height,
			viewportX + overlayX - 0x27, viewportY + overlayY, revealedText);
	} else {
		char hitsStr[16];
		Common::sprintf_s(hitsStr, "<<HITS: %d", (int)_killCount);
		drawFontBankString(dst, pitch, width, height,
			viewportX + overlayX, viewportY + overlayY, hitsStr);
	}
}

void InsaneRebel1::resetEnemyShotSlots() {
	memset(_enemyShotSlots, 0, sizeof(_enemyShotSlots));
}

// single incoming projectile slot inline in the L7PLAY frontend loop.
void InsaneRebel1::renderLevel7RouteOverlays(byte *dst, int pitch, int width, int height) {
	if (_currentLevel != 6 || !_interactiveVideoActive || _health < 0)
		return;

	if (_level7WarningFrames != 0) {
		const int16 oldWarningFrames = _level7WarningFrames;
		_level7WarningFrames--;

		if ((oldWarningFrames & 7) == 0)
			playSfx(kSfxLockOn, 127, 0);

		const char *warningText = nullptr;
		const int16 warningX = getGameplayCursorX();
		if (_level7WarningFrames < 0x0B) {
			warningText = (_level7WarningThreshold < warningX) ? "<<_" : "<<`";
		} else if ((_level7WarningFrames & 4) != 0) {
			warningText = "<<`_";
		}

		if (warningText != nullptr)
			drawFontBankString(dst, pitch, width, height, 0x11C, 0x16, warningText);
	}

	const int targetCount = CLIP<int>(_prevTargetCount, 0, kMaxTargetBoxes);
	EnemyShotSlot &slot = _enemyShotSlots[0];

	int16 playerX = (int16)(_shipPosX - kRA1CenterX + _perspectiveX);
	int16 playerY = (int16)(_shipPosY - kRA1CenterY + _perspectiveY);
	unprojectGameplayPoint(playerX, playerY);

	if (slot.timer == 0 && targetCount > 0 && _vm->_rnd.getRandomNumber(14) == 0) {
		const int targetIdx = _vm->_rnd.getRandomNumber(targetCount - 1);
		slot.startX = _targetBoxX[targetIdx];
		slot.startY = _targetBoxY[targetIdx];
		slot.targetX = playerX;
		slot.targetY = playerY;
		slot.timer = 7;
		slot.direction = ra1ProjectileDirectionWithFlags(slot.startX, slot.startY,
			slot.targetX, slot.targetY, slot.flags);
		playSfx(kSfxBlast, 127, 0);
	}

	if (slot.timer == 0)
		return;

	const int progress = 9 - slot.timer;
	const int drawX = slot.startX + (((slot.targetX - slot.startX) * progress) >> 2);
	const int drawY = slot.startY + (((slot.targetY - slot.startY) * progress) >> 2);
	const RA1SpriteBank &laserBank = (_enemyLaserBank.numSprites > 0) ? _enemyLaserBank : _laserBank;
	if (slot.direction >= 0 && slot.direction < laserBank.numSprites) {
		renderSpriteWithFlags(dst, pitch, width, height, drawX, drawY,
			laserBank.sprites[slot.direction], slot.flags | 0x3);
	}

	slot.timer--;
	if (drawX - 0x14 < playerX && playerX < drawX + 0x14 &&
		drawY - 0x1E < playerY && playerY < drawY + 0x1E) {
		_damageFlags |= 0x80;
		slot.timer = 0;
	}
}

// five local stack arrays around the L13PLAY frontend loop, not in a named function.
void InsaneRebel1::renderLevel13EnemyShots(byte *dst, int pitch, int width, int height) {
	if (_currentLevel != 12 || !_interactiveVideoActive || _health < 0)
		return;

	const int targetCount = CLIP<int>(_prevTargetCount, 0, kMaxTargetBoxes);
	const uint16 effectiveOpcode = getEffectiveGameOpcode();

	int16 playerX = getGameplayCursorX();
	int16 playerY = getGameplayCursorY();
	if (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A) {
		getTurretShipCenter(playerX, playerY);
	} else if (effectiveOpcode == 0x0B) {
		unprojectGameplayPoint(playerX, playerY);
	}

	for (int i = 0; i < targetCount; i++) {
		if (_targetBoxX[i] - 7 < playerX && playerX < _targetBoxX[i] + 7 &&
			_targetBoxY[i] - 7 < playerY && playerY < _targetBoxY[i] + 7) {
			_damageFlags |= 0x10;
		}
	}

	if (targetCount > 0 && _vm->_rnd.getRandomNumber(19) == 0) {
		for (int i = 0; i < kMaxEnemyShotSlots; i++) {
			EnemyShotSlot &slot = _enemyShotSlots[i];
			if (slot.timer != 0)
				continue;

			const int targetIdx = _vm->_rnd.getRandomNumber(targetCount - 1);
			slot.startX = _targetBoxX[targetIdx];
			slot.startY = _targetBoxY[targetIdx];
			slot.targetX = playerX;
			slot.targetY = playerY;
			slot.timer = 0x0F;
			slot.direction = ra1ProjectileDirectionWithFlags(slot.startX, slot.startY,
				slot.targetX, slot.targetY, slot.flags);
			playSfx(kSfxBlast, 127, 0);
			break;
		}
	}

	for (int i = 0; i < kMaxEnemyShotSlots; i++) {
		EnemyShotSlot &slot = _enemyShotSlots[i];
		if (slot.timer == 0)
			continue;

		const int progress = 0x0F - slot.timer;
		const int drawX = slot.startX + ((slot.targetX - slot.startX) * progress) / 10;
		const int drawY = slot.startY + ((slot.targetY - slot.startY) * progress) / 10;

		if (_enemyLaserBank.numSprites > 0) {
			const int baseSprite = (drawY < 0x50) ? 0x12 : ((drawY < 0xA0) ? 9 : 0);
			const int spriteIdx = baseSprite + slot.direction;
			if (spriteIdx >= 0 && spriteIdx < _enemyLaserBank.numSprites) {
				renderSpriteWithFlags(dst, pitch, width, height, drawX, drawY,
					_enemyLaserBank.sprites[spriteIdx], slot.flags | 0x3);
			}
		}

		slot.timer--;
		if (drawX - 0x0F < playerX && playerX < drawX + 0x0F &&
			drawY - 7 < playerY && playerY < drawY + 7) {
			_damageFlags |= 0x80;
			slot.timer = 0;
		}
	}
}

void InsaneRebel1::renderLaserShots(byte *dst, int pitch, int width, int height) {
	const char kRA1TorpedoTrailLeft[] = "<<&";
	const char kRA1TorpedoTrailRight[] = "<<'";
	const bool torpedoMode = isTorpedoModeActive();

	if (_laserBank.numSprites <= 0 && !torpedoMode)
		return;

	// Entry 0 unused.
	const int kShotLerpByTimer[6] = { 0, 8, 7, 6, 4, 0 };
	const int kOnFootShotLerp[6] = { 0, 8, 7, 6, 4, 1 };
	// Indices 11..19 are the active firing poses from L9PILOT.NUT.
	const int16 kOnFootGunBarrelX[20] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, -56, -47, -23, -13, 0, 13, 30, 54, 59
	};
	const int16 kOnFootGunBarrelY[20] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, -3, -19, -24, -30, -28, -30, -29, -20, -5
	};
	const int spritesPerSet = 5;
	const int overlayX = ra1GameplayWindowOffsetX(this);
	const int overlayY = ra1GameplayWindowOffsetY(this);
	const int leftStartX = 0;
	const int rightStartX = 0x13F; // 319
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	const bool onFootMode = (effectiveOpcode == 0x19 || effectiveOpcode == 0x1A);
	const bool turretMode = (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A);
	const bool flightVariantMode = (effectiveOpcode == 0x09);
	int shipBaseX = overlayX + _shipPosX;
	int shipBaseY = overlayY + _shipPosY;
	if (turretMode) {
		int16 centerX, centerY;
		getTurretShipCenter(centerX, centerY);
		shipBaseX = overlayX + centerX;
		shipBaseY = overlayY + centerY;
	}

	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer > 0 && _shotSlots[i].timer <= spritesPerSet) {
			const int timer = _shotSlots[i].timer;
			const int frame = spritesPerSet - timer;
			const int targetX = CLIP<int>(overlayX + getGameplayCursorX(), 0, width - 1);
			const int targetY = CLIP<int>(overlayY + getGameplayCursorY(), 0, height - 1);

			if (onFootMode) {
				if (timer == 5 && _shipDirIndex > 10 && _shipDirIndex < 20) {
					_shotSlots[i].centerX += kOnFootGunBarrelX[_shipDirIndex];
					_shotSlots[i].centerY += kOnFootGunBarrelY[_shipDirIndex];
				}

				if ((_gameplayFlags75fe & 8) != 0)
					continue;

				const int srcX = _shotSlots[i].centerX;
				const int srcY = _shotSlots[i].centerY;
				const int dstX = _shotSlots[i].posX;
				const int dstY = _shotSlots[i].posY;
				const int onFootLerp = kOnFootShotLerp[timer];

				renderAimedShotSprite(dst, pitch, width, height, _laserBank,
					srcX, srcY, dstX, dstY, onFootLerp);
				continue;
			}

			const int lerp = kShotLerpByTimer[timer];

			if (turretMode) {
				int start1X = 0;
				int start1Y = 0;
				int start2X = 0;
				int start2Y = 0;
				bool haveEmitters = false;
				if (_flyControlMode == 2) {
					start1X = shipBaseX - _turretEmitterLeftX;
					start1Y = shipBaseY + _turretEmitterLeftY;
					start2X = shipBaseX + _turretEmitterRightX;
					start2Y = shipBaseY + _turretEmitterRightY;
					haveEmitters = true;
				} else if (_shipDirIndex >= 0 && _shipDirIndex < 27) {
					const RA1ShotEmitterPair &emit =
						(_shotSlots[i].variant != 0) ? kRA1ShotEmitters244A[_shipDirIndex]
													 : kRA1ShotEmitters251A[_shipDirIndex];
					start1X = shipBaseX + emit.x1;
					start1Y = shipBaseY + emit.y1;
					start2X = shipBaseX + emit.x2;
					start2Y = shipBaseY + emit.y2;
					haveEmitters = true;
				}

				if (!haveEmitters)
					continue;

				renderAimedShotPair(dst, pitch, width, height, _laserBank,
					start1X, start1Y, start2X, start2Y, targetX, targetY, lerp);
				continue;
			}

			if (flightVariantMode) {
				if ((_gameplayFlags75fe & 8) != 0)
					continue;

				if (_shipDirIndex < 0 || _shipDirIndex >= ARRAYSIZE(kRA1FlightShotEmitters28BC))
					continue;

				const RA1ShotEmitterPair &emit =
					(_flyControlMode == 1)
						? ((_shotSlots[i].variant != 0) ? kRA1FlightShotEmitters25EC[_shipDirIndex]
														: kRA1FlightShotEmitters25F0[_shipDirIndex])
						: kRA1FlightShotEmitters28BC[_shipDirIndex];
				const int start1X = shipBaseX + emit.x1;
				const int start1Y = shipBaseY + emit.y1;
				const int start2X = shipBaseX + emit.x2;
				const int start2Y = shipBaseY + emit.y2;
				debugC(DEBUG_INSANE, "op09 shotRender: frame=%d timer=%d shipBase=(%d,%d) target=(%d,%d) emit1=(%d,%d) emit2=(%d,%d) dir=%d variant=%d mode=%d",
					_gameCounter, timer, shipBaseX, shipBaseY, targetX, targetY,
					start1X, start1Y, start2X, start2Y, _shipDirIndex,
					_shotSlots[i].variant, _flyControlMode);

				renderAimedShotPair(dst, pitch, width, height, _laserBank,
					start1X, start1Y, start2X, start2Y, targetX, targetY, lerp);
				continue;
			}

			if (torpedoMode) {
				const int trailY = targetY;
				const int leftTrailX = targetX - 0x14 - (timer << 3);
				const int rightTrailX = targetX + 0x14 + (timer << 3);
				const int leftWidth = getFontBankStringWidth(kRA1TorpedoTrailLeft);
				const int rightWidth = getFontBankStringWidth(kRA1TorpedoTrailRight);
				drawFontBankString(dst, pitch, width, height,
					leftTrailX - leftWidth / 2, trailY, kRA1TorpedoTrailLeft);
				drawFontBankString(dst, pitch, width, height,
					rightTrailX - rightWidth / 2, trailY, kRA1TorpedoTrailRight);
				continue;
			}

			int leftStartY = overlayY + 0x96;
			int rightStartY = overlayY + 0x96;
			uint32 leftFlags = 0x83;
			uint32 rightFlags = 0x2083;

			if (_flyControlMode == 1) {
				if (_shotSideToggle) {
					leftFlags = 0x4083;
					leftStartY = overlayY;
					rightStartY = overlayY + 0x96;
				} else {
					rightFlags = 0x6083;
					leftStartY = overlayY + 0x96;
					rightStartY = overlayY;
				}
				if (timer == 1)
					_shotSideToggle = !_shotSideToggle;
			}

			const int startLeftX = overlayX + leftStartX;
			const int startRightX = overlayX + rightStartX;
			renderBucketedShotSprite(dst, pitch, width, height, _laserBank,
				startLeftX, leftStartY, targetX, targetY, lerp, frame, leftFlags);
			renderBucketedShotSprite(dst, pitch, width, height, _laserBank,
				startRightX, rightStartY, targetX, targetY, lerp, frame, rightFlags);
		}
	}
}

// revealStartFrame/revealEndFrame control the frame range during which text is visible.
const struct {
	const char *titleText;      // Top line (chapter number)
	const char *subtitleText;   // Bottom line (level name)
	int16 revealStartFrame;     // Frame at which text begins appearing
	int16 revealEndFrame;       // Frame after which text stops
} kLevelTitles[] = {
	{ "Chapter 1",  "Flight Training",        -80, -30 },
	{ "Chapter 2",  "Asteroid Field Training",  40, 100 },
	{ "Chapter 3",  "Planet Kolaador",           0,  50 },
	{ "Chapter 4",  "Star Destroyer Attack",     5,  50 },
	{ "Chapter 5",  "Tatooine Attack",         346, 390 },
	{ "Chapter 6",  "Asteroid Field Chase",     30,  80 },
	{ "Chapter 7",  "Imperial Probe Droids",   166, 230 },
	{ "Chapter 8",  "Imperial Walkers",         30,  75 },
	{ "Chapter 9",  "Stormtroopers",             2,  45 },
	{ "Chapter 10", "Protect Rebel Transport",  10,  75 },
	{ "Chapter 11", "Yavin Training",            1,  45 },
	{ "Chapter 12", "Tie Attack",               15,  70 },
	{ "Chapter 13", "Death Star Surface",       10,  60 },
	{ "Chapter 14", "Surface Cannon",          122, 175 },
	{ "Chapter 15", "Death Star Trench",         1,  49 },
};

void InsaneRebel1::beginLevelTitleOverlay(int level) {
	if (level < 0 || level >= ARRAYSIZE(kLevelTitles)) {
		_introTextActive = false;
		return;
	}
	_introTextActive = true;
	_introTextLevel = level;
	_introTextStartFrame = kLevelTitles[level].revealStartFrame;
	_introTextEndFrame = kLevelTitles[level].revealEndFrame;
}

void InsaneRebel1::drawLevelTitleOverlay(byte *dst, int pitch, int width, int height, int32 curFrame, int32 maxFrame) {
	if (!_introTextActive || _introTextLevel < 0 || _introTextLevel >= ARRAYSIZE(kLevelTitles))
		return;

	// Resolve negative frame values (relative to end of video, e.g. Chapter 1)
	if (_introTextStartFrame < 0)
		_introTextStartFrame = maxFrame + _introTextStartFrame;
	if (_introTextEndFrame < 0)
		_introTextEndFrame = maxFrame + _introTextEndFrame;

	// Only draw within the frame window
	if (curFrame < _introTextStartFrame || curFrame >= _introTextEndFrame) {
		if (curFrame >= _introTextEndFrame)
			_introTextActive = false;
		return;
	}

	const char *title = kLevelTitles[_introTextLevel].titleText;
	const char *subtitle = kLevelTitles[_introTextLevel].subtitleText;

	int titleW = getFontBankStringWidth(title);
	int subtitleW = getFontBankStringWidth(subtitle);
	int centerX = width / 2;

	drawFontBankString(dst, pitch, width, height, centerX - titleW / 2, 10, title);
	drawFontBankString(dst, pitch, width, height, centerX - subtitleW / 2, 25, subtitle);
}

void InsaneRebel1::beginChapterSummaryOverlay(int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1, const char *detailText1, int bonusValue1,
		const char *bonusLabel2, const char *detailText2, int bonusValue2,
		int passwordIndex) {
	memset(&_chapterSummary, 0, sizeof(_chapterSummary));
	_chapterSummary.active = true;
	_chapterSummary.revealOffsetFromEnd = revealOffsetFromEnd;
	_chapterSummary.stopOffsetFromEnd = stopOffsetFromEnd;
	_chapterSummary.bonusValue1 = bonusValue1;
	_chapterSummary.bonusValue2 = bonusValue2;
	_chapterSummary.passwordIndex = passwordIndex;

	if (bonusLabel1 && detailText1) {
		_chapterSummary.hasBonus1 = true;
		Common::strlcpy(_chapterSummary.bonusLabel1, bonusLabel1, sizeof(_chapterSummary.bonusLabel1));
		Common::strlcpy(_chapterSummary.detailText1, detailText1, sizeof(_chapterSummary.detailText1));
	}
	if (bonusLabel2 && detailText2) {
		_chapterSummary.hasBonus2 = true;
		Common::strlcpy(_chapterSummary.bonusLabel2, bonusLabel2, sizeof(_chapterSummary.bonusLabel2));
		Common::strlcpy(_chapterSummary.detailText2, detailText2, sizeof(_chapterSummary.detailText2));
	}
}

// Passcodes are stored in clear text.
static const char *const kChapterCompletePasswords[] = {
	"FALCON", "BIGGS", "ACKBAR", "ANOAT", "KAIBURR",
	"FORNAX", "YUZZEM", "MYNOCK", "BESPIN", "BRIGIA",
	"DAGOBAH", "KESSEL", "GREEDO", "MIMBAN", "ORGANA"
};

const char *InsaneRebel1::getChapterCompletePassword(int passwordIndex) const {
	if (passwordIndex < 1 || passwordIndex > (int)ARRAYSIZE(kChapterCompletePasswords))
		return nullptr;

	return kChapterCompletePasswords[passwordIndex - 1];
}

void InsaneRebel1::drawChapterSummaryOverlay(byte *dst, int pitch, int width, int height,
		int32 curFrame, int32 maxFrame) {
	if (!_chapterSummary.active || !dst || maxFrame <= 0)
		return;

	const int32 revealBaseFrame = maxFrame - _chapterSummary.revealOffsetFromEnd;
	const int32 stopFrame = maxFrame - _chapterSummary.stopOffsetFromEnd;
	if (curFrame < revealBaseFrame || curFrame >= stopFrame)
		return;

	const int centerX = width / 2;
	const int titleChars = MAX<int>(0, (int)(curFrame - revealBaseFrame));
	drawCenteredRebel1String(this, dst, pitch, width, height,
		centerX, 5, "Chapter Complete", titleChars);

	if (revealBaseFrame + 0x0F < curFrame) {
		char completionText[40];
		Common::sprintf_s(completionText, "Completion bonus: %d", (int)_tuning.levelPts);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x19, completionText);
	}

	if (_chapterSummary.hasBonus1 &&
			revealBaseFrame + 0x28 < curFrame && curFrame < revealBaseFrame + 0x46) {
		char bonusText[32];
		Common::sprintf_s(bonusText, "Bonus: %d", _chapterSummary.bonusValue1);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x32, _chapterSummary.bonusLabel1);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x46, _chapterSummary.detailText1);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x5A, bonusText);
	}

	if (_chapterSummary.hasBonus2 &&
			revealBaseFrame + 0x55 < curFrame && curFrame < revealBaseFrame + 0x73) {
		char bonusText[32];
		Common::sprintf_s(bonusText, "Bonus: %d", _chapterSummary.bonusValue2);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x32, _chapterSummary.bonusLabel2);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x46, _chapterSummary.detailText2);
		drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x5A, bonusText);
	}

	if (_chapterSummary.passwordIndex > 0 &&
			revealBaseFrame + 10 < curFrame) {
		const char *password = getChapterCompletePassword(_chapterSummary.passwordIndex);
		if (password) {
			char passwordText[40];
			Common::sprintf_s(passwordText, "Password: %s", password);
			drawCenteredRebel1String(this, dst, pitch, width, height, centerX, 0x73, passwordText);
		}
	}
}

// supports '<'/'>' layer markup and layer-2 space handling used by RA1 HUD/targeting strings.
void InsaneRebel1::drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text) {
	if (!text || !dst)
		return;

	// Current RA1 integration maps subtitle/HUD-relevant layers as:
	//   layer 0   -> TITLE font bank
	//   layer 1   -> TALK/HUD font bank
	//   layer 2+  -> TECH font bank
	int layer = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		char ch = text[i];
		if (ch == '<') {
			layer++;
			continue;
		}
		if (ch == '>') {
			layer = MAX(0, layer - 1);
			continue;
		}

		const bool techLayer = (layer >= 2);
		const RA1SpriteBank &bank = selectLayerBank(_titleFontBank, _hudFontBank, _techFontBank, layer);
		if (bank.numSprites <= 0)
			return;

		if (ch == ' ') {
			if (techLayer) {
				drawBankString(bank, dst, pitch, width, height, x, y, "!");
				x += getBankStringWidth(bank, "!");
			} else {
				x += getBankSpaceAdvance(bank);
			}
			continue;
		}

		char glyph[2] = { ch, '\0' };
		drawBankString(bank, dst, pitch, width, height, x, y, glyph);
		x += getBankStringWidth(bank, glyph);
	}
}

int InsaneRebel1::getFontBankStringWidth(const char *text) {
	if (!text)
		return 0;

	int w = 0;
	int layer = 0;
	for (int i = 0; text[i] != '\0'; i++) {
		char ch = text[i];
		if (ch == '<') {
			layer++;
			continue;
		}
		if (ch == '>') {
			layer = MAX(0, layer - 1);
			continue;
		}

		const bool techLayer = (layer >= 2);
		const RA1SpriteBank &bank = selectLayerBank(_titleFontBank, _hudFontBank, _techFontBank, layer);
		if (bank.numSprites <= 0)
			return w;

		if (ch == ' ') {
			w += techLayer ? getBankStringWidth(bank, "!") : getBankSpaceAdvance(bank);
			continue;
		}

		char glyph[2] = { ch, '\0' };
		w += getBankStringWidth(bank, glyph);
	}
	return w;
}

int InsaneRebel1::getFontBankLineAdvance(const char *text) {
	int layer = 0;
	if (text) {
		for (int i = 0; text[i] != '\0'; i++) {
			if (text[i] == '<')
				layer++;
			else if (text[i] == '>')
				layer = MAX(layer - 1, 0);
		}
	}

	const RA1SpriteBank &bank = selectLayerBank(_titleFontBank, _hudFontBank, _techFontBank, layer);
	return getBankSpaceHeight(bank) + 4;
}

void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	if (_health < 0 && _deathTimer <= 20)
		return;

	const RA1SpriteBank *shipBank = &_shipBank;
	if (_currentLevel == 0 && _flyControlMode == 2 && _shipBankAlt.numSprites > 0)
		shipBank = &_shipBankAlt;

	if (_shipDirIndex < 0 || _shipDirIndex >= shipBank->numSprites)
		return;

	const RA1Sprite &spr = shipBank->sprites[_shipDirIndex];

	// In 0x08/0x0A turret handlers, _shipPos holds targeting/cursor state.
	// Flight handlers already store the ship sprite center in _shipPos.
	int shipScreenX = _shipPosX;
	int shipScreenY = _shipPosY;
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	if (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A) {
		int16 centerX, centerY;
		getTurretShipCenter(centerX, centerY);
		shipScreenX = centerX;
		shipScreenY = centerY;
	}

	int drawX = ra1GameplayWindowOffsetX(this) + shipScreenX - spr.width / 2;
	int drawY = ra1GameplayWindowOffsetY(this) + shipScreenY - spr.height / 2;

	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

void InsaneRebel1::renderExplosions(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	const int overlayX = ra1GameplayWindowOffsetX(this);
	const int overlayY = ra1GameplayWindowOffsetY(this);
	// In 0x08/0x0A turret handlers, explosion anchors use the ship center, not
	// the targeting cursor stored in _shipPos. Flight handlers already keep the
	// ship center in _shipPos.
	int shipScreenX = overlayX + _shipPosX;
	int shipScreenY = overlayY + _shipPosY;
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	if (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A) {
		int16 centerX, centerY;
		getTurretShipCenter(centerX, centerY);
		shipScreenX = overlayX + centerX;
		shipScreenY = overlayY + centerY;
	}

	if (_health < 0 && _deathTimer > 10) {
		int intensity = _deathTimer - 10;
		if (intensity > 10)
			intensity = 20 - intensity;

		int rangeY = intensity * 4 + 1;
		int rangeX = -20 + intensity * 4;
		if (rangeX < 1) rangeX = 1;

		for (int i = 0; i < intensity; i++) {
			int sprIdx = _vm->_rnd.getRandomNumber(_bangBank.numSprites - 1);

			int randX = (int)_vm->_rnd.getRandomNumber(rangeX * 2) - rangeX;
			int randY = (int)_vm->_rnd.getRandomNumber(rangeY * 2) - rangeY;

			int drawX = shipScreenX + randX;
			int drawY = shipScreenY + randY;

			const RA1Sprite &spr = _bangBank.sprites[sprIdx];
			renderSprite(dst, pitch, width, height,
				drawX - spr.width / 2, drawY - spr.height / 2, spr);
		}
		return;
	}

	if (_health >= 0 && _damageCooldown > 0) {
		int sprIdx = _bangBank.numSprites - _damageCooldown;
		if (sprIdx < 0 || sprIdx >= _bangBank.numSprites)
			return;

		int drawX = shipScreenX;
		int drawY = shipScreenY;

		const RA1Sprite &spr = _bangBank.sprites[sprIdx];
		renderSprite(dst, pitch, width, height,
			drawX - spr.width / 2, drawY - spr.height / 2, spr);
	}
}

void InsaneRebel1::renderHUD(byte *dst, int pitch, int width, int height) {
	if (_score / 10000 > _prevScore / 10000) {
		_lives++;
		playSfx(kSfxBonus, 127, 0);
	}
	_prevScore = _score;

	if (_displayBank.numSprites == 0)
		return;

	const RA1Sprite &bar = _displayBank.sprites[0];

	// Anchor HUD at the window origin so perspective cropping does not move it.
	int hudOriginX = 0;
	int hudOriginY = 0;
	if (_interactiveVideoActive && _player) {
		hudOriginX = ra1Player()->_ra1ViewportOffsetX;
		hudOriginY = ra1Player()->_ra1ViewportOffsetY;
	}

	const int hudX = hudOriginX + bar.xoffs;
	const int hudY = hudOriginY + bar.yoffs;
	// masks/text below use the unshifted screen-space coordinate origin.
	const int hudPlateX = hudX + 5;
	const int hudPlateY = hudY;

	// which selects the opaque blit path. Keep zero-valued pixels black instead
	// of treating them as transparent.
	if (bar.data && bar.width > 0 && bar.height > 0) {
		int drawX = hudPlateX, drawY = hudPlateY, drawW = bar.width, drawH = bar.height;
		int srcOffX = 0, srcOffY = 0;
		const int clipLeft = hudX + 4;
		const int clipTop = hudY;
		const int clipRight = hudX + 315;
		const int clipBottom = hudY + 19;
		if (drawX < clipLeft) {
			srcOffX = clipLeft - drawX;
			drawW -= srcOffX;
			drawX = clipLeft;
		}
		if (drawY < clipTop) {
			srcOffY = clipTop - drawY;
			drawH -= srcOffY;
			drawY = clipTop;
		}
		if (drawX + drawW - 1 > clipRight) drawW = clipRight - drawX + 1;
		if (drawY + drawH - 1 > clipBottom) drawH = clipBottom - drawY + 1;
		if (drawX < 0) { srcOffX = -drawX; drawW += drawX; drawX = 0; }
		if (drawY < 0) { srcOffY = -drawY; drawH += drawY; drawY = 0; }
		if (drawX + drawW > width) drawW = width - drawX;
		if (drawY + drawH > height) drawH = height - drawY;

		if (drawW > 0 && drawH > 0) {
			for (int iy = 0; iy < drawH; iy++) {
				const byte *s = bar.data + (srcOffY + iy) * bar.width + srcOffX;
				byte *d = dst + (drawY + iy) * pitch + drawX;
				memcpy(d, s, drawW);
			}
		}
	}

	{
		int barMaxW = kMaxHealth;
		int barH = 5;
		int healthWidth = CLIP<int16>(_health, 0, kMaxHealth);
		int barX = hudX + (0x92 - healthWidth);
		int barY = hudY + 8;
		int fillW = CLIP(healthWidth, 0, barMaxW);

		for (int iy = 0; iy < barH && barY + iy < height; iy++) {
			byte *d = dst + (barY + iy) * pitch + barX;
			for (int ix = 0; ix < fillW && barX + ix < width; ix++) {
				d[ix] = 0x00;
			}
		}
	}

	// Lives: black out excess pilot icons embedded in DISPLAY.NUT background.
	// Icons are 5 slots at x=186..236, each ~10px wide. Cover unused slots with black.
	if (_lives >= 0 && _lives < 5) {
		int coverX = hudX + _lives * 10 + 186;
		int coverY = hudY + 6;
		int coverW = 51 - _lives * 10;
		int coverH = 9;
		if (coverX >= 0 && coverY >= 0) {
			for (int iy = 0; iy < coverH && coverY + iy < height; iy++) {
				byte *d = dst + (coverY + iy) * pitch + coverX;
				for (int ix = 0; ix < coverW && coverX + ix < width; ix++) {
					d[ix] = 0x00;
				}
			}
		}
	}

	if (_hudFontBank.numSprites > 0 || _techFontBank.numSprites > 0) {
		char scoreStr[24];
		Common::sprintf_s(scoreStr, "<<%06d", MAX<int>(_score, 0));
		drawFontBankString(dst, pitch, width, height, hudX + 273, hudY + 5, scoreStr);
	}

	// if (health < miss*2 || health < wham*2 || health < shot*2) and (frame & 8),
	// draw warning glyph at (0x49, 0x07). Two variants:
	//   "<<[" when above critical thresholds, "<<\\" when critical.
	{
		const bool lowHealthBand =
			((_health < _tuning.miss * 2) ||
			 (_health < _tuning.wham * 2) ||
			 (_health < _tuning.shot * 2)) &&
			((_frameCounter & 8) != 0);
		if (lowHealthBand) {
			const bool aboveCritical =
				(_health > _tuning.miss) &&
				(_health > _tuning.wham) &&
				(_health > _tuning.shot);
			const char *warningStr = aboveCritical ? "<<[" : "<<\\";
			drawFontBankString(dst, pitch, width, height, hudX + 0x49, hudY + 0x07, warningStr);
			if (!aboveCritical && ((_frameCounter & 0x7) == 0))
				playSfx(kSfxKlaxon, 127, 0);
		}
	}

	_hudDirtyFlag = 0xFF;
}

// Each route has up to 3 attack windows. -2 means disabled.
const int16 InsaneRebel1::kWalkerAttackWindow1[3] = { 2588, 2323, 877 };
const int16 InsaneRebel1::kWalkerAttackWindow2[3] = { 1709, 1444, -2 };
const int16 InsaneRebel1::kWalkerAttackWindow3[3] = { 262, -2, -2 };

void InsaneRebel1::updateLevel8WalkerState() {
	if (_walkerHealth >= 11) {
		_walkerHealth = (int16)(100 - (_killCount + (_killCount >> 2)));
	} else if (_walkerHealth > 0 && (_gameCounter & 3) == 0) {
		_walkerHealth--;
	}

	if (_walkerHealth <= 0) {
		_vm->_smushVideoShouldFinish = true;
		return;
	}

	int route = CLIP(_levelRouteIndex, 0, 2);
	uint16 fc = (uint16)_gameCounter;

	const int16 *windows[3] = {
		&kWalkerAttackWindow1[route],
		&kWalkerAttackWindow2[route],
		&kWalkerAttackWindow3[route]
	};
	int16 frameNum = (int16)fc;

	bool inWindow = false;
	for (int w = 0; w < 3; w++) {
		int16 windowEnd = *windows[w];
		if (windowEnd < 0) continue;
		if (frameNum > windowEnd - 100 && frameNum <= windowEnd) {
			inWindow = true;

			if (frameNum == windowEnd - 99)
				_walkerTimer = 100;
			break;
		}
	}

	if (inWindow && _walkerBranchChoice == 0) {
		_walkerTimer--;

		bool inDirectionalPhase = false;
		for (int w = 0; w < 3; w++) {
			int16 windowEnd = *windows[w];
			if (windowEnd < 0) continue;
			if (frameNum > windowEnd - 0x32 && frameNum <= windowEnd) {
				inDirectionalPhase = true;
				break;
			}
		}

		if (inDirectionalPhase) {
			if (_playerFired && _inputAxisDeltaX == 0) {
				_walkerBranchChoice = (_shipPosX < 0xA0) ? 1 : 2;
			}
		} else {
			if ((_gameCounter & 7) == 0)
				playSfx(kSfxLockOn, 127, 0);
		}
	}

	for (int w = 0; w < 3; w++) {
		int16 windowEnd = *windows[w];
		if (windowEnd < 0) continue;
		if (fc != (uint16)windowEnd) continue;

		int newRoute = 0;
		bool goLeft = (_walkerBranchChoice == 1) ||
					  (_shipPosX < 0xA0 && _walkerBranchChoice != 2);

		if (goLeft) {
			if (w != 2)
				newRoute = 1;
		} else {
			if (w != 1)
				newRoute = 2;
		}

		if (newRoute != 0 && newRoute != route) {
			_pendingRouteIndex = newRoute;
			_pendingRouteCutoverFrame = _gameCounter + 7;
			_pendingRouteStartFrame = _pendingRouteCutoverFrame;
			debugC(DEBUG_INSANE, "L8 branch: route=%d -> %d at frame=%u shipX=%d resumeTimelineFrame=%d cutoverFrame=%d",
				route, newRoute, (unsigned)_gameCounter, _shipPosX,
				(int)_pendingRouteStartFrame, (int)_pendingRouteCutoverFrame);
		}
		_walkerBranchChoice = 0;
		break;
	}
}

void InsaneRebel1::renderLevel8Overlay(byte *dst, int pitch, int width, int height,
		int viewportX, int viewportY) {
	if (_currentLevel != 7)
		return;

	if (_walkerHealth > 0 && (_walkerHealth >= 16 || (_gameCounter & 2) != 0)) {
		int16 projX = 0x61, projY = 0x8D;
		projectGameplayPoint(projX, projY);
		projX = (int16)(0x61 - ((projX - 0x61) >> 2));
		projY = (int16)(0x8D - ((projY - 0x8D) >> 2));

		char walkerStr[24];
		Common::sprintf_s(walkerStr, "<<WALKER %d%%", (int)_walkerHealth);
		drawFontBankString(dst, pitch, width, height,
			viewportX + projX, viewportY + projY, walkerStr);
	}

	int route = CLIP(_levelRouteIndex, 0, 2);
	const int16 *windows[3] = {
		&kWalkerAttackWindow1[route],
		&kWalkerAttackWindow2[route],
		&kWalkerAttackWindow3[route]
	};
	int16 frameNum = (int16)(uint16)_gameCounter;

	bool inWindow = false;
	bool inDirectionalPhase = false;
	for (int w = 0; w < 3; w++) {
		int16 windowEnd = *windows[w];
		if (windowEnd < 0) continue;
		if (frameNum > windowEnd - 100 && frameNum <= windowEnd) {
			inWindow = true;
			if (frameNum > windowEnd - 0x32 && frameNum <= windowEnd)
				inDirectionalPhase = true;
			break;
		}
	}

	if (!inWindow || _walkerBranchChoice != 0)
		return;

	{
		int16 projX = 0x62, projY = 0x9C;
		projectGameplayPoint(projX, projY);
		projX = (int16)(0x62 - ((projX - 0x62) >> 2));
		projY = (int16)(0x9C - ((projY - 0x9C) >> 2));

		char timerStr[16];
		Common::sprintf_s(timerStr, "<<TIME %d", (int)_walkerTimer);
		drawFontBankString(dst, pitch, width, height,
			viewportX + projX, viewportY + projY, timerStr);
	}

	if (inDirectionalPhase) {
		int16 projX = 0, projY = 0;
		projectGameplayPoint(projX, projY);
		const int16 parallaxX = (int16)(projX >> 2);
		const int16 parallaxY = (int16)(projY >> 2);

		if (_shipPosX < 0xA0) {
			drawFontBankString(dst, pitch, width, height,
				viewportX + 0xA6 - parallaxX, viewportY + 0x92 - parallaxY, "<<v");
		} else {
			drawFontBankString(dst, pitch, width, height,
				viewportX + 0xA8 - parallaxX, viewportY + 0x93 - parallaxY, "<<u");
		}
	} else {
		if ((_gameCounter & 4) == 0) {
			int16 projX = 0, projY = 0;
			projectGameplayPoint(projX, projY);
			int16 drawX = (int16)(0xA9 - (projX >> 2));
			int16 drawY = (int16)(0x9A - (projY >> 2));
			drawFontBankString(dst, pitch, width, height,
				viewportX + drawX, viewportY + drawY, "<<w");
		}
	}
}

void InsaneRebel1::renderSprite(byte *dst, int pitch, int width, int height,
								int x, int y, const RA1Sprite &spr) {
	if (!spr.data || spr.width <= 0 || spr.height <= 0)
		return;

	int drawX = x, drawY = y, drawW = spr.width, drawH = spr.height;
	int srcOffsetX = 0, srcOffsetY = 0;

	if (drawX < 0) { srcOffsetX = -drawX; drawW += drawX; drawX = 0; }
	if (drawY < 0) { srcOffsetY = -drawY; drawH += drawY; drawY = 0; }
	if (drawX + drawW > width) drawW = width - drawX;
	if (drawY + drawH > height) drawH = height - drawY;
	if (drawW <= 0 || drawH <= 0)
		return;

	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = spr.data + (srcOffsetY + iy) * spr.width + srcOffsetX;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			byte px = s[ix];
			if (px != 0)
				d[ix] = px;
		}
	}
}

} // End of namespace Scumm
