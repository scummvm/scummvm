// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef TEXTSPLIT_HH
#define TEXTSPLIT_HH

#include <cstdlib>

// A utility class to help in parsing the text-format files.  Splits
// the text data into lines, skipping comments, trailing whitespace,
// and empty lines.  Also folds everything to lowercase.

class TextSplitter {
public:
	TextSplitter(const char *data, int len);

	char *nextLine() {
		curr_line_ = next_line_;
		processLine();
		return curr_line_;
	}

	char *currentLine() { return curr_line_; }
	const char *currentLine() const { return curr_line_; }
	bool eof() const { return curr_line_ == NULL; }

	// Check if the current line contains 'needle'
	bool TextSplitter::checkString(const char *needle);

	// Expect a certain fixed string; bail out with an error if not
	// found.  Advance to the next line.
	void expectString(const char *expected);

	// Scan a line according to the given format (compatible with
	// scanf); if not all fields are read (according to the field_count
	// argument), bail out with an error.  Advance to the next line.
	void scanString(const char *fmt, int field_count, ...)
#ifdef __GNUC__
	__attribute__((format (scanf, 2, 4)))
#endif
	;

	~TextSplitter() { delete[] data_; }

private:
	char *data_, *curr_line_, *next_line_;

	void processLine();
};

#endif
