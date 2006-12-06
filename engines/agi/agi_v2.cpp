/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#include "common/stdafx.h"
#include "common/file.h"

#include "agi/agi.h"

namespace Agi {

int AgiLoader_v2::version() {
	return 2;
}

void AgiLoader_v2::setIntVersion(int version) {
	int_version = version;
}

int AgiLoader_v2::getIntVersion() {
	return int_version;
}

int AgiLoader_v2::detect_game() {
	if (!Common::File::exists(LOGDIR) ||
			!Common::File::exists(PICDIR) ||
			!Common::File::exists(SNDDIR) ||
			!Common::File::exists(VIEWDIR))
		return err_InvalidAGIFile;

	int_version = 0x2917;	/* setup for 2.917 */
	return _vm->v2id_game();
}

int AgiLoader_v2::load_dir(struct agi_dir *agid, const char *fname) {
	Common::File fp;
	uint8 *mem;
	uint32 flen;
	uint i;

	report("Loading directory: %s\n", fname);

	if (!fp.open(fname)) {
		return err_BadFileOpen;
	}

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)malloc(flen + 32)) == NULL) {
		fp.close();
		return err_NotEnoughMemory;
	}

	fp.read(mem, flen);

	/* set all directory resources to gone */
	for (i = 0; i < MAX_DIRS; i++) {
		agid[i].volume = 0xff;
		agid[i].offset = _EMPTY;
	}

	/* build directory entries */
	for (i = 0; i < flen; i += 3) {
		agid[i / 3].volume = *(mem + i) >> 4;
		agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		debugC(3, kDebugLevelResources, "%d: volume %d, offset 0x%05x", i / 3, agid[i / 3].volume, agid[i / 3].offset);
	}

	free(mem);
	fp.close();

	return err_OK;
}

int AgiLoader_v2::init() {
	int ec = err_OK;

	/* load directory files */
	ec = load_dir(_vm->game.dir_logic, LOGDIR);
	if (ec == err_OK)
		ec = load_dir(_vm->game.dir_pic, PICDIR);
	if (ec == err_OK)
		ec = load_dir(_vm->game.dir_view, VIEWDIR);
	if (ec == err_OK)
		ec = load_dir(_vm->game.dir_sound, SNDDIR);

	return ec;
}

int AgiLoader_v2::deinit() {
	int ec = err_OK;

#if 0
	/* unload words */
	agi_v2_unload_words();

	/* unload objects */
	agi_v2_unload_objects();
#endif

	return ec;
}

int AgiLoader_v2::unload_resource(int t, int n) {
	debugC(3, kDebugLevelResources, "unload resource");

	switch (t) {
	case rLOGIC:
		_vm->unload_logic(n);
		break;
	case rPICTURE:
		_vm->_picture->unload_picture(n);
		break;
	case rVIEW:
		_vm->unload_view(n);
		break;
	case rSOUND:
		_vm->_sound->unload_sound(n);
		break;
	}

	return err_OK;
}

/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */
uint8 *AgiLoader_v2::load_vol_res(struct agi_dir *agid) {
	uint8 *data = NULL;
	char x[MAX_PATH], *path;
	Common::File fp;
	unsigned int sig;

	sprintf(x, "vol.%i", agid->volume);
	path = x;
	debugC(3, kDebugLevelResources, "Vol res: path = %s", path);

	if (agid->offset != _EMPTY && fp.open(path)) {
		debugC(3, kDebugLevelResources, "loading resource at offset %d", agid->offset);
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&x, 5);
		if ((sig = READ_BE_UINT16((uint8 *) x)) == 0x1234) {
			agid->len = READ_LE_UINT16((uint8 *) x + 3);
			data = (uint8 *) calloc(1, agid->len + 32);
			if (data != NULL) {
				fp.read(data, agid->len);
			} else {
				abort();
			}
		} else {
#if 0
			/* FIXME: call some panic handler instead of
			 *        deiniting directly
			 */
			deinit_video_mode();
#endif
			report("Error: bad signature %04x\n", sig);
			// fprintf (stderr, "ACK! BAD RESOURCE!!!\n");
			return 0;
		}
		fp.close();
	} else {
		/* we have a bad volume resource */
		/* set that resource to NA */
		agid->offset = _EMPTY;
	}

	return data;
}

/*
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int AgiLoader_v2::load_resource(int t, int n) {
	int ec = err_OK;
	uint8 *data = NULL;

	debugC(3, kDebugLevelResources, "(t = %d, n = %d)", t, n);
	if (n > MAX_DIRS)
		return err_BadResource;

	switch (t) {
	case rLOGIC:
		if (~_vm->game.dir_logic[n].flags & RES_LOADED) {
			debugC(3, kDebugLevelResources, "loading logic resource %d", n);
			unload_resource(rLOGIC, n);

			/* load raw resource into data */
			data = load_vol_res(&_vm->game.dir_logic[n]);

			_vm->game.logics[n].data = data;
			ec = data ? _vm->decode_logic(n) : err_BadResource;

			_vm->game.logics[n].sIP = 2;
		}

		/* if logic was cached, we get here */
		/* reset code pointers incase it was cached */

		_vm->game.logics[n].cIP = _vm->game.logics[n].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching == off) and reload it
		 */

		debugC(3, kDebugLevelResources, "loading picture resource %d", n);
		if (_vm->game.dir_pic[n].flags & RES_LOADED)
			break;

		/* if loaded but not cached, unload it */
		/* if cached but not loaded, etc */
		unload_resource(rPICTURE, n);
		data = load_vol_res(&_vm->game.dir_pic[n]);

		if (data != NULL) {
			_vm->game.pictures[n].rdata = data;
			_vm->game.dir_pic[n].flags |= RES_LOADED;
		} else {
			ec = err_BadResource;
		}
		break;
	case rSOUND:
		debugC(3, kDebugLevelResources, "loading sound resource %d", n);
		if (_vm->game.dir_sound[n].flags & RES_LOADED)
			break;

		data = load_vol_res(&_vm->game.dir_sound[n]);

		if (data != NULL) {
			_vm->game.sounds[n].rdata = data;
			_vm->game.dir_sound[n].flags |= RES_LOADED;
			_vm->_sound->decode_sound(n);
		} else {
			ec = err_BadResource;
		}
		break;
	case rVIEW:
		/* Load a VIEW resource into memory...
		 * Since VIEWS alter the view table ALL the time
		 * can we cache the view? or must we reload it all
		 * the time?
		 */
		if (_vm->game.dir_view[n].flags & RES_LOADED)
			break;

		debugC(3, kDebugLevelResources, "loading view resource %d", n);
		unload_resource(rVIEW, n);
		data = load_vol_res(&_vm->game.dir_view[n]);
		if (data) {
			_vm->game.views[n].rdata = data;
			_vm->game.dir_view[n].flags |= RES_LOADED;
			ec = _vm->decode_view(n);
		} else {
			ec = err_BadResource;
		}
		break;
	default:
		ec = err_BadResource;
		break;
	}

	return ec;
}

int AgiLoader_v2::load_objects(const char *fname) {
	return _vm->load_objects(fname);
}

int AgiLoader_v2::load_words(const char *fname) {
	return _vm->load_words(fname);
}

}                             // End of namespace Agi
