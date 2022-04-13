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

#include "ags/shared/util/math.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/text_stream_reader.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

TextStreamReader::TextStreamReader(Stream *stream)
	: _stream(stream) {
}

TextStreamReader::~TextStreamReader() {
	// TODO: use shared ptr
	delete _stream;
}

bool TextStreamReader::IsValid() const {
	return _stream && _stream->CanRead();
}

const Stream *TextStreamReader::GetStream() const {
	return _stream;
}

void TextStreamReader::ReleaseStream() {
	_stream = nullptr;
}

bool TextStreamReader::EOS() const {
	return _stream ? _stream->EOS() : true;
}

char TextStreamReader::ReadChar() {
	return _stream->ReadInt8();
}

String TextStreamReader::ReadString(size_t length) {
	// TODO: remove carriage-return characters
	return String::FromStreamCount(_stream, length);
}

String TextStreamReader::ReadLine() {
	// TODO
	// Probably it is possible to group Stream::ReadString with this,
	// both use similar algorythm, difference is only in terminator chars

	String str;
	int chars_read_last = 0;
	int line_break_position = -1;
	// Read a chunk of memory to buffer and seek for null-terminator,
	// if not found, repeat until EOS
	const int single_chunk_length = 3000;
	const int max_chars = 5000000;
	char char_buffer[single_chunk_length + 1];
	do {
		chars_read_last = _stream->Read(char_buffer, single_chunk_length);
		char *seek_ptr = char_buffer;
		int c;
		for (c = 0; c < chars_read_last && *seek_ptr != '\n'; ++c, ++seek_ptr);

		int append_length = 0;
		int str_len = str.GetLength();
		if (c < chars_read_last && *seek_ptr == '\n') {
			line_break_position = seek_ptr - char_buffer;
			if (str_len < max_chars) {
				append_length = Math::Min(line_break_position, max_chars - str_len);
			}
		} else {
			append_length = Math::Min(chars_read_last, max_chars - str_len);
		}

		if (append_length > 0) {
			char_buffer[append_length] = '\0';
			str.Append(char_buffer);
		}
	} while (!EOS() && line_break_position < 0);

	// If null-terminator was found make sure stream is positioned at the next
	// byte after line end
	if (line_break_position >= 0) {
		// CHECKME: what if stream does not support seek? need an algorythm fork for that
		// the seek offset should be negative
		_stream->Seek(line_break_position - chars_read_last + 1 /* beyond line feed */);
	}

	str.TrimRight('\r'); // skip carriage-return, if any
	return str;
}

String TextStreamReader::ReadAll() {
	soff_t len = _stream->GetLength() - _stream->GetPosition();
	return ReadString(len > SIZE_MAX ? SIZE_MAX : (size_t)len);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
