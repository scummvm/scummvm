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
#include "common/translation.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "graphics/fonts/amigafont.h"
#include "graphics/fonts/dosfont.h"

#include "agi/agi.h"
#include "agi/font.h"
#include "agi/text.h"

namespace Agi {

GfxFont::GfxFont(AgiBase *vm) {
	_vm = vm;

	_fontData = nullptr;
	_fontDataAllocated = nullptr;
	_fontIsHires = false;
}

GfxFont::~GfxFont() {
	free(_fontDataAllocated);
}

// Arrow to the right character, used for original saved game dialogs
// Needs to get patched into at least the Apple IIgs font, because the font didn't support
// that character and original AGI on Apple IIgs used Apple II menus for saving/restoring.
static const uint8 fontData_ArrowRightCharacter[8] = {
	0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,
};

void GfxFont::init() {
	if (ConfMan.getBool("herculesfont")) {
		// User wants, that we use Hercules hires font, try to load it
		loadFontHercules();
	} else {
		switch (_vm->_renderMode) {
		case Common::kRenderHercA:
		case Common::kRenderHercG:
			// Render mode is Hercules, we try to load Hercules hires font
			loadFontHercules();
			break;
		default:
			break;
		}
	}

	if (!_fontData) {
		switch (_vm->_renderMode) {
		case Common::kRenderAmiga:
			// Try user-file first, if that fails use our internal inaccurate topaz font
			loadFontScummVMFile("agi-font-amiga.bin");
			if (!_fontData) {
				loadFontAmigaPseudoTopaz();
			}
			break;
		case Common::kRenderApple2GS:
			// Special font, stored in file AGIFONT
			loadFontAppleIIgs();
			break;
		case Common::kRenderAtariST:
			// TODO: Atari ST uses another font
			// Seems to be the standard Atari ST 8x8 system font
			loadFontScummVMFile("agi-font-atarist.bin");
			if (!_fontData) {
				loadFontAtariST("agi-font-atarist-system.fnt");
				if (!_fontData) {
					// TODO: in case we find a recreation of the font, add it in here
				}
			}
			break;
		case Common::kRenderHercA:
		case Common::kRenderHercG:
		case Common::kRenderCGA:
		case Common::kRenderEGA:
		case Common::kRenderVGA:
			switch (_vm->getGameID()) {
			case GID_MICKEY:
				// load mickey mouse font from interpreter file
				loadFontMickey();
				break;
			default:
				loadFontScummVMFile("agi-font-dos.bin");
				break;
			}
			break;

		default:
			break;
		}

		if (!_fontData) {
			// no font assigned?
			// use regular PC-BIOS font (taken from Dos-Box with a few modifications)
			_fontData = Graphics::DosFont::fontData_PCBIOS;
			debug("AGI: Using PC-BIOS font");
		}
	}

	if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian versions need special extended set
		overwriteExtendedWithRussianSet();
	}
}

const byte *GfxFont::getFontData() {
	assert(_fontData);
	return _fontData;
}

bool GfxFont::isFontHires() {
	return _fontIsHires;
}

void GfxFont::overwriteSaveRestoreDialogCharacter() {
	// overwrite character 0x1A with the standard Sierra arrow to the right character
	// required for the original save/restore dialogs
	memcpy(_fontDataAllocated + (0x1A * 8), fontData_ArrowRightCharacter, sizeof(fontData_ArrowRightCharacter));
}

// Overwrite extended character set (0x80-0xFF) with Russian characters
void GfxFont::overwriteExtendedWithRussianSet() {
	if (_fontIsHires) {
		// TODO: Implement overwriting hires font characters too
		return;
	}

	if (!_fontDataAllocated) {
		// nothing allocated, we need to allocate space ourselves to be able to modify an internal font
		_fontDataAllocated = (uint8 *)calloc(256, 8);
		memcpy(_fontDataAllocated, _fontData, 128 * 8); // copy ASCII set over
		_fontData = _fontDataAllocated;
	}
	// Overwrite extended set with Russian characters
	memcpy(_fontDataAllocated + (128 * 8), Graphics::DosFont::fontData_ExtendedRussian, 128 * 8);

	debug("AGI: Using Russian extended font set");
}

// This code loads a ScummVM-specific user-supplied binary font file
// It's assumed that it's a plain binary file, that contains 256 characters. 8 bytes per character.
// 8x8 pixels per character. File size 2048 bytes.
//
// Currently used for:
//  Atari ST - "agi-font-atarist.bin" -> should be the Atari ST 8x8 system font
//  Amiga    - "agi-font-amiga.bin"   -> should be the Amiga 8x8 Topaz font
//  DOS      - "agi-font-dos.bin"
void GfxFont::loadFontScummVMFile(Common::String fontFilename) {
	Common::File fontFile;
	int32 fontFileSize = 0;

	if (!fontFile.open(fontFilename)) {
		// Continue, if file not found
		// These ScummVM font files are totally optional, so don't show a warning
		return;
	}

	fontFileSize = fontFile.size();
	if (fontFileSize != (256 * 8)) {
		// unexpected file size
		fontFile.close();
		warning("Fontfile '%s': unexpected file size", fontFilename.c_str());
		return;
	}

	// allocate space for font bitmap data
	_fontDataAllocated = (uint8 *)calloc(256, 8);
	_fontData = _fontDataAllocated;

	// read font data, is already in the format that we need (plain bitmap 8x8)
	fontFile.read(_fontDataAllocated, 256 * 8);
	fontFile.close();

	overwriteSaveRestoreDialogCharacter();

	debug("AGI: Using user-supplied font");
}

// We load the Mickey Mouse font from MICKEY.EXE
void GfxFont::loadFontMickey() {
	Common::File interpreterFile;
	int32 interpreterFileSize = 0;
	byte *fontData = nullptr;

	if (!interpreterFile.open("mickey.exe")) {
		// Continue, if file not found
		warning("Could not open file 'mickey.exe' for Mickey Mouse font");
		return;
	}

	interpreterFileSize = interpreterFile.size();
	if (interpreterFileSize != 55136) {
		// unexpected file size
		interpreterFile.close();
		warning("File 'mickey.exe': unexpected file size");
		return;
	}
	interpreterFile.seek(32476); // offset of font data

	// allocate space for font bitmap data
	fontData = (uint8 *)calloc(256, 8);
	_fontData = fontData;
	_fontDataAllocated = fontData;

	// read font data, is already in the format that we need (plain bitmap 8x8)
	interpreterFile.read(fontData, 256 * 8);
	interpreterFile.close();

	debug("AGI: Using Mickey Mouse font");
}

// we create a bitmap out of the topaz data used in parallaction (which is normally found in staticres.cpp)
// it's a recreation of the Amiga Topaz font but not really accurate
void GfxFont::loadFontAmigaPseudoTopaz() {
	Graphics::Surface surf;
	surf.create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
	byte *fontData = (uint8 *)calloc(256, 8);

	_fontData = fontData;
	_fontDataAllocated = fontData;

	// copy first 32 PC-BIOS characters over
	memcpy(fontData, Graphics::DosFont::fontData_PCBIOS, FONT_DISPLAY_WIDTH * 32);
	fontData += FONT_DISPLAY_WIDTH * 32;

	Graphics::AmigaFont topaz;

	for (uint16 curChar = topaz.getLoChar(); curChar <= topaz.getHiChar(); curChar++) {
		topaz.drawChar(&surf, curChar, 0, 0, 0xff);

		// Turn it into 1bpp
		for (int y = 0; y < 8; y++) {
			byte line = 0;
			byte *ptr = (byte *)surf.getBasePtr(0, y);
			for (int x = 0; x < 8; x++, ptr++) {
				line <<= 1;
				line |= *ptr & 0x1;
			}
			*fontData++ = line;
		}
		memset(surf.getPixels(), 0, 8 * 8);
	}

	surf.free();

	debug("AGI: Using recreation of Amiga Topaz font");
}

void GfxFont::loadFontAppleIIgs() {
	Common::File fontFile;
	uint16 headerIIgs_OffsetMacHeader = 0;
	uint16 headerIIgs_Version = 0;
	uint16 macRecord_FirstChar = 0;
	uint16 macRecord_LastChar = 0;
	int16 macRecord_MaxKern = 0;
	uint16 macRecord_RectHeight = 0;
	uint16 macRecord_StrikeWidth = 0;
	uint16 strikeDataLen = 0;
	byte *strikeDataPtr = nullptr;
	uint16 actualCharacterCount = 0;
	uint16 totalCharacterCount = 0;
	uint16 *locationTablePtr = nullptr;
	uint16 *offsetWidthTablePtr = nullptr;

	uint16 curCharNr = 0;
	uint16 curRow = 0;
	uint16 curLocation = 0;
	uint16 curLocationBytes = 0;
	uint16 curLocationBits = 0;
	uint16 curCharOffsetWidth = 0;
	uint16 curCharOffset = 0;
	uint16 curCharWidth = 0;
	uint16 curStrikeWidth = 0;

	uint16 curPixelNr = 0;
	uint16 curBitMask = 0;
	int16 positionAdjust = 0;
	byte curByte = 0;
	byte fontByte = 0;

	uint16 strikeRowOffset = 0;
	uint16 strikeCurOffset = 0;

	byte *fontData = nullptr;

	if (!fontFile.open("agifont")) {
		// Continue,
		// This also happens when the user selected Apple IIgs as render for the palette for non-AppleIIgs games
		warning("Could not open file 'agifont' for Apple IIgs font");
		return;
	}

	// Apple IIgs header
	headerIIgs_OffsetMacHeader = fontFile.readUint16LE();
	fontFile.skip(2); // font family
	fontFile.skip(2); // font style
	fontFile.skip(2); // point size
	headerIIgs_Version = fontFile.readUint16LE();
	fontFile.skip(2); // bounds type
	// end of Apple IIgs header
	// Macintosh font record
	fontFile.skip(2); // font type
	macRecord_FirstChar = fontFile.readUint16LE();
	macRecord_LastChar = fontFile.readUint16LE();
	fontFile.skip(2); // max width
	macRecord_MaxKern = fontFile.readSint16LE();
	fontFile.skip(2); // negative descent
	fontFile.skip(2); // rect width
	macRecord_RectHeight = fontFile.readUint16LE();
	fontFile.skip(2); // low word ptr table
	fontFile.skip(2); // font ascent
	fontFile.skip(2); // font descent
	fontFile.skip(2); // leading
	macRecord_StrikeWidth = fontFile.readUint16LE();

	// security-checks
	if (headerIIgs_OffsetMacHeader != 6)
		error("AppleIIgs-font: unexpected header");
	if (headerIIgs_Version != 0x0101)
		error("AppleIIgs-font: not a 1.1 font");
	if ((macRecord_FirstChar != 0) || (macRecord_LastChar != 255))
		error("AppleIIgs-font: unexpected characters");
	if (macRecord_RectHeight != 8)
		error("AppleIIgs-font: expected 8x8 font");

	// Calculate table sizes
	strikeDataLen = macRecord_StrikeWidth * macRecord_RectHeight * 2;
	actualCharacterCount = (macRecord_LastChar - macRecord_FirstChar + 1);
	totalCharacterCount = actualCharacterCount + 2; // replacement-char + extra character

	// Allocate memory for tables
	strikeDataPtr = (byte *)calloc(strikeDataLen, 1);
	locationTablePtr = (uint16 *)calloc(totalCharacterCount, 2); // 1 word per character
	offsetWidthTablePtr = (uint16 *)calloc(totalCharacterCount, 2); // ditto

	// read tables
	fontFile.read(strikeDataPtr, strikeDataLen);
	for (curCharNr = 0; curCharNr < totalCharacterCount; curCharNr++) {
		locationTablePtr[curCharNr] = fontFile.readUint16LE();
	}
	for (curCharNr = 0; curCharNr < totalCharacterCount; curCharNr++) {
		offsetWidthTablePtr[curCharNr] = fontFile.readUint16LE();
	}
	fontFile.close();

	// allocate space for font bitmap data
	fontData = (uint8 *)calloc(256, 8);
	_fontData = fontData;
	_fontDataAllocated = fontData;

	// extract font bitmap data
	for (curCharNr = 0; curCharNr < actualCharacterCount; curCharNr++) {
		curCharOffsetWidth = offsetWidthTablePtr[curCharNr];
		curLocation = locationTablePtr[curCharNr];
		if (curCharOffsetWidth == 0xFFFF) {
			// character does not exist in font, use replacement character instead
			curCharOffsetWidth = offsetWidthTablePtr[actualCharacterCount];
			curLocation = locationTablePtr[actualCharacterCount];
			curStrikeWidth = locationTablePtr[actualCharacterCount + 1] - curLocation;
		} else {
			curStrikeWidth = locationTablePtr[curCharNr + 1] - curLocation;
		}

		// Figure out bytes + bits location
		curLocationBytes = curLocation >> 3;
		curLocationBits = curLocation & 0x0007;
		curCharWidth = curCharOffsetWidth & 0x00FF; // isolate width
		curCharOffset = curCharOffsetWidth >> 8; // isolate offset

		if (!curCharWidth) {
			fontData += 8; // skip over this character
			continue;
		}

		if (curCharWidth != 8) {
			if (curCharNr != 0x3B)
				error("AppleIIgs-font: expected 8x8 font");
		}

		// Get all rows of the current character
		strikeRowOffset = 0;
		for (curRow = 0; curRow < macRecord_RectHeight; curRow++) {
			strikeCurOffset = strikeRowOffset + curLocationBytes;

			// Copy over bits
			fontByte = 0;
			curByte = strikeDataPtr[strikeCurOffset];
			curBitMask = 0x80 >> curLocationBits;

			for (curPixelNr = 0; curPixelNr < curStrikeWidth; curPixelNr++) {
				fontByte = fontByte << 1;
				if (curByte & curBitMask) {
					fontByte |= 0x01;
				}
				curBitMask = curBitMask >> 1;
				if (!curBitMask) {
					curByte = strikeDataPtr[strikeCurOffset + 1];
					curBitMask = 0x80;
				}
			}

			// adjust, so that it's aligned to the left (starting at 0x80 bit)
			fontByte = fontByte << (8 - curStrikeWidth);

			// now adjust according to offset + MaxKern
			positionAdjust = macRecord_MaxKern + curCharOffset;

			// adjust may be negative for space, or 8 for "empty" characters
			if (positionAdjust > 8)
				error("AppleIIgs-font: invalid character spacing");

			if (positionAdjust < 0) {
				// negative adjust strangely happens for empty characters like space
				if (curStrikeWidth)
					error("AppleIIgs-font: invalid character spacing");
			}

			if (positionAdjust > 0) {
				// move the amount of pixels to the right
				fontByte = fontByte >> positionAdjust;
			}

			*fontData = fontByte;
			fontData++;

			strikeRowOffset += macRecord_StrikeWidth * 2;
		}
	}

	free(offsetWidthTablePtr);
	free(locationTablePtr);
	free(strikeDataPtr);

	overwriteSaveRestoreDialogCharacter();

	debug("AGI: Using Apple IIgs font");
}

// Loads Atari ST font file
// It's found inside Atari ST ROMs. Just search for "8x8 system font". Font starts 4 bytes before that.
void GfxFont::loadFontAtariST(Common::String fontFilename) {
	Common::File fontFile;
	uint16 header_FirstChar = 0;
	uint16 header_LastChar = 0;
	uint16 header_MaxWidth = 0;
	uint16 header_MaxHeight = 0;
	uint16 header_Flags = 0;
	//uint32 header_OffsetOfCharOffsets = 0;
	//uint32 header_OffsetOfFontData = 0;
	uint16 header_FormWidth = 0;
	uint16 header_FormHeight = 0;
	uint16 totalCharacterCount = 0;
	uint16 *charOffsetTablePtr = nullptr;
	byte *rawDataTablePtr = nullptr;

	uint16 curCharNr = 0;
	uint16 curCharRawOffset = 0;
	uint16 curCharDestOffset = 0;
	uint16 curRow = 0;

	byte *fontData = nullptr;

	if (!fontFile.open(fontFilename)) {
		// Continue, if file not found
		warning("Could not open file 'agi-font-atarist-system.bin' for Atari ST 8x8 system font");
		return;
	}

	// Atari ST font header
	fontFile.skip(2); // face identifier
	fontFile.skip(2); // point size
	fontFile.skip(32); // font name
	header_FirstChar = fontFile.readUint16BE();
	header_LastChar = fontFile.readUint16BE();
	fontFile.skip(10); // aligntment of cells
	header_MaxWidth = fontFile.readUint16BE();
	header_MaxHeight = fontFile.readUint16BE();
	fontFile.skip(2); // left offset cel
	fontFile.skip(2); // right offset cel
	fontFile.skip(2); // number of pixels to thicken pixels
	fontFile.skip(2); // underline width
	fontFile.skip(2); // lightning mask
	fontFile.skip(2); // skewing mask
	header_Flags = fontFile.readUint16BE();
	// bit 0 - default system font
	// bit 1 - horizontal offset table (not supported)
	// bit 2 - byte orientation word is high->low
	// bit 3 - mono spaced font
	fontFile.skip(4); // horizontal table offset
	fontFile.skip(4); // header_OffsetOfCharOffsets = fontFile.readUint32BE();
	fontFile.skip(4); // header_OffsetOfFontData = fontFile.readUint32BE();
	header_FormWidth = fontFile.readUint16BE();
	header_FormHeight = fontFile.readUint16BE();
	fontFile.skip(4); // pointer to next font

	totalCharacterCount = header_LastChar - header_FirstChar + 1;

	// security-checks
	if (header_MaxWidth > 8)
		error("AtariST-font: not a 8x8 font");
	if (header_MaxHeight != 8)
		error("AtariST-font: not a 8x8 font");
	if (header_FormHeight != 8)
		error("AtariST-font: not a 8x8 font");
	if ((header_FirstChar != 0) || (header_LastChar != 255))
		error("AtariST-font: unexpected characters");
	if (header_FormWidth != totalCharacterCount)
		error("AtariST-font: header inconsistency");
	if (!(header_Flags & 0x04))
		error("AtariST-font: font data not in high->low order");
	if (!(header_Flags & 0x08))
		error("AtariST-font: not a mono-spaced font");

	// Now we should normally use the offsets, but they don't make sense to me
	// So I just read the data directly. For the 8x8 system font that works
	fontFile.skip(2); // extra bytes

	// Allocate memory for tables
	charOffsetTablePtr = (uint16 *)calloc(totalCharacterCount, 2); // 1 word per character
	rawDataTablePtr = (byte *)calloc(header_FormWidth, header_FormHeight);

	// Char-Offset Table (2 * total number of characters)
	for (curCharNr = 0; curCharNr < totalCharacterCount; curCharNr++) {
		charOffsetTablePtr[curCharNr] = fontFile.readUint16BE();
	}

	// Followed by actual font data
	// Attention: Atari ST fonts contain every same row of all characters after each other.
	// So it's basically like this:
	// [character data of first row of first character]
	// [character data of first row of second character]
	// ...
	// [character data of first row of last character]
	// [character data of second row of first character]
	fontFile.skip(2); // extra bytes
	fontFile.read(rawDataTablePtr, header_FormWidth * header_FormHeight);
	fontFile.close();

	// allocate space for font bitmap data
	fontData = (uint8 *)calloc(256, 8);
	_fontData = fontData;
	_fontDataAllocated = fontData;

	// extract font bitmap data
	for (curCharNr = 0; curCharNr < totalCharacterCount; curCharNr++) {
		// Figure out base offset from char offset table
		curCharRawOffset = charOffsetTablePtr[curCharNr] >> 3;
		curCharDestOffset = curCharNr * 8; // destination offset into our font data

		// now copy over every row of the character
		for (curRow = 0; curRow < header_FormHeight; curRow++) {
			fontData[curCharDestOffset] = rawDataTablePtr[curCharRawOffset];
			curCharDestOffset++;
			curCharRawOffset += header_FormWidth;
		}
	}

	free(rawDataTablePtr);
	free(charOffsetTablePtr);

	overwriteSaveRestoreDialogCharacter();

	debug("AGI: Using Atari ST 8x8 system font");
}

// Loads a Sierra Hercules font file
void GfxFont::loadFontHercules() {

	if (_vm->getLanguage() == Common::RU_RUS) {
		warning("Hercules font does not contain Russian characters, switching to default");

		return;
	}

	Common::File fontFile;
	int32 fontFileSize = 0;
	byte *fontData = nullptr;
	byte *rawData = nullptr;

	uint16 rawDataPos = 0;
	uint16 curCharNr = 0;
	uint16 curCharLine = 0;

	if (fontFile.open("hgc_font")) {
		// hgc_font file found, this is interleaved font data 16x12, should be 3072 bytes
		// 24 bytes per character, 128 characters
		fontFileSize = fontFile.size();
		if (fontFileSize == (128 * 24)) {
			// size seems to be fine
			fontData = (uint8 *)calloc(256, 32);
			_fontDataAllocated = fontData;

			rawData = (byte *)calloc(128, 24);
			fontFile.read(rawData, 128 * 24);

			// convert interleaved 16x12 -> non-interleaved 16x16
			for (curCharNr = 0; curCharNr < 128; curCharNr++) {
				fontData += 4; // skip the first 2 lines
				for (curCharLine = 0; curCharLine < 6; curCharLine++) {
					fontData[0] = rawData[rawDataPos + 2 + 0];
					fontData[1] = rawData[rawDataPos + 2 + 1];
					fontData[2] = rawData[rawDataPos + 0 + 0];
					fontData[3] = rawData[rawDataPos + 0 + 1];
					rawDataPos += 4;
					fontData += 4;
				}
				fontData += 4; // skip the last 2 lines
			}

			free(rawData);
		} else {
			warning("Fontfile 'hgc_font': unexpected file size");
		}
		fontFile.close();

	}

	// It seems hgc_graf.ovl holds a low-res font. It makes no real sense to use it.
	// This was only done to AGI3 games and those rendered differently (2 pixel lines -> 3 pixel lines instead of 4)
	// User could copy hgc_font from another AGI game over to get the hires font working.
#if 0
	if (!_fontDataAllocated) {
		if (fontFile.open("hgc_graf.ovl")) {
			// hgc_graf.ovl file found, this is font data + code. non-interleaved font data, should be 3075 bytes
			// 16 bytes per character, 128 characters, 2048 bytes of font data, starting offset 21
			fontFileSize = fontFile.size();
			if (fontFileSize == 3075) {
				// size seems to be fine
				fontData = (uint8 *)calloc(256, 32);
				_fontDataAllocated = fontData;

				fontFile.seek(21);
				rawData = (byte *)calloc(128, 16);
				fontFile.read(rawData, 128 * 16);

				// repeat every line 2 times to get 16x16 pixels
				for (curCharNr = 0; curCharNr < 128; curCharNr++) {
					for (curCharLine = 0; curCharLine < 8; curCharLine++) {
						fontData[0] = rawData[rawDataPos + 0];
						fontData[1] = rawData[rawDataPos + 1];
						fontData[2] = rawData[rawDataPos + 0];
						fontData[3] = rawData[rawDataPos + 1];
						rawDataPos += 2;
						fontData += 4;
					}
				}

				free(rawData);

			} else {
				warning("Fontfile 'hgc_graf.ovl': unexpected file size");
			}
			fontFile.close();
		}
	}
#endif

	if (_fontDataAllocated) {
		// font loaded
		_fontData = _fontDataAllocated;
		_fontIsHires = true;

		debug("AGI: Using Hercules hires font");

	} else {
		// Continue, if no file was not found
		warning("Could not open/use file 'hgc_font' for Hercules hires font");
		if (GUI::GuiManager::hasInstance()) {
			GUI::MessageDialog dialog(_("Could not open/use file 'hgc_font' for Hercules hires font.\nIf you have such file in other AGI (Sierra) game, you can copy it to the game directory"));
			dialog.runModal();
		};

	}
}

} // End of namespace Agi
