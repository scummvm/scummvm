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

#ifndef SCI_GUI_PALETTE_H
#define SCI_GUI_PALETTE_H

#include "sci/gui/gui_helpers.h"

namespace Sci {

class SciGuiScreen;
class SciGuiPalette {
public:
	SciGuiPalette(ResourceManager *resMan, SciGuiScreen *screen, bool autoSetPalette = true);
	~SciGuiPalette();

	void createFromData(byte *data, GuiPalette *paletteOut);
	bool setAmiga();
	void setEGA();
	bool setFromResource(int16 resourceNo, int16 flag);
	void set(GuiPalette *sciPal, int16 flag);
	void merge(GuiPalette *pFrom, GuiPalette *pTo, uint16 flag);
	uint16 matchColor(GuiPalette *pPal, byte r, byte g, byte b);
	void getSys(GuiPalette *pal);

	void setOnScreen();

	void setIntensity(int fromColor, int toColor, int intensity, GuiPalette *destPalette);
	void animate(byte fromColor, byte toColor, int speed);

	GuiPalette _sysPalette;

private:
	SciGuiScreen *_screen;
	ResourceManager *_resMan;

	uint16 _clrPowers[256];

	Common::Array<GuiPalSchedule> _schedules;
};

} // End of namespace Sci

#endif
