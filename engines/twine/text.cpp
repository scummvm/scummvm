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
#include "common/algorithm.h"
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
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA movie extension */
#define VOX_EXT ".vox"

#define INDEXOFFSET 0
#define DIALOGSOFFSET 1

Text::~Text() {
	free(dialTextPtr);
	free(dialOrderPtr);
}

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

	Common::MemoryReadStream stream((const byte *)dialOrderPtr, dialOrderSize);
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

	const uint8 usedColor = _dialTextColor;

	uint8 *screen2 = (uint8 *)_engine->frontVideoBuffer.getBasePtr(x, y);

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
	if (character == ' ') {
		return;
	}
	// shadow color
	setFontColor(0);
	drawCharacter(x + 2, y + 4, character);

	// text color
	setFontColor(color);
	drawCharacter(x, y, character);

	int32 left = x;
	int32 top = y;
	// FIXME: get right font size
	int32 right = x + 32;
	int32 bottom = y + 38;

	_engine->copyBlockPhys(left, top, right, bottom);
}

void Text::drawText(int32 x, int32 y, const char *dialogue) {
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
			x += _dialCharSpace;
		} else {
			_dialTextSize = getCharWidth(currChar);
			drawCharacter(x, y, currChar); // draw the character on screen
			// add the length of the space between 2 characters
			x += _dialSpaceBetween;
			// add the length of the current character
			x += _dialTextSize;
		}
	} while (1);
}

int32 Text::getTextSize(const char *dialogue) { // SizeFont
	_dialTextSize = 0;

	do {
		const uint8 currChar = (uint8) * (dialogue++);
		if (currChar == 0) {
			break;
		}

		if (currChar == ' ') {
			_dialTextSize += _dialCharSpace;
		} else {
			_dialTextSize += _dialSpaceBetween;
			_dialTextSize += getCharWidth(currChar);
		}
	} while (1);

	return _dialTextSize;
}

void Text::initDialogueBox() { // InitDialWindow
	_engine->_interface->blitBox(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom, _engine->workVideoBuffer, _dialTextBoxLeft, _dialTextBoxTop, _engine->frontVideoBuffer);

	if (drawTextBoxBackground) {
		_engine->_menu->drawBox(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom);
		_engine->_interface->drawTransparentBox(_dialTextBoxLeft + 1, _dialTextBoxTop + 1, _dialTextBoxRight - 1, _dialTextBoxBottom - 1, 3);
	}

	_engine->copyBlockPhys(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom);
	_fadeInCharactersPos = 0;
	_engine->_interface->blitBox(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom, _engine->frontVideoBuffer, _dialTextBoxLeft, _dialTextBoxTop, _engine->workVideoBuffer);
}

void Text::initInventoryDialogueBox() { // SecondInitDialWindow
	_engine->_interface->blitBox(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom, _engine->workVideoBuffer, _dialTextBoxLeft, _dialTextBoxTop, _engine->frontVideoBuffer);
	_engine->copyBlockPhys(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom);
	_fadeInCharactersPos = 0;
}

// TODO: refactor this code
void Text::initText(int32 index) {
	if (!getText(index)) {
		_hasValidTextHandle = false;
		return;
	}

	_progressiveTextBufferPtr = _progressiveTextBuffer;

	_hasValidTextHandle = true;

	_dialTextBoxCurrentLine = 0;
	_progressiveTextBuffer[0] = '\0';
	_fadeInCharactersPos = 0;
	_dialTextYPos = _dialTextBoxLeft + 8;
	_progressiveTextEnd = false;
	_progressiveTextNextPage = false;
	_dialTextXPos = _dialTextBoxTop + 8;
	printText8Var8 = _currDialTextPtr;

	// lba font is get while engine start
	setFontParameters(2, 7);
}

void Text::initProgressiveTextBuffer() {
	Common::fill(&_progressiveTextBuffer[0], &_progressiveTextBuffer[256], ' ');
	_progressiveTextBuffer[255] = '\0';
	_progressiveTextBufferPtr = _progressiveTextBuffer;
	_dialTextBoxCurrentLine = 0;
}

void Text::fillFadeInBuffer(int16 x, int16 y, int16 chr) {
	if (_fadeInCharactersPos < TEXT_MAX_FADE_IN_CHR) {
		_fadeInCharacters[_fadeInCharactersPos].chr = chr;
		_fadeInCharacters[_fadeInCharactersPos].x = x;
		_fadeInCharacters[_fadeInCharactersPos].y = y;
		_fadeInCharactersPos++;
		return;
	}
	int32 counter2 = 0;
	while (counter2 < TEXT_MAX_FADE_IN_CHR - 1) {
		const int32 var1 = (counter2 + 1);
		const int32 var2 = counter2;
		_fadeInCharacters[var2] = _fadeInCharacters[var1];
		counter2++;
	}
	_fadeInCharacters[TEXT_MAX_FADE_IN_CHR - 1].chr = chr;
	_fadeInCharacters[TEXT_MAX_FADE_IN_CHR - 1].x = x;
	_fadeInCharacters[TEXT_MAX_FADE_IN_CHR - 1].y = y;
}

Text::WordSize Text::getWordSize(const char *completeText, char *wordBuf, int32 wordBufSize) {
	int32 temp = 0;
	const char *arg2Save = wordBuf;

	while (*completeText != '\0' && *completeText != '\1' && *completeText != ' ') {
		temp++;
		*wordBuf++ = *completeText++;
		if (temp >= wordBufSize - 1) {
			break;
		}
	}

	WordSize size;
	size.inChar = temp;
	*wordBuf = '\0';
	size.inPixel = getTextSize(arg2Save);
	return size;
}

void Text::processTextLine() {
	char *buffer = printText8Var8;
	_dialCharSpace = 7;
	bool var4 = true;

	int32 lineBreakX = 0;
	int32 printText8PrepareBufferVar2 = 0;
	_progressiveTextBuffer[0] = '\0';

	for (;;) {
		if (*buffer == ' ') {
			buffer++;
			continue;
		}
		if (*buffer == '\0') {
			break;
		}

		printText8Var8 = buffer;
		char wordBuf[256] = "";
		WordSize wordSize = getWordSize(buffer, wordBuf, sizeof(wordBuf));
		if (lineBreakX + _dialCharSpace + wordSize.inPixel >= _dialTextBoxParam2) {
			break;
		}

		if (*buffer == '\1') {
			var4 = false;
			buffer++;
			break;
		}

		if (*wordBuf == '@') {
			var4 = false;
			buffer++;
			if (lineBreakX == 0) {
				lineBreakX = 7;
				*(_progressiveTextBuffer + 0) = ' ';
				*(_progressiveTextBuffer + 1) = ' ';
			}
			if (wordBuf[1] == 'P') {
				_dialTextBoxCurrentLine = _dialTextBoxLines;
				buffer++;
			}
			break;
		}

		buffer += wordSize.inChar;
		printText8Var8 = buffer;
		strncat(_progressiveTextBuffer, wordBuf, sizeof(_progressiveTextBuffer) - strlen(_progressiveTextBuffer) - 1);
		strncat(_progressiveTextBuffer, " ", sizeof(_progressiveTextBuffer) - strlen(_progressiveTextBuffer) - 1); // not 100% accurate
		printText8PrepareBufferVar2++;

		lineBreakX += wordSize.inPixel + _dialCharSpace;
		if (*printText8Var8 != '\0') {
			printText8Var8++;
			continue;
		}
		break;
	}

	if (printText8PrepareBufferVar2 != 0) {
		printText8PrepareBufferVar2--;
	}

	if (*printText8Var8 != '\0' && var4) {
		if (printText8PrepareBufferVar2 == 0) {
			printText8PrepareBufferVar2 = 1;
		}
		_dialCharSpace += (_dialTextBoxParam2 - lineBreakX) / printText8PrepareBufferVar2;
		printText10Var1 = -2 * lineBreakX;
	}

	printText8Var8 = buffer;

	_progressiveTextBufferPtr = _progressiveTextBuffer;
}

void Text::renderContinueReadingTriangle() {
	const int32 right = _dialTextBoxRight - 3;
	const int32 left = _dialTextBoxRight - 24;
	const int32 top = _dialTextBoxBottom - 24;
	const int32 bottom = _dialTextBoxBottom - 3;

	_engine->_renderer->vertexCoordinates[0] = _dialTextStopColor;
	_engine->_renderer->vertexCoordinates[1] = right;
	_engine->_renderer->vertexCoordinates[2] = top;

	_engine->_renderer->vertexCoordinates[3] = _dialTextStopColor;
	_engine->_renderer->vertexCoordinates[4] = left;
	_engine->_renderer->vertexCoordinates[5] = bottom;

	_engine->_renderer->vertexCoordinates[6] = _dialTextStartColor;
	_engine->_renderer->vertexCoordinates[7] = _engine->_renderer->vertexCoordinates[1];
	_engine->_renderer->vertexCoordinates[8] = _engine->_renderer->vertexCoordinates[5];

	_engine->_renderer->numOfVertex = 3;

	_engine->_renderer->renderPolygons(POLYGONTYPE_FLAT, _dialTextStopColor);

	_engine->copyBlockPhys(left, top, right, bottom);
}

void Text::fadeInCharacters(int32 counter, int32 fontColor) {
	_engine->_system->delayMillis(15);
	while (--counter >= 0) {
		const BlendInCharacter *ptr = &_fadeInCharacters[counter];
		setFontColor(fontColor);
		drawCharacterShadow(ptr->x, ptr->y, ptr->chr, fontColor);
		fontColor -= _dialTextStepSize;
		if (fontColor > _dialTextStopColor) {
			fontColor = _dialTextStopColor;
		}
	}
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
int Text::updateProgressiveText() {
	if (!_hasValidTextHandle) {
		return 0;
	}

	if (*_progressiveTextBufferPtr == '\0') {
		if (_progressiveTextEnd) {
			if (renderTextTriangle) {
				renderContinueReadingTriangle();
			}
			_hasValidTextHandle = false;
			return 0;
		}
		if (_progressiveTextNextPage) {
			_engine->_interface->blitBox(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom, _engine->workVideoBuffer, _dialTextBoxLeft, _dialTextBoxTop, _engine->frontVideoBuffer);
			_engine->copyBlockPhys(_dialTextBoxLeft, _dialTextBoxTop, _dialTextBoxRight, _dialTextBoxBottom);
			_fadeInCharactersPos = 0;
			_progressiveTextNextPage = false;
			_dialTextYPos = _dialTextBoxLeft + 8;
			_dialTextXPos = _dialTextBoxTop + 8;
		}
		if (*printText8Var8 == '\0') {
			initProgressiveTextBuffer();
			_progressiveTextEnd = true;
			return 1;
		}
		processTextLine();
	}

	// RECHECK this later
	if (*_progressiveTextBufferPtr == '\0') {
		return 1;
	}

	fillFadeInBuffer(_dialTextYPos, _dialTextXPos, *_progressiveTextBufferPtr);
	fadeInCharacters(_fadeInCharactersPos, _dialTextStartColor);
	int8 charWidth = getCharWidth(*_progressiveTextBufferPtr);

	if (*_progressiveTextBufferPtr != ' ') {
		_dialTextYPos += charWidth + 2;
	} else {
		if (printText10Var1 != 0) {
			_dialTextYPos++;
			printText10Var1--;
		}
		_dialTextYPos += _dialCharSpace;
	}

	// next character
	_progressiveTextBufferPtr++;

	if (*_progressiveTextBufferPtr != '\0') {
		return 1;
	}

	const int32 lineHeight = 38;
	_dialTextXPos += lineHeight;
	_dialTextYPos = _dialTextBoxLeft + 8;

	if (_progressiveTextNextPage && !_progressiveTextEnd) {
		renderContinueReadingTriangle();
		return 2;
	}

	_dialTextBoxCurrentLine++;
	if (_dialTextBoxCurrentLine < _dialTextBoxLines) {
		return 1;
	}

	initProgressiveTextBuffer();
	_progressiveTextNextPage = true;

	if (*printText8Var8 == '\0') {
		_progressiveTextEnd = true;
	}

	return 1;
}

// TODO: refactor this code
bool Text::drawTextFullscreen(int32 index) {
	ScopedKeyMap scoped(_engine, cutsceneKeyMapId);
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
			printedText = updateProgressiveText();
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

		_hasValidTextHandle = false;

		if (printedText == 0) {
			stopVox(currDialTextEntry);
			// wait displaying text
			for (;;) {
				_engine->readKeys();
				if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
					aborted = true;
					break;
				}
				_engine->_system->delayMillis(1);
			}
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
	_dialSpaceBetween = spaceBetween;
	_dialCharSpace = charSpace;
}

void Text::setFontCrossColor(int32 color) {
	_dialTextStepSize = -1;
	_dialTextBufferSize = 14;
	_dialTextStartColor = color << 4;
	_dialTextStopColor = _dialTextStartColor + 12;
}

void Text::setFontColor(int32 color) {
	_dialTextColor = color;
}

void Text::setTextCrossColor(int32 stopColor, int32 startColor, int32 stepSize) {
	_dialTextStartColor = startColor;
	_dialTextStopColor = stopColor;
	_dialTextStepSize = stepSize;
	_dialTextBufferSize = ((startColor - stopColor) + 1) / stepSize;
}

bool Text::getText(int32 index) {
	const int16 *localTextBuf = (const int16 *)dialTextPtr;
	const int16 *localOrderBuf = (const int16 *)dialOrderPtr;

	const int32 numEntries = numDialTextEntries;
	int32 currIdx = 0;
	// choose right text from order index
	do {
		const int32 orderIdx = *(localOrderBuf++);
		if (orderIdx == index) {
			break;
		}
		currIdx++;
	} while (currIdx < numEntries);

	if (currIdx >= numEntries) {
		return false;
	}

	const int32 ptrCurrentEntry = READ_LE_INT16(&localTextBuf[currIdx]);
	const int32 ptrNextEntry = READ_LE_INT16(&localTextBuf[currIdx + 1]);

	_currDialTextPtr = dialTextPtr + ptrCurrentEntry;
	_currDialTextSize = ptrNextEntry - ptrCurrentEntry;

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

	if (_currDialTextSize - 1 > 0xFF) {
		_currDialTextSize = 0xFF;
	}

	copyText(_currDialTextPtr, text, _currDialTextSize);
	_currDialTextSize++;
	copyText(text, _engine->_menu->currMenuTextBuffer, _currDialTextSize);

	_engine->_menu->currMenuTextIndex = index;
	_engine->_menu->currMenuTextBank = _engine->_scene->sceneTextBank;
	return true;
}

void Text::textClipFull() {
	const int padding = 9;
	_dialTextBoxLeft = padding - 1;
	_dialTextBoxTop = padding - 1;
	_dialTextBoxRight = SCREEN_WIDTH - padding;
	_dialTextBoxBottom = SCREEN_HEIGHT - padding;

	_dialTextBoxLines = 11;
	_dialTextBoxParam2 = 607;
}

void Text::textClipSmall() {
	const int padding = 17;
	_dialTextBoxLeft = padding - 1;
	_dialTextBoxTop = 334;
	_dialTextBoxRight = SCREEN_WIDTH - padding;
	_dialTextBoxBottom = SCREEN_HEIGHT - padding;

	_dialTextBoxLines = 3;
	_dialTextBoxParam2 = 591;
}

void Text::drawAskQuestion(int32 index) {
	// get right VOX entry index
	initVoxToPlay(index);

	initText(index);
	initDialogueBox();

	int32 textStatus = 1;
	do {
		_engine->readKeys();
		textStatus = updateProgressiveText();

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
	_hasValidTextHandle = false;
}

} // namespace TwinE
