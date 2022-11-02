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
#include "graphics/tinygl/zdirtyrect.h"

namespace TinyGL {

void GLContext::glopNormal(GLParam *p) {
	current_normal.X = p[1].f;
	current_normal.Y = p[2].f;
	current_normal.Z = p[3].f;
	current_normal.W = 0.0f;
}

void GLContext::glopTexCoord(GLParam *p) {
	current_tex_coord.X = p[1].f;
	current_tex_coord.Y = p[2].f;
	current_tex_coord.Z = p[3].f;
	current_tex_coord.W = p[4].f;
}

void GLContext::glopEdgeFlag(GLParam *p) {
	current_edge_flag = p[1].i;
}

void GLContext::glopColor(GLParam *p) {
	current_color.X = p[1].f;
	current_color.Y = p[2].f;
	current_color.Z = p[3].f;
	current_color.W = p[4].f;

	if (color_material_enabled) {
		GLParam q[7];
		q[0].op = OP_Material;
		q[1].i = current_color_material_mode;
		q[2].i = current_color_material_type;
		q[3].f = p[1].f;
		q[4].f = p[2].f;
		q[5].f = p[3].f;
		q[6].f = p[4].f;
		glopMaterial(q);
	}
}

void GLContext::gl_eval_viewport() {
	GLViewport *v;
	float zsize = (1 << (ZB_Z_BITS + ZB_POINT_Z_FRAC_BITS));

	v = &viewport;

	// v->ymin needs to be upside down for transformation
	int ymin = fb->getPixelBufferHeight() - v->ysize - v->ymin;
	v->trans.X = (float)(((v->xsize - 0.5) / 2.0) + v->xmin);
	v->trans.Y = (float)(((v->ysize - 0.5) / 2.0) + ymin);
	v->trans.Z = (float)(((zsize - 0.5) / 2.0) + ((1 << ZB_POINT_Z_FRAC_BITS)) / 2);

	v->scale.X = (float)((v->xsize - 0.5) / 2.0);
	// v->ysize needs to be upside down for scaling
	v->scale.Y = (float)(-(v->ysize - 0.5) / 2.0);
	v->scale.Z = (float)(-((zsize - 0.5) / 2.0));
}

void GLContext::glopBegin(GLParam *p) {
	int type;

	assert(in_begin == 0);

	type = p[1].i;
	begin_type = type;
	in_begin = 1;
	vertex_n = 0;
	vertex_cnt = 0;

	if (matrix_model_projection_updated) {
		if (lighting_enabled) {
			// precompute inverse modelview
			matrix_model_view_inv = *matrix_stack_ptr[0];
			matrix_model_view_inv.invert();
			matrix_model_view_inv.transpose();
		} else {
			// precompute projection matrix
			matrix_model_projection = (*matrix_stack_ptr[1]) * (*matrix_stack_ptr[0]);
			// test to accelerate computation
			matrix_model_projection_no_w_transform = 0;
			if (matrix_model_projection._m[3][0] == 0.0 && matrix_model_projection._m[3][1] == 0.0 && matrix_model_projection._m[3][2] == 0.0)
				matrix_model_projection_no_w_transform = 1;
		}

		matrix_model_projection_updated = 0;
	}
	// test if the texture matrix is not Identity
	apply_texture_matrix = !matrix_stack_ptr[2]->isIdentity();

	// viewport
	if (viewport.updated) {
		gl_eval_viewport();
		viewport.updated = 0;
	}
	// triangle drawing functions
	if (render_mode == TGL_SELECT) {
		draw_triangle_front = gl_draw_triangle_select;
		draw_triangle_back = gl_draw_triangle_select;
	} else {
		switch (polygon_mode_front) {
		case TGL_POINT:
			draw_triangle_front = gl_draw_triangle_point;
			break;
		case TGL_LINE:
			draw_triangle_front = gl_draw_triangle_line;
			break;
		default:
			draw_triangle_front = gl_draw_triangle_fill;
			break;
		}

		switch (polygon_mode_back) {
		case TGL_POINT:
			draw_triangle_back = gl_draw_triangle_point;
			break;
		case TGL_LINE:
			draw_triangle_back = gl_draw_triangle_line;
			break;
		default:
			draw_triangle_back = gl_draw_triangle_fill;
			break;
		}
	}
}

// coords, tranformation, clip code and projection
// TODO : handle all cases
void GLContext::gl_vertex_transform(GLVertex *v) {
	Matrix4 *m;

	if (lighting_enabled || fog_enabled) {
		// eye coordinates needed for lighting and fog
		m = matrix_stack_ptr[0];
		m->transform3x4(v->coord, v->ec);
	}

	if (fog_enabled) {
		gl_calc_fog_factor(v);
	}

	if (lighting_enabled) {
		// projection coordinates
		m = matrix_stack_ptr[1];
		m->transform(v->ec, v->pc);

		m = &matrix_model_view_inv;
		m->transform3x3(current_normal, v->normal);

		if (normalize_enabled) {
			v->normal.normalize();
		}
	} else {
		// no eye coordinates needed, no normal
		// NOTE: W = 1 is assumed
		m = &matrix_model_projection;
		m->transform3x4(v->coord, v->pc);

		if (matrix_model_projection_no_w_transform) {
			v->pc.W = (m->_m[3][3]);
		}
		v->normal.X = v->normal.Y = v->normal.Z = 0;
		v->ec.X = v->ec.Y = v->ec.Z = v->ec.W = 0;
	}

	v->clip_code = gl_clipcode(v->pc.X, v->pc.Y, v->pc.Z, v->pc.W);
}

void GLContext::glopVertex(GLParam *p) {
	GLVertex *v;
	int n, cnt;

	assert(in_begin != 0);

	n = vertex_n;
	cnt = vertex_cnt;
	cnt++;
	vertex_cnt = cnt;

	// quick fix to avoid crashes on large polygons
	if (n >= vertex_max) {
		GLVertex *newarray;
		vertex_max <<= 1;    // just double size
		newarray = (GLVertex *)gl_malloc(sizeof(GLVertex) * vertex_max);
		if (!newarray) {
			error("unable to allocate GLVertex array.");
		}
		memcpy(newarray, vertex, n * sizeof(GLVertex));
		gl_free(vertex);
		vertex = newarray;
	}
	// new vertex entry
	v = &vertex[n];
	n++;

	v->coord.X = p[1].f;
	v->coord.Y = p[2].f;
	v->coord.Z = p[3].f;
	v->coord.W = p[4].f;

	gl_vertex_transform(v);

	// color

	if (lighting_enabled) {
		gl_shade_vertex(v);
	} else {
		v->color = current_color;
	}

	// tex coords

	if (texture_2d_enabled) {
		if (apply_texture_matrix) {
			matrix_stack_ptr[2]->transform(current_tex_coord, v->tex_coord);
		} else {
			v->tex_coord = current_tex_coord;
		}
	}
	// precompute the mapping to the viewport
	if (v->clip_code == 0)
		gl_transform_to_viewport(v);

	// edge flag

	v->edge_flag = current_edge_flag;

	vertex_n = n;
}

void GLContext::glopEnd(GLParam *) {
	assert(in_begin == 1);

	if (vertex_cnt > 0) {
		issueDrawCall(new RasterizationDrawCall());
	}

	in_begin = 0;
}

} // end of namespace TinyGL
