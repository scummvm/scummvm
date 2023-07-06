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
 *
 */

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/archive.h"
#include "common/translation.h"
#include "common/formats/winexe_pe.h"
#include "common/formats/winexe_ne.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "gui/message.h"

#include "image/iff.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/expression.h"
#include "gob/videoplayer.h"
#include "gob/resources.h"
#include "gob/sound/sound.h"
#include "gob/save/saveload.h"

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
	OPCODEDRAW(0x0D, o7_setCursorToLoadFromExec);
	OPCODEDRAW(0x15, o7_freeMult);
	OPCODEDRAW(0x17, o7_loadMultObject);
	OPCODEDRAW(0x44, o7_displayWarning);
	OPCODEDRAW(0x45, o7_logString);
	OPCODEDRAW(0x52, o7_moveGoblin);
	OPCODEDRAW(0x55, o7_setGoblinState);
	OPCODEDRAW(0x57, o7_intToString);
	OPCODEDRAW(0x59, o7_callFunction);
	OPCODEDRAW(0x5A, o7_loadFunctions);
	OPCODEDRAW(0x60, o7_copyFile);
	OPCODEDRAW(0x61, o7_deleteFile);
	OPCODEDRAW(0x62, o7_moveFile);
	OPCODEDRAW(0x80, o7_initScreen);
	OPCODEDRAW(0x83, o7_playVmdOrMusic);
	OPCODEDRAW(0x89, o7_setActiveCD);
	OPCODEDRAW(0x8A, o7_findFile);
	OPCODEDRAW(0x8B, o7_findNextFile);
	OPCODEDRAW(0x8C, o7_getSystemProperty);
	OPCODEDRAW(0x90, o7_loadImage);
	OPCODEDRAW(0x93, o7_setVolume);
	OPCODEDRAW(0x95, o7_zeroVar);
	OPCODEDRAW(0xA1, o7_getINIValue);
	OPCODEDRAW(0xA2, o7_setINIValue);
	OPCODEDRAW(0xA4, o7_loadIFFPalette);
	OPCODEDRAW(0xC4, o7_opendBase);
	OPCODEDRAW(0xC5, o7_closedBase);
	OPCODEDRAW(0xC6, o7_getDBString);
}

void Inter_v7::setupOpcodesFunc() {
	Inter_Playtoons::setupOpcodesFunc();
	OPCODEFUNC(0x03, o7_loadCursor);
	OPCODEFUNC(0x11, o7_printText);
	OPCODEFUNC(0x33, o7_fillRect);
	OPCODEFUNC(0x34, o7_drawLine);
	OPCODEFUNC(0x36, o7_invalidate);
	OPCODEFUNC(0x3F, o7_checkData);
	OPCODEFUNC(0x4D, o7_readData);
	OPCODEFUNC(0x4E, o7_writeData);
}

void Inter_v7::setupOpcodesGob() {
	Inter_Playtoons::setupOpcodesGob();

	OPCODEGOB(420, o7_oemToANSI);
	OPCODEGOB(513, o7_gob0x201);
}

void Inter_v7::o7_draw0x0C() {
	WRITE_VAR(17, 0);
}

void Inter_v7::o7_loadCursor(OpFuncParams &params) {
	int16 id = _vm->_game->_script->readInt16();

	if ((id == -1) || (id == -2)) {
		char file[10];

		if (id == -1) {
			for (int i = 0; i < 9; i++)
				file[i] = _vm->_game->_script->readChar();
		} else
			strncpy(file, GET_VAR_STR(_vm->_game->_script->readInt16()), 10);

		file[9] = '\0';

		uint16 start = _vm->_game->_script->readUint16();
		int8 index = _vm->_game->_script->readInt8();

		VideoPlayer::Properties props;

		props.sprite = -1;

		int vmdSlot = _vm->_vidPlayer->openVideo(false, file, props);
		if (vmdSlot == -1) {
			warning("Can't open video \"%s\" as cursor", file);
			return;
		}

		int16 framesCount = _vm->_vidPlayer->getFrameCount(vmdSlot);

		for (int i = 0; i < framesCount; i++) {
			props.startFrame   = i;
			props.lastFrame    = i;
			props.waitEndFrame = false;

			_vm->_vidPlayer->play(vmdSlot, props);
			_vm->_vidPlayer->copyFrame(vmdSlot, *_vm->_draw->_cursorSprites,
									   0, 0, _vm->_draw->_cursorWidth, _vm->_draw->_cursorWidth,
									   (start + i) * _vm->_draw->_cursorWidth, 0);
		}

		_vm->_vidPlayer->closeVideo(vmdSlot);

		_vm->_draw->_cursorAnimLow[index] = start;
		_vm->_draw->_cursorAnimHigh[index] = framesCount + start - 1;
		_vm->_draw->_cursorAnimDelays[index] = 10;

		return;
	}

	int8 index = _vm->_game->_script->readInt8();

	Resource *resource = _vm->_game->_resources->getResource((uint16) id);
	if (!resource)
		return;

	int16 cursorWidth = MAX(_vm->_draw->_cursorWidth, resource->getWidth());
	int16 cursorHeight = MAX(_vm->_draw->_cursorHeight, resource->getHeight());
	_vm->_draw->resizeCursors(cursorWidth, cursorHeight, index + 1, true);
	if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
		return;

	_vm->_draw->_cursorSprites->fillRect(index * _vm->_draw->_cursorWidth, 0,
										 index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
										 _vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite(resource->getData(),
								  resource->getWidth(), resource->getHeight(),
								  index * _vm->_draw->_cursorWidth, 0, 0, *_vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	delete resource;
}

void Inter_v7::o7_setCursorToLoadFromExec() {
	int16          cursorIndex = _vm->_game->_script->readValExpr();
	Common::String cursorName  = _vm->_game->_script->evalString();
	debugC(1, kDebugGraphics, "o7_setCursorToLoadFromExec : index=%d, cursorName=%s", cursorIndex, cursorName.c_str());

	_vm->_draw->_cursorNames[cursorIndex] = cursorName;
}

void Inter_v7::o7_freeMult() {
	_vm->_mult->freeMult(true);
}

void Inter_v7::o7_loadMultObject() {
	assert(_vm->_mult->_objects);

	uint16 objIndex = _vm->_game->_script->readValExpr();

	Mult::Mult_Object &obj = _vm->_mult->_objects[objIndex];
	Mult::Mult_AnimData &objAnim = *(obj.pAnimData);

	auto x = _vm->_game->_script->readValExpr();
	auto y = _vm->_game->_script->readValExpr();
	debugC(4, kDebugGameFlow, "Loading mult object %d -> x = %d, y = %d", objIndex, x ,y);

	*obj.pPosX = x;
	*obj.pPosY = y;

	byte *multData = (byte *) &objAnim;
	for (int i = 0; i < 11; i++) {
		if (_vm->_game->_script->peekByte() != 99)
			multData[i] = _vm->_game->_script->readValExpr();
		else
			_vm->_game->_script->skip(1);
	}

	if (((int32)*obj.pPosX == -1234) && ((int32)*obj.pPosY == -4321)) {
		if (obj.videoSlot > 0) {
			_vm->_mult->closeObjVideo(obj);
		} else
			_vm->_draw->freeSprite(50 + objIndex);

		objAnim.isStatic = 1;

		obj.animVariables = nullptr;
		obj.lastLeft   = -1;
		obj.lastTop    = -1;
		obj.lastBottom = -1;
		obj.lastRight  = -1;
	}
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

void Inter_v7::o7_moveGoblin() {
	int16 destX, destY;
	int16 index;

	destX = _vm->_game->_script->readValExpr();
	destY = _vm->_game->_script->readValExpr();
	index = _vm->_game->_script->readValExpr();

	Mult::Mult_Object &obj = _vm->_mult->_objects[index];
	Mult::Mult_AnimData &animData = *(obj.pAnimData);

	if (animData.animType < 10 || animData.animType > 12)
		return;

	animData.pathExistence = 0;
	animData.animTypeBak = 0;
	animData.framesLeft = 0;
	animData.isBusy = 0;
	obj.gobDestX = destX;
	animData.gobDestX_maybe = destX;
	obj.gobDestY = destY;
	animData.gobDestY_maybe = destY;
	animData.newState = 0;
	obj.goblinX = animData.destX;
	obj.goblinY = animData.destY;

	if (animData.animType == 10 && animData.curLookDir <= 10)
		return;

	animData.destXBak = animData.destX;
	animData.destYBak = animData.destY;
	animData.animType = 10;

	debugC(1, kDebugVideo, "o7_moveGoblin Obj %s destX = %d, destY = %d (gobX = %d, gobY = %d) ", obj.animName, destX,  destY, obj.goblinX, obj.goblinY);

	_vm->_goblin->initiateMove(&obj);
}

void Inter_v7::o7_setGoblinState() {
	int16 index = _vm->_game->_script->readValExpr();
	int16 state = _vm->_game->_script->readValExpr();
	int16 type = _vm->_game->_script->readValExpr();

	Mult::Mult_Object &obj = _vm->_mult->_objects[index];
	Mult::Mult_AnimData &animData = *(obj.pAnimData);

	if (animData.animType < 10 || animData.animType > 12)
		return;

	animData.pathExistence = 1;
	animData.animType = 10;
	obj.destX = obj.goblinX;
	obj.destY = obj.goblinY;

	debugC(1, kDebugVideo, "o7_setGoblinState Obj %s state = %d, type = %d = %d ", obj.animName, state, type, state + type*100);

	_vm->_goblin->setGoblinState(&obj, state + type * 100);
	if (type != 0) {
		animData.pathExistence = 3;
		animData.animType = 12;
		animData.frame = 0;
	}
}


void Inter_v7::o7_intToString() {
	uint16 valueIndex = _vm->_game->_script->readVarIndex();
	uint16 destIndex  = _vm->_game->_script->readVarIndex();
	uint32 maxLength = _vm->_global->_inter_animDataSize * 4 - 1;

	Common::sprintf_s(GET_VARO_STR(destIndex), maxLength, "%d", (int32)READ_VARO_UINT32(valueIndex));
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

void Inter_v7::copyFile(const Common::String &sourceFile, const Common::String &destFile) {
	SaveLoad::SaveMode mode1 = _vm->_saveLoad->getSaveMode(sourceFile.c_str());
	SaveLoad::SaveMode mode2 = _vm->_saveLoad->getSaveMode(destFile.c_str());

	if (mode2 == SaveLoad::kSaveModeIgnore || mode2 == SaveLoad::kSaveModeExists)
		return;
	else if (mode2 == SaveLoad::kSaveModeSave) {
		if (mode1 == SaveLoad::kSaveModeNone) {
			Common::SeekableReadStream *stream = _vm->_dataIO->getFile(sourceFile);
			if (!stream)
				return;

			int32 size = stream->size();
			// TODO: avoid this allocation by implementing a Stream version of saveFromRaw (e.g. "saveFromStream")
			byte *data = new byte[size];
			stream->read(data, size);

			_vm->_saveLoad->saveFromRaw(destFile.c_str(), data, size, 0);
			delete[] data;
			delete stream;
		} else if (mode1 == SaveLoad::kSaveModeSave) {
			_vm->_saveLoad->copySaveGame(sourceFile.c_str(), destFile.c_str());
		} else
			warning("o7_copyFile(): unsupported mode %d for source \"%s\" while copying to \"%s\" ",
					mode1,
					sourceFile.c_str(),
					destFile.c_str());
	} else
		warning("Attempted to write to file \"%s\" while copying from \"%s\"",
				destFile.c_str(),
				sourceFile.c_str());

}

void Inter_v7::o7_copyFile() {
	Common::String path1 = _vm->_game->_script->evalString();
	Common::String path2 = _vm->_game->_script->evalString();

	debugC(2, kDebugFileIO, "Copy file \"%s\" to \"%s", path1.c_str(), path2.c_str());

	Common::String file1 = getFile(path1.c_str(), false);
	Common::String file2 = getFile(path2.c_str(), false);
	if (file1.equalsIgnoreCase(file2)) {
		warning("o7_copyFile(): \"%s\" == \"%s\"", path1.c_str(), path2.c_str());
		return;
	}

	copyFile(file1, file2);
}

void Inter_v7::o7_deleteFile() {
	Common::String file =_vm->_game->_script->evalString();

	debugC(2, kDebugFileIO, "Delete file \"%s\"", file.c_str());

	bool isPattern = file.contains('*') || file.contains('?');
	Common::List<Common::String> files;
	if (isPattern) {
		files = _vm->_saveLoad->getFilesMatchingPattern(file.c_str());
		debugC(2, kDebugFileIO, "Delete file matching pattern \"%s\" (%d matching file(s))", file.c_str(), files.size());
		for (const Common::String &matchingFile : files)
			debugC(5, kDebugFileIO, "Matching file: %s", matchingFile.c_str());
	} else {
		files.push_back(file);
		debugC(2, kDebugFileIO, "Delete file \"%s\"", file.c_str());
	}

	if (_vm->getGameType() == kGameTypeAdibou2
		&& isPattern
		&& file.hasPrefix("DATA\\??????")) {
		// WORKAROUND a bug in original game: files APPLI_<N>.INF and CRITE_<N>.INF should not be deleted when removing character <N>
		// Those files contain *application <N>* data, not "character <N>" data
		for (Common::List<Common::String>::iterator it = files.begin(); it != files.end(); ++it) {
			if (it->matchString("DATA\\\\appli_??.inf", true) || it->matchString("DATA\\\\crite_??.inf", true)) {
				debugC(2, kDebugFileIO, "o7_deleteFile: ignoring deletion of file \"%s\" when processing pattern %s (delete character bug workaround)",
					   it->c_str(),
					   file.c_str());
				it = files.reverse_erase(it);
			}
		}
	}

	for (const Common::String &fileToDelete : files) {
		SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(fileToDelete.c_str());
		if (mode == SaveLoad::kSaveModeSave) {
			if (!_vm->_saveLoad->deleteFile(fileToDelete.c_str())) {
				warning("Cannot delete file \"%s\"", fileToDelete.c_str());
			}
		} else if (mode == SaveLoad::kSaveModeNone)
			warning("Attempted to delete file \"%s\"", fileToDelete.c_str());
	}
}

void Inter_v7::o7_moveFile() {
	Common::String path1 = _vm->_game->_script->evalString();
	Common::String path2 = _vm->_game->_script->evalString();

	Common::String file1 = getFile(path1.c_str(), false);
	Common::String file2 = getFile(path2.c_str(), false);

	if (file1.equalsIgnoreCase(file2)) {
		warning("o7_moveFile(): \"%s\" == \"%s\"", path1.c_str(), path2.c_str());
		return;
	}

	copyFile(file1, file2);
	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file1.c_str());
	if (mode == SaveLoad::kSaveModeSave) {
		_vm->_saveLoad->deleteFile(file1.c_str());
	} else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to delete file \"%s\" while moving it to \"%s\"", file1.c_str(), file2.c_str());
}


void Inter_v7::o7_initScreen() {
	// TODO: continue implementation
	int16 offY;
	int16 videoMode;
	int16 width, height;

	offY = _vm->_game->_script->readInt16();

	videoMode = offY & 0xFF;
	offY = (offY >> 8) & 0xFF;

	width = _vm->_game->_script->readValExpr();
	height = _vm->_game->_script->readValExpr();

	if (videoMode == 0)
		videoMode = 0x14;

	if (videoMode != 0x18)
		_vm->_video->clearScreen();

	if (videoMode == 0x13) {

		if (width == -1)
			width = 320;
		if (height == -1)
			height = 200;

		_vm->_width = 320;
		_vm->_height = 200;

		_vm->_video->setSize();

	} else if (_vm->_global->_videoMode == 0x13) {
		width = _vm->_width = 640;
		height = _vm->_height = 480;

		_vm->_video->setSize();
	}

	_vm->_global->_fakeVideoMode = videoMode;

	// Some versions require this
	if (videoMode == 0x18)
		_vm->_global->_fakeVideoMode = 0x37;

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (width > 0)
		_vm->_video->_surfWidth = width;
	if (height > 0)
		_vm->_video->_surfHeight = height;

	_vm->_video->_screenDeltaX = 0;
	if (_vm->_video->_surfWidth < _vm->_width)
		_vm->_video->_screenDeltaX = (_vm->_width - _vm->_video->_surfWidth) / 2;

	_vm->_global->_mouseMinX = _vm->_video->_screenDeltaX;
	_vm->_global->_mouseMaxX = _vm->_video->_screenDeltaX + _vm->_video->_surfWidth - 1;

	_vm->_video->_splitStart = _vm->_video->_surfHeight - offY;

	_vm->_video->_splitHeight1 = MIN<int16>(_vm->_height, _vm->_video->_surfHeight);
	_vm->_video->_splitHeight2 = offY;

	if ((_vm->_video->_surfHeight + offY) < _vm->_height)
		_vm->_video->_screenDeltaY = (_vm->_height - (_vm->_video->_surfHeight + offY)) / 2;
	else
		_vm->_video->_screenDeltaY = 0;

	_vm->_global->_mouseMaxY = (_vm->_video->_surfHeight + _vm->_video->_screenDeltaY) - offY - 1;
	_vm->_global->_mouseMinY = _vm->_video->_screenDeltaY;

	if (videoMode != 0x18)
	{
		_vm->_draw->closeScreen();
		_vm->_util->clearPalette();
		memset(_vm->_global->_redPalette, 0, 256);
		memset(_vm->_global->_greenPalette, 0, 256);
		memset(_vm->_global->_bluePalette, 0, 256);
		_vm->_video->_splitSurf.reset();
		_vm->_draw->_spritesArray[24].reset();
		_vm->_draw->_spritesArray[25].reset();
	}

	_vm->_global->_videoMode = videoMode;
	if (videoMode != 0x18)
		_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
							_vm->_global->_inter_mouseY);

	if (videoMode != 0x18)
		_vm->_util->clearPalette();

	_vm->_draw->initScreen();

	_vm->_util->setScrollOffset();
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
		_vm->_vidPlayer->closeLiveSound();
		return;
	}

	bool close = false;
	if (props.lastFrame == -1) {
		close = true;
	} else if (props.lastFrame == -2) {
		// TODO: when props.lastFrame == -2, the VMD seems to be loaded/played in two steps.
		// First o7_playVmdOrMusic is called with props.firstFrame == -2, then later with
		// props.firstFrame == -1.
		// We simply ignore the first step for now, and play the whole video in the second step.
		if (props.startFrame == -2)
			return;

		props.startFrame = 0;
		props.lastFrame = -1;
	} else if (props.lastFrame == -3) {

		if (file.empty()) {
			if (props.flags & 0x400) {
				if (!_vm->_mult->_objects[props.startFrame].pAnimData->isStatic) {
					if (_vm->_mult->_objects[props.startFrame].videoSlot > 0) {
						// TODO: something more is needed here, but unclear what
						//_vm->_vidPlayer->pauseVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1, true);
					}
				}
			}
			return;
		}

		props.flags  = VideoPlayer::kFlagOtherSurface;

		_vm->_mult->_objects[props.startFrame].pAnimData->animation = -props.startFrame - 1;
		props.sprite = 50 - _vm->_mult->_objects[props.startFrame].pAnimData->animation - 1;

		if (_vm->_mult->_objects[props.startFrame].videoSlot > 0)
			_vm->_mult->closeObjVideo(_vm->_mult->_objects[props.startFrame]);

		uint32 x = props.x;
		uint32 y = props.y;

		int slot = _vm->_mult->openObjVideo(file, props, -props.startFrame - 1);

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

	// TODO: conditions below for unblocking videos have been found partly from asm, partly by trial and errors.
	// Reality may be more complex...
	if (props.startFrame == -2 ||
		(props.startFrame == props.lastFrame &&
		 props.lastFrame != -1 &&
		 !(props.flags & VideoPlayer::kFlagOtherSurface))) {
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
void Inter_v7::o7_setActiveCD() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, str0);
	Common::String savedCDpath = _currentCDPath;

	for (Common::ArchiveMemberPtr file : files) {
		auto *node = dynamic_cast<Common::FSNode *>(file.get());
		if (node != nullptr &&
			setCurrentCDPath(node->getParent())) {
			debugC(5, kDebugFileIO, "o7_setActiveCD: %s -> %s", savedCDpath.c_str(),  _currentCDPath.c_str());
			storeValue(1);
			return;
		}
	}

	storeValue(0);
}

void Inter_v7::o7_findFile() {
	Common::String file_pattern = getFile(_vm->_game->_script->evalString());
	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, file_pattern);
	Common::ArchiveMemberList filesWithoutDuplicates;
	for (Common::ArchiveMemberPtr file : files) {
		bool found = false;
		for (Common::ArchiveMemberPtr fileWithoutDuplicates : filesWithoutDuplicates) {
			if (file->getName() == fileWithoutDuplicates->getName()) {
				found = true;
				break;
			}
		}

		if (!found)
			filesWithoutDuplicates.push_back(file);
	}

	debugC(5, kDebugFileIO, "o7_findFile(%s): %d matches (%d including duplicates)",
		   file_pattern.c_str(),
		   filesWithoutDuplicates.size(),
		   files.size());

	if (filesWithoutDuplicates.empty()) {
		storeString("");
		storeValue(0);
	} else {
		Common::String file = files.front()->getName();
		filesWithoutDuplicates.pop_front();
		debugC(5, kDebugFileIO, "o7_findFile(%s): first match = %s",
			   file_pattern.c_str(),
			   file.c_str());

		storeString(file.c_str());
		storeValue(1);
	}

	_remainingFilesFromPreviousSearch = filesWithoutDuplicates;
}

void Inter_v7::o7_findNextFile() {
	uint16 type;
	uint16 varIndex = _vm->_game->_script->readVarIndex(0, &type);

	Common::String file;
	if (!_remainingFilesFromPreviousSearch.empty()) {
		file = _remainingFilesFromPreviousSearch.front()->getName();
		_remainingFilesFromPreviousSearch.pop_front();
	}

	debugC(5, kDebugFileIO, "o7_findNextFile: new match = %s",
		   file.c_str());

	storeString(varIndex, type, file.c_str());
	storeValue(file.empty() ? 0 : 1);
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
	Common::String file = getFile(_vm->_game->_script->evalString());
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
	destSprite->blit(*image, left, top, right, bottom, x, y, (transp == 0) ? -1 : 0);
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

void Inter_v7::o7_loadIFFPalette() {
	Common::String file = _vm->_game->_script->evalString();
	if (!file.contains('.'))
		file += ".LBM";

	int16 startIndex = CLIP<int16>(_vm->_game->_script->readValExpr(), 0, 255);
	int16 stopIndex  = CLIP<int16>(_vm->_game->_script->readValExpr(), 0, 255);

	if (startIndex > stopIndex)
		SWAP(startIndex, stopIndex);

	Common::SeekableReadStream *iffFile = _vm->_dataIO->getFile(file);
	if (!iffFile) {
		warning("o7_loadIFFPalette(): No such file \"%s\"", file.c_str());
		return;
	}

	ImageType type = Surface::identifyImage(*iffFile);
	if (type != kImageTypeIFF) {
		warning("o7_loadIFFPalette(): \"%s\" is no IFF", file.c_str());
		return;
	}

	Image::IFFDecoder decoder;
	decoder.loadStream(*iffFile);
	if (!decoder.getPalette() || decoder.getPaletteColorCount() != 256) {
		warning("o7_loadIFFPalette(): Failed reading palette from IFF \"%s\"", file.c_str());
		return;
	}

	const byte *palette = decoder.getPalette();

	startIndex *= 3;
	stopIndex  *= 3;

	byte *dst = (byte *)_vm->_draw->_vgaPalette + startIndex;
	const byte *src = palette + startIndex;
	for (int i = startIndex; i <= stopIndex + 2; ++i) {
		*dst++ = *src++ >> 2;
	}

	if (startIndex == 0) {
		dst = (byte *)_vm->_draw->_vgaPalette;
		dst[0] = dst[1] = dst[2] = 0x00 >> 2;
	}

	if (stopIndex == 765) {
		dst = (byte *)_vm->_draw->_vgaPalette + 765;
		dst[0] = dst[1] = dst[2] = 0xFF >> 2;
	}

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Inter_v7::o7_opendBase() {
	Common::String dbFile = getFile(_vm->_game->_script->evalString());
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

void Inter_v7::o7_printText(OpFuncParams &params) {
	char buf[60];
	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_backColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_fontIndex = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSurface = Draw::kBackSurface;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor & 0xFF00) {
		_vm->_draw->_destSurface = _vm->_draw->_backColor >> 8;
		_vm->_draw->_backColor &= 0xFF;
	}

	if (_vm->_draw->_backColor == 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

	// colMod is read from conf file (_off_=xxx).
	// in Playtoons, it's not present in the conf file, thus always equal to the default value (0).
	// Maybe used in ADIs...
	//	if (!_vm->_draw->_transparency)
	//		_vm->_draw->_backColor += colMod;
	//	_vm->_draw->_frontColor += colMod;

	int16 oldTransparency = _vm->_draw->_transparency;
	int i = 0;
	do {
		for (; (_vm->_game->_script->peekChar() != '.') &&
			   (_vm->_game->_script->peekByte() != 200); i++) {
			buf[i] = _vm->_game->_script->readChar();
		}

		if (_vm->_game->_script->peekByte() != 200) {
			_vm->_game->_script->skip(1);
			switch (_vm->_game->_script->peekByte()) {
			case TYPE_VAR_INT8:
			case TYPE_ARRAY_INT8:
				Common::sprintf_s(buf + i, sizeof(buf) - i, "%d",
								  (int8) READ_VARO_UINT8(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_INT16:
			case TYPE_VAR_INT32_AS_INT16:
			case TYPE_ARRAY_INT16:
				Common::sprintf_s(buf + i, sizeof(buf) - i, "%d",
								  (int16) READ_VARO_UINT16(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_INT32:
			case TYPE_ARRAY_INT32:
				Common::sprintf_s(buf + i, sizeof(buf) - i, "%d",
								  (int32)VAR_OFFSET(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_STR:
			case TYPE_ARRAY_STR:
				Common::sprintf_s(buf + i, sizeof(buf) - i, "%s",
								  GET_VARO_STR(_vm->_game->_script->readVarIndex()));
				break;

			default:
				break;
			}
			_vm->_game->_script->skip(1);
		} else
			buf[i] = 0;

		if (_vm->_game->_script->peekByte() == 200) {
			_vm->_draw->_spriteBottom = _vm->_draw->_fonts[_vm->_draw->_fontIndex]->getCharHeight();
			_vm->_draw->_spriteRight = _vm->_draw->stringLength(_vm->_draw->_textToPrint, _vm->_draw->_fontIndex);
			_vm->_draw->adjustCoords(1, &_vm->_draw->_spriteBottom, &_vm->_draw->_spriteRight);
			if (_vm->_draw->_transparency == 0) {
				_vm->_draw->spriteOperation(DRAW_FILLRECT);
				_vm->_draw->_transparency = 1;
			}
			_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
			_vm->_draw->_transparency = oldTransparency;
			i = 0;
		} else
			i = strlen(buf);
	} while (_vm->_game->_script->peekByte() != 200);

	_vm->_game->_script->skip(1);
}
void Inter_v7::o7_fillRect(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	uint32 patternColor = _vm->_game->_script->evalInt();

	int16 savedPattern = _vm->_draw->_pattern;

	_vm->_draw->_backColor = patternColor & 0xFFFF;
	_vm->_draw->_pattern   = patternColor >> 16;

	if (_vm->_draw->_spriteRight < 0) {
		_vm->_draw->_destSpriteX += _vm->_draw->_spriteRight - 1;
		_vm->_draw->_spriteRight = -_vm->_draw->_spriteRight + 2;
	}
	if (_vm->_draw->_spriteBottom < 0) {
		_vm->_draw->_destSpriteY += _vm->_draw->_spriteBottom - 1;
		_vm->_draw->_spriteBottom = -_vm->_draw->_spriteBottom + 2;
	}

	if (_vm->_draw->_destSurface & 0x80) {
		_vm->_draw->_destSurface &= 0x7F;
		if (_vm->_draw->_destSurface > 100)
			_vm->_draw->_destSurface -= 80;

		if (_vm->_draw->_pattern & 0x8000)
			warning("o7_fillRect: pattern %d & 0x8000 != 0 stub", _vm->_draw->_pattern);
		else {
			// Replace a specific color in the rectangle
			uint8 colorToReplace = (_vm->_draw->_backColor >> 8) & 0xFF;
			_vm->_draw->_pattern = 4;
			_vm->_draw->_backColor = _vm->_draw->_backColor & 0xFF;
			// Additional condition on surface.field_10 in executables (video mode ?), seems to be always true for Adibou2
			// if (_vm->_draw->_spritesArray[_vm->_draw->_destSurface].field_10  & 0x80)) {
			SurfacePtr newSurface = _vm->_video->initSurfDesc(_vm->_draw->_spriteRight,
															  _vm->_draw->_spriteBottom,
															  8);
			newSurface->blit(*_vm->_draw->_spritesArray[_vm->_draw->_destSurface],
							 _vm->_draw->_destSpriteX,
							 _vm->_draw->_destSpriteY,
							 _vm->_draw->_destSpriteX + _vm->_draw->_spriteRight - 1,
							 _vm->_draw->_destSpriteY + _vm->_draw->_spriteRight - 1,
							 0,
							 0,
							 0);

			for (int y = 0; y < _vm->_draw->_spriteBottom; y++) {
				for (int x = 0; x < _vm->_draw->_spriteRight; x++) {
					if ((colorToReplace & 0xFFu) == newSurface->get(x, y).get())
						newSurface->putPixel(x, y, _vm->_draw->_backColor);
				}
			}

			_vm->_draw->_spritesArray[_vm->_draw->_destSurface]->blit(*newSurface,
																	  0,
																	  0,
																	  _vm->_draw->_spriteRight - 1,
																	  _vm->_draw->_spriteBottom - 1,
																	  _vm->_draw->_destSpriteX,
																	  _vm->_draw->_destSpriteY,
																	  0);

			_vm->_draw->dirtiedRect(_vm->_draw->_destSurface,
									_vm->_draw->_destSpriteX,
									_vm->_draw->_destSpriteY,
									_vm->_draw->_destSpriteX + _vm->_draw->_spriteRight - 1,
									_vm->_draw->_destSpriteY + _vm->_draw->_spriteBottom - 1);

		}
	}
	else
		_vm->_draw->spriteOperation(DRAW_FILLRECT);
	_vm->_draw->_pattern = savedPattern;
}

bool Inter_v7::setCurrentCDPath(const Common::FSNode &newDir) {
	Common::FSNode gameDataDir(ConfMan.get("path"));
	bool newDirIsGameDir = (newDir.getPath() == gameDataDir.getPath());
	Common::String newDirName = newDir.getName();

	if (!newDirIsGameDir &&
		(newDirName.equalsIgnoreCase("applis") || newDirName.equalsIgnoreCase("envir")))
		return false;

	if (!_currentCDPath.empty())
		SearchMan.setPriority(_currentCDPath, 0);

	if (newDirIsGameDir)
		_currentCDPath = "";
	else {
		_currentCDPath = newDirName;
		SearchMan.setPriority(newDirName, 1);
	}

	return true;
}

Common::Array<uint32> Inter_v7::getAdibou2InstalledApplications() {
	byte buffer[4];
	Common::Array<uint32> applicationNumbers;
	if (_vm->_saveLoad->loadToRaw("applis.inf", buffer, 4, 0)) {
		int numApplications = READ_LE_UINT32(buffer);
		for (int i = 0; i < numApplications; i++) {
			if (_vm->_saveLoad->loadToRaw("applis.inf", buffer, 4, 4 + i * 4)) {
				uint32 applicationNumber = READ_LE_UINT32(buffer);
				applicationNumbers.push_back(applicationNumber);
			}
		}
	}

	return applicationNumbers;
}

void Inter_v7::o7_drawLine(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	_vm->_game->_script->readExpr(99, 0);
	//unk_var is always set to 0
	_vm->_draw->_frontColor = _vm->_game->_script->getResultInt() & 0xFFFF; // + unk_var;
	_vm->_draw->_pattern = _vm->_game->_script->getResultInt()>>16;
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
}

void Inter_v7::o7_invalidate(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();
	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();

	_vm->_game->_script->readExpr(99, 0);
	//unk_var is always set to 0
	_vm->_draw->_frontColor = _vm->_game->_script->getResultInt() & 0xFFFF; // + unk_var;
	_vm->_draw->_pattern = _vm->_game->_script->getResultInt()>>16;

	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
}

void Inter_v7::o7_checkData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString());

	if (_vm->getGameType() == kGameTypeAdibou2
		&&
		file == "CD.INF") {
		// WORKAROUND: some versions of Adibou2 are only able to handle one CD at a time.
		// In such versions, scripts always begin to look for CD.INF file in the CD, and check
		// if its contents matches the selected application. We insert a hack here, to set
		// the directory of the wanted application as "current CD" directory.
		// This way, applications can be made available by copying them as a subdirectory of the
		// game directory, just like in multi-cd-aware versions.
		Common::Array<uint32> installedApplications = getAdibou2InstalledApplications();
		int32 indexAppli = VAR_OFFSET(20196);
		if (indexAppli == -1) {
			// New appli, find the first directory containing an application still not installed, and set it as "current CD" path.
			Common::ArchiveMemberList files;
			SearchMan.listMatchingMembers(files, file); // Search for CD.INF files
			for (Common::ArchiveMemberPtr &cdInfFile : files) {
				Common::SeekableReadStream *stream = cdInfFile->createReadStream();
				while (stream->pos() + 4 <= stream->size()) {
					// CD.INF contains a list of applications, as uint32 LE values
					uint32 applicationNumber = stream->readUint32LE();
					if (Common::find(installedApplications.begin(), installedApplications.end(), applicationNumber) == installedApplications.end()) {
						// Application not installed yet, set it as current CD path
						Common::FSNode cdInfFileNode(cdInfFile->getName());
						setCurrentCDPath(cdInfFileNode.getParent());
						break;
					}
				}
			}
		} else if (indexAppli >= 0 && (size_t) indexAppli <= installedApplications.size()) {
			// Already installed appli, find its directory and set it as "current CD" path
			int32 applicationNumber = installedApplications[indexAppli - 1];
			Common::String appliVmdName = Common::String::format("appli_%02d.vmd", applicationNumber);
			Common::ArchiveMemberList files;
			SearchMan.listMatchingMembers(files, appliVmdName);
			for (Common::ArchiveMemberPtr &member : files) {
				auto *node = dynamic_cast<Common::FSNode *>(member.get());
				if (node != nullptr) {
					if (setCurrentCDPath(node->getParent()))
						break;
				}
			}
		}
	}

	uint16 varOff = _vm->_game->_script->readVarIndex();

	int32 size   = -1;
	int16 handle = 1;
	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeNone) {
		size = _vm->_dataIO->fileSize(file);
		if (size == -1)
			warning("File \"%s\" not found", file.c_str());

	} else if (mode == SaveLoad::kSaveModeSave)
		size = _vm->_saveLoad->getSize(file.c_str());
	else if (mode == SaveLoad::kSaveModeExists)
		size = 23;

	if (size == -1)
		handle = -1;

	debugC(2, kDebugFileIO, "Requested size of file \"%s\": %d",
		   file.c_str(), size);

	WRITE_VAR_OFFSET(varOff, handle);
	if (_vm->getGameType() == kGameTypeAdibou2
		&&
		_vm->isCurrentTot("BE_CD.TOT")) {
		// WORKAROUND: in script BE_CD.TOT of Adibou 2, o7_checkData() can be called in the "leave" callback of a hotspot.
		// This corrupts the "current hotspot" variable, which is also VAR(16) (!), and lead to an infinite loop.
		// We skip writing the file size into VAR(16) here as a workarond (the value is not used anyway).
	} else
		WRITE_VAR(16, (uint32) size);
}

void Inter_v7::o7_readData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString(), false);

	uint16 dataVar = _vm->_game->_script->readVarIndex();
	int32  size    = _vm->_game->_script->readValExpr();
	int32  offset  = _vm->_game->_script->evalInt();
	int32  retSize = 0;

	debugC(2, kDebugFileIO, "Read from file \"%s\" (%d, %d bytes at %d)",
		   file.c_str(), dataVar, size, offset);

	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeSave) {

		WRITE_VAR(1, 1);

		if (!_vm->_saveLoad->load(file.c_str(), dataVar, size, offset)) {
			// Too noisy, the scripts often try to load "save" file not existing (yet)
			// GUI::MessageDialog dialog(_("Failed to load saved game from file."));
			// dialog.runModal();
		} else
			WRITE_VAR(1, 0);

		return;

	} else if (mode == SaveLoad::kSaveModeIgnore)
		return;

	if (size < 0) {
		if (readSprite(file, dataVar, size, offset))
			WRITE_VAR(1, 0);
		return;
	} else if (size == 0) {
		dataVar = 0;
		size = _vm->_game->_script->getVariablesCount() * 4;
	}

	byte *buf = _variables->getAddressOff8(dataVar);

	if (file.empty()) {
		WRITE_VAR(1, size);
		return;
	}

	WRITE_VAR(1, 1);
	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(file);
	if (!stream)
		return;

	_vm->_draw->animateCursor(4);
	if (offset > stream->size()) {
		warning("oPlaytoons_readData: File \"%s\", Offset (%d) > file size (%d)",
				file.c_str(), offset, (int)stream->size());
		delete stream;
		return;
	}

	if (offset < 0)
		stream->seek(offset + 1, SEEK_END);
	else
		stream->seek(offset);

	if (((dataVar >> 2) == 59) && (size == 4)) {
		WRITE_VAR(59, stream->readUint32LE());
		// The scripts in some versions divide through 256^3 then,
		// effectively doing a LE->BE conversion
		if ((_vm->getPlatform() != Common::kPlatformDOS) && (VAR(59) < 256))
			WRITE_VAR(59, SWAP_BYTES_32(VAR(59)));
	} else
		retSize = stream->read(buf, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete stream;
}

void Inter_v7::o7_writeData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString(), false);

	int16 dataVar = _vm->_game->_script->readVarIndex();
	int32 size    = _vm->_game->_script->readValExpr();
	int32 offset  = _vm->_game->_script->evalInt();

	debugC(2, kDebugFileIO, "Write to file \"%s\" (%d, %d bytes at %d)",
		   file.c_str(), dataVar, size, offset);

	WRITE_VAR(1, 1);

	SaveLoad::SaveMode mode = _vm->_saveLoad ? _vm->_saveLoad->getSaveMode(file.c_str()) : SaveLoad::kSaveModeNone;
	if (mode == SaveLoad::kSaveModeSave) {

		if (!_vm->_saveLoad->save(file.c_str(), dataVar, size, offset)) {

			GUI::MessageDialog dialog(_("Failed to save game to file."));
			dialog.runModal();

		} else
			WRITE_VAR(1, 0);

	} else if (mode == SaveLoad::kSaveModeIgnore)
		return;
	else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to write to file \"%s\"", file.c_str());
}


void Inter_v7::o7_oemToANSI(OpGobParams &params) {
	_vm->_game->_script->skip(2);
}

void Inter_v7::o7_gob0x201(OpGobParams &params) {
	uint16 varIndex = _vm->_game->_script->readUint16();

	WRITE_VAR(varIndex, 1);
}
} // End of namespace Gob
