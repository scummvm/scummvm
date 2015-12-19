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

/* This file is based on Mesa 3-D's gl.h and GLES/gl.h from Khronos Registry.
 *
 * Mesa 3-D's gl.h file is distributed under the following license:
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * GLES/gl.h from Khronos Registry is distributed under the following license:
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/*
 * This file is a template file to be used inside specific locations in the
 * OpenGL graphics code. It is not to be included otherwise. It intentionally
 * does not contain include guards because it can be required to include it
 * multiple times in a source file.
 *
 * Functions are defined by two different user supplied macros:
 * GL_FUNC_DEF: Define a (builtin) OpenGL (ES) function.
 * GL_EXT_FUNC_DEF: Define an OpenGL (ES) extension function.
 */

GL_FUNC_DEF(void, glEnable, (GLenum cap));
GL_FUNC_DEF(void, glDisable, (GLenum cap));
GL_FUNC_DEF(void, glClear, (GLbitfield mask));
GL_FUNC_DEF(void, glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
GL_FUNC_DEF(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height));
GL_FUNC_DEF(void, glMatrixMode, (GLenum mode));
GL_FUNC_DEF(void, glLoadIdentity, ());
GL_FUNC_DEF(void, glLoadMatrixf, (const GLfloat *m));
GL_FUNC_DEF(void, glShadeModel, (GLenum mode));
GL_FUNC_DEF(void, glHint, (GLenum target, GLenum mode));
GL_FUNC_DEF(void, glClearColor, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
GL_FUNC_DEF(void, glBlendFunc, (GLenum sfactor, GLenum dfactor));
GL_FUNC_DEF(void, glEnableClientState, (GLenum array));
GL_FUNC_DEF(void, glPixelStorei, (GLenum pname, GLint param));
GL_FUNC_DEF(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height));
GL_FUNC_DEF(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels));
GL_FUNC_DEF(void, glGetIntegerv, (GLenum pname, GLint *params));
GL_FUNC_DEF(void, glDeleteTextures, (GLsizei n, const GLuint *textures));
GL_FUNC_DEF(void, glGenTextures, (GLsizei n, GLuint *textures));
GL_FUNC_DEF(void, glBindTexture, (GLenum target, GLuint texture));
GL_FUNC_DEF(void, glTexParameteri, (GLenum target, GLenum pname, GLint param));
GL_FUNC_DEF(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
GL_FUNC_DEF(void, glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
GL_FUNC_DEF(void, glVertexPointer, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
GL_FUNC_DEF(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count));
GL_FUNC_DEF(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
GL_FUNC_DEF(const GLubyte *, glGetString, (GLenum name));
GL_FUNC_DEF(GLenum, glGetError, ());
