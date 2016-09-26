#include "bladerunner/light.h"
#include "common/util.h"

namespace BladeRunner {

Light::Light() {
}

Light::~Light() {
}

void Light::read(Common::ReadStream *stream, int framesCount, int frame, int animated) {
	_framesCount = framesCount;
	_animated = animated;

	int size = stream->readUint32LE();
	size = size - 32;

	stream->read(_name, 20);

	_animatedParameters = stream->readUint32LE();

	_animationData = new float[size / sizeof(float)];
	stream->read(_animationData, size);

	_m11ptr = _animationData;
	_m12ptr = _m11ptr + (_animatedParameters & 0x1 ? framesCount : 1);
	_m13ptr = _m12ptr + (_animatedParameters & 0x2 ? framesCount : 1);
	_m14ptr = _m13ptr + (_animatedParameters & 0x4 ? framesCount : 1);
	_m21ptr = _m14ptr + (_animatedParameters & 0x8 ? framesCount : 1);
	_m22ptr = _m21ptr + (_animatedParameters & 0x10 ? framesCount : 1);
	_m23ptr = _m22ptr + (_animatedParameters & 0x20 ? framesCount : 1);
	_m24ptr = _m23ptr + (_animatedParameters & 0x40 ? framesCount : 1);
	_m31ptr = _m24ptr + (_animatedParameters & 0x80 ? framesCount : 1);
	_m32ptr = _m31ptr + (_animatedParameters & 0x100 ? framesCount : 1);
	_m33ptr = _m32ptr + (_animatedParameters & 0x200 ? framesCount : 1);
	_m34ptr = _m33ptr + (_animatedParameters & 0x400 ? framesCount : 1);
	_colorRPtr = _m34ptr + (_animatedParameters & 0x800 ? framesCount : 1);
	_colorGPtr = _colorRPtr + (_animatedParameters & 0x1000 ? framesCount : 1);
	_colorBPtr = _colorGPtr + (_animatedParameters & 0x2000 ? framesCount : 1);
	_field16ptr = _colorGPtr + (_animatedParameters & 0x4000 ? framesCount : 1);
	_field17ptr = _field16ptr + (_animatedParameters & 0x8000 ? framesCount : 1);
	_field18ptr = _field17ptr + (_animatedParameters & 0x10000 ? framesCount : 1);
	_field19ptr = _field18ptr + (_animatedParameters & 0x20000 ? framesCount : 1);

	setupFrame(frame);
}

void Light::readVqa(Common::ReadStream *stream, int framesCount, int frame, int animated) {
	_framesCount = framesCount;
	_animated = animated;

	_animatedParameters = stream->readUint32LE();

	int size = stream->readUint32LE();

	_animationData = new float[size / sizeof(float)];
	stream->read(_animationData, size);

	_m11ptr = _animationData;
	_m12ptr = _m11ptr + (_animatedParameters & 0x1 ? framesCount : 1);
	_m13ptr = _m12ptr + (_animatedParameters & 0x2 ? framesCount : 1);
	_m14ptr = _m13ptr + (_animatedParameters & 0x4 ? framesCount : 1);
	_m21ptr = _m14ptr + (_animatedParameters & 0x8 ? framesCount : 1);
	_m22ptr = _m21ptr + (_animatedParameters & 0x10 ? framesCount : 1);
	_m23ptr = _m22ptr + (_animatedParameters & 0x20 ? framesCount : 1);
	_m24ptr = _m23ptr + (_animatedParameters & 0x40 ? framesCount : 1);
	_m31ptr = _m24ptr + (_animatedParameters & 0x80 ? framesCount : 1);
	_m32ptr = _m31ptr + (_animatedParameters & 0x100 ? framesCount : 1);
	_m33ptr = _m32ptr + (_animatedParameters & 0x200 ? framesCount : 1);
	_m34ptr = _m33ptr + (_animatedParameters & 0x400 ? framesCount : 1);
	_colorRPtr = _m34ptr + (_animatedParameters & 0x800 ? framesCount : 1);
	_colorGPtr = _colorRPtr + (_animatedParameters & 0x1000 ? framesCount : 1);
	_colorBPtr = _colorGPtr + (_animatedParameters & 0x2000 ? framesCount : 1);
	_field16ptr = _colorGPtr + (_animatedParameters & 0x4000 ? framesCount : 1);
	_field17ptr = _field16ptr + (_animatedParameters & 0x8000 ? framesCount : 1);
	_field18ptr = _field17ptr + (_animatedParameters & 0x10000 ? framesCount : 1);
	_field19ptr = _field18ptr + (_animatedParameters & 0x20000 ? framesCount : 1);

	setupFrame(frame);
}

void Light::setupFrame(int frame) {
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
	_color.r = (_animatedParameters & 0x1000 ? _colorRPtr[offset] : *_colorRPtr);
	_color.g = (_animatedParameters & 0x2000 ? _colorGPtr[offset] : *_colorGPtr);
	_color.b = (_animatedParameters & 0x4000 ? _colorBPtr[offset] : *_colorBPtr);
	_field16 = (_animatedParameters & 0x8000 ? _field16ptr[offset] : *_field16ptr);
	_field17 = (_animatedParameters & 0x10000 ? _field17ptr[offset] : *_field17ptr);
	_field18 = (_animatedParameters & 0x20000 ? _field18ptr[offset] : *_field18ptr);
	_field19 = (_animatedParameters & 0x40000 ? _field19ptr[offset] : *_field19ptr);

}

float Light::calculate(Vector3 start, Vector3 end) {
	return calculateCoeficient(_matrix * start, _matrix * end, _field16, _field17);
}

void Light::calculateColor(Color *outColor, Vector3 position) {
	Vector3 positionT = _matrix * position;
	float att = attenuation(_field16, _field17, positionT.length());
	outColor->r = _color.r * att;
	outColor->g = _color.g * att;
	outColor->b = _color.b * att;
}

float Light::calculateCoeficient(Vector3 start, Vector3 end, float a3, float a4) {
	if (a4 == 0.0f) {
		return 1.0e30f;
	}

	if (a3 >= start.length() && a3 >= end.length()) {
		return 1.0e30f;
	}

	float v40 = (end - start).length();
	float v31 = 0.0f;
	if (v40 != 0.0f) {
		Vector3 v27 = Vector3::cross(start, end);
		v31 = v27.length() / v40;
	}

	if (v31 < a4) {
		return 1.0f / (1.0f - (v31 / a4));
	}
	return 1.0e30f;
}

float Light::attenuation(float min, float max, float distance) {
	if (max == 0.0f) {
		return 1.0f;
	}
	if (min < max) {
		distance = CLIP(distance, min, max);
		float x = (max - distance) / (max - min);
		return x * x * (3.0f - 2.0f * x);
	}
	if (min > distance) {
		return 1.0f;
	}
	return 0.0f;
}

float Light1::calculate(Vector3 start, Vector3 end) {
	start = _matrix * start;
	end = _matrix * end;

	float v40 = 0.0f;
	if (_field17 != 0.0f) {
		v40 = calculateCoeficient(start, end, _field16, _field17);
	}

	float v41 = atan2(start.length(), -start.z);
	float v42 = atan2(end.length(), -end.z);

	float v43;
	if ((_field18 >= v41 && _field18 >= v42) || (_field19 <= v41 && _field19 <= v42)) {
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

void Light1::calculateColor(Color *outColor, Vector3 position) {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v12 = attenuation(_field18, _field19, atan2(sqrt(positionT.x * positionT.x + positionT.y * positionT.y), -positionT.z));
		float v13 = attenuation(_field16, _field17, positionT.length());

		outColor->r = v12 * v13 * _color.r;
		outColor->g = v12 * v13 * _color.g;
		outColor->b = v12 * v13 * _color.b;
	}
}

float Light2::calculate(Vector3 start, Vector3 end) {
	start = _matrix * start;
	end = _matrix * end;

	float v54 = 0.0f;
	if (_field17 != 0.0f) {
		v54 = calculateCoeficient(start, end, _field16, _field17);
	}

	float v55 = atan2(fabs(start.x), -start.z);
	float v58 = atan2(fabs(start.y), -start.z);
	float v57 = atan2(fabs(end.x), -end.z);
	float v56 = atan2(fabs(end.y), -end.z);

	float v59;
	if ((_field18 >= v55 && _field18 >= v57 && _field18 >= v58 && _field18 >= v56) || (_field19 <= v55 && _field19 <= v57 && _field19 <= v58 && _field19 <= v56)) {
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

void Light2::calculateColor(Color *outColor, Vector3 position) {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v11 = attenuation(_field18, _field19, atan2(fabs(positionT.y), -positionT.z));
		float v12 = attenuation(_field18, _field19, atan2(fabs(positionT.x), -positionT.z));
		float v13 = attenuation(_field16, _field17, positionT.length());

		outColor->r = v11 * v12 * v13 * _color.r;
		outColor->g = v11 * v12 * v13 * _color.g;
		outColor->b = v11 * v12 * v13 * _color.b;
	}
}

void Light3::calculateColor(Color *outColor, Vector3 position) {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v12 = attenuation(_field18, _field19, sqrt(positionT.x * positionT.x + positionT.y * positionT.y));
		float v13 = attenuation(_field16, _field17, positionT.length());

		outColor->r = v12 * v13 * _color.r;
		outColor->g = v12 * v13 * _color.g;
		outColor->b = v12 * v13 * _color.b;
	}
}

void Light4::calculateColor(Color *outColor, Vector3 position) {
	Vector3 positionT = _matrix * position;

	outColor->r = 0.0f;
	outColor->g = 0.0f;
	outColor->b = 0.0f;

	if (positionT.z < 0.0f) {
		float v11 = attenuation(_field18, _field19, fabs(positionT.y));
		float v12 = attenuation(_field18, _field19, fabs(positionT.x));
		float v13 = attenuation(_field16, _field17, positionT.length());

		outColor->r = v11 * v12 * v13 * _color.r;
		outColor->g = v11 * v12 * v13 * _color.g;
		outColor->b = v11 * v12 * v13 * _color.b;
	}
}

float Light5::calculate(Vector3 start, Vector3 end) {
	return 1.0e30f;
}

void Light5::calculateColor(Color *outColor, Vector3 position) {
	outColor->r = _color.r;
	outColor->g = _color.g;
	outColor->b = _color.b;
}

} // End of namespace BladeRunner
