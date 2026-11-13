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
#include "tesoro/core/cursor.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/walkfloor.h"
#include "tesoro/core/pathobject.h"

namespace Tesoro {

static const char *const kLevelNames[World::kLevelCount] = {
	"DESPACHO.LVL", "PUERTO.LVL", "TABERNA.LVL", "CUBIERTA.LVL", "CAMAROTE.LVL", "ALMACEN.LVL", "COCINA.LVL", "PLAYA.LVL",
	"MAPA.LVL", "POBLADO.LVL", "PANTANO.LVL", "CAMPAMENTO.LVL", "VOLCAN.LVL", "ARMARIO.LVL", "TABLERO.LVL", "TIERRA.LVL",
	"RISCO.LVL", "CUBIERTA2.LVL", "CAMAROTE2.LVL", "ALMACEN2.LVL", "COCINA2.LVL", "LUCHA.LVL", "TRONO.LVL", "ENTRADA.LVL",
	"1.LVL", "2.LVL", "3.LVL", "4.LVL", "5.LVL", "6.LVL", "7.LVL", "8.LVL", "9.LVL", "10.LVL", "11.LVL", "12.LVL", "13.LVL",
	"14.LVL", "15.LVL", "16.LVL", "17.LVL", "18.LVL", "19.LVL", "20.LVL", "RISCOFINAL.LVL", "FINAL.LVL", "CREDITOS.LVL"
};

// Jim's animations in the order of Jim::Bank.
static const char *const kJimFiles[Jim::kBanks] = {
	"JIMHABIZ.ANM", "JIMHABDER.ANM", "JIMQUIETO.ANM", "JIMARRIBA.ANM", "JIMABAJO.ANM",
	"JIMIZQUIERDA.ANM", "JIMDERECHA.ANM", "JIMHABAB.ANM", "JIMHABAR.ANM"
};


/** Level whose objects of the clickable family are built here. */
class WorldLevel : public Level {
public:
	explicit WorldLevel(World &world) : _world(world) {}

protected:
	int update2() override { return _world.levelSecondStep(*this); }

private:
	World &_world;
	byte _variables = 0;
};

World::~World() {
	delete _jim;
	for (int i = 0; i < kLevelCount; ++i)
		delete _levels[i];
}

Level *World::loadLevelFile(const char *name, int mode, int index) {
	const uint32 offset = _engine._archive.findEntry(name);
	if (!offset) {
		warning("Tesoro: level %s is not in the archive", name);
		return nullptr;
	}
	Level *level = new WorldLevel(*this);
	if (!level->load(*_engine._archive.seekEntry(offset), offset, mode, now())) {
		warning("Tesoro: level %s could not be read", name);
		delete level;
		return nullptr;
	}
	// Objects look at their level's index when they attach, so it is set first.
	if (index >= 0)
		level->setIndex(index);
	level->attachObjects();
	return level;
}

int World::levelIndexOf(const char *name) const {
	for (int i = 0; i < kLevelCount; ++i) {
		if (!scumm_stricmp(kLevelNames[i], name))
			return i;
	}
	return -1;
}

Level *World::level(int index) {
	if (index < 0 || index >= kLevelCount)
		return nullptr;
	if (!_levels[index]) {
		_levels[index] = loadLevelFile(kLevelNames[index], AnimBank::kLoadDeferred, index);
		if (_levels[index]) {
			_levels[index]->setIndex(index);
			applyStartState(*_levels[index], index);
		}
	}
	return _levels[index];
}

void World::applyStartState(Level &where, int index) {
	// The game starts with these objects switched off (hidden, or hidden and off); the story switches them on.
	struct Start {
		int level;
		const char *name;
		bool hide;
		bool deactivate;
	};
	static const Start kStart[] = {
		{ 0, "PUERTA1", true, false }, { 4, "BAULOPEN", false, true }, { 7, "CANOA", false, true },
		{ 8, "RISCO", false, true }, { 8, "CUEVA", false, true }, { 10, "DICK", false, true },
		{ 17, "SOGACORTADA", false, true }, { 17, "CAPIMUERTO", false, true },
		{ 9, "JIMCUBO", true, true }, { 10, "FERNAN", false, true }
	};
	for (const Start &start : kStart) {
		if (start.level != index)
			continue;
		Object *object = where.findObject(nameKey64(start.name));
		if (!object)
			continue;
		if (start.hide)
			object->setVisible(false);
		if (start.deactivate)
			object->setActive(false);
	}
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

void World::setPointerActive(bool active) {
	if (_engine._cursor)
		_engine._cursor->setActive(active);
}

int World::levelSecondStep(Level &level) { return _engine.levelSecondStep(level); }

void World::insertInMainList(Object *object, int priority) {
	_main.insertByPriority(object, priority);
}

void World::insertInMainListFront(Object *object, int priority) {
	_main.insertByPriorityFront(object, priority);
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
	_main.remove(object);
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
