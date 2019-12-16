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
#include "ultima/ultima8/std/containers.h"
#include "common/debug.h"
#include "common/hashmap.h"
#include "common/file.h"

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

enum MsgMask {
	/* Output no message(s). For obvious reasons this probably should never need to be used... */
	MM_NONE = 0x00,
	/* general informative messages like the `virtual path "@home"...` stuff.
	    non-critical and in the general case non-useful unless trying to debug a problem. */
	MM_INFO = 0x01,
	/* similar in urgency to a general message, except it's just noting a possible, though
	    unlikely problem. The:
	    `Error mounting virtual path "@data": directory not found: /usr/local/share/pentagram`
	    message would be a good example of this, I believe. */
	MM_MINOR_WARN = 0x02,
	/* The compatriot to MINOR_WARN, this one effectively says "there's likely a problem here" */
	MM_MAJOR_WARN = 0x04,
	/* "We had a problem, and we're trying to recover from it, hopefully successfully." */
	MM_MINOR_ERR = 0x08,
	/* A message noting that we encountered a significant problem and that we're going to
	    (hopefully!) gracefully terminate the engine... Probably though a call to
	    CoreApp::application->ForceQuit();
	    "We had a rather significant problem. Shutting down pentagram now..."
	    */
	MM_MAJOR_ERR = 0x10,
	/* Significant failures that simply can't be recovered from, since there's either no way to
	    recover, or it's likely we'll crash in the final execution of the main loop before everything
	    shuts down.
	    Effectively things you'll want to be calling `exit(-1);` on, immediately after issuing the
	    warning. *grin* */
	MM_TERMINAL_ERR = 0x20,
	/* Displays no matter what the warning level. Only really useful to setup the relevant internal masks. */
	MM_ALL = 0x3F
	         /* TODO:
	             Thoughts:
	             * Maybe there should be a MM_UNINPORTANT for MM_INFO messages that, whilst
	                 potentially useful, are useful in very, very rare circumstances...
	                 Would be turned on by a -v flag or something.
	             * This is probably too detailed already though.
	         */
};

class Console {
	char        text[CON_TEXTSIZE];
	int32       current;                // line where next message will be printed
	int32       xoff;                   // offset in current line for next print
	int32       display;                // bottom of console displays this line

	int32       linewidth;              // characters across screen
	int32       totallines;             // total lines in console scrollback

	int32       vislines;

	bool        wordwrap;               // Enable/Disable word wrapping
	bool        cr;                     // Line feed marker

	int32       putchar_count;          // Number of characters that have been putchar'd
	char        putchar_buf[CON_PUTCHAR_SIZE];  // The Characters that have been putchar'd

	uint32      std_output_enabled;

	// stdout and stderr redirection
	ODataSource *stdout_redir;
	ODataSource *stderr_redir;

	// Confont
	FixedWidthFont      *confont;

	void (*auto_paint)(void);

	MsgMask     msgMask;                // mask to determine which messages are printed or not

	// Overlay timing
	uint32      framenum;
	uint32      times[CON_NUM_TIMES];   // framenum the line was generated
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
		confont = cf;
	}

	// Get the console font texture
	FixedWidthFont *GetConFont() {
		return confont;
	}

	// Autopaint will cause the GUIApp to re-paint everything after a linefeed
	void    SetAutoPaint(void (*func)(void)) {
		auto_paint = func;
	}

	// Scroll through the console. - is up + is down
	void    ScrollConsole(int32 lines);

	// Redirection
	void    RedirectOutputStream(uint32 mask, ODataSource *ds) {
		if (mask & CON_STDOUT) stdout_redir = ds;
		if (mask & CON_STDERR) stderr_redir = ds;
	}

	// Enabling output
	void    setOutputEnabled(uint32 mask) {
		std_output_enabled |= mask;
	}

	// Disabling output
	void    unsetOutputEnabled(uint32 mask) {
		std_output_enabled &= ~mask;
	}

	// Enabling output
	uint32  getOutputEnabled() {
		return std_output_enabled;
	}

	// sets what message types to ignore
	void setMsgMask(const MsgMask mm) {
		msgMask = mm;
	}

	void setFrameNum(uint32 f) {
		framenum = f;
	}

	//
	// STDOUT Methods
	//

	// Print a text string to the console, and output to stdout
	void    Print(const char *txt);
	void    Print(const MsgMask mm, const char *txt);

	// printf, and output to stdout
	int32   Printf(const char *fmt, ...);
	int32   Printf(const MsgMask mm, const char *fmt, ...); // with msg filtering

	// printf, and output to stdout (va_list)
	int     vPrintf(const char *fmt, va_list);

	// putchar, and output to stdout
	void    Putchar(int c);

	// Print a text string to the console, and output to stdout
	void    PrintRaw(const char *txt, int n);


	//
	// STDERR Methods
	//

	// Print a text string to the console, and output to stderr
	void    Print_err(const char *txt);
	void    Print_err(const MsgMask mm, const char *txt);

	// printf, and output to stderr
	int32   Printf_err(const char *fmt, ...);
	int32   Printf_err(const MsgMask mm, const char *fmt, ...);

	// printf, and output to stderr (va_list)
	int     vPrintf_err(const char *fmt, va_list);

	// putchar, and output to stderr
	void    Putchar_err(int c);

	// Print a text string to the console, and output to stderr
	void    PrintRaw_err(const char *txt, int n);


	// Enable/Disable word wrapping
	void    EnableWordWrap() {
		wordwrap = true;
	}
	void    DisableWordWrap() {
		wordwrap = false;
	}

	//
	// Console Commands
	//

	typedef Pentagram::istring ArgsType;
	typedef std::vector<ArgsType> ArgvType;
	typedef void (*Function)(const ArgvType &argv);

	//! Add a command to the console
	//! \param command The command to add
	//! \param function Function pointer for command
	void            AddConsoleCommand(const ArgsType &command, Console::Function function);

	//! Remove all commands associated with a function from the console
	//! \param function Function pointer for command
	void            RemoveConsoleCommand(Console::Function function);

	//! Execute a specific console command
	//! \param command The command to execute with args
	void            ExecuteConsoleCommand(const Console::ArgsType &args);

	//! Execute a specific console command
	//! \param command The command to execute with argv
	void            ExecuteConsoleCommand(const Console::ArgvType &argv);

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

	//! "CmdList" console command
	static void     ConCmd_CmdList(const Console::ArgvType &argv);

	//! "CmdHistory" console command
	static void     ConCmd_CmdHistory(const Console::ArgvType &argv);

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
	ArgsType                    commandBuffer;
	int                         commandCursorPos;
	bool                        commandInsert;
	std::vector<ArgsType>       commandHistory;
	int                         commandHistoryPos;

	typedef Common::HashMap<Common::String, Function> CommandsMap;
	CommandsMap ConsoleCommands;
};

// Console object
extern  Console *con;

//
// Console Ouput Streams
//

/*
//
// Standard Output Streambuf
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_streambuf : public std::basic_streambuf<_E, _Tr>
{
public:
    console_streambuf() : std::basic_streambuf<_E, _Tr>() { }
    virtual ~console_streambuf() { }
    typedef typename _Tr::int_type int_type;
    typedef typename _Tr::char_type char_type;

protected:

    // Output a character
    virtual int_type overflow(int_type c = _Tr::eof())
    {
        if (!_Tr::eq_int_type(_Tr::eof(), c)) con->Putchar(_Tr::to_char_type(c));
        return (_Tr::not_eof(c));
    }

    // Flush
    virtual int sync()
    {
        return 0;
    }
};

//
// Standard Output Stream
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_ostream : public std::basic_ostream<_E, _Tr>
{
//#ifndef SAFE_CONSOLE_STREAMS
    console_streambuf<_E, _Tr> _Fb;
//#endif

public:
    console_ostream() : std::basic_ostream<_E, _Tr>(&_Fb), _Fb() {}
    console_ostream(console_streambuf<_E, _Tr> *Fb) : std::basic_ostream<_E, _Tr>(Fb) {}
    virtual ~console_ostream() { }

#if defined(MACOSX) && defined(__GNUC__)
    // Work around a bug in Apple GCC 3.x which incorrectly tries to inline this method
    int __attribute__ ((noinline)) printf (const char *fmt, ...)
#else
    int printf (const char *fmt, ...)
#endif
    {
        va_list argptr;
        va_start (argptr,fmt);
        int ret = con->vPrintf(fmt, argptr);
        va_end (argptr);
        return ret;
    }
};
*/

namespace std {
enum Precision { hex = 16, dec = 10 };
}

class ConsoleStream : public Common::WriteStream {
private:
	std::Precision _precision;
public:
	ConsoleStream() : Common::WriteStream(), _precision(std::dec) {}

	virtual int32 pos() const override {
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
		Common::String str = Common::String::format("%d", val);
		write(str.c_str(), str.size());
		return *this;
	}
};

template<class T>
class console_ostream : public ConsoleStream {
	virtual uint32 write(const void *dataPtr, uint32 dataSize) override {
		Common::String str((const char *)dataPtr, (const char *)dataPtr + dataSize);
		debugN("%s", str.c_str());
		return dataSize;
	}
};

//
// Standard Output Stream Object
//
#ifndef SAFE_CONSOLE_STREAMS
extern console_ostream<char>        pout;
extern console_ostream<char>        *ppout;
#else
#define pout (*ppout)
extern console_ostream<char>        *ppout;
#endif

template<class T>
class console_err_ostream : public ConsoleStream {
public:
	virtual uint32 write(const void *dataPtr, uint32 dataSize) override {
		Common::String str((const char *)dataPtr, dataSize);
		::warning("%s", str.c_str());
		return str.size();
	}
};

//
// Error Output Stream Object
//
#ifndef SAFE_CONSOLE_STREAMS
extern console_err_ostream<char>    perr;
extern console_err_ostream<char>    *pperr;
#else
#define perr (*pperr)
extern console_err_ostream<char>    *pperr;
#endif

} // End of namespace Ultima8

#endif
