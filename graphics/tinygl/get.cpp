/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "graphics/tinygl/zgl.h"

void tglGetIntegerv(int pname, int *params) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	switch (pname) {
	case TGL_VIEWPORT:
		params[0] = c->viewport.xmin;
		params[1] = c->viewport.ymin;
		params[2] = c->viewport.xsize;
		params[3] = c->viewport.ysize;
		break;
	case TGL_MAX_MODELVIEW_STACK_DEPTH:
		*params = MAX_MODELVIEW_STACK_DEPTH;
		break;
	case TGL_MAX_PROJECTION_STACK_DEPTH:
		*params = MAX_PROJECTION_STACK_DEPTH;
		break;
	case TGL_MAX_LIGHTS:
		*params = T_MAX_LIGHTS;
		break;
	case TGL_MAX_TEXTURE_SIZE:
		*params = c->_textureSize;
		break;
	case TGL_MAX_TEXTURE_STACK_DEPTH:
		*params = MAX_TEXTURE_STACK_DEPTH;
		break;
	case TGL_BLEND:
		*params = c->fb->isBlendingEnabled();
		break;
	case TGL_ALPHA_TEST:
		*params = c->fb->isAlphaTestEnabled();
		break;
	default:
		error("tglGet: option not implemented");
		break;
	}
}

void tglGetFloatv(int pname, float *v) {
	int i;
	int mnr = 0; // just a trick to return the correct matrix
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
			*v++ = p[0];
			*v++ = p[4];
			*v++ = p[8];
			*v++ = p[12];
			p++;
		}
	}
	break;
	case TGL_LINE_WIDTH:
		*v = 1.0f;
		break;
	case TGL_LINE_WIDTH_RANGE:
		v[0] = v[1] = 1.0f;
		break;
	case TGL_POINT_SIZE:
		*v = 1.0f;
		break;
	case TGL_POINT_SIZE_RANGE:
		v[0] = v[1] = 1.0f;
		break;
	default:
		fprintf(stderr, "warning: unknown pname in glGetFloatv()\n");
		break;
	}
}
