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

#ifndef AGS_SHARED_UTIL_FILE_STREAM_H
#define AGS_SHARED_UTIL_FILE_STREAM_H

#include "common/savefile.h"
#include "common/stream.h"
#include "common/std/functional.h"
#include "ags/shared/util/data_stream.h"
#include "ags/shared/util/file.h" // TODO: extract filestream mode constants

namespace AGS3 {
namespace AGS {
namespace Shared {

class FileStream : public DataStream {
public:
	struct CloseNotifyArgs {
		String Filepath;
		FileWorkMode WorkMode;
	};

	// Represents an open file object
	// The constructor may raise std::runtime_error if
	// - there is an issue opening the file (does not exist, locked, permissions, etc)
	// - the open mode could not be determined
	FileStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode,
			   DataEndianess stream_endianess = kLittleEndian);
	~FileStream() override;

	FileWorkMode GetWorkMode() const { return _workMode; }

	// Tells if there were errors during previous io operation(s);
	// the call to GetError() *resets* the error record.
	bool GetError() const override;
	void Close() override;
	bool Flush() override;

	// Is stream valid (underlying data initialized properly)
	bool IsValid() const override;
	// Is end of stream
	bool EOS() const override;
	// Total length of stream (if known)
	soff_t GetLength() const override;
	// Current position (if known)
	soff_t GetPosition() const override;
	bool CanRead() const override;
	bool CanWrite() const override;
	bool CanSeek() const override;

	size_t Read(void *buffer, size_t size) override;
	int32_t ReadByte() override;
	size_t Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;

	soff_t Seek(soff_t offset, StreamSeek origin) override;

private:
	void Open(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode);
	String getSaveName(const String &filename);
	Common::OutSaveFile *openForWriting(const String &saveName, FileOpenMode open_mode, FileWorkMode work_mode);

	Common::Stream *_file;
	const FileWorkMode  _workMode;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
