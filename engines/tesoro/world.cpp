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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/managed_surface.h"
#include "graphics/scaler.h"

#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/kinds/actionstrip.h"
#include "tesoro/kinds/boardcolumn.h"
#include "tesoro/kinds/cameraframe.h"
#include "tesoro/kinds/clickcatcher.h"
#include "tesoro/kinds/cursor.h"
#include "tesoro/kinds/exitdoor.h"
#include "tesoro/kinds/interactive.h"
#include "tesoro/kinds/inventoryitem.h"
#include "tesoro/kinds/jim.h"
#include "tesoro/kinds/mapplace.h"
#include "tesoro/kinds/menubutton.h"
#include "tesoro/kinds/noteobject.h"
#include "tesoro/kinds/objectgroup.h"
#include "tesoro/kinds/pathobject.h"
#include "tesoro/kinds/slotlabel.h"
#include "tesoro/kinds/storycharacter.h"
#include "tesoro/kinds/storyprop.h"
#include "tesoro/kinds/walkfloor.h"
#include "tesoro/kinds/wanderingsprite.h"
#include "tesoro/scripts.h"
#include "tesoro/tesoro.h"
#include "tesoro/world.h"

namespace Tesoro {

static const char *const kLevelNames[World::kLevelCount] = {
	"DESPACHO.LVL", "PUERTO.LVL", "TABERNA.LVL", "CUBIERTA.LVL", "CAMAROTE.LVL", "ALMACEN.LVL", "COCINA.LVL", "PLAYA.LVL",
	"MAPA.LVL", "POBLADO.LVL", "PANTANO.LVL", "CAMPAMENTO.LVL", "VOLCAN.LVL", "ARMARIO.LVL", "TABLERO.LVL", "TIERRA.LVL",
	"RISCO.LVL", "CUBIERTA2.LVL", "CAMAROTE2.LVL", "ALMACEN2.LVL", "COCINA2.LVL", "LUCHA.LVL", "TRONO.LVL", "ENTRADA.LVL",
	"1.LVL", "2.LVL", "3.LVL", "4.LVL", "5.LVL", "6.LVL", "7.LVL", "8.LVL", "9.LVL", "10.LVL", "11.LVL", "12.LVL", "13.LVL",
	"14.LVL", "15.LVL", "16.LVL", "17.LVL", "18.LVL", "19.LVL", "20.LVL", "RISCOFINAL.LVL", "FINAL.LVL", "CREDITOS.LVL"
};

static const char *const kSpeechFiles[SpeechTables::kCount] = { "OBJETOS.TXT", "ANIMS.TXT", "CONVERS.TXT", "ICONS.TXT" };

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
	Object *createObject(const ObjectRecord &record, Room &room) override {
		if (Object *object = _world.createObject(*this, record, room, loadTime()))
			return object;
		return Level::createObject(record, room);
	}

	int update2() override { return _world.levelSecondStep(*this); }

	// The labyrinth levels borrow the floor of level 42; that level has its own, and so is not asked.
	Level *sharedFloorLevel() override { return index() == kSharedFloorLevel ? nullptr : _world.level(kSharedFloorLevel); }

	int picturesReloaded() override {
		// The island map has a step of its own that replaces the usual one: no music, no collision pass.
		if (index() == kMapLevel)
			return mapEvents();
		// The music of the level starts first, then the level's usual step runs.
		if (Level *where = _world.jimLevel()) {
			const CameraFrame *frame = where->cameraFrame();
			if (frame && frame->flag())
				_world.playMusicTrack(frame->flag());
		}
		const int result = update();
		return index() == kDeckLevel ? deckEvents(result) : result;
	}

private:
	enum { kDeckLevel = 3, kMapLevel = 8, kPlayaLevel = 7, kPoblado = 9, kPantano = 10, kVolcan = 12, kSharedFloorLevel = 42 };

	/** The state record of an object of another level. */
	byte *stateOf(int level, const char *name, uint size) {
		Object *object = _world.findObject(level, name);
		return object && object->stateBlob().size() >= size ? object->stateBlob().begin() : nullptr;
	}

	/** The story flags that open places on the island map. */
	int mapEvents() {
		byte *orzoguei = stateOf(kPoblado, "ORZOGUEI", 0xf);
		if (orzoguei && orzoguei[0xb] && orzoguei[0xc] && orzoguei[0xd] && orzoguei[0xe])
			orzoguei[1] = 5;
		Object *canoa = _world.findObject(kPlayaLevel, "CANOA");
		if (canoa && !(canoa->flags() & 2) && orzoguei && (int8)orzoguei[1] > 4) {
			canoa->setActive(true);
			if (Object *justino = _world.findObject(kVolcan, "JUSTINO"))
				justino->setActive(false);
		}
		Object *fernan = _world.findObject(kPantano, "FERNAN");
		const byte *justino = stateOf(kVolcan, "JUSTINO", 1);
		const byte *fernanState = stateOf(kPantano, "FERNAN", 1);
		if (fernan && fernanState && fernanState[0] == 0 && justino && justino[0] == 2)
			fernan->setActive(true);
		return 1;
	}
	/** The ship's deck events: music changes and the mutiny scene. */
	int deckEvents(int result) {
		CameraFrame *own = cameraFrame();
		if (_world._jimStep != 0 && own && own->flag() == 8) {
			own->setFlag(9);
			_world.playMusicTrack(9);
		}
		const byte step = _world._jimStep;
		if ((int8)step > 2) {
			// The script's variable bytes are the step it started at.
			_variables = step;
			_world.startScriptInJimLevel("FANMOTIN", "main", &_variables);
			_world._jimStep = step + 1;
			_world._jimPhase = 1;
			return 3;
		}
		if (step == 0) {
			// The fisherman is told the story has not begun: state 2 becomes 3.
			Object *fisherman = _world.findObject(kDeckLevel, "PESCADOR");
			if (fisherman && !fisherman->stateBlob().empty() && fisherman->stateBlob()[0] == 2)
				fisherman->stateBlob()[0] = 3;
		}
		return 1;
	}

	World &_world;
	byte _variables = 0;
};

World::World(TesoroEngine &engine) : _engine(engine), _sound(engine._mixer) {}

World::~World() {
	delete _jim;
	for (int i = 0; i < kLevelCount; ++i)
		delete _levels[i];
	delete _scripts;
	_sceneThumb.free();
}

bool World::loadGameData() {
	// The font the texts are drawn with
	const uint32 fontOffset = _engine._archive.findEntry("FUENTES.ANM");
	if (!fontOffset || !_font.load(*_engine._archive.seekEntry(fontOffset), fontOffset, AnimBank::kLoadNow)) {
		warning("Tesoro: the font FUENTES.ANM cannot be read");
		return false;
	}
	_engine._display.setFont(_font.bank());

	// The table that darkens the picture through shadows
	const uint32 tableOffset = _engine._archive.findEntry("TABLA1.TBL");
	byte table[0x8000];
	if (tableOffset && _engine._archive.seekEntry(tableOffset)->read(table, sizeof table) == sizeof table)
		_engine._display.setShadeTable(table);
	else
		warning("Tesoro: the shade table TABLA1.TBL cannot be read");

	for (uint i = 0; i < SpeechTables::kCount; ++i) {
		const uint32 offset = _engine._archive.findEntry(kSpeechFiles[i]);
		if (!offset) {
			warning("Tesoro: %s is not in the archive", kSpeechFiles[i]);
			return false;
		}

		Common::SeekableReadStream &dat = *_engine._archive.seekEntry(offset - 4);
		const uint32 size = dat.readUint32LE();
		_speechText[i].resize(size);
		if (dat.read(_speechText[i].data(), size) != size)
			return false;
		_speech.table(i).load(_speechText[i].data(), size, _engine._archive);
	}
	_scripts = new Scripts(*this);
	return _scripts->load();
}

void World::updateSound() {
	_sound.update();
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
		if (!offset || !levelPlayers[i].load(*_engine._archive.seekEntry(offset), offset, AnimBank::kLoadDeferred)) {
			warning("Tesoro: Jim's animation %s cannot be read", kJimFiles[i]);
			return nullptr;
		}
	}
	_jim = new Jim(*this);
	_jim->init(levelPlayers, phrase("JIM"));
	return _jim;
}

bool World::putJimInLevel(int index, int32 x, int32 y) {
	return putJimInLevel(level(index), x, y);
}

bool World::putJimInLevel(Level *target, int32 x, int32 y) {
	Jim *player = jim();
	if (!target || !player || target->roomCount() == 0)
		return false;
	player->enterLevel(target, &target->room(0), x, y);
	return true;
}

Level *World::startGame() {
	Level *port = level(1);
	if (!port || port->roomCount() == 0 || !jim())
		return nullptr;
	// Jim comes in where the level's walk-area object stands.
	const CameraFrame *frame = port->cameraFrame();
	Object *floor = frame ? frame->target() : nullptr;
	_jim->enterLevel(port, &port->room(0), floor ? floor->x() : 0, floor ? floor->y() : 0);
	_jim->setActive(true);
	_jim->setVisible(true);
	startScriptAt(port, "FANPRESENT", "main", nullptr);
	return port;
}

void World::saveGame(Common::WriteStream &stream) {
	Jim *player = jim();
	Level *where = jimLevel();
	// The file starts with the small picture of the scene.
	Common::Array<uint16> picture(Cursor::kThumbWidth * Cursor::kThumbHeight, 0);
	if (_thumbnail.size() == picture.size())
		picture = _thumbnail;
	stream.write(&picture[0], picture.size() * 2);
	stream.writeSint32LE(where ? where->index() : -1);
	player->saveState(stream);

	uint32 count = 0;
	for (int i = 0; i < kLevelCount; ++i)
		count += _levels[i] ? 1 : 0;
	stream.writeUint32LE(count);
	for (int i = 0; i < kLevelCount; ++i) {
		if (!_levels[i])
			continue;
		stream.writeUint32LE((uint32)i);
		_levels[i]->saveState(stream);
		_levels[i]->saveRecords(stream, player);
	}
	if (_interface)
		_interface->saveRecords(stream, player);
}

Level *World::loadGame(Common::ReadStream &stream) {
	Jim *player = jim();
	Common::Array<uint16> picture(Cursor::kThumbWidth * Cursor::kThumbHeight);
	if (stream.read(&picture[0], picture.size() * 2) != picture.size() * 2)
		return nullptr;
	setSlotPicture(&picture[0]);
	const int32 slot = stream.readSint32LE();
	player->loadState(stream);

	const uint32 count = stream.readUint32LE();
	for (uint32 n = 0; n < count && !stream.eos(); ++n) {
		const uint32 index = stream.readUint32LE();
		Level *visited = index < (uint32)kLevelCount ? level((int)index) : nullptr;
		if (!visited) {
			warning("Tesoro: the saved game names level %u, which does not exist", index);
			return nullptr;
		}
		visited->loadState(stream);
		if (!visited->loadRecords(stream))
			return nullptr;
	}
	if (_interface && !_interface->loadRecords(stream))
		return nullptr;

	Level *where = level(slot);
	if (!where || where->roomCount() == 0 || stream.eos())
		return nullptr;
	player->enterLevel(where, &where->room(0), player->x(), player->y());
	player->setFacing(player->facing());
	return where;
}

void World::runPendingLoad() {
	if (_pendingLoadSlot < 0)
		return;
	const int slot = _pendingLoadSlot;
	_pendingLoadSlot = -1;
	// A load from the save screen ends by sliding the interface out, so the game goes straight on.
	if (_engine.loadGameState(slot).getCode() == Common::kNoError)
		closeSaveLoadMenu();
}

static SaveStateList listSlots(TesoroEngine &engine) {
	SaveStateList saves = engine.getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	Common::sort(saves.begin(), saves.end(), SaveStateDescriptorSlotComparator());
	return saves;
}

void World::setSlotPicture(const uint16 *pixels) {
	Object *holder = interfaceObject("PLANTILLA1");
	if (!holder || holder->animation().bankCount() == 0)
		return;
	AnimPlayer &player = holder->animation().current();
	if (!player.bank() || player.bank()->stepCount() == 0 || !player.bank()->step(0).hasImage())
		return;
	byte *target = player.bank()->mutableImage((uint)player.bank()->step(0).imageIndex).pixelData();
	const uint32 bytes = Cursor::kThumbWidth * Cursor::kThumbHeight * 2;
	if (!target) {
		warning("Tesoro: the save screen's picture is not loaded");
		return;
	}
	if (pixels)
		memcpy(target, pixels, bytes);
	else
		memset(target, 0, bytes);
}

void World::showSlotPicture(int slot) {
	Common::Array<uint16> pixels(Cursor::kThumbWidth * Cursor::kThumbHeight);
	Common::InSaveFile *file = slot >= 0 ? _engine.getSaveFileManager()->openForLoading(_engine.getSaveStateName(slot)) : nullptr;
	if (file && file->read(&pixels[0], pixels.size() * 2) == pixels.size() * 2) {
		delete file;
		setSlotPicture(&pixels[0]);
		return;
	}
	delete file;
	setSlotPicture(nullptr);
}

void World::showChosenSlotPicture() {
	SaveStateList saves = listSlots(_engine);
	showSlotPicture(_slotChoice >= 1 && _slotChoice <= (int)saves.size() ? saves[_slotChoice - 1].getSaveSlot() : -1);
}

Object *World::createObject(Level &level, const ObjectRecord &record, Room &room, int32 loadTime) {
	const int16 sort = room.sortOffset();
	const Common::Array<AnimPlayer> &levelPlayers = level.levelPlayers();
	switch (record.kind) {
		case 2: {
			ObjectGroup *object = new ObjectGroup();
			object->init(record, sort, levelPlayers);
			return object;
		}
		case 5: {
			StoryCharacter *object = new StoryCharacter();
			object->init(record.dataByte(5), record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 6: {
			ExitDoor *object = new ExitDoor();
			object->init(record.dataByte(1), record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 9: {
			InteractiveObject *object = new InteractiveObject();
			object->init(record, sort, levelPlayers, loadTime, *this, 0);
			return object;
		}
		case 12:
		case 17:
		case 20: {
			const uint sub = record.dataByte(1);
			if (sub >= (record.kind == 12 ? 12u : record.kind == 17 ? 17u : 31u) || (record.kind == 20 && sub == 17)) {
				warning("Level: object '%s' of kind %u has no subtype %u, skipped", record.name, record.kind, sub);
				return nullptr;
			}
			StoryProp *object = new StoryProp();
			object->init(record.kind, sub, record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 11: {
			ActionStrip *object = new ActionStrip();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 13: {
			InventoryItem *object = new InventoryItem();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		// kind 14 is unused
		case 15: {
			ClickCatcher *object = new ClickCatcher();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 16: {
			WanderingSprite *object = new WanderingSprite();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		// kind 18 is unused
		case 19: {
			MapPlace *object = new MapPlace();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 21: {
			BoardColumn *object = new BoardColumn();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 22: {
			MenuButton *object = new MenuButton();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 23: {
			SlotLabel *object = new SlotLabel();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		case 24: {
			NoteObject *object = new NoteObject();
			object->init(record, sort, levelPlayers, loadTime, *this);
			return object;
		}
		default:
			return nullptr;
	}
}

int32 World::now() {
	return _engine.gameTime();
}

const SpeechTable::Line *World::phrase(const char *name) {
	const uint32 ref = _speech.find(name);
	if (!ref) {
		warning("Tesoro: the line '%s' is not in the text files", name);
		return nullptr;
	}
	return &_speech.lineOf(ref);
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

bool World::pointerHasItem() { return pointerItem() != nullptr; }

bool World::combinationReply(uint64 objectKey, const SpeechTable::Line *&line) {
	InventoryItem *held = pointerItem();
	if (!held)
		return false;
	for (uint i = 0; i < held->combinationCount(); ++i) {
		if (held->combinationKey(i) == objectKey) {
			line = held->combinationLine(i);
			return true;
		}
	}
	return false;
}

bool World::jimSpeaking() { return jim() && jim()->speaker().speaking(); }
bool World::isJim(const Object *object) { return object == _jim; }
int World::jimFacing() { return jim()->facing(); }
void World::jimFace(int facing) { jim()->setFacing(facing); }
void World::jimStopTalking() { jim()->speaker().clearSpeech(); }
void World::jimForgetLine() { jim()->speaker().forgetLine(); }
void World::jimSayAnswer(const SpeechTable::Line &line) { jim()->sayAnswer(line); }
void World::jimWalkTo(int32 x, int32 y, WalkTarget *target) { jim()->walkTo(x, y, target, false); }
void World::jimUpdate() { jim()->update(); }

void World::walkJimToPointer(int32 mouseX, int32 mouseY) {
	// Jim only exists once a game has begun; in the menus a click on bare ground does nothing.
	Level *where = _jim ? _jim->level() : nullptr;
	if (!where)
		return;
	if (_jim->speaker().speaking())
		_jim->speaker().clearSpeech();
	const Camera &camera = where->camera();
	_jim->walkTo(mouseX + camera.x(), camera.y() - mouseY, nullptr, false);
}

void World::setHoverText(const char *text) {
	_engine.setHoverText(text);
}

void World::removeFromMainList(Object *object) {
	_main.remove(object);
}

uint32 World::playVoice(uint32 archiveOffset) {
	_engine._archive.seekEntry(archiveOffset);
	return _sound.play(archiveStream(), archiveOffset);
}

bool World::voiceActive(uint32 id) { return _sound.active(id); }
void World::stopVoice(uint32 id) {
	_sound.stop(id);
}

void World::returnPointerItem() {
	if (InventoryItem *held = pointerItem())
		held->setVisible(true);
	clearPointerItem();
}

void World::clearPointerItem() {
	setPointerItem(nullptr);
	cursorFrame(0, 0);
}

bool World::actionStripAvailable() {
	return place() == Cursor::kGame;
}

uint32 World::randomNumber(uint32 lo, uint32 hi) { return _engine.randomBetween(lo, hi); }

// Escape or the right button was released this frame (the left button does not skip).
bool World::skipPressed() {
	return ((_engine._escape | _engine._pointer.buttonState(PointerState::kRight)) & PointerState::kReleased) != 0;
}

bool World::subtitlesOn() { return _subtitles; }


void World::previousSlot() {
	const int count = (int)listSlots(_engine).size();
	if (count == 0)
		return;
	_slotChoice = _slotChoice == 0 ? count : _slotChoice == 1 ? 0 : _slotChoice - 1;
	showChosenSlotPicture();
}

void World::nextSlot() {
	const int count = (int)listSlots(_engine).size();
	if (count == 0)
		return;
	_slotChoice = _slotChoice >= count ? 0 : _slotChoice + 1;
	showChosenSlotPicture();
}

void World::loadSlot() {
	SaveStateList saves = listSlots(_engine);
	if (_slotChoice >= 1 && _slotChoice <= (int)saves.size())
		_pendingLoadSlot = saves[_slotChoice - 1].getSaveSlot();
}

void World::saveSlot() {
	SaveStateList saves = listSlots(_engine);
	int slot = 0;
	if (_slotChoice >= 1 && _slotChoice <= (int)saves.size()) {
		slot = saves[_slotChoice - 1].getSaveSlot();
	} else {
		for (uint i = 0; i < saves.size(); ++i)
			slot = MAX(slot, saves[i].getSaveSlot() + 1);
		_slotChoice = (int)saves.size() + 1;
	}
	_engine.saveGameState(slot, Common::U32String::format(_("Partida %d"), slot + 1).encode(), false);
	showSlotPicture(slot);
}

Common::String World::slotTime() {
	SaveStateList saves = listSlots(_engine);
	if (_slotChoice < 1 || _slotChoice > (int)saves.size())
		return Common::String();
	return Common::String::format("%-10s %s", saves[_slotChoice - 1].getSaveDate().c_str(), saves[_slotChoice - 1].getSaveTime().c_str());
}

void World::closeSaveLoadMenu() {
	if (_cursor)
		_cursor->closeSaveLoadMenu();
}

void World::drawMenuText(const char *text, uint32 colour, int32 x, int32 y, int32 width) {
	_engine._display.queueText((const byte *)text, colour, x, y, width, 0, false, 0);
}

Camera &World::updateCamera(int32 divisor) {
	Camera &camera = jimLevel()->camera();
	camera.updateToward(divisor, now());
	return camera;
}

void World::queueText(const byte *text, uint32 colour, int32 centreX, int32 y, int32 maxWidth) {
	_engine._display.queueText(text, colour, centreX, y, maxWidth, 0, true, 0);
}

void World::drawSubtitle(Jim &jim, const SpeechTable::Line &line) {
	const byte *text = (const byte *)line.str();
	const int32 colour = jim.speaker().colour();
	if (colour < 1) {
		queueText(text, (uint16)-(int16)colour, 320, 240, 640);
		return;
	}
	Camera &camera = jim.level()->camera();
	camera.updateToward(jim.room()->cameraDivisor(), now());
	queueText(text, (uint16)colour, jim.anchorX() - camera.scaledX(), jim.anchorY() + camera.scaledY(), jim.textWidth());
}

int World::interactiveUpdate(Jim &jim) {
	if (_jimClickPending && !jimSpeaking()) {
		setPointerActive(true);
		_jimClickPending = false;
		removeFromMainList(&jim);
		if (!_jimClickAnswered)
			return jimRunVerb();
		_jimClickAnswered = false;
	}
	jim.updateBounds(now());
	return 1;
}

void World::playMusicTrack(byte track) {
	_music.playTrack(track);
}

Level *World::jimLevel() {
	return _jim ? _jim->level() : nullptr;
}
int World::jimLevelIndex() { return jimLevel() ? jimLevel()->index() : -1; }
Object *World::findObjectByKey(int index, uint64 key) {
	Level *where = index < 0 ? _engine._interface : level(index);
	return where ? where->findObject(key) : nullptr;
}

Object *World::findObject(int index, const char *name) { return findObjectByKey(index, nameKey64(name)); }

bool World::isActive(int index, const char *name) {
	Object *object = findObject(index, name);
	if (!object) {
		warning("Tesoro: no object %s", name);
		return false;
	}
	return (object->flags() & Object::kActive) != 0;
}
void World::jimShow(bool visible) { jim()->setVisible(visible); }

void World::jimMoveTo(int32 x, int32 y) { jim()->setPosition(x, y); }
void World::jimRemoveFromMainList() { removeFromMainList(jim()); }

InventoryItem *World::pointerItem() { return _cursor ? static_cast<InventoryItem *>(_cursor->heldItem()) : nullptr; }

void World::setPointerItem(InventoryItem *item) {
	if (_cursor)
		_cursor->setHeldItem(item);
}

void World::cursorFrame(int first, int last) {
	if (_cursor)
		_cursor->animation().selectBank(0, first, last, false, false, now());
}

int32 World::interfaceCameraX() { return _interface ? _interface->camera().x() : 0; }

void World::hideInventory() {
	if (_cursor)
		_cursor->hideInventory();
}

void World::playFrames(Object &object, int first, int last) { object.animation().selectBank(0, first, last, false, false, now()); }

void World::setWalkAreaFrames(Level *where, WalkPart part, int first, int last) {

	const CameraFrame *frame = where ? where->cameraFrame() : nullptr;
	Object *floor = frame ? frame->target() : nullptr;
	Object *object = floor;
	if (part == kRoute)
		object = floor && floor->kindId() == 7 ? static_cast<WalkFloor *>(floor)->path() : nullptr;
	if (!object) {
		warning("Tesoro: the level has no %s to set frames on", part == kRoute ? "route" : "floor");
		return;
	}
	playFrames(*object, first, last);
}

void World::startMovie(int number) { _engine._video.playVideo(number); }

int World::startScript(Character &who, const char *block, const char *label, byte *variables) {
	return startScript(static_cast<InteractiveObject &>(who), block, label, variables);
}

int World::startScript(InteractiveObject &who, const char *block, const char *label, byte *variables) {
	return startScriptInLevel(who.level() ? who.level()->index() : -1, block, label, variables);
}

int World::startScriptInJimLevel(const char *block, const char *label, byte *variables) { return startScriptInLevel(jimLevelIndex(), block, label, variables); }

int World::startScriptInLevel(int index, const char *block, const char *label, byte *variables) {
	return startScriptAt(index < 0 ? _engine._interface : level(index), block, label, variables);
}

int World::startScriptAt(Level *where, const char *block, const char *label, byte *variables) {
	return _scripts ? _scripts->start(where, block, label, variables) : 0;
}
void World::resetRenderQueue() { _engine._display.startFrame(); }

void World::drawRooms(Level *level) {
	if (level)
		level->drawRooms(now(), _engine._display);
}
void World::composeFrame() { _engine._display.compose(); }

void World::clearScreens() {
	_engine._screen->clear();
	_engine._screen->update();
}
void World::fadeOut(int effect, int speed) { _engine.fadeOut(effect, speed); }
void World::wait(int milliseconds) { _engine._system->delayMillis(milliseconds); }

void World::showPieces() {
	if (Level *where = jimLevel())
		_game.showOn(where->objects(), now());
}

Common::SeekableReadStream &World::archiveStream() { return *_engine._archive.seekEntry(0); }

int World::useItemOnInterface() {
	// Item 37 is forwarded to the bucket in the interface, which is used with the "no item" value.
	Object *bucket = findObject(-1, "ICUBOMAR");
	return bucket ? static_cast<InteractiveObject *>(bucket)->useItem(0xff) : 0;
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

int World::jimClicked(int verb) {
	_jimVerb = verb;
	if (jimSpeaking())
		jimStopTalking();
	jimFace(jimFacing());
	jimRunVerb();
	return 1;
}

int World::jimRunVerb() {
	if (_jimVerb >= 7) {
		returnPointerItem();
		return jim()->useItem(_jimVerb - 7);
	}
	// Verbs 0, 5 and 6 have no default answer for him; the others answer in their usual words.
	static const char *const kDefaults[5] = { nullptr, "OBJ125T03", "OBJ096T07", "OBJ106T05", "OBJ106T05" };
	if (_jimVerb < 0 || _jimVerb > 4)
		return 0;
	_jimClickAnswered = true;
	const SpeechTable::Line *line = kDefaults[_jimVerb] ? phrase(kDefaults[_jimVerb]) : nullptr;
	if (!line)
		return 0;
	jimSayAnswer(*line);
	setPointerActive(false);
	_jimClickPending = true;
	insertInMainList(jim(), 5);
	return 3;
}

bool World::menuKeyPressed() { return (_engine._escape & PointerState::kPressed) != 0; }

void World::acknowledgeKeys() { _engine.resetKeys(); }

Object *World::interfaceObject(const char *name) { return _interface ? _interface->findObject(nameKey64(name)) : nullptr; }

void World::enterMainMenu() { _engine.enterMainMenu(); }

Cursor::Place World::place() {
	if (_engine._inMenu)
		return _engine._level == _engine._mainMenu ? Cursor::kMainMenu : Cursor::kNewGameMenu;
	return jimLevelIndex() == 14 ? Cursor::kBoard : Cursor::kGame;
}

void World::captureThumbnail() {

	Display &display = _engine._display;
	display.startFrame();
	if (Level *where = jimLevel())
		where->drawRooms(now(), display);
	display.compose();

	_thumbnail.resize(Cursor::kThumbWidth * Cursor::kThumbHeight);
	Cursor::sampleScene(display.frame(), &_thumbnail[0]);

	// Saved the composed scene also for the scummvm save
	Graphics::ManagedSurface composed(640, 480, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	memcpy(composed.getPixels(), display.frame(), 640 * 480 * 2);
	_sceneThumb.free();
	::createThumbnail(&_sceneThumb, &composed);
	display.startFrame();
}

} // End of namespace Tesoro
