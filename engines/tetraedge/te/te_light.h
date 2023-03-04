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

#ifndef TETRAEDGE_TE_TE_LIGHT_H
#define TETRAEDGE_TE_TE_LIGHT_H

#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_color.h"

namespace Tetraedge {

enum TeLightType {
	LightTypePoint = 0,
	LightTypeDirectional = 1,
	LightTypeSpot = 2
};

class TeCamera;

class TeLight {
public:
	TeLight();
	virtual ~TeLight() {};

	TeVector3f32 directionVector() const;
	virtual void disable(uint lightno) = 0;
	virtual void enable(uint lightno) = 0;

	virtual void draw(TeCamera &camera) = 0;

	void transformDirPoint(const TeVector3f32 &pt1, TeVector3f32 &pt2);
	void transformSpotPoint(TeVector3f32 &pt1);

	virtual void update(uint lightno) = 0;
	static void setGlobalAmbient(const TeColor &col) { _globalAmbientColor = col.getPacked32(); }
	static TeColor globalAmbient() { return TeColor(_globalAmbientColor); }

	void setSpecular(const TeColor &col) { _colSpecular = col; }
	void setDiffuse(const TeColor &col) { _colDiffuse = col; }
	void setAmbient(const TeColor &col) { _colAmbient = col; }

	void setConstAtten(float val) { _constAtten = val; }
	void setLinearAtten(float val) { _linearAtten = val; }
	void setQuadraticAtten(float val) { _quadraticAtten = val; }
	void setCutoff(float val) { _cutoff = val; }
	void setExponent(float val) { _exponent = val; }
	void setDisplaySize(float val) { _displaySize = val; }
	void setPosition3d(const TeVector3f32 &pos) { _position3d = pos; }
	void setPositionRadial(const TeVector2f32 &pos) { _positionRadial = pos; }
	void setType(TeLightType ltype) { _type = ltype; }

	const TeVector2f32 &positionRadial() const { return _positionRadial; }
	const TeVector3f32 &position3d() const { return _position3d; }

	Common::String dump() const;
	void correctAttenuation();

	static TeLight *makeInstance();

protected:
	TeVector3f32 _position3d;
	TeVector2f32 _positionRadial;

	TeColor _colAmbient;
	TeColor _colDiffuse;
	TeColor _colSpecular;

	enum TeLightType _type;

	static uint32 _globalAmbientColor;

	float _constAtten;
	float _linearAtten;
	float _quadraticAtten;
	float _cutoff;
	float _exponent;
	float _displaySize;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LIGHT_H
