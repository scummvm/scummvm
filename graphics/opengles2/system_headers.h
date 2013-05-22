/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_SYSTEM_HEADERS_H
#define GRAPHICS_SYSTEM_HEADERS_H

#include "common/scummsys.h"

#ifdef USE_GLES2

#define GL_GLEXT_PROTOTYPES
#ifdef IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#undef GL_GLEXT_PROTOTYPES

#define glMapBuffer glMapBufferOES
#define glUnmapBuffer glUnmapBufferOES
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES

#ifndef GL_BGRA
#	define GL_BGRA GL_BGRA_EXT
#endif

#else
#include <GL/glew.h>
#endif

#endif
