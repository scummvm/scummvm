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

#ifndef STARK_GFX_OPENGLS_H
#define STARK_GFX_OPENGLS_H

#include "common/system.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "engines/stark/gfx/driver.h"

#include "common/rect.h"

#include "math/vector2d.h"

namespace Graphics {
class Shader;
}

namespace Stark {
namespace Gfx {

class OpenGLSDriver : public Driver {
public:
	OpenGLSDriver();
	~OpenGLSDriver();

	void init() override;

	void setScreenViewport(bool noScaling) override;
	void setViewport(Common::Rect rect, bool noScaling) override;

	void clearScreen() override;
	void flipBuffer() override;

	Texture *createTexture(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) override;
	VisualActor *createActorRenderer() override;
	VisualProp *createPropRenderer() override;
	SurfaceRenderer *createSurfaceRenderer() override;

	Graphics::Shader *createActorShaderInstance();
	Graphics::Shader *createSurfaceShaderInstance();

	void start2DMode();
	void end2DMode();
	void set3DMode();

	Common::Rect getViewport() const;
	Common::Rect getUnscaledViewport() const;

private:
	Math::Vector2d scaled(float x, float y) const;

	Common::Rect _viewport;
	Common::Rect _unscaledViewport;

	Graphics::Shader *_boxShader;
	Graphics::Shader *_actorShader;
	uint32 _boxVBO;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#endif // STARK_GFX_OPENGLS_H
