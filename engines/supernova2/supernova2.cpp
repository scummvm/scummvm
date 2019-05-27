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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "supernova2/supernova2.h"


namespace Supernova2 {

Supernova2Engine::Supernova2Engine(OSystem *syst)
	: Engine(syst)
	, _allowLoadGame(true)
	, _allowSaveGame(true)
	, _sleepAutoSave(nullptr)
	, _sleepAuoSaveVersion(-1)
	, _delay(33)
	, _textSpeed(1) {
	if (ConfMan.hasKey("textspeed"))
		_textSpeed = ConfMan.getInt("textspeed");

	DebugMan.addDebugChannel(1 , "general", "Supernova 2 general debug channel");
}

Supernova2Engine::~Supernova2Engine() {
	DebugMan.clearAllDebugChannels();

	delete _sleepAutoSave;
}

Common::Error Supernova2Engine::run() {
	init();

	while (!shouldQuit()) {
		uint32 start = _system->getMillis();
		int end = _delay - (_system->getMillis() - start);
		if (end > 0)
			_system->delayMillis(end);
	}

	return Common::kNoError;
}

void Supernova2Engine::init() {
}

}
