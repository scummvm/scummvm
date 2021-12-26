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

//=============================================================================
//
// LogFile, the IOutputHandler implementation that writes to file.
//
// When created LogFile may open file right away or delay doing this.
// In the latter case it will buffer output up to certain size limit.
// When told to open the file, it will first flush its buffer. This allows to
// log events even before the log path is decided (for example, before or
// during reading configuration and/or parsing command line).
//
//=============================================================================

#ifndef AGS_ENGINE_DEBUGGING_LOG_FILE_H
#define AGS_ENGINE_DEBUGGING_LOG_FILE_H

#include "ags/lib/std/memory.h"
#include "ags/shared/debugging/output_handler.h"

namespace AGS3 {
namespace AGS {

namespace Shared {
class Stream;
}

namespace Engine {

using Shared::DebugMessage;
using Shared::Stream;
using Shared::String;

class LogFile : public AGS::Shared::IOutputHandler {
public:
	enum OpenMode {
		kLogFile_Overwrite,
		kLogFile_OverwriteAtFirstMessage,
		kLogFile_Append
	};

public:
	LogFile();

	void PrintMessage(const Shared::DebugMessage &msg) override;

	// Open file using given file path, optionally appending if one exists
	//
	// TODO: filepath parameter here may be actually used as a pattern
	// or prefix, while the actual filename could be made by combining
	// this prefix with current date, game name, and similar additional
	// useful information. Whether this is to be determined here or on
	// high-level side remains a question.
	//
	bool         OpenFile(const String &file_path, OpenMode open_mode = kLogFile_Overwrite);
	// Close file
	void         CloseFile();

private:
	std::unique_ptr<Stream> _file;
	String                _filePath;
	OpenMode              _openMode;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
