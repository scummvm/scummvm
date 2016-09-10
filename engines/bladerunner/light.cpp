#include "bladerunner/light.h"

namespace BladeRunner {

Light::Light()
{
}

Light::~Light()
{
}

void Light::read(Common::ReadStream* stream, int framesCount, int frame, int animated)
{
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

void Light::readVqa(Common::ReadStream* stream)
{
}

void Light::setupFrame(int frame)
{
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

float Light::attenuation(float min, float max, float distance)
{
	return 0.0;
}

} // End of namespace BladeRunner
