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

//#include "engines/stark/color.h"
#include "graphics/surface.h"
#include "common/rect.h"
#include "math/vector3d.h"

namespace Stark {

class GfxDriver {
public:
	static GfxDriver *create();

	virtual ~GfxDriver() {}

	virtual const char *getVideoDeviceName() = 0;

	virtual void setupScreen(int screenW, int screenH, bool fullscreen) = 0;

	virtual void setupPerspective(float fov, float nearClipPlane, float farClipPlane) = 0;
	virtual void setupCamera(const Math::Vector3d &position, const Math::Vector3d &lookAt) = 0;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void drawSurface(const Graphics::Surface *surface, Common::Point dest = Common::Point(), Common::Rect rect = Common::Rect()) = 0;

	/*
	virtual bool isHardwareAccelerated() = 0;
*/
	virtual void set3DMode() = 0;
/*
	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = 0;
	virtual void positionCamera(Math::Vector3d pos, Math::Vector3d interest) = 0;

	virtual void translateViewpointStart(Math::Vector3d pos, float pitch, float yaw, float roll) = 0;
	virtual void translateViewpointFinish() = 0;

	virtual void disableLights() = 0;
	virtual void setupLight(Scene::Light *light, int lightId) = 0;

	virtual void createMaterial(Material *material, const char *data, const CMap *cmap) = 0;
	virtual void selectMaterial(const Material *material) = 0;
	virtual void destroyMaterial(Material *material) = 0;

	virtual void createBitmap(Bitmap *bitmap) = 0;
	virtual void drawBitmap(const Bitmap *bitmap) = 0;
	virtual void destroyBitmap(Bitmap *bitmap) = 0;

	virtual void drawDepthBitmap(int x, int y, int w, int h, char *data) = 0;

//	virtual Bitmap *getScreenshot(int w, int h) = 0;
	virtual void storeDisplay() = 0;
	virtual void copyStoredToDisplay() = 0;
	virtual void dimScreen() = 0;
	virtual void dimRegion(int x, int y, int w, int h, float level) = 0;

	virtual void drawRectangle(PrimitiveObject *primitive) = 0;
	virtual void drawLine(PrimitiveObject *primitive) = 0;
	virtual void drawPolygon(PrimitiveObject *primitive) = 0;

	virtual void prepareSmushFrame(int width, int height, byte *bitmap) = 0;
	virtual void drawSmushFrame(int offsetX, int offsetY) = 0;
	virtual void releaseSmushFrame() = 0;
	*/

protected:
	int _screenWidth, _screenHeight, _screenBPP;
};

} // End of namespace Stark

#endif // STARK_GFX_DRIVER_H
