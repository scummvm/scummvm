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

#include "common/textconsole.h"
#include "titanic/support/font.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

STFont::STFont() {
	_dataPtr = nullptr;
	_dataSize = 0;
	_field8 = 0;
	_maxCharWidth = 0;
	_field810 = 0;
	_field814 = 0;
	_field818 = 0;
}

STFont::~STFont() {
	delete[] _dataPtr;
}

void STFont::load(int fontNumber) {
	assert(!_dataPtr);
	Common::SeekableReadStream *stream = g_vm->_filesManager.getResource(
		Common::WinResourceID("STFONT"), fontNumber);
	if (!stream)
		error("Could not locate the specified font");

	_field8 = stream->readUint32LE();
	_maxCharWidth = stream->readUint32LE();
	for (uint idx = 0; idx < 256; ++idx)
		_chars[idx]._charWidth = stream->readUint32LE();
	for (uint idx = 0; idx < 256; ++idx)
		_chars[idx]._offset = stream->readUint32LE();

	_dataSize = stream->readUint32LE();
	_dataPtr = new byte[_dataSize];
	stream->read(_dataPtr, _dataSize);

	delete stream;
}

void STFont::writeString(int maxWidth, const CString &text, int *v1, int *v2) {
	warning("TODO: STFont::writeString");
}

int STFont::stringWidth(const CString &text) const {
	if (text.empty())
		return 0;

	const char *srcP = text.c_str();
	int total = 0;
	char c;
	while (c = *srcP++) {
		if (c == 26) {
			// Skip over command parameter bytes
			srcP += 3;
		} else if (c == 27) {
			// Skip over command parameter bytes
			srcP += 4;
		} else if (c != '\n') {
			total += _chars[c]._charWidth;
		}
	}

	return total;
}

} // End of namespace Titanic
