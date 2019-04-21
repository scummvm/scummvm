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

#include "titanic/star_control/orientation_changer.h"
#include "titanic/star_control/fpose.h"
#include "titanic/star_control/fmatrix.h"

namespace Titanic {

void COrientationChanger::load(const FMatrix &minOrient, const FMatrix &maxOrient) {
	_minOrient = minOrient;
	_maxOrient = maxOrient;

	_sub1.fn4(_minOrient);
	_sub2.fn4(_maxOrient);
}

FMatrix COrientationChanger::getOrientation(double percent) {
	if (percent <= 0.0) {
		return _minOrient;
	} else if (percent > 1.0) {
		return _maxOrient;
	} else {
		CMatrixTransform tfm = _sub1.fn5(percent, _sub2);

		FPose m1;
		m1.loadTransform(tfm);
		return m1;
	}
}

} // End of namespace Titanic
