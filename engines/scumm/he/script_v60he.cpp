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

#include "common/archive.h"
#include "common/savefile.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/imuse/imuse.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/map_main.h"

namespace Scumm {

struct vsUnpackCtx {
	uint8 size;
	uint8 type;
	uint8 b;
	uint8 *ptr;
};

struct vsPackCtx {
	int size;
	uint8 buf[256];
};

static void virtScreenSavePackBuf(vsPackCtx *ctx, uint8 *&dst, int len);
static void virtScreenSavePackByte(vsPackCtx *ctx, uint8 *&dst, int len, uint8 b);
static uint8 virtScreenLoadUnpack(vsUnpackCtx *ctx, byte *data);
static int virtScreenSavePack(byte *dst, byte *src, int len, int unk);

// Compatibility notes:
//
// FBEAR (fbear, fbeardemo)
//     transparency in akos.cpp
//     negative size in file read/write

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v60he, x)

void ScummEngine_v60he::setupOpcodes() {
	ScummEngine_v6::setupOpcodes();

	_opcodes[0x63].setProc(nullptr, nullptr);
	_opcodes[0x64].setProc(nullptr, nullptr);
	OPCODE(0x70, o60_setState);
	OPCODE(0x98, o60_isSoundRunning);
	_opcodes[0x9a].setProc(nullptr, nullptr);
	OPCODE(0x9c, o60_roomOps);
	OPCODE(0x9d, o60_actorOps);
	_opcodes[0xac].setProc(nullptr, nullptr);
	OPCODE(0xbd, o6_stopObjectCode);
	OPCODE(0xc8, o60_kernelGetFunctions);
	OPCODE(0xc9, o60_kernelSetFunctions);
	OPCODE(0xd9, o60_closeFile);
	OPCODE(0xda, o60_openFile);
	OPCODE(0xdb, o60_readFile);
	OPCODE(0xdc, o60_writeFile);
	OPCODE(0xde, o60_deleteFile);
	OPCODE(0xdf, o60_rename);
	OPCODE(0xe0, o60_soundOps);
	OPCODE(0xe2, o60_localizeArrayToScript);
	OPCODE(0xe9, o60_seekFilePos);
	OPCODE(0xea, o60_redimArray);
	OPCODE(0xeb, o60_readFilePos);
	_opcodes[0xec].setProc(nullptr, nullptr);
	_opcodes[0xed].setProc(nullptr, nullptr);
}

Common::Path ScummEngine_v60he::convertFilePath(const byte *src) {
	debug(2, "convertFilePath in: '%s'", (const char *)src);

	int srcSize = resStrLen(src);
	int start = 0;

	if (srcSize > 2) {
		if (src[0] == ':') { // Game Data Path (Macintosh)
			// The default game data path is set to ':' by ScummVM
			start = 1;
		} else if (src[0] == '.' && src[1] == '\\') { // Game Data Path (Windows)
			// The default game data path is set to '.\\' by ScummVM
			start = 2;
		} else if (src[0] == '*' && src[1] == '\\') { // Save Game Path (Windows HE72 - HE100)
			// The default save game path is set to '*\\' by ScummVM
			start = 2;
		} else if (src[0] == '*' && src[1] == ':') { // Save Game Path (Macintosh HE72 - HE100)
			// The default save game path is set to '*:' by ScummVM
			start = 2;
		} else if (src[0] == 'c' && src[1] == ':') { // Save Game Path (HE60 - HE71)
			// The default save path is game path (DOS) or 'c:\\hegames\\' (Windows)
			for (start = srcSize; start != 0; start--)
				if (src[start - 1] == '\\')
					break;
		} else if (src[0] == 'u' && src[1] == 's') { // Save Game Path (Moonbase Commander)
			// The default save path is 'user\\'
			start = 5;
		}
	}

	Common::String dst;

	for (int i = start; i < srcSize; i++) {
		// Convert path separators
		if (src[i] == '\\' || src[i] == ':')
			dst += '/';
		else
			dst += src[i];
	}

	// Sanity check
	if (dst.lastChar() == '/')
		dst.deleteLastChar();

	debug(2, "convertFilePath out: '%s'", dst.c_str());

	return Common::Path(dst, '/');
}

Common::String ScummEngine_v60he::convertSavePath(const byte *src) {
	debug(2, "convertSavePath in: '%s'", (const char *)src);

	// Strip us down to only the file
	Common::String filePath = convertFilePath(src).baseName();

	// Prepend the target name
	filePath = _targetName + '-' + filePath;

	debug(2, "convertSavePath out: '%s'", filePath.c_str());

	return filePath;
}

Common::String ScummEngine_v60he::convertSavePathOld(const byte *src) {
	// This is provided solely for loading older saved games.
	// No new saves should go through this function.

	int srcSize = resStrLen(src);

	// Old hacky target name insertion
	// (This breaks the soccer and football games)
	if (src[srcSize - 3] == 's' && src[srcSize - 2] == 'g')
		return _targetName + ".sg" + (char)src[srcSize - 1];

	if (src[0] == 'u' && src[1] == 's') {
		// Save Game Path (Moonbase Commander)
		// The default save path is 'user/'
		return (const char *)src + 5;
	} else if (src[0] == '*' && (src[1] == '\\' || src[1] == ':')) {
		// Save Game Path (HE72 - HE100)
		// The default save game path is set to '*\\' by ScummVM for Windows
		// and '*:' for Macintosh
		return (const char *)src + 2;
	} else if (src[0] == 'c' && src[1] == ':') {
		// The default save path is game path (DOS) or 'c:\\hegames\\' (Windows)
		for (int i = srcSize; i > 0; i--)
			if (src[i] == '\\')
				return (const char *)src + i + 1;
	}

	// Can't reach here
	return "";
}

Common::SeekableReadStream *ScummEngine_v60he::openFileForReading(const byte *fileName) {
#ifdef ENABLE_HE
	if (_moonbase) {
		Common::SeekableReadStream *substitutedFile = _moonbase->_map->substituteFile(fileName);
		if (substitutedFile)
			return substitutedFile;
	}
#endif
	Common::SeekableReadStream *saveFile = openSaveFileForReading(fileName);

	if (saveFile)
		return saveFile;

	return SearchMan.createReadStreamForMember(convertFilePath(fileName));
}

Common::SeekableReadStream *ScummEngine_v60he::openSaveFileForReading(const byte *fileName) {
	Common::SeekableReadStream *file = _saveFileMan->openForLoading(convertSavePath(fileName));

	if (file)
		return file;

	return _saveFileMan->openForLoading(convertSavePathOld(fileName));
}

Common::SeekableWriteStream *ScummEngine_v60he::openSaveFileForWriting(const byte *fileName) {
	// HACK: Disable compression for Moonbase.  Fixes custom map saving.
	return _saveFileMan->openForSaving(convertSavePath(fileName), _game.id != GID_MOONBASE);
}

void ScummEngine_v60he::deleteSaveFile(const byte *fileName) {
	Common::String convertedName = convertSavePath(fileName);

	if (!_saveFileMan->listSavefiles(convertedName).empty()) {
		_saveFileMan->removeSavefile(convertedName);
		return;
	}

	convertedName = convertSavePathOld(fileName);

	if (!_saveFileMan->listSavefiles(convertedName).empty())
		_saveFileMan->removeSavefile(convertedName);
}

void ScummEngine_v60he::renameSaveFile(const byte *from, const byte *to) {
	Common::String toName = convertSavePath(to);

	if (_saveFileMan->renameSavefile(convertSavePathOld(from), toName))
		return;

	_saveFileMan->renameSavefile(convertSavePath(from), toName);
}

Common::SeekableWriteStream *ScummEngine_v60he::openSaveFileForAppending(const byte *fileName) {
	Common::SeekableReadStream *initialFile = openSaveFileForReading(fileName);
	byte *initialData = nullptr;
	uint32 initialDataSize = 0;

	if (initialFile) {
		initialDataSize = initialFile->size();

		if (initialDataSize > 0) {
			initialData = new byte[initialDataSize];
			initialFile->read(initialData, initialDataSize);
		}

		delete initialFile;
	}

	Common::SeekableWriteStream *output = openSaveFileForWriting(fileName);

	if (!output) {
		delete[] initialData;
		return nullptr;
	}

	if (initialData) {
		output->write(initialData, initialDataSize);
		delete[] initialData;
	}

	return output;
}

Common::SeekableReadStream *ScummEngine_v60he::openSaveFileForReading(int slot, bool compat, Common::String &fileName) {
	if (slot == 255) {
		// HACK: Allow custom filenames for save game system in HE Games
		fileName = convertSavePath((const byte *)_saveLoadFileName.c_str());

		Common::SeekableReadStream *stream = _saveFileMan->openForLoading(fileName);
		if (stream)
			return stream;

		Common::String oldFileName = convertSavePathOld((const byte *)_saveLoadFileName.c_str());
		stream = _saveFileMan->openForLoading(oldFileName);

		if (stream) {
			fileName = oldFileName;
			return stream;
		}

		return nullptr;
	}

	return ScummEngine::openSaveFileForReading(slot, compat, fileName);
}

Common::SeekableWriteStream *ScummEngine_v60he::openSaveFileForWriting(int slot, bool compat, Common::String &fileName) {
	if (slot == 255) {
		// HACK: Allow custom filenames for save game system in HE Games
		fileName = convertSavePath((const byte *)_saveLoadFileName.c_str());
		return _saveFileMan->openForSaving(fileName);
	}

	return ScummEngine::openSaveFileForWriting(slot, compat, fileName);
}

void ScummEngine_v60he::o60_setState() {
	int state = pop();
	int obj = pop();

	// WORKAROUND for bug #4776, knocker floats while door is open
	// This hides the knocker when the closed door object is set to hide.
	if (_game.id == GID_FREDDI && _currentRoom == 7) {
		if (_game.heversion == 71 && obj == 35 && state == 0) {
			putState(26, 0);
		}
		if (_game.heversion == 73 && obj == 52 && state == 0) {
			putState(43, 0);
		}
	}

	if (state & 0x8000) {
		state &= 0x7FFF;
		putState(obj, state);
		if (_game.heversion >= 72)
			removeObjectFromDrawQue(obj);
	} else {
		putState(obj, state);
		markObjectRectAsDirty(obj);
		if (_bgNeedsRedraw)
			clearDrawObjectQueue();
	}
}

void ScummEngine_v60he::o60_roomOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ROOM_SCROLL:
		b = pop();
		a = pop();
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;

	case SO_ROOM_SCREEN:
		b = pop();
		a = pop();
		if (_game.heversion >= 71)
			initScreens(a, _screenHeight);
		else
			initScreens(a, b);
		break;

	case SO_ROOM_PALETTE:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case SO_ROOM_SHAKE_ON:
		setShake(1);
		break;

	case SO_ROOM_SHAKE_OFF:
		setShake(0);
		break;

	case SO_ROOM_INTENSITY:
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case SO_ROOM_SAVEGAME:
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case SO_ROOM_FADE:
		a = pop();
		if (_game.heversion >= 70) {
			// Defaults to 1 but doesn't use fade effects
		} else if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;

	case SO_RGB_ROOM_INTENSITY:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case SO_ROOM_SHADOW:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		if (_game.heversion == 60)
			setShadowPalette(a, b, c, d, e, 0, 256);
		break;

	case SO_ROOM_TRANSFORM:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		palManipulateInit(a, b, c, d);
		break;

	case SO_CYCLE_SPEED:
		b = pop();
		a = pop();
		assertRange(1, a, 16, "o60_roomOps: 187: color cycle");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;

	case SO_ROOM_NEW_PALETTE:
		a = pop();
		setCurrentPalette(a);
		break;
	case SO_ROOM_COPY_PALETTE:
		a = pop();
		b = pop();
		copyPalColor(a, b);
		break;
	case SO_ROOM_SAVEGAME_BY_NAME:
		byte buffer[100];
		int len;

		convertMessageToString(_scriptPointer, buffer, sizeof(buffer));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;

		_saveLoadFileName = (char *)buffer;
		debug(1, "o60_roomOps: case 221: filename %s", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;
		break;
	case SO_OBJECT_ORDER:		// HE 7.1
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;
	case SO_ROOM_PALETTE_IN_ROOM:		// HE 7.1
		b = pop();
		a = pop();
		setRoomPalette(a, b);
		break;
	default:
		error("o60_roomOps: default case %d", subOp);
	}
}

void ScummEngine_v60he::swapObjects(int object1, int object2) {
	int idx1 = -1, idx2 = -1;

	for (int i = 0; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr == object1)
			idx1 = i;

		if (_objs[i].obj_nr == object2)
			idx2 = i;
	}

	if (idx1 == -1 || idx2 == -1 || idx1 <= idx2)
		return;

	stopObjectScript(object1);
	stopObjectScript(object2);

	ObjectData tmpOd;

	memcpy(&tmpOd, &_objs[idx1], sizeof(tmpOd));
	memcpy(&_objs[idx1], &_objs[idx2], sizeof(tmpOd));
	memcpy(&_objs[idx2], &tmpOd, sizeof(tmpOd));
}

void ScummEngine_v60he::o60_actorOps() {
	ActorHE *a;
	int i, j, k;
	int args[8];

	byte subOp = fetchScriptByte();
	if (subOp == SO_ACTOR_INIT) {
		_curActor = pop();
		return;
	}

	a = (ActorHE *)derefActorSafe(_curActor, "o60_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case SO_ACTOR_DEFAULT_CLIPPED:
		// _game.heversion >= 70
		{
			int x1, y1, x2, y2;
			y2 = pop();
			x2 = pop();
			y1 = pop();
			x1 = pop();
			setActorClippingRect(-1, x1, y1, x2, y2);
			break;
		}
	case SO_COSTUME:
		a->setActorCostume(pop());
		break;
	case SO_STEP_DIST:
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case SO_SOUND:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case SO_WALK_ANIMATION:
		a->_walkFrame = pop();
		break;
	case SO_TALK_ANIMATION:
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case SO_STAND_ANIMATION:
		a->_standFrame = pop();
		break;
	case SO_ANIMATION:
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case SO_DEFAULT:
		a->initActor(0);
		break;
	case SO_ELEVATION:
		a->setElevation(pop());
		break;
	case SO_ANIMATION_DEFAULT:
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case SO_PALETTE:
		j = pop();
		i = pop();
		assertRange(0, i, 255, "o60_actorOps: palette slot");
		a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case SO_TALK_COLOR:
		a->_talkColor = pop();
		break;
	case SO_ACTOR_NAME:
		loadPtrToResource(rtActorName, a->_number, nullptr);
		break;
	case SO_INIT_ANIMATION:
		a->_initFrame = pop();
		break;
	case SO_ACTOR_WIDTH:
		a->_width = pop();
		break;
	case SO_SCALE:
		i = pop();
		a->setScale(i, i);
		break;
	case SO_NEVER_ZCLIP:
		a->_forceClip = 0;
		break;
	case SO_ALWAYS_ZCLIP:
		a->_forceClip = pop();
		break;
	case SO_IGNORE_BOXES:
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_FOLLOW_BOXES:
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_ANIMATION_SPEED:
		a->setAnimSpeed(pop());
		break;
	case SO_SHADOW:
		a->_shadowMode = pop();
		a->_needRedraw = true;
		if (_game.heversion >= 70) {
			a->_needBgReset = true;
		}
		break;
	case SO_TEXT_OFFSET:
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case SO_ACTOR_VARIABLE:
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case SO_ACTOR_IGNORE_TURNS_ON:
		a->_ignoreTurns = true;
		break;
	case SO_ACTOR_IGNORE_TURNS_OFF:
		a->_ignoreTurns = false;
		break;
	case SO_NEW:
		a->initActor(2);
		break;
	case SO_BACKGROUND_ON:
		a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case SO_BACKGROUND_OFF:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case SO_TALKIE:
		{
		byte string[128];
		copyScriptString(string);
		int slot = pop();

		int len = resStrLen(string) + 1;
		convertMessageToString(string, a->_heTalkQueue[slot].sentence, len);

		a->_heTalkQueue[slot].posX = a->_talkPosX;
		a->_heTalkQueue[slot].posY = a->_talkPosY;
		a->_heTalkQueue[slot].color = a->_talkColor;
		break;
		}
	default:
		error("o60_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v60he::o60_kernelSetFunctions() {
	int args[29];
	int num;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		// Used to restore images when decorating cake in
		// Fatty Bear's Birthday Surprise
		virtScreenLoad(args[1], args[2], args[3], args[4], args[5]);
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 8:
		//Used before mini games in 3DO versions, seems safe to ignore.
		break;
	default:
		error("o60_kernelSetFunctions: default case %d (param count %d)", args[0], num);
	}
}

void ScummEngine_v60he::virtScreenLoad(int resIdx, int x1, int y1, int x2, int y2) {
	vsUnpackCtx ctx;
	memset(&ctx, 0, sizeof(ctx));
	VirtScreen &vs = _virtscr[kMainVirtScreen];

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, resIdx);
	virtScreenLoadUnpack(&ctx, ah->data);
	for (int j = y1; j <= y2; ++j) {
		uint8 *p1 = vs.getPixels(x1, j - vs.topline);
		uint8 *p2 = vs.getBackPixels(x1, j - vs.topline);
		if (x2 >= x1) {
			uint32 w = x2 - x1 + 1;
			while (w--) {
				uint8 decByte = virtScreenLoadUnpack(&ctx, nullptr);
				*p1++ = decByte;
				*p2++ = decByte;
			}
		}
	}
	markRectAsDirty(kMainVirtScreen, x1, x2, y1, y2 + 1, USAGE_BIT_RESTORED);
}

uint8 virtScreenLoadUnpack(vsUnpackCtx *ctx, byte *data) {
	uint8 decByte;
	if (data != nullptr) {
		ctx->type = 0;
		ctx->ptr = data;
		decByte = 0;
	} else {
		uint8 a;
		if (ctx->type == 0) {
			a = *(ctx->ptr)++;
			if (a & 1) {
				ctx->type = 1;
				ctx->b = *(ctx->ptr)++;
			} else {
				ctx->type = 2;
			}
			ctx->size = a;
			a = (a >> 1) + 1;
		} else {
			a = ctx->size;
		}
		if (ctx->type == 2) {
			ctx->b = *(ctx->ptr)++;
		}
		ctx->size = a - 1;
		if (ctx->size == 0) {
			ctx->type = 0;
		}
		decByte = ctx->b;
	}
	return decByte;
}


void ScummEngine_v60he::o60_kernelGetFunctions() {
	int args[29];
	byte *data;
	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		// Used to store images when decorating cake in
		// Fatty Bear's Birthday Surprise
		writeVar(0, 0);
		data = defineArray(0, kByteArray, 0, virtScreenSave(nullptr, args[1], args[2], args[3], args[4]));
		virtScreenSave(data, args[1], args[2], args[3], args[4]);
		push(readVar(0));
		break;
	default:
		error("o60_kernelGetFunctions: default case %d", args[0]);
	}
}

int ScummEngine_v60he::virtScreenSave(byte *dst, int x1, int y1, int x2, int y2) {
	int packedSize = 0;
	VirtScreen &vs = _virtscr[kMainVirtScreen];

	for (int j = y1; j <= y2; ++j) {
		uint8 *p = vs.getBackPixels(x1, j - vs.topline);

		int size = virtScreenSavePack(dst, p, x2 - x1 + 1, 0);
		if (dst != nullptr) {
			dst += size;
		}
		packedSize += size;
	}
	return packedSize;
}

int virtScreenSavePack(byte *dst, byte *src, int len, int unk) {
	vsPackCtx ctx;
	memset(&ctx, 0, sizeof(ctx));

	uint8 prevByte, curByte;

	ctx.buf[0] = prevByte = *src++;
	int flag = 0;
	int iend = 1;
	int ibeg = 0;

	for (--len; len != 0; --len, prevByte = curByte) {
		bool pass = false;

		assert(iend < 0x100);
		ctx.buf[iend] = curByte = *src++;
		++iend;

		if (flag == 0) {
			if (iend > 0x80) {
				virtScreenSavePackBuf(&ctx, dst, iend - 1);
				ctx.buf[0] = curByte;
				iend = 1;
				ibeg = 0;
				continue;
			}
			if (prevByte != curByte) {
				ibeg = iend - 1;
				continue;
			}
			if (iend - ibeg < 3) {
				if (ibeg != 0) {
					pass = true;
				} else {
					flag = 1;
				}
			} else {
				if (ibeg > 0) {
					virtScreenSavePackBuf(&ctx, dst, ibeg);
				}
				flag = 1;
			}
		}
		if (flag == 1 || pass) {
			if (prevByte != curByte || iend - ibeg > 0x80) {
				virtScreenSavePackByte(&ctx, dst, iend - ibeg - 1, prevByte);
				ctx.buf[0] = curByte;
				iend = 1;
				ibeg = 0;
				flag = 0;
			}
		}
	}

	if (flag == 0) {
		virtScreenSavePackBuf(&ctx, dst, iend);
	} else if (flag == 1) {
		virtScreenSavePackByte(&ctx, dst, iend - ibeg, prevByte);
	}
	return ctx.size;
}

void virtScreenSavePackBuf(vsPackCtx *ctx, uint8 *&dst, int len) {
	if (dst) {
		*dst++ = (len - 1) * 2;
	}
	++ctx->size;
	if (len > 0) {
		ctx->size += len;
		if (dst) {
			memcpy(dst, ctx->buf, len);
			dst += len;
		}
	}
}

void virtScreenSavePackByte(vsPackCtx *ctx, uint8 *&dst, int len, uint8 b) {
	if (dst) {
		*dst++ = ((len - 1) * 2) | 1;
	}
	++ctx->size;
	if (dst) {
		*dst++ = b;
	}
	++ctx->size;
}

void ScummEngine_v60he::o60_openFile() {
	int mode, len, slot, i;
	byte buffer[100];

	convertMessageToString(_scriptPointer, buffer, sizeof(buffer));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1, "Trying to open file '%s'", (char *)buffer);

	mode = pop();
	slot = -1;
	for (i = 0; i < 17; i++) {
		if (_hInFileTable[i] == nullptr && _hOutFileTable[i] == nullptr) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch (mode) {
		case 1:
			_hInFileTable[slot] = openFileForReading(buffer);
			break;
		case 2:
			_hOutFileTable[slot] = openSaveFileForWriting(buffer);
			break;
		default:
			error("o60_openFile(): wrong open file mode %d", mode);
		}

		if (_hInFileTable[slot] == nullptr && _hOutFileTable[slot] == nullptr)
			slot = -1;

	}
	push(slot);
}

void ScummEngine_v60he::o60_closeFile() {
	int slot = pop();
	if (0 <= slot && slot < 17) {
		if (_hOutFileTable[slot]) {
			_hOutFileTable[slot]->finalize();
			delete _hOutFileTable[slot];
			_hOutFileTable[slot] = nullptr;
		}

		delete _hInFileTable[slot];
		_hInFileTable[slot] = nullptr;
	}
}

void ScummEngine_v60he::o60_deleteFile() {
	int len;
	byte buffer[100];

	convertMessageToString(_scriptPointer, buffer, sizeof(buffer));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1, "o60_deleteFile (\"%s\")", (char *)buffer);

	deleteSaveFile(buffer);
}

void ScummEngine_v60he::o60_rename() {
	int len;
	byte buffer1[100], buffer2[100];

	convertMessageToString(_scriptPointer, buffer1, sizeof(buffer1));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	convertMessageToString(_scriptPointer, buffer2, sizeof(buffer2));
	len = resStrLen(_scriptPointer);
	_scriptPointer += len + 1;

	debug(1, "o60_rename (\"%s\" to \"%s\")", (char *)buffer1, (char *)buffer2);

	renameSaveFile(buffer1, buffer2);
}

int ScummEngine_v60he::readFileToArray(int slot, int32 size) {
	assert(_hInFileTable[slot]);
	if (size == 0)
		size = _hInFileTable[slot]->size() - _hInFileTable[slot]->pos();

	writeVar(0, 0);
	byte *data = defineArray(0, kByteArray, 0, size);
	_hInFileTable[slot]->read(data, size);

	return readVar(0);
}

void ScummEngine_v60he::o60_readFile() {
	int32 size = pop();
	int slot = pop();
	int val;

	// Fatty Bear uses positive values
	if (_game.platform == Common::kPlatformDOS && _game.id == GID_FBEAR)
		size = -size;

	assert(_hInFileTable[slot]);
	if (size == -2) {
		val = _hInFileTable[slot]->readUint16LE();
		push(val);
	} else if (size == -1) {
		val = _hInFileTable[slot]->readByte();
		push(val);
	} else {
		val = readFileToArray(slot, size);
		push(val);
	}
}

void ScummEngine_v60he::writeFileFromArray(int slot, int resID) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, resID);
	int32 size = FROM_LE_16(ah->dim1) * FROM_LE_16(ah->dim2);

	assert(_hOutFileTable[slot]);
	_hOutFileTable[slot]->write(ah->data, size);
}

void ScummEngine_v60he::o60_writeFile() {
	int32 size = pop();
	int16 resID = pop();
	int slot = pop();

	// Fatty Bear uses positive values
	if (_game.platform == Common::kPlatformDOS && _game.id == GID_FBEAR)
		size = -size;

	assert(_hOutFileTable[slot]);
	if (size == -2) {
		_hOutFileTable[slot]->writeUint16LE(resID);
	} else if (size == -1) {
		_hOutFileTable[slot]->writeByte(resID);
	} else {
		writeFileFromArray(slot, resID);
	}
}

void ScummEngine_v60he::o60_soundOps() {
	byte subOp = fetchScriptByte();
	int arg = pop();

	switch (subOp) {
	case SO_SOUND_START_VOLUME:
		if (_imuse) {
			_imuse->setMusicVolume(arg);
		}
		break;
	case SO_SOUND_VOLUME_RAMP:
		// No-op in the original as well.
		break;
	case SO_SOUND_FREQUENCY:
		// Fatty Bear's Birthday surprise uses this when playing the
		// piano, but only when using one of the digitized instruments.
		// See also o6_startSound().
		((SoundHE *)_sound)->setOverrideFreq(arg);
		break;
	default:
		error("o60_soundOps: default case 0x%x", subOp);
	}
}

void ScummEngine_v60he::localizeArray(int slot, byte scriptSlot) {
	if (_game.heversion >= 80)
		slot &= ~MAGIC_ARRAY_NUMBER;

	if (slot >= _numArray)
		error("o60_localizeArrayToScript(%d): array slot out of range", slot);

	_arraySlot[slot] = scriptSlot;
}

void ScummEngine_v60he::o60_localizeArrayToScript() {
	int slot = pop();
	localizeArray(slot, _currentScript);
}

void ScummEngine_v60he::o60_seekFilePos() {
	int mode, offset, slot;

	mode = pop();
	offset = pop();
	slot = pop();

	if (slot == -1)
		return;

	int whence;
	switch(mode) {
	case 1:
		whence = SEEK_SET;
		break;
	case 2:
		whence = SEEK_CUR;
		break;
	case 3:
		whence = SEEK_END;
		break;
	default:
		error("o60_seekFilePos: default case %d", mode);
	}

	if (_hInFileTable[slot])
		_hInFileTable[slot]->seek(offset, whence);
	else if (_hOutFileTable[slot])
		_hOutFileTable[slot]->seek(offset, whence);
	else
		error("o60_seekFilePos: file slot %d not loaded", slot);
}

void ScummEngine_v60he::o60_readFilePos() {
	int slot = pop();

	if (slot == -1) {
		push(0);
		return;
	}

	// The original uses standard file handles, and not
	// double in/out handles, so a script can open a file
	// (as out file) and call this function to perform a ftell().
	// This causes crashes in at least Backyard Basketball.
	// 
	// Let's try imitating that...
	if (_hInFileTable[slot]) {
		push(_hInFileTable[slot]->pos());
	} else if (_hOutFileTable[slot]) {
		push(_hOutFileTable[slot]->pos());
	} else {
		push(0);
	}
}

void ScummEngine_v60he::o60_redimArray() {
	int newX, newY;
	newY = pop();
	newX = pop();

	if (newY == 0)
		SWAP(newX, newY);

	byte subOp = fetchScriptByte();
	switch (subOp) {
	case SO_INT_ARRAY:
		redimArray(fetchScriptWord(), newX, newY, kIntArray);
		break;
	case SO_BYTE_ARRAY:
		redimArray(fetchScriptWord(), newX, newY, kByteArray);
		break;
	default:
		error("o60_redimArray: default type %d", subOp);
	}
}

void ScummEngine_v60he::redimArray(int arrayId, int newX, int newY, int type) {
	// Used in mini game at Cosmic Dust Diner in puttmoon
	int newSize, oldSize;

	if (readVar(arrayId) == 0)
		error("redimArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(arrayId));

	if (!ah)
		error("redimArray: Invalid array (%d) reference", readVar(arrayId));

	newSize = (type == kIntArray) ? 2 : 1;
	oldSize = (FROM_LE_16(ah->type) == kIntArray) ? 2 : 1;

	newSize *= (newX + 1) * (newY + 1);
	oldSize *= FROM_LE_16(ah->dim1) * FROM_LE_16(ah->dim2);

	if (newSize != oldSize)
		error("redimArray: array %d redim mismatch", readVar(arrayId));

	ah->type = TO_LE_16(type);
	ah->dim1 = TO_LE_16(newY + 1);
	ah->dim2 = TO_LE_16(newX + 1);
}

void ScummEngine_v60he::decodeParseString(int m, int n) {
	int i, colors;
	int args[31];

	byte b = fetchScriptByte();

	switch (b) {
	case SO_AT:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case SO_COLOR:
		_string[m].color = pop();
		break;
	case SO_CLIPPED:
		_string[m].right = pop();
		break;
	case SO_CENTER:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case SO_LEFT:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case SO_OVERHEAD:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case SO_MUMBLE:
		_string[m].no_talk_anim = true;
		break;
	case SO_TEXTSTRING:
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case SO_COLOR_LIST:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[m]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case SO_BASEOP:
		_string[m].loadDefault();
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case SO_END:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

void ScummEngine_v60he::o60_isSoundRunning() {
	int snd = pop();

	if (snd)
		snd = _sound->isSoundRunning(snd);

	push(snd);
}

} // End of namespace Scumm
