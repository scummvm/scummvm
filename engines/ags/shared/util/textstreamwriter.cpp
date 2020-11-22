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

//include <stdarg.h>
//include <stdio.h> // sprintf
#include "ags/shared/core/platform.h"
#include "ags/shared/util/textstreamwriter.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

#if AGS_PLATFORM_OS_WINDOWS
static const char Endl[2] = { '\r', '\n' };
#else
static const char Endl[1] = { '\n' };
#endif


TextStreamWriter::TextStreamWriter(Stream *stream)
	: _stream(stream) {
}

TextStreamWriter::~TextStreamWriter() {
	// TODO use shared ptr
	delete _stream;
}

bool TextStreamWriter::IsValid() const {
	return _stream && _stream->CanWrite();
}

const Stream *TextStreamWriter::GetStream() const {
	return _stream;
}

void TextStreamWriter::ReleaseStream() {
	_stream = nullptr;
}

bool TextStreamWriter::EOS() const {
	return _stream ? _stream->EOS() : true;
}

void TextStreamWriter::WriteChar(char c) {
	if (_stream) {
		_stream->WriteByte(c);
	}
}

void TextStreamWriter::WriteString(const String &str) {
	if (_stream) {
		// TODO: replace line-feed characters in string with platform-specific line break
		_stream->Write(str.GetCStr(), str.GetLength());
	}
}

void TextStreamWriter::WriteLine(const String &str) {
	if (!_stream) {
		return;
	}

	// TODO: replace line-feed characters in string with platform-specific line break
	_stream->Write(str.GetCStr(), str.GetLength());
	_stream->Write(Endl, sizeof(Endl));
}

void TextStreamWriter::WriteFormat(const char *fmt, ...) {
	if (!_stream) {
		return;
	}

	// TODO: replace line-feed characters in format string with platform-specific line break

	va_list argptr;
	va_start(argptr, fmt);
	int need_length = vsnprintf(nullptr, 0, fmt, argptr);
	va_start(argptr, fmt); // Reset argptr
	char *buffer = new char[need_length + 1];
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);

	_stream->Write(buffer, need_length);
	delete[] buffer;
}

void TextStreamWriter::WriteLineBreak() {
	if (_stream)
		_stream->Write(Endl, sizeof(Endl));
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
