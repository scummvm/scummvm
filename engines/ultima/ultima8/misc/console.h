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

#ifndef ULTIMA8_MISC_CONSOLE_H
#define ULTIMA8_MISC_CONSOLE_H

#include "ultima/ultima8/misc/istring.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "common/debug.h"
#include "common/hashmap.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima8 {

//
// The Console
//
#define     CON_TEXTSIZE        32768
#define     CON_PUTCHAR_SIZE    256
#define     CON_NUM_TIMES       4

// For Enable/Disable flags
#define     CON_STDOUT          0x01
#define     CON_STDERR          0x02

class ODataSource;
class RenderSurface;
struct FixedWidthFont;

class ConsoleStream : public Common::WriteStream {
private:
	Std::Precision _precision;
public:
	ConsoleStream() : Common::WriteStream(), _precision(Std::dec) {
	}

	int32 pos() const override {
		return 0;
	}

	void Print(const char *fmt, ...) {
		va_list argptr;
		va_start(argptr, fmt);
		Common::String str = Common::String::vformat(fmt, argptr);
		va_end(argptr);

		write(str.c_str(), str.size());
	}

	ConsoleStream &operator<<(const char *s) {
		write(s, strlen(s));
		return *this;
	}

	ConsoleStream &operator<<(const void *ptr) {
		Common::String str = Common::String::format("%p", ptr);
		write(str.c_str(), str.size());
		return *this;
	}

	ConsoleStream &operator<<(const Common::String &str) {
		write(str.c_str(), str.size());
		return *this;
	}

	ConsoleStream &operator<<(int val) {
		Common::String str = Common::String::format(
			(_precision == Std::hex) ? "%x" : "%d", val);
		write(str.c_str(), str.size());
		return *this;
	}
};

template<class T>
class console_ostream : public ConsoleStream {
	uint32 write(const void *dataPtr, uint32 dataSize) override {
		Common::String str((const char *)dataPtr, (const char *)dataPtr + dataSize);
		debugN("%s", str.c_str());
		return dataSize;
	}
};

template<class T>
class console_err_ostream : public ConsoleStream {
public:
	uint32 write(const void *dataPtr, uint32 dataSize) override {
		Common::String str((const char *)dataPtr, dataSize);
		::warning("%s", str.c_str());
		return str.size();
	}
};

// Standard Output Stream Object
extern console_ostream<char> *ppout;
// Error Output Stream Object
extern console_err_ostream<char> *pperr;

#define pout (*ppout)
#define perr (*pperr)


class Console {
	char        _text[CON_TEXTSIZE];
	int32       _current;                // line where next message will be printed
	int32       _xOff;                   // offset in current line for next print
	int32       _display;                // bottom of console displays this line

	int32       _lineWidth;              // characters across screen
	int32       _totalLines;             // total lines in console scrollback

	int32       _visLines;

	bool        _wordWrap;               // Enable/Disable word wrapping
	bool        _cr;                     // Line feed marker

	int32       _putChar_count;          // Number of characters that have been putchar'd
	char        _putChar_buf[CON_PUTCHAR_SIZE];  // The Characters that have been putchar'd

	uint32      _stdOutputEnabled;

	// Standard Output Stream Object
	console_ostream<char> _strOut;
	// Error Output Stream Object
	console_err_ostream<char> _errOut;

	// stdout and stderr redirection
	ODataSource *_stdout_redir;
	ODataSource *_stderr_redir;

	// Confont
	FixedWidthFont      *_conFont;

	void (*_autoPaint)(void);

	uint32 _int;                // mask to determine which messages are printed or not

	int _msgMask;
	// Overlay timing
	uint32      _frameNum;
	uint32      _times[CON_NUM_TIMES];   // framenum the line was generated
	// for transparent notify lines
public:
	enum SpecialChars {
		Tab         = '\t',
		Backspace   = '\b',
		Enter       = '\n'
	};


	Console();
	~Console();

	// Clear the buffer
	void    Clear();

	// Dump the buffer to a text file
	void    Dump(const char *name);

	// Resize the console buffer (on screen change)
	void    CheckResize(int scrwidth);

	// Draw the Console
	void    DrawConsole(RenderSurface *surf, int height);

	// Draw the Console Notify Overlay
	void    DrawConsoleNotify(RenderSurface *surf);

	// Set the console font texture
	void    SetConFont(FixedWidthFont *cf) {
		_conFont = cf;
	}

	// Get the console font texture
	FixedWidthFont *GetConFont() {
		return _conFont;
	}

	// Autopaint will cause the GUIApp to re-paint everything after a linefeed
	void    SetAutoPaint(void (*func)(void)) {
		_autoPaint = func;
	}

	// Scroll through the console. - is up + is down
	void    ScrollConsole(int32 lines);

	// Redirection
	void    RedirectOutputStream(uint32 mask, ODataSource *ds) {
		if (mask & CON_STDOUT) _stdout_redir = ds;
		if (mask & CON_STDERR) _stderr_redir = ds;
	}

	// Enabling output
	void    setOutputEnabled(uint32 mask) {
		_stdOutputEnabled |= mask;
	}

	// Disabling output
	void    unsetOutputEnabled(uint32 mask) {
		_stdOutputEnabled &= ~mask;
	}

	// Enabling output
	uint32  getOutputEnabled() {
		return _stdOutputEnabled;
	}

	void setFrameNum(uint32 f) {
		_frameNum = f;
	}

	//
	// STDOUT Methods
	//

	// Print a text string to the console, and output to stdout
	void    Print(const char *txt);
	void    Print(const int mm, const char *txt);

	// printf, and output to stdout
	int32   Printf(const char *fmt, ...);
	int32   Printf(const int mm, const char *fmt, ...); // with msg filtering

	// printf, and output to stdout (va_list)
	int32   vPrintf(const char *fmt, va_list);

	// putchar, and output to stdout
	void    Putchar(int c);

	// Print a text string to the console, and output to stdout
	void    PrintRaw(const char *txt, int n);


	//
	// STDERR Methods
	//

	// Print a text string to the console, and output to stderr
	void    Print_err(const char *txt);
	void    Print_err(const int mm, const char *txt);

	// printf, and output to stderr
	int32   Printf_err(const char *fmt, ...);
	int32   Printf_err(const int mm, const char *fmt, ...);

	// printf, and output to stderr (va_list)
	int32   vPrintf_err(const char *fmt, va_list);

	// putchar, and output to stderr
	void    Putchar_err(int c);

	// Print a text string to the console, and output to stderr
	void    PrintRaw_err(const char *txt, int n);


	// Enable/Disable word wrapping
	void    EnableWordWrap() {
		_wordWrap = true;
	}
	void    DisableWordWrap() {
		_wordWrap = false;
	}

	//
	// Console Commands
	//

	//! Execute the currently queued console command
	void            ExecuteCommandBuffer();

	//! Scroll the command history
	//! \param num Number of commands to scroll through (neg numbers are older)
	void            ScrollCommandHistory(int num);

	//! Clear the queued console command buffer
	void            ClearCommandBuffer();

	//! Add a character to the Queued Console command input buffer
	//! \param ch Character to add.
	//! \note '\n' will execute the command
	//! \note '\t' will do command completion
	//! \note '\b' will do backspace
	void            AddCharacterToCommandBuffer(int ch);

	//! Delete the character before or after the current cursor position
	//! \param num Number of chars to delete and direction (neg = left, pos = right)
	void            DeleteCommandBufferChars(int num);

	//! Move the command input cursor
	//! \param num Number of chars to move by
	void            MoveCommandCursor(int num);

	//! Toggle command input insert mode
	void            ToggleCommandInsert() {
		commandInsert = !commandInsert;
	}

private:

	// Print a text string to the console
	void    PrintInternal(const char *txt);

	// Print a text string to the console
	void    PrintRawInternal(const char *txt, int n);

	// Put char
	void    PutcharInternal(int c);

	// Add a linefeed to the buffer
	void    Linefeed();

	// Print the Putchar data, if possible
	void    PrintPutchar();

	// Console Commands
	int                         commandCursorPos;
	bool                        commandInsert;
	int                         commandHistoryPos;
};

// Console object
extern  Console *con;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
