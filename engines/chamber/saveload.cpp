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

#include "common/textconsole.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/saveload.h"
#include "chamber/resdata.h"
#include "chamber/script.h"
#include "chamber/cga.h"
#include "chamber/room.h"
#include "chamber/dialog.h"
#include "chamber/cursor.h"

namespace Chamber {

static const byte kSaveVersion = 1;

// Serializes a pointer as a byte offset from base; 0xFFFF represents null
template<typename T>
static void syncPtrOffset(Common::Serializer &s, T *&ptr, void *base) {
	uint16 ofs = s.isSaving() ? (ptr ? (uint16)((byte *)ptr - (byte *)base) : 0xFFFF) : 0;
	s.syncAsUint16LE(ofs);
	if (s.isLoading())
		ptr = (ofs == 0xFFFF) ? nullptr : (T *)((byte *)base + ofs);
}

static void syncItem(Common::Serializer &s, item_t &it) {
	s.syncAsByte(it.flags);
	s.syncAsByte(it.area);
	s.syncAsByte(it.sprite);
	s.syncAsByte(it.name);
	s.syncAsUint16LE(it.command);
}

static void syncPers(Common::Serializer &s, pers_t &p) {
	s.syncAsByte(p.area);
	s.syncAsByte(p.flags);
	s.syncAsByte(p.name);
	s.syncAsByte(p.index);
	s.syncAsByte(p.item);
}

static void syncDirtyRect(Common::Serializer &s, dirty_rect_t &dr) {
	s.syncAsByte(dr.kind);
	s.syncAsUint16LE(dr.offs);
	s.syncAsByte(dr.height);
	s.syncAsByte(dr.width);
	s.syncAsByte(dr.y);
	s.syncAsByte(dr.x);
}

static void syncRect(Common::Serializer &s, rect_t &r) {
	s.syncAsByte(r.sx);
	s.syncAsByte(r.ex);
	s.syncAsByte(r.sy);
	s.syncAsByte(r.ey);
}

Common::Error ChamberEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	if (!s.syncVersion(kSaveVersion))
		return Common::Error(Common::kReadingFailed, "Save from a newer engine version");
	syncGameStream(s);
	if (s.err())
		return Common::kReadingFailed;
	// Prevent door transition animation from firing on load (mirrors restartGame behavior)
	script_byte_vars.cur_spot_flags = 0xFF;
	memset(backbuffer, 0, sizeof(backbuffer));
	drawRoomStatics();
	backupSpotsImages();
	drawPersons();
	_renderer->backBufferToRealFull();
	_renderer->selectCursor(CURSOR_FINGER);
	return Common::kNoError;
}

Common::Error ChamberEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	s.syncVersion(kSaveVersion);
	syncGameStream(s);
	if (s.err())
		return Common::kWritingFailed;
	return Common::kNoError;
}

void ChamberEngine::syncGameStream(Common::Serializer &s) {
	s.syncBytes((byte *)&script_byte_vars, sizeof(script_byte_vars));
	s.syncBytes((byte *)&script_word_vars, sizeof(script_word_vars));
	s.syncBytes(zones_data, RES_ZONES_MAX);

	for (int i = 0; i < MAX_INV_ITEMS; i++)
		syncItem(s, inventory_items[i]);
	for (int i = 0; i < PERS_MAX; i++)
		syncPers(s, pers_list[i]);
	for (int i = 0; i < MAX_DIRTY_RECT; i++)
		syncDirtyRect(s, dirty_rects[i]);

	for (int i = 0; i < SPECIAL_COMMANDS_MAX; i++) {
		s.syncAsUint16LE(menu_commands_12[i]);
		s.syncAsUint16LE(menu_commands_22[i]);
		s.syncAsUint16LE(menu_commands_24[i]);
		s.syncAsUint16LE(menu_commands_23[i]);
	}
	syncRect(s, room_bounds_rect);

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

int16 loadScena(void) {
	return g_vm->loadGameDialog() ? 0 : 1;
}

int16 saveScena(void) {
	return g_vm->saveGameDialog() ? 0 : 1;
}

void saveRestartGame(void) {
	warning("STUB: saveRestartGame()");
}

void restartGame(void) {
	warning("STUB: restartGame()");
	g_vm->_shouldRestart = true;
}

} // End of namespace Chamber
