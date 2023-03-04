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

#include "common/math.h"
#include "graphics/renderer.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_light_opengl.h"
#include "tetraedge/te/te_light_tinygl.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

/*static*/
uint32 TeLight::_globalAmbientColor = 0xffffffff;

TeLight::TeLight() : _colAmbient(0, 0, 0, 0xff), _colDiffuse(0, 0, 0, 0xff), _colSpecular(0xff, 0xff, 0xff, 0xff),
_constAtten(1.0f), _linearAtten(0.0f), _quadraticAtten(0.0f), _cutoff(0.0f), _exponent(0.0f), _type(LightTypePoint),
_displaySize(3.0f)
{
}

TeVector3f32 TeLight::directionVector() const {
	float cosx = cosf(_positionRadial.getX());
	float cosy = cosf(_positionRadial.getY());
	float sinx = sinf(_positionRadial.getX());
	float siny = sinf(_positionRadial.getY());
	return TeVector3f32(cosx * cosy, siny, sinx * cosy);
}

void TeLight::transformDirPoint(const TeVector3f32 &pt1, TeVector3f32 &pt2) {
	const TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), _positionRadial.getX() + M_PI);
	const TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, -1), -_positionRadial.getY());
	pt2.rotate(q2);
	pt2.rotate(q1);
	pt2 += pt1;
}

void TeLight::transformSpotPoint(TeVector3f32 &pt) {
	const TeQuaternion q1 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), _positionRadial.getX());
	const TeQuaternion q2 = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 0, -1), _positionRadial.getY());
	pt.rotate(q2);
	pt.rotate(q1);
	pt += _position3d;
}

Common::String TeLight::dump() const {
	const char *ltype;
	switch (_type) {
		case LightTypeSpot:
			ltype = "Spot";
			break;
		case LightTypePoint:
			ltype = "Point";
			break;
		case LightTypeDirectional:
			ltype = "Directional";
			break;
		default:
			error("Invalid light type %d", (int)_type);
	}

	return Common::String::format("%sLight(\n\tamb:%s diff:%s spec:%s\n\tpos:%s posRad:%s atten:%.02f %.02f %.02f\n\tcutoff:%.02f exp:%.02f dispSz:%.02f\n)",
		ltype, _colAmbient.dump().c_str(), _colDiffuse.dump().c_str(),
		_colSpecular.dump().c_str(), _position3d.dump().c_str(),
		_positionRadial.dump().c_str(), _constAtten, _linearAtten,
		_quadraticAtten, _cutoff, _exponent, _displaySize);
}

void TeLight::correctAttenuation() {
	if (!_constAtten && !_linearAtten && !_quadraticAtten)
		_constAtten = 1.0;
}

/*static*/
TeLight *TeLight::makeInstance() {
	Graphics::RendererType r = g_engine->preferredRendererType();

#if defined(USE_OPENGL_GAME)
	if (r == Graphics::kRendererTypeOpenGL)
		return new TeLightOpenGL();
#endif

#if defined(USE_TINYGL)
	if (r == Graphics::kRendererTypeTinyGL)
		return new TeLightTinyGL();
#endif
	error("Couldn't create TeLight for selected renderer");
}

} // end namespace Tetraedge
