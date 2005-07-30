/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "scumm/actor.h"
#include "scumm/boxes.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

/**
 * Start a 'scene' by loading the specified room with the given main actor.
 * The actor is placed next to the object indicated by objectNr.
 */
void ScummEngine::startScene(int room, Actor *a, int objectNr) {
	int i, where;

	CHECK_HEAP;
	debugC(DEBUG_GENERAL, "Loading room %d", room);

	stopTalk();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	ScriptSlot *ss =  &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride && _version >= 5)
				error("Object %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(_currentScript);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride && _version >= 5)
				error("Script %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(_currentScript);
			_currentScript = 0xFF;
		}
	}

	if (VAR_NEW_ROOM != 0xFF)
		VAR(VAR_NEW_ROOM) = room;

	runExitScript();

	killScriptsAndResources();
	if (_version >= 4 && _heversion <= 61)
		stopCycle(0);
	_sound->processSound();

	clearDrawQueues();

	// For HE80+ games
	for (i = 0; i < _numRoomVariables; i++)
		_roomVars[i] = 0;
	nukeArrays(0xFF);

	for (i = 1; i < _numActors; i++) {
		_actors[i].hideActor();
	}

	if (_version >= 7) {
		// Set the shadow palette(s) to all black. This fixes
		// bug #795940, and actually makes some sense (after all,
		// shadows tend to be rather black, don't they? ;-)
		memset(_shadowPalette, 0, NUM_SHADOW_PALETTE * 256);
	} else {
		for (i = 0; i < 256; i++) {
			_roomPalette[i] = i;
			if (_shadowPalette)
				_shadowPalette[i] = i;
		}
		if (_features & GF_SMALL_HEADER)
			setDirtyColors(0, 255);
	}

	VAR(VAR_ROOM) = room;
	_fullRedraw = true;

	res.increaseResourceCounter();

	_currentRoom = room;
	VAR(VAR_ROOM) = room;

	if (room >= 0x80 && _version < 7 && _heversion <= 71)
		_roomResource = _resourceMapper[room & 0x7F];
	else
		_roomResource = room;

	if (VAR_ROOM_RESOURCE != 0xFF)
		VAR(VAR_ROOM_RESOURCE) = _roomResource;

	if (room != 0)
		ensureResourceLoaded(rtRoom, room);

	clearRoomObjects();

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	loadRoomSubBlocks();
	initRoomSubBlocks();

	initBGBuffers(_roomHeight);

	loadRoomObjects();


	if (VAR_ROOM_WIDTH != 0xFF && VAR_ROOM_HEIGHT != 0xFF) {
		VAR(VAR_ROOM_WIDTH) = _roomWidth;
		VAR(VAR_ROOM_HEIGHT) = _roomHeight;
	}

	VAR(VAR_CAMERA_MIN_X) = _screenWidth / 2;
	VAR(VAR_CAMERA_MAX_X) = _roomWidth - (_screenWidth / 2);

	if (_features & GF_NEW_CAMERA) {
		VAR(VAR_CAMERA_MIN_Y) = _screenHeight / 2;
		VAR(VAR_CAMERA_MAX_Y) = _roomHeight - (_screenHeight / 2);
		setCameraAt(_screenWidth / 2, _screenHeight / 2);
	} else {
		camera._mode = kNormalCameraMode;
		if (_version > 2)
			camera._cur.x = camera._dest.x = _screenWidth / 2;
		camera._cur.y = camera._dest.y = _screenHeight / 2;
	}

	if (_roomResource == 0)
		return;

	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	if (_version >= 5 && a) {
		where = whereIsObject(objectNr);
		if (where != WIO_ROOM && where != WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr,
					_currentRoom);
		int x, y, dir;
		getObjectXYPos(objectNr, x, y, dir);
		a->putActor(x, y, _currentRoom);
		a->setDirection(dir + 180);
		a->stopActorMoving();
		if (_gameId == GID_SAMNMAX) {
			camera._cur.x = camera._dest.x = a->_pos.x;
			setCameraAt(a->_pos.x, a->_pos.y);
		}
	}

	showActors();

	_egoPositioned = false;
	runEntryScript();
	if (_gameId == GID_MANIAC && _platform == Common::kPlatformC64) {
		runScript(18, 0, 0, 0);
	} else if (_version <= 2) {
		runScript(5, 0, 0, 0);
	} else if (_version >= 5 && _version <= 6) {
		if (a && !_egoPositioned) {
			int x, y;
			getObjectXYPos(objectNr, x, y);
			a->putActor(x, y, _currentRoom);
			a->_moving = 0;
		}
	} else if (_version >= 7) {
		if (camera._follows) {
			a = derefActor(camera._follows, "startScene: follows");
			setCameraAt(a->_pos.x, a->_pos.y);
		}
	}

	_doEffect = true;

	CHECK_HEAP;
}

/**
 * Init some static room data after a room has been loaded.
 * E.g. the room dimension, the offset to the graphics data, the room scripts,
 * the offset to the room palette and other things which won't be changed
 * late on.
 * So it is possible to call this after loading a savegame.
 */
void ScummEngine::loadRoomSubBlocks() {
	int i;
	const byte *ptr;
	byte *roomptr, *searchptr, *roomResPtr = 0;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_EPAL_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	// Determine the room and room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	if (!roomptr || !roomResPtr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Determine the room dimensions (width/height)
	//
	rmhd = (const RoomHeader *)findResourceData(MKID('RMHD'), roomptr);

	if (_version == 8) {
		_roomWidth = READ_LE_UINT32(&(rmhd->v8.width));
		_roomHeight = READ_LE_UINT32(&(rmhd->v8.height));
		_numObjectsInRoom = (byte)READ_LE_UINT32(&(rmhd->v8.numObjects));
	} else if (_version == 7) {
		_roomWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->v7.height));
		_numObjectsInRoom = (byte)READ_LE_UINT16(&(rmhd->v7.numObjects));
	} else {
		_roomWidth = READ_LE_UINT16(&(rmhd->old.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->old.height));
		_numObjectsInRoom = (byte)READ_LE_UINT16(&(rmhd->old.numObjects));
	}

	//
	// Find the room image data
	//
	if (_version == 8) {
		_IM00_offs = getObjectImage(roomptr, 1) - roomptr;
	} else if (_features & GF_SMALL_HEADER) {
		_IM00_offs = findResourceData(MKID('IM00'), roomptr) - roomptr;
	} else if (_heversion >= 70) {
		byte *roomImagePtr = getResourceAddress(rtRoomImage, _roomResource);
		_IM00_offs = findResource(MKID('IM00'), roomImagePtr) - roomImagePtr;
	} else {
		_IM00_offs = findResource(MKID('IM00'), findResource(MKID('RMIM'), roomptr)) - roomptr;
	}

	//
	// Look for an exit script
	//
	ptr = findResourceData(MKID('EXCD'), roomResPtr);
	if (ptr)
		_EXCD_offs = ptr - roomResPtr;
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomResPtr + _EXCD_offs - _resourceHeaderSize, -1);

	//
	// Look for an entry script
	//
	ptr = findResourceData(MKID('ENCD'), roomResPtr);
	if (ptr)
		_ENCD_offs = ptr - roomResPtr;
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomResPtr + _ENCD_offs - _resourceHeaderSize, -1);

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	searchptr = roomResPtr;

	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));

	if (_features & GF_SMALL_HEADER) {
		ResourceIterator localScriptIterator(searchptr, true);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;
			ptr += _resourceHeaderSize;	/* skip tag & size */
			id = ptr[0];

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}

			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomptr;
		}
	} else if (_heversion >= 90) {
		ResourceIterator localScriptIterator2(searchptr, false);
		while ((ptr = localScriptIterator2.findNext(MKID('LSC2'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = READ_LE_UINT32(ptr);

			checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = ptr[0];
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

	} else {
		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKID('LSCR'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_version == 8) {
				id = READ_LE_UINT32(ptr);
				checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;
			} else if (_version == 7) {
				id = READ_LE_UINT16(ptr);
				checkRange(_numLocalScripts + _numGlobalScripts, _numGlobalScripts, id, "Invalid local script %d");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 2 - roomResPtr;
			} else {
				id = ptr[0];
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;
			}

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}
	}

	// Locate the EGA palette (currently unused).
	ptr = findResourceData(MKID('EPAL'), roomptr);
	if (ptr)
		_EPAL_offs = ptr - roomptr;

	// Locate the standard room palette (for V3-V5 games).
	ptr = findResourceData(MKID('CLUT'), roomptr);
	if (ptr)
		_CLUT_offs = ptr - roomptr;

	// Locate the standard room palettes (for V6+ games).
	if (_version >= 6) {
		ptr = findResource(MKID('PALS'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
		}
	}

	// Transparent color
	byte trans;
	if (_version == 8)
		trans = (byte)READ_LE_UINT32(&(rmhd->v8.transparency));
	else {
		ptr = findResourceData(MKID('TRNS'), roomptr);
		if (ptr)
			trans = ptr[0];
		else
			trans = 255;
	}

	// Actor Palette in HE 70 games
	if (_heversion == 70) {
		ptr = findResourceData(MKID('REMP'), roomptr);
		if (ptr) {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = *ptr++;
		} else {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = i;
		}
	}

	gdi.roomChanged(roomptr, _IM00_offs, trans);
}

/**
 * Init some dynamic room data after a room has been loaded.
 * E.g. the initial box data is loaded, the initial palette is set etc.
 * All of the things setup in here can be modified later on by scripts.
 * So it is not appropriate to call it after loading a savegame.
 */
void ScummEngine::initRoomSubBlocks() {
	int i;
	const byte *ptr;
	byte *roomptr;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Load box data
	//
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));

	res.nukeResource(rtMatrix, 1);
	res.nukeResource(rtMatrix, 2);
	if (_features & GF_SMALL_HEADER) {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			byte numOfBoxes = *ptr;
			int size;
			if (_version == 3)
				size = numOfBoxes * SIZEOF_BOX_V3 + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;

			res.createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;

			size = getResourceDataSize(ptr - size - _resourceHeaderSize) - size;
			if (size > 0) {					// do this :)
				res.createResource(rtMatrix, 1, size);
				memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
			}

		}
	} else {
		ptr = findResourceData(MKID('BOXD'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			res.createResource(rtMatrix, 2, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXD'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		}

		ptr = findResourceData(MKID('BOXM'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			res.createResource(rtMatrix, 1, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKID('BOXM'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}
	}

	//
	// Load scale data
	//
	for (i = 1; i < res.num[rtScaleTable]; i++)
		res.nukeResource(rtScaleTable, i);

	ptr = findResourceData(MKID('SCAL'), roomptr);
	if (ptr) {
		int s1, s2, y1, y2;
		if (_version == 8) {
			for (i = 1; i < res.num[rtScaleTable]; i++, ptr += 16) {
				s1 = READ_LE_UINT32(ptr);
				y1 = READ_LE_UINT32(ptr + 4);
				s2 = READ_LE_UINT32(ptr + 8);
				y2 = READ_LE_UINT32(ptr + 12);
				setScaleSlot(i, 0, y1, s1, 0, y2, s2);
			}
		} else {
			for (i = 1; i < res.num[rtScaleTable]; i++, ptr += 8) {
				s1 = READ_LE_UINT16(ptr);
				y1 = READ_LE_UINT16(ptr + 2);
				s2 = READ_LE_UINT16(ptr + 4);
				y2 = READ_LE_UINT16(ptr + 6);
				if (s1 || y1 || s2 || y2) {
					setScaleSlot(i, 0, y1, s1, 0, y2, s2);
				}
			}
		}
	}

	// Color cycling
	// HE 7.0 games load resources but don't use them.
	if (_version >= 4 && _heversion <= 60) {
		ptr = findResourceData(MKID('CYCL'), roomptr);
		if (ptr) {
			initCycl(ptr);
		}
	}

#ifndef DISABLE_HE
	// Polygons in HE 80+ games
	if (_heversion >= 80) {
		ptr = findResourceData(MKID('POLD'), roomptr);
		if (ptr) {
			((ScummEngine_v70he *)this)->_wiz->polygonLoad(ptr);
		}
	}
#endif

	if (_PALS_offs || _CLUT_offs)
		setPalette(0);
}


void ScummEngine_v3old::loadRoomSubBlocks() {
	const byte *ptr;
	byte *roomptr, *searchptr = 0;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_EPAL_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Determine the room dimensions (width/height)
	//
	rmhd = (const RoomHeader *)(roomptr + 4);

	if (_version == 1) {
		if (_platform == Common::kPlatformNES) {
			_roomWidth = READ_LE_UINT16(&(rmhd->old.width)) * 8;
			_roomHeight = READ_LE_UINT16(&(rmhd->old.height)) * 8;

			// HACK: To let our code work normal with narrow rooms we
			// adjust width. It will render garbage on right edge but we do
			// not render it anyway
			if (_roomWidth < 32 * 8)
				_roomWidth = 32 * 8;
		} else {
			_roomWidth = roomptr[4] * 8;
			_roomHeight = roomptr[5] * 8;
		}
	} else {
		_roomWidth = READ_LE_UINT16(&(rmhd->old.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->old.height));
	}
	_numObjectsInRoom = roomptr[20];

	//
	// Find the room image data
	//
	if (_version == 1) {
		_IM00_offs = 0;
	} else {
		_IM00_offs = READ_LE_UINT16(roomptr + 0x0A);
	}

	//
	// Look for an exit script
	//
	int EXCD_len = -1;
	if (_version <= 2) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x18);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1A) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x19);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1B) - _EXCD_offs + _resourceHeaderSize;	// HACK
	}
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomptr + _EXCD_offs - _resourceHeaderSize, EXCD_len);

	//
	// Look for an entry script
	//
	int ENCD_len = -1;
	if (_version <= 2) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1A);
		ENCD_len = READ_LE_UINT16(roomptr) - _ENCD_offs + _resourceHeaderSize; // HACK
	} else {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1B);
		// FIXME - the following is a hack which assumes that immediately after
		// the entry script the first local script follows.
		int num_objects = *(roomptr + 20);
		int num_sounds = *(roomptr + 23);
		int num_scripts = *(roomptr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		ENCD_len = READ_LE_UINT16(ptr + 1) - _ENCD_offs + _resourceHeaderSize; // HACK
	}
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomptr + _ENCD_offs - _resourceHeaderSize, ENCD_len);

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	searchptr = roomptr;

	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));

	int num_objects = *(roomptr + 20);
	int num_sounds;
	int num_scripts;

	if (_version <= 2) {
		num_sounds = *(roomptr + 22);
		num_scripts = *(roomptr + 23);
		ptr = roomptr + 28 + num_objects * 4;
		while (num_sounds--)
			loadResource(rtSound, *ptr++);
		while (num_scripts--)
			loadResource(rtScript, *ptr++);
	} else /* if (_version == 3) */ {
		num_sounds = *(roomptr + 23);
		num_scripts = *(roomptr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		while (*ptr) {
			int id = *ptr;

			_localScriptOffsets[id - _numGlobalScripts] = READ_LE_UINT16(ptr + 1);
			ptr += 3;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);

				// HACK: to determine the sizes of the local scripts, we assume that
				// a) their order in the data file is the same as in the index
				// b) the last script at the same time is the last item in the room "header"
				int len = - (int)_localScriptOffsets[id - _numGlobalScripts] + _resourceHeaderSize;
				if (*ptr)
					len += READ_LE_UINT16(ptr + 1);
				else
					len += READ_LE_UINT16(roomptr);
				dumpResource(buf, id, roomptr + _localScriptOffsets[id - _numGlobalScripts] - _resourceHeaderSize, len);
			}
		}
	}

	gdi.roomChanged(roomptr, _IM00_offs, 255);
}

void ScummEngine_v3old::initRoomSubBlocks() {
	int i;
	const byte *ptr;
	byte *roomptr;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	// Reset room color for V1 zak
	if (_version == 1)
		_roomPalette[0] = 0;

	//
	// Load box data
	//
	res.nukeResource(rtMatrix, 1);
	res.nukeResource(rtMatrix, 2);

	if (_version <= 2)
		ptr = roomptr + *(roomptr + 0x15);
	else
		ptr = roomptr + READ_LE_UINT16(roomptr + 0x15);
	if (ptr) {
		byte numOfBoxes = *ptr;
		int size;
		if (_version <= 2)
			size = numOfBoxes * SIZEOF_BOX_V2 + 1;
		else
			size = numOfBoxes * SIZEOF_BOX_V3 + 1;

		res.createResource(rtMatrix, 2, size);
		memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		ptr += size;
		if (_version <= 2) {
			size = numOfBoxes * (numOfBoxes + 1);
		} else {
			// FIXME. This is an evil HACK!!!
			size = (READ_LE_UINT16(roomptr + 0x0A) - READ_LE_UINT16(roomptr + 0x15)) - size;
		}

		if (size > 0) {					// do this :)
			res.createResource(rtMatrix, 1, size);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}

	}

	//
	// No scale data in old bundle games
	//
	for (i = 1; i < res.num[rtScaleTable]; i++)
		res.nukeResource(rtScaleTable, i);

}

} // End of namespace Scumm
