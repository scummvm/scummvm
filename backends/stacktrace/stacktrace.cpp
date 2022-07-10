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


// Pick your poison.
//
// On GNU/Linux, you have few choices to get the most out of your stack trace.
//
// By default you get:
//	- object filename
//	- function name
//
// In order to add:
//	- source filename
//	- line and column numbers
//	- source code snippet (assuming the file is accessible)

// Install one of the following libraries then uncomment one of the macro (or
// better, add the detection of the lib and the macro definition in your build
// system)

// - apt-get install libdw-dev ...
// - g++/clang++ -ldw ...
// #define BACKWARD_HAS_DW 1

// - apt-get install binutils-dev ...
// - g++/clang++ -lbfd ...
// #define BACKWARD_HAS_BFD 1

// - apt-get install libdwarf-dev ...
// - g++/clang++ -ldwarf ...
// #define BACKWARD_HAS_DWARF 1

// Regardless of the library you choose to read the debug information,
// for potentially more detailed stack traces you can use libunwind
// - apt-get install libunwind-dev
// - g++/clang++ -lunwind
// #define BACKWARD_HAS_LIBUNWIND 1

#include "backends/stacktrace/stacktrace.h"

#if defined(USE_STACKTRACE_DW)
#define BACKWARD_HAS_DW 1
#elif defined(USE_STACKTRACE_DWARF)
#define BACKWARD_HAS_DWARF 1
#elif defined(USE_STACKTRACE_BFD)
#define BACKWARD_HAS_BFD 1
#elif defined(USE_STACKTRACE_UNWIND)
#define BACKWARD_HAS_LIBUNWIND 1
#endif

#include "backends/stacktrace/backward.h"

// backward::SignalHandling sh;

#ifdef USE_STACKTRACE

class logger_streambuf : public std::streambuf {
public:
	logger_streambuf(void (*_logger)(const char *)) : logger(_logger) {}

	int_type underflow() override { return traits_type::eof(); }

	int_type overflow(int_type ch) override {
		char buff[2];
		buff[0] = ch;
		buff[1] = 0;
		logger(buff);
		return ch;
	}

	std::streamsize xsputn(const char_type *s, std::streamsize count) override {
		const int maxLen = 1024;
		char buff[maxLen];

		int len = std::min((int)count, maxLen-1);
		memcpy(buff, s, len);
		buff[len] = 0;

		logger(buff);
		return count;
	}

public:
	logger_streambuf(const logger_streambuf &) = delete;
	logger_streambuf &operator=(const logger_streambuf &) = delete;

private:
	void (*logger)(const char *);
};

void createStacktrace(void (*logger)(const char *)) {
	backward::StackTrace st;
	st.load_here(32);
	backward::Printer p;
	p.object = true;
	p.color_mode = backward::ColorMode::always;
	p.address = true;

	logger_streambuf stream_buff(logger);
	std::ostream os(&stream_buff);
	p.print(st, os);
}
#endif
