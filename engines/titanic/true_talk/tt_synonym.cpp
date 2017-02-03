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

#include "titanic/true_talk/tt_synonym.h"

namespace Titanic {

TTsynonym::TTsynonym() : TTstringNode() {
}

TTsynonym::TTsynonym(const TTsynonym *src) : TTstringNode() {
	_string = src->_string;
	initialize(src->_mode);
	_file = src->_file;
}

TTsynonym::TTsynonym(int mode, const char *str, FileHandle file) :
		TTstringNode() {
	_string = str;
	initialize(mode);
	_file = file;
}

TTsynonym::TTsynonym(int mode, TTstring *str) : TTstringNode() {
	_string = *str;
	initialize(mode);
}

TTsynonym *TTsynonym::copyFrom(const TTsynonym *src) {
	if (src->_field1C) {
		_field1C = 5;
	} else {
		_field1C = 0;
		if (src != this)
			_string = src->_string;
	}

	return this;
}

int TTsynonym::save(SimpleFile *file) {
	for (TTstringNode *synP = this; synP; synP = dynamic_cast<TTstringNode *>(synP->_nextP)) {
		file->writeFormat("%s", " 0 ");
		synP->_string.save(file);
		file->writeFormat("%c", ' ');

		if (synP->_mode) {
			file->writeFormat("%1.0d", synP->_mode);
		} else {
			file->writeFormat("%c", '0');
		}

		file->writeFormat("%c", ' ');

		if (synP->_file) {
			file->writeFormat("%2.0d", synP->_file);
		} else {
			file->writeFormat("%c", ' ');
		}
		file->writeFormat("%c", '\n');
	}

	return 0;
}

} // End of namespace Titanic
