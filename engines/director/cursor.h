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
 */

#ifndef DIRECTOR_CURSOR_H
#define DIRECTOR_CURSOR_H

#include "graphics/maccursor.h"

#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {
class ManagedSurface;
class MacCursor;
}

namespace Director {

class Cursor : public Graphics::MacCursor {
 public:
	Cursor();

	void readFromCast(CastMemberID cursorId, CastMemberID maskId);
	void readFromResource(int resourceId);

	bool isEmpty() { return _cursorResId == 0 && _cursorCastId.member == 0; }
	bool operator==(const Cursor &c);

	virtual byte getKeyColor() const override { return _keyColor; }
	virtual const byte *getPalette() const override { return _usePalette ? _palette : nullptr; }

 public:
	Graphics::MacCursorType _cursorType;
	int _cursorResId;

	CastMemberID _cursorCastId;
	CastMemberID _cursorMaskId;

private:
	void resetCursor(Graphics::MacCursorType type, bool shouldClear = false, int resId = 0, CastMemberID castId = CastMemberID(0, 0), CastMemberID maskId = CastMemberID(0, 0));

private:
	bool _usePalette;
	byte _keyColor;
};

} // End of namespace Director

#endif
