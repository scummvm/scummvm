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

#ifndef ULTIMA4_DEBUG_H
#define ULTIMA4_DEBUG_H

#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Define XU4_FUNCTION as the function name.  Most compilers define
 * __FUNCTION__.  GCC provides __FUNCTION__ as a variable, not as a
 * macro, so detecting with #if __FUNCTION__ doesn't work.
 */
#   define XU4_FUNCTION ""

#undef TRACE
#define TRACE(dbg, msg) (dbg).trace(msg, __FILE__, XU4_FUNCTION, __LINE__)
#define TRACE_LOCAL(dbg, msg) (dbg).trace(msg, __FILE__, XU4_FUNCTION, __LINE__, false);

/*
 * Derived from XINE_ASSERT in the xine project.  I've updated it to
 * be C99 compliant, to use stderr rather than stdout, and to compile
 * out when NDEBUG is set, like a regular assert.  Finally, an
 * alternate ASSERT stub is provided for pre C99 systems.
 */

void print_trace(Common::WriteStream *file);

#if HAVE_VARIADIC_MACROS
#   ifdef NDEBUG
#       define ASSERT(exp, desc, ...)  /* nothing */
#   else
#       define ASSERT(exp, ...)                                             \
	do {                                                            \
		if (!(exp)) {                                               \
			fprintf(stderr, "%s:%s:%d: assertion `%s' failed. ",    \
			        __FILE__, XU4_FUNCTION, __LINE__, #exp);         \
			fprintf(stderr, __VA_ARGS__);                           \
			fprintf(stderr, "\n\n");                                \
			print_trace(stderr);                                    \
			abort();                                                \
		}                                                           \
	} while(0)
#   endif /* ifdef NDEBUG */
#else

void ASSERT(bool exp, const char *desc, ...);

#endif /* if HAVE_VARIADIC_MACROS */

/**
 * A debug class that uses the TRACE() and TRACE_LOCAL() macros.
 * It writes debug info to the filename provided, creating
 * any directory structure it needs to ensure the file will
 * be created successfully.
 */
class Debug {
public:
	Debug(const Common::String &filename, const Common::String &name = "", bool append = false);

	static void initGlobal(const Common::String &filename);
	void trace(const Common::String &msg, const Common::String &file = "", const Common::String &func = "", const int line = -1, bool glbl = true);

private:
	// disallow assignments, copy contruction
	Debug(const Debug &);
	const Debug &operator=(const Debug &);

	static bool loggingEnabled(const Common::String &name);

	bool _disabled;
	Common::String _filename, _name;
	Common::File *_file;

	Common::String l_filename, l_func;
	int l_line;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
