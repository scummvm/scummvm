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

#ifndef STARK_GFX_OPENGL_H
#define STARK_GFX_OPENGL_H

#include "common/system.h"
#include "math/vector3d.h"

#if defined(USE_OPENGL_GAME)

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/renderentry.h"

#include "graphics/opengl/system_headers.h"

namespace Stark {
namespace Gfx {

class OpenGLDriver : public Driver {
public:
	OpenGLDriver();
	~OpenGLDriver();

	void init() override;

	void setScreenViewport(bool noScaling) override;
	void setViewport(const Common::Rect &rect) override;

	void clearScreen() override;
	void flipBuffer() override;

	Texture *createTexture(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) override;
	VisualActor *createActorRenderer() override;
	VisualProp *createPropRenderer() override;
	SurfaceRenderer *createSurfaceRenderer() override;
	FadeRenderer *createFadeRenderer() override;

	void start2DMode();
	void end2DMode();
	void set3DMode() override;

	Common::Rect getViewport() const;
	Common::Rect getUnscaledViewport() const;
	void setupLights(const LightEntryArray &lights);

	Graphics::Surface *getViewportScreenshot() const override;

private:
	Common::Rect _viewport;
	Common::Rect _unscaledViewport;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME)

#endif // STARK_GFX_OPENGL_H
