/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "./codewriter.h"

namespace LingoDec {

void CodeWriter::write(Common::String str) {
	if (str.empty())
		return;

	writeIndentation();
	_res += str;
	_lineWidth += str.size();
	_size += str.size();
}

void CodeWriter::write(char ch) {
	writeIndentation();
	_res += ch;
	_lineWidth += 1;
	_size += 1;
}

void CodeWriter::writeLine(Common::String str) {
	if (str.empty()) {
		_res += _lineEnding;
	} else {
		writeIndentation();
		_res += str;
		_res += _lineEnding;
	}
	_indentationWritten = false;
	_lineWidth = 0;
	_size += str.size() + _lineEnding.size();
}

void CodeWriter::writeLine() {
	_res += _lineEnding;
	_indentationWritten = false;
	_lineWidth = 0;
	_size += _lineEnding.size();
}

void CodeWriter::indent() {
	_indentationLevel += 1;
}

void CodeWriter::unindent() {
	if (_indentationLevel > 0) {
		_indentationLevel -= 1;
	}
}

Common::String CodeWriter::str() const {
	return _res;
}

void CodeWriter::writeIndentation() {
	if (_indentationWritten || !doIndentation)
		return;

	for (int i = 0; i < _indentationLevel; i++) {
		_res += _indentation;
	}

	_indentationWritten = true;
	_lineWidth = _indentationLevel * _indentation.size();
	_size += _lineWidth;
}

} // namespace Common
