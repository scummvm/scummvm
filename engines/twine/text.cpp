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
#include "common/text-to-speech.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/parser/text.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA movie extension */
#define VOX_EXT ".vox"

static const int32 PADDING = 8;

Text::Text(TwinEEngine *engine) : _engine(engine) {
	Common::fill(&_currMenuTextBuffer[0], &_currMenuTextBuffer[256], 0);
}

Text::~Text() {
}

void Text::initVoxBank(TextBankId bankIdx) {
	static const char *LanguageSuffixTypes[] = {
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
	if ((int)bankIdx < 0 || (int)bankIdx >= ARRAYSIZE(LanguageSuffixTypes)) {
		error("bankIdx is out of bounds: %i", (int)bankIdx);
	}
	// get the correct vox hqr file
	currentVoxBankFile = Common::String::format("%s%s" VOX_EXT, LanguageTypes[_engine->cfgfile.LanguageId].id, LanguageSuffixTypes[(int)bankIdx]);
	// TODO: loop through other languages and take the scummvm settings regarding voices into account...

	// TODO check the rest to reverse
}

bool Text::initVoxToPlayTextId(TextId textId) {
	const TextEntry *text = _engine->_resources->getText(_currentBankIdx, textId);
	return initVoxToPlay(text);
}

bool Text::initVoxToPlay(const TextEntry *text) {
	currDialTextEntry = text;
	voxHiddenIndex = 0;
	hasHiddenVox = false;

	if (text == nullptr) {
		return false;
	}

	if (!_engine->cfgfile.Voice) {
		debug(3, "Voices are disabled");
		return false;
	}

	return _engine->_sound->playVoxSample(currDialTextEntry);
}

bool Text::playVox(const TextEntry *text) {
	if (!_engine->cfgfile.Voice) {
		return false;
	}
	if (text == nullptr) {
		return false;
	}
	if (hasHiddenVox && !_engine->_sound->isSamplePlaying(text->index)) {
		_engine->_sound->playVoxSample(text);
		return true;
	}

	return false;
}

bool Text::playVoxSimple(const TextEntry *text) {
	if (text == nullptr) {
		return false;
	}
	if (_engine->_sound->isSamplePlaying(text->index)) {
		return true;
	}
	return playVox(text);
}

bool Text::stopVox(const TextEntry *text) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr) {
		ttsMan->stop();
	}
	if (text == nullptr) {
		return false;
	}
	if (!_engine->_sound->isSamplePlaying(text->index)) {
		return false;
	}
	hasHiddenVox = false;
	_engine->_sound->stopSample(text->index);
	return true;
}

void Text::initTextBank(TextBankId bankIdx) {
	// don't load if we already have the dialogue text bank loaded
	if (bankIdx == _currentBankIdx) {
		return;
	}

	_currentBankIdx = bankIdx;
	initVoxBank(bankIdx);
}

void Text::initSceneTextBank() {
	initTextBank((TextBankId)((int)_engine->_scene->sceneTextBank + (int)TextBankId::Citadel_Island));
}

void Text::drawCharacter(int32 x, int32 y, uint8 character) {
	Common::MemoryReadStream stream(_engine->_resources->fontPtr, _engine->_resources->fontBufSize);
	stream.seek(character * 4);
	stream.seek(stream.readSint16LE());
	/*uint8 charWidth =*/ stream.readByte();
	const uint8 sizeY = stream.readByte();
	x += stream.readByte();
	y += stream.readByte();

	const uint8 usedColor = _dialTextColor;

	int32 tempX = x;
	int32 tempY = y;

	for (uint8 fontY = 0; fontY < sizeY; ++fontY) {
		uint8 index = stream.readByte();
		do {
			const uint8 jump = stream.readByte();
			if (--index == 0) {
				tempY++;
				tempX = x;
				break;
			}
			const uint8 number = stream.readByte();
			tempX += jump;
			uint8* basePtr = (uint8 *)_engine->frontVideoBuffer.getBasePtr(tempX, tempY);
			for (uint8 i = 0; i < number; i++) {
				if (tempX >= 0 && tempX < (_engine->width() - 1) && tempY >= 0 && tempY < (_engine->height() - 1)) {
					*basePtr = usedColor;
				}

				++basePtr;
				tempX++;
			}

			if (--index == 0) {
				tempY++;
				tempX = x;
				break;
			}
		} while (1);
	}
}

void Text::drawCharacterShadow(int32 x, int32 y, uint8 character, int32 color, Common::Rect &dirtyRect) {
	if (character == ' ') {
		return;
	}
	// shadow color
	setFontColor(COLOR_BLACK);
	drawCharacter(x + 2, y + 4, character);

	// text color
	setFontColor(color);
	drawCharacter(x, y, character);

	// TODO: get font size
	const Common::Rect rect(x, y, x + 32, y + 38);
	if (dirtyRect.isEmpty()) {
		dirtyRect = rect;
	} else {
		dirtyRect.extend(rect);
	}
}

void Text::drawText(int32 x, int32 y, const char *dialogue) {
	// if the font is not defined
	if (_engine->_resources->fontPtr == nullptr) {
		return;
	}

	do {
		const uint8 currChar = (uint8)*dialogue++; // read the next char from the string
		if (currChar == '\0') {
			break;
		}

		if (currChar == ' ') {
			x += _dialCharSpace;
		} else {
			const int32 dialTextSize = getCharWidth(currChar);
			drawCharacter(x, y, currChar); // draw the character on screen
			// add the length of the space between 2 characters
			x += _dialSpaceBetween;
			// add the length of the current character
			x += dialTextSize;
		}
	} while (1);
}

int32 Text::getTextSize(const char *dialogue) {
	int32 dialTextSize = 0;

	do {
		const uint8 currChar = (uint8) * (dialogue++);
		if (currChar == '\0') {
			break;
		}

		if (currChar == ' ') {
			dialTextSize += _dialCharSpace;
		} else {
			dialTextSize += _dialSpaceBetween;
			dialTextSize += getCharWidth(currChar);
		}
	} while (1);

	return dialTextSize;
}

void Text::initDialogueBox() {
	_engine->_interface->blitBox(_dialTextBox, _engine->workVideoBuffer, _engine->frontVideoBuffer);

	if (drawTextBoxBackground) {
		_engine->_menu->drawBox(_dialTextBox);
		Common::Rect rect(_dialTextBox);
		rect.grow(-1);
		_engine->_interface->drawTransparentBox(rect, 3);
	}

	_engine->copyBlockPhys(_dialTextBox);
	_fadeInCharactersPos = 0;
	_engine->_interface->blitBox(_dialTextBox, _engine->frontVideoBuffer, _engine->workVideoBuffer);
}

void Text::initInventoryDialogueBox() {
	_engine->_interface->blitBox(_dialTextBox, _engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->copyBlockPhys(_dialTextBox);
	_fadeInCharactersPos = 0;
}

void Text::initInventoryText(InventoryItems index) {
	// 100 if the offset for the inventory item descriptions
	initText((TextId)(100 + (int)index));
}

void Text::initItemFoundText(InventoryItems index) {
	initText((TextId)(100 + (int)index));
}

void Text::initText(TextId index) {
	if (!getText(index)) {
		_hasValidTextHandle = false;
		return;
	}

	_progressiveTextBufferPtr = _progressiveTextBuffer;

	_hasValidTextHandle = true;

	_dialTextBoxCurrentLine = 0;
	_progressiveTextBuffer[0] = '\0';
	_fadeInCharactersPos = 0;
	_dialTextXPos = _dialTextBox.left + PADDING;
	_dialTextYPos = _dialTextBox.top + PADDING;
	_currentTextPosition = _currDialTextPtr;

	// lba font is get while engine start
	setFontParameters(2, 7);

	// fetch the first line
	processTextLine();
}

void Text::initProgressiveTextBuffer() {
	Common::fill(&_progressiveTextBuffer[0], &_progressiveTextBuffer[sizeof(_progressiveTextBuffer)], ' ');
	// the end of the buffer defines how fast the next page is shown - as the
	// whitespaces are handled in the fade in process, too. But we need at least 32 chars,
	// to completly fade in the last characters of a full page (see TEXT_MAX_FADE_IN_CHR)
	_progressiveTextBuffer[sizeof(_progressiveTextBuffer) - 1] = '\0';
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
	const char *buffer = _currentTextPosition;
	_dialCharSpace = 7;
	bool moreWordsFollowing = true;

	int32 lineBreakX = 0;
	int32 spaceCharCount = 0;
	_progressiveTextBuffer[0] = '\0';

	for (;;) {
		if (*buffer == ' ') {
			buffer++;
			continue;
		}
		if (*buffer == '\0') {
			break;
		}

		_currentTextPosition = buffer;
		char wordBuf[256] = "";
		WordSize wordSize = getWordSize(buffer, wordBuf, sizeof(wordBuf));
		if (lineBreakX + _dialCharSpace + wordSize.inPixel >= _dialTextBoxMaxX) {
			break;
		}

		if (*wordBuf == '\1') {
			moreWordsFollowing = false;
			buffer++;
			break;
		}

		// @ is a line break
		if (*wordBuf == '@') {
			moreWordsFollowing = false;
			buffer++;
			if (lineBreakX == 0) {
				lineBreakX = 7;
				*(_progressiveTextBuffer + 0) = ' ';
				*(_progressiveTextBuffer + 1) = '\0';
			}
			// new page?
			if (wordBuf[1] == 'P') {
				_dialTextBoxCurrentLine = _dialTextBoxLines;
				buffer++;
			}
			break;
		}

		buffer += wordSize.inChar;
		_currentTextPosition = buffer;
		strncat(_progressiveTextBuffer, wordBuf, sizeof(_progressiveTextBuffer) - strlen(_progressiveTextBuffer) - 1);
		strncat(_progressiveTextBuffer, " ", sizeof(_progressiveTextBuffer) - strlen(_progressiveTextBuffer) - 1);
		spaceCharCount++;

		lineBreakX += wordSize.inPixel + _dialCharSpace;
		if (*_currentTextPosition != '\0') {
			_currentTextPosition++;
			continue;
		}
		break;
	}

	if (spaceCharCount > 0) {
		spaceCharCount--;
	}

	if (*_currentTextPosition != '\0' && moreWordsFollowing) {
		if (spaceCharCount <= 0) {
			spaceCharCount = 1;
		}
		// split the remaining space between the words
		_dialCharSpace += (_dialTextBoxMaxX - lineBreakX) / spaceCharCount;
	}

	_currentTextPosition = buffer;

	_progressiveTextBufferPtr = _progressiveTextBuffer;
}

void Text::renderContinueReadingTriangle() {
	const int32 border = 3;
	const int32 size = 21;
	const int16 color = 136;
	const int32 right = _dialTextBox.right - border;
	const int32 left = _dialTextBox.right - (size + border);
	const int32 top = _dialTextBox.bottom - (size + border);
	const int32 bottom = _dialTextBox.bottom - border;

	Vertex vertices[3];

	vertices[0].colorIndex = color;
	vertices[0].x = right;
	vertices[0].y = top;

	vertices[1].colorIndex = color;
	vertices[1].x = left;
	vertices[1].y = bottom;

	vertices[2].colorIndex = color;
	vertices[2].x = right;
	vertices[2].y = bottom;

	CmdRenderPolygon polygon;
	polygon.numVertices = ARRAYSIZE(vertices);
	polygon.colorIndex = _dialTextStopColor;
	polygon.renderType = POLYGONTYPE_FLAT;
	_engine->_renderer->renderPolygons(polygon, vertices, top, bottom);

	_engine->copyBlockPhys(Common::Rect(left, top, right, bottom));
}

void Text::fadeInCharacters(int32 counter, int32 fontColor) {
	Common::Rect dirtyRect;
	while (--counter >= 0) {
		const BlendInCharacter *ptr = &_fadeInCharacters[counter];
		setFontColor(fontColor);
		drawCharacterShadow(ptr->x, ptr->y, ptr->chr, fontColor, dirtyRect);
		fontColor -= _dialTextStepSize;
		if (fontColor > _dialTextStopColor) {
			fontColor = _dialTextStopColor;
		}
	}
	if (dirtyRect.isEmpty()) {
		return;
	}
	_engine->copyBlockPhys(dirtyRect);
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

void Text::fadeInRemainingChars() {
	if (_fadeInCharactersPos <= 0) {
		return;
	}
	fadeInCharacters(_fadeInCharactersPos, _dialTextStopColor);
	--_fadeInCharactersPos;
}

ProgressiveTextState Text::updateProgressiveText() {
	if (!_hasValidTextHandle) {
		return ProgressiveTextState::End;
	}

	if (*_progressiveTextBufferPtr == '\0') {
		initProgressiveTextBuffer();
		processTextLine();
		initDialogueBox();
		_dialTextXPos = _dialTextBox.left + PADDING;
		_dialTextYPos = _dialTextBox.top + PADDING;
	}
	const char currentChar = *_progressiveTextBufferPtr;
	assert(currentChar != '\0');
	fillFadeInBuffer(_dialTextXPos, _dialTextYPos, currentChar);
	fadeInCharacters(_fadeInCharactersPos, _dialTextStartColor);
	const int8 charWidth = getCharWidth(currentChar);

	if (currentChar == ' ') {
		_dialTextXPos += _dialCharSpace + 1;
	} else {
		_dialTextXPos += charWidth + 2;
	}

	// next character
	_progressiveTextBufferPtr++;

	// reaching 0-byte means a new line - as we are fading in per line
	if (*_progressiveTextBufferPtr != '\0') {
		return ProgressiveTextState::ContinueRunning;
	}

	if (*_currentTextPosition == '\0') {
		_hasValidTextHandle = false;
		renderContinueReadingTriangle();
		return ProgressiveTextState::End;
	}

	// reached a new line that is about get faded in
	_dialTextBoxCurrentLine++;

	_dialTextYPos += lineHeight;
	_dialTextXPos = _dialTextBox.left + PADDING;

	if (_dialTextBoxCurrentLine >= _dialTextBoxLines) {
		renderContinueReadingTriangle();
		return ProgressiveTextState::NextPage;
	}

	processTextLine();

	return ProgressiveTextState::ContinueRunning;
}

bool Text::displayText(TextId index, bool showText, bool playVox, bool loop) {
	debug(3, "displayText(index = %i, showText = %s, playVox = %s)",
		(int)index, showText ? "true" : "false", playVox ? "true" : "false");
	if (playVox) {
		const TextEntry *textEntry = _engine->_resources->getText(_currentBankIdx, index);
		// get right VOX entry index
		initVoxToPlay(textEntry);
	}

	bool aborted = false;

	// if we don't display text, than still plays vox file
	if (showText) {
		initText(index);
		initDialogueBox();

		ScopedKeyMap uiKeyMap(_engine, uiKeyMapId);
		ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;
		for (;;) {
			FrameMarker frame;
			ScopedFPS scopedFps(66);
			_engine->readKeys();
			if (textState == ProgressiveTextState::ContinueRunning) {
				textState = updateProgressiveText();
			} else {
				fadeInRemainingChars();
			}

			if (!loop) {
				if (textState == ProgressiveTextState::End) {
					fadeInRemainingChars();
					break;
				}
				if (textState == ProgressiveTextState::NextPage) {
					textState = ProgressiveTextState::ContinueRunning;
				}
			}

			if (_engine->_input->toggleActionIfActive(TwinEActionType::UINextPage)) {
				if (textState == ProgressiveTextState::End) {
					stopVox(currDialTextEntry);
					break;
				}
				if (textState == ProgressiveTextState::NextPage) {
					textState = ProgressiveTextState::ContinueRunning;
				}
			}
			if (_engine->_input->toggleAbortAction() || _engine->shouldQuit()) {
				stopVox(currDialTextEntry);
				aborted = true;
				break;
			}

			if (playVox) {
				playVoxSimple(currDialTextEntry);
			}
		}
	}
	while (playVox && playVoxSimple(currDialTextEntry)) {
		FrameMarker frame;
		ScopedFPS scopedFps;
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			stopVox(currDialTextEntry);
			aborted = true;
			break;
		}
	}
	voxHiddenIndex = 0;
	hasHiddenVox = false;
	_hasValidTextHandle = false;
	_engine->_input->resetHeroActions();

	return aborted;
}

bool Text::drawTextProgressive(TextId index, bool playVox, bool loop) {
	_engine->exitSceneryView();
	_engine->_interface->saveClip();
	_engine->_interface->resetClip();
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	const bool aborted = displayText(index, _engine->cfgfile.FlagDisplayText, playVox, loop);
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
	_dialTextStartColor = color * 16;
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

bool Text::getText(TextId index) {
	const TextEntry *textEntry = _engine->_resources->getText(_currentBankIdx, index);
	if (textEntry == nullptr) {
		return false;
	}
	_currDialTextPtr = textEntry->string.c_str();
	_currDialTextSize = textEntry->string.size();

	// RECHECK: this was added for vox playback
	currDialTextEntry = textEntry;

	debug(3, "text for bank %i with index %i (currIndex: %i): %s", (int)_currentBankIdx, textEntry->index, (int)textEntry->textIndex, _currDialTextPtr);
	return true;
}

bool Text::getMenuText(TextId index, char *text, uint32 textSize) {
	if (index == _currMenuTextIndex) {
		if (_currMenuTextBank == _engine->_scene->sceneTextBank) {
			Common::strlcpy(text, _currMenuTextBuffer, textSize);
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

	Common::strlcpy(text, _currDialTextPtr, MIN<int32>(textSize, _currDialTextSize + 1));
	_currDialTextSize++;
	Common::strlcpy(_currMenuTextBuffer, text, MIN<int32>(sizeof(_currMenuTextBuffer), _currDialTextSize));

	_currMenuTextIndex = index;
	_currMenuTextBank = _engine->_scene->sceneTextBank;
	return true;
}

void Text::textClipFull() {
	const int32 margin = 8;
	_dialTextBox.left = margin;
	_dialTextBox.top = margin;
	_dialTextBox.right = _engine->width() - margin;
	_dialTextBox.bottom = _engine->height() - margin;

	_dialTextBoxLines = (int32)(_dialTextBox.height() / lineHeight) - 1;
	_dialTextBoxMaxX = _engine->width() - 2 * margin - 2 * PADDING;
}

void Text::textClipSmall() {
	const int32 margin = 16;
	_dialTextBoxLines = 3;
	const int32 textHeight = _dialTextBoxLines * lineHeight;

	_dialTextBox.left = margin;
	_dialTextBox.top = _engine->height() - textHeight - margin - PADDING;
	_dialTextBox.right = _engine->width() - margin;
	_dialTextBox.bottom = _engine->height() - margin;

	_dialTextBoxMaxX = _engine->width() - 2 * margin - 2 * PADDING;
}

void Text::drawAskQuestion(TextId index) {
	displayText(index, true, true, true);
}

void Text::drawHolomapLocation(TextId index) {
	textClipSmall();
	setFontCrossColor(COLOR_WHITE);
	_engine->_interface->drawFilledRect(_dialTextBox, COLOR_BLACK);
	const bool displayText = _engine->cfgfile.FlagDisplayText;
	_engine->cfgfile.FlagDisplayText = true;
	drawTextProgressive(index, false, false);
	_engine->cfgfile.FlagDisplayText = displayText;
}

} // namespace TwinE
