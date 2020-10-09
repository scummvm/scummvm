/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_TEXTSPLIT_HH
#define GRIM_TEXTSPLIT_HH

namespace Common {
class SeekableReadStream;
}

namespace Grim {

// A utility class to help in parsing the text-format files.  Splits
// the text data into lines, skipping comments, trailing whitespace,
// and empty lines.  Also folds everything to lowercase.

class TextSplitter {
public:
	TextSplitter(const Common::String &fname, Common::SeekableReadStream *data);
	~TextSplitter();

	char *nextLine() {
		processLine();
		return _currLine;
	}

	char *getCurrentLine() { return _currLine; }
	const char *getCurrentLine() const { return _currLine; }
	bool isEof() const { return _lineIndex == _numLines; }
	int getLineNumber() { return _lineIndex; }
	void setLineNumber(int line) { _lineIndex = line - 1; processLine(); }

	// Check if the current line contains 'needle'
	bool checkString(const char *needle);

	// Expect a certain fixed string; bail out with an error if not
	// found.  Advance to the next line.
	void expectString(const char *expected);

	// Scan a line according to the given format (compatible with
	// scanf); if not all fields are read (according to the field_count
	// argument), bail out with an error.  Advance to the next line.
	void scanString(const char *fmt, int field_count, ...);

	// Scan a line starting at offset 'offset' according to the given format
	// (compatible with scanf); if not all fields are read (according to the
	// field_count argument), bail out with an error.  Advance to the next line.
	void scanStringAtOffset(int offset, const char *fmt, int field_count, ...);

	// Just like scanString(), but without advancing to the next line.
	void scanStringNoNewLine(const char *fmt, int field_count, ...);

	// Just like scanStringAtOffset(), but without advancing to the next line.
	void scanStringAtOffsetNoNewLine(int offset, const char *fmt, int field_count, ...);

private:
	Common::String _fname;
	char *_stringData;
	char *_currLine;
	int _numLines, _lineIndex;
	char **_lines;

	void processLine();
};

} // end of namespace Grim

#endif
