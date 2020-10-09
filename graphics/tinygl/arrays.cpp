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

#define VERTEX_ARRAY    0x0001
#define COLOR_ARRAY     0x0002
#define NORMAL_ARRAY    0x0004
#define TEXCOORD_ARRAY  0x0008

namespace TinyGL {

void glopArrayElement(GLContext *c, GLParam *param) {
	int i;
	int states = c->client_states;
	int idx = param[1].i;

	if (states & COLOR_ARRAY) {
		GLParam p[5];
		int size = c->color_array_size;
		i = idx * (size + c->color_array_stride);
		p[1].f = c->color_array[i];
		p[2].f = c->color_array[i + 1];
		p[3].f = c->color_array[i + 2];
		p[4].f = size > 3 ? c->color_array[i + 3] : 1.0f;
		glopColor(c, p);
	}
	if (states & NORMAL_ARRAY) {
		i = idx * (3 + c->normal_array_stride);
		c->current_normal.X = c->normal_array[i];
		c->current_normal.Y = c->normal_array[i + 1];
		c->current_normal.Z = c->normal_array[i + 2];
		c->current_normal.W = 0.0f; // NOTE: this used to be Z but assigning Z again seemed like a bug...
	}
	if (states & TEXCOORD_ARRAY) {
		int size = c->texcoord_array_size;
		i = idx * (size + c->texcoord_array_stride);
		c->current_tex_coord.X = c->texcoord_array[i];
		c->current_tex_coord.Y = c->texcoord_array[i + 1];
		c->current_tex_coord.Z = size > 2 ? c->texcoord_array[i + 2] : 0.0f;
		c->current_tex_coord.W = size > 3 ? c->texcoord_array[i + 3] : 1.0f;
	}
	if (states & VERTEX_ARRAY) {
		GLParam p[5];
		int size = c->vertex_array_size;
		i = idx * (size + c->vertex_array_stride);
		p[1].f = c->vertex_array[i];
		p[2].f = c->vertex_array[i + 1];
		p[3].f = size > 2 ? c->vertex_array[i + 2] : 0.0f;
		p[4].f = size > 3 ? c->vertex_array[i + 3] : 1.0f;
		glopVertex(c, p);
	}
}

void glopDrawArrays(GLContext *c, GLParam *p) {
	GLParam array_element[2];
	GLParam begin[2];
	begin[1].i = p[1].i;
	glopBegin(c, begin);
	for (int i=0; i < p[3].i; i++) {
		array_element[1].i = p[2].i + i;
		glopArrayElement(c, array_element);
	}
	glopEnd(c, NULL);
}

void glopEnableClientState(GLContext *c, GLParam *p) {
	c->client_states |= p[1].i;
}

void glopDisableClientState(GLContext *c, GLParam *p) {
	c->client_states &= p[1].i;
}

void glopVertexPointer(GLContext *c, GLParam *p) {
	c->vertex_array_size = p[1].i;
	c->vertex_array_stride = p[2].i;
	c->vertex_array = (float *)p[3].p;
}

void glopColorPointer(GLContext *c, GLParam *p) {
	c->color_array_size = p[1].i;
	c->color_array_stride = p[2].i;
	c->color_array = (float *)p[3].p;
}

void glopNormalPointer(GLContext *c, GLParam *p) {
	c->normal_array_stride = p[1].i;
	c->normal_array = (float *)p[2].p;
}

void glopTexCoordPointer(GLContext *c, GLParam *p) {
	c->texcoord_array_size = p[1].i;
	c->texcoord_array_stride = p[2].i;
	c->texcoord_array = (float *)p[3].p;
}

} // end of namespace TinyGL

void tglArrayElement(TGLint i) {
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_ArrayElement;
	p[1].i = i;
	gl_add_op(p);
}

void tglDrawArrays(TGLenum mode, TGLint first, TGLsizei count) {
	TinyGL::GLParam p[4];
	p[0].op = TinyGL::OP_DrawArrays;
	p[1].i = mode;
	p[2].i = first;
	p[3].i = count;
	gl_add_op(p);
}

void tglEnableClientState(TGLenum array) {
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_EnableClientState;

	switch (array) {
	case TGL_VERTEX_ARRAY:
		p[1].i = VERTEX_ARRAY;
		break;
	case TGL_NORMAL_ARRAY:
		p[1].i = NORMAL_ARRAY;
		break;
	case TGL_COLOR_ARRAY:
		p[1].i = COLOR_ARRAY;
		break;
	case TGL_TEXTURE_COORD_ARRAY:
		p[1].i = TEXCOORD_ARRAY;
		break;
	default:
		assert(0);
		break;
	}
	gl_add_op(p);
}

void tglDisableClientState(TGLenum array) {
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_DisableClientState;

	switch (array) {
	case TGL_VERTEX_ARRAY:
		p[1].i = ~VERTEX_ARRAY;
		break;
	case TGL_NORMAL_ARRAY:
		p[1].i = ~NORMAL_ARRAY;
		break;
	case TGL_COLOR_ARRAY:
		p[1].i = ~COLOR_ARRAY;
		break;
	case TGL_TEXTURE_COORD_ARRAY:
		p[1].i = ~TEXCOORD_ARRAY;
		break;
	default:
		assert(0);
		break;
	}
	gl_add_op(p);
}

void tglVertexPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLParam p[4];
	assert(type == TGL_FLOAT);
	p[0].op = TinyGL::OP_VertexPointer;
	p[1].i = size;
	p[2].i = stride;
	p[3].p = const_cast<void *>(pointer);
	gl_add_op(p);
}

void tglColorPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLParam p[4];
	assert(type == TGL_FLOAT);
	p[0].op = TinyGL::OP_ColorPointer;
	p[1].i = size;
	p[2].i = stride;
	p[3].p = const_cast<void *>(pointer);
	gl_add_op(p);
}

void tglNormalPointer(TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLParam p[3];
	assert(type == TGL_FLOAT);
	p[0].op = TinyGL::OP_NormalPointer;
	p[1].i = stride;
	p[2].p = const_cast<void *>(pointer);
	gl_add_op(p);
}

void tglTexCoordPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLParam p[4];
	assert(type == TGL_FLOAT);
	p[0].op = TinyGL::OP_TexCoordPointer;
	p[1].i = size;
	p[2].i = stride;
	p[3].p = const_cast<void *>(pointer);
	gl_add_op(p);
}
