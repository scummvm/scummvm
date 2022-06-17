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

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void GLContext::glopFog(GLParam *p) {
	int pname = p[1].i;

	switch (pname) {
	case TGL_FOG_MODE:
		switch (p[2].i) {
		case TGL_LINEAR:
		case TGL_EXP:
		case TGL_EXP2:
			fog_mode = p[2].i;
			break;
		default:
			error("tglFog: unknown fog mode");
			return;
		}
		break;
	case TGL_FOG_DENSITY:
		if (p[2].f < 0.0f) {
			error("tglFog: fog density negate param");
			return;
		}
		fog_density = p[2].f;
		break;
	case TGL_FOG_START:
		fog_start = p[2].f;
		break;
	case TGL_FOG_END:
		fog_end = p[2].f;
		break;
	case TGL_FOG_COLOR:
		fog_color = Vector4(clampf(p[2].f, 0.0f, 1.0f),
		                    clampf(p[3].f, 0.0f, 1.0f),
		                    clampf(p[4].f, 0.0f, 1.0f),
		                    clampf(p[5].f, 0.0f, 1.0f));
		break;
	default:
		error("tglFog: param not implemented");
		return;
	}
}

void GLContext::gl_calc_fog_factor(GLVertex *v) {
	float eye_distance = sqrtf(v->ec.X * v->ec.X + v->ec.Y * v->ec.Y + v->ec.Z * v->ec.Z);

	switch (fog_mode) {
	case TGL_LINEAR: {
		float fog_distance = 1.0f;
		if (fog_start != fog_end)
			fog_distance /= (fog_end - fog_start);
		v->fog_factor = (fog_end - eye_distance) * fog_distance;
		}
		break;
	case TGL_EXP:
		v->fog_factor = expf(-fog_density * eye_distance);
		break;
	case TGL_EXP2:
		v->fog_factor = expf(-(fog_density * fog_density * eye_distance * eye_distance));
		break;
	default:
		break;
	}

	clampf(v->fog_factor, 0.0f, 1.0f);
}

} // end of namespace TinyGL
