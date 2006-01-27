/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 */

#ifndef CONSOLE_DIALOG_H
#define CONSOLE_DIALOG_H

#include "gui/dialog.h"
#include "gui/newgui.h"

#include <stdarg.h>

namespace GUI {

enum {
	kBufferSize	= 32768,
	kLineBufferSize = 256,

	kHistorySize = 20
};

class ScrollBarWidget;

class ConsoleDialog : public Dialog {
public:
	typedef bool (*InputCallbackProc)(ConsoleDialog *console, const char *input, void *refCon);
	typedef bool (*CompletionCallbackProc)(ConsoleDialog* console, const char *input, char*& completion, void *refCon);

protected:
	const Graphics::Font *_font;

	char	_buffer[kBufferSize];
	int		_linesInBuffer;

	int		_lineWidth;
	int		_linesPerPage;

	int		_currentPos;
	int		_scrollLine;
	int		_firstLineInBuffer;

	int		_promptStartPos;
	int 	_promptEndPos;

	bool	_caretVisible;
	uint32	_caretTime;

	enum SlideMode {
		kNoSlideMode,
		kUpSlideMode,
		kDownSlideMode
	};

	SlideMode	_slideMode;
	uint32	_slideTime;

	ScrollBarWidget *_scrollBar;

	// The _callbackProc is called whenver a data line is entered
	//
	InputCallbackProc _callbackProc;
	void *_callbackRefCon;

	// _completionCallbackProc is called when tab is pressed
	CompletionCallbackProc _completionCallbackProc;
	void *_completionCallbackRefCon;

	char _history[kHistorySize][kLineBufferSize];
	int _historySize;
	int _historyIndex;
	int _historyLine;

	float _widthPercent, _heightPercent;

	void slideUpAndClose();

public:
	ConsoleDialog(float widthPercent, float heightPercent);

	void open();
	void close();
	void drawDialog();

	void handleTickle();
	void handleScreenChanged();
	void handleMouseWheel(int x, int y, int direction);
	void handleKeyDown(uint16 ascii, int keycode, int modifiers);
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	int printf(const char *format, ...);
	int vprintf(const char *format, va_list argptr);
#undef putchar
	void putchar(int c);

	void setInputCallback(InputCallbackProc proc, void *refCon) {
		_callbackProc = proc;
		_callbackRefCon = refCon;
	}
	void setCompletionCallback(CompletionCallbackProc proc, void *refCon) {
		_completionCallbackProc = proc;
		_completionCallbackRefCon = refCon;
	}

protected:
	inline char &buffer(int idx) {
		return _buffer[idx % kBufferSize];
	}

	void drawCaret(bool erase);
	void putcharIntern(int c);
	void insertIntoPrompt(const char *str);
	void print(const char *str);
	void updateScrollBuffer();
	void scrollToCurrent();

	// Line editing
	void specialKeys(int keycode);
	void nextLine();
	void killChar();
	void killLine();
	void killLastWord();

	// History
	void addToHistory(const char *str);
	void historyScroll(int direction);
};

} // End of namespace GUI

#endif
