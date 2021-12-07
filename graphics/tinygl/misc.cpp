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

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void GLContext::glopViewport(GLParam *p) {
	int xsize, ysize, xmin, ymin, xsize_req, ysize_req;

	xmin = p[1].i;
	ymin = p[2].i;
	xsize = p[3].i;
	ysize = p[4].i;

	// we may need to resize the zbuffer

	if (viewport.xmin != xmin || viewport.ymin != ymin ||
			viewport.xsize != xsize || viewport.ysize != ysize) {

		xsize_req = xmin + xsize;
		ysize_req = ymin + ysize;

		if (gl_resize_viewport && gl_resize_viewport(&xsize_req, &ysize_req) != 0) {
			error("glViewport: error while resizing display");
		}

		xsize = xsize_req - xmin;
		ysize = ysize_req - ymin;
		if (xsize <= 0 || ysize <= 0) {
			error("glViewport: size too small");
		}

		viewport.xmin = xmin;
		viewport.ymin = ymin;
		viewport.xsize = xsize;
		viewport.ysize = ysize;

		viewport.updated = 1;
	}
}

void GLContext::glopEnableDisable(GLParam *p) {
	int code = p[1].i;
	int v = p[2].i;

	switch (code) {
	case TGL_CULL_FACE:
		cull_face_enabled = v;
		break;
	case TGL_LIGHTING:
		lighting_enabled = v;
		break;
	case TGL_COLOR_MATERIAL:
		color_material_enabled = v;
		break;
	case TGL_TEXTURE_2D:
		texture_2d_enabled = v;
		break;
	case TGL_NORMALIZE:
		normalize_enabled = v;
		break;
	case TGL_DEPTH_TEST:
		depth_test = v;
		fb->enableDepthTest(v);
		break;
	case TGL_ALPHA_TEST:
		fb->enableAlphaTest(v);
		break;
	case TGL_BLEND:
		fb->enableBlending(v);
		break;
	case TGL_POLYGON_OFFSET_FILL:
		if (v)
			offset_states |= TGL_OFFSET_FILL;
		else
			offset_states &= ~TGL_OFFSET_FILL;
		break;
	case TGL_POLYGON_OFFSET_POINT:
		if (v)
			offset_states |= TGL_OFFSET_POINT;
		else
			offset_states &= ~TGL_OFFSET_POINT;
		break;
	case TGL_POLYGON_OFFSET_LINE:
		if (v)
			offset_states |= TGL_OFFSET_LINE;
		else
			offset_states &= ~TGL_OFFSET_LINE;
		break;
	case TGL_SHADOW_MASK_MODE:
		if (v)
			shadow_mode |= 1;
		else
			shadow_mode &= ~1;
		break;
	case TGL_SHADOW_MODE:
		if (v)
			shadow_mode |= 2;
		else
			shadow_mode &= ~2;
		break;
	default:
		if (code >= TGL_LIGHT0 && code < TGL_LIGHT0 + T_MAX_LIGHTS) {
			gl_enable_disable_light(code - TGL_LIGHT0, v);
		} else {
			//warning("glEnableDisable: 0x%X not supported.", code);
		}
		break;
	}
}

void GLContext::glopBlendFunc(GLParam *p) {
	TGLenum sfactor = p[1].i;
	TGLenum dfactor = p[2].i;
	fb->setBlendingFactors(sfactor, dfactor);
}

void GLContext::glopAlphaFunc(GLParam *p) {
	TGLenum func = p[1].i;
	float ref = p[2].f;
	fb->setAlphaTestFunc(func, (int)(ref * 255));
}

void GLContext::glopDepthFunc(GLParam *p) {
	TGLenum func = p[1].i;
	fb->setDepthFunc(func);
}

void GLContext::glopShadeModel(GLParam *p) {
	int code = p[1].i;
	current_shade_model = code;
}

void GLContext::glopCullFace(GLParam *p) {
	int code = p[1].i;
	current_cull_face = code;
}

void GLContext::glopFrontFace(GLParam *p) {
	int code = p[1].i;
	current_front_face = code;
}

void GLContext::glopPolygonMode(GLParam *p) {
	int face = p[1].i;
	int mode = p[2].i;

	switch (face) {
	case TGL_BACK:
		polygon_mode_back = mode;
		break;
	case TGL_FRONT:
		polygon_mode_front = mode;
		break;
	case TGL_FRONT_AND_BACK:
		polygon_mode_front = mode;
		polygon_mode_back = mode;
		break;
	default:
		assert(0);
	}
}

void GLContext::glopHint(GLParam *) {
	// do nothing
}

void GLContext::glopPolygonOffset(GLParam *p) {
	offset_factor = p[1].f;
	offset_units = p[2].f;
}

void GLContext::glopColorMask(GLParam *p) {
	color_mask = p[1].i;
}

void GLContext::glopDepthMask(TinyGL::GLParam *p) {
	fb->enableDepthWrite(p[1].i);
}

} // end of namespace TinyGL
