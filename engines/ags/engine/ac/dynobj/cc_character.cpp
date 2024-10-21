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

#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/global_character.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/script/cc_common.h" // cc_error
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// return the type name of the object
const char *CCCharacter::GetType() {
	return "Character";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
size_t CCCharacter::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t);
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
void CCCharacter::Serialize(const void *address, Stream *out) {
	const CharacterInfo *chaa = static_cast<const CharacterInfo *>(address);
	out->WriteInt32(chaa->index_id);
}

void CCCharacter::Unserialize(int index, Stream *in, size_t data_sz) {
	int num = in->ReadInt32();
	ccRegisterUnserializedObject(index, &_GP(game).chars[num], this);
}

uint8_t CCCharacter::ReadInt8(void *address, intptr_t offset) {
	const CharacterInfo *ci = static_cast<CharacterInfo *>(address);
	const int on_offset = 28 * sizeof(int32_t) /* first var group */
						  + 301 * sizeof(int16_t) /* inventory */ + sizeof(int16_t) * 2 /* two shorts */ + 40 /* name */ + 20 /* scrname */;
	if (offset == on_offset)
		return ci->on;
	cc_error("ScriptCharacter: unsupported 'char' variable offset %d", offset);
	return 0;
}

void CCCharacter::WriteInt8(void *address, intptr_t offset, uint8_t val) {
	CharacterInfo *ci = static_cast<CharacterInfo *>(address);
	const int on_offset = 28 * sizeof(int32_t) /* first var group */
						  + 301 * sizeof(int16_t) /* inventory */ + sizeof(int16_t) * 2 /* two shorts */ + 40 /* name */ + 20 /* scrname */;
	if (offset == on_offset)
		ci->on = val;
	else
		cc_error("ScriptCharacter: unsupported 'char' variable offset %d", offset);
}

int16_t CCCharacter::ReadInt16(void *address, intptr_t offset) {
	const CharacterInfo *ci = static_cast<CharacterInfo *>(address);

	// Handle inventory fields
	const int invoffset = 112;
	if (offset >= invoffset && offset < (uint)(invoffset + MAX_INV * sizeof(short))) {
		return ci->inv[(offset - invoffset) / sizeof(short)];
	}

	switch (offset) {
	// +9 int32 = 36
	case 36:
		return ci->following;
	case 38:
		return ci->followinfo;
	// 40 +1 int32 = 44
	case 44:
		return ci->idletime;
	case 46:
		return ci->idleleft;
	case 48:
		return ci->transparency;
	case 50:
		return ci->baseline;
	// 52 +3 int32 = 64
	case 64:
		return ci->blinkview;
	case 66:
		return ci->blinkinterval;
	case 68:
		return ci->blinktimer;
	case 70:
		return ci->blinkframe;
	case 72:
		return ci->walkspeed_y;
	case 74:
		return ci->pic_yoffs;
	// 76 +2 int32 = 84
	case 84:
		return ci->speech_anim_speed;
	case 86:
		return ci->idle_anim_speed;
	case 88:
		return ci->blocking_width;
	case 90:
		return ci->blocking_height;
	// 92 +1 int32 = 96
	case 96:
		return ci->pic_xoffs;
	case 98:
		return ci->walkwaitcounter;
	case 100:
		return ci->loop;
	case 102:
		return ci->frame;
	case 104:
		return ci->walking;
	case 106:
		return ci->animating;
	case 108:
		return ci->walkspeed;
	case 110:
		return ci->animspeed;
	// 112 +301 int16 = 714 (skip inventory)
	case 714:
		return ci->actx;
	case 716:
		return ci->acty;
	default:
		cc_error("ScriptCharacter: unsupported 'short' variable offset %d", offset);
		return 0;
	}
}

void CCCharacter::WriteInt16(void *address, intptr_t offset, int16_t val) {
	CharacterInfo *ci = static_cast<CharacterInfo *>(address);

	// Detect when a game directly modifies the inventory, which causes the displayed
	// and actual inventory to diverge since 2.70. Force an update of the displayed
	// inventory for older games that rely on this behaviour.
	const int invoffset = 112;
	if (offset >= invoffset && offset < (uint)(invoffset + MAX_INV * sizeof(short))) {
		ci->inv[(offset - invoffset) / sizeof(short)] = val;
		update_invorder();
		return;
	}

	// TODO: for safety, find out which of the following fields
	// must be readonly, and add assertions for them, i.e.:
	// cc_error("ScriptCharacter: attempt to write readonly 'short' variable at offset %d", offset);
	switch (offset) {
	// +9 int32 = 36
	case 36:
		ci->following = val;
		break;
	case 38:
		ci->followinfo = val;
		break;
	// 40 +1 int32 = 44
	case 44:
		ci->idletime = val;
		break;
	case 46:
		ci->idleleft = val;
		break;
	case 48:
		ci->transparency = val;
		break;
	case 50:
		ci->baseline = val;
		break;
	// 52 +3 int32 = 64
	case 64:
		ci->blinkview = val;
		break;
	case 66:
		ci->blinkinterval = val;
		break;
	case 68:
		ci->blinktimer = val;
		break;
	case 70:
		ci->blinkframe = val;
		break;
	case 72:
		ci->walkspeed_y = val;
		break;
	case 74:
		ci->pic_yoffs = val;
		break;
	// 76 +2 int32 = 84
	case 84:
		ci->speech_anim_speed = val;
		break;
	case 86:
		ci->idle_anim_speed = val;
		break;
	case 88:
		ci->blocking_width = val;
		break;
	case 90:
		ci->blocking_height = val;
		break;
	// 92 +1 int32 = 96
	case 96:
		ci->pic_xoffs = val;
		break;
	case 98:
		ci->walkwaitcounter = val;
		break;
	case 100:
		ci->loop = val;
		break;
	case 102:
		ci->frame = val;
		break;
	case 104:
		ci->walking = val;
		break;
	case 106:
		ci->animating = val;
		break;
	case 108:
		ci->walkspeed = val;
		break;
	case 110:
		ci->animspeed = val;
		break;
	// 112 +301 int16 = 714 (skip inventory)
	case 714:
		ci->actx = val;
		break;
	case 716:
		ci->acty = val;
		break;
	default:
		cc_error("ScriptCharacter: unsupported 'short' variable offset %d", offset);
		break;
	}
}

int32_t CCCharacter::ReadInt32(void *address, intptr_t offset) {
	const CharacterInfo *ci = static_cast<CharacterInfo *>(address);

	switch (offset) {
	case 0:
		return ci->defview;
	case 4:
		return ci->talkview;
	case 8:
		return ci->view;
	case 12:
		return ci->room;
	case 16:
		return ci->prevroom;
	case 20:
		return ci->x;
	case 24:
		return ci->y;
	case 28:
		return ci->wait;
	case 32:
		return ci->flags;
	// 36 +2 int16 = 40
	case 40:
		return ci->idleview;
	// 44 +4 int16 = 52
	case 52:
		return ci->activeinv;
	case 56:
		return ci->talkcolor;
	case 60:
		return ci->thinkview;
	// 64 +6 int16 = 76
	case 76:
		return ci->z;
	case 80:
		return ci->walkwait;
	// 84 +4 int16 = 100
	case 92:
		return ci->index_id;
	default:
		cc_error("ScriptCharacter: unsupported 'int' variable offset %d", offset);
		return 0;
	}
}

void CCCharacter::WriteInt32(void *address, intptr_t offset, int32_t val) {
	CharacterInfo *ci = static_cast<CharacterInfo *>(address);

	// TODO: for safety, find out which of the following fields
	// must be readonly, and add assertions for them, i.e.:
	// cc_error("ScriptCharacter: attempt to write readonly 'int' variable at offset %d", offset);
	switch (offset) {
	case 0:
		ci->defview = val;
		break;
	case 4:
		ci->talkview = val;
		break;
	case 8:
		ci->view = val;
		break;
	case 12:
		ci->room = val;
		break;
	case 16:
		ci->prevroom = val;
		break;
	case 20:
		ci->x = val;
		break;
	case 24:
		ci->y = val;
		break;
	case 28:
		ci->wait = val;
		break;
	case 32:
		ci->flags = val;
		break;
	// 36 +2 int16 = 40
	case 40:
		ci->idleview = val;
		break;
	// 44 +4 int16 = 52
	case 52:
		ci->activeinv = val;
		break;
	case 56:
		ci->talkcolor = val;
		break;
	case 60:
		ci->thinkview = val;
		break;
	// 64 +6 int16 = 76
	case 76:
		ci->z = val;
		break;
	case 80:
		ci->walkwait = val;
		break;
	// 84 +4 int16 = 100
	case 92:
		ci->index_id = val;
		break;
	default:
		cc_error("ScriptCharacter: unsupported 'int' variable offset %d", offset);
		break;
	}
}

} // namespace AGS3
