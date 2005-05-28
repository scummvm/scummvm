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

#include "stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "scumm/wiz_he.h"

#include "sound/audiocd.h"
#include "sound/mixer.h"

namespace Scumm {

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 ver;
	char name[32];
};


void ScummEngine::requestSave(int slot, const char *name, bool temporary) {
	_saveLoadSlot = slot;
	_saveTemporaryState = temporary;
	_saveLoadFlag = 1;		// 1 for save
	assert(name);
	strcpy(_saveLoadName, name);
}

void ScummEngine::requestLoad(int slot) {
	_saveLoadSlot = slot;
	_saveTemporaryState = false;
	_saveLoadFlag = 2;		// 2 for load
}

bool ScummEngine::saveState(int slot, bool compat) {
	char filename[256];
	Common::OutSaveFile *out;
	SaveGameHeader hdr;

	makeSavegameName(filename, slot, compat);

	if (!(out = _saveFileMan->openForSaving(filename)))
		return false;

	memcpy(hdr.name, _saveLoadName, sizeof(hdr.name));

	hdr.type = MKID('SCVM');
	hdr.size = 0;
	hdr.ver = TO_LE_32(CURRENT_VER);

	out->write(&hdr, sizeof(hdr));
	saveThumbnail(out);

	Serializer ser(0, out, CURRENT_VER);
	saveOrLoad(&ser, CURRENT_VER);
	delete out;
	debug(1, "State saved as '%s'", filename);
	return true;
}

bool ScummEngine::loadState(int slot, bool compat) {
	char filename[256];
	Common::InSaveFile *in;
	int i, j;
	SaveGameHeader hdr;
	int sb, sh;

	makeSavegameName(filename, slot, compat);
	if (!(in = _saveFileMan->openForLoading(filename)))
		return false;

	in->read(&hdr, sizeof(hdr));
	if (hdr.type != MKID('SCVM')) {
		warning("Invalid savegame '%s'", filename);
		delete in;
		return false;
	}

	// In older versions of ScummVM, the header version was not endian safe.
	// We account for that by retrying once with swapped byte order.
	if (hdr.ver > CURRENT_VER)
		hdr.ver = SWAP_BYTES_32(hdr.ver);
	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER)
	{
		warning("Invalid version of '%s'", filename);
		delete in;
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(50) && _heversion >= 71) {
		warning("Unsupported version of '%s'", filename);
		delete in;
		return false;
	}
	
	// Since version 52 a thumbnail is saved directly after the header
	if (hdr.ver >= VER(52)) {
		uint32 type;
		in->read(&type, 4);

		// Check for the THMB header. Also, work around a bug which caused
		// the chunk type (incorrectly) to be written in LE on LE machines.
		if (! (type == MKID('THMB') || (hdr.ver < VER(55) && type == MKID('BMHT')))){
			warning("Can not load thumbnail");
			delete in;
			return false;
		}
		uint32 size = in->readUint32BE();
		in->skip(size - 8);
	}

	// Due to a bug in scummvm up to and including 0.3.0, save games could be saved
	// in the V8/V9 format but were tagged with a V7 mark. Ouch. So we just pretend V7 == V8 here
	if (hdr.ver == VER(7))
		hdr.ver = VER(8);

	memcpy(_saveLoadName, hdr.name, sizeof(hdr.name));

	// Unless specifically requested with _saveSound, we do not save the iMUSE
	// state for temporary state saves - such as certain cutscenes in DOTT,
	// FOA, Sam and Max, etc.
	//
	// Thusly, we should probably not stop music when restoring from one of 
	// these saves. This change stops the Mole Man theme from going quiet in
	// Sam & Max when Doug tells you about the Ball of Twine, as mentioned in
	// patch #886058.
	//
	// If we don't have iMUSE at all we may as well stop the sounds. The previous
	// default behavior here was to stopAllSounds on all state restores.

	if (!_imuse || _saveSound || !_saveTemporaryState)
		_sound->stopAllSounds();

#ifndef DISABLE_SCUMM_7_8
	if (_imuseDigital) {
		_imuseDigital->stopAllSounds();
		_imuseDigital->resetState();
	}
#endif

	_sound->stopCD();

	_sound->pauseSounds(true);

	CHECK_HEAP

	closeRoom();

	memset(_inventory, 0, sizeof(_inventory[0]) * _numInventory);
	memset(_newNames, 0, sizeof(_newNames[0]) * _numNewNames);
	
	// Because old savegames won't fill the entire gfxUsageBits[] array,
	// clear it here just to be sure it won't hold any unforseen garbage.
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	// Nuke all resources
	for (i = rtFirst; i <= rtLast; i++)
		if (i != rtTemp && i != rtBuffer && (i != rtSound || _saveSound || !compat))
			for (j = 0; j < res.num[i]; j++) {
				res.nukeResource(i, j);
				res.flags[i][j] = 0;
			}

	initScummVars();

	if (_features & GF_OLD_BUNDLE)
		loadCharset(0); // FIXME - HACK ?

	//
	// Now do the actual loading
	//
	Serializer ser(in, 0, hdr.ver);
	saveOrLoad(&ser, hdr.ver);
	delete in;
	
	// Update volume settings
	setupVolumes();

	// Init NES costume data
	if (_platform == Common::kPlatformNES) {
		if (hdr.ver < VER(47))
			_NESCostumeSet = 0;
		NES_loadCostumeSet(_NESCostumeSet);
	}

	// Normally, _vm->_screenTop should always be >= 0, but for some old save games
	// it is not, hence we check & correct it here.
	if (_screenTop < 0)
		_screenTop = 0;
	
	if (hdr.ver < VER(33) && _version >= 7) {
		// For a long time, we didn't set these vars to default values.
		VAR(VAR_DEFAULT_TALK_DELAY) = 60;
		if (_version == 7)
			VAR(VAR_NUM_GLOBAL_OBJS) = _numGlobalObjects - 1;
	}

	if (hdr.ver < VER(30)) {
		// For a long time, we used incorrect location, causing it to default to zero.
		if (_version == 8)
			_scummVars[VAR_CHARINC] = (_features & GF_DEMO) ? 3 : 1;
		// Needed due to subtitle speed changes
		_defaultTalkDelay /= 20;
	}

	// For a long time, we used incorrect locations for some camera related
	// scumm vars. We now know the proper locations. To be able to properly use
	// old save games, we update the old (bad) variables to the new (correct)
	// ones.
	if (hdr.ver < VER(28) && _version == 8) {
		_scummVars[VAR_CAMERA_MIN_X] = _scummVars[101];
		_scummVars[VAR_CAMERA_MAX_X] = _scummVars[102];
		_scummVars[VAR_CAMERA_MIN_Y] = _scummVars[103];
		_scummVars[VAR_CAMERA_MAX_Y] = _scummVars[104];
		_scummVars[VAR_CAMERA_THRESHOLD_X] = _scummVars[105];
		_scummVars[VAR_CAMERA_THRESHOLD_Y] = _scummVars[106];
		_scummVars[VAR_CAMERA_SPEED_X] = _scummVars[107];
		_scummVars[VAR_CAMERA_SPEED_Y] = _scummVars[108];
		_scummVars[VAR_CAMERA_ACCEL_X] = _scummVars[109];
		_scummVars[VAR_CAMERA_ACCEL_Y] = _scummVars[110];
	}

	// With version 22, we replaced the scale items with scale slots. So when
	// loading such an old save game, try to upgrade the old to new format.
	if (hdr.ver < VER(22)) {
		// Convert all rtScaleTable resources to matching scale items
		for (i = 1; i < res.num[rtScaleTable]; i++) {
			convertScaleTableToScaleSlot(i);
		}
	}

	// We could simply dirty colours 0-15 for 16-colour games -- nowadays
	// they handle their palette pretty much like the more recent games
	// anyway. There was a time, though, when re-initializing was necessary
	// for backwards compatibility, and it may still prove useful if we
	// ever add options for using different 16-colour palettes.
	if (_version == 1) {
		if (_platform == Common::kPlatformC64)
			setupC64Palette();
		else {
			if (_gameId == GID_MANIAC)
				if (_platform == Common::kPlatformNES)
					setupNESPalette();
				else
					setupV1ManiacPalette();
			else
				setupV1ZakPalette();
		}
	} else if (_features & GF_16COLOR) {
		switch (_renderMode) {
		case Common::kRenderEGA:
			setupEGAPalette();
			break;

		case Common::kRenderAmiga:
			setupAmigaPalette();
			break;

		case Common::kRenderCGA:
			setupCGAPalette();
			break;

		case Common::kRenderHercA:
		case Common::kRenderHercG:
			setupHercPalette();
			break;

		default:
			if ((_platform == Common::kPlatformAmiga) || (_platform == Common::kPlatformAtariST))
				setupAmigaPalette();
			else
				setupEGAPalette();
		}
	} else
		setDirtyColors(0, 255);


	if (hdr.ver < VER(35) && _gameId == GID_MANIAC && _version == 1)
		setupV1ActorTalkColor();

	// Load the static room data
	loadRoomSubBlocks();

	if (!(_features & GF_NEW_CAMERA)) {
		camera._last.x = camera._cur.x;
	}

	sb = _screenB;
	sh = _screenH;

	// Restore the virtual screens and force a fade to black.
	initScreens(0, _screenHeight);

	VirtScreen *vs = &virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	vs->setDirtyRange(0, vs->h);
	updateDirtyScreen(kMainVirtScreen);
	updatePalette();
	initScreens(sb, sh);

	_completeScreenRedraw = true;

	// Reset charset mask
	_charset->_hasMask = false;
	_charset->clearTextSurface();

	_lastCodePtr = NULL;
	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

	cameraMoved();

	initBGBuffers(_roomHeight);

	if (VAR_ROOM_FLAG != 0xFF)
		VAR(VAR_ROOM_FLAG) = 1;

	// Sync with current config setting
	if (_version >= 7)
		VAR(VAR_VOICE_MODE) = ConfMan.getBool("subtitles");

	CHECK_HEAP
	debug(1, "State loaded from '%s'", filename);

	_sound->pauseSounds(false);

	return true;
}

void ScummEngine::makeSavegameName(char *out, int slot, bool temporary) {
	sprintf(out, "%s.%c%.2d", _targetName.c_str(), temporary ? 'c' : 's', slot);
}

void ScummEngine::listSavegames(bool *marks, int num) {
	char prefix[256];
	makeSavegameName(prefix, 99, false);
	prefix[strlen(prefix)-2] = 0;
	_saveFileMan->listSavefiles(prefix, marks, num);
}

bool ScummEngine::getSavegameName(int slot, char *desc) {
	char filename[256];
	Common::InSaveFile *in;
	SaveGameHeader hdr;
	int len;

	makeSavegameName(filename, slot, false);
	if (!(in = _saveFileMan->openForLoading(filename))) {
		strcpy(desc, "");
		return false;
	}
	len = in->read(&hdr, sizeof(hdr));
	delete in;

	if (len != sizeof(hdr) || hdr.type != MKID('SCVM')) {
		strcpy(desc, "Invalid savegame");
		return false;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER) {
		strcpy(desc, "Invalid version");
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(50) && _heversion >= 71) {
		strcpy(desc, "Unsupported version");
		return false;
	}

	memcpy(desc, hdr.name, sizeof(hdr.name));
	desc[sizeof(hdr.name) - 1] = 0;
	return true;
}

Graphics::Surface *ScummEngine::loadThumbnailFromSlot(int slot) {
	char filename[256];
	Common::InSaveFile *in;
	SaveGameHeader hdr;
	int len;
	
	makeSavegameName(filename, slot, false);
	if (!(in = _saveFileMan->openForLoading(filename))) {
		return 0;
	}
	len = in->read(&hdr, sizeof(hdr));

	if (len != sizeof(hdr) || hdr.type != MKID('SCVM')) {
		delete in;
		return 0;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER(52)) {
		delete in;
		return 0;
	}

	Graphics::Surface *thumb = loadThumbnail(in);

	delete in;
	return thumb;
}

void ScummEngine::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	const SaveLoadEntry objectEntries[] = {
		MKLINE(ObjectData, OBIMoffset, sleUint32, VER(8)),
		MKLINE(ObjectData, OBCDoffset, sleUint32, VER(8)),
		MKLINE(ObjectData, walk_x, sleUint16, VER(8)),
		MKLINE(ObjectData, walk_y, sleUint16, VER(8)),
		MKLINE(ObjectData, obj_nr, sleUint16, VER(8)),
		MKLINE(ObjectData, x_pos, sleInt16, VER(8)),
		MKLINE(ObjectData, y_pos, sleInt16, VER(8)),
		MKLINE(ObjectData, width, sleUint16, VER(8)),
		MKLINE(ObjectData, height, sleUint16, VER(8)),
		MKLINE(ObjectData, actordir, sleByte, VER(8)),
		MKLINE(ObjectData, parentstate, sleByte, VER(8)),
		MKLINE(ObjectData, parent, sleByte, VER(8)),
		MKLINE(ObjectData, state, sleByte, VER(8)),
		MKLINE(ObjectData, fl_object_index, sleByte, VER(8)),
		MKLINE(ObjectData, flags, sleByte, VER(46)),
		MKEND()
	};

	const SaveLoadEntry *actorEntries = Actor::getSaveLoadEntries();
	const SaveLoadEntry *soundEntries = _sound->getSaveLoadEntries();

	const SaveLoadEntry verbEntries[] = {
		MKLINE(VerbSlot, curRect.left, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.top, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.right, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.bottom, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.left, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.top, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.right, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.bottom, sleInt16, VER(8)),

		MKLINE_OLD(VerbSlot, verbid, sleByte, VER(8), VER(11)),
		MKLINE(VerbSlot, verbid, sleInt16, VER(12)),

		MKLINE(VerbSlot, color, sleByte, VER(8)),
		MKLINE(VerbSlot, hicolor, sleByte, VER(8)),
		MKLINE(VerbSlot, dimcolor, sleByte, VER(8)),
		MKLINE(VerbSlot, bkcolor, sleByte, VER(8)),
		MKLINE(VerbSlot, type, sleByte, VER(8)),
		MKLINE(VerbSlot, charset_nr, sleByte, VER(8)),
		MKLINE(VerbSlot, curmode, sleByte, VER(8)),
		MKLINE(VerbSlot, saveid, sleByte, VER(8)),
		MKLINE(VerbSlot, key, sleByte, VER(8)),
		MKLINE(VerbSlot, center, sleByte, VER(8)),
		MKLINE(VerbSlot, prep, sleByte, VER(8)),
		MKLINE(VerbSlot, imgindex, sleUint16, VER(8)),
		MKEND()
	};

	const SaveLoadEntry mainEntries[] = {
		MKARRAY(ScummEngine, _gameMD5[0], sleUint8, 16, VER(39)),
		MK_OBSOLETE(ScummEngine, _roomWidth, sleUint16, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _roomHeight, sleUint16, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _ENCD_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _EXCD_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _IM00_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _CLUT_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _EPAL_offs, sleUint32, VER(8), VER(9)),
		MK_OBSOLETE(ScummEngine, _PALS_offs, sleUint32, VER(8), VER(50)),
		MKLINE(ScummEngine, _curPalIndex, sleByte, VER(8)),
		MKLINE(ScummEngine, _currentRoom, sleByte, VER(8)),
		MKLINE(ScummEngine, _roomResource, sleByte, VER(8)),
		MKLINE(ScummEngine, _numObjectsInRoom, sleByte, VER(8)),
		MKLINE(ScummEngine, _currentScript, sleByte, VER(8)),
		MK_OBSOLETE_ARRAY(ScummEngine, _localScriptOffsets[0], sleUint32, _numLocalScripts, VER(8), VER(50)),


		// vm.localvar grew from 25 to 40 script entries and then from
		// 16 to 32 bit variables (but that wasn't reflect here)... and
		// THEN from 16 to 25 variables.
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 17, 25, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER(8), VER(8)),
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 17, 40, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER(9), VER(14)),

		// We used to save 25 * 40 = 1000 blocks; but actually, each 'row consisted of 26 entry,
		// i.e. 26 * 40 = 1040. Thus the last 40 blocks of localvar where not saved at all. To be
		// able to load this screwed format, we use a trick: We load 26 * 38 = 988 blocks.
		// Then, we mark the followin 12 blocks (24 bytes) as obsolete.
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 26, 38, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER(15), VER(17)),
		MK_OBSOLETE_ARRAY(ScummEngine, vm.localvar[39][0], sleUint16, 12, VER(15), VER(17)),

		// This was the first proper multi dimensional version of the localvars, with 32 bit values
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint32, 26, 40, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER(18), VER(19)),

		// Then we doubled the script slots again, from 40 to 80
		MKARRAY2(ScummEngine, vm.localvar[0][0], sleUint32, 26, NUM_SCRIPT_SLOT, (byte*)vm.localvar[1] - (byte*)vm.localvar[0], VER(20)),


		MKARRAY(ScummEngine, _resourceMapper[0], sleByte, 128, VER(8)),
		MKARRAY(ScummEngine, _charsetColorMap[0], sleByte, 16, VER(8)),
		
		// _charsetData grew from 10*16 to 15*16 bytes
		MKARRAY_OLD(ScummEngine, _charsetData[0][0], sleByte, 10 * 16, VER(8), VER(9)),
		MKARRAY(ScummEngine, _charsetData[0][0], sleByte, 15 * 16, VER(10)),

		MKLINE(ScummEngine, _curExecScript, sleUint16, VER(8)),

		MKLINE(ScummEngine, camera._dest.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._dest.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._cur.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._cur.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._last.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._last.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._accel.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._accel.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, _screenStartStrip, sleInt16, VER(8)),
		MKLINE(ScummEngine, _screenEndStrip, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._mode, sleByte, VER(8)),
		MKLINE(ScummEngine, camera._follows, sleByte, VER(8)),
		MKLINE(ScummEngine, camera._leftTrigger, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._rightTrigger, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._movingToActor, sleUint16, VER(8)),

		MKLINE(ScummEngine, _actorToPrintStrFor, sleByte, VER(8)),
		MKLINE(ScummEngine, _charsetColor, sleByte, VER(8)),

		// _charsetBufPos was changed from byte to int
		MKLINE_OLD(ScummEngine, _charsetBufPos, sleByte, VER(8), VER(9)),
		MKLINE(ScummEngine, _charsetBufPos, sleInt16, VER(10)),

		MKLINE(ScummEngine, _haveMsg, sleByte, VER(8)),
		MKLINE(ScummEngine, _useTalkAnims, sleByte, VER(8)),

		MKLINE(ScummEngine, _talkDelay, sleInt16, VER(8)),
		MKLINE(ScummEngine, _defaultTalkDelay, sleInt16, VER(8)),
		MK_OBSOLETE(ScummEngine, _numInMsgStack, sleInt16, VER(8), VER(27)),
		MKLINE(ScummEngine, _sentenceNum, sleByte, VER(8)),

		MKLINE(ScummEngine, vm.cutSceneStackPointer, sleByte, VER(8)),
		MKARRAY(ScummEngine, vm.cutScenePtr[0], sleUint32, 5, VER(8)),
		MKARRAY(ScummEngine, vm.cutSceneScript[0], sleByte, 5, VER(8)),
		MKARRAY(ScummEngine, vm.cutSceneData[0], sleInt16, 5, VER(8)),
		MKLINE(ScummEngine, vm.cutSceneScriptIndex, sleInt16, VER(8)),

		MKLINE(ScummEngine, vm.numNestedScripts, sleByte, VER(8)),
		MKLINE(ScummEngine, _userPut, sleByte, VER(8)),
		MKLINE(ScummEngine, _userState, sleUint16, VER(17)),
		MKLINE(ScummEngine, _cursor.state, sleByte, VER(8)),
		MK_OBSOLETE(ScummEngine, gdi._cursorActive, sleByte, VER(8), VER(20)),
		MKLINE(ScummEngine, _currentCursor, sleByte, VER(8)),
		MKARRAY(ScummEngine, _grabbedCursor[0], sleByte, 8192, VER(20)),
		MKLINE(ScummEngine, _cursor.width, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.height, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.hotspotX, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.hotspotY, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.animate, sleByte, VER(20)),
		MKLINE(ScummEngine, _cursor.animateIndex, sleByte, VER(20)),
		MKLINE(ScummEngine, _mouse.x, sleInt16, VER(20)),
		MKLINE(ScummEngine, _mouse.y, sleInt16, VER(20)),

		MKLINE(ScummEngine, _doEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _switchRoomEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _newEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _switchRoomEffect2, sleByte, VER(8)),
		MKLINE(ScummEngine, _bgNeedsRedraw, sleByte, VER(8)),

		// The state of palManipulate is stored only since V10
		MKLINE(ScummEngine, _palManipStart, sleByte, VER(10)),
		MKLINE(ScummEngine, _palManipEnd, sleByte, VER(10)),
		MKLINE(ScummEngine, _palManipCounter, sleUint16, VER(10)),

		// gfxUsageBits grew from 200 to 410 entries. Then 3 * 410 entries:
		MKARRAY_OLD(ScummEngine, gfxUsageBits[0], sleUint32, 200, VER(8), VER(9)),
		MKARRAY_OLD(ScummEngine, gfxUsageBits[0], sleUint32, 410, VER(10), VER(13)),
		MKARRAY(ScummEngine, gfxUsageBits[0], sleUint32, 3 * 410, VER(14)),

		MK_OBSOLETE(ScummEngine, gdi._transparentColor, sleByte, VER(8), VER(50)),
		MKARRAY(ScummEngine, _currentPalette[0], sleByte, 768, VER(8)),
		MKARRAY(ScummEngine, _darkenPalette[0], sleByte, 768, VER(53)),

		// Sam & Max specific palette replaced by _shadowPalette now.
		MK_OBSOLETE_ARRAY(ScummEngine, _proc_special_palette[0], sleByte, 256, VER(8), VER(33)),

		MKARRAY(ScummEngine, _charsetBuffer[0], sleByte, 256, VER(8)),

		MKLINE(ScummEngine, _egoPositioned, sleByte, VER(8)),

		// gdi._imgBufOffs grew from 4 to 5 entries. Then one day we realized
		// that we don't have to store it since initBGBuffers() recomputes it.
		MK_OBSOLETE_ARRAY(ScummEngine, gdi._imgBufOffs[0], sleUint16, 4, VER(8), VER(9)),
		MK_OBSOLETE_ARRAY(ScummEngine, gdi._imgBufOffs[0], sleUint16, 5, VER(10), VER(26)),

		// See _imgBufOffs: _numZBuffer is recomputed by initBGBuffers().
		MK_OBSOLETE(ScummEngine, gdi._numZBuffer, sleByte, VER(8), VER(26)),

		MKLINE(ScummEngine, _screenEffectFlag, sleByte, VER(8)),

		MK_OBSOLETE(ScummEngine, _randSeed1, sleUint32, VER(8), VER(9)),
		MK_OBSOLETE(ScummEngine, _randSeed2, sleUint32, VER(8), VER(9)),

		// Converted _shakeEnabled to boolean and added a _shakeFrame field.
		MKLINE_OLD(ScummEngine, _shakeEnabled, sleInt16, VER(8), VER(9)),
		MKLINE(ScummEngine, _shakeEnabled, sleByte, VER(10)),
		MKLINE(ScummEngine, _shakeFrame, sleUint32, VER(10)),

		MKLINE(ScummEngine, _keepText, sleByte, VER(8)),

		MKLINE(ScummEngine, _screenB, sleUint16, VER(8)),
		MKLINE(ScummEngine, _screenH, sleUint16, VER(8)),

		MKLINE(ScummEngine, _NESCostumeSet, sleUint16, VER(47)),

		MK_OBSOLETE(ScummEngine, _cd_track, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_loops, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_frame, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_end, sleInt16, VER(9), VER(9)),
		
		MKEND()
	};

	const SaveLoadEntry scriptSlotEntries[] = {
		MKLINE(ScriptSlot, offs, sleUint32, VER(8)),
		MKLINE(ScriptSlot, delay, sleInt32, VER(8)),
		MKLINE(ScriptSlot, number, sleUint16, VER(8)),
		MKLINE(ScriptSlot, delayFrameCount, sleUint16, VER(8)),
		MKLINE(ScriptSlot, status, sleByte, VER(8)),
		MKLINE(ScriptSlot, where, sleByte, VER(8)),
		MKLINE(ScriptSlot, freezeResistant, sleByte, VER(8)),
		MKLINE(ScriptSlot, recursive, sleByte, VER(8)),
		MKLINE(ScriptSlot, freezeCount, sleByte, VER(8)),
		MKLINE(ScriptSlot, didexec, sleByte, VER(8)),
		MKLINE(ScriptSlot, cutsceneOverride, sleByte, VER(8)),
		MKLINE(ScriptSlot, cycle, sleByte, VER(46)),
		MK_OBSOLETE(ScriptSlot, unk5, sleByte, VER(8), VER(10)),
		MKEND()
	};

	const SaveLoadEntry nestedScriptEntries[] = {
		MKLINE(NestedScript, number, sleUint16, VER(8)),
		MKLINE(NestedScript, where, sleByte, VER(8)),
		MKLINE(NestedScript, slot, sleByte, VER(8)),
		MKEND()
	};

	const SaveLoadEntry sentenceTabEntries[] = {
		MKLINE(SentenceTab, verb, sleUint8, VER(8)),
		MKLINE(SentenceTab, preposition, sleUint8, VER(8)),
		MKLINE(SentenceTab, objectA, sleUint16, VER(8)),
		MKLINE(SentenceTab, objectB, sleUint16, VER(8)),
		MKLINE(SentenceTab, freezeCount, sleUint8, VER(8)),
		MKEND()
	};

	const SaveLoadEntry stringTabEntries[] = {
		// Then _default/restore of a StringTab entry becomes a one liner.
		MKLINE(StringTab, xpos, sleInt16, VER(8)),
		MKLINE(StringTab, _default.xpos, sleInt16, VER(8)),
		MKLINE(StringTab, ypos, sleInt16, VER(8)),
		MKLINE(StringTab, _default.ypos, sleInt16, VER(8)),
		MKLINE(StringTab, right, sleInt16, VER(8)),
		MKLINE(StringTab, _default.right, sleInt16, VER(8)),
		MKLINE(StringTab, color, sleInt8, VER(8)),
		MKLINE(StringTab, _default.color, sleInt8, VER(8)),
		MKLINE(StringTab, charset, sleInt8, VER(8)),
		MKLINE(StringTab, _default.charset, sleInt8, VER(8)),
		MKLINE(StringTab, center, sleByte, VER(8)),
		MKLINE(StringTab, _default.center, sleByte, VER(8)),
		MKLINE(StringTab, overhead, sleByte, VER(8)),
		MKLINE(StringTab, _default.overhead, sleByte, VER(8)),
		MKLINE(StringTab, no_talk_anim, sleByte, VER(8)),
		MKLINE(StringTab, _default.no_talk_anim, sleByte, VER(8)),
		MKEND()
	};

	const SaveLoadEntry colorCycleEntries[] = {
		MKLINE(ColorCycle, delay, sleUint16, VER(8)),
		MKLINE(ColorCycle, counter, sleUint16, VER(8)),
		MKLINE(ColorCycle, flags, sleUint16, VER(8)),
		MKLINE(ColorCycle, start, sleByte, VER(8)),
		MKLINE(ColorCycle, end, sleByte, VER(8)),
		MKEND()
	};

	const SaveLoadEntry scaleSlotsEntries[] = {
		MKLINE(ScaleSlot, x1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, y1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, scale1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, x2, sleUint16, VER(13)),
		MKLINE(ScaleSlot, y2, sleUint16, VER(13)),
		MKLINE(ScaleSlot, scale2, sleUint16, VER(13)),
		MKEND()
	};

	// MSVC6 FIX (Jamieson630):
	// MSVC6 has a problem with any notation that involves
	// more than one set of double colons ::
	// The following MKLINE macros expand to such things
	// as AudioCDManager::Status::playing, and MSVC6 has
	// a fit with that. This typedef simplifies the notation
	// to something MSVC6 can grasp.
	typedef AudioCDManager::Status AudioCDManager_Status;
	const SaveLoadEntry audioCDEntries[] = {
		MKLINE(AudioCDManager_Status, playing, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, track, sleInt32, VER(24)),
		MKLINE(AudioCDManager_Status, start, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, duration, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, numLoops, sleInt32, VER(24)),
		MKEND()
	};

	int i, j;
	int var120Backup;
	int var98Backup;
	uint8 md5Backup[16];

	// MD5 Operations: Backup on load, compare, and reset.
	if (s->isLoading())
		memcpy(md5Backup, _gameMD5, 16);


	//
	// Save/load main state (many members of class ScummEngine get saved here)
	//
	s->saveLoadEntries(this, mainEntries);

	// MD5 Operations: Backup on load, compare, and reset.
	if (s->isLoading())
		if (memcmp(md5Backup, _gameMD5, 16) != 0) {
			warning("Game was saved with different gamedata - you may encounter problems.");
 			memcpy(_gameMD5, md5Backup, 16);
		}


	// Starting V14, we extended the usage bits, to be able to cope with games
	// that have more than 30 actors (up to 94 are supported now, in theory).
	// Since the format of the usage bits was changed by this, we have to
	// convert them when loading an older savegame.
	if (s->isLoading() && savegameVersion < VER(14))
		upgradeGfxUsageBits();

	// When loading, move the mouse to the saved mouse position.
	if (s->isLoading() && savegameVersion >= VER(20)) {
		updateCursor();
		_system->warpMouse(_mouse.x, _mouse.y);
	}

	//
	// Save/load actors
	//
	if (s->isLoading()) {
		// Not all actor data is saved; so when loading, we first reset
		// all actors, to ensure completely reproducible behaviour (else,
		// some not saved value in the actor class can cause odd things)
		for (i = 0; i < _numActors; i++)
			_actors[i].initActor(-1);
	}
	s->saveLoadArrayOf(_actors, _numActors, sizeof(_actors[0]), actorEntries);

	
	//
	// Save/load sound data
	//
	s->saveLoadEntries(_sound, soundEntries);


	//
	// Save/load script data
	//
	if (savegameVersion < VER(9))
		s->saveLoadArrayOf(vm.slot, 25, sizeof(vm.slot[0]), scriptSlotEntries);
	else if (savegameVersion < VER(20))
		s->saveLoadArrayOf(vm.slot, 40, sizeof(vm.slot[0]), scriptSlotEntries);
	else
		s->saveLoadArrayOf(vm.slot, NUM_SCRIPT_SLOT, sizeof(vm.slot[0]), scriptSlotEntries);

	if (savegameVersion < VER(46)) {
		// When loading an old savegame, make sure that the 'cycle'
		// field is set to something sensible, otherwise the scripts
		// that were running probably won't be.

		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			vm.slot[i].cycle = 1;
		}
	}


	//
	// Save/load local objects
	//
	s->saveLoadArrayOf(_objs, _numLocalObjects, sizeof(_objs[0]), objectEntries);
	if (s->isLoading() && savegameVersion < VER(13)) {
		// Since roughly v13 of the save games, the objs storage has changed a bit
		for (i = _numObjectsInRoom; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}

	}


	//
	// Save/load misc stuff
	//
	s->saveLoadArrayOf(_verbs, _numVerbs, sizeof(_verbs[0]), verbEntries);
	s->saveLoadArrayOf(vm.nest, 16, sizeof(vm.nest[0]), nestedScriptEntries);
	s->saveLoadArrayOf(_sentence, 6, sizeof(_sentence[0]), sentenceTabEntries);
	s->saveLoadArrayOf(_string, 6, sizeof(_string[0]), stringTabEntries);
	s->saveLoadArrayOf(_colorCycle, 16, sizeof(_colorCycle[0]), colorCycleEntries);
	if (savegameVersion >= VER(13))
		s->saveLoadArrayOf(_scaleSlots, 20, sizeof(_scaleSlots[0]), scaleSlotsEntries);


	//
	// Save/load resources
	//
	int type, idx;
	if (savegameVersion >= VER(26)) {
		// New, more robust resource save/load system. This stores the type
		// and index of each resource. Thus if we increase e.g. the maximum
		// number of script resources, savegames won't break.
		if (s->isSaving()) {
			for (type = rtFirst; type <= rtLast; type++) {
				if (res.mode[type] != 1 && type != rtTemp && type != rtBuffer) {
					s->saveUint16(type);	// Save the res type...
					for (idx = 0; idx < res.num[type]; idx++) {
						// Only save resources which actually exist...
						if (res.address[type][idx]) {
							s->saveUint16(idx);	// Save the index of the resource
							saveResource(s, type, idx);
						}
					}
					s->saveUint16(0xFFFF);	// End marker
				}
			}
			s->saveUint16(0xFFFF);	// End marker
		} else {
			while ((type = s->loadUint16()) != 0xFFFF) {
				while ((idx = s->loadUint16()) != 0xFFFF) {
					assert(0 <= idx && idx < res.num[type]);
					loadResource(s, type, idx);
				}
			}
		}
	} else {
		// Old, fragile resource save/load system. Doesn't save resources
		// with index 0, and breaks whenever we change the limit on a given
		// resource type.
 		for (type = rtFirst; type <= rtLast; type++)
 			if (res.mode[type] != 1 && type != rtTemp && type != rtBuffer) {
 				// For V1-V5 games, there used to be no object name resources.
 				// At some point this changed. But since old savegames rely on
 				// unchanged resource counts, we have to hard code the following check
 				if (_version < 6 && type == rtObjectName)
 					continue;
 				for (idx = 1; idx < res.num[type]; idx++)
 					saveLoadResource(s, type, idx);
 			}
	}


	//
	// Save/load global object state
	//
	s->saveLoadArrayOf(_objectOwnerTable, _numGlobalObjects, sizeof(_objectOwnerTable[0]), sleByte);
	s->saveLoadArrayOf(_objectStateTable, _numGlobalObjects, sizeof(_objectStateTable[0]), sleByte);
	if (_objectRoomTable)
		s->saveLoadArrayOf(_objectRoomTable, _numGlobalObjects, sizeof(_objectRoomTable[0]), sleByte);


	//
	// Save/load palette data
	//
	if (_shadowPaletteSize) {
		s->saveLoadArrayOf(_shadowPalette, _shadowPaletteSize, 1, sleByte);
		// _roomPalette didn't show up until V21 save games
		if (savegameVersion >= VER(21) && _version < 5)
			s->saveLoadArrayOf(_roomPalette, sizeof(_roomPalette), 1, sleByte);
	}

	// PalManip data was not saved before V10 save games
	if (savegameVersion < VER(10))
		_palManipCounter = 0;
	if (_palManipCounter) {
		if (!_palManipPalette)
			_palManipPalette = (byte *)calloc(0x300, 1);
		if (!_palManipIntermediatePal)
			_palManipIntermediatePal = (byte *)calloc(0x600, 1);
		s->saveLoadArrayOf(_palManipPalette, 0x300, 1, sleByte);
		s->saveLoadArrayOf(_palManipIntermediatePal, 0x600, 1, sleByte);
	}
	
	// darkenPalette was not saved before V53
	if (s->isLoading() && savegameVersion < VER(53)) {
		memcpy(_darkenPalette, _currentPalette, 768);
	}


	//
	// Save/load more global object state
	//
	s->saveLoadArrayOf(_classData, _numGlobalObjects, sizeof(_classData[0]), sleUint32);


	//
	// Save/load script variables
	//
	var120Backup = _scummVars[120];
	var98Backup = _scummVars[98];

	if (savegameVersion > VER(37))
		s->saveLoadArrayOf(_roomVars, _numRoomVariables, sizeof(_roomVars[0]), sleInt32);

	// The variables grew from 16 to 32 bit.
	if (savegameVersion < VER(15))
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt16);
	else
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt32);

	if (_gameId == GID_TENTACLE)	// Maybe misplaced, but that's the main idea
		_scummVars[120] = var120Backup;
	if (_gameId == GID_INDY4)
		_scummVars[98] = var98Backup;

	s->saveLoadArrayOf(_bitVars, _numBitVariables >> 3, 1, sleByte);


	//
	// Save/load a list of the locked objects
	//
	if (s->isSaving()) {
		for (i = rtFirst; i <= rtLast; i++)
			for (j = 1; j < res.num[i]; j++) {
				if (res.flags[i][j] & RF_LOCK) {
					s->saveByte(i);
					s->saveUint16(j);
				}
			}
		s->saveByte(0xFF);
	} else {
		while ((i = s->loadByte()) != 0xFF) {
			j = s->loadUint16();
			res.flags[i][j] |= RF_LOCK;
		}
	}


	//
	// Save/load the Audio CD status
	//
	if (savegameVersion >= VER(24)) {
		AudioCDManager::Status info;
		if (s->isSaving())
			info = AudioCD.getStatus();
		s->saveLoadArrayOf(&info, 1, sizeof(info), audioCDEntries);
		// If we are loading, and the music being loaded was supposed to loop
		// forever, then resume playing it. This helps a lot when the audio CD
		// is used to provide ambient music (see bug #788195).
		if (s->isLoading() && info.playing && info.numLoops < 0)
			AudioCD.play(info.track, info.numLoops, info.start, info.duration);
	}


	//
	// Save/load the iMuse status
	//
	if (_imuse && (_saveSound || !_saveTemporaryState)) {
		_imuse->save_or_load(s, this);
	}
}

void ScummEngine_v5::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	ScummEngine::saveOrLoad(s, savegameVersion);

	const SaveLoadEntry cursorEntries[] = {
		MKARRAY2(ScummEngine_v5, _cursorImages[0][0], sleUint16, 16, 4, (byte*)_cursorImages[1] - (byte*)_cursorImages[0], VER(44)),
		MKARRAY(ScummEngine_v5, _cursorHotspots[0], sleByte, 8, VER(44)),
		MKEND()
	};

	// This is probably only needed for Loom.
	s->saveLoadEntries(this, cursorEntries);
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	ScummEngine::saveOrLoad(s, savegameVersion);

	assert(_imuseDigital);
	_imuseDigital->saveOrLoad(s);
}
#endif

#ifndef DISABLE_HE
void ScummEngine_v70he::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	ScummEngine::saveOrLoad(s, savegameVersion);

	const SaveLoadEntry polygonEntries[] = {
		MKLINE(WizPolygon, vert[0].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[0].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[1].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[1].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[2].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[2].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[3].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[3].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[4].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[4].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.left, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.top, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.right, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.bottom, sleInt16, VER(40)),
		MKLINE(WizPolygon, id, sleInt16, VER(40)),
		MKLINE(WizPolygon, numVerts, sleInt16, VER(40)),
		MKLINE(WizPolygon, flag, sleByte, VER(40)),
		MKEND()
	};

	const SaveLoadEntry HE70Entries[] = {
		MKLINE(ScummEngine_v70he, _heSndSoundId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndOffset, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndChannel, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndFlags, sleInt32, VER(51)),
		MKEND()
	};

	if (_heversion >= 71) {
		s->saveLoadArrayOf(_wiz->_polygons, ARRAYSIZE(_wiz->_polygons), sizeof(_wiz->_polygons[0]), polygonEntries);
	}

	s->saveLoadEntries(this, HE70Entries);
}

void ScummEngine_v90he::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	ScummEngine_v70he::saveOrLoad(s, savegameVersion);

	const SaveLoadEntry floodStateEntries[] = {
		MKLINE(FloodStateParameters, box.left, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, box.top, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, box.right, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, box.bottom, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, field_10, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, field_14, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, field_18, sleInt32, VER(51)),
		MKLINE(FloodStateParameters, field_1C, sleInt32, VER(51)),
		MKEND()
	};

	const SaveLoadEntry HE90Entries[] = {
		MKLINE(ScummEngine_v90he, _curMaxSpriteId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _curSpriteId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _curSpriteGroupId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _numSpritesToProcess, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _heObject, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _heObjectNum, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _hePaletteNum, sleInt32, VER(51)),
		MKEND()
	};

	_sprite->saveOrLoadSpriteData(&*s, savegameVersion);

	s->saveLoadArrayOf(&_floodStateParams, 1, sizeof(_floodStateParams), floodStateEntries);

	_numSpritesToProcess = _sprite->_numSpritesToProcess;
	s->saveLoadEntries(this, HE90Entries);
	_sprite->_numSpritesToProcess = _numSpritesToProcess;

	if (_heversion >= 99) {
		s->saveLoadArrayOf(_hePalettes, _numPalettes, sizeof(_hePalettes[0]), sleUint8);
	}
}

void ScummEngine_v100he::saveOrLoad(Serializer *s, uint32 savegameVersion) {
	ScummEngine_v90he::saveOrLoad(s, savegameVersion);

	const SaveLoadEntry HE100Entries[] = {
		MKLINE(ScummEngine_v100he, _heResId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v100he, _heResType, sleInt32, VER(51)),
		MKEND()
	};

	s->saveLoadEntries(this, HE100Entries);
}
#endif

void ScummEngine::saveLoadResource(Serializer *ser, int type, int idx) {
	byte *ptr;
	uint32 size;

	if (!res.mode[type]) {
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
				ser->saveUint16(_inventory[idx]);
			}
			if (type == rtObjectName && ser->getVersion() >= VER(25)) {
				ser->saveUint16(_newNames[idx]);
			}
		} else {
			size = ser->loadUint32();
			if (size) {
				res.createResource(type, idx, size);
				ser->loadBytes(getResourceAddress(type, idx), size);
				if (type == rtInventory) {
					_inventory[idx] = ser->loadUint16();
				}
				if (type == rtObjectName && ser->getVersion() >= VER(25)) {
					// Paranoia: We increased the possible number of new names
					// to fix bugs #933610 and #936323. The savegame format
					// didn't change, but at least during the transition
					// period there is a slight chance that we try to load
					// more names than we have allocated space for. If so,
					// discard them.
					if (idx < _numNewNames)
						_newNames[idx] = ser->loadUint16();
				}
			}
		}
	} else if (res.mode[type] == 2 && ser->getVersion() >= VER(23)) {
		// Save/load only a list of resource numbers that need reloaded.
		if (ser->isSaving()) {
			ser->saveUint16(res.address[type][idx] ? 1 : 0);
		} else {
			if (ser->loadUint16())
				ensureResourceLoaded(type, idx);
		}
	}
}

void ScummEngine::saveResource(Serializer *ser, int type, int idx) {
	assert(res.address[type][idx]);

	if (res.mode[type] == 0) {
		byte *ptr = res.address[type][idx];
		uint32 size = ((MemBlkHeader *)ptr)->size;

		ser->saveUint32(size);
		ser->saveBytes(ptr + sizeof(MemBlkHeader), size);

		if (type == rtInventory) {
			ser->saveUint16(_inventory[idx]);
		}
		if (type == rtObjectName) {
			ser->saveUint16(_newNames[idx]);
		}
	}
}

void ScummEngine::loadResource(Serializer *ser, int type, int idx) {
	if (res.mode[type] == 0) {
		uint32 size = ser->loadUint32();
		assert(size);
		res.createResource(type, idx, size);
		ser->loadBytes(getResourceAddress(type, idx), size);

		if (type == rtInventory) {
			_inventory[idx] = ser->loadUint16();
		}
		if (type == rtObjectName) {
			_newNames[idx] = ser->loadUint16();
		}
	} else if (res.mode[type] == 2) {
		ensureResourceLoaded(type, idx);
	}
}

void Serializer::saveBytes(void *b, int len) {
	_saveStream->write(b, len);
}

void Serializer::loadBytes(void *b, int len) {
	_loadStream->read(b, len);
}

void Serializer::saveUint32(uint32 d) {
	_saveStream->writeUint32LE(d);
}

void Serializer::saveUint16(uint16 d) {
	_saveStream->writeUint16LE(d);
}

void Serializer::saveByte(byte b) {
	_saveStream->writeByte(b);
}

uint32 Serializer::loadUint32() {
	return _loadStream->readUint32LE();
}

uint16 Serializer::loadUint16() {
	return _loadStream->readUint16LE();
}

byte Serializer::loadByte() {
	return _loadStream->readByte();
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
			saveUint16((int16)data);
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
			data = loadUint16();
			break;
		case sleInt16:
			data = (int16)loadUint16();
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
			saveUint16(ptr ? ((*_save_ref) (_ref_me, type, ptr) + 1) : 0);
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
			int num = loadUint16();
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

} // End of namespace Scumm
