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


#include "ultima/ultima8/games/start_u8_process.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gfx/palette_fader_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(StartU8Process)

StartU8Process::StartU8Process(int saveSlot) : Process(),
		_init(false), _saveSlot(saveSlot), _skipStart(saveSlot >= 0) {
	_flags |= PROC_PREVENT_SAVE;
}


void StartU8Process::run() {
	if (!_skipStart && !_init) {
		_init = true;
		ProcId moviepid = Game::get_instance()->playIntroMovie(false);
		Process *movieproc = Kernel::get_instance()->getProcess(moviepid);
		if (movieproc) {
			waitFor(movieproc);
			return;
		}
	}

	// Try to load the save game, if succeeded this process will terminate
	if (_saveSlot >= 0) {
		Common::Error loadError = Ultima8Engine::get_instance()->loadGameState(_saveSlot);
		if (loadError.getCode() != Common::kNoError) {
			Ultima8Engine::get_instance()->setError(loadError);
			fail();
			return;
		}

		PaletteFaderProcess::I_fadeFromBlack(0, 0);
		terminate();
		return;
	}

	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	UCList uclist(2);

	if (!_skipStart) {
		LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(36)));
		currentmap->areaSearch(&uclist, script, sizeof(script),
		                       0, 256, false, 16188, 7500);
		if (uclist.getSize() < 1) {
			warning("Unable to find FIRST egg");
			return;
		}

		uint16 objid = uclist.getuint16(0);
		Egg *egg = dynamic_cast<Egg *>(getObject(objid));
		Point3 pt = egg->getLocation();
		// Center on egg
		CameraProcess::SetCameraProcess(new CameraProcess(pt));
		egg->hatch();
	}

	// Music Egg
	// Item 2145 (class Item, shape 562, 0, (11551,2079,48) q:52, m:0, n:0, f:2000, ef:2)
	uclist.free();
	LOOPSCRIPT(musicscript, LS_SHAPE_EQUAL1(562));
	currentmap->areaSearch(&uclist, musicscript, sizeof(musicscript),
	                       0, 256, false, 11551, 2079);

	if (uclist.getSize() < 1) {
		warning("Unable to find MUSIC egg");
	} else {
		ObjId objid = uclist.getuint16(0);
		Item *musicEgg = getItem(objid);
		musicEgg->callUsecodeEvent_cachein();
	}

	if (!_skipStart)
		MenuGump::inputName();
	else
		Ultima8Engine::get_instance()->setAvatarInStasis(false);


	terminate();
}

void StartU8Process::saveData(Common::WriteStream *ws) {
	warning("Attempted save of process with prevent save flag");

	Process::saveData(ws);
}

bool StartU8Process::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
