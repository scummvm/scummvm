/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/text.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/system.h"
#include "twine/hqr.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/screens.h"
#include "twine/scene.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA movie extension */
#define VOX_EXT ".vox"

#define INDEXOFFSET 0
#define DIALOGSOFFSET 1

void Text::initVoxBank(int32 bankIdx) {
	static const char *LanguageSufixTypes[] = {
	    "sys",
	    "cre",
	    "gam", // global game voices (e.g. inventory descriptions)
	    "000", // Citadel Island voices
	    "001", // Principal Island voices
	    "002", // White Leaf Desert voices
	    "003", // Proxima Island voices
	    "004", // Rebellion Island voices
	    "005", // Hamalayi Mountains - sourthern range voices
	    "006", // Hamalayi Mountains - northern range voices
	    "007", // Tippett Island voices
	    "008", // Brundle Island voices
	    "009", // Fortress Island voices
	    "010", // Polar Island voices
	    "011"  //
	};
	if (bankIdx < 0 || bankIdx >= ARRAYSIZE(LanguageSufixTypes)) {
		error("bankIdx is out of bounds: %i", bankIdx);
	}
	// get the correct vox hqr file
	currentVoxBankFile = Common::String::format("%s%s" VOX_EXT, LanguageTypes[_engine->cfgfile.LanguageId].id, LanguageSufixTypes[bankIdx]);
	// TODO: loop through other languages and take the scummvm settings regarding voices into account...

	// TODO check the rest to reverse
}

bool Text::initVoxToPlay(int32 index) { // setVoxFileAtDigit
	currDialTextEntry = 0;
	voxHiddenIndex = 0;
	hasHiddenVox = false;

	Common::MemoryReadStream stream((const byte*)dialOrderPtr, dialOrderSize);
	// choose right text from order index
	for (int32 i = 0; i < numDialTextEntries; i++) {
		int32 orderIdx = stream.readSint16LE();
		if (orderIdx == index) {
			currDialTextEntry = i;
			break;
		}
	}

	_engine->_sound->playVoxSample(currDialTextEntry);

	return true;
}

bool Text::playVox(int32 index) {
	if (!_engine->cfgfile.Voice) {
		return false;
	}
	if (hasHiddenVox && !_engine->_sound->isSamplePlaying(index)) {
		_engine->_sound->playVoxSample(index);
		return true;
	}

	return false;
}

bool Text::playVoxSimple(int32 index) {
	if (_engine->_sound->isSamplePlaying(index)) {
		return true;
	}
	return playVox(index);
}

bool Text::stopVox(int32 index) {
	if (!_engine->_sound->isSamplePlaying(index)) {
		return false;
	}
	hasHiddenVox = false;
	_engine->_sound->stopSample(index);
	return true;
}

void Text::initTextBank(int32 bankIdx) {
	// don't load if we already have the dialogue text bank loaded
	if (bankIdx == currentBankIdx) {
		return;
	}

	currentBankIdx = bankIdx;
	textVar2[0] = '\0';

	// get index according with language
	const int32 size = _engine->isLBA1() ? 28 : 30;
	// the text banks indices are split into index and dialogs - each entry thus consists of two entries in the hqr
	// every 28 entries starts a new language
	const int32 languageIndex = _engine->cfgfile.LanguageId * size + (int)bankIdx * 2;
	dialOrderSize = HQR::getAllocEntry((uint8 **)&dialOrderPtr, Resources::HQR_TEXT_FILE, languageIndex + INDEXOFFSET);
	if (dialOrderSize == 0) {
		warning("Failed to initialize text bank %i from file %s", languageIndex, Resources::HQR_TEXT_FILE);
		return;
	}

	numDialTextEntries = dialOrderSize / 2;

	if (HQR::getAllocEntry((uint8 **)&dialTextPtr, Resources::HQR_TEXT_FILE, languageIndex + DIALOGSOFFSET) == 0) {
		warning("Failed to initialize additional text bank %i from file %s", languageIndex + 1, Resources::HQR_TEXT_FILE);
		return;
	}
	initVoxBank(bankIdx);
}

void Text::drawCharacter(int32 x, int32 y, uint8 character) { // drawCharacter
	const uint8 sizeX = getCharWidth(character);
	uint8 sizeY = getCharHeight(character);
	Common::MemoryReadStream stream(_engine->_resources->fontPtr, _engine->_resources->fontBufSize);
	stream.seek(character * 4);
	stream.seek(stream.readSint16LE());
	stream.skip(2);
	x += stream.readByte();
	y += stream.readByte();

	const uint8 usedColor = dialTextColor;

	uint8 *screen2 = (uint8 *)_engine->frontVideoBuffer.getPixels() + _engine->screenLookupTable[y] + x;

	int32 tempX = x;
	int32 tempY = y;

	const int32 toNextLine = SCREEN_WIDTH - sizeX;

	do {
		uint8 index = stream.readByte();
		do {
			const uint8 jump = stream.readByte();
			screen2 += jump;
			tempX += jump;
			if (--index == 0) {
				screen2 += toNextLine;
				tempY++;
				tempX = x;
				sizeY--;
				if (sizeY <= 0) {
					return;
				}
				break;
			}
			uint8 number = stream.readByte();
			for (uint8 i = 0; i < number; i++) {
				if (tempX >= SCREEN_TEXTLIMIT_LEFT && tempX < SCREEN_TEXTLIMIT_RIGHT && tempY >= SCREEN_TEXTLIMIT_TOP && tempY < SCREEN_TEXTLIMIT_BOTTOM) {
					*((uint8 *)_engine->frontVideoBuffer.getBasePtr(tempX, tempY)) = usedColor;
				}

				screen2++;
				tempX++;
			}

			if (--index == 0) {
				screen2 += toNextLine;
				tempY++;
				tempX = x;

				sizeY--;
				if (sizeY <= 0) {
					return;
				}
				break;
			}
		} while (1);
	} while (1);
}

void Text::drawCharacterShadow(int32 x, int32 y, uint8 character, int32 color) { // drawDoubleLetter
	int32 left, top, right, bottom;

	if (character != ' ') {
		// shadow color
		setFontColor(0);
		drawCharacter(x + 2, y + 4, character);

		// text color
		setFontColor(color);
		drawCharacter(x, y, character);

		left = x;
		top = y;
		// FIXME: get right font size
		right = x + 32;
		bottom = y + 38;

		_engine->copyBlockPhys(left, top, right, bottom);
	}
}

void Text::drawText(int32 x, int32 y, const char *dialogue) { // Font
	                                                          // if the font is not defined
	if (_engine->_resources->fontPtr == nullptr) {
		return;
	}

	do {
		const uint8 currChar = (uint8) * (dialogue++); // read the next char from the string

		if (currChar == '\0') {
			break;
		}

		if (currChar == ' ') {
			x += dialCharSpace;
		} else {
			dialTextSize = getCharWidth(currChar);
			drawCharacter(x, y, currChar); // draw the character on screen
			// add the length of the space between 2 characters
			x += dialSpaceBetween;
			// add the length of the current character
			x += dialTextSize;
		}
	} while (1);
}

int32 Text::getTextSize(const char *dialogue) { // SizeFont
	dialTextSize = 0;

	do {
		uint8 currChar = (uint8) * (dialogue++);
		if (currChar == 0)
			break;

		if (currChar == ' ') {
			dialTextSize += dialCharSpace;
		} else {
			dialTextSize += dialSpaceBetween;
			dialTextSize += getCharWidth(currChar);
		}
	} while (1);

	return (dialTextSize);
}

void Text::initDialogueBox() { // InitDialWindow
	_engine->_interface->blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (const int8 *)_engine->workVideoBuffer.getPixels(), dialTextBoxLeft, dialTextBoxTop, (int8 *)_engine->frontVideoBuffer.getPixels());

	if (newGameVar4 != 0) {
		_engine->_menu->drawBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
		_engine->_interface->drawTransparentBox(dialTextBoxLeft + 1, dialTextBoxTop + 1, dialTextBoxRight - 1, dialTextBoxBottom - 1, 3);
	}

	_engine->copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
	printText8Var3 = 0;
	_engine->_interface->blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (const int8 *)_engine->frontVideoBuffer.getPixels(), dialTextBoxLeft, dialTextBoxTop, (int8 *)_engine->workVideoBuffer.getPixels());
}

void Text::initInventoryDialogueBox() { // SecondInitDialWindow
	_engine->_interface->blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (const int8 *)_engine->workVideoBuffer.getPixels(), dialTextBoxLeft, dialTextBoxTop, (int8 *)_engine->frontVideoBuffer.getPixels());
	_engine->copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
	printText8Var3 = 0;
}

// TODO: refactor this code
void Text::initText(int32 index) {
	printTextVar13 = 0;

	if (!getText(index)) {
		return;
	}

	printText8Ptr1 = buf1;
	printText8Ptr2 = buf2;

	printTextVar13 = 1;

	printText8Var1 = 0;
	buf1[0] = 0;
	buf2[0] = 0;
	printText8Var2 = index;
	printText8Var3 = 0;
	TEXT_CurrentLetterX = dialTextBoxLeft + 8;
	printText8Var5 = 0;
	printText8Var6 = 0;
	TEXT_CurrentLetterY = dialTextBoxTop + 8;
	printText8Var8 = currDialTextPtr;

	// lba font is get while engine start
	setFontParameters(2, 7);
}

void Text::initProgressiveTextBuffer() {
	int32 i = 0;

	buf2[0] = '\0';

	while (i < dialTextBufferSize) {
		strncat(buf2, " ", sizeof(buf2));
		i++;
	};

	printText8Ptr2 = buf2;
	addLineBreakX = 16;
	printText8Var1 = 0;
}

void Text::printText8Sub4(int16 a, int16 b, int16 c) {
	int32 counter2 = 0;

	if (printText8Var3 < 32) {
		const int32 temp = printText8Var3 * 3;
		pt8s4[temp] = c;
		pt8s4[temp + 1] = a;
		pt8s4[temp + 2] = b;

		printText8Var3++;
	} else {
		while (counter2 < 31) {
			const int32 var1 = (counter2 + 1) * 3;
			const int32 var2 = counter2 * 3;
			pt8s4[var2] = pt8s4[var1];
			pt8s4[var2 + 1] = pt8s4[var1 + 1];
			pt8s4[var2 + 2] = pt8s4[var1 + 2];
			counter2++;
		}
		pt8s4[93] = c;
		pt8s4[94] = a;
		pt8s4[95] = b;
	}
}

Text::WordSize Text::getWordSize(const char *arg1, char *arg2) {
	int32 temp = 0;
	const char *arg2Save = arg2;

	while (*arg1 != '\0' && *arg1 != '\1' && *arg1 != ' ') {
		temp++;
		*arg2++ = *arg1++;
	}

	WordSize size;
	size.inChar = temp;
	*arg2 = '\0';
	size.inPixel = getTextSize(arg2Save);
	return size;
}

void Text::processTextLine() {
	char *buffer = printText8Var8;
	dialCharSpace = 7;
	int16 var4 = 1;

	addLineBreakX = 0;
	printText8PrepareBufferVar2 = 0;
	buf2[0] = 0;

	for (;;) {
		if (*buffer == ' ') {
			buffer++;
			continue;
		}

		if (*buffer != 0) {
			printText8Var8 = buffer;
			WordSize wordSize = getWordSize(buffer, buf1);
			if (addLineBreakX + dialCharSpace + wordSize.inPixel < dialTextBoxParam2) {
				char *temp = buffer + 1;
				if (*buffer == 1) {
					var4 = 0;
					buffer = temp;
				} else {
					if (*buf1 == '@') {
						var4 = 0;
						buffer = temp;
						if (addLineBreakX == 0) {
							addLineBreakX = 7;
							*((int16 *)buf2) = spaceChar;
						}
						if (buf1[1] == 'P') {
							printText8Var1 = dialTextBoxParam1;
							buffer++;
						}
					} else {
						buffer += wordSize.inChar;
						printText8Var8 = buffer;
						strncat(buf2, buf1, sizeof(buf2));
						strncat(buf2, " ", sizeof(buf2)); // not 100% accurate
						printText8PrepareBufferVar2++;

						addLineBreakX += wordSize.inPixel + dialCharSpace;
						if (*printText8Var8 != 0) {
							printText8Var8++;
							continue;
						}
					}
				}
			}
		}
		break;
	}

	if (printText8PrepareBufferVar2 != 0) {
		printText8PrepareBufferVar2--;
	}

	if (*printText8Var8 != '\0' && var4 == 1) {
		if (printText8PrepareBufferVar2 == 0) {
			printText8PrepareBufferVar2 = 1;
		}
		dialCharSpace += (dialTextBoxParam2 - addLineBreakX) / printText8PrepareBufferVar2;
		printText10Var1 = dialTextBoxParam2 - addLineBreakX - dialTextBoxParam2 - addLineBreakX; // stupid... recheck
	}

	printText8Var8 = buffer;

	printText8Ptr2 = buf2;
}

void Text::printText10Sub() {
	_engine->_renderer->vertexCoordinates[0] = dialTextStopColor;
	_engine->_renderer->vertexCoordinates[1] = dialTextBoxRight - 3;
	_engine->_renderer->vertexCoordinates[2] = dialTextBoxBottom - 24;
	_engine->_renderer->vertexCoordinates[3] = dialTextStopColor;
	_engine->_renderer->vertexCoordinates[4] = dialTextBoxRight - 24;
	_engine->_renderer->vertexCoordinates[5] = dialTextBoxBottom - 3;
	_engine->_renderer->vertexCoordinates[6] = dialTextStartColor;
	_engine->_renderer->vertexCoordinates[7] = _engine->_renderer->vertexCoordinates[1];
	_engine->_renderer->vertexCoordinates[8] = _engine->_renderer->vertexCoordinates[5];

	_engine->_renderer->numOfVertex = 3;

	int32 polyRenderType = 0; // POLYGONTYPE_FLAT
	if (_engine->_renderer->computePolygons(polyRenderType)) {
		_engine->_renderer->renderPolygons(polyRenderType, dialTextStopColor);
	}

	_engine->copyBlockPhys(dialTextBoxRight - 24, dialTextBoxBottom - 24, dialTextBoxRight - 3, dialTextBoxBottom - 3);
}

void Text::printText10Sub2() {
	const int32 currentLetter = printText8Var3 - 1;
	const int32 currentIndex = currentLetter * 3;
	int16 *ptr = pt8s4 + currentIndex;
	int32 counter = printText8Var3;
	int32 fontColor = dialTextStartColor;

	_engine->_system->delayMillis(15);

	while (--counter >= 0) {
		setFontColor(fontColor);
		drawCharacterShadow(*(ptr + 1), *(ptr + 2), (uint8)*ptr, fontColor);
		fontColor -= dialTextStepSize;
		if (fontColor > dialTextStopColor) {
			fontColor = dialTextStopColor;
		}
		ptr -= 3;
	};
}

int32 Text::getCharWidth(uint8 chr) const {
	Common::MemoryReadStream stream(_engine->_resources->fontPtr, _engine->_resources->fontBufSize);
	stream.seek(chr * 4);
	stream.seek(stream.readSint16LE());
	return stream.readByte();
}

int32 Text::getCharHeight(uint8 chr) const {
	Common::MemoryReadStream stream(_engine->_resources->fontPtr, _engine->_resources->fontBufSize);
	stream.seek(chr * 4);
	stream.seek(stream.readSint16LE() + 1);
	return stream.readByte();
}

// TODO: refactor this code
int Text::printText10() {
	if (printTextVar13 == 0) {
		return 0;
	}

	if (*printText8Ptr2 == '\0') {
		if (printText8Var5 != 0) {
			if (newGameVar5 != 0) {
				printText10Sub();
			}
			printTextVar13 = 0;
			return 0;
		}
		if (printText8Var6 != 0) {
			_engine->_interface->blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (const int8 *)_engine->workVideoBuffer.getPixels(), dialTextBoxLeft, dialTextBoxTop, (int8 *)_engine->frontVideoBuffer.getPixels());
			_engine->copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
			printText8Var3 = 0;
			printText8Var6 = 0;
			TEXT_CurrentLetterX = dialTextBoxLeft + 8;
			TEXT_CurrentLetterY = dialTextBoxTop + 8;
		}
		if (*printText8Var8 == '\0') {
			initProgressiveTextBuffer();
			printText8Var5 = 1;
			return 1;
		}
		processTextLine();
	}

	// RECHECK this later
	if (*printText8Ptr2 == '\0') {
		return 1;
	}

	printText8Sub4(TEXT_CurrentLetterX, TEXT_CurrentLetterY, *printText8Ptr2);
	printText10Sub2();
	int8 charWidth = getCharWidth(*printText8Ptr2);

	if (*printText8Ptr2 != ' ') {
		TEXT_CurrentLetterX += charWidth + 2;
	} else {
		if (printText10Var1 != 0) {
			TEXT_CurrentLetterX++;
			printText10Var1--;
		}
		TEXT_CurrentLetterX += dialCharSpace;
	}

	// next character
	printText8Ptr2++;

	if (*printText8Ptr2 != '\0') {
		return 1;
	}

	TEXT_CurrentLetterY += 38;
	TEXT_CurrentLetterX = dialTextBoxLeft + 8;

	if (printText8Var6 == 1 && printText8Var5 == 0) {
		printText10Sub();
		return 2;
	}

	printText8Var1++;
	if (printText8Var1 < dialTextBoxParam1) {
		return 1;
	}

	initProgressiveTextBuffer();
	printText8Var6 = 1;

	if (*printText8Var8 == '\0') {
		printText8Var5 = 1;
	}

	return 1;
}

// TODO: refactor this code
bool Text::drawTextFullscreen(int32 index) {
	ScopedKeyMap scopedKeyMap(_engine, cutsceneKeyMapId);

	_engine->_interface->saveClip();
	_engine->_interface->resetClip();
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	// get right VOX entry index
	initVoxToPlay(index);

	bool aborted = false;

	// if we don't display text, than still plays vox file
	if (_engine->cfgfile.FlagDisplayText) {
		initText(index);
		initDialogueBox();

		int32 printedText;
		for (;;) {
			_engine->readKeys();
			printedText = printText10();
			playVox(currDialTextEntry);

			if (!printedText && !_engine->_sound->isSamplePlaying(currDialTextEntry)) {
				break;
			}

			if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
				aborted = true;
				break;
			}
			_engine->_system->delayMillis(1);
		}
		hasHiddenVox = false;

		stopVox(currDialTextEntry);

		printTextVar13 = 0;

		if (printedText != 0) {
			_engine->_interface->loadClip();
			return aborted;
		}

		// wait displaying text
		for (;;) {
			_engine->readKeys();
			if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
				aborted = true;
				break;
			}
			_engine->_system->delayMillis(1);
		}
	} else { // RECHECK THIS
		while (playVox(currDialTextEntry)) {
			_engine->readKeys();
			if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
				aborted = true;
				break;
			}
			_engine->_system->delayMillis(1);
		}
		hasHiddenVox = false;
		voxHiddenIndex = 0;
	}

	stopVox(currDialTextEntry);

	_engine->_interface->loadClip();
	return aborted;
}

void Text::setFontParameters(int32 spaceBetween, int32 charSpace) {
	dialSpaceBetween = spaceBetween;
	dialCharSpace = charSpace;
}

void Text::setFontCrossColor(int32 color) {
	dialTextStepSize = -1;
	dialTextBufferSize = 14;
	dialTextStartColor = color << 4;
	dialTextStopColor = (color << 4) + 12;
}

void Text::setFontColor(int32 color) {
	dialTextColor = color;
}

void Text::setTextCrossColor(int32 stopColor, int32 startColor, int32 stepSize) {
	dialTextStartColor = startColor;
	dialTextStopColor = stopColor;
	dialTextStepSize = stepSize;
	dialTextBufferSize = ((startColor - stopColor) + 1) / stepSize;
}

bool Text::getText(int32 index) {
	int32 currIdx = 0;
	int32 orderIdx = 0;

	const int16 *localTextBuf = (const int16 *)dialTextPtr;
	const int16 *localOrderBuf = (const int16 *)dialOrderPtr;

	int32 numEntries = numDialTextEntries;

	// choose right text from order index
	do {
		orderIdx = *(localOrderBuf++);
		if (orderIdx == index) {
			break;
		}
		currIdx++;
	} while (currIdx < numDialTextEntries);

	if (currIdx >= numEntries) {
		return false;
	}

	int32 ptrCurrentEntry = READ_LE_INT16(&localTextBuf[currIdx]);
	int32 ptrNextEntry = READ_LE_INT16(&localTextBuf[currIdx + 1]);

	currDialTextPtr = (dialTextPtr + ptrCurrentEntry);
	currDialTextSize = ptrNextEntry - ptrCurrentEntry;
	numDialTextEntries = numEntries;

	// RECHECK: this was added for vox playback
	currDialTextEntry = currIdx;

	return true;
}

void Text::copyText(const char *src, char *dst, int32 size) {
	for (int32 i = 0; i < size; i++) {
		*(dst++) = *(src++);
	}
}

bool Text::getMenuText(int32 index, char *text, uint32 textSize) {
	if (index == _engine->_menu->currMenuTextIndex) {
		if (_engine->_menu->currMenuTextBank == _engine->_scene->sceneTextBank) {
			Common::strlcpy(text, _engine->_menu->currMenuTextBuffer, textSize);
			return true;
		}
	}
	if (!getText(index)) {
		// if doesn't have text
		text[0] = '\0';
		return false;
	}

	if ((currDialTextSize - 1) > 0xFF) {
		currDialTextSize = 0xFF;
	}

	copyText(currDialTextPtr, text, currDialTextSize);
	currDialTextSize++;
	copyText(text, _engine->_menu->currMenuTextBuffer, currDialTextSize);

	_engine->_menu->currMenuTextIndex = index;
	_engine->_menu->currMenuTextBank = _engine->_scene->sceneTextBank;
	return true;
}

void Text::textClipFull() { // newGame2
	dialTextBoxLeft = 8;
	dialTextBoxTop = 8;
	dialTextBoxRight = 631;

	dialTextBoxBottom = 471;
	dialTextBoxParam1 = 11;
	dialTextBoxParam2 = 607;
}

void Text::textClipSmall() { // newGame4
	dialTextBoxLeft = 16;
	dialTextBoxTop = 334;
	dialTextBoxRight = 623;
	dialTextBoxBottom = 463;
	dialTextBoxParam1 = 3;
	dialTextBoxParam2 = 591;
}

void Text::drawAskQuestion(int32 index) {
	int32 textStatus = 1;

	// get right VOX entry index
	initVoxToPlay(index);

	initText(index);
	initDialogueBox();

	do {
		_engine->readKeys();
		textStatus = printText10();

		if (textStatus == 2) {
			do {
				_engine->readKeys();
				if (_engine->shouldQuit()) {
					break;
				}
				if (!playVoxSimple(currDialTextEntry)) {
					break;
				}
				_engine->_system->delayMillis(1);
			} while (!_engine->_input->toggleAbortAction());
		}

		_engine->_system->delayMillis(1);
	} while (textStatus);

	while (playVoxSimple(currDialTextEntry)) {
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			stopVox(currDialTextEntry);
			break;
		}
		_engine->_system->delayMillis(1);
	}

	hasHiddenVox = false;
	voxHiddenIndex = 0;
	printTextVar13 = 0;
}

} // namespace TwinE
