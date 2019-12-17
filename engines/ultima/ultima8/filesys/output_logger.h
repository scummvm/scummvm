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

#ifndef ULTIMA8_FILESYS_OUTPUTLOGGER_H
#define ULTIMA8_FILESYS_OUTPUTLOGGER_H

#include "ultima/shared/std/string.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

//! Class that will duplicate output sent to FILE into another file
class OutputLogger {
	int         fd;             //< Original fd of file being redirected
	Common::WriteStream *fileOld;       //< Duplicated stream that points to the original file

	std::string filenameLog;    //< Name of log file
	Common::WriteStream *fileLog;       //< Stream for log file

	int         fdPipeRead;     //< file descriptor to the read end of the pipe

//	SDL_Thread*  pThread;        //< Pointer to the thread that reads the pipe

	int ThreadMain();
//	static int SDLCALL sThreadMain(OutputLogger *instance) { return instance->ThreadMain(); }

public:
	OutputLogger(Common::WriteStream *file, const std::string &filename);   // Should be using unicode in this day and age
	~OutputLogger(void);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
