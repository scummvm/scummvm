/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_BIOCHIP_VIEW_H
#define BURIED_BIOCHIP_VIEW_H

#include "buried/window.h"

namespace Buried {

class BioChipMainViewWindow : public Window {
public:
	BioChipMainViewWindow(BuriedEngine *vm, Window *parent, int currentBioChipID = 0);
	~BioChipMainViewWindow();

	bool changeCurrentBioChip(int newBioChip);

	bool onSetCursor(uint message);

private:
	int _currentBioChipID;
	Window *_bioChipDisplayWindow;
	uint _oldCursor;

	Window *createBioChipSpecificViewWindow(int bioChipID);
};

} // End of namespace Buried

#endif
