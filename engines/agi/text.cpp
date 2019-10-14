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

#include "common/config-manager.h"
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

	_systemUI = NULL;

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
	_inputStringEntered = false;
	_inputStringMaxLen = 0;
	_inputStringCursorPos = 0;
	_inputString[0] = 0;

	configureScreen(2);

	_messageBoxCancelled = false;

	_optionCommandPromptWindow = false;

	if (ConfMan.getBool("commandpromptwindow")) {
		_optionCommandPromptWindow = true;
	}
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
	_gfx->setRenderStartOffset(row_Min * FONT_VISUAL_HEIGHT);
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
			} else {
				if (foreground > 14) {
					_textAttrib.combinedForeground = 3;
				} else {
					_textAttrib.combinedForeground = charAttrib_CGA_Conversion[foreground & 0x0F];
				}
				_textAttrib.combinedBackground = 0;
			}
			break;
		case Common::kRenderHercA:
		case Common::kRenderHercG:
			if (background) {
				_textAttrib.combinedForeground = 0;
				_textAttrib.combinedBackground = 1;
			} else {
				_textAttrib.combinedForeground = 1;
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
	uint32 windowTimer = _vm->getVar(VM_VAR_WINDOW_AUTO_CLOSE_TIMER);
	debugC(3, kDebugLevelText, "blocking window v21=%d", windowTimer);

	windowTimer = windowTimer * 10; // 1 = 0.5 seconds
	_messageBoxCancelled = false;

	_vm->inGameTimerResetPassedCycles();
	_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_MESSAGEBOX);
	do {
		_vm->processAGIEvents();
		_vm->inGameTimerUpdate();

		if (windowTimer > 0) {
			if (_vm->inGameTimerGetPassedCycles() >= windowTimer) {
				// Timer reached, close automatically
				_vm->cycleInnerLoopInactive();
			}
		}
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	_vm->inGameTimerResetPassedCycles();

	_vm->setVar(VM_VAR_WINDOW_AUTO_CLOSE_TIMER, 0);

	closeWindow();
	_vm->_noSaveLoadAllowed = false;

	if (_messageBoxCancelled)
		return false;
	return true;
}

void TextMgr::messageBox_KeyPress(uint16 newKey) {
	switch (newKey) {
	case AGI_KEY_ENTER:
		_vm->cycleInnerLoopInactive(); // exit messagebox-loop
		break;
	case AGI_KEY_ESCAPE:
		_messageBoxCancelled = true;
		_vm->cycleInnerLoopInactive(); // exit messagebox-loop
		break;
	case AGI_MOUSE_BUTTON_LEFT: {
		// Check, if mouse cursor is within message box
		// If it is, take the click as ENTER.
		// That's what AGI on Amiga + Apple IIgs did.
		// On Atari ST at least via emulator it seems that the mouse cursor froze when messageboxes were diplayed.
		if (isMouseWithinMessageBox()) {
			_vm->cycleInnerLoopInactive(); // exit messagebox-loop
		}
		break;
	}
	default:
		break;
	}
}

void TextMgr::drawMessageBox(const char *textPtr, int16 forcedHeight, int16 wantedWidth, bool forcedWidth) {
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
	if (forcedHeight)
		_messageState.textSize_Height = forcedHeight;

	if (forcedWidth) {
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
	_messageState.backgroundPos_y = (startingRow * FONT_VISUAL_HEIGHT) - 5;
	// original AGI used lowerY here, calculated using (_messageState.textPos_Edge.row - _window_Row_Min + 1) * FONT_VISUAL_HEIGHT + 4;

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

void TextMgr::getMessageBoxInnerDisplayDimensions(int16 &x, int16 &y, int16 &width, int16 &height) {
	if (!_messageState.window_Active)
		return;

	y = _messageState.textPos.row;
	x = _messageState.textPos.column;
	width = _messageState.textSize_Width;
	height = _messageState.textSize_Height;
	_gfx->translateFontRectToDisplayScreen(x, y, width, height);
}

bool TextMgr::isMouseWithinMessageBox() {
	// Find out, where current mouse cursor actually is
	int16 mouseY = _vm->_mouse.pos.y;
	int16 mouseX = _vm->_mouse.pos.x;

	if (_messageState.window_Active) {
		_gfx->translateDisplayPosToGameScreen(mouseX, mouseY);

		if ((mouseX >= _messageState.backgroundPos_x) && (mouseX < (_messageState.backgroundPos_x + _messageState.backgroundSize_Width))) {
			if ((mouseY >= _messageState.backgroundPos_y) && (mouseY < (_messageState.backgroundPos_y + _messageState.backgroundSize_Height))) {
				return true;
			}
		}
	}
	return false;
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

	int16 x = column_Upper;
	int16 y = row_Upper;
	int16 width = (column_Lower + 1 - column_Upper);
	int16 height = (row_Lower + 1 - row_Upper);
	_gfx->translateFontRectToDisplayScreen(x, y, width, height);

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

void TextMgr::promptKeyPress(uint16 newKey) {
	int16 maxChars = 0;
	int16 scriptsInputLen = _vm->getVar(VM_VAR_MAX_INPUT_CHARACTERS);

	bool acceptableInput = false;

	// FEATURE: Sierra didn't check for valid characters (filtered out umlauts etc.)
	// In text-mode this sort of worked at least with the DOS interpreter
	// but as soon as invalid characters were used in graphics mode they weren't properly shown
	switch (_vm->getLanguage()) {
	case Common::RU_RUS:
		if (newKey >= 0x20)
			acceptableInput = true;
		break;
	default:
		if ((newKey >= 0x20) && (newKey <= 0x7f))
			acceptableInput = true;
		break;
	}

	if (_optionCommandPromptWindow) {
		// Forward to command prompt window, using last command
		if (acceptableInput) {
			promptCommandWindow(false, newKey);
		}
		return;
	}

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

	switch (newKey) {
	case AGI_KEY_BACKSPACE: {
		if (_promptCursorPos) {
			_promptCursorPos--;
			_prompt[_promptCursorPos] = 0;
			displayCharacter(newKey);

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
			if (acceptableInput) {
				_prompt[_promptCursorPos] = newKey;
				_promptCursorPos++;
				_prompt[_promptCursorPos] = 0;
				displayCharacter(newKey);

				promptRememberForAutoComplete();
			}
		}
		break;
	}

	inputEditOff();
}

void TextMgr::promptCancelLine() {
	if (_optionCommandPromptWindow) {
		// Abort, in case command prompt window is active
		return;
	}

	while (_promptCursorPos) {
		promptKeyPress(0x08); // Backspace until prompt is empty
	}
}

void TextMgr::promptEchoLine() {
	int16 previousLen = strlen((char *)_promptPrevious);

	if (_optionCommandPromptWindow) {
		// Forward to command prompt window, using last command
		promptCommandWindow(true, 0);
		return;
	}

	if (_promptCursorPos < previousLen) {
		inputEditOn();

		while (_promptPrevious[_promptCursorPos]) {
			promptKeyPress(_promptPrevious[_promptCursorPos]);
		}
		promptRememberForAutoComplete();

		inputEditOff();
	}
}

void TextMgr::promptRedraw() {
	char *textPtr = nullptr;

	if (_promptEnabled) {
		if (_optionCommandPromptWindow) {
			// Abort, in case command prompt window is active
			return;
		}

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
	if (_optionCommandPromptWindow) {
		// Abort, in case command prompt window is active
		return;
	}
	clearLine(_promptRow, _textAttrib.background);
}

void TextMgr::promptRememberForAutoComplete(bool entered) {
#ifdef __DS__
	DS::findWordCompletions((char *)_prompt);
#endif
}

void TextMgr::promptCommandWindow(bool recallLastCommand, uint16 newKey) {
	Common::String commandText;

	if (recallLastCommand) {
		commandText += Common::String((char *)_promptPrevious);
	}
	if (newKey) {
		if (newKey != ' ') {
			// Only add char, when it's not a space.
			// Original AGI did not filter space, but it makes no sense to start with a space.
			// Space would get filtered anyway during dictionary parsing.
			commandText += newKey;
		}
	}

	if (_systemUI->askForCommand(commandText)) {
		if (commandText.size()) {
			// Something actually was entered?
			strncpy((char *)&_prompt, commandText.c_str(), sizeof(_prompt));
			promptRememberForAutoComplete(true);
			memcpy(&_promptPrevious, &_prompt, sizeof(_prompt));
			// parse text
			_vm->_words->parseUsingDictionary((char *)&_prompt);

			_prompt[0] = 0;
		}
	}
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

	if (_inputCursorChar) {
		// Cursor character is shown, which means we are one beyond the start of the input
		// Adjust the column for predictive input dialog
		_inputStringColumn--;
	}

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
		_vm->processAGIEvents();
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	inputEditOn();

	// Forget non-blocking text, user was asked to enter something
	_vm->nonBlockingText_Forget();
}

void TextMgr::stringKeyPress(uint16 newKey) {
	inputEditOn();

	switch (newKey) {
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
			displayCharacter(newKey);

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
				if (newKey >= 0x20)
					acceptableInput = true;
				break;
			default:
				if ((newKey >= 0x20) && (newKey <= 0x7f))
					acceptableInput = true;
				break;
			}

			if (acceptableInput) {
				if ((_vm->_game.cycleInnerLoopType == CYCLE_INNERLOOP_GETSTRING) || ((newKey >= '0') && (newKey <= '9'))) {
					// Additionally check for GETNUMBER-mode, if character is a number
					// Sierra also did not do this
					_inputString[_inputStringCursorPos] = newKey;
					_inputStringCursorPos++;
					_inputString[_inputStringCursorPos] = 0;
					displayCharacter(newKey);

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
 * Wraps text line to the specified width.
 * @param originalText  String to wrap.
 * @param maxWidth      Length of line.
 */
char *TextMgr::stringWordWrap(const char *originalText, int16 maxWidth, int16 *calculatedWidthPtr, int16 *calculatedHeightPtr) {
	static char resultWrappedBuffer[2000];
	int16 boxWidth = 0;
	int16 boxHeight = 0;
	int16 lineWidth = 0; // width of current line

	int16 lineWidthLeft = maxWidth; // width left of current line

	int16 wordStartPos = 0;
	int16 wordLen = 0;
	int16 curReadPos = 0;
	int16 curWritePos = 0;
	byte  wordEndChar = 0;

	//memset(resultWrappedBuffer, 0, sizeof(resultWrappedBuffer)); for debugging

	// Good testcases:
	// King's Quest 1 intro:              the scrolling text is filled up with spaces, so that old lines are erased
	// Apple IIgs restart system UI:      spaces used to make the window larger
	// Gold Rush Stagecoach path room 60: "  Lake Michigan!", with max length 9 -> should get split into "  Lake" / "Michigan!"

	while (originalText[curReadPos]) {
		// Try to find out length of next word

		// If first character is a space, skip it, so that we process at least this space
		if (originalText[curReadPos] == ' ')
			curReadPos++;

		while (originalText[curReadPos]) {
			if (originalText[curReadPos] == ' ')
				break;
			if (originalText[curReadPos] == 0x0A)
				break;
			curReadPos++;
		}
		wordEndChar = originalText[curReadPos];

		// Calculate word length
		wordLen = curReadPos - wordStartPos;

		if (wordLen >= lineWidthLeft) {
			// Not enough space left

			// If first character right after the new line is a space, skip over it
			if (wordLen) {
				if (originalText[wordStartPos] == ' ') {
					wordStartPos++;
					wordLen--;
				}
			}

			if (wordLen > maxWidth) {
				// Word way too long, split it in half
				curReadPos = curReadPos - (wordLen - maxWidth);
				wordLen = maxWidth;
			}

			// Add new line
			resultWrappedBuffer[curWritePos++] = 0x0A;
			if (lineWidth > boxWidth)
				boxWidth = lineWidth;
			boxHeight++; lineWidth = 0;
			lineWidthLeft = maxWidth;

			// Reached absolute maximum? -> exit now
			if (boxHeight >= HEIGHT_MAX)
				break;
		}

		// Copy current word over
		memcpy(&resultWrappedBuffer[curWritePos], &originalText[wordStartPos], wordLen);
		lineWidth += wordLen;
		lineWidthLeft -= wordLen;
		curWritePos += wordLen;

		if (wordEndChar == 0x0A) {
			// original text had a new line, so force it
			curReadPos++;

			resultWrappedBuffer[curWritePos++] = 0x0A;
			if (lineWidth > boxWidth)
				boxWidth = lineWidth;
			boxHeight++; lineWidth = 0;
			lineWidthLeft = maxWidth;

			// Reached absolute maximum? -> exit now
			if (boxHeight >= HEIGHT_MAX)
				break;
		}

		wordStartPos = curReadPos;
	}

	resultWrappedBuffer[curWritePos] = 0;

	if (curReadPos > 0) {
		if (lineWidth > boxWidth)
			boxWidth = lineWidth;
		boxHeight++;
	}

	if (calculatedWidthPtr) {
		*calculatedWidthPtr = boxWidth;
	}
	if (calculatedHeightPtr) {
		*calculatedHeightPtr = boxHeight;
	}
	return resultWrappedBuffer;
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
			default:
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
	Common::strlcpy(resultPrintfBuffer, resultString.c_str(), 2000);
	return resultPrintfBuffer;
}

} // End of namespace Agi
