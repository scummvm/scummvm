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

#include "bagel/bagel.h"
#include "bagel/detection.h"
#include "backends/keymapper/keymapper.h"

namespace Bagel {

BagelEngine *g_engine;

BagelEngine::BagelEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Bagel") {
	g_engine = this;
}

BagelEngine::~BagelEngine() {
	delete _midi;
}

uint32 BagelEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String BagelEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform BagelEngine::getPlatform() const {
	return _gameDescription->platform;
}

bool BagelEngine::isDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

SaveStateList BagelEngine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

bool BagelEngine::savesExist() const {
	return !listSaves().empty();
}

void BagelEngine::errorDialog(const char *msg) const {
	GUIErrorMessage(msg);
}

void BagelEngine::enableKeymapper(bool enabled) {
	getEventManager()->getKeymapper()->setEnabled(enabled);
}

} // End of namespace Bagel
