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

void GLContext::glopArrayElement(GLParam *param) {
	int offset;
	int states = client_states;
	int idx = param[1].i;

	if (states & COLOR_ARRAY) {
		GLParam p[5];
		int size = color_array_size;
		offset = idx * color_array_stride;
		switch (color_array_type) {
		case TGL_UNSIGNED_BYTE: {
				TGLubyte *array = (TGLubyte *)color_array + offset;
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_BYTE: {
				TGLbyte *array = (TGLbyte *)color_array + offset;
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_UNSIGNED_INT: {
				TGLuint *array = (TGLuint *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_INT: {
				TGLint *array = (TGLint *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_UNSIGNED_SHORT: {
				TGLushort *array = (TGLushort *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_SHORT: {
				TGLshort *array = (TGLshort *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_FLOAT: {
				TGLfloat *array = (TGLfloat *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_DOUBLE: {
				TGLdouble *array = (TGLdouble *)((TGLbyte *)color_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = array[2];
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		default:
			assert(0);
		}
		glopColor(p);
	}
	if (states & NORMAL_ARRAY) {
		offset = idx * normal_array_stride;
		current_normal.W = 0.0f;
		switch (normal_array_type) {
		case TGL_FLOAT: {
				TGLfloat *array = (TGLfloat *)((TGLbyte *)normal_array + offset);
				current_normal.X = array[0];
				current_normal.Y = array[1];
				current_normal.Z = array[2];
				break;
			}
		case TGL_DOUBLE: {
				TGLdouble *array = (TGLdouble *)((TGLbyte *)normal_array + offset);
				current_normal.X = array[0];
				current_normal.Y = array[1];
				current_normal.Z = array[2];
				break;
			}
		case TGL_INT: {
				TGLint *array = (TGLint *)((TGLbyte *)normal_array + offset);
				current_normal.X = array[0];
				current_normal.Y = array[1];
				current_normal.Z = array[2];
				break;
			}
		case TGL_SHORT: {
				TGLshort *array = (TGLshort *)((TGLbyte *)normal_array + offset);
				current_normal.X = array[0];
				current_normal.Y = array[1];
				current_normal.Z = array[2];
			break;
		}
		default:
			assert(0);
		}
	}
	if (states & TEXCOORD_ARRAY) {
		int size = texcoord_array_size;
		offset = idx * texcoord_array_stride;
		switch (texcoord_array_type) {
		case TGL_FLOAT: {
				TGLfloat *array = (TGLfloat *)((TGLbyte *)texcoord_array + offset);
				current_tex_coord.X = array[0];
				current_tex_coord.Y = array[1];
				current_tex_coord.Z = size > 2 ? array[2] : 0.0f;
				current_tex_coord.W = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_DOUBLE: {
				TGLdouble *array = (TGLdouble *)((TGLbyte *)texcoord_array + offset);
				current_tex_coord.X = array[0];
				current_tex_coord.Y = array[1];
				current_tex_coord.Z = size > 2 ? array[2] : 0.0f;
				current_tex_coord.W = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_INT: {
				TGLint *array = (TGLint *)((TGLbyte *)texcoord_array + offset);
				current_tex_coord.X = array[0];
				current_tex_coord.Y = array[1];
				current_tex_coord.Z = size > 2 ? array[2] : 0.0f;
				current_tex_coord.W = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_SHORT: {
				TGLshort *array = (TGLshort *)((TGLbyte *)texcoord_array + offset);
				current_tex_coord.X = array[0];
				current_tex_coord.Y = array[1];
				current_tex_coord.Z = size > 2 ? array[2] : 0.0f;
				current_tex_coord.W = size > 3 ? array[3] : 1.0f;
				break;
			}
		default:
			assert(0);
		}
	}
	if (states & VERTEX_ARRAY) {
		GLParam p[5];
		int size = vertex_array_size;
		offset = idx * vertex_array_stride;
		switch (vertex_array_type) {
		case TGL_FLOAT: {
				TGLfloat *array = (TGLfloat *)((TGLbyte *)vertex_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = size > 2 ? array[2] : 0.0f;
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_DOUBLE: {
				TGLdouble *array = (TGLdouble *)((TGLbyte *)vertex_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = size > 2 ? array[2] : 0.0f;
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_INT: {
				TGLint *array = (TGLint *)((TGLbyte *)vertex_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = size > 2 ? array[2] : 0.0f;
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		case TGL_SHORT: {
				TGLshort *array = (TGLshort *)((TGLbyte *)vertex_array + offset);
				p[1].f = array[0];
				p[2].f = array[1];
				p[3].f = size > 2 ? array[2] : 0.0f;
				p[4].f = size > 3 ? array[3] : 1.0f;
				break;
			}
		default:
			assert(0);
		}
		glopVertex(p);
	}
}

void GLContext::glopDrawArrays(GLParam *p) {
	GLParam array_element[2];
	GLParam begin[2];

	begin[1].i = p[1].i;
	glopBegin(begin);
	for (int i = 0; i < p[3].i; i++) {
		array_element[1].i = p[2].i + i;
		glopArrayElement(array_element);
	}
	glopEnd(nullptr);
}

void GLContext::glopDrawElements(GLParam *p) {
	GLParam array_element[2];
	void *indices;
	GLParam begin[2];

	indices = (char *)p[4].p;
	begin[1].i = p[1].i;

	glopBegin(begin);
	for (int i = 0; i < p[2].i; i++) {
		switch (p[3].i) {
		case TGL_UNSIGNED_BYTE:
			array_element[1].i = ((TGLbyte *)indices)[i];
			break;
		case TGL_UNSIGNED_SHORT:
			array_element[1].i = ((TGLshort *)indices)[i];
			break;
		case TGL_UNSIGNED_INT:
			array_element[1].i = ((TGLint *)indices)[i];
			break;
		default:
			assert(0);
			break;
		}
		glopArrayElement(array_element);
	}
	glopEnd(nullptr);
}

void GLContext::glopEnableClientState(GLParam *p) {
	client_states |= p[1].i;
}

void GLContext::glopDisableClientState(GLParam *p) {
	client_states &= p[1].i;
}

void GLContext::glopVertexPointer(GLParam *p) {
	vertex_array_size = p[1].i;
	vertex_array_type = p[2].i;
	vertex_array = p[4].p;
	switch (vertex_array_type) {
	case TGL_FLOAT:
		vertex_array_stride = p[3].i != 0 ? p[3].i : vertex_array_size * sizeof(TGLfloat);
		break;
	case TGL_DOUBLE:
		vertex_array_stride = p[3].i != 0 ? p[3].i : vertex_array_size * sizeof(TGLdouble);
		break;
	case TGL_INT:
		vertex_array_stride = p[3].i != 0 ? p[3].i : vertex_array_size * sizeof(TGLint);
		break;
	case TGL_SHORT:
		vertex_array_stride = p[3].i != 0 ? p[3].i : vertex_array_size * sizeof(TGLshort);
		break;
	default:
		assert(0);
		break;
	}
}

void GLContext::glopColorPointer(GLParam *p) {
	color_array_size = p[1].i;
	color_array_type = p[2].i;
	color_array = p[4].p;
	switch (color_array_type) {
	case TGL_BYTE:
	case TGL_UNSIGNED_BYTE:
		color_array_stride = p[3].i != 0 ? p[3].i : color_array_stride * sizeof(TGLbyte);
		break;
	case TGL_SHORT:
	case TGL_UNSIGNED_SHORT:
		color_array_stride = p[3].i != 0 ? p[3].i : color_array_stride * sizeof(TGLshort);
		break;
	case TGL_INT:
	case TGL_UNSIGNED_INT:
		color_array_stride = p[3].i != 0 ? p[3].i : color_array_stride * sizeof(TGLint);
		break;
	case TGL_FLOAT:
		color_array_stride = p[3].i != 0 ? p[3].i : color_array_stride * sizeof(TGLfloat);
		break;
	case TGL_DOUBLE:
		color_array_stride = p[3].i != 0 ? p[3].i : color_array_stride * sizeof(TGLdouble);
		break;
	default:
		assert(0);
		break;
	}
}

void GLContext::glopNormalPointer(GLParam *p) {
	normal_array_type = p[1].i;
	normal_array = p[3].p;
	switch (p[1].i) {
	case TGL_FLOAT:
		normal_array_stride = p[2].i != 0 ? p[2].i : 3 * sizeof(TGLfloat);
		break;
	case TGL_DOUBLE:
		normal_array_stride = p[2].i != 0 ? p[2].i : 3 * sizeof(TGLdouble);
		break;
	case TGL_INT:
		normal_array_stride = p[2].i != 0 ? p[2].i : 3 * sizeof(TGLint);
		break;
	case TGL_SHORT:
		normal_array_stride = p[2].i != 0 ? p[2].i : 3 * sizeof(TGLshort);
		break;
	default:
		assert(0);
		break;
	}
}

void GLContext::glopTexCoordPointer(GLParam *p) {
	texcoord_array_size = p[1].i;
	texcoord_array_type = p[2].i;
	texcoord_array = p[4].p;
	switch (texcoord_array_type) {
	case TGL_FLOAT:
		texcoord_array_stride = p[3].i != 0 ? p[3].i : texcoord_array_size * sizeof(TGLfloat);
		break;
	case TGL_DOUBLE:
		texcoord_array_stride = p[3].i != 0 ? p[3].i : texcoord_array_size * sizeof(TGLdouble);
		break;
	case TGL_INT:
		texcoord_array_stride = p[3].i != 0 ? p[3].i : texcoord_array_size * sizeof(TGLint);
		break;
	case TGL_SHORT:
		texcoord_array_stride = p[3].i != 0 ? p[3].i : texcoord_array_size * sizeof(TGLshort);
		break;
	default:
		assert(0);
		break;
	}
}

} // end of namespace TinyGL
