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
	return (rebel1->getActiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveX() : 0;
}

static inline int ra1OverlayViewOffsetY(const InsaneRebel1 *rebel1) {
	if (!rebel1 || !rebel1->isInteractiveVideoActive())
		return 0;

	return (rebel1->getActiveGameOpcode() == 0x0B) ? rebel1->getPerspectiveY() : 0;
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
		// FUN_224FD stores absolute 320x200 window origin in a 384x242 frame:
		// X in [0..0x40], Y in [0..0x2E], centered at (0x20,0x17).
		_player->_ra1ViewportOffsetX = _perspectiveX;
		_player->_ra1ViewportOffsetY = _perspectiveY;
	} else if (_player) {
		_player->_ra1ViewportOffsetX = 0;
		_player->_ra1ViewportOffsetY = 0;
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

	if (!_interactiveVideoActive || !renderBitmap)
		return;

	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _screenWidth;
	if (height == 0) height = _screenHeight;
	int pitch = width;

	const bool haveFrameGameOpcodes = (_frameGameOpcodeMask != 0);
	const bool asteroidMode = hasFrameGameOpcode(0x0B) ||
		(!haveFrameGameOpcodes && _activeGameOpcode == 0x0B);
	if (asteroidMode) {
		// First-person asteroid/surface handler — opcode 0x0B (FUN_1CDA7).
		updateAsteroidPhysics();
	} else {
		const bool turretMode = hasFrameGameOpcode(0x08) || hasFrameGameOpcode(0x0A) ||
			(!haveFrameGameOpcodes && (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A));
		const bool flightMode = hasFrameGameOpcode(0x07) || hasFrameGameOpcode(0x09) ||
			hasFrameGameOpcode(0x19) || hasFrameGameOpcode(0x1A) ||
			(!haveFrameGameOpcodes &&
				(_activeGameOpcode == 0x07 || _activeGameOpcode == 0x09 ||
				 _activeGameOpcode == 0x19 || _activeGameOpcode == 0x1A));

		// Dispatch movement path by GAME handler family:
		//   0x08/0x0A -> FUN_1E6A7/FUN_1D79C (turret/cockpit)
		//   0x07/0x09/0x19/0x1A -> flight-family handlers
		if (turretMode) {
			updateTurretPhysics();
		} else if (flightMode) {
			updateShipPhysics();
		}

		// LVL1 assembly flow exits gameplay loops as soon as health drops below 0
		// (see 0x1626E/0x162EE -> 0x165DD and 0x1640B -> 0x16614), then plays crash video.
		// Do not render the in-engine death overlay in this path; finish immediately.
		if (_currentLevel == 0 && _health < 0) {
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
	if (_player) {
		_player->_ra1ViewportOffsetX = _perspectiveX;
		_player->_ra1ViewportOffsetY = _perspectiveY;
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
		const bool turretTargetingMode =
			hasFrameGameOpcode(0x0A) ||
			(!haveFrameGameOpcodes && _activeGameOpcode == 0x0A);
		renderTargetBoxes(renderBitmap, pitch, width, height);
		processShot();
		for (int i = 0; i < kMaxShotSlots; i++) {
			if (_shotSlots[i].timer > 0)
				_shotSlots[i].timer--;
		}
		renderLaserShots(renderBitmap, pitch, width, height);
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
	renderHUD(renderBitmap, pitch, width, height);
	_fireCooldown = _playerFired ? 1 : 0;
}

// renderTargetBoxes — FUN_1C940 (0x1C940). Per-target green box overlays.
void InsaneRebel1::renderTargetBoxes(byte *dst, int pitch, int width, int height) {
	const int overlayX = ra1OverlayViewOffsetX(this);
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	const bool projectTargetMarkers = (_activeGameOpcode == 0x0B);

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
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	const int overlayX = ra1OverlayViewOffsetX(this);
	const int overlayY = ra1OverlayViewOffsetY(this);
	if (markerBank.numSprites > 0) {
		// FUN_1CB22 can switch marker sets via DAT_75FF bit 1.
		// Baseline RA1 targeting uses '^' and animation e..h.
		const bool altMarkerSet = (_gameplayFlags75ff & 0x2) != 0;

		// Lock indicator at fixed center positions:
		// FUN_1CB22 draws marker strings at (0xA0,0x78) and (0xA0,0x7E).
		if (_targetProximity > 0) {
			drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x78, ']');
			if (_targetProximity > 1)
				drawCenteredBankGlyph(markerBank, dst, pitch, width, height, overlayX + 0xA0, overlayY + 0x7E, 'a');
		}

		// Pointer glyph at current aim position. Original uses two variants:
		// default marker ('^' or 'x') and animated lock marker (e..h or y..|).
		char marker[2] = { (char)(altMarkerSet ? 'x' : '^'), '\0' };
		if (_targetProximity > 1) {
			_targetAnimCounter++;
			marker[0] = (char)((altMarkerSet ? 'y' : 'e') + (_targetAnimCounter & 3));
		}

		int cursorX = CLIP<int>(overlayX + _shipPosX, 0, width - 1);
		int cursorY = CLIP<int>(overlayY + _shipPosY, 0, height - 1);
		drawCenteredBankGlyph(markerBank, dst, pitch, width, height, cursorX, cursorY, marker[0]);
	}

	// Save previous proximity for next frame
	_prevTargetProx = _targetProximity;
	_targetProximity = 0;
	_lastHitTarget = 0;
}

// renderGostSlots — FUN_1C9CD (0x1C9CD). Hit explosion animations at target positions.
// Renders explosion sprites from bangBank at each GOST slot's recorded position.
void InsaneRebel1::renderGostSlots(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	const int overlayX = ra1OverlayViewOffsetX(this);
	const bool projectGostMarkers = (_activeGameOpcode == 0x0B);
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

			_gostSlots[i].frame++;
			if (_gostSlots[i].frame >= 10)
				_gostSlots[i].targetId = 0;  // Animation complete
		}
	}
}

// renderLaserShots — FUN_1CDA7/FUN_1D79C shot visual path.
void InsaneRebel1::renderLaserShots(byte *dst, int pitch, int width, int height) {
	if (_laserBank.numSprites <= 0)
		return;

	// DAT_2407 lookup used by FUN_1CDA7/FUN_1D79C for timer 1..5 interpolation.
	// Entry 0 unused.
	static const int kShotLerpByTimer[6] = { 0, 8, 7, 6, 4, 0 };
	const int spritesPerSet = 5;
	const int overlayX = ra1OverlayViewOffsetX(this);
	const int overlayY = ra1OverlayViewOffsetY(this);
	const int leftStartX = 0;
	const int rightStartX = 0x13F; // 319
	const bool turretMode = (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A);
	const int shipBaseX = turretMode ? (kRA1CenterX + (_perspectiveX - 0x20)) : _shipPosX;
	const int shipBaseY = turretMode ? (kRA1CenterY + (_perspectiveY - 0x17)) : (overlayY + _shipPosY);

	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer > 0 && _shotSlots[i].timer <= spritesPerSet) {
			const int timer = _shotSlots[i].timer;
			const int lerp = kShotLerpByTimer[timer];
			const int frame = spritesPerSet - timer;
			const int targetX = CLIP<int>(overlayX + _shipPosX, 0, width - 1);
			const int targetY = CLIP<int>(overlayY + _shipPosY, 0, height - 1);

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

	// In 0x08/0x0A turret handlers, _shipPos holds pointer center (_74BE/_74C0),
	// while ship sprite center is still (_74B6+_74BA, _74B8+_74BC).
	int shipScreenX = _shipPosX;
	int shipScreenY = _shipPosY;
	if (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A) {
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
	// In 0x08/0x0A turret handlers, explosion anchors use ship center
	// (_74B6+_74BA, _74B8+_74BC), not pointer center (_74BE/_74C0).
	int shipScreenX = overlayX + _shipPosX;
	int shipScreenY = overlayY + _shipPosY;
	if (_activeGameOpcode == 0x08 || _activeGameOpcode == 0x0A) {
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
		hudOriginX = _player->_ra1ViewportOffsetX;
		hudOriginY = _player->_ra1ViewportOffsetY;
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
