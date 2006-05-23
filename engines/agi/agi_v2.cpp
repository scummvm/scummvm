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

static int agi_v2_init(void);
static int agi_v2_deinit(void);
static int agi_v2_detect_game();
static int agi_v2_load_resource(int, int);
static int agi_v2_unload_resource(int, int);
static int agi_v2_load_objects(char *);
static int agi_v2_load_words(char *);

struct agi_loader agi_v2 = {
	2,
	0,
	agi_v2_init,
	agi_v2_deinit,
	agi_v2_detect_game,
	agi_v2_load_resource,
	agi_v2_unload_resource,
	agi_v2_load_objects,
	agi_v2_load_words
};

static int agi_v2_detect_game() {
	if (!Common::File::exists(LOGDIR) ||
			!Common::File::exists(PICDIR) ||
			!Common::File::exists(SNDDIR) ||
			!Common::File::exists(VIEWDIR))
		return err_InvalidAGIFile;

	agi_v2.int_version = 0x2917;	/* setup for 2.917 */
	return v2id_game();
}

static int agi_v2_load_dir(struct agi_dir *agid, char *fname) {
	Common::File fp;
	uint8 *mem;
	uint32 flen;
	unsigned int i;
	char *path;

	path = fname;
	report("Loading directory: %s\n", path);

	if ((!fp.open(path))) {
		return err_BadFileOpen;
	}

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *) malloc(flen + 32)) == NULL) {
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

static int agi_v2_init() {
	int ec = err_OK;

	/* load directory files */
	ec = agi_v2_load_dir(game.dir_logic, LOGDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir(game.dir_pic, PICDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir(game.dir_view, VIEWDIR);
	if (ec == err_OK)
		ec = agi_v2_load_dir(game.dir_sound, SNDDIR);

	return ec;
}

static int agi_v2_deinit() {
	int ec = err_OK;

#if 0
	/* unload words */
	agi_v2_unload_words();

	/* unload objects */
	agi_v2_unload_objects();
#endif

	return ec;
}

static int agi_v2_unload_resource(int t, int n) {
	debugC(3, kDebugLevelResources, "unload resource");

	switch (t) {
	case rLOGIC:
		unload_logic(n);
		break;
	case rPICTURE:
		unload_picture(n);
		break;
	case rVIEW:
		unload_view(n);
		break;
	case rSOUND:
		unload_sound(n);
		break;
	}

	return err_OK;
}

/*
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */
static uint8 *agi_v2_load_vol_res(struct agi_dir *agid) {
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
int agi_v2_load_resource(int t, int n) {
	int ec = err_OK;
	uint8 *data = NULL;

	debugC(3, kDebugLevelResources, "(t = %d, n = %d)", t, n);
	if (n > MAX_DIRS)
		return err_BadResource;

	switch (t) {
	case rLOGIC:
		if (~game.dir_logic[n].flags & RES_LOADED) {
			debugC(3, kDebugLevelResources, "loading logic resource %d", n);
			agi_v2.unload_resource(rLOGIC, n);

			/* load raw resource into data */
			data = agi_v2_load_vol_res(&game.dir_logic[n]);

			game.logics[n].data = data;
			ec = data ? decode_logic(n) : err_BadResource;

			game.logics[n].sIP = 2;
		}

		/* if logic was cached, we get here */
		/* reset code pointers incase it was cached */

		game.logics[n].cIP = game.logics[n].sIP;
		break;
	case rPICTURE:
		/* if picture is currently NOT loaded *OR* cacheing is off,
		 * unload the resource (caching == off) and reload it
		 */

		debugC(3, kDebugLevelResources, "loading picture resource %d", n);
		if (game.dir_pic[n].flags & RES_LOADED)
			break;

		/* if loaded but not cached, unload it */
		/* if cached but not loaded, etc */
		agi_v2.unload_resource(rPICTURE, n);
		data = agi_v2_load_vol_res(&game.dir_pic[n]);

		if (data != NULL) {
			game.pictures[n].rdata = data;
			game.dir_pic[n].flags |= RES_LOADED;
		} else {
			ec = err_BadResource;
		}
		break;
	case rSOUND:
		debugC(3, kDebugLevelResources, "loading sound resource %d", n);
		if (game.dir_sound[n].flags & RES_LOADED)
			break;

		data = agi_v2_load_vol_res(&game.dir_sound[n]);

		if (data != NULL) {
			game.sounds[n].rdata = data;
			game.dir_sound[n].flags |= RES_LOADED;
			decode_sound(n);
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
		if (game.dir_view[n].flags & RES_LOADED)
			break;

		debugC(3, kDebugLevelResources, "loading view resource %d", n);
		agi_v2.unload_resource(rVIEW, n);
		data = agi_v2_load_vol_res(&game.dir_view[n]);
		if (data) {
			game.views[n].rdata = data;
			game.dir_view[n].flags |= RES_LOADED;
			ec = decode_view(n);
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

static int agi_v2_load_objects(char *fname) {
	return load_objects(fname);
}

static int agi_v2_load_words(char *fname) {
	return load_words(fname);
}

}                             // End of namespace Agi
