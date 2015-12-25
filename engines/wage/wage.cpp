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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"

#include "engines/engine.h"
#include "engines/util.h"
#include "gui/EventRecorder.h"
#include "graphics/palette.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/entities.h"
#include "wage/script.h"
#include "wage/world.h"

namespace Wage {

WageEngine::WageEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _gameDescription(desc) {
	_rnd = new Common::RandomSource("wage");

	_aim = -1;

	debug("WageEngine::WageEngine()");
}

WageEngine::~WageEngine() {
	debug("WageEngine::~WageEngine()");

	DebugMan.clearAllDebugChannels();
	delete _rnd;
}

static byte palette[] = {
	0, 0, 0,
	0x80, 0x80, 0x80,
	0xff, 0xff, 0xff
};

Common::Error WageEngine::run() {
	initGraphics(640, 480, true);

	g_system->getPaletteManager()->setPalette(palette, 0, 3);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	debug("WageEngine::init");

	// Your main event loop should be (invoked from) here.
	_resManager = new Common::MacResManager();
	_resManager->open(getGameFile());

	_world = new World(this);

	if (!_world->loadWorld(_resManager))
		return Common::kNoGameDataFoundError;

	Graphics::Surface screen;
	screen.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect r(0, 0, screen.w, screen.h);

	performInitialSetup();

	Common::String input("look");

	_world->_player->_currentScene = _world->_orderedScenes[1];
	_world->_globalScript->execute(_world, 1, &input, NULL, this);

	//_world->_orderedScenes[1]->_design->paint(&screen, _world->_patterns, false);
	//_world->_objs["frank.1"]->_design->setBounds(&r);
	//_world->_objs["frank.1"]->_design->paint(&screen, _world->_patterns, false);
	//_world->_scenes["temple of the holy mackeral"]->_design->setBounds(&r);
	//_world->_scenes["temple of the holy mackeral"]->_design->paint(&screen, _world->_patterns, false);
	//_world->_scenes["tower level 3"]->_design->setBounds(&r);
	//_world->_scenes["tower level 3"]->_design->paint(&screen, _world->_patterns, false);

	return Common::kNoError;
}

void WageEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			error("Exiting");
			break;
		default:
			break;
		}
	}
}

void WageEngine::playSound(String soundName) {
	warning("STUB: playSound");
}

void WageEngine::setMenu(String soundName) {
	warning("STUB: setMenu");
}

void WageEngine::appendText(String str) {
	warning("STUB: appendText(%s)", str.c_str());
}

Obj *WageEngine::getOffer() {
	warning("STUB: getOffer");

	return NULL;
}

void WageEngine::performInitialSetup() {
	for (int i = 0; i < _world->_orderedObjs.size(); i++)
		_world->move(_world->_orderedObjs[i], &_world->_storageScene);
	for (int i = 0; i < _world->_orderedChrs.size(); i++)
		_world->move(_world->_orderedChrs[i], &_world->_storageScene);

	for (int i = 0; i < _world->_orderedObjs.size(); i++) {
		Obj *obj = _world->_orderedObjs[i];
		if (!obj->_sceneOrOwner.equalsIgnoreCase(STORAGESCENE)) {
			String location = obj->_sceneOrOwner;
			location.toLowercase();
			if (_world->_scenes.contains(location)) {
				_world->move(obj, _world->_scenes[location]);
			} else {
				if (!_world->_chrs.contains(location)) {
					// Note: PLAYER@ is not a valid target here.
					warning("Couldn't move %s to %s", obj->_name.c_str(), obj->_sceneOrOwner.c_str());
				} else {
					// TODO: Add check for max items.
					_world->move(obj, _world->_chrs[location]);
				}
			}
		}
	}

	bool playerPlaced = false;
	for (int i = 0; i < _world->_orderedChrs.size(); i++) {
		Chr *chr = _world->_orderedChrs[i];
		if (!chr->_initialScene.equalsIgnoreCase(STORAGESCENE)) {
			String key = chr->_initialScene;
			key.toLowercase();
			if (_world->_scenes.contains(key)) {
				_world->move(chr, _world->_scenes[key]);

				if (chr->_playerCharacter)
					warning("Initial scene: %s", key.c_str());
			} else {
				_world->move(chr, _world->getRandomScene());
			}
			if (chr->_playerCharacter) {
				playerPlaced = true;
			}
		}
		chr->wearObjs();
	}
	if (!playerPlaced) {
		_world->move(_world->_player, _world->getRandomScene());
	}
}

} // End of namespace Wage
