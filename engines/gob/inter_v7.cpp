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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/archive.h"

#include "graphics/cursorman.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/expression.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v7
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v7::Inter_v7(GobEngine *vm) : Inter_Playtoons(vm) {
}

void Inter_v7::setupOpcodesDraw() {
	Inter_Playtoons::setupOpcodesDraw();

	OPCODEDRAW(0x0C, o7_draw0x0C);
	OPCODEDRAW(0x0D, o7_loadCursor);
	OPCODEDRAW(0x44, o7_displayWarning);
	OPCODEDRAW(0x45, o7_logString);
	OPCODEDRAW(0x57, o7_intToString);
	OPCODEDRAW(0x59, o7_callFunction);
	OPCODEDRAW(0x5A, o7_loadFunctions);
	OPCODEDRAW(0x83, o7_playVmdOrMusic);
	OPCODEDRAW(0x89, o7_draw0x89);
	OPCODEDRAW(0x8A, o7_findFile);
	OPCODEDRAW(0x8C, o7_getSystemProperty);
	OPCODEDRAW(0x90, o7_loadImage);
	OPCODEDRAW(0x93, o7_setVolume);
	OPCODEDRAW(0x95, o7_zeroVar);
	OPCODEDRAW(0xA1, o7_getINIValue);
	OPCODEDRAW(0xA2, o7_setINIValue);
	OPCODEDRAW(0xA4, o7_draw0xA4);
	OPCODEDRAW(0xC4, o7_opendBase);
	OPCODEDRAW(0xC5, o7_closedBase);
	OPCODEDRAW(0xC6, o7_getDBString);
}

void Inter_v7::setupOpcodesFunc() {
	Inter_Playtoons::setupOpcodesFunc();
}

void Inter_v7::setupOpcodesGob() {
	Inter_Playtoons::setupOpcodesGob();

	OPCODEGOB(420, o7_oemToANSI);
}

void Inter_v7::o7_draw0x0C() {
	WRITE_VAR(17, 0);
}

void Inter_v7::o7_loadCursor() {
	int16          cursorIndex = _vm->_game->_script->readValExpr();
	Common::String cursorFile  = _vm->_game->_script->evalString();

	warning("Addy Stub: Load cursor \"%s\" to %d", cursorFile.c_str(), cursorIndex);

	byte cursor[9];
	byte palette[6];

	cursor[0] = 0; cursor[1] = 0; cursor[2] = 0;
	cursor[3] = 0; cursor[4] = 1; cursor[5] = 0;
	cursor[6] = 0; cursor[7] = 0; cursor[8] = 0;

	palette[0] =   0; palette[1] =   0; palette[2] =   0;
	palette[3] = 255; palette[4] = 255; palette[5] = 255;

	CursorMan.pushCursorPalette(palette, 0, 2);
	CursorMan.disableCursorPalette(false);
	CursorMan.replaceCursor(cursor, 3, 3, 1, 1, 255);

	CursorMan.showMouse(true);
}

void Inter_v7::o7_displayWarning() {
	Common::String caption = _vm->_game->_script->evalString();
	Common::String text    = _vm->_game->_script->evalString();
	Common::String source  = _vm->_game->_script->evalString();
	Common::String msg     = _vm->_game->_script->evalString();
	Common::String param   = _vm->_game->_script->evalString();

	warning("%s: %s (%s)", source.c_str(), msg.c_str(), param.c_str());
}

void Inter_v7::o7_logString() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	TimeDate t;
	_vm->_system->getTimeAndDate(t);

	debug(1, "%-9s%04d-%02d-%02dT%02d:%02d:%02d --> %s", str0.c_str(),
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec, str1.c_str());
}

void Inter_v7::o7_intToString() {
	uint16 valueIndex = _vm->_game->_script->readVarIndex();
	uint16 destIndex  = _vm->_game->_script->readVarIndex();

	sprintf(GET_VARO_STR(destIndex), "%d", READ_VARO_UINT32(valueIndex));
}

void Inter_v7::o7_callFunction() {
	Common::String tot      = _vm->_game->_script->evalString();
	Common::String function = _vm->_game->_script->evalString();

	int16 param = _vm->_game->_script->readValExpr();

	if (!tot.contains('.'))
		tot += ".TOT";

	_vm->_game->callFunction(tot, function, param);
}

void Inter_v7::o7_loadFunctions() {
	Common::String tot = _vm->_game->_script->evalString();

	int16 flags = _vm->_game->_script->readValExpr();

	if (!tot.contains('.'))
		tot += ".TOT";

	_vm->_game->loadFunctions(tot, flags);
}

void Inter_v7::o7_playVmdOrMusic() {
	Common::String file = _vm->_game->_script->evalString();

	VideoPlayer::Properties props;

	props.x          = _vm->_game->_script->readValExpr();
	props.y          = _vm->_game->_script->readValExpr();
	props.startFrame = _vm->_game->_script->readValExpr();
	props.lastFrame  = _vm->_game->_script->readValExpr();
	props.breakKey   = _vm->_game->_script->readValExpr();
	props.flags      = _vm->_game->_script->readValExpr();
	props.palStart   = _vm->_game->_script->readValExpr();
	props.palEnd     = _vm->_game->_script->readValExpr();
	props.palCmd     = 1 << (props.flags & 0x3F);
	props.forceSeek  = true;

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", file.c_str(),
			props.x, props.y, props.startFrame, props.lastFrame,
			props.palCmd, props.palStart, props.palEnd, props.flags);

	if (file == "RIEN") {
		_vm->_vidPlayer->closeAll();
		return;
	}

	bool close = false;
	if (props.lastFrame == -1) {
		close = true;
	} else if (props.lastFrame == -3) {

		if (file.empty()) {
			_vm->_vidPlayer->closeVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1);
			_vm->_mult->_objects[props.startFrame].videoSlot = 0;
			return;
		}

		props.flags  = VideoPlayer::kFlagOtherSurface;
		props.sprite = -1;

		_vm->_mult->_objects[props.startFrame].pAnimData->animation = -props.startFrame - 1;

		if (_vm->_mult->_objects[props.startFrame].videoSlot > 0)
			_vm->_vidPlayer->closeVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1);

		uint32 x = props.x;
		uint32 y = props.y;

		int slot = _vm->_vidPlayer->openVideo(false, file, props);

		_vm->_mult->_objects[props.startFrame].videoSlot = slot + 1;

		if (x == 0xFFFFFFFF) {
			*_vm->_mult->_objects[props.startFrame].pPosX = _vm->_vidPlayer->getDefaultX(slot);
			*_vm->_mult->_objects[props.startFrame].pPosY = _vm->_vidPlayer->getDefaultY(slot);
		} else {
			*_vm->_mult->_objects[props.startFrame].pPosX = x;
			*_vm->_mult->_objects[props.startFrame].pPosY = y;
		}

		return;
	} else if (props.lastFrame == -4) {
		warning("Woodruff Stub: Video/Music command -4: Play background video %s", file.c_str());
		return;
	} else if (props.lastFrame == -5) {
//		warning("Urban/Playtoons Stub: Stop without delay");
		_vm->_sound->bgStop();
		return;
	} else if (props.lastFrame == -6) {
//		warning("Urban/Playtoons Stub: Video/Music command -6 (cache video)");
		return;
	} else if (props.lastFrame == -7) {
//		warning("Urban/Playtoons Stub: Video/Music command -6 (flush cache)");
		return;
	} else if ((props.lastFrame == -8) || (props.lastFrame == -9)) {
		if (!file.contains('.'))
			file += ".WA8";

		probe16bitMusic(file);

		if (props.lastFrame == -9)
			debugC(0, kDebugVideo, "Urban/Playtoons Stub: Delayed music stop?");

		_vm->_sound->bgStop();
		_vm->_sound->bgPlay(file.c_str(), SOUND_WAV);
		return;
	} else if (props.lastFrame <= -10) {
		_vm->_vidPlayer->closeVideo();

		if (!(props.flags & VideoPlayer::kFlagNoVideo))
			props.loop = true;

	} else if (props.lastFrame < 0) {
		warning("Urban/Playtoons Stub: Unknown Video/Music command: %d, %s", props.lastFrame, file.c_str());
		return;
	}

	if (props.startFrame == -2) {
		props.startFrame = 0;
		props.lastFrame  = -1;
		props.noBlock    = true;
	}

	_vm->_vidPlayer->evaluateFlags(props);

	bool primary = true;
	if (props.noBlock && (props.flags & VideoPlayer::kFlagNoVideo))
		primary = false;

	int slot = 0;
	if (!file.empty() && ((slot = _vm->_vidPlayer->openVideo(primary, file, props)) < 0)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	if (props.hasSound)
		_vm->_vidPlayer->closeLiveSound();

	if (props.startFrame >= 0)
		_vm->_vidPlayer->play(slot, props);

	if (close && !props.noBlock) {
		if (!props.canceled)
			_vm->_vidPlayer->waitSoundEnd(slot);
		_vm->_vidPlayer->closeVideo(slot);
	}

}
void Inter_v7::o7_draw0x89() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	warning("Addy Stub Draw 0x89: \"%s\", \"%s\"", str0.c_str(), str1.c_str());

	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, str0);

	if (files.empty()) {
		storeValue(0);
		return;
	}

	storeValue(1);
}

void Inter_v7::o7_findFile() {
	Common::String file = getFile(_vm->_game->_script->evalString());

	uint16 pathIndex = _vm->_game->_script->readVarIndex();

	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, file);

	if (files.empty()) {
		GET_VARO_STR(pathIndex)[0] = '\0';
		storeValue(0);
		return;
	}

	strcpy(GET_VARO_STR(pathIndex), files.front()->getName().c_str());
	storeValue(1);
}

void Inter_v7::o7_getSystemProperty() {
	const char *property = _vm->_game->_script->evalString();
	if (!scumm_stricmp(property, "TotalPhys")) {
		// HACK
		storeValue(1000000);
		return;
	}

	if (!scumm_stricmp(property, "AvailPhys")) {
		// HACK
		storeValue(1000000);
		return;
	}

	if (!scumm_stricmp(property, "TimeGMT")) {
		renewTimeInVars();
		storeValue(0);
		return;
	}

	warning("Inter_v7::o7_getSystemProperty(): Unknown property \"%s\"", property);
	storeValue(0);
}

void Inter_v7::o7_loadImage() {
	Common::String file = _vm->_game->_script->evalString();
	if (!file.contains('.'))
		file += ".TGA";

	int16 spriteIndex = _vm->_game->_script->readValExpr();
	int16 left        = _vm->_game->_script->readValExpr();
	int16 top         = _vm->_game->_script->readValExpr();
	int16 width       = _vm->_game->_script->readValExpr();
	int16 height      = _vm->_game->_script->readValExpr();
	int16 x           = _vm->_game->_script->readValExpr();
	int16 y           = _vm->_game->_script->readValExpr();
	int16 transp      = _vm->_game->_script->readValExpr();

	if (spriteIndex > 100)
		spriteIndex -= 80;

	if ((spriteIndex < 0) || (spriteIndex >= Draw::kSpriteCount)) {
		warning("o7_loadImage(): Sprite %d out of range", spriteIndex);
		return;
	}

	SurfacePtr destSprite = _vm->_draw->_spritesArray[spriteIndex];
	if (!destSprite) {
		warning("o7_loadImage(): Sprite %d does not exist", spriteIndex);
		return;
	}

	Common::SeekableReadStream *imageFile = _vm->_dataIO->getFile(file);
	if (!imageFile) {
		warning("o7_loadImage(): No such file \"%s\"", file.c_str());
		return;
	}

	SurfacePtr image = _vm->_video->initSurfDesc(1, 1);
	if (!image->loadImage(*imageFile)) {
		warning("o7_loadImage(): Failed to load image \"%s\"", file.c_str());
		return;
	}

	int16 right  = left + width  - 1;
	int16 bottom = top  + height - 1;
	destSprite->blit(*image, left, top, right, bottom, x, y, transp);
}

void Inter_v7::o7_setVolume() {
	uint32 volume = _vm->_game->_script->readValExpr();

	warning("Addy Stub: Set volume %d (0 - 100)", volume);
}

void Inter_v7::o7_zeroVar() {
	uint16 index = _vm->_game->_script->readVarIndex();

	WRITE_VARO_UINT32(index, 0);
}

void Inter_v7::o7_getINIValue() {
	Common::String file = getFile(_vm->_game->_script->evalString());

	Common::String section = _vm->_game->_script->evalString();
	Common::String key     = _vm->_game->_script->evalString();
	Common::String def     = _vm->_game->_script->evalString();

	Common::String value;
	_inis.getValue(value, file, section, key, def);

	storeString(value.c_str());
}

void Inter_v7::o7_setINIValue() {
	Common::String file = getFile(_vm->_game->_script->evalString());

	Common::String section = _vm->_game->_script->evalString();
	Common::String key     = _vm->_game->_script->evalString();
	Common::String value   = _vm->_game->_script->evalString();

	_inis.setValue(file, section, key, value);
}

void Inter_v7::o7_draw0xA4() {
	Common::String file = _vm->_game->_script->evalString();
	if (!file.contains('.'))
		file += ".LBM";

	int16 expr0 = _vm->_game->_script->readValExpr();
	int16 expr1 = _vm->_game->_script->readValExpr();

	warning("Addy Stub Draw 0xA4: \"%s\", %d, %d", file.c_str(), expr0, expr1);
}

void Inter_v7::o7_opendBase() {
	Common::String dbFile = _vm->_game->_script->evalString();
	Common::String id     = _vm->_game->_script->evalString();

	dbFile += ".DBF";

	_databases.setLanguage(_vm->_language);
	if (!_databases.open(id, dbFile)) {
		WRITE_VAR(27, 0); // Failure
		return;
	}

	WRITE_VAR(27, 1); // Success
}

void Inter_v7::o7_closedBase() {
	Common::String id = _vm->_game->_script->evalString();

	if (_databases.close(id))
		WRITE_VAR(27, 1); // Success
	else
		WRITE_VAR(27, 0); // Failure
}

void Inter_v7::o7_getDBString() {
	Common::String id      = _vm->_game->_script->evalString();
	Common::String group   = _vm->_game->_script->evalString();
	Common::String section = _vm->_game->_script->evalString();
	Common::String keyword = _vm->_game->_script->evalString();

	Common::String result;
	if (!_databases.getString(id, group, section, keyword, result)) {
		WRITE_VAR(27, 0); // Failure
		storeString("");
		return;
	}

	storeString(result.c_str());
	WRITE_VAR(27, 1); // Success
}

void Inter_v7::o7_oemToANSI(OpGobParams &params) {
	_vm->_game->_script->skip(2);
}

void Inter_v7::storeValue(uint16 index, uint16 type, uint32 value) {
	switch (type) {
	case OP_ARRAY_INT8:
	case TYPE_VAR_INT8:
		WRITE_VARO_UINT8(index, value);
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(index, value);
		break;

	default:
		WRITE_VARO_UINT32(index, value);
	}
}

void Inter_v7::storeValue(uint32 value) {
	uint16 type;
	uint16 index = _vm->_game->_script->readVarIndex(0, &type);

	storeValue(index, type, value);
}

void Inter_v7::storeString(uint16 index, uint16 type, const char *value) {
	uint32 maxLength = _vm->_global->_inter_animDataSize * 4 - 1;
	char  *str       = GET_VARO_STR(index);

	switch (type) {
	case TYPE_VAR_STR:
		if (strlen(value) > maxLength)
			warning("Inter_v7::storeString(): String too long");

		Common::strlcpy(str, value, maxLength);
		break;

	case TYPE_IMM_INT8:
	case TYPE_VAR_INT8:
		strcpy(str, value);
		break;

	case TYPE_ARRAY_INT8:
		WRITE_VARO_UINT8(index, atoi(value));
		break;

	case TYPE_VAR_INT16:
	case TYPE_VAR_INT32_AS_INT16:
	case TYPE_ARRAY_INT16:
		WRITE_VARO_UINT16(index, atoi(value));
		break;

	case TYPE_VAR_INT32:
	case TYPE_ARRAY_INT32:
		WRITE_VARO_UINT32(index, atoi(value));
		break;

	default:
		warning("Inter_v7::storeString(): Requested to store a string into type %d", type);
		break;
	}
}

void Inter_v7::storeString(const char *value) {
	uint16 type;
	int16 varIndex = _vm->_game->_script->readVarIndex(0, &type);

	storeString(varIndex, type, value);
}

} // End of namespace Gob
