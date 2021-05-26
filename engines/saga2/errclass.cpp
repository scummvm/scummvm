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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/errclass.h"

namespace Saga2 {

void (*gError::dumpHandler)(char *);

char                *gError::errBuf = new char[ 128 ];
int16               gError::errBufSize = 128,
                            gError::errBufOffset = 0;
uint8               gError::saveErrFlag = 0;

/****** errclass.cpp/gError [class] *******************************
*
*   NAME
*       class gError -- an error-handling exception class
*
*   FUNCTION
*       gError is (or will be) the basis for error-handling
*       in the DGI libraries. It has the ability to store an
*       ASCII error message text which can be printed to the
*       display.
*
*       The display of the error messages can be handled
*       automatically by gError, or a custom error-reporting
*       routine can be specified for things like an error
*       window.
*
*   SEE ALSO
*       gError::gError
*       gError::warn
*       class MemoryError
*       class DOSError
*
*******************************************************************/

/****** errclass.cpp/gError::gError *******************************
*
*   NAME
*       gError::gError -- constructor for gError class
*
*   SYNOPSIS
*       throw gError( "message", arg );
*
*       gError::gError( char *msg, ... );
*
*   FUNCTION
*       The gError constructor is called whenever an exception is
*       thrown. It appends the message (after it is expanded) into
*       the internal error text buffer. If there is no custom error
*       handler currently installed, then the error message is
*       printed to stderr. If a custom error handler is installed,
*       then the error message is left in the buffer until the
*       "dump" member function is called.
*
*   INPUTS
*       message     a text string, which can have printf-like
*                   formatting. The additional args are used just
*                   like printf.
*                   NOTE: A linefeed '\n' should be included
*                   at the end of the error message.
*
*       args        printf-like variable arguments.
*
*   EXAMPLE
*       /c/throw gError( "Attempt to open window failed.\n" );
*
*   SEE ALSO
*       gError [class]
*       gError::warn
*
*******************************************************************/

gError::gError(char *msg, ...) {
	va_list argptr;

	va_start(argptr, msg);
	append(msg, argptr);
	va_end(argptr);
}

gError::gError(void) {}

/****** errclass.cpp/gError::dump *******************************
*
*   NAME
*       gError::dump -- prints out the error message
*
*   SYNOPSIS
*       gError::dump();
*
*       static gError::dump( void );
*
*   FUNCTION
*       The dump member function can be called by the application
*       to force printing of the error message. This is only needed
*       when a dump-handler has been installed.
*
*   INPUTS
*       none
*
*   SEE ALSO
*       gError [class]
*
*******************************************************************/

void gError::dump(void) {
	if (errBufOffset > 0) {
		if (dumpHandler) dumpHandler(errBuf);
		else fputs(errBuf, stderr);

		errBuf[ 0 ] = 0;
		errBufOffset = 0;
	}
}

/****** errclass.cpp/gError::warn *******************************
*
*   NAME
*       gError::warn -- print a non-fatal error
*
*   SYNOPSIS
*       gError::warn( "message", arg );
*
*       void gError::warn( char *msg, ... );
*
*   FUNCTION
*       the 'warn' member function appends an error message to
*       the internal gError message buffer, but does not throw
*       an exception. This can be useful for mixing non-fatal
*       error messages in the same message stream.
*
*   INPUTS
*       message     a text string, which can have printf-like
*                   formatting. The additional args are used just
*                   like printf.
*                   NOTE: A linefeed '\n' should be included
*                   at the end of the error message.
*
*       args        printf-like variable arguments.
*
*   SEE ALSO
*       gError [class]
*       gError::gError [class]
*
*******************************************************************/

void gError::warn(char *msg, ...) {
	va_list argptr;

	va_start(argptr, msg);
	append(msg, argptr);
	va_end(argptr);
}

void gError::appendf(char *msg, ...) {
	va_list argptr;

	va_start(argptr, msg);
	append(msg, argptr);
	va_end(argptr);
}

void gError::append(char *msg, va_list args) {
	uint8       tempBuf[ 256 ];
	int16       size = vsprintf((char *) tempBuf, msg, args);

	size = MIN<int16>(size, errBufSize - errBufOffset);
	memcpy(errBuf + errBufOffset, tempBuf, size + 1);
	errBufOffset += size;
	if (!saveErrFlag) dump();
}

/****** errclass.cpp/gError::setErrBufSize *******************************
*
*   NAME
*       gError::setErrBufSize -- set the size of the internal error buffer
*
*   SYNOPSIS
*       gError::setErrBufSize( size );
*
*       void gError::setErrBufSize( int32 );
*
*   FUNCTION
*       the default internal error buffer is 128 bytes. This is
*       fine for most applications. However, applications which
*       make substantial use of 'warn' to create a log of
*       errors will quickly run out of room. This function can be
*       used to allocate a larger buffer.
*
*       Note that messages which overflow the error buffer are
*       truncated.
*
*   INPUTS
*       size        the desired size of the error text buffer.
*
*   SEE ALSO
*       gError [class]
*       gError::gError [class]
*
*******************************************************************/

void gError::setErrBufSize(int32 size) {
	if (errBuf) free(errBuf);
	errBuf = new char[ size ];
	errBufOffset = 0;
	errBufSize = (int16) size;
}

/****** errclass.cpp/gError::setDumpHandler *******************************
*
*   NAME
*       gError::setDumpHandler -- set a custom error reporting function
*
*   SYNOPSIS
*       gError::setDumpHandler( handlerFunc );
*
*       void gError::setDumpHandler( void (*dumpHand)( char * ) );
*
*   FUNCTION
*       This function can be used to set a custom "dump" handler
*       for gError exceptions. The dump handler is called whenever
*       an error occurs, unless the "saveErrors" flag has been set,
*       in which case the dump handler is called whenever
*       gError::dump() is called by the application.
*
*       The "dump" function is called with a single argument, which
*       is a character pointer to the error buffer( which is NULL
*       terminated).
*
*   INPUTS
*       handlerFunc     the custom error reporting function.
*
*   SEE ALSO
*       gError [class]
*       gError::gError [class]
*
*******************************************************************/

/****** errclass.cpp/gError::saveErrs *******************************
*
*   NAME
*       gError::saveErrs -- tell gError to accumulate error messages.
*
*   SYNOPSIS
*       gError::saveErrs( save );
*
*       void gError::saveErrs( bool );
*
*   FUNCTION
*       This member function affects the "saveErrorFlag" which
*       determines if errors are dumped immediately when they
*       occur, or if they are accumulated and dumped when the
*       application calls gError::dump();
*
*   INPUTS
*       save        TRUE - do not dump errors automatically,
*                   accumulate them in buffer.
*                   FALSE - dump errors immediately and clear buffer.
*
*       (the gError class defaults to saveErrors=FALSE)
*
*   SEE ALSO
*       gError [class]
*       gError::gError [class]
*
*******************************************************************/

/* ======================================================================= *
   class MemoryError
 * ======================================================================= */

/****** errclass.cpp/MemoryError [class] *******************************
*
*   NAME
*       class MemoryError -- exception class for memory errors
*
*   FUNCTION
*       MemoryError is a class designed to be used for
*       out-of-memory errors. It is derived from class gError,
*       and inherits most of it's behavior from that class.
*
*       It has two forms:
*
*           /c/
*           /c/throw MemoryError( size );
*           /c/throw MemoryError();
*           /c/
*
*           -- where 'size' is the amount of memory that the
*           application was attempting to allocate.
*
*   EXAMPLE
*
*       the code:
*
*           /c/
*           /c/throw MemoryError( 1000 );
*           /c/
*
*       produces the message:
*
*           /c/
*           /c/"Attempt to allocate object size 1000 failed."
*           /c/
*
*   SEE ALSO
*       gError::gError
*       gError::warn
*       class gError
*       class DOSError
*
*******************************************************************/

MemoryError::MemoryError(int32 size)
	: gError("Attempt to allocate object size %d failed.\n", size)
{}

MemoryError::MemoryError(void)
	: gError("Memory allocation failed.\n")
{}

} // end of namespace Saga2
