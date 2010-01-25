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
class SciPalette {
public:
	SciPalette(ResourceManager *resMan, Screen *screen, bool autoSetPalette = true);
	~SciPalette();

	void createFromData(byte *data, Palette *paletteOut);
	bool setAmiga();
	int16 mapAmigaColor(int16 color);
	void modifyAmigaPalette(byte *data);
	void setEGA();
	bool setFromResource(GuiResourceId resourceId, uint16 flag);
	void set(Palette *sciPal, uint16 flag);
	void merge(Palette *pFrom, Palette *pTo, uint16 flag);
	uint16 matchColor(Palette *pPal, byte r, byte g, byte b);
	void getSys(Palette *pal);

	void setOnScreen();

	void setFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void unsetFlag(uint16 fromColor, uint16 toColor, uint16 flag);
	void setIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette);
	bool animate(byte fromColor, byte toColor, int speed);

	Palette _sysPalette;

private:
	Screen *_screen;
	ResourceManager *_resMan;

	Common::Array<PalSchedule> _schedules;

	byte _amigaEGAtable[16];
};

} // End of namespace Sci

#endif
