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
// Class for writing plain text to the stream
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXT_STREAM_WRITER_H
#define AGS_SHARED_UTIL_TEXT_STREAM_WRITER_H

#include "ags/shared/util/text_writer.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Stream;

class TextStreamWriter : public TextWriter {
public:
	// TODO: use shared ptr
	TextStreamWriter(Stream *stream);
	~TextStreamWriter() override;

	bool    IsValid() const override;
	const Stream *GetStream() const;
	// TODO: use shared ptr instead
	void            ReleaseStream();

	bool            EOS() const;

	// Write single character
	void    WriteChar(char c) override;
	// Write string as a plain text (without null-terminator)
	void    WriteString(const String &str) override;
	// Write string and add line break at the end
	void    WriteLine(const String &str) override;
	// Write formatted string (see *printf)
	void    WriteFormat(const char *fmt, ...) override;
	void    WriteLineBreak() override;

private:
	Stream *_stream;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
