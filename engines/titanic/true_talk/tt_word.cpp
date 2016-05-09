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

#include "titanic/true_talk/tt_word.h"

namespace Titanic {

TTWord::TTWord(TTString &str, int val1, int val2) : _string(str),
		_field18(val1), _field1C(val2), _fieldC(0), _field10(0),
		_field20(0), _field24(0), _field28(0) {
	_status = str.getStatus() == SS_VALID ? SS_VALID : SS_5;
}

int TTWord::readSyn(SimpleFile *file) {
	return 0;
}

int TTWord::load(SimpleFile *file, int *mode) {
	// TODO
	return 0;
}

/*------------------------------------------------------------------------*/

TTWord1::TTWord1(TTString &str, int val1, int val2, int val3) :
		TTWord(str, val1, val2), _field2C(val3) {
}

/*------------------------------------------------------------------------*/

TTWord2::TTWord2(TTString &str, int val1, int val2, int val3, int val4) :
		TTWord1(str, val1, val2, val3), _field30(val4) {
}

int TTWord2::load(SimpleFile *file) {
	// TODO
	return 0;
}

/*------------------------------------------------------------------------*/

TTWord3::TTWord3(TTString &str, int val1, int val2, int val3, int val4, int val5, int val6) :
		TTWord1(str, val1, val2, val4), _field34(val3), _field30(val5), _field3C(val6),
		_field38(0) {
}

int TTWord3::load(SimpleFile *file) {
	// TODO
	return 0;
}

/*------------------------------------------------------------------------*/

TTWord4::TTWord4(TTString &str, int val1, int val2, int val3, int val4) :
		TTWord1(str, val1, val2, val3) {
	if (val4 >= 0 && val4 <= 9) {
		_field30 = val4;
	} else {
		_field30 = 0;
		_status = SS_5;
	}
}

int TTWord4::load(SimpleFile *file) {
	// TODO
	return 0;
}

/*------------------------------------------------------------------------*/

TTWord5::TTWord5(TTString &str, int val1, int val2, int val3, int val4) :
	TTWord1(str, val1, val2, val3), _field30(val4) {
}

int TTWord5::load(SimpleFile *file) {
	// TODO
	return 0;
}

} // End of namespace Titanic
