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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/macega.h"
#include "graphics/paletteman.h"

#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v8.h"
#include "scumm/util.h"
#include "scumm/charset.h"
#include "scumm/macgui/macgui.h"

namespace Scumm {

uint8 *ScummEngine::getHEPaletteSlot(uint16 palSlot) {
	assertRange(0, palSlot, _numPalettes, "palette");

	if (_game.heversion >= 99) {
		if (palSlot)
			return _hePalettes + palSlot * _hePaletteSlot + 768;
		else
			return _hePalettes + _hePaletteSlot + 768;
	}

	return nullptr;
}

uint16 ScummEngine::get16BitColor(uint8 r, uint8 g, uint8 b) {
	return _outputPixelFormat.RGBToColor(r, g, b);
}

void ScummEngine::resetPalette() {
	static const byte tableC64Palette[] = {
#if 1  // VICE-based palette. See bug #4576
		0x00, 0x00, 0x00,	0xFF, 0xFF, 0xFF,	0x7E, 0x35, 0x2B,	0x6E, 0xB7, 0xC1,
		0x7F, 0x3B, 0xA6,	0x5C, 0xA0, 0x35,	0x33, 0x27, 0x99,	0xCB, 0xD7, 0x65,
		0x85, 0x53, 0x1C,	0x50, 0x3C, 0x00,	0xB4, 0x6B, 0x61,	0x4A, 0x4A, 0x4A,
		0x75, 0x75, 0x75,	0xA3, 0xE7, 0x7C,	0x70, 0x64, 0xD6,	0xA3, 0xA3, 0xA3,
#else
		0x00, 0x00, 0x00,	0xFD, 0xFE, 0xFC,	0xBE, 0x1A, 0x24,	0x30, 0xE6, 0xC6,
		0xB4, 0x1A, 0xE2,	0x1F, 0xD2, 0x1E,	0x21, 0x1B, 0xAE,	0xDF, 0xF6, 0x0A,
		0xB8, 0x41, 0x04,	0x6A, 0x33, 0x04,	0xFE, 0x4A, 0x57,	0x42, 0x45, 0x40,
		0x70, 0x74, 0x6F,	0x59, 0xFE, 0x59,	0x5F, 0x53, 0xFE,	0xA4, 0xA7, 0xA2,
#endif
		// Use 17 color table for v1 games to allow correct color for inventory and
		// sentence line. Original games used some kind of dynamic color table
		// remapping between rooms.
		0x7F, 0x3B, 0xA6
	};

	/** This is Mesen's NTSC palette and the new default palette for ScummVM.
	 *  It was chosen due to the accuracy of Mesen when it comes to emulating a NES
	 */
	static const byte tableNESNTSCPalette[] = {
		0x66, 0x66, 0x66,	0x00, 0x2A, 0x88,	0x14, 0x12, 0xA7,	0x3B, 0x00, 0xA4,
		0x5C, 0x00, 0x7E,	0x6E, 0x00, 0x40,	0x6C, 0x06, 0x00,	0x56, 0x1D, 0x00,
		0x33, 0x35, 0x00,	0x0B, 0x48, 0x00,	0x00, 0x52, 0x00,	0x00, 0x4F, 0x08,
		0x00, 0x40, 0x4D,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xAD, 0xAD, 0xAD,	0x15, 0x5F, 0xD9,	0x42, 0x40, 0xFF,	0x75, 0x27, 0xFE,
		0xA0, 0x1A, 0xCC,	0xB7, 0x1E, 0x7B,	0xB5, 0x31, 0x20,	0x99, 0x4E, 0x00,
		0x6B, 0x6D, 0x00,	0x38, 0x87, 0x00,	0x0C, 0x93, 0x00,	0x00, 0x8F, 0x32,
		0x00, 0x7C, 0x8D,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xFF, 0xFE, 0xFF,	0x64, 0xB0, 0xFF,	0x92, 0x90, 0xFF,	0xC6, 0x76, 0xFF,
		0xF3, 0x6A, 0xFF,	0xFE, 0x6E, 0xCC,	0xFE, 0x81, 0x70,	0xEA, 0x9E, 0x22,
		0xBC, 0xBE, 0x00,	0x88, 0xD8, 0x00,	0x5C, 0xE4, 0x30,	0x45, 0xE0, 0x82,
		0x48, 0xCD, 0xDE,	0x4F, 0x4F, 0x4F,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xFF, 0xFE, 0xFF,	0xC0, 0xDF, 0xFF,	0xD3, 0xD2, 0xFF,	0xE8, 0xC8, 0xFF,
		0xFB, 0xC2, 0xFF,	0xFE, 0xC4, 0xEA,	0xFE, 0xCC, 0xC5,	0xF7, 0xD8, 0xA5,
		0xE4, 0xE5, 0x94,	0xCF, 0xEF, 0x96,	0xBD, 0xF4, 0xAB,	0xB3, 0xF3, 0xCC,
		0xB5, 0xEB, 0xF2,	0xB8, 0xB8, 0xB8,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00
	};

	/** This is an alternative palette based on FirebrandX's NES Classic
	 * palette. This palette is what Nintendo ships on the NES Classic which
	 * makes it somewhat of an "official" digital palette for the NES
	 */
	static const byte tableNESClassicPalette[] = {
		0x60, 0x61, 0x5F,	0x00, 0x00, 0x83,	0x1D, 0x01, 0x95,	0x34, 0x08, 0x75,
		0x51, 0x05, 0x5E,	0x56, 0x00, 0x0F,	0x4C, 0x07, 0x00,	0x37, 0x23, 0x08,
		0x20, 0x3A, 0x0B,	0x0F, 0x4B, 0x0E,	0x19, 0x4C, 0x16,	0x02, 0x42, 0x1E,
		0x02, 0x31, 0x54,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xA9, 0xAA, 0xA8,	0x10, 0x4B, 0xBF,	0x47, 0x12, 0xD8,	0x63, 0x00, 0xCA,
		0x88, 0x00, 0xA9,	0x93, 0x0B, 0x46,	0x8A, 0x2D, 0x04,	0x6F, 0x52, 0x06,
		0x5C, 0x71, 0x14,	0x1B, 0x8D, 0x12,	0x19, 0x95, 0x09,	0x17, 0x84, 0x48,
		0x20, 0x6B, 0x8E,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xFB, 0xFB, 0xFB,	0x66, 0x99, 0xF8,	0x89, 0x74, 0xF9,	0xAB, 0x58, 0xF8,
		0xD5, 0x57, 0xEF,	0xDE, 0x5F, 0xA9,	0xDC, 0x7F, 0x59,	0xC7, 0xA2, 0x24,
		0xA7, 0xBE, 0x03,	0x75, 0xD7, 0x03,	0x60, 0xE3, 0x4F,	0x3C, 0xD6, 0x8D,
		0x56, 0xC9, 0xCC,	0x41, 0x42, 0x40,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,

		0xFB, 0xFB, 0xFB,	0xBE, 0xD4, 0xFA,	0xC9, 0xC7, 0xF9,	0xD7, 0xBE, 0xFA,
		0xE8, 0xB8, 0xF9,	0xF5, 0xBA, 0xE5,	0xF3, 0xCA, 0xC2,	0xDF, 0xCD, 0xA7,
		0xD9, 0xE0, 0x9C,	0xC9, 0xEB, 0x9E,	0xC0, 0xED, 0xB8,	0xB5, 0xF4, 0xC7,
		0xB9, 0xEA, 0xE9,	0xAB, 0xAB, 0xAB,	0x00, 0x00, 0x00,	0x00, 0x00, 0x00,
	};

	static const byte tableAmigaPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xBB,	0x00, 0xBB, 0x00,	0x00, 0xBB, 0xBB,
		0xBB, 0x00, 0x00,	0xBB, 0x00, 0xBB,	0xBB, 0x77, 0x00,	0xBB, 0xBB, 0xBB,
		0x77, 0x77, 0x77,	0x77, 0x77, 0xFF,	0x00, 0xFF, 0x00,	0x00, 0xFF, 0xFF,
		0xFF, 0x88, 0x88,	0xFF, 0x00, 0xFF,	0xFF, 0xFF, 0x00,	0xFF, 0xFF, 0xFF
	};

	static const byte tableAmigaMIPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xAA,	0x00, 0x88, 0x22,	0x00, 0x66, 0x77,
		0xBB, 0x66, 0x66,	0xAA, 0x22, 0xAA,	0x88, 0x55, 0x22,	0x77, 0x77, 0x77,
		0x33, 0x33, 0x33,	0x22, 0x55, 0xDD,	0x22, 0xDD, 0x44,	0x00, 0xCC, 0xFF,
		0xFF, 0x99, 0x99,	0xFF, 0x55, 0xFF,	0xFF, 0xFF, 0x77,	0xFF, 0xFF, 0xFF
	};

	static const byte tableEGAPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xAA,	0x00, 0xAA, 0x00,	0x00, 0xAA, 0xAA,
		0xAA, 0x00, 0x00,	0xAA, 0x00, 0xAA,	0xAA, 0x55, 0x00,	0xAA, 0xAA, 0xAA,
		0x55, 0x55, 0x55,	0x55, 0x55, 0xFF,	0x55, 0xFF, 0x55,	0x55, 0xFF, 0xFF,
		0xFF, 0x55, 0x55,	0xFF, 0x55, 0xFF,	0xFF, 0xFF, 0x55,	0xFF, 0xFF, 0xFF
	};

	static const byte _cgaColors[4][12] = {
		{ 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0x55, 0x00 },
		{ 0x00, 0x00, 0x00, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x55, 0xFF, 0xFF, 0x55 },
		{ 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0x00, 0xAA, 0xAA, 0xAA, 0xAA },
		{ 0x00, 0x00, 0x00, 0x55, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0xFF }
	};

	static const byte tableHercAPalette[] = {
		0x00, 0x00, 0x00,	0xAE, 0x69, 0x38
	};

	static const byte tableHercGPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0xFF, 0x00
	};

	// Palette based on Apple IIgs Technical Notes: IIgs 2523063 Master Color Values
	// Rearranged to match C64 color positions
	static const byte tableApple2gsPalette[] = {
		0x00, 0x00, 0x00,	0xFF, 0xFF, 0xFF,	0xDD, 0x00, 0x33,	0x44, 0xFF, 0x99,
		0xDD, 0x22, 0xDD,	0x00, 0x77, 0x22,	0x00, 0x00, 0x99,	0xFF, 0xFF, 0x00,
		0xFF, 0x66, 0x00,	0x88, 0x55, 0x00,	0xFF, 0x99, 0x88,	0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA,	0x11, 0xDD, 0x00,	0x22, 0x22, 0xFF,	0xAA, 0xAA, 0xAA,

		0x7F, 0x3B, 0xA6
	};

#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	static const byte tableTownsV3Palette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xA0,	0x00, 0xA0, 0x00,	0x00, 0xA0, 0xA0,
		0xA0, 0x00, 0x00,	0xA0, 0x00, 0xA0,	0xA0, 0x60, 0x00,	0xA0, 0xA0, 0xA0,
		0x60, 0x60, 0x60,	0x60, 0x60, 0xE0,	0x00, 0xE0, 0x00,	0x00, 0xE0, 0xE0,
		0xE0, 0x80, 0x80,	0xE0, 0x00, 0xE0,	0xE0, 0xE0, 0x00,	0xE0, 0xE0, 0xE0
	};

	static const byte tableTownsLoomPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xAB,	0x00, 0xAB, 0x00,	0x00, 0xAB, 0xAB,
		0xAB, 0x00, 0x00,	0x69, 0x29, 0x45,	0x8C, 0x4D, 0x14,	0xAB, 0xAB, 0xAB,
		0x57, 0x3F, 0x57,	0x57, 0x57, 0xFF,	0x57, 0xFF, 0x57,	0x57, 0xFF, 0xFF,
		0xFF, 0x57, 0x57,	0xD6, 0x94, 0x40,	0xFF, 0xFF, 0x57,	0xFF, 0xFF, 0xFF
	};
#endif
#endif

	int cgaPalIndex = 1;
	int cgaPalIntensity = 1;
	_enableEGADithering = false;

	if (_renderMode == Common::kRenderHercA) {
		setPaletteFromTable(tableHercAPalette, sizeof(tableHercAPalette) / 3);
	} else if (_renderMode == Common::kRenderHercG) {
		setPaletteFromTable(tableHercGPalette, sizeof(tableHercGPalette) / 3);
	} else if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderCGAComp) {
		setPaletteFromTable(_cgaColors[cgaPalIndex * 2 + cgaPalIntensity], sizeof(_cgaColors[0]) / 3);
		// Cursor palette
		if (_game.version >= 3) {
			setPalColor( 7, 170, 170, 170);
			setPalColor( 8,  85,  85,  85);
			setPalColor(15, 255, 255, 255);
		}
	} else if (_game.version <= 1) {
		if (_game.platform == Common::kPlatformApple2GS) {
			setPaletteFromTable(tableApple2gsPalette, sizeof(tableApple2gsPalette) / 3);
		} else if (_game.platform == Common::kPlatformC64) {
			setPaletteFromTable(tableC64Palette, sizeof(tableC64Palette) / 3);
		} else if (_game.platform == Common::kPlatformNES) {
			if (ConfMan.getBool("mm_nes_classic_palette"))
				setPaletteFromTable(tableNESClassicPalette, sizeof(tableNESClassicPalette) / 3);
			else
				setPaletteFromTable(tableNESNTSCPalette, sizeof(tableNESNTSCPalette) / 3);
		} else if (_renderMode == Common::kRenderCGA_BW) {
			setPalColor(0, 0x00, 0x00, 0x00);
			setPalColor(1, 0xff, 0xff, 0xff);
		} else {
			setPaletteFromTable(tableEGAPalette, sizeof(tableEGAPalette) / 3);
		}
	} else if (_game.features & GF_16COLOR) {
		switch (_renderMode) {
		case Common::kRenderEGA:
			setPaletteFromTable(tableEGAPalette, sizeof(tableEGAPalette) / 3);
			break;

		case Common::kRenderAmiga:
			setPaletteFromTable(tableAmigaPalette, sizeof(tableAmigaPalette) / 3);
			break;

		default:
			if ((_game.platform == Common::kPlatformAmiga) || (_game.platform == Common::kPlatformAtariST))
				setPaletteFromTable(tableAmigaPalette, sizeof(tableAmigaPalette) / 3);
			else
				setPaletteFromTable(tableEGAPalette, sizeof(tableEGAPalette) / 3);
		}
		setDirtyColors(0, 255);
	} else {
		if ((_game.platform == Common::kPlatformAmiga) && _game.version == 4) {
			// if rendermode is set to EGA we use the full palette from the resources
			// else we initialize and then lock down the first 16 colors.
			if (_renderMode != Common::kRenderEGA)
				setPaletteFromTable(tableAmigaMIPalette, sizeof(tableAmigaMIPalette) / 3);
		} else if (_renderMode == Common::kRenderEGA && _supportsEGADithering) {
			setPaletteFromTable(tableEGAPalette, sizeof(tableEGAPalette) / 3);
			_enableEGADithering = true;
			// Set the color tables to sane values (for games that dither the mouse cursor
			// right at the beginning, before these tables get filled normally.
			if (_currentRoom == 0) {
				for (uint16 i = 0; i < 256; ++i)
					_egaColorMap[0][i] = _egaColorMap[1][i] = i & 0x0F;
			}
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
		} else if (_game.platform == Common::kPlatformFMTowns) {
			if (_game.id == GID_INDY4 || _game.id == GID_MONKEY2)
				_townsClearLayerFlag = 0;
#ifdef USE_RGB_COLOR
			else if (_game.id == GID_LOOM)
				towns_setTextPaletteFromPtr(tableTownsLoomPalette);
			else if (_game.version == 3)
				towns_setTextPaletteFromPtr(tableTownsV3Palette);
#endif

			_townsScreen->toggleLayers(_townsActiveLayerFlags);
#endif // DISABLE_TOWNS_DUAL_LAYER_MODE
		}
		setDirtyColors(0, 255);
	}
}

void ScummEngine::setPaletteFromTable(const byte *ptr, int numcolor, int index) {
	for ( ; numcolor > 0; --numcolor, ++index, ptr += 3)
		setPalColor( index, ptr[0], ptr[1], ptr[2]);
}

void ScummEngine::colorPCEToRGB(uint16 color, byte *r, byte *g, byte *b) {
	// 3 bits for each color component: 0xgggrrrbbb
	*b = ((color)      & 0x7) * 0xFF / 0x7;
	*r = ((color >> 3) & 0x7) * 0xFF / 0x7;
	*g = ((color >> 6) & 0x7) * 0xFF / 0x7;
}

void ScummEngine::setPCETextPalette(uint8 color) {
	const uint16 CHARSET_COLORS[16] = {
		0x0000, 0x0096, 0x0140, 0x0145, 0x0059, 0x002D, 0x00A8, 0x016D,
		0x0092, 0x016F, 0x01CD, 0x01DF, 0x00F7, 0x00B6, 0x01B0, 0x01B6
	};

	byte r, g, b;
	colorPCEToRGB(CHARSET_COLORS[color], &r, &g, &b);
	setPalColor(15, r, g, b);
}

void ScummEngine::readPCEPalette(const byte **ptr, byte **dest, int numEntries) {
	byte r, g, b;
	byte msbs = 0;

	for (int i = 0; i < numEntries; ++i) {
		if (i % 8 == 0) {
			// byte contains MSBs (bit 8) for the next 8 bytes
			msbs = *(*ptr)++;
		}
		uint16 msb = (msbs & 0x1) << 8;
		uint16 paletteEntry = msb | *(*ptr)++;
		colorPCEToRGB(paletteEntry, &r, &g, &b);
		*(*dest)++ = r;
		*(*dest)++ = g;
		*(*dest)++ = b;
		msbs >>= 1;
	}
}

void ScummEngine::setPCEPaletteFromPtr(const byte *ptr) {
	byte *dest;
	byte bgSpriteR, bgSpriteG, bgSpriteB;
	byte charsetR, charsetG, charsetB;

	int paletteOffset = *ptr++;
	int numPalettes = *ptr++;

	int firstIndex = paletteOffset * 16;
	int numcolor = numPalettes * 16;

	// the only color over which a background sprite
	// (bit 7 of the sprite attributes) will be visible
	colorPCEToRGB(READ_LE_UINT16(ptr), &bgSpriteR, &bgSpriteG, &bgSpriteB);
	ptr += 2;

	// CHARSET_COLORS[_curTextColor] (unused?)
	colorPCEToRGB(0x01B6, &charsetR, &charsetG, &charsetB);

	dest = _currentPalette + firstIndex * 3;

	for (int i = 0; i < numPalettes; ++i) {
		// entry 0
		*dest++ = bgSpriteR;
		*dest++ = bgSpriteG;
		*dest++ = bgSpriteB;

		// entry 1 - 14
		readPCEPalette(&ptr, &dest, 14);

		// entry 15
		*dest++ = charsetR;
		*dest++ = charsetG;
		*dest++ = charsetB;
	}

	if (_game.features & GF_16BIT_COLOR) {
		for (int i = firstIndex; i < firstIndex + numcolor; ++i)
			_16BitPalette[i] = get16BitColor(_currentPalette[i * 3 + 0], _currentPalette[i * 3 + 1], _currentPalette[i * 3 + 2]);
	}
	setDirtyColors(firstIndex, firstIndex + numcolor - 1);
}

void ScummEngine::setPaletteFromPtr(const byte *ptr, int numcolor) {
	int firstIndex = 0;
	int i;
	byte *dest, r, g, b;

	if (numcolor < 0) {
		if (_game.features & GF_SMALL_HEADER) {
			if (_game.features & GF_OLD256)
				numcolor = READ_LE_UINT16(ptr);
			else
				numcolor = READ_LE_UINT16(ptr) / 3;
			ptr += 2;
		} else {
			numcolor = getResourceDataSize(ptr) / 3;
		}
	}

	assertRange(0, numcolor, 256, "setPaletteFromPtr: numcolor");

	dest = _currentPalette;

	// Test for Amiga Monkey Island and EGA Mode unset, if true then skip the first 16 colors.
	if ((_game.platform == Common::kPlatformAmiga) && _game.version == 4 && _renderMode != Common::kRenderEGA) {
		firstIndex = 16;
		dest += 3 * 16;
		ptr += 3 * 16;
	}

	for (i = firstIndex; i < numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;

		// Only SCUMM 5/6 games use 6/6/6 style palettes
		if (_game.version >= 5 && _game.version <= 6) {
			if ((_game.heversion <= 74 && i < 15) || i == 15 || r < 252 || g < 252 || b < 252) {
				*dest++ = r;
				*dest++ = g;
				*dest++ = b;
			} else {
				dest += 3;
			}
		} else {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		}
	}

	if (_game.heversion >= 90 || _game.version == 8) {
		memcpy(_darkenPalette, _currentPalette, 768);
	}

	setDirtyColors(firstIndex, numcolor - 1);

	if (_game.id == GID_SAMNMAX && !enhancementEnabled(kEnhMinorBugFixes))
		VAR(77) = 0;
}

void ScummEngine::setAmigaPaletteFromPtr(const byte *ptr) {
	memcpy(_currentPalette, ptr, 768);

	for (int i = 0; i < 32; ++i) {
		_shadowPalette[i] = i;
		_colorUsedByCycle[i] = 0;
	}

	amigaPaletteFindFirstUsedColor();

	for (int i = 0; i < 64; ++i) {
		_amigaPalette[i * 3 + 0] = _currentPalette[(i + 16) * 3 + 0] >> 4;
		_amigaPalette[i * 3 + 1] = _currentPalette[(i + 16) * 3 + 1] >> 4;
		_amigaPalette[i * 3 + 2] = _currentPalette[(i + 16) * 3 + 2] >> 4;
	}

	for (int i = 0; i < 256; ++i) {
		if (i < 16 || i >= _amigaFirstUsedColor) {
			mapRoomPalette(i);
			mapVerbPalette(i);
		} else {
			int idx = (i - 16) & 31;
			// We adjust our verb palette map from [0, 31] to [32, 63], since unlike
			// the original we set up the verb palette at colors [32, 63].
			// The original instead used two different palettes for the verb virtual
			// screen and all the rest.
			if (idx != 17) {
				_roomPalette[i] = idx;
				_verbPalette[i] = idx + 32;
			} else {
				// In all my tests it seems the colors 0 and 32 in
				// _amigaPalette are in fact black. Thus 17 is probably black.
				// For the room map the color 17 is 33 (17+16), for the verb
				// map it is 65 (17+32).
				_roomPalette[i] = 0;
				_verbPalette[i] = 32;
			}
		}
	}

	setDirtyColors(0, 255);
}

void ScummEngine::setV1ColorTable(int renderMode) {
	static const byte v1ColorMaps[5][16] = {
		// C-64: Just leave everything the way it is
		{	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F },
		// ZAK: EGA, Tandy, MCGA, CGA Composite
		{	0x00, 0x0F, 0x04, 0x03, 0x05, 0x02, 0x01, 0x0E,	0x0C, 0x06, 0x0D, 0x08, 0x07, 0x0A, 0x09, 0x07 },
		// ZAK: CGA, CGA B&W, Hercules
		{	0x00, 0x0F, 0x08, 0x05, 0x0A, 0x05, 0x01, 0x0D,	0x0A, 0x02, 0x0A, 0x0C, 0x0F, 0x0A, 0x05, 0x0C },
		// MM: EGA, Tandy, MCGA, CGA Composite
		{	0x00, 0x0F, 0x04, 0x03, 0x05, 0x02, 0x01, 0x0E, 0x0C, 0x06, 0x0C, 0x08, 0x07, 0x0A, 0x09, 0x08 },
		// MM: CGA, CGA B&W, Hercules
		{	0x00, 0x0F, 0x08, 0x05, 0x0A, 0x05, 0x01, 0x0D, 0x0A, 0x02, 0x0A, 0x0C, 0x0F, 0x0A, 0x05, 0x0C }
	};

	int tbl = (_game.platform == Common::kPlatformC64 || _game.platform == Common::kPlatformApple2GS) ? 0 : (_game.id == GID_ZAK ? 1 : 3);
	if (renderMode == Common::kRenderHercA || renderMode == Common::kRenderHercG || renderMode == Common::kRenderCGA || renderMode == Common::kRenderCGA_BW)
		++tbl;

	assert(_gdi);
	_gdi->setRenderModeColorMap(v1ColorMaps[tbl]);
}

void ScummEngine::amigaPaletteFindFirstUsedColor() {
	for (_amigaFirstUsedColor = 80; _amigaFirstUsedColor < 256; ++_amigaFirstUsedColor) {
		// We look for the first used color here. If all color components are
		// >= 252 the color seems to be unused. Check remapPaletteColor for
		// the same behavior.
		if (_currentPalette[_amigaFirstUsedColor * 3 + 0] <= 251
		    || _currentPalette[_amigaFirstUsedColor * 3 + 1] <= 251
		    || _currentPalette[_amigaFirstUsedColor * 3 + 2] <= 251)
			break;
	}
}

void ScummEngine::mapRoomPalette(int idx) {
	// For Color 33 (which is in fact 17+16) see the special case in
	// setAmigaPaletteFromPtr.
	if (idx >= 16 && idx < 48 && idx != 33)
		_roomPalette[idx] = idx - 16;
	else
		_roomPalette[idx] = remapRoomPaletteColor(_currentPalette[idx * 3 + 0] >> 4,
		                                          _currentPalette[idx * 3 + 1] >> 4,
		                                          _currentPalette[idx * 3 + 2] >> 4);
}

static const uint8 amigaWeightTable[16] = {
	  0,   1,   4,   9,  16,  25,  36,  49,
	 64,  81, 100, 121, 144, 169, 196, 225
};

int ScummEngine::remapRoomPaletteColor(int r, int g, int b) {
	int idx = 0;
	uint16 minValue = 0xFFFF;

	const byte *pal = _amigaPalette;
	const byte *cycle = _colorUsedByCycle;

	for (int i = 0; i < 32; ++i) {
		if (!*cycle++ && i != 17) {
			int rD = ABS<int>(*pal++ - r);
			int gD = ABS<int>(*pal++ - g);
			int bD = ABS<int>(*pal++ - b);

			const uint16 weight = amigaWeightTable[rD] + amigaWeightTable[gD] + amigaWeightTable[bD];
			if (weight < minValue) {
				minValue = weight;
				idx = i;
			}
		} else {
			pal += 3;
		}
	}

	return idx;
}

void ScummEngine::mapVerbPalette(int idx) {
	// We adjust our verb palette map from [0, 31] to [32, 63], since unlike
	// the original we set up the verb palette at colors [32, 63].
	// The original instead used two different palettes for the verb virtual
	// screen and all the rest.
	// For Color 65 (which is in fact 17+32) see the special case in
	// setAmigaPaletteFromPtr.
	if (idx >= 48 && idx < 80 && idx != 65)
		_verbPalette[idx] = idx - 16;
	else
		_verbPalette[idx] = remapVerbPaletteColor(_currentPalette[idx * 3 + 0] >> 4,
		                                          _currentPalette[idx * 3 + 1] >> 4,
		                                          _currentPalette[idx * 3 + 2] >> 4) + 32;
}

int ScummEngine::remapVerbPaletteColor(int r, int g, int b) {
	int idx = 0;
	uint16 minValue = 0xFFFF;

	const byte *pal = _amigaPalette + 32 * 3;

	for (int i = 0; i < 32; ++i) {
		if (i != 17) {
			int rD = ABS<int>(*pal++ - r);
			int gD = ABS<int>(*pal++ - g);
			int bD = ABS<int>(*pal++ - b);

			const uint16 weight = amigaWeightTable[rD] + amigaWeightTable[gD] + amigaWeightTable[bD];
			if (weight < minValue) {
				minValue = weight;
				idx = i;
			}
		} else {
			pal += 3;
		}
	}

	return idx;
}

void ScummEngine::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;

	_paletteChangedCounter++; // HE99+
}

void ScummEngine::initCycl(const byte *ptr) {
	int j;
	ColorCycle *cycl;

	memset(_colorCycle, 0, sizeof(_colorCycle));

	if (_game.features & GF_SMALL_HEADER) {
		// Code verified from LOOM CD and MI1 VGA disasms, which
		// are the only executables which contain said code at all

		cycl = _colorCycle;
		for (j = 0; j < 16; ++j, ++cycl) {
			cycl->delay = READ_BE_UINT16(ptr);
			ptr += 2;

			cycl->counter = 0;
			cycl->start = *ptr++;
			cycl->end = *ptr++;

			if (cycl->delay && cycl->delay != 0x0AAA)
				cycl->counter = cycl->start;
		}
	} else {
		memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
		while ((j = *ptr++) != 0) {
			if (j < 1 || j > 16) {
				error("Invalid color cycle index %d", j);
			}
			cycl = &_colorCycle[j - 1];

			ptr += 2;
			cycl->counter = 0;
			cycl->delay = 16384 / READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->flags = READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->start = *ptr++;
			cycl->end = *ptr++;

			if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
				cycl->start = CLIP(cycl->start - 16, 0, 31);
				cycl->end = CLIP(cycl->end - 16, 0, 31);
			}

			for (int i = cycl->start; i <= cycl->end; ++i) {
				_colorUsedByCycle[i] = 1;
			}
		}
	}

	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		for (int i = 0; i < 256; ++i) {
			if (i >= 16 && i < _amigaFirstUsedColor)
				continue;

			if (_colorUsedByCycle[_roomPalette[i]])
				mapRoomPalette(i);
		}
	}
}

void ScummEngine::stopCycle(int i) {
	ColorCycle *cycl;

	assertRange(0, i, 16, "stopCycle: cycle");
	if (i != 0) {
		_colorCycle[i - 1].delay = 0;
		if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
			cycl = &_colorCycle[i - 1];
			for (int j = cycl->start; j <= cycl->end && j < 32; ++j) {
				_shadowPalette[j] = j;
				_colorUsedByCycle[j] = 0;
			}
		}
		return;
	}

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		cycl->delay = 0;
		if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
			for (int j = cycl->start; j <= cycl->end && j < 32; ++j) {
				_shadowPalette[j] = j;
				_colorUsedByCycle[j] = 0;
			}
		}
	}
}

/**
 * Cycle the colors in the given palette in the interval [cycleStart, cycleEnd]
 * either one step forward or backward.
 */
static void doCyclePalette(byte *palette, int cycleStart, int cycleEnd, int size, bool forward) {
	byte *start = palette + cycleStart * size;
	byte *end = palette + cycleEnd * size;
	int num = cycleEnd - cycleStart;
	byte tmp[6];

	assert(size <= 6);

	if (forward) {
		memmove(tmp, end, size);
		memmove(start + size, start, num * size);
		memmove(start, tmp, size);
	} else {
		memmove(tmp, start, size);
		memmove(start, start + size, num * size);
		memmove(end, tmp, size);
	}
}

/**
 * Adjust an 'indirect' color palette for the color cycling performed on its
 * master palette. An indirect palette is a palette which contains indices
 * pointing into another palette - it provides a level of indirection to map
 * palette colors to other colors. Now when the target palette is cycled, the
 * indirect palette suddenly point at the wrong color(s). This function takes
 * care of adjusting an indirect palette by searching through it and replacing
 * all indices that are in the cycle range by the new (cycled) index.
 *
 * Finally, the palette entries still have to be cycled normally.
 */
static void doCycleIndirectPalette(byte *palette, int cycleStart, int cycleEnd, bool forward) {
	int num = cycleEnd - cycleStart + 1;
	int i;
	int offset = forward ? 1 : num - 1;

	for (i = 0; i < 256; i++) {
		if (cycleStart <= palette[i] && palette[i] <= cycleEnd) {
			palette[i] = (palette[i] - cycleStart + offset) % num + cycleStart;
		}
	}

	doCyclePalette(palette, cycleStart, cycleEnd, 1, forward);
}


void ScummEngine::cyclePalette() {
	ColorCycle *cycl;
	int valueToAdd;
	int i, j;

	if (_game.features & GF_SMALL_HEADER) {
		// Code verified from LOOM CD and MI1 VGA disasms, which
		// are the only executables which contain said code at all

		for (i = 0; i < 16; i++) {
			if (_colorCycle[i].counter) {
				_colorCycle[i].counter++;
				if (_colorCycle[i].counter > _colorCycle[i].end)
					_colorCycle[i].counter = _colorCycle[i].start;

				byte start = _colorCycle[i].start;
				byte end = _colorCycle[i].end;

				if (start <= end) {
					byte cycleVal = _colorCycle[i].counter;
					for (j = start; j <= end; j++) {
						_shadowPalette[j] = cycleVal--;
						if (cycleVal < start)
							cycleVal = end;
					}
				}

				setDirtyColors(_colorCycle[i].start, _colorCycle[i].end);
				moveMemInPalRes(_colorCycle[i].start, _colorCycle[i].end, 0);
			}
		}
		return;
	}

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns && !(_townsPaletteFlags & 1))
		return;
#endif

	if (_enableEGADithering)
		return;

	valueToAdd = VAR(VAR_TIMER);
	if (valueToAdd < VAR(VAR_TIMER_NEXT))
		valueToAdd = VAR(VAR_TIMER_NEXT);

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		if (!cycl->delay || cycl->start > cycl->end)
			continue;
		cycl->counter += valueToAdd;
		if (cycl->counter >= cycl->delay) {
			cycl->counter %= cycl->delay;

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags & 2);

			if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
				doCyclePalette(_shadowPalette, cycl->start, cycl->end, 1, !(cycl->flags & 2));
			} else {
				doCyclePalette(_currentPalette, cycl->start, cycl->end, 3, !(cycl->flags & 2));

				if (_shadowPalette) {
					if (_game.version >= 7) {
						for (j = 0; j < NUM_SHADOW_PALETTE; j++)
							doCycleIndirectPalette(_shadowPalette + j * 256, cycl->start, cycl->end, !(cycl->flags & 2));
					} else {
						doCycleIndirectPalette(_shadowPalette, cycl->start, cycl->end, !(cycl->flags & 2));
					}
				}
			}
		}
	}
}

/**
 * Perform color cycling on the palManipulate data, too, otherwise
 * color cycling will be disturbed by the palette fade.
 */
void ScummEngine::moveMemInPalRes(int start, int end, byte direction) {
	if (!_palManipCounter)
		return;

	doCyclePalette(_palManipPalette, start, end, 3, !direction);
	doCyclePalette(_palManipIntermediatePal, start, end, 6, !direction);
}

void ScummEngine::palManipulateInit(int resID, int start, int end, int time) {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns && !(_townsPaletteFlags & 1))
		return;
#endif

	// This function is actually a nullsub in Indy4 Amiga.
	// It might very well be a nullsub in other Amiga games, but for now I
	// limit this to Indy4 Amiga, since that is the only game I can check.
	// UPDATE: Disable it for EGA mode, too. The original does not handle this. For
	// Indy4 they just disabled the EGA mode completely. I have tried to make an EGA
	// implementation, but it looks glitchy and unpleasant.
	if ((_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) || _enableEGADithering)
		return;

	byte *string1 = getStringAddress(resID);
	byte *string2 = getStringAddress(resID + 1);
	byte *string3 = getStringAddress(resID + 2);
	if (!string1 || !string2 || !string3) {
		error("palManipulateInit(%d,%d,%d,%d): Cannot obtain string resources %d, %d and %d",
				resID, start, end, time, resID, resID + 1, resID + 2);
		return;
	}

	string1 += start;
	string2 += start;
	string3 += start;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	byte *pal = _currentPalette + start * 3;
	byte *target = _palManipPalette + start * 3;
	uint16 *between = (uint16 *)(_palManipIntermediatePal + start * 6);

	for (int i = start; i < end; ++i) {
		*target++ = *string1++;
		*target++ = *string2++;
		*target++ = *string3++;
		*between++ = ((uint16) *pal++) << 8;
		*between++ = ((uint16) *pal++) << 8;
		*between++ = ((uint16) *pal++) << 8;
	}

	_palManipCounter = time;
}

void ScummEngine_v6::palManipulateInit(int resID, int start, int end, int time) {
	const byte *new_pal;

	if (_enableEGADithering)
		return;

	new_pal = getPalettePtr(resID, _roomResource);

	new_pal += start*3;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	byte *pal = _currentPalette + start * 3;
	byte *target = _palManipPalette + start * 3;
	uint16 *between = (uint16 *)(_palManipIntermediatePal + start * 6);

	for (int i = start; i < end; ++i) {
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*between++ = ((uint16) *pal++) << 8;
		*between++ = ((uint16) *pal++) << 8;
		*between++ = ((uint16) *pal++) << 8;
	}

	_palManipCounter = time;
}


void ScummEngine::palManipulate() {
	if (!_palManipCounter || !_palManipPalette || !_palManipIntermediatePal)
		return;

	byte *target = _palManipPalette + _palManipStart * 3;
	byte *pal = _currentPalette + _palManipStart * 3;
	uint16 *between = (uint16 *)(_palManipIntermediatePal + _palManipStart * 6);

	for (int i = _palManipStart; i < _palManipEnd; ++i) {
		int j;
		j = (*between += ((*target++ << 8) - *between) / _palManipCounter);
		*pal++ = j >> 8;
		between++;
		j = (*between += ((*target++ << 8) - *between) / _palManipCounter);
		*pal++ = j >> 8;
		between++;
		j = (*between += ((*target++ << 8) - *between) / _palManipCounter);
		*pal++ = j >> 8;
		between++;
	}
	setDirtyColors(_palManipStart, _palManipEnd);
	_palManipCounter--;
}

void ScummEngine::setShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	byte *table;
	int i;
	byte *curpal;

	if (slot < 0 || slot >= NUM_SHADOW_PALETTE)
		error("setShadowPalette: invalid slot %d", slot);

	if (startColor < 0 || startColor > 255 || endColor < 0 || endColor > 255 || endColor < startColor)
		error("setShadowPalette: invalid range from %d to %d", startColor, endColor);

	table = _shadowPalette + slot * 256;
	for (i = 0; i < 256; i++)
		table[i] = i;

	table += startColor;
	curpal = _currentPalette + startColor * 3;
	for (i = startColor; i <= endColor; i++) {
		*table++ = remapPaletteColor((curpal[0] * redScale) >> 8,
									 (curpal[1] * greenScale) >> 8,
									 (curpal[2] * blueScale) >> 8,
									 -1);
		curpal += 3;
	}
}

static inline uint colorWeight(int red, int green, int blue) {
	return 3 * red * red + 6 * green * green + 2 * blue * blue;
}

void ScummEngine::setShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor, int start, int end) {
	// This function is actually a nullsub in Indy4 Amiga.
	// It might very well be a nullsub in other Amiga games, but for now I
	// limit this to Indy4 Amiga, since that is the only game I can check.
	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4)
		return;

	const byte *basepal = getPalettePtr(_curPalIndex, _roomResource);
	const byte *compareptr;
	const byte *pal = basepal + start * 3;
	byte *table = _shadowPalette + start;
	int i;

	// This is an implementation based on the original games code.
	//
	// The four known rooms where setShadowPalette is used in atlantis are:
	//
	// 1) FOA Room 53: subway departing Knossos for Atlantis.
	// 2) FOA Room 48: subway crashing into the Atlantis entrance area
	// 3) FOA Room 82: boat/sub shadows while diving near Thera
	// 4) FOA Room 23: the big machine room inside Atlantis
	//
	// There seems to be no explanation for why this function is called
	// from within Room 23 (the big machine), as it has no shadow effects
	// and thus doesn't result in any visual differences.

	if (_game.id == GID_SAMNMAX) {
		for (i = 0; i < 256; i++)
			_shadowPalette[i] = i;
	}

	for (i = start; i < end; i++) {
		int r = (int)((pal[0] >> 2) * redScale) >> 8;
		int g = (int)((pal[1] >> 2) * greenScale) >> 8;
		int b = (int)((pal[2] >> 2) * blueScale) >> 8;
		pal += 3;

		uint8 bestitem = 0;
		uint bestsum = 32000;

		compareptr = basepal + startColor * 3;
		for (int j = startColor; j <= endColor; j++, compareptr += 3) {
			int ar = compareptr[0] >> 2;
			int ag = compareptr[1] >> 2;
			int ab = compareptr[2] >> 2;

			uint sum = ABS(ar - r) + ABS(ag - g) + ABS(ab - b);

			if (sum < bestsum) {
				bestsum = sum;
				bestitem = j;
			}
		}
		*table++ = bestitem;
	}
}

byte egaFindBestMatch(int r, int g, int b) {
	// This is almost like the normal EGA palette, but a bit different
	static const byte matchPalette[] = {
		0x00, 0x00, 0x00,	0x00, 0x00, 0xAB,	0x00, 0xAB, 0x00,	0x00, 0xAB, 0xAB,
		0xAB, 0x00, 0x00,	0xAB, 0x00, 0xAB,	0xAB, 0x57, 0x00,	0xAB, 0xAB, 0xAB,
		0x57, 0x57, 0x57,	0x57, 0x57, 0xFF,	0x57, 0xFF, 0x57,	0x57, 0xFF, 0xFF,
		0xFF, 0x57, 0x57,	0xFF, 0x57, 0xFF,	0xFF, 0xFF, 0x57,	0xFF, 0xFF, 0xFF
	};

	uint32 best = (uint32)-1;
	byte res = 0;

	for (uint16 i = 0; i < 256; ++i) {
		const byte *c1 = &matchPalette[(i >> 4) * 3];
		const byte *c2 = &matchPalette[(i & 0x0F) * 3];

		int dr = ((c1[0] + c2[0]) >> 1) - r;
		int dg = ((c1[1] + c2[1]) >> 1) - g;
		int db = ((c1[2] + c2[2]) >> 1) - b;

		uint32 sum = dr * dr + dg * dg + db * db;
		if (sum < best) {
			best = sum;
			res = i;
		}
	}

	return res;
}

void ScummEngine::darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		startColor = CLIP(startColor, 0, 255);

		//bool remappedVerbColors = false;
		bool remappedRoomColors = false;
		bool cycleFlag = (blueScale >= 250 && greenScale >= 250 && redScale >= 250);

		const byte *palptr = getPalettePtr(_curPalIndex, _roomResource) + startColor * 3;

		for (int i = startColor; i <= endColor; ++i) {
			if (i > 16 && i < 48) {
				if (cycleFlag)
					_colorUsedByCycle[i - 16] &= ~2;
				else
					_colorUsedByCycle[i - 16] |=  2;
			}

			_currentPalette[i * 3 + 0] = (*palptr++ * redScale) >> 8;
			_currentPalette[i * 3 + 1] = (*palptr++ * greenScale) >> 8;
			_currentPalette[i * 3 + 2] = (*palptr++ * blueScale) >> 8;
		}

		for (int i = startColor; i <= endColor; ++i) {
			// Colors 33 (17+16) and 65 (17+32) will never get changed. For
			// more information about these check setAmigaPaletteFromPtr.
			if (i >= 16 && i < 48 && i != 33) {
				remappedRoomColors = true;
				_amigaPalette[(i - 16) * 3 + 0] = _currentPalette[i * 3 + 0] >> 4;
				_amigaPalette[(i - 16) * 3 + 1] = _currentPalette[i * 3 + 1] >> 4;
				_amigaPalette[(i - 16) * 3 + 2] = _currentPalette[i * 3 + 2] >> 4;
			} else if (i >= 48 && i < 80 && i != 65) {
				//remappedVerbColors = true;
				_amigaPalette[(i - 16) * 3 + 0] = _currentPalette[i * 3 + 0] >> 4;
				_amigaPalette[(i - 16) * 3 + 1] = _currentPalette[i * 3 + 1] >> 4;
				_amigaPalette[(i - 16) * 3 + 2] = _currentPalette[i * 3 + 2] >> 4;
			}
		}

		for (int i = 0; i < 256; ++i) {
			if (i >= 16 && i < _amigaFirstUsedColor)
				continue;

			bool inRange = (startColor <= i && i <= endColor);
			int idx = _roomPalette[i] + 16;
			bool mappedInRange = (startColor <= idx && idx <= endColor);

			if (inRange != mappedInRange || (remappedRoomColors && cycleFlag))
				mapRoomPalette(i);
		}

		setDirtyColors(startColor, endColor);
	} else if (_enableEGADithering) {
		if (redScale == 0 || greenScale == 0 || blueScale == 0) {
			for (int i = startColor; i <= endColor; ++i)
				_egaColorMap[0][i] = _egaColorMap[1][i] = 0;

		} else if (redScale == 0xFF || greenScale == 0xFF || blueScale == 0xFF) {
			if (!_EPAL_offs) {
				// We can support the EGA mode for games that aren't supposed to have it, like this.
				// Might be glitchy, though...
				const byte *p = getPalettePtr(_curPalIndex, _roomResource) + startColor * 3;
				for (int i = startColor; i <= endColor; ++i) {
					byte col = egaFindBestMatch(p[0], p[1], p[2]);
					_egaColorMap[0][i] = col & 0x0F;
					_egaColorMap[1][i] = col >> 4;
					p += 3;
				}
			} else {
				const byte *p = getResourceAddress(rtRoom, _roomResource) + _EPAL_offs + startColor;
				for (int i = startColor; i <= endColor; ++i) {
					_egaColorMap[0][i] = *p & 0x0F;
					_egaColorMap[1][i] = *p++ >> 4;
				}
			}
		} else {
			const byte *p = getPalettePtr(_curPalIndex, _roomResource) + startColor * 3;
			for (int i = startColor; i <= endColor; ++i) {
				int vr = MIN<int>(*p++ * redScale / 0xFF, 0xFF);
				int vg = MIN<int>(*p++ * greenScale / 0xFF, 0xFF);
				int vb = MIN<int>(*p++ * blueScale / 0xFF, 0xFF);
				byte col = egaFindBestMatch(vr, vg, vb);
				_egaColorMap[0][i] = col & 0x0F;
				_egaColorMap[1][i] = col >> 4;
			}
		}

		_virtscr[kMainVirtScreen].setDirtyRange(0, _virtscr[kMainVirtScreen].h);
		_virtscr[kVerbVirtScreen].setDirtyRange(0, _virtscr[kVerbVirtScreen].h);

	} else {
		int max;
		if (_game.version >= 5 && _game.version <= 6 && _game.heversion <= 60) {
			max = 252;
		} else {
			max = 255;
		}

		if (startColor <= endColor) {
			const byte *cptr;
			const byte *palptr;
			int color, idx, j;

			if (_game.heversion >= 90 || _game.version == 8) {
				palptr = _darkenPalette;
			} else {
				palptr = getPalettePtr(_curPalIndex, _roomResource);
			}
			for (j = startColor; j <= endColor; j++) {
				idx = (_game.heversion == 70) ? _HEV7ActorPalette[j] : j;
				cptr = palptr + idx * 3;

				if (_game.heversion == 70)
					setDirtyColors(idx, idx);

				color = *cptr++;
				color = color * redScale / 0xFF;
				if (color > max)
					color = max;
				_currentPalette[idx * 3 + 0] = color;

				color = *cptr++;
				color = color * greenScale / 0xFF;
				if (color > max)
					color = max;
				_currentPalette[idx * 3 + 1] = color;

				color = *cptr++;
				color = color * blueScale / 0xFF;
				if (color > max)
					color = max;
				_currentPalette[idx * 3 + 2] = color;

				if (_game.features & GF_16BIT_COLOR)
					_16BitPalette[idx] = get16BitColor(_currentPalette[idx * 3 + 0], _currentPalette[idx * 3 + 1], _currentPalette[idx * 3 + 2]);
			}
			if (_game.heversion != 70)
				setDirtyColors(startColor, endColor);

			// Original noir mode behavior
			if (_game.id == GID_SAMNMAX && !enhancementEnabled(kEnhMinorBugFixes) && VAR(77) == 1)
				applyGrayscaleToPaletteRange(startColor, endColor);
		}
	}
}

#ifdef ENABLE_SCUMM_7_8
static int HSL2RGBHelper(int n1, int n2, int hue) {
	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		return n1 + (n2 - n1) * hue / 60;
	if (hue < 180)
		return n2;
	if (hue < 240)
		return n1 + (n2 - n1) * (240 - hue) / 60;
	return n1;
}

/**
 * This function scales the HSL (Hue, Saturation and Lightness)
 * components of the palette colors. It's used in CMI when Guybrush
 * walks from the beach towards the swamp.
 */
void ScummEngine_v8::desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor) {

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;

		cptr = _darkenPalette + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			int R = *cptr++;
			int G = *cptr++;
			int B = *cptr++;

			// RGB to HLS (Foley and VanDam)

			const int min = MIN(R, MIN(G, B));
			const int max = MAX(R, MAX(G, B));
			const int diff = (max - min);
			const int sum = (max + min);

			if (diff != 0) {
				int H, S, L;

				if (sum <= 255)
					S = 255 * diff / sum;
				else
					S = 255 * diff / (255 * 2 - sum);

				if (R == max)
					H = 60 * (G - B) / diff;
				else if (G == max)
					H = 120 + 60 * (B - R) / diff;
				else
					H = 240 + 60 * (R - G) / diff;

				if (H < 0)
					H = H + 360;

				// Scale the result

				H = (H * hueScale) / 255;
				S = (S * satScale) / 255;
				L = (sum * lightScale) / 255;

				// HLS to RGB (Foley and VanDam)

				int m1, m2;
				if (L <= 255)
					m2 = L * (255 + S) / (255 * 2);
				else
					m2 = L * (255 - S) / (255 * 2) + S;

				m1 = L - m2;

				R = HSL2RGBHelper(m1, m2, H + 120);
				G = HSL2RGBHelper(m1, m2, H);
				B = HSL2RGBHelper(m1, m2, H - 120);
			} else {
				// Maximal color = minimal color -> R=G=B -> it's a grayscale.
				R = G = B = (R * lightScale) / 255;
			}

			*cur++ = R;
			*cur++ = G;
			*cur++ = B;
		}

		setDirtyColors(startColor, endColor);
	}
}
#endif


int ScummEngine::remapPaletteColor(int r, int g, int b, int threshold) {
	byte *pal;
	int ar, ag, ab, i;
	uint sum, bestsum, bestitem = 0;

	int startColor = (_game.version == 8) ? 24 : 1;

	if (_game.heversion >= 99)
		pal = _hePalettes + 1024 + startColor * 3;
	else
		pal = _currentPalette + startColor * 3;

	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;

	bestsum = 0x7FFFFFFF;

	r &= ~3;
	g &= ~3;
	b &= ~3;

	for (i = startColor; i < 255; i++, pal += 3) {
		if (_game.version == 7 && _colorUsedByCycle[i])
			continue;

		ar = pal[0] & ~3;
		ag = pal[1] & ~3;
		ab = pal[2] & ~3;
		if (ar == r && ag == g && ab == b)
			return i;

		sum = colorWeight(ar - r, ag - g, ab - b);

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != -1 && bestsum > colorWeight(threshold, threshold, threshold)) {
		// Best match exceeded threshold. Try to find an unused palette entry and
		// use it for our purpose.
		pal = _currentPalette + (256 - 2) * 3;
		for (i = 254; i > 48; i--, pal -= 3) {
			if (pal[0] >= 252 && pal[1] >= 252 && pal[2] >= 252) {
				setPalColor(i, r, g, b);
				return i;
			}
		}
	}

	return bestitem;
}

void ScummEngine::swapPalColors(int a, int b) {
	byte *ap, *bp;
	byte t;

	if ((uint) a >= 256 || (uint) b >= 256)
		error("swapPalColors: invalid values, %d, %d", a, b);

	ap = &_currentPalette[a * 3];
	bp = &_currentPalette[b * 3];

	t = ap[0];
	ap[0] = bp[0];
	bp[0] = t;
	t = ap[1];
	ap[1] = bp[1];
	bp[1] = t;
	t = ap[2];
	ap[2] = bp[2];
	bp[2] = t;

	if (_game.features & GF_16BIT_COLOR) {
		_16BitPalette[a] = get16BitColor(ap[0], ap[1], ap[2]);
		_16BitPalette[b] = get16BitColor(bp[0], bp[1], bp[2]);
	}

	setDirtyColors(a, a);
	setDirtyColors(b, b);
}

void ScummEngine::copyPalColor(int dst, int src) {
	byte *dp, *sp;

	if ((uint) dst >= 256 || (uint) src >= 256)
		error("copyPalColor: invalid values, %d, %d", dst, src);

	dp = &_currentPalette[dst * 3];
	sp = &_currentPalette[src * 3];

	dp[0] = sp[0];
	dp[1] = sp[1];
	dp[2] = sp[2];

	if (_game.features & GF_16BIT_COLOR)
		_16BitPalette[dst] = get16BitColor(sp[0], sp[1], sp[2]);

	setDirtyColors(dst, dst);

	// Original noir mode behavior
	if (_game.id == GID_SAMNMAX && !enhancementEnabled(kEnhMinorBugFixes) && VAR(77) == 1)
		applyGrayscaleToPaletteRange(src, src);
}

void ScummEngine::setPalColor(int idx, int r, int g, int b) {
	if (_enableEGADithering) {
		byte col = egaFindBestMatch(r, g, b);
		_egaColorMap[0][idx] = col & 0x0F;
		_egaColorMap[1][idx] = col >> 4;
		_virtscr[kMainVirtScreen].setDirtyRange(0, _virtscr[kMainVirtScreen].h);
		_virtscr[kVerbVirtScreen].setDirtyRange(0, _virtscr[kVerbVirtScreen].h);
		return;
	}

	if (_game.heversion == 70)
		idx = _HEV7ActorPalette[idx];

	_currentPalette[idx * 3 + 0] = r;
	_currentPalette[idx * 3 + 1] = g;
	_currentPalette[idx * 3 + 2] = b;
	if (_game.version == 8) {
		_darkenPalette[idx * 3 + 0] = r;
		_darkenPalette[idx * 3 + 1] = g;
		_darkenPalette[idx * 3 + 2] = b;
	}

	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		// Colors 33 (17+16) and 65 (17+32) will never get changed. For
		// more information about these check setAmigaPaletteFromPtr.
		if (idx < 16 || idx >= _amigaFirstUsedColor) {
			mapRoomPalette(idx);
			mapVerbPalette(idx);
		} else if (idx >= 16 && idx < 48 && idx != 33) {
			_amigaPalette[(idx - 16) * 3 + 0] = _currentPalette[idx * 3 + 0] >> 4;
			_amigaPalette[(idx - 16) * 3 + 1] = _currentPalette[idx * 3 + 1] >> 4;
			_amigaPalette[(idx - 16) * 3 + 2] = _currentPalette[idx * 3 + 2] >> 4;

			for (int i = 0; i < 256; ++i) {
				if (i >= 16 && i < _amigaFirstUsedColor)
					continue;

				if (idx - 16 == _roomPalette[i])
					mapRoomPalette(i);
			}
		} else if (idx >= 48 && idx < 80 && idx != 65) {
			_amigaPalette[(idx - 16) * 3 + 0] = _currentPalette[idx * 3 + 0] >> 4;
			_amigaPalette[(idx - 16) * 3 + 1] = _currentPalette[idx * 3 + 1] >> 4;
			_amigaPalette[(idx - 16) * 3 + 2] = _currentPalette[idx * 3 + 2] >> 4;

			for (int i = 0; i < 256; ++i) {
				if (i >= 16 && i < _amigaFirstUsedColor)
					continue;

				// We do - 16 instead of - 48 like the original, since our
				// verb palette map is using [32, 63] instead of [0, 31].
				if (idx - 16 == _verbPalette[i])
					mapVerbPalette(i);
			}
		}
	}

	if (_game.features & GF_16BIT_COLOR)
		_16BitPalette[idx] = get16BitColor(r, g, b);

	setDirtyColors(idx, idx);

	// Original noir mode behavior
	if (_game.id == GID_SAMNMAX && !enhancementEnabled(kEnhMinorBugFixes) && VAR(77) == 1)
		applyGrayscaleToPaletteRange(idx, idx);
}

void ScummEngine::setCurrentPalette(int palindex) {
	const byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr(_curPalIndex, _roomResource);
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
		setPCEPaletteFromPtr(pals);
#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	} else if (_game.platform == Common::kPlatformFMTowns) {
		towns_setPaletteFromPtr(pals);
#endif
#endif
	} else if (_game.id == GID_INDY4 && _game.platform == Common::kPlatformAmiga) {
		setAmigaPaletteFromPtr(pals);
	} else if (_enableEGADithering) {
		if (!_EPAL_offs) {
			// We can support the EGA mode for games that aren't supposed to have it, like this.
			// Might be glitchy, though...
			const byte *p = getPalettePtr(_curPalIndex, _roomResource);
			for (int i = 0; i < 256; ++i) {
				byte col = egaFindBestMatch(p[0], p[1], p[2]);
				_egaColorMap[0][i] = col & 0x0F;
				_egaColorMap[1][i] = col >> 4;
				p += 3;
			}
		} else {
			pals = getResourceAddress(rtRoom, _roomResource) + _EPAL_offs;
			for (int i = 0; i < 256; ++i) {
				_egaColorMap[0][i] = *pals & 0x0F;
				_egaColorMap[1][i] = *pals++ >> 4;
			}
		}
	} else {
		setPaletteFromPtr(pals);
	}
}

void ScummEngine::setRoomPalette(int palindex, int room) {
	const byte *roomptr = getResourceAddress(rtRoom, room);
	assert(roomptr);
	const byte *pals = findResource(MKTAG('P','A','L','S'), roomptr);
	assert(pals);
	const byte *rgbs = findPalInPals(pals, palindex);
	assert(rgbs);
	setPaletteFromPtr(rgbs);
}

const byte *ScummEngine::findPalInPals(const byte *pal, int idx) {
	const byte *offs;
	uint32 size;

	pal = findResource(MKTAG('W','R','A','P'), pal);
	if (pal == nullptr)
		return nullptr;

	offs = findResourceData(MKTAG('O','F','F','S'), pal);
	if (offs == nullptr)
		return nullptr;

	size = getResourceDataSize(offs) / 4;
	if ((uint32)idx >= (uint32)size)
		return nullptr;

	return offs + READ_LE_UINT32(offs + idx * sizeof(uint32));
}

const byte *ScummEngine::getPalettePtr(int palindex, int room) {
	const byte *cptr;

	cptr = getResourceAddress(rtRoom, room);
	assert(cptr);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, palindex);
		assert(cptr);
	}
	return cptr;
}

uint32 ScummEngine::getPaletteColorFromRGB(byte *palette, byte r, byte g, byte b) {
	uint32 color, black = 0x00, white = 0xFF;

	if ((r == 0xFF && b == 0xFF && g == 0xFF) || (r == 0x00 && g == 0x00 && b == 0x00)) {
		fetchBlackAndWhite(black, white, palette, 256);

		if (!r) {
			color = black;
		} else {
			color = white;
		}
	} else {
		color = findClosestPaletteColor(palette, 256, r, g, b);
	}

	return color;
}

uint32 ScummEngine::getPackedRGBColorFromPalette(byte *palette, uint32 color) {
	return palette[3 * color] | (palette[3 * color + 1] << 8) | (palette[3 * color + 2] << 16);
}

void ScummEngine::fetchBlackAndWhite(uint32 &black, uint32 &white, byte *palette, int paletteEntries) {
	int max = 0;
	int r, g, b;
	int componentsSum;
	int min = 1000;

	for (int elementId = 0; elementId < paletteEntries; elementId++) {
		r = palette[0];
		g = palette[1];
		b = palette[2];

		componentsSum = r + g + b;
		if (elementId > 0 && componentsSum >= max) {
			max = componentsSum;
			white = elementId;
		}

		if (componentsSum <= min) {
			min = componentsSum;
			black = elementId;
		}

		palette += 3;
	}
}

uint32 ScummEngine::findClosestPaletteColor(byte *palette, int paletteLength, byte r, byte g, byte b) {
	_pl.setPalette(palette, paletteLength);
	return (uint32)_pl.findBestColor(r, g, b, Graphics::kColorDistanceNaive);
}

void ScummEngine::applyGrayscaleToPaletteRange(int min, int max) {
	assertRange(0, min, 256, "ScummEngine::applyGrayscaleToPaletteRange(): min");
	assertRange(0, max, 256, "ScummEngine::applyGrayscaleToPaletteRange(): min");

	if (min <= max) {
		int count = max - min + 1;
		byte *paletteEntry = &_currentPalette[3 * min];

		for (int i = 0; i < count; i++) {
			int r = paletteEntry[0];
			int g = paletteEntry[1];
			int b = paletteEntry[2];
			byte average = (byte)((r + g + b) / 3);

			// The original checks for colors to be < 126,
			// but we handle things a little bit differently,
			// so this is not needed...
			//
			// if (r < 126 || g < 126 || b < 126) {
				paletteEntry[0] = average;
				paletteEntry[1] = average;
				paletteEntry[2] = average;
			// }

			paletteEntry += 3;
		}
	}

	setDirtyColors(min, max);
}

bool ScummEngine::haveToApplyMonkey1PaletteFix() {
	if (_game.id != GID_MONKEY)
		return false;

	bool canChangeMonkey1PaletteSlot = _game.platform == Common::kPlatformMacintosh;

	canChangeMonkey1PaletteSlot |= enhancementEnabled(kEnhVisualChanges) &&
								   (_game.platform != Common::kPlatformSegaCD && _game.platform != Common::kPlatformFMTowns &&
									!(_game.features & GF_ULTIMATE_TALKIE));

	return canChangeMonkey1PaletteSlot;
}

void ScummEngine::updatePalette() {
	if (_game.features & GF_16BIT_COLOR)
		return;

	if (_palDirtyMax == -1)
		return;

	byte paletteColors[3 * 256];
	byte *p = paletteColors;
	int first;
	int num;

	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		// Indy4 Amiga has a special palette handling scheme
		first = 0;
		num = 64;

		for (int i = 0; i < 64; ++i) {
			byte *data;

			if (i < 32)
				data = _amigaPalette + _shadowPalette[i] * 3;
			else
				data = _amigaPalette + i * 3;

			*p++ = data[0] * 255 / 15;
			*p++ = data[1] * 255 / 15;
			*p++ = data[2] * 255 / 15;
		}

		// Setup colors for the mouse cursor
		// Color values taken from Indy4 DOS
		static const uint8 mouseCursorPalette[] = {
			255, 255, 255,
			171, 171, 171,
			 87,  87,  87
		};

		_system->getPaletteManager()->setPalette(mouseCursorPalette, 252, 3);
	} else {
		bool noirMode = (_game.id == GID_SAMNMAX && readVar(0x8000));
		int i;

		first = _palDirtyMin;
		num = _palDirtyMax - first + 1;

		for (i = _palDirtyMin; i <= _palDirtyMax; i++) {
			byte *data;

			if (_shadowPalRemap)
				data = _currentPalette + _shadowPalette[i] * 3;
			else
				data = _currentPalette + i * 3;

			// Sam & Max film noir mode - Enhanced version.
			// Convert the colors to grayscale before uploading them to the backend.
			if (noirMode && enhancementEnabled(kEnhMinorBugFixes)) {
				int r, g, b;
				byte brightness;

				r = data[0];
				g = data[1];
				b = data[2];

				brightness = (byte)((r + g + b) / 3);

				*p++ = brightness;
				*p++ = brightness;
				*p++ = brightness;
			} else {
				*p++ = data[0];
				*p++ = data[1];
				*p++ = data[2];
			}
		}
	}

	_palDirtyMax = -1;
	_palDirtyMin = 256;

#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns) {
		p = paletteColors;
		for (int i = first; i < first + num; ++i) {
			_16BitPalette[i] = get16BitColor(p[0], p[1], p[2]);
			p += 3;
		}
		return;
	}
#endif
#endif

	if (_game.platform == Common::kPlatformMacintosh && _game.heversion == 0) {
		for (int i = 0; i < 3 * num; ++i)
			paletteColors[i] = _macGammaCorrectionLookUp[paletteColors[i]];
	}

	_system->getPaletteManager()->setPalette(paletteColors, first, num);

	if (_macGui)
		_macGui->setPaletteDirty();
}

} // End of namespace Scumm
