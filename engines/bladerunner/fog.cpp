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

#include "bladerunner/fog.h"

#include "common/stream.h"

namespace BladeRunner {

Fog::Fog() {
	_frameCount         = 0;
	_animatedParameters = 0;
	_fogDensity         = 0.0f;
	_animationData      = nullptr;
	_m11ptr             = nullptr;
	_m12ptr             = nullptr;
	_m13ptr             = nullptr;
	_m14ptr             = nullptr;
	_m21ptr             = nullptr;
	_m22ptr             = nullptr;
	_m23ptr             = nullptr;
	_m24ptr             = nullptr;
	_m31ptr             = nullptr;
	_m32ptr             = nullptr;
	_m33ptr             = nullptr;
	_m34ptr             = nullptr;
	_parameter1         = 0.0f;
	_parameter2         = 0.0f;
	_parameter3         = 0.0f;
	_next               = nullptr;
}

Fog::~Fog() {
}

int Fog::readCommon(Common::ReadStream *stream) {
	int offset = stream->readUint32LE();
	char buf[20];
	stream->read(buf, sizeof(buf));
	_name = buf;
	_fogColor.r = stream->readFloatLE();
	_fogColor.g = stream->readFloatLE();
	_fogColor.b = stream->readFloatLE();
	_fogDensity = stream->readFloatLE();
	return offset;
}

void Fog::readAnimationData(Common::ReadStream *stream, int size) {
	_animatedParameters = stream->readUint32LE();

	int floatCount = size / 4;
	_animationData = new float[floatCount];
	for (int i = 0; i < floatCount; i++) {
		_animationData[i] = stream->readFloatLE();
	}

	_m11ptr = _animationData;
	_m12ptr = _m11ptr + (_animatedParameters & 0x1 ? _frameCount : 1);
	_m13ptr = _m12ptr + (_animatedParameters & 0x2 ? _frameCount : 1);
	_m14ptr = _m13ptr + (_animatedParameters & 0x4 ? _frameCount : 1);
	_m21ptr = _m14ptr + (_animatedParameters & 0x08 ? _frameCount : 1);
	_m22ptr = _m21ptr + (_animatedParameters & 0x10 ? _frameCount : 1);
	_m23ptr = _m22ptr + (_animatedParameters & 0x20 ? _frameCount : 1);
	_m24ptr = _m23ptr + (_animatedParameters & 0x40 ? _frameCount : 1);
	_m31ptr = _m24ptr + (_animatedParameters & 0x80 ? _frameCount : 1);
	_m32ptr = _m31ptr + (_animatedParameters & 0x100 ? _frameCount : 1);
	_m33ptr = _m32ptr + (_animatedParameters & 0x200 ? _frameCount : 1);
	_m34ptr = _m33ptr + (_animatedParameters & 0x400 ? _frameCount : 1);

	setupFrame(0);
}

void Fog::reset() {
}

void Fog::setupFrame(int frame) {
	int offset = frame % _frameCount;
	_matrix._m[0][0] = (_animatedParameters & 0x1 ? _m11ptr[offset] : *_m11ptr);
	_matrix._m[0][1] = (_animatedParameters & 0x2 ? _m12ptr[offset] : *_m12ptr);
	_matrix._m[0][2] = (_animatedParameters & 0x4 ? _m13ptr[offset] : *_m13ptr);
	_matrix._m[0][3] = (_animatedParameters & 0x8 ? _m14ptr[offset] : *_m14ptr);
	_matrix._m[1][0] = (_animatedParameters & 0x10 ? _m21ptr[offset] : *_m21ptr);
	_matrix._m[1][1] = (_animatedParameters & 0x20 ? _m22ptr[offset] : *_m22ptr);
	_matrix._m[1][2] = (_animatedParameters & 0x40 ? _m23ptr[offset] : *_m23ptr);
	_matrix._m[1][3] = (_animatedParameters & 0x80 ? _m24ptr[offset] : *_m24ptr);
	_matrix._m[2][0] = (_animatedParameters & 0x100 ? _m31ptr[offset] : *_m31ptr);
	_matrix._m[2][1] = (_animatedParameters & 0x200 ? _m32ptr[offset] : *_m32ptr);
	_matrix._m[2][2] = (_animatedParameters & 0x400 ? _m33ptr[offset] : *_m33ptr);
	_matrix._m[2][3] = (_animatedParameters & 0x800 ? _m34ptr[offset] : *_m34ptr);
	_inverted = invertMatrix(_matrix);
}

void FogCone::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogCone::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	Vector3 positionT = _matrix * position;
	Vector3 viewPositionT = _matrix * viewPosition;

	Vector3 vectorT = (viewPositionT - positionT).normalize();

	float v67 = - positionT.x * vectorT.x - positionT.y * vectorT.y - positionT.z * vectorT.z;
	float v66 = - (positionT.z * positionT.z) - (positionT.y * positionT.y) - (positionT.x * positionT.x) + (v67 * v67) + (_parameter1 * _parameter1);

	if (v66 >= 0.0f) {
		float v24 = sqrt(v66);

		Vector3 v29 = positionT + (v67 - v24) * vectorT;
		Vector3 v36 = positionT + (v67 + v24) * vectorT;

		Vector3 v39 = _inverted * v29;
		Vector3 v42 = _inverted * v36;

		float v74 = (v39 - position).length();
		float v76 = (v42 - position).length();

		Vector3 vector = viewPosition - position;

		float vectorLength = vector.length();

		if (v74 < 0.0f) {
			v74 = 0.0f;
		}
		if (v76 > vectorLength) {
			v76 = vectorLength;
		}
		if (v76 >= v74) {
			*coeficient = v76 - v74;
		}
	}
}

void FogSphere::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogSphere::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	Vector3 positionT = _matrix * position;
	Vector3 viewPositionT = _matrix * viewPosition;

	Vector3 v158 = Vector3::cross(positionT, viewPositionT);

	if (v158.x != 0.0f || v158.y != 0.0f || v158.z != 0.0f) {
		Vector3 v167 = v158.normalize();
		if (v167.z < 0.0f) {
			v167 = -1.0f * v167;
		}

		float v173 = sqrt(1.0f - v167.z * v167.z);
		if (v173 > cos(_parameter1)) {
			Vector3 v37 = Vector3(v167.y, -v167.x, 0.0f).normalize();

			float v41 = 1.0f / v173 / v173 - 1.0f;
			float v42 = sqrt(v41);
			float v43 = tan(_parameter1);
			float v44 = sqrt(v43 * v43 - v41);

			Vector3 v45 = v44 * v37;

			Vector3 v48 = Vector3(
				-v37.y * v42,
				v37.x * v42,
				0.0f * v42);

			Vector3 v51 = v48 + Vector3(0.0f, 0.0f, -1.0f);

			Vector3 v186 = v51 - v45;
			Vector3 v183 = v51 + v45;

			Vector3 vector = viewPositionT - positionT;

			Vector3 v177 = -1.0f * positionT;
			Vector3 v174 = Vector3::cross(v186, vector);

			float v189, v191;
			if (fabs(v174.x) <= fabs(v174.y)) {
				if (fabs(v174.y) <= fabs(v174.z)) {
					v191 = v177.x * v186.y - v177.y * v186.x;
					v189 = v186.y * vector.x - v186.x * vector.y;
				} else {
					v191 = v177.z * v186.x - v186.z * v177.x;
					v189 = v186.x * vector.z - v186.z * vector.x;
				}
			} else {
				if (fabs(v174.x) <= fabs(v174.z)) {
					v191 = v177.x * v186.y - v177.y * v186.x;
					v189 = v186.y * vector.x - v186.x * vector.y;
				} else {
					v191 = v186.z * v177.y - v186.y * v177.z;
					v189 = v186.z * vector.y - v186.y * vector.z;
				}
			}

			float v88;
			if (v189 == 0.0f) {
				v88 = 0.0f;
			} else {
				v88 = v191 / v189;
			}

			Vector3 v196 = -1.0f * positionT;
			Vector3 v193 = Vector3::cross(v183, vector);

			float v190, v192;
			if (fabs(v193.x) <= fabs(v193.y)) {
				if (fabs(v193.y) <= fabs(v193.z)) {
					v192 = v196.x * v183.y - v196.y * v183.x;
					v190 = v183.y * vector.x - v183.x * vector.y;
				} else {
					v192 = v196.z * v183.x - v183.z * v196.x;
					v190 = v183.x * vector.z - v183.z * vector.x;
				}
			} else {
				if (fabs(v193.x) <= fabs(v193.z)) {
					v192 = v196.x * v183.y - v196.y * v183.x;
					v190 = v183.y * vector.x - v183.x * vector.y;
				} else {
					v192 = v183.z * v196.y - v183.y * v196.z;
					v190 = v183.z * vector.y - v183.y * vector.z;
				}
			}

			float v114;
			if (v190 == 0.0f) {
				v114 = 0.0f;
			} else {
				v114 = v192 / v190;
			}

			if (v114 < v88) {
				float temp = v88;
				v88 = v114;
				v114 = temp;
			}

			if (v88 <= 1.0f && v114 >= 0.0f) {
				if (v88 < 0.0f) {
					v88 = 0.0;
				}
				if (v114 > 1.0f) {
					v114 = 1.0;
				}

				Vector3 v139 = positionT + (v88 * vector);
				Vector3 v142 = _inverted * v139;

				Vector3 v148 = positionT + (v114 * vector);
				Vector3 v151 = _inverted * v148;

				*coeficient = (v151 - v142).length();
			}
		}
	}
}

void FogBox::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	_parameter2 = stream->readFloatLE();
	_parameter3 = stream->readFloatLE();
	readAnimationData(stream, size - 60);
}

void FogBox::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	Vector3 positionT = _matrix * position;
	Vector3 viewPositionT = _matrix * viewPosition;

	Vector3 positionTadj = positionT;
	Vector3 viewPositionTadj = viewPositionT;

	Vector3 direction = viewPositionT - positionT;

	float parameter1half = _parameter1 * 0.5f;
	if (positionT.x < -parameter1half) {
		if (viewPositionT.x < -parameter1half) {
			*coeficient = 0.0f;
			return;
		}
		float v28 = (-positionT.x - parameter1half) / direction.x;
		Vector3 v29 = v28 * direction;
		positionTadj = positionT + v29;
	} else if (viewPositionT.x < -parameter1half) {
		float v19 = (-viewPositionT.x - parameter1half) / direction.x;
		Vector3 v20 = v19 * direction;
		viewPositionTadj = viewPositionT + v20;
	}

	if (parameter1half < positionTadj.x) {
		if (parameter1half < viewPositionTadj.x) {
			*coeficient = 0.0f;
			return;
		}
		float v48 = (parameter1half - positionTadj.x) / direction.x;
		Vector3 v49 = v48 * direction;
		positionTadj = positionTadj + v49;
	} else if (parameter1half < viewPositionTadj.x) {
		float v40 = (parameter1half - viewPositionTadj.x) / direction.x;
		Vector3 v41 = v40 * direction;
		viewPositionTadj = viewPositionTadj + v41;
	}

	float parameter2half = _parameter2 * 0.5f;
	if (positionTadj.y < -parameter2half) {
		if (viewPositionTadj.y < -parameter2half) {
			*coeficient = 0.0f;
			return;
		}
		float v71 = (-positionTadj.y - parameter2half) / direction.y;
		Vector3 v72 = v71 * direction;
		positionTadj = positionTadj + v72;
	} else if (viewPositionTadj.y < -parameter2half) {
		float v62 = (-viewPositionTadj.y - parameter2half) / direction.y;
		Vector3 v63 = v62 * direction;
		viewPositionTadj = viewPositionTadj + v63;
	}

	if (parameter2half < positionTadj.y) {
		if (parameter2half < viewPositionTadj.y) {
			*coeficient = 0.0f;
			return;
		}
		float v91 = (parameter2half - positionTadj.y) / direction.y;
		Vector3 v92 = v91 * direction;
		positionTadj = positionTadj + v92;
	} else if (parameter2half < viewPositionTadj.y) {
		float v83 = (parameter2half - viewPositionTadj.y) / direction.y;
		Vector3 v84 = v83 * direction;
		viewPositionTadj = viewPositionTadj + v84;
	}

	if (0.0f > positionTadj.z) {
		if (0.0f > viewPositionTadj.z) {
			*coeficient = 0.0f;
			return;
		}
		float v111 = -positionTadj.z / direction.z;
		Vector3 v112 = v111 * direction;
		positionTadj = positionTadj + v112;
	} else if (0.0f > viewPositionTadj.z) {
		float v103 = -viewPositionTadj.z / direction.z;
		Vector3 v104 = v103 * direction;
		viewPositionTadj = viewPositionTadj + v104;
	}

	if (positionTadj.z > _parameter3) {
		if (viewPositionTadj.z > _parameter3) {
			*coeficient = 0.0f;
			return;
		}
		float v132 = (_parameter3 - positionTadj.z) / direction.z;
		Vector3 v133 = v132 * direction;
		positionTadj = positionTadj + v133;
	} else if (viewPositionTadj.z > _parameter3) {
		float v124 = (_parameter3 - viewPositionTadj.z) / direction.z;
		Vector3 v125 = v124 * direction;
		viewPositionTadj = viewPositionTadj + v125;
	}

	Vector3 v137 = _inverted * positionTadj;
	Vector3 v140 = _inverted * viewPositionTadj;
	Vector3 v143 = v140 - v137;

	*coeficient = v143.length();
}
} // End of namespace BladeRunner
