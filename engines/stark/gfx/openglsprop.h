/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_GFX_OPENGL_S_RENDERED_H
#define STARK_GFX_OPENGL_S_RENDERED_H

#include "engines/stark/model/model.h"
#include "engines/stark/visual/prop.h"

#include "common/hashmap.h"
#include "common/hash-ptr.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

namespace OpenGL {
	class ShaderGL;
}

namespace Stark {

namespace Gfx {

class Driver;

class OpenGLSPropRenderer : public VisualProp {
public:
	explicit OpenGLSPropRenderer(Driver *gfx);
	~OpenGLSPropRenderer() override;

	void render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) override;

protected:
	typedef Common::HashMap<const Face *, GLuint> FaceBufferMap;

	Driver *_gfx;
	OpenGL::ShaderGL *_shader;

	bool _modelIsDirty;
	GLuint _faceVBO;
	FaceBufferMap _faceEBO;

	void clearVertices();
	void uploadVertices();
	GLuint createFaceVBO();
	GLuint createFaceEBO(const Face *face);

	void setLightArrayUniform(const LightEntryArray &lights);

};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_OPENGL_S_RENDERED_H

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
