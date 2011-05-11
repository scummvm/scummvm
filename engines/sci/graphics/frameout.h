/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_GRAPHICS_FRAMEOUT_H
#define SCI_GRAPHICS_FRAMEOUT_H

namespace Sci {

class GfxPicture;

struct PlaneEntry {
	reg_t object;
	uint16 priority;
	uint16 lastPriority;
	int16 planeOffsetX;
	GuiResourceId pictureId;
	Common::Rect planeRect;
	Common::Rect planeClipRect;
	Common::Rect upscaledPlaneRect;
	Common::Rect upscaledPlaneClipRect;
	bool planePictureMirrored;
	byte planeBack;
};

typedef Common::List<PlaneEntry> PlaneList;

struct FrameoutEntry {
	uint16 givenOrderNr;
	reg_t object;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 x, y, z;
	int16 priority;
	uint16 signal;
	uint16 scaleSignal;
	int16 scaleX;
	int16 scaleY;
	Common::Rect celRect;
	GfxPicture *picture;
	int16 picStartX;
};

typedef Common::List<FrameoutEntry *> FrameoutList;

struct PlanePictureEntry {
	reg_t object;
	int16 startX;
	GuiResourceId pictureId;
	GfxPicture *picture;
	FrameoutEntry *pictureCels; // temporary
};

typedef Common::List<PlanePictureEntry> PlanePictureList;

class GfxCache;
class GfxCoordAdjuster32;
class GfxPaint32;
class GfxPalette;
class GfxScreen;
/**
 * Frameout class, kFrameout and relevant functions for SCI32 games
 */
class GfxFrameout {
public:
	GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32);
	~GfxFrameout();

	void kernelAddPlane(reg_t object);
	void kernelUpdatePlane(reg_t object);
	void kernelRepaintPlane(reg_t object);
	void kernelDeletePlane(reg_t object);
	void kernelAddScreenItem(reg_t object);
	void kernelUpdateScreenItem(reg_t object);
	void kernelDeleteScreenItem(reg_t object);
	int16 kernelGetHighPlanePri();
	void kernelAddPicAt(reg_t planeObj, int16 forWidth, GuiResourceId pictureId);
	void kernelFrameout();

	void addPlanePicture(reg_t object, GuiResourceId pictureId, uint16 startX);
	void deletePlanePictures(reg_t object);
	void clear();

private:
	SegManager *_segMan;
	ResourceManager *_resMan;
	GfxCoordAdjuster32 *_coordAdjuster;
	GfxCache *_cache;
	GfxPalette *_palette;
	GfxScreen *_screen;
	GfxPaint32 *_paint32;

	FrameoutList _screenItems;
	PlaneList _planes;
	PlanePictureList _planePictures;

	void sortPlanes();

	uint16 scriptsRunningWidth;
	uint16 scriptsRunningHeight;
};

} // End of namespace Sci

#endif
