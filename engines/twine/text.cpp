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
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA movie extension */
#define VOX_EXT ".vox"

static const int32 PADDING = 8;
static const int kLBASJISCharWidth = 24;
static const int kLBASJISCharHeight = 24;

Text::Text(TwinEEngine *engine) : _engine(engine) {
	Common::fill(&_currMenuTextBuffer[0], &_currMenuTextBuffer[256], 0);
	_isShiftJIS = _engine->getGameLang() == Common::Language::JA_JPN;
	_isVisualRTL = _engine->getGameLang() == Common::Language::HE_ISR;
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
	    "005", // Hamalayi Mountains - southern range voices
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
	// get the correct vox hqr file - english is the default
	_currentVoxBankFile = Common::String::format("%s%s" VOX_EXT, LanguageTypes[0].id, LanguageSuffixTypes[(int)bankIdx]);
	_currentOggBaseFile = Common::String::format("%s%s_", LanguageTypes[0].id, LanguageSuffixTypes[(int)bankIdx]);

	const int voice = ConfMan.getInt("audio_language");
	const int32 length = ARRAYSIZE(LanguageTypes);
	for (int32 i = 0; i < length; i++) {
		if (LanguageTypes[i].voice == voice) {
			_currentVoxBankFile = Common::String::format("%s%s" VOX_EXT, LanguageTypes[i].id, LanguageSuffixTypes[(int)bankIdx]);
			_currentOggBaseFile = Common::String::format("%s%s_", LanguageTypes[i].id, LanguageSuffixTypes[(int)bankIdx]);
			return;
		}
	}
	warning("Could not find voice mapping for %i", voice);
	// TODO check the rest to reverse
}

bool Text::initVoxToPlayTextId(TextId textId) {
	const TextEntry *text = _engine->_resources->getText(_currentBankIdx, textId);
	return initVoxToPlay(text);
}

bool Text::initVoxToPlay(const TextEntry *text) {
	_currDialTextEntry = text;
	_voxHiddenIndex = 0;
	_hasHiddenVox = false;

	if (text == nullptr) {
		return false;
	}

	const int voice = ConfMan.getInt("audio_language");
	if (voice <= 0) {
		debug(3, "Voices are disabled");
		return false;
	}

	return _engine->_sound->playVoxSample(_currDialTextEntry);
}

bool Text::playVox(const TextEntry *text) {
	if (ConfMan.getInt("audio_language") <= 0) {
		return false;
	}
	if (text == nullptr) {
		return false;
	}
	if (_hasHiddenVox && !_engine->_sound->isSamplePlaying(text->index)) {
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
	_hasHiddenVox = false;
	_engine->_sound->stopSample(text->index);
	return true;
}

void Text::initSceneTextBank() {
	const int textBankId = (int)_engine->_scene->_sceneTextBank;
	initDial((TextBankId)(textBankId + (int)TextBankId::Citadel_Island));
}

void Text::drawCharacter(int32 x, int32 y, uint16 character) { // CarFont
	const uint8 usedColor = _dialTextColor;
	if (_isShiftJIS && character > 0x100 && _engine->_resources->_sjisFontPtr) {
		int index = 0;
		if (character >= 0x8140 && character <= 0x9fff)
			index = character - 0x8140;
		else if (character >= 0xe040 && character <= 0xeaff)
			index = character - 0xe040 + 8320;
		else {
			drawCharacter(x, y, '?');
			return;
		}

		byte *glyphPtr = _engine->_resources->_sjisFontPtr + index * (kLBASJISCharHeight * kLBASJISCharWidth / 8);

		for (uint8 fontY = 0; fontY < kLBASJISCharHeight; ++fontY) {
			byte bits = 0;
			int remBits = 0;
			for (uint8 fontX = 0; fontX < kLBASJISCharWidth; ++fontX) {
				if (remBits == 0) {
					remBits = 8;
					bits = *glyphPtr++;
				}
				int32 tempX = x + fontX;
				int32 tempY = y + fontY;
				if ((bits & 0x80) && tempX >= 0 && tempX < (_engine->width() - 1) && tempY >= 0 && tempY < (_engine->height() - 1)) {
					_engine->_frontVideoBuffer.setPixel(tempX, tempY, usedColor);
				}

				remBits--;
				bits <<= 1;
			}
		}
		return;
	}

	if (character > 0x100)
		character = '?';
	Common::MemoryReadStream stream(_engine->_resources->_fontPtr, _engine->_resources->_fontBufSize);
	stream.seek(character * 4);
	stream.seek(stream.readSint16LE());
	/*uint8 charWidth =*/stream.readByte();
	const uint8 sizeY = stream.readByte();
	x += stream.readByte();
	y += stream.readByte();

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
			uint8 *basePtr = (uint8 *)_engine->_frontVideoBuffer.getBasePtr(tempX, tempY);
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

uint16 Text::getNextChar(const char *&dialogue) {
	uint16 currChar = *dialogue++ & 0xff;
	if (_isShiftJIS && ((currChar >= 0x81 && currChar <= 0x9f) || (currChar >= 0xe0 && currChar <= 0xea)) && ((*dialogue & 0xff) >= 0x40))
		currChar = (currChar << 8) | (*dialogue++ & 0xff);
	return currChar;
}

void Text::drawText(int32 x, int32 y, const char *dialogue, bool shadow) {
	// if the font is not defined
	if (_engine->_resources->_fontPtr == nullptr) {
		return;
	}

	do {
		const uint16 currChar = getNextChar(dialogue); // read the next char from the string
		if (currChar == '\0') {
			break;
		}

		if (currChar == ' ') {
			x += _interSpace;
		} else {
			const int32 dialTextSize = getCharWidth(currChar);
			if (shadow) {
				Common::Rect dirtyRect;
				drawCharacterShadow(x, y, currChar, COLOR_WHITE, dirtyRect);
			} else {
				drawCharacter(x, y, currChar); // draw the character on screen
			}
			// add the length of the space between 2 characters
			x += _interLeave;
			// add the length of the current character
			x += dialTextSize;
		}
	} while (1);
}

int32 Text::sizeFont(const char *dialogue) {
	int32 dialTextSize = 0;

	do {
		const uint16 currChar = getNextChar(dialogue);
		if (currChar == '\0') {
			break;
		}

		if (currChar == ' ') {
			dialTextSize += _interSpace;
		} else {
			dialTextSize += _interLeave;
			dialTextSize += getCharWidth(currChar);
		}
	} while (1);

	return dialTextSize;
}

void Text::initInventoryText(InventoryItems index) { // OpenDialNoWindow
	// 100 if the offset for the inventory item descriptions
	commonOpenDial((TextId)(100 + (int)index));
}

void Text::initItemFoundText(InventoryItems index) { // OpenDialNoWindow
	commonOpenDial((TextId)index);
}

void Text::initDial(TextBankId bankIdx) {
	// don't load if we already have the dialogue text bank loaded
	if (bankIdx == _currentBankIdx) {
		return;
	}

	_currentBankIdx = bankIdx;
	initVoxBank(bankIdx);
}

Text::WordSize Text::getNextWord(const char *completeText, char *dst, int32 wordBufSize) {
	int32 lenword = 0;
	const char *word = dst;

	while (*completeText != '\0' && *completeText != '\1' && *completeText != ' ') {
		lenword++;
		*dst++ = *completeText++;
		if (lenword >= wordBufSize - 1) {
			break;
		}
	}

	*dst = '\0';

	WordSize size;
	size.lenWord = lenword;
	size.sizeWord = sizeFont(word);
	return size;
}

void Text::pushChar(int16 x, int16 y, const LineCharacter &chr) {
	if (_nbChar < TEXT_MAX_FADE_IN_CHR) {
		_stackChar[_nbChar].chr = chr.chr;
		_stackChar[_nbChar].x = x;
		_stackChar[_nbChar].y = y;
		_nbChar++;
		return;
	}
	for (int32 i = 0; i < TEXT_MAX_FADE_IN_CHR - 1; ++i) {
		_stackChar[i] = _stackChar[i + 1];
	}
	_stackChar[TEXT_MAX_FADE_IN_CHR - 1].chr = chr.chr;
	_stackChar[TEXT_MAX_FADE_IN_CHR - 1].x = x;
	_stackChar[TEXT_MAX_FADE_IN_CHR - 1].y = y;
}

void Text::drawCharacterShadow(int32 x, int32 y, uint16 character, int32 color, Common::Rect &dirtyRect) { // AffOneCar
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

void Text::fadeInCharacters(int32 counter, int32 fontColor) { // AffAllCar
	Common::Rect dirtyRect;
	while (--counter >= 0) {
		const BlendInCharacter *ptr = &_stackChar[counter];
		setFontColor(fontColor);
		drawCharacterShadow(ptr->x, ptr->y, ptr->chr, fontColor, dirtyRect);
		fontColor -= _stepDegrade;
		if (fontColor > _minDegrade) {
			fontColor = _minDegrade;
		}
	}
	if (dirtyRect.isEmpty()) {
		return;
	}
	_engine->copyBlockPhys(dirtyRect);
}

void Text::setTextCrossColor(int32 stopColor, int32 startColor, int32 stepSize) { // CoulDial
	_maxDegrade = startColor;
	_minDegrade = stopColor;
	_stepDegrade = stepSize;
	_nbDegrade = ((startColor - stopColor) + 1) / stepSize;
}

void Text::setFontCrossColor(int32 color) { // TestCoulDial
	_stepDegrade = -1;
	_nbDegrade = 14;
	_maxDegrade = color * 16;
	_minDegrade = _maxDegrade + 12;
}

bool Text::getText(TextId index) {
	const TextEntry *textEntry = _engine->_resources->getText(_currentBankIdx, index);
	if (textEntry == nullptr) {
		return false;
	}
	_ptText = textEntry->string.c_str();
	_currDialTextSize = textEntry->string.size();

	// RECHECK: this was added for vox playback
	_currDialTextEntry = textEntry;

	debug(3, "text for bank %i with index %i (currIndex: %i): %s", (int)_currentBankIdx, textEntry->index, (int)textEntry->textIndex, _ptText);
	return true;
}

void Text::renderContinueReadingTriangle() { // AffFleche
	const int32 border = 3;
	const int32 size = 21;
	const int16 color = 136;
	const int32 right = _dialTextBox.right - border;
	const int32 left = _dialTextBox.right - (size + border);
	const int32 top = _dialTextBox.bottom - (size + border);
	const int32 bottom = _dialTextBox.bottom - border;

	ComputedVertex vertices[3];

	vertices[0].intensity = color;
	vertices[0].x = right;
	vertices[0].y = top;

	vertices[1].intensity = color;
	vertices[1].x = left;
	vertices[1].y = bottom;

	vertices[2].intensity = color;
	vertices[2].x = right;
	vertices[2].y = bottom;

	CmdRenderPolygon polygon;
	polygon.numVertices = ARRAYSIZE(vertices);
	polygon.colorIndex = _minDegrade;
	polygon.renderType = POLYGONTYPE_FLAT;
	_engine->_renderer->renderPolygons(polygon, vertices);

	_engine->copyBlockPhys(left, top, right, bottom);
}

void Text::initDialWindow() {
	_engine->blitWorkToFront(_dialTextBox);

	if (_flagMessageShade) {
		_engine->_menu->drawRectBorders(_dialTextBox);
		Common::Rect rect(_dialTextBox);
		rect.grow(-1);
		_engine->_interface->shadeBox(rect, 3);
	}

	_nbChar = 0;
	_engine->blitFrontToWork(_dialTextBox);
}

void Text::secondInitDialWindow() {
	_engine->blitWorkToFront(_dialTextBox);
	_nbChar = 0;
}

void Text::normalWinDial() {
	const int32 margin = 16;
	_maxLineDial = 3; //(int32)(_dialTextBox.height() / lineHeight) - 1;
	const int32 textHeight = _maxLineDial * lineHeight;

	_dialTextBox.left = margin;
	_dialTextBox.top = _engine->height() - textHeight - margin - PADDING;
	_dialTextBox.right = _engine->width() - 1 - margin;
	_dialTextBox.bottom = _engine->height() - 1 - margin;

	_dialMaxSize = _dialTextBox.width() - (2 * PADDING);
}

void Text::bigWinDial() {
	const int32 margin = 8;
	_dialTextBox.left = margin;
	_dialTextBox.top = margin;
	_dialTextBox.right = _engine->width() - 1 - margin;
	_dialTextBox.bottom = _engine->height() - 1 - margin;

	_maxLineDial = (int32)(_dialTextBox.height() / lineHeight) - 1;
	_dialMaxSize = _dialTextBox.width() - (2 * PADDING);
}

void Text::commonOpenDial(TextId index) {
	closeDial();

	if (!getText(index)) {
		return;
	}
	_xDial = _dialTextBox.left + PADDING;
	_yDial = _dialTextBox.top + PADDING;
	_nbLineDial = 0;
	_ptDial = _ptText;
	// _word = '\0';
	// _ptWord = _word;
	_bufLine[0].chr = '\0';
	_bufLine[0].width = 0;
	_ptLine = _bufLine;
	_nbChar = 0;
	_flagEndDial = false;
	_flagEnd3Line = false;
	_flagRunningDial = true;

	// lba font is loaded during engine start
	setFont(INTER_LEAVE, INTER_SPACE);
}

void Text::getNextLine() {
	_sizeLine = 0;
	_nbSpace = 0;
	_sizeSpace = INTER_SPACE;
	_bufLine[0].chr = '\0';
	_bufLine[0].width = 0;
	bool flag = true;

	uint i = 0;

	while (true) {
		if (*_ptDial == ' ') {
			_ptDial++;
			continue;
		}
		if (*_ptDial == '\0') {
			break;
		}

		char word[256] = "";
		WordSize sizeWord = getNextWord(_ptDial, word, sizeof(word));
		if (_sizeLine + _sizeSpace + sizeWord.sizeWord > _dialMaxSize) {
			break;
		}

		if (*word == '\1') {
			_ptDial++;
			flag = false;
			break;
		}

		// @ is a line break
		if (*word == '@') {
			flag = false;
			_ptDial++;
			if (_sizeLine == 0) {
				_bufLine[0].chr = '\0';
				_bufLine[0].width = 0;
				_sizeLine = INTER_SPACE;
			}
			// new page?
			if (word[1] == 'P') {
				_nbLineDial = _maxLineDial;
				_ptDial++;
			}
			break;
		}

		_ptDial += sizeWord.lenWord;
		appendText(word, i);
		appendText(" ", i);

		_sizeLine += sizeWord.sizeWord + _sizeSpace;
		_nbSpace++;

		if (*_ptDial == '\0') {
			break;
		}
		_ptDial++;
	}

	_bufLine[i].chr = 0;
	_bufLine[i].width = 0;

	if (_isVisualRTL) {
		for (uint j = 0; j < i / 2; j++) {
			LineCharacter t = _bufLine[j];
			_bufLine[j] = _bufLine[i - j - 1];
			_bufLine[i - j - 1] = t;
		}
	}

	if (_nbSpace) {
		_nbSpace--;
	}

	if (*_ptDial != '\0' && flag) {
		if (_nbSpace == 0) {
			_nbSpace = 1;
		}
		int32 dx = _dialMaxSize - _sizeLine;
		int32 dd = dx / _nbSpace;
		_sizeSpace += dd;
		dd = _nbSpace * dd;
		_nbBigSpace = dx - dd;
	}

	_ptLine = _bufLine;
}

void Text::initEndPage() {
	for (uint i = 0; i < ARRAYSIZE(_bufLine); i++) {
		_bufLine[i].chr = ' ';
		_bufLine[i].width = _sizeSpace;
	}
	// the end of the buffer defines how fast the next page is shown - as the
	// whitespaces are handled in the fade in process, too. But we need at least 32 chars,
	// to completly fade in the last characters of a full page (see TEXT_MAX_FADE_IN_CHR)
	_bufLine[ARRAYSIZE(_bufLine) - 1].chr = '\0';
	_bufLine[ARRAYSIZE(_bufLine) - 1].width = 0;
	_ptLine = _bufLine;
	_nbLineDial = 0;
	_sizeLine = 16; // hack
}

ProgressiveTextState Text::nextDialChar() { // NextDialCar
	if (!_flagRunningDial) {
		return ProgressiveTextState::End;
	}

	if (_ptLine->chr == '\0') {
		if (_flagEndDial) {
			_flagRunningDial = false;
			return ProgressiveTextState::End;
		}
		if (_flagEnd3Line) {
			secondInitDialWindow();
			_flagEnd3Line = false;
			_xDial = _dialTextBox.left + PADDING;
			_yDial = _dialTextBox.top + PADDING;
		}
		if (*_ptDial == '\0') {
			initEndPage();
			_flagEndDial = true;
			return ProgressiveTextState::ContinueRunning;
		}

		getNextLine();
	}

	if (_ptLine->chr != '\0') {
		pushChar(_xDial, _yDial, *_ptLine);
		fadeInCharacters(_nbChar, _maxDegrade);

		if (_ptLine->chr != ' ')
			_xDial += _ptLine->width + INTER_LEAVE;
		else {
			if (_nbBigSpace) {
				_nbBigSpace--;
				_xDial++;
			}
			_xDial += _sizeSpace;
		}

		++_ptLine;

		if (_ptLine->chr == '\0') {
			_xDial = _dialTextBox.left + PADDING;
			_yDial += lineHeight;

			if (_flagEnd3Line && !_flagEndDial) {
				renderContinueReadingTriangle();
				return ProgressiveTextState::NextPage;
			}

			_nbLineDial++;
			if (_nbLineDial >= _maxLineDial) {
				initEndPage();
				_flagEnd3Line = true;
				if (*_ptDial == '\0') {
					_flagEndDial = true;
				}
			}
		}
	}

	return ProgressiveTextState::ContinueRunning;
}

void Text::closeDial() {
	_flagRunningDial = false;
}

void Text::appendText(const char *s, uint &i) {
	while (1) {
		if (i >= ARRAYSIZE(_bufLine) - 1)
			return;
		uint16 chr = getNextChar(s);
		if (!chr)
			return;
		_bufLine[i].chr = chr;
		_bufLine[i].width = getCharWidth(chr);
		i++;
	}
}

int32 Text::getCharWidth(uint16 chr) const {
	if (_isShiftJIS && (chr > 0x100))
		return kLBASJISCharWidth;
	if (chr > 0x100)
		chr = '?';
	Common::MemoryReadStream stream(_engine->_resources->_fontPtr, _engine->_resources->_fontBufSize);
	stream.seek(chr * 4);
	stream.seek(stream.readSint16LE());
	return stream.readByte();
}

int32 Text::getCharHeight(uint16 chr) const {
	if (_isShiftJIS && (chr > 0x100))
		return kLBASJISCharHeight;
	if (chr > 0x100)
		chr = '?';
	Common::MemoryReadStream stream(_engine->_resources->_fontPtr, _engine->_resources->_fontBufSize);
	stream.seek(chr * 4);
	stream.seek(stream.readSint16LE() + 1);
	return stream.readByte();
}

void Text::fadeInRemainingChars() {
	if (_nbChar <= 0) {
		return;
	}
	fadeInCharacters(_nbChar, _minDegrade);
	--_nbChar;
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
		commonOpenDial(index);
		initDialWindow();

		ScopedKeyMap uiKeyMap(_engine, uiKeyMapId);
		ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;
		for (;;) {
			FrameMarker frame(_engine, 66);
			_engine->readKeys();
			if (textState == ProgressiveTextState::ContinueRunning) {
				textState = nextDialChar();
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
					stopVox(_currDialTextEntry);
					break;
				}
				if (textState == ProgressiveTextState::NextPage) {
					textState = ProgressiveTextState::ContinueRunning;
				}
			}
			if (_engine->_input->toggleAbortAction() || _engine->shouldQuit()) {
				stopVox(_currDialTextEntry);
				aborted = true;
				break;
			}

			if (playVox) {
				playVoxSimple(_currDialTextEntry);
			}
		}
	}

	// Wait until silence
	while (playVox && playVoxSimple(_currDialTextEntry)) {
		FrameMarker frame(_engine);
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			stopVox(_currDialTextEntry);
			aborted = true;
			break;
		}
	}
	_voxHiddenIndex = 0;
	_hasHiddenVox = false;
	_flagRunningDial = false;
	_engine->_input->resetHeroActions();

	return aborted;
}

bool Text::drawTextProgressive(TextId index, bool playVox, bool loop) { // Dial
	_engine->extInitSvga();
	_engine->_interface->memoClip();
	_engine->_interface->unsetClip();
	_engine->saveFrontBuffer();
	const bool aborted = displayText(index, _engine->_cfgfile.FlagDisplayText, playVox, loop);
	_engine->_interface->restoreClip();
	return aborted;
}

void Text::setFont(int32 interLeave, int32 interSpace) {
	_interLeave = interLeave;
	_interSpace = interSpace;
}

void Text::setFontColor(int32 color) { // CoulFont
	_dialTextColor = color;
}

bool Text::getMenuText(TextId index, char *text, uint32 textSize) {
	if (index == _currMenuTextIndex) {
		if (_currMenuTextBank == _engine->_scene->_sceneTextBank) {
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

	Common::strlcpy(text, _ptText, MIN<int32>(textSize, _currDialTextSize + 1));
	_currDialTextSize++;
	Common::strlcpy(_currMenuTextBuffer, text, MIN<int32>(sizeof(_currMenuTextBuffer), _currDialTextSize));

	_currMenuTextIndex = index;
	_currMenuTextBank = _engine->_scene->_sceneTextBank;
	return true;
}

void Text::drawAskQuestion(TextId index) {
	displayText(index, true, true, true);
}

void Text::drawHolomapLocation(TextId index) {
	normalWinDial();
	setFontCrossColor(COLOR_WHITE);
	_engine->_interface->box(_dialTextBox, COLOR_BLACK);
	const bool displayText = _engine->_cfgfile.FlagDisplayText;
	_engine->_cfgfile.FlagDisplayText = true;
	drawTextProgressive(index, false, false);
	_engine->_cfgfile.FlagDisplayText = displayText;
}

} // namespace TwinE
