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
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

void InsaneRebel1::procPreRendering(byte *renderBitmap) {
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
	renderExplosions(renderBitmap, pitch, width, height);
	renderCrosshair(renderBitmap, pitch, width, height);
	renderHUD(renderBitmap, pitch, width, height);
}

void InsaneRebel1::renderCrosshair(byte *dst, int pitch, int width, int height) {
	int cx = _vm->_mouse.x;
	int cy = _vm->_mouse.y;

	// Palette index 119 = (255,0,0) pure red in L2PLAY.ANM palette.
	// Palette index 15 = (255,255,255) white, used as outline for visibility.
	const byte colorInner = 119;
	const byte colorOutline = 15;
	const int size = 7;  // arm length

	// Helper lambda to draw a pixel with outline
	auto drawPx = [&](int x, int y, byte c) {
		if (x >= 0 && x < width && y >= 0 && y < height)
			dst[y * pitch + x] = c;
	};

	// Draw outline first (1px border around each arm pixel)
	for (int d = -size; d <= size; d++) {
		if (d >= -1 && d <= 1) continue; // skip center area for outline
		// Horizontal arm outline
		drawPx(cx + d, cy - 1, colorOutline);
		drawPx(cx + d, cy + 1, colorOutline);
		// Vertical arm outline
		drawPx(cx - 1, cy + d, colorOutline);
		drawPx(cx + 1, cy + d, colorOutline);
	}
	// Arm endpoints
	drawPx(cx - size - 1, cy, colorOutline);
	drawPx(cx + size + 1, cy, colorOutline);
	drawPx(cx, cy - size - 1, colorOutline);
	drawPx(cx, cy + size + 1, colorOutline);

	// Draw red cross arms
	for (int d = -size; d <= size; d++) {
		if (d == 0) continue; // gap at center
		drawPx(cx + d, cy, colorInner);  // horizontal
		drawPx(cx, cy + d, colorInner);  // vertical
	}
	// Center dot
	drawPx(cx, cy, colorInner);
}

void InsaneRebel1::drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text) {
	if (!dst || !text || _hudFontBank.numSprites <= 0)
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
		if (fontIdx < 0 || fontIdx >= _hudFontBank.numSprites) {
			x += 4;
			continue;
		}

		const RA1Sprite &glyph = _hudFontBank.sprites[fontIdx];
		const int gw = glyph.width;
		const int gh = glyph.height;
		const int gx = x + glyph.xoffs;
		const int gy = y + glyph.yoffs;
		const uint64 glyphPixels = (uint64)gw * (uint64)gh;
		if (!glyph.data || gw <= 0 || gh <= 0 || glyphPixels == 0 || glyphPixels > 0x10000) {
			x += 4;
			continue;
		}
		if (!(_hudFontBank.decodedData && _hudFontBank.decodedSize > 0)) {
			x += 4;
			continue;
		}
		const byte *bankStart = _hudFontBank.decodedData;
		const byte *bankEnd = _hudFontBank.decodedData + _hudFontBank.decodedSize;
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

// getFontBankStringWidth -- Measure pixel width of a string using the HUD font bank.
// Matches the pre-pass width calculation in the original drawString (FUN_221B7).
int InsaneRebel1::getFontBankStringWidth(const char *text) {
	if (!text || _hudFontBank.numSprites <= 0)
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
		if (fontIdx < 0 || fontIdx >= _hudFontBank.numSprites) {
			w += 4;
			continue;
		}
		const RA1Sprite &glyph = _hudFontBank.sprites[fontIdx];
		w += glyph.width > 0 ? glyph.width : 4;
	}
	return w;
}

void InsaneRebel1::renderShip(byte *dst, int pitch, int width, int height) {
	// From FUN_1DEB5 LAB_1e2b2: ship drawn when health >= 0 OR deathTimer > 20
	// Hidden during last 20 frames of death sequence (deathTimer 20→0)
	if (_health < 0 && _deathTimer <= 20)
		return;

	if (_shipDirIndex < 0 || _shipDirIndex >= _shipBank.numSprites)
		return;

	const RA1Sprite &spr = _shipBank.sprites[_shipDirIndex];

	// Position: game coords → screen coords via perspective transform
	// Adapted from RA2's renderHandler7Ship:
	//   shipCenterX = (shipX - center) + perspX + screenCenterX
	int drawX = (_shipPosX - kRA1CenterX) + _perspectiveX + kRA1CenterX - spr.width / 2;
	int drawY = (_shipPosY - kRA1CenterY) + _perspectiveY + kRA1CenterY - spr.height / 2;

	renderSprite(dst, pitch, width, height, drawX, drawY, spr);
}

// Render explosion sprites during damage cooldown and death sequence.
// From FUN_1DEB5 at LAB_1e185 (damage hit) and LAB_1e0e3 (death shake).
void InsaneRebel1::renderExplosions(byte *dst, int pitch, int width, int height) {
	if (_bangBank.numSprites <= 0)
		return;

	// Ship screen center position (matches assembly: DAT_74b6+DAT_74ba, DAT_74b8+DAT_74bc)
	int shipScreenX = (_shipPosX - kRA1CenterX) + _perspectiveX + kRA1CenterX;
	int shipScreenY = (_shipPosY - kRA1CenterY) + _perspectiveY + kRA1CenterY;

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

// Render bottom status bar from DISPLAY.NUT with dynamic damage bar and score.
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
	// Video FOBJs fill the full 384×242 buffer from (0,0), so use sprite offsets directly.
	int hudX = bar.xoffs;
	int hudY = bar.yoffs;

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
