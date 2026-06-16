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

#include "mads/madsv2/engine.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/lib.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/btype.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/nebular/main_menu.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

bool new_background;
int selected_item;

void menu_control() {
	// TODO
	selected_item = 0;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
