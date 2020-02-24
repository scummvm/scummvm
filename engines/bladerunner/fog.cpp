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
	_next               = nullptr;
}

Fog::~Fog() {
	if (_animationData != nullptr) {
		delete[] _animationData;
	}
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

	if (_animationData != nullptr) {
		delete[] _animationData;
	}

	int floatCount = size / 4;
	_animationData = new float[floatCount];
	for (int i = 0; i < floatCount; ++i) {
		_animationData[i] = stream->readFloatLE();
	}

	_m11ptr = _animationData;
	_m12ptr = _m11ptr + ((_animatedParameters &   0x1) ? _frameCount : 1);
	_m13ptr = _m12ptr + ((_animatedParameters &   0x2) ? _frameCount : 1);
	_m14ptr = _m13ptr + ((_animatedParameters &   0x4) ? _frameCount : 1);
	_m21ptr = _m14ptr + ((_animatedParameters &   0x8) ? _frameCount : 1);
	_m22ptr = _m21ptr + ((_animatedParameters &  0x10) ? _frameCount : 1);
	_m23ptr = _m22ptr + ((_animatedParameters &  0x20) ? _frameCount : 1);
	_m24ptr = _m23ptr + ((_animatedParameters &  0x40) ? _frameCount : 1);
	_m31ptr = _m24ptr + ((_animatedParameters &  0x80) ? _frameCount : 1);
	_m32ptr = _m31ptr + ((_animatedParameters & 0x100) ? _frameCount : 1);
	_m33ptr = _m32ptr + ((_animatedParameters & 0x200) ? _frameCount : 1);
	_m34ptr = _m33ptr + ((_animatedParameters & 0x400) ? _frameCount : 1);

	setupFrame(0);
}

void Fog::reset() {
}

void Fog::setupFrame(int frame) {
	int offset = frame % _frameCount;
	_matrix._m[0][0] = ((_animatedParameters &   0x1) ? _m11ptr[offset] : *_m11ptr);
	_matrix._m[0][1] = ((_animatedParameters &   0x2) ? _m12ptr[offset] : *_m12ptr);
	_matrix._m[0][2] = ((_animatedParameters &   0x4) ? _m13ptr[offset] : *_m13ptr);
	_matrix._m[0][3] = ((_animatedParameters &   0x8) ? _m14ptr[offset] : *_m14ptr);
	_matrix._m[1][0] = ((_animatedParameters &  0x10) ? _m21ptr[offset] : *_m21ptr);
	_matrix._m[1][1] = ((_animatedParameters &  0x20) ? _m22ptr[offset] : *_m22ptr);
	_matrix._m[1][2] = ((_animatedParameters &  0x40) ? _m23ptr[offset] : *_m23ptr);
	_matrix._m[1][3] = ((_animatedParameters &  0x80) ? _m24ptr[offset] : *_m24ptr);
	_matrix._m[2][0] = ((_animatedParameters & 0x100) ? _m31ptr[offset] : *_m31ptr);
	_matrix._m[2][1] = ((_animatedParameters & 0x200) ? _m32ptr[offset] : *_m32ptr);
	_matrix._m[2][2] = ((_animatedParameters & 0x400) ? _m33ptr[offset] : *_m33ptr);
	_matrix._m[2][3] = ((_animatedParameters & 0x800) ? _m34ptr[offset] : *_m34ptr);
	_inverted = invertMatrix(_matrix);
}

void FogSphere::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_radius = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogSphere::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	// Ray - sphere intersection, where sphere center is always at 0, 0, 0 as everything else tranformed by the fog matrix.
	// Quadratic formula can and was simplified becasue rayDirection is normalized and hence a = 1.
	// Explained on wikipedia https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// There is also alternative approach which will end-up with this formula where plane is created from ray origin, ray destination
	// and sphere center, then there is only need to solve two right triangles.
	// Explained in book Andrew S. Glassner (1995), Graphics Gems I (p. 388-389)

	Vector3 rayOrigin = _matrix * position;
	Vector3 rayDestination = _matrix * viewPosition;
	Vector3 rayDirection = (rayDestination - rayOrigin).normalize();

	float b = Vector3::dot(rayDirection, rayOrigin);
	float c = Vector3::dot(rayOrigin, rayOrigin) - (_radius * _radius);
	float d = b * b - c;

	if (d >= 0.0f) { // there is an interstection between ray and the sphere
		Vector3 intersection1 = rayOrigin + (-b - sqrt(d)) * rayDirection;
		Vector3 intersection2 = rayOrigin + (-b + sqrt(d)) * rayDirection;

		Vector3 intersection1World = _inverted * intersection1;
		Vector3 intersection2World = _inverted * intersection2;

		float intersection1Distance = (intersection1World - position).length();
		float intersection2Distance = (intersection2World - position).length();

		float distance = (viewPosition - position).length();

		if (intersection1Distance < 0.0f) {
			intersection1Distance = 0.0f;
		}
		if (intersection2Distance > distance) {
			intersection2Distance = distance;
		}
		if (intersection2Distance >= intersection1Distance) {
			*coeficient = intersection2Distance - intersection1Distance;
		}
	}
}

void FogCone::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_coneAngle = stream->readFloatLE();
	readAnimationData(stream, size - 52);
}

void FogCone::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	// ray - cone intersection, cone vertex V lies at (0,0,0) and direction v = (0,0,-1)
	// The algorithm looks like from book Alan W. Paeth (1995), Graphics Gems V (p. 228-230)

	Vector3 positionT = _matrix * position;
	Vector3 viewPositionT = _matrix * viewPosition;

	Vector3 v(0.0f, 0.0f, -1.0f);

	Vector3 planeNormal = Vector3::cross(positionT, viewPositionT).normalize();

	if (planeNormal.x != 0.0f || planeNormal.y != 0.0f || planeNormal.z != 0.0f) {

		if (planeNormal.z < 0.0f) {
			planeNormal = -1.0f * planeNormal;
		}

		float cosTheta = sqrt(1.0f - Vector3::dot(planeNormal, v) * Vector3::dot(planeNormal, v));

		if (cosTheta > cos(_coneAngle)) {
			Vector3 u = Vector3::cross(v, planeNormal).normalize();
			Vector3 w = Vector3::cross(u, v).normalize();

			float tanTheta = sqrt(1.0f - cosTheta * cosTheta) / cosTheta;

			Vector3 temp1 = tanTheta * w;
			Vector3 temp2 = sqrt(tan(_coneAngle) * tan(_coneAngle) - tanTheta * tanTheta) * u;

			Vector3 delta1 = v + temp1 - temp2;
			Vector3 delta2 = v + temp1 + temp2;

			Vector3 d = viewPositionT - positionT;
			Vector3 vecVD = -1.0f * positionT;

			Vector3 crossddelta1 = Vector3::cross(d, delta1);
			Vector3 crossddelta2 = Vector3::cross(d, delta2);

			float r1 = Vector3::dot(Vector3::cross(vecVD, delta1), crossddelta1) / Vector3::dot(crossddelta1, crossddelta1);
			float r2 = Vector3::dot(Vector3::cross(vecVD, delta2), crossddelta2) / Vector3::dot(crossddelta2, crossddelta2);

			if (r2 < r1) {
				float temp = r1;
				r1 = r2;
				r2 = temp;
			}

			if (r1 <= 1.0f && r2 >= 0.0f) {
				if (r1 < 0.0f) {
					r1 = 0.0;
				}
				if (r2 > 1.0f) {
					r2 = 1.0;
				}

				Vector3 intersection1 = positionT + (r1 * d);
				Vector3 intersection1World = _inverted * intersection1;

				Vector3 intersection2 = positionT + (r2 * d);
				Vector3 intersection2World = _inverted * intersection2;

				*coeficient = (intersection2World - intersection1World).length();
			}
		}
	}
}

void FogBox::read(Common::ReadStream *stream, int frameCount) {
	_frameCount = frameCount;
	int size = readCommon(stream);
	_size.x = stream->readFloatLE();
	_size.y = stream->readFloatLE();
	_size.z = stream->readFloatLE();
	readAnimationData(stream, size - 60);
}

void FogBox::calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) {
	*coeficient = 0.0f;

	// line - box intersection, where everything is rotated to box orientation by the fog matrix

	Vector3 point1 = _matrix * position;
	Vector3 point2 = _matrix * viewPosition;

	Vector3 intersection1 = point1;
	Vector3 intersection2 = point2;

	Vector3 direction = point2 - point1;

	// clip X
	float minX = -(_size.x * 0.5f);
	if (point1.x < minX) {
		if (point2.x < minX) {
			return;
		}
		float scale = (minX - point1.x) / direction.x;
		intersection1 = point1 + scale * direction;
	} else if (point2.x < minX) {
		float scale = (minX - point2.x) / direction.x;
		intersection2 = point2 + scale * direction;
	}

	float maxX = _size.x * 0.5f;
	if (intersection1.x > maxX ) {
		if (intersection2.x > maxX) {
			return;
		}
		float scale = (maxX - intersection1.x) / direction.x;
		intersection1 = intersection1 + scale * direction;
	} else if (intersection2.x > maxX) {
		float scale = (maxX - intersection2.x) / direction.x;
		intersection2 = intersection2 + scale * direction;
	}

	// clip Y
	float minY = -(_size.y * 0.5f);
	if (intersection1.y < minY) {
		if (intersection2.y < minY) {
			return;
		}
		float scale = (minY - intersection1.y) / direction.y;
		intersection1 = intersection1 + scale * direction;
	} else if (intersection2.y < minY) {
		float scale = (minY - intersection2.y) / direction.y;
		intersection2 = intersection2 + scale * direction;
	}

	float maxY = _size.y * 0.5f;
	if (intersection1.y > maxY) {
		if (intersection2.y > maxY) {
			return;
		}
		float scale = (maxY - intersection1.y) / direction.y;
		intersection1 = intersection1 + scale * direction;
	} else if (intersection2.y > maxY) {
		float scale = (maxY - intersection2.y) / direction.y;
		intersection2 = intersection2 + scale * direction;
	}

	// clip Z
	if (intersection1.z < 0.0f) {
		if (intersection2.z < 0.0f) {
			return;
		}
		float scale = -intersection1.z / direction.z;
		intersection1 = intersection1 + scale * direction;
	} else if (intersection2.z < 0.0f) {
		float scale = -intersection2.z / direction.z;
		intersection2 = intersection2 + scale * direction;
	}

	if (intersection1.z > _size.z) {
		if (intersection2.z > _size.z) {
			return;
		}
		float scale = (_size.z - intersection1.z) / direction.z;
		intersection1 = intersection1 + scale * direction;
	} else if (intersection2.z > _size.z) {
		float scale = (_size.z - intersection2.z) / direction.z;
		intersection2 = intersection2 + scale * direction;
	}

	Vector3 intersection1World = _inverted * intersection1;
	Vector3 intersection2World = _inverted * intersection2;

	*coeficient = (intersection2World - intersection1World).length();
}
} // End of namespace BladeRunner
