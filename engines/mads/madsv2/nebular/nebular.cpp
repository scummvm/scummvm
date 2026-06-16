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

#include "engines/util.h"
#include "mads/core/mps_installer.h"
#include "mads/madsv2/console.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/main.h"
#include "mads/madsv2/nebular/sound_nebular.h"
#include "mads/madsv2/nebular/rooms/section1.h"
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/section3.h"
#include "mads/madsv2/nebular/rooms/section4.h"
#include "mads/madsv2/nebular/rooms/section5.h"
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/rooms/section7.h"
#include "mads/madsv2/nebular/rooms/section8.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Common::Error RexNebularEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up to read mpslabs installer archive if needed
	if (_gameDescription->desc.flags & GF_INSTALLER) {
		Common::Archive *arch = MpsInstaller::open("MPSLABS");
		if (arch)
			SearchMan.add("mpslabs", arch);
	}

	// Set up sound manager
	_soundManager = new RexSoundManager(_mixer, _soundFlag);
	_soundManager->validate();

	// Run the game
		RexNebular::nebular_main();

	return Common::kNoError;
}

void RexNebularEngine::global_init_code() {
	RexNebular::global_init_code();
}

void RexNebularEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	case 6: Rooms::section_6_music(); break;
	case 7: Rooms::section_7_music(); break;
	case 8: Rooms::section_8_music(); break;
	}
}

void RexNebularEngine::global_section_constructor() {
	RexNebular::global_section_constructor();
}

void RexNebularEngine::syncRoom(Common::Serializer &s) {
	RexNebular::sync_room(s);
}

void RexNebularEngine::global_daemon_code() {
}

void RexNebularEngine::global_pre_parser_code() {

}

void RexNebularEngine::global_parser_code() {
}

void RexNebularEngine::global_error_code() {
}

void RexNebularEngine::global_room_init() {
}

void RexNebularEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "/");
	env_catint(kernel.sound_driver, new_section, 1);
}

void RexNebularEngine::player_keep_walking() {
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
