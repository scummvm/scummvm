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

#ifndef STARK_GFX_DRIVER_H
#define STARK_GFX_DRIVER_H

#include "common/rect.h"
#include "graphics/surface.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Stark {

class GfxDriver {
public:
	static GfxDriver *create();

	virtual ~GfxDriver() {}

	virtual void setupScreen(int screenW, int screenH, bool fullscreen) = 0;

	virtual void setGameViewport() = 0;
	virtual void setScreenViewport() = 0;

	virtual void setupPerspective(const Math::Matrix4 &projectionMatrix) = 0;
	virtual void setupCamera(const Math::Vector3d &position, const Math::Matrix4 &lookAt) = 0;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void drawSurface(const Graphics::Surface *surface, Common::Point dest = Common::Point(), Common::Rect rect = Common::Rect()) = 0;

	virtual void set3DMode() = 0;

	static const int32 kTopBorderHeight = 36;
	static const int32 kGameViewportHeight = 365;

protected:
	int _screenWidth;
	int _screenHeight;
};

} // End of namespace Stark

#endif // STARK_GFX_DRIVER_H
