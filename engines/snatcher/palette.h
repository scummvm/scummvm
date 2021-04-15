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

#ifndef SNATCHER_PALETTE_H
#define SNATCHER_PALETTE_H

#include "common/platform.h"

class PaletteManager;

namespace Snatcher {

class Palette {
public:
	Palette(PaletteManager *pm) : _palMan(pm) {}
	virtual ~Palette() {}

	virtual bool enqueueEvent(const uint8*, uint32) { return true; }
	virtual void processEventQueue() {}

protected:
	PaletteManager *_palMan;

private:
	static Palette *createSegaPalette(PaletteManager *pm);

public:
	static Palette *create(PaletteManager *pm, Common::Platform platform) {
		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSegaPalette(pm);
		default:
			break;
		};
		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_PALETTE_H
