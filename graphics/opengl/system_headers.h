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

#ifndef GRAPHICS_OPENGL_SYSTEM_HEADERS_H
#define GRAPHICS_OPENGL_SYSTEM_HEADERS_H

#include "common/scummsys.h"

// On macOS we only support GL contexts. The reason is that Apple's GL interface
// uses "void *" for GLhandleARB which is not type compatible with GLint. This
// kills our aliasing trick for extension functions and thus would force us to
// supply two different Shader class implementations or introduce other
// wrappers. macOS only supports GL contexts right now anyway (at least
// according to SDL2 sources), thus it is not much of an issue.
#if defined(MACOSX) && (!defined(USE_GLES_MODE) || USE_GLES_MODE != 0)
//#warning "Only forced OpenGL mode is supported on macOS. Overriding settings."
#undef USE_GLES_MODE
#define USE_GLES_MODE 0
#endif

// We allow to force GL or GLES modes on compile time.
// For this the USE_GLES_MODE define is used. The following values represent
// the given selection choices:
//  0 - Force OpenGL context
//  1 - Force OpenGL ES context
//  2 - Force OpenGL ES 2.0 context
#ifdef USE_GLES_MODE
	#define USE_FORCED_GL    (USE_GLES_MODE == 0)
	#define USE_FORCED_GLES  (USE_GLES_MODE == 1)
	#define USE_FORCED_GLES2 (USE_GLES_MODE == 2)
#else
	#define USE_FORCED_GL    0
	#define USE_FORCED_GLES  0
	#define USE_FORCED_GLES2 0
#endif

// Don't include any OpenGL stuff if we didn't enable it
#ifdef USE_OPENGL
#ifdef USE_GLAD

	#include "graphics/opengl/glad.h"

#elif USE_FORCED_GLES2

	#define GL_GLEXT_PROTOTYPES
	#if defined(IPHONE)
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#else
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
	#endif
	#undef GL_GLEXT_PROTOTYPES

#elif USE_FORCED_GLES

	#define GL_GLEXT_PROTOTYPES
	#if defined(IPHONE)
		#include <OpenGLES/ES1/gl.h>
		#include <OpenGLES/ES1/glext.h>
	#else
		#include <GLES/gl.h>
		#include <GLES/glext.h>
	#endif
	#undef GL_GLEXT_PROTOTYPES

#endif
#endif

#if !defined(GL_TEXTURE_MAX_LEVEL) && defined(GL_TEXTURE_MAX_LEVEL_APPLE)
	#define GL_TEXTURE_MAX_LEVEL GL_TEXTURE_MAX_LEVEL_APPLE
#endif

#if !defined(GL_BGRA) && defined(GL_BGRA_EXT)
	#define GL_BGRA GL_BGRA_EXT
#endif

#if !defined(GL_UNPACK_ROW_LENGTH)
	// The Android SDK does not declare GL_UNPACK_ROW_LENGTH_EXT
	#define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif

#if !defined(GL_MAX_SAMPLES)
	// The Android SDK and SDL1 don't declare GL_MAX_SAMPLES
	#define GL_MAX_SAMPLES 0x8D57
#endif

#if !defined(GL_STACK_OVERFLOW)
	#define GL_STACK_OVERFLOW 0x0503
#endif

#if !defined(GL_STACK_UNDERFLOW)
	#define GL_STACK_UNDERFLOW 0x0504
#endif

#if !defined(GL_CLAMP_TO_BORDER)
	#define GL_CLAMP_TO_BORDER 0x812D
#endif

#if !defined(GL_DRAW_FRAMEBUFFER_BINDING)
	#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#endif

#endif
