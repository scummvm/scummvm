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

#include "titanic/star_control/star_control_sub27.h"
#include "common/textconsole.h"

namespace Titanic {

void CStarControlSub27::proc2(FVector &v1, FVector &v2, FMatrix &m1, FMatrix &m2) {
	CStarControlSub23::proc2(v1, v2, m1, m2);

	int v24 = _field24;
	if (_field24 > 0.0) {
		_field8 = 1;
		_field34 = 1;
		proc6(120, 4, _field24);
	}

	if (m1 != m2) {
		_sub25.fn1(m1, m2);
		_field58 = 0;
		_field5C = 0.0;

		if (_field4C == 0) {
			_field60 = -1.5881868e-23;
			_field64 = 1.4499999;
			_field8 = 1;
		} else {
			_field60 = 1.0 / (double)v24;
			_field8 = 1;
		}
	}
}

int CStarControlSub27::proc5(CErrorCode &errorCode, FVector &v, const FMatrix &m) {
	// TODO
	return 0;
}

} // End of namespace Titanic
