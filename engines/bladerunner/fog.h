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

#ifndef BLADERUNNER_FOG_H
#define BLADERUNNER_FOG_H

#include "bladerunner/color.h"
#include "bladerunner/matrix.h"

namespace Common {
	class ReadStream;
}

namespace BladeRunner {

class SetEffects;

class Fog {
	friend class SetEffects;
	friend class Debugger;

protected:
	Common::String _name;

	int        _frameCount;
	int        _animatedParameters;
	Matrix4x3  _matrix;
	Matrix4x3  _inverted;
	Color      _fogColor;
	float      _fogDensity;
	float     *_animationData;
	float     *_m11ptr;
	float     *_m12ptr;
	float     *_m13ptr;
	float     *_m14ptr;
	float     *_m21ptr;
	float     *_m22ptr;
	float     *_m23ptr;
	float     *_m24ptr;
	float     *_m31ptr;
	float     *_m32ptr;
	float     *_m33ptr;
	float     *_m34ptr;

	Fog       *_next;

public:
	Fog();
	virtual ~Fog();

	virtual void read(Common::ReadStream *stream, int frameCount) = 0;
	virtual void calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) = 0;
	void reset();

	void setupFrame(int frame);

protected:
	int readCommon(Common::ReadStream *stream);
	void readAnimationData(Common::ReadStream *stream, int count);

};

class FogSphere : public Fog {
private:
	float _radius;

public:
	FogSphere():_radius(0.0f) {};

	void read(Common::ReadStream *stream, int frameCount) override;
	void calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) override;
};

class FogCone : public Fog {
private:
	float _coneAngle;

public:
	FogCone():_coneAngle(0.0f) {};

	void read(Common::ReadStream *stream, int frameCount) override;
	void calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) override;
};

class FogBox : public Fog {
private:
	Vector3 _size;

public:
	FogBox():_size() {};

	void read(Common::ReadStream *stream, int frameCount) override;
	void calculateCoeficient(Vector3 position, Vector3 viewPosition, float *coeficient) override;
};

} // End of namespace BladeRunner

#endif
