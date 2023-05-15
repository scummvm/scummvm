/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TE_TE_INTERPOLATION_H
#define TETRAEDGE_TE_TE_INTERPOLATION_H

#include "common/array.h"
#include "common/stream.h"
#include "common/fs.h"

#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class TeInterpolation {
public:
	TeInterpolation();

	void load(Common::ReadStream &stream);
	void load(TetraedgeFSNode &node);

	// Note: this function is not in the original but simplifies
	// the code for TeCurveAnim2 a lot.
	void load(const Common::Array<double> &array);
	void load(const Common::Array<float> &array);

	double interpole(double amount, double max) const;

private:
	Common::Array<double> _array;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_INTERPOLATION_H
