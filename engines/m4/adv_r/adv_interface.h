
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

#ifndef M4_ADV_R_ADV_INTERFACE_H
#define M4_ADV_R_ADV_INTERFACE_H

#include "m4/m4_types.h"

namespace M4 {

#define INTERFACE_SPRITES 22

class Interface {
public:
	bool _shown = false;
	bool _visible = false;
	int _x1 = 0, _y1 = 0, _x2 = 0, _y2 = 0;

	int _arrow = 0;
	int _wait = 0;
	int _look = 0;
	int _grab = 0;
	int _use = 0;

public:
	virtual ~Interface() {
	}

	virtual bool init(int arrow, int wait, int look, int grab, int use);

	virtual void cancel_sentence() = 0;

	virtual void freshen_sentence() = 0;

	virtual bool set_interface_palette(RGB8 *myPalette) = 0;

	virtual bool eventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) = 0;

	virtual void track_hotspots_refresh() = 0;

	/**
	 * Show the interface
	 */
	virtual void show();

	/**
	 * Hide the interface
	 */
	void hide();

	/**
	 * Show the wait cursor
	 */
	void showWaitCursor();
};

void interface_hide();
void interface_show();

void track_hotspots_refresh();
bool intr_EventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z);

void intr_cancel_sentence();
void intr_freshen_sentence();
void intr_freshen_sentence(int cursor);

} // End of namespace M4

#endif
