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

#include "tsage/debugger.h"
#include "tsage/globals.h"
#include "tsage/graphics.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

Debugger::Debugger() : GUI::Debugger() {
	registerCmd("continue",         WRAP_METHOD(Debugger, cmdExit));
	registerCmd("scene",            WRAP_METHOD(Debugger, Cmd_Scene));
	registerCmd("walk_regions",     WRAP_METHOD(Debugger, Cmd_WalkRegions));
	registerCmd("priority_regions", WRAP_METHOD(Debugger, Cmd_PriorityRegions));
	registerCmd("scene_regions",    WRAP_METHOD(Debugger, Cmd_SceneRegions));
	registerCmd("setflag",          WRAP_METHOD(Debugger, Cmd_SetFlag));
	registerCmd("getflag",          WRAP_METHOD(Debugger, Cmd_GetFlag));
	registerCmd("clearflag",        WRAP_METHOD(Debugger, Cmd_ClearFlag));
	registerCmd("listobjects",      WRAP_METHOD(Debugger, Cmd_ListObjects));
	registerCmd("moveobject",       WRAP_METHOD(Debugger, Cmd_MoveObject));
	registerCmd("hotspots",         WRAP_METHOD(Debugger, Cmd_Hotspots));
	registerCmd("sound",            WRAP_METHOD(Debugger, Cmd_Sound));
	registerCmd("setdebug",         WRAP_METHOD(Debugger, Cmd_SetOutpostAlphaDebug));
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
		debugPrintf("Usage: %s <scene number> [prior scene #]\n", argv[0]);
		return true;
	}

	if (argc == 3)
		g_globals->_sceneManager._sceneNumber = strToInt(argv[2]);

	g_globals->_sceneManager.changeScene(strToInt(argv[1]));
	return false;
}

/**
 * This command draws the walk regions onto the screen
 */
bool Debugger::Cmd_WalkRegions(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	// Color index to use for the first walk region
	int color = 16;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	// Loop through drawing each walk region in a different color to the background surface
	Common::String regionsDesc;

	for (uint regionIndex = 0; regionIndex < g_globals->_walkRegions._regionList.size(); ++regionIndex, ++color) {
		WalkRegion &wr = g_globals->_walkRegions._regionList[regionIndex];

		// Skip the region if it's in the list of explicitly disabled regions
		if (contains(g_globals->_walkRegions._disabledRegions, (int)regionIndex + 1))
			continue;

		for (int yp = wr._bounds.top; yp < wr._bounds.bottom; ++yp) {
			LineSliceSet sliceSet = wr.getLineSlices(yp);

			for (uint idx = 0; idx < sliceSet.items.size(); ++idx)
				destSurface.hLine(sliceSet.items[idx].xs - g_globals->_sceneOffset.x, yp,
				sliceSet.items[idx].xe - g_globals->_sceneOffset.x, color);
		}

		regionsDesc += Common::String::format("Region #%d d bounds=%d,%d,%d,%d\n",
					regionIndex, wr._bounds.left, wr._bounds.top, wr._bounds.right, wr._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	debugPrintf("Total regions = %d\n", g_globals->_walkRegions._regionList.size());
	debugPrintf("%s\n", regionsDesc.c_str());

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
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = g_globals->_sceneManager._scene->_priorities.begin();
	Common::String regionsDesc;

	for (; i != g_globals->_sceneManager._scene->_priorities.end(); ++i, ++color, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + x,
							g_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = color;
					++destP;
				}
			}
		}

		regionsDesc += Common::String::format("Region Priority = %d bounds=%d,%d,%d,%d\n",
			r._regionId, r._bounds.left, r._bounds.top, r._bounds.right, r._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	debugPrintf("Total regions = %d\n", count);
	debugPrintf("%s", regionsDesc.c_str());

	return true;
}

/*
 * This command draws the scene regions onto the screen. These are the regions
 * used by hotspots that have non-rectangular areas.
 */
bool Debugger::Cmd_SceneRegions(int argc, const char **argv) {
	int regionNum = 0;

	// Check for an optional specific region to display
	if (argc == 2)
		regionNum = strToInt(argv[1]);

	// Color index to use for the first priority region
	int color = 16;
	int count = 0;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = g_globals->_sceneRegions.begin();
	Common::String regionsDesc;

	for (; i != g_globals->_sceneRegions.end(); ++i, ++color, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + x,
							g_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = color;
					++destP;
				}
			}
		}

		regionsDesc += Common::String::format("Region id = %d bounds=%d,%d,%d,%d\n",
			r._regionId, r._bounds.left, r._bounds.top, r._bounds.right, r._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	debugPrintf("Total regions = %d\n", count);
	debugPrintf("%s", regionsDesc.c_str());

	return true;
}

/*
 * This command sets a flag
 */
bool Debugger::Cmd_SetFlag(int argc, const char **argv) {
	// Check for a flag to set
	if (argc != 2) {
		debugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	g_globals->setFlag(flagNum);
	return true;
}

/*
 * This command gets the value of a flag
 */
bool Debugger::Cmd_GetFlag(int argc, const char **argv) {
	// Check for an flag to display
	if (argc != 2) {
		debugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	debugPrintf("Value: %d\n", g_globals->getFlag(flagNum));
	return true;
}

/*
 * This command clears a flag
 */
bool Debugger::Cmd_ClearFlag(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		debugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	g_globals->clearFlag(flagNum);
	return true;
}

/**
 * Show any active hotspot areas in the scene
 */
bool Debugger::Cmd_Hotspots(int argc, const char **argv) {
	int colIndex = 16;
	const Rect &sceneBounds = g_globals->_sceneManager._scene->_sceneBounds;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	// Iterate through the scene items
	SynchronizedList<SceneItem *>::iterator i;
	for (i = g_globals->_sceneItems.reverse_begin(); i != g_globals->_sceneItems.end(); --i, ++colIndex) {
		SceneItem *o = *i;

		// Draw the contents of the hotspot area
		if (o->_sceneRegionId == 0) {
			// Scene item doesn't use a region, so fill in the entire area
			if ((o->_bounds.right > o->_bounds.left) && (o->_bounds.bottom > o->_bounds.top))
				destSurface.fillRect(Rect(o->_bounds.left - sceneBounds.left, o->_bounds.top - sceneBounds.top,
					o->_bounds.right - sceneBounds.left - 1, o->_bounds.bottom - sceneBounds.top - 1), colIndex);
		} else {
			// Scene uses a region, so get it and use it to fill out only the correct parts
			SceneRegions::iterator ri = g_globals->_sceneRegions.begin();
			while ((ri != g_globals->_sceneRegions.end()) && ((*ri)._regionId != o->_sceneRegionId))
				++ri;

			if (ri != g_globals->_sceneRegions.end()) {
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
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	return false;
}

/**
 * Play the specified sound
 */
bool Debugger::Cmd_Sound(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <sound number>\n", argv[0]);
		return true;
	}

	int soundNum = strToInt(argv[1]);
	g_globals->_soundHandler.play(soundNum);
	return false;
}

/**
 * Activate internal debugger, when available
 */
bool Debugger::Cmd_SetOutpostAlphaDebug(int argc, const char **argv) {
	debugPrintf("Not available in this game\n");
	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool DemoDebugger::Cmd_ListObjects(int argc, const char **argv) {
	debugPrintf("Not available in Demo\n");
	return true;
}

bool DemoDebugger::Cmd_MoveObject(int argc, const char **argv) {
	debugPrintf("Not available in Demo\n");
	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool RingworldDebugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Available objects for this game are:\n");
	debugPrintf("0 - Stunner\n");
	debugPrintf("1 - Scanner\n");
	debugPrintf("2 - Stasis Box\n");
	debugPrintf("3 - Info Disk\n");
	debugPrintf("4 - Stasis Negator\n");
	debugPrintf("5 - Key Device\n");
	debugPrintf("6 - Medkit\n");
	debugPrintf("7 - Ladder\n");
	debugPrintf("8 - Rope\n");
	debugPrintf("9 - Key\n");
	debugPrintf("10 - Translator\n");
	debugPrintf("11 - Ale\n");
	debugPrintf("12 - Paper\n");
	debugPrintf("13 - Waldos\n");
	debugPrintf("14 - Stasis Box 2\n");
	debugPrintf("15 - Ring\n");
	debugPrintf("16 - Cloak\n");
	debugPrintf("17 - Tunic\n");
	debugPrintf("18 - Candle\n");
	debugPrintf("19 - Straw\n");
	debugPrintf("20 - Scimitar\n");
	debugPrintf("21 - Sword\n");
	debugPrintf("22 - Helmet\n");
	debugPrintf("23 - Items\n");
	debugPrintf("24 - Concentrator\n");
	debugPrintf("25 - Nullifier\n");
	debugPrintf("26 - Peg\n");
	debugPrintf("27 - Vial\n");
	debugPrintf("28 - Jacket\n");
	debugPrintf("29 - Tunic 2\n");
	debugPrintf("30 - Bone\n");
	debugPrintf("31 - Empty Jar\n");
	debugPrintf("32 - Jar\n");
	return true;
}

/*
 * This command gets an item, or move it to a room
 */
bool RingworldDebugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		debugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		debugPrintf("If no scene is specified, the object will be added to inventory\n");
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
		debugPrintf("Invalid object Id %s\n", argv[1]);
		break;
	}

	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool BlueForceDebugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Available objects for this game are:\n");
	debugPrintf("1  - INV_COLT45\n");
	debugPrintf("2  - INV_AMMO_CLIP\n");
	debugPrintf("3  - INV_SPARE_CLIP\n");
	debugPrintf("4  - INV_HANDCUFFS\n");
	debugPrintf("5  - INV_GREENS_GUN\n");
	debugPrintf("6  - INV_TICKET_BOOK\n");
	debugPrintf("7  - INV_MIRANDA_CARD\n");
	debugPrintf("8  - INV_FOREST_RAP\n");
	debugPrintf("9  - INV_GREEN_ID\n");
	debugPrintf("10 - INV_BASEBALL_CARD\n");
	debugPrintf("11 - INV_BOOKING_GREEN\n");
	debugPrintf("12 - INV_FLARE\n");
	debugPrintf("13 - INV_COBB_RAP\n");
	debugPrintf("14 - INV_22_BULLET\n");
	debugPrintf("15 - INV_AUTO_RIFLE\n");
	debugPrintf("16 - INV_WIG\n");
	debugPrintf("17 - INV_FRANKIE_ID\n");
	debugPrintf("18 - INV_TYRONE_ID\n");
	debugPrintf("19 - INV_22_SNUB\n");
	debugPrintf("20 - INV_BOOKING_FRANKIE\n");
	debugPrintf("21 - INV_BOOKING_GANG\n");
	debugPrintf("22 - INV_FBI_TELETYPE\n");
	debugPrintf("23 - INV_DA_NOTE\n");
	debugPrintf("24 - INV_PRINT_OUT\n");
	debugPrintf("25 - INV_WAREHOUSE_KEYS\n");
	debugPrintf("26 - INV_CENTER_PUNCH\n");
	debugPrintf("27 - INV_TRANQ_GUN\n");
	debugPrintf("28 - INV_HOOK\n");
	debugPrintf("29 - INV_RAGS\n");
	debugPrintf("30 - INV_JAR\n");
	debugPrintf("31 - INV_SCREWDRIVER\n");
	debugPrintf("32 - INV_D_FLOPPY\n");
	debugPrintf("33 - INV_BLANK_DISK\n");
	debugPrintf("34 - INV_STICK\n");
	debugPrintf("35 - INV_CRATE1\n");
	debugPrintf("36 - INV_CRATE2\n");
	debugPrintf("37 - INV_SHOEBOX\n");
	debugPrintf("38 - INV_BADGE\n");
	debugPrintf("39 - INV_RENTAL_COUPON\n");
	debugPrintf("40 - INV_NICKEL\n");
	debugPrintf("41 - INV_LYLE_CARD\n");
	debugPrintf("42 - INV_CARTER_NOTE\n");
	debugPrintf("43 - INV_MUG_SHOT\n");
	debugPrintf("44 - INV_CLIPPING\n");
	debugPrintf("45 - INV_MICROFILM \n");
	debugPrintf("46 - INV_WAVE_KEYS\n");
	debugPrintf("47 - INV_RENTAL_KEYS\n");
	debugPrintf("48 - INV_NAPKIN\n");
	debugPrintf("49 - INV_DMV_PRINTOUT\n");
	debugPrintf("50 - INV_FISHING_NET\n");
	debugPrintf("51 - INV_ID\n");
	debugPrintf("52 - INV_9MM_BULLETS\n");
	debugPrintf("53 - INV_SCHEDULE\n");
	debugPrintf("54 - INV_GRENADES\n");
	debugPrintf("55 - INV_YELLOW_CORD\n");
	debugPrintf("56 - INV_HALF_YELLOW_CORD\n");
	debugPrintf("57 - INV_BLACK_CORD\n");
	debugPrintf("58 - INV_HALF_BLACK_CORD\n");
	debugPrintf("59 - INV_WARRANT\n");
	debugPrintf("60 - INV_JACKET\n");
	debugPrintf("61 - INV_GREENS_KNIFE\n");
	debugPrintf("62 - INV_DOG_WHISTLE\n");
	debugPrintf("63 - INV_AMMO_BELT\n");
	debugPrintf("64 - INV_CARAVAN_KEY\n");
	return true;
}

bool BlueForceDebugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		debugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		debugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	if ((objNum > 0) && (objNum < 65))
		BF_INVENTORY.setObjectScene(objNum, sceneNum);
	else
		debugPrintf("Invalid object Id %s\n", argv[1]);

	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool Ringworld2Debugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Available objects for this game are:\n");
	debugPrintf("1  - Scene %d - R2_OPTO_DISK\n", BF_INVENTORY.getObjectScene(1));
	debugPrintf("2  - Scene %d - R2_READER\n", BF_INVENTORY.getObjectScene(2));
	debugPrintf("3  - Scene %d - R2_NEGATOR_GUN\n", BF_INVENTORY.getObjectScene(3));
	debugPrintf("4  - Scene %d - R2_STEPPING_DISKS\n", BF_INVENTORY.getObjectScene(4));
	debugPrintf("5  - Scene %d - R2_ATTRACTOR_UNIT\n", BF_INVENTORY.getObjectScene(5));
	debugPrintf("6  - Scene %d - R2_SENSOR_PROBE\n", BF_INVENTORY.getObjectScene(6));
	debugPrintf("7  - Scene %d - R2_SONIC_STUNNER\n", BF_INVENTORY.getObjectScene(7));
	debugPrintf("8  - Scene %d - R2_CABLE_HARNESS\n", BF_INVENTORY.getObjectScene(8));
	debugPrintf("9  - Scene %d - R2_COM_SCANNER\n", BF_INVENTORY.getObjectScene(9));
	debugPrintf("10 - Scene %d - R2_SPENT_POWER_CAPSULE\n", BF_INVENTORY.getObjectScene(10));
	debugPrintf("11 - Scene %d - R2_CHARGED_POWER_CAPSULE\n", BF_INVENTORY.getObjectScene(11));
	debugPrintf("12 - Scene %d - R2_AEROSOL\n", BF_INVENTORY.getObjectScene(12));
	debugPrintf("13 - Scene %d - R2_REMOTE_CONTROL\n", BF_INVENTORY.getObjectScene(13));
	debugPrintf("14 - Scene %d - R2_OPTICAL_FIBER\n", BF_INVENTORY.getObjectScene(14));
	debugPrintf("15 - Scene %d - R2_CLAMP\n", BF_INVENTORY.getObjectScene(15));
	debugPrintf("16 - Scene %d - R2_ATTRACTOR_CABLE_HARNESS\n", BF_INVENTORY.getObjectScene(16));
	debugPrintf("17 - Scene %d - R2_FUEL_CELL\n", BF_INVENTORY.getObjectScene(17));
	debugPrintf("18 - Scene %d - R2_GYROSCOPE\n", BF_INVENTORY.getObjectScene(18));
	debugPrintf("19 - Scene %d - R2_AIRBAG\n", BF_INVENTORY.getObjectScene(19));
	debugPrintf("20 - Scene %d - R2_REBREATHER_TANK\n", BF_INVENTORY.getObjectScene(20));
	debugPrintf("21 - Scene %d - R2_RESERVE_REBREATHER_TANK\n", BF_INVENTORY.getObjectScene(21));
	debugPrintf("22 - Scene %d - R2_GUIDANCE_MODULE\n", BF_INVENTORY.getObjectScene(22));
	debugPrintf("23 - Scene %d - R2_THRUSTER_VALVE\n", BF_INVENTORY.getObjectScene(23));
	debugPrintf("24 - Scene %d - R2_BALLOON_BACKPACK\n", BF_INVENTORY.getObjectScene(24));
	debugPrintf("25 - Scene %d - R2_RADAR_MECHANISM\n", BF_INVENTORY.getObjectScene(25));
	debugPrintf("26 - Scene %d - R2_JOYSTICK\n", BF_INVENTORY.getObjectScene(26));
	debugPrintf("27 - Scene %d - R2_IGNITOR\n", BF_INVENTORY.getObjectScene(27));
	debugPrintf("28 - Scene %d - R2_DIAGNOSTICS_DISPLAY\n", BF_INVENTORY.getObjectScene(28));
	debugPrintf("29 - Scene %d - R2_GLASS_DOME\n", BF_INVENTORY.getObjectScene(29));
	debugPrintf("30 - Scene %d - R2_WICK_LAMP\n", BF_INVENTORY.getObjectScene(30));
	debugPrintf("31 - Scene %d - R2_SCRITH_KEY\n", BF_INVENTORY.getObjectScene(31));
	debugPrintf("32 - Scene %d - R2_TANNER_MASK\n", BF_INVENTORY.getObjectScene(32));
	debugPrintf("33 - Scene %d - R2_PURE_GRAIN_ALCOHOL\n", BF_INVENTORY.getObjectScene(33));
	debugPrintf("34 - Scene %d - R2_SAPPHIRE_BLUE\n", BF_INVENTORY.getObjectScene(34));
	debugPrintf("35 - Scene %d - R2_ANCIENT_SCROLLS\n", BF_INVENTORY.getObjectScene(35));
	debugPrintf("36 - Scene %d - R2_FLUTE\n", BF_INVENTORY.getObjectScene(36));
	debugPrintf("37 - Scene %d - R2_GUNPOWDER\n", BF_INVENTORY.getObjectScene(37));
	debugPrintf("38 - Scene %d - R2_NONAME\n", BF_INVENTORY.getObjectScene(38));
	debugPrintf("39 - Scene %d - R2_COM_SCANNER_2\n", BF_INVENTORY.getObjectScene(39));
	debugPrintf("40 - Scene %d - R2_SUPERCONDUCTOR_WIRE\n", BF_INVENTORY.getObjectScene(40));
	debugPrintf("41 - Scene %d - R2_PILLOW\n", BF_INVENTORY.getObjectScene(41));
	debugPrintf("42 - Scene %d - R2_FOOD_TRAY\n", BF_INVENTORY.getObjectScene(42));
	debugPrintf("43 - Scene %d - R2_LASER_HACKSAW\n", BF_INVENTORY.getObjectScene(43));
	debugPrintf("44 - Scene %d - R2_PHOTON_STUNNER\n", BF_INVENTORY.getObjectScene(44));
	debugPrintf("45 - Scene %d - R2_BATTERY\n", BF_INVENTORY.getObjectScene(45));
	debugPrintf("46 - Scene %d - R2_SOAKED_FACEMASK\n", BF_INVENTORY.getObjectScene(46));
	debugPrintf("47 - Scene %d - R2_LIGHT_BULB\n", BF_INVENTORY.getObjectScene(47));
	debugPrintf("48 - Scene %d - R2_ALCOHOL_LAMP\n", BF_INVENTORY.getObjectScene(48));
	debugPrintf("49 - Scene %d - R2_ALCOHOL_LAMP_2\n", BF_INVENTORY.getObjectScene(49));
	debugPrintf("50 - Scene %d - R2_ALCOHOL_LAMP_3\n", BF_INVENTORY.getObjectScene(50));
	debugPrintf("51 - Scene %d - R2_BROKEN_DISPLAY\n", BF_INVENTORY.getObjectScene(51));
	debugPrintf("52 - Scene %d - R2_TOOLBOX\n", BF_INVENTORY.getObjectScene(52));

	return true;
}

bool Ringworld2Debugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		debugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		debugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	if ((objNum > 0) && (objNum < 53))
		R2_INVENTORY.setObjectScene(objNum, sceneNum);
	else
		debugPrintf("Invalid object Id %s\n", argv[1]);

	return true;
}

/**
 * Activate internal debugger, when available
 */
bool Ringworld2Debugger::Cmd_SetOutpostAlphaDebug(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	// Set the internal debugger flag(s?) to true
	// _debugCardGame is reset by scene1337::subPostInit()
	R2_GLOBALS._debugCardGame = true;
	return true;
}
} // End of namespace TsAGE
