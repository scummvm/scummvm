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

#include "tetraedge/te/te_model_vertex_animation.h"
#include "common/math.h"

namespace Tetraedge {

TeModelVertexAnimation::TeModelVertexAnimation() : _lastMillis(0.0f), _modelAnim(nullptr) {
	_rot.fromAxisAndAngle(TeVector3f32(0.0f, 1.0f, 0.0f), (float)-M_PI_2);
}

void TeModelVertexAnimation::bind(TeIntrusivePtr<TeModel> &model) {
	_model = model;
	_lastMillis = 0.0f;
}

void TeModelVertexAnimation::destroy() {
	_keydata.clear();
}

TeVector3f32 TeModelVertexAnimation::getKeyVertex(uint keyno, uint vertexno) {
	assert(keyno < _keydata.size());
	const KeyData &data = _keydata[keyno];
	assert(vertexno < data._vectors.size());
	TeVector3f32 retval = data._vectors[vertexno];
	if (!data._matricies.empty()) {
		retval = data._matricies[vertexno] * retval;
		retval.rotate(_rot);
	}
	return retval;
}

Common::Array<TeVector3f32> TeModelVertexAnimation::getVertices() {
	Common::Array<TeVector3f32> lerpVtx;

	if (_keydata.size() < 2)
		return lerpVtx;

	float frame = fmod((_lastMillis / 1000.0) * 30, _keydata[_keydata.size() - 1]._frame);
	uint keyno = 0;
	while (keyno < _keydata.size() - 1 && _keydata[keyno]._frame < frame)
		keyno++;

	lerpVtx.resize(_keydata[0]._vectors.size());
	float prevFrame = _keydata[keyno]._frame;
	float nextFrame = _keydata[keyno + 1]._frame;
	float interp = (frame - nextFrame) / (nextFrame - prevFrame);

	for (uint i = 0; i < _keydata[0]._vectors.size(); i++) {
		const TeVector3f32 prevVector = getKeyVertex(keyno, i);
		const TeVector3f32 nextVector = getKeyVertex(keyno + 1, i);
		lerpVtx[i] = prevVector * (1.0 - interp) + nextVector * interp;
	}

	return lerpVtx;
}

bool TeModelVertexAnimation::load(Common::ReadStream &stream) {
	error("TODO: implement TeModelVertexAnimation::load");
}

void TeModelVertexAnimation::save(Common::WriteStream &stream) const {
	error("TODO: implement TeModelVertexAnimation::save");
}

void TeModelVertexAnimation::update(double millis) {
	if (_keydata.empty())
		return;

	double lastMillis = _lastMillis;
	double lastFrame = fmod((lastMillis / 1000.0) * 30.0, _keydata.back()._frame);

	if (_modelAnim) {
		int frame = _modelAnim->calcCurrentFrame(millis);
		millis = (frame * 1000.0) / 30.0;
	}
	_lastMillis = millis;
	double thisFrame = fmod((millis / 1000.0) * 30.0, _keydata.back()._frame);
	if (lastFrame > thisFrame)
		_onFinishedSignal.call();
}

} // end namespace Tetraedge
