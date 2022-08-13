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

#ifndef TETRAEDGE_TE_TE_BEZIER_CURVE_H
#define TETRAEDGE_TE_TE_BEZIER_CURVE_H

#include "common/array.h"

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_references_counter.h"

namespace Tetraedge {

class TeBezierCurve : public Te3DObject2, public TeReferencesCounter {
public:
	TeBezierCurve();

	long bounds(int val);
	void clear();
	void draw() override;
	float length();

	float rawLength();

	TeVector3f32 retrievePoint(float offset) const;
	void setControlPoints(const Common::Array<TeVector3f32> &points);
	void setNbIterations(unsigned long iterations);

	static void serialize(Common::WriteStream &stream, const TeBezierCurve &mesh);
	static void deserialize(Common::ReadStream &stream, TeBezierCurve &mesh);

private:
	int _numiterations;
	float _length;
	float _rawLength;
	bool _lengthNeedsUpdate;
	bool _rawLengthNeedsUpdate;
	Common::Array<TeVector3f32> _controlPoints;
	Common::Array<float> _rawLengths;
	// TODO add private members

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_BEZIER_CURVE_H
