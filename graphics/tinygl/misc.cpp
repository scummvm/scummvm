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
		cull_face_enabled = v != 0;
		break;
	case TGL_LIGHTING:
		lighting_enabled = v != 0;
		break;
	case TGL_COLOR_MATERIAL:
		color_material_enabled = v != 0;
		break;
	case TGL_FOG:
		fog_enabled = v != 0;
		break;
	case TGL_TEXTURE_2D:
		texture_2d_enabled = v != 0;
		break;
	case TGL_NORMALIZE:
		normalize_enabled = v != 0;
		break;
	case TGL_DEPTH_TEST:
		depth_test_enabled = v != 0;
		break;
	case TGL_ALPHA_TEST:
		alpha_test_enabled = v != 0;
		break;
	case TGL_POLYGON_STIPPLE:
		polygon_stipple_enabled = v != 0;
		break;
	case TGL_STENCIL_TEST:
		stencil_test_enabled = v != 0;
		break;
	case TGL_BLEND:
		blending_enabled = v != 0;
		break;
	case TGL_SCISSOR_TEST:
		scissor_test_enabled = v != 0;
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
	source_blending_factor = p[1].i;
	destination_blending_factor = p[2].i;
}

void GLContext::glopAlphaFunc(GLParam *p) {
	alpha_test_func = p[1].i;
	alpha_test_ref_val = (int)(p[2].f * 255);
}

void GLContext::glopDepthFunc(GLParam *p) {
	depth_func = p[1].i;
}

void GLContext::glopStencilFunc(GLParam *p) {
	TGLenum func = p[1].i;
	TGLint ref = p[2].i;
	TGLuint mask = p[3].ui;
	if (func < TGL_NEVER || func > TGL_ALWAYS)
		return;
	if (ref < 0)
		ref = 0;
	else if (ref > 255)
		ref = 255;
	stencil_test_func = func;
	stencil_ref_val = ref;
	stencil_mask = mask;
}

void GLContext::glopStencilOp(GLParam *p) {
	stencil_sfail = p[1].i;
	stencil_dpfail = p[2].i;
	stencil_dppass = p[3].i;
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

void GLContext::glopScissor(GLParam *p) {
	// top left corner
	_scissorTestRect.top = p[2].i;
	_scissorTestRect.left = p[1].i;

	// bottom right corner
	_scissorTestRect.bottom = p[4].i;
	_scissorTestRect.right = p[3].i;
}

void GLContext::glopHint(GLParam *) {
	// do nothing
}

void GLContext::glopPolygonStipple(GLParam *p) {
	for (int i = 0; i < 128; i++) {
		polygon_stipple_pattern[i] = p[i + 1].ui;
	}
}

void GLContext::glopPolygonOffset(GLParam *p) {
	offset_factor = p[1].f;
	offset_units = p[2].f;
}

void GLContext::glopColorMask(GLParam *p) {
	color_mask_red = p[1].i == TGL_TRUE;
	color_mask_green = p[2].i == TGL_TRUE;
	color_mask_blue = p[3].i == TGL_TRUE;
	color_mask_alpha = p[4].i == TGL_TRUE;
}

void GLContext::glopDepthMask(GLParam *p) {
	depth_write_mask = p[1].i == TGL_TRUE;
}

void GLContext::glopStencilMask(TinyGL::GLParam *p) {
	stencil_write_mask = p[1].ui;
}

} // end of namespace TinyGL
