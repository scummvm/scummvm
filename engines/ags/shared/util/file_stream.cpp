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

#include "ags/shared/util/file_stream.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/directory.h"
#include "ags/ags.h"
#include "common/file.h"
#include "common/system.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

FileStream::FileStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode,
                       DataEndianess stream_endianess)
		: DataStream(stream_endianess), _workMode(work_mode), _file(nullptr) {
	Open(file_name, open_mode, work_mode);
}

FileStream::~FileStream() {
	FileStream::Close();
}

bool FileStream::GetError() const {
	if (!_file)
		return false;
	bool err = _file->err();
	_file->clearErr();
	return err;
}

void FileStream::Close() {
	delete _file;
	_file = nullptr;
}

bool FileStream::Flush() {
	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(_file);
	if (ws)
		ws->flush();

	return false;
}

bool FileStream::IsValid() const {
	return _file != nullptr;
}

bool FileStream::EOS() const {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(_file);
	return rs && rs->eos();
}

soff_t FileStream::GetLength() const {
	soff_t pos = (soff_t)ags_ftell(_file);
	ags_fseek(_file, 0, SEEK_END);
	soff_t end = (soff_t)ags_ftell(_file);
	ags_fseek(_file, pos, SEEK_SET);
	return end;
}

soff_t FileStream::GetPosition() const {
	if (IsValid()) {
		return (soff_t)ags_ftell(_file);
	}
	return -1;
}

bool FileStream::CanRead() const {
	return IsValid() && _workMode != kFile_Write;
}

bool FileStream::CanWrite() const {
	return IsValid() && _workMode != kFile_Read;
}

bool FileStream::CanSeek() const {
	return IsValid();
}

size_t FileStream::Read(void *buffer, size_t size) {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(_file);

	if (rs && buffer) {
		return rs->read(buffer, size);
	}

	return 0;
}

int32_t FileStream::ReadByte() {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(_file);

	if (rs) {
		return rs->eos() ? -1 : (int32_t)rs->readByte();
	}

	return -1;
}

size_t FileStream::Write(const void *buffer, size_t size) {
	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(_file);

	if (ws && buffer) {
		return ws->write(buffer, size);
	}

	return 0;
}

int32_t FileStream::WriteByte(uint8_t val) {
	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(_file);

	if (ws) {
		ws->writeByte(val);
		return 1;
	}

	return -1;
}

soff_t FileStream::Seek(soff_t offset, StreamSeek origin) {
	int stdclib_origin;
	switch (origin) {
	case kSeekBegin:
		stdclib_origin = SEEK_SET;
		break;
	case kSeekCurrent:
		stdclib_origin = SEEK_CUR;
		break;
	case kSeekEnd:
		stdclib_origin = SEEK_END;
		break;
	default:
		return -1;
	}

	return (ags_fseek(_file, (file_off_t)offset, stdclib_origin) == 0) ? ags_ftell(_file) : -1;
}

void FileStream::Open(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode) {
	if (open_mode == kFile_Open) {
		if (!file_name.CompareLeftNoCase(SAVE_FOLDER_PREFIX)) {
			String saveName = getSaveName(file_name);
			_file = g_system->getSavefileManager()->openForLoading(saveName);

		} else {
			// First try to open file in game folder
			Common::ArchiveMemberPtr desc = getFile(file_name.GetCStr());
			_file = desc ? desc->createReadStream() : nullptr;
		}

	} else {
		String saveName;

		if (!file_name.CompareLeftNoCase(SAVE_FOLDER_PREFIX)) {
			saveName = getSaveName(file_name);

		} else {
			Common::String fname = file_name;
			if (fname.hasPrefix("./"))
				fname = fname.substr(2);
			else if (fname.hasPrefix("/"))
				fname.deleteChar(0);
			else if (fname.findFirstOf('/') != Common::String::npos)
				error("Invalid attempt to create file - %s", fname.c_str());

			saveName = fname;
		}

		_file = openForWriting(saveName, open_mode, work_mode);

		if (!_file)
			error("Invalid attempt to create file - %s", file_name.GetCStr());

		_path = file_name;
	}
}

String FileStream::getSaveName(const String &filename) {
	return String(filename.GetCStr() + strlen(SAVE_FOLDER_PREFIX));
}

Common::OutSaveFile *FileStream::openForWriting(const String &saveName, FileOpenMode open_mode, FileWorkMode work_mode) {
	assert(open_mode != kFile_Open);

	if (work_mode == kFile_Read || work_mode == kFile_ReadWrite)
		// In the original these modes result in [aw]+b, which seems to result
		// in a file with arbitrary reading, but writing always appending
		warning("FileOpen: independent read/write positions not supported");

	Common::InSaveFile *existing = nullptr;
	if (open_mode == kFile_Create &&
		(existing = g_system->getSavefileManager()->openForLoading(saveName)) != nullptr) {
		// kFile_Create mode allows opening existing files for read/write.
		// Since ScummVM doesn't support this via the save file manager,
		// we need to do a bit of a hack and load the existing contents,
		// then recreate the file and write out the contents so that further
		// writes will be possible without affecting the old data
		size_t fileSize = existing->size();
		byte *data = new byte[fileSize];
		existing->read(data, fileSize);
		delete existing;

		Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(saveName, false);
		assert(out);

		out->write(data, fileSize);
		delete[] data;

		return out;
	}

	return g_system->getSavefileManager()->openForSaving(saveName, false);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
