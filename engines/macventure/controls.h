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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_CONTROLS_H
#define MACVENTURE_CONTROLS_H

namespace MacVenture {

class Gui;

enum ControlType { // HACK, should correspond exactly with the types of controls (sliders etc)
	kControlExitBox = 0,
	kControlExamine = 1,
	kControlOpen = 2,
	kControlClose = 3,
	kControlSpeak = 4,
	kControlOperate = 5,
	kControlGo = 6,
	kControlHit = 7,
	kControlConsume = 8,
	kControlClickToContinue = 9
};

enum ControlAction {
	kNoCommand = 0,
	kStartOrResume = 1,
	kClose = 2,
	kTick = 3,
	kActivateObject = 4,
	kMoveObject = 5,
	kConsume = 6,
	kExamine = 7,
	kGo = 8,
	kHit = 9,
	kOpen = 10,
	kOperate = 11,
	kSpeak = 12,
	kBabble = 13,
	kTargetName = 14,
	kDebugObject = 15,
	kClickToContinue = 16
};

struct ControlData {
	Common::Rect bounds;
	uint16 scrollValue;
	uint8 visible;
	uint16 scrollMax;
	uint16 scrollMin;
	uint16 cdef;
	ControlAction refcon;
	ControlType type;
	uint8 titleLength;
	Common::String title;
	uint16 border;
};

class CommandButton {

enum {
	kCommandsLeftPadding = 0,
	kCommandsTopPadding = 0
};

public:

	CommandButton();

	CommandButton(ControlData data, Gui *g);
	~CommandButton() {}

	void draw(Graphics::ManagedSurface &surface) const;
	bool isInsideBounds(const Common::Point point) const;
	const ControlData &getData() const;
	void select();
	void unselect();
	bool isSelected();

private:
	bool _selected;
	ControlData _data;
	Gui *_gui;
};

} // End of namespace MacVenture

#endif
