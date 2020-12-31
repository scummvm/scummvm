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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/filestream.h"
#include "ags/shared/util/bufferedstream.h"
#include "ags/shared/util/file.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

soff_t File::GetFileSize(const String &filename) {
	return ags_file_size(filename.GetCStr());
}

bool File::TestReadFile(const String &filename) {
	return Common::File::exists(filename.GetNullableCStr());
}

bool File::TestWriteFile(const String &filename) {
	return TestCreateFile(filename);
}

bool File::TestCreateFile(const String &filename) {
	Common::DumpFile df;

	bool result = df.open(filename.GetNullableCStr());
	df.close();

	return result;
}

bool File::DeleteFile(const String &filename) {
	return g_system->getSavefileManager()->removeSavefile(filename.GetNullableCStr());
}

bool File::GetFileModesFromCMode(const String &cmode, FileOpenMode &open_mode, FileWorkMode &work_mode) {
	// We do not test for 'b' and 't' here, because text mode reading/writing should be done with
	// the use of ITextReader and ITextWriter implementations.
	// The number of supported variants here is quite limited due the restrictions AGS makes on them.
	bool read_base_mode = false;
	// Default mode is open/read for safety reasons
	open_mode = kFile_Open;
	work_mode = kFile_Read;
	for (size_t c = 0; c < cmode.GetLength(); ++c) {
		if (read_base_mode) {
			if (cmode[c] == '+') {
				work_mode = kFile_ReadWrite;
			}
			break;
		} else {
			if (cmode[c] == 'r') {
				open_mode = kFile_Open;
				work_mode = kFile_Read;
				read_base_mode = true;
			} else if (cmode[c] == 'a') {
				open_mode = kFile_Create;
				work_mode = kFile_Write;
				read_base_mode = true;
			} else if (cmode[c] == 'w') {
				open_mode = kFile_CreateAlways;
				work_mode = kFile_Write;
				read_base_mode = true;
			}
		}
	}
	return read_base_mode;
}

String File::GetCMode(FileOpenMode open_mode, FileWorkMode work_mode) {
	String mode;
	if (open_mode == kFile_Open) {
		if (work_mode == kFile_Read)
			mode.AppendChar('r');
		else if (work_mode == kFile_Write || work_mode == kFile_ReadWrite)
			mode.Append("r+");
	} else if (open_mode == kFile_Create) {
		if (work_mode == kFile_Write)
			mode.AppendChar('a');
		else if (work_mode == kFile_Read || work_mode == kFile_ReadWrite)
			mode.Append("a+");
	} else if (open_mode == kFile_CreateAlways) {
		if (work_mode == kFile_Write)
			mode.AppendChar('w');
		else if (work_mode == kFile_Read || work_mode == kFile_ReadWrite)
			mode.Append("w+");
	}
	mode.AppendChar('b');
	return mode;
}

Stream *File::OpenFile(const String &filename, FileOpenMode open_mode, FileWorkMode work_mode) {
	FileStream *fs = nullptr;
//	try {
		if (work_mode == kFile_Read) // NOTE: BufferedStream does not work correctly in the write mode
			fs = new BufferedStream(filename, open_mode, work_mode);
		else
			fs = new FileStream(filename, open_mode, work_mode);
 		if (fs != nullptr && !fs->IsValid()) {
			delete fs;
			fs = nullptr;
		}
//	} catch (std::runtime_error) {
//		fs = nullptr;
//	}

	return fs;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
