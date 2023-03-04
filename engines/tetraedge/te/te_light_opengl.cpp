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

#include "tetraedge/te/te_light_opengl.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector3f32.h"

#include "graphics/opengl/system_headers.h"

namespace Tetraedge {

static inline uint _toGlLight(uint lightno) {
	return GL_LIGHT0 + lightno;
}

TeLightOpenGL::TeLightOpenGL() {
}

void TeLightOpenGL::disable(uint lightno) {
	glDisable(_toGlLight(lightno));
}

void TeLightOpenGL::enable(uint lightno) {
	// Note: original casts to float and compares to 0.01, but that's the same?
	if (_colDiffuse.r() == 0 && _colDiffuse.g() == 0 && _colDiffuse.b() == 0)
		glDisable(_toGlLight(lightno));
	else
		glEnable(_toGlLight(lightno));
}

/*static*/
void TeLightOpenGL::disableAll() {
	glDisable(GL_LIGHTING);
}

/*static*/
void TeLightOpenGL::enableAll() {
	glEnable(GL_LIGHTING);
}

void TeLightOpenGL::draw(TeCamera &camera) {
	error("TODO: Finish TeLightOpenGL::draw");
}

void TeLightOpenGL::update(uint lightno) {
	if (lightno > GL_MAX_LIGHTS)
		error("Invalid light no %d", lightno);
	const uint glLight = _toGlLight(lightno);

	const float ambient[4] = {_colAmbient.r() / 255.0f, _colAmbient.g() / 255.0f,
			_colAmbient.b() / 255.0f, 1.0f};
	glLightfv(glLight, GL_AMBIENT, ambient);

	const float diff[4] = {_colDiffuse.r() / 255.0f, _colDiffuse.g() / 255.0f,
			_colDiffuse.b() / 255.0f, 1.0f};
	glLightfv(glLight, GL_DIFFUSE, diff);

	// WORKAROUND: Original game sets 0.01 as threshold here to avoid enabling
	// the "shadow" light.  However, Syberia CitStation/31130 has shadowlight with
	// values (4, 0, 0) which means it gets enabled and everything is dark.

	if (diff[0] < 0.02f && diff[1] < 0.02f && diff[2] < 0.02f)
		glDisable(glLight);

	const float spec[4] = {_colSpecular.r() / 255.0f, _colSpecular.g() / 255.0f,
			_colSpecular.b() / 255.0f, 1.0f};
	glLightfv(glLight, GL_SPECULAR, spec);

	if (_type == LightTypeSpot || _type == LightTypePoint) {
		const float pos[4] = {_position3d.x(), _position3d.y(), _position3d.z(), 1.0f};
		glLightfv(glLight, GL_POSITION, pos);
		glLightf(glLight, GL_CONSTANT_ATTENUATION, _constAtten);
		glLightf(glLight, GL_LINEAR_ATTENUATION, _linearAtten);
		glLightf(glLight, GL_QUADRATIC_ATTENUATION, _quadraticAtten);
	}

	if (_type == LightTypeDirectional) {
		const TeVector3f32 dirv = directionVector();
		const float dir[4] = {dirv.x(), dirv.y(), dirv.z(), 0.0f};
		glLightfv(glLight, GL_POSITION, dir);
	}

	if (_type == LightTypeSpot) {
		const TeVector3f32 dirv = directionVector();
		const float dir[4] = {dirv.x(), dirv.y(), dirv.z(), 0.0f};
		glLightfv(glLight, GL_SPOT_DIRECTION, dir);
		glLightf(glLight, GL_SPOT_CUTOFF, (_cutoff * 180.0f) / M_PI);
		// Exponent doesn't get set in Syberia 2
		if (g_engine->gameType() == TetraedgeEngine::kSyberia)
			glLightf(glLight, GL_SPOT_EXPONENT, _exponent);
	} else {
		glLightf(glLight, GL_SPOT_CUTOFF, 180.0f);
	}
}

/*static*/
void TeLightOpenGL::updateGlobal() {
	const TeColor globalAmbient(_globalAmbientColor);
	const float col[4] = {globalAmbient.r() / 255.0f,
			globalAmbient.g() / 255.0f, globalAmbient.b() / 255.0f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

} // end namespace Tetraedge
