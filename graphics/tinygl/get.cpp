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
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "graphics/tinygl/zgl.h"

void tglGetIntegerv(TGLenum pname, TGLint *data) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	switch (pname) {
	case TGL_VIEWPORT:
		data[0] = c->viewport.xmin;
		data[1] = c->viewport.ymin;
		data[2] = c->viewport.xsize;
		data[3] = c->viewport.ysize;
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
		*data = c->_textureSize;
		break;
	case TGL_MAX_TEXTURE_STACK_DEPTH:
		*data = MAX_TEXTURE_STACK_DEPTH;
		break;
	case TGL_BLEND:
		*data = c->blending_enabled;
		break;
	case TGL_ALPHA_TEST:
		*data = c->alpha_test_enabled;
		break;
	case TGL_DEPTH_TEST:
		*data = c->depth_test_enabled;
		break;
	case TGL_STENCIL_TEST:
		*data = c->stencil_test_enabled;
		break;
	default:
		error("tglGet: option not implemented");
		break;
	}
}

void tglGetFloatv(TGLenum pname, TGLfloat *data) {
	int i, mnr = 0; // just a trick to return the correct matrix
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	switch (pname) {
	case TGL_TEXTURE_MATRIX:
		mnr++;
		// fall through
	case TGL_PROJECTION_MATRIX:
		mnr++;
		// fall through
	case TGL_MODELVIEW_MATRIX: {
		float *p = &c->matrix_stack_ptr[mnr]->_m[0][0];
		for (i = 0; i < 4; i++) {
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
		fprintf(stderr, "warning: unknown pname in glGetFloatv()\n");
		break;
	}
}
