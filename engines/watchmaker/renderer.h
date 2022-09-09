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
	unsigned int bpp;
};

class Renderer {
	WorkDirs *_workDirs;
public:
	Rect _viewport;
	Renderer(WorkDirs *workDirs, sdl_wrapper *wrapper) : sdl(wrapper), _workDirs(workDirs) {}
	MaterialPtr addMaterial(MaterialPtr MList, const Common::String &name, int NumFaces, unsigned int LoaderFlags);

	void initGL();

	void showFrame();

	void setVirtualScreen(unsigned int dimX, unsigned int dimY);
	void getScreenInfos(unsigned int &width, unsigned int &height, unsigned int &bpp) const;
	WindowInfo getScreenInfos() const;
	bool createScreenBuffer();
	bool initBlitterViewPort();

	void setCurCameraViewport(t3dF32 fov, uint8 sup);

	void renderBitmap(SDDBitmap &bitmap);

	int rFitX(int x);
	int rFitY(int y);
	int rInvFitX(int x);
	int rInvFitY(int y);
private:
	sdl_wrapper *sdl;
	// aspect correction
	float gAspectX = 1, gAspectY = 1;
	float gInvAspectX = 1, gInvAspectY = 1;
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_RENDERER_H
