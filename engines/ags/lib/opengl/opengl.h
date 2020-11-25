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

#ifndef AGS_LIB_OPENGL_OPENGL_H
#define AGS_LIB_OPENGL_OPENGL_H

#include "common/scummsys.h"

namespace AGS3 {

#define GL_TRUE   1
#define GL_FALSE  2

#define GFX_XWINDOWS MKTAG('S', 'C', 'V', 'M')
#define GFX_XWINDOWS_FULLSCREEN MKTAG('S', 'C', 'V', 'M')
#define GL_FLOAT 1

// glEnable/glDisable
#define GL_BLEND 1
#define GL_SCISSOR_TEST 2
#define GL_CULL_FACE 3
#define GL_DEPTH_TEST 4
#define GL_LIGHTING 5
#define GL_TEXTURE_2D         0x0DE1
#define GL_NEAREST            0x2600
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801

// glBlendFunc
#define GL_SRC_ALPHA 1
#define GL_ONE_MINUS_SRC_ALPHA 2

// glShadeModel
#define GL_FLAT 1

// glGetString
#define GL_VERSION 1
#define GL_EXTENSIONS 2

// glGetProgramiv
#define GL_LINK_STATUS 1

// glClear
#define GL_COLOR_BUFFER_BIT 1

// glMatrixMode
#define GL_PROJECTION 1
#define GL_MODELVIEW 2

// glEnableClientState/glDisableClientState
#define GL_COLOR_ARRAY 1
#define GL_NORMAL_ARRAY 2
#define GL_VERTEX_ARRAY 3
#define GL_TEXTURE_COORD_ARRAY 4

// glTexParameteri
#define GL_LINEAR 1
#define GL_CLAMP 2
#define GL_TEXTURE_WRAP_T 3
#define GL_TEXTURE_WRAP_S 4

// glBindFramebufferEXT
#define GL_FRAMEBUFFER_EXT 1

// glReadBuffer
#define GL_FRONT 1

// glReadPixels
#define GL_RGBA 1
#define GL_UNSIGNED_BYTE 2

// glDrawArrays
#define GL_TRIANGLE_STRIP 1

// glGetIntegerv/glGetFloatv
#define GL_MAX_TEXTURE_SIZE 1
#define GL_MODELVIEW_MATRIX 2


typedef char GLchar;
typedef byte GLubyte;
typedef uint GLenum;
typedef int GLint;
typedef uint32 GLuint;
typedef uint GLbitfield;
typedef float GLfloat;
typedef double GLdouble;
typedef size_t GLsizei;

struct GLXContextStruct {
};
typedef GLXContextStruct *GLXContext;

inline bool gladLoadGL() { return true; }
inline void glFinish() {}

inline int glCreateProgram() { return 1; }
inline void glDeleteProgram(int prog) {}
inline void glEnable(GLenum cap) {}
inline void glDisable(GLenum cap) {}
inline void glBlendFunc(GLenum sfactor, GLenum dfactor) {}
inline void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {}
inline void glMatrixMode(GLenum mode) {}
inline void glLoadIdentity() {}
inline void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
	GLdouble nearVal, GLdouble farVal);
inline void glEnableClientState(GLenum arr) {}
inline void glDisableClientState(GLenum arr) {}
inline void glUseProgram(GLuint program) {}
inline void glUniform1f(GLint location, GLfloat v0) {}
inline void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
inline void glUniform1i(GLint location, GLint v0) {}
inline void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {}
inline void glMultMatrixf(const GLfloat *m) {}
inline void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
inline void glScalef(GLfloat x, GLfloat y, GLfloat z);
inline void glBindTexture(GLenum target, GLuint texture) {}
inline void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
inline void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {}
inline void glBindFramebufferEXT(GLenum v1, uint v2) {}
inline void glDeleteFramebuffersEXT(int v1, uint *v2) {}
inline void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {}

extern const GLubyte *glGetString(GLenum name);
extern void glTexParameteri(GLenum target, GLenum pname, GLint param);
extern void glDeleteTextures(GLsizei n, const GLuint *textures);
extern GLint glGetUniformLocation(GLuint program, const GLchar *name);
extern void glShadeModel(GLenum mode);
extern void glGetProgramiv(GLuint program, GLenum pname, GLint *params);

extern void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glClear(GLbitfield mask);
extern void glDrawArrays(GLenum mode, GLint first, GLsizei count);
extern void glReadBuffer(GLenum mode);
extern void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data);
extern void glGetIntegerv(GLenum pname, GLint *data);
extern void glGetFloatv(GLenum  pname, GLfloat *params);
extern void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void glTexImage2D(GLenum target, GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data);
extern void glGenTextures(GLsizei n, GLuint *textures);

} // namespace AGS3

#endif
