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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GRAPHICS_PALETTE_H
#define SCI_GRAPHICS_PALETTE_H

#include "sci/graphics/helpers.h"

namespace Sci {

class Screen;
/**
 * Palette class, handles palette operations like changing intensity, setting up the palette, merging different palettes
 */
class GfxPalette {
public:
	GfxPalette(ResourceManager *resMan, GfxScreen *screen);
	~GfxPalette();

	void setDefault();
	void createFromData(byte *data, Palette *paletteOut);
	bool setAmiga();
	void modifyAmigaPalette(byte *data);
	void setEGA();
	void set(Palette *sciPal, bool force, bool forceRealMerge = false);
	bool merge(Palette *pFrom, bool force, bool forceRealMerge);
	uint16 matchColor(byte r, byte g, byte b);
	void getSys(Palette *pal);

	void setOnScreen();

	void increaseSysTimestamp();

	bool kernelSetFromResource(GuiResourceId resourceId, bool force);
	void kernelSetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void kernelUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void kernelSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette);
	int16 kernelFindColor(uint16 r, uint16 g, uint16 b);
	bool kernelAnimate(byte fromColor, byte toColor, int speed);
	void kernelAnimateSet();
	void kernelAssertPalette(GuiResourceId resourceId);

	void kernelPalVaryInit(GuiResourceId resourceId, uint16 ticks, uint16 stopPercentage, int16 direction);
	void kernelPalVaryToggle(bool pause);
	void kernelPalVaryDeinit();
	void palVaryUpdate();
	void palVaryProcess(int signal, bool setPalette);

	Palette _sysPalette;

private:
	void palVaryInit();
	static void palVaryCallback(void *refCon);
	void palVaryIncreaseSignal();

	GfxScreen *_screen;
	ResourceManager *_resMan;

	bool _sysPaletteChanged;
	bool _alwaysForceRealMerge;

	Common::Array<PalSchedule> _schedules;

	GuiResourceId _palVaryResourceId;
	Palette _palVaryOriginPalette;
	Palette _palVaryTargetPalette;
	uint16 _palVaryStep;
	uint16 _palVaryStepStop;
	int16 _palVaryDirection;
	int _palVaryPaused;
	int _palVarySignal;
};

} // End of namespace Sci

#endif
