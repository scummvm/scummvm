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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/text_stream_writer.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// TODO: perhaps let configure line break character per TextWriter object?
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
	return _stream->EOS();
}

void TextStreamWriter::WriteChar(char c) {
	_stream->WriteByte(c);
}

void TextStreamWriter::WriteString(const String &str) {
	_stream->Write(str.GetCStr(), str.GetLength());
}

void TextStreamWriter::WriteLine(const String &str) {
	// TODO: perhaps let configure line break character?
	_stream->Write(str.GetCStr(), str.GetLength());
	_stream->Write(Endl, sizeof(Endl));
}

void TextStreamWriter::WriteFormat(const char *fmt, ...) {
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
	_stream->Write(Endl, sizeof(Endl));
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
