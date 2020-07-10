/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/readable_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/gumps/main_menu_process.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarDeathProcess)

AvatarDeathProcess::AvatarDeathProcess() : Process() {
	_itemNum = 1;
	_type = 1; // CONSTANT !
}

void AvatarDeathProcess::run() {
	MainActor *av = getMainActor();

	if (!av) {
		perr << "AvatarDeathProcess: MainActor object missing" << Std::endl;
		// avatar gone??
		terminate();
		return;
	}

	if (!av->hasActorFlags(Actor::ACT_DEAD)) {
		perr << "AvatarDeathProcess: MainActor not dead" << Std::endl;
		// avatar not dead?
		terminate();
		return;
	}

	PaletteManager *palman = PaletteManager::get_instance();
	palman->untransformPalette(PaletteManager::Pal_Game);

	Process *menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);

	if (GAME_IS_U8) {
		// Show the avatar gravestone
		ReadableGump *gump = new ReadableGump(1, 27, 11,
											  _TL_("HERE LIES*THE AVATAR*REST IN PEACE"));
		gump->InitGump(0);
		gump->setRelativePosition(Gump::CENTER);
		Process *gumpproc = gump->GetNotifyProcess();
		menuproc->waitFor(gumpproc);
	} else {
		// Play "Silencer Terminated" audio and wait
		// a couple of seconds before showing menu
		AudioProcess *ap = AudioProcess::get_instance();
		ap->playSFX(9, 0x10, 0, 1);
		DelayProcess *delayproc = new DelayProcess(60);
		Kernel::get_instance()->addProcess(delayproc);
		menuproc->waitFor(delayproc);
	}

	// done
	terminate();
}

void AvatarDeathProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool AvatarDeathProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
