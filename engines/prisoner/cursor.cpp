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

#include "graphics/cursorman.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Mouse cursor */

/*
DEFAULT_CURSOR=S_CURSOR.PUK 0 5
MENU_CURSOR=S_CURSOR.PUK 0 6
LOOK_CURSOR=S_CURSOR.PUK 0 5
MOVE_CURSOR=S_CURSOR.PUK 0 5
DOOR_CURSOR=S_CURSOR.PUK 0 3
READ_CURSOR=S_CURSOR.PUK 0 1
TALK_CURSOR=S_CURSOR.PUK 0 2
ACT_CURSOR=S_CURSOR.PUK 0 5
DISK_CURSOR=S_CURSOR.PUK 0 0
IDM_CURSOR=S_CURSOR.PUK 0 7
AUTOMATIC_CURSOR=S_CURSOR.PUK 0 4
DEBUG_CURSOR=S_DEBUG.PUK 0 0
ANM_CURSOR=S_DEBUG.PUK 0 1
*/

void PrisonerEngine::loadMouseCursors() {

	// TODO: Use cursor definitions from the Exe instead of these hackishly hardcoded values

	_mouseCursors[0].key = "DEFAULT_CURSOR=";
	_mouseCursors[0].pakSlot = 0;
	_mouseCursors[0].frameListIndex = 5;
	_mouseCursors[1].key = "LOOK_CURSOR=";
	_mouseCursors[1].pakSlot = 0;
	_mouseCursors[1].frameListIndex = 5;
	_mouseCursors[2].key = "MOVE_CURSOR=";
	_mouseCursors[2].pakSlot = 0;
	_mouseCursors[2].frameListIndex = 5;
	_mouseCursors[3].key = "DOOR_CURSOR=";
	_mouseCursors[3].pakSlot = 0;
	_mouseCursors[3].frameListIndex = 3;
	_mouseCursors[4].key = "READ_CURSOR=";
	_mouseCursors[4].pakSlot = 0;
	_mouseCursors[4].frameListIndex = 1;
	_mouseCursors[5].key = "ACT_CURSOR=";
	_mouseCursors[5].pakSlot = 0;
	_mouseCursors[5].frameListIndex = 5;
	_mouseCursors[6].key = "TALK_CURSOR=";
	_mouseCursors[6].pakSlot = 0;
	_mouseCursors[6].frameListIndex = 2;
	_mouseCursors[7].key = "DISK_CURSOR=";
	_mouseCursors[7].pakSlot = 0;
	_mouseCursors[7].frameListIndex = 0;
	_mouseCursors[8].key = "IDM_CURSOR=";
	_mouseCursors[8].pakSlot = 0;
	_mouseCursors[8].frameListIndex = 7;
	_mouseCursors[9].key = "AUTOMATIC_CURSOR=";
	_mouseCursors[9].pakSlot = 0;
	_mouseCursors[9].frameListIndex = 4;
	_mouseCursors[10].pakSlot = -1;
	_mouseCursors[11].pakSlot = -1;
	_mouseCursors[12].pakSlot = -1;
	_mouseCursors[13].key = "MENU_CURSOR=";
	_mouseCursors[13].pakSlot = 0;
	_mouseCursors[13].frameListIndex = 6;

	Common::String pakName = "S_CURSOR";
	for (int16 i = 0; i < kMouseCursors; i++) {
		MouseCursor *mouseCursor = &_mouseCursors[i];
		if (mouseCursor->pakSlot != -1) {
			mouseCursor->resourceCacheSlot = _res->load<AnimationResource>(pakName, mouseCursor->pakSlot, 17);
		} else {
			mouseCursor->resourceCacheSlot = -1;
		}
	}

}

Graphics::Surface *PrisonerEngine::decompressAnimationCel(AnimationCel *animationCel) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(animationCel->width, animationCel->height, 1);
	byte *src = animationCel->data;
	byte *dst = (byte*)surface->pixels;
	int height = animationCel->height;
	while (height--) {
		byte *row = dst;
		byte chunks = *src++;
		while (chunks--) {
			byte skip = src[0];
			int count = src[1] * 4 + src[2];
			src += 3;
			row += skip;
			memcpy(row, src, count);
			row += count;
			src += count;
		}
		src++;
		dst += surface->pitch;
	}
	return surface;
}

void PrisonerEngine::setMouseCursor(int16 elementIndex, AnimationResource *animationResource) {
	AnimationElement *element = animationResource->_elements[elementIndex];
	AnimationCommand *command = element->commands[0];
	AnimationCel *animationCel = animationResource->_cels[command->argAsInt16()];
	int16 hotspotX = CLIP(-command->points[0].x, 0, animationCel->width - 1);
	int16 hotspotY = CLIP(animationCel->height - command->points[0].y - 1, 0, animationCel->height - 1);
	Graphics::Surface *surface = decompressAnimationCel(animationCel);
	CursorMan.replaceCursor((const byte *)surface->pixels, surface->w, surface->h, hotspotX, hotspotY, 0);
	delete surface;
	CursorMan.showMouse(true);
}

void PrisonerEngine::updateMouseCursor() {

	bool doSetCursor = false, doClearTextArea = false;

	if (_menuMouseCursorActive) {
		if (_menuMouseCursor != _currMouseCursor) {
			resetMouseCursorValues();
			_currMouseCursor = _menuMouseCursor;
			doSetCursor = true;
		}
	} else if (_dialogRunning) {
		if (_currMouseCursor != kCursorDialog) {
			resetMouseCursorValues();
			_currMouseCursor = kCursorDialog;
			doSetCursor = true;
			doClearTextArea = true;
		}
	} else if (_screenTextShowing) {
		if (_currMouseCursor != kCursorRead && _currMouseCursor != kCursorTalk) {
			resetMouseCursorValues();
			if (_talkieEnabled && _screenTextHasSpeech) {
				_currMouseCursor = kCursorTalk;
			} else {
				_currMouseCursor = kCursorRead;
			}
			_currAnimatedMouseCursor = _currMouseCursor;
			doSetCursor = true;
			doClearTextArea = true;
		}
	} else if (_userInputCounter > 0) {
		if (_currMouseCursor != kCursorAutomatic) {
			resetMouseCursorValues();
			_currMouseCursor = kCursorAutomatic;
			_currAnimatedMouseCursor = kCursorAutomatic;
			doSetCursor = true;
			doClearTextArea = true;
		}
	} else if (_zoneMouseCursorActive) {
		if (_zoneMouseCursor != _currMouseCursor) {
			resetMouseCursorValues();
			_currMouseCursor = _zoneMouseCursor;
			_currAnimatedMouseCursor = _currMouseCursor;
			doSetCursor = true;
		}
	} else if (_inventoryItemCursor >= 0) {
		if (_inventoryItemCursor != _currInventoryItemCursor) {
			resetMouseCursorValues();
			_currInventoryItemCursor =_inventoryItemCursor;
			_currMouseCursor = kCursorItem;
			doSetCursor = true;
		}
	} else if (_currMouseCursor != kCursorDefault) {
		resetMouseCursorValues();
		_currMouseCursor = kCursorDefault;
		_currAnimatedMouseCursor = kCursorDefault;
		doSetCursor = true;
	}

	if (doSetCursor) {
		AnimationResource *animationResource;
		MouseCursor *mouseCursor;
		int16 frameListIndex;
		if (doClearTextArea) {
			_screen->fillRect(0, 398, 639, 479, 0);
			addDirtyRect(0, 398, 640, 82, 1);
		}
		if (_currAnimatedMouseCursor != _currMouseCursor) {
			// The mouse cursor is not animated
			if (_currMouseCursor != kCursorItem) {
				mouseCursor = &_mouseCursors[_currMouseCursor];
				animationResource = _res->get<AnimationResource>(mouseCursor->resourceCacheSlot);
				frameListIndex = mouseCursor->frameListIndex;
			} else {
				InventoryItem *inventoryItem = &_inventoryItems[_currInventoryItemCursor];
				animationResource = _res->get<AnimationResource>(inventoryItem->resourceCacheSlot);
				frameListIndex = inventoryItem->id + 1;
			}
		} else {
			// The mouse cursor is animated
			mouseCursor = &_mouseCursors[_currMouseCursor];
			animationResource = _res->get<AnimationResource>(mouseCursor->resourceCacheSlot);
			frameListIndex = mouseCursor->frameListIndex;
			_mouseCursorAnimationResourceCacheSlot = mouseCursor->resourceCacheSlot;
			_mouseCursorAnimationFrameListIndex = mouseCursor->frameListIndex;
			_mouseCursorAnimationCurrFrame = 1;
			_mouseCursorAnimationLastUpdate = getTicks();
			_mouseCursorAnimationUpdateTicks = animationResource->_anims[frameListIndex]->frames[0]->ticks;
			_mouseCursorAnimationFrameCount = animationResource->_anims[frameListIndex]->frames.size();
		}
		setMouseCursor(animationResource->_anims[frameListIndex]->frames[0]->elementIndex, animationResource);
	}

}

void PrisonerEngine::updateMouseCursorAnimation() {

	if (_currMouseCursor == _currAnimatedMouseCursor && _mouseCursorAnimationFrameCount > 1 &&
		_mouseCursorAnimationLastUpdate + _mouseCursorAnimationUpdateTicks < getTicks()) {

		if (_mouseCursorAnimationCurrFrame >= _mouseCursorAnimationFrameCount)
			_mouseCursorAnimationCurrFrame = 0;

		AnimationResource *animationResource = _res->get<AnimationResource>(_mouseCursorAnimationResourceCacheSlot);
		_mouseCursorAnimationLastUpdate = getTicks();
		_mouseCursorAnimationUpdateTicks = animationResource->_anims[_mouseCursorAnimationFrameListIndex]->
			frames[_mouseCursorAnimationCurrFrame]->ticks;
		setMouseCursor(animationResource->_anims[_mouseCursorAnimationFrameListIndex]->
			frames[_mouseCursorAnimationCurrFrame]->elementIndex, animationResource);
		_mouseCursorAnimationCurrFrame++;
	}

}

void PrisonerEngine::resetMouseCursorValues() {
	_currAnimatedMouseCursor = -1;
	_currInventoryItemCursor = -1;
}

} // End of namespace Prisoner
