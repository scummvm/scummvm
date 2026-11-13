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

#include "common/textconsole.h"

#include "tesoro/tesoro.h"
#include "tesoro/world.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/walkfloor.h"
#include "tesoro/core/pathobject.h"

namespace Tesoro {

// Jim's animations in the order of Jim::Bank.
static const char *const kJimFiles[Jim::kBanks] = {
	"JIMHABIZ.ANM", "JIMHABDER.ANM", "JIMQUIETO.ANM", "JIMARRIBA.ANM", "JIMABAJO.ANM",
	"JIMIZQUIERDA.ANM", "JIMDERECHA.ANM", "JIMHABAB.ANM", "JIMHABAR.ANM"
};

World::~World() {
	delete _jim;
}

Jim *World::jim() {
	if (_jim)
		return _jim;

	Common::Array<AnimPlayer> levelPlayers;
	levelPlayers.resize(Jim::kBanks);
	for (int i = 0; i < Jim::kBanks; ++i) {
		const uint32 offset = _engine._archive.findEntry(kJimFiles[i]);
		if (!offset || !levelPlayers[i].load(*_engine._archive.seekEntry(offset), offset, AnimBank::kLoadNow)) {
			warning("Tesoro: Jim's animation %s cannot be read", kJimFiles[i]);
			return nullptr;
		}
	}

	_jim = new Jim(*this);
	_jim->init(levelPlayers);
	return _jim;
}

int32 World::now() {
	return _engine.gameTime();
}

void World::insertInMainList(Object *object, int priority) {
	_engine._main.insertByPriority(object, priority);
}

void World::insertInMainListFront(Object *object, int priority) {
	_engine._main.insertByPriorityFront(object, priority);
}

void World::walkJimToPointer(int32 mouseX, int32 mouseY) {
	// Jim only exists once a game has begun; in the menus a click on bare ground does nothing.
	Level *where = _jim ? _jim->level() : nullptr;
	if (!where)
		return;
	const Camera &camera = where->camera();
	_jim->walkTo(mouseX + camera.x(), camera.y() - mouseY, nullptr, false);
}

void World::setHoverText(const char *text) {
	_engine.setHoverText(text);
}

void World::removeFromMainList(Object *object) {
	_engine._main.remove(object);
}

Level *World::jimLevel() {
	return _jim ? _jim->level() : nullptr;
}

bool World::hasWalkArea() {
	Level *level = jimLevel();
	const WalkFloor *floor = level ? level->floor() : nullptr;
	return floor && floor->path();
}

int32 World::scaleAt(int32 x, int32 y) {
	Level *level = jimLevel();
	WalkFloor *floor = level ? level->floor() : nullptr;
	return floor ? floor->scaleAt(x, y) : 0x100;
}

bool World::findPath(int32 destX, int32 destY, WalkTarget *target) {
	WalkFloor *floor = jimLevel()->floor();
	if (!floor || !floor->path())
		return false;
	return floor->path()->area(*jim()).findPath(destX, destY, jim()->x(), jim()->y(), target);
}

bool World::hasWalkAreaOf(Object *other) { return other && other->kindId() == 7 && static_cast<WalkFloor *>(other)->path(); }

} // End of namespace Tesoro
