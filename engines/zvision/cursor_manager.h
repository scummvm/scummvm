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
 */

#ifndef ZVISION_CURSOR_MANAGER_H
#define ZVISION_CURSOR_MANAGER_H

#include "common/types.h"

#include "zvision/cursor.h"


namespace Graphics {
struct PixelFormat;
}

namespace ZVision {

class ZVision;

/**
 * Class to manage cursor changes. The actual changes have to be done
 * through CursorMan. Otherwise the cursor will disappear after GMM
 * or debug console.
 * TODO: Figure out a way to get rid of the extraneous data copying due to having to use CursorMan
 */
class CursorManager {
public:
	CursorManager(ZVision *engine, const Graphics::PixelFormat *pixelFormat);

private:
	ZVision *_engine;
	const Graphics::PixelFormat *_pixelFormat;
	ZorkCursor _idleCursor;
	Common::String _currentCursor;
	bool _cursorIsPushed;

	static const char *_cursorNames[];
	static const char *_zgiCursorFileNames[];
	static const char *_zNemCursorFileNames[];

public:
	void initialize();

	void changeCursor(const Common::String &cursorName);
	void changeCursor(const Common::String &cursorName, bool pushed);
	void cursorDown(bool pushed);
	void revertToIdle();

private:
	void changeCursor(const ZorkCursor &cursor);
};

} // End of namespace ZVision

#endif
