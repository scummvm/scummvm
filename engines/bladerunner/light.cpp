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

#include "bladerunner/light.h"

#include "common/util.h"

namespace BladeRunner {

Light::Light() {
	_frameCount         = 0;
	_animated           = 0;
	_animationData      = nullptr;
	_animatedParameters = 0;
	_falloffStart       = 0.0f;
	_falloffEnd         = 0.0f;
	_angleStart         = 0.0f;
	_angleEnd           = 0.0f;
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
	_colorRPtr          = nullptr;
	_colorGPtr          = nullptr;
	_colorBPtr          = nullptr;
	_falloffStartPtr    = nullptr;
	_falloffEndPtr      = nullptr;
	_angleStartPtr      = nullptr;
	_angleEndPtr        = nullptr;
}

Light::~Light() {
	if (_animationData != nullptr) {
		delete[] _animationData;
	}
}

void Light::read(Common::ReadStream *stream, int frameCount, int frame, int animated) {
	_frameCount = frameCount;
	_animated = animated;

	int size = stream->readUint32LE();
	size = size - 32;

	char buf[20];
	stream->read(buf, sizeof(buf));
	_name = buf;

	_animatedParameters = stream->readUint32LE();

	if (_animationData != nullptr) {
		delete[] _animationData;
	}
	int floatCount = size / 4;
	_animationData = new float[floatCount];
	for (int i = 0; i < floatCount; ++i) {
		_animationData[i] = stream->readFloatLE();
	}

	_m11ptr          = _animationData;
	_m12ptr          = _m11ptr          + ((_animatedParameters &     0x1) ? frameCount : 1);
	_m13ptr          = _m12ptr          + ((_animatedParameters &     0x2) ? frameCount : 1);
	_m14ptr          = _m13ptr          + ((_animatedParameters &     0x4) ? frameCount : 1);
	_m21ptr          = _m14ptr          + ((_animatedParameters &     0x8) ? frameCount : 1);
	_m22ptr          = _m21ptr          + ((_animatedParameters &    0x10) ? frameCount : 1);
	_m23ptr          = _m22ptr          + ((_animatedParameters &    0x20) ? frameCount : 1);
	_m24ptr          = _m23ptr          + ((_animatedParameters &    0x40) ? frameCount : 1);
	_m31ptr          = _m24ptr          + ((_animatedParameters &    0x80) ? frameCount : 1);
	_m32ptr          = _m31ptr          + ((_animatedParameters &   0x100) ? frameCount : 1);
	_m33ptr          = _m32ptr          + ((_animatedParameters &   0x200) ? frameCount : 1);
	_m34ptr          = _m33ptr          + ((_animatedParameters &   0x400) ? frameCount : 1);
	_colorRPtr       = _m34ptr          + ((_animatedParameters &   0x800) ? frameCount : 1);
	_colorGPtr       = _colorRPtr       + ((_animatedParameters &  0x1000) ? frameCount : 1);
	_colorBPtr       = _colorGPtr       + ((_animatedParameters &  0x2000) ? frameCount : 1);
	_falloffStartPtr = _colorBPtr       + ((_animatedParameters &  0x4000) ? frameCount : 1);
	_falloffEndPtr   = _falloffStartPtr + ((_animatedParameters &  0x8000) ? frameCount : 1);
	_angleStartPtr   = _falloffEndPtr   + ((_animatedParameters & 0x10000) ? frameCount : 1);
	_angleEndPtr     = _angleStartPtr   + ((_animatedParameters & 0x20000) ? frameCount : 1);

	setupFrame(frame);
}

void Light::readVqa(Common::ReadStream *stream, int frameCount, int frame, int animated) {
	_frameCount = frameCount;
	_animated = animated;

	_animatedParameters = stream->readUint32LE();

	int size = stream->readUint32LE();

	if (_animationData != nullptr) {
		delete[] _animationData;
	}

	int floatCount = size / 4;
	_animationData = new float[floatCount];
	for (int i = 0; i < floatCount; ++i) {
		_animationData[i] = stream->readFloatLE();
	}

	_m11ptr          = _animationData;
	_m12ptr          = _m11ptr          + ((_animatedParameters &     0x1) ? frameCount : 1);
	_m13ptr          = _m12ptr          + ((_animatedParameters &     0x2) ? frameCount : 1);
	_m14ptr          = _m13ptr          + ((_animatedParameters &     0x4) ? frameCount : 1);
	_m21ptr          = _m14ptr          + ((_animatedParameters &     0x8) ? frameCount : 1);
	_m22ptr          = _m21ptr          + ((_animatedParameters &    0x10) ? frameCount : 1);
	_m23ptr          = _m22ptr          + ((_animatedParameters &    0x20) ? frameCount : 1);
	_m24ptr          = _m23ptr          + ((_animatedParameters &    0x40) ? frameCount : 1);
	_m31ptr          = _m24ptr          + ((_animatedParameters &    0x80) ? frameCount : 1);
	_m32ptr          = _m31ptr          + ((_animatedParameters &   0x100) ? frameCount : 1);
	_m33ptr          = _m32ptr          + ((_animatedParameters &   0x200) ? frameCount : 1);
	_m34ptr          = _m33ptr          + ((_animatedParameters &   0x400) ? frameCount : 1);
	_colorRPtr       = _m34ptr          + ((_animatedParameters &   0x800) ? frameCount : 1);
	_colorGPtr       = _colorRPtr       + ((_animatedParameters &  0x1000) ? frameCount : 1);
	_colorBPtr       = _colorGPtr       + ((_animatedParameters &  0x2000) ? frameCount : 1);
	_falloffStartPtr = _colorBPtr       + ((_animatedParameters &  0x4000) ? frameCount : 1);
	_falloffEndPtr   = _falloffStartPtr + ((_animatedParameters &  0x8000) ? frameCount : 1);
	_angleStartPtr   = _falloffEndPtr   + ((_animatedParameters & 0x10000) ? frameCount : 1);
	_angleEndPtr     = _angleStartPtr   + ((_animatedParameters & 0x20000) ? frameCount : 1);

	setupFrame(frame);
}

void Light::setupFrame(int frame) {
	int offset = frame % _frameCount;
	_matrix._m[0][0] = ((_animatedParameters &     0x1) ? _m11ptr[offset]          : *_m11ptr);
	_matrix._m[0][1] = ((_animatedParameters &     0x2) ? _m12ptr[offset]          : *_m12ptr);
	_matrix._m[0][2] = ((_animatedParameters &     0x4) ? _m13ptr[offset]          : *_m13ptr);
	_matrix._m[0][3] = ((_animatedParameters &     0x8) ? _m14ptr[offset]          : *_m14ptr);
	_matrix._m[1][0] = ((_animatedParameters &    0x10) ? _m21ptr[offset]          : *_m21ptr);
	_matrix._m[1][1] = ((_animatedParameters &    0x20) ? _m22ptr[offset]          : *_m22ptr);
	_matrix._m[1][2] = ((_animatedParameters &    0x40) ? _m23ptr[offset]          : *_m23ptr);
	_matrix._m[1][3] = ((_animatedParameters &    0x80) ? _m24ptr[offset]          : *_m24ptr);
	_matrix._m[2][0] = ((_animatedParameters &   0x100) ? _m31ptr[offset]          : *_m31ptr);
	_matrix._m[2][1] = ((_animatedParameters &   0x200) ? _m32ptr[offset]          : *_m32ptr);
	_matrix._m[2][2] = ((_animatedParameters &   0x400) ? _m33ptr[offset]          : *_m33ptr);
	_matrix._m[2][3] = ((_animatedParameters &   0x800) ? _m34ptr[offset]          : *_m34ptr);
	_color.r         = ((_animatedParameters &  0x1000) ? _colorRPtr[offset]       : *_colorRPtr);
	_color.g         = ((_animatedParameters &  0x2000) ? _colorGPtr[offset]       : *_colorGPtr);
	_color.b         = ((_animatedParameters &  0x4000) ? _colorBPtr[offset]       : *_colorBPtr);
	_falloffStart    = ((_animatedParameters &  0x8000) ? _falloffStartPtr[offset] : *_falloffStartPtr);
	_falloffEnd      = ((_animatedParameters & 0x10000) ? _falloffEndPtr[offset]   : *_falloffEndPtr);
	_angleStart      = ((_animatedParameters & 0x20000) ? _angleStartPtr[offset]   : *_angleStartPtr);
	_angleEnd        = ((_animatedParameters & 0x40000) ? _angleEndPtr[offset]     : *_angleEndPtr);
}

float Light::calculate(Vector3 start, Vector3 end) const {
	return calculateFalloutCoefficient(_matrix * start, _matrix * end, _falloffStart, _falloffEnd);
}

void Light::calculateColor(Color *outColor, Vector3 position) const {
	Vector3 positionT = _matrix * position;
	float att = attenuation(_falloffStart, _falloffEnd, positionT.length());
	outColor->r = _color.r * att;
	outColor->g = _color.g * att;
	outColor->b = _color.b * att;
}

float Light::calculateFalloutCoefficient(Vector3 start, Vector3 end, float falloffStart, float falloffEnd) const {
	if (falloffEnd == 0.0f) {
		return 1.0e30f;
	}

	if (falloffStart * falloffStart >= start.length() && falloffStart * falloffStart >= end.length()) {
		return 1.0e30f;
	}

	float diff = (end - start).length();
	float v31 = 0.0f;
	if (diff != 0.0f) {
		Vector3 v27 = Vector3::cross(start, (end - start));
		v31 = v27.length() / diff;
	}

	if (v31 < falloffEnd) {
		return 1.0f / (1.0f - (v31 / falloffEnd));
	}
	return 1.0e30f;
}

float Light::attenuation(float min, float max, float distance) const {
	if (max == 0.0f) {
		return 1.0f;
	}
	if (min < max) {
		distance = CLIP(distance, min, max);
		float x = (max - distance) / (max - min);
		return x * x * (3.0f - 2.0f * x);
	}
	if (distance < min) {
		return 1.0f;
	}
	return 0.0f;
}

float Light1::calculate(Vector3 start, Vector3 end) const {
	start = _matrix * start;
	end = _matrix * end;

	float v40 = 0.0f;
	if (_falloffEnd != 0.0f) {
		v40 = calculateFalloutCoefficient(start, end, _falloffStart, _falloffEnd);
	}

	float v41 = atan2(sqrt(start.x * start.x + start.y * start.y), -start.z);
	float v42 = atan2(sqrt(end.x * end.x + end.y * end.y), -end.z);

	float v43;
	if ((_angleStart >= v41 && _angleStart >= v42) || (_angleEnd <= v41 && _angleEnd <= v42)) {
		v43 = 1.0e30f;
	} else {
		v43 = 2.0;
	}
	if (v43 < v40) {
		return v40;
	} else {
		return v43;
	}
}

void Light1::calculateColor(Color *outColor, Vector3 position) const {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v12 = attenuation(_angleStart, _angleEnd, atan2(sqrt(positionT.x * positionT.x + positionT.y * positionT.y), -positionT.z));
		float v13 = attenuation(_falloffStart, _falloffEnd, positionT.length());

		outColor->r = v12 * v13 * _color.r;
		outColor->g = v12 * v13 * _color.g;
		outColor->b = v12 * v13 * _color.b;
	}
}

float Light2::calculate(Vector3 start, Vector3 end) const {
	start = _matrix * start;
	end = _matrix * end;

	float v54 = 0.0f;
	if (_falloffEnd != 0.0f) {
		v54 = calculateFalloutCoefficient(start, end, _falloffStart, _falloffEnd);
	}

	float v55 = atan2(fabs(start.x), -start.z);
	float v58 = atan2(fabs(start.y), -start.z);
	float v57 = atan2(fabs(end.x), -end.z);
	float v56 = atan2(fabs(end.y), -end.z);

	float v59;
	if ((_angleStart >= v55 && _angleStart >= v57 && _angleStart >= v58 && _angleStart >= v56) || (_angleEnd <= v55 && _angleEnd <= v57 && _angleEnd <= v58 && _angleEnd <= v56)) {
		v59 = 1.0e30f;
	} else {
		v59 = 2.0f;
	}
	if (v59 < v54) {
		return v54;
	} else {
		return v59;
	}
}

void Light2::calculateColor(Color *outColor, Vector3 position) const {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v11 = attenuation(_angleStart, _angleEnd, atan2(fabs(positionT.y), -positionT.z));
		float v12 = attenuation(_angleStart, _angleEnd, atan2(fabs(positionT.x), -positionT.z));
		float v13 = attenuation(_falloffStart, _falloffEnd, positionT.length());

		outColor->r = v11 * v12 * v13 * _color.r;
		outColor->g = v11 * v12 * v13 * _color.g;
		outColor->b = v11 * v12 * v13 * _color.b;
	}
}

void Light3::calculateColor(Color *outColor, Vector3 position) const {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v12 = attenuation(_angleStart, _angleEnd, sqrt(positionT.x * positionT.x + positionT.y * positionT.y));
		float v13 = attenuation(_falloffStart, _falloffEnd, positionT.length());

		outColor->r = v12 * v13 * _color.r;
		outColor->g = v12 * v13 * _color.g;
		outColor->b = v12 * v13 * _color.b;
	}
}

void Light4::calculateColor(Color *outColor, Vector3 position) const {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v11 = attenuation(_angleStart, _angleEnd, fabs(positionT.y));
		float v12 = attenuation(_angleStart, _angleEnd, fabs(positionT.x));
		float v13 = attenuation(_falloffStart, _falloffEnd, positionT.length());

		outColor->r = v11 * v12 * v13 * _color.r;
		outColor->g = v11 * v12 * v13 * _color.g;
		outColor->b = v11 * v12 * v13 * _color.b;
	}
}

float LightAmbient::calculate(Vector3 start, Vector3 end) const {
	return 1.0e30f;
}

void LightAmbient::calculateColor(Color *outColor, Vector3 position) const {
	outColor->r = _color.r;
	outColor->g = _color.g;
	outColor->b = _color.b;
}

} // End of namespace BladeRunner
