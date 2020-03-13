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

#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/filesystem.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/utils.h"
#include "ultima/shared/std/containers.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

#ifdef TODO
#if HAVE_BACKTRACE

/**
 * Get a backtrace and print it to the file.  Note that gcc requires
 * the -rdynamic flag to have access to the actual backtrace symbols;
 * otherwise they will be simple hex offsets.
 */
void print_trace(FILE *file) {
    /* Code Taken from GNU C Library manual */
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);

    fprintf(file, "Stack trace:\n");

    /* start at one to omit print_trace */
    for (i = 1; i < size; i++) {
        fprintf(file, "%s\n", strings[i]);
    }
    free(strings);
}

#else

/**
 * Stub for systems without access to the stack backtrace.
 */
void print_trace(Common::WriteStream *file) {
	file->writeString("Stack trace not available\n");
}

#endif
#endif

#if !HAVE_VARIADIC_MACROS

/**
 * Stub for systems without variadic macros.  Unfortunately, this
 * assert won't be very useful.
 */
void ASSERT(bool exp, const char *desc, ...) {
#ifndef NDEBUG
    if (!exp) {
		va_list args;
		va_start(args, desc);
		Common::String msg = Common::String::vformat(desc, args);
		va_end(args);

		error("Assertion failed: %s", msg.c_str());
    }
#endif
}

#endif

//Common::File *Debug::global = NULL;

/**
 * A debug class that uses the TRACE() and TRACE_LOCAL() macros.
 * It writes debug info to the filename provided, creating
 * any directory structure it needs to ensure the file will
 * be created successfully.
 *
 * @param fn        The file path used to write debug info
 * @param nm        The name of this debug object, used to
 *                  identify it in the global debug file.
 * @param append    If true, appends to the debug file
 *                  instead of overwriting it.
 */
Debug::Debug(const Common::String &fn, const Common::String &nm, bool append) : _disabled(false), _filename(fn), _name(nm) {
    if (!loggingEnabled(_name)) {
        _disabled = true;
        return;
    }

    if (!_file) {} // FIXME: throw exception here
    else if (!_name.empty())
        debug(1, "=== %s ===\n", _name.c_str());
}

/**
 * Initializes a global debug file, if desired.
 * This file will contain the results of any TRACE()
 * macro used, whereas TRACE_LOCAL() only captures
 * the debug info in its own debug file.
 */
void Debug::initGlobal(const Common::String &filename) {    
    if (settings._logging.empty())
        return;
}

/**
 * Traces information into the debug file.
 * This function is used by the TRACE() and TRACE_LOCAL()
 * macros to provide trace functionality.
 */
void Debug::trace(const Common::String &msg, const Common::String &fn, const Common::String &func, const int line, bool glbl) {
    if (_disabled)
        return;

    bool brackets = false;
    Common::String message, filename;

    Path path(fn);
    filename = path.getFilename();    
    
    if (!_file)
        return;
    
    if (!msg.empty())
        message += msg;        
    
    if (!filename.empty() || line > 0) {
        brackets = true;
        message += " [";        
    }

    if ((l_filename == filename) && (l_func == func) && (l_line == line))
        message += "...";
    else {
        if (!func.empty()) {
            l_func = func;
            message += func + "() - ";
		} else {
			l_func.clear();
		}

        if (!filename.empty()) {
            l_filename = filename;
            message += filename + ": ";
		} else {
			l_filename.clear();
		}

        if (line > 0) {
            l_line = line;
            char ln[8];
            sprintf(ln, "%d", line);
            message += "line ";
            message += ln;        
        }
        else l_line = -1;
    }

    if (brackets)
        message += "]";
    message += "\n";
#ifdef TODO
    fprintf(file, "%s", message.c_str());
    if (global && glbl)
        fprintf(global, "%12s: %s", name.c_str(), message.c_str());
#endif
}

/**
 * Determines whether or not this debug element is enabled in our game settings.
 */
bool Debug::loggingEnabled(const Common::String &name) {
    if (settings._logging == "all")
        return true;

    Std::vector<Common::String> enabledLogs = split(settings._logging, ", ");
    if (Common::find(enabledLogs.begin(), enabledLogs.end(), name) != enabledLogs.end())
        return true;

    return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
