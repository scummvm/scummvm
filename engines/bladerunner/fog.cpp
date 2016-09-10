#include "bladerunner/fog.h"

#include "common/stream.h"

namespace BladeRunner {

Fog::Fog()
{
}

Fog::~Fog()
{
}

int Fog::readCommon(Common::ReadStream* stream)
{
	int offset = stream->readUint32LE();
	stream->read(_name, 20);
	_fogColor.r = stream->readFloatLE();
	_fogColor.g = stream->readFloatLE();
	_fogColor.b = stream->readFloatLE();
	_fogDensity = stream->readFloatLE();
	return offset;
}

void Fog::readAnimationData(Common::ReadStream* stream, int size)
{
	_animatedParameters = stream->readUint32LE();
	_animationData = new float[size / sizeof(float)];
	stream->read(_animationData, size);

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

void Fog::reset()
{
}

void Fog::setupFrame(int frame)
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
	_inverted = invertMatrix(_matrix);
}

void FogCone::read(Common::ReadStream* stream, int framesCount)
{
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogSphere::read(Common::ReadStream* stream, int framesCount)
{
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogBox::read(Common::ReadStream* stream, int framesCount)
{
	_framesCount = framesCount;
	int size = readCommon(stream);
	_parameter1 = stream->readFloatLE();
	_parameter2 = stream->readFloatLE();
	_parameter3 = stream->readFloatLE();
	readAnimationData(stream, size - 60);
}

} // End of namespace BladeRunner
