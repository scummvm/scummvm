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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void GLContext::gl_GetIntegerv(TGLenum pname, TGLint *data) {
	switch (pname) {
	case TGL_VIEWPORT:
		data[0] = viewport.xmin;
		data[1] = viewport.ymin;
		data[2] = viewport.xsize;
		data[3] = viewport.ysize;
		break;
	case TGL_MAX_MODELVIEW_STACK_DEPTH:
		*data = MAX_MODELVIEW_STACK_DEPTH;
		break;
	case TGL_MAX_PROJECTION_STACK_DEPTH:
		*data = MAX_PROJECTION_STACK_DEPTH;
		break;
	case TGL_MAX_LIGHTS:
		*data = T_MAX_LIGHTS;
		break;
	case TGL_MAX_TEXTURE_SIZE:
		*data = _textureSize;
		break;
	case TGL_MAX_TEXTURE_STACK_DEPTH:
		*data = MAX_TEXTURE_STACK_DEPTH;
		break;
	case TGL_BLEND:
		*data = blending_enabled;
		break;
	case TGL_ALPHA_TEST:
		*data = alpha_test_enabled;
		break;
	case TGL_DEPTH_TEST:
		*data = depth_test_enabled;
		break;
	case TGL_STENCIL_TEST:
		*data = stencil_test_enabled;
		break;
	default:
		error("tglGet: option not implemented");
		break;
	}
}

void GLContext::gl_GetFloatv(TGLenum pname, TGLfloat *data) {
	int mnr = 0; // just a trick to return the correct matrix

	switch (pname) {
	case TGL_TEXTURE_MATRIX:
		mnr++;
		// fall through
	case TGL_PROJECTION_MATRIX:
		mnr++;
		// fall through
	case TGL_MODELVIEW_MATRIX: {
			float *p = &matrix_stack_ptr[mnr]->_m[0][0];
			for (int i = 0; i < 4; i++) {
				*data++ = p[0];
				*data++ = p[4];
				*data++ = p[8];
				*data++ = p[12];
				p++;
			}
		}
		break;
	case TGL_LINE_WIDTH:
		*data = 1.0f;
		break;
	case TGL_LINE_WIDTH_RANGE:
		data[0] = data[1] = 1.0f;
		break;
	case TGL_POINT_SIZE:
		*data = 1.0f;
		break;
	case TGL_POINT_SIZE_RANGE:
		data[0] = data[1] = 1.0f;
		break;
	default:
		warning("gl_GetFloatv: unknown pname");
		break;
	}
}

} // end of namespace TinyGL
