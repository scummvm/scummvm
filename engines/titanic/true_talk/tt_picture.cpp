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

#include "titanic/true_talk/tt_picture.h"

namespace Titanic {

TTpicture::TTpicture(TTString &str, int val1, int val2, int val3, int val4, int val5, int val6) :
		TTmajorWord(str, val1, val2, val4), _field34(val3), _field30(val5), _field3C(val6),
		_field38(0) {
}

TTpicture::TTpicture(TTpicture *src) : TTmajorWord(src) {
	if (getStatus()) {
		_field34 = 0;
		_field30 = 0;
		_field38 = 0;
		_field3C = 0;
		_status = SS_5;
	} else {
		_field34 = src->_field34;
		_field30 = src->_field30;
		_field38 = src->_field38;
		_field3C = src->_field3C;
	}
}

int TTpicture::load(SimpleFile *file) {
	CString str;
	int val1, val2;

	if (!TTword::load(file, 2) && file->scanf("%s %d %d", &str, &val1, &val2)) {
		_field34 = readNumber(str.c_str());
		_field30 = val1;
		_field3C = val2;
		return 0;
	} else {
		return 3;
	}
}

} // End of namespace Titanic
