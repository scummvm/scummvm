/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "common/debug-channels.h"
#include "common/events.h"
#include "common/error.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "gui/debugger.h"

#include "engines/engine.h"

#include "engines/myst3/console.h"
#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/nodecube.h"
#include "engines/myst3/nodeframe.h"
#include "engines/myst3/state.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/script.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/sound.h"

#include "graphics/jpeg.h"
#include "graphics/conversion.h"
#include "graphics/pixelbuffer.h"

#include "math/vector2d.h"

namespace Myst3 {

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst), _system(syst),
		_db(0), _console(0), _scriptEngine(0),
		_state(0), _node(0), _scene(0), _archiveNode(0),
		_cursor(0), _inventory(0), _gfx(0), _menu(0),
		_rnd(0), _sound(0), _shouldQuit(false),
		_menuAction(0), _projectorBackground(0) {
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kDebugNode, "Node", "Track Node Changes");

	// Add subdirectories to the search path to allow running from a full HDD install
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "bin");
	SearchMan.addSubDirectoryMatching(gameDataDir, "M3Data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "M3Data/TEXT");
	SearchMan.addSubDirectoriesMatching(gameDataDir, "EXILE Disc ?/Data", true);

	// Win DVD version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "English");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Data");

	// Mac DVD version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "Exile DVD");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Exile DVD/data");

	// PS2 version directories
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA/WORLD");
	SearchMan.addSubDirectoryMatching(gameDataDir, "GAMEDATA/WORLD/SOUND");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/DISCS");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/DISCS/DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT/NTSC");
}

Myst3Engine::~Myst3Engine() {
	DebugMan.clearAllDebugChannels();

	for (uint i = 0; i < _archivesCommon.size(); i++)
		delete _archivesCommon[i];

	delete _menu;
	delete _inventory;
	delete _cursor;
	delete _scene;
	delete _archiveNode;
	delete _db;
	delete _scriptEngine;
	delete _console;
	delete _state;
	delete _rnd;
	delete _sound;
	delete _gfx;
}

bool Myst3Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime);
}

Common::Error Myst3Engine::run() {
	const int w = 640;
	const int h = 480;

	_gfx = new Renderer(_system);
	_sound = new Sound(this);
	_rnd = new Common::RandomSource("sprint");
	_console = new Console(this);
	_scriptEngine = new Script(this);
	_db = new Database();
	_state = new GameState(this);
	_scene = new Scene(this);
	_menu = new Menu(this);
	_archiveNode = new Archive();

	_system->setupScreen(w, h, false, true);
	_system->showMouse(false);
	_system->lockMouse(true);

	addArchive("OVER101.m3o", false);
	addArchive("ENGLISH.m3t", true);
	addArchive("ENGLISH.m3u", false);
	addArchive("LANGUAGE.m3u", false);
	addArchive("RSRC.m3r", true);

	_cursor = new Cursor(this);
	_inventory = new Inventory(this);

	_gfx->init();

	// Init the font
	Graphics::Surface *font = loadTexture(1206);
	_gfx->initFont(font);
	font->free();
	delete font;

	if (ConfMan.hasKey("save_slot")) {
		// Load game from specified slot, if any
		loadGameState(ConfMan.getInt("save_slot"));
	} else {
		// Game init script, loads the menu
		loadNode(1, 101, 1);
	}

	while (!shouldQuit() && !_shouldQuit) {
		_sound->update();
		runNodeBackgroundScripts();
		processInput(false);
		updateCursor();

		if (_menuAction) {
			_menu->updateMainMenu(_menuAction);
			_menuAction = 0;
		}

		drawFrame();
	}

	for (uint i = 0; i < _movies.size(); i++) {
		delete _movies[i];
	}
	_movies.clear();

	delete _node;

	_archiveNode->close();

	_system->lockMouse(false);

	return Common::kNoError;
}

void Myst3Engine::addArchive(const Common::String &file, bool mandatory) {
	Archive *archive = new Archive();
	bool opened = archive->open(file.c_str(), 0);

	if (opened) {
		_archivesCommon.push_back(archive);
	} else {
		delete archive;
		if (mandatory)
			error("Unable to open archive %s", file.c_str());
	}
}

Common::Array<HotSpot *> Myst3Engine::listHoveredHotspots(NodePtr nodeData) {
	Common::Array<HotSpot *> hovered;

	if (_state->getViewType() == kCube) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		Common::Point mouse = Common::Point(heading, pitch);

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			if (nodeData->hotspots[j].isPointInRectsCube(mouse)
					&& _state->evaluate(nodeData->hotspots[j].condition)
					&& nodeData->hotspots[j].cursor < 12) {
				hovered.push_back(&nodeData->hotspots[j]);
			}
		}
	} else {
		Common::Point mouse = _cursor->getPosition();
		Common::Point scaledMouse;

		if (_state->getViewType() == kMenu)  {
			scaledMouse = Common::Point(
					mouse.x * Renderer::kOriginalWidth / _system->getWidth(),
					CLIP<uint>(mouse.y * Renderer::kOriginalHeight / _system->getHeight(),
							0, Renderer::kOriginalHeight));
		} else {
			scaledMouse = Common::Point(
					mouse.x * Renderer::kOriginalWidth / _system->getWidth(),
					CLIP<uint>(mouse.y * Renderer::kOriginalHeight / _system->getHeight()
							- Scene::kTopBorderHeight, 0, Scene::kFrameHeight));
		}

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			if (nodeData->hotspots[j].isPointInRectsFrame(_state, scaledMouse)
					&& _state->evaluate(nodeData->hotspots[j].condition)
					&& nodeData->hotspots[j].cursor < 12) {
				hovered.push_back(&nodeData->hotspots[j]);
			}
		}
	}

	return hovered;
}

void Myst3Engine::updateCursor() {
	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());

	Common::Array<HotSpot *> hovered = listHoveredHotspots(nodeData);
	uint16 hoveredInventory = _inventory->hoveredItem();

	if (hovered.size() > 0) {
		HotSpot *h = hovered.front();
		_cursor->changeCursor(h->cursor);
	} else if (hoveredInventory > 0) {
		_cursor->changeCursor(1);
	} else {
		_cursor->changeCursor(8);
	}
}

void Myst3Engine::processInput(bool lookOnly) {
	// Process events
	Common::Event event;
	while (getEventManager()->pollEvent(event)) {
		// Check for "Hard" quit"
		if (event.type == Common::EVENT_QUIT) {
			_shouldQuit = true;
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_state->getViewType() == kCube) {
				_scene->updateCamera(event.relMouse);
			}

			_cursor->updatePosition(event.relMouse);

		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			// Skip the event when in look only mode
			if (lookOnly) continue;

			uint16 hoveredInventory = _inventory->hoveredItem();
			if (hoveredInventory > 0) {
				_inventory->useItem(hoveredInventory);
				continue;
			}

			NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());
			Common::Array<HotSpot *> hovered = listHoveredHotspots(nodeData);

			if (hovered.size() > 0) {
				_scriptEngine->run(&hovered.front()->script);
				continue;
			}

			// Bad click
			_sound->play(697, 5);
		} else if (event.type == Common::EVENT_KEYDOWN) {
			// Save file name input
			_menu->handleInput(event.kbd);

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				// Open main menu
				if (_cursor->isVisible()) {
					if (_state->getLocationRoom() != 901)
						_menu->goToNode(100);
					else
						_state->setMenuEscapePressed(1);
				}
				break;
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_system->lockMouse(false);
					_console->attach();
					_console->onFrame();
					_system->lockMouse(true);
				}
				break;
			default:
				break;
			}
		}
	}
}

void Myst3Engine::drawFrame() {
	_gfx->clear();

	if (_state->getViewType() == kCube) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		_gfx->setupCameraPerspective(pitch, heading);
	} else {
		_gfx->setupCameraOrtho2D();
	}

	_node->update();
	_node->draw();

	for (uint i = 0; i < _movies.size(); i++) {
		_movies[i]->update();
		_movies[i]->draw();
	}

	if (_state->getViewType() == kMenu) {
		_menu->draw();
	}

	for (uint i = 0; i < _drawables.size(); i++) {
		_drawables[i]->draw();
	}

	if (_state->getViewType() == kCube) {
		_gfx->setupCameraOrtho2D();

		// Draw overlay 2D movies
		for (uint i = 0; i < _movies.size(); i++) {
			_movies[i]->drawOverlay();
		}

		for (uint i = 0; i < _drawables.size(); i++) {
			_drawables[i]->drawOverlay();
		}
	}

	if (_state->getViewType() != kMenu) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		SunSpot flare = _node->computeSunspotsIntensity(pitch, heading);
		if (flare.intensity >= 0)
			_scene->drawSunspotFlare(flare);

		_scene->drawBlackBorders();
		_inventory->draw();
	}

	if (_cursor->isVisible())
		_cursor->draw();

	_system->updateScreen();
	_system->delayMillis(10);
	_state->updateFrameCounters();
}

void Myst3Engine::goToNode(uint16 nodeID, uint transition) {
	uint16 node = _state->getLocationNextNode();
	if (node == 0)
		node = nodeID;

	uint16 room = _state->getLocationNextRoom();
	uint16 age = _state->getLocationNextAge();

	loadNode(node, room, age);

	_state->setLocationNextNode(0);
	_state->setLocationNextRoom(0);
	_state->setLocationNextAge(0);
}

void Myst3Engine::loadNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (_node) {
		// Delete all movies
		removeMovie(0);

		delete _node;
		_node = 0;
	}

	_scriptEngine->run(&_db->getNodeInitScript());

	if (nodeID)
		_state->setLocationNode(_state->valueOrVarValue(nodeID));

	if (roomID)
		_state->setLocationRoom(_state->valueOrVarValue(roomID));
	else
		roomID = _state->getLocationRoom();

	if (ageID)
		_state->setLocationAge(_state->valueOrVarValue(ageID));

	char oldRoomName[8];
	char newRoomName[8];
	_db->getRoomName(oldRoomName);
	_db->getRoomName(newRoomName, roomID);

	if (strcmp(newRoomName, "JRNL") && strcmp(newRoomName, "XXXX")
			 && strcmp(newRoomName, "MENU") && strcmp(newRoomName, oldRoomName)) {

		_db->setCurrentRoom(roomID);
		Common::String nodeFile = Common::String::format("%snodes.m3a", newRoomName);

		_archiveNode->close();
		if (!_archiveNode->open(nodeFile.c_str(), newRoomName)) {
			error("Unable to open archive %s", nodeFile.c_str());
		}
	}

	runNodeInitScripts();
}

void Myst3Engine::runNodeInitScripts() {
	NodePtr nodeData = _db->getNodeData(
			_state->getLocationNode(),
			_state->getLocationRoom(),
			_state->getLocationAge());

	NodePtr nodeDataInit = _db->getNodeData(
			32765,
			_state->getLocationRoom(),
			_state->getLocationAge());

	if (nodeDataInit)
		runScriptsFromNode(32765);

	if (!nodeData)
		error("Node %d unknown in the database", _state->getLocationNode());

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_state->evaluate(nodeData->scripts[j].condition)) {
			_scriptEngine->run(&nodeData->scripts[j].script);
		}
	}
}

void Myst3Engine::runNodeBackgroundScripts() {
	NodePtr nodeDataRoom = _db->getNodeData(32675, _state->getLocationRoom());

	if (nodeDataRoom) {
		for (uint j = 0; j < nodeDataRoom->hotspots.size(); j++) {
			if (nodeDataRoom->hotspots[j].condition == -1) {
				if (!_scriptEngine->run(&nodeDataRoom->hotspots[j].script))
					break;
			}
		}
	}

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());

	for (uint j = 0; j < nodeData->hotspots.size(); j++) {
		if (nodeData->hotspots[j].condition == -1) {
			if (!_scriptEngine->run(&nodeData->hotspots[j].script))
				break;
		}
	}
}

void Myst3Engine::loadNodeCubeFaces(uint16 nodeID) {
	_state->setViewType(kCube);

	_cursor->lockPosition(true);
	updateCursor();

	_node = new NodeCube(this, nodeID);
}

void Myst3Engine::loadNodeFrame(uint16 nodeID) {
	_state->setViewType(kFrame);

	_cursor->lockPosition(false);
	updateCursor();

	_node = new NodeFrame(this, nodeID);
}

void Myst3Engine::loadNodeMenu(uint16 nodeID) {
	_state->setViewType(kMenu);

	_cursor->lockPosition(false);
	updateCursor();

	_node = new NodeFrame(this, nodeID);
}

void Myst3Engine::runScriptsFromNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (roomID == 0)
		roomID = _state->getLocationRoom();

	if (ageID == 0)
		ageID = _state->getLocationAge();

	NodePtr nodeData = _db->getNodeData(nodeID, roomID, ageID);

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_state->evaluate(nodeData->scripts[j].condition)) {
			if (!_scriptEngine->run(&nodeData->scripts[j].script))
				break;
		}
	}
}

void Myst3Engine::runBackgroundSoundScriptsFromNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (roomID == 0)
		roomID = _state->getLocationRoom();

	if (ageID == 0)
		ageID = _state->getLocationAge();

	NodePtr nodeData = _db->getNodeData(nodeID, roomID, ageID);

	for (uint j = 0; j < nodeData->backgroundSoundScripts.size(); j++) {
		if (_state->evaluate(nodeData->backgroundSoundScripts[j].condition)) {
			if (!_scriptEngine->run(&nodeData->backgroundSoundScripts[j].script))
				break;
		}
	}
}

void Myst3Engine::loadMovie(uint16 id, uint16 condition, bool resetCond, bool loop) {
	ScriptedMovie *movie;

	if (!_state->getMovieUseBackground()) {
		movie = new ScriptedMovie(this, id);
	} else {
		movie = new ProjectorMovie(this, id, _projectorBackground);
		_projectorBackground = 0;
		_state->setMovieUseBackground(0);
	}

	movie->setCondition(condition);
	movie->setDisableWhenComplete(resetCond);
	movie->setLoop(loop);

	if (_state->getMovieScriptDriven()) {
		movie->setScriptDriven(_state->getMovieScriptDriven());
		_state->setMovieScriptDriven(0);
	}

	if (_state->getMovieStartFrameVar()) {
		movie->setStartFrameVar(_state->getMovieStartFrameVar());
		_state->setMovieStartFrameVar(0);
	}

	if (_state->getMovieEndFrameVar()) {
		movie->setEndFrameVar(_state->getMovieEndFrameVar());
		_state->setMovieEndFrameVar(0);
	}

	if (_state->getMovieStartFrame()) {
		movie->setStartFrame(_state->getMovieStartFrame());
		_state->setMovieStartFrame(0);
	}

	if (_state->getMovieEndFrame()) {
		movie->setEndFrame(_state->getMovieEndFrame());
		_state->setMovieEndFrame(0);
	}

	if (_state->getMovieNextFrameGetVar()) {
		movie->setNextFrameReadVar(_state->getMovieNextFrameGetVar());
		_state->setMovieNextFrameGetVar(0);
	}

	if (_state->getMovieNextFrameSetVar()) {
		movie->setNextFrameWriteVar(_state->getMovieNextFrameSetVar());
		_state->setMovieNextFrameSetVar(0);
	}

	if (_state->getMoviePlayingVar()) {
		movie->setPlayingVar(_state->getMoviePlayingVar());
		_state->setMoviePlayingVar(0);
	}

	if (_state->getMovieOverridePosition()) {
		movie->setPosU(_state->getMovieOverridePosU());
		movie->setPosV(_state->getMovieOverridePosV());
		_state->setMovieOverridePosition(0);
	}

	if (_state->getMovieUVar()) {
		movie->setPosUVar(_state->getMovieUVar());
		_state->setMovieUVar(0);
	}

	if (_state->getMovieVVar()) {
		movie->setPosVVar(_state->getMovieVVar());
		_state->setMovieVVar(0);
	}

	if (_state->getMovieOverrideCondition()) {
		movie->setCondition(_state->getMovieOverrideCondition());
		_state->setMovieOverrideCondition(0);
	}

	if (_state->getMovieConditionBit()) {
		movie->setConditionBit(_state->getMovieConditionBit());
		_state->setMovieConditionBit(0);
	}

	if (_state->getMovieForce2d()) {
		movie->setForce2d(_state->getMovieForce2d());
		_state->setMovieForce2d(0);
	}

	_movies.push_back(movie);
}

void Myst3Engine::playSimpleMovie(uint16 id, bool fullframe) {
	SimpleMovie movie = SimpleMovie(this, id);

	if (_state->getMovieSynchronized()) {
		movie.setSynchronized(_state->getMovieSynchronized());
		_state->setMovieSynchronized(0);
	}

	if (_state->getMovieStartFrame()) {
		movie.setStartFrame(_state->getMovieStartFrame());
		_state->setMovieStartFrame(0);
	}

	if (_state->getMovieEndFrame()) {
		movie.setEndFrame(_state->getMovieEndFrame());
		_state->setMovieEndFrame(0);
	}

	if (fullframe) {
		movie.setForce2d(_state->getViewType() == kCube);
		movie.setPosU(0);
		movie.setPosV(0);
	}

	_drawables.push_back(&movie);

	bool skip = false;

	while (!skip && !shouldQuit() && !_shouldQuit && movie.update()) {
		// Process events
		Common::Event event;
		while (getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_MOUSEMOVE) {
				if (_state->getViewType() == kCube)
					_scene->updateCamera(event.relMouse);

				_cursor->updatePosition(event.relMouse);

			} else if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_SPACE
						|| event.kbd.keycode == Common::KEYCODE_ESCAPE)
					skip = true;
			}

		drawFrame();
	}

	_drawables.pop_back();
}

void Myst3Engine::removeMovie(uint16 id) {
	if (id == 0) {
		for (uint i = 0; i < _movies.size(); i++)
			delete _movies[i];

		_movies.clear();
		return;
	} else {
		for (uint i = 0; i < _movies.size(); i++)
			if (_movies[i]->getId() == id) {
				delete _movies[i];
				_movies.remove_at(i);
				break;
			}
	}
}

void Myst3Engine::setMovieLooping(uint16 id, bool loop) {
	for (uint i = 0; i < _movies.size(); i++) {
		if (_movies[i]->getId() == id) {
			// Enable or disable looping
			_movies[i]->setLoop(loop);
			_movies[i]->setDisableWhenComplete(!loop);
			break;
		}
	}
}

void Myst3Engine::addSpotItem(uint16 id, uint16 condition, bool fade) {
	_node->loadSpotItem(id, condition, fade);
}

void Myst3Engine::addMenuSpotItem(uint16 id, uint16 condition, const Common::Rect &rect) {
	_node->loadMenuSpotItem(id, condition, rect);
}

void Myst3Engine::addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
		uint16 color, uint16 var, bool varControlledIntensity, uint16 radius) {

	SunSpot s;

	s.pitch = pitch;
	s.heading = heading;
	s.intensity = intensity * 2.55;
	s.color = (color & 0xF) | 16
			* ((color & 0xF) | 16
			* (((color >> 4) & 0xF) | 16
			* (((color >> 4) & 0xF) | 16
			* (((color >> 8) & 0xF) | 16
			* (((color >> 8) & 0xF))))));
	s.var = var;
	s.variableIntensity = varControlledIntensity;
	s.radius = radius;

	_node->addSunSpot(s);
}

const DirectorySubEntry *Myst3Engine::getFileDescription(const char* room, uint16 index, uint16 face, DirectorySubEntry::ResourceType type) {
	char currentRoom[8];
	if (!room) {
		_db->getRoomName(currentRoom, _state->getLocationRoom());
		room = currentRoom;
	}

	const DirectorySubEntry *desc = 0;

	// Search common archives
	uint i = 0;
	while (!desc && i < _archivesCommon.size()) {
		desc = _archivesCommon[i]->getDescription(room, index, face, type);
		i++;
	}

	// Search currently loaded node archive
	if (!desc && _archiveNode)
		desc = _archiveNode->getDescription(room, index, face, type);

	return desc;
}

Graphics::Surface *Myst3Engine::loadTexture(uint16 id) {
	const DirectorySubEntry *desc = getFileDescription("GLOB", id, 0, DirectorySubEntry::kCursor);

	if (!desc)
		error("Texture %d does not exist", id);

	Common::MemoryReadStream *data = desc->getData();

	uint32 magic = data->readUint32LE();
	if (magic != 0x2E544558)
		error("Wrong texture format %d", id);

	data->readUint32LE(); // unk 1
	uint32 width = data->readUint32LE();
	uint32 height = data->readUint32LE();
	data->readUint32LE(); // unk 2
	data->readUint32LE(); // unk 3

	Graphics::Surface *s = new Graphics::Surface();
	s->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	data->read(s->pixels, height * s->pitch);
	delete data;

	// ARGB => RGBA
	uint32 *p = (uint32 *)s->pixels;
	for (uint i = 0; i < width * height; i++) {
		uint8 a = (*p >> 0) & 0xFF;
		uint8 r = (*p >> 24) & 0xFF;
		uint8 g = (*p >> 16) & 0xFF;
		uint8 b = (*p >>  8) & 0xFF;

		*p = (a << 24) | (r << 16) | (g << 8) | b;

		p++;
	}

	return s;
}

int16 Myst3Engine::openDialog(uint16 id) {
	Dialog dialog(this, id);

	_drawables.push_back(&dialog);

	int16 result = -1;

	while (result == -1 && !shouldQuit()) {
		result = dialog.update();
		drawFrame();
	}

	_drawables.pop_back();

	return result;
}

bool Myst3Engine::canLoadGameStateCurrently() {
	return true;
}

Common::Error Myst3Engine::loadGameState(int slot) {
	if (_state->load(_saveFileMan->listSavefiles("*.M3S")[slot])) {
		_inventory->loadFromState();

		_state->setLocationNextAge(_state->getMenuSavedAge());
		_state->setLocationNextRoom(_state->getMenuSavedRoom());
		_state->setLocationNextNode(_state->getMenuSavedNode());
		_state->setMenuSavedAge(0);
		_state->setMenuSavedRoom(0);
		_state->setMenuSavedNode(0);

		goToNode(0, 1);
		return Common::kNoError;
	}

	return Common::kUnknownError;
}

void Myst3Engine::animateDirectionChange(float targetPitch, float targetHeading, uint16 scriptFrames) {
	float startPitch = _state->getLookAtPitch();
	float startHeading = _state->getLookAtHeading();

	if (startPitch == targetPitch && startHeading == targetHeading)
		return; // Fast path

	float pitchDistance = targetPitch - startPitch;
	float headingDistance = targetHeading - startHeading;

	// Compute animation duration in frames
	float numFrames;
	if (scriptFrames) {
		numFrames = scriptFrames;
	} else {
		numFrames = sqrt(pitchDistance * pitchDistance + headingDistance * headingDistance)
				* 30.0f / _state->getCameraMoveSpeed();

		if (numFrames > 0.0f)
			numFrames += 10.0f;
	}

	uint startFrame = _state->getFrameCount();

	// Draw animation
	if (numFrames != 0.0f) {
		while (1) {
			uint elapsedFrames = _state->getFrameCount() - startFrame;
			if (elapsedFrames >= numFrames)
				break;

			float step;
			if (numFrames >= 15) {
				// Fast then slow movement
				if (elapsedFrames > numFrames / 2.0f)
					step = 1.0f - (numFrames - elapsedFrames) * (numFrames - elapsedFrames)
								/ (numFrames / 2.0f * numFrames / 2.0f) / 2.0f;
				else
					step = elapsedFrames * elapsedFrames / (numFrames / 2.0f * numFrames / 2.0f) / 2.0f;

			} else {
				// Constant speed movement
				step = elapsedFrames / numFrames;
			}

			float nextPitch = startPitch + pitchDistance * step;
			float nextHeading = startHeading + headingDistance * step;

			_state->lookAt(nextPitch, nextHeading);
			drawFrame();
		}
	}

	_state->lookAt(targetPitch, targetHeading);
	drawFrame();
}

void Myst3Engine::getMovieLookAt(uint16 id, bool start, float &pitch, float &heading) {
	const DirectorySubEntry *desc = getFileDescription(0, id, 0, DirectorySubEntry::kMovie);

	if (!desc)
		desc = getFileDescription(0, id, 0, DirectorySubEntry::kMultitrackMovie);

	if (!desc)
		error("Movie %d does not exist", id);

	Math::Vector3d v;
	if (start)
		v = desc->getVideoData().v1;
	else
		v = desc->getVideoData().v2;

	Math::Vector2d horizontalProjection = Math::Vector2d(v.x(), v.z());
	horizontalProjection.normalize();

	pitch = 90 - Math::Angle::arcCosine(v.y()).getDegrees();
	heading = Math::Angle::arcCosine(horizontalProjection.getY()).getDegrees();

	if (horizontalProjection.getX() < 0.0) {
		heading = 360 - heading;
	}
}

void Myst3Engine::playMovieGoToNode(uint16 movie, uint16 node) {
	uint16 room = _state->getLocationNextRoom();
	uint16 age = _state->getLocationNextAge();

	if (_state->getLocationNextNode() != 0) {
		node = _state->getLocationNextNode();
	}

	if (_state->getViewType() == kCube && !_state->getCameraSkipAnimation()) {
		float startPitch, startHeading;
		getMovieLookAt(movie, true, startPitch, startHeading);
		animateDirectionChange(startPitch, startHeading, 0);
		_state->setCameraSkipAnimation(0);
	}

	loadNode(node, room, age);

	playSimpleMovie(movie, true);

	_state->setLocationNextNode(0);
	_state->setLocationNextRoom(0);
	_state->setLocationNextAge(0);

	if (_state->getViewType() == kCube) {
		float endPitch, endHeading;
		getMovieLookAt(movie, false, endPitch, endHeading);
		_state->lookAt(endPitch, endHeading);
	}
}

void Myst3Engine::playMovieFullFrame(uint16 movie) {
	if (_state->getViewType() == kCube && !_state->getCameraSkipAnimation()) {
		float startPitch, startHeading;
		getMovieLookAt(movie, true, startPitch, startHeading);
		animateDirectionChange(startPitch, startHeading, 0);
		_state->setCameraSkipAnimation(0);
	}

	playSimpleMovie(movie, true);

	if (_state->getViewType() == kCube) {
		float endPitch, endHeading;
		getMovieLookAt(movie, false, endPitch, endHeading);
		_state->lookAt(endPitch, endHeading);
	}
}

} // end of namespace Myst3
