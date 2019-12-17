/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "ultima/ultima8/misc/console.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/graphics/fonts/fixed_width_font.h"
#include "ultima/shared/std/misc.h"
#include "common/file.h"
#include "common/debug.h"

using namespace std;

namespace Ultima {
namespace Ultima8 {

#define MAXPRINTMSG 4096

// The console
Console *con;

// Standard Output Stream Object
#ifndef SAFE_CONSOLE_STREAMS
console_ostream<char>       pout;
console_ostream<char>       *ppout = &pout;
#else
console_ostream<char>       *ppout = 0;
#endif

// Error Output Stream Object
#ifndef SAFE_CONSOLE_STREAMS
console_err_ostream<char>   perr;
console_err_ostream<char>   *pperr = &perr;
#else
console_err_ostream<char>   *pperr = 0;
#endif



//
// Constructor
//
Console::Console() : current(0), xoff(0), display(0), linewidth(-1),
		totallines(0), vislines(0), wordwrap(true), cr(false),
		putchar_count(0), std_output_enabled(0xFFFFFFFF),
		stdout_redir(0), stderr_redir(0), confont(0),
		auto_paint(0), msgMask(MM_ALL), framenum(0),
		commandCursorPos(0), commandInsert(true), commandHistoryPos(0) {
	con = this;
	linewidth = -1;

	CheckResize(0);

	std::memset(times, 0, sizeof(times));

	// Lets try adding a Console command!
	AddConsoleCommand("Console::CmdList", ConCmd_CmdList);
	AddConsoleCommand("Console::CmdHistory", ConCmd_CmdHistory);

	PrintInternal("Console initialized.\n");
}

//
// Destructor
//
Console::~Console() {
	RemoveConsoleCommand(Console::ConCmd_CmdList);
	RemoveConsoleCommand(Console::ConCmd_CmdHistory);

	// Need to do this first
	PrintPutchar();
}

/*
================
Con_Clear_f
================
*/
void Console::Clear() {
	std::memset(text, ' ', CON_TEXTSIZE);
	putchar_count = 0;
}


/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Console::Dump(const char *name) {
	int     l, x;
	char    *line;
	Common::DumpFile f;
	char    buffer[1024];

	// Need to do this first
	PrintPutchar();

	Printf("Dumped console text to %s.\n", name);

	if (!f.open(name)) {
		Print("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = current - totallines + 1 ; l <= current ; l++) {
		line = text + (l % totallines) * linewidth;
		for (x = 0 ; x < linewidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != linewidth)
			break;
	}

	// write the remaining lines
	buffer[linewidth] = 0;
	for (; l <= current ; l++) {
		line = text + (l % totallines) * linewidth;
		std::strncpy(buffer, line, linewidth);
		for (x = linewidth - 1 ; x >= 0 ; x--) {
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x = 0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		f.writeString(buffer);
		f.writeByte('\n');
	}

	f.close();
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Console::CheckResize(int scrwidth) {
	int     i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char    tbuf[CON_TEXTSIZE];

	// Need to do this first
	PrintPutchar();

	if (!confont)
		width = (scrwidth >> 3) - 2;
	else
		width = (scrwidth / confont->width) - 2;

	if (width == linewidth)
		return;

	if (width < 1) {        // video hasn't been initialized yet
		width = 78;
		linewidth = width;
		totallines = CON_TEXTSIZE / linewidth;
		std::memset(text, ' ', CON_TEXTSIZE);
	} else {
		oldwidth = linewidth;
		linewidth = width;
		oldtotallines = totallines;
		totallines = CON_TEXTSIZE / linewidth;
		numlines = oldtotallines;

		if (totallines < numlines)
			numlines = totallines;

		numchars = oldwidth;

		if (linewidth < numchars)
			numchars = linewidth;

		std::memcpy(tbuf, text, CON_TEXTSIZE);
		std::memset(text, ' ', CON_TEXTSIZE);

		for (i = 0 ; i < numlines ; i++) {
			for (j = 0 ; j < numchars ; j++) {
				text[(totallines - 1 - i) * linewidth + j] =
				    tbuf[((current - i + oldtotallines) %
				          oldtotallines) * oldwidth + j];
			}
		}
	}

	current = totallines - 1;
	display = current;
}


//
// Internal Methods
//

// Print a text string to the console
void Console::PrintInternal(const char *txt) {
	int     y;
	int     c, l;

	// Need to do this first
	PrintPutchar();

	while (0 != (c = *txt)) {
		if (wordwrap) {
			// count word length
			for (l = 0 ; l < linewidth ; l++)
				if (txt[l] <= ' ')
					break;

			// word wrap
			if (l != linewidth && (xoff + l > linewidth))
				xoff = 0;
		}

		txt++;

		if (cr) {
			current--;
			cr = false;
		}

		if (!xoff) {
			Linefeed();
			// mark time for transparent overlay
			if (current >= 0) times[current % CON_NUM_TIMES] = framenum;
		}

		switch (c) {
		case '\n':
			xoff = 0;
			break;

		case '\r':
			xoff = 0;
			cr = true;
			break;

		default:    // display character and advance
			y = current % totallines;
			text[y * linewidth + xoff] = static_cast<char>(c);
			xoff++;
			if (xoff >= linewidth) xoff = 0;
			break;
		}

	}
}

// Print a text string to the console
void Console::PrintRawInternal(const char *txt, int n) {
	int     y;
	int     c, l;

	// Need to do this first
	PrintPutchar();

	for (int i = 0; i < n; i++) {
		c = *txt;

		if (wordwrap) {
			// count word length
			for (l = 0 ; l < linewidth && l < n; l++)
				if (txt[l] <= ' ') break;

			// word wrap
			if (l != linewidth && (xoff + l > linewidth))
				xoff = 0;
		}

		txt++;

		if (cr) {
			current--;
			cr = false;
		}

		if (!xoff) {
			Linefeed();
			// mark time for transparent overlay
			if (current >= 0) times[current % CON_NUM_TIMES] = framenum;
		}

		switch (c) {
		case '\n':
			xoff = 0;
			break;

		case '\r':
			xoff = 0;
			cr = true;
			break;

		default:    // display character and advance
			y = current % totallines;
			text[y * linewidth + xoff] = static_cast<char>(c);
			xoff++;
			if (xoff >= linewidth) xoff = 0;
			break;
		}

	}
}

// Add a linefeed to the buffer
void Console::Linefeed(void) {
	xoff = 0;
	display++;
	current++;
	std::memset(&text[(current % totallines)*linewidth], ' ', linewidth);

	if (auto_paint) auto_paint();
}

// Print a text string to the console
void Console::PutcharInternal(int c) {
	// Add the character
	putchar_buf[putchar_count] = static_cast<char>(c);

	// Increment the counter
	putchar_count++;

	// If it was a space or less, or we've hit the limit we'll add it to the
	// actual buffer
	if (c <= ' ' || putchar_count == (CON_PUTCHAR_SIZE - 1)) PrintPutchar();
}

// Print the Putchar data, if possible
void Console::PrintPutchar() {
	if (!putchar_count) return;

	// Get the count
	//int count = putchar_count; //Darke: UNUSED?

	// Terminate the string
	putchar_buf[putchar_count] = 0;

	// Clear the counter
	putchar_count = 0;

	// Print it
	PrintInternal(putchar_buf);
}

//
// STDOUT Methods
//

// Print a text string to the console, and output to stdout
void Console::Print(const char *txt) {
	if (std_output_enabled & CON_STDOUT)
		debug("%s", txt);

	if (stdout_redir)
		stdout_redir->write(txt, std::strlen(txt));
	PrintInternal(txt);
}

// Print a text string to the console, and output to stdout, with message filtering
void Console::Print(const MsgMask mm, const char *txt) {
	if (mm & msgMask) Print(txt);
}

// printf, and output to stdout
int32 Console::Printf(const char *fmt, ...) {
	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stdout, with message filtering.
int32 Console::Printf(const MsgMask mm, const char *fmt, ...) {
	if (!(mm & msgMask)) return 0;

	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stdout (va_list)
int32 Console::vPrintf(const char *fmt, va_list argptr) {
	Common::String str = Common::String::vformat(fmt, argptr);

	if (std_output_enabled & CON_STDOUT) {
		debug("%s", str.c_str());
	}

	if (stdout_redir)
		stdout_redir->write(str.c_str(), str.size());
	PrintInternal(str.c_str());

	return str.size();
}

// Print a text string to the console, and output to stdout
void Console::PrintRaw(const char *txt, int n) {
	if (std_output_enabled & CON_STDOUT) {
		Common::String str(txt, txt + n);
		debug("%s", str.c_str());
	}

	if (stdout_redir) stdout_redir->write(txt, n);
	PrintRawInternal(txt, n);
}

// putchar, and output to stdout
void Console::Putchar(int c) {
	if (std_output_enabled & CON_STDOUT)
		debug("%c", c);

	if (stdout_redir) stdout_redir->write1(c);
	PutcharInternal(c);
}


//
// STDERR Methods
//

// Print a text string to the console, and output to stderr
void Console::Print_err(const char *txt) {
	if (std_output_enabled & CON_STDERR)
		debug("%s", txt);

	if (stderr_redir) stderr_redir->write(txt, std::strlen(txt));
	PrintInternal(txt);
}

// Print a text string to the console, and output to stderr, with message filtering
void Console::Print_err(const MsgMask mm, const char *txt) {
	if (mm & msgMask) Print_err(txt);
}

// printf, and output to stderr
int32 Console::Printf_err(const char *fmt, ...) {
	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf_err(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stderr, with message filtering
int32 Console::Printf_err(const MsgMask mm, const char *fmt, ...) {
	if (!(mm & msgMask)) return 0;

	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf_err(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stderr (va_list)
int32 Console::vPrintf_err(const char *fmt, va_list argptr) {
	Common::String str = Common::String::format(fmt, argptr);

	if (std_output_enabled & CON_STDERR)
		debug("%s", str.c_str());

	if (stderr_redir)
		stderr_redir->write(str.c_str(), str.size());
	PrintInternal(str.c_str());

	return str.size();
}

// Print a text string to the console, and output to stderr
void Console::PrintRaw_err(const char *txt, int n) {
	if (std_output_enabled & CON_STDERR)
		debug("%d", n);

	if (stderr_redir) stderr_redir->write(txt, n);
	PrintRawInternal(txt, n);
}

// putchar, and output to stderr
void Console::Putchar_err(int c) {
	if (std_output_enabled & CON_STDERR)
		debug("%c", c);

	if (stderr_redir) stderr_redir->write1(c);
	PutcharInternal(c);
}

void Console::ScrollConsole(int32 lines) {
	display += lines;

	if (display < 0) display = 0;
	if (display > current) display = current;
}

//
// Console commands
//

void Console::AddConsoleCommand(const ArgsType &command, Console::Function function) {
	ConsoleCommands[command] = function;
}

void Console::RemoveConsoleCommand(Console::Function function) {
	for (CommandsMap::iterator it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it) {
		if (it->_value == function) {
			//pout << "Removing command: " << it->_key << std::endl;
			it->_value = 0;
		}
	}
}
void Console::ExecuteConsoleCommand(const Console::ArgsType &args) {
	Console::ArgvType argv;
	Pentagram::StringToArgv(args, argv);

	ExecuteConsoleCommand(argv);
}

void Console::ExecuteConsoleCommand(const Console::ArgvType &argv) {
	CommandsMap::iterator it;

	// Empty?!?
	if (argv.empty())
		return;

	// Get the command name. Transparently handle conversions from original GUIApp
	Common::String commandName = argv[0];
	if (commandName.hasPrefix("GUIApp::"))
		commandName = "Ultima8Engine::" + Common::String(commandName.c_str() + 8);

	// Handle the command
	it = ConsoleCommands.find(commandName);

	if (it != ConsoleCommands.end() && it->_value)
		it->_value(argv);
	else
		Print(Common::String::format("Unknown command: %s\n", argv[0].c_str()).c_str());
}

void Console::ExecuteCommandBuffer() {
	if (commandBuffer.empty()) return;

	Console::ArgsType args = commandBuffer;
	commandBuffer.clear();

	// TODO: Fix this
	//pout << "]" << args << std::endl;

	ExecuteConsoleCommand(args);

	commandHistory.push_back(args);
	commandHistoryPos = 0;
	commandCursorPos = 0;
}

void Console::ScrollCommandHistory(int num) {
	int total = commandHistory.size();

	// No history, don't do anything
	if (!total) return;

	if ((commandHistoryPos - num) <= 0) {
		if (commandHistoryPos == 1) return;
		commandHistoryPos = 1;
	} else
		commandHistoryPos -= num;

	if (commandHistoryPos > total)
		commandHistoryPos = total;

	commandBuffer = commandHistory[total - commandHistoryPos];
	commandCursorPos = commandBuffer.size();
}

void Console::ClearCommandBuffer() {
	commandBuffer.clear();
	commandCursorPos = 0;
}

void Console::AddCharacterToCommandBuffer(int ch) {
	// Enter (execute command)
	if (ch == Console::Enter) {

		ExecuteCommandBuffer();
	}
	// Backspace
	else if (ch == Console::Backspace) {

		DeleteCommandBufferChars(-1);
	}
	// Tab (command completion)
	else if (ch == Console::Tab) {

		if (!commandBuffer.empty()) {

			int count = 0;
			Console::ArgsType common;
			CommandsMap::iterator it;
			CommandsMap::iterator found;

			for (it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it)
				if (it->_value) {
					if (it->_key.compareToIgnoreCase(commandBuffer))
						continue;

					if (!count) {
						common = it->_key;
						found = it;
					} else {
#ifdef TODO
						Console::ArgsType::iterator it1 = common.begin();
						Console::ArgsType::const_iterator it2 = it->_key.begin();
						int comsize = 0;

						while (it1 != common.end()) {
							if (!Console::ArgsType::traits_type::eq(*it1, *it2)) break;

							comsize++;
							++it1;
							++it2;
						}

						common.resize(comsize);
#endif
					}
					count++;
				}

			if (count) {
#ifdef TODO

				if (count > 1) {
					pout << "]" << commandBuffer << std::endl;

					ArgsType args = "CmdList \"";
					args += commandBuffer;
					args += '\"';

					ArgvType argv;
					Pentagram::StringToArgv(args, argv);

					ConCmd_CmdList(argv);
					commandBuffer = common;
				} else
					commandBuffer = common;

				commandCursorPos = commandBuffer.size();
#endif
			}
		}
	}
	// Add the character to the command buffer
	else {
#ifdef TODO
		if (commandCursorPos == static_cast<int>(commandBuffer.size())) {
			commandBuffer += ch;
		} else if (commandInsert) {
			commandBuffer.insert(commandCursorPos, 1, ch);
		} else {
			commandBuffer[commandCursorPos] = ch;
		}

		commandCursorPos++;
#endif
	}
}

void Console::DeleteCommandBufferChars(int num) {
	if (!num || commandBuffer.empty()) return;

	if (num < 0) {
		num = -num;
		if (num > commandCursorPos) num = commandCursorPos;
		commandCursorPos -= num;
	} else {
		if ((num + commandCursorPos) > static_cast<int>(commandBuffer.size()))
			num = commandBuffer.size() - commandCursorPos;
	}

	commandBuffer.erase(commandCursorPos, num);
}

void Console::MoveCommandCursor(int num) {
	commandCursorPos += num;

	if (commandCursorPos < 0) commandCursorPos = 0;
	if (commandCursorPos > static_cast<int>(commandBuffer.size())) commandCursorPos = static_cast<int>(commandBuffer.size());
}

void Console::ConCmd_CmdList(const Console::ArgvType &argv) {
	CommandsMap::iterator it;
	int i = 0;

	//pout << std::endl;

	if (argv.size() > 1) {
		for (size_t a = 1; a < argv.size(); a++) {
			const ArgsType &arg = argv[a];

			for (it = con->ConsoleCommands.begin(); it != con->ConsoleCommands.end(); ++it)
				if (it->_value) {
					if (it->_key.compareToIgnoreCase(arg)) continue;

					// TODO: Fix this
					//pout << " " << it->_key << std::endl;
					i ++;
				}
		}
	} else {
		for (it = con->ConsoleCommands.begin(); it != con->ConsoleCommands.end(); ++it)
			if (it->_value) {
				// TODO
				//pout << " " << it->_key << std::endl;
				i ++;
			}
	}

	// TODO
	//pout << i << " commands" << std::endl;
}

void Console::ConCmd_CmdHistory(const Console::ArgvType & /*argv*/) {
#ifdef TODO
	std::vector<ArgsType>::iterator it;

	for (it = con->commandHistory.begin(); it != con->commandHistory.end(); ++it)
		pout << " " << *it << std::endl;

	pout << con->commandHistory.size() << " commands" << std::endl;
#endif
}

/*
==============================================================================

DRAWING

==============================================================================
*/

void Console::DrawConsole(RenderSurface *surf, int height) {
	int             i, x, y;
	int             rows;
	int             row;
	int             lines;
/// char            version[64];

	// Need to do this first
	PrintPutchar();

	lines = height;
	if (lines <= 0)
		return;

	//if (lines > viddef.height)
	//  lines = viddef.height;

	//Com_sprintf (version, sizeof(version), "v%4.2f", VERSION);
	//for (x=0 ; x<5 ; x++)
	//  re.DrawChar (viddef.width-44+x*8, lines-12, 128 + version[x] );

	// draw the text
	vislines = lines;

#if 0
	rows = (lines - 8) >> 3;    // rows of text to draw

	y = lines - 24;
#else
	rows = (lines / confont->height) - 2;   // rows of text to draw

	y = lines - (confont->height * 3);
#endif

// draw from the bottom up
	if (display != current) {
		// draw arrows to show the buffer is backscrolled
		for (x = 0 ; x < linewidth ; x += 4)
			surf->PrintCharFixed(confont, '^', (x + 1) * confont->width, y);

		y -= confont->height;
		rows--;
	}

	row = display;
	for (i = 0 ; i < rows ; i++, y -= confont->height, row--) {
		if (row < 0)
			break;
		if (current - row >= totallines)
			break;      // past scrollback wrap point

		char *txt = text + (row % totallines) * linewidth;

		for (x = 0 ; x < linewidth ; x++) {
			surf->PrintCharFixed(confont, txt[x], (x + 1)*confont->width, y);
			//  putchar (txt[x]);
		}
		//putchar ('\n');
	}

	const char *com = commandBuffer.c_str();
	int com_size = commandBuffer.size();
	int cur_pos = commandCursorPos;

	if (com_size >= (linewidth - 1)) {
		com_size = cur_pos;
	}

	//  prestep if horizontally scrolling
	if (com_size >= (linewidth - 1)) {
		com += 1 + com_size - (linewidth - 1);
		cur_pos = linewidth - 2;
	}

	y = lines - (confont->height * 2);

	surf->PrintCharFixed(confont, ']', confont->width, y);

	for (x = 0 ; x < (linewidth - 2) && com[x]; x++) {
		surf->PrintCharFixed(confont, com[x], (x + 2)*confont->width, y);
		//  putchar (txt[x]);
	}

	// Now for cursor position
	if (commandInsert)
		surf->Fill32(0xFFFFFFFF, ((cur_pos + 2)*confont->width) + 1, y, 2, confont->height);
	else
		surf->Fill32(0xFFFFFFFF, ((cur_pos + 2)*confont->width) + 1, y + confont->height - 2, confont->width, 2);
}


void Console::DrawConsoleNotify(RenderSurface *surf) {
	int     x, v;
	char    *txt;
	int     i;
	int     time;

	v = 0;
	for (i = current - CON_NUM_TIMES + 1 ; i <= current ; i++) {
		if (i < 0) continue;
		time = con->times[i % CON_NUM_TIMES];
		if (time == 0) continue;

		time = framenum - time;
		//if (time > con_notifytime->value*1000)
		if (time > 150) // Each message lasts 5 seconds  (30*5=150 frames)
			continue;
		txt = text + (i % totallines) * linewidth;

		for (x = 0 ; x < con->linewidth ; x++)
			surf->PrintCharFixed(confont, txt[x], (x + 1)*confont->width, v);

		v += confont->height;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
