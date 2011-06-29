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
 */

#include "tsage/debugger.h"
#include "tsage/globals.h"
#include "tsage/graphics.h"
#include "tsage/ringworld_logic.h"

namespace tSage {

Debugger::Debugger() : GUI::Debugger() {
	DCmd_Register("continue",		WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("scene",			WRAP_METHOD(Debugger, Cmd_Scene));
	DCmd_Register("walk_regions",	WRAP_METHOD(Debugger, Cmd_WalkRegions));
	DCmd_Register("priority_regions",	WRAP_METHOD(Debugger, Cmd_PriorityRegions));
	DCmd_Register("setflag",		WRAP_METHOD(Debugger, Cmd_SetFlag));
	DCmd_Register("getflag",		WRAP_METHOD(Debugger, Cmd_GetFlag));
	DCmd_Register("clearflag",		WRAP_METHOD(Debugger, Cmd_ClearFlag));
	DCmd_Register("listobjects",	WRAP_METHOD(Debugger, Cmd_ListObjects));
	DCmd_Register("moveobject",		WRAP_METHOD(Debugger, Cmd_MoveObject));
	DCmd_Register("hotspots",		WRAP_METHOD(Debugger, Cmd_Hotspots));
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
	}

	if (argc == 3)
		_globals->_sceneManager._sceneNumber = strToInt(argv[2]);

	_globals->_sceneManager.changeScene(strToInt(argv[1]));
	return false;
}

/**
 * This command draws the walk regions onto the screen
 */
bool Debugger::Cmd_WalkRegions(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	// Color index to use for the first walk region
	int color = 16;

	// Lock the background surface for access
	Graphics::Surface destSurface = _globals->_sceneManager._scene->_backSurface.lockSurface();

	// Loop through drawing each walk region in a different color to the background surface
	Common::String regionsDesc;

	for (uint regionIndex = 0; regionIndex < _globals->_walkRegions._regionList.size(); ++regionIndex, ++color) {
		WalkRegion &wr = _globals->_walkRegions._regionList[regionIndex];

		for (int yp = wr._bounds.top; yp < wr._bounds.bottom; ++yp) {
			LineSliceSet sliceSet = wr.getLineSlices(yp);

			for (uint idx = 0; idx < sliceSet.items.size(); ++idx)
				destSurface.hLine(sliceSet.items[idx].xs - _globals->_sceneOffset.x, yp,
				sliceSet.items[idx].xe - _globals->_sceneOffset.x, color);
		}

		regionsDesc += Common::String::format("Region #%d d bounds=%d,%d,%d,%d\n",
					regionIndex, wr._bounds.left, wr._bounds.top, wr._bounds.right, wr._bounds.bottom);
	}

	// Release the surface
	_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	_globals->_paneRefreshFlag[0] = 2;

	DebugPrintf("Total regions = %d\n", _globals->_walkRegions._regionList.size());
	DebugPrintf("%s\n", regionsDesc.c_str());

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

	// Color index to use for the first priority region
	int color = 16;
	int count = 0;

	// Lock the background surface for access
	Graphics::Surface destSurface = _globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = _globals->_sceneManager._scene->_priorities.begin();
	Common::String regionsDesc;

	for (; i != _globals->_sceneManager._scene->_priorities.end(); ++i, ++color, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(_globals->_sceneManager._scene->_sceneBounds.left + x,
							_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = color;
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

/*
 * This command sets a flag
 */
bool Debugger::Cmd_SetFlag(int argc, const char **argv) {
	// Check for a flag to set
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	_globals->setFlag(flagNum);
	return true;
}

/*
 * This command gets the value of a flag
 */
bool Debugger::Cmd_GetFlag(int argc, const char **argv) {
	// Check for an flag to display
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	DebugPrintf("Value: %d\n", _globals->getFlag(flagNum));
	return true;
}

/*
 * This command clears a flag
 */
bool Debugger::Cmd_ClearFlag(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	_globals->clearFlag(flagNum);
	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool Debugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available objects for this game are:\n");
	DebugPrintf("0 - Stunner\n");
	DebugPrintf("1 - Scanner\n");
	DebugPrintf("2 - Stasis Box\n");
	DebugPrintf("3 - Info Disk\n");
	DebugPrintf("4 - Stasis Negator\n");
	DebugPrintf("5 - Key Device\n");
	DebugPrintf("6 - Medkit\n");
	DebugPrintf("7 - Ladder\n");
	DebugPrintf("8 - Rope\n");
	DebugPrintf("9 - Key\n");
	DebugPrintf("10 - Translator\n");
	DebugPrintf("11 - Ale\n");
	DebugPrintf("12 - Paper\n");
	DebugPrintf("13 - Waldos\n");
	DebugPrintf("14 - Stasis Box 2\n");
	DebugPrintf("15 - Ring\n");
	DebugPrintf("16 - Cloak\n");
	DebugPrintf("17 - Tunic\n");
	DebugPrintf("18 - Candle\n");
	DebugPrintf("19 - Straw\n");
	DebugPrintf("20 - Scimitar\n");
	DebugPrintf("21 - Sword\n");
	DebugPrintf("22 - Helmet\n");
	DebugPrintf("23 - Items\n");
	DebugPrintf("24 - Concentrator\n");
	DebugPrintf("25 - Nullifier\n");
	DebugPrintf("26 - Peg\n");
	DebugPrintf("27 - Vial\n");
	DebugPrintf("28 - Jacket\n");
	DebugPrintf("29 - Tunic 2\n");
	DebugPrintf("30 - Bone\n");
	DebugPrintf("31 - Empty Jar\n");
	DebugPrintf("32 - Jar\n");
	return true;
}

/*
 * This command gets an item, or move it to a room
 */
bool Debugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		DebugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		DebugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	switch (objNum) {
	case OBJECT_STUNNER:
		RING_INVENTORY._stunner._sceneNumber = sceneNum;
		break;
	case OBJECT_SCANNER:
		RING_INVENTORY._scanner._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_BOX:
		RING_INVENTORY._stasisBox._sceneNumber = sceneNum;
		break;
	case OBJECT_INFODISK:
		RING_INVENTORY._infoDisk._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_NEGATOR:
		RING_INVENTORY._stasisNegator._sceneNumber = sceneNum;
		break;
	case OBJECT_KEY_DEVICE:
		RING_INVENTORY._keyDevice._sceneNumber = sceneNum;
		break;
	case OBJECT_MEDKIT:
		RING_INVENTORY._medkit._sceneNumber = sceneNum;
		break;
	case OBJECT_LADDER:
		RING_INVENTORY._ladder._sceneNumber = sceneNum;
		break;
	case OBJECT_ROPE:
		RING_INVENTORY._rope._sceneNumber = sceneNum;
		break;
	case OBJECT_KEY:
		RING_INVENTORY._key._sceneNumber = sceneNum;
		break;
	case OBJECT_TRANSLATOR:
		RING_INVENTORY._translator._sceneNumber = sceneNum;
		break;
	case OBJECT_ALE:
		RING_INVENTORY._ale._sceneNumber = sceneNum;
		break;
	case OBJECT_PAPER:
		RING_INVENTORY._paper._sceneNumber = sceneNum;
		break;
	case OBJECT_WALDOS:
		RING_INVENTORY._waldos._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_BOX2:
		RING_INVENTORY._stasisBox2._sceneNumber = sceneNum;
		break;
	case OBJECT_RING:
		RING_INVENTORY._ring._sceneNumber = sceneNum;
		break;
	case OBJECT_CLOAK:
		RING_INVENTORY._cloak._sceneNumber = sceneNum;
		break;
	case OBJECT_TUNIC:
		RING_INVENTORY._tunic._sceneNumber = sceneNum;
		break;
	case OBJECT_CANDLE:
		RING_INVENTORY._candle._sceneNumber = sceneNum;
		break;
	case OBJECT_STRAW:
		RING_INVENTORY._straw._sceneNumber = sceneNum;
		break;
	case OBJECT_SCIMITAR:
		RING_INVENTORY._scimitar._sceneNumber = sceneNum;
		break;
	case OBJECT_SWORD:
		RING_INVENTORY._sword._sceneNumber = sceneNum;
		break;
	case OBJECT_HELMET:
		RING_INVENTORY._helmet._sceneNumber = sceneNum;
		break;
	case OBJECT_ITEMS:
		RING_INVENTORY._items._sceneNumber = sceneNum;
		break;
	case OBJECT_CONCENTRATOR:
		RING_INVENTORY._concentrator._sceneNumber = sceneNum;
		break;
	case OBJECT_NULLIFIER:
		RING_INVENTORY._nullifier._sceneNumber = sceneNum;
		break;
	case OBJECT_PEG:
		RING_INVENTORY._peg._sceneNumber = sceneNum;
		break;
	case OBJECT_VIAL:
		RING_INVENTORY._vial._sceneNumber = sceneNum;
		break;
	case OBJECT_JACKET:
		RING_INVENTORY._jacket._sceneNumber = sceneNum;
		break;
	case OBJECT_TUNIC2:
		RING_INVENTORY._tunic2._sceneNumber = sceneNum;
		break;
	case OBJECT_BONE:
		RING_INVENTORY._bone._sceneNumber = sceneNum;
		break;
	case OBJECT_EMPTY_JAR:
		RING_INVENTORY._emptyJar._sceneNumber = sceneNum;
		break;
	case OBJECT_JAR:
		RING_INVENTORY._jar._sceneNumber = sceneNum;
		break;
	default:
		DebugPrintf("Invalid object Id %s\n", argv[1]);
		break;
	}

	return true;
}

/**
 * Show any active hotspot areas in the scene
 */
bool Debugger::Cmd_Hotspots(int argc, const char **argv) {
	int colIndex = 16;
	const Rect &sceneBounds = _globals->_sceneManager._scene->_sceneBounds;

	// Lock the background surface for access
	Graphics::Surface destSurface = _globals->_sceneManager._scene->_backSurface.lockSurface();

	// Iterate through the scene items
	SynchronizedList<SceneItem *>::iterator i;
	for (i = _globals->_sceneItems.reverse_begin(); i != _globals->_sceneItems.end(); --i, ++colIndex) {
		SceneItem *o = *i;

		// Draw the contents of the hotspot area
		if (o->_sceneRegionId == 0) {
			// Scene item doesn't use a region, so fill in the entire area
			destSurface.fillRect(Rect(o->_bounds.left - sceneBounds.left, o->_bounds.top - sceneBounds.top,
				o->_bounds.right - sceneBounds.left - 1, o->_bounds.bottom - sceneBounds.top - 1), colIndex);
		} else {
			// Scene uses a region, so get it and use it to fill out only the correct parts
			SceneRegions::iterator ri = _globals->_sceneRegions.begin();
			while ((ri != _globals->_sceneRegions.end()) && ((*ri)._regionId != o->_sceneRegionId))
				++ri;

			if (ri != _globals->_sceneRegions.end()) {
				// Fill out the areas defined by the region
				Region &r = *ri;

				for (int y = r._bounds.top; y < r._bounds.bottom; ++y) {
					LineSliceSet set = r.getLineSlices(y);

					for (uint p = 0; p < set.items.size(); ++p)
						destSurface.hLine(set.items[p].xs - sceneBounds.left, y - sceneBounds.top,
							set.items[p].xe - sceneBounds.left - 1, colIndex);
				}
			}
		}
	}

	// Release the surface
	_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	_globals->_paneRefreshFlag[0] = 2;

	return false;
}


} // End of namespace tSage
