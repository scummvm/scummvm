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

#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void GLContext::gl_print_matrix(const float *m) {
	for (int i = 0; i < 4; i++) {
		fprintf(stderr, "%f %f %f %f\n", m[i], m[4 + i], m[8 + i], m[12 + i]);
	}
}

static inline void gl_matrix_update(GLContext *c) {
	c->matrix_model_projection_updated |= (c->matrix_mode <= 1);
}

void GLContext::glopMatrixMode(GLParam *p) {
	int mode = p[1].i;
	switch (mode) {
	case TGL_MODELVIEW:
		matrix_mode = 0;
		break;
	case TGL_PROJECTION:
		matrix_mode = 1;
		break;
	case TGL_TEXTURE:
		matrix_mode = 2;
		break;
	default:
		assert(0);
	}
}

void GLContext::glopLoadMatrix(GLParam *p) {
	Matrix4 *m;
	GLParam *q;

	m = matrix_stack_ptr[matrix_mode];
	q = p + 1;

	for (int i = 0; i < 4; i++) {
		m->_m[0][i] = q[0].f;
		m->_m[1][i] = q[1].f;
		m->_m[2][i] = q[2].f;
		m->_m[3][i] = q[3].f;
		q += 4;
	}

	gl_matrix_update(this);
}

void GLContext::glopLoadIdentity(GLParam *) {
	matrix_stack_ptr[matrix_mode]->identity();
	gl_matrix_update(this);
}

void GLContext::glopMultMatrix(GLParam *p) {
	Matrix4 m;
	GLParam *q;
	q = p + 1;

	for (int i = 0; i < 4; i++) {
		m._m[0][i] = q[0].f;
		m._m[1][i] = q[1].f;
		m._m[2][i] = q[2].f;
		m._m[3][i] = q[3].f;
		q += 4;
	}

	*matrix_stack_ptr[matrix_mode] *= m;

	gl_matrix_update(this);
}

void GLContext::glopPushMatrix(GLParam *) {
	int n = matrix_mode;
	Matrix4 *m;

	assert((matrix_stack_ptr[n] - matrix_stack[n] + 1) < matrix_stack_depth_max[n]);

	m = ++matrix_stack_ptr[n];

	m[0] = m[-1];

	gl_matrix_update(this);
}

void GLContext::glopPopMatrix(GLParam *) {
	int n = matrix_mode;

	assert(matrix_stack_ptr[n] > matrix_stack[n]);
	matrix_stack_ptr[n]--;
	gl_matrix_update(this);
}

void GLContext::glopRotate(GLParam *p) {
	Matrix4 m;
	float u[3];
	float angle;
	int dir_code;

	angle = (float)(p[1].f * (float)M_PI / 180.0);
	u[0] = p[2].f;
	u[1] = p[3].f;
	u[2] = p[4].f;

	// simple case detection
	dir_code = ((u[0] != 0) << 2) | ((u[1] != 0) << 1) | (u[2] != 0);

	switch (dir_code) {
	case 0:
		m.identity();
		break;
	case 4:
		if (u[0] < 0)
			angle = -angle;
		m.rotation(angle, 0);
		break;
	case 2:
		if (u[1] < 0)
			angle = -angle;
		m.rotation(angle, 1);
		break;
	case 1:
		if (u[2] < 0)
			angle = -angle;
		m.rotation(angle, 2);
		break;
	default: {
		float cost, sint;

		// normalize vector
		float len = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		if (len == 0.0f)
			return;
		len = 1.0f / sqrt(len);
		u[0] *= len;
		u[1] *= len;
		u[2] *= len;

		// store cos and sin values
		cost = cos(angle);
		sint = sin(angle);

		// fill in the values
		m._m[3][0] = 0.0f;
		m._m[3][2] = 0.0f;
		m._m[0][3] = 0.0f;
		m._m[1][3] = 0.0f;
		m._m[2][3] = 0.0f;
		m._m[3][3] = 1.0f;

		// do the math
		m._m[0][0] = u[0] * u[0] + cost * (1 - u[0] * u[0]);
		m._m[1][0] = u[0] * u[1] * (1 -cost) - u[2] * sint;
		m._m[2][0] = u[2] * u[0] * (1 -cost) + u[1] * sint;
		m._m[0][1] = u[0] * u[1] * (1 -cost) + u[2] * sint;
		m._m[1][1] = u[1] * u[1] + cost * (1 - u[1] * u[1]);
		m._m[2][1] = u[1] * u[2] * (1 - cost) - u[0] * sint;
		m._m[0][2] = u[2] * u[0] * (1 - cost) - u[1] * sint;
		m._m[1][2] = u[1] * u[2] * (1 - cost) + u[0] * sint;
		m._m[2][2] = u[2] * u[2] + cost * (1 - u[2] * u[2]);
	}
	}

	*matrix_stack_ptr[matrix_mode] *= m;

	gl_matrix_update(this);
}

void GLContext::glopScale(GLParam *p) {
	matrix_stack_ptr[matrix_mode]->scale(p[1].f, p[2].f, p[3].f);
	gl_matrix_update(this);
}

void GLContext::glopTranslate(GLParam *p) {
	matrix_stack_ptr[matrix_mode]->translate(p[1].f, p[2].f, p[3].f);
	gl_matrix_update(this);
}

void GLContext::glopFrustum(GLParam *p) {
	float left = p[1].f;
	float right = p[2].f;
	float bottom = p[3].f;
	float top = p[4].f;
	float nearp = p[5].f;
	float farp = p[6].f;
	Matrix4 m = Matrix4::frustum(left, right, bottom, top, nearp, farp);

	*matrix_stack_ptr[matrix_mode] *= m;

	gl_matrix_update(this);
}

void GLContext::glopOrtho(GLParam *p) {
	float *r;
	TinyGL::Matrix4 m;
	float left = p[1].f;
	float right = p[2].f;
	float bottom = p[3].f;
	float top = p[4].f;
	float zNear = p[5].f;
	float zFar = p[6].f;

	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (zFar - zNear);

	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -(zFar + zNear) / (zFar - zNear);

	r = &m._m[0][0];
	r[0] = a; r[1] = 0; r[2] = 0; r[3] = tx;
	r[4] = 0; r[5] = b; r[6] = 0; r[7] = ty;
	r[8] = 0; r[9] = 0; r[10] = c; r[11] = tz;
	r[12] = 0; r[13] = 0; r[14] = 0; r[15] = 1;

	*matrix_stack_ptr[matrix_mode] *= m;
	gl_matrix_update(this);
}

} // end of namespace TinyGL
