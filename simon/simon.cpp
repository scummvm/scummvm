/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"
#include "sound/mididrv.h"
#include "common/config-file.h"
#include "common/file.h"
#include "common/gameDetector.h"
#include <errno.h>
#include <time.h>

extern uint16 _debugLevel;

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const VersionSettings simon_settings[] = {
	// Simon the Sorcerer 1 & 2 (not SCUMM games)
	{"simon1dos", "Simon the Sorcerer 1 (DOS)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1DOS, "GAMEPC"},
	{"simon1amiga", "Simon the Sorcerer 1 (Amiga)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1AMIGA, "gameamiga"},
	{"simon2dos", "Simon the Sorcerer 2 (DOS)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON2DOS, "GAME32"},
	{"simon1talkie", "Simon the Sorcerer 1 Talkie (DOS)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1TALKIE, "SIMON.GME"},
	{"simon2talkie", "Simon the Sorcerer 2 Talkie (DOS)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON2TALKIE, "GSPTR30"},
	{"simon1win", "Simon the Sorcerer 1 Talkie (Windows)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1WIN, "SIMON.GME"},
	{"simon1cd32", "Simon the Sorcerer 1 Talkie (Amiga CD32)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1CD32, "gameamiga"},
	{"simon2win", "Simon the Sorcerer 2 Talkie (Windows)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON2WIN, "GSPTR30"},
	{"simon2mac", "Simon the Sorcerer 2 Talkie (Amiga or Mac)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON2MAC, "GSPTR30"},
	{"simon1demo", "Simon the Sorcerer 1 (DOS Demo)", GID_SIMON_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, GAME_SIMON1DEMO, "GDEMO"}, 

	{NULL, NULL, 0, 0, VersionSettings::ADLIB_DONT_CARE, 0, NULL}
};

const VersionSettings *Engine_SIMON_targetList() {
	return simon_settings;
}

static const GameSpecificSettings simon1_settings = {
	1,										// VGA_DELAY_BASE
	1576 / 4,									// TABLE_INDEX_BASE
	1460 / 4,									// TEXT_INDEX_BASE
	64,										// NUM_VIDEO_OP_CODES
	1000000,									// VGA_MEM_SIZE
	50000,										// TABLES_MEM_SIZE
	1316 / 4,									// MUSIC_INDEX_BASE
	0,										// SOUND_INDEX_BASE
	"SIMON.GME",									// gme_filename
	"SIMON.WAV",									// wav_filename
	"SIMON.VOC",									// voc_filename
	"SIMON.MP3",									// mp3_filename
	"EFFECTS.VOC",									// voc_effects_filename
	"EFFECTS.MP3",									// mp3_effects_filename
	"GAMEPC",									// gamepc_filename
};

static const GameSpecificSettings simon1amiga_settings = {
	1,										// VGA_DELAY_BASE
	1576 / 4,									// TABLE_INDEX_BASE
	1460 / 4,									// TEXT_INDEX_BASE
	64,										// NUM_VIDEO_OP_CODES
	1000000,									// VGA_MEM_SIZE
	50000,										// TABLES_MEM_SIZE
	1316 / 4,									// MUSIC_INDEX_BASE
	0,										// SOUND_INDEX_BASE
	"",										// gme_filename
	"",										// wav_filename
	"",										// voc_filename
	"SIMON.MP3",									// mp3_filename
	"",										// voc_effects_filename
	"",										// mp3_effects_filename
	"gameamiga",									// gamepc_filename
};

static const GameSpecificSettings simon1demo_settings = {
	1,										// VGA_DELAY_BASE
	1576 / 4,									// TABLE_INDEX_BASE
	1460 / 4,									// TEXT_INDEX_BASE
	64,										// NUM_VIDEO_OP_CODES
	1000000,									// VGA_MEM_SIZE
	50000,										// TABLES_MEM_SIZE
	1316 / 4,									// MUSIC_INDEX_BASE
	0,										// SOUND_INDEX_BASE
	"",										// gme_filename
	"",										// wav_filename
	"",										// voc_filename
	"",										// mp3_filename
	"",										// voc_effects_filename
	"",										// mp3_effects_filename
	"GDEMO",									// gamepc_filename
};

static const GameSpecificSettings simon2win_settings = {
	5,										// VGA_DELAY_BASE
	1580 / 4,									// TABLE_INDEX_BASE
	1500 / 4,									// TEXT_INDEX_BASE
	75,										// NUM_VIDEO_OP_CODES
	2000000,									// VGA_MEM_SIZE
	100000,										// TABLES_MEM_SIZE
	1128 / 4,									// MUSIC_INDEX_BASE
	1660 / 4,									// SOUND_INDEX_BASE
	"SIMON2.GME",									// gme_filename
	"SIMON2.WAV",									// wav_filename
	"SIMON2.VOC",									// voc_filename
	"SIMON2.MP3",									// mp3_filename
	"",										// voc_effects_filename
	"",										// mp3_effects_filename
	"GSPTR30",									// gamepc_filename
};

static const GameSpecificSettings simon2mac_settings = {
	5,										// VGA_DELAY_BASE
	1580 / 4,									// TABLE_INDEX_BASE
	1500 / 4,									// TEXT_INDEX_BASE
	75,										// NUM_VIDEO_OP_CODES
	2000000,									// VGA_MEM_SIZE
	100000,										// TABLES_MEM_SIZE
	1128 / 4,									// MUSIC_INDEX_BASE
	1660 / 4,									// SOUND_INDEX_BASE
	"Simon2.gme",									// gme_filename
	"",										// wav_filename
	"",										// voc_filename
	"SIMON2.MP3",									// mp3_filename
	"",										// voc_effects_filename
	"",										// mp3_effects_filename
	"gsptr30",									// gamepc_filename
};

static const GameSpecificSettings simon2dos_settings = {
	5,										// VGA_DELAY_BASE
	1580 / 4,									// TABLE_INDEX_BASE
	1500 / 4,									// TEXT_INDEX_BASE
	75,										// NUM_VIDEO_OP_CODES
	2000000,									// VGA_MEM_SIZE
	100000,										// TABLES_MEM_SIZE
	1128 / 4,									// MUSIC_INDEX_BASE
	1660 / 4,									// SOUND_INDEX_BASE
	"SIMON2.GME",									// gme_filename
	"",										// wav_filename
	"",										// voc_filename
	"",										// mp3_filename
	"",										// voc_effects_filename
	"",										// mp3_effects_filename
	"GAME32",									// gamepc_filename
};


Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst) {
	return new SimonEngine(detector, syst);
}

SimonEngine::SimonEngine(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst), midi (syst) {
	OSystem::Property prop;

	MidiDriver *driver = detector->createMidi();
	
	_vc_ptr = 0;
	_game_offsets_ptr = 0;
	
	_game = (byte)detector->_game.features;

	if (_game == GAME_SIMON2MAC) {
		gss = &simon2mac_settings;
	} else if (_game == GAME_SIMON2TALKIE || _game == GAME_SIMON2WIN) {
		gss = &simon2win_settings;
	} else if (_game == GAME_SIMON2DOS) {
		gss = &simon2dos_settings;
	} else if (_game & GF_AMIGA) {
		gss = &simon1amiga_settings;
	} else if (_game == GAME_SIMON1DEMO) {
		gss = &simon1demo_settings;
	} else {
		gss = &simon1_settings;
	}

	_key_pressed = 0;

	_game_file = 0;
	
	_stripped_txt_mem = 0;
	_text_size = 0;
	_stringtab_num = 0;
	_stringtab_pos = 0;
	_stringtab_numalloc = 0;
	_stringtab_ptr = 0;

	_itemarray_ptr = 0;
	_itemarray_size = 0;
	_itemarray_inited = 0;

	_itemheap_ptr = 0;
	_itemheap_curpos = 0;
	_itemheap_size = 0;

	_icon_file_ptr = 0;

	_tbl_list = 0;

	_code_ptr = 0;


	_local_stringtable = 0;
	_string_id_local_min = 0;
	_string_id_local_max = 0;

	_tablesheap_ptr = 0;
	_tablesheap_ptr_org = 0;
	_tablesheap_ptr_new = 0;
	_tablesheap_size = 0;
	_tablesheap_curpos = 0;
	_tablesheap_curpos_org = 0;
	_tablesheap_curpos_new = 0;

	_subroutine_list = 0;
	_subroutine_list_org = 0;

	_dx_surface_pitch = 0;

	_recursion_depth = 0;

	_last_vga_tick = 0;

	_op_189_flags = 0;

	_scriptvar_2 = 0;
	_run_script_return_1 = 0;
	_skip_vga_wait = 0;
	_no_parent_notify = 0;
	_vga_res_328_loaded = 0;
	_hitarea_unk_3 = 0;
	_mortal_flag = 0;
	_sync_flag_1 = 0;
	_video_var_8 = 0;
	_use_palette_delay = 0;
	_sync_flag_2 = 0;
	_hitarea_unk_6 = 0;
	_in_callback = 0;
	_cepe_flag = 0;
	_copy_partial_mode = 0;
	_fast_mode = 0;
	_dx_use_3_or_4_for_lock = 0;

	_debugMode = 0;
	_debugLevel = 0;
	_language = 0;
	_start_mainscript = 0;
	_continous_mainscript = 0;
	_continous_vgascript = 0;
	_draw_images_debug = 0;
	_subtitles = true;
	_mouse_cursor = 0;
	_vga_var9 = 0;
	_script_unk_1 = 0;
	_vga_var6 = 0;
	_x_scroll = 0;
	_vga_var1 = 0;
	_vga_var2 = 0;
	_vga_var3 = 0;
	_vga_var5 = 0;
	_vga_var7 = 0;
	_vga_var8 = 0;

	_script_cond_a = 0;
	_script_cond_b = 0;
	_script_cond_c = 0;

	_fcs_unk_1 = 0;
	_fcs_ptr_1 = 0;

	_subject_item = 0;
	_object_item = 0;
	_item_1 = 0;

	_hitarea_object_item = 0;
	_last_hitarea = 0;
	_last_hitarea_2_ptr = 0;
	_last_hitarea_3 = 0;
	_left_button_down = 0;
	_hitarea_subject_item = 0;
	_hitarea_ptr_5 = 0;
	_hitarea_ptr_7 = 0;
	_need_hitarea_recalc = 0;
	_verb_hitarea = 0;
	_hitarea_unk_4 = 0;
	_lock_counter = 0;

	_video_palette_mode = 0;

	_print_char_unk_1 = 0;
	_print_char_unk_2 = 0;
	_num_letters_to_print = 0;

	_last_time = 0;

	_first_time_struct = 0;
	_pending_delete_time_event = 0;
	
	_base_time = 0;

	_mouse_x = 0;
	_mouse_y = 0;
	_mouse_x_old = 0;
	_mouse_y_old = 0;
	
	_dummy_item_1 = new Item();
	_dummy_item_2 = new Item();
	_dummy_item_3 = new Item();

	_lock_word = 0;
	_scroll_up_hit_area = 0;
	_scroll_down_hit_area = 0;

	_video_var_7 = 0;
	_palette_color_count = 0;

	_video_var_4 = 0;
	_video_var_5 = 0;
	_video_var_3 = 0;
	_unk_pal_flag = 0;
	_exit_cutscene = 0;
	_skip_speech = 0;
	_video_var_9 = 0;

	_sound_file_id = 0;
	_last_music_played = -1;
	_next_music_to_play = -1;

	_show_preposition = 0;
	_showmessage_flag = 0;

	_video_num_pal_colors = 0;

	_invoke_timer_callback = 0;

	_vga_sprite_changed = 0;

	_vga_buf_free_start = 0;
	_vga_buf_end = 0;
	_vga_buf_start = 0;
	_vga_file_buf_org = 0;
	_vga_file_buf_org_2 = 0;

	_cur_vga_file_1 = 0;
	_cur_vga_file_2 = 0;

	_timer_1 = 0;
	_timer_5 = 0;
	_timer_4 = 0;

	_vga_base_delay = 0;

	_vga_cur_file_2 = 0;
	_vga_wait_for = 0;
	_vga_cur_file_id = 0;
	_vga_cur_sprite_id = 0;

	_next_vga_timer_to_process = 0;

	memset(_vc_item_array, 0, sizeof(_vc_item_array));
	memset(_item_array_6, 0, sizeof(_item_array_6));

	memset(_stringid_array_2, 0, sizeof(_stringid_array_2));
	memset(_stringid_array_3, 0, sizeof(_stringid_array_3));
	memset(_array_4, 0, sizeof(_array_4));

	memset(_bit_array, 0, sizeof(_bit_array));
	memset(_variableArray, 0, sizeof(_variableArray));

	memset(_fcs_ptr_array_3, 0, sizeof(_fcs_ptr_array_3));

	memset(_fcs_data_1, 0, sizeof(_fcs_data_1));
	memset(_fcs_data_2, 0, sizeof(_fcs_data_2));

	_free_string_slot = 0;
	
	memset(_stringReturnBuffer, 0, sizeof(_stringReturnBuffer));

	memset(_pathfind_array, 0, sizeof(_pathfind_array));

	memset(_palette_backup, 0, sizeof(_palette_backup));
	memset(_palette, 0, sizeof(_palette));

	memset(_video_buf_1, 0, sizeof(_video_buf_1));

	_fcs_list = new FillOrCopyStruct[16];

	memset(_letters_to_print_buf, 0, sizeof(_letters_to_print_buf));

	_num_screen_updates = 0;
	_vga_tick_counter = 0;

	_sound = 0;

	_effects_paused = false;
	_ambient_paused = false;
	_music_paused = false;

	_timer_id = 0;

	_dump_file = 0;

	_saveload_row_curpos = 0;
	_num_savegame_rows = 0;
	_savedialog_flag = false;
	_save_or_load = false;
	_saveload_flag = false;

	_sdl_mouse_x = 0;
	_sdl_mouse_y = 0;
	
	_sdl_buf_3 = 0;
	_sdl_buf = 0;
	_sdl_buf_attached = 0;

	_vc_10_base_ptr_old = 0;
	memcpy (_hebrew_char_widths,
		"\x5\x5\x4\x6\x5\x3\x4\x5\x6\x3\x5\x5\x4\x6\x5\x3\x4\x6\x5\x6\x6\x6\x5\x5\x5\x6\x5\x6\x6\x6\x6\x6", 32);


	// Setup midi driver
	if (!driver)
		driver = MidiDriver_ADLIB_create();

	midi.mapMT32toGM (!(_game & GF_SIMON2) && !detector->_native_mt32);
	midi.set_driver(driver);
	int ret = midi.open();
	if (ret)
		warning ("MIDI Player init failed: \"%s\"", midi.getErrorName (ret));

	// Setup mixer
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed. "
						"Features of the game that depend on sound synchronization will most likely break");
	midi.set_volume(detector->_music_volume);
	set_volume(detector->_sfx_volume);

	_debugMode = detector->_debugMode;
	_debugLevel = detector->_debugLevel;
	_language = detector->_language;
	_noSubtitles = detector->_noSubtitles;

	_system->init_size(320, 200);

	// FIXME Use auto dirty rects cleanup code to reduce CPU usage
	_system->property(OSystem::PROP_WANT_RECT_OPTIM,0);

	// Override global scaler with any game-specific define
	if (g_config->get("gfx_mode")) {
		prop.gfx_mode = detector->parseGraphicsMode(g_config->get("gfx_mode"));
		_system->property(OSystem::PROP_SET_GFX_MODE, &prop);
	}

	// Override global scaler with any game-specific define
	if (g_config->getBool("fullscreen", false)) {
		if (!_system->property(OSystem::PROP_GET_FULLSCREEN, 0))
			_system->property(OSystem::PROP_TOGGLE_FULLSCREEN, 0);
	}
}

SimonEngine::~SimonEngine() {
	delete _dummy_item_1;
	delete _dummy_item_2;
	delete _dummy_item_3;
	
	delete [] _fcs_list;
	
	delete _sound;
}

void SimonEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void palette_fadeout(uint32 *pal_values, uint num) {
	byte *p = (byte *)pal_values;

	do {
		if (p[0] >= 8)
			p[0] -= 8;
		else
			p[0] = 0;
		if (p[1] >= 8)
			p[1] -= 8;
		else
			p[1] = 0;
		if (p[2] >= 8)
			p[2] -= 8;
		else
			p[2] = 0;
		p += sizeof(uint32);
	} while (--num);
}

byte *SimonEngine::allocateItem(uint size) {
	byte *org = _itemheap_ptr;
	size = (size + 3) & ~3;

	_itemheap_ptr += size;
	_itemheap_curpos += size;

	if (_itemheap_curpos > _itemheap_size)
		error("Itemheap overflow");

	return org;
}

void SimonEngine::alignTableMem() {
	if ((uint32)_tablesheap_ptr & 3) {
		_tablesheap_ptr += 2;
		_tablesheap_curpos += 2;
	}
}

byte *SimonEngine::allocateTable(uint size) {
	byte *org = _tablesheap_ptr;

	size = (size + 1) & ~1;

	_tablesheap_ptr += size;
	_tablesheap_curpos += size;

	if (_tablesheap_curpos > _tablesheap_size)
		error("Tablesheap overflow");

	return org;
}

int SimonEngine::allocGamePcVars(File *in) {
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = in->readUint32BE();
	version = in->readUint32BE();
	item_array_inited = in->readUint32BE();
	stringtable_num = in->readUint32BE();

	item_array_inited += 2;				// first two items are predefined
	item_array_size += 2;

	if (version != 0x80)
		error("Not a runtime database");

	_itemarray_ptr = (Item **)calloc(item_array_size, sizeof(Item *));
	if (_itemarray_ptr == NULL)
		error("Out of memory for Item array");

	_itemarray_size = item_array_size;
	_itemarray_inited = item_array_inited;

	for (i = 1; i < item_array_inited; i++) {
		_itemarray_ptr[i] = (Item *)allocateItem(sizeof(Item));
	}

	// The rest is cleared automatically by calloc
	allocateStringTable(stringtable_num + 10);
	_stringtab_num = stringtable_num;

	return item_array_inited;
}

void SimonEngine::loginPlayerHelper(Item *item, int a, int b) {
	Child9 *child;

	child = (Child9 *) findChildOfType(item, 9);
	if (child == NULL) {
		child = (Child9 *) allocateChildBlock(item, 9, sizeof(Child9));
	}

	if (a >= 0 && a <= 3)
		child->array[a] = b;
}

void SimonEngine::loginPlayer() {
	Child *child;

	_item_1 = _itemarray_ptr[1];
	_item_1->unk2 = -1;
	_item_1->unk1 = 10000;

	child = (Child *)allocateChildBlock(_item_1, 3, sizeof(Child));
	if (child == NULL)
		error("player create failure");

	loginPlayerHelper(_item_1, 0, 0);
}

void SimonEngine::allocateStringTable(int num) {
	_stringtab_ptr = (byte **)calloc(num, sizeof(byte *));
	_stringtab_pos = 0;
	_stringtab_numalloc = num;
}

void SimonEngine::setupStringTable(byte *mem, int num) {
	int i = 0;
	for (;;) {
		_stringtab_ptr[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}

	_stringtab_pos = i;
}

void SimonEngine::setupLocalStringTable(byte *mem, int num) {
	int i = 0;
	for (;;) {
		_local_stringtable[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}
}

void SimonEngine::readSubroutineLine(File *in, SubroutineLine *sl, Subroutine *sub) {
	byte line_buffer[1024], *q = line_buffer;
	int size;

	if (sub->id == 0) {
		sl->cond_a = in->readUint16BE();
		sl->cond_b = in->readUint16BE();
		sl->cond_c = in->readUint16BE();
	}

	while ((*q = in->readByte()) != 0xFF) {
		if (*q == 87) {
			in->readUint16BE();
		} else {
			q = readSingleOpcode(in, q);
		}
	}

	size = q - line_buffer + 1;

	memcpy(allocateTable(size), line_buffer, size);
}

SubroutineLine *SimonEngine::createSubroutineLine(Subroutine *sub, int where) {
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	// where is what offset to insert the line at, locate the proper beginning line
	if (sub->first != 0) {
		cur_sl = (SubroutineLine *)((byte *)sub + sub->first);
		while (where) {
			last_sl = cur_sl;
			cur_sl = (SubroutineLine *)((byte *)sub + cur_sl->next);
			if ((byte *)cur_sl == (byte *)sub)
				break;
			where--;
		}
	}

	if (last_sl != NULL) {
		// Insert the subroutine line in the middle of the link
		last_sl->next = (byte *)sl - (byte *)sub;
		sl->next = (byte *)cur_sl - (byte *)sub;
	} else {
		// Insert the subroutine line at the head of the link
		sl->next = sub->first;
		sub->first = (byte *)sl - (byte *)sub;
	}

	return sl;
}

void SimonEngine::readSubroutine(File *in, Subroutine *sub) {
	while (in->readUint16BE() == 0) {
		readSubroutineLine(in, createSubroutineLine(sub, 0xFFFF), sub);
	}
}

Subroutine *SimonEngine::createSubroutine(uint id) {
	Subroutine *sub;

	alignTableMem();

	sub = (Subroutine *)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutine_list;
	_subroutine_list = sub;
	return sub;
}

void SimonEngine::readSubroutineBlock(File *in) {
	while (in->readUint16BE() == 0) {
		readSubroutine(in, createSubroutine(in->readUint16BE()));
	}
}

Child *SimonEngine::findChildOfType(Item *i, uint type) {
	Child *child = i->children;
	for (; child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

bool SimonEngine::hasChildOfType1(Item *item) {
	return findChildOfType(item, 1) != NULL;
}

bool SimonEngine::hasChildOfType2(Item *item) {
	return findChildOfType(item, 2) != NULL;
}

uint SimonEngine::getOffsetOfChild2Param(Child2 *child, uint prop) {
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->avail_props & m)
			offset++;
		m <<= 1;
	}
	return offset;
}

Child *SimonEngine::allocateChildBlock(Item *i, uint type, uint size) {
	Child *child = (Child *)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

void SimonEngine::allocItemHeap() {
	_itemheap_size = 10000;
	_itemheap_curpos = 0;
	_itemheap_ptr = (byte *)calloc(10000, 1);
}

void SimonEngine::allocTablesHeap() {
	_tablesheap_size = gss->TABLES_MEM_SIZE;
	_tablesheap_curpos = 0;
	_tablesheap_ptr = (byte *)calloc(gss->TABLES_MEM_SIZE, 1);
}

void SimonEngine::setItemUnk3(Item *item, int value) {
	item->unk3 = value;
}

int SimonEngine::getNextWord() {
	_code_ptr += 2;
	return (int16)((_code_ptr[-2] << 8) | _code_ptr[-1]);
}

uint SimonEngine::getNextStringID() {
	return (uint16)getNextWord();
}

uint SimonEngine::getVarOrByte() {
	uint a = *_code_ptr++;
	if (a != 255)
		return a;
	return readVariable(*_code_ptr++);
}

uint SimonEngine::getVarOrWord() {
	uint a = (_code_ptr[0] << 8) | _code_ptr[1];
	_code_ptr += 2;
	if (a >= 30000 && a < 30512)
		return readVariable(a - 30000);
	return a;
}

Item *SimonEngine::getNextItemPtr() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subject_item;
	case -3:
		return _object_item;
	case -5:
		return getItem1Ptr();
	case -7:
		return getItemPtrB();
	case -9:
		return derefItem(getItem1Ptr()->parent);
	default:
		return derefItem(a);
	}
}

Item *SimonEngine::getNextItemPtrStrange() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subject_item;
	case -3:
		return _object_item;
	case -5:
		return _dummy_item_2;
	case -7:
		return NULL;
	case -9:
		return _dummy_item_3;
	default:
		return derefItem(a);
	}
}

uint SimonEngine::getNextItemID() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return itemPtrToID(_subject_item);
	case -3:
		return itemPtrToID(_object_item);
	case -5:
		return getItem1ID();
	case -7:
		return 0;
	case -9:
		return getItem1Ptr()->parent;
	default:
		return a;
	}
}

Item *SimonEngine::getItem1Ptr() {
	if (_item_1)
		return _item_1;
	return _dummy_item_1;
}

Item *SimonEngine::getItemPtrB() {
	error("getItemPtrB: is this code ever used?");
	return _dummy_item_1;
}

uint SimonEngine::getNextVarContents() {
	return (uint16)readVariable(getVarOrByte());
}

uint SimonEngine::readVariable(uint variable) {
	if (variable >= 255)
		error("Variable %d out of range in read", variable);
	return _variableArray[variable];
}

void SimonEngine::writeNextVarContents(uint16 contents) {
	writeVariable(getVarOrByte(), contents);
}

void SimonEngine::writeVariable(uint variable, uint16 contents) {
	if (variable >= 256)
		error("Variable %d out of range in write", variable);
	_variableArray[variable] = contents;
}

void SimonEngine::setItemParent(Item *item, Item *parent) {
	Item *old_parent = derefItem(item->parent);

	if (item == parent)
		error("Trying to set item as its own parent");

	// unlink it if it has a parent
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void SimonEngine::itemChildrenChanged(Item *item) {
	int i;
	FillOrCopyStruct *fcs;

	if (_no_parent_notify)
		return;

	lock();

	for (i = 0; i != 8; i++) {
		fcs = _fcs_ptr_array_3[i];
		if (fcs && fcs->fcs_data && fcs->fcs_data->item_ptr == item) {
			if (_fcs_data_1[i]) {
				_fcs_data_2[i] = true;
			} else {
				_fcs_data_2[i] = false;
				fcs_unk_proc_1(i, item, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
			}
		}
	}

	unlock();
}

void SimonEngine::unlinkItem(Item *item) {
	Item *first, *parent, *next;

	// can't unlink item without parent
	if (item->parent == 0)
		return;

	// get parent and first child of parent
	parent = derefItem(item->parent);
	first = derefItem(parent->child);

	// the node to remove is first in the parent's children?
	if (first == item) {
		parent->child = item->sibling;
		item->parent = 0;
		item->sibling = 0;
		return;
	}

	for (;;) {
		if (!first)
			error("unlinkItem: parent empty");
		if (first->sibling == 0)
			error("unlinkItem: parent does not contain child");

		next = derefItem(first->sibling);
		if (next == item) {
			first->sibling = next->sibling;
			item->parent = 0;
			item->sibling = 0;
			return;
		}
		first = next;
	}
}

void SimonEngine::linkItem(Item *item, Item *parent) {
	uint id;
	// Don't allow that an item that is already linked is relinked
	if (item->parent)
		return;

	id = itemPtrToID(parent);
	item->parent = id;

	if (parent != 0) {
		item->sibling = parent->child;
		parent->child = itemPtrToID(item);
	} else {
		item->sibling = 0;
	}
}

const byte *SimonEngine::getStringPtrByID(uint string_id) {
	const byte *string_ptr;
	byte *dst;

	_free_string_slot ^= 1;

	if (string_id < 0x8000) {
		string_ptr = _stringtab_ptr[string_id];
	} else {
		string_ptr = getLocalStringByID(string_id);
	}

	dst = _stringReturnBuffer[_free_string_slot];
	strcpy((char *)dst, (const char *)string_ptr);
	return dst;
}

const byte *SimonEngine::getLocalStringByID(uint string_id) {
	if (string_id < _string_id_local_min || string_id >= _string_id_local_max) {
		loadTextIntoMem(string_id);
	}
	return _local_stringtable[string_id - _string_id_local_min];
}

void SimonEngine::loadTextIntoMem(uint string_id) {
	byte *p;
	char filename[30];
	int i;
	uint base_min = 0x8000, base_max, size;

	_tablesheap_ptr = _tablesheap_ptr_new;
	_tablesheap_curpos = _tablesheap_curpos_new;

	p = _stripped_txt_mem;

	// get filename
	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		base_max = (p[0] << 8) | p[1];
		p += 2;

		if (string_id < base_max) {
			_string_id_local_min = base_min;
			_string_id_local_max = base_max;

			_local_stringtable = (byte **)_tablesheap_ptr;

			size = (base_max - base_min + 1) * sizeof(byte *);
			_tablesheap_ptr += size;
			_tablesheap_curpos += size;

			size = loadTextFile(filename, _tablesheap_ptr);

			setupLocalStringTable(_tablesheap_ptr, base_max - base_min + 1);

			_tablesheap_ptr += size;
			_tablesheap_curpos += size;

			if (_tablesheap_curpos > _tablesheap_size) {
				error("loadTextIntoMem: Out of table memory");
			}
			return;
		}

		base_min = base_max;
	}

	error("loadTextIntoMem: didn't find %d", string_id);
}

void SimonEngine::loadTablesIntoMem(uint subr_id) {
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	File *in;

	p = _tbl_list;
	if (p == NULL)
		return;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = (p[0] << 8) | p[1];
			p += 2;

			if (min_num == 0)
				break;

			max_num = (p[0] << 8) | p[1];
			p += 2;

			if (subr_id >= min_num && subr_id <= max_num) {
				_subroutine_list = _subroutine_list_org;
				_tablesheap_ptr = _tablesheap_ptr_org;
				_tablesheap_curpos = _tablesheap_curpos_org;
				_string_id_local_min = 1;
				_string_id_local_max = 0;

				in = openTablesFile(filename);
				readSubroutineBlock(in);
				closeTablesFile(in);

				if (_game == GAME_SIMON1WIN) {
					memcpy(filename, "SFXXXX", 6);
					_sound->readSfxFile(filename, _gameDataPath);
				} else if (_game & GF_SIMON2) {
					_sound->loadSfxTable(_game_file, _game_offsets_ptr[atoi(filename + 6) - 1 + gss->SOUND_INDEX_BASE]);
				}

				alignTableMem();

				_tablesheap_ptr_new = _tablesheap_ptr;
				_tablesheap_curpos_new = _tablesheap_curpos;

				if (_tablesheap_curpos > _tablesheap_size)
					error("loadTablesIntoMem: Out of table memory");
				return;
			}
		}
	}

		debug(1,"loadTablesIntoMem: didn't find %d", subr_id);
}

void SimonEngine::playSting(uint a) {
	if (!midi._enable_sfx)
		return;

	char filename[11];

	File mus_file;
	uint16 mus_offset;

	sprintf(filename, "STINGS%i.MUS", _sound_file_id);
	mus_file.open(filename, _gameDataPath);
	if (!mus_file.isOpen()) {
		warning("Can't load sound effect from '%s'", filename);
		return;
	}

	//uint16 size = mus_file.readUint16LE();

	mus_file.seek(a * 2, SEEK_SET);
	mus_offset = mus_file.readUint16LE();
	if (mus_file.ioFailed())
		error("Can't read sting %d offset", a);

	mus_file.seek(mus_offset, SEEK_SET);
	midi.loadSMF(&mus_file, a, true);
	midi.startTrack(0);
}

Subroutine *SimonEngine::getSubroutineByID(uint subroutine_id) {
	Subroutine *cur;

	for (cur = _subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	loadTablesIntoMem(subroutine_id);

	for (cur = _subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	debug(1,"getSubroutineByID: subroutine %d not found", subroutine_id);
	return NULL;
}

uint SimonEngine::loadTextFile_gme(const char *filename, byte *dst) {
	uint res;
	uint32 offs;
	uint32 size;

	res = atoi(filename + 4) + gss->TEXT_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];
	size = _game_offsets_ptr[res + 1] - offs;

	resfile_read(dst, offs, size);

	return size;
}

File *SimonEngine::openTablesFile_gme(const char *filename) {
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + gss->TABLE_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];

	_game_file->seek(offs, SEEK_SET);
	return _game_file;
}

uint SimonEngine::loadTextFile_simon1(const char *filename, byte *dst) {
	File fo;
	fo.open(filename, _gameDataPath);
	uint32 size;

	if (fo.isOpen() == false)
		error("loadTextFile: Can't open '%s'", filename);

	size = fo.size();

	if (fo.read(dst, size) != size)
		error("loadTextFile: fread failed");
	fo.close();

	return size;
}

File *SimonEngine::openTablesFile_simon1(const char *filename) {
	File *fo = new File();
	fo->open(filename, _gameDataPath);
	if (fo->isOpen() == false)
		error("openTablesFile: Can't open '%s'", filename);
	return fo;
}

uint SimonEngine::loadTextFile(const char *filename, byte *dst) {
	if (_game & GF_AMIGA || _game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS)
		return loadTextFile_simon1(filename, dst);
	else
		return loadTextFile_gme(filename, dst);
}

File *SimonEngine::openTablesFile(const char *filename) {
	if (_game & GF_AMIGA || _game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

void SimonEngine::closeTablesFile(File *in) {
	if (_game & GF_AMIGA || _game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS) {
		in->close();
		delete in;
	}
}

void SimonEngine::addTimeEvent(uint timeout, uint subroutine_id) {
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	time_t cur_time;

	time(&cur_time);

	te->time = cur_time + timeout - _base_time;
	te->subroutine_id = subroutine_id;

	first = _first_time_struct;
	while (first) {
		if (te->time <= first->time) {
			if (last) {
				last->next = te;
				te->next = first;
				return;
			}
			te->next = _first_time_struct;
			_first_time_struct = te;
			return;
		}

		last = first;
		first = first->next;
	}

	if (last) {
		last->next = te;
		te->next = NULL;
	} else {
		_first_time_struct = te;
		te->next = NULL;
	}
}

void SimonEngine::delTimeEvent(TimeEvent *te) {
	TimeEvent *cur;

	if (te == _pending_delete_time_event)
		_pending_delete_time_event = NULL;

	if (te == _first_time_struct) {
		_first_time_struct = te->next;
		free(te);
		return;
	}

	cur = _first_time_struct;
	if (cur == NULL)
		error("delTimeEvent: none available");

	for (;;) {
		if (cur->next == NULL)
			error("delTimeEvent: no such te");
		if (te == cur->next) {
			cur->next = te->next;
			free(te);
			return;
		}
		cur = cur->next;
	}
}

void SimonEngine::killAllTimers() {
	TimeEvent *cur, *next;

	for (cur = _first_time_struct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
}

bool SimonEngine::kickoffTimeEvents() {
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	time(&cur_time);
	cur_time -= _base_time;

	while ((te = _first_time_struct) != NULL && te->time <= (uint32)cur_time) {
		result = true;
		_pending_delete_time_event = te;
		invokeTimeEvent(te);
		if (_pending_delete_time_event) {
			_pending_delete_time_event = NULL;
			delTimeEvent(te);
		}
	}

	return result;
}

void SimonEngine::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_script_cond_a = 0;
	if (_run_script_return_1)
		return;
	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);
	_run_script_return_1 = false;
}

void SimonEngine::o_setup_cond_c() {

	setup_cond_c_helper();

	_object_item = _hitarea_object_item;

	if (_object_item == _dummy_item_2)
		_object_item = getItem1Ptr();

	if (_object_item == _dummy_item_3)
		_object_item = derefItem(getItem1Ptr()->parent);

	if (_object_item != NULL) {
		_script_cond_c = _object_item->unk1;
	} else {
		_script_cond_c = -1;
	}
}

void SimonEngine::setup_cond_c_helper() {
	HitArea *last;

	if (_game & GF_SIMON2) {
		_mouse_cursor = 0;
		if (_hitarea_unk_4 != 999) {
			_mouse_cursor = 9;
			_need_hitarea_recalc++;
			_hitarea_unk_4 = 0;
		}
	}

	_last_hitarea = 0;
	_hitarea_object_item = NULL;
	_hitarea_unk_6 = true;

	last = _last_hitarea_2_ptr;
	defocusHitarea();
	_last_hitarea_2_ptr = last;

	for (;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = 0;
		_left_button_down = 0;

		do {
			if (_exit_cutscene && (_bit_array[0] & 0x200)) {
				startSubroutine170();
				goto out_of_here;
			}

			delay(100);
		} while (_last_hitarea_3 == (HitArea *) 0xFFFFFFFF || _last_hitarea_3 == 0);

		if (_last_hitarea == NULL) {
		} else if (_last_hitarea->id == 0x7FFB) {
			handle_uparrow_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->id == 0x7FFC) {
			handle_downarrow_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->item_ptr != NULL) {
			_hitarea_object_item = _last_hitarea->item_ptr;
			_variableArray[60] = (_last_hitarea->flags & 1) ? (_last_hitarea->flags >> 8) : 0xFFFF;
			break;
		}
	}

out_of_here:
	_last_hitarea_3 = 0;
	_last_hitarea = 0;
	_last_hitarea_2_ptr = NULL;
	_hitarea_unk_6 = false;
}

void SimonEngine::startSubroutine170() {
	Subroutine *sub;

	_sound->stopVoice();
	
	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_run_script_return_1 = true;
}

uint SimonEngine::get_fcs_ptr_3_index(FillOrCopyStruct *fcs) {
	uint i;

	for (i = 0; i != ARRAYSIZE(_fcs_ptr_array_3); i++)
		if (_fcs_ptr_array_3[i] == fcs)
			return i;

	error("get_fcs_ptr_3_index: not found");
}

void SimonEngine::lock() {
	_lock_counter++;
}

void SimonEngine::unlock() {
	_lock_word |= 1;

	if (_lock_counter != 0)
		_lock_counter--;

	_lock_word &= ~1;
}

void SimonEngine::handle_mouse_moved() {
	uint x;

	if (_lock_counter) {
		_system->show_mouse(false);
		return;
	}

	_system->show_mouse(true);
	pollMouseXY();

	if (_mouse_x >= 32768)
		_mouse_x = 0;
	if (_mouse_x >= 638 / 2)
		_mouse_x = 638 / 2;

	if (_mouse_y >= 32768)
		_mouse_y = 0;
	if (_mouse_y >= 199)
		_mouse_y = 199;

	if (_hitarea_unk_4) {
		uint id = 101;
		if (_mouse_y >= 136)
			id = 102;
		if (_hitarea_unk_4 != id)
			hitarea_proc_1();
	}

	if (_game & GF_SIMON2) {
		if (_bit_array[4] & 0x8000) {
			if (!_vga_var9) {
				if (_mouse_x >= 630 / 2 || _mouse_x < 9)
					goto get_out2;
				_vga_var9 = 1;
			}
			if (_vga_var2 == 0) {
				if (_mouse_x >= 631 / 2) {
					if (_x_scroll != _vga_var1)
						_vga_var3 = 1;
				} else if (_mouse_x < 8) {
					if (_x_scroll != 0)
						_vga_var3 = -1;
				}
			}
		} else {
		get_out2:;
			_vga_var9 = 0;
		}
	}

	if (_mouse_x != _mouse_x_old || _mouse_y != _mouse_y_old)
		_need_hitarea_recalc++;

	x = 0;
	if (_last_hitarea_3 == 0 && _left_button_down != 0) {
		_left_button_down = 0;
		x = 1;
	} else {
		if (_hitarea_unk_3 == 0 && _need_hitarea_recalc == 0)
			goto get_out;
	}

	setup_hitarea_from_pos(_mouse_x, _mouse_y, x);
	_last_hitarea_3 = _last_hitarea;
	if (x == 1 && _last_hitarea == NULL)
		_last_hitarea_3 = (HitArea *) - 1;

get_out:
	draw_mouse_pointer();
	_need_hitarea_recalc = 0;
}

void SimonEngine::fcs_unk_proc_1(uint fcs_index, Item *item_ptr, int unk1, int unk2) {
	Item *item_ptr_org = item_ptr;
	FillOrCopyStruct *fcs_ptr;
	uint width_div_3, height_div_3;
	uint j, k, i, num_sibs_with_flag;
	bool item_again;
	uint x_pos, y_pos;

	fcs_ptr = _fcs_ptr_array_3[fcs_index & 7];

	if (!(_game & GF_SIMON2)) {
		width_div_3 = fcs_ptr->width / 3;
		height_div_3 = fcs_ptr->height / 3;
	} else {
		width_div_3 = 100;
		height_div_3 = 40;
	}

	i = 0;

	if (fcs_ptr == NULL)
		return;

	if (fcs_ptr->fcs_data)
		fcs_unk1(fcs_index);

	fcs_ptr->fcs_data = (FillOrCopyData *) malloc(sizeof(FillOrCopyData));
	fcs_ptr->fcs_data->item_ptr = item_ptr;
	fcs_ptr->fcs_data->unk3 = -1;
	fcs_ptr->fcs_data->unk4 = -1;
	fcs_ptr->fcs_data->unk1 = unk1;
	fcs_ptr->fcs_data->unk2 = unk2;

	item_ptr = derefItem(item_ptr->child);

	while (item_ptr && unk1-- != 0) {
		num_sibs_with_flag = 0;
		while (item_ptr && width_div_3 > num_sibs_with_flag) {
			if ((unk2 == 0 || item_ptr->unk4 & unk2) && has_item_childflag_0x10(item_ptr))
				if (!(_game & GF_SIMON2)) {
					num_sibs_with_flag++;
				} else {
					num_sibs_with_flag += 20;
				}
			item_ptr = derefItem(item_ptr->sibling);
		}
	}

	if (item_ptr == NULL) {
		fcs_ptr->fcs_data->unk1 = 0;
		item_ptr = derefItem(item_ptr_org->child);
	}

	x_pos = 0;
	y_pos = 0;
	item_again = false;
	k = 0;
	j = 0;

	while (item_ptr) {
		if ((unk2 == 0 || item_ptr->unk4 & unk2) && has_item_childflag_0x10(item_ptr)) {
			if (item_again == false) {
				fcs_ptr->fcs_data->e[k].item = item_ptr;
				if (!(_game & GF_SIMON2)) {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos * 3, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos * 3, y_pos,
																item_get_icon_number(item_ptr), item_ptr);
				} else {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos, y_pos, item_get_icon_number(item_ptr), item_ptr);
				}
				k++;
			} else {
				fcs_ptr->fcs_data->e[k].item = NULL;
				j = 1;
			}
			x_pos += (_game & GF_SIMON2) ? 20 : 1;

			if (x_pos >= width_div_3) {
				x_pos = 0;

				y_pos += (_game & GF_SIMON2) ? 20 : 1;
				if (y_pos >= height_div_3)
					item_again = true;
			}
		}
		item_ptr = derefItem(item_ptr->sibling);
	}

	fcs_ptr->fcs_data->e[k].item = NULL;

	if (j != 0 || fcs_ptr->fcs_data->unk1 != 0) {
		fcs_unk_proc_2(fcs_ptr, fcs_index);
	}
}

void SimonEngine::fcs_unk_proc_2(FillOrCopyStruct *fcs, uint fcs_index) {
	setup_hit_areas(fcs, fcs_index);

	fcs->fcs_data->unk3 = _scroll_up_hit_area;
	fcs->fcs_data->unk4 = _scroll_down_hit_area;
}

void SimonEngine::setup_hit_areas(FillOrCopyStruct *fcs, uint fcs_index) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scroll_up_hit_area = ha - _hit_areas;
	if (!(_game & GF_SIMON2)) {
		ha->x = 308;
		ha->y = 149;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	} else {
		ha->x = 81;
		ha->y = 158;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}

	ha = findEmptyHitArea();
	_scroll_down_hit_area = ha - _hit_areas;

	if (!(_game & GF_SIMON2)) {
		ha->x = 308;
		ha->y = 176;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;

		// Simon1 specific
		o_kill_sprite_simon1(0x80);
		start_vga_code(0, 1, 0x80, 0, 0, 0xE);
	} else {
		ha->x = 227;
		ha->y = 162;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}
}


bool SimonEngine::has_item_childflag_0x10(Item *item) {
	Child2 *child = (Child2 *)findChildOfType(item, 2);
	return child && (child->avail_props & 0x10) != 0;
}

uint SimonEngine::item_get_icon_number(Item *item) {
	Child2 *child = (Child2 *)findChildOfType(item, 2);
	uint offs;

	if (child == NULL || !(child->avail_props & 0x10))
		return 0;

	offs = getOffsetOfChild2Param(child, 0x10);
	return child->array[offs];
}

void SimonEngine::loadIconFile() {
	File in;
	if (_game & GF_AMIGA)
		in.open("icon.pkd", _gameDataPath);
	else
		in.open("ICON.DAT", _gameDataPath);
	uint size;

	if (in.isOpen() == false)
		error("Can't open icons file 'ICON.DAT'");

	size = in.size();

	_icon_file_ptr = (byte *)malloc(size);
	if (_icon_file_ptr == NULL)
		error("Out of icon memory");

	in.read(_icon_file_ptr, size);
	in.close();
}

uint SimonEngine::setup_icon_hit_area(FillOrCopyStruct *fcs, uint x, uint y, uint icon_number,
																		 Item *item_ptr) {
	HitArea *ha;

	ha = findEmptyHitArea();

	if (!(_game & GF_SIMON2)) {
		ha->x = (x + fcs->x) << 3;
		ha->y = y * 25 + fcs->y;
		ha->item_ptr = item_ptr;
		ha->width = 24;
		ha->height = 24;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	} else {
		ha->x = x + 110;
		ha->y = fcs->y + y;
		ha->item_ptr = item_ptr;
		ha->width = 20;
		ha->height = 20;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	}

	return ha - _hit_areas;
}

void SimonEngine::f10_key() {
	HitArea *ha;
	uint count;
	uint y_, x_;
	byte *dst;
	uint b;

	_lock_word |= 0x8000;
	
	for (int i = 0; i < 5; i++) {
		ha = _hit_areas;
		count = ARRAYSIZE(_hit_areas);

		timer_vga_sprites();

		do {
			if (ha->id != 0 && ha->flags & 0x20 && !(ha->flags & 0x40)) {
				if (ha->y >= 0xc8 || ha->y >= _vga_var8)
					continue;

				y_ = (ha->height >> 1) - 4 + ha->y;

				x_ = (ha->width >> 1) - 4 + ha->x - (_x_scroll << 3);

				if (x_ >= 0x137)
					continue;

				dst = dx_lock_attached();

				dst += (((_dx_surface_pitch >> 2) * y_) << 2) + x_;

				b = _dx_surface_pitch;
				dst[4] = 0xec;
				dst[b+1] = 0xec;
				dst[b+4] = 0xec;
				dst[b+7] = 0xec;
				b += _dx_surface_pitch;
				dst[b+2] = 0xec;
				dst[b+4] = 0xec;
				dst[b+6] = 0xec;
				b += _dx_surface_pitch;
				dst[b+3] = 0xec;
				dst[b+5] = 0xec;
				b += _dx_surface_pitch;
				dst[b] = 0xec;
				dst[b+1] = 0xec;
				dst[b+2] = 0xec;
				dst[b+6] = 0xec;
				dst[b+7] = 0xec;
				dst[b+8] = 0xec;
				b += _dx_surface_pitch;
				dst[b+3] = 0xec;
				dst[b+5] = 0xec;
				b += _dx_surface_pitch;
				dst[b+2] = 0xec;
				dst[b+4] = 0xec;
				dst[b+6] = 0xec;
				b += _dx_surface_pitch;
				dst[b+1] = 0xec;
				dst[b+4] = 0xec;
				dst[b+7] = 0xec;
				b += _dx_surface_pitch;
				dst[b+4] = 0xec;

				dx_unlock_attached();
			}
		} while (ha++, --count);

		dx_update_screen_and_palette();
		delay(100);
		timer_vga_sprites();
		dx_update_screen_and_palette();
		delay(100);
	}

	_lock_word &= ~0x8000;
}

void SimonEngine::hitarea_stuff() {
	HitArea *ha;
	uint id;

	_left_button_down = 0;
	_last_hitarea = 0;
	_verb_hitarea = 0;
	_hitarea_subject_item = NULL;
	_hitarea_object_item = NULL;

	hitarea_proc_1();

startOver:
	for (;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = NULL;
		for (;;) {
			if (_game & GF_SIMON2 && _key_pressed == 35)
				f10_key();
			processSpecialKeys();
			if (_last_hitarea_3 == (HitArea *) 0xFFFFFFFF)
				goto startOver;
			if (_last_hitarea_3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _last_hitarea;

		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			handle_uparrow_hitarea(ha->fcs);
		} else if (ha->id == 0x7FFC) {
			handle_downarrow_hitarea(ha->fcs);
		} else if (ha->id >= 101 && ha->id < 113) {
			_verb_hitarea = ha->unk3;
			handle_verb_hitarea(ha);
			_hitarea_unk_4 = 0;
		} else {
			if ((_verb_hitarea != 0 || _hitarea_subject_item != ha->item_ptr && ha->flags & 0x80) &&
					ha->item_ptr) {
			if_1:;
				_hitarea_subject_item = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags & 1)
					id = ha->flags >> 8;
				_variableArray[60] = id;
				new_current_hitarea(ha);
				if (_verb_hitarea != 0)
					break;
			} else {
				// else 1
				if (ha->unk3 == 0) {
					if (ha->item_ptr)
						goto if_1;
				} else {
					_verb_hitarea = ha->unk3 & 0xBFFF;
					if (ha->unk3 & 0x4000) {
						_hitarea_subject_item = ha->item_ptr;
						break;
					}
					if (_hitarea_subject_item != NULL)
						break;
				}
			}
		}
	}

	_need_hitarea_recalc++;
}

void SimonEngine::hitarea_stuff_helper() {
	time_t cur_time;

	if (!(_game & GF_SIMON2)) {
		uint subr_id = _variableArray[0x1FC / 2];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				startUp_helper_2();
			}
			_variableArray[0x1FC / 2] = 0;
			_run_script_return_1 = false;
		}
	} else {
		if (_variableArray[0x1FC / 2] || _variableArray[0x1F2 / 2]) {
			hitarea_stuff_helper_2();
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _last_time) {
		_last_time = cur_time;
		if (kickoffTimeEvents())
			startUp_helper_2();
	}
}

// Simon 2 specific
void SimonEngine::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = _variableArray[0x1F2 / 2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1F2 / 2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1F2 / 2] = 0;
	}

	subr_id = _variableArray[0x1FC / 2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1FC / 2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1FC / 2] = 0;
	}

	_run_script_return_1 = false;
}

void SimonEngine::startUp_helper_2() {
	if (!_mortal_flag) {
		_mortal_flag = true;
		startUp_helper_3();
		_fcs_unk_1 = 0;
		if (_fcs_ptr_array_3[0] != 0) {
			_fcs_ptr_1 = _fcs_ptr_array_3[0];
			showmessage_helper_3(_fcs_ptr_1->textLength, _fcs_ptr_1->textMaxLength);
		}
		_mortal_flag = false;
	}
}

void SimonEngine::startUp_helper_3() {
	showmessage_print_char(0);
}

void SimonEngine::pollMouseXY() {
	_mouse_x = _sdl_mouse_x;
	_mouse_y = _sdl_mouse_y;
}

void SimonEngine::handle_verb_clicked(uint verb) {
	Subroutine *sub;
	int result;

	_object_item = _hitarea_object_item;
	if (_object_item == _dummy_item_2) {
		_object_item = getItem1Ptr();
	}
	if (_object_item == _dummy_item_3) {
		_object_item = derefItem(getItem1Ptr()->parent);
	}

	_subject_item = _hitarea_subject_item;
	if (_subject_item == _dummy_item_2) {
		_subject_item = getItem1Ptr();
	}
	if (_subject_item == _dummy_item_3) {
		_subject_item = derefItem(getItem1Ptr()->parent);
	}

	if (_subject_item) {
		_script_cond_b = _subject_item->unk1;
	} else {
		_script_cond_b = -1;
	}

	if (_object_item) {
		_script_cond_c = _object_item->unk1;
	} else {
		_script_cond_c = -1;
	}

	_script_cond_a = _verb_hitarea;

	sub = getSubroutineByID(0);
	if (sub == NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_run_script_return_1 = false;

	sub = getSubroutineByID(100);
	if (sub)
		startSubroutine(sub);

	if (_game & GF_SIMON2)
		_run_script_return_1 = false;

	startUp_helper_2();
}

ThreeValues *SimonEngine::getThreeValues(uint a) {
	switch (a) {
	case 1:
		return &_threevalues_1;
	case 2:
		return &_threevalues_2;
	case 101:
		return &_threevalues_3;
		break;
	case 102:
		return &_threevalues_4;
		break;
	default:
		error("text, invalid value %d", a);
	}
}

void SimonEngine::o_print_str() {
	uint num_1 = getVarOrByte();
	uint num_2 = getVarOrByte();
	uint string_id = getNextStringID();
	const byte *string_ptr = NULL;
	uint speech_id = 0;
	ThreeValues *tv;

	if (_game & GF_TALKIE) {
		if (string_id != 0xFFFF)
			string_ptr = getStringPtrByID(string_id);

		speech_id = (uint16)getNextWord();
	} else {
		string_ptr = getStringPtrByID(string_id);
	}

	tv = getThreeValues(num_1);

	switch (_game) {
	case GAME_SIMON1TALKIE:
	case GAME_SIMON1WIN:
	case GAME_SIMON1CD32:
		if (speech_id != 0)
			talk_with_speech(speech_id, num_1);
		if (string_ptr != NULL && (_subtitles || speech_id == 0))
			talk_with_text(num_1, num_2, (const char *)string_ptr, tv->a, tv->b, tv->c);
		break;

	case GAME_SIMON1DEMO:
	case GAME_SIMON1DOS:
	case GAME_SIMON1AMIGA:
	case GAME_SIMON2DOS:
		talk_with_text(num_1, num_2, (const char *)string_ptr, tv->a, tv->b, tv->c);
		break;

	case GAME_SIMON2TALKIE:
	case GAME_SIMON2WIN:
	case GAME_SIMON2MAC:
		if (speech_id != 0 && num_1 == 1 && (_language == 20 || !_subtitles))
			talk_with_speech(speech_id, num_1);

		if ((_game & GF_TALKIE) && (speech_id == 0))
			o_kill_sprite_simon2(2, num_1 + 2);

		if (_subtitles)
			talk_with_text(num_1, num_2, (const char *)string_ptr, tv->a, tv->b, tv->c);
		break;
	}
}

void SimonEngine::ensureVgaResLoadedC(uint vga_res) {
	_lock_word |= 0x80;
	ensureVgaResLoaded(vga_res);
	_lock_word &= ~0x80;
}

void SimonEngine::ensureVgaResLoaded(uint vga_res) {
	VgaPointersEntry *vpe;

	CHECK_BOUNDS(vga_res, _vga_buffer_pointers);

	vpe = _vga_buffer_pointers + vga_res;
	if (vpe->vgaFile1 != NULL)
		return;

	vpe->vgaFile2 = read_vga_from_datfile_2(vga_res * 2 + 1);
	vpe->vgaFile1 = read_vga_from_datfile_2(vga_res * 2);

}

byte *SimonEngine::setup_vga_destination(uint32 size) {
	byte *dest, *end;

	_video_var_4 = 0;

	for (;;) {
		dest = _vga_buf_free_start;

		end = dest + size;

		if (end >= _vga_buf_end) {
			_vga_buf_free_start = _vga_buf_start;
		} else {
			_video_var_5 = false;
			vga_buf_unk_proc3(end);
			if (_video_var_5)
				continue;
			vga_buf_unk_proc1(end);
			if (_video_var_5)
				continue;
			delete_memptr_range(end);
			_vga_buf_free_start = end;
			return dest;
		}
	}
}

void SimonEngine::setup_vga_file_buf_pointers() {
	byte *alloced;

	alloced = (byte *)malloc(gss->VGA_MEM_SIZE);

	_vga_buf_free_start = alloced;
	_vga_buf_start = alloced;
	_vga_file_buf_org = alloced;
	_vga_file_buf_org_2 = alloced;
	_vga_buf_end = alloced + gss->VGA_MEM_SIZE;
}

void SimonEngine::vga_buf_unk_proc3(byte *end) {
	VgaPointersEntry *vpe;

	if (_video_var_7 == 0xFFFF)
		return;

	if (_video_var_4 == 2)
		error("vga_buf_unk_proc3: _video_var_4 == 2");

	vpe = &_vga_buffer_pointers[_video_var_7];

	if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
			_vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_video_var_5 = 1;
		_video_var_4++;
		_vga_buf_free_start = vpe->vgaFile1 + 0x5000;
	} else {
		_video_var_5 = 0;
	}
}

void SimonEngine::vga_buf_unk_proc1(byte *end) {
	VgaSprite *vsp;
	if (_lock_word & 0x20)
		return;

	for (vsp = _vga_sprites; vsp->id; vsp++) {
		vga_buf_unk_proc2(vsp->unk7, end);
		if (_video_var_5 == true)
			return;
	}
}

void SimonEngine::delete_memptr_range(byte *end) {
	uint count = ARRAYSIZE(_vga_buffer_pointers);
	VgaPointersEntry *vpe = _vga_buffer_pointers;
	do {
		if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
				_vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
			vpe->dd = 0;
			vpe->vgaFile1 = NULL;
			vpe->vgaFile2 = NULL;
		}

	} while (++vpe, --count);
}

void SimonEngine::vga_buf_unk_proc2(uint a, byte *end) {
	VgaPointersEntry *vpe;

	vpe = &_vga_buffer_pointers[a];

	if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
			_vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_video_var_5 = true;
		_video_var_4++;
		_vga_buf_free_start = vpe->vgaFile1 + 0x5000;
	} else {
		_video_var_5 = false;
	}
}

void SimonEngine::o_clear_vgapointer_entry(uint a) {
	VgaPointersEntry *vpe;

	vpe = &_vga_buffer_pointers[a];

	vpe->dd = 0;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void SimonEngine::o_set_video_mode(uint mode, uint vga_res) {
	if (mode == 4)
		vc_29_stop_all_sounds();

	if (_lock_word & 0x10) {
		error("o_set_video_mode_ex: _lock_word & 0x10");
//    _unk21_word_array[a] = b; 
	} else {
		set_video_mode(mode, vga_res);
	}
}

void SimonEngine::set_video_mode_internal(uint mode, uint vga_res_id) {
	uint num;
	VgaPointersEntry *vpe;
	byte *bb, *b;
	uint16 c;
	byte *vc_ptr_org;

	_video_palette_mode = mode;
	_lock_word |= 0x20;

	if (vga_res_id == 0) {

		if (!(_game & GF_SIMON2)) {
			_unk_pal_flag = true;
		} else {
			_dx_use_3_or_4_for_lock = true;
			_vga_var6 = true;
		}
	}

	_vga_cur_file_2 = num = vga_res_id / 100;

	for (;;) {
		vpe = &_vga_buffer_pointers[num];

		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;

		if (vpe->vgaFile1 != NULL)
			break;

		ensureVgaResLoaded(num);
	}

	// ensure flipping complete

	bb = _cur_vga_file_1;
	b = bb + READ_BE_UINT16(&((VgaFile1Header *) bb)->hdr2_start);
	c = READ_BE_UINT16(&((VgaFile1Header2 *) b)->unk1);
	b = bb + READ_BE_UINT16(&((VgaFile1Header2 *) b)->unk2_offs);

	while (READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->id) != vga_res_id)
		b += sizeof(VgaFile1Struct0x8);

	if (!(_game & GF_SIMON2)) {
		if (num == 16300) {
			dx_clear_attached_from_top(134);
			_use_palette_delay = true;
		}
	} else {
		_x_scroll = 0;
		_vga_var1 = 0;
		_vga_var2 = 0;
		_vga_var3 = 0;
		_vga_var5 = 134;
		if (_variableArray[34] != -1)
			_variableArray[502 / 2] = 0;
	}

	vc_ptr_org = _vc_ptr;

	_vc_ptr = _cur_vga_file_1 + READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->script_offs);
	//dump_vga_script(_vc_ptr, num, vga_res_id);
	run_vga_script();
	_vc_ptr = vc_ptr_org;


	if (_game & GF_SIMON2) {
		if (!_dx_use_3_or_4_for_lock) {
			uint num_lines = _video_palette_mode == 4 ? 134 : 200;
			_vga_var8 = num_lines;
			dx_copy_from_attached_to_2(0, 0, 320, num_lines);
			dx_copy_from_attached_to_3(num_lines);
			_sync_flag_2 = 1;
		}
		_dx_use_3_or_4_for_lock = false;
	} else {
		uint num_lines = _video_palette_mode == 4 ? 134 : 200;
		dx_copy_from_attached_to_2(0, 0, 320, num_lines);
		dx_copy_from_attached_to_3(num_lines);
		_sync_flag_2 = 1;
		_timer_5 = 0;
	}

	_lock_word &= ~0x20;

	if (!(_game & GF_SIMON2)) {
		if (_unk_pal_flag) {
			_unk_pal_flag = false;
			while (*(volatile int *)&_palette_color_count != 0) {
				delay(10);
			}
		}
	}
}

void SimonEngine::set_video_mode(uint mode, uint vga_res_id) {
	if (_lock_counter == 0) {
		lock();
		if (_lock_word == 0) {
			_sync_flag_1 = true;
			while ((*(volatile bool *)&_sync_flag_1) == true) {
				delay(10);
			}
		}
	}

	_lock_word |= 0x20;

	while ((*(volatile uint16*)&_lock_word) & 2) {
		delay(10);
	}

	unlock();

	set_video_mode_internal(mode, vga_res_id);
}

void SimonEngine::o_fade_to_black() {
	uint i;

	memcpy(_video_buf_1, _palette_backup, 256 * sizeof(uint32));

	i = NUM_PALETTE_FADEOUT;
	do {
		palette_fadeout((uint32 *)_video_buf_1, 32);
		palette_fadeout((uint32 *)_video_buf_1 + 32 + 16, 144);
		palette_fadeout((uint32 *)_video_buf_1 + 32 + 16 + 144 + 16, 48);

		_system->set_palette(_video_buf_1, 0, 256);
		_system->update_screen();
		delay(5);
	} while (--i);

	memcpy(_palette_backup, _video_buf_1, 256 * sizeof(uint32));
	memcpy(_palette, _video_buf_1, 256 * sizeof(uint32));
}

void SimonEngine::delete_vga_timer(VgaTimerEntry * vte) {
	_lock_word |= 1;

	if (vte + 1 <= _next_vga_timer_to_process) {
		_next_vga_timer_to_process--;
	}

	do {
		memcpy(vte, vte + 1, sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);

	_lock_word &= ~1;
}

void SimonEngine::expire_vga_timers() {
	if (_game & GF_SIMON2) {
		VgaTimerEntry *vte = _vga_timer_list;

		_vga_tick_counter++;

		while (vte->delay) {
			// not quite ok, good enough
			if ((int16)(vte->delay -= 5) <= 0) {
				uint16 cur_file = vte->cur_vga_file;
				uint16 cur_unk = vte->sprite_id;
				byte *script_ptr = vte->script_pointer;

				_next_vga_timer_to_process = vte + 1;
				delete_vga_timer(vte);

				if (script_ptr == NULL) {
					// special scroll timer
					scroll_timeout();
				} else {
					vc_resume_sprite(script_ptr, cur_file, cur_unk);
				}
				vte = _next_vga_timer_to_process;
			} else {
				vte++;
			}
		}
	} else {
		VgaTimerEntry *vte = _vga_timer_list;

		_vga_tick_counter++;

		while (vte->delay) {
			if (!--vte->delay) {
				uint16 cur_file = vte->cur_vga_file;
				uint16 cur_unk = vte->sprite_id;
				byte *script_ptr = vte->script_pointer;

				_next_vga_timer_to_process = vte + 1;
				delete_vga_timer(vte);

				vc_resume_sprite(script_ptr, cur_file, cur_unk);
				vte = _next_vga_timer_to_process;
			} else {
				vte++;
			}
		}
	}
}

// Simon2 specific
void SimonEngine::scroll_timeout() {
	if (_vga_var2 == 0)
		return;

	if (_vga_var2 < 0) {
		if (_vga_var3 != -1) {
			_vga_var3 = -1;
			if (++_vga_var2 == 0)
				return;
		}
	} else {
		if (_vga_var3 != 1) {
			_vga_var3 = 1;
			if (--_vga_var2 == 0)
				return;
		}
	}

	add_vga_timer(10, NULL, 0, 0);
}

void SimonEngine::vc_resume_sprite(byte *code_ptr, uint16 cur_file, uint16 cur_sprite) {
	VgaPointersEntry *vpe;

	_vga_cur_sprite_id = cur_sprite;

	_vga_cur_file_id = cur_file;
	_vga_cur_file_2 = cur_file;
	vpe = &_vga_buffer_pointers[cur_file];

	_cur_vga_file_1 = vpe->vgaFile1;
	_cur_vga_file_2 = vpe->vgaFile2;

	_vc_ptr = code_ptr;

	run_vga_script();
}

void SimonEngine::add_vga_timer(uint num, byte *code_ptr, uint cur_sprite, uint cur_file) {
	VgaTimerEntry *vte;

	_lock_word |= 1;

	for (vte = _vga_timer_list; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = cur_file;

	_lock_word &= ~1;
}

void SimonEngine::o_force_unlock() {
	if (_game & GF_SIMON2 && _bit_array[4] & 0x8000)
		_mouse_cursor = 0;
	_lock_counter = 0;
}

void SimonEngine::o_force_lock() {
	if (_game & GF_SIMON2) {
		_lock_word |= 0x8000;
		vc_34_force_lock();
		_lock_word &= ~0x8000;
	} else {
		_lock_word |= 0x4000;
		vc_34_force_lock();
		_lock_word &= ~0x4000;
	}
}

void SimonEngine::o_save_game() {
	save_or_load_dialog(false);
}

void SimonEngine::o_load_game() {
	save_or_load_dialog(true);
}

int SimonEngine::count_savegames() {
	File f;
	uint i = 1;

	while (i < 256) {
		f.open(gen_savename(i), getSavePath());
		if (f.isOpen() == false)
			break;

		f.close();
		i++;
	}
	return i;
}

int SimonEngine::display_savegame_list(int curpos, bool load, char *dst) {
	int slot, last_slot;
	File in;

	showMessageFormat("\xC");

	memset(dst, 0, 18 * 6);

	slot = curpos;

	while (curpos + 6 > slot) {
		in.open(gen_savename(slot), getSavePath());
		if (in.isOpen() == false)
			break;

		in.read(dst, 18);
		in.close();
		last_slot = slot;
		if (slot < 10)
			showMessageFormat(" ");
		showMessageFormat("%d", slot);
		showMessageFormat(".%s\n", dst);
		dst += 18;
		slot++;
	}
	// while_break
	if (!load) {
		if (curpos + 6 == slot)
			slot++;
		else {
			if (slot < 10)
				showMessageFormat(" ");
			showMessageFormat("%d.\n", slot);
		}
	} else {
		if (curpos + 6 == slot) {
			in.open(gen_savename(slot), getSavePath());
			if (in.isOpen() == true) {
				slot++;
				in.close();
			}
		}
	}

	return slot - curpos;
}

void SimonEngine::savegame_dialog(char *buf) {
	int i;

	o_unk_132_helper_3();

	i = display_savegame_list(_saveload_row_curpos, _save_or_load, buf);

	_savedialog_flag = true;

	if (i != 7) {
		i++;
		if (!_save_or_load)
			i++;
		_savedialog_flag = false;
	}

	if (!--i)
		return;

	do {
		clear_hitarea_bit_0x40(0xd0 + i - 1);
	} while (--i);
}

void SimonEngine::save_or_load_dialog(bool load) {
	time_t save_time;
	int number_of_savegames;
	int i;
	int unk132_result;
	FillOrCopyStruct *fcs;
	char *name;
	int name_len;
	bool b;
	char buf[108];

	_save_or_load = load;

	save_time = time(NULL);

	_copy_partial_mode = 1;

	number_of_savegames = count_savegames();
	if (!load)
		number_of_savegames++;
	number_of_savegames -= 6;
	if (number_of_savegames < 0)
		number_of_savegames = 0;
	number_of_savegames++;
	_num_savegame_rows = number_of_savegames;

	_saveload_row_curpos = 1;
	if (!load)
		_saveload_row_curpos = number_of_savegames;

	_saveload_flag = false;

restart:;
	do {
		i = o_unk_132_helper(&b, buf);
	} while (!b);

	if (i == 205)
		goto get_out;
	if (!load) {
		// if_1
	if_1:;
		unk132_result = i;

		set_hitarea_bit_0x40(0xd0 + i);
		leaveHitAreaById(0xd0 + i);

		// some code here

		fcs = _fcs_ptr_array_3[5];

		fcs->textRow = unk132_result;

		if (_language == 20) {
			// init x offset with a 2 character savegame number + a period (18 pix)
			fcs->textColumn = 3;
			fcs->textColumnOffset = 6;
			fcs->textLength = 3;
		} else {
			// init x offset with a 2 character savegame number + a period (18 pix)
			fcs->textColumn = 2;
			fcs->textColumnOffset = 2;
			fcs->textLength = 3;
		}

		name = buf + i * 18;

		// now process entire savegame name to get correct x offset for cursor
		name_len = 0;
		while (name[name_len]) {
			if (_language == 20) {
				byte width = 6;
				if (name[name_len] >= 64 && name[name_len] < 91)
					width = _hebrew_char_widths [name[name_len]-64];
				fcs->textLength++;
				fcs->textColumnOffset -= width;
				if (fcs->textColumnOffset >= width) {
					fcs->textColumnOffset += 8;
					fcs->textColumn++;
				}
			} else {
				fcs->textLength++;
				fcs->textColumnOffset += 6;
				if (name[name_len] == 'i' || name[name_len] == 'l')
					fcs->textColumnOffset -= 2;
				if (fcs->textColumnOffset >= 8) {
					fcs->textColumnOffset -= 8;
					fcs->textColumn++;
				}
				name_len++;
			}
		}
		// while_1_end

		// do_3_start
		for (;;) {
			video_putchar(fcs, 0x7f);

			_saveload_flag = true;

			// do_2
			do {
				i = o_unk_132_helper(&b, buf);

				if (b) {
					if (i == 205)
						goto get_out;
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					if (_saveload_flag) {
						o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
						// move code
					}
					goto if_1;
				}

				// is_not_b
				if (!_saveload_flag) {
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					goto restart;
				}
			} while (i >= 0x80 || i == 0);

			// after_do_2
			o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
			if (i == 10 || i == 13)
				break;
			if (i == 8) {
				// do_backspace
				if (name_len != 0) {
					int x;

					name_len--;

					x = (name[name_len] == 'i' || name[name_len] == 'l') ? 1 : 8;
					name[name_len] = 0;

					o_unk_132_helper_2(_fcs_ptr_array_3[5], x);
				}
			} else if (i >= 32 && name_len != 17) {
				name[name_len++] = i;

				video_putchar(_fcs_ptr_array_3[5], i);
			}
		}

		// do_save
		if (!save_game(_saveload_row_curpos + unk132_result, buf + unk132_result * 18))
			warning("Save failed");
	} else {
		if (!load_game(_saveload_row_curpos + i))
			warning("Load failed");
	}

get_out:;
	o_unk_132_helper_3();

	_base_time = time(NULL) - save_time + _base_time;
	_copy_partial_mode = 0;

	dx_copy_rgn_from_3_to_2(94, 208, 46, 80);

	i = _timer_4;
	do {
		delay(10);
	} while (i == _timer_4);

#ifdef _WIN32_WCE

	if (draw_keyboard) {
		draw_keyboard = false;
		toolbar_drawn = false;
	}

#endif
}

void SimonEngine::o_wait_for_vga(uint a) {
	_vga_wait_for = a;
	_timer_1 = 0;
	_exit_cutscene = false;
	_skip_speech = false;
	while (_vga_wait_for != 0) {
		if (_skip_speech && _game & GF_SIMON2) {
			if (_vga_wait_for == 200 && !vc_get_bit(14)) {
				skip_speech();
				break;
			}
		} else if (_exit_cutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);

		if (_game & GF_SIMON2) {
			if (_timer_1 >= 1000) {
				warning("wait timed out");
				break;
			}
		} else if (_timer_1 >= 500) {
			warning("wait timed out");
			break;
		}

	}
}

void SimonEngine::skip_speech() {
	_sound->stopVoice();
	if (!(_bit_array[1] & 0x1000)) {
		_bit_array[0] |= 0x4000;
		_variableArray[100] = 5;
		start_vga_code(4, 1, 0x1e, 0, 0, 0);
		o_wait_for_vga(0x82);
		o_kill_sprite_simon2(2, 1);
	}       
}

void SimonEngine::timer_vga_sprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5];							// parameters to vc_10

	if (_video_var_9 == 2)
		_video_var_9 = 1;

	if (_continous_vgascript)
		fprintf(_dump_file, "***\n");

	if (_game & GF_SIMON2 && _vga_var3) {
		timer_vga_sprites_helper();
	}

	vsp = _vga_sprites;
	while (vsp->id != 0) {
		vsp->unk6 &= 0x7FFF;

		vpe = &_vga_buffer_pointers[vsp->unk7];
		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		_video_palette_mode = vsp->unk6;
		_vga_cur_sprite_id = vsp->id;

		params[0] = READ_BE_UINT16(&vsp->image);
		params[1] = READ_BE_UINT16(&vsp->base_color);
		params[2] = READ_BE_UINT16(&vsp->x);
		params[3] = READ_BE_UINT16(&vsp->y);

		if (_game & GF_SIMON2) {
			*(byte *)(&params[4]) = (byte)vsp->unk4;
		} else {
			params[4] = READ_BE_UINT16(&vsp->unk4);
		}

		_vc_ptr = (byte *)params;
		vc_10_draw();

		vsp++;
	}

	if (_draw_images_debug)
		memset(_sdl_buf_attached, 0, 320 * 200);

	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}

void SimonEngine::timer_vga_sprites_helper() {
	byte *dst = dx_lock_2(), *src;
	uint x;

	if (_vga_var3 < 0) {
		memmove(dst + 8, dst, 320 * _vga_var5 - 8);
	} else {
		memmove(dst, dst + 8, 320 * _vga_var5 - 8);
	}

	x = _x_scroll - 1;

	if (_vga_var3 > 0) {
		dst += 320 - 8;
		x += 41;
	}

	src = _vga_var7 + x * 4;
	decodeStripA(dst, src + READ_BE_UINT32(&*((uint32 *)src)), _vga_var5);

	dx_unlock_2();


	memcpy(_sdl_buf_attached, _sdl_buf, 320 * 200);
	dx_copy_from_attached_to_3(_vga_var5);


	_x_scroll += _vga_var3;

	vc_write_var(0xfB, _x_scroll);

	_vga_var3 = 0;
}

void SimonEngine::timer_vga_sprites_2() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5];							// parameters to vc_10_draw

	if (_video_var_9 == 2)
		_video_var_9 = 1;

	vsp = _vga_sprites;
	while (vsp->id != 0) {
		vsp->unk6 &= 0x7FFF;

		vpe = &_vga_buffer_pointers[vsp->unk7];
		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		_video_palette_mode = vsp->unk6;
		_vga_cur_sprite_id = vsp->id;

		if (vsp->image)
			fprintf(_dump_file, "id:%5d image:%3d base-color:%3d x:%3d y:%3d flags:%x\n",
							vsp->id, vsp->image, vsp->base_color, vsp->x, vsp->y, vsp->unk4);
		params[0] = READ_BE_UINT16(&vsp->image);
		params[1] = READ_BE_UINT16(&vsp->base_color);
		params[2] = READ_BE_UINT16(&vsp->x);
		params[3] = READ_BE_UINT16(&vsp->y);
		params[4] = READ_BE_UINT16(&vsp->unk4);
		_vc_ptr = (byte *)params;
		vc_10_draw();

		vsp++;
	}

	fprintf(_dump_file, "***\n");

	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}

void SimonEngine::timer_proc1() {
	_timer_4++;

	if (_game & GF_SIMON2) {
		if (_lock_word & 0x80E9 || _lock_word & 2)
			return;
	} else {
		if (_lock_word & 0xC0E9 || _lock_word & 2)
			return;
	}

	_timer_1++;

	_lock_word |= 2;

	if (!(_lock_word & 0x10)) {
		if (!(_game & GF_SIMON2)) {
			expire_vga_timers();
			expire_vga_timers();
			_sync_flag_2 ^= 1;
			_cepe_flag ^= 1;
			if (!_cepe_flag)
				expire_vga_timers();

		} else {
			_sync_flag_2 ^= 1;
			if (!_sync_flag_2)
				expire_vga_timers();

		}

		if (_lock_counter != 0 && !_sync_flag_2) {
			_lock_word &= ~2;
			return;
		}
	}

	timer_vga_sprites();
	if (_draw_images_debug)
		timer_vga_sprites_2();

	if (_copy_partial_mode == 1) {
		dx_copy_from_2_to_attached(80, 46, 208 - 80, 94 - 46);
	}

	if (_copy_partial_mode == 2) {
		// copy partial from attached to 2
		dx_copy_from_attached_to_2(176, 61, 320 - 176, 134 - 61);
		_copy_partial_mode = 0;
	}

	if (_video_var_8) {
		handle_mouse_moved();
		dx_update_screen_and_palette();
		_sync_flag_1 = false;
		_video_var_8 = false;
	}

	_lock_word &= ~2;
}

void SimonEngine::timer_callback() {
//	uint32 start, end;

	if (_timer_5 != 0) {
		_sync_flag_2 = true;
		_timer_5--;
	} else {
//		start = _system->get_msecs();
		timer_proc1();
//		end = _system->get_msecs();

//		if (start + 45 < end) {
//			_timer_5 = (uint16)( (end - start) / 45);
//		}
	}
}

void SimonEngine::fcs_setTextColor(FillOrCopyStruct *fcs, uint value) {
	fcs->text_color = value;
}

void SimonEngine::o_vga_reset() {
	if (_game & GF_SIMON2) {
		_lock_word |= 0x8000;
		vc_27_reset();
		_lock_word &= ~0x8000;
	} else {
		_lock_word |= 0x4000;
		vc_27_reset();
		_lock_word &= ~0x4000;
	}	
}

bool SimonEngine::itemIsSiblingOf(uint16 a) {
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;

	return getItem1Ptr()->parent == item->parent;
}

bool SimonEngine::itemIsParentOf(uint16 a, uint16 b) {
	Item *item_a, *item_b;

	CHECK_BOUNDS(a, _vc_item_array);
	CHECK_BOUNDS(b, _vc_item_array);

	item_a = _vc_item_array[a];
	item_b = _vc_item_array[b];

	if (item_a == NULL || item_b == NULL)
		return true;

	return derefItem(item_a->parent) == item_b;
}

bool SimonEngine::vc_maybe_skip_proc_1(uint16 a, int16 b) {
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;
	return item->unk3 == b;
}

// OK
void SimonEngine::fcs_delete(uint a) {
	if (_fcs_ptr_array_3[a] == NULL)
		return;
	fcs_unk1(a);
	video_copy_if_flag_0x8_c(_fcs_ptr_array_3[a]);
	_fcs_ptr_array_3[a] = NULL;
	if (_fcs_unk_1 == a) {
		_fcs_ptr_1 = NULL;
		fcs_unk_2(0);
	}
}

// OK
void SimonEngine::fcs_unk_2(uint a) {
	a &= 7;

	if (_fcs_ptr_array_3[a] == NULL || _fcs_unk_1 == a)
		return;

	_fcs_unk_1 = a;
	startUp_helper_3();
	_fcs_ptr_1 = _fcs_ptr_array_3[a];

	showmessage_helper_3(_fcs_ptr_1->textLength, _fcs_ptr_1->textMaxLength);
}

// OK
FillOrCopyStruct *SimonEngine::fcs_alloc(uint x, uint y, uint w, uint h, uint flags, uint fill_color,
																				uint unk4) {
	FillOrCopyStruct *fcs;

	fcs = _fcs_list;
	while (fcs->mode != 0)
		fcs++;

	fcs->mode = 2;
	fcs->x = x;
	fcs->y = y;
	fcs->width = w;
	fcs->height = h;
	fcs->flags = flags;
	fcs->fill_color = fill_color;
	fcs->text_color = unk4;
	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textMaxLength = fcs->width * 8 / 6; // characters are 6 pixels
	return fcs;
}

Item *SimonEngine::derefItem(uint item) {
	if (item >= _itemarray_size)
		error("derefItem: invalid item %d", item);
	return _itemarray_ptr[item];
}

uint SimonEngine::itemPtrToID(Item *id) {
	uint i;
	for (i = 0; i != _itemarray_size; i++)
		if (_itemarray_ptr[i] == id)
			return i;
	error("itemPtrToID: not found");
}

void SimonEngine::o_pathfind(int x, int y, uint var_1, uint var_2) {
	uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;

	if (_game & GF_SIMON2) {
		x += _x_scroll * 8;
	}

	prev_i = 21 - _variableArray[12];
	for (i = 20; i != 0; --i) {
		p = (uint16 *)_pathfind_array[20 - i];
		if (!p)
			continue;
		for (j = 0; READ_BE_UINT16(&p[0]) != 999; j++, p += 2) {	// 0xE703 = byteswapped 999
			x_diff = abs((int)(READ_BE_UINT16(&p[0]) - x));
			y_diff = abs((int)(READ_BE_UINT16(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff >>= 2;
				y_diff <<= 2;
			}
			x_diff += y_diff >> 2;

			if (x_diff < best_dist || x_diff == best_dist && prev_i == i) {
				best_dist = x_diff;
				best_i = 21 - i;
				best_j = j;
			}
		}
	}

	_variableArray[var_1] = best_i;
	_variableArray[var_2] = best_j;
}

// ok
void SimonEngine::fcs_unk1(uint fcs_index) {
	FillOrCopyStruct *fcs;
	uint16 fcsunk1;
	uint16 i;

	fcs = _fcs_ptr_array_3[fcs_index & 7];
	fcsunk1 = _fcs_unk_1;

	if (fcs == NULL || fcs->fcs_data == NULL)
		return;

	fcs_unk_2(fcs_index);
	fcs_putchar(12);
	fcs_unk_2(fcsunk1);

	for (i = 0; fcs->fcs_data->e[i].item != NULL; i++) {
		delete_hitarea_by_index(fcs->fcs_data->e[i].hit_area);
	}

	if (fcs->fcs_data->unk3 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk3);
	}

	if (fcs->fcs_data->unk4 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk4);
		fcs_unk_5(fcs, fcs_index);
	}

	free(fcs->fcs_data);
	fcs->fcs_data = NULL;

	_fcs_data_1[fcs_index] = 0;
	_fcs_data_2[fcs_index] = 0;
}

// ok
void SimonEngine::fcs_unk_5(FillOrCopyStruct *fcs, uint fcs_index) {
	if (_game == GAME_SIMON1WIN) {
		o_kill_sprite_simon1(0x80);
	} else if (!(_game & GF_SIMON2)) {
		o_kill_sprite_simon1(0x81);
		start_vga_code(0, 1, 0x81, 0, 0, 0xE);
	}
}

void SimonEngine::delete_hitarea_by_index(uint index) {
	CHECK_BOUNDS(index, _hit_areas);
	_hit_areas[index].flags = 0;
}

// ok
void SimonEngine::fcs_putchar(uint a) {
	if (_fcs_ptr_1 != _fcs_ptr_array_3[0])
		video_putchar(_fcs_ptr_1, a);
}

// ok
void SimonEngine::video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs) {
	if (fcs->flags & 0x10)
		copy_img_from_3_to_2(fcs);
	else
		video_erase(fcs);

	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textLength = 0;
}

// ok
void SimonEngine::copy_img_from_3_to_2(FillOrCopyStruct *fcs) {
	_lock_word |= 0x8000;

	if (!(_game & GF_SIMON2)) {
		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8 + ((fcs == _fcs_ptr_array_3[2]) ? 1 : 0), (fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	} else {
		if (_vga_var6 && _fcs_ptr_array_3[2] == fcs) {
			fcs = _fcs_ptr_array_3[0x18 / 4];
			_vga_var6 = 0;
		}

		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8, (fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	}

	_lock_word &= ~0x8000;
}

void SimonEngine::video_erase(FillOrCopyStruct *fcs) {
	byte *dst;
	uint h;

	_lock_word |= 0x8000;

	dst = dx_lock_2();
	dst += _dx_surface_pitch * fcs->y + fcs->x * 8;

	h = fcs->height * 8;
	do {
		memset(dst, fcs->fill_color, fcs->width * 8);
		dst += _dx_surface_pitch;
	} while (--h);

	dx_unlock_2();
	_lock_word &= ~0x8000;
}

VgaSprite *SimonEngine::find_cur_sprite() {
	VgaSprite *vsp = _vga_sprites;
	while (vsp->id) {
		if (_game & GF_SIMON2) {
			if (vsp->id == _vga_cur_sprite_id && vsp->unk7 == _vga_cur_file_id)
				break;
		} else {
			if (vsp->id == _vga_cur_sprite_id)
				break;
		}
		vsp++;
	}
	return vsp;
}

bool SimonEngine::has_vgastruct_with_id(uint16 id, uint16 file) {
	VgaSprite *vsp = _vga_sprites;
	while (vsp->id) {
		if (_game & GF_SIMON2) {
			if (vsp->id == id && vsp->unk7 == file)
				return true;
		} else {
			if (vsp->id == id)
				return true;
		}
		vsp++;
	}
	return false;
}

void SimonEngine::processSpecialKeys() {
	switch (_key_pressed) {
		case 27: // escape
			_exit_cutscene = true;
			break;

		case 59: // F1
			vc_write_var(5, 50);
			vc_write_var(86, 0);
			break;
		case 60: // F2
			vc_write_var(5, 75);
			vc_write_var(86, 1);
			break;
		case 61: // F3
			vc_write_var(5, 125);
			vc_write_var(86, 2);
			break;
		case 63: // F5
			if (_game & GF_SIMON2)
				_exit_cutscene = true;
			break;

		case 't':
			if (_game & GF_SIMON2 && _game & GF_TALKIE || _game & GF_TALKIE && _language > 1)
				_subtitles ^= 1;
			break;

		case '+':
			midi.set_volume(midi.get_volume() + 16);
			break;

		case '-':
			midi.set_volume(midi.get_volume() - 16);
			break;

		case 'm':
			midi.pause(_music_paused ^= 1);
			break;

		case 's':
			if (_game == GAME_SIMON1DOS)
				midi._enable_sfx ^= 1;
			else
				_sound->effectsPause(_effects_paused ^= 1);
			break;

		case 'b':
			_sound->ambientPause(_ambient_paused ^= 1);
			break;

		case 'r':
			if (_debugMode)
				_start_mainscript ^= 1;
			break;

		case 'o':
			if (_debugMode)
				_continous_mainscript ^= 1;
			break;

		case 'v':
			if (_debugMode)
				_continous_vgascript ^= 1;
			break;
		case 'i':
			if (_debugMode)
				_draw_images_debug ^= 1;
			break;
	}
	
	_key_pressed = 0;
}

static const byte _simon1_cursor[256] = {
	0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe1,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe1,0xe1,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xe1,0xe1,0xe1,0xe0,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xe1,0xff,0xff,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xe1,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

static const byte _simon2_cursors[10][256] = {
	// cross hair
	{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xec,0xec,0xec,0xec,0xec,0xef,0xff,0xea,0xff,0xef,0xec,0xec,0xec,0xec,0xec,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xec,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// examine
	{ 0xff,0xff,0xef,0xef,0xef,0xef,0xef,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xef,0xee,0xeb,0xe4,0xe4,0xe4,0xee,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xee,0xeb,0xee,0xef,0xef,0xee,0xec,0xee,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xeb,0xee,0xef,0xee,0xee,0xef,0xee,0xe4,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xeb,0xef,0xef,0xef,0xec,0xee,0xef,0xe4,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xeb,0xef,0xef,0xee,0xef,0xef,0xef,0xe4,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xeb,0xee,0xef,0xef,0xef,0xef,0xee,0xe4,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xef,0xee,0xeb,0xee,0xef,0xef,0xee,0xe4,0xee,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xef,0xee,0xeb,0xeb,0xeb,0xeb,0xee,0xe4,0xec,0xef,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xef,0xef,0xef,0xef,0xef,0xef,0xeb,0xe4,0xee,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xee,0xe4,0xeb,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xeb,0xe4,0xeb,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xeb,0xec,0xeb,0xef,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xeb,0xe4,0xef,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// pick up  
	{ 0xff,0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe6,0xe6,0xe7,0xe7,0xe6,0xe6,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe7,0xe7,0xe7,0xe7,0xe8,0xe8,0xe8,0xe8,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xe5,0xe6,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe8,0xe9,0xe7,0xe5,0xff,0xff,
	  0xff,0xe5,0xe6,0xe7,0xe6,0xe5,0xff,0xff,0xff,0xff,0xe5,0xe6,0xe8,0xe6,0xe5,0xff,
	  0xff,0xe5,0xe7,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe8,0xe7,0xe5,0xff,
	  0xff,0xe5,0xe7,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe7,0xe7,0xe5,0xff,
	  0xff,0xef,0xeb,0xeb,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xeb,0xeb,0xef,0xff,
	  0xff,0xef,0xee,0xeb,0xee,0xef,0xff,0xff,0xff,0xff,0xef,0xee,0xeb,0xee,0xef,0xff,
	  0xff,0xff,0xef,0xeb,0xeb,0xef,0xff,0xff,0xff,0xff,0xef,0xeb,0xeb,0xef,0xff,0xff,
	  0xff,0xff,0xef,0xee,0xe4,0xee,0xef,0xff,0xff,0xef,0xee,0xe4,0xee,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xe4,0xeb,0xef,0xff,0xff,0xef,0xeb,0xe4,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xeb,0xeb,0xeb,0xef,0xef,0xeb,0xeb,0xeb,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xef,0xee,0xee,0xee,0xee,0xe1,0xe1,0xef,0xff,0xff,0xff,0xe4,
	  0xef,0xee,0xeb,0xeb,0xeb,0xeb,0xeb,0xe4,0xe4,0xe4,0xe4,0xe4,0xe4,0xe4,0xeb,0xec,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xe4 },
	// give
	{ 0xff,0xff,0xff,0xff,0xff,0xe5,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe7,0xe8,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe9,0xe7,0xe8,0xe8,0xe8,0xe7,0xe9,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xe5,0xe7,0xea,0xe8,0xe8,0xe8,0xea,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe7,0xe8,0xe8,0xea,0xe9,0xea,0xe8,0xe8,0xe7,0xe5,0xff,0xff,0xff,0xff,
	  0xe5,0xe7,0xe9,0xe8,0xe8,0xe9,0xec,0xe9,0xe8,0xe8,0xe8,0xe7,0xe5,0xff,0xff,0xff,
	  0xe5,0xe7,0xe7,0xe9,0xe8,0xec,0xe9,0xec,0xe8,0xe9,0xe7,0xe6,0xe5,0xff,0xff,0xff,
	  0xe5,0xe7,0xe7,0xe8,0xec,0xe9,0xe9,0xe9,0xec,0xe7,0xe6,0xe6,0xe5,0xff,0xff,0xff,
	  0xe5,0xe7,0xe7,0xea,0xe8,0xe9,0xe9,0xe9,0xe7,0xec,0xec,0xe4,0xe5,0xff,0xff,0xff,
	  0xe5,0xe7,0xe7,0xe9,0xe7,0xe8,0xe9,0xe7,0xe6,0xec,0xe4,0xec,0xe4,0xef,0xff,0xff,
	  0xe5,0xe6,0xe7,0xe9,0xe7,0xe7,0xe8,0xe6,0xe6,0xe4,0xec,0xe4,0xec,0xe4,0xef,0xff,
	  0xff,0xe5,0xe6,0xe9,0xe7,0xe7,0xe8,0xe6,0xe6,0xe8,0xe4,0xec,0xe4,0xec,0xeb,0xff,
	  0xff,0xff,0xe5,0xe9,0xe7,0xe7,0xe8,0xe6,0xe6,0xe8,0xe6,0xe4,0xec,0xeb,0xef,0xff,
	  0xff,0xff,0xff,0xe8,0xe7,0xe7,0xe8,0xe6,0xe6,0xe7,0xff,0xef,0xeb,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe7,0xe8,0xe6,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe6,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// talk
	{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xe5,0xe7,0xe8,0xe8,0xe8,0xe7,0xe6,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe6,0xe9,0xea,0xe6,0xea,0xe9,0xe8,0xe9,0xe8,0xe7,0xe5,0xff,0xff,0xff,
	  0xff,0xe5,0xe7,0xe5,0xef,0xe5,0xec,0xea,0xe5,0xea,0xec,0xe5,0xe9,0xe6,0xff,0xff,
	  0xff,0xe5,0xe6,0xe5,0xef,0xef,0xef,0xe5,0xef,0xef,0xe5,0xef,0xef,0xe8,0xe5,0xff,
	  0xff,0xe5,0xe9,0xea,0xe5,0xe8,0xe7,0xe6,0xe6,0xe8,0xe7,0xe5,0xec,0xe9,0xe5,0xff,
	  0xff,0xe5,0xe9,0xe8,0xe5,0xe7,0xe8,0xe8,0xe9,0xe9,0xe8,0xe5,0xe9,0xe9,0xe5,0xff,
	  0xff,0xe5,0xe6,0xec,0xea,0xe5,0xe6,0xe6,0xe7,0xe7,0xe6,0xe5,0xec,0xe8,0xe5,0xff,
	  0xff,0xff,0xe5,0xe9,0xe8,0xe9,0xe5,0xe8,0xe5,0xe8,0xe5,0xe9,0xe9,0xe7,0xe5,0xff,
	  0xff,0xff,0xe5,0xe7,0xe9,0xec,0xe8,0xec,0xe8,0xec,0xe8,0xec,0xe8,0xe5,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe6,0xe8,0xe9,0xe9,0xe9,0xe9,0xe9,0xe8,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// use
	{ 0xff,0xff,0xff,0xff,0xff,0xee,0xe1,0xeb,0xee,0xef,0xef,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xef,0xef,0xef,0xe4,0xeb,0xee,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xeb,0xe4,0xe4,0xeb,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xeb,0xe4,0xec,0xe4,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xeb,0xeb,0xe4,0xe4,0xee,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xee,0xeb,0xeb,0xeb,0xe1,0xef,0xee,0xef,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe6,0xef,0xef,0xee,0xeb,0xeb,0xe4,0xee,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe6,0xff,0xff,0xff,0xef,0xeb,0xec,0xeb,0xef,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe6,0xe5,0xff,0xff,0xff,0xee,0xe4,0xeb,0xef,0xff,
	  0xff,0xff,0xff,0xe5,0xe5,0xe6,0xe5,0xff,0xff,0xff,0xff,0xef,0xee,0xef,0xff,0xff,
	  0xff,0xff,0xe5,0xe6,0xe8,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0xff,
	  0xff,0xe5,0xe6,0xe8,0xe6,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xe5,0xe6,0xe8,0xe6,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xe5,0xe6,0xe6,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// wear
	{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xeb,0xed,0xe4,0xe2,0xeb,0xee,0xee,0xee,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xe2,0xec,0xe2,0xe1,0xee,0xef,0xef,0xee,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xeb,0xed,0xeb,0xee,0xef,0xef,0xef,0xee,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xee,0xe4,0xeb,0xee,0xef,0xef,0xee,0xef,0xef,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xef,0xe4,0xeb,0xee,0xef,0xef,0xee,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xef,0xe2,0xeb,0xee,0xef,0xef,0xee,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xef,0xeb,0xe1,0xee,0xef,0xef,0xee,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xef,0xeb,0xe1,0xee,0xef,0xef,0xef,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xef,0xef,0xef,0xe1,0xe4,0xe4,0xe4,0xe1,0xeb,0xee,0xef,0xef,0xef,0xff,0xff,
	  0xef,0xee,0xee,0xef,0xee,0xee,0xee,0xee,0xee,0xef,0xef,0xef,0xee,0xee,0xef,0xff,
	  0xff,0xef,0xef,0xee,0xe1,0xe2,0xe4,0xe4,0xe4,0xeb,0xe1,0xee,0xef,0xef,0xff,0xff,
	  0xff,0xff,0xff,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xef,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// move
	{ 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xff,
	  0xff,0xe1,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe1,0xff,
	  0xff,0xe1,0xe3,0xe3,0xe3,0xed,0xe3,0xe3,0xe3,0xe3,0xed,0xe3,0xe3,0xe3,0xe1,0xff,
	  0xff,0xe1,0xe3,0xe3,0xed,0xec,0xe3,0xe3,0xe3,0xe3,0xec,0xed,0xe3,0xe3,0xe1,0xff,
	  0xff,0xe1,0xe3,0xed,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xed,0xe3,0xe1,0xff,
	  0xff,0xe1,0xed,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xed,0xe1,0xff,
	  0xff,0xe1,0xe3,0xed,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xec,0xed,0xe3,0xe1,0xff,
	  0xff,0xe1,0xe3,0xe3,0xed,0xec,0xe3,0xe3,0xe3,0xe3,0xec,0xed,0xe3,0xe3,0xe1,0xff,
	  0xff,0xe1,0xe3,0xe3,0xe3,0xed,0xe3,0xe3,0xe3,0xe3,0xed,0xe3,0xe3,0xe3,0xe1,0xff,
	  0xff,0xe1,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe2,0xe1,0xff,
	  0xff,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xe1,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
	// open
	{ 0xff,0xff,0xe5,0xe8,0xe8,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xe5,0xe8,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xe5,0xe7,0xe5,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xff,0xe5,0xe7,0xe6,0xe9,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xe6,0xea,0xe6,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe6,0xea,0xe6,0xe7,0xe5,0xff,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe6,0xea,0xe6,0xff,0xe5,0xe7,0xe5,0xe7,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe6,0xea,0xe6,0xff,0xff,0xff,0xe5,0xe7,0xe8,0xe5,0xff,0xff,0xff,
	  0xff,0xe5,0xe6,0xea,0xe6,0xff,0xff,0xff,0xe5,0xe7,0xe8,0xe8,0xe5,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xea,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xe7,0xea,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe8,0xe8,0xe8,0xe8,0xe8,0xe7,0xe7,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe6,0xe6,0xe6,0xe6,0xe6,0xe6,0xe5,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe8,0xe8,0xe8,0xe8,0xe8,0xe8,0xe7,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xe5,0xe9,0xe9,0xe9,0xe9,0xe9,0xe9,0xe9,0xe9,0xe9,0xe5,0xff,0xff,0xff,0xff },
	// question mark
	{ 0xff,0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe7,0xea,0xec,0xec,0xec,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe7,0xea,0xec,0xea,0xe9,0xea,0xec,0xe9,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe9,0xec,0xe9,0xe8,0xe7,0xe8,0xea,0xec,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xe5,0xe8,0xe9,0xe8,0xe5,0xe5,0xe8,0xe9,0xec,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xe5,0xe8,0xe9,0xec,0xe9,0xe5,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe9,0xec,0xec,0xe9,0xe5,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe8,0xec,0xea,0xe8,0xe5,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe9,0xec,0xe9,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe9,0xea,0xe9,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe7,0xe9,0xe7,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe8,0xe9,0xe8,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe9,0xec,0xe9,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xe5,0xe8,0xe9,0xe8,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,
	  0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xe5,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff },
}; 

void SimonEngine::draw_mouse_pointer() {
	if (_game & GF_SIMON2)
		_system->set_mouse_cursor(_simon2_cursors[_mouse_cursor], 16, 16, 7, 7);
	else
		_system->set_mouse_cursor(_simon1_cursor, 16, 16, 0, 0);
}

void decompress_icon(byte *dst, byte *src, uint w, uint h_org, byte base, uint pitch) {
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = h_org;

	for (;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0)
				color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0)
				color_2 |= base;

			do {
				if (color_1 != 0)
					*dst = color_1;
				dst += pitch;
				if (color_2 != 0)
					*dst = color_2;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (++reps != 0);
		} else {
			do {
				color_1 = *src >> 4;
				if (color_1 != 0)
					*dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0)
					*dst = color_2 | base;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (--reps >= 0);
		}
	}
}


void SimonEngine::draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_lock_word |= 0x8000;
	dst = dx_lock_2();

	if (!(_game & GF_SIMON2)) {
		dst += (x + fcs->x) * 8;
		dst += (y * 25 + fcs->y) * _dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon]);

		decompress_icon(dst, src, 24, 12, 0xE0, _dx_surface_pitch);
	} else {
		dst += 110;
		dst += x;
		dst += (y + fcs->y) * _dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 0]);
		decompress_icon(dst, src, 20, 10, 0xE0, _dx_surface_pitch);

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 1]);
		decompress_icon(dst, src, 20, 10, 0xD0, _dx_surface_pitch);
	}

	dx_unlock_2();
	_lock_word &= ~0x8000;
}

void SimonEngine::video_toggle_colors(HitArea * ha, byte a, byte b, byte c, byte d) {
	byte *src, color;
	uint w, h, i;

	_lock_word |= 0x8000;
	src = dx_lock_2() + ha->y * _dx_surface_pitch + ha->x;

	w = ha->width;
	h = ha->height;

	// Works around bug in original Simon the Sorcerer 2
	// Animations continue in background when load/save dialog is open
	// often causing the savegame name highlighter to be cut short
	if (!(h > 0 && w > 0 && ha->x + w <= 320 && ha->y + h <= 200)) {
		debug(1,"Invalid coordinates in video_toggle_colors (%d,%d,%d,%d)", ha->x, ha->y, ha->width, ha->height);
		_lock_word &= ~0x8000;
		return;
	}

	do {
		for (i = 0; i != w; ++i) {
			color = src[i];
			if (a >= color && b < color) {
				if (c >= color)
					color += d;
				else
					color -= d;
				src[i] = color;
			}
		}
		src += _dx_surface_pitch;
	} while (--h);


	dx_unlock_2();
	_lock_word &= ~0x8000;
}

void SimonEngine::video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs) {
	if (fcs->flags & 8)
		copy_img_from_3_to_2(fcs);
	fcs->mode = 0;
}

void SimonEngine::start_vga_code(uint b, uint vga_res, uint vga_struct_id, uint c, uint d, uint f) {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	uint count;

	_lock_word |= 0x40;

	if (has_vgastruct_with_id(vga_struct_id, vga_res)) {
		_lock_word &= ~0x40;
		return;
	}

	vsp = _vga_sprites;
	while (vsp->id != 0)
		vsp++;

	vsp->unk6 = b;
	vsp->priority = 0;
	vsp->unk4 = 0;

	vsp->y = d;
	vsp->x = c;
	vsp->image = 0;
	vsp->base_color = f;
	vsp->id = vga_struct_id;
	vsp->unk7 = vga_res;

	for (;;) {
		vpe = &_vga_buffer_pointers[vga_res];
		_vga_cur_file_2 = vga_res;
		_cur_vga_file_1 = vpe->vgaFile1;
		if (vpe->vgaFile1 != NULL)
			break;
		ensureVgaResLoaded(vga_res);
	}

	pp = _cur_vga_file_1;
	p = pp + READ_BE_UINT16(&((VgaFile1Header *) pp)->hdr2_start);

	count = READ_BE_UINT16(&((VgaFile1Header2 *) p)->id_count);
	p = pp + READ_BE_UINT16(&((VgaFile1Header2 *) p)->id_table);

	for (;;) {
		if (READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->id) == vga_struct_id) {

			//dump_vga_script(pp + READ_BE_UINT16(&((VgaFile1Struct0x6*)p)->script_offs), vga_res, vga_struct_id);

			add_vga_timer(gss->VGA_DELAY_BASE, pp + READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->script_offs), vga_struct_id, vga_res);
			break;
		}
		p += sizeof(VgaFile1Struct0x6);
		if (!--count) {
			vsp->id = 0;
			break;
		}
	}

	_lock_word &= ~0x40;
}

void SimonEngine::talk_with_speech(uint speech_id, uint num_1) {
	if (!(_game & GF_SIMON2)) {
		if (speech_id == 9999) {
			if (_subtitles)
				return;
			if (!(_bit_array[0] & 0x4000) && !(_bit_array[1] & 0x1000)) {
				_bit_array[0] |= 0x4000;
				_variableArray[100] = 0xF;
				start_vga_code(4, 1, 0x82, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skip_vga_wait = true;
		} else {
			if (_subtitles && _scriptvar_2) {
				start_vga_code(4, 2, 204, 0, 0, 0);
				o_wait_for_vga(204);
				o_kill_sprite_simon1(204);
			}
			o_kill_sprite_simon1(num_1 + 201);
			_sound->playVoice(speech_id);
			start_vga_code(4, 2, num_1 + 201, 0, 0, 0);
		}
	} else {
		if (speech_id == 0xFFFF) {
			if (_subtitles)
				return;
			if (!(_bit_array[0] & 0x4000 || _bit_array[1] & 0x1000)) {
				_bit_array[0] |= 0x4000;
				_variableArray[100] = 5;
				start_vga_code(4, 1, 0x1e, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skip_vga_wait = true;
		} else {
			if (_subtitles && _scriptvar_2) {
				start_vga_code(4, 2, 5, 0, 0, 0);
				o_wait_for_vga(0xcd);
				o_kill_sprite_simon2(2, 5);
			}
			o_kill_sprite_simon2(2, num_1 + 2);
			_sound->playVoice(speech_id);
			start_vga_code(4, 2, num_1 + 2, 0, 0, 0);
		}
	}
}

void SimonEngine::talk_with_text(uint num_1, uint num_2, const char *string_ptr, uint threeval_a, int threeval_b, uint width) {
	char print_str_buf[0x140];
	char *char_buf;
	const char *string_ptr_2, *string_ptr_3;
	int j;
	uint letters_per_row, len_div_3, num_of_rows;
	uint m, n;
	uint height;

	// FIXME: Simon1dos Dwarf Mine - Fix text for dwarf song
	if (num_1 >= 100)
		num_1 -= 100;

	char_buf = print_str_buf;
	string_ptr_3 = string_ptr_2 = string_ptr;

	height = 10;
	j = 0;

	letters_per_row = width / 6;

	len_div_3 = (strlen(string_ptr) + 3) / 3;

	if (!(_game & GF_SIMON2)) {
		if (_variableArray[141] == 0)
			_variableArray[141] = 9;
		_variableArray[85] = _variableArray[141] * len_div_3;
	} else {
		if (_variableArray[86] == 0)
			len_div_3 >>= 1;
		if (_variableArray[86] == 2)
			len_div_3 <<= 1;
		_variableArray[85] = len_div_3 * 5;
	}

	num_of_rows = strlen(string_ptr) / letters_per_row;

	while (num_of_rows == 1 && j != -1) {
		m = strlen(string_ptr) >> 1;
		m -= j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row && strlen(string_ptr_2) < letters_per_row) {
			// if_1
			n = (letters_per_row - m + 1) >> 1;

			while (n != 0) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = 10;

			height += 10;
			threeval_b -= 10;
			j = -1;
		} else {
			// else_1
			j -= 4;
			if (j == -12) {
				j = 0;
				num_of_rows = 2;
			}
			string_ptr_2 = string_ptr_3;
		}
	}

	if (j != -1 && width * 30 > 8000)
		num_of_rows = 4;

	while (num_of_rows == 2 && j != -1) {
		m = strlen(string_ptr) / 3;
		m += j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row) {
			// if_4
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = 10;

			string_ptr = string_ptr_2;
			string_ptr_2 += m;

			while (*string_ptr_2-- != ' ' && m > 0)
				m--;
			// while_6_end

			string_ptr_2 += 2;

			if (strlen(string_ptr_2) <= m && m > 0) {
				// if_6
				n = (letters_per_row - m + 1) >> 1;
				while (n) {
					*char_buf++ = ' ';
					n--;
				}
				strncpy(char_buf, string_ptr, m);
				char_buf += m;
				*char_buf++ = 10;
				height += 20;
				threeval_b -= 20;
				j = -1;
			} else {
				// else_6
				j += 2;
				string_ptr_2 = string_ptr_3;
				string_ptr = string_ptr_3;
				char_buf = print_str_buf;
			}
		} else {
			num_of_rows = 3;
			string_ptr_2 = string_ptr_3;
			string_ptr = string_ptr_3;
			char_buf = print_str_buf;
			j = 0;
		}
	}

	if (j != -1 && width * 40 > 8000)
		num_of_rows = 4;

	// while_8
	while (num_of_rows == 3 && j != -1) {
		m = strlen(string_ptr) >> 2;
		m += j;
		string_ptr_2 += m;
		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row) {
			// if_10
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = '\n';
			string_ptr = string_ptr_2;
			string_ptr_2 += m;
			while (*string_ptr_2-- != ' ' && m > 0)
				m--;
			string_ptr_2 += 2;

			if (strlen(string_ptr_2) < m * 2 && m > 0) {
				// if_11
				n = (letters_per_row - m + 1) >> 1;
				while (n) {
					*char_buf++ = ' ';
					n--;
				}
				strncpy(char_buf, string_ptr, m);
				char_buf += m;
				*char_buf++ = 10;
				string_ptr = string_ptr_2;
				string_ptr_2 += m;

				while (*string_ptr_2-- != ' ' && m > 0)
					m--;
				string_ptr_2 += 2;

				if (strlen(string_ptr_2) <= m && m > 0) {
					// if_15
					n = (letters_per_row - m + 1) >> 1;
					while (n) {
						*char_buf++ = ' ';
						n--;
					}
					strncpy(char_buf, string_ptr, m);
					char_buf += m;
					*char_buf++ = '\n';
					height += 30;
					threeval_b -= 30;
					j = -1;
				} else {
					// else_15
					j += 2;
					string_ptr_2 = string_ptr_3;
					string_ptr = string_ptr_3;
					char_buf = print_str_buf;
				}
			} else {
				// else_11
				j += 2;
				string_ptr_2 = string_ptr_3;
				string_ptr = string_ptr_3;
				char_buf = print_str_buf;
			}
		} else {
			// else_10
			num_of_rows = 4;
			string_ptr = string_ptr_3;
			string_ptr_2 = string_ptr_3;
			char_buf = print_str_buf;
		}
	}

	// while_8_end
	if (num_of_rows == 4) {
		while (strlen(string_ptr) > letters_per_row) {
			m = letters_per_row;
			string_ptr_2 += m;
			while (*string_ptr_2-- != ' ' && m)
				m--;
			string_ptr_2 += 2;
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = 10;
			height += 10;
			threeval_b -= 10;
			string_ptr = string_ptr_2;
		}
	}

	n = (letters_per_row - strlen(string_ptr_2) + 1) >> 1;
	while (n) {
		*char_buf++ = ' ';
		n--;
	}

	strcpy(char_buf, string_ptr_2);
	if (!(_game & GF_SIMON2)) {
		o_kill_sprite_simon1(199 + num_1);
	} else {
		o_kill_sprite_simon2(2, num_1);
	}
	num_2 = num_2 * 3 + 192;

	render_string(num_1, num_2, width, height, print_str_buf);
	num_of_rows = 4;
	if (!(_bit_array[8] & 0x20))
		num_of_rows = 3;

	if (threeval_b < 2)
		threeval_b = 2;

	if (!(_game & GF_SIMON2)) {
		start_vga_code(num_of_rows, 2, 199 + num_1, threeval_a >> 3, threeval_b, 12);
	} else {
		start_vga_code(num_of_rows, 2, num_1, threeval_a >> 3, threeval_b, 12);
	}
}

void SimonEngine::read_vga_from_datfile_1(uint vga_id) {
	if (_game & GF_AMIGA || _game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS) {
		File in;
		char buf[50];
		uint32 size;
		if (vga_id == 23) 
			vga_id = 112;
		if (vga_id == 328)
			vga_id = 119;

		if (_game == GAME_SIMON1CD32) {
			sprintf(buf, "0%d.out", vga_id); 
		} else if (_game == GAME_SIMON1AMIGA) {
			// TODO Add support for decruncher
			sprintf(buf, "0%d.pkd.out", vga_id); 
		} else {
			sprintf(buf, "0%d.VGA", vga_id); 
		}

		in.open(buf, _gameDataPath);
		if (in.isOpen() == false)
			error("read_vga_from_datfile_1: can't open %s", buf);

		size = in.size();

		if (in.read(_vga_buffer_pointers[11].vgaFile2, size) != size)
			error("read_vga_from_datfile_1: read failed");

		in.close();
	} else {
		uint32 offs_a = _game_offsets_ptr[vga_id];
		uint32 size = _game_offsets_ptr[vga_id + 1] - offs_a;

		resfile_read(_vga_buffer_pointers[11].vgaFile2, offs_a, size);
	}
}

byte *SimonEngine::read_vga_from_datfile_2(uint id) {
	if (_game & GF_AMIGA || _game == GAME_SIMON1DEMO || _game == GAME_SIMON1DOS) {
		File in;
		char buf[50];
		uint32 size;
		byte *dst;

		if (_game == GAME_SIMON1CD32) {
			sprintf(buf, "%.3d%d.out", id >> 1, (id & 1) + 1);
		} else if (_game == GAME_SIMON1AMIGA) {
			// TODO Add support for decruncher
			sprintf(buf, "%.3d%d.pkd.out", id >> 1, (id & 1) + 1);
		} else {
			sprintf(buf, "%.3d%d.VGA", id >> 1, (id & 1) + 1);
		}

		in.open(buf, _gameDataPath);
		if (in.isOpen() == false)
			error("read_vga_from_datfile_2: can't open %s", buf);

		size = in.size();

		dst = setup_vga_destination(size);

		if (in.read(dst, size) != size)
			error("read_vga_from_datfile_2: read failed");

		in.close();

		return dst;
	} else {
		uint32 offs_a = _game_offsets_ptr[id];
		uint32 size = _game_offsets_ptr[id + 1] - offs_a;
		byte *dst;

		dst = setup_vga_destination(size);
		resfile_read(dst, offs_a, size);

		return dst;
	}
}

void SimonEngine::resfile_read(void *dst, uint32 offs, uint32 size) {
	_game_file->seek(offs, SEEK_SET);
	if (_game_file->read(dst, size) != size)
		error("resfile_read(%d,%d) read failed", offs, size);
}

void SimonEngine::openGameFile() {
	if (!(_game & GF_AMIGA) && _game != GAME_SIMON1DEMO && _game != GAME_SIMON1DOS) {
		_game_file = new File();
		_game_file->open(gss->gme_filename, _gameDataPath);

		if (_game_file->isOpen() == false)
			error("Can't open game file '%s'", gss->gme_filename);

		uint32 size = _game_file->readUint32LE();

		_game_offsets_ptr = (uint32 *)malloc(size);
		if (_game_offsets_ptr == NULL)
			error("out of memory, game offsets");

		resfile_read(_game_offsets_ptr, 0, size);
#if defined(SCUMM_BIG_ENDIAN)
		for (uint r = 0; r < size / sizeof(uint32); r++)
			_game_offsets_ptr[r] = FROM_LE_32(_game_offsets_ptr[r]);
#endif
	}

	loadIconFile();

	vc_34_force_lock();
	
	startUp(1);
}

void SimonEngine::startUp(uint a) {
	if (a == 1)
		startUp_helper();
}

void SimonEngine::startUp_helper() {
	runSubroutine101();
	startUp_helper_2();
}

void SimonEngine::runSubroutine101() {
	Subroutine *sub;

	sub = getSubroutineByID(101);
	if (sub != NULL)
		startSubroutineEx(sub);

	startUp_helper_2();
}

void SimonEngine::dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x) {
	byte *dst, *src;
	uint i;

	dst = dx_lock_2();
	src = _sdl_buf_3;

	dst += y * _dx_surface_pitch;
	src += y * _dx_surface_pitch;

	while (y < b) {
		for (i = x; i < r; i++)
			dst[i] = src[i];
		y++;
		dst += _dx_surface_pitch;
		src += _dx_surface_pitch;
	}

	dx_unlock_2();
}

void SimonEngine::dx_clear_surfaces(uint num_lines) {
	memset(_sdl_buf_attached, 0, num_lines * 320);

	_system->copy_rect(_sdl_buf_attached, 320, 0, 0, 320, 200);

	if (_dx_use_3_or_4_for_lock) {
		memset(_sdl_buf, 0, num_lines * 320);
		memset(_sdl_buf_3, 0, num_lines * 320);
	}
}

void SimonEngine::dx_clear_attached_from_top(uint lines) {
	memset(_sdl_buf_attached, 0, lines * 320);
}

void SimonEngine::dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h) {
	uint offs = x + y * 320;
	byte *s = _sdl_buf_attached + offs;
	byte *d = _sdl_buf + offs;

	do {
		memcpy(d, s, w);
		d += 320;
		s += 320;
	} while (--h);
}

void SimonEngine::dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h) {
	uint offs = x + y * 320;
	byte *s = _sdl_buf + offs;
	byte *d = _sdl_buf_attached + offs;

	do {
		memcpy(d, s, w);
		d += 320;
		s += 320;
	} while (--h);
}

void SimonEngine::dx_copy_from_attached_to_3(uint lines) {
	memcpy(_sdl_buf_3, _sdl_buf_attached, lines * 320);
}

void SimonEngine::dx_update_screen_and_palette() {
	_num_screen_updates++;

	if (_palette_color_count == 0 && _video_var_9 == 1) {
		_video_var_9 = 0;
		if (memcmp(_palette, _palette_backup, 256 * 4) != 0) {
			memcpy(_palette_backup, _palette, 256 * 4);
			_system->set_palette(_palette, 0, 256);
		}
	}

	_system->copy_rect(_sdl_buf_attached, 320, 0, 0, 320, 200);
	_system->update_screen();

	memcpy(_sdl_buf_attached, _sdl_buf, 320 * 200);

	if (_palette_color_count != 0) {
		if (!(_game & GF_SIMON2) && _use_palette_delay) {
			delay(100);
			_use_palette_delay = false;
		}
		realizePalette();
	}
}

void SimonEngine::realizePalette() {
	_video_var_9 = false;
	memcpy(_palette_backup, _palette, 256 * 4);

	if (_palette_color_count & 0x8000) {
		fadeUpPalette();
	} else {
		_system->set_palette(_palette, 0, _palette_color_count);
	}

	_palette_color_count = 0;
}

void SimonEngine::fadeUpPalette() {
	bool done;

	_palette_color_count = (_palette_color_count & 0x7fff) / 4;

	memset(_video_buf_1, 0, _palette_color_count * sizeof(uint32));

	// This function is used by Simon 2 when riding the lion to the goblin
	// camp. Note that _palette_color_count is not 1024 in this scene, so
	// only part of the palette is faded up. But apparently that's enough,
	// as long as we make sure that the remaining palette colours aren't
	// completely ignored.

	if (_palette_color_count < _video_num_pal_colors)
		memcpy(_video_buf_1 + _palette_color_count * sizeof(uint32),
			_palette + _palette_color_count * sizeof(uint32),
			(_video_num_pal_colors - _palette_color_count) * sizeof(uint32));

	do {
		uint8 *src;
		byte *dst;
		int i;

		done = true;
	  	src = _palette;
		dst = _video_buf_1;

		for (i = 0; i < _palette_color_count; i++) {
			if (src[0] > dst[0]) {
				if (dst[0] > src[0] - 4)
					dst[0] = src[0];
				else
					dst[0] += 4;
				done = false;
			}
			if (src[1] > dst[1]) {
				if (dst[1] > src[1] - 4)
					dst[1] = src[1];
				else
					dst[1] += 4;
				done = false;
			}
			if (src[2] > dst[2]) {
				if (dst[2] > src[2] - 4)
					dst[2] = src[2];
				else
					dst[2] += 4;
				done = false;
			}
			dst += 4;
			src += 4;
		}

		_system->set_palette(_video_buf_1, 0, _video_num_pal_colors);
		_system->update_screen();
		delay(5);
 	} while (!done);
}

void SimonEngine::go() {
	if (!_dump_file)
		_dump_file = stdout;

	// allocate buffers
	_sdl_buf_3 = (byte *)calloc(320 * 200, 1);
	_sdl_buf = (byte *)calloc(320 * 200, 1);
	_sdl_buf_attached = (byte *)calloc(320 * 200, 1);

	allocItemHeap();
	allocTablesHeap();

	setup_vga_file_buf_pointers();

	_sound = new SimonSound(_game, gss, _gameDataPath, _mixer);

	loadGamePcFile(gss->gamepc_filename);

	addTimeEvent(0, 1);
	openGameFile();

	_last_music_played = -1;
	_vga_base_delay = 1;
	
	_start_mainscript = false;
	_continous_mainscript = false;
	_continous_vgascript = false;
	_draw_images_debug=false;

	if (_debugLevel == 2)
		_continous_mainscript = true;
	if (_debugLevel == 3)
		_continous_vgascript = true;
	if (_debugLevel == 4)
		_start_mainscript = true;

	if (_game & GF_TALKIE) {
		if (_game & GF_SIMON2) {
			// Allow choice in Hebrew version of Simon the Sorcerer 2
			if (_language == 20) {
				if (_noSubtitles)
					_subtitles = false;
			// Check for speech file in other versions of Simon the Sorcerer 2
			} else {
				if (_sound->hasVoice())
					_subtitles = false;
			}
		// English and German versions of Simon the Sorcerer 1 don't have full subtitles
		} else if (_language < 2) {
			_subtitles = false;
		// Allow choice in other versions of Simon the Sorcerer 1
		} else {
			if (_noSubtitles)
				_subtitles = false;
		}
	}

	if (_language == 4 || (_language > 5 && _language < 20))
		error("The only known versions are English, French, German, Hebrew, Italian and Spanish");

	while (1) {
		hitarea_stuff();
		handle_verb_clicked(_verb_hitarea);
		delay(100);
	}
}

void SimonEngine::shutdown() {
	if (_game_file) {
		delete _game_file;
		_game_file = NULL;
	}
	_system->quit();
}

void SimonEngine::delay(uint amount) {
	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	uint vga_period;

	if (_fast_mode)
	 	vga_period = 10;
	else if (_game & GF_SIMON2)
		vga_period = 45;
	else
		vga_period = 50;

	_rnd.getRandomNumber(2);

	do {
		while (!_in_callback && cur >= _last_vga_tick + vga_period) {
			_last_vga_tick += vga_period;

			// don't get too many frames behind
			if (cur >= _last_vga_tick + vga_period * 2)
				_last_vga_tick = cur;

			_in_callback = true;
			timer_callback();
			_in_callback = false;
		}

		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					if (event.kbd.flags==OSystem::KBD_CTRL) {
						if (event.kbd.keycode == 'f')
							_fast_mode ^= 1;
					}
					// Make sure backspace works right (this fixes a small issue on OS X)
					if (event.kbd.keycode == 8)
						_key_pressed = 8;
					else
						_key_pressed = (byte)event.kbd.ascii;
					break;

				case OSystem::EVENT_MOUSEMOVE:
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
					break;

					case OSystem::EVENT_LBUTTONDOWN:
					_left_button_down++;
#ifdef _WIN32_WCE
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					if (_game & GF_SIMON2)
 						_skip_speech = true;
					else
						_exit_cutscene = true;
 					break;

				case OSystem::EVENT_QUIT:
					_system->quit();
					break;
				
				default:
					break;
			}
		}

		if (amount == 0)
			break;

		{
			uint this_delay = _fast_mode ? 1 : 20;
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

bool SimonEngine::save_game(uint slot, char *caption) {
	File f;
	uint item_index, num_item, i, j;
	TimeEvent *te;

	_lock_word |= 0x100;

#ifndef _WIN32_WCE
	errno = 0;
#endif

	f.open(gen_savename(slot), getSavePath(), 2);
	if (f.isOpen() == false) {
		_lock_word &= ~0x100;
		return false;
	}

	f.write(caption, 0x12);

	f.writeUint32BE(_itemarray_inited - 1);
	f.writeUint32BE(0xFFFFFFFF);
	f.writeUint32BE(0);
	f.writeUint32BE(0);

	i = 0;
	for (te = _first_time_struct; te; te = te->next)
		i++;
	f.writeUint32BE(i);

	for (te = _first_time_struct; te; te = te->next) {
		f.writeUint32BE(te->time + _base_time);
		f.writeUint16BE(te->subroutine_id);
	}

	item_index = 1;
	for (num_item = _itemarray_inited - 1; num_item; num_item--) {
		Item *item = _itemarray_ptr[item_index++];

		f.writeUint16BE(item->parent);
		f.writeUint16BE(item->sibling);
		f.writeUint16BE(item->unk3);
		f.writeUint16BE(item->unk4);

		Child1 *child1 = (Child1 *)findChildOfType(item, 1);
		if (child1) {
			f.writeUint16BE(child1->fr2);
		}

		Child2 *child2 = (Child2 *)findChildOfType(item, 2);
		if (child2) {
			f.writeUint32BE(child2->avail_props);
			i = child2->avail_props & 1;

			for (j = 1; j < 16; j++) {
				if ((1 << j) & child2->avail_props) {
					f.writeUint16BE(child2->array[i++]);
				}
			}
		}

		Child9 *child9 = (Child9 *) findChildOfType(item, 9);
		if (child9) {
			for (i = 0; i != 4; i++) {
				f.writeUint16BE(child9->array[i]);
			}
		}
	}

	// write the 255 variables
	for (i = 0; i != 255; i++) {
		f.writeUint16BE(readVariable(i));
	}

	// write the items in array 6
	for (i = 0; i != 10; i++) {
		f.writeUint16BE(itemPtrToID(_item_array_6[i]));
	}

	// Write the bits in array 1 & 2
	for (i = 0; i != 32; i++)
		f.writeUint16BE(_bit_array[i]);

	f.close();

	_lock_word &= ~0x100;

	return true;
}

char *SimonEngine::gen_savename(int slot) {
	static char buf[256];

	if (_game & GF_SIMON2) {
	sprintf(buf, "simon2.%.3d", slot);
	} else {
	sprintf(buf, "simon1.%.3d", slot);
	}
	return buf;
}

bool SimonEngine::load_game(uint slot) {
	char ident[18];
	File f;
	uint num, item_index, i, j;

	_lock_word |= 0x100;

#ifndef _WIN32_WCE
	errno = 0;
#endif

	f.open(gen_savename(slot), getSavePath(), 1);
	if (f.isOpen() == false) {
		_lock_word &= ~0x100;
		return false;
	}

	f.read(ident, 18);

	num = f.readUint32BE();

	if (f.readUint32BE() != 0xFFFFFFFF || num != _itemarray_inited - 1) {
		f.close();
		_lock_word &= ~0x100;
		return false;
	}

	f.readUint32BE();
	f.readUint32BE();
	_no_parent_notify = true;


	// add all timers
	killAllTimers();
	for (num = f.readUint32BE(); num; num--) {
		uint32 timeout = f.readUint32BE();
		uint16 func_to_call = f.readUint16BE();
		addTimeEvent(timeout, func_to_call);
	}

	item_index = 1;
	for (num = _itemarray_inited - 1; num; num--) {
		Item *item = _itemarray_ptr[item_index++], *parent_item;

		uint parent = f.readUint16BE();
		uint sibling = f.readUint16BE();

		parent_item = derefItem(parent);

		setItemParent(item, parent_item);

		if (parent_item == NULL) {
			item->parent = parent;
			item->sibling = sibling;
		}

		item->unk3 = f.readUint16BE();
		item->unk4 = f.readUint16BE();

		Child1 *child1 = (Child1 *)findChildOfType(item, 1);
		if (child1 != NULL) {
			child1->fr2 = f.readUint16BE();
		}

		Child2 *child2 = (Child2 *)findChildOfType(item, 2);
		if (child2 != NULL) {
			child2->avail_props = f.readUint32BE();
			i = child2->avail_props & 1;

			for (j = 1; j < 16; j++) {
				if ((1 << j) & child2->avail_props) {
					child2->array[i++] = f.readUint16BE();
				}
			}
		}

		Child9 *child9 = (Child9 *) findChildOfType(item, 9);
		if (child9) {
			for (i = 0; i != 4; i++) {
				child9->array[i] = f.readUint16BE();
			}
		}
	}


	// read the 255 variables
	for (i = 0; i != 255; i++) {
		writeVariable(i, f.readUint16BE());
	}

	// write the items in array 6
	for (i = 0; i != 10; i++) {
		_item_array_6[i] = derefItem(f.readUint16BE());
	}

	// Write the bits in array 1 & 2
	for (i = 0; i != 32; i++)
		_bit_array[i] = f.readUint16BE();

	f.close();

	_no_parent_notify = false;

	_lock_word &= ~0x100;

#ifndef _WIN32_WCE
	if (errno != 0)
		error("load failed");
#endif

	return true;
}

void SimonEngine::loadMusic (uint music) {
	if (_game & GF_SIMON2) {        // Simon 2 music
		midi.stop();
		_game_file->seek(_game_offsets_ptr[gss->MUSIC_INDEX_BASE + music - 1], SEEK_SET);
		if (_game & GF_WIN) {	
			midi.loadMultipleSMF (_game_file);
		} else {
			midi.loadXMIDI (_game_file);
		}

		_last_music_played = music;
		_next_music_to_play = -1;
	} else { // Simon 1 music
		if (_game & GF_AMIGA) {
			if (_game != GAME_SIMON1CD32) {
				// TODO Add support for decruncher
				debug(5,"playMusic - Decrunch %dtune attempt", music);
			}
			// TODO Add Protracker support for simon1amiga/cd32
			debug(5,"playMusic - Load %dtune attempt", music);
		} else {
			midi.stop();
			midi.setLoop (true); // Must do this BEFORE loading music. (GMF may have its own override.)

			if (_game & GF_WIN) {	
				// FIXME: The very last music resource, a cymbal crash for when the
				// two demons crash into each other, should NOT be looped like the 
				// other music tracks. In simon1dos/talkie the GMF resource includes 
				// a loop override that acomplishes this, but there seems to be nothing 
				// for this in the SMF resources.
				if (music == 35)
					midi.setLoop (false);

				_game_file->seek(_game_offsets_ptr[gss->MUSIC_INDEX_BASE + music], SEEK_SET);
				midi.loadMultipleSMF (_game_file);
			} else if (_game & GF_TALKIE) {	
				_game_file->seek(_game_offsets_ptr[gss->MUSIC_INDEX_BASE + music], SEEK_SET);
				midi.loadSMF (_game_file, music);
			} else {
				char buf[50];
				File f;
				sprintf(buf, "MOD%d.MUS", music);
				f.open(buf, _gameDataPath);
				if (f.isOpen() == false) {
					warning("Can't load music from '%s'", buf);
					return;
				}
				if (_game & GF_DEMO)
					midi.loadS1D (&f);
				else
					midi.loadSMF (&f, music);
			}

			midi.startTrack (0);
		}
	}
}

byte *SimonEngine::dx_lock_2() {
	_dx_surface_pitch = 320;
	return _sdl_buf;
}

void SimonEngine::dx_unlock_2() {
}

byte *SimonEngine::dx_lock_attached() {
	_dx_surface_pitch = 320;
	return _dx_use_3_or_4_for_lock ? _sdl_buf_3 : _sdl_buf_attached;
}

void SimonEngine::dx_unlock_attached() {
}

void SimonEngine::set_volume(byte volume) {
	_mixer->setVolume(volume);
}

byte SimonEngine::getByte() {
	return *_code_ptr++;
}
