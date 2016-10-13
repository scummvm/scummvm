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
}

Fog::~Fog() {
}

int Fog::readCommon(Common::ReadStream *stream) {
	int offset = stream->readUint32LE();
	stream->read(_name, 20);
	_fogColor.r = stream->readFloatLE();
	_fogColor.g = stream->readFloatLE();
	_fogColor.b = stream->readFloatLE();
	_fogDensity = stream->readFloatLE();
	return offset;
}

void Fog::readAnimationData(Common::ReadStream *stream, int size) {
	_animatedParameters = stream->readUint32LE();
	
	int floatsCount = size / 4;
	_animationData = new float[floatsCount];
	for (int i = 0; i < floatsCount; i++) {
		_animationData[i] = stream->readFloatLE();
	}

	_m11ptr = _animationData;
	_m12ptr = _m11ptr + (_animatedParameters & 0x1 ? _framesCount : 1);
	_m13ptr = _m12ptr + (_animatedParameters & 0x2 ? _framesCount : 1);
	_m14ptr = _m13ptr + (_animatedParameters & 0x4 ? _framesCount : 1);
	_m21ptr = _m14ptr + (_animatedParameters & 0x08 ? _framesCount : 1);
	_m22ptr = _m21ptr + (_animatedParameters & 0x10 ? _framesCount : 1);
	_m23ptr = _m22ptr + (_animatedParameters & 0x20 ? _framesCount : 1);
	_m24ptr = _m23ptr + (_animatedParameters & 0x40 ? _framesCount : 1);
	_m31ptr = _m24ptr + (_animatedParameters & 0x80 ? _framesCount : 1);
	_m32ptr = _m31ptr + (_animatedParameters & 0x100 ? _framesCount : 1);
	_m33ptr = _m32ptr + (_animatedParameters & 0x200 ? _framesCount : 1);
	_m34ptr = _m33ptr + (_animatedParameters & 0x400 ? _framesCount : 1);

	setupFrame(0);
}

void Fog::reset() {
}

void Fog::setupFrame(int frame) {
	int offset = frame % _framesCount;
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

void FogCone::read(Common::ReadStream *stream, int framesCount) {
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogCone::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	Vector3 positionT = this->_matrix * position;
	Vector3 viewPositionT = this->_matrix * viewPosition;

	Vector3 vectorT = (viewPositionT - positionT).normalize();

	float v67 = - positionT.x * vectorT.x - positionT.y * vectorT.y - positionT.z * vectorT.z;
	float v66 = - (positionT.z * positionT.z) - (positionT.y * positionT.y) - (positionT.x * positionT.x) + (v67 * v67) + (this->_parameter1 * this->_parameter1);

	if (v66 >= 0.0f) {
		float v24 = sqrt(v66);

		Vector3 v29 = positionT + (v67 - v24) * vectorT;
		Vector3 v36 = positionT + (v67 + v24) * vectorT;

		Vector3 v39 = this->_inverted * v29;
		Vector3 v42 = this->_inverted * v36;

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

void FogSphere::read(Common::ReadStream *stream, int framesCount) {
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogSphere::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	Vector3 positionT = this->_matrix * position;
	Vector3 viewPositionT = this->_matrix * viewPosition;

	Vector3 v158 = Vector3::cross(positionT, viewPositionT);

	if (v158.x != 0.0f || v158.y != 0.0f || v158.z != 0.0f) {
		Vector3 v167 = v158.normalize();
		if (v167.z < 0.0f) {
			v167 = -1.0f * v167;
		}

		float v173 = sqrt(1.0f - v167.z * v167.z);
		if (v173 > cos(this->_parameter1)) {
			Vector3 v37 = Vector3(v167.y, -v167.x, 0.0f).normalize();

			float v41 = 1.0f / v173 / v173 - 1.0f;
			float v42 = sqrt(v41);
			float v43 = tan(this->_parameter1);
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
				Vector3 v142 = this->_inverted * v139;

				Vector3 v148 = positionT + (v114 * vector);
				Vector3 v151 = this->_inverted * v148;

				*coeficient = (v151 - v142).length();
			}
		}
	}
}

void FogBox::read(Common::ReadStream *stream, int framesCount) {
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	_parameter2 = stream->readFloatLE();
	_parameter3 = stream->readFloatLE();
	readAnimationData(stream, size - 60);
}

void FogBox::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	Vector3 v159 = this->_matrix * position;
	Vector3 v146 = v159;
	Vector3 v156 = this->_matrix * viewPosition;
	Vector3 v153 = v156;
	Vector3 v150 = v156 - v159;

	float v149 = this->_parameter1 * 0.5f;
	if (v159.x < -v149) {
		if (v156.x < -v149) {
			return;
		}
		float v28 = (-v159.x - v149) / v150.x;
		Vector3 v29 = v28 * v150;
		v146 = v159 + v29;
	} else {
		if (v156.x < -v149) {
			float v19 = (-v156.x - v149) / v150.x;
			Vector3 v20 = v19 * v150;
			v153 = v156 + v20;
		}
	}
	if (v149 < v146.x) {
		if (v149 < v153.x) {
			return;
		}
		float v48 = (v149 - v146.x) / v150.x;
		Vector3 v49 = v48 * v150;
		v146 = v146 + v49;
	} else {
		if (v149 < v153.x) {
			float v40 = (v149 - v153.x) / v150.x;
			Vector3 v41 = v40 * v150;
			v153 = v153 + v41;
		}
	}
	float v162 = this->_parameter2 * 0.5f;
	if (v146.y < -v162) {
		if (v153.y < -v162) {
			return;
		}
		float v71 = (-v146.y - v162) / v150.y;
		Vector3 v72 = v71 * v150;
		v146 = v146 + v72;
	} else {
		if (v153.y < -v162) {
			float v62 = (-v153.y - v162) / v150.y;
			Vector3 v63 = v62 * v150;
			v153 = v153 + v63;
		}
	}
	if (v162 < v146.y) {
		if (v162 < v153.y) {
			return;
		}
		float v91 = (v162 - v146.y) / v150.y;
		Vector3 v92 = v91 * v150;
		v146 = v146 + v92;
	} else {
		if (v162 < v153.y) {
			float v83 = (v162 - v153.y) / v150.y;
			Vector3 v84 = v83 * v150;
			v153 = v153 + v84;
		}
	}

	if (0.0f <= v146.z) {
		if (0.0f > v153.z) {
			float v103 = -v153.z / v150.z;
			Vector3 v104 = v103 * v150;
			v153 = v153 + v104;
		}
	} else {
		if (0.0f > v153.z) {
			return;
		}
		float v111 = -v146.z / v150.z;
		Vector3 v112 = v111 * v150;
		v146 = v146 + v112;
	}

	if (v146.z <= this->_parameter3) {
		if (v153.z > this->_parameter3) {
			float v124 = (this->_parameter3 - v153.z) / v150.z;
			Vector3 v125 = v124 * v150;
			v153 = v153 + v125;
		}
	} else {
		if (v153.z <= this->_parameter3) {
			float v132 = (this->_parameter3 - v146.z) / v150.z;
			Vector3 v133 = v132 * v150;
			v146 = v146 + v133;
		}
	}

	Vector3 v137 = this->_inverted * v146;
	Vector3 v140 = this->_inverted * v153;
	Vector3 v143 = v140 - v137;

	*coeficient = v143.length();
}
} // End of namespace BladeRunner
