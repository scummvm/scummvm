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
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class TeBezierCurve : public Te3DObject2, public TeReferencesCounter {
public:
	TeBezierCurve();

	int bounds(int val);
	void clear();
	void draw() override;
	float length();

	void pseudoTangent(float f, TeVector3f32 &v1, TeVector3f32 &v2);

	float rawLength();

	TeVector3f32 retrievePoint(float offset);
	void setControlPoints(const Common::Array<TeVector3f32> &points);
	void setNbIterations(uint iterations);

	static TeVector3f32 hermiteInterpolate(float param_2, const TeVector3f32 *points, float param_4, float param_5);

	static void serialize(Common::WriteStream &stream, const TeBezierCurve &curve);
	static void deserialize(Common::ReadStream &stream, TeBezierCurve &curve);
	void loadBin(TetraedgeFSNode &node);

	const Common::Array<TeVector3f32> &controlPoints() { return _controlPoints; }
	uint numIterations() const { return _numIterations; }

private:
	uint _numIterations;
	float _length;
	float _rawLength;
	bool _lengthNeedsUpdate;
	bool _rawLengthNeedsUpdate;
	Common::Array<TeVector3f32> _controlPoints;
	Common::Array<float> _rawLengths;
	Common::Array<float> _lengths;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_BEZIER_CURVE_H
