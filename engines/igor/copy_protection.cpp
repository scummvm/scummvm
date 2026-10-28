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
#include "common/events.h"

#include "igor/floppy_menu.h"
#include "igor/igor.h"

namespace Igor {

// Copy protection screen of the floppy version
static const int kFloppyProtectionTries = 3;
static const int kFloppyProtectionWordCount = 100;
static const int kFloppyProtectionEntrySize = 14;
static const int kFloppyProtectionWordLength = 10;
static const int kFloppyProtectionCodeOffset = 0x45;
static const int kFloppyProtectionInputX = 122;
static const int kFloppyProtectionInputY = 75;
static const int kFloppyProtectionInputFullX = 197;
static const int kFloppyProtectionTitleX = 136;
static const int kFloppyProtectionPromptX = 93;
static const int kFloppyProtectionPromptY = 42;
static const int kFloppyProtectionQuestionX = 81;
static const int kFloppyProtectionQuestionY = 52;
static const int kFloppyProtectionReplyX = 100;
static const int kFloppyProtectionReplyY = 86;
static const int kFloppyProtectionReplyTicks = 2 * 200;
static const int kFloppyProtectionBlinkTick = 61;
static const int kFloppyProtectionBlinkPeriod = 64;
static const int kFloppyProtectionBoxX = 120;
static const int kFloppyProtectionBoxY = 56;
static const int kFloppyProtectionBoxWidth = 80;
static const int kFloppyProtectionBoxHeight = 11;
static const int kFloppyProtectionCaretRow = 8;
static const int kFloppyProtectionCaretWidth = 7;
static const int kFloppyProtectionGlyphRows = 9;
static const int kFloppyProtectionMaxInput = 32;

enum {
	kFloppyColorInputFill = 0xFB
};

enum FloppyProtectionText {
	kFloppyProtectionTitle = 0,
	kFloppyProtectionPage,
	kFloppyProtectionLine,
	kFloppyProtectionWord,
	kFloppyProtectionPrompt,
	kFloppyProtectionMaster,
	kFloppyProtectionWrong,
	kFloppyProtectionTextCount
};

// Pascal strings stored one after the other in a resource.
static void readProtectionStrings(Common::Array<Common::String> &texts, const uint8 *data, int size, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		if (offset >= size || data[offset] > size - offset - 1)
			error("Invalid copy protection text resource");
		const int length = data[offset++];
		texts.push_back(Common::String((const char *)data + offset, length));
		offset += length;
	}
	if (offset != size)
		error("Unexpected data in copy protection text resource");
}

static int protectionInputChar(const Common::KeyState &kbd) {
	const uint32 chr = kbd.ascii;
	if (chr >= 0x20 && chr < 0x7F)
		return chr;
	switch (chr) {
	case 0xE9: return 0x82; // e acute
	case 0xE1: return 0xA0; // a acute
	case 0xED: return 0xA1; // i acute
	case 0xF3: return 0xA2; // o acute
	case 0xFA: return 0xA3; // u acute
	case 0xF1: return 0xA4; // n tilde
	case 0xD1: return 0xA5; // N tilde
	case 0xBF: return 0xA8; // inverted question mark
	case 0xA1: return 0xAD; // inverted exclamation mark
	default: return 0;
	}
}
// only ASCII + characters handled in protectionInputChar()
static bool protectionCharAllowed(int chr) {
	static const uint8 allowed[32] = {
		0, 0, 0, 0, 0x07, 0xF3, 0xFF, 0xDF, 0xFE, 0xFF, 0xFF, 0x07, 0xFE, 0xFF, 0xFF, 0x07,
		0x04, 0, 0, 0, 0x3F, 0x21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	return (allowed[chr >> 3] & (1 << (chr & 7))) != 0;
}

void IgorEngine::floppyMenuFillRow(int x, int y, int count, int color) {
	memset(_screenTextLayer + (y - kFloppyMenuY) * 320 + x, color, count);
}

bool IgorEngine::isGameEnding() const {
	return _eventQuitGame || _currentPart == kInvalidPart;
}


// Draws the panel with the question for [entry] and the empty input box.
void IgorEngine::drawProtectionScreen(const Common::Array<Common::String> &texts, const uint8 *button, const uint8 *entry) {
	const int numbers[3] = { entry[11], entry[12], entry[13] };
	floppyOptionsDrawPanel(button);
	floppyOptionsClearCaptionBar(button);
	_sentenceFont->drawString(_screenTextLayer, texts[kFloppyProtectionTitle].c_str(), kFloppyProtectionTitleX, kFloppyCaptionY - kFloppyMenuY, kFloppyColorCaption, -1, -1);
	Common::String question = texts[kFloppyProtectionPage] + Common::String::format("%d", numbers[0]) +
		texts[kFloppyProtectionLine] + Common::String::format("%d", numbers[1]) +
		texts[kFloppyProtectionWord] + Common::String::format("%d", numbers[2]);
	int questionX = kFloppyProtectionQuestionX;
	for (int i = 0; i < 3; ++i) {
		if (numbers[i] > 9)
			questionX -= 2;
	}
	_sentenceFont->drawString(_screenTextLayer, texts[kFloppyProtectionPrompt].c_str(), kFloppyProtectionPromptX, kFloppyProtectionPromptY - kFloppyMenuY, kFloppyColorCaption, -1, -1);
	_sentenceFont->drawString(_screenTextLayer, question.c_str(), questionX, kFloppyProtectionQuestionY - kFloppyMenuY, kFloppyColorCaption, -1, -1);

	// Frame of the input box: light top and left edges, dark bottom and right edges.
	const int boxY = kFloppyMenuY + kFloppyProtectionBoxY;
	floppyMenuFillRow(kFloppyProtectionBoxX, boxY, kFloppyProtectionBoxWidth, kFloppyColorFrameLight);
	for (int i = 1; i < kFloppyProtectionBoxHeight - 1; ++i) {
		_screenTextLayer[(kFloppyProtectionBoxY + i) * 320 + kFloppyProtectionBoxX] = kFloppyColorFrameLight;
		_screenTextLayer[(kFloppyProtectionBoxY + i) * 320 + kFloppyProtectionBoxX + kFloppyProtectionBoxWidth - 1] = kFloppyColorFrameDark;
	}
	floppyMenuFillRow(kFloppyProtectionBoxX, boxY + kFloppyProtectionBoxHeight - 1, kFloppyProtectionBoxWidth, kFloppyColorFrameDark);
	floppyOptionsPresent(0);
}

bool IgorEngine::readProtectionWord(uint8 *typed) {
	memset(typed, ' ', kFloppyProtectionMaxInput + 1);
	int position = 1;
	int x = kFloppyProtectionInputX;
	bool full = false;
	bool done = false;
	int caretColor = kFloppyColorCaption;
	int blinkCounter = 0;
	const uint32 startMillis = _system->getMillis();
	uint32 lastTick = 0;
	while (!done && !isGameEnding()) {
		Common::Event ev;
		bool changed = false;
		while (!done && _eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT || ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const int chr = protectionInputChar(ev.kbd);
			if (chr && protectionCharAllowed(chr)) {
				if (!full && position <= kFloppyProtectionMaxInput) {
					typed[position++] = chr;
					floppyMenuFillRow(x, kFloppyProtectionInputY + kFloppyProtectionCaretRow, kFloppyProtectionCaretWidth, kFloppyColorInputFill);
					const char str[2] = { (char)chr, 0 };
					_sentenceFont->drawString(_screenTextLayer, str, x, kFloppyProtectionInputY - kFloppyMenuY, kFloppyColorCaption, -1, -1);
					x += _sentenceFont->getCharWidth(chr);
					full = (x + 6 > kFloppyProtectionInputFullX);
					changed = true;
				}
			} else if (ev.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				if (position > 1) {
					--position;
					if (!full)
						floppyMenuFillRow(x, kFloppyProtectionInputY + kFloppyProtectionCaretRow, kFloppyProtectionCaretWidth, kFloppyColorInputFill);
					x -= _sentenceFont->getCharWidth(typed[position]);
					full = (x + 6 > kFloppyProtectionInputFullX);
					typed[position] = ' ';
					for (int row = 1; row <= kFloppyProtectionGlyphRows; ++row)
						floppyMenuFillRow(x, kFloppyProtectionInputY + row, kFloppyProtectionCaretWidth, kFloppyColorInputFill);
					changed = true;
				}
			} else if (ev.kbd.keycode == Common::KEYCODE_RETURN || ev.kbd.keycode == Common::KEYCODE_KP_ENTER) {
				if (!full)
					floppyMenuFillRow(x, kFloppyProtectionInputY + kFloppyProtectionCaretRow, kFloppyProtectionCaretWidth, kFloppyColorInputFill);
				done = true;
				changed = true;
			}
		}
		if (blinkCounter == kFloppyProtectionBlinkTick && !full && !done) {
			floppyMenuFillRow(x, kFloppyProtectionInputY + kFloppyProtectionCaretRow, kFloppyProtectionCaretWidth, caretColor);
			caretColor = (caretColor == kFloppyColorCaption) ? (int)kFloppyColorInputFill : (int)kFloppyColorCaption;
			changed = true;
		}
		if (changed)
			floppyOptionsPresent(0);

		while (!done && !_eventQuitGame && ticksSince(startMillis) == lastTick)
			_system->delayMillis(1);
		lastTick = ticksSince(startMillis);
		blinkCounter = (blinkCounter == kFloppyProtectionBlinkPeriod - 1) ? 0 : blinkCounter + 1;
	}
	return !isGameEnding();
}

// Returns false if the test failed
bool IgorEngine::copyProtection() {
	int size;
	Common::Array<Common::String> texts;
	uint8 *data = loadData(TXT_FloppyProtectionScreen, 0, &size);
	readProtectionStrings(texts, data, size, 5);
	free(data);
	data = loadData(TXT_FloppyProtectionReply, 0, &size);
	readProtectionStrings(texts, data, size, 2);
	free(data);
	assert(texts.size() == kFloppyProtectionTextCount);
	uint8 *words = loadData(DAT_FloppyProtectionWords, 0, &size);
	assert(size == kFloppyProtectionWordCount * kFloppyProtectionEntrySize);
	uint8 *button = loadData(IMG_OptionsButton);

	PauseToken pauseToken = pauseEngine();
	hideCursor();
	floppyMenuFadeOut();
	memcpy(_screenTextLayer, _screenVGA + kFloppyMenuY * 320, kFloppyMenuSize);

	bool passed = false;
	for (int attempt = 1; attempt <= kFloppyProtectionTries && !passed && !isGameEnding(); ++attempt) {
		const uint8 *entry = words + getRandomNumber(kFloppyProtectionWordCount - 1) * kFloppyProtectionEntrySize;
		drawProtectionScreen(texts, button, entry);
		uint8 typed[kFloppyProtectionMaxInput + 1];
		if (!readProtectionWord(typed))
			break;

		// Compare the typed word, in the stored form, with the expected word and the master code.
		uint8 encoded[kFloppyProtectionWordLength + 1];
		encoded[0] = kFloppyProtectionWordLength;
		for (int i = 1; i <= kFloppyProtectionWordLength; ++i) {
			const uint8 chr = (typed[i] >= 'a' && typed[i] <= 'z') ? typed[i] - 0x20 : typed[i];
			encoded[i] = (uint8)(chr + kFloppyProtectionCodeOffset);
		}
		const Common::String &master = texts[kFloppyProtectionMaster];
		passed = !memcmp(encoded, entry, kFloppyProtectionWordLength + 1) ||
			(master.size() == kFloppyProtectionWordLength && !memcmp(encoded + 1, master.c_str(), kFloppyProtectionWordLength));
		if (!passed && attempt < kFloppyProtectionTries) {
			// TODO: the original also beeps here; the tone and length of that beep are not established.
			_sentenceFont->drawString(_screenTextLayer, texts[kFloppyProtectionWrong].c_str(), kFloppyProtectionReplyX, kFloppyProtectionReplyY - kFloppyMenuY, kFloppyColorCaption, -1, -1);
			floppyOptionsPresent(0);
			waitForTimer(kFloppyProtectionReplyTicks, false);
		}
	}

	free(button);
	free(words);
	_inputVars[kInputClick] = 0;
	_inputVars[kInputRightClick] = 0;
	_inputVars[kInputSkipDialogue] = 0;
	_inputVars[kInputOptions] = 0;
	_inputVars[kInputPause] = 0;
	_inputVars[kInputEscape] = 0;
	_inputVars[kInputConfirm] = 0;
	showCursor();
	if (!passed) {
		// Three wrong words end the game.
		_currentPart = kInvalidPart;
		return false;
	}
	setPaletteColor(255, 63, 63, 63);
	fadeIn(kFloppyFadeCount);
	return true;
}

bool IgorEngine::checkCopyProtection() {
	if (_game.version != kIdSpaFloppy || _floppyProtectionPassed || (ConfMan.hasKey("skip_copy_protection") && ConfMan.getBool("skip_copy_protection")))
		return true;
	if (!copyProtection())
		return false;
	_floppyProtectionPassed = true;
	return true;
}

} // End of namespace Igor
