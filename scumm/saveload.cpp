/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdafx.h"
#include "actor.h"
#include "charset.h"
#include "imuse.h"
#include "imuse_digi.h"
#include "resource.h"
#include "saveload.h"
#include "scumm.h"
#include "sound.h"
#include "verbs.h"
#include "common/config-file.h"
#include "sound/mixer.h"

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 ver;
	char name[32];
};


void Scumm::requestSave(int slot, const char *name) {
	_saveLoadSlot = slot;
	_saveLoadCompatible = false;
	_saveLoadFlag = 1;		// 1 for save
	strcpy(_saveLoadName, name);
}

void Scumm::requestLoad(int slot) {
	_saveLoadSlot = slot;
	_saveLoadCompatible = false;
	_saveLoadFlag = 2;		// 2 for load
}

bool Scumm::saveState(int slot, bool compat, SaveFileManager *mgr) {
	char filename[256];
	SaveFile *out;
	SaveGameHeader hdr;

	makeSavegameName(filename, slot, compat);

	if (!(out = mgr->open_savefile(filename, true)))
		return false;

	memcpy(hdr.name, _saveLoadName, sizeof(hdr.name));

	hdr.type = MKID('SCVM');
	hdr.size = 0;
	hdr.ver = TO_LE_32(CURRENT_VER);

	out->fwrite(&hdr, sizeof(hdr), 1);

	Serializer ser(out, true, CURRENT_VER);
	saveOrLoad(&ser, CURRENT_VER);
	delete out;
	debug(1, "State saved as '%s'", filename);
	return true;
}

bool Scumm::loadState(int slot, bool compat, SaveFileManager *mgr) {
	char filename[256];
	SaveFile *out;
	int i, j;
	SaveGameHeader hdr;
	int sb, sh;
	byte *roomptr;

	makeSavegameName(filename, slot, compat);
	if (!(out = mgr->open_savefile(filename, false)))
		return false;

	out->fread(&hdr, sizeof(hdr), 1);
	if (hdr.type != MKID('SCVM')) {
		warning("Invalid savegame '%s'", filename);
		delete out;
		return false;
	}

	// In older versions of ScummVM, the header version was not endian safe.
	// We account for that by retrying once with swapped byte order.
	if (hdr.ver > CURRENT_VER)
		hdr.ver = SWAP_BYTES_32(hdr.ver);
	if (hdr.ver < VER_V7 || hdr.ver > CURRENT_VER)
	{
		warning("Invalid version of '%s'", filename);
		delete out;
		return false;
	}

	// Due to a bug in scummvm up to and including 0.3.0, save games could be saved
	// in the V8/V9 format but were tagged with a V7 mark. Ouch. So we just pretend V7 == V8 here
	if (hdr.ver == VER_V7)
		hdr.ver = VER_V8;

	memcpy(_saveLoadName, hdr.name, sizeof(hdr.name));

	if (_imuseDigital) {
		_imuseDigital->stopAll();
	}

	_sound->stopBundleMusic();
	_sound->stopCD();
	_sound->pauseSounds(true);

	CHECK_HEAP
	closeRoom();
	memset(_inventory, 0, sizeof(_inventory[0]) * _numInventory);

	/* Nuke all resources */
	for (i = rtFirst; i <= rtLast; i++)
		if (i != rtTemp && i != rtBuffer && (i != rtSound || _saveSound || !compat))
			for (j = 0; j < res.num[i]; j++) {
				nukeResource(i, j);
				res.flags[i][j] = 0;
			}

	initScummVars();

	if (_features & GF_OLD_BUNDLE)
		loadCharset(0); // FIXME - HACK ?
	
	Serializer ser(out, false, hdr.ver);
	saveOrLoad(&ser, hdr.ver);
	delete out;

	sb = _screenB;
	sh = _screenH;

	gdi._mask.top = gdi._mask.left = 32767;
	gdi._mask.right = gdi._mask.bottom = 0;
	_charset->_hasMask = false;

	initScreens(0, 0, _screenWidth, _screenHeight);

	// Force a fade to black
	int old_screenEffectFlag = _screenEffectFlag;
	_screenEffectFlag = true;
	fadeOut(129);
	_screenEffectFlag = old_screenEffectFlag ? true : false;

	initScreens(0, sb, _screenWidth, sh);

	_completeScreenRedraw = true;

	// We could simply dirty colours 0-15 for 16-colour games -- nowadays
	// they handle their palette pretty much like the more recent games
	// anyway. There was a time, though, when re-initializing was necessary
	// for backwards compatibility, and it may still prove useful if we
	// ever add options for using different 16-colour palettes.

	if (_version == 1) {
		if (_gameId == GID_MANIAC)
			setupV1ManiacPalette();
		else
			setupV1ZakPalette();
	} else if (_features & GF_16COLOR) {
		if (_features & GF_AMIGA)
			setupAmigaPalette();
		else
			setupEGAPalette();
	} else
		setDirtyColors(0, 255);

	_lastCodePtr = NULL;

	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

	cameraMoved();

	initBGBuffers(_roomHeight);

	// Regenerate strip table when loading
	if (_version == 1) {
		roomptr = getResourceAddress(rtRoom, _roomResource);
		_IM00_offs = 0;
		for (i = 0; i < 4; i++){
			gdi._C64Colors[i] = roomptr[6 + i];
		}
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 10), gdi._C64CharMap, 2048);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 12), gdi._C64PicMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 14), gdi._C64ColorMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 16), gdi._C64MaskMap, roomptr[4] * roomptr[5]);
		gdi.decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 18) + 2, gdi._C64MaskChar, READ_LE_UINT16(roomptr + READ_LE_UINT16(roomptr + 18)));
		gdi._C64ObjectMode = true;
	} else if (_version == 2) {
		_roomStrips = gdi.generateStripTable(getResourceAddress(rtRoom, _roomResource) + _IM00_offs,
		                                     _roomWidth, _roomHeight, _roomStrips);
	}

	if ((_features & GF_AUDIOTRACKS) && VAR(VAR_MUSIC_TIMER) > 0)
		_sound->startCDTimer();

	CHECK_HEAP debug(1, "State loaded from '%s'", filename);

	_sound->pauseSounds(false);

	return true;
}

void Scumm::makeSavegameName(char *out, int slot, bool compatible) {
	const char *dir = getSavePath();

#ifndef __PALM_OS__
	sprintf(out, "%s%s.%c%.2d", dir, _game_name, compatible ? 'c' : 's', slot);
#else
	sprintf(out, "%s%s.%s%.2d", dir, _game_name, compatible ? "c" : "s", slot);
#endif
}

void Scumm::listSavegames(bool *marks, int num, SaveFileManager *mgr) {
	char prefix[256];
	makeSavegameName(prefix, 99, false);
	prefix[strlen(prefix)-2] = 0;
	mgr->list_savefiles(prefix, marks, num);
}

bool Scumm::getSavegameName(int slot, char *desc, SaveFileManager *mgr) {
	char filename[256];
	SaveFile *out;
	SaveGameHeader hdr;
	int len;

	makeSavegameName(filename, slot, false);
	if (!(out = mgr->open_savefile(filename, false))) {
		strcpy(desc, "");
		return false;
	}
	len = out->fread(&hdr, sizeof(hdr), 1);
	delete out;

	if (len != 1 || hdr.type != MKID('SCVM')) {
		strcpy(desc, "Invalid savegame");
		return false;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER_V7 || hdr.ver > CURRENT_VER) {
		strcpy(desc, "Invalid version");
		return false;
	}

	memcpy(desc, hdr.name, sizeof(hdr.name));
	desc[sizeof(hdr.name) - 1] = 0;
	return true;
}

void Scumm::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	const SaveLoadEntry objectEntries[] = {
		MKLINE(ObjectData, OBIMoffset, sleUint32, VER_V8),
		MKLINE(ObjectData, OBCDoffset, sleUint32, VER_V8),
		MKLINE(ObjectData, walk_x, sleUint16, VER_V8),
		MKLINE(ObjectData, walk_y, sleUint16, VER_V8),
		MKLINE(ObjectData, obj_nr, sleUint16, VER_V8),
		MKLINE(ObjectData, x_pos, sleInt16, VER_V8),
		MKLINE(ObjectData, y_pos, sleInt16, VER_V8),
		MKLINE(ObjectData, width, sleUint16, VER_V8),
		MKLINE(ObjectData, height, sleUint16, VER_V8),
		MKLINE(ObjectData, actordir, sleByte, VER_V8),
		MKLINE(ObjectData, parentstate, sleByte, VER_V8),
		MKLINE(ObjectData, parent, sleByte, VER_V8),
		MKLINE(ObjectData, state, sleByte, VER_V8),
		MKLINE(ObjectData, fl_object_index, sleByte, VER_V8),
		MKEND()
	};

	const SaveLoadEntry *actorEntries = Actor::getSaveLoadEntries();

	const SaveLoadEntry verbEntries[] = {
		MKLINE(VerbSlot, x, sleInt16, VER_V8),
		MKLINE(VerbSlot, y, sleInt16, VER_V8),
		MKLINE(VerbSlot, right, sleInt16, VER_V8),
		MKLINE(VerbSlot, bottom, sleInt16, VER_V8),
		MKLINE(VerbSlot, old.left, sleInt16, VER_V8),
		MKLINE(VerbSlot, old.top, sleInt16, VER_V8),
		MKLINE(VerbSlot, old.right, sleInt16, VER_V8),
		MKLINE(VerbSlot, old.bottom, sleInt16, VER_V8),

		MKLINE_OLD(VerbSlot, verbid, sleByte, VER_V8, VER_V11),
		MKLINE(VerbSlot, verbid, sleInt16, VER_V12),

		MKLINE(VerbSlot, color, sleByte, VER_V8),
		MKLINE(VerbSlot, hicolor, sleByte, VER_V8),
		MKLINE(VerbSlot, dimcolor, sleByte, VER_V8),
		MKLINE(VerbSlot, bkcolor, sleByte, VER_V8),
		MKLINE(VerbSlot, type, sleByte, VER_V8),
		MKLINE(VerbSlot, charset_nr, sleByte, VER_V8),
		MKLINE(VerbSlot, curmode, sleByte, VER_V8),
		MKLINE(VerbSlot, saveid, sleByte, VER_V8),
		MKLINE(VerbSlot, key, sleByte, VER_V8),
		MKLINE(VerbSlot, center, sleByte, VER_V8),
		MKLINE(VerbSlot, prep, sleByte, VER_V8),
		MKLINE(VerbSlot, imgindex, sleUint16, VER_V8),
		MKEND()
	};

	const SaveLoadEntry mainEntries[] = {
		MKLINE(Scumm, _roomWidth, sleUint16, VER_V8),
		MKLINE(Scumm, _roomHeight, sleUint16, VER_V8),
		MKLINE(Scumm, _ENCD_offs, sleUint32, VER_V8),
		MKLINE(Scumm, _EXCD_offs, sleUint32, VER_V8),
		MKLINE(Scumm, _IM00_offs, sleUint32, VER_V8),
		MKLINE(Scumm, _CLUT_offs, sleUint32, VER_V8),
		MK_OBSOLETE(Scumm, _EPAL_offs, sleUint32, VER_V8, VER_V9),
		MKLINE(Scumm, _PALS_offs, sleUint32, VER_V8),
		MKLINE(Scumm, _curPalIndex, sleByte, VER_V8),
		MKLINE(Scumm, _currentRoom, sleByte, VER_V8),
		MKLINE(Scumm, _roomResource, sleByte, VER_V8),
		MKLINE(Scumm, _numObjectsInRoom, sleByte, VER_V8),
		MKLINE(Scumm, _currentScript, sleByte, VER_V8),
		MKARRAY(Scumm, _localScriptList[0], sleUint32, NUM_LOCALSCRIPT, VER_V8),


		// vm.localvar grew from 25 to 40 script entries and then from
		// 16 to 32 bit variables (but that wasn't reflect here)... and
		// THEN from 16 to 25 variables.
		MKARRAY2_OLD(Scumm, vm.localvar[0][0], sleUint16, 17, 25, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER_V8, VER_V8),
		MKARRAY2_OLD(Scumm, vm.localvar[0][0], sleUint16, 17, 40, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER_V9, VER_V14),

		// We used to save 25 * 40 = 1000 blocks; but actually, each 'row consisted of 26 entry,
		// i.e. 26 * 40 = 1040. Thus the last 40 blocks of localvar where not saved at all. To be
		// able to load this screwed format, we use a trick: We load 26 * 38 = 988 blocks.
		// Then, we mark the followin 12 blocks (24 bytes) as obsolete.
		MKARRAY2_OLD(Scumm, vm.localvar[0][0], sleUint16, 26, 38, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER_V15, VER_V17),
		MK_OBSOLETE_ARRAY(Scumm, vm.localvar[39][0], sleUint16, 12, VER_V15, VER_V17),

		// This was the first proper multi dimensional version of the localvars, with 32 bit values
		MKARRAY2_OLD(Scumm, vm.localvar[0][0], sleUint32, 26, 40, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER_V18, VER_V19),

		// Then we doubled the script slots again, from 40 to 80
		MKARRAY2(Scumm, vm.localvar[0][0], sleUint32, 26, NUM_SCRIPT_SLOT, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER_V20),


		MKARRAY(Scumm, _resourceMapper[0], sleByte, 128, VER_V8),
		MKARRAY(Scumm, _charsetColorMap[0], sleByte, 16, VER_V8),
		
		// _charsetData grew from 10*16 to 15*16 bytes
		MKARRAY_OLD(Scumm, _charsetData[0][0], sleByte, 10 * 16, VER_V8, VER_V9),
		MKARRAY(Scumm, _charsetData[0][0], sleByte, 15 * 16, VER_V10),

		MKLINE(Scumm, _curExecScript, sleUint16, VER_V8),

		MKLINE(Scumm, camera._dest.x, sleInt16, VER_V8),
		MKLINE(Scumm, camera._dest.y, sleInt16, VER_V8),
		MKLINE(Scumm, camera._cur.x, sleInt16, VER_V8),
		MKLINE(Scumm, camera._cur.y, sleInt16, VER_V8),
		MKLINE(Scumm, camera._last.x, sleInt16, VER_V8),
		MKLINE(Scumm, camera._last.y, sleInt16, VER_V8),
		MKLINE(Scumm, camera._accel.x, sleInt16, VER_V8),
		MKLINE(Scumm, camera._accel.y, sleInt16, VER_V8),
		MKLINE(Scumm, _screenStartStrip, sleInt16, VER_V8),
		MKLINE(Scumm, _screenEndStrip, sleInt16, VER_V8),
		MKLINE(Scumm, camera._mode, sleByte, VER_V8),
		MKLINE(Scumm, camera._follows, sleByte, VER_V8),
		MKLINE(Scumm, camera._leftTrigger, sleInt16, VER_V8),
		MKLINE(Scumm, camera._rightTrigger, sleInt16, VER_V8),
		MKLINE(Scumm, camera._movingToActor, sleUint16, VER_V8),

		MKLINE(Scumm, _actorToPrintStrFor, sleByte, VER_V8),
		MKLINE(Scumm, _charsetColor, sleByte, VER_V8),

		// _charsetBufPos was changed from byte to int
		MKLINE_OLD(Scumm, _charsetBufPos, sleByte, VER_V8, VER_V9),
		MKLINE(Scumm, _charsetBufPos, sleInt16, VER_V10),

		MKLINE(Scumm, _haveMsg, sleByte, VER_V8),
		MKLINE(Scumm, _useTalkAnims, sleByte, VER_V8),

		MKLINE(Scumm, _talkDelay, sleInt16, VER_V8),
		MKLINE(Scumm, _defaultTalkDelay, sleInt16, VER_V8),
		MKLINE(Scumm, _numInMsgStack, sleInt16, VER_V8),
		MKLINE(Scumm, _sentenceNum, sleByte, VER_V8),

		MKLINE(Scumm, vm.cutSceneStackPointer, sleByte, VER_V8),
		MKARRAY(Scumm, vm.cutScenePtr[0], sleUint32, 5, VER_V8),
		MKARRAY(Scumm, vm.cutSceneScript[0], sleByte, 5, VER_V8),
		MKARRAY(Scumm, vm.cutSceneData[0], sleInt16, 5, VER_V8),
		MKLINE(Scumm, vm.cutSceneScriptIndex, sleInt16, VER_V8),

		MKLINE(Scumm, _numNestedScripts, sleByte, VER_V8),
		MKLINE(Scumm, _userPut, sleByte, VER_V8),
		MKLINE(Scumm, _userState, sleUint16, VER_V17),
		MKLINE(Scumm, _cursor.state, sleByte, VER_V8),
		MK_OBSOLETE(Scumm, gdi._cursorActive, sleByte, VER_V8, VER_V20),
		MKLINE(Scumm, _currentCursor, sleByte, VER_V8),
		MKARRAY(Scumm, _grabbedCursor[0], sleByte, 8192, VER_V20),
		MKLINE(Scumm, _cursor.width, sleInt16, VER_V20),
		MKLINE(Scumm, _cursor.height, sleInt16, VER_V20),
		MKLINE(Scumm, _cursor.hotspotX, sleInt16, VER_V20),
		MKLINE(Scumm, _cursor.hotspotY, sleInt16, VER_V20),
		MKLINE(Scumm, _cursor.animate, sleByte, VER_V20),
		MKLINE(Scumm, _cursor.animateIndex, sleByte, VER_V20),
		MKLINE(Scumm, _mouse.x, sleInt16, VER_V20),
		MKLINE(Scumm, _mouse.y, sleInt16, VER_V20),

		MKLINE(Scumm, _doEffect, sleByte, VER_V8),
		MKLINE(Scumm, _switchRoomEffect, sleByte, VER_V8),
		MKLINE(Scumm, _newEffect, sleByte, VER_V8),
		MKLINE(Scumm, _switchRoomEffect2, sleByte, VER_V8),
		MKLINE(Scumm, _BgNeedsRedraw, sleByte, VER_V8),

		// The state of palManipulate is stored only since V10
		MKLINE(Scumm, _palManipStart, sleByte, VER_V10),
		MKLINE(Scumm, _palManipEnd, sleByte, VER_V10),
		MKLINE(Scumm, _palManipCounter, sleUint16, VER_V10),

		// gfxUsageBits grew from 200 to 410 entries. Then 3 * 410 entries:
		MKARRAY_OLD(Scumm, gfxUsageBits[0], sleUint32, 200, VER_V8, VER_V9),
		MKARRAY_OLD(Scumm, gfxUsageBits[0], sleUint32, 410, VER_V10, VER_V13),
		MKARRAY(Scumm, gfxUsageBits[0], sleUint32, 3 * 410, VER_V14),

		MKLINE(Scumm, gdi._transparentColor, sleByte, VER_V8),
		MKARRAY(Scumm, _currentPalette[0], sleByte, 768, VER_V8),

		MKARRAY(Scumm, _proc_special_palette[0], sleByte, 256, VER_V8),

		MKARRAY(Scumm, _charsetBuffer[0], sleByte, 256, VER_V8),

		MKLINE(Scumm, _egoPositioned, sleByte, VER_V8),

		// gdi._imgBufOffs grew from 4 to 5 entries :
		MKARRAY_OLD(Scumm, gdi._imgBufOffs[0], sleUint16, 4, VER_V8, VER_V9),
		MKARRAY(Scumm, gdi._imgBufOffs[0], sleUint16, 5, VER_V10),

		MKLINE(Scumm, gdi._numZBuffer, sleByte, VER_V8),

		MKLINE(Scumm, _screenEffectFlag, sleByte, VER_V8),

		MK_OBSOLETE(Scumm, _randSeed1, sleUint32, VER_V8, VER_V9),
		MK_OBSOLETE(Scumm, _randSeed2, sleUint32, VER_V8, VER_V9),

		// Converted _shakeEnabled to boolean and added a _shakeFrame field.
		MKLINE_OLD(Scumm, _shakeEnabled, sleInt16, VER_V8, VER_V9),
		MKLINE(Scumm, _shakeEnabled, sleByte, VER_V10),
		MKLINE(Scumm, _shakeFrame, sleUint32, VER_V10),

		MKLINE(Scumm, _keepText, sleByte, VER_V8),

		MKLINE(Scumm, _screenB, sleUint16, VER_V8),
		MKLINE(Scumm, _screenH, sleUint16, VER_V8),

		MK_OBSOLETE(Scumm, _cd_track, sleInt16, VER_V9, VER_V9),
		MK_OBSOLETE(Scumm, _cd_loops, sleInt16, VER_V9, VER_V9),
		MK_OBSOLETE(Scumm, _cd_frame, sleInt16, VER_V9, VER_V9),
		MK_OBSOLETE(Scumm, _cd_end, sleInt16, VER_V9, VER_V9),
		
		MKEND()
	};

	const SaveLoadEntry scriptSlotEntries[] = {
		MKLINE(ScriptSlot, offs, sleUint32, VER_V8),
		MKLINE(ScriptSlot, delay, sleInt32, VER_V8),
		MKLINE(ScriptSlot, number, sleUint16, VER_V8),
		MKLINE(ScriptSlot, delayFrameCount, sleUint16, VER_V8),
		MKLINE(ScriptSlot, status, sleByte, VER_V8),
		MKLINE(ScriptSlot, where, sleByte, VER_V8),
		MKLINE(ScriptSlot, freezeResistant, sleByte, VER_V8),
		MKLINE(ScriptSlot, recursive, sleByte, VER_V8),
		MKLINE(ScriptSlot, freezeCount, sleByte, VER_V8),
		MKLINE(ScriptSlot, didexec, sleByte, VER_V8),
		MKLINE(ScriptSlot, cutsceneOverride, sleByte, VER_V8),
		MK_OBSOLETE(ScriptSlot, unk5, sleByte, VER_V8, VER_V10),
		MKEND()
	};

	const SaveLoadEntry nestedScriptEntries[] = {
		MKLINE(NestedScript, number, sleUint16, VER_V8),
		MKLINE(NestedScript, where, sleByte, VER_V8),
		MKLINE(NestedScript, slot, sleByte, VER_V8),
		MKEND()
	};

	const SaveLoadEntry sentenceTabEntries[] = {
		MKLINE(SentenceTab, verb, sleUint8, VER_V8),
		MKLINE(SentenceTab, preposition, sleUint8, VER_V8),
		MKLINE(SentenceTab, objectA, sleUint16, VER_V8),
		MKLINE(SentenceTab, objectB, sleUint16, VER_V8),
		MKLINE(SentenceTab, freezeCount, sleUint8, VER_V8),
		MKEND()
	};

	const SaveLoadEntry stringTabEntries[] = {
		// TODO - It makes no sense to have all these t_* fields in StringTab
		// Rather let's dump them all when the save game format changes, and 
		// keep two StringTab objects where we have one now: a "normal" one,
		// and a temporar y"t_" one.
		// Then backup/restore of a StringTab entry becomes a one liner.
		MKLINE(StringTab, xpos, sleInt16, VER_V8),
		MKLINE(StringTab, t_xpos, sleInt16, VER_V8),
		MKLINE(StringTab, ypos, sleInt16, VER_V8),
		MKLINE(StringTab, t_ypos, sleInt16, VER_V8),
		MKLINE(StringTab, right, sleInt16, VER_V8),
		MKLINE(StringTab, t_right, sleInt16, VER_V8),
		MKLINE(StringTab, color, sleInt8, VER_V8),
		MKLINE(StringTab, t_color, sleInt8, VER_V8),
		MKLINE(StringTab, charset, sleInt8, VER_V8),
		MKLINE(StringTab, t_charset, sleInt8, VER_V8),
		MKLINE(StringTab, center, sleByte, VER_V8),
		MKLINE(StringTab, t_center, sleByte, VER_V8),
		MKLINE(StringTab, overhead, sleByte, VER_V8),
		MKLINE(StringTab, t_overhead, sleByte, VER_V8),
		MKLINE(StringTab, no_talk_anim, sleByte, VER_V8),
		MKLINE(StringTab, t_no_talk_anim, sleByte, VER_V8),
		MKEND()
	};

	const SaveLoadEntry colorCycleEntries[] = {
		MKLINE(ColorCycle, delay, sleUint16, VER_V8),
		MKLINE(ColorCycle, counter, sleUint16, VER_V8),
		MKLINE(ColorCycle, flags, sleUint16, VER_V8),
		MKLINE(ColorCycle, start, sleByte, VER_V8),
		MKLINE(ColorCycle, end, sleByte, VER_V8),
		MKEND()
	};

	const SaveLoadEntry scaleSlotsEntries[] = {
		MKLINE(ScaleSlot, x1, sleUint16, VER_V13),
		MKLINE(ScaleSlot, y1, sleUint16, VER_V13),
		MKLINE(ScaleSlot, scale1, sleUint16, VER_V13),
		MKLINE(ScaleSlot, x2, sleUint16, VER_V13),
		MKLINE(ScaleSlot, y2, sleUint16, VER_V13),
		MKLINE(ScaleSlot, scale2, sleUint16, VER_V13),
		MKEND()
	};

	int i, j;
	int var120Backup;
	int var98Backup;

	if (!s->isSaving() && (_saveSound || !_saveLoadCompatible)) {
		_sound->stopAllSounds();
		if (_mixer) {
			if (_imuseDigital) {
				_imuseDigital->stopAll();
			} else {
				_mixer->stopAll();
			}
		}
	}

	// Because old savegames won't fill the entire gfxUsageBits[] array,
	// clear it here just to be sure it won't hold any unforseen garbage.
	if (!s->isSaving())
		memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	s->saveLoadEntries(this, mainEntries);

	if (!s->isSaving() && savegameVersion < VER_V14)
		upgradeGfxUsageBits();

	if (!s->isSaving() && savegameVersion >= VER_V20) {
		updateCursor();
		_system->warp_mouse(_mouse.x, _mouse.y);
	}

	s->saveLoadArrayOf(_actors, _numActors, sizeof(_actors[0]), actorEntries);

	if (savegameVersion < VER_V9)
		s->saveLoadArrayOf(vm.slot, 25, sizeof(vm.slot[0]), scriptSlotEntries);
	else if (savegameVersion < VER_V20)
		s->saveLoadArrayOf(vm.slot, 40, sizeof(vm.slot[0]), scriptSlotEntries);
	else
		s->saveLoadArrayOf(vm.slot, NUM_SCRIPT_SLOT, sizeof(vm.slot[0]), scriptSlotEntries);

	s->saveLoadArrayOf(_objs, _numLocalObjects, sizeof(_objs[0]), objectEntries);
	if (!s->isSaving() && savegameVersion < VER_V13) {
		// Since roughly v13 of the save games, the objs storage has changed a bit
		for (i = _numObjectsInRoom; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}

	}
	s->saveLoadArrayOf(_verbs, _numVerbs, sizeof(_verbs[0]), verbEntries);
	s->saveLoadArrayOf(vm.nest, 16, sizeof(vm.nest[0]), nestedScriptEntries);
	s->saveLoadArrayOf(_sentence, 6, sizeof(_sentence[0]), sentenceTabEntries);
	s->saveLoadArrayOf(_string, 6, sizeof(_string[0]), stringTabEntries);
	s->saveLoadArrayOf(_colorCycle, 16, sizeof(_colorCycle[0]), colorCycleEntries);

	if (savegameVersion >= VER_V13)
		s->saveLoadArrayOf(_scaleSlots, 20, sizeof(_scaleSlots[0]), scaleSlotsEntries);

	// Save all resource. Fingolfin doesn't like this part of the save/load code a bit.
	// It is very fragile: e.g. if we change the num limit for one resource type, this
	// code will break down. Worse, there is no way such a problem could easily be detected.
	// We should at least store for each save resource it's type and ID. Then at least
	// we can perform some integrety checks when loading.
	for (i = rtFirst; i <= rtLast; i++)
		if (res.mode[i] == 0)
			for (j = 1; j < res.num[i]; j++)
				saveLoadResource(s, i, j);

	s->saveLoadArrayOf(_objectOwnerTable, _numGlobalObjects, sizeof(_objectOwnerTable[0]), sleByte);
	s->saveLoadArrayOf(_objectStateTable, _numGlobalObjects, sizeof(_objectStateTable[0]), sleByte);
	if (_objectRoomTable)
		s->saveLoadArrayOf(_objectRoomTable, _numGlobalObjects, sizeof(_objectRoomTable[0]), sleByte);

	if (_shadowPaletteSize)
		s->saveLoadArrayOf(_shadowPalette, _shadowPaletteSize, 1, sleByte);

	// PalManip data was not saved before V10 save games
	if (savegameVersion < VER_V10)
		_palManipCounter = 0;
	if (_palManipCounter) {
		if (!_palManipPalette)
			_palManipPalette = (byte *)calloc(0x300, 1);
		if (!_palManipIntermediatePal)
			_palManipPalette = (byte *)calloc(0x300, 1);
		s->saveLoadArrayOf(_palManipPalette, 0x300, 1, sleByte);
		s->saveLoadArrayOf(_palManipIntermediatePal, 0x600, 1, sleByte);
	}

	s->saveLoadArrayOf(_classData, _numGlobalObjects, sizeof(_classData[0]), sleUint32);

	var120Backup = _scummVars[120];
	var98Backup = _scummVars[98];

	// The variables grew from 16 to 32 bit.
	if (savegameVersion < VER_V15)
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt16);
	else
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt32);

	if (_gameId == GID_TENTACLE)	// Maybe misplaced, but that's the main idea
		_scummVars[120] = var120Backup;
	if (_gameId == GID_INDY4)
		_scummVars[98] = var98Backup;;

	s->saveLoadArrayOf(_bitVars, _numBitVariables >> 3, 1, sleByte);

	/* Save or load a list of the locked objects */
	if (s->isSaving()) {
		for (i = rtFirst; i <= rtLast; i++)
			for (j = 1; j < res.num[i]; j++) {
				if (res.flags[i][j] & RF_LOCK) {
					s->saveByte(i);
					s->saveWord(j);
				}
			}
		s->saveByte(0xFF);
	} else {
		int r;
		while ((r = s->loadByte()) != 0xFF) {
			res.flags[r][s->loadWord()] |= RF_LOCK;
		}
	}
	
	if (_imuse && (_saveSound || !_saveLoadCompatible)) {
		_imuse->save_or_load(s, this);
		_imuse->set_master_volume (_sound->_sound_volume_master);
		_imuse->set_music_volume (_sound->_sound_volume_music);
	}
}

void Scumm::saveLoadResource(Serializer *ser, int type, int idx) {
	byte *ptr;
	uint32 size;

	/* don't save/load these resource types */
	if (type == rtTemp || type == rtBuffer || res.mode[type])
		return;

	if (ser->isSaving()) {
		ptr = res.address[type][idx];
		if (ptr == NULL) {
			ser->saveUint32(0);
			return;
		}

		size = ((MemBlkHeader *)ptr)->size;

		ser->saveUint32(size);
		ser->saveBytes(ptr + sizeof(MemBlkHeader), size);

		if (type == rtInventory) {
			ser->saveWord(_inventory[idx]);
		}
	} else {
		size = ser->loadUint32();
		if (size) {
			createResource(type, idx, size);
			ser->loadBytes(getResourceAddress(type, idx), size);
			if (type == rtInventory) {
				_inventory[idx] = ser->loadWord();
			}
		}
	}
}

void Serializer::saveBytes(void *b, int len) {
	_saveLoadStream->fwrite(b, 1, len);
}

void Serializer::loadBytes(void *b, int len) {
	_saveLoadStream->fread(b, 1, len);
}

#if defined(__PALM_OS__)
bool Serializer::checkEOFLoadStream() {

	if (_saveLoadStream->feof())
		return true;

	return false;
}
#endif


void Serializer::saveUint32(uint32 d) {
	uint32 e = FROM_LE_32(d);
	saveBytes(&e, 4);
}

void Serializer::saveWord(uint16 d) {
	uint16 e = FROM_LE_16(d);
	saveBytes(&e, 2);
}

void Serializer::saveByte(byte b) {
	saveBytes(&b, 1);
}

uint32 Serializer::loadUint32() {
	uint32 e;
	loadBytes(&e, 4);
	return FROM_LE_32(e);
}

uint16 Serializer::loadWord() {
	uint16 e;
	loadBytes(&e, 2);
	return FROM_LE_16(e);
}

byte Serializer::loadByte() {
	byte e;
	loadBytes(&e, 1);
	return e;
}

void Serializer::saveArrayOf(void *b, int len, int datasize, byte filetype) {
	byte *at = (byte *)b;
	uint32 data;

	// speed up byte arrays
	if (datasize == 1 && filetype == sleByte) {
		saveBytes(b, len);
		return;
	}

	while (--len >= 0) {
		if (datasize == 0) {
			// Do nothing for obsolete data
			data = 0;
		} else if (datasize == 1) {
			data = *(byte *)at;
			at += 1;
		} else if (datasize == 2) {
			data = *(uint16 *)at;
			at += 2;
		} else if (datasize == 4) {
			data = *(uint32 *)at;
			at += 4;
		} else {
			error("saveLoadArrayOf: invalid size %d", datasize);
		}
		switch (filetype) {
		case sleByte:
			saveByte((byte)data);
			break;
		case sleUint16:
		case sleInt16:
			saveWord((int16)data);
			break;
		case sleInt32:
		case sleUint32:
			saveUint32(data);
			break;
		default:
			error("saveLoadArrayOf: invalid filetype %d", filetype);
		}
	}
}

void Serializer::loadArrayOf(void *b, int len, int datasize, byte filetype) {
	byte *at = (byte *)b;
	uint32 data;

	// speed up byte arrays
	if (datasize == 1 && filetype == sleByte) {
		loadBytes(b, len);
		return;
	}

	while (--len >= 0) {
		switch (filetype) {
		case sleByte:
			data = loadByte();
			break;
		case sleUint16:
			data = loadWord();
			break;
		case sleInt16:
			data = (int16)loadWord();
			break;
		case sleUint32:
			data = loadUint32();
			break;
		case sleInt32:
			data = (int32)loadUint32();
			break;
		default:
			error("saveLoadArrayOf: invalid filetype %d", filetype);
		}
		if (datasize == 0) {
			// Do nothing for obsolete data
		} else if (datasize == 1) {
			*(byte *)at = (byte)data;
			at += 1;
		} else if (datasize == 2) {
			*(uint16 *)at = (uint16)data;
			at += 2;
		} else if (datasize == 4) {
			*(uint32 *)at = data;
			at += 4;
		} else {
			error("saveLoadArrayOf: invalid size %d", datasize);
		}
	}
}

void Serializer::saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle) {
	byte *data = (byte *)b;

	if (isSaving()) {
		while (--num >= 0) {
			saveEntries(data, sle);
			data += datasize;
		}
	} else {
		while (--num >= 0) {
			loadEntries(data, sle);
			data += datasize;
		}
	}
}

void Serializer::saveLoadArrayOf(void *b, int len, int datasize, byte filetype) {
	if (isSaving())
		saveArrayOf(b, len, datasize, filetype);
	else
		loadArrayOf(b, len, datasize, filetype);
}

void Serializer::saveLoadEntries(void *d, const SaveLoadEntry *sle) {
	if (isSaving())
		saveEntries(d, sle);
	else
		loadEntries(d, sle);
}

void Serializer::saveEntries(void *d, const SaveLoadEntry *sle) {
	byte type;
	byte *at;
	int size;

	while (sle->offs != 0xFFFF) {
		at = (byte *)d + sle->offs;
		size = sle->size;
		type = (byte) sle->type;

		if (sle->maxVersion != CURRENT_VER) {
			// Skip obsolete entries
			if (type & 128)
				sle++;
		} else if (size == 0xFF) {
			// save reference
			void *ptr = *((void **)at);
			saveWord(ptr ? ((*_save_ref) (_ref_me, type, ptr) + 1) : 0);
		} else {
			// save entry
			int columns = 1;
			int rows = 1;
			int rowlen = 0;
			if (type & 128) {
				sle++;
				columns = sle->offs;
				rows = sle->type;
				rowlen = sle->size;
				type &= ~128;
			}
			while (rows--) {
				saveArrayOf(at, columns, size, type);
				at += rowlen;
			}
		}
		sle++;
	}
}

void Serializer::loadEntries(void *d, const SaveLoadEntry *sle) {
	byte type;
	byte *at;
	int size;

	while (sle->offs != 0xFFFF) {
		at = (byte *)d + sle->offs;
		size = sle->size;
		type = (byte) sle->type;

		if (_savegameVersion < sle->minVersion || _savegameVersion > sle->maxVersion) {
			// Skip entries which are not present in this save game version
			if (type & 128)
				sle++;
		} else if (size == 0xFF) {
			// load reference...
			int num = loadWord();
			// ...but only use it if it's still there in CURRENT_VER
			if (sle->maxVersion == CURRENT_VER)
				*((void **)at) = num ? (*_load_ref) (_ref_me, type, num - 1) : NULL;
		} else {
			// load entry
			int columns = 1;
			int rows = 1;
			int rowlen = 0;

			if (type & 128) {
				sle++;
				columns = sle->offs;
				rows = sle->type;
				rowlen = sle->size;
				type &= ~128;
			}
			while (rows--) {
				loadArrayOf(at, columns, size, type);
				at += rowlen;
			}
		}
		sle++;
	}
}
