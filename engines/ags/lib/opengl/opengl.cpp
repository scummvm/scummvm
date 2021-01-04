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

#include "ags/lib/opengl/opengl.h"
#include "common/array.h"
#include "graphics/managed_surface.h"
#include "common/system.h"

namespace AGS3 {

Common::Array<Graphics::ManagedSurface> texturesArray;
uint32 currentColor, clearColor;

const GLubyte *glGetString(GLenum name) {
	switch (name) {
	case GL_VERSION:
		return (const GLubyte *)"ScummVM";
	case GL_EXTENSIONS:
		return (const GLubyte *)"ScummVM Extensions";
	default:
		return nullptr;
	}
}

void glTexParameteri(GLenum target, GLenum pname, GLint param) {
	// No implementation
}

void glDeleteTextures(GLsizei n, const GLuint *textures) {
	for (; n > 0; --n, ++textures) {
		texturesArray[*textures].clear();
	}
}

GLint glGetUniformLocation(GLuint program, const GLchar *name) {
	error("TODO: glGetUniformLocation");
}

void glShadeModel(GLenum mode) {
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *params) {
	assert(pname == GL_LINK_STATUS);
	*params = GL_TRUE;
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	Graphics::PixelFormat format = g_system->getScreenFormat();
	currentColor = format.RGBToColor((byte)(red * 255), (byte)(green * 255), (byte)(blue * 255));
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	Graphics::PixelFormat format = g_system->getScreenFormat();
	clearColor = format.RGBToColor((byte)(red * 255), (byte)(green * 255), (byte)(blue * 255));
}

void glClear(GLbitfield mask) {
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
	warning("TODO: glDrawArrays");
}

void glReadBuffer(GLenum mode) {
	warning("TODO: glReadBuffer");
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data) {
	warning("TODO: glReadPixels");
}

void glGetIntegerv(GLenum pname, GLint *data) {
	// TODO: glGetIntegerv
	*data = 0;
}

void glGetFloatv(GLenum pname, GLfloat *params) {
	// TODO: glGetFloatv
	*params = 0.0;
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
	warning("TODO: glTexSubImage2D");
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data) {
	warning("TODO: glTexImage2D");
}

void glGenTextures(GLsizei n, GLuint *textures) {
	warning("TODO: glGenTextures");
}

} // namespace AGS3
