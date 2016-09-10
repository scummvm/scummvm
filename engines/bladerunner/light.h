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

#ifndef BLADERUNNER_LIGHT_H
#define BLADERUNNER_LIGHT_H

#include "bladerunner/matrix.h"
#include "bladerunner/color.h"

#include "common/stream.h"

namespace Common{
	class ReadStream;
}

namespace BladeRunner {

class Lights;

class Light
{
	friend class Lights;

	char _name[20];
	int _framesCount;
	int _animated;
	int _animatedParameters;
	Matrix4x3 _matrix;
	Color _color;
	float _field16;
	float _field17;
	float _field18;
	float _field19;
	float *_animationData;
	float *_m11ptr;
	float *_m12ptr;
	float *_m13ptr;
	float *_m14ptr;
	float *_m21ptr;
	float *_m22ptr;
	float *_m23ptr;
	float *_m24ptr;
	float *_m31ptr;
	float *_m32ptr;
	float *_m33ptr;
	float *_m34ptr;
	float *_colorRPtr;
	float *_colorGPtr;
	float *_colorBPtr;
	float *_field16ptr;
	float *_field17ptr;
	float *_field18ptr;
	float *_field19ptr;
	Light *_next;

public:
	Light();
	~Light();

	void read(Common::ReadStream *stream, int framesCount, int frame, int animated);
	void readVqa(Common::ReadStream *stream);

	void setupFrame(int frame);

private:
	static float attenuation(float min, float max, float distance);
};

class Light1 : public Light {

};

class Light2 : public Light {

};

class Light3 : public Light {

};

class Light4 : public Light {

};

class Light5 : public Light {

};

} // End of namespace BladeRunner
#endif
