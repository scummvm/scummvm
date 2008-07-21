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

#ifndef PARALLACTION_INPUT_H
#define PARALLACTION_INPUT_H

#include "parallaction/objects.h"
#include "parallaction/inventory.h"

namespace Parallaction {

enum {
	kMouseNone			= 0,
	kMouseLeftUp		= 1,
	kMouseLeftDown		= 2,
	kMouseRightUp		= 4,
	kMouseRightDown		= 8
};

struct InputData {
	uint16			_event;
	Common::Point	_mousePos;
	int16		_inventoryIndex;
	ZonePtr		_zone;
	uint		_label;
};

class Input {
	void updateGameInput();
	void updateCommentInput();

	// input-only
	InputData	_inputData;
	bool		_actionAfterWalk;  // actived when the character needs to move before taking an action
	// these two could/should be merged as they carry on the same duty in two member functions,
	// respectively processInput and translateInput
	int16		_transCurrentHoverItem;

	InputData	*translateInput();
	bool		translateGameInput();
	bool		translateInventoryInput();

	Parallaction	*_vm;

	Common::Point	_mousePos;
	uint16	_mouseButtons;
	int32	_lastKeyDownAscii;

	bool		_mouseHidden;
	ZonePtr			_hoverZone;

public:
	enum {
		kInputModeGame = 0,
		kInputModeComment = 1,
		kInputModeDialogue = 2
	};


	Input(Parallaction *vm) : _vm(vm) {
		_transCurrentHoverItem = 0;
		_actionAfterWalk = false;  // actived when the character needs to move before taking an action
		_mouseHidden = false;
		_activeItem._index = 0;
		_activeItem._id = 0;
		_mouseButtons = 0;
	}

	virtual ~Input() { }


	void		showCursor(bool visible);
	void			getCursorPos(Common::Point& p) {
		p = _mousePos;
	}

	int				_inputMode;
	InventoryItem	_activeItem;

	uint16	readInput();
	InputData* 	updateInput();
	void	trackMouse(ZonePtr z);
	void 	waitUntilLeftClick();
	void	waitForButtonEvent(uint32 buttonEventMask, int32 timeout = -1);
	uint32	getLastButtonEvent() { return _mouseButtons; }
	bool  	getLastKeyDown(uint16 &ascii);

	void stopHovering();
};

} // namespace Parallaction

#endif
