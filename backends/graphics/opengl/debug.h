/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef BACKENDS_GRAPHICS_OPENGL_DEBUG_H
#define BACKENDS_GRAPHICS_OPENGL_DEBUG_H

#define OPENGL_DEBUG

#ifdef OPENGL_DEBUG

namespace OpenGL {
void checkGLError(const char *expr, const char *file, int line);
} // End of namespace OpenGL

#define GL_WRAP_DEBUG(call, name) do { (call); OpenGL::checkGLError(#name, __FILE__, __LINE__); } while (false)
#else
#define GL_WRAP_DEBUG(call, name) do { (call); } while (false)
#endif

#endif
