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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_OPENGL_FRAMEBUFFER_H
#define GRAPHICS_OPENGL_FRAMEBUFFER_H

#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

namespace OpenGL {

class FrameBuffer : public TextureGL {
public:
	FrameBuffer(uint width, uint height);
	FrameBuffer(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height);
#if defined(AMIGAOS) || defined(__MORPHOS__)
	virtual ~FrameBuffer() {}

	void attach() {}
	void detach() {}
#else
	virtual ~FrameBuffer();

	virtual void attach();
	virtual void detach();
#endif

protected:
	GLuint getFrameBufferName() const { return _frameBuffer; }

private:
	void init();
	GLuint _renderBuffers[2];
	GLuint _frameBuffer;
};

#if !defined(USE_GLES2) && !defined(AMIGAOS) && !defined(__MORPHOS__)
class MultiSampleFrameBuffer : public FrameBuffer {
public:
	MultiSampleFrameBuffer(uint width, uint height, int samples);
	virtual ~MultiSampleFrameBuffer();

	virtual void attach();
	virtual void detach();

private:
	void init();
	GLuint _msFrameBufferId;
	GLuint _msColorId;
	GLuint _msDepthId;
	GLuint _msSamples;
};
#endif

} // End of namespace OpenGL

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#endif
