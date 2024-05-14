/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_CODEWRITER_H
#define LINGODEC_CODEWRITER_H

#include "common/str.h"

namespace LingoDec {

class CodeWriter {
protected:
	Common::String _res;

	Common::String _lineEnding;
	Common::String _indentation;

	int _indentationLevel = 0;
	bool _indentationWritten = false;
	size_t _lineWidth = 0;
	size_t _size = 0;

public:
	bool doIndentation = true;

public:
	CodeWriter(Common::String lineEnding, Common::String indentation = "  ")
		: _lineEnding(lineEnding), _indentation(indentation) {}

	void write(Common::String str);
	void write(char ch);
	void writeLine(Common::String str);
	void writeLine();

	void indent();
	void unindent();

	Common::String str() const;
	size_t lineWidth() const { return _lineWidth; }
	size_t size() const { return _size; }

protected:
	void writeIndentation();
};

} // namespace Common

#endif // LINGODEC_CODEWRITER_H
