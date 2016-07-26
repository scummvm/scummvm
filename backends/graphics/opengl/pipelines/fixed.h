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

#ifndef BACKENDS_GRAPHICS_OPENGL_PIPELINES_FIXED_H
#define BACKENDS_GRAPHICS_OPENGL_PIPELINES_FIXED_H

#include "backends/graphics/opengl/pipelines/pipeline.h"

namespace OpenGL {

#if !USE_FORCED_GLES2
class FixedPipeline : public Pipeline {
public:
	virtual void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

	virtual void drawTexture(const GLTexture &texture, const GLfloat *coordinates);

	virtual void setProjectionMatrix(const GLfloat *projectionMatrix);

protected:
	virtual void activateInternal();
};
#endif // !USE_FORCED_GLES2

} // End of namespace OpenGL

#endif
