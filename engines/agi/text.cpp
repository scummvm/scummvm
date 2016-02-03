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

#include "agi/agi.h"
#include "agi/sprite.h"     // for commit_both()
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/text.h"
#include "agi/systemui.h"
#include "agi/words.h"
#ifdef __DS__
#include "wordcompletion.h"
#endif

namespace Agi {

TextMgr::TextMgr(AgiEngine *vm, Words *words, GfxMgr *gfx) {
	_vm = vm;
	_words = words;
	_gfx = gfx;

	memset(&_messageState, 0, sizeof(_messageState));
	_textPos.row = 0;
	_textPos.column = 0;
	_reset_Column = 0;

	charAttrib_Set(15, 0);

	_messageState.wanted_TextPos.row = -1;
	_messageState.wanted_TextPos.column = -1;
	_messageState.wanted_Text_Width = -1;

	_textPosArrayCount = 0;
	memset(&_textPosArray, 0, sizeof(_textPosArray));
	_textAttribArrayCount = 0;
	memset(&_textAttribArray, 0, sizeof(_textAttribArray));

	_inputEditEnabled = false;
	_inputCursorChar = 0;

	_statusEnabled = false;
	_statusRow = 0;

	_promptRow = 0;
	promptDisable();
	promptReset();

	_inputStringRow = 0;
	_inputStringColumn = 0;
	_inputStringMaxLen = 0;
	_inputStringCursorPos = 0;
	_inputString[0] = 0;

	configureScreen(2);

	_messageBoxCancelled = false;
}

TextMgr::~TextMgr() {
}

void TextMgr::init(SystemUI *systemUI) {
	_systemUI = systemUI;
}

void TextMgr::configureScreen(uint16 row_Min) {
	_window_Row_Min = row_Min;
	_window_Row_Max = row_Min + 21;

	// forward data to GfxMgr as well
	_gfx->setRenderStartOffset(row_Min * FONT_DISPLAY_HEIGHT);
}
uint16 TextMgr::getWindowRowMin() {
	return _window_Row_Min;
}

void TextMgr::dialogueOpen() {
	_messageState.dialogue_Open = true;
}

void TextMgr::dialogueClose() {
	_messageState.dialogue_Open = false;
}

void TextMgr::charPos_Clip(int16 &row, int16 &column) {
	row = CLIP<int16>(row, 0, FONT_ROW_CHARACTERS - 1);
	column = CLIP<int16>(column, 0, FONT_COLUMN_CHARACTERS - 1);
}

void TextMgr::charPos_Set(int16 row, int16 column) {
	_textPos.row = row;
	_textPos.column = column;
}

void TextMgr::charPos_Set(TextPos_Struct *posPtr) {
	_textPos.row    = posPtr->row;
	_textPos.column = posPtr->column;
}

void TextMgr::charPos_Get(int16 &row, int16 &column) {
	row = _textPos.row;
	column = _textPos.column;
}

void TextMgr::charPos_Get(TextPos_Struct *posPtr) {
	posPtr->row    = _textPos.row;
	posPtr->column = _textPos.column;
}

void TextMgr::charPos_Push() {
	if (_textPosArrayCount < TEXTPOSARRAY_MAX) {
		charPos_Get(&_textPosArray[_textPosArrayCount]);
		_textPosArrayCount++;
	}
}

void TextMgr::charPos_Pop() {
	if (_textPosArrayCount > 0) {
		_textPosArrayCount--;
		charPos_Set(&_textPosArray[_textPosArrayCount]);
	}
}

void TextMgr::charPos_SetInsideWindow(int16 windowRow, int16 windowColumn) {
	if (!_messageState.window_Active)
		return;

	_textPos.row = _messageState.textPos.row + windowRow;
	_textPos.column = _messageState.textPos.column + windowColumn;
}

static byte charAttrib_CGA_Conversion[] = {
	0, 1, 1, 1, 2, 2, 2, 3, 3, 1, 1, 1, 2, 2, 2
};

void TextMgr::charAttrib_Set(byte foreground, byte background) {
	_textAttrib.foreground = foreground;
	_textAttrib.background = calculateTextBackground(background);

	if (!_vm->_game.gfxMode) {
		// Text-mode:
		// just use the given colors directly
		_textAttrib.combinedForeground = foreground;
		_textAttrib.combinedBackground = background;
	} else {
		// Graphics-mode:
		switch (_vm->_renderMode) {
		case Common::kRenderCGA:
			// CGA
			if (background) {
				_textAttrib.combinedForeground = 3;
				_textAttrib.combinedBackground = 8; // enable invert of colors
			} else if (foreground > 14) {
				if (foreground > 14) {
					_textAttrib.combinedForeground = 3;
				} else {
					_textAttrib.combinedForeground = charAttrib_CGA_Conversion[foreground & 0x0F];
				}
				_textAttrib.combinedBackground = 0;
			}
			break;
		default:
			// EGA-handling:
			if (background) {
				_textAttrib.combinedForeground = 15;
				_textAttrib.combinedBackground = 8; // enable invert of colors
			} else {
				_textAttrib.combinedForeground = foreground;
				_textAttrib.combinedBackground = 0;
			}
			break;
		}
	}
}

byte TextMgr::charAttrib_GetForeground() {
	return _textAttrib.foreground;
}
byte TextMgr::charAttrib_GetBackground() {
	return _textAttrib.background;
}

void TextMgr::charAttrib_Push() {
	if (_textAttribArrayCount < TEXTATTRIBARRAY_MAX) {
		memcpy(&_textAttribArray[_textAttribArrayCount], &_textAttrib, sizeof(_textAttrib));
		_textAttribArrayCount++;
	}
}

void TextMgr::charAttrib_Pop() {
	if (_textAttribArrayCount > 0) {
		_textAttribArrayCount--;
		memcpy(&_textAttrib, &_textAttribArray[_textAttribArrayCount], sizeof(_textAttrib));
	}
}

byte TextMgr::calculateTextBackground(byte background) {
	if ((_vm->_game.gfxMode) && (background)) {
		return 15; // interpreter sets 0xFF, but drawClearCharacter() would use upper 4 bits by shift
	}
	return 0;
}

void TextMgr::display(int16 textNr, int16 textRow, int16 textColumn) {
	const char *logicTextPtr = NULL;
	char *processedTextPtr   = NULL;

	charPos_Push();
	charPos_Set(textRow, textColumn);

	if (textNr >= 1 && textNr <= _vm->_game._curLogic->numTexts) {
		logicTextPtr = _vm->_game._curLogic->texts[textNr - 1];
		processedTextPtr = stringPrintf(logicTextPtr);
		processedTextPtr = stringWordWrap(processedTextPtr, 40);
		displayText(processedTextPtr);

		// Signal, that non-blocking text is shown at the moment
		if (textRow > 0) {
			// only signal, when it's not the status line (kq3)
			_vm->nonBlockingText_IsShown();
		}
	}
	charPos_Pop();
}

void TextMgr::displayTextInsideWindow(const char *textPtr, int16 windowRow, int16 windowColumn) {
	int16 textRow = 0;
	int16 textColumn = 0;

	if (!_messageState.window_Active)
		return;

	charPos_Push();
	textRow = _messageState.textPos.row + windowRow;
	textColumn = _messageState.textPos.column + windowColumn;
	charPos_Set(textRow, textColumn);
	displayText(textPtr);
	charPos_Pop();
}

void TextMgr::displayText(const char *textPtr, bool disabledLook) {
	const char *curTextPtr = textPtr;
	byte  curCharacter = 0;

	while (1) {
		curCharacter = *curTextPtr;
		if (!curCharacter)
			break;

		curTextPtr++;
		displayCharacter(curCharacter, disabledLook);
	}
}

void TextMgr::displayCharacter(byte character, bool disabledLook) {
	TextPos_Struct charCurPos;

	charPos_Get(&charCurPos);

	switch (character) {
	case 0x08: // backspace
		if (charCurPos.column) {
			charCurPos.column--;
		} else if (charCurPos.row > 21) {
			charCurPos.column = (FONT_COLUMN_CHARACTERS - 1);
			charCurPos.row--;
		}
		clearBlock(charCurPos.row, charCurPos.column, charCurPos.row, charCurPos.column, _textAttrib.background);
		charPos_Set(&charCurPos);
		break;

	case 0x0D:
	case 0x0A: // CR/LF
		if (charCurPos.row < (FONT_ROW_CHARACTERS - 1))
			charCurPos.row++;
		charCurPos.column = _reset_Column;
		charPos_Set(&charCurPos);
		break;
	default:
		// ch_attrib(state.text_comb, conversion);
		_gfx->drawCharacter(charCurPos.row, charCurPos.column, character, _textAttrib.combinedForeground, _textAttrib.combinedBackground, disabledLook);

		charCurPos.column++;
		if (charCurPos.column <= (FONT_COLUMN_CHARACTERS - 1)) {
			charPos_Set(&charCurPos);
		} else {
			displayCharacter(0x0D); // go to next line
		}
	}
}

void TextMgr::print(int16 textNr) {
	const char *logicTextPtr = NULL;
	if (textNr >= 1 && textNr <= _vm->_game._curLogic->numTexts) {
		logicTextPtr = _vm->_game._curLogic->texts[textNr - 1];
		messageBox(logicTextPtr);
	}
}

void TextMgr::printAt(int16 textNr, int16 textPos_Row, int16 textPos_Column, int16 text_Width) {
	// Sierra didn't do clipping, we do it for security
	charPos_Clip(textPos_Row, textPos_Column);

	_messageState.wanted_TextPos.row = textPos_Row;
	_messageState.wanted_TextPos.column = textPos_Column;
	_messageState.wanted_Text_Width = text_Width;

	if (_messageState.wanted_Text_Width == 0) {
		_messageState.wanted_Text_Width = 30;
	}
	print(textNr);

	_messageState.wanted_TextPos.row = -1;
	_messageState.wanted_TextPos.column = -1;
	_messageState.wanted_Text_Width = -1;
}

bool TextMgr::messageBox(const char *textPtr) {
	drawMessageBox(textPtr);

	if (_vm->getFlag(VM_FLAG_OUTPUT_MODE)) {
		// non-blocking window
		_vm->setFlag(VM_FLAG_OUTPUT_MODE, false);

		// Signal, that non-blocking text is shown at the moment
		_vm->nonBlockingText_IsShown();
		return true;
	}

	// blocking window
	_vm->_noSaveLoadAllowed = true;
	_vm->nonBlockingText_Forget();

	// timed window
	uint32 windowTimer = _vm->getVar(VM_VAR_WINDOW_RESET);
	debugC(3, kDebugLevelText, "blocking window v21=%d", windowTimer);

	windowTimer = windowTimer * 10; // 1 = 0.5 seconds
	_messageBoxCancelled = false;

	_vm->inGameTimerResetPassedCycles();
	_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_MESSAGEBOX);
	do {
		_vm->mainCycle();
		_vm->inGameTimerUpdate();

		if (windowTimer > 0) {
			if (_vm->inGameTimerGetPassedCycles() >= windowTimer) {
				// Timer reached, close automatically
				_vm->cycleInnerLoopInactive();
			}
		}
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	_vm->inGameTimerResetPassedCycles();

	_vm->setVar(VM_VAR_WINDOW_RESET, 0);

	closeWindow();
	_vm->_noSaveLoadAllowed = false;

	if (_messageBoxCancelled)
		return false;
	return true;
}

void TextMgr::messageBox_CharPress(uint16 newKey) {
	switch (newKey) {
	case AGI_KEY_ENTER:
		_vm->cycleInnerLoopInactive(); // exit messagebox-loop
		break;
	case AGI_KEY_ESCAPE:
		_messageBoxCancelled = true;
		_vm->cycleInnerLoopInactive(); // exit messagebox-loop
		break;
	default:
		break;
	}
}

void TextMgr::drawMessageBox(const char *textPtr, int16 wantedHeight, int16 wantedWidth, bool wantedForced) {
	int16 maxWidth = wantedWidth;
	int16 startingRow = 0;
	char *processedTextPtr;

	if (_messageState.window_Active) {
		closeWindow();
	}
	charAttrib_Push();
	charPos_Push();
	charAttrib_Set(0, 15);

	if ((_messageState.wanted_Text_Width == -1) && (maxWidth == 0)) {
		maxWidth = 30;
	} else if (_messageState.wanted_Text_Width != -1) {
		maxWidth = _messageState.wanted_Text_Width;
	}

	processedTextPtr = stringPrintf(textPtr);

	int16 calculatedWidth = 0;
	int16 calculatedHeight = 0;

	processedTextPtr = stringWordWrap(processedTextPtr, maxWidth, &calculatedWidth, &calculatedHeight);
	_messageState.textSize_Width  = calculatedWidth;
	_messageState.textSize_Height = calculatedHeight;

	_messageState.printed_Height = _messageState.textSize_Height;

	// Caller wants to force specified width/height? set it
	if (wantedForced) {
		if (wantedHeight)
			_messageState.textSize_Height = wantedHeight;
		if (wantedWidth)
			_messageState.textSize_Width = wantedWidth;
	}

	if (_messageState.wanted_TextPos.row == -1) {
		startingRow = ((HEIGHT_MAX - _messageState.textSize_Height - 1) / 2) + 1;
	} else {
		startingRow = _messageState.wanted_TextPos.row;
	}
	_messageState.textPos.row = startingRow + _window_Row_Min;
	_messageState.textPos_Edge.row = _messageState.textSize_Height + _messageState.textPos.row - 1;

	if (_messageState.wanted_TextPos.column == -1) {
		_messageState.textPos.column = (FONT_COLUMN_CHARACTERS - _messageState.textSize_Width) / 2;
	} else {
		_messageState.textPos.column = _messageState.wanted_TextPos.column;
	}
	_messageState.textPos_Edge.column = _messageState.textPos.column + _messageState.textSize_Width;

	charPos_Set(_messageState.textPos.row, _messageState.textPos.column);

	_messageState.backgroundSize_Width = (_messageState.textSize_Width * FONT_VISUAL_WIDTH) + 10;
	_messageState.backgroundSize_Height = (_messageState.textSize_Height * FONT_VISUAL_HEIGHT) + 10;
	_messageState.backgroundPos_x = (_messageState.textPos.column * FONT_VISUAL_WIDTH) - 5;
	_messageState.backgroundPos_y = (_messageState.textPos_Edge.row - _window_Row_Min + 1) * FONT_VISUAL_HEIGHT + 4;

	// Hardcoded colors: white background and red lines
	_gfx->drawBox(_messageState.backgroundPos_x, _messageState.backgroundPos_y, _messageState.backgroundSize_Width, _messageState.backgroundSize_Height, 15, 4);

	_messageState.window_Active = true;

	_reset_Column = _messageState.textPos.column;
	displayText(processedTextPtr);
	_reset_Column = 0;

	charPos_Pop();
	charAttrib_Pop();

	_messageState.dialogue_Open = true;
}

void TextMgr::closeWindow() {
	if (_messageState.window_Active) {
		_gfx->render_Block(_messageState.backgroundPos_x, _messageState.backgroundPos_y, _messageState.backgroundSize_Width, _messageState.backgroundSize_Height);
	}
	_messageState.dialogue_Open = false;
	_messageState.window_Active = false;
}

void TextMgr::statusRow_Set(int16 row) {
	_statusRow = row;
}
int16 TextMgr::statusRow_Get() {
	return _statusRow;
}

void TextMgr::statusEnable() {
	_statusEnabled = true;
}
void TextMgr::statusDisable() {
	_statusEnabled = false;
}
bool TextMgr::statusEnabled() {
	return _statusEnabled;
}

void TextMgr::statusDraw() {
	char *statusTextPtr = NULL;

	charAttrib_Push();
	charPos_Push();

	if (_statusEnabled) {
		clearLine(_statusRow, 15);

		charAttrib_Set(0, 15);
		charPos_Set(_statusRow, 1);
		statusTextPtr = stringPrintf(_systemUI->getStatusTextScore());
		displayText(statusTextPtr);

		charPos_Set(_statusRow, 30);
		if (_vm->getFlag(VM_FLAG_SOUND_ON)) {
			statusTextPtr = stringPrintf(_systemUI->getStatusTextSoundOn());
		} else {
			statusTextPtr = stringPrintf(_systemUI->getStatusTextSoundOff());
		}
		displayText(statusTextPtr);
	}

	charPos_Pop();
	charAttrib_Pop();
}

void TextMgr::statusClear() {
	clearLine(_statusRow, 0);
}

void TextMgr::clearLine(int16 row, byte color) {
	clearLines(row, row, color);
}

void TextMgr::clearLines(int16 row_Upper, int16 row_Lower, byte color) {
	clearBlock(row_Upper, 0, row_Lower, FONT_COLUMN_CHARACTERS - 1, color);
}

void TextMgr::clearBlock(int16 row_Upper, int16 column_Upper, int16 row_Lower, int16 column_Lower, byte color) {
	// Sierra didn't do clipping of the coordinates, we do it for security
	//  and b/c there actually are some games, that call commands with invalid coordinates
	//  see cmdClearLines() comments.
	charPos_Clip(row_Upper, column_Upper);
	charPos_Clip(row_Lower, column_Lower);

	int16 x = column_Upper * FONT_DISPLAY_WIDTH;
	int16 y = row_Upper * FONT_DISPLAY_HEIGHT;
	int16 width = (column_Lower + 1 - column_Upper) * FONT_DISPLAY_WIDTH;
	int16 height = (row_Lower + 1 - row_Upper) * FONT_DISPLAY_HEIGHT;

	y = y + height - 1; // drawDisplayRect wants lower Y-coordinate
	_gfx->drawDisplayRect(x, y, width, height, color);
}

void TextMgr::clearBlockInsideWindow(int16 windowRow, int16 windowColumn, int16 width, byte color) {
	int16 row;
	int16 column;
	if (!_messageState.window_Active)
		return;

	row = _messageState.textPos.row + windowRow;
	column = _messageState.textPos.column + windowColumn;
	clearBlock(row, column, row, column + width - 1, color);
}

bool TextMgr::inputGetEditStatus() {
	return _inputEditEnabled;
}

void TextMgr::inputEditOn() {
	if (!_inputEditEnabled) {
		_inputEditEnabled = true;
		if (_inputCursorChar) {
			displayCharacter(0x08); // backspace
		}
	}
}

void TextMgr::inputEditOff() {
	if (_inputEditEnabled) {
		_inputEditEnabled = false;
		if (_inputCursorChar) {
			displayCharacter(_inputCursorChar);
		}
	}
}

void TextMgr::inputSetCursorChar(int16 cursorChar) {
	_inputCursorChar = cursorChar;
}

byte TextMgr::inputGetCursorChar() {
	return _inputCursorChar;
}

void TextMgr::promptRow_Set(int16 row) {
	_promptRow = row;
}

int16 TextMgr::promptRow_Get() {
	return _promptRow;
}

void TextMgr::promptReset() {
	_promptCursorPos = 0;
	memset(_prompt, 0, sizeof(_prompt));
	memset(_promptPrevious, 0, sizeof(_promptPrevious));
}

void TextMgr::promptEnable() {
	_promptEnabled = true;
}
void TextMgr::promptDisable() {
	_promptEnabled = false;
}
bool TextMgr::promptIsEnabled() {
	return _promptEnabled;
}

void TextMgr::promptCharPress(int16 newChar) {
	int16 maxChars = 0;
	int16 scriptsInputLen = _vm->getVar(VM_VAR_MAX_INPUT_CHARACTERS);

	if (_messageState.dialogue_Open) {
		maxChars = TEXT_STRING_MAX_SIZE - 4;
	} else {
		maxChars = TEXT_STRING_MAX_SIZE - strlen(_vm->_game.strings[0]); // string 0 is the prompt string prefix
	}

	if (_promptCursorPos)
		maxChars--;

	if (scriptsInputLen < maxChars)
		maxChars = scriptsInputLen;

	inputEditOn();

	switch (newChar) {
	case AGI_KEY_BACKSPACE: {
		if (_promptCursorPos) {
			_promptCursorPos--;
			_prompt[_promptCursorPos] = 0;
			displayCharacter(newChar);

			promptRememberForAutoComplete();
		}
		break;
	}
	case 0x0A: // LF
		break;
	case AGI_KEY_ENTER: {
		if (_promptCursorPos) {
			// something got entered? -> process it and pass it to the scripts
			promptRememberForAutoComplete(true);

			memcpy(&_promptPrevious, &_prompt, sizeof(_prompt));
			// parse text
			_vm->_words->parseUsingDictionary((char *)&_prompt);

			_promptCursorPos = 0;
			_prompt[0] = 0;
			promptRedraw();
		}
		break;
	}
	default:
		if (maxChars > _promptCursorPos) {
			bool acceptableInput = false;

			// FEATURE: Sierra didn't check for valid characters (filtered out umlauts etc.)
			// In text-mode this sort of worked at least with the DOS interpreter
			// but as soon as invalid characters were used in graphics mode they weren't properly shown
			switch (_vm->getLanguage()) {
			case Common::RU_RUS:
				if (newChar >= 0x20)
					acceptableInput = true;
				break;
			default:
				if ((newChar >= 0x20) && (newChar <= 0x7f))
					acceptableInput = true;
				break;
			}

			if (acceptableInput) {
				_prompt[_promptCursorPos] = newChar;
				_promptCursorPos++;
				_prompt[_promptCursorPos] = 0;
				displayCharacter(newChar);

				promptRememberForAutoComplete();
			}
		}
		break;
	}

	inputEditOff();
}

void TextMgr::promptCancelLine() {
	while (_promptCursorPos) {
		promptCharPress(0x08); // Backspace until prompt is empty
	}
}

void TextMgr::promptEchoLine() {
	int16 previousLen = strlen((char *)_promptPrevious);

	if (_promptCursorPos < previousLen) {
		inputEditOn();

		while (_promptPrevious[_promptCursorPos]) {
			promptCharPress(_promptPrevious[_promptCursorPos]);
		}
		promptRememberForAutoComplete();

		inputEditOff();
	}
}

void TextMgr::promptRedraw() {
	char *textPtr = nullptr;

	if (_promptEnabled) {
		inputEditOn();
		clearLine(_promptRow, _textAttrib.background);
		charPos_Set(_promptRow, 0);
		// agi_printf(str_wordwrap(msg, state.string[0], 40) );

		textPtr = _vm->_game.strings[0];
		textPtr = stringPrintf(textPtr);
		textPtr = stringWordWrap(textPtr, 40);

		displayText(textPtr);
		displayText((char *)&_prompt);
		inputEditOff();
	}
}

// for AGI1
void TextMgr::promptClear() {
	clearLine(_promptRow, _textAttrib.background);
}

void TextMgr::promptRememberForAutoComplete(bool entered) {
#ifdef __DS__
	DS::findWordCompletions((char *)_prompt);
#endif
}

bool TextMgr::stringWasEntered() {
	return _inputStringEntered;
}

void TextMgr::stringSet(const char *text) {
	strncpy((char *)_inputString, text, sizeof(_inputString));
	_inputString[sizeof(_inputString) - 1] = 0; // terminator
}

void TextMgr::stringPos_Get(int16 &row, int16 &column) {
	row = _inputStringRow;
	column = _inputStringColumn;
}
int16 TextMgr::stringGetMaxLen() {
	return _inputStringMaxLen;
}

void TextMgr::stringEdit(int16 stringMaxLen) {
	int16 inputStringLen = strlen((const char *)_inputString);

	// Remember current position for predictive dialog
	_inputStringRow = _textPos.row;
	_inputStringColumn = _textPos.column;

	// Caller can set the input string
	_inputStringCursorPos = 0;
	while (_inputStringCursorPos < inputStringLen) {
		displayCharacter(_inputString[_inputStringCursorPos]);
		_inputStringCursorPos++;
	}

	// should never happen unless there is a coding glitch
	assert(_inputStringCursorPos <= stringMaxLen);

	_inputStringMaxLen = stringMaxLen;
	_inputStringEntered = false;

	inputEditOff();

	do {
		_vm->mainCycle();
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	inputEditOn();

	// Forget non-blocking text, user was asked to enter something
	_vm->nonBlockingText_Forget();
}

void TextMgr::stringCharPress(int16 newChar) {
	inputEditOn();

	switch (newChar) {
	case 0x3:       // ctrl-c
	case 0x18: {    // ctrl-x
		// clear string
		while (_inputStringCursorPos) {
			_inputStringCursorPos--;
			_inputString[_inputStringCursorPos] = 0;
			displayCharacter(0x08);
		}
		break;
	}

	case AGI_KEY_BACKSPACE: {
		if (_inputStringCursorPos) {
			_inputStringCursorPos--;
			_inputString[_inputStringCursorPos] = 0;
			displayCharacter(newChar);

			stringRememberForAutoComplete();
		}
		break;
	}

	case AGI_KEY_ENTER: {
		stringRememberForAutoComplete(true);

		_inputStringEntered = true;

		_vm->cycleInnerLoopInactive(); // exit GetString-loop
		break;
	}

	case AGI_KEY_ESCAPE: {
		_inputString[0] = 0;
		_inputStringCursorPos = 0;
		_inputStringEntered = false;

		_vm->cycleInnerLoopInactive(); // exit GetString-loop
		break;
	}

	default:
		if (_inputStringMaxLen > _inputStringCursorPos) {
			bool acceptableInput = false;

			// FEATURE: Sierra didn't check for valid characters (filtered out umlauts etc.)
			// In text-mode this sort of worked at least with the DOS interpreter
			// but as soon as invalid characters were used in graphics mode they weren't properly shown
			switch (_vm->getLanguage()) {
			case Common::RU_RUS:
				if (newChar >= 0x20)
					acceptableInput = true;
				break;
			default:
				if ((newChar >= 0x20) && (newChar <= 0x7f))
					acceptableInput = true;
				break;
			}

			if (acceptableInput) {
				if ((_vm->_game.cycleInnerLoopType == CYCLE_INNERLOOP_GETSTRING) || ((newChar >= '0') && (newChar <= '9'))) {
					// Additionally check for GETNUMBER-mode, if character is a number
					// Sierra also did not do this
					_inputString[_inputStringCursorPos] = newChar;
					_inputStringCursorPos++;
					_inputString[_inputStringCursorPos] = 0;
					displayCharacter(newChar);

					stringRememberForAutoComplete();
				}
			}
		}
		break;
	}

	inputEditOff();
}

void TextMgr::stringRememberForAutoComplete(bool entered) {
#ifdef __DS__
	DS::findWordCompletions((char *)_inputString);
#endif
}

/**
 * Wrap text line to the specified width.
 * @param str  String to wrap.
 * @param len  Length of line.
 *
 * Based on GBAGI implementation with permission from the author
 */
char *TextMgr::stringWordWrap(const char *originalText, int16 maxWidth, int16 *calculatedWidthPtr, int16 *calculatedHeightPtr) {
	static char resultWrapBuffer[2000];
	char *outStr = resultWrapBuffer;
	const char *wordStartPtr;
	int16 lineLen = 0;
	int16 wordLen = 0;
	int curMaxWidth = 0;
	int curHeight = 0;

	assert(maxWidth > 0); // this routine would create heap corruption in case maxWidth <= 0

	while (*originalText) {
		wordStartPtr = originalText;

		while (*originalText != '\0' && *originalText != ' ' && *originalText != '\n' && *originalText != '\r')
			originalText++;

		wordLen = originalText - wordStartPtr;

		if (wordLen && *originalText == '\n' && originalText[-1] == ' ')
			wordLen--;

		if (wordLen + lineLen >= maxWidth) {
			// Check if outStr isn't msgBuf. If this is the case, outStr hasn't advanced
			// yet, so no output has been written yet
			if (outStr != resultWrapBuffer) {
				if (outStr[-1] == ' ')
					outStr[-1] = '\n';
				else
					*outStr++ = '\n';
			}
			curHeight++;

			lineLen = 0;

			while (wordLen >= maxWidth) {
				curMaxWidth = maxWidth;

				memcpy(outStr, wordStartPtr, maxWidth);

				wordLen -= maxWidth;
				outStr += maxWidth;
				wordStartPtr  += maxWidth;
				*outStr++ = '\n';
				curHeight++;
			}
		}

		if (wordLen) {
			memcpy(outStr, wordStartPtr, wordLen);
			outStr += wordLen;
		}
		lineLen += wordLen + 1;

		if (lineLen > curMaxWidth) {
			curMaxWidth = lineLen;

			if (*originalText == '\0' || *originalText == ' ' || *originalText == '\n' || *originalText == '\r')
				curMaxWidth--;
		}

		if (*originalText == '\n') {
			lineLen = 0;
			curHeight++;
		}

		if (*originalText)
			*outStr++ = *originalText++;
	}
	*outStr = '\0';
	curHeight++;

	if (calculatedWidthPtr) {
		*calculatedWidthPtr = curMaxWidth;
	}
	if (calculatedHeightPtr) {
		*calculatedHeightPtr = curHeight;
	}
	return resultWrapBuffer;
}

// ===============================================================

static void safeStrcat(Common::String &p, const char *t) {
	if (t != NULL)
		p += t;
}

/**
 * Formats AGI string.
 * This function turns a AGI string into a real string expanding values
 * according to the AGI format specifiers.
 * @param s  string containing the format specifier
 * @param n  logic number
 */
char *TextMgr::stringPrintf(const char *originalText) {
	static char resultPrintfBuffer[2000];
	Common::String resultString;
	char z[16];

	debugC(3, kDebugLevelText, "logic %d, '%s'", _vm->_game.curLogicNr, originalText);

	while (*originalText) {
		switch (*originalText) {
		case '%':
			originalText++;
			switch (*originalText++) {
				int i;
			case 'v':
				i = strtoul(originalText, NULL, 10);
				while (*originalText >= '0' && *originalText <= '9')
					originalText++;
				sprintf(z, "%015i", _vm->getVar(i));

				i = 99;
				if (*originalText == '|') {
					originalText++;
					i = strtoul(originalText, NULL, 10);
					while (*originalText >= '0' && *originalText <= '9')
						originalText++;
				}

				if (i == 99) {
					// remove all leading 0
					// don't remove the 3rd zero if 000
					for (i = 0; z[i] == '0' && i < 14; i++)
						;
				} else {
					i = 15 - i;
				}
				safeStrcat(resultString, z + i);
				break;
			case '0':
				i = strtoul(originalText, NULL, 10) - 1;
				safeStrcat(resultString, _vm->objectName(i));
				break;
			case 'g':
				i = strtoul(originalText, NULL, 10) - 1;
				safeStrcat(resultString, _vm->_game.logics[0].texts[i]);
				break;
			case 'w':
				i = strtoul(originalText, NULL, 10) - 1;
				safeStrcat(resultString, _vm->_words->getEgoWord(i));
				break;
			case 's':
				i = strtoul(originalText, NULL, 10);
				safeStrcat(resultString, stringPrintf(_vm->_game.strings[i]));
				break;
			case 'm':
				i = strtoul(originalText, NULL, 10) - 1;
				if (_vm->_game.logics[_vm->_game.curLogicNr].numTexts > i)
					safeStrcat(resultString, stringPrintf(_vm->_game.logics[_vm->_game.curLogicNr].texts[i]));
				break;
			}

			while (*originalText >= '0' && *originalText <= '9')
				originalText++;
			break;

		case '\\':
			originalText++;
			// FALL THROUGH

		default:
			resultString += *originalText++;
			break;
		}
	}

	assert(resultString.size() < sizeof(resultPrintfBuffer));
	strcpy(resultPrintfBuffer, resultString.c_str());
	return resultPrintfBuffer;
}

} // End of namespace Agi
