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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/room302.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];       /* Sprite series handles */
	int16 sequence[10];     /* Sequence handles      */
	int16 animation[10];     /* Animation handles     */
	AnimationInfo animation_info[10];
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_302_init() {
}

static void room_302_daemon() {
}

static void room_302_pre_parser() {
}

static void room_302_parser() {
}

void room_302_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_302_preload() {
	room_init_code_pointer = room_302_init;
	room_pre_parser_code_pointer = room_302_pre_parser;
	room_parser_code_pointer = room_302_parser;
	room_daemon_code_pointer = room_302_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
