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
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

static inline int ra1OverlayViewOffsetX(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	// In opcode 0x0B (FUN_1CDA7), marker/shot coordinates are in the gameplay
	// window. Under ScummVM's FUN_224FD crop emulation, shift them into the
	// 384-wide source buffer so they stay aligned after the source-window crop.
	return (rebel1->getEffectiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveX() : 0;
}

static inline int ra1OverlayViewOffsetY(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	return (rebel1->getEffectiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveY() : 0;
}

static void drawBankString(const RA1SpriteBank &bank, byte *dst, int pitch, int width, int height,
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

static const RA1Sprite *lookupBankGlyph(const RA1SpriteBank &bank, char ch) {
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

// Glyph markers in FUN_1C940/FUN_1CB22 go through DrawStringEx(..., flags=3),
// which centers the glyph and ignores the NUT x/y offsets. Use the same anchor
// rules here instead of the generic left-anchored text path.
static void drawCenteredBankGlyph(const RA1SpriteBank &bank, byte *dst, int pitch, int width, int height,
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

static int getBankStringWidth(const RA1SpriteBank &bank, const char *text) {
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

// Approximate FUN_221B7/FUN_20BD3 space-advance behavior from available NUT glyphs.
// The original reads per-font space width from metadata tables and caps it to 8.
static int getBankSpaceAdvance(const RA1SpriteBank &bank) {
	const int exclWidth = getBankStringWidth(bank, "!");
	if (exclWidth <= 0)
		return 6;
	return MIN(exclWidth, 8);
}

static const RA1SpriteBank &selectLayerBank(const RA1SpriteBank &titleBank,
		const RA1SpriteBank &hudBank, const RA1SpriteBank &techBank, int layer) {
	const bool techLayer = (layer >= 2);
	const bool talkLayer = (layer == 1);
	if (techLayer)
		return (techBank.numSprites > 0) ? techBank : hudBank;
	if (talkLayer)
		return hudBank;
	return (titleBank.numSprites > 0) ? titleBank : hudBank;
}

static int getBankSpaceHeight(const RA1SpriteBank &bank) {
	// In FUN_221B7 line advance is derived from the layer's space-glyph height (+4).
	// With current NUT decoding we approximate that using the '!' glyph (index 0).
	if (bank.numSprites > 0) {
		const RA1Sprite &glyph = bank.sprites[0];
		if (glyph.height > 0)
			return glyph.height;
	}
	return 8;
}

// FUN_1C794: direction bucket in range -4..4 from two points.
static int ra1ShotDirection(int16 x1, int16 y1, int16 x2, int16 y2) {
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

// FUN_1CDA7 maps abs(FUN_1C794) to sprite base index: <=1 -> 0, ==2 -> 5, else -> 10.
static int ra1ShotDirectionBucket(int dir) {
	const int absDir = ABS(dir);
	if (absDir <= 1)
		return 0;
	if (absDir == 2)
		return 5;
	return 10;
}

struct RA1ShotEmitterPair {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
};

// DAT_244A and DAT_251A in ASSAULT.EXE data section, used by FUN_1D79C.
static const RA1ShotEmitterPair kRA1ShotEmitters244A[27] = {
	{ 11, -11, -11, 0 }, { 16, -9, -16, -1 }, { 20, -6, -19, -3 }, { 20, -5, -21, -4 }, { -20, -6, 20, -5 },
	{ -18, -9, 16, -1 }, { -13, -11, 13, 0 }, { -7, -13, 8, 2 }, { 1, -10, 3, 2 }, { 11, -16, -11, 4 },
	{ 16, -14, -15, 1 }, { 19, -10, -19, -2 }, { 20, -5, -20, -4 }, { -20, -8, 19, -2 }, { -17, -11, 17, 1 },
	{ -12, -15, 14, 3 }, { -7, -17, 8, 3 }, { 0, -18, 3, 0 }, { 10, -17, -10, 8 }, { 15, -14, -15, 5 },
	{ 18, -10, -19, 1 }, { 20, -8, -19, -3 }, { -19, -8, 18, -6 }, { -16, -12, 17, 1 }, { -12, -16, 12, 3 },
	{ -5, -18, 9, 6 }, { -1, -11, -3, -6 }
};

static const RA1ShotEmitterPair kRA1ShotEmitters251A[27] = {
	{ -1, -11, -3, -6 }, { 7, -12, -8, 1 }, { 14, -11, -12, 0 }, { 18, -9, -17, -1 }, { 21, -7, -19, -4 },
	{ -20, -6, 21, -5 }, { -18, -8, 19, -2 }, { -16, -10, 16, -1 }, { -11, -12, 11, 0 }, { 1, -18, -2, -1 },
	{ 8, -17, -5, 1 }, { 13, -15, -12, 2 }, { 17, -13, -15, 0 }, { 21, -8, -19, -2 }, { -19, -6, 21, -4 },
	{ -18, -10, 19, -3 }, { -15, -14, 17, 1 }, { -10, -15, 11, 4 }, { 1, -19, -2, 6 }, { 7, -18, -7, 8 },
	{ 13, -16, -11, 5 }, { 18, -12, -14, 3 }, { 19, -8, -18, -2 }, { -17, -7, 20, -3 }, { -17, -10, 19, 1 },
	{ -15, -14, 16, 5 }, { 0, -38, -14, 37 }
};

// DAT_25EC/DAT_25F0 and DAT_28BC in ASSAULT.EXE. GAME opcode 0x09 uses these
// emitter offsets instead of the generic edge-beam fallback.
static const RA1ShotEmitterPair kRA1FlightShotEmitters25EC[45] = {
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

static const RA1ShotEmitterPair kRA1FlightShotEmitters25F0[45] = {
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

static const RA1ShotEmitterPair kRA1FlightShotEmitters28BC[45] = {
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

// Small subset of FUN_20D43 draw flags used by RA1 shot sprites.
static void renderSpriteWithFlags(byte *dst, int pitch, int width, int height,
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

// procPreRendering — Sets viewport window offset (FUN_224FD at 0x224FD).
// RA1 decodes FOBJs at chunk coordinates, then displays a scrolled 320x200
// window inside the 384x242 framebuffer.
void InsaneRebel1::procPreRendering(byte *renderBitmap) {
	_frameGameOpcodeMask = 0;
	_frameDispatchFlags = 0;

	if (_interactiveVideoActive && _player) {
		const bool usePerspectiveViewport =
			_activeGameOpcode == 0x07 || _activeGameOpcode == 0x08 ||
			_activeGameOpcode == 0x09 || _activeGameOpcode == 0x0A ||
			_activeGameOpcode == 0x0B;
		// Only gameplay handlers that actually execute FUN_224FD own the scrolling
		// 320x200 window inside the 384x242 buffer. Interactive movies with no
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

	// Intro title overlay (RunTwoLineTextSplash) — drawn during intro cinematics
	if (_introTextActive && renderBitmap) {
		int w = _player ? _player->_width : _screenWidth;
		int h = _player ? _player->_height : _screenHeight;
		if (w == 0) w = _screenWidth;
		if (h == 0) h = _screenHeight;
		drawLevelTitleOverlay(renderBitmap, w, w, h, curFrame, maxFrame);
	}

	if (!_interactiveVideoActive || !renderBitmap)
		return;

	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _screenWidth;
	if (height == 0) height = _screenHeight;
	int pitch = width;

	// Set HUD render flag when interactive gameplay is active (0x7600)
	_hudRenderFlag = 0xFF;

	const bool haveFrameGameOpcodes = (_frameGameOpcodeMask != 0);
	const bool asteroidMode = hasFrameGameOpcode(0x0B) ||
		(!haveFrameGameOpcodes && _activeGameOpcode == 0x0B);
	const bool onFootMode = hasFrameGameOpcode(0x19) || hasFrameGameOpcode(0x1A) ||
		(!haveFrameGameOpcodes &&
			(_activeGameOpcode == 0x19 || _activeGameOpcode == 0x1A));
	const bool turretMode = hasFrameGameOpcode(0x08) || hasFrameGameOpcode(0x0A) ||
		(!haveFrameGameOpcodes && (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A));
	const bool flightMode = hasFrameGameOpcode(0x07) || hasFrameGameOpcode(0x09) ||
		(!haveFrameGameOpcodes &&
			(_activeGameOpcode == 0x07 || _activeGameOpcode == 0x09));
	if (asteroidMode) {
		// First-person asteroid/surface handler — opcode 0x0B (FUN_1CDA7).
		updateAsteroidPhysics();

		// DOS 0x0B loops test health after each frontend frame and leave the
		// interactive movie as soon as it drops below zero. Mirror that here so
		// asteroid/surface chapters transition to their retry/death clips like
		// the other gameplay families do.
		if (_health < 0) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}
	} else if (onFootMode) {
		// On-foot handler — opcodes 0x19/0x1A (Level 9 Stormtroopers)
		updateOnFootPhysics();

		if (_health < 0 && _deathTimer < 2) {
			_fireCooldown = _playerFired ? 1 : 0;
			_vm->_smushVideoShouldFinish = true;
			return;
		}

		// Draw character sprite at on-foot position (DrawFobjGlyph with flag 0x80)
		if (_shipBank.numSprites > 0 && _shipDirIndex >= 0 &&
			_shipDirIndex < _shipBank.numSprites) {
			const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];
			int drawX = _onFootCharX + spr.xoffs;
			int drawY = _onFootCharY + spr.yoffs;
			renderSprite(renderBitmap, pitch, width, height, drawX, drawY, spr);
		}
	} else {
		// Dispatch movement path by GAME handler family:
		//   0x08/0x0A -> FUN_1E6A7/FUN_1D79C (turret/cockpit)
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

		// Ship sprite is present in both flight (0x07 family) and 0x08 turret path.
		if (flightMode || turretMode)
			renderShip(renderBitmap, pitch, width, height);
	}

	// GAME handlers in the original update FUN_224FD during the same frame that
	// the new control state is computed. Sync the current frame's viewport window
	// before HUD/screen copy so 0x0B doesn't lag one frame behind the mouse.
	// On-foot mode uses SetCameraOffset(0,0) — no viewport crop.
	if (_player) {
		if (onFootMode || (!asteroidMode && !turretMode && !flightMode)) {
			ra1Player()->_ra1ViewportOffsetX = 0;
			ra1Player()->_ra1ViewportOffsetY = 0;
		} else {
			ra1Player()->_ra1ViewportOffsetX = _perspectiveX;
			ra1Player()->_ra1ViewportOffsetY = _perspectiveY;
		}

		// Screen shake — SetCameraOffset (0x224FD): random [-2,+2] jitter when
		// _screenFlash > 0. Original uses RandScaleByte(5) - 2 for each axis.
		if (_screenFlash > 0) {
			ra1Player()->_ra1ViewportOffsetX += (int16)(_vm->_rnd.getRandomNumber(4) - 2);
			ra1Player()->_ra1ViewportOffsetY += (int16)(_vm->_rnd.getRandomNumber(4) - 2);
		}
	}

	// Assembly dispatch (FUN_1BE1B) only runs the targeting/shot overlay pipeline
	// in handlers 0x09/0x0A/0x0B/0x1A. LVL1 stage-2 works because the stream emits
	// both 0x0A and 0x08 in the same frame, not because 0x08 owns the overlay path.
	const bool hasTargetingPipeline =
		hasFrameGameOpcode(0x09) || hasFrameGameOpcode(0x0A) ||
		hasFrameGameOpcode(0x0B) || hasFrameGameOpcode(0x1A) ||
		(!haveFrameGameOpcodes &&
			(_activeGameOpcode == 0x09 || _activeGameOpcode == 0x0A ||
			 _activeGameOpcode == 0x0B || _activeGameOpcode == 0x1A));
	if (hasTargetingPipeline) {
		const bool flightVariantTargetingMode =
			hasFrameGameOpcode(0x09) ||
			(!haveFrameGameOpcodes && _activeGameOpcode == 0x09);
		const bool turretTargetingMode =
			hasFrameGameOpcode(0x0A) ||
			(!haveFrameGameOpcodes && _activeGameOpcode == 0x0A);
		renderTargetBoxes(renderBitmap, pitch, width, height);
		processShot();
		renderLaserShots(renderBitmap, pitch, width, height);
		// Timer decrement AFTER rendering (original decrements inside the render loop).
		// This ensures timer==5 first frame is rendered with gun barrel offset and lerp=1.
		for (int i = 0; i < kMaxShotSlots; i++) {
			if (_shotSlots[i].timer > 0)
				_shotSlots[i].timer--;
		}
		renderGostSlots(renderBitmap, pitch, width, height);
		renderTargeting(renderBitmap, pitch, width, height);

		// FUN_1D79C (GAME 0x0A) owns the cursor center in stage-2 turret mode.
		// The preceding overlay/shot pass uses the previous frame's cursor; the
		// handler then publishes the next cursor position from the current
		// ship-offset and camera state.
		if (turretTargetingMode) {
			const int16 shipOffsetX = (int16)(_posAccumX >> 8);
			const int16 shipOffsetY = (int16)(_posAccumY >> 8);
			_shipPosX = (int16)(kRA1CenterX + shipOffsetX);
			_shipPosY = (int16)((kRA1CenterY + shipOffsetY - 0x23) - (shipOffsetY >> 3));
		} else if (flightVariantTargetingMode) {
			updateFlightVariantCursor();
		}
	} else {
		// Keep lock/target accumulators quiescent when current handler doesn't
		// execute FUN_1C940/FUN_1CCA0/FUN_1C9CD/FUN_1CB22.
		_targetProximity = 0;
		_prevTargetProx = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
	}

	renderExplosions(renderBitmap, pitch, width, height);

	// Level 8 (Imperial Walkers) — walker-specific state update + UI overlay.
	// In the original, RunLevel8Flow runs the walker logic inline in the per-frame
	// game loop. We call it from procPostRendering when _currentLevel == 7.
	if (_currentLevel == 7) {
		updateLevel8WalkerState();
		renderLevel8Overlay(renderBitmap, pitch, width, height);
	}

	renderHUD(renderBitmap, pitch, width, height);
	_fireCooldown = _playerFired ? 1 : 0;
}

// renderTargetBoxes — FUN_1C940 (0x1C940). Per-target green box overlays.
// Original gates on g_hudDisableFlags (0x75FE) bit 1: skip when set (Hard difficulty).
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

// renderTargeting — FUN_1CB22 (0x1CB22). Targeting/lock-on indicator.
// The original does not draw a hardcoded pixel cross; it renders glyph markers
// whose state depends on _targetProximity.
void InsaneRebel1::renderTargeting(byte *dst, int pitch, int width, int height) {
	static const char kRA1TorpedoIndicator[] = "<d";
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	const int overlayX = ra1OverlayViewOffsetX(this);
	const int overlayY = ra1OverlayViewOffsetY(this);
	if (markerBank.numSprites > 0) {
		// FUN_1CB22 can switch marker sets via DAT_75FF bit 1.
		// Baseline RA1 targeting uses '^' and animation e..h.
		const bool altMarkerSet = (_gameplayFlags75ff & 0x2) != 0;

		// DAT_75FF bit 2 suppresses the fixed lock/readiness overlay.
		if ((_gameplayFlags75ff & 0x4) == 0) {
			// Lock indicator at fixed center positions:
			// FUN_1CB22 draws marker strings at (0xA0,0x78) and (0xA0,0x7E).
			if (_targetProximity > 0) {
				drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x78, ']');
				if (_targetProximity > 1)
					drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x7E, 'a');
			}
		}

		// DAT_75FE bit 2 suppresses the cursor glyph entirely.
		if ((_gameplayFlags75fe & 0x4) == 0) {
			// Pointer glyph at current aim position. Original uses two variants:
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

	// Save previous proximity for next frame
	_prevTargetProx = _targetProximity;
	_targetProximity = 0;
	_lastHitTarget = 0;
}

// renderGostScorePopup — Per-kill score glyph from RenderGostOverlaySlots (0x1C9CD).
// Maps kill score to tech-font glyph and draws it rising upward from the kill position.
void InsaneRebel1::renderGostScorePopup(byte *dst, int pitch, int width, int height,
										int16 centerX, int16 centerY, int16 frame) {
	// Score-to-glyph mapping from original (0x1CA5D-0x1CACB)
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

	// Original: DrawStringEx(buf, colorMap, centerX-4, centerY-frame, 1, 100, 3, scoreText)
	// "<<{glyph}" string selects tech font layer via the << markup
	char scoreText[4] = { '<', '<', glyphChar, '\0' };
	drawFontBankString(dst, pitch, width, height,
					   centerX - 4, centerY - frame, scoreText);
}

// renderGostSlots — FUN_1C9CD (0x1C9CD). Hit explosion animations at target positions.
// Renders explosion sprites from bangBank + per-kill score popup glyphs.
void InsaneRebel1::renderGostSlots(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0) {
		// Warn if there are active GOST slots but no bang sprites to render
		for (int i = 0; i < kMaxGostSlots; i++) {
			if (_gostSlots[i].targetId != 0 && _gostSlots[i].frame < 10) {
				debug(1, "RA1 renderGostSlots: bangBank empty but gost slot %d active (target=%d frame=%d)",
					i, _gostSlots[i].targetId, _gostSlots[i].frame);
				_gostSlots[i].targetId = 0;  // Clear stale slot
			}
		}
		return;
	}

	const int overlayX = ra1OverlayViewOffsetX(this);
	const bool projectGostMarkers = (getEffectiveGameOpcode() == 0x0B);
	for (int i = 0; i < kMaxGostSlots; i++) {
		if (_gostSlots[i].targetId != 0 && _gostSlots[i].frame < 10) {
			int sprIdx = _gostSlots[i].frame;
			if (sprIdx >= _bangBank.numSprites)
				sprIdx = _bangBank.numSprites - 1;

			const RA1Sprite &spr = _bangBank.sprites[sprIdx];
			int16 centerX = _gostSlots[i].posX;
			int16 centerY = _gostSlots[i].posY;
			if (projectGostMarkers)
				centerX = (int16)(centerX - _perspectiveX);

			int drawX = overlayX + centerX - spr.width / 2;
			int drawY = centerY - spr.height / 2;
			renderSprite(dst, pitch, width, height, drawX, drawY, spr);

			// Per-kill score popup glyph — RenderGostOverlaySlots (0x1CA35)
			// Suppressed when DAT_75FF bit 3 is set.
			if ((_gameplayFlags75ff & 8) == 0) {
				renderGostScorePopup(dst, pitch, width, height,
									overlayX + centerX, centerY, _gostSlots[i].frame);
			}

			_gostSlots[i].frame++;
			if (_gostSlots[i].frame >= 10)
				_gostSlots[i].targetId = 0;  // Animation complete
		}
	}
}

// renderLaserShots — FUN_1CDA7/FUN_1D79C/HandleGameOp1A shot visual path.
void InsaneRebel1::renderLaserShots(byte *dst, int pitch, int width, int height) {
	static const char kRA1TorpedoTrailLeft[] = "<<&";
	static const char kRA1TorpedoTrailRight[] = "<<'";
	const bool torpedoMode = (_gameplayFlags75ff & 0x2) != 0;

	if (_laserBank.numSprites <= 0 && !torpedoMode)
		return;

	// DAT_2407 lookup used by FUN_1CDA7/FUN_1D79C for timer 1..5 interpolation.
	// Entry 0 unused.
	static const int kShotLerpByTimer[6] = { 0, 8, 7, 6, 4, 0 };
	// DAT_2413: on-foot lerp table (timer 5 = 1, not 0 like flight mode).
	static const int kOnFootShotLerp[6] = { 0, 8, 7, 6, 4, 1 };
	// DAT_240e: gun barrel X offset indexed by shipDirIndex (for timer==5 first frame).
	static const int16 kOnFootGunBarrelX[20] = {
		4, 0, 8, 8, 7, 6, 4, 1, 0, 0,
		0, -56, -47, -23, -13, 0, 13, 30, 54, 59
	};
	// DAT_2420: gun barrel Y offset indexed by shipDirIndex (for timer==5 first frame).
	static const int16 kOnFootGunBarrelY[20] = {
		0, 0, -56, -47, -23, -13, 0, 13, 30, 54,
		59, -3, -19, -24, -30, -28, -30, -29, -20, -5
	};
	const int spritesPerSet = 5;
	const int overlayX = ra1OverlayViewOffsetX(this);
	const int overlayY = ra1OverlayViewOffsetY(this);
	const int leftStartX = 0;
	const int rightStartX = 0x13F; // 319
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	const bool onFootMode = (effectiveOpcode == 0x19 || effectiveOpcode == 0x1A);
	const bool turretMode = (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A);
	const bool flightVariantMode = (effectiveOpcode == 0x09);
	const int shipBaseX = turretMode ? (kRA1CenterX + (_perspectiveX - 0x20)) : _shipPosX;
	const int shipBaseY = turretMode ? (kRA1CenterY + (_perspectiveY - 0x17))
		: (flightVariantMode ? _shipPosY : (overlayY + _shipPosY));

	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer > 0 && _shotSlots[i].timer <= spritesPerSet) {
			const int timer = _shotSlots[i].timer;
			const int frame = spritesPerSet - timer;
			const int targetX = CLIP<int>(overlayX + getGameplayCursorX(), 0, width - 1);
			const int targetY = CLIP<int>(overlayY + getGameplayCursorY(), 0, height - 1);

			if (onFootMode) {
				// HandleGameOp1A_OnFootVariant: single beam from character to crosshair.
				// Gun barrel offset on first frame (timer==5, dirIndex in aiming range).
				if (timer == 5 && _shipDirIndex > 10 && _shipDirIndex < 20) {
					_shotSlots[i].centerX += kOnFootGunBarrelX[_shipDirIndex];
					_shotSlots[i].centerY += kOnFootGunBarrelY[_shipDirIndex];
				}

				// Skip visible rendering when flags indicate invisible shots (auto-fire).
				if ((_gameplayFlags75fe & 8) != 0)
					continue;

				const int srcX = _shotSlots[i].centerX;
				const int srcY = _shotSlots[i].centerY;
				const int dstX = _shotSlots[i].posX;
				const int dstY = _shotSlots[i].posY;
				const int dir = ra1ShotDirection((int16)srcX, (int16)srcY,
					(int16)dstX, (int16)dstY);
				const int sprIdx = MIN<int>(ABS(dir), _laserBank.numSprites - 1);
				const uint32 flags = 0x83 | ((dir < 0) ? 0x2000 : 0);
				const int onFootLerp = kOnFootShotLerp[timer];
				const int interpX = srcX + (((dstX - srcX) * onFootLerp) >> 3);
				const int interpY = srcY + (((dstY - srcY) * onFootLerp) >> 3);

				renderSpriteWithFlags(dst, pitch, width, height,
					interpX, interpY, _laserBank.sprites[sprIdx], flags);
				continue;
			}

			const int lerp = kShotLerpByTimer[timer];

			if (turretMode) {
				// FUN_1D79C chooses emitters in two ways:
				// - DAT_75E4 == 2: use DAT_75DC..DAT_75E2 fixed offsets
				// - otherwise: table path DAT_244A/DAT_251A keyed by DAT_74D2
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

				const int dir1 = ra1ShotDirection((int16)start1X, (int16)start1Y, (int16)targetX, (int16)targetY);
				const int dir2 = ra1ShotDirection((int16)start2X, (int16)start2Y, (int16)targetX, (int16)targetY);
				const int sprIdx1 = MIN<int>(ABS(dir1), _laserBank.numSprites - 1);
				const int sprIdx2 = MIN<int>(ABS(dir2), _laserBank.numSprites - 1);
				const uint32 flags1 = 0x83 | ((dir1 < 0) ? 0x2000 : 0);
				const uint32 flags2 = 0x83 | ((dir2 < 0) ? 0x2000 : 0);
				const int interp1X = start1X + (((targetX - start1X) * lerp) >> 3);
				const int interp1Y = start1Y + (((targetY - start1Y) * lerp) >> 3);
				const int interp2X = start2X + (((targetX - start2X) * lerp) >> 3);
				const int interp2Y = start2Y + (((targetY - start2Y) * lerp) >> 3);

				renderSpriteWithFlags(dst, pitch, width, height,
					interp1X, interp1Y, _laserBank.sprites[sprIdx1], flags1);
				renderSpriteWithFlags(dst, pitch, width, height,
					interp2X, interp2Y, _laserBank.sprites[sprIdx2], flags2);
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
				const int dir1 = ra1ShotDirection((int16)start1X, (int16)start1Y, (int16)targetX, (int16)targetY);
				const int dir2 = ra1ShotDirection((int16)start2X, (int16)start2Y, (int16)targetX, (int16)targetY);
				const int sprIdx1 = MIN<int>(ABS(dir1), _laserBank.numSprites - 1);
				const int sprIdx2 = MIN<int>(ABS(dir2), _laserBank.numSprites - 1);
				const uint32 flags1 = 0x83 | ((dir1 < 0) ? 0x2000 : 0);
				const uint32 flags2 = 0x83 | ((dir2 < 0) ? 0x2000 : 0);
				const int interp1X = start1X + (((targetX - start1X) * lerp) >> 3);
				const int interp1Y = start1Y + (((targetY - start1Y) * lerp) >> 3);
				const int interp2X = start2X + (((targetX - start2X) * lerp) >> 3);
				const int interp2Y = start2Y + (((targetY - start2Y) * lerp) >> 3);

				if (_currentLevel == 4) {
					debug(1, "RA1 op09 shotRender: frame=%d timer=%d shipBase=(%d,%d) target=(%d,%d) emit1=(%d,%d) emit2=(%d,%d) dir=%d variant=%d mode=%d",
						_gameCounter, timer, shipBaseX, shipBaseY, targetX, targetY,
						start1X, start1Y, start2X, start2Y, _shipDirIndex,
						_shotSlots[i].variant, _flyControlMode);
				}

				renderSpriteWithFlags(dst, pitch, width, height,
					interp1X, interp1Y, _laserBank.sprites[sprIdx1], flags1);
				renderSpriteWithFlags(dst, pitch, width, height,
					interp2X, interp2Y, _laserBank.sprites[sprIdx2], flags2);
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

			// Fallback for non-turret handlers that still run shot overlays.
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
			const int dirLeft = ra1ShotDirection((int16)startLeftX, (int16)leftStartY, (int16)targetX, (int16)targetY);
			const int dirRight = ra1ShotDirection((int16)startRightX, (int16)rightStartY, (int16)targetX, (int16)targetY);
			const int bucketLeft = ra1ShotDirectionBucket(dirLeft);
			const int bucketRight = ra1ShotDirectionBucket(dirRight);
			const int sprIdxLeft = frame + bucketLeft;
			const int sprIdxRight = frame + bucketRight;
			const int interpLeftX = startLeftX + (((targetX - startLeftX) * lerp) >> 3);
			const int interpLeftY = leftStartY + (((targetY - leftStartY) * lerp) >> 3);
			const int interpRightX = startRightX + (((targetX - startRightX) * lerp) >> 3);
			const int interpRightY = rightStartY + (((targetY - rightStartY) * lerp) >> 3);

			if (sprIdxLeft >= 0 && sprIdxLeft < _laserBank.numSprites) {
				renderSpriteWithFlags(dst, pitch, width, height,
					interpLeftX, interpLeftY, _laserBank.sprites[sprIdxLeft], leftFlags);
			}
			if (sprIdxRight >= 0 && sprIdxRight < _laserBank.numSprites) {
				renderSpriteWithFlags(dst, pitch, width, height,
					interpRightX, interpRightY, _laserBank.sprites[sprIdxRight], rightFlags);
			}
		}
	}
}

// Level intro title data (from RunTwoLineTextSplash calls in original binary).
// titleText is drawn at y=10, subtitleText at y=25 (matching original DrawUiString positions).
// revealStartFrame/revealEndFrame control the frame range during which text is visible.
static const struct {
	const char *titleText;      // Top line (chapter number)
	const char *subtitleText;   // Bottom line (level name)
	int16 revealStartFrame;     // Frame at which text begins appearing
	int16 revealEndFrame;       // Frame after which text stops
} kLevelTitles[] = {
	{ "Chapter 1",  "Flight Training",        -80, -30 },  // Original computes totalFrames-0x50..totalFrames-0x1e
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

	// Center horizontally (matching original DrawUiString x_center=0xA0=160)
	int titleW = getFontBankStringWidth(title);
	int subtitleW = getFontBankStringWidth(subtitle);
	int centerX = width / 2;

	// Original positions: title at y=10, subtitle at y=25 (0x19)
	drawFontBankString(dst, pitch, width, height, centerX - titleW / 2, 10, title);
	drawFontBankString(dst, pitch, width, height, centerX - subtitleW / 2, 25, subtitle);
}

// drawFontBankString — FUN_221B7 (0x221B7), partial parity:
// supports '<'/'>' layer markup and layer-2 space handling used by RA1 HUD/targeting strings.
void InsaneRebel1::drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text) {
	if (!text || !dst)
		return;

	// Original FUN_221B7 layer mapping is table-driven at DAT_2D56 (0x406-byte entries).
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

		// FUN_221B7 special-case: when layer==2 and char is space, remap to '!'.
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

// getFontBankStringWidth — Width pre-pass from FUN_221B7 (0x221B7), including '<'/'>' markup.
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

// renderShip — Ship sprite rendering from FUN_1DEB5 (0x1DEB5) at LAB_1e2b2.
// Also used by FUN_1E6A7 (0x1E6A7) turret handler via FUN_20BD3.
void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	// From FUN_1DEB5 LAB_1e2b2: ship drawn when health >= 0 OR deathTimer > 20
	// Hidden during last 20 frames of death sequence (deathTimer 20→0)
	if (_health < 0 && _deathTimer <= 20)
		return;

	const RA1SpriteBank *shipBank = &_shipBank;
	if (_currentLevel == 0 && _flyControlMode == 2 && _shipBankAlt.numSprites > 0)
		shipBank = &_shipBankAlt;

	if (_shipDirIndex < 0 || _shipDirIndex >= shipBank->numSprites)
		return;

	const RA1Sprite &spr = shipBank->sprites[_shipDirIndex];

	// In 0x08/0x0A turret handlers, _shipPos holds targeting/cursor state, while
	// the ship sprite is still anchored from camera perspective + ship drift
	// offsets. Flight handlers already store the ship sprite center in _shipPos.
	int shipScreenX = _shipPosX;
	int shipScreenY = _shipPosY;
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	if (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A) {
		shipScreenX = kRA1CenterX + (_perspectiveX - 0x20);
		shipScreenY = kRA1CenterY + (_perspectiveY - 0x17);
	}

	int drawX = shipScreenX - spr.width / 2;
	int drawY = shipScreenY - spr.height / 2;

	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

// renderExplosions — Explosion sprites from FUN_1DEB5 (0x1DEB5) LAB_1e185 (damage hit)
// and LAB_1e0e3 (death shake). See also FUN_1CCA0 (0x1CCA0) explosion spawner.
void InsaneRebel1::renderExplosions(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	const int overlayX = ra1OverlayViewOffsetX(this);
	const int overlayY = ra1OverlayViewOffsetY(this);
	// In 0x08/0x0A turret handlers, explosion anchors use the ship center, not
	// the targeting cursor stored in _shipPos. Flight handlers already keep the
	// ship center in _shipPos.
	int shipScreenX = overlayX + _shipPosX;
	int shipScreenY = overlayY + _shipPosY;
	const uint16 effectiveOpcode = getEffectiveGameOpcode();
	if (effectiveOpcode == 0x08 || effectiveOpcode == 0x0A) {
		shipScreenX = kRA1CenterX + (_perspectiveX - 0x20);
		shipScreenY = kRA1CenterY + (_perspectiveY - 0x17);
	}

	// --- Death shake explosions (FUN_1DEB5 LAB_1e0e3) ---
	// When dead and deathTimer > 10: random explosion sprites scatter around ship
	if (_health < 0 && _deathTimer > 10) {
		int intensity = _deathTimer - 10;  // 20→1 as timer goes 30→11
		if (intensity > 10)
			intensity = 20 - intensity;     // Triangle: 0→10→0

		// di = intensity * 4 + 1 (vertical scatter range)
		// si = -20 + intensity * 4 (horizontal scatter range, DAT_75d8 is 0)
		int rangeY = intensity * 4 + 1;
		int rangeX = -20 + intensity * 4;
		if (rangeX < 1) rangeX = 1;

		for (int i = 0; i < intensity; i++) {
			// Random sprite from bang bank (FUN_21db0(10))
			int sprIdx = _vm->_rnd.getRandomNumber(_bangBank.numSprites - 1);

			// Random position around ship (matching assembly random scatter)
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

	// --- Damage hit explosion (FUN_1DEB5 LAB_1e185) ---
	// When alive, in cooldown, and bang bank loaded
	if (_health >= 0 && _damageCooldown > 0) {
		// Sprite index = 10 - damageCooldown (frames 0→9 as cooldown 10→1)
		int sprIdx = _bangBank.numSprites - _damageCooldown;
		if (sprIdx < 0 || sprIdx >= _bangBank.numSprites)
			return;

		// Position at ship center (DAT_75d8 is always 0 in RA1)
		int drawX = shipScreenX;
		int drawY = shipScreenY;

		const RA1Sprite &spr = _bangBank.sprites[sprIdx];
		renderSprite(dst, pitch, width, height,
			drawX - spr.width / 2, drawY - spr.height / 2, spr);
	}
}

// renderHUD — FUN_1BBCB (0x1BBCB). Status bar from DISPLAY.NUT with health/lives/score overlays.
void InsaneRebel1::renderHUD(byte *dst, int pitch, int width, int height) {
	// Extra life bonus: every 10,000 points (FUN_1BBCB lines 11-27)
	if (_score / 10000 > _prevScore / 10000) {
		_lives++;
	}
	_prevScore = _score;

	if (_displayBank.numSprites == 0)
		return;

	const RA1Sprite &bar = _displayBank.sprites[0];

	// DISPLAY.NUT sprite is 320×19 at xoffs=0, yoffs=176 in the original game.
	// FUN_224FD (0x224FD) sets the 320x200 window origin inside the 384x242 buffer.
	// FUN_1BBCB (0x1BBCB) HUD coordinates are screen-space, so when we emulate
	// perspective via source-window cropping, anchor HUD at window origin to keep
	// it fixed on-screen.
	int hudOriginX = 0;
	int hudOriginY = 0;
	if (_interactiveVideoActive && _player) {
		hudOriginX = ra1Player()->_ra1ViewportOffsetX;
		hudOriginY = ra1Player()->_ra1ViewportOffsetY;
	}

	int hudX = hudOriginX + bar.xoffs;
	int hudY = hudOriginY + bar.yoffs;

	// DOS RA1 draws the HUD plate through DrawFobjGlyph(..., flags=0x181),
	// which selects the opaque blit path. Keep zero-valued pixels black instead
	// of treating them as transparent.
	if (bar.data && bar.width > 0 && bar.height > 0) {
		int drawX = hudX, drawY = hudY, drawW = bar.width, drawH = bar.height;
		int srcOffX = 0, srcOffY = 0;
		if (drawX < 0) { srcOffX = -drawX; drawW += drawX; drawX = 0; }
		if (drawY < 0) { srcOffY = -drawY; drawH += drawY; drawY = 0; }
		if (drawX + drawW > width) drawW = width - drawX;
		if (drawY + drawH > height) drawH = height - drawY;

		for (int iy = 0; iy < drawH; iy++) {
			const byte *s = bar.data + (srcOffY + iy) * bar.width + srcOffX;
			byte *d = dst + (drawY + iy) * pitch + drawX;
			memcpy(d, s, drawW);
		}

		debug(5, "RA1 HUD: drawn at (%d,%d) size=%dx%d",
			hudX, hudY, bar.width, bar.height);
	}

	// Draw health bar from FUN_1BBCB (0x1BBCB) + FUN_21D66 (0x21D66):
	// fill rect at (0x92-health, 8), width=health, height=5, color=0.
	// This is a black "remaining health" fill over the HUD template.
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
	// Original FUN_1BBCB: FUN_21D66(buf, lives*10+186, 6, 51-lives*10, 9, 0, 320)
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

	// Score: FUN_1BBCB (0x1BBCB) -> FUN_21FAF (0x21FAF) with format at 0x6713: "<<%06ld".
	// Keep the leading "<<" markup so FUN_221B7-equivalent path selects TECH font layer.
	if (_hudFontBank.numSprites > 0 || _techFontBank.numSprites > 0) {
		char scoreStr[24];
		Common::sprintf_s(scoreStr, "<<%06d", MAX<int>(_score, 0));
		drawFontBankString(dst, pitch, width, height, hudX + 273, hudY + 5, scoreStr);
	}

	// Low-health indicator from FUN_1BBCB (0x1BBCB):
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
			// FUN_1BBCB pushes string pointers 0x671b ("<<[") or 0x671f ("<<\\") into FUN_221B7.
			const char *warningStr = aboveCritical ? "<<[" : "<<\\";
			drawFontBankString(dst, pitch, width, height, hudX + 0x49, hudY + 0x07, warningStr);
		}
	}

	_hudDirtyFlag = 0xFF;  // Mark HUD as freshly drawn (0x7601)
}

// Attack window frame tables — RunLevel8Flow (0x18546), data at 0x236D/0x2373/0x2379.
// Each route has up to 3 attack windows. -2 means disabled.
const int16 InsaneRebel1::kWalkerAttackWindow1[3] = { 2588, 2323, 877 };
const int16 InsaneRebel1::kWalkerAttackWindow2[3] = { 1709, 1444, -2 };
const int16 InsaneRebel1::kWalkerAttackWindow3[3] = { 262, -2, -2 };

// Per-route walker damage frame tables — FUN_12fe1/FUN_130c9/FUN_13195.
// type: 0=proximity(41,32), 1=offsetY(15), 2=offsetX(24), 3=proximity(40,31), 4=offsetY(31)
const InsaneRebel1::WalkerDamageFrame InsaneRebel1::kWalkerDamageRoute0[] = {
	{0x00CD, 2}, {0x00EF, 2}, {0x0294, 1}, {0x03A2, 2},
	{0x04BE, 0}, {0x05C9, 2}, {0x076C, 0}, {0x085A, 4}, {0x096F, 2}
};
const int InsaneRebel1::kWalkerDamageRoute0Count = ARRAYSIZE(kWalkerDamageRoute0);

const InsaneRebel1::WalkerDamageFrame InsaneRebel1::kWalkerDamageRoute1[] = {
	{0x0189, 1}, {0x0297, 2}, {0x03B3, 0}, {0x04BE, 4},
	{0x0661, 0}, {0x074F, 4}, {0x0864, 4}
};
const int InsaneRebel1::kWalkerDamageRoute1Count = ARRAYSIZE(kWalkerDamageRoute1);

const InsaneRebel1::WalkerDamageFrame InsaneRebel1::kWalkerDamageRoute2[] = {
	{0x00BB, 0}, {0x01A9, 4}, {0x02BE, 4}
};
const int InsaneRebel1::kWalkerDamageRoute2Count = ARRAYSIZE(kWalkerDamageRoute2);

// updateLevel8WalkerState — Per-frame walker health + attack window + damage logic.
// Called from procPostRendering when _currentLevel == 7.
void InsaneRebel1::updateLevel8WalkerState() {
	// Walker health computation — RunLevel8Flow (0x18634-0x18655)
	if (_walkerHealth >= 11) {
		_walkerHealth = (int16)(100 - (_killCount + (_killCount >> 2)));
	} else if (_walkerHealth > 0 && (_frameCounter & 3) == 0) {
		_walkerHealth--;
	}

	// Walker destroyed — exit interactive video (original loop: `while (sVar6 != 0)`)
	if (_walkerHealth <= 0) {
		_vm->_smushVideoShouldFinish = true;
		return;
	}

	// Per-route damage check — FUN_12fe1/FUN_130c9/FUN_13195
	int route = CLIP(_levelRouteIndex, 0, 2);
	const WalkerDamageFrame *table;
	int tableCount;
	switch (route) {
	case 0:  table = kWalkerDamageRoute0; tableCount = kWalkerDamageRoute0Count; break;
	case 1:  table = kWalkerDamageRoute1; tableCount = kWalkerDamageRoute1Count; break;
	default: table = kWalkerDamageRoute2; tableCount = kWalkerDamageRoute2Count; break;
	}

	uint16 fc = (uint16)_frameCounter;
	for (int i = 0; i < tableCount; i++) {
		if (fc != (uint16)table[i].frame)
			continue;

		int16 dx = _shipPosX - kRA1CenterX;  // distance from center
		int16 dy = _shipPosY - kRA1CenterY;
		if (dx < 0) dx = -dx;
		if (dy < 0) dy = -dy;

		bool hit = false;
		switch (table[i].type) {
		case 0: hit = (dx < 0x29 && dy < 0x20); break;  // proximity(41,32)
		case 1: hit = (dy < 0x0F); break;                // offsetY(15)
		case 2: hit = (dx < 0x18); break;                // offsetX(24)
		case 3: hit = (dx < 0x28 && dy < 0x1F); break;   // proximity(40,31)
		case 4: hit = (dy < 0x1F); break;                // offsetY(31)
		default: break;
		}

		if (hit)
			_damageFlags |= 0x20;
		break;
	}

	// Attack window logic — RunLevel8Flow (0x18778-0x18B4A)
	const int16 *windows[3] = {
		&kWalkerAttackWindow1[route],
		&kWalkerAttackWindow2[route],
		&kWalkerAttackWindow3[route]
	};
	int16 frameNum = (int16)fc;

	// Check if we're inside any attack window (window-100 < frame <= window)
	bool inWindow = false;
	for (int w = 0; w < 3; w++) {
		int16 windowEnd = *windows[w];
		if (windowEnd < 0) continue;
		if (frameNum > windowEnd - 100 && frameNum <= windowEnd) {
			inWindow = true;

			// Reset timer at window start (first frame of window)
			if (frameNum == windowEnd - 99)
				_walkerTimer = 100;
			break;
		}
	}

	if (inWindow && _walkerBranchChoice == 0) {
		_walkerTimer--;

		// Check if we're in the directional phase (last 50 frames)
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
			// Player can choose direction during last 50 frames
			if (_playerFired && _avgInputX == 0) {
				_walkerBranchChoice = (_shipPosX < 0xA0) ? 1 : 2;
			}
		} else {
			// Torpedo sound every 8 frames during targeting phase
			if ((_frameCounter & 7) == 0)
				playSfx(kSfxLockOn, 127, 0);
		}
	}

	// At window boundary: decide route branch — RunLevel8Flow (0x18A7F-0x18B4A)
	// Original: left branches unless at window3, right branches unless at window2.
	for (int w = 0; w < 3; w++) {
		int16 windowEnd = *windows[w];
		if (windowEnd < 0) continue;
		if (fc != (uint16)windowEnd) continue;

		int newRoute = 0;
		bool goLeft = (_walkerBranchChoice == 1) ||
					  (_shipPosX < 0xA0 && _walkerBranchChoice != 2);

		if (goLeft) {
			// Left: branch to route 1 unless at window3 (w==2)
			if (w != 2)
				newRoute = 1;
		} else {
			// Right: branch to route 2 unless at window2 (w==1)
			if (w != 1)
				newRoute = 2;
		}

		if (newRoute != 0) {
			_pendingRouteIndex = newRoute;
			_vm->_smushVideoShouldFinish = true;
		}
		_walkerBranchChoice = 0;
		break;
	}
}

// renderLevel8Overlay — Walker-specific UI from RunLevel8Flow (0x18660-0x18A7E).
// Draws walker health %, attack timer, directional arrows, and target reticle.
// Original draws via DrawStringEx/FormatAndDrawText in screen-space (within viewport).
// We draw into 384x242 buffer, so add viewport offset to convert screen→buffer coords.
void InsaneRebel1::renderLevel8Overlay(byte *dst, int pitch, int width, int height) {
	if (_currentLevel != 7)
		return;

	// Viewport offset: screen-space → buffer-space (same approach as renderHUD)
	int viewX = _player ? ra1Player()->_ra1ViewportOffsetX : _perspectiveX;
	int viewY = _player ? ra1Player()->_ra1ViewportOffsetY : _perspectiveY;

	// Walker health display — "<<WALKER %d%%" at projected (0x61, 0x8D)
	// Blinks when health < 16: only drawn when (frameCounter & 2) != 0
	if (_walkerHealth > 0 && (_walkerHealth >= 16 || (_frameCounter & 2) != 0)) {
		int16 projX = 0x61, projY = 0x8D;
		projectGameplayPoint(projX, projY);
		// Apply 1/4 parallax compensation (original: 0x61 - (projX - 0x61) >> 2)
		projX = (int16)(0x61 - ((projX - 0x61) >> 2));
		projY = (int16)(0x8D - ((projY - 0x8D) >> 2));

		char walkerStr[24];
		Common::sprintf_s(walkerStr, "<<WALKER %d%%%%", (int)_walkerHealth);
		drawFontBankString(dst, pitch, width, height, viewX + projX, viewY + projY, walkerStr);
	}

	// Attack window overlay (timer + arrows/reticle)
	int route = CLIP(_levelRouteIndex, 0, 2);
	const int16 *windows[3] = {
		&kWalkerAttackWindow1[route],
		&kWalkerAttackWindow2[route],
		&kWalkerAttackWindow3[route]
	};
	int16 frameNum = (int16)(uint16)_frameCounter;

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

	// Timer countdown — "<<TIME %d" at projected (0x62, 0x9C)
	{
		int16 projX = 0x62, projY = 0x9C;
		projectGameplayPoint(projX, projY);
		projX = (int16)(0x62 - ((projX - 0x62) >> 2));
		projY = (int16)(0x9C - ((projY - 0x9C) >> 2));

		char timerStr[16];
		Common::sprintf_s(timerStr, "<<TIME %d", (int)_walkerTimer);
		drawFontBankString(dst, pitch, width, height, viewX + projX, viewY + projY, timerStr);
	}

	if (inDirectionalPhase) {
		// Directional arrows — projected from (0,0) with parallax compensation
		int16 projX = 0, projY = 0;
		projectGameplayPoint(projX, projY);
		int16 px = (int16)(-(projX >> 2));
		int16 py = (int16)(-(projY >> 2));

		if (_shipPosX < 0xA0) {
			// Left arrow "<<v"
			drawFontBankString(dst, pitch, width, height,
				viewX + 0xA6 + px, viewY + 0x92 + py, "<<v");
		} else {
			// Right arrow "<<u"
			drawFontBankString(dst, pitch, width, height,
				viewX + 0xA8 - px, viewY + 0x93 - py, "<<u");
		}
	} else {
		// Target reticle — "<<w" at projected (0xA9, 0x9A), blinks on (frame & 4)
		if ((_frameCounter & 4) == 0) {
			int16 projX = 0, projY = 0;
			projectGameplayPoint(projX, projY);
			int16 drawX = (int16)(0xA9 - (projX >> 2));
			int16 drawY = (int16)(0x9A - (projY >> 2));
			drawFontBankString(dst, pitch, width, height,
				viewX + drawX, viewY + drawY, "<<w");
		}
	}
}

// renderSprite — Simplified version of FUN_20BD3 (0x20BD3) glyph/sprite renderer.
// Original dispatches through full codec pipeline; this does flat pixel blit with transparency.
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
