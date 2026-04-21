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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/rendermode.h"
#include "common/system.h"

#include "engines/util.h"

#include "chamber/chamber.h"
#include "chamber/renderer.h"
#include "chamber/script.h"
#include "chamber/resdata.h"
#include "chamber/room.h"
#include "chamber/dialog.h"
#include "chamber/cga.h"

namespace Chamber {

ChamberEngine *g_vm;

ChamberEngine::ChamberEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst) {
	g_vm = this;
	_gameDescription = desc;

	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("chamber");

	_shouldQuit = false;
	_shouldRestart = false;
	_prioritycommand_1 = false;
	_prioritycommand_2 = false;
	_pxiData = NULL;

	_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	if (_renderMode == Common::kRenderEGA || _renderMode == Common::kRenderHercG || _renderMode == Common::kRenderHercA)
		_videoMode = _renderMode;
	else
		_videoMode = Common::kRenderCGA;

	if (_renderMode == Common::kRenderHercA)
		_videoMode = Common::kRenderHercG;

	_screenH = _screenW = _screenBits = _screenBPL = _screenPPB = 0;
	_line_offset = _line_offset2 = _fontHeight = _fontWidth = 0;

	if (_videoMode == Common::kRenderEGA)
		_renderer = new EGARenderer();
	else
		_renderer = new CGARenderer();
}

ChamberEngine::~ChamberEngine() {
	// Dispose your resources here
	delete _rnd;
	delete[] _pxiData;
	delete _renderer;

	deinitSound();
}

bool ChamberEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

// Serializes a pointer as a byte offset from base; 0xFFFF represents null
template<typename T>
static void syncPtrOffset(Common::Serializer &s, T *&ptr, void *base) {
	uint16 ofs = s.isSaving() ? (ptr ? (uint16)((byte *)ptr - (byte *)base) : 0xFFFF) : 0;
	s.syncAsUint16LE(ofs);
	if (s.isLoading())
		ptr = (ofs == 0xFFFF) ? nullptr : (T *)((byte *)base + ofs);
}

Common::Error ChamberEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	// Prevent door transition animation from firing on load (mirrors restartGame behavior)
	script_byte_vars.cur_spot_flags = 0xFF;
	memset(backbuffer, 0, sizeof(backbuffer));
	drawRoomStatics();
	blitSpritesToBackBuffer();
	_renderer->backBufferToRealFull();
	return Common::kNoError;
}

Common::Error ChamberEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void ChamberEngine::syncGameStream(Common::Serializer &s) {
	s.syncBytes((byte *)&script_byte_vars, sizeof(script_byte_vars));
	s.syncBytes((byte *)&script_word_vars, sizeof(script_word_vars));
	s.syncBytes(zones_data, RES_ZONES_MAX);
	s.syncBytes((byte *)inventory_items, sizeof(item_t) * MAX_INV_ITEMS);
	s.syncBytes((byte *)pers_list, sizeof(pers_t) * PERS_MAX);
	s.syncBytes((byte *)dirty_rects, sizeof(dirty_rect_t) * MAX_DIRTY_RECT);
	s.syncBytes((byte *)menu_commands_12, sizeof(uint16) * SPECIAL_COMMANDS_MAX);
	s.syncBytes((byte *)menu_commands_22, sizeof(uint16) * SPECIAL_COMMANDS_MAX);
	s.syncBytes((byte *)menu_commands_24, sizeof(uint16) * SPECIAL_COMMANDS_MAX);
	s.syncBytes((byte *)menu_commands_23, sizeof(uint16) * SPECIAL_COMMANDS_MAX);
	s.syncBytes((byte *)&room_bounds_rect, sizeof(rect_t));

	s.syncAsUint16LE(next_vorts_cmd);
	s.syncAsUint16LE(next_vorts_ticks);
	s.syncAsUint16LE(next_turkey_cmd);
	s.syncAsUint16LE(next_turkey_ticks);
	s.syncAsUint16LE(next_protozorqs_ticks);
	s.syncAsUint16LE(fight_pers_ofs);
	s.syncAsUint16LE(drops_cleanup_time);

	s.syncAsByte(wait_delta);
	s.syncAsByte(in_de_profundis);
	s.syncAsByte(zone_name);
	s.syncAsByte(room_hint_bar_width);
	s.syncAsByte(room_hint_bar_coords_x);
	s.syncAsByte(room_hint_bar_coords_y);
	s.syncAsByte(last_object_hint);
	s.syncAsByte(object_hint);
	s.syncAsByte(command_hint);
	s.syncAsByte(last_command_hint);
	s.syncAsByte(zone_spr_index);
	s.syncAsByte(zone_obj_count);

	for (int i = 0; i < 2; i++) {
		s.syncAsByte(the_wall_doors[i].height);
		s.syncAsByte(the_wall_doors[i].width);
		s.syncAsUint16LE(the_wall_doors[i].pitch);
		s.syncAsUint16LE(the_wall_doors[i].offs);
		syncPtrOffset(s, the_wall_doors[i].pixels, sprit_load_buffer);
	}

	syncPtrOffset(s, script_vars[kScrPool0_WordVars0], &script_word_vars);
	syncPtrOffset(s, script_vars[kScrPool1_WordVars1], &script_word_vars);
	syncPtrOffset(s, script_vars[kScrPool2_ByteVars], &script_byte_vars);
	syncPtrOffset(s, script_vars[kScrPool3_CurrentItem], inventory_items);
	syncPtrOffset(s, script_vars[kScrPool4_ZoneSpots], zones_data);
	syncPtrOffset(s, script_vars[kScrPool5_Persons], pers_list);
	syncPtrOffset(s, script_vars[kScrPool6_Inventory], inventory_items);
	syncPtrOffset(s, script_vars[kScrPool7_Zapstiks], inventory_items);
	syncPtrOffset(s, script_vars[kScrPool8_CurrentPers], pers_list);

	for (int i = 0; i < MAX_SPRITES; i++)
		syncPtrOffset(s, sprites_list[i], scratch_mem1);
	for (int i = 0; i < MAX_DOORS; i++)
		syncPtrOffset(s, doors_list[i], arpla_data);

	syncPtrOffset(s, zone_spots, zones_data);
	syncPtrOffset(s, zone_spots_end, zones_data);
	syncPtrOffset(s, zone_spots_cur, zones_data);

	syncPtrOffset(s, script_stack_ptr, script_stack);
	for (int i = 0; i < 5 * 2; i++)
		syncPtrOffset(s, script_stack[i], templ_data);

	syncPtrOffset(s, vort_ptr, pers_list);
	syncPtrOffset(s, vortanims_ptr, vortsanim_list);
	syncPtrOffset(s, turkeyanims_ptr, turkeyanim_list);
	syncPtrOffset(s, aspirant_ptr, pers_list);
	syncPtrOffset(s, aspirant_spot, zones_data);
	syncPtrOffset(s, found_spot, zones_data);
	syncPtrOffset(s, spot_sprite, sprites_list);

	uint16 timedSeqOfs = s.isSaving() ? (timed_seq_ptr ? (uint16)(timed_seq_ptr - patrol_route) : 0xFFFF) : 0;
	s.syncAsUint16LE(timedSeqOfs);
	if (s.isLoading())
		timed_seq_ptr = (timedSeqOfs == 0xFFFF) ? nullptr : patrol_route + timedSeqOfs;
}

int ChamberEngine::getX(int original_x) {
	return original_x;
}

int ChamberEngine::getY(int original_y) {
	return original_y;
}

} // End of namespace Chamber
