/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

// MAKETEXT	- Constructs a single-frame text sprite: returns a handle to a
//		  FLOATING memory block containing the sprite, given a
//		  null-terminated string, max width allowed, pen color and
//		  pointer to required character set.
//
//		  NB 1) The routine does not create a standard file header or
//		  an anim header for the text sprite - the data simply begins
//		  with the frame header.
//
//		  NB 2) If pen color is zero, it copies the characters into
//		  the sprite without remapping the colors.
//		  ie. It can handle both the standard 2-color font for speech
//		  and any multicolored fonts for control panels, etc.
//
//		  Based on textsprt.c as used for Broken Sword 1, but updated
//		  for new system by JEL on 9oct96 and updated again (for font
//		  as a resource) on 5dec96.


#include "common/system.h"
#include "common/unicode-bidi.h"
#include "common/textconsole.h"
#include "common/file.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/screen.h"

namespace Sword2 {

#define MAX_LINES	30	// max character lines in output sprite

#define BORDER_COL	200	// source color for character border (only
						// needed for remapping colors)

#define LETTER_COL	193	// source color for bulk of character ( " )
#define LETTER_COL_PSX1 33
#define LETTER_COL_PSX2 34
#define SPACE		' '
#define FIRST_CHAR	SPACE	// first character in character set
#define LAST_CHAR	255	// last character in character set
#define DUD		64	// the first "chequered flag" (dud) symbol in
				// our character set is in the '@' position

namespace {
Common::String readLine(Common::ReadStream &stream) {
	Common::String ret = stream.readString('\n');
	if (ret.hasSuffix("\r"))
		ret = ret.substr(0, ret.size() - 1);
	return ret;
}
}  // end of anonymous namespace

void FontRenderer::loadTranslations() {
	Common::File bs2en, bs2, font;

	if(bs2en.open("sub/bs2_en.dat") && bs2.open("sub/bs2.dat") && font.open("font/bs1.fnt")) {
		while (!bs2.eos() && !bs2en.eos()) {
			Common::String id = readLine(bs2);
			Common::String val = readLine(bs2);
			Common::String valen = readLine(bs2en);
			Common::String iden = readLine(bs2en);
			if (val.empty() || valen.empty())
				continue;
			debug(5, "id: %s->%s", Common::U32String(iden, Common::CodePage::kWindows936).encode().c_str(),
			      Common::U32String(id, Common::CodePage::kWindows936).encode().c_str());
			debug(5, "val: %s->%s", Common::U32String(valen, Common::CodePage::kWindows936).encode().c_str(),
			      Common::U32String(val, Common::CodePage::kWindows936).encode().c_str());
			_subTranslations[valen] = val;
		}

		while (!font.eos()) {
			ChineseGlyph glyph;
			if (font.read(glyph.bitmap, sizeof (glyph.bitmap)) != sizeof (glyph.bitmap))
				break;
			_chineseFont.push_back(glyph);
		}
	}
}

/**
 * This function creates a new text sprite. The sprite data contains a
 * FrameHeader, but not a standard file header.
 *
 * @param  sentence  pointer to a null-terminated string
 * @param  maxWidth  the maximum allowed text sprite width in pixels
 * @param  pen       the text color, or zero to use the source colors
 * @param  fontRes   the font resource id
 * @param  border    the border color; black by default
 * @return a handle to a floating memory block containing the text sprite
 * @note   The sentence must contain no leading, trailing or extra spaces.
 *         Out-of-range characters in the string are replaced by a special
 *         error-signal character (chequered flag)
 */

byte *FontRenderer::makeTextSprite(const byte *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes, uint8 border) {
	// Keep this at the function scope to make sure we hold a reference even in case of unintentional copies.
	// Normally we should keep using the copy from hashmap.
	Common::String translatedSentence;

	debug(5, "makeTextSprite(\"%s\", maxWidth=%u)", sentence, maxWidth);

	bool isTranslated = false;

	_borderPen = border;

	if (!_subTranslations.empty()) {
		if (_subTranslations.tryGetVal(Common::String((const char *)sentence), translatedSentence)) {
			isTranslated = true;
			debug(5, "Translating <%s> -> <%s>", sentence, translatedSentence.decode(Common::CodePage::kWindows936).encode().c_str());
			sentence = (const byte *)translatedSentence.c_str();
		} else {
			debug(5, "Keeping <%s> untranslated", sentence);
		}
	}

	// Line- and character spacing are hard-wired, rather than being part
	// of the resource.

	if (fontRes == _vm->_speechFontId) {
		if (Sword2Engine::isPsx())
			_lineSpacing = -4; // Text would be unreadable with psx font if linespacing is higher
		else
			_lineSpacing = -6;
		_charSpacing = -3;
	} else if (fontRes == CONSOLE_FONT_ID) {
		_lineSpacing = 0;
		_charSpacing = 1;
	} else {
		_lineSpacing = 0;
		_charSpacing = 0;
	}

	// Allocate memory for array of lineInfo structures

	byte *line = (byte *)malloc(MAX_LINES * sizeof(LineInfo));

	// Get details of sentence breakdown into array of LineInfo structures
	// and get the number of lines involved

	uint16 noOfLines = analyzeSentence(sentence, maxWidth, fontRes, (LineInfo *)line, isTranslated);

	// Construct the sprite based on the info gathered - returns floating
	// mem block

	byte *textSprite = buildTextSprite(sentence, fontRes, pen, (LineInfo *)line, noOfLines, isTranslated);

	free(line);
	return textSprite;
}

uint16 FontRenderer::analyzeSentence(const byte *sentence, uint16 maxWidth, uint32 fontRes, LineInfo *line, bool isChinese) {
	// joinWidth = how much extra space is needed to append a word to a
	// line. NB. SPACE requires TWICE the '_charSpacing' to join a word
	// to line

	uint16 joinWidth = charWidth(SPACE, fontRes) + 2 * _charSpacing;

	uint16 lineNo = 0;
	uint16 pos = 0;
	bool firstWord = true;

	line[0].skipSpace = false;
	line[0].width = 0;
	line[0].length = 0;

	do {
		uint16 wordWidth = 0;
		uint16 wordLength = 0;

		// Calculate the width of the word.
		while (1) {
			byte ch = sentence[pos];
			if (ch == 0 || ch == SPACE)
				break;
			int w = 0, l = 0;
			if (isChinese && (ch & 0x80)) {
				w = kChineseWidth + _charSpacing;
				l = 2;
			} else {
				w = charWidth(ch, fontRes) + _charSpacing;
				l = 1;
			}
				
			wordWidth += w;
			wordLength += l;
			pos += l;
		}

		// Don't include any character spacing at the end of the word.
		wordWidth -= _charSpacing;

		// 'ch' is now the SPACE or NULL following the word
		// 'pos' indexes to the position following 'ch'

		// Word longer than line. Happens for Chinese since it doesn't use spaces.
		if (wordWidth > maxWidth) {
			pos -= wordLength;
			// Add separator if needed.
			if (!firstWord) {
				byte ch = sentence[pos];
				uint16 spaceNeededForOneCharacter = joinWidth;

				if (isChinese && (ch & 0x80)) {
					spaceNeededForOneCharacter += kChineseWidth + _charSpacing;
				}
				spaceNeededForOneCharacter += charWidth(ch, fontRes) + _charSpacing;

				if (line[lineNo].width + spaceNeededForOneCharacter <= maxWidth) {
					// The separator fits on this line.
					line[lineNo].width += spaceNeededForOneCharacter;
					line[lineNo].length += (1 + spaceNeededForOneCharacter);
					line[lineNo].skipSpace = false;
				} else {
					// The word spills over to the next line, i.e.
					// no separating space.
					line[lineNo].skipSpace = true;
					
					lineNo++;

					assert(lineNo < MAX_LINES);

					line[lineNo].width = wordWidth;
					line[lineNo].length = wordLength;
					line[lineNo].skipSpace = false;
				}
			}

			while (1) {
				byte ch = sentence[pos];
				if (ch == 0 || ch == SPACE)
					break;
				int w = 0, l = 0;
				if (isChinese && (ch & 0x80)) {
					w = kChineseWidth + _charSpacing;
					l = 2;
				} else {
					w = charWidth(ch, fontRes) + _charSpacing;
					l = 1;
				}
				if (line[lineNo].width + w <= maxWidth) {
					line[lineNo].width += w;
					line[lineNo].length += l;
				} else {
					line[lineNo].skipSpace = false;
					lineNo++;
					line[lineNo].skipSpace = false;
					line[lineNo].width = w;
					line[lineNo].length = l;
				}
				pos += l;
			}

			continue;
		}

		while (sentence[pos] == SPACE)
			pos++;

		if (firstWord) {
			// This is the first word on the line, so no separating
			// space is needed.

			line[0].width = wordWidth;
			line[0].length = wordLength;
			line[0].skipSpace = false;
			firstWord = false;
		} else {
			// See how much extra space this word will need to
			// fit on current line (with a separating space
			// character - also overlapped)

			uint16 spaceNeeded = joinWidth + wordWidth;

			if (line[lineNo].width + spaceNeeded <= maxWidth) {
				// The word fits on this line.
				line[lineNo].width += spaceNeeded;
				line[lineNo].length += (1 + wordLength);
			} else {
				// The word spills over to the next line, i.e.
				// no separating space.
				line[lineNo].skipSpace = true;

				lineNo++;

				assert(lineNo < MAX_LINES);

				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
				line[lineNo].skipSpace = false;
			}
		}
	} while (sentence[pos]);

	return lineNo + 1;
}

/**
 * This function creates a new text sprite in a movable memory block. It must
 * be locked before use, i.e. lock, draw sprite, unlock/free. The sprite data
 * contains a FrameHeader, but not a standard file header.
 *
 * @param  sentence  pointer to a null-terminated string
 * @param  fontRes   the font resource id
 * @param  pen       the text color, or zero to use the source colors
 * @param  line      array of LineInfo structures, created by analyzeSentence()
 * @param  noOfLines the number of lines, i.e. the number of elements in 'line'
 * @return a handle to a floating memory block containing the text sprite
 * @note   The sentence must contain no leading, trailing or extra spaces.
 *         Out-of-range characters in the string are replaced by a special
 *         error-signal character (chequered flag)
 */

byte *FontRenderer::buildTextSprite(const byte *sentence, uint32 fontRes, uint8 pen, LineInfo *line, uint16 noOfLines, bool isChinese) {
	uint16 i;

	// Find the width of the widest line in the output text

	uint16 spriteWidth = 0;

	for (i = 0; i < noOfLines; i++)
		if (line[i].width > spriteWidth)
			spriteWidth = line[i].width;


	// Check that text sprite has even horizontal resolution in PSX version
	// (needed to work around a problem in some sprites, which reports an odd
	// number as horiz resolution, but then have the next even number as true width)
	if (Sword2Engine::isPsx())
		spriteWidth = (spriteWidth % 2) ? spriteWidth + 1 : spriteWidth;

	// Find the total height of the text sprite: the total height of the
	// text lines, plus the total height of the spacing between them.

	uint16 char_height = charHeight(fontRes);
	uint16 spriteHeight = char_height * noOfLines + _lineSpacing * (noOfLines - 1);

	// Allocate memory for the text sprite

	uint32 sizeOfSprite = spriteWidth * spriteHeight;
	byte *textSprite = (byte *)malloc(FrameHeader::size() + sizeOfSprite);

	// At this stage, textSprite points to an unmovable memory block. Set
	// up the frame header.

	FrameHeader frame_head;

	frame_head.compSize = 0;
	frame_head.width = spriteWidth;
	frame_head.height = spriteHeight;

	// Normally for PSX frame header we double the height
	// of the sprite artificially to regain correct aspect
	// ratio, but this is an "artificially generated" text
	// sprite, which gets created with correct aspect, so
	// fix the height.
	if (Sword2Engine::isPsx())
		frame_head.height /= 2;

	frame_head.write(textSprite);

	debug(4, "Text sprite size: %ux%u", spriteWidth, spriteHeight);

	// Clear the entire sprite to make it transparent.

	byte *linePtr = textSprite + FrameHeader::size();
	memset(linePtr, 0, sizeOfSprite);

	byte *charSet = _vm->_resman->openResource(fontRes);

	// Build the sprite, one line at a time

	for (i = 0; i < noOfLines; i++) {
		// Center each line
		byte *spritePtr = linePtr + (spriteWidth - line[i].width) / 2;
		const byte *currTxtLine = sentence;

		Common::String reversedString;
		if (_vm->_isRTL) {
			const Common::String textLogical((const char *)currTxtLine, line[i].length);
			reversedString = Common::convertBiDiString(textLogical, Common::kWindows1255);
			currTxtLine  = reinterpret_cast<const byte *>(reversedString.c_str());
		}
		// copy the sprite for each character in this line to the
		// text sprite and inc the sprite ptr by the character's
		// width minus the 'overlap'

		for (uint j = 0; j < line[i].length; j++) {
			byte ch = *currTxtLine++;
			if (isChinese && (ch & 0x80)) {
				byte low = *currTxtLine++;
				int fullidx;
				j++;
				if (ch >= 0xa1 && ch <= 0xfe
				    && low >= 0xa1 && low <= 0xfe)
					fullidx = (ch - 0xa1) * 94 + (low - 0xa1);
				else
					fullidx = -1;
				if (fullidx < 0 || fullidx >= (int)_chineseFont.size())
					fullidx = 2 * 94 + 30; // Question mark

				assert(kChineseHeight == char_height);

				copyCharRaw(_chineseFont[fullidx].bitmap, kChineseWidth, kChineseHeight, spritePtr, spriteWidth, pen);

				spritePtr += kChineseWidth + _charSpacing;

				continue;
			}
			byte *charPtr = findChar(ch, charSet);

			frame_head.read(charPtr);

			assert(frame_head.height == char_height);
			copyChar(charPtr, spritePtr, spriteWidth, pen);

			// We must remember to free memory for generated character in psx,
			// as it is extracted differently than pc version (copyed from a
			// char atlas).
			if (Sword2Engine::isPsx())
				free(charPtr);

			spritePtr += frame_head.width + _charSpacing;
		}

		sentence += line[i].length;
		// Skip space at end of last word in this line
		if (line[i].skipSpace)
			sentence++;

		if (Sword2Engine::isPsx())
			linePtr += (char_height / 2 + _lineSpacing) * spriteWidth;
		else
			linePtr += (char_height + _lineSpacing) * spriteWidth;
	}

	_vm->_resman->closeResource(fontRes);

	return textSprite;
}

/**
 * @param  ch      the ASCII code of the character
 * @param  fontRes the font resource id
 * @return the width of the character
 */

uint16 FontRenderer::charWidth(byte ch, uint32 fontRes) {
	byte *charSet = _vm->_resman->openResource(fontRes);
	byte *charBuf;

	FrameHeader frame_head;

	charBuf = findChar(ch, charSet);

	frame_head.read(charBuf);

	if (Sword2Engine::isPsx())
		free(charBuf);

	_vm->_resman->closeResource(fontRes);

	return frame_head.width;
}

/**
 * @param  fontRes the font resource id
 * @return the height of a character sprite
 * @note   All characters in a font are assumed to have the same height, so
 *         there is no need to specify which one to look at.
 */

// Returns the height of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 FontRenderer::charHeight(uint32 fontRes) {
	byte *charSet = _vm->_resman->openResource(fontRes);
	byte *charbuf;

	FrameHeader frame_head;

	charbuf = findChar(FIRST_CHAR, charSet);

	frame_head.read(charbuf);

	if (Sword2Engine::isPsx())
		free(charbuf);

	_vm->_resman->closeResource(fontRes);

	return frame_head.height;
}

/**
 * @param  ch      the ASCII code of the character to find
 * @param  charSet pointer to the start of the character set
 * @return pointer to the requested character or, if it's out of range, the
 *         'dud' character (chequered flag)
 */

byte *FontRenderer::findChar(byte ch, byte *charSet) {

	// PSX version doesn't use an animation table to keep all letters,
	// instead a big sprite (char atlas) is used, and the single char
	// must be extracted from that.

	if (Sword2Engine::isPsx()) {
		byte *buffer;
		PSXFontEntry header;
		FrameHeader bogusHeader;

		charSet += ResHeader::size() + 2;

		if (ch < FIRST_CHAR)
			ch = DUD;

		// Read font entry of the corresponding char.
		header.read(charSet + PSXFontEntry::size() * (ch - 32));

		// We have no such character, generate an empty one
		// on the fly, size 6x12.
		if (header.charWidth == 0) {

			// Prepare a "bogus" FrameHeader to be returned with
			// "empty" character data.
			bogusHeader.compSize = 0;
			bogusHeader.width = 6;
			bogusHeader.height = 12;

			buffer = (byte *)malloc(24 * 3 + FrameHeader::size());
			memset(buffer, 0, 24 * 3 + FrameHeader::size());
			bogusHeader.write(buffer);

			return buffer;
		}

		buffer = (byte *)malloc(FrameHeader::size() + header.charWidth * header.charHeight * 4);
		byte *tempchar = (byte *)malloc(header.charWidth * header.charHeight);

		// Prepare the "bogus" header to be returned with character
		bogusHeader.compSize = 0;
		bogusHeader.width = header.charWidth * 2;
		bogusHeader.height = header.charHeight;

		// Go to the beginning of char atlas
		charSet += 2062;

		memset(buffer, 0, FrameHeader::size() + header.charWidth * header.charHeight * 4);

		bogusHeader.write(buffer);

		// Copy and stretch the char into destination buffer
		for (int idx = 0; idx < header.charHeight; idx++) {
			memcpy(tempchar + header.charWidth * idx, charSet + header.offset + 128 * (header.skipLines + idx), header.charWidth);
		}

		for (int line = 0; line < header.charHeight; line++) {
			for (int col = 0; col < header.charWidth; col++) {
				*(buffer + FrameHeader::size() + line * bogusHeader.width + col * 2) = *(tempchar + line * header.charWidth + col);
				*(buffer + FrameHeader::size() + line * bogusHeader.width + col * 2 + 1) = *(tempchar + line * header.charWidth + col);
			}
		}

		free(tempchar);

		return buffer;

	} else {
		if (ch < FIRST_CHAR)
			ch = DUD;
		return _vm->fetchFrameHeader(charSet, ch - FIRST_CHAR);
	}
}

/**
 * Copies a character sprite to the sprite buffer.
 * @param charPtr     pointer to the character sprite
 * @param spritePtr   pointer to the sprite buffer
 * @param spriteWidth the width of the character
 * @param pen         If zero, copy the data directly. Otherwise remap the
 *                    sprite's colors from BORDER_COL to _borderPen and from
 *                    LETTER_COL to pen.
 */

void FontRenderer::copyChar(const byte *charPtr, byte *spritePtr, uint16 spriteWidth, uint8 pen) {
	FrameHeader frame;

	frame.read(charPtr);

	copyCharRaw(charPtr + FrameHeader::size(), frame.width, frame.height, spritePtr, spriteWidth, pen);
}

void FontRenderer::copyCharRaw(const byte *source, uint16 charWidth, uint16 charHeight, byte *spritePtr, uint16 spriteWidth, uint8 pen) {
	byte *rowPtr = spritePtr;

	for (uint i = 0; i < charHeight; i++) {
		byte *dest = rowPtr;

		if (pen) {
			// Use the specified colors
			for (uint j = 0; j < charWidth; j++) {
				switch (*source++) {
				case 0:
					// Do nothing if source pixel is zero,
					// ie. transparent
					break;
				case LETTER_COL_PSX1: // Values for colored zone
				case LETTER_COL_PSX2:
				case LETTER_COL:
					*dest = pen;
					break;
				case BORDER_COL:
				default:
					// Don't do a border pixel if there's
					// already a bit of another character
					// underneath (for overlapping!)
					if (!*dest)
						*dest = _borderPen;
					break;
				}
				dest++;
			}
		} else {
			// Pen is zero, so just copy character sprites
			// directly into text sprite without remapping colors.
			// Apparently overlapping is never considered here?
			memcpy(dest, source, charWidth);
			source += charWidth;
		}
		rowPtr += spriteWidth;
	}
}

// Distance to keep speech text from edges of screen
#define TEXT_MARGIN 12

/**
 * Creates a text bloc in the list and returns the bloc number. The list of
 * blocs is read and blitted at render time. Choose alignment type
 * RDSPR_DISPLAYALIGN or 0
 */

uint32 FontRenderer::buildNewBloc(byte *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification) {
	uint32 i = 0;

	while (i < MAX_text_blocs && _blocList[i].text_mem)
		i++;

	assert(i < MAX_text_blocs);

	// Create and position the sprite

	_blocList[i].text_mem = makeTextSprite(ascii, width, pen, fontRes);

	// 'NO_JUSTIFICATION' means print sprite with top-left at (x,y)
	// without margin checking - used for debug text

	if (justification != NO_JUSTIFICATION) {
		FrameHeader frame_head;

		frame_head.read(_blocList[i].text_mem);

		switch (justification) {
		case POSITION_AT_CENTER_OF_BASE:
			// This one is always used for SPEECH TEXT; possibly
			// also for pointer text
			x -= (frame_head.width / 2);
			y -= frame_head.height;
			break;
		case POSITION_AT_CENTER_OF_TOP:
			x -= (frame_head.width / 2);
			break;
		case POSITION_AT_CENTER_OF_CENTER:
			x -= (frame_head.width / 2);
			y -= (frame_head.height) / 2;
			break;
		case POSITION_AT_LEFT_OF_TOP:
			// The given coords are already correct for this!
			break;
		case POSITION_AT_RIGHT_OF_TOP:
			x -= frame_head.width;
			break;
		case POSITION_AT_LEFT_OF_BASE:
			y -= frame_head.height;
			break;
		case POSITION_AT_RIGHT_OF_BASE:
			x -= frame_head.width;
			y -= frame_head.height;
			break;
		case POSITION_AT_LEFT_OF_CENTER:
			y -= (frame_head.height / 2);
			break;
		case POSITION_AT_RIGHT_OF_CENTER:
			x -= frame_head.width;
			y -= (frame_head.height) / 2;
			break;
		default:
			break;
		}

		// Ensure text sprite is a few pixels inside the visible screen
		// remember - it's RDSPR_DISPLAYALIGN

		uint16 text_left_margin = TEXT_MARGIN;
		uint16 text_right_margin = 640 - TEXT_MARGIN - frame_head.width;
		uint16 text_top_margin = TEXT_MARGIN;
		uint16 text_bottom_margin = 400 - TEXT_MARGIN - frame_head.height;

		// Move if too far left or too far right

		if (x < text_left_margin)
			x = text_left_margin;
		else if (x > text_right_margin)
			x = text_right_margin;

		// Move if too high or too low

		if (y < text_top_margin)
			y = text_top_margin;
		else if (y > text_bottom_margin)
			y = text_bottom_margin;
	}

	// The sprite is always uncompressed
	_blocList[i].type = type | RDSPR_NOCOMPRESSION;

	_blocList[i].x = x;
	_blocList[i].y = y;

	return i + 1;
}

/**
 * Called by buildDisplay()
 */

void FontRenderer::printTextBlocs() {
	for (uint i = 0; i < MAX_text_blocs; i++) {
		if (_blocList[i].text_mem) {
			FrameHeader frame_head;
			SpriteInfo spriteInfo;

			frame_head.read(_blocList[i].text_mem);

			spriteInfo.x = _blocList[i].x;
			spriteInfo.y = _blocList[i].y;
			spriteInfo.w = frame_head.width;
			spriteInfo.h = frame_head.height;
			spriteInfo.scale = 0;
			spriteInfo.scaledWidth = 0;
			spriteInfo.scaledHeight = 0;
			spriteInfo.type = _blocList[i].type;
			spriteInfo.blend = 0;
			spriteInfo.data = _blocList[i].text_mem + FrameHeader::size();
			spriteInfo.colorTable = 0;
			spriteInfo.isText = true;

			uint32 rv = _vm->_screen->drawSprite(&spriteInfo);
			if (rv)
				error("Driver Error %.8x in printTextBlocs", rv);
		}
	}
}

void FontRenderer::killTextBloc(uint32 bloc_number) {
	bloc_number--;
	free(_blocList[bloc_number].text_mem);
	_blocList[bloc_number].text_mem = NULL;
}

// Resource 3258 contains text from location script for 152 (install, save &
// restore text, etc)

#define TEXT_RES	3258

// Local line number of "save" (actor no. 1826)

#define SAVE_LINE_NO	1

void Sword2Engine::initializeFontResourceFlags() {
	byte *textFile = _resman->openResource(TEXT_RES);

	// If language is Polish or Finnish it requires alternate fonts.
	// Otherwise, use regular fonts

	// "tallenna"   Finnish for "save"
	// "zapisz"     Polish for "save"

	// Get the text line (& skip the 2 chars containing the wavId)
	char *textLine = (char *)fetchTextLine(textFile, SAVE_LINE_NO) + 2;

	if (strcmp(textLine, "tallenna") == 0)
		initializeFontResourceFlags(FINNISH_TEXT);
	else if (strcmp(textLine, "zapisz") == 0)
		initializeFontResourceFlags(POLISH_TEXT);
	else
		initializeFontResourceFlags(DEFAULT_TEXT);

	// Get the game name for the windows application

	// According to the GetGameName(), which was never called and has
	// therefore been removed, the name of the game is:
	//
	// ENGLISH:  "Broken Sword II"
	// AMERICAN: "Circle of Blood II"
	// GERMAN:   "Baphomet's Fluch II"
	// default:  "Some game or other, part 86"
	//
	// But we get it from the text resource instead.

	if (_logic->readVar(DEMO))
		textLine = (char *)fetchTextLine(textFile, 451) + 2;
	else
		textLine = (char *)fetchTextLine(textFile, 54) + 2;

	_system->setWindowCaption(Common::U32String(textLine, _isRTL ? Common::kWindows1255 : Common::kUtf8));
	_resman->closeResource(TEXT_RES);
}

/**
 * Called from initializeFontResourceFlags(), and also from console.cpp
 */

void Sword2Engine::initializeFontResourceFlags(uint8 language) {
	switch (language) {
	case FINNISH_TEXT:
		_speechFontId = FINNISH_SPEECH_FONT_ID;
		_controlsFontId = FINNISH_CONTROLS_FONT_ID;
		_redFontId = FINNISH_RED_FONT_ID;
		break;
	case POLISH_TEXT:
		_speechFontId = POLISH_SPEECH_FONT_ID;
		_controlsFontId = POLISH_CONTROLS_FONT_ID;
		_redFontId = POLISH_RED_FONT_ID;
		break;
	default:
		_speechFontId = ENGLISH_SPEECH_FONT_ID;
		_controlsFontId = ENGLISH_CONTROLS_FONT_ID;
		_redFontId = ENGLISH_RED_FONT_ID;
		break;
	}
}

} // End of namespace Sword2
