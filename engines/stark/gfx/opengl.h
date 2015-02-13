/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_GFX_OPENGL_H
#define STARK_GFX_OPENGL_H

#include "common/system.h"

#ifdef USE_OPENGL

#include "engines/stark/gfx/driver.h"

namespace Stark {
namespace Gfx {

class OpenGLGfxDriver : public GfxDriver {
public:
	OpenGLGfxDriver();
	~OpenGLGfxDriver();

	void setupScreen(int screenW, int screenH, bool fullscreen);

	void setGameViewport() override;
	void setScreenViewport() override;

	void setupPerspective(const Math::Matrix4 &projectionMatrix) override;
	void setupCamera(const Math::Vector3d &position, const Math::Matrix4 &lookAt) override;

	void clearScreen();
	void flipBuffer();

	MipMapTexture *createMipMapTexture() override;

	void drawSurface(const Graphics::Surface *surface, Common::Point dest, Common::Rect rect);

	void set3DMode();

private:
	void start2DMode();
	void end2DMode();
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // USE_OPENGL

#endif // STARK_GFX_OPENGL_H
