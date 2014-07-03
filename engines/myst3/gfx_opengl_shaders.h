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

#ifndef GFX_OPENGL_SHADERS_H_
#define GFX_OPENGL_SHADERS_H_

#include "common/rect.h"
#include "math/rect2d.h"

#include "graphics/opengles2/shader.h"

#include "engines/myst3/gfx.h"

namespace Myst3 {

class ShaderRenderer : public BaseRenderer {
public:
	ShaderRenderer(OSystem *_system);
	virtual ~ShaderRenderer();

	virtual void init();

	virtual void clear();
	virtual void setupCameraOrtho2D();
	virtual void setupCameraPerspective(float pitch, float heading, float fov);

	virtual Texture *createTexture(const Graphics::Surface *surface);
	virtual void freeTexture(Texture *texture);

	virtual void drawRect2D(const Common::Rect &rect, uint32 color);
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture,
	                                float transparency = -1.0, bool additiveBlending = false);
	virtual void drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
	                                const Math::Vector3d &topRight, const Math::Vector3d &bottomRight,
	                                Texture *texture);

	virtual void drawCube(Texture **textures);
	virtual void draw2DText(const Common::String &text, const Common::Point &position);

	virtual Graphics::Surface *getScreenshot();

	virtual void screenPosToDirection(const Common::Point screen, float &pitch, float &heading);

private:
	void setupQuadEBO();

	Graphics::Shader *_box_shader;
	Graphics::Shader *_cube_shader;
	Graphics::Shader *_rect3d_shader;
	Graphics::Shader *_text_shader;

	GLuint _boxVBO;
	GLuint _cubeVBO;
	GLuint _rect3dVBO;
	GLuint _textVBO;
	GLuint _quadEBO;

	Math::Matrix4 _mvpMatrix;
	Math::Rect2d _viewport;

	Common::String _prevText;
	Common::Point _prevTextPosition;
};

} // End of namespace Myst3

#endif
