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
// Class for reading plain text from the stream
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_TEXT_STREAM_READER_H
#define AGS_SHARED_UTIL_TEXT_STREAM_READER_H

#include "ags/shared/util/text_reader.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Stream;

class TextStreamReader : public TextReader {
public:
	// TODO: use shared ptr
	TextStreamReader(Stream *stream);
	~TextStreamReader() override;

	bool    IsValid() const override;
	const Stream *GetStream() const;
	// TODO: use shared ptr instead
	void            ReleaseStream();

	bool            EOS() const;

	// Read single character
	char    ReadChar() override;
	// Read defined number of characters
	String  ReadString(size_t length) override;
	// Read till line break
	String  ReadLine() override;
	// Read till end of available data
	String  ReadAll() override;

private:
	Stream *_stream;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
