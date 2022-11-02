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

TGLint GLContext::gl_RenderMode(TGLenum mode) {
	int result = 0;

	switch (render_mode) {
	case TGL_RENDER:
		break;
	case TGL_SELECT:
		if (select_overflow) {
			result = -select_hits;
		} else {
			result = select_hits;
		}
		select_overflow = 0;
		select_ptr = select_buffer;
		name_stack_size = 0;
		break;
	default:
		assert(0);
	}
	switch (mode) {
	case TGL_RENDER:
		render_mode = TGL_RENDER;
		break;
	case TGL_SELECT:
		render_mode = TGL_SELECT;
		assert(select_buffer != nullptr);
		select_ptr = select_buffer;
		select_hits = 0;
		select_overflow = 0;
		select_hit = nullptr;
		break;
	default:
		assert(0);
	}
	return result;
}

void GLContext::gl_SelectBuffer(TGLsizei size, TGLuint *buffer) {
	assert(render_mode != TGL_SELECT);

	select_buffer = buffer;
	select_size = size;
}

void GLContext::glopInitNames(GLParam *) {
	if (render_mode == TGL_SELECT) {
		name_stack_size = 0;
		select_hit = nullptr;
	}
}

void GLContext::glopPushName(GLParam *p) {
	if (render_mode == TGL_SELECT) {
		assert(name_stack_size < MAX_NAME_STACK_DEPTH);
		name_stack[name_stack_size++] = p[1].i;
		select_hit = nullptr;
	}
}

void GLContext::glopPopName(GLParam *) {
	if (render_mode == TGL_SELECT) {
		assert(name_stack_size > 0);
		name_stack_size--;
		select_hit = nullptr;
	}
}

void GLContext::glopLoadName(GLParam *p) {
	if (render_mode == TGL_SELECT) {
		assert(name_stack_size > 0);
		name_stack[name_stack_size - 1] = p[1].i;
		select_hit = nullptr;
	}
}

void GLContext::gl_add_select(uint zmin, uint zmax) {
	uint *ptr;
	int n;

	if (!select_overflow) {
		if (!select_hit) {
			n = name_stack_size;
			if ((select_ptr - select_buffer + 3 + n) > select_size) {
				select_overflow = 1;
			} else {
				ptr = select_ptr;
				select_hit = ptr;
				*ptr++ = name_stack_size;
				*ptr++ = zmin;
				*ptr++ = zmax;
				for (int i = 0; i < n; i++)
					*ptr++ = name_stack[i];
				select_ptr = ptr;
				select_hits++;
			}
		} else {
			if (zmin < select_hit[1])
				select_hit[1] = zmin;
			if (zmax > select_hit[2])
				select_hit[2] = zmax;
		}
	}
}

} // end of namespace TinyGL
