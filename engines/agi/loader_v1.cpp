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
 */

#include "agi/agi.h"
#include "common/md5.h"

#define SECTOR_OFFSET(s) ((s) * 512)

#define BASE_SECTOR	0x1C2

#define LOGDIR_SEC	SECTOR_OFFSET(171) + 5
#define LOGDIR_NUM	43

#define PICDIR_SEC	SECTOR_OFFSET(180) + 5
#define PICDIR_NUM	30

#define VIEWDIR_SEC	SECTOR_OFFSET(189) + 5
#define VIEWDIR_NUM	171

#define SNDDIR_SEC	SECTOR_OFFSET(198) + 5
#define SNDDIR_NUM	64

namespace Agi {


AgiLoader_v1::AgiLoader_v1(AgiEngine *vm) {
	_vm = vm;
	
	// Find filenames for the disk images
	Common::String md5Disk0, md5Disk1;
	getBooterMD5Sums((AgiGameID)_vm->getGameID(), md5Disk0, md5Disk1);
	diskImageExists(md5Disk0, _filenameDisk0);
	if (!md5Disk1.empty())
		diskImageExists(md5Disk1, _filenameDisk1);
}

int AgiLoader_v1::detectGame() {
	return _vm->setupV2Game(_vm->getVersion());
}

int AgiLoader_v1::loadDir(AgiDir *agid, int offset, int num) {
	Common::File fp;
	
	if (!fp.open(_filenameDisk0))
		return errBadFileOpen;

	// Cleanup
	for (int i = 0; i < MAX_DIRS; i++) {
		agid[i].volume = 0xFF;
		agid[i].offset = _EMPTY;
	}
	
	fp.seek(offset, SEEK_SET);
	for (int i = 0; i < num; i++) {
		int b0 = fp.readByte();
		int b1 = fp.readByte();
		int b2 = fp.readByte();
		
		if (b0 == 0xFF && b1 == 0xFF && b2 == 0xFF) {
			agid[i].volume = 0xFF;
			agid[i].offset = _EMPTY;
		} else {
			int sec = (BASE_SECTOR + (((b0 & 0xF) << 8) | b1)) >> 1;
			int off = ((b1 & 0x1) << 8) | b2;
			agid[i].volume = 0;
			agid[i].offset = SECTOR_OFFSET(sec) + off;
		}
	}

	fp.close();

	return errOK;
}

int AgiLoader_v1::init() {
	int ec = errOK;

	ec = loadDir(_vm->_game.dirLogic, LOGDIR_SEC, LOGDIR_NUM);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirPic, PICDIR_SEC, PICDIR_NUM);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirView, VIEWDIR_SEC, VIEWDIR_NUM);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirSound, SNDDIR_SEC, SNDDIR_NUM);

	return ec;
}

int AgiLoader_v1::deinit() {
	int ec = errOK;
	return ec;
}

uint8 *AgiLoader_v1::loadVolRes(struct AgiDir *agid) {
	uint8 *data = NULL;
	Common::File fp;

	if (agid->offset == _EMPTY)
		return NULL;
	
	fp.open(_filenameDisk0);
	fp.seek(agid->offset, SEEK_SET);

	int signature = fp.readUint16BE();
	if (signature != 0x1234) {
		warning("AgiLoader_v1::loadVolRes: bad signature %04x", signature);
		return NULL;
	}

	fp.readByte();
	agid->len = fp.readUint16LE();
	data = (uint8 *)calloc(1, agid->len + 32);
	fp.read(data, agid->len);
	
	fp.close();

	return data;
}

int AgiLoader_v1::loadResource(int t, int n) {
	int ec = errOK;
	uint8 *data = NULL;

	debugC(3, kDebugLevelResources, "(t = %d, n = %d)", t, n);
	if (n > MAX_DIRS)
		return errBadResource;

	switch (t) {
	case rLOGIC:
		if (~_vm->_game.dirLogic[n].flags & RES_LOADED) {
			debugC(3, kDebugLevelResources, "loading logic resource %d", n);
			unloadResource(rLOGIC, n);

			// load raw resource into data
			data = loadVolRes(&_vm->_game.dirLogic[n]);

			_vm->_game.logics[n].data = data;
			ec = data ? _vm->decodeLogic(n) : errBadResource;

			_vm->_game.logics[n].sIP = 2;
		}

		// if logic was cached, we get here
		// reset code pointers incase it was cached

		_vm->_game.logics[n].cIP = _vm->_game.logics[n].sIP;
		break;
	case rPICTURE:
		// if picture is currently NOT loaded *OR* cacheing is off,
		// unload the resource (caching == off) and reload it

		debugC(3, kDebugLevelResources, "loading picture resource %d", n);
		if (_vm->_game.dirPic[n].flags & RES_LOADED)
			break;

		// if loaded but not cached, unload it
		// if cached but not loaded, etc
		unloadResource(rPICTURE, n);
		data = loadVolRes(&_vm->_game.dirPic[n]);

		if (data != NULL) {
			_vm->_game.pictures[n].rdata = data;
			_vm->_game.dirPic[n].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case rSOUND:
		debugC(3, kDebugLevelResources, "loading sound resource %d", n);
		if (_vm->_game.dirSound[n].flags & RES_LOADED)
			break;

		data = loadVolRes(&_vm->_game.dirSound[n]);

		if (data != NULL) {
			// Freeing of the raw resource from memory is delegated to the createFromRawResource-function
			_vm->_game.sounds[n] = AgiSound::createFromRawResource(data, _vm->_game.dirSound[n].len, n, *_vm->_sound, _vm->_soundemu);
			_vm->_game.dirSound[n].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case rVIEW:
		// Load a VIEW resource into memory...
		// Since VIEWS alter the view table ALL the time
		// can we cache the view? or must we reload it all
		// the time?
		if (_vm->_game.dirView[n].flags & RES_LOADED)
			break;

		debugC(3, kDebugLevelResources, "loading view resource %d", n);
		unloadResource(rVIEW, n);
		data = loadVolRes(&_vm->_game.dirView[n]);
		if (data) {
			_vm->_game.views[n].rdata = data;
			_vm->_game.dirView[n].flags |= RES_LOADED;
			ec = _vm->decodeView(n);
		} else {
			ec = errBadResource;
		}
		break;
	default:
		ec = errBadResource;
		break;
	}

	return ec;
}

int AgiLoader_v1::unloadResource(int t, int n) {
	switch (t) {
	case rLOGIC:
		_vm->unloadLogic(n);
		break;
	case rPICTURE:
		_vm->_picture->unloadPicture(n);
		break;
	case rVIEW:
		_vm->unloadView(n);
		break;
	case rSOUND:
		_vm->_sound->unloadSound(n);
		break;
	}

	return errOK;
}

// TODO: Find the disk image equivalent.
int AgiLoader_v1::loadObjects(const char *fname) {
	return _vm->loadObjects(fname);
}

// TODO: Find the disk image equivalent.
int AgiLoader_v1::loadWords(const char *fname) {
	return _vm->loadWords(fname);
}

}
