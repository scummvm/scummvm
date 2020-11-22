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

#include <string.h>
#include "debug/logfile.h"
#include "util/file.h"
#include "util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using namespace Shared;

LogFile::LogFile()
	: _openMode(kLogFile_Overwrite) {
}

void LogFile::PrintMessage(const DebugMessage &msg) {
	if (!_file.get()) {
		if (_filePath.IsEmpty())
			return;
		_file.reset(File::OpenFile(_filePath, _openMode == kLogFile_Append ? Common::kFile_Create : Common::kFile_CreateAlways,
			Common::kFile_Write));
		if (!_file) {
			Debug::Printf("Unable to write log to '%s'.", _filePath.GetCStr());
			_filePath = "";
			return;
		}
	}

	if (!msg.GroupName.IsEmpty()) {
		_file->Write(msg.GroupName, msg.GroupName.GetLength());
		_file->Write(" : ", 3);
	}
	_file->Write(msg.Text, msg.Text.GetLength());
	_file->WriteInt8('\n');
	// We should flush after every write to the log; this will make writing
	// bit slower, but will increase the chances that all latest output
	// will get to the disk in case of program crash.
	_file->Flush();
}

bool LogFile::OpenFile(const String &file_path, OpenMode open_mode) {
	CloseFile();

	_filePath = file_path;
	_openMode = open_mode;
	if (open_mode == OpenMode::kLogFile_OverwriteAtFirstMessage) {
		return File::TestWriteFile(_filePath);
	} else {
		_file.reset(File::OpenFile(file_path,
			open_mode == kLogFile_Append ? Common::kFile_Create : Common::kFile_CreateAlways,
			Common::kFile_Write));
		return _file.get() != nullptr;
	}
}

void LogFile::CloseFile() {
	_file.reset();
	_filePath.Empty();
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
