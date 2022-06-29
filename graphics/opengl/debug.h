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

#ifndef BACKENDS_GRAPHICS_OPENGL_DEBUG_H
#define BACKENDS_GRAPHICS_OPENGL_DEBUG_H

#include "graphics/opengl/context.h"

#define OPENGL_DEBUG

#ifdef OPENGL_DEBUG

namespace OpenGL {
void clearGLError();
void checkGLError(const char *expr, const char *file, int line);
} // End of namespace OpenGL

#define GL_WRAP_DEBUG(call, name) do { OpenGL::clearGLError(); (call); OpenGL::checkGLError(#name, __FILE__, __LINE__); } while (false)
#else
#define GL_WRAP_DEBUG(call, name) do { (call); } while (false)
#endif

#define GL_CALL(x)                 GL_WRAP_DEBUG(x, x)
#define GL_CALL_SAFE(func, params) \
	do { \
		if (OpenGLContext.type != kContextNone) { \
			GL_CALL(func params); \
		} \
	} while (0)
#define GL_ASSIGN(var, x)          GL_WRAP_DEBUG(var = x, x)

#endif
