/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_TEXTSPLIT_HH
#define GRIM_TEXTSPLIT_HH

namespace Grim {

// A utility class to help in parsing the text-format files.  Splits
// the text data into lines, skipping comments, trailing whitespace,
// and empty lines.  Also folds everything to lowercase.

class TextSplitter {
public:
	TextSplitter(const char *data, int len);

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
	class TextLines {
	public:
		TextLines() {};
		~TextLines() { delete[] _lineData; }
		void setData(char *data, int length);
		char *getData() { return _lineData; }

	protected:
		char *_lineData;
		int _lineLength;

		friend class TextSplitter;
	};

	~TextSplitter() { delete[] _lines; }

private:
	char *_currLine;
	int _numLines, _lineIndex;
	TextLines *_lines;

	void processLine();
};

} // end of namespace Grim

#endif
