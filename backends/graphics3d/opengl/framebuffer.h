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

#ifndef BACKENDS_GRAPHICS3D_OPENGL_FRAMEBUFFER_H
#define BACKENDS_GRAPHICS3D_OPENGL_FRAMEBUFFER_H

#include "graphics/opengl/system_headers.h"

#include "graphics/opengl/texture.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

namespace OpenGL {

class FrameBuffer : public Texture {
public:
	FrameBuffer(uint width, uint height);
	virtual ~FrameBuffer();

	virtual void attach();
	virtual void detach();

protected:
	FrameBuffer(GLenum glIntFormat, GLenum glFormat, GLenum glType, bool autoCreate)
		: Texture(glIntFormat, glFormat, glType, autoCreate) {}

	GLuint getFrameBufferName() const { return _frameBuffer; }

	void init();

private:
	GLuint _renderBuffers[2];
	GLuint _frameBuffer;
};

#if !USE_FORCED_GLES2 || defined(USE_GLAD)
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

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#endif
