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

#include "hpl1/opengl.h"
#include "hpl1/debug.h"

namespace Hpl1 {
	static const char* getErrorString(const GLenum code) {
		switch (code) {
		case GL_INVALID_ENUM:
			return "invalid enum";
		case GL_INVALID_VALUE:
			return "invalid value";
		case GL_INVALID_OPERATION:
			return "invalid operation";
		}
		return "unrecognized error";
	}

	void checkOGLErrors(const char *function, const int line) {
		GLenum code;
		while((code = glGetError()) != GL_NO_ERROR)
			logError(kDebugOpenGL, "Opengl error: \'%s\' in function %s - %d\n", getErrorString(code), function, line);
	}
}