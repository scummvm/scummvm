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

#ifndef STARK_GFX_OPENGLS_H
#define STARK_GFX_OPENGLS_H

#include "common/system.h"

#if defined(USE_OPENGL_SHADERS)

#include "engines/stark/gfx/driver.h"

#include "graphics/opengl/system_headers.h"

namespace OpenGL {
class ShaderGL;
}

namespace Stark {
namespace Gfx {

class OpenGLSDriver : public Driver {
public:
	OpenGLSDriver();
	~OpenGLSDriver();

	void init() override;

	void setScreenViewport(bool noScaling) override;
	void setViewport(const Common::Rect &rect) override;

	void clearScreen() override;
	void flipBuffer() override;

	Texture *createTexture(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) override;
	Texture *createBitmap(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) override;
	VisualActor *createActorRenderer() override;
	VisualProp *createPropRenderer() override;
	SurfaceRenderer *createSurfaceRenderer() override;
	FadeRenderer *createFadeRenderer() override;

	OpenGL::ShaderGL *createActorShaderInstance();
	OpenGL::ShaderGL *createSurfaceShaderInstance();
	OpenGL::ShaderGL *createFadeShaderInstance();
	OpenGL::ShaderGL *createShadowShaderInstance();

	void start2DMode();
	void end2DMode();
	void set3DMode() override;
	bool computeLightsEnabled() override;

	Common::Rect getViewport() const;
	Common::Rect getUnscaledViewport() const;

	Graphics::Surface *getViewportScreenshot() const override;

private:
	Common::Rect _viewport;
	Common::Rect _unscaledViewport;

	OpenGL::ShaderGL *_surfaceShader;
	OpenGL::ShaderGL *_actorShader;
	OpenGL::ShaderGL *_fadeShader;
	OpenGL::ShaderGL *_shadowShader;
	GLuint _surfaceVBO;
	GLuint _fadeVBO;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_SHADERS)

#endif // STARK_GFX_OPENGLS_H
