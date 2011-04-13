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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/debugger.cpp $
 * $Id: debugger.cpp 223 2011-02-09 13:03:31Z dreammaster $
 *
 */

#include "tsage/debugger.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "tsage/globals.h"
#include "tsage/graphics.h"


namespace tSage {

Debugger::Debugger(): GUI::Debugger() {
	DCmd_Register("continue",		WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("scene",			WRAP_METHOD(Debugger, Cmd_Scene));
	DCmd_Register("walk_regions",	WRAP_METHOD(Debugger, Cmd_WalkRegions));
	DCmd_Register("priority_regions",	WRAP_METHOD(Debugger, Cmd_PriorityRegions));
	DCmd_Register("item",			WRAP_METHOD(Debugger, Cmd_Item));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

/**
 * This command loads up the specified new scene number
 */
bool Debugger::Cmd_Scene(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: %s <scene number> [prior scene #]\n", argv[0]);
		return true;
	} else {
		if (argc == 3)
			_globals->_sceneManager._sceneNumber = strToInt(argv[2]);

		_globals->_sceneManager.changeScene(strToInt(argv[1]));		
		return false;
	}
}

/**
 * This command draws the walk regions onto the screen
 */
bool Debugger::Cmd_WalkRegions(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	// Colour index to use for the first walk region
	int colour = 16;	

	// Lock the background surface for access
	Graphics::Surface destSurface = _globals->_sceneManager._scene->_backSurface.lockSurface();

	// Loop through drawing each walk region in a different colour to the background surface
	for (uint regionIndex = 0; regionIndex < _globals->_walkRegions._regionList.size(); ++regionIndex, ++colour) {
		WalkRegion &wr = _globals->_walkRegions._regionList[regionIndex];

		for (int yp = wr._bounds.top; yp < wr._bounds.bottom; ++yp) {
			LineSliceSet sliceSet = wr.getLineSlices(yp);

			for (uint idx = 0; idx < sliceSet.items.size(); ++idx)
				destSurface.hLine(sliceSet.items[idx].xs - _globals->_sceneOffset.x, yp,
				sliceSet.items[idx].xe - _globals->_sceneOffset.x, colour);
		}
	}

	// Release the surface
	_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	_globals->_paneRefreshFlag[0] = 2;

	return false;
}

/*
 * This command draws the priority regions onto the screen
 */
bool Debugger::Cmd_PriorityRegions(int argc, const char **argv) {
	int regionNum = 0;

	// Check for an optional specific region to display
	if (argc == 2)
		regionNum = strToInt(argv[1]);

	// Colour index to use for the first priority region
	int colour = 16;	
	int count = 0;

	// Lock the background surface for access
	Graphics::Surface destSurface = _globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = _globals->_sceneManager._scene->_priorities.begin();
	Common::String regionsDesc;

	for (; i != _globals->_sceneManager._scene->_priorities.end(); ++i, ++colour, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(_globals->_sceneManager._scene->_sceneBounds.left + x,
							_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = colour;
					++destP;
				}
			}
		}

		regionsDesc += Common::String::format("Region Priority = %d bounds=%d,%d,%d,%d\n", 
			r._regionId, r._bounds.left, r._bounds.top, r._bounds.right, r._bounds.bottom);
	}

	// Release the surface
	_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	_globals->_paneRefreshFlag[0] = 2;

	DebugPrintf("Total regions = %d\n", count);
	DebugPrintf("%s", regionsDesc.c_str());

	return true;
}

/**
 * Give a specified item to the player
 */
bool Debugger::Cmd_Item(int argc, const char **argv) {
	_globals->_inventory._infoDisk._sceneNumber = 1;
	return true;
}

} // End of namespace tSage
