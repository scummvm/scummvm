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

#include "graphics/opengl/debug.h"

#include "common/str.h"
#include "common/textconsole.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL)

namespace OpenGL {

namespace {
Common::String getGLErrStr(GLenum error) {
	switch (error) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		break;
	}

	return Common::String::format("(Unknown GL error code 0x%X)", error);
}
} // End of anonymous namespace

void clearGLError() {
	GLenum error;

	while ((error = glGetError()) != GL_NO_ERROR)
		;
}

bool checkGLError(const char *expr, const char *file, int line) {
	bool ret = false;
	GLenum error;

	while ((error = glGetError()) != GL_NO_ERROR) {
		// We cannot use error here because we do not know whether we have a
		// working screen or not.
		warning("GL ERROR: %s on %s (%s:%d)", getGLErrStr(error).c_str(), expr, file, line);
		ret = true;
	}
	return ret;
}
} // End of namespace OpenGL

#endif
