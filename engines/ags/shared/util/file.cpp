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
#include "ags/shared/util/buffered_stream.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/file_stream.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/stdio_compat.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

soff_t File::GetFileSize(const String &filename) {
	if (filename.IsEmpty())
		return 0;
	return ags_file_size(filename.GetCStr());
}

bool File::TestReadFile(const String &filename) {
	if (filename.IsEmpty())
		return false;
	return ags_file_exists(filename.GetCStr());
}

bool File::TestWriteFile(const String &filename) {
	if (filename.IsEmpty())
		return false;
	return TestCreateFile(filename);
}

bool File::TestCreateFile(const String &filename) {
	if (filename.IsEmpty())
		return false;

	Common::OutSaveFile *sf = g_system->getSavefileManager()->openForSaving(filename);
	bool result = sf != nullptr;
	delete sf;

	return result;
}

bool File::DeleteFile(const String &filename) {
	// Only allow deleting files in the savegame folder
	if (filename.CompareLeftNoCase(SAVE_FOLDER_PREFIX) != 0) {
		warning("Cannot delete file %s. Only files in the savegame directory can be deleted", filename.GetCStr());
		return false;
	}
	Common::String file(filename.GetCStr() + strlen(SAVE_FOLDER_PREFIX));
	return g_system->getSavefileManager()->removeSavefile(file);
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
	//  try {
	if (work_mode == kFile_Read) // NOTE: BufferedStream does not work correctly in the write mode
		fs = new BufferedStream(filename, open_mode, work_mode);
	else
		fs = new FileStream(filename, open_mode, work_mode);
	if (fs != nullptr && !fs->IsValid()) {
		delete fs;
		fs = nullptr;
	}
	//  } catch (std::runtime_error) {
	//      fs = nullptr;
	//  }

	return fs;
}

Stream *File::OpenStdin() {
	error("TODO: File::OpenStdin");
}

Stream *File::OpenStdout() {
	error("TODO: File::OpenStdout");
}

Stream *File::OpenStderr() {
	error("TODO: File::OpenStderr");
}

String File::FindFileCI(const String &dir_name, const String &file_name) {
#if !defined (AGS_CASE_SENSITIVE_FILESYSTEM)
	// Simply concat dir and filename paths
	return Path::ConcatPaths(dir_name, file_name);
#else
	// Case insensitive file find - on case sensitive filesystems
	//
	// TODO: still not covered: a situation when the file_name contains
	// nested path -and- the case of at least one path parts does not match
	// (with all matching case the file will be found by an early check).
	//
	struct stat   statbuf;
	struct dirent *entry = nullptr;

	if (dir_name.IsEmpty() && file_name.IsEmpty())
		return nullptr;

	String directory;
	String filename;
	String buf;

	if (!dir_name.IsEmpty()) {
		directory = dir_name;
		Path::FixupPath(directory);
	}
	if (!file_name.IsEmpty()) {
		filename = file_name;
		Path::FixupPath(filename);
	}

	if (!filename.IsEmpty()) {
		// TODO: move this case to ConcatPaths too?
		if (directory.IsEmpty() && filename[0] == '/')
			buf = filename;
		else
			buf = Path::ConcatPaths(directory.IsEmpty() ? "." : directory, filename);

		if (lstat(buf.GetCStr(), &statbuf) == 0 &&
			(S_ISREG(statbuf.st_mode) || S_ISLNK(statbuf.st_mode))) {
			return buf;
		}
	}

	if (directory.IsEmpty()) {
		String match = Path::GetFilename(filename);
		if (match.IsEmpty())
			return nullptr;
		directory = Path::GetParent(filename);
		filename = match;
	}

	DIR *rough = nullptr;
	if ((rough = opendir(directory.GetCStr())) == nullptr) {
		fprintf(stderr, "ci_find_file: cannot open directory: %s\n", directory.GetCStr());
		return nullptr;
	}

	String diamond;
	while ((entry = readdir(rough)) != nullptr) {
		if (strcasecmp(filename.GetCStr(), entry->d_name) == 0) {
			if (lstat(entry->d_name, &statbuf) == 0 &&
				(S_ISREG(statbuf.st_mode) || S_ISLNK(statbuf.st_mode))) {
#if AGS_PLATFORM_DEBUG
				fprintf(stderr, "ci_find_file: Looked for %s in rough %s, found diamond %s.\n",
					filename.GetCStr(), directory.GetCStr(), entry->d_name);
#endif // AGS_PLATFORM_DEBUG
				diamond = Path::ConcatPaths(directory, entry->d_name);
				break;
			}
		}
	}
	closedir(rough);
	return diamond;
#endif
}

Stream *File::OpenFileCI(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode) {
#if !defined (AGS_CASE_SENSITIVE_FILESYSTEM)
	return File::OpenFile(file_name, open_mode, work_mode);
#else
	String fullpath = FindFileCI(nullptr, file_name);
	if (!fullpath.IsEmpty())
		return File::OpenFile(fullpath, open_mode, work_mode);
	// If the file was not found, and it's Create mode, then open new file
	if (open_mode != kFile_Open)
		return File::OpenFile(file_name, open_mode, work_mode);
	return nullptr;
#endif
}

Stream *File::OpenFile(const String &filename, soff_t start_off, soff_t end_off) {
	FileStream *fs = new BufferedSectionStream(filename, start_off, end_off, kFile_Open, kFile_Read);
	if (fs != nullptr && !fs->IsValid()) {
		delete fs;
		return nullptr;
	}

	return fs;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
