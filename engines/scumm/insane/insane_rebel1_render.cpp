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

	if (_currentLevel == 1) {
		// Level 2: first-person asteroid dodge — no ship sprite, input averaging physics
		updateAsteroidPhysics();
	} else {
		// Level 1 (and others): third-person ship flight with accumulators
		if (_shipBank.numSprites > 0) {
			updateShipPhysics();
			renderShip(renderBitmap, pitch, width, height);
		}
	}

	// Assembly dispatch (FUN_1BE1B) only runs the targeting/shot overlay pipeline
	// in handlers 0x09/0x0A/0x0B/0x1A (FUN_1DABB/FUN_1D79C/FUN_1CDA7/FUN_1D57E).
	const bool hasTargetingPipeline =
		(_activeGameOpcode == 0x09 || _activeGameOpcode == 0x0A ||
		 _activeGameOpcode == 0x0B || _activeGameOpcode == 0x1A);
	if (hasTargetingPipeline) {
		processShot();
		for (int i = 0; i < kMaxShotSlots; i++) {
			if (_shotSlots[i].timer > 0)
				_shotSlots[i].timer--;
		}
		renderLaserShots(renderBitmap, pitch, width, height);
		renderGostSlots(renderBitmap, pitch, width, height);
		renderTargeting(renderBitmap, pitch, width, height);
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
}

// renderTargeting — FUN_1CB22 (0x1CB22). Targeting/lock-on indicator.
// The original does not draw a hardcoded pixel cross; it renders glyph markers
// whose state depends on _targetProximity.
void InsaneRebel1::renderTargeting(byte *dst, int pitch, int width, int height) {
	const RA1SpriteBank &markerBank = (_techFontBank.numSprites > 0) ? _techFontBank : _hudFontBank;
	if (markerBank.numSprites > 0) {
		// FUN_1CB22 can switch marker sets via DAT_75FF bit 1.
		// Baseline RA1 targeting uses '^' and animation e..h.
		const bool altMarkerSet = false;

		// Lock indicator at fixed center positions:
		// FUN_1CB22 draws marker strings at (0xA0,0x78) and (0xA0,0x7E).
		if (_targetProximity > 0) {
			drawBankString(markerBank, dst, pitch, width, height, 0xA0, 0x78, "]");
			if (_targetProximity > 1)
				drawBankString(markerBank, dst, pitch, width, height, 0xA0, 0x7E, "a");
		}

		// Pointer glyph at current aim position. Original uses two variants:
		// default marker ('^' or 'x') and animated lock marker (e..h or y..|).
		char marker[2] = { (char)(altMarkerSet ? 'x' : '^'), '\0' };
		if (_targetProximity > 1) {
			marker[0] = (char)((altMarkerSet ? 'y' : 'e') + (_frameCounter & 3));
		}

		int cursorX = CLIP<int>(_shipPosX, 0, width - 1);
		int cursorY = CLIP<int>(_shipPosY, 0, height - 1);
		int markerW = getBankStringWidth(markerBank, marker);
		drawBankString(markerBank, dst, pitch, width, height, cursorX - markerW / 2, cursorY, marker);
	}

	// Save previous proximity for next frame
	_prevTargetProx = _targetProximity;
	_targetProximity = 0;

	// Reset per-frame target count — FUN_1C940 (0x1C940)
	_prevTargetCount = _targetCount;
	_targetCount = 0;
	_lastHitTarget = 0;
}

// renderGostSlots — FUN_1C9CD (0x1C9CD). Hit explosion animations at target positions.
// Renders explosion sprites from bangBank at each GOST slot's recorded position.
void InsaneRebel1::renderGostSlots(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	for (int i = 0; i < kMaxGostSlots; i++) {
		if (_gostSlots[i].targetId != 0 && _gostSlots[i].frame < 10) {
			int sprIdx = _gostSlots[i].frame;
			if (sprIdx >= _bangBank.numSprites)
				sprIdx = _bangBank.numSprites - 1;

			const RA1Sprite &spr = _bangBank.sprites[sprIdx];
			int drawX = _gostSlots[i].posX - spr.width / 2;
			int drawY = _gostSlots[i].posY - spr.height / 2;
			renderSprite(dst, pitch, width, height, drawX, drawY, spr);

			_gostSlots[i].frame++;
			if (_gostSlots[i].frame >= 10)
				_gostSlots[i].targetId = 0;  // Animation complete
		}
	}
}

// renderLaserShots — FUN_1CDA7/FUN_1D79C shot visual path:
// per active slot, compute left/right direction with FUN_1C794, pick one
// of 3x5 sprite bands, and render interpolated sprite positions via FUN_20BD3.
void InsaneRebel1::renderLaserShots(byte *dst, int pitch, int width, int height) {
	if (_laserBank.numSprites <= 0)
		return;

	// DAT_2407 lookup used by FUN_1CDA7/FUN_1D79C for timer 1..5 interpolation.
	// Entry 0 unused.
	static const int kShotLerpByTimer[6] = { 0, 8, 7, 6, 4, 0 };
	const int spritesPerSet = 5;
	const int leftStartX = 0;
	const int rightStartX = 0x13F; // 319

	for (int i = 0; i < kMaxShotSlots; i++) {
		if (_shotSlots[i].timer > 0 && _shotSlots[i].timer <= spritesPerSet) {
			const int timer = _shotSlots[i].timer;
			const int lerp = kShotLerpByTimer[timer];
			const int frame = spritesPerSet - timer;
			const int targetX = CLIP<int>(_shipPosX, 0, width - 1);
			const int targetY = CLIP<int>(_shipPosY, 0, height - 1);

			int leftStartY = 0x96;
			int rightStartY = 0x96;
			uint32 leftFlags = 0x83;
			uint32 rightFlags = 0x2083;

			// FUN_1CDA7 special mode branch (_DAT_75E4 == 1): toggles emitter origin
			// and flip flags via DAT_2423. Keep behavior for parity when mode is used.
			if (_flyControlMode == 1) {
				if (_shotAlternator != 0) {
					leftFlags = 0x4083;
					leftStartY = 0;
					rightStartY = 0x96;
				} else {
					rightFlags = 0x6083;
					leftStartY = 0x96;
					rightStartY = 0;
				}
				if (timer == 1)
					_shotAlternator = 1 - _shotAlternator;
			}

			const int dirLeft = ra1ShotDirection(targetX, targetY, leftStartX, leftStartY);
			const int dirRight = ra1ShotDirection(rightStartX, targetY, targetX, rightStartY);
			const int bucketLeft = ra1ShotDirectionBucket(dirLeft);
			const int bucketRight = ra1ShotDirectionBucket(dirRight);
			const int sprIdxLeft = frame + bucketLeft;
			const int sprIdxRight = frame + bucketRight;

			const int interpLeftX = leftStartX + (((targetX - leftStartX) * lerp) >> 3);
			const int interpLeftY = leftStartY + (((targetY - leftStartY) * lerp) >> 3);
			const int interpRightX = rightStartX + (((targetX - rightStartX) * lerp) >> 3);
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

// drawFontBankString — Simplified version of FUN_221B7 (0x221B7).
// Original is a multi-font markup-capable renderer; this uses a single font bank.
void InsaneRebel1::drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text) {
	drawBankString(_hudFontBank, dst, pitch, width, height, x, y, text);
}

// getFontBankStringWidth — Pre-pass width calculation from FUN_221B7 (0x221B7).
int InsaneRebel1::getFontBankStringWidth(const char *text) {
	return getBankStringWidth(_hudFontBank, text);
}

// renderShip — Ship sprite rendering from FUN_1DEB5 (0x1DEB5) at LAB_1e2b2.
// Also used by FUN_1E6A7 (0x1E6A7) turret handler via FUN_20BD3.
void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	// From FUN_1DEB5 LAB_1e2b2: ship drawn when health >= 0 OR deathTimer > 20
	// Hidden during last 20 frames of death sequence (deathTimer 20→0)
	if (_health < 0 && _deathTimer <= 20)
		return;

	if (_shipDirIndex < 0 || _shipDirIndex >= _shipBank.numSprites)
		return;

	const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];

	// FUN_1DEB5 draws at (_74B6 + _74BA, _74B8 + _74BC).
	// In the current mapping, _shipPosX/_shipPosY already store that screen position.
	int drawX = _shipPosX - spr.width / 2;
	int drawY = _shipPosY - spr.height / 2;

	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

// renderExplosions — Explosion sprites from FUN_1DEB5 (0x1DEB5) LAB_1e185 (damage hit)
// and LAB_1e0e3 (death shake). See also FUN_1CCA0 (0x1CCA0) explosion spawner.
void InsaneRebel1::renderExplosions(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	// Ship screen center position (matches assembly DAT_74B6+DAT_74BA, DAT_74B8+DAT_74BC).
	int shipScreenX = _shipPosX;
	int shipScreenY = _shipPosY;

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

// renderHUD — FUN_1BBCB (0x1BBCB). Status bar from DISPLAY.NUT with health bar and score.
// Original layout (320-wide): DAMAGE [green bar] | PILOTS [3 icons] | SCORE [number]
void InsaneRebel1::renderHUD(byte *dst, int pitch, int width, int height) {
	if (_displayBank.numSprites == 0)
		return;

	// Extra life bonus: every 10,000 points (FUN_1BBCB lines 11-27)
	if (_score / 10000 > _prevScore / 10000) {
		_lives++;
	}
	_prevScore = _score;

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
		// Asteroid path (opcode 0x0B / FUN_1CDA7) applies Y correction through
		// FUN_223FE coordinate transforms in the original renderer, not as a
		// simple global framebuffer Y window shift.
		if (_activeGameOpcode != 0x0B)
			hudOriginY = _player->_ra1ViewportOffsetY;
	}

	int hudX = hudOriginX + bar.xoffs;
	int hudY = hudOriginY + bar.yoffs;

	// Draw the status bar background with transparency (pixel 0 = transparent)
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
			for (int ix = 0; ix < drawW; ix++) {
				byte px = s[ix];
				if (px != 0)
					d[ix] = px;
			}
		}

		debug(5, "RA1 HUD: drawn at (%d,%d) size=%dx%d",
			hudX, hudY, bar.width, bar.height);
	}

	// Draw health bar from FUN_1BBCB behavior.
	// Original logic uses current health as fill width and computes x as (0x92 - health),
	// so the bar is right-anchored and shrinks from left to right as damage increases.
	{
		int barMaxW = kMaxHealth;
		int barH = 5;
		int healthWidth = CLIP<int16>(_health, 0, kMaxHealth);
		int barX = hudX + (0x92 - healthWidth);
		int barY = hudY + 8;
		int fillW = CLIP(healthWidth, 0, barMaxW);

		// Color based on damage level (matching original thresholds from FUN_1BBCB)
		// Palette indices: 0xD0-0xD7 = greens, 0x60-0x67 = yellows, 0xD8-0xDF = reds
		byte barColor;
		if (_health > _tuning.shot * 2)
			barColor = 0xD5;  // Green (0,192,0) — low damage
		else if (_health > _tuning.wham * 2)
			barColor = 0x63;  // Yellow (255,255,31) — moderate damage
		else
			barColor = 0xDD;  // Red (192,0,0) — critical

		// Flash effect on damage
		if (_screenFlash > 0)
			barColor = 0xFF;  // White flash

		for (int iy = 0; iy < barH && barY + iy < height; iy++) {
			byte *d = dst + (barY + iy) * pitch + barX;
			for (int ix = 0; ix < fillW && barX + ix < width; ix++) {
				d[ix] = barColor;
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

	// Score: 6-digit zero-padded at x=273, y=5 (original format '<<%%06ld' at 0x111,5)
	if (_hudFontBank.numSprites > 0) {
		char scoreStr[16];
		Common::sprintf_s(scoreStr, "%06d", MAX<int>(_score, 0));
		drawFontBankString(dst, pitch, width, height, hudX + 273, hudY + 5, scoreStr);
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
