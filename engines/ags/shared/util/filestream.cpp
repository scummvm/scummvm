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

#include "ags/shared/util/filestream.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"
#include "ags/ags.h"
#include "common/file.h"
#include "common/system.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

FileStream::FileStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode,
	DataEndianess stream_endianess)
		: DataStream(stream_endianess), _writeBuffer(DisposeAfterUse::YES),
		_openMode(open_mode), _workMode(work_mode), _file(nullptr), _outSave(nullptr) {
	Open(file_name, open_mode, work_mode);
}

FileStream::~FileStream() {
	FileStream::Close();
}

bool FileStream::HasErrors() const {
	return IsValid() && _file->err();
}

void FileStream::Close() {
	if (_outSave) {
		_outSave->write(_writeBuffer.getData(), _writeBuffer.size());
		_outSave->finalize();
		delete _outSave;

	} else if (_file) {
		delete _file;
	}

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
	return !rs || rs->eos();
}

soff_t FileStream::GetLength() const {
	if (IsValid()) {
		soff_t pos = (soff_t)ags_ftell(_file);
		ags_fseek(_file, 0, SEEK_END);
		soff_t end = (soff_t)ags_ftell(_file);
		ags_fseek(_file, pos, SEEK_SET);
		return end;
	}

	return 0;
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

bool FileStream::Seek(soff_t offset, StreamSeek origin) {
	if (!_file) {
		return false;
	}

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
		// TODO: warning to the log
		return false;
	}

	return ags_fseek(_file, (file_off_t)offset, stdclib_origin) == 0;
}

void FileStream::Open(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode) {
	if (open_mode == kFile_Open) {
		// First try to open file in game folder
		Common::File *f = new Common::File();
		if (!file_name.CompareLeftNoCase("agssave.") || !f->open(getFSNode(file_name.GetNullableCStr()))) {
			delete f;

			// Fall back on any save file with the given name
			String saveName = file_name;
			if (!file_name.CompareLeftNoCase("agssave.")) {
				int saveSlot = atoi(file_name.GetNullableCStr() + 8);
				saveName = ::AGS::g_vm->getSaveStateName(saveSlot);
			}

			_file = g_system->getSavefileManager()->openForLoading(saveName);
		} else {
			_file = f;
		}

	} else {
		// All newly created files are created as save files
		_outSave = g_system->getSavefileManager()->openForSaving(file_name, false);
		if (_outSave) {
			// Any data written has to first go through the memory stream buffer,
			// since the savegame code uses Seeks, which OutSaveFile doesn't support
			_file = &_writeBuffer;
		}
	}
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
