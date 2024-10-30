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

#include "common/system.h"
#include "common/savefile.h"
#include "common/util.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/adv_chk.h"
#include "m4/adv_r/adv_walk.h"
#include "m4/adv_r/db_env.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/fileio/extensions.h"
#include "m4/fileio/info.h"
#include "m4/graphics/gr_pal.h"
#include "m4/gui/gui_buffer.h"
#include "m4/gui/gui_vmng.h"
#include "m4/platform/tile/tile_read.h"
#include "m4/m4.h"

namespace M4 {

static Common::String get_background_filename(const SceneDef *rdef);
static Common::String get_attribute_filename(const SceneDef *rdef);
static void recreate_animation_draw_screen(GrBuff **loadBuf);
static void troll_for_colors(RGB8 *newPal, uint8 minPalEntry, uint8 maxPalEntry);

void kernel_unload_room(SceneDef *rdef, GrBuff **code_data, GrBuff **loadBuffer) {
	term_message("Unloading scene %d", _G(game).room_id);

	if (_G(gameDrawBuff)) {
		gui_buffer_deregister((Buffer *)_G(gameDrawBuff));
		delete _G(gameDrawBuff);
		_G(gameDrawBuff) = nullptr;
	}

	if (*code_data)
		delete *code_data;
	*code_data = nullptr;

	if (*loadBuffer)
		delete *loadBuffer;
	*loadBuffer = nullptr;

	if (!rdef)
		return;

	// Must we deallocate existing hot spots?
	if (rdef->hotspots != nullptr) {
		hotspot_delete_all(rdef->hotspots);
		rdef->hotspots = nullptr;
	}
	rdef->num_hotspots = 0;

	// Must we deallocate existing parallax?
	if (rdef->parallax != nullptr) {
		hotspot_delete_all(rdef->parallax);
		rdef->parallax = nullptr;
	}
	rdef->num_parallax = 0;

	// Must we deallocate existing props?
	if (rdef->props != nullptr) {
		hotspot_delete_all(rdef->props);
		rdef->props = nullptr;
	}
	rdef->num_props = 0;

	ClearRails();
}


bool kernel_load_room(int minPalEntry, int maxPalEntry, SceneDef *rdef, GrBuff **scr_orig_data, GrBuff **scr_orig) {
	char *tempName;

	if (!scr_orig_data || !scr_orig) {
		error_show(FL, 'BUF!', "load_picture_and_codes");
		return false;
	}

	term_message("Reading scene %d", _G(game).new_room);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_G(game).previous_room = _G(game).room_id;
	}

	// Read DEF file
	if (db_def_chk_read(_G(game).new_room, rdef) != -1) {
		error_show(FL, 'DF:(', "trying to find %d.CHK", (uint32)_G(game).new_room);
		return false;
	}

	set_walker_scaling(rdef);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Select background picture

	_G(currBackgroundFN) = get_background_filename(rdef);

	tempName = env_find(_G(currBackgroundFN));
	if (tempName) {
		// In normal rooms.db mode
		_G(currBackgroundFN) = f_extension_new(tempName, "TT");
	} else {
		// In concat hag mode
		_G(currBackgroundFN) = f_extension_new(_G(currBackgroundFN), "TT");
	}

	SysFile *pic_file = new SysFile(_G(currBackgroundFN), BINARY);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Select attributes code file

	_G(currCodeFN) = get_attribute_filename(rdef);

	tempName = env_find(_G(currCodeFN));
	if (tempName) {
		// In normal rooms.db mode
		_G(currCodeFN) = f_extension_new(tempName, "COD");
	} else {
		// In concat hag mode
		_G(currCodeFN) = f_extension_new(_G(currCodeFN), "COD");
	}

	SysFile *code_file = new SysFile(_G(currCodeFN), BINARY);
	if (!code_file->exists()) {
		delete code_file;
		code_file = nullptr;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Load background picture

	term_message("load background");
	RGB8 newPal[256];
	load_background(pic_file, scr_orig, newPal);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Load attributes code file

	term_message("load codes");
	*scr_orig_data = load_codes(code_file);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Prepare buffers for display

	recreate_animation_draw_screen(scr_orig);
	troll_for_colors(newPal, minPalEntry, maxPalEntry);

	gr_pal_reset_ega_colors(&_G(master_palette)[0]);

	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);

	if (pic_file) {
		pic_file->close();
		delete pic_file;
	}
	if (code_file) {
		code_file->close();
		delete code_file;
	}

	if (*scr_orig_data) {
		Buffer *scr_orig_data_buffer = (**scr_orig_data).get_buffer();
		RestoreEdgeList(scr_orig_data_buffer);
		(**scr_orig_data).release();
	} else
		RestoreEdgeList(nullptr);

	_G(game).room_id = _G(game).new_room;
	return true;
}

bool kernel_load_variant(const char *variant) {
	auto &sceneDef = _G(currentSceneDef);
	auto *codeBuff = _G(screenCodeBuff);
	Common::String filename;

	if (!codeBuff)
		return false;

	if (_G(kernel).hag_mode) {
		filename = f_extension_new(variant, "COD");
	} else {
		char lastChar = variant[strlen(variant) - 1];

		char *base = env_find(sceneDef.art_base);
		char *dotPos = strchr(base, '.');
		if (!dotPos)
			return false;

		*dotPos++ = lastChar;
		*dotPos++ = '.';
		filename = f_extension_new(base, "COD");

		if (!f_info_exists(Common::Path(filename)))
			return false;
	}

	SysFile code_file(filename);
	if (!code_file.exists())
		error("Failed to load variant %s", filename.c_str());

	// TODO: This is just copied from the room loading code,
	// rather than disassembling the reset of the original method.
	// Need to determine whether this is correct or not
	GrBuff *scr_orig_data = load_codes(&code_file);

	code_file.close();

	if (scr_orig_data) {
		Buffer *scr_orig_data_buffer = scr_orig_data->get_buffer();
		RestoreEdgeList(scr_orig_data_buffer);
		scr_orig_data->release();
	}

	return true;
}

GrBuff *load_codes(SysFile *code_file) {
	// No this is not a cheat to allow bugs to live,
	// if there is no code file, then we don't need a code buffer, either.
	// it's perfectly acceptable, so there, NYAH!
	if (!code_file)			
		return nullptr;

	int16 x_size = code_file->readSint16LE();
	int16 y_size = code_file->readSint16LE();

	GrBuff *temp = new GrBuff(x_size, y_size);
	if (!temp) {
		error_show(FL, 'OOM!', "load_codes: %d bytes", (int16)(x_size * y_size));
		return nullptr;
	}

	Buffer *mybuff = temp->get_buffer();
	byte *bufferHandle = mybuff->data;

	for (int i = 0; i < y_size; i++) {
		code_file->read(bufferHandle, x_size);
		bufferHandle += mybuff->stride;
	}

	// Let the memory float
	temp->release();
	return temp;
}

bool load_background(SysFile *pic_file, GrBuff **loadBuffer, RGB8 *palette) {
	int32 num_x_tiles, num_y_tiles, tile_x, tile_y, file_x, file_y, x_end, y_end;
	int i, j;
	int32 count = 0;
	Buffer *out;

	tt_read_header(pic_file, &file_x, &file_y,
		&num_x_tiles, &num_y_tiles, &tile_x, &tile_y, palette);

	*loadBuffer = new GrBuff(file_x, file_y);

	if (!*loadBuffer)
		error_show(FL, 'OOM!');

	Buffer *theBuff = (**loadBuffer).get_buffer();

	for (i = 0; i < num_y_tiles; i++) {
		for (j = 0; j < num_x_tiles; j++) {
			out = tt_read(pic_file, count, tile_x, tile_y);
			count++;

			if (out && (out->data)) {
				x_end = imath_min(file_x, (1 + j) * tile_x);
				y_end = imath_min(file_y, (1 + i) * tile_y);
				gr_buffer_rect_copy_2(out, theBuff, 0, 0, j * tile_x, i * tile_y,
					x_end - (j * tile_x), y_end - (i * tile_y));
				mem_free(out->data);
			}

			if (out)
				mem_free(out);
		}
	}

	(**loadBuffer).release();
	return true;
}

static Common::SeekableReadStream *openForLoading(int slot) {
	Common::String slotName = g_engine->getSaveStateName(slot);
	return g_system->getSavefileManager()->openForLoading(slotName);
}

bool kernel_save_game_exists(int32 slot) {
	Common::SeekableReadStream *save = openForLoading(slot);
	bool result = save != nullptr;
	delete save;

	return result;
}

int kernel_save_game(int slot, const char *desc, int32 sizeofDesc, M4sprite *thumbNail, int32 sizeofThumbData) {
	return g_engine->saveGameState(slot, desc, slot == 0).getCode() == Common::kNoError ? 0 : 1;
}

bool kernel_load_game(int slot) {
	return g_engine->loadGameStateDoIt(slot).getCode() == Common::kNoError;
}

int32 extract_room_num(const Common::String &name) {
	if ((name[0] == 'C' || name[0] == 'c') &&
		(name[1] == 'O' || name[1] == 'o') &&
		(name[2] == 'M' || name[2] == 'm'))
		return _G(global_sound_room);

	if (Common::isDigit(name[0]) && Common::isDigit(name[1]) && Common::isDigit(name[2])) {
		return ((int32)(name[0] - '0')) * 100 + ((int32)(name[1] - '0')) * 10 + ((int32)(name[2] - '0'));
	} else
		return _G(game).room_id;
}

static Common::String get_background_filename(const SceneDef *rdef) {
	if (_G(art_base_override) != nullptr) {
		return _G(art_base_override);
	} else {
		return rdef->art_base;
	}
}

static Common::String get_attribute_filename(const SceneDef *rdef) {
	if (_G(art_base_override) == nullptr || !_G(use_alternate_attribute_file)) {
		return rdef->art_base;
	} else {
		return _G(art_base_override);
	}
}

static void recreate_animation_draw_screen(GrBuff **loadBuf) {
	// Remove previous animation draw screen
	if (_G(gameDrawBuff)) {
		gui_buffer_deregister((Buffer *)_G(gameDrawBuff));
		delete _G(gameDrawBuff);
		_G(gameDrawBuff) = nullptr;
		_G(game_buff_ptr) = nullptr;
	}
	_G(gameDrawBuff) = new GrBuff((**loadBuf).w, (**loadBuf).h);
	if (!_G(gameDrawBuff)) error_show(FL, 'OOM!', "no memory for GrBuff");
	gui_GrBuff_register(_G(kernel).letter_box_x, _G(kernel).letter_box_y, _G(gameDrawBuff),
		SF_BACKGRND | SF_GET_ALL | SF_BLOCK_NONE, nullptr);
	gui_buffer_activate((Buffer *)_G(gameDrawBuff));
	vmng_screen_to_back((void *)_G(gameDrawBuff));
	_G(game_buff_ptr) = vmng_screen_find(_G(gameDrawBuff), nullptr);

	Buffer *theBuff = (**loadBuf).get_buffer();
	Buffer *game_buff = (*_G(gameDrawBuff)).get_buffer();
	gr_buffer_rect_copy_2(theBuff, game_buff, 0, 0, 0, 0,
		imath_min((**loadBuf).w, game_buff->w), imath_min((**loadBuf).h, game_buff->h));

	(**loadBuf).release();
	(*_G(gameDrawBuff)).release();
}

static void troll_for_colors(RGB8 *newPal, uint8 minPalEntry, uint8 maxPalEntry) {
	bool gotOne = false;
	int16 pal_iter;
	for (pal_iter = minPalEntry; pal_iter <= maxPalEntry; pal_iter++)	// accept any colours that came with the background
		if (gotOne || (newPal[pal_iter].r | newPal[pal_iter].g | newPal[pal_iter].b))
		{
			gotOne = true;
			// colors are 6 bit...
			_G(master_palette)[pal_iter].r = newPal[pal_iter].r << 2;
			_G(master_palette)[pal_iter].g = newPal[pal_iter].g << 2;
			_G(master_palette)[pal_iter].b = newPal[pal_iter].b << 2;
		}
	if (gotOne) {
		gr_pal_interface(&_G(master_palette)[0]); // enforce interface colours
	}
}

Common::String expand_name_2_RAW(const Common::String &name, int32 room_num) {
	Common::String tempName = f_extension_new(name, "RAW");

	if (!_G(kernel).hag_mode) {
		if (room_num == -1)
			room_num = extract_room_num(name);

		return Common::String::format("%d\\%s", room_num, tempName.c_str());

	} else {
		return tempName;
	}
}

} // End of namespace M4
