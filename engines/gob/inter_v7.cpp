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

#include "common/endian.h"
#include "common/archive.h"
#include "common/winexe_pe.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "image/iff.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/expression.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"
#include "gob/save/saveload.h"

namespace Gob {

#define OPCODEVER Inter_v7
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v7::Inter_v7(GobEngine *vm) : Inter_Playtoons(vm), _cursors(nullptr) {
}

Inter_v7::~Inter_v7() {
	delete _cursors;
}

void Inter_v7::setupOpcodesDraw() {
	Inter_Playtoons::setupOpcodesDraw();

	OPCODEDRAW(0x0C, o7_draw0x0C);
	OPCODEDRAW(0x0D, o7_loadCursor);
	OPCODEDRAW(0x17, o7_loadMultObject);
	OPCODEDRAW(0x44, o7_displayWarning);
	OPCODEDRAW(0x45, o7_logString);
	OPCODEDRAW(0x52, o7_moveGoblin);
	OPCODEDRAW(0x55, o7_setGoblinState);
	OPCODEDRAW(0x57, o7_intToString);
	OPCODEDRAW(0x59, o7_callFunction);
	OPCODEDRAW(0x5A, o7_loadFunctions);
	OPCODEDRAW(0x60, o7_copyFile);
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
	OPCODEFUNC(0x4D, o7_readData);
}

void Inter_v7::setupOpcodesGob() {
	Inter_Playtoons::setupOpcodesGob();

	OPCODEGOB(420, o7_oemToANSI);
	OPCODEGOB(513, o7_gob0x201);
}

void Inter_v7::o7_draw0x0C() {
	WRITE_VAR(17, 0);
}

void Inter_v7::resizeCursors(int16 width, int16 height, int16 count, bool transparency) {
	if (width <= 0)
		width = _vm->_draw->_cursorWidth;
	if (height <= 0)
		height = _vm->_draw->_cursorHeight;

	width  = MAX<uint16>(width , _vm->_draw->_cursorWidth);
	height = MAX<uint16>(height, _vm->_draw->_cursorHeight);

	_vm->_draw->_transparentCursor = transparency;

	// Cursors sprite already big enough
	if ((_vm->_draw->_cursorWidth >= width) && (_vm->_draw->_cursorHeight >= height) &&
	    (_vm->_draw->_cursorCount >= count))
		return;

	_vm->_draw->_cursorCount  = count;
	_vm->_draw->_cursorWidth  = width;
	_vm->_draw->_cursorHeight = height;

	_vm->_draw->freeSprite(Draw::kCursorSurface);
	_vm->_draw->_cursorSprites.reset();
	_vm->_draw->_cursorSpritesBack.reset();
	_vm->_draw->_scummvmCursor.reset();

	_vm->_draw->initSpriteSurf(Draw::kCursorSurface, width * count, height, 2);

	_vm->_draw->_cursorSpritesBack = _vm->_draw->_spritesArray[Draw::kCursorSurface];
	_vm->_draw->_cursorSprites     = _vm->_draw->_cursorSpritesBack;

	_vm->_draw->_scummvmCursor = _vm->_video->initSurfDesc(width, height, SCUMMVM_CURSOR);

	for (int i = 0; i < 40; i++) {
		_vm->_draw->_cursorAnimLow[i] = -1;
		_vm->_draw->_cursorAnimDelays[i] = 0;
		_vm->_draw->_cursorAnimHigh[i] = 0;
	}
	_vm->_draw->_cursorAnimLow[1] = 0;

	delete[] _vm->_draw->_doCursorPalettes;
	delete[] _vm->_draw->_cursorPalettes;
	delete[] _vm->_draw->_cursorKeyColors;
	delete[] _vm->_draw->_cursorPaletteStarts;
	delete[] _vm->_draw->_cursorPaletteCounts;
	delete[] _vm->_draw->_cursorHotspotsX;
	delete[] _vm->_draw->_cursorHotspotsY;

	_vm->_draw->_cursorPalettes      = new byte[256 * 3 * count];
	_vm->_draw->_doCursorPalettes    = new bool[count];
	_vm->_draw->_cursorKeyColors     = new byte[count];
	_vm->_draw->_cursorPaletteStarts = new uint16[count];
	_vm->_draw->_cursorPaletteCounts = new uint16[count];
	_vm->_draw->_cursorHotspotsX     = new int32[count];
	_vm->_draw->_cursorHotspotsY     = new int32[count];

	memset(_vm->_draw->_cursorPalettes     , 0, count * 256 * 3);
	memset(_vm->_draw->_doCursorPalettes   , 0, count * sizeof(bool));
	memset(_vm->_draw->_cursorKeyColors    , 0, count * sizeof(byte));
	memset(_vm->_draw->_cursorPaletteStarts, 0, count * sizeof(uint16));
	memset(_vm->_draw->_cursorPaletteCounts, 0, count * sizeof(uint16));
	memset(_vm->_draw->_cursorHotspotsX    , 0, count * sizeof(int32));
	memset(_vm->_draw->_cursorHotspotsY    , 0, count * sizeof(int32));
}

void Inter_v7::o7_loadCursor() {
	int16          cursorIndex = _vm->_game->_script->readValExpr();
	Common::String cursorName  = _vm->_game->_script->evalString();

	// Clear the cursor sprite at that index
	_vm->_draw->_cursorSprites->fillRect(cursorIndex * _vm->_draw->_cursorWidth, 0,
			cursorIndex * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	// If the cursor name is empty, that cursor will be drawn by the scripts
	if (cursorName.empty() || cursorName == "VIDE") { // "VIDE" is "empty" in french
		// Make sure the cursors sprite is big enough and set to non-extern palette
		resizeCursors(-1, -1, cursorIndex + 1, true);
		_vm->_draw->_doCursorPalettes[cursorIndex] = false;
		return;
	}

	Graphics::WinCursorGroup *cursorGroup = nullptr;
	Graphics::Cursor *defaultCursor = nullptr;

	// Load the cursor file and cursor group
	if (loadCursorFile())
		cursorGroup = Graphics::WinCursorGroup::createCursorGroup(_cursors, Common::WinResourceID(cursorName));

	// If the requested cursor does not exist, create a default one
	const Graphics::Cursor *cursor = nullptr;
	if (!cursorGroup || cursorGroup->cursors.empty() || !cursorGroup->cursors[0].cursor) {
		defaultCursor = Graphics::makeDefaultWinCursor();

		cursor = defaultCursor;
	} else
		cursor = cursorGroup->cursors[0].cursor;

	// Make sure the cursors sprite it big enough
	resizeCursors(cursor->getWidth(), cursor->getHeight(), cursorIndex + 1, true);

	Surface cursorSurf(cursor->getWidth(), cursor->getHeight(), 1, cursor->getSurface());

	_vm->_draw->_cursorSprites->blit(cursorSurf, cursorIndex * _vm->_draw->_cursorWidth, 0);

	memcpy(_vm->_draw->_cursorPalettes + cursorIndex * 256 * 3, cursor->getPalette(), cursor->getPaletteCount() * 3);

	_vm->_draw->_doCursorPalettes   [cursorIndex] = true;
	_vm->_draw->_cursorKeyColors    [cursorIndex] = cursor->getKeyColor();
	_vm->_draw->_cursorPaletteStarts[cursorIndex] = cursor->getPaletteStartIndex();
	_vm->_draw->_cursorPaletteCounts[cursorIndex] = cursor->getPaletteCount();
	_vm->_draw->_cursorHotspotsX    [cursorIndex] = cursor->getHotspotX();
	_vm->_draw->_cursorHotspotsY    [cursorIndex] = cursor->getHotspotY();

	delete cursorGroup;
	delete defaultCursor;
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
			_vm->_vidPlayer->closeVideo(obj.videoSlot - 1);
			obj.videoSlot = 0;
			_vm->_draw->freeSprite(50 - obj.pAnimData->animation - 1);
		}

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
	}
	else
		warning("Attempted to write to file \"%s\" while copying from \"%s\"",
				destFile.c_str(),
				sourceFile.c_str());

}

void Inter_v7::o7_copyFile() {
	Common::String path1 = _vm->_game->_script->evalString();
	Common::String path2 = _vm->_game->_script->evalString();

	debugC(2, kDebugFileIO, "Copy file \"%s\" to \"%s", path1.c_str(), path2.c_str());

	Common::String file1 = getFile(path1.c_str());
	Common::String file2 = getFile(path2.c_str());
	if (file1.equalsIgnoreCase(file2)) {
		warning("o7_copyFile(): \"%s\" == \"%s\"", path1.c_str(), path2.c_str());
		return;
	}

	copyFile(file1, file2);
}

void Inter_v7::o7_moveFile() {
	Common::String path1 = _vm->_game->_script->evalString();
	Common::String path2 = _vm->_game->_script->evalString();

	Common::String file1 = getFile(path1.c_str());
	Common::String file2 = getFile(path2.c_str());

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

	_vm->_draw->closeScreen();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);

	_vm->_video->_splitSurf.reset();
	_vm->_draw->_spritesArray[24].reset();
	_vm->_draw->_spritesArray[25].reset();

	_vm->_global->_videoMode = videoMode;
	_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
							_vm->_global->_inter_mouseY);
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
		{
			_vm->_vidPlayer->closeVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1);
			_vm->_mult->_objects[props.startFrame].videoSlot = 0;
			_vm->_draw->freeSprite(50 - _vm->_mult->_objects[props.startFrame].pAnimData->animation - 1);
		}

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

		Common::strlcpy(_vm->_mult->_objects[props.startFrame].animName, file.c_str(), 16);

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
		// TODO Adibou2 temp sound workaround assert(_finished) failed
		/*if (!props.canceled)
			_vm->_vidPlayer->waitSoundEnd(slot);*/
		_vm->_vidPlayer->closeVideo(slot);
	}

}
void Inter_v7::o7_setActiveCD() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, str0);

	for (Common::ArchiveMemberPtr file : files) {
		auto *node = dynamic_cast<Common::FSNode *>(file.get());
		if (node != nullptr) {
			Common::String path = node->getParent().getName();
			if (path.equalsIgnoreCase("applis")
				|| path.equalsIgnoreCase("envir"))
				continue;

			debugC(5, kDebugFileIO, "o7_setActiveCD: %s -> %s",  _currentCDPath.c_str(), path.c_str());
			if (!_currentCDPath.empty())
				SearchMan.setPriority(_currentCDPath, 0);

			_currentCDPath = path;
			SearchMan.setPriority(path, 1);
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
	for (Common::ArchiveMemberPtr file : files)
	{
		bool found = false;
		for (Common::ArchiveMemberPtr fileWithoutDuplicates : filesWithoutDuplicates)
		{
			if (file->getName() == fileWithoutDuplicates->getName())
			{
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
	}
	else {
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
	Common::String id     = getFile(_vm->_game->_script->evalString());

	dbFile += ".DBF";

	_databases.setLanguage(_vm->_language);
	if (!_databases.open(id, dbFile)) {
		WRITE_VAR(27, 0); // Failure
		return;
	}

	WRITE_VAR(27, 1); // Success
}

void Inter_v7::o7_closedBase() {
	Common::String id = getFile(_vm->_game->_script->evalString());

	if (_databases.close(id))
		WRITE_VAR(27, 1); // Success
	else
		WRITE_VAR(27, 0); // Failure
}

void Inter_v7::o7_getDBString() {
	Common::String id      = getFile(_vm->_game->_script->evalString());
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

void Inter_v7::o7_readData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString());

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

	if (file[0] == 0) {
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

void Inter_v7::o7_oemToANSI(OpGobParams &params) {
	_vm->_game->_script->skip(2);
}

void Inter_v7::o7_gob0x201(OpGobParams &params) {
	uint16 varIndex = _vm->_game->_script->readUint16();

	WRITE_VAR(varIndex, 1);
}

bool Inter_v7::loadCursorFile() {
	if (_cursors)
		return true;

	_cursors = new Common::PEResources();

	if (_cursors->loadFromEXE("cursor32.dll"))
		return true;

	delete _cursors;
	_cursors = nullptr;

	return false;
}

} // End of namespace Gob
