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

#include "gui/console.h"
#include "common/savefile.h"
#include "gui/widgets/scrollbar.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"

#include "base/version.h"

#include "common/system.h"
#include "common/util.h"

#include "graphics/fontman.h"

namespace GUI {

#define kConsoleCharWidth  (_font->getCharWidth('M'))
#define kConsoleLineHeight (_font->getFontHeight())

#define HISTORY_FILENAME "scummvm-history.txt"

enum {
	kConsoleSlideDownDuration = 200	// Time in milliseconds
};


#define PROMPT	") "

/* TODO:
 * - it is very inefficient to redraw the full thingy when just one char is added/removed.
 *   Instead, we could just copy the GFX of the blank console (i.e. after the transparent
 *   background is drawn, before any text is drawn). Then using that, it becomes trivial
 *   to erase a single character, do scrolling etc.
 * - a *lot* of others things, this code is in no way complete and heavily under progress
 */
ConsoleDialog::ConsoleDialog(float widthPercent, float heightPercent)
	: Dialog(0, 0, 1, 1),
	_widthPercent(widthPercent), _heightPercent(heightPercent) {

	// Reset the line buffer
	memset(_buffer, ' ', kBufferSize);

	// Dummy
	_scrollBar = new ScrollBarWidget(this, 0, 0, 5, 10);
	_scrollBar->setTarget(this);

	init();

	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;
	_firstLineInBuffer = 0;

	_caretVisible = false;
	_caretTime = 0;
	_selectionTime = 0;

	_slideMode = kNoSlideMode;
	_slideTime = 0;

	_promptStartPos = _promptEndPos = -1;

	// Init callback
	_callbackProc = nullptr;
	_callbackRefCon = nullptr;

	// Init History
	_historyIndex = 0;
	_historyLine = 0;
	_historySize = 0;

	// Display greetings & prompt
	print(gScummVMFullVersion);
	print("\nConsole is ready\n");
}

ConsoleDialog::~ConsoleDialog() {
	saveHistory();
}

void ConsoleDialog::init() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_font = &g_gui.getFont(ThemeEngine::kFontStyleConsole);

	_leftPadding = g_gui.xmlEval()->getVar("Globals.Console.Padding.Left", 0);
	_rightPadding = g_gui.xmlEval()->getVar("Globals.Console.Padding.Right", 0);
	_topPadding = g_gui.xmlEval()->getVar("Globals.Console.Padding.Top", 0);
	_bottomPadding = g_gui.xmlEval()->getVar("Globals.Console.Padding.Bottom", 0);

	// Calculate the real width/height (rounded to char/line multiples)
	_w = (uint16)(_widthPercent * screenW);
	_h = (uint16)((_heightPercent * screenH - 2) / kConsoleLineHeight);

	_w = _w - _w / 20;
	_h = _h * kConsoleLineHeight + 2;
	_x = _w / 40;

	// Set scrollbar dimensions
	int scrollBarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_scrollBar->resize(_w - scrollBarWidth - 1, 0, scrollBarWidth, _h, false);

	_pageWidth = (_w - scrollBarWidth - 2 - _leftPadding - _topPadding - scrollBarWidth) / kConsoleCharWidth;
	_linesPerPage = (_h - 2 - _topPadding - _bottomPadding) / kConsoleLineHeight;
	_linesInBuffer = kBufferSize / kCharsPerLine;

	_isDragging = false;

	_selBegin = -1;
	_selEnd = -1;

	_scrollDirection = 0;

	resetPrompt();
}

void ConsoleDialog::setPrompt(Common::String prompt) {
	_prompt = prompt;
}

void ConsoleDialog::resetPrompt() {
	_prompt = PROMPT;
}

void ConsoleDialog::clearBuffer() {
	// Reset the line buffer.
	memset(_buffer, ' ', kBufferSize);

	// Along with a few key vars.
	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;
	_firstLineInBuffer = 0;

	updateScrollBuffer();
}

void ConsoleDialog::slideUpAndClose() {
	if (_slideMode == kNoSlideMode) {
		_slideTime = g_system->getMillis();
		_slideMode = kUpSlideMode;
	}
}

void ConsoleDialog::open() {
	// TODO: find a new way to do this
	// Initiate sliding the console down. We do a very simple trick to achieve
	// this effect: we simply move the console dialog just above (outside) the
	// visible screen area, then shift it down in handleTickle() over a
	// certain period of time.

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	// Calculate the real width/height (rounded to char/line multiples)
	uint16 w = (uint16)(_widthPercent * screenW);
	uint16 h = (uint16)((_heightPercent * screenH - 2) / kConsoleLineHeight);

	h = h * kConsoleLineHeight + 2;
	w = w - w / 20;

	if (_w != w || _h != h)
		init();

	_y = -_h;

	_slideTime = g_system->getMillis();
	_slideMode = kDownSlideMode;

	Dialog::open();
	if ((_promptStartPos == -1) || (_currentPos > _promptEndPos)) {
		// we print a prompt, if this is the first time we are called or if the
		//  engine wrote onto us since the last call
		print(_prompt.c_str());
		_promptStartPos = _promptEndPos = _currentPos;
	}

	if (_historySize == 0) {
		loadHistory();
	}
}

void ConsoleDialog::close() {
	Dialog::close();
}

void ConsoleDialog::drawDialog(DrawLayer layerToDraw) {
	Dialog::drawDialog(layerToDraw);

	for (int line = 0; line < _linesPerPage; line++)
		drawLine(line);
}

void ConsoleDialog::drawLine(int line) {
	int x = _x + 1 + _leftPadding;
	int start = _scrollLine - _linesPerPage + 1;
	int y = _y + 2 + _topPadding;
	int limit = MIN(_pageWidth, (int)kCharsPerLine);

	y += line * kConsoleLineHeight;

	for (int column = 0; column < limit; column++) {
#if 0
		int l = (start + line) % _linesInBuffer;
		byte c = buffer(l * kCharsPerLine + column);
#else
		int idx = (start + line) * kCharsPerLine + column;
		byte c = buffer(idx);
#endif
		if (idx >= MIN(_selBegin, _selEnd) && idx < MAX(_selBegin, _selEnd))
			g_gui.theme()->drawChar(Common::Rect(x, y, x + kConsoleCharWidth, y + kConsoleLineHeight), c, _font, ThemeEngine::kFontColorNormal, ThemeEngine::kTextInversionFocus);
		else
			g_gui.theme()->drawChar(Common::Rect(x, y, x + kConsoleCharWidth, y + kConsoleLineHeight), c, _font);
		x += kConsoleCharWidth;
	}
}

void ConsoleDialog::reflowLayout() {
	init();

	_scrollLine = _promptEndPos / kCharsPerLine;
	if (_scrollLine < _linesPerPage - 1)
		_scrollLine = _linesPerPage - 1;
	updateScrollBuffer();

	Dialog::reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void ConsoleDialog::handleTickle() {
	uint32 time = g_system->getMillis();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
	if (_selectionTime < time) {
		_selectionTime += kDraggingTime;
		if (_isDragging && _scrollDirection != 0) {
			handleMouseWheel(0, 0, -_scrollDirection);
			_selEnd -= kCharsPerLine * _scrollDirection;
			if (clampSelection(_selEnd))
				// Scrolled as far as possible. Don't re-execute this block
				// unnecessarily.
				_scrollDirection = 0;
		}
	}
	// Perform the "slide animation".
	if (_slideMode != kNoSlideMode) {
		const float tmp = (float)(g_system->getMillis() - _slideTime) / kConsoleSlideDownDuration;
		if (_slideMode == kUpSlideMode) {
			_y = (int)(_h * (0.0 - tmp));
		} else {
			_y = (int)(_h * (tmp - 1.0));
		}

		if (_slideMode == kDownSlideMode && _y > 0) {
			// End the slide
			_slideMode = kNoSlideMode;
			_y = 0;
			g_gui.scheduleTopDialogRedraw();
		} else if (_slideMode == kUpSlideMode && _y <= -_h) {
			// End the slide
			//_slideMode = kNoSlideMode;
			close();
		} else
			g_gui.scheduleFullRedraw();
	}

	_scrollBar->handleTickle();
}

void ConsoleDialog::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

Common::String ConsoleDialog::getUserInput() {
	assert(_promptEndPos >= _promptStartPos);
	int len = _promptEndPos - _promptStartPos;

	// Copy the user input to str
	Common::String str;
	for (int i = 0; i < len; i++)
		str.insertChar(buffer(_promptStartPos + i), i);

	return str;
}

void ConsoleDialog::handleKeyDown(Common::KeyState state) {
	if (_slideMode != kNoSlideMode)
		return;

	switch (state.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER: {
		if (_caretVisible)
			drawCaret(true);

		nextLine();

		bool keepRunning = true;

		Common::String userInput = getUserInput();
		if (!userInput.empty()) {
			// Add the input to the history
			addToHistory(userInput);

			// Pass it to the input callback, if any
			if (_callbackProc)
				keepRunning = (*_callbackProc)(this, userInput.c_str(), _callbackRefCon);
		}

		print(_prompt.c_str());
		_promptStartPos = _promptEndPos = _currentPos;

		g_gui.scheduleTopDialogRedraw();
		if (!keepRunning)
			slideUpAndClose();
		break;
		}

	case Common::KEYCODE_ESCAPE:
		slideUpAndClose();
		break;

	case Common::KEYCODE_BACKSPACE:
		if (_caretVisible)
			drawCaret(true);

		if (_currentPos > _promptStartPos) {
			_currentPos--;
			killChar();
		}
		scrollToCurrent();
		drawLine(pos2line(_currentPos));
		break;

	case Common::KEYCODE_TAB: {
		if (_completionCallbackProc) {
			int len = _currentPos - _promptStartPos;
			assert(len >= 0);
			char *str = new char[len + 1];

			// Copy the user input to str
			for (int i = 0; i < len; i++)
				str[i] = buffer(_promptStartPos + i);
			str[len] = '\0';

			Common::String completion;
			if ((*_completionCallbackProc)(this, str, completion, _callbackRefCon)) {
				if (_caretVisible)
					drawCaret(true);
				insertIntoPrompt(completion.c_str());
				scrollToCurrent();
				drawLine(pos2line(_currentPos));
			}
			delete[] str;
		}
		break;
		}

	// Keypad & special keys
	//   - if num lock is set, we always go to the default case
	//   - if num lock is not set, we either fall down to the special key case
	//     or ignore the key press in case of 0 (INSERT) or 5

	case Common::KEYCODE_KP0:
	case Common::KEYCODE_KP5:
		if (state.flags & Common::KBD_NUM)
			defaultKeyDownHandler(state);
		break;

	case Common::KEYCODE_KP_PERIOD:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_DELETE:
		if (_currentPos < _promptEndPos) {
			killChar();
			drawLine(pos2line(_currentPos));
		}
		break;

	case Common::KEYCODE_KP1:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_END:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			_scrollLine = _promptEndPos / kCharsPerLine;
			if (_scrollLine < _linesPerPage - 1)
				_scrollLine = _linesPerPage - 1;
			updateScrollBuffer();
		} else {
			_currentPos = _promptEndPos;
		}
		g_gui.scheduleTopDialogRedraw();
		break;

	case Common::KEYCODE_KP2:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_DOWN:
		historyScroll(-1);
		break;

	case Common::KEYCODE_KP3:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_PAGEDOWN:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			_scrollLine += _linesPerPage - 1;
			if (_scrollLine > _promptEndPos / kCharsPerLine) {
				_scrollLine = _promptEndPos / kCharsPerLine;
				if (_scrollLine < _firstLineInBuffer + _linesPerPage - 1)
					_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			}
			updateScrollBuffer();
			g_gui.scheduleTopDialogRedraw();
		}
		break;

	case Common::KEYCODE_KP4:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_LEFT:
		if (_currentPos > _promptStartPos)
			_currentPos--;
		drawLine(pos2line(_currentPos));
		break;

	case Common::KEYCODE_KP6:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_RIGHT:
		if (_currentPos < _promptEndPos)
			_currentPos++;
		drawLine(pos2line(_currentPos));
		break;

	case Common::KEYCODE_KP7:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_HOME:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			updateScrollBuffer();
		} else {
			_currentPos = _promptStartPos;
		}
		g_gui.scheduleTopDialogRedraw();
		break;

	case Common::KEYCODE_KP8:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_UP:
		historyScroll(+1);
		break;

	case Common::KEYCODE_KP9:
		if (state.flags & Common::KBD_NUM) {
			defaultKeyDownHandler(state);
			break;
		}
		// fall through
	case Common::KEYCODE_PAGEUP:
		if (state.hasFlags(Common::KBD_SHIFT)) {
			_scrollLine -= _linesPerPage - 1;
			if (_scrollLine < _firstLineInBuffer + _linesPerPage - 1)
				_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			updateScrollBuffer();
			g_gui.scheduleTopDialogRedraw();
		}
		break;

	default:
		defaultKeyDownHandler(state);
	}
}

void ConsoleDialog::defaultKeyDownHandler(Common::KeyState &state) {
	if (state.hasFlags(Common::KBD_CTRL)) {
		specialKeys(state.keycode);
	} else if ((state.ascii >= 32 && state.ascii <= 127) || (state.ascii >= 160 && state.ascii <= 255)) {
		_selBegin = -1;
		_selEnd = -1;
		drawDialog(kDrawLayerForeground);

		for (int i = _promptEndPos - 1; i >= _currentPos; i--)
			buffer(i + 1) = buffer(i);
		_promptEndPos++;
		printChar((byte)state.ascii);
		scrollToCurrent();
	}
}

void ConsoleDialog::insertIntoPrompt(const char* str) {
	unsigned int l = strlen(str);
	for (int i = _promptEndPos - 1; i >= _currentPos; i--)
		buffer(i + l) = buffer(i);
	for (unsigned int j = 0; j < l; ++j) {
		_promptEndPos++;
		printCharIntern(str[j]);
	}
}

void ConsoleDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		{
			int newPos = (int)data + _linesPerPage - 1 + _firstLineInBuffer;
			if (newPos != _scrollLine) {
				_scrollLine = newPos;
				g_gui.scheduleTopDialogRedraw();
			}
		}
		break;
	default:
		break;
	}
}

void ConsoleDialog::handleOtherEvent(const Common::Event &evt) {
	if (evt.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
		switch (evt.customType) {
		case kActionCopy: {
			if (_selBegin == -1 || _selEnd == -1) {
				Common::String userInput = getUserInput();
				if (!userInput.empty())
					g_system->setTextInClipboard(userInput);
			} else {
				Common::String str;
				Common::String whitespaces; // for dealing with trailing whitespaces
				for (int i = MIN(_selBegin, _selEnd); i < MAX(_selBegin, _selEnd); i++) {
					if (i % kCharsPerLine != kCharsPerLine - 1) {
						if (buffer(i) == ' ') {
							whitespaces += buffer(i); // to deal with trailing whitespaces
						} else {
							str += whitespaces;
							str += buffer(i);
							whitespaces.clear();
						}
					} else {
						whitespaces.clear();
						str += "\n";
					}
				}
				g_system->setTextInClipboard(str);
			}
		} break;
		case kActionPaste:
			if (g_system->hasTextInClipboard()) {
				Common::U32String text = g_system->getTextFromClipboard();
				insertIntoPrompt(text.encode().c_str());
				scrollToCurrent();
				drawLine(pos2line(_currentPos));

				_selBegin = -1;
				_selEnd = -1;
				drawDialog(kDrawLayerForeground);
			}
			break;
		default:
			break;
		}
	}
}

void ConsoleDialog::specialKeys(Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_a:
		_currentPos = _promptStartPos;
		g_gui.scheduleTopDialogRedraw();
		break;
	case Common::KEYCODE_d:
		if (_currentPos < _promptEndPos) {
			killChar();
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	case Common::KEYCODE_e:
		_currentPos = _promptEndPos;
		g_gui.scheduleTopDialogRedraw();
		break;
	case Common::KEYCODE_k:
		killLine();
		g_gui.scheduleTopDialogRedraw();
		break;
	case Common::KEYCODE_w:
		killLastWord();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void ConsoleDialog::killChar() {
	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = buffer(i + 1);
	if (_promptEndPos > _promptStartPos) {
		buffer(_promptEndPos) = ' ';
		_promptEndPos--;
	}
}

void ConsoleDialog::killLine() {
	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = ' ';
	_promptEndPos = _currentPos;
}

void ConsoleDialog::killLastWord() {
	int cnt = 0;
	bool space = true;
	while (_currentPos > _promptStartPos) {
		if (buffer(_currentPos - 1) == ' ') {
			if (!space)
				break;
		} else
			space = false;
		_currentPos--;
		cnt++;
	}

	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = buffer(i + cnt);
	if (_promptEndPos > _promptStartPos) {
		buffer(_promptEndPos) = ' ';
		_promptEndPos -= cnt;
	}
}

void ConsoleDialog::loadHistory() {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::InSaveFile *loadFile = saveFileMan->openRawFile(HISTORY_FILENAME);
	if (!loadFile) {
		return;
	}
	for (int i = 0; i < kHistorySize; ++i) {
		const Common::String &line = loadFile->readLine();
		if (line.empty()) {
			break;
		}
		addToHistory(line);
	}
	delete loadFile;
	debug("Read %i history entries", _historySize);
}

void ConsoleDialog::saveHistory() {
	if (_historySize == 0) {
		return;
	}
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::WriteStream *saveFile = saveFileMan->openForSaving(HISTORY_FILENAME, false);
	if (!saveFile) {
		warning("Failed to open " HISTORY_FILENAME " for writing");
		return;
	}

	// Saving the history entries in the proper order;
	// The most recent entry should be the last to be saved.
	// NOTE When the _history table is full, we need to start saving
	//      from one slot after (in a circular manner) the _historyIndex slot.
	//      In this case the _historyIndex slot contains the temporary stored user input,
	//      which we do not want to persist.
	//      This means that when full, (kHistorySize - 1) entries will be saved.
	//      When the table is not full, storing always begins from index 0.
	int idx = (kHistorySize == _historySize) ? ((_historyIndex + 1) % kHistorySize) : 0;
	int entriesWritten = 0;
	while (idx != _historyIndex) {
		if (!_history[idx].empty()) {
			saveFile->writeString(_history[idx]);
			saveFile->writeByte('\n');
			++entriesWritten;
		}
		idx = (idx + 1) % kHistorySize;
	}
	saveFile->finalize();
	delete saveFile;
	debug("Wrote %i history entries", entriesWritten);
}

void ConsoleDialog::addToHistory(const Common::String &str) {
	_history[_historyIndex] = str;
	_historyIndex = (_historyIndex + 1) % kHistorySize;
	_historyLine = 0;
	if (_historySize < kHistorySize)
		_historySize++;
}

void ConsoleDialog::historyScroll(int direction) {
	if (_historySize == 0)
		return;

	if (_historyLine == 0 && direction > 0)
		// Save current line in history
		_history[_historyIndex] = getUserInput();

	// Advance to the next line in the history
	// NOTE Due to temporarily storing the user input line in the
	//      _history table, without executing an addToHistory() call,
	//      that user input line is stored into the slot _historyIndex
	//      where the next committed command will replace it.
	//      However, since this slot is still a slot from the _history table,
	//      when the table is full (kHistorySize entries) and while scrolling
	//      the history upwards, the user can reach this slot (top most)
	//      and get their user input again instead of a historic entry.
	//      We prevent this by stopping upwards navigation one slot earlier,
	//      when the table is full.
	int line = _historyLine + direction;
	if ((direction < 0 && line < 0)
	    || (direction > 0 && (line > _historySize
	                         || (_historySize == kHistorySize && line == _historySize))) )
		return;
	_historyLine = line;

	// Hide caret if visible
	if (_caretVisible)
		drawCaret(true);

	// Remove the current user text
	_currentPos = _promptStartPos;
	killLine();

	// ... and ensure the prompt is visible
	scrollToCurrent();

	// Print the text from the history
	int idx;
	if (_historyLine > 0)
		idx = (_historyIndex - _historyLine + _historySize) % _historySize;
	else
		idx = _historyIndex;
	int length = _history[idx].size();
	for (int i = 0; i < length; i++)
		printCharIntern(_history[idx][i]);
	_promptEndPos = _currentPos;

	// Ensure once more the caret is visible (in case of very long history entries)
	scrollToCurrent();

	g_gui.scheduleTopDialogRedraw();
}

void ConsoleDialog::nextLine() {
	int line = _currentPos / kCharsPerLine;
	if (line == _scrollLine)
		_scrollLine++;
	_currentPos = (line + 1) * kCharsPerLine;

	updateScrollBuffer();
}


// Call this (at least) when the current line changes or when
// a new line is added
void ConsoleDialog::updateScrollBuffer() {
	int lastchar = MAX(_promptEndPos, _currentPos);
	int line = lastchar / kCharsPerLine;
	int numlines = (line < _linesInBuffer) ? line + 1 : _linesInBuffer;
	int firstline = line - numlines + 1;
	if (firstline > _firstLineInBuffer) {
		// clear old line from buffer
		for (int i = lastchar; i < (line+1) * kCharsPerLine; ++i)
			buffer(i) = ' ';
		_firstLineInBuffer = firstline;
	}

	_scrollBar->_numEntries = numlines;
	_scrollBar->_currentPos = _scrollBar->_numEntries - (line - _scrollLine + _linesPerPage);
	_scrollBar->_entriesPerPage = _linesPerPage;
	_scrollBar->recalc();
}

int ConsoleDialog::printFormat(int dummy, const char *format, ...) {
	va_list	argptr;

	va_start(argptr, format);
	int count = this->vprintFormat(dummy, format, argptr);
	va_end (argptr);
	return count;
}

int ConsoleDialog::vprintFormat(int dummy, const char *format, va_list argptr) {
	Common::String buf = Common::String::vformat(format, argptr);
	const int size = buf.size();

	print(buf.c_str());
	debugN("%s", buf.c_str());

	return size;
}

void ConsoleDialog::printChar(int c) {
	if (_caretVisible)
		drawCaret(true);

	printCharIntern(c);
	drawLine(pos2line(_currentPos));
}

void ConsoleDialog::printCharIntern(int c) {
	if (c == '\n')
		nextLine();
	else {
		buffer(_currentPos) = (char)c;
		_currentPos++;
		if (_currentPos % kCharsPerLine == _pageWidth) {
			nextLine();
			updateScrollBuffer();
		}
	}
}

void ConsoleDialog::print(const char *str) {
	if (_caretVisible)
		drawCaret(true);

	while (*str)
		printCharIntern(*str++);

	g_gui.scheduleTopDialogRedraw();
}

void ConsoleDialog::drawCaret(bool erase) {
	// TODO: use code from EditableWidget::drawCaret here
	int line = _currentPos / kCharsPerLine;
	int displayLine = line - _scrollLine + _linesPerPage - 1;

	// Only draw caret if visible
	if (!isVisible() || displayLine < 0 || displayLine >= _linesPerPage) {
		_caretVisible = false;
		return;
	}

	int x = _x + 1 + _leftPadding + (_currentPos % kCharsPerLine) * kConsoleCharWidth;
	int y = _y + 2 + _topPadding + displayLine * kConsoleLineHeight;

	_caretVisible = !erase;
	g_gui.theme()->drawCaret(Common::Rect(x, y, x + 1, y + kConsoleLineHeight), erase);
}

void ConsoleDialog::scrollToCurrent() {
	int line = _promptEndPos / kCharsPerLine;

	if (line + _linesPerPage <= _scrollLine) {
		// TODO - this should only occur for loong edit lines, though
	} else if (line > _scrollLine) {
		_scrollLine = line;
		updateScrollBuffer();
		g_gui.scheduleTopDialogRedraw();
	}
}

void ConsoleDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	Widget *w;

	w = findWidget(x, y);

	if (w) {
		if (!(w->getFlags() & WIDGET_IGNORE_DRAG))
			_dragWidget = w;

		if (w != _focusedWidget && w->wantsFocus()) {
			setFocusWidget(w);
		}

		w->handleMouseDown(x - (w->getAbsX() - _x), y - (w->getAbsY() - _y), button, clickCount);
	} else if (_selBegin == -1 || _selEnd == -1) {
		x = MIN(MAX(x, _leftPadding), kCharsPerLine * kConsoleCharWidth + _leftPadding);
		y = MIN(MAX(y, _topPadding), (decltype(y))_h - kConsoleLineHeight);

		int lineNumber = (y - _topPadding) / kConsoleLineHeight;
		int ind = (x - _leftPadding) / kConsoleCharWidth;
		_selBegin = (_scrollLine - _linesPerPage + 1 + lineNumber) * kCharsPerLine + ind;
		clampSelection(_selBegin);
		_selEnd = _selBegin;
		_isDragging = true;
	} else {
		_selBegin = -1;
		_selEnd = -1;
		drawDialog(kDrawLayerForeground);
	}
}

bool ConsoleDialog::clampSelection(int &sel) {
	int oldSel = sel;
	int lowerBound = 0;
	int upperBound;

	upperBound = MAX(_promptEndPos, _currentPos);
	upperBound = MAX(upperBound, _linesPerPage * kCharsPerLine); // at least the whole first page
	upperBound += kCharsPerLine - (upperBound % kCharsPerLine); // to end of line

	sel = MAX(lowerBound, MIN(upperBound, sel));
	return sel != oldSel;
}

void ConsoleDialog::handleMouseMoved(int x, int y, int button) {
	if (!_isDragging)
		Dialog::handleMouseMoved(x, y, button);
	else {
		int selEndPreviousMove = _selEnd;
		x = MIN(MAX(x, _leftPadding), kCharsPerLine * kConsoleCharWidth + _leftPadding);
		y = MIN(MAX(y, _topPadding), (decltype(y))_h - kConsoleLineHeight);
		int lineNumber = (y - _topPadding) / kConsoleLineHeight;
		lineNumber = MIN(lineNumber, _linesPerPage - 1);
		int col = (x - _leftPadding) / kConsoleCharWidth;
		_selEnd = (_scrollLine - _linesPerPage + 1 + lineNumber) * kCharsPerLine + col;
		clampSelection(_selEnd);

		if (_selEnd == selEndPreviousMove)
			return;

		if (lineNumber > _linesPerPage - 2)
			_scrollDirection = -1;
		else if (lineNumber < 1)
			_scrollDirection = 1;
		else
			_scrollDirection = 0;

		for (int i = MIN(_selEnd / kCharsPerLine, selEndPreviousMove / kCharsPerLine); i <= MAX(_selEnd / kCharsPerLine, selEndPreviousMove / kCharsPerLine); i++)
			drawLine(i - _scrollBar->_currentPos / _scrollBar->_singleStep);
	}
}

void ConsoleDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	Dialog::handleMouseUp(x, y, button, clickCount);
	_isDragging = false;
	if (_selBegin == _selEnd) {
		_selBegin = -1;
		_selEnd = -1;
	}
	_scrollDirection = 0;
}

} // End of namespace GUI
