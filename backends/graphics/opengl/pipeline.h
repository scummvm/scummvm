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

#ifndef BACKENDS_GRAPHICS_OPENGL_PIEPLINE_H
#define BACKENDS_GRAPHICS_OPENGL_PIEPLINE_H

#include "backends/graphics/opengl/opengl-sys.h"
#include "backends/graphics/opengl/texture.h"

namespace OpenGL {

class Framebuffer;
#if !USE_FORCED_GLES
class Shader;
#endif

/**
 * Interface for OpenGL pipeline functionality.
 *
 * This encapsulates differences in various rendering pipelines used for
 * OpenGL, OpenGL ES 1, and OpenGL ES 2.
 */
class Pipeline {
public:
	Pipeline();
	virtual ~Pipeline() {}

	/**
	 * Activate the pipeline.
	 *
	 * This sets the OpenGL state to make use of drawing with the given
	 * OpenGL pipeline.
	 */
	virtual void activate() = 0;

	/**
	 * Set framebuffer to render to.
	 *
	 * Client is responsible for any memory management related to framebuffer.
	 *
	 * @param framebuffer Framebuffer to activate.
	 * @return Formerly active framebuffer.
	 */
	Framebuffer *setFramebuffer(Framebuffer *framebuffer);

#if !USE_FORCED_GLES
	/**
	 * Set shader program.
	 *
	 * Not all pipelines support shader programs. This is method exits at this
	 * place for convenience only.
	 *
	 * Client is responsible for any memory management related to shader.
	 *
	 * @param shader Shader program to activate.
	 * @return Formerly active shader program.
	 */
	virtual Shader *setShader(Shader *shader) { return nullptr; }
#endif

	/**
	 * Set modulation color.
	 *
	 * @param r Red component in [0,1].
	 * @param g Green component in [0,1].
	 * @param b Blue component in [0,1].
	 * @param a Alpha component in [0,1].
	 */
	virtual void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) = 0;

	/**
	 * Draw a texture rectangle to the currently active framebuffer.
	 *
	 * @param texture     Texture to use for drawing.
	 * @param coordinates x1, y1, x2, y2 coordinates where to draw the texture.
	 */
	virtual void drawTexture(const GLTexture &texture, const GLfloat *coordinates) = 0;

	void drawTexture(const GLTexture &texture, GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
		const GLfloat coordinates[4*2] = {
			x,     y,
			x + w, y,
			x,     y + h,
			x + w, y + h
		};
		drawTexture(texture, coordinates);
	}

	/**
	 * Set the projection matrix.
	 *
	 * This is intended to be only ever be used by Framebuffer subclasses.
	 */
	virtual void setProjectionMatrix(const GLfloat *projectionMatrix) = 0;

protected:
	Framebuffer *_activeFramebuffer;
};

#if !USE_FORCED_GLES2
class FixedPipeline : public Pipeline {
public:
	virtual void activate();

	virtual void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

	virtual void drawTexture(const GLTexture &texture, const GLfloat *coordinates);

	virtual void setProjectionMatrix(const GLfloat *projectionMatrix);
};
#endif // !USE_FORCED_GLES2

#if !USE_FORCED_GLES
class ShaderPipeline : public Pipeline {
public:
	ShaderPipeline();

	virtual void activate();

	virtual Shader *setShader(Shader *shader);

	virtual void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

	virtual void drawTexture(const GLTexture &texture, const GLfloat *coordinates);

	virtual void setProjectionMatrix(const GLfloat *projectionMatrix);

private:
	Shader *_activeShader;
};
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL

#endif
