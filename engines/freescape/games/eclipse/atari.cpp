
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

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 16, 288, 118);

	// On-screen control hotspots (from binary hotspot table at prog $869A)
	// Right-side arrow buttons
	_lookUpArea = Common::Rect(268, 133, 288, 153);
	_lookDownArea = Common::Rect(290, 133, 310, 153);
	_turnLeftArea = Common::Rect(268, 155, 288, 175);
	_turnRightArea = Common::Rect(290, 155, 310, 175);
	_uTurnArea = Common::Rect(268, 177, 288, 197);
	_faceForwardArea = Common::Rect(290, 177, 310, 197);

	// Left-side buttons
	_moveBackwardArea = Common::Rect(9, 133, 29, 153);
	_stepBackwardArea = Common::Rect(9, 155, 29, 175);
	_interactArea = Common::Rect(31, 155, 51, 175);
	_infoDisplayArea = Common::Rect(31, 133, 51, 153);

	// Center/functional areas
	_lanternArea = Common::Rect(57, 138, 75, 168);
	_restArea = Common::Rect(85, 140, 127, 177);

	// Status bar indicators
	_stepSizeArea = Common::Rect(74, 117, 86, 129);
	_heightArea = Common::Rect(222, 117, 234, 129);

	// Save/load (menu screen)
	_saveGameArea = Common::Rect(180, 36, 190, 47);
	_loadGameArea = Common::Rect(180, 50, 190, 60);
}

/*void EclipseEngine::loadAssetsCPCFullGame() {
	Common::File file;

	if (isEclipse2())
		file.open("TE2.BI1");
	else
		file.open("TESCR.SCR");

	if (!file.isOpen())
		error("Failed to open TESCR.SCR/TE2.BI1");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI3");
	else
		file.open("TECON.SCR");

	if (!file.isOpen())
		error("Failed to open TECON.SCR/TE2.BI3");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI2");
	else
		file.open("TECODE.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	if (isEclipse2()) {
		loadFonts(&file, 0x60bc, _font);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x62b4, 16);
	} else {
		loadFonts(&file, 0x6076, _font);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x626e, 16);
	}

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		if (isEclipse2() && it._value->getAreaID() == 1)
			continue;

		if (isEclipse2() && it._value->getAreaID() == _startArea)
			continue;

		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}
	loadColorPalette();
	swapPalette(1);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _currentArea->_underFireBackgroundColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_inkColor;

	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 135, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 135, back, front, surface);

	Common::String scoreStr = Common::String::format("%07d", score);
	drawStringInSurface(scoreStr, 136, 6, back, other, surface, 'Z' - '0' + 1);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	Common::String shieldStr = Common::String::format("%d", shield);
	drawStringInSurface(shieldStr, x, 162, back, other, surface);

	drawStringInSurface(Common::String('0' + _angleRotationIndex - 3), 79, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('3' - _playerStepIndex), 63, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('7' - _playerHeightNumber), 240, 135, back, front, surface, 'Z' - '$' + 1);

	if (_shootingFrames > 0) {
		drawStringInSurface("4", 232, 135, back, front, surface, 'Z' - '$' + 1);
		drawStringInSurface("<", 240, 135, back, front, surface, 'Z' - '$' + 1);
	}
	drawAnalogClock(surface, 90, 172, back, other, front);
	drawIndicator(surface, 45, 4, 12);
	drawEclipseIndicator(surface, 228, 0, front, other);
}*/

// Border palette from CONSOLE.NEO (Atari ST 9-bit $0RGB, scaled to 8-bit).
// Used for font rendering and sprite conversion.
static const byte kBorderPalette[16 * 3] = {
	0, 0, 0,        // 0: $000 black
	145, 72, 0,     // 1: $420 dark brown
	182, 109, 36,   // 2: $531 medium brown
	218, 145, 36,   // 3: $641 golden brown
	255, 182, 36,   // 4: $751 bright gold
	255, 218, 145,  // 5: $764
	218, 218, 218,  // 6: $666
	182, 182, 182,  // 7: $555
	145, 145, 145,  // 8: $444
	109, 109, 109,  // 9: $333
	72, 72, 72,     // 10: $222
	182, 36, 0,     // 11: $510 dark red
	255, 72, 0,     // 12: $720
	255, 109, 0,    // 13: $730
	255, 145, 0,    // 14: $740
	255, 255, 255,  // 15: $777 white
};

// Load raw 4-plane pixel data (no mask) from stream into a CLUT8 surface.
static Graphics::ManagedSurface *loadAtariSTRawSprite(Common::SeekableReadStream *stream,
		int pixelOffset, int cols, int rows) {
	stream->seek(pixelOffset);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(cols * 16, rows, Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			uint16 p0 = stream->readUint16BE();
			uint16 p1 = stream->readUint16BE();
			uint16 p2 = stream->readUint16BE();
			uint16 p3 = stream->readUint16BE();
			for (int bit = 15; bit >= 0; bit--) {
				int x = col * 16 + (15 - bit);
				byte color = ((p0 >> bit) & 1)
				           | (((p1 >> bit) & 1) << 1)
				           | (((p2 >> bit) & 1) << 2)
				           | (((p3 >> bit) & 1) << 3);
				surface->setPixel(x, row, color);
			}
		}
	}
	return surface;
}

static Graphics::ManagedSurface *loadAtariSTSprite(Common::SeekableReadStream *stream,
		int maskOffset, int pixelOffset, int cols, int rows) {
	// Read per-column mask (1 word per column, same for all rows)
	stream->seek(maskOffset);
	Common::Array<uint16> mask(cols);
	for (int c = 0; c < cols; c++)
		mask[c] = stream->readUint16BE();

	// Read pixel data: sequential 4-plane words, cols per row
	stream->seek(pixelOffset);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(cols * 16, rows, Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			uint16 p0 = stream->readUint16BE();
			uint16 p1 = stream->readUint16BE();
			uint16 p2 = stream->readUint16BE();
			uint16 p3 = stream->readUint16BE();
			for (int bit = 15; bit >= 0; bit--) {
				int x = col * 16 + (15 - bit);
				byte color = ((p0 >> bit) & 1)
				           | (((p1 >> bit) & 1) << 1)
				           | (((p2 >> bit) & 1) << 2)
				           | (((p3 >> bit) & 1) << 3);
				surface->setPixel(x, row, color);
			}
		}
	}
	return surface;
}

void EclipseEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	// Border palette colors for the 4-plane font (from CONSOLE.NEO).
	// The Atari ST uses raster interrupts to switch palettes between
	// the 3D viewport (area palette) and the border/UI area (border palette).
	uint32 pal[5];
	pal[0] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	pal[1] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 145, 72, 0);
	pal[2] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 182, 109, 36);
	pal[3] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 218, 145, 36);
	pal[4] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 255, 182, 36);
	_font.setBackground(pal[0]);
	_font.setSecondaryColor(pal[2]);
	_font.setTertiaryColor(pal[3]);
	_font.setQuaternaryColor(pal[4]);

	// Score: Font B at x=$8B(139), y=$06(6) — from $11A66/$11A6E
	int score = _gameStateVars[k8bitVariableScore];
	drawScoreString(score, 139, 6, pal[1], pal[0], surface);

	// Room name / messages: $CE2 at x=$55(85), y=$77(119) — from $11BDC-$11C02
	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 85, 119, pal[1], pal[2], pal[0], surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 85, 119, pal[1], pal[2], pal[0], surface);

	// Step indicator: $CDC at x=$4C(76), y=$77(119)
	// d7 = $42 + (2 - _playerStepIndex) * 2, drawChar chr = d7 + 31
	{
		int d7 = 0x42 + (2 - _playerStepIndex) * 2;
		int chr = d7 + 31;
		_font.drawChar(surface, chr, 76, 119, pal[1]);
	}

	// Height indicator: $CDC at x=$E0(224), y=$77(119)
	// d7 = $48 + (2 - _playerHeightNumber) * 2, drawChar chr = d7 + 31
	{
		int d7 = 0x48 + (2 - _playerHeightNumber) * 2;
		int chr = d7 + 31;
		_font.drawChar(surface, chr, 224, 119, pal[1]);
	}

	// Rotation/shooting indicator: $CDC at x=$E9(233), y=$77(119)
	// d7 = $4E normally, $50 when shooting
	if (_shootingFrames > 0) {
		int chr = 0x50 + 31;
		_font.drawChar(surface, chr, 233, 119, pal[1]);
	} else {
		int chr = 0x4E + 31;
		_font.drawChar(surface, chr, 233, 119, pal[1]);
	}

	// Heart indicator: sprite blit at x=$A0(160), y=$86(134) — from $1E9E/$1EA6
	// 2 frames: 0 = heart visible, 1 = heart hidden/dimmed. Blink cycle.
	if (_eclipseSprites.size() >= 2) {
		int frame = (_ticks / 30) % 2;
		surface->copyRectToSurface(*_eclipseSprites[frame], 160, 134,
			Common::Rect(_eclipseSprites[frame]->w, _eclipseSprites[frame]->h));
	}

	// Eclipse progress indicator: sprite blit at x=$80(128), y=$84(132) — from $11CA0-$11CE0
	// 16 frames showing sun being progressively eclipsed.
	// Frame = countdown-based: frame 0 = full sun, frame 15 = nearly fully eclipsed.
	if (_eclipseProgressSprites.size() >= 16) {
		// Frame 0 = fully eclipsed, frame 15 = full sun
		// progress: 1.0 at game start (full time left), 0.0 when time's up
		int frame = 0;
		if (_initialCountdown > 0 && _countdown > 0) {
			float progress = float(_countdown) / float(_initialCountdown);
			frame = (int)(15.0f * progress);
		}
		frame = CLIP(frame, 0, 15);
		surface->copyRectToSurface(*_eclipseProgressSprites[frame], 128, 132,
			Common::Rect(_eclipseProgressSprites[frame]->w, _eclipseProgressSprites[frame]->h));
	}

	// Ankh indicators at y=$B6(182), x = i*16 + 3 — from $11D88
	// Draw collected ankhs with transparency (skip black/color-0 pixels)
	if (_ankhSprites.size() >= 5) {
		uint32 transparentColor = pal[0]; // black
		Graphics::ManagedSurface *ankh = _ankhSprites[3]; // frame 3 = fully visible
		for (int i = 0; i < _gameStateVars[kVariableEclipseAnkhs] && i < 5; i++) {
			int destX = 3 + 16 * i;
			int destY = 182;
			for (int y = 0; y < ankh->h; y++) {
				for (int x = 0; x < ankh->w; x++) {
					uint32 pixel = ankh->getPixel(x, y);
					if (pixel != transparentColor)
						surface->setPixel(destX + x, destY + y, pixel);
				}
			}
		}
	}

	// Compass at x=$B0(176), y=$97(151) — from sprite header at prog $2097C/$2097E
	if (_compassSprites.size() >= 37) {
		// Normalize yaw to 0-359 first (C++ modulo can return negative), then map to table index
		int deg = ((int)_yaw % 360 + 360) % 360;
		int lookupIdx = deg / 5;
		int needleFrame = _compassLookup[lookupIdx];
		if (needleFrame < (int)_compassSprites.size()) {
			surface->copyRectToSurface(*_compassSprites[needleFrame], 176, 151,
				Common::Rect(_compassSprites[needleFrame]->w, _compassSprites[needleFrame]->h));
		}
	}

	// Lantern switch at x=$30(48), y=$91(145) — only drawn when lantern is ON
	if (_flashlightOn && _lanternSwitchSprites.size() >= 2) {
		surface->copyRectToSurface(*_lanternSwitchSprites[0], 48, 145,
			Common::Rect(_lanternSwitchSprites[0]->w, _lanternSwitchSprites[0]->h));
	}

	// Lantern light animation overlay at (48, 139) — 6 frames, 32x6, toggled with 'T' key
	if (_flashlightOn && _lanternLightSprites.size() >= 6) {
		int lightFrame = (_ticks / 8) % 6;
		surface->copyRectToSurface(*_lanternLightSprites[lightFrame], 48, 139,
			Common::Rect(_lanternLightSprites[lightFrame]->w, _lanternLightSprites[lightFrame]->h));
	}

	// Shooting crosshair overlay at x=$80(128), y=$9F(159)
	if (_shootingFrames > 0 && _shootSprites.size() >= 2) {
		int shootFrame = (_shootingFrames > 5) ? 1 : 0;
		surface->copyRectToSurface(*_shootSprites[shootFrame], 128, 159,
			Common::Rect(_shootSprites[shootFrame]->w, _shootSprites[shootFrame]->h));
	}

	// Analog clock — kept from existing implementation
	uint8 r, g, b;
	uint32 color = _currentArea->_underFireBackgroundColor;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color))
		color = (*_gfx->_colorRemaps)[color];
	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_inkColor;
	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	drawAnalogClock(surface, 90, 172, back, other, front);
}

void EclipseEngine::loadAssetsAtariFullGame() {
	Common::File file;
	file.open("0.tec");
	_title = loadAndConvertNeoImage(&file, 0x17ac);
	file.close();

	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.tec", "0.tec", 0x1774 - 4 * 1024);
	parseAmigaAtariHeader(stream);

	loadMessagesVariableSize(stream, 0x87a6, 28);
	load8bitBinary(stream, 0x2a53c, 16);

	_border = loadAndConvertNeoImage(stream, 0x139c8);
	loadPalettes(stream, 0x2a0fa);
	_sound = loadSoundsFx(stream, 0x3030c, 6);

	// Load TEMUSIC.ST (GEMDOS executable at file offset $11F5A, skip $1C header, TEXT size $11E8)
	static const uint32 kTEMusicOffset = 0x11F5A;
	static const uint32 kGemdosHeaderSize = 0x1C;
	static const uint32 kTEMusicTextSize = 0x11E8;
	stream->seek(kTEMusicOffset + kGemdosHeaderSize);
	_musicData.resize(kTEMusicTextSize);
	stream->read(_musicData.data(), kTEMusicTextSize);
	debug(3, "TE-Atari: Loaded TEMUSIC.ST TEXT segment (%d bytes)", kTEMusicTextSize);

	// UI font (Font A): 4-plane 16-color bordered font at prog $24C3E (file offset $24C5A)
	// 85 characters: ASCII text (32-116) plus special indicator glyphs (65-84)
	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getChars4Plane(stream, 0x24C5A, 85);
	_font = Font(chars);

	// Score font (Font B): 4-plane 10-glyph font at prog $249BE (file offset $249DA)
	// Dedicated score digits 0-9 with different bordered style
	Common::Array<Graphics::ManagedSurface *> scoreChars;
	scoreChars = getChars4Plane(stream, 0x249DA, 11);
	_fontScore = Font(scoreChars);

	// All sprite addresses below are program addresses from the 68K disassembly.
	// The decrypted stream includes a $1C-byte GEMDOS header, so add $1C to
	// convert program addresses to stream offsets.
	static const int kHdr = 0x1C;

	// Heart indicator sprites: 2 frames, 16x13 pixels
	// Descriptor at prog $1D2B8 (1 col × 13 rows), mask at +6, pixels at +8
	// Frame 0 = heart visible, Frame 1 = heart hidden/dimmed
	_eclipseSprites.resize(2);
	_eclipseSprites[0] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + kHdr, 1, 13);
	_eclipseSprites[1] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + 104 + kHdr, 1, 13);

	// Eclipse progress indicator: 16 frames, 16x16 pixels
	// Descriptor at prog $1DA90 (1 col × 16 rows), mask at +6, pixels at +8
	// Frame 0 = full sun, frame 15 = nearly fully eclipsed. Each frame = 128 bytes.
	_eclipseProgressSprites.resize(16);
	for (int i = 0; i < 16; i++)
		_eclipseProgressSprites[i] = loadAtariSTSprite(stream, 0x1DA96 + kHdr, 0x1DA98 + kHdr + i * 128, 1, 16);

	// Ankh indicator: 5 fade-in frames at prog $1B734, 16x15 (1 col, stride 120 bytes)
	// Mask at prog $1B732. Frame 3 = fully visible ankh.
	_ankhSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_ankhSprites[i] = loadAtariSTSprite(stream, 0x1B732 + kHdr, 0x1B734 + kHdr + i * 120, 1, 15);
		_ankhSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Compass background at prog $20986 (32x27, raw 4-plane) and needle at prog $20B36
	// (37 frames, 32x27 each, stride 432 bytes). Pre-composite background + needle.
	{
		Graphics::ManagedSurface *compassBG = loadAtariSTRawSprite(stream, 0x20986 + kHdr, 2, 27);

		// Load compass direction lookup table (72 entries at prog $1542)
		stream->seek(0x1542 + kHdr);
		stream->read(_compassLookup, 72);

		// Find max needle frame index
		int maxFrame = 0;
		for (int i = 0; i < 72; i++)
			if (_compassLookup[i] < 200 && _compassLookup[i] > maxFrame)
				maxFrame = _compassLookup[i];

		int numFrames = maxFrame + 1;
		_compassSprites.resize(numFrames);
		for (int f = 0; f < numFrames; f++) {
			// Load needle frame (raw 4-plane, no mask)
			Graphics::ManagedSurface *needle = loadAtariSTRawSprite(stream,
				0x20B36 + kHdr + f * 432, 2, 27);

			// Composite: copy background, then overlay needle where non-zero
			Graphics::ManagedSurface *composite = new Graphics::ManagedSurface();
			composite->create(32, 27, Graphics::PixelFormat::createFormatCLUT8());
			composite->copyFrom(*compassBG);
			for (int y = 0; y < 27; y++) {
				for (int x = 0; x < 32; x++) {
					byte needlePixel = *(const byte *)needle->getBasePtr(x, y);
					if (needlePixel != 0)
						composite->setPixel(x, y, needlePixel);
				}
			}
			delete needle;

			// Convert to target format
			composite->convertToInPlace(_gfx->_texturePixelFormat,
				const_cast<byte *>(kBorderPalette), 16);
			_compassSprites[f] = composite;
		}
		delete compassBG;
	}

	// Lantern light animation: 6 frames, 32x6, at prog $2026A, stride 96 bytes
	_lanternLightSprites.resize(6);
	for (int i = 0; i < 6; i++) {
		_lanternLightSprites[i] = loadAtariSTRawSprite(stream, 0x2026A + kHdr + i * 96, 2, 6);
		_lanternLightSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Lantern switch: 2 frames, 32x23, at prog $204B4, stride $170 bytes
	// Frame 0 = on, frame 1 = off (toggled with 'T' key)
	_lanternSwitchSprites.resize(2);
	_lanternSwitchSprites[0] = loadAtariSTRawSprite(stream, 0x204B4 + kHdr, 2, 23);
	_lanternSwitchSprites[1] = loadAtariSTRawSprite(stream, 0x204B4 + 0x170 + kHdr, 2, 23);
	for (auto &sprite : _lanternSwitchSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);

	// Sound ON/OFF toggle: 5 frames, 16x11, at prog $20794, stride 96 bytes
	// Drawn at (32, 138) when sound is toggled
	_soundToggleSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_soundToggleSprites[i] = loadAtariSTRawSprite(stream, 0x20794 + kHdr + i * 96, 1, 11);
		_soundToggleSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Water ripple animation: 9 frames, 32x9, at prog $27714, stride 144 bytes
	// Mask at prog $27710. Drawn at (0, 28) in left border strip.
	_waterSprites.resize(9);
	for (int i = 0; i < 9; i++) {
		_waterSprites[i] = loadAtariSTSprite(stream, 0x27710 + kHdr, 0x27714 + kHdr + i * 144, 2, 9);
		_waterSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Shooting crosshair sprites: 2 frames with mask, at prog $1CC26 and $1CDC0
	// Frame 0: 32x25 (2 cols), frame 1: 48x25 (3 cols)
	_shootSprites.resize(2);
	_shootSprites[0] = loadAtariSTSprite(stream, 0x1CC2C + kHdr, 0x1CC30 + kHdr, 2, 25);
	_shootSprites[1] = loadAtariSTSprite(stream, 0x1CDC6 + kHdr, 0x1CDCC + kHdr, 3, 25);
	for (auto &sprite : _shootSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);

	// Convert heart and eclipse progress sprites from CLUT8 to target format using border palette
	for (auto &sprite : _eclipseSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat, const_cast<byte *>(kBorderPalette), 16);
	for (auto &sprite : _eclipseProgressSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat, const_cast<byte *>(kBorderPalette), 16);

	_fontLoaded = true;
}

} // End of namespace Freescape
