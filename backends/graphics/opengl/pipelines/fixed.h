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

#ifndef BACKENDS_GRAPHICS_OPENGL_PIPELINES_FIXED_H
#define BACKENDS_GRAPHICS_OPENGL_PIPELINES_FIXED_H

#include "backends/graphics/opengl/pipelines/pipeline.h"

namespace OpenGL {

#if !USE_FORCED_GLES2
class FixedPipeline : public Pipeline {
public:
	FixedPipeline() : _r(0.f), _g(0.f), _b(0.f), _a(0.f) {}

	void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) override;

	void setProjectionMatrix(const Math::Matrix4 &projectionMatrix) override;

protected:
	void activateInternal() override;
	void drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) override;

	GLfloat _r, _g, _b, _a;
};
#endif // !USE_FORCED_GLES2

} // End of namespace OpenGL

#endif
