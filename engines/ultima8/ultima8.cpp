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

#include "ultima8/ultima8.h"
#include "ultima8/detection.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

/*
//#include "ultima8/misc/pent_include.h"
#include "ultima8/kernel/gui_app.h"
#include "ultima8/kernel/memory_manager.h"
#include "ultima8/misc/version.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/filesys/output_logger.h"
*/
#include "ultima8/filesys/archive.h"
#include "ultima8/filesys/archive_file.h"
#include "ultima8/filesys/dir_file.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/filesys/flex_file.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/named_archive_file.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/filesys/output_logger.h"
#include "ultima8/filesys/raw_archive.h"
#include "ultima8/filesys/savegame.h"
#include "ultima8/filesys/savegame_writer.h"
#include "ultima8/filesys/u8_save_file.h"
#include "ultima8/filesys/zip_file.h"

#include "ultima8/misc/args.h"
#include "ultima8/misc/box.h"
#include "ultima8/misc/common_types.h"
#include "ultima8/misc/console.h"
#include "ultima8/misc/direction.h"
#include "ultima8/misc/encoding.h"
#include "ultima8/misc/errors.h"
#include "ultima8/misc/id_man.h"
#include "ultima8/misc/istring.h"
#include "ultima8/misc/md5.h"
#include "ultima8/misc/memset_n.h"
#include "ultima8/misc/pent_include.h"
#include "ultima8/misc/pent_valgrind.h"
#include "ultima8/misc/p_dynamic_cast.h"
#include "ultima8/misc/rect.h"
#include "ultima8/misc/util.h"
#include "ultima8/misc/version.h"


namespace Ultima8 {

Ultima8Engine *g_vm;

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima8GameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Ultima8") {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
}

Ultima8Engine::~Ultima8Engine() {
}

uint32 Ultima8Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Error Ultima8Engine::run() {


	return Common::kNoError;
}

Common::FSNode Ultima8Engine::getGameDirectory() const {
	return Common::FSNode(ConfMan.get("path"));
}

} // End of namespace Ultima8
