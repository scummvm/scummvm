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

#ifndef BACKENDS_GRAPHICS_IOS_RENDERBUFFER_H
#define BACKENDS_GRAPHICS_IOS_RENDERBUFFER_H

#include "backends/graphics/opengl/framebuffer.h"

namespace OpenGL {

/**
 * Render to renderbuffer framebuffer implementation.
 *
 * This target allows to render to a renderbuffer, which can then be used as
 * a rendering source like expected on iOS.
 */
class RenderbufferTarget : public Framebuffer {
public:
	RenderbufferTarget(GLuint renderbufferID);
	~RenderbufferTarget() override;

	/**
	 * Set size of the render target.
	 */
	bool setSize(uint width, uint height) override;

protected:
	void activateInternal() override;

private:
	GLuint _glRBO;
	GLuint _glFBO;
};

} // End of namespace OpenGL

#endif
