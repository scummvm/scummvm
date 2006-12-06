/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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

#include "agi/agi.h"
#include "agi/lzw.h"

#include "common/config-manager.h"
#include "common/fs.h"

namespace Agi {

int AgiLoader_v3::version() {
	return 3;
}

void AgiLoader_v3::setIntVersion(int ver) {
	int_version = ver;
}

int AgiLoader_v3::getIntVersion() {
	return int_version;
}

int AgiLoader_v3::detect_game() {
	int ec = err_Unk;
	bool found = false;

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));

	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("AgiEngine: invalid game path '%s'",
		    dir.path().c_str());
		return err_InvalidAGIFile;
	}

	for (FSList::const_iterator file = fslist.begin();
	    file != fslist.end() && !found; ++file) {
		Common::String f = file->name();
		f.toLowercase();

		if (f.hasSuffix("vol.0")) {
			strncpy(_vm->game.name, f.c_str(), f.size() > 5 ? f.size() - 5 : f.size());
			debugC(3, kDebugLevelMain, "game.name = %s", _vm->game.name);
			int_version = 0x3149;	// setup for 3.002.149
			ec = _vm->v3id_game();

			found = true;
		}
	}

	if (!found) {
		debugC(3, kDebugLevelMain, "not found");
		ec = err_InvalidAGIFile;
	}

	return ec;
}

int AgiLoader_v3::load_dir(struct agi_dir *agid, Common::File *fp,
						   uint32 offs, uint32 len) {
	int ec = err_OK;
	uint8 *mem;
	unsigned int i;

	fp->seek(offs, SEEK_SET);
	if ((mem = (uint8 *) malloc(len + 32)) != NULL) {
		fp->read(mem, len);

		/* set all directory resources to gone */
		for (i = 0; i < MAX_DIRS; i++) {
			agid[i].volume = 0xff;
			agid[i].offset = _EMPTY;
		}

		/* build directory entries */
		for (i = 0; i < len; i += 3) {
			agid[i / 3].volume = *(mem + i) >> 4;
			agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		}

		free(mem);
	} else {
		ec = err_NotEnoughMemory;
	}

	return ec;
}

struct agi3vol {
	uint32 sddr;
	uint32 len;
};

int AgiLoader_v3::init() {
	int ec = err_OK;
	struct agi3vol agi_vol3[4];
	int i;
	uint16 xd[4];
	Common::File fp;
	Common::String path;

	path = Common::String(_vm->game.name) + DIR_;

	if (!fp.open(path)) {
		printf("Failed to open \"%s\"\n", path.c_str());
		return err_BadFileOpen;
	}
	/* build offset table for v3 directory format */
	fp.read(&xd, 8);
	fp.seek(0, SEEK_END);

	for (i = 0; i < 4; i++)
		agi_vol3[i].sddr = READ_LE_UINT16((uint8 *) & xd[i]);

	agi_vol3[0].len = agi_vol3[1].sddr - agi_vol3[0].sddr;
	agi_vol3[1].len = agi_vol3[2].sddr - agi_vol3[1].sddr;
	agi_vol3[2].len = agi_vol3[3].sddr - agi_vol3[2].sddr;
	agi_vol3[3].len = fp.pos() - agi_vol3[3].sddr;

	if (agi_vol3[3].len > 256 * 3)
		agi_vol3[3].len = 256 * 3;

	fp.seek(0, SEEK_SET);

	/* read in directory files */
	ec = load_dir(_vm->game.dir_logic, &fp, agi_vol3[0].sddr,
	    agi_vol3[0].len);

	if (ec == err_OK) {
		ec = load_dir(_vm->game.dir_pic, &fp, agi_vol3[1].sddr, agi_vol3[1].len);
	}

	if (ec == err_OK) {
		ec = load_dir(_vm->game.dir_view, &fp, agi_vol3[2].sddr, agi_vol3[2].len);
	}

	if (ec == err_OK) {
		ec = load_dir(_vm->game.dir_sound, &fp, agi_vol3[3].sddr, agi_vol3[3].len);
	}

	return ec;
}

int AgiLoader_v3::deinit() {
	int ec = err_OK;

#if 0
	/* unload words */
	agi_v3_unload_words();

	/* unload objects */
	agi_v3_unload_objects();
#endif

	return ec;
}

int AgiLoader_v3::unload_resource(int t, int n) {
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
 * This function does noting but load a raw resource into memory.
 * If further decoding is required, it must be done by another
 * routine.
 *
 * NULL is returned if unsucsessful.
 */
uint8 *AgiLoader_v3::load_vol_res(struct agi_dir *agid) {
	char x[MAX_PATH];
	uint8 *data = NULL, *comp_buffer;
	Common::File fp;
	Common::String path;

	debugC(3, kDebugLevelResources, "(%p)", (void *)agid);
	sprintf(x, "vol.%i", agid->volume);
	path = Common::String(_vm->game.name) + x;

	if (agid->offset != _EMPTY && fp.open(path)) {
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&x, 7);

		if (READ_BE_UINT16((uint8 *) x) != 0x1234) {
#if 0
			/* FIXME */
			deinit_video_mode();
#endif
			debugC(3, kDebugLevelResources, "path = %s", path.c_str());
			debugC(3, kDebugLevelResources, "offset = %d", agid->offset);
			debugC(3, kDebugLevelResources, "x = %x %x", x[0], x[1]);
			error("ACK! BAD RESOURCE");

			g_system->quit();
		}

		agid->len = READ_LE_UINT16((uint8 *) x + 3);	/* uncompressed size */
		agid->clen = READ_LE_UINT16((uint8 *) x + 5);	/* compressed len */

		comp_buffer = (uint8 *)calloc(1, agid->clen + 32);
		fp.read(comp_buffer, agid->clen);

		if (x[2] & 0x80 || agid->len == agid->clen) {
			/* do not decompress */
			data = comp_buffer;

#if 0
			/* CM: added to avoid problems in
			 *     convert_v2_v3_pic() when clen > len
			 *     e.g. Sierra demo 4, first picture
			 *     (Tue Mar 16 13:13:43 EST 1999)
			 */
			agid->len = agid->clen;

			/* Now removed to fix Gold Rush! in demo4 */
#endif
		} else {
			/* it is compressed */
			data = (uint8 *)calloc(1, agid->len + 32);
			LZW_expand(comp_buffer, data, agid->len);
			free(comp_buffer);
			agid->flags |= RES_COMPRESSED;
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
int AgiLoader_v3::load_resource(int t, int n) {
	int ec = err_OK;
	uint8 *data = NULL;

	if (n > MAX_DIRS)
		return err_BadResource;

	switch (t) {
	case rLOGIC:
		/* load resource into memory, decrypt messages at the end
		 * and build the message list (if logic is in memory)
		 */
		if (~_vm->game.dir_logic[n].flags & RES_LOADED) {
			/* if logic is already in memory, unload it */
			unload_resource(rLOGIC, n);

			/* load raw resource into data */
			data = load_vol_res(&_vm->game.dir_logic[n]);
			_vm->game.logics[n].data = data;

			/* uncompressed logic files need to be decrypted */
			if (data != NULL) {
				/* resloaded flag gets set by decode logic */
				/* needed to build string table */
				ec = _vm->decode_logic(n);
				_vm->game.logics[n].sIP = 2;
			} else {
				ec = err_BadResource;
			}

			/*logics[n].sIP=2; *//* saved IP = 2 */
			/*logics[n].cIP=2; *//* current IP = 2 */

			_vm->game.logics[n].cIP = _vm->game.logics[n].sIP;
		}

		/* if logic was cached, we get here */
		/* reset code pointers incase it was cached */

		_vm->game.logics[n].cIP = _vm->game.logics[n].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching==off) and reload it
		 */
		if (~_vm->game.dir_pic[n].flags & RES_LOADED) {
			unload_resource(rPICTURE, n);
			data = load_vol_res(&_vm->game.dir_pic[n]);
			if (data != NULL) {
				data = _vm->_picture->convert_v3_pic(data, _vm->game.dir_pic[n].len);
				_vm->game.pictures[n].rdata = data;
				_vm->game.dir_pic[n].flags |= RES_LOADED;
			} else {
				ec = err_BadResource;
			}
		}
		break;
	case rSOUND:
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
		 * Since VIEWS alter the view table ALL the time can we
		 * cache the view? or must we reload it all the time?
		 */
		/* load a raw view from a VOL file into data */
		if (_vm->game.dir_view[n].flags & RES_LOADED)
			break;

		unload_resource(rVIEW, n);
		data = load_vol_res(&_vm->game.dir_view[n]);
		if (data != NULL) {
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

int AgiLoader_v3::load_objects(const char *fname) {
	return _vm->load_objects(fname);
}

int AgiLoader_v3::load_words(const char *fname) {
	return _vm->load_words(fname);
}

}                             // End of namespace Agi
