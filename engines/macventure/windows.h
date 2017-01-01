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

#ifndef MACVENTURE_WINDOWS_H
#define MACVENTURE_WINDOWS_H

#include "common/rect.h"
#include "common/array.h"

namespace MacVenture {
// massive HACK
typedef uint32 ObjID;

enum WindowReference {
	kNoWindow = 0,
	kInventoryStart = 1,
	kCommandsWindow = 0x80,
	kMainGameWindow = 0x81,
	kOutConsoleWindow = 0x82,
	kSelfWindow = 0x83,
	kExitsWindow = 0x84,
	kDiplomaWindow = 0x85
};

enum MVWindowType {
	kDocument = 0x00,
	kDBox = 0x01,
	kPlainDBox = 0x02,
	kAltBox = 0x03,
	kNoGrowDoc = 0x04,
	kMovableDBox = 0x05,
	kZoomDoc = 0x08,
	kZoomNoGrow = 0x0c,
	// WebVenture assigns arbitrary kinds post-loading
	kInvWindow = 0x0e,
	kRDoc16 = 0x10,
	kRDoc4 = 0x12,
	kRDoc6 = 0x14,
	kRDoc10 = 0x16,
	kNoType = 0xFF
};

struct DrawableObject {
	ObjID obj;
	byte mode;
	DrawableObject(ObjID id, byte md) {
		obj = id;
		mode = md;
	}
};

enum {
	kScrollAmount = 10
};

struct WindowData {
	Common::Rect bounds;
	MVWindowType type;
	ObjID objRef;
	uint16 visible;
	uint16 hasCloseBox;
	WindowReference refcon;
	uint8 titleLength;
	Common::String title;
	Common::Array<DrawableObject> children;
	bool updateScroll;
	Common::Point scrollPos;
};

struct BorderBounds {
	uint16 leftOffset;
	uint16 topOffset;
	uint16 rightOffset;
	uint16 bottomOffset;

	BorderBounds(uint16 l, uint16 t, uint16 r, uint16 b) :
	leftOffset(l), topOffset(t), rightOffset(r), bottomOffset(b) {}
};
}
#endif
