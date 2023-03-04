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

#include "graphics/tinygl/tinygl.h"

#include "tetraedge/te/te_light_tinygl.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

static inline uint _toGlLight(uint lightno) {
	return TGL_LIGHT0 + lightno;
}

TeLightTinyGL::TeLightTinyGL() {
}

void TeLightTinyGL::disable(uint lightno) {
	tglDisable(_toGlLight(lightno));
}

void TeLightTinyGL::enable(uint lightno) {
	if (_colDiffuse.r() == 0 && _colDiffuse.g() == 0 && _colDiffuse.b() == 0)
		tglDisable(_toGlLight(lightno));
	else
		tglEnable(_toGlLight(lightno));
}

/*static*/
void TeLightTinyGL::disableAll() {
	tglDisable(TGL_LIGHTING);
}

/*static*/
void TeLightTinyGL::enableAll() {
	tglEnable(TGL_LIGHTING);
}

void TeLightTinyGL::draw(TeCamera &camera) {
	error("TODO: Finish TeLight::draw");
}

void TeLightTinyGL::update(uint lightno) {
	if (lightno > TGL_MAX_LIGHTS)
		error("Invalid light no %d", lightno);
	const uint glLight = _toGlLight(lightno);

	const float ambient[4] = {_colAmbient.r() / 255.0f, _colAmbient.g() / 255.0f,
			_colAmbient.b() / 255.0f, 1.0f};
	tglLightfv(glLight, TGL_AMBIENT, ambient);

	const float diff[4] = {_colDiffuse.r() / 255.0f, _colDiffuse.g() / 255.0f,
			_colDiffuse.b() / 255.0f, 1.0f};
	tglLightfv(glLight, TGL_DIFFUSE, diff);

	// WORKAROUND: Original game sets 0.01 as threshold here to avoid enabling
	// the "shadow" light.  However, Syberia CitStation/31130 has shadowlight with
	// values (4, 0, 0) which means it gets enabled and everything is dark.

	if (diff[0] < 0.02f && diff[1] < 0.02f && diff[2] < 0.02f)
		tglDisable(glLight);

	const float spec[4] = {_colSpecular.r() / 255.0f, _colSpecular.g() / 255.0f,
			_colSpecular.b() / 255.0f, 1.0f};
	tglLightfv(glLight, TGL_SPECULAR, spec);

	if (_type == LightTypeSpot || _type == LightTypePoint) {
		const float pos[4] = {_position3d.x(), _position3d.y(), _position3d.z(), 1.0f};
		tglLightfv(glLight, TGL_POSITION, pos);
		tglLightf(glLight, TGL_CONSTANT_ATTENUATION, _constAtten);
		tglLightf(glLight, TGL_LINEAR_ATTENUATION, _linearAtten);
		tglLightf(glLight, TGL_QUADRATIC_ATTENUATION, _quadraticAtten);
	}

	if (_type == LightTypeDirectional) {
		const TeVector3f32 dirv = directionVector();
		const float dir[4] = {dirv.x(), dirv.y(), dirv.z(), 0.0f};
		tglLightfv(glLight, TGL_POSITION, dir);
	}

	if (_type == LightTypeSpot) {
		const TeVector3f32 dirv = directionVector();
		const float dir[4] = {dirv.x(), dirv.y(), dirv.z(), 0.0f};
		tglLightfv(glLight, TGL_SPOT_DIRECTION, dir);
		tglLightf(glLight, TGL_SPOT_CUTOFF, (_cutoff * 180.0f) / M_PI);
		// Exponent doesn't get set in Syberia 2
		if (g_engine->gameType() == TetraedgeEngine::kSyberia)
			tglLightf(glLight, TGL_SPOT_EXPONENT, _exponent);
	} else {
		tglLightf(glLight, TGL_SPOT_CUTOFF, 180.0f);
	}
}

/*static*/
void TeLightTinyGL::updateGlobal() {
	const TeColor globalAmbient(_globalAmbientColor);
	const float col[4] = {globalAmbient.r() / 255.0f,
			globalAmbient.g() / 255.0f, globalAmbient.b() / 255.0f, 1.0f};
	tglLightModelfv(TGL_LIGHT_MODEL_AMBIENT, col);
}

} // end namespace Tetraedge
