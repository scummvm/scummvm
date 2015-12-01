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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/events.h"

#ifndef LAB_MOUSE_H
#define LAB_MOUSE_H

namespace Lab {

struct Gadget;

class LabEngine;

class EventManager {
private:
	LabEngine *_vm;
	bool _leftClick;
	bool _rightClick;

	bool _mouseHidden;
	int32 _numHidden;
	Gadget *_lastGadgetHit;

public:
	EventManager (LabEngine *vm);

	Gadget *_screenGadgetList;

	Gadget *checkGadgetHit(Gadget *gadgetList, Common::Point pos);
	void initMouse();
	void updateMouse();
	void mouseShow();
	void mouseHide();
	Common::Point getMousePos();
	void setMousePos(Common::Point pos);
	bool mouseButton(uint16 *x, uint16 *y, bool leftButton);
	Gadget *mouseGadget();
	void attachGadgetList(Gadget *gadgetList);
	void mouseHandler(int flag, Common::Point pos);
};

} // End of namespace Lab

#endif /* LAB_MOUSE_H */
