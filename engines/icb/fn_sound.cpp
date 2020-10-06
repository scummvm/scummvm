/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound.h"

namespace ICB {

mcodeFunctionReturnCodes fn_play_sfx(int32 &result, int32 *params) { return (MS->fn_play_sfx(result, params)); }

mcodeFunctionReturnCodes fn_stop_sfx(int32 &result, int32 *params) { return (MS->fn_stop_sfx(result, params)); }

mcodeFunctionReturnCodes fn_play_sfx_xyz(int32 &result, int32 *params) { return (MS->fn_play_sfx_xyz(result, params)); }

mcodeFunctionReturnCodes fn_play_sfx_offset(int32 &result, int32 *params) { return (MS->fn_play_sfx_offset(result, params)); }

mcodeFunctionReturnCodes fn_play_sfx_time(int32 &result, int32 *params) { return (MS->fn_play_sfx_time(result, params)); }

mcodeFunctionReturnCodes fn_play_sfx_offset_time(int32 &result, int32 *params) { return (MS->fn_play_sfx_offset_time(result, params)); }

mcodeFunctionReturnCodes fn_play_sfx_special(int32 &result, int32 *params) { return (MS->fn_play_sfx_special(result, params)); }

mcodeFunctionReturnCodes fn_set_sfx(int32 &result, int32 *params) { return (MS->fn_set_sfx(result, params)); }

// fn_set_sfx("sfx name","sfx");
// where name can be:
// gunshot      0
// ricochet     1
// tinkle       2
// open         0
// close        1
mcodeFunctionReturnCodes _game_session::fn_set_sfx(int32 &, int32 *params) {
	int whichVar;

	const char *name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *sfx = (const char *)MemoryUtil::resolvePtr(params[1]);

	// none
	whichVar = -1;

	// check for them all
	if (strcmp(name, "gunshot") == 0)
		whichVar = GUNSHOT_SFX_VAR;
	if (strcmp(name, "ricochet") == 0)
		whichVar = RICOCHET_SFX_VAR;
	if (strcmp(name, "tinkle") == 0)
		whichVar = TINKLE_SFX_VAR;
	if (strcmp(name, "open") == 0)
		whichVar = OPEN_SFX_VAR;
	if (strcmp(name, "close") == 0)
		whichVar = CLOSE_SFX_VAR;

	if (whichVar == -1)
		Fatal_error("error message, in fn_sound.cpp, quite friendly and polite. You are trying to do "
		            "fn_set_sfx(\"%s\",\"%s\"), but I don't know what you mean by \"%s\". I support gunshot, ricochet, tinkle, "
		            "open and close\n",
		            name, sfx, name);

	logic_structs[cur_id]->sfxVars[whichVar] = HashString(sfx);

	return IR_CONT;
}

// fn_play_sfx_xyz("sfx","id");
mcodeFunctionReturnCodes _game_session::fn_play_sfx(int32 &, int32 *params) {
	const char *sfx = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[1]);

	RegisterSound(cur_id, sfx, id);
	return (IR_CONT);
}

// fn_play_sfx_xyz("sfx","id",x,y,z);
mcodeFunctionReturnCodes _game_session::fn_play_sfx_xyz(int32 &, int32 *params) {
	const char *sfx = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[1]);
	RegisterSoundAbsolute(cur_id, sfx, id, (PXreal)params[2], (PXreal)params[3], (PXreal)params[4]);
	return (IR_CONT);
}

// fn_play_sfx_offset("object","sound","id",x,y,z,is_nico);
mcodeFunctionReturnCodes _game_session::fn_play_sfx_offset(int32 &, int32 *params) {
	const char *obj = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *sound = (const char *)MemoryUtil::resolvePtr(params[1]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[2]);

	RegisterSoundOffset(cur_id, obj, sound, id, (PXreal)params[3], (PXreal)params[4], (PXreal)params[5], (int32)params[6], 0);
	return (IR_CONT);
}

// fn_play_sfx_time("sfx","id",time);
mcodeFunctionReturnCodes _game_session::fn_play_sfx_time(int32 &, int32 *params) {
	const char *sfx = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[1]);
	RegisterSoundTime(cur_id, sfx, id, (int32)params[2]);
	return (IR_CONT);
}

// fn_play_sfx_offset_time("object","sound","id",x,y,z,is_nico,time);
mcodeFunctionReturnCodes _game_session::fn_play_sfx_offset_time(int32 &, int32 *params) {
	const char *obj = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *sound = (const char *)MemoryUtil::resolvePtr(params[1]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[2]);

	RegisterSoundOffset(cur_id, obj, sound, id, (PXreal)params[3], (PXreal)params[4], (PXreal)params[5], (int32)params[6], (int32)params[7]);
	return (IR_CONT);
}

// fn_play_sfx_special("sfx","id",volume,pan);
mcodeFunctionReturnCodes _game_session::fn_play_sfx_special(int32 &, int32 *params) {
	const char *sfx = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *id = (const char *)MemoryUtil::resolvePtr(params[1]);

	int32 vol, pan;

	vol = params[2];
	pan = params[3];

	if ((vol < 0) || (vol > 127))
		Fatal_error("volume out of range in fn_play_sfx_special (you did %d max is 127)", vol);

	if ((pan < -127) || (pan > 127))
		Fatal_error("pan out of range in fn_play_sfx_special (you did %d range is -127-127)", pan);

	RegisterSoundSpecial(sfx, id, vol, pan);
	return (IR_CONT);
}

// fn_stop_sfx("id");
mcodeFunctionReturnCodes _game_session::fn_stop_sfx(int32 &, int32 *params) {
	// snd is full string
	const char *snd = (const char *)MemoryUtil::resolvePtr(params[0]);

	// see if :: in filename...
	const char *sub = strstr(const_cast<char *>(snd), "::");

	// if object name is given in form obj::snd then extract both parts
	if (sub != NULL) {
		char tempObj[64];
		char tempSnd[64];

		// first part (obj)
		strncpy(tempObj, const_cast<char *>(snd), sub - snd);
		tempObj[sub - snd] = 0; // null terminate

		strcpy(tempSnd, sub + strlen("::"));

		int obj = MS->objects->Fetch_item_number_by_name(tempObj);

		if (obj != -1)
			RemoveRegisteredSound(obj, tempSnd);
	}
	// otherwise sound is of this object
	else
		RemoveRegisteredSound(cur_id, snd);

	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_common_xa_music(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_common_xa_music() no longer supported");
	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_common_vag_music(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_common_vag_music() no longer supported");
	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_mission_xa_music(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_mission_xa_music() no longer supported");
	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_mission_vag_music(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_mission_vag_music() no longer supported");
	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_common_fx(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_common_fx() no longer supported");
	return (IR_CONT);
}

mcodeFunctionReturnCodes fn_play_mission_fx(int32 &, int32 * /*params*/) {
	Fatal_error("fn_play_mission_fx() no longer supported");
	return (IR_CONT);
}

} // End of namespace ICB
