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

#include "graphics/decoders/jpeg.h"
#include "graphics/conversion.h"
#include "graphics/pixelbuffer.h"

#include "math/vector2d.h"

namespace Myst3 {

enum MystLanguage {
	kEnglish,
	kDutch,
	kFrench,
	kGerman,
	kItalian,
	kSpanish
};

Myst3Engine::Myst3Engine(OSystem *syst, const Myst3GameDescription *version) :
		Engine(syst), _system(syst), _gameDescription(version),
		_db(0), _console(0), _scriptEngine(0),
		_state(0), _node(0), _scene(0), _archiveNode(0),
		_cursor(0), _inventory(0), _gfx(0), _menu(0),
		_rnd(0), _sound(0),
		_inputSpacePressed(false), _inputEnterPressed(false),
		_inputEscapePressed(false), _inputTildePressed(false),
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
	SearchMan.addSubDirectoryMatching(gameDataDir, "MYST3BIN/M3DATA/TEXT/PAL");

	settingsInitDefaults();
}

Myst3Engine::~Myst3Engine() {
	DebugMan.clearAllDebugChannels();

	closeArchives();

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
	_db = new Database(this);
	_state = new GameState(this);
	_scene = new Scene(this);
	_menu = new Menu(this);
	_archiveNode = new Archive();

	_system->setupScreen(w, h, false, true);
	_system->showMouse(false);
	_system->lockMouse(true);

	openArchives();

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

	while (!shouldQuit()) {
		runNodeBackgroundScripts();
		processInput(false);
		updateCursor();

		if (_menuAction) {
			_menu->updateMainMenu(_menuAction);
			_menuAction = 0;
		}

		drawFrame();
	}

	unloadNode();

	_archiveNode->close();

	_system->lockMouse(false);

	return Common::kNoError;
}

bool Myst3Engine::addArchive(const Common::String &file, bool mandatory) {
	Archive *archive = new Archive();
	bool opened = archive->open(file.c_str(), 0);

	if (opened) {
		_archivesCommon.push_back(archive);
	} else {
		delete archive;
		if (mandatory)
			error("Unable to open archive %s", file.c_str());
	}

	return opened;
}

void Myst3Engine::openArchives() {
	// The language of the menus is always the same as the executable
	// The English CD version can only display English text
	// The non English CD versions can display their localized language and English
	// The DVD version can display 6 different languages

	Common::String menuLanguage;
	Common::String textLanguage;

	switch (getDefaultLanguage()) {
	case Common::NL_NLD:
		menuLanguage = "DUTCH";
		break;
	case Common::FR_FRA:
		menuLanguage = "FRENCH";
		break;
	case Common::DE_DEU:
		menuLanguage = "GERMAN";
		break;
	case Common::IT_ITA:
		menuLanguage = "ITALIAN";
		break;
	case Common::ES_ESP:
		menuLanguage = "SPANISH";
		break;
	case Common::JA_JPN:
		menuLanguage = "JAPANESE";
		break;
	case Common::PL_POL:
		menuLanguage = "POLISH";
		break;
	case Common::EN_ANY:
	default:
		menuLanguage = "ENGLISH";
		break;
	}

	if (getExecutableVersion()->flags & kFlagDVD) {
		switch (ConfMan.getInt("text_language")) {
		case kDutch:
			textLanguage = "DUTCH";
			break;
		case kFrench:
			textLanguage = "FRENCH";
			break;
		case kGerman:
			textLanguage = "GERMAN";
			break;
		case kItalian:
			textLanguage = "ITALIAN";
			break;
		case kSpanish:
			textLanguage = "SPANISH";
			break;
		case kEnglish:
		default:
			textLanguage = "ENGLISH";
			break;
		}
	} else {
		if (getDefaultLanguage() == Common::EN_ANY
				|| ConfMan.getInt("text_language")) {
			textLanguage = menuLanguage;
		} else {
			textLanguage = "ENGLISHjp";
		}
	}

	addArchive("OVERMainMenuLogo.m3o", false);
	addArchive("OVER101.m3o", false);
	addArchive(textLanguage + ".m3t", true);

	if (getExecutableVersion()->flags & kFlagDVD || getDefaultLanguage() != Common::EN_ANY)
		if (!addArchive("language.m3u", false))
			addArchive(menuLanguage + ".m3u", true);

	addArchive("RSRC.m3r", true);
}

void Myst3Engine::closeArchives() {
	for (uint i = 0; i < _archivesCommon.size(); i++)
		delete _archivesCommon[i];
	_archivesCommon.clear();
}

HotSpot *Myst3Engine::getHoveredHotspot(NodePtr nodeData, uint16 var) {
	_state->setHotspotHovered(false);
	_state->setHotspotActiveRect(0);

	if (_state->getViewType() == kCube) {
		float pitch, heading;
		_cursor->getDirection(pitch, heading);

		Common::Point mouse = Common::Point((int16)heading, (int16)pitch);

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			int32 hitRect = nodeData->hotspots[j].isPointInRectsCube(mouse);
			if (hitRect >= 0
					&& nodeData->hotspots[j].isEnabled(_state, var)) {
				if (nodeData->hotspots[j].rects.size() > 1) {
					_state->setHotspotHovered(true);
					_state->setHotspotActiveRect(hitRect);
				}
				return &nodeData->hotspots[j];
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
							- Renderer::kTopBorderHeight, 0, Renderer::kFrameHeight));
		}

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			int32 hitRect = nodeData->hotspots[j].isPointInRectsFrame(_state, scaledMouse);
			if (hitRect >= 0
					&& nodeData->hotspots[j].isEnabled(_state, var)) {
				if (nodeData->hotspots[j].rects.size() > 1) {
					_state->setHotspotHovered(true);
					_state->setHotspotActiveRect(hitRect);
				}
				return &nodeData->hotspots[j];
			}
		}
	}

	return 0;
}

void Myst3Engine::updateCursor() {
	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());

	HotSpot *hovered = getHoveredHotspot(nodeData);
	uint16 hoveredInventory = _inventory->hoveredItem();

	if (hovered) {
		_cursor->changeCursor(hovered->cursor);
	} else if (isInventoryVisible() && hoveredInventory > 0) {
		_cursor->changeCursor(1);
	} else {
		_cursor->changeCursor(8);
	}
}

void Myst3Engine::processInput(bool lookOnly) {
	// Process events
	Common::Event event;
	while (getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_state->getViewType() == kCube
					&& _cursor->isPositionLocked()) {
				_scene->updateCamera(event.relMouse);
			}

			_cursor->updatePosition(event.relMouse);

		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			// Skip the event when in look only mode
			if (lookOnly) continue;

			uint16 hoveredInventory = _inventory->hoveredItem();
			if (isInventoryVisible() && hoveredInventory > 0) {
				_inventory->useItem(hoveredInventory);
				continue;
			}

			NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());
			HotSpot *hovered = getHoveredHotspot(nodeData);

			if (hovered) {
				_scriptEngine->run(&hovered->script);
				continue;
			}

			// Bad click
			_sound->playEffect(697, 5);
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			// Skip the event when in look only mode
			if (lookOnly) continue;
			// Nothing to do if not in cube view
			if (_state->getViewType() != kCube) continue;
			// Don't unlock if the cursor is transparent
			if (!_state->getCursorTransparency()) continue;

			bool cursorLocked = _cursor->isPositionLocked();
			_cursor->lockPosition(!cursorLocked);

		} else if (event.type == Common::EVENT_KEYDOWN) {
			// Save file name input
			_menu->handleInput(event.kbd);

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_inputEscapePressed = true;

				// Open main menu
				if (_cursor->isVisible()) {
					if (_state->getLocationRoom() != 901)
						_menu->goToNode(100);
					else
						_state->setMenuEscapePressed(1);
				}
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				_inputEnterPressed = true;
				break;
			case Common::KEYCODE_SPACE:
				_inputSpacePressed = true;
				break;
			case Common::KEYCODE_TILDE:
				_inputTildePressed = true;
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
		} else if (event.type == Common::EVENT_KEYUP) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_inputEscapePressed = false;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				_inputEnterPressed = false;
				break;
			case Common::KEYCODE_SPACE:
				_inputSpacePressed = false;
				break;
			case Common::KEYCODE_TILDE:
				_inputTildePressed = false;
				break;
			default:
				break;
			}
		}
	}
}

void Myst3Engine::drawFrame() {
	_sound->update();
	_gfx->clear();

	if (_state->getViewType() == kCube) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		float fov = _state->getLookAtFOV();
		_gfx->setupCameraPerspective(pitch, heading, fov);
	} else {
		_gfx->setupCameraOrtho2D();
	}

	if (_node) {
		_node->update();
		_node->draw();
	}

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
	}

	if (_state->getViewType() != kMenu) {
		float pitch = _state->getLookAtPitch();
		float heading = _state->getLookAtHeading();
		SunSpot flare = computeSunspotsIntensity(pitch, heading);
		if (flare.intensity >= 0)
			_scene->drawSunspotFlare(flare);

		_scene->drawBlackBorders();
	}

	if (isInventoryVisible())
		_inventory->draw();

	// Draw overlay 2D movies
	for (uint i = 0; i < _movies.size(); i++) {
		_movies[i]->drawOverlay();
	}

	for (uint i = 0; i < _drawables.size(); i++) {
		_drawables[i]->drawOverlay();
	}

	// Draw spot subtitles
	if (_node) {
		_node->drawOverlay();
	}

	if (_cursor->isVisible())
		_cursor->draw();

	_system->updateScreen();
	_system->delayMillis(10);
	_state->updateFrameCounters();
}

bool Myst3Engine::isInventoryVisible() {
	if (_state->getViewType() == kMenu)
		return false;

	if (_node && _node->hasSubtitlesToDraw())
		return false;

	return true;
}

void Myst3Engine::goToNode(uint16 nodeID, uint transition) {
	uint16 node = _state->getLocationNextNode();
	if (node == 0)
		node = nodeID;

	uint16 room = _state->getLocationNextRoom();
	uint16 age = _state->getLocationNextAge();

	if (_state->getViewType() == kCube) {
		// The lookat direction in the next node should be
		// the direction of the mouse cursor
		float pitch, heading;
		_cursor->getDirection(pitch, heading);
		_state->lookAt(pitch, heading);
	}

	loadNode(node, room, age);

	_state->setLocationNextNode(0);
	_state->setLocationNextRoom(0);
	_state->setLocationNextAge(0);
}

void Myst3Engine::loadNode(uint16 nodeID, uint32 roomID, uint32 ageID) {
	unloadNode();

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

	// WORKAROUND: In Narayan, the scripts in node NACH 9 test on var 39
	// without first reinitializing it leading to Saavedro not always giving
	// Releeshan to the player when he is trapped between both shields.
	if (nodeID == 9 && roomID == 801) _state->setVar(39, 0);
}

void Myst3Engine::unloadNode() {
	if (!_node)
		return;

	// Delete all movies
	removeMovie(0);

	// Remove all sunspots
	for (uint i = 0; i < _sunspots.size(); i++)
		delete _sunspots[i];
	_sunspots.clear();

	delete _node;
	_node = 0;
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

	if (!nodeData) return;

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

	while (!skip && !shouldQuit() && movie.update()) {
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
	assert(_node);

	_node->loadSpotItem(id, condition, fade);
}

void Myst3Engine::addMenuSpotItem(uint16 id, uint16 condition, const Common::Rect &rect) {
	assert(_node);

	_node->loadMenuSpotItem(id, condition, rect);
}

void Myst3Engine::loadNodeSubtitles(uint32 id) {
	assert(_node);

	_node->loadSubtitles(id);
}

const DirectorySubEntry *Myst3Engine::getFileDescription(const char* room, uint32 index, uint16 face, DirectorySubEntry::ResourceType type) {
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

void Myst3Engine::dragSymbol(uint16 var, uint16 id) {
	DragItem drag(this, id);

	_drawables.push_back(&drag);

	_cursor->changeCursor(2);
	_state->setVar(var, -1);

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());

	while (inputValidatePressed() && !shouldQuit()) {
		processInput(true);
		drawFrame();
	}

	_state->setVar(var, 1);
	_drawables.pop_back();

	HotSpot *hovered = getHoveredHotspot(nodeData, var);
	if (hovered)
		_scriptEngine->run(&hovered->script);
}

void Myst3Engine::dragItem(uint16 statusVar, uint16 movie, uint16 frame, uint16 hoverFrame, uint16 itemVar) {
	DragItem drag(this, movie);

	_drawables.push_back(&drag);

	_cursor->changeCursor(2);
	_state->setVar(statusVar, 0);
	_state->setVar(itemVar, 1);

	NodePtr nodeData = _db->getNodeData(_state->getLocationNode(), _state->getLocationRoom());

	while (inputValidatePressed() && !shouldQuit()) {
		processInput(true);

		HotSpot *hovered = getHoveredHotspot(nodeData, itemVar);
		drag.setFrame(hovered ? hoverFrame : frame);

		drawFrame();
	}

	_drawables.pop_back();

	HotSpot *hovered = getHoveredHotspot(nodeData, itemVar);
	if (hovered) {
		_scriptEngine->run(&hovered->script);
	} else {
		_state->setVar(statusVar, 1);
		_state->setVar(itemVar, 0);
	}
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

bool Myst3Engine::inputValidatePressed() {
	return _inputEnterPressed ||
			_inputSpacePressed ||
			getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
}

bool Myst3Engine::inputEscapePressed() {
	return _inputEscapePressed;
}

bool Myst3Engine::inputSpacePressed() {
	return _inputSpacePressed;
}

bool Myst3Engine::inputTilePressed() {
	return _inputTildePressed;
}

void Myst3Engine::addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
		uint16 color, uint16 var, bool varControlledIntensity, uint16 radius) {

	SunSpot *s = new SunSpot();

	s->pitch = pitch;
	s->heading = heading;
	s->intensity = intensity * 2.55;
	s->color = (color & 0xF) | 16
			* ((color & 0xF) | 16
			* (((color >> 4) & 0xF) | 16
			* (((color >> 4) & 0xF) | 16
			* (((color >> 8) & 0xF) | 16
			* (((color >> 8) & 0xF))))));
	s->var = var;
	s->variableIntensity = varControlledIntensity;
	s->radius = radius;

	_sunspots.push_back(s);
}

SunSpot Myst3Engine::computeSunspotsIntensity(float pitch, float heading) {
	SunSpot result;
	result.intensity = -1;
	result.color = 0;
	result.radius = 0;

	for (uint i = 0; i < _sunspots.size(); i++) {
		SunSpot *s = _sunspots[i];

		uint32 value = _state->getVar(s->var);

		// Skip disabled items
		if (value == 0) continue;

		float distance = _scene->distanceToZone(s->heading, s->pitch, s->radius, heading, pitch);

		if (distance > result.radius) {
			result.radius = distance;
			result.color = s->color;
			result.intensity = s->intensity;

			if (s->variableIntensity) {
				result.radius = value * distance / 100;
			}
		}
	}

	return result;
}

void Myst3Engine::settingsInitDefaults() {
	Common::Language executableLanguage = getDefaultLanguage();
	int defaultLanguage;

	switch (executableLanguage) {
	case Common::NL_NLD:
		defaultLanguage = kDutch;
		break;
	case Common::FR_FRA:
		defaultLanguage = kFrench;
		break;
	case Common::DE_DEU:
		defaultLanguage = kGerman;
		break;
	case Common::IT_ITA:
		defaultLanguage = kItalian;
		break;
	case Common::ES_ESP:
		defaultLanguage = kSpanish;
		break;
	case Common::EN_ANY:
	default:
		defaultLanguage = kEnglish;
		break;
	}

	int defaultTextLanguage;
	if (getExecutableVersion()->flags & kFlagDVD)
		defaultTextLanguage = defaultLanguage;
	else
		defaultTextLanguage = executableLanguage != Common::EN_ANY;

	ConfMan.registerDefault("overall_volume", Audio::Mixer::kMaxMixerVolume);
	ConfMan.registerDefault("music_volume", Audio::Mixer::kMaxMixerVolume / 2);
	ConfMan.registerDefault("music_frequency", 75);
	ConfMan.registerDefault("audio_language", defaultLanguage);
	ConfMan.registerDefault("text_language", defaultTextLanguage);
	ConfMan.registerDefault("water_effects", true);
	ConfMan.registerDefault("transition_speed", 50);
	ConfMan.registerDefault("mouse_speed", 50);
	ConfMan.registerDefault("zip_mode", false);
	ConfMan.registerDefault("subtitles", false);
}

void Myst3Engine::settingsLoadToVars() {
	_state->setOverallVolume(CLIP<uint>(ConfMan.getInt("overall_volume") * 100 / 256, 0, 100));
	_state->setMusicVolume(CLIP<uint>(ConfMan.getInt("music_volume") * 100 / 256, 0, 100));
	_state->setMusicFrequency(ConfMan.getInt("music_frequency"));
	_state->setLanguageAudio(ConfMan.getInt("audio_language"));
	_state->setLanguageText(ConfMan.getInt("text_language"));
	_state->setWaterEffects(ConfMan.getBool("water_effects"));
	_state->setTransitionSpeed(ConfMan.getInt("transition_speed"));
	_state->setMouseSpeed(ConfMan.getInt("mouse_speed"));
	_state->setZipModeEnabled(ConfMan.getBool("zip_mode"));
	_state->setSubtitlesEnabled(ConfMan.getBool("subtitles"));
}

void Myst3Engine::settingsApplyFromVars() {
	int32 oldTextLanguage = ConfMan.getInt("text_language");

	ConfMan.setInt("overall_volume", _state->getOverallVolume() * 256 / 100);
	ConfMan.setInt("music_volume", _state->getMusicVolume() * 256 / 100);
	ConfMan.setInt("music_frequency", _state->getMusicFrequency());
	ConfMan.setInt("audio_language", _state->getLanguageAudio());
	ConfMan.setInt("text_language", _state->getLanguageText());
	ConfMan.setBool("water_effects", _state->getWaterEffects());
	ConfMan.setInt("transition_speed", _state->getTransitionSpeed());
	ConfMan.setInt("mouse_speed", _state->getMouseSpeed());
	ConfMan.setBool("zip_mode", _state->getZipModeEnabled());
	ConfMan.setBool("subtitles", _state->getSubtitlesEnabled());

	// The language changed, reload the correct archives
	if (_state->getLanguageText() != oldTextLanguage) {
		closeArchives();
		openArchives();
	}
}

} // end of namespace Myst3
