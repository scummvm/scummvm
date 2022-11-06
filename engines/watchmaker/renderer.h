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

#ifndef WATCHMAKER_RENDERER_H
#define WATCHMAKER_RENDERER_H

#include "math/matrix4.h"
#include "watchmaker/utils.h"
#include "watchmaker/sdl_wrapper.h"
#include "watchmaker/types.h"
#include "watchmaker/struct.h"
#include "watchmaker/work_dirs.h"
#include "watchmaker/rect.h"

namespace Watchmaker {

class sdl_wrapper;
struct gMaterial;

struct WindowInfo {
	unsigned int width;
	unsigned int height;
};

class Fonts;
class Renderer {
	WorkDirs *_workDirs;
	WGame *_game;
public:
	Fonts *_fonts = nullptr;
	Rect _viewport;
	Renderer(WGame *game, sdl_wrapper *wrapper);
	bool addMaterial(gMaterial &material, const Common::String &name, int NumFaces, unsigned int LoaderFlags);

	void initGL();

	void showFrame();

	void setVirtualScreen(unsigned int dimX, unsigned int dimY);
	void getScreenInfos(unsigned int &width, unsigned int &height) const;
	WindowInfo getScreenInfos() const;
	bool createScreenBuffer();
	void initBlitterViewPort();

	void setCurCameraViewport(t3dF32 fov, uint8 sup);

	void renderBitmap(SDDBitmap &bitmap);
	void printText(const char *s, unsigned int dst, FontKind font, FontColor color, uint16 x, uint16 y);

	int rFitX(int x);
	int rFitY(int y);
	int rInvFitX(int x);
	int rInvFitY(int y);

	static const int MAX_BITMAP_LIST = 1024;
	struct BitmapList {
		unsigned int _numBitmaps = 0;
		gTexture bitmaps[MAX_BITMAP_LIST];

		unsigned int acquirePosition();
	};
	BitmapList _bitmapList;
	void clearBitmap(int dst, int dposx, int dposy, int sdimx, int sdimy, unsigned char r, unsigned char g, unsigned char b);
	void blitScreenBuffer();
	unsigned int getBitmapDimX(int32 id) const;
	unsigned int getBitmapDimY(int32 id) const;
	unsigned int getBitmapRealDimX(int32 id) const;
	unsigned int getBitmapRealDimY(int32 id) const;

	bool setProjectionMatrix(float width, float height, float fAspect, float fNearPlane, float fFarPlane);
	Math::Vector3d screenSpaceToCameraSpace(float x, float y);
private:
	sdl_wrapper *sdl;
	// aspect correction
	float gAspectX = 1, gAspectY = 1;
	float gInvAspectX = 1, gInvAspectY = 1;
	float _nearPlane;
	Math::Matrix4 _projectionMatrix;
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_RENDERER_H
