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
#include "graphics/cursorman.h"

#include "audio/mods/protracker.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

byte kAmigaCastlePalette[16][3] = {
	{0x00, 0x00, 0x00},
	{0x44, 0x44, 0x44},
	{0x66, 0x66, 0x66},
	{0x88, 0x88, 0x88},
	{0xaa, 0xaa, 0xaa},
	{0xcc, 0xcc, 0xcc},
	{0x00, 0x00, 0x88},
	{0x66, 0xaa, 0x00},
	{0x88, 0xcc, 0x00},
	{0xcc, 0xee, 0x00},
	{0xee, 0xee, 0x66},
	{0x44, 0x88, 0x00},
	{0xee, 0xaa, 0x00},
	{0xcc, 0x44, 0x00},
	{0x88, 0x44, 0x00},
	{0xee, 0xee, 0xee},
};

byte kAmigaCastleRiddlePalette[16][3] = {
	{0x00, 0x00, 0x00},
	{0x44, 0x44, 0x44},
	{0x66, 0x66, 0x66},
	{0x88, 0x88, 0x88},
	{0xaa, 0xaa, 0xaa},
	{0xcc, 0x44, 0x00},
	{0xee, 0xaa, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0x66, 0x22, 0x00},
	{0xaa, 0x88, 0x00},
	{0xaa, 0x66, 0x00},
	{0x88, 0x44, 0x00},
	{0xee, 0xcc, 0x66},
};

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesVertical(Common::SeekableReadStream *file, int widthInBytes, int height) {
	Graphics::ManagedSurface *surface;
	surface = new Graphics::ManagedSurface();
	surface->create(widthInBytes * 8 / 4, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInBytes * 8 / 4, height), 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 0);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 1);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 2);
	loadFrameFromPlanesInternalVertical(file, surface, widthInBytes / 4, height, 3);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInternalVertical(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, int plane) {
	byte *colors = (byte *)malloc(sizeof(byte) * height * width);
	file->read(colors, height * width);

	for (int i = 0; i < height * width; i++) {
		byte color = colors[i];
		for (int n = 0; n < 8; n++) {
			int y = i / width;
			int x = (i % width) * 8 + (7 - n);

			int bit = ((color >> n) & 0x01) << plane;
			int sample = surface->getPixel(x, y) | bit;
			assert(sample < 16);
			surface->setPixel(x, y, sample);
		}
	}
	free(colors);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInterleaved(Common::SeekableReadStream *file, int widthInWords, int height) {
	int widthInPixels = widthInWords * 16;
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(widthInPixels, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInPixels, height), 0);

	for (int y = 0; y < height; y++) {
		for (int col = 0; col < widthInWords; col++) {
			uint16 planes[4];
			for (int p = 0; p < 4; p++)
				planes[p] = file->readUint16BE();

			for (int bit = 0; bit < 16; bit++) {
				int x = col * 16 + (15 - bit);
				byte color = 0;
				for (int p = 0; p < 4; p++) {
					if (planes[p] & (1 << bit))
						color |= (1 << p);
				}
				surface->setPixel(x, y, color);
			}
		}
	}
	return surface;
}

void CastleEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(&file, 0x8bb2, 178);
	loadRiddles(&file, 0x96c8 - 2 - 19 * 2, 19);


	file.seek(0x11eec);
	Common::Array<Graphics::ManagedSurface *> chars;
	Common::Array<Graphics::ManagedSurface *> charsRiddle;
	for (int i = 0; i < 90; i++) {
		Graphics::ManagedSurface *img = loadFrameFromPlanes(&file, 8, 8);
		Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
		imgRiddle->copyFrom(*img);

		chars.push_back(img);
		chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

		charsRiddle.push_back(imgRiddle);
		charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	}

	_font = Font(chars);
	_font.setCharWidth(9);

	_fontRiddle = Font(charsRiddle);
	_fontRiddle.setCharWidth(9);

	load8bitBinary(&file, 0x162a6, 16);
	for (int i = 0; i < 3; i++) {
		debugC(1, kFreescapeDebugParser, "Continue to parse area index %d at offset %x", _areaMap.size() + i + 1, (int)file.pos());
		Area *newArea = load8bitArea(&file, 16);
		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				error("Repeated area ID: %d", newArea->getAreaID());
		} else {
			error("Invalid area %d?", i);
		}
	}

	loadPalettes(&file, 0x151a6);

	// COLOR15 cycling table (mem $8B78, file 0x8B94): 14 entries of 12-bit Amiga RGB + 0xFFFF end.
	// From assembly: interrupt handler at $12BA cycles $DFF19E through this table every 4 frames.
	file.seek(0x8b94);
	while (true) {
		uint16 val = file.readUint16BE();
		if (val == 0xFFFF) break;
		_gfx->_colorCyclingTable.push_back(val);
	}

	file.seek(0x2be96); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);

	file.seek(0x2cf28 + 0x28 - 0x2 + 0x28);
	_border = loadFrameFromPlanesVertical(&file, 160, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Mountains skybox panorama used by FUN_3E38.
	// Source at memory 0x406A (file 0x4086), 63 words x 22 rows, interleaved 4-plane.
	file.seek(0x4086);
	_background = loadFrameFromPlanesInterleaved(&file, 63, 22);
	_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Menu image used by drawInfoMenu:
	// Assembly at ~0x1AE0 copies from mem 0x350CA in a 14-word x 116-row loop.
	// File offset = mem + 0x1C header => 0x350E6.
	file.seek(0x350e6);
	_menu = loadFrameFromPlanesInterleaved(&file, 14, 116);
	_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Additional 224x54 menu-related block (memory 0x36B9A, file 0x36BB6).
	// Kept as a separate parsed surface for future use.
	file.seek(0x36bb6);
	_menuButtons = loadFrameFromPlanesInterleaved(&file, 14, 54);
	_menuButtons->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x38952); // Spirit meter indicator background (memory 0x38936)
	_spiritsMeterIndicatorBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 5, 10);
	_spiritsMeterIndicatorBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file.seek(0x38ae2); // Spirit meter indicator (memory 0x38AC6)
	_spiritsMeterIndicatorFrame = loadFrameFromPlanesInterleaved(&file, 1, 10);
	_spiritsMeterIndicatorFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Strength weight sprites (file 0x395F2, 1 word x 14 rows x 4 frames)
	file.seek(0x395f2);
	for (int i = 0; i < 4; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 14);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_strenghtWeightsFrames.push_back(frame);
	}

	// Strength background with bar (file 0x397B2, 5 words x 20 rows)
	//file.seek(0x397b2);
	//_strenghtBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 5, 4);
	//_strenghtBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Eye icon sprites (memory 0x3C096, 12 frames, 16x7 each, interleaved 4-plane)
	// Used for strength/compass display at screen (224, 164). Header at 0x3C08E.
	// TODO: load as separate eye icon member, not _keysBorderFrames
	file.seek(0x3c0b2);
	for (int i = 0; i < 12; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 1, 7);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_keysBorderFrames.push_back(frame);
	}

	// Crawl/Walk/Run + Sound indicators (memory 0x3838A, file 0x383A6, 5 frames, 48x12)
	// Header (6 bytes) + mask (3 words = 6 bytes) + graphics.
	// From assembly: frames 0-2 = crawl/walk/run at (96,118), frames 3-4 = sound off/on at (96,103)
	file.seek(0x383a6 + 6 + 6); // skip header + mask
	{
		_menuCrawlIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuCrawlIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuWalkIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuWalkIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuRunIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuRunIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOffIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOffIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_menuFxOnIndicator = loadFrameFromPlanesInterleaved(&file, 3, 12);
		_menuFxOnIndicator->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
	}

	// Mouse pointer from paired sprites at mem $22E/$276 (file 0x24A/0x292).
	// SPR0 at $22E + SPR1 at $276 form the diagonal arrow cursor.
	// Each: 2 control words + 16 data rows (p0,p1 interleaved) + end marker = 72 bytes.
	// SPR0 contributes color bits 0-1, SPR1 contributes bits 2-3 (4-bit combined).
	{
		_cursorW = 16;
		_cursorH = 16;
		_cursorData = new byte[16 * 16];
		memset(_cursorData, 0, 16 * 16);
		// Read SPR0 (bits 0-1)
		file.seek(0x24A + 4); // skip control
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] = c;
			}
		}
		// Overlay SPR1 (bits 2-3)
		file.seek(0x292 + 4); // skip control
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_cursorData[row * 16 + bit] |= (c << 2);
			}
		}
	}

	// Crosshair pointer from paired sprites at mem $19E/$1E6 (file 0x1BA/0x202).
	// Used outside the view area. Same format as diagonal arrow.
	{
		_crosshairData = new byte[16 * 16];
		memset(_crosshairData, 0, 16 * 16);
		file.seek(0x1BA + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] = c;
			}
		}
		file.seek(0x202 + 4);
		for (int row = 0; row < 16; row++) {
			uint16 p0 = file.readUint16BE();
			uint16 p1 = file.readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				byte c = ((p0 >> (15 - bit)) & 1) | (((p1 >> (15 - bit)) & 1) << 1);
				_crosshairData[row * 16 + bit] |= (c << 2);
			}
		}
	}

	// Flag animation (memory 0x3C340, 5 frames, 32x11 each, interleaved 4-plane)
	file.seek(0x3c35c);
	for (int i = 0; i < 5; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(&file, 2, 11);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_flagFrames.push_back(frame);
	}

	// Riddle mask (memory 0x3C6DA, file 0x3C6F6): 16 words, one per 16-pixel column.
	// Applied per-pixel: frame_pixel = (mask_bit == 1) ? frame_pixel : 0.
	// Same mask for every row. Trims the frame edges for proper compositing.
	file.seek(0x3c6f6);
	uint16 riddleMask[16];
	for (int i = 0; i < 16; i++)
		riddleMask[i] = file.readUint16BE();

	// Riddle frames (memory 0x3C6FA: top 20 rows + bg 1 row + bottom 8 rows, 256px wide)
	file.seek(0x3c716);
	_riddleTopFrame = loadFrameFromPlanesInterleaved(&file, 16, 20);
	_riddleBackgroundFrame = loadFrameFromPlanesInterleaved(&file, 16, 1);
	_riddleBottomFrame = loadFrameFromPlanesInterleaved(&file, 16, 8);

	// Apply mask to CLUT8 frames before palette conversion
	Graphics::ManagedSurface *riddleFrames[] = {_riddleTopFrame, _riddleBackgroundFrame, _riddleBottomFrame};
	for (int f = 0; f < 3; f++) {
		Graphics::ManagedSurface *frame = riddleFrames[f];
		for (int y = 0; y < frame->h; y++) {
			for (int x = 0; x < frame->w; x++) {
				int col = x / 16;
				int bit = 15 - (x % 16);
				if (!(riddleMask[col] & (1 << bit)))
					frame->setPixel(x, y, 0);
			}
		}
	}

	_riddleTopFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);

	// Castle gate (game over background frame)
	// Pixel data: 43 rows × 96 bytes (16 columns × 3 words) at file 0x39AE2
	// Mask data: 43 rows × 32 bytes (16 words) at file 0x3AB02
	// FUN_2CCA tiles 24 top rows + 19 bottom rows into a 256×120 gate image
	{
		static const int kTopRows = 24;
		static const int kBottomRows = 19;
		static const int kTotalSrcRows = kTopRows + kBottomRows;
		static const int kColumnsPerRow = 16;
		static const int kPixelBytesPerRow = kColumnsPerRow * 6; // 3 words × 2 bytes
		static const int kMaskBytesPerRow = kColumnsPerRow * 2;  // 1 word × 2 bytes
		static const int kGateWidth = 256;
		static const int kGateHeight = 120;

		byte pixelData[kTotalSrcRows * kPixelBytesPerRow];
		byte maskData[kTotalSrcRows * kMaskBytesPerRow];

		file.seek(0x39AE2);
		file.read(pixelData, sizeof(pixelData));
		file.seek(0x3AB02);
		file.read(maskData, sizeof(maskData));

		uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
		uint32 paletteColors[8];
		for (int i = 0; i < 8; i++)
			paletteColors[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
				kAmigaCastlePalette[i][0], kAmigaCastlePalette[i][1], kAmigaCastlePalette[i][2]);

		_gameOverBackgroundFrame = new Graphics::ManagedSurface();
		_gameOverBackgroundFrame->create(kGateWidth, kGateHeight, _gfx->_texturePixelFormat);
		_gameOverBackgroundFrame->fillRect(Common::Rect(0, 0, kGateWidth, kGateHeight), keyColor);

		// Build row mapping: FUN_2CCA tiling for N=120
		// 5 tail rows from top portion (rows 19-23), then 4×24 full top blocks, then 19 bottom rows
		int destRow = 0;
		// Tail of top portion
		for (int r = kTopRows - 5; r < kTopRows; r++) {
			int srcRow = r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
		// 4 full repetitions of top portion (24 rows each)
		for (int block = 0; block < 4; block++) {
			for (int r = 0; r < kTopRows; r++) {
				int srcRow = r;
				for (int col = 0; col < kColumnsPerRow; col++) {
					uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
					int pOff = srcRow * kPixelBytesPerRow + col * 6;
					uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
					uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
					uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
					for (int bit = 15; bit >= 0; bit--) {
						if (!(mask & (1 << bit))) {
							int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
							_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
						}
					}
				}
				destRow++;
			}
		}
		// Bottom portion (19 rows)
		for (int r = 0; r < kBottomRows; r++) {
			int srcRow = kTopRows + r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
	}

	// Load synthesized sound effects from command table
	// Table at file offset 0x1469E (memory 0x14682), 30 entries
	loadSoundsAmigaDemo(&file, 0x1469E, 30);

	// Load embedded ProTracker module for background music
	// Module is at file offset 0x3D5A6 (memory 0x3D58A), ~86260 bytes
	static const int kModOffset = 0x3D5A6;
	file.seek(0, SEEK_END);
	int fileSize = file.pos();
	int modSize = fileSize - kModOffset;
	if (modSize > 0) {
		file.seek(kModOffset);
		_modData.resize(modSize);
		file.read(_modData.data(), modSize);
	}

	file.close();

	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);
}

void CastleEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	drawLiftingGate(surface);
	drawDroppingGate(surface);

	uint8 r, g, b;
	_gfx->readFromPalette(15, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, black);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, black, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateControl != kFreescapeGameStateEnd) {
			if (ghostInArea())
				drawStringInSurface(_ghostInAreaMessage, 97, 182, front, black, surface);
			else
				drawStringInSurface(_currentArea->_name, 97, 182, front, black, surface);
		}
	}

	// TODO: Draw collected keys - key sprites location in binary still unknown

	// Draw flag animation at (288, 5)
	if (!_flagFrames.empty()) {
		int flagFrameIndex = (_ticks / 10) % _flagFrames.size();
		surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 288, 5,
			Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));
	}

	// Draw energy meter (strength) - background placed at (0, 154) to match border
	drawEnergyMeter(surface, Common::Point(40, 158));

	// Draw spirit meter
	if (_spiritsMeterIndicatorBackgroundFrame)
		surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorBackgroundFrame, 128, 160,
			Common::Rect(0, 0, _spiritsMeterIndicatorBackgroundFrame->w, _spiritsMeterIndicatorBackgroundFrame->h));

	if (_spiritsMeterIndicatorFrame) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 128 + _spiritsMeterPosition, 160,
			Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h), black);
	}
}

} // End of namespace Freescape
