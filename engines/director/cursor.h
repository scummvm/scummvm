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
 */

#ifndef DIRECTOR_CURSOR_H
#define DIRECTOR_CURSOR_H

#include "graphics/maccursor.h"

#include "graphics/macgui/macwindowmanager.h"

#include "engines/director/lingo/lingo.h"

namespace Graphics {
class ManagedSurface;
class MacCursor;
}

namespace Director {

class Archive;
struct CursorRef;

class Cursor : public Graphics::MacCursor {
 public:
	Cursor();

	CursorRef getRef();

	void readFromCast(Datum casts);
	void readFromResource(Datum resourceId);
	void readBuiltinType(Datum resourceId);
	bool readFromArchive(Archive *archive, uint16 resourceId);

	bool isEmpty() {return Datum(0).equalTo(_cursorResId);}
	bool operator==(const Cursor &c);
	bool operator==(const CursorRef &c);

	byte getKeyColor() const override { return _keyColor; }
	const byte *getPalette() const override { return _usePalette ? _palette.data : nullptr; }

 public:
	Graphics::MacCursorType _cursorType;
	Datum _cursorResId;

private:
	void resetCursor(Graphics::MacCursorType type, bool shouldClear = false, Datum resId = Datum(0));

private:
	bool _usePalette;
	byte _keyColor;
};

// CursorRef acts as a reference to a cursor.
// Doesn't contain a surface, palette, etc. like the cursor itself.
struct CursorRef {
	CursorRef();
	bool operator==(const Cursor &c);
	bool operator==(const CursorRef &c);

	Graphics::MacCursorType _cursorType;
	Datum _cursorResId;
};

} // End of namespace Director

#endif
