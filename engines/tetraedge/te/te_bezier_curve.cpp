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

#include "common/file.h"

#include "tetraedge/te/te_bezier_curve.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

TeBezierCurve::TeBezierCurve() : _length(0.0), _rawLength(0.0), _lengthNeedsUpdate(true),
_rawLengthNeedsUpdate(true), _numIterations(1000) {
}

//int TeBezierCurve::bounds(int start);

void TeBezierCurve::clear() {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_length = 0.0;
	_controlPoints.clear();
}

void TeBezierCurve::draw() {
	if (!worldVisible() || _controlPoints.empty())
		return;

	Common::SharedPtr<TeMesh> mesh1(TeMesh::makeInstance());
	Common::SharedPtr<TeMesh> mesh2(TeMesh::makeInstance());
	uint npoints = _controlPoints.size();

	mesh1->setConf(npoints, npoints, TeMesh::MeshMode_Points, 0, 0);
	for (uint i = 0; i < npoints; i++) {
		mesh1->setVertex(i, _controlPoints[i]);
		mesh1->setIndex(i, i);
	}

	mesh2->setConf(npoints, npoints, TeMesh::MeshMode_LineStrip, 0, 0);
	for (uint i = 0; i < npoints; i++) {
		mesh2->setVertex(i, _controlPoints[i]);
		mesh2->setNormal(i, TeVector3f32(0.0f, 1.0f, 0.0));
		mesh2->setIndex(i, i);
	}

	TeRenderer *renderer = g_engine->getRenderer();
	const TeColor prevColor = renderer->currentColor();
	renderer->pushMatrix();
	renderer->multiplyMatrix(worldTransformationMatrix());
	renderer->setCurrentColor(TeColor(0, 0xff, 0xff, 0xff));
	mesh2->draw();
	renderer->setCurrentColor(TeColor(0xff, 0, 0xff, 0xff));
	mesh1->draw();
	renderer->popMatrix();
	renderer->setCurrentColor(prevColor);
}

float TeBezierCurve::length() {
	if (_lengthNeedsUpdate) {
		_length = 0.0;
		_lengthNeedsUpdate = false;
		_lengths.clear();

		TeVector3f32 lastpt = _controlPoints[0];
		lastpt.y() = 0;
		for (uint i = 0; i < _numIterations; i++) {
			float amount = (float)i / _numIterations;
			TeVector3f32 pt = retrievePoint(amount);
			pt.y() = 0;
			float len = (lastpt - pt).length();
			_length += len;
			_lengths.push_back(_length);
			lastpt = pt;
		}
	}
	return _length;
}

void TeBezierCurve::pseudoTangent(float offset, TeVector3f32 &v1, TeVector3f32 &v2) {
	const float step = 1.0f / _numIterations;
	if (step + offset <= 1.0f) {
		v1 = retrievePoint(offset);
		v2 = retrievePoint(offset + step);
	} else {
		v1 = retrievePoint(offset - step);
		v2 = retrievePoint(offset);
	}
}

float TeBezierCurve::rawLength() {
	if (_rawLengthNeedsUpdate) {
		_rawLengthNeedsUpdate = false;
		_rawLength = 0.0;
		_rawLengths.clear();
		_rawLengths.push_back(0.0);
		for (uint i = 1; i < _controlPoints.size(); i++) {
			const TeVector3f32 diff = _controlPoints[i] - _controlPoints[i - 1];
			_rawLength += diff.length();
			_rawLengths.push_back(_rawLength);
		}
	}
	return _rawLength;
}

TeVector3f32 TeBezierCurve::retrievePoint(float offset) {
	const int npoints = _controlPoints.size();

	// Simple cases for small numbers of points.
	if (npoints == 0)
		return TeVector3f32();
	else if (npoints == 1)
		return _controlPoints[0];
	else if (npoints == 2)
		return _controlPoints[0] + (_controlPoints[1] - _controlPoints[0]) * offset;

	// else, there are at least 3 points so need to actually interpolate.
	const float rawlen = rawLength();

	float proportion = 0.0f;
	int startpt = 0;
	while (startpt < npoints) {
		proportion = _rawLengths[startpt] / rawlen;
		if (proportion >= offset)
			break;
		startpt++;
	}

	float t;
	if (proportion == offset) {
		// Exactly on a point
		t = 0.0f;
	} else {
		// Proportion between two points
		float p1 = _rawLengths[startpt - 1];
		float p2 = _rawLengths[startpt];
		t = (rawlen * offset - p1) / (p2 - p1);
		startpt--;
	}

	// Collect 4 points around the startpt (1 before, 2 after)
	TeVector3f32 points[4];
	const int maxPt = _controlPoints.size() - 1;
	for (int p = 0; p < 4; p++) {
		int ptno = CLIP(startpt + p - 1, 0, maxPt);
		points[p] = _controlPoints[ptno];
	}

	// If we hit the end, linearly extend the last gradient.
	if (startpt <= 0)
		points[0] += (points[1] - points[2]);

	if (startpt + 1 >= maxPt)
		points[3] += (points[2] - points[1]);

	return hermiteInterpolate(t, points, 0.0, 0.0);
}

void TeBezierCurve::setControlPoints(const Common::Array<TeVector3f32> &points) {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_controlPoints = points;
}

void TeBezierCurve::setNbIterations(uint iterations) {
	_lengthNeedsUpdate = true;
	_rawLengthNeedsUpdate = true;
	_numIterations = iterations;
}

/*static*/
void TeBezierCurve::serialize(Common::WriteStream &stream, const TeBezierCurve &curve) {
	error("TODO: Implement TeBezierCurve::serialize");
}

/*static*/
void TeBezierCurve::deserialize(Common::ReadStream &stream, TeBezierCurve &curve) {
	Te3DObject2::deserialize(stream, curve);

	curve._lengthNeedsUpdate = false;
	curve._length = stream.readFloatLE();
	uint32 npoints = stream.readUint32LE();
	if (npoints > 1000000)
		error("TeBezierCurve::deserialize improbable number of control ponts %d", npoints);

	for (uint i = 0; i < npoints; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(stream, vec);
		curve._controlPoints.push_back(vec);
	}
}

void TeBezierCurve::loadBin(TetraedgeFSNode &node) {
	Common::ScopedPtr<Common::SeekableReadStream> file(node.createReadStream());
	Common::String fname = node.getPath().baseName();
	if (fname.size() < 4)
		error("TeBezierCurve::loadBin fname %s is too short", fname.c_str());
	setName(fname.substr(0, fname.size() - 4));

	// Load position / rotation / size
	Te3DObject2::deserialize(*file, *this, false);
	// Then it resets them?
	setPosition(TeVector3f32());
	setRotation(TeQuaternion());
	setSize(TeVector3f32(1, 1, 1));

	_lengthNeedsUpdate = true;
	uint32 npoints = file->readUint32LE();
	if (npoints > 1000000)
		error("TeBezierCurve::loadBin improbable number of control ponts %d", npoints);

	for (uint i = 0; i < npoints; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(*file, vec);
		_controlPoints.push_back(vec);
	}
}

/*static*/
TeVector3f32 TeBezierCurve::hermiteInterpolate(float t, const TeVector3f32 *points, float param_4, float param_5) {
	assert(points);
	const TeVector3f32 delta1 =  ((points[1] - points[0]) * (param_5 + 1.0) * (1.0 - param_4)) / 2.0;
	const TeVector3f32 delta2a = ((points[2] - points[1]) * (1.0 - param_5) * (1.0 - param_4)) / 2.0;
	const TeVector3f32 delta2b = ((points[2] - points[1]) * (param_5 + 1.0) * (1.0 - param_4)) / 2.0;
	const TeVector3f32 delta3  = ((points[3] - points[2]) * (1.0 - param_5) * (1.0 - param_4)) / 2.0;

	const TeVector3f32 x1 = delta1 + delta2a;
	const TeVector3f32 x2 = delta2b + delta3;

	const float t2 = t * t;
	const float t3 = t * t * t;
	const TeVector3f32 h1a = points[1] * ((t3 + t3) - t2 * 3.0 + 1.0);
	const TeVector3f32 h1b = x1 * ((t3 - (t2 + t2)) + t);
	const TeVector3f32 h1 = (h1a + h1b) + (x2 * (t3 - t2));
	return h1 + (points[2] * (t3 * -2.0 + t2 * 3.0));
}


} // end namespace Tetraedge
