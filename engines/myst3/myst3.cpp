/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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

#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/nodecube.h"
#include "engines/myst3/nodeframe.h"
#include "engines/myst3/variables.h"

#include "graphics/jpeg.h"
#include "graphics/conversion.h"

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Myst3 {

Myst3Engine::Myst3Engine(OSystem *syst, int gameFlags) :
		Engine(syst), _system(syst),
		_db(0), _console(0), _scriptEngine(0),
		_vars(0), _node(0), _scene(0), _archive(0),
		_frameCount(0), _rnd(0), _shouldQuit(false) {
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kDebugNode, "Node", "Track Node Changes");

	// Add subdirectories to the search path to allow running from a full HDD install
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "bin");
	SearchMan.addSubDirectoryMatching(gameDataDir, "M3Data");
	SearchMan.addSubDirectoriesMatching(gameDataDir, "EXILE Disc ?/Data", true);
}

Myst3Engine::~Myst3Engine() {
	DebugMan.clearAllDebugChannels();

	delete _cursor;
	delete _scene;
	delete _archive;
	delete _db;
	delete _scriptEngine;
	delete _console;
	delete _vars;
	delete _rnd;
}

Common::Error Myst3Engine::run() {
	const int w = 640;
	const int h = 480;

	_rnd = new Common::RandomSource("sprint");
	_console = new Console(this);
	_scriptEngine = new Script(this);
	_db = new Database("M3.exe");
	_vars = new Variables(this);
	_scene = new Scene();
	_archive = new Archive();

	_system->setupScreen(w, h, false, true);

	Archive *archiveRSRC = new Archive();
	archiveRSRC->open("RSRC.m3r");
	_cursor = new Cursor(archiveRSRC);
	archiveRSRC->close();
	delete archiveRSRC;

	_scene->init(w, h);

	// Var init script
	runScriptsFromNode(1000, 101);

	goToNode(1, 245); // LEIS
	
	while (!_shouldQuit) {
		runNodeBackgroundScripts();
		processInput(false);
		drawFrame();
	}

	for (uint i = 0; i < _movies.size(); i++) {
		delete _movies[i];
	}
	_movies.clear();

	delete _node;

	_archive->close();

	return Common::kNoError;
}

Common::Array<HotSpot *> Myst3Engine::listHoveredHotspots() {
	Common::Array<HotSpot *> hovered;

	NodePtr nodeData = _db->getNodeData(_vars->getLocationNode());
	if (_viewType == kCube) {
		Common::Point mouse = _scene->getMousePos();

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			if (nodeData->hotspots[j].isPointInRectsCube(mouse)) {
				hovered.push_back(&nodeData->hotspots[j]);
			}
		}
	} else if (_viewType == kFrame) {
		Common::Point mouse = _system->getEventManager()->getMousePos();
		Common::Point scaledMouse = Common::Point(
				mouse.x * Scene::_originalWidth / _system->getWidth(),
				CLIP<uint>(mouse.y * Scene::_originalHeight / _system->getHeight()
						- Scene::_topBorderHeight, 0, Scene::_frameHeight));

		for (uint j = 0; j < nodeData->hotspots.size(); j++) {
			if (nodeData->hotspots[j].isPointInRectsFrame(scaledMouse)) {
				hovered.push_back(&nodeData->hotspots[j]);
			}
		}
	}

	return hovered;
}

void Myst3Engine::updateCursor() {
	Common::Array<HotSpot *> hovered = listHoveredHotspots();
	if (hovered.size() > 0) {
		HotSpot *h = hovered.back();
		_cursor->changeCursor(h->cursor);
	} else {
		_cursor->changeCursor(8);
	}
}

void Myst3Engine::processInput(bool lookOnly) {
	// Process events
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		// Check for "Hard" quit"
		if (event.type == Common::EVENT_QUIT) {
			_shouldQuit = true;
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_viewType == kCube) {
				_scene->updateCamera(event.relMouse);
			}

			updateCursor();

		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			// Skip the event when in look only mode
			if (lookOnly) continue;

			Common::Array<HotSpot *> hovered = listHoveredHotspots();

			for (uint j = 0; j < hovered.size(); j++) {
				if (_vars->evaluate(hovered[j]->condition)) {
					_scriptEngine->run(&hovered[j]->script);
				}
			}
			
		} else if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
				break;
			default:
				break;
			}
		}
	}
}

void Myst3Engine::drawFrame() {
	_scene->clear();

	if (_viewType == kCube) {
		_scene->setupCameraPerspective();
	} else {
		_scene->setupCameraOrtho2D();
	}

	_node->update();
	_node->draw();

	for (uint i = 0; i < _movies.size(); i++) {
		_movies[i]->update();
		_movies[i]->draw();
	}

	for (uint i = 0; i < _drawables.size(); i++) {
		_drawables[i]->draw();
	}

	if (_viewType == kCube) {
		_scene->setupCameraOrtho2D();
	}

	_scene->drawBlackBorders();
	_cursor->draw();

	_system->updateScreen();
	_system->delayMillis(10);
	_frameCount++;
}

void Myst3Engine::goToNode(uint16 nodeID, uint8 roomID) {
	if (_node) {
		for (uint i = 0; i < _movies.size(); i++) {
			delete _movies[i];
		}
		_movies.clear();

		delete _node;
		_node = 0;
	}

	loadNode(nodeID, roomID);
}

void Myst3Engine::loadNode(uint16 nodeID, uint8 roomID, uint32 ageID) {
	_scriptEngine->run(&_db->getNodeInitScript());

	if (nodeID)
		_vars->setLocationNode(_vars->valueOrVarValue(nodeID));

	if (roomID)
		_vars->setLocationRoom(_vars->valueOrVarValue(roomID));

	if (ageID)
		_vars->setLocationAge(_vars->valueOrVarValue(ageID));

	char oldRoomName[8];
	char newRoomName[8];
	_db->getRoomName(oldRoomName);
	_db->getRoomName(newRoomName, roomID);

	if (strcmp(newRoomName, "JRNL") && strcmp(newRoomName, "XXXX")
			 && strcmp(newRoomName, "MENU") && strcmp(newRoomName, oldRoomName)) {

		_db->setCurrentRoom(roomID);
		Common::String nodeFile = Common::String::format("%snodes.m3a", newRoomName);

		_archive->close();
		if (!_archive->open(nodeFile.c_str())) {
			error("Unable to open archive %s", nodeFile.c_str());
		}
	}

	runNodeInitScripts();
}

void Myst3Engine::runNodeInitScripts() {
	NodePtr nodeData = _db->getNodeData(
			_vars->getLocationNode(),
			_vars->getLocationRoom(),
			_vars->getLocationAge());

	NodePtr nodeDataInit = _db->getNodeData(32765);
	if (nodeDataInit)
		runScriptsFromNode(32765);

	if (!nodeData)
		error("Node %d unknown in the database", _vars->getLocationNode());

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_vars->evaluate(nodeData->scripts[j].condition)) {
			_scriptEngine->run(&nodeData->scripts[j].script);
		}
	}
}

void Myst3Engine::runNodeBackgroundScripts() {
	NodePtr nodeDataRoom = _db->getNodeData(32675);

	if (nodeDataRoom) {
		for (uint j = 0; j < nodeDataRoom->hotspots.size(); j++) {
			if (nodeDataRoom->hotspots[j].condition == -1) {
				if (!_scriptEngine->run(&nodeDataRoom->hotspots[j].script))
					break;
			}
		}
	}

	NodePtr nodeData = _db->getNodeData(_vars->getLocationNode());

	for (uint j = 0; j < nodeData->hotspots.size(); j++) {
		if (nodeData->hotspots[j].condition == -1) {
			if (!_scriptEngine->run(&nodeData->hotspots[j].script))
				break;
		}
	}
}

void Myst3Engine::loadNodeCubeFaces(uint16 nodeID) {
	_viewType = kCube;

	updateCursor();
	_system->showMouse(false);

	_node = new NodeCube(this, _archive, nodeID);
}

void Myst3Engine::loadNodeFrame(uint16 nodeID) {
	_viewType = kFrame;

	updateCursor();
	_system->showMouse(true);

	_node = new NodeFrame(this, _archive, nodeID);
}

void Myst3Engine::runScriptsFromNode(uint16 nodeID, uint8 roomID, uint32 ageID) {
	NodePtr nodeData = _db->getNodeData(nodeID, roomID, ageID);

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_vars->evaluate(nodeData->scripts[j].condition)) {
			if (!_scriptEngine->run(&nodeData->scripts[j].script))
				break;
		}
	}
}

void Myst3Engine::loadMovie(uint16 id, uint16 condition, bool resetCond, bool loop) {
	ScriptedMovie *movie = new ScriptedMovie(this, _archive, id);
	movie->setCondition(condition);
	movie->setDisableWhenComplete(resetCond);
	movie->setLoop(loop);

	if (_vars->getMovieScriptDriven()) {
		movie->setScriptDriven(_vars->getMovieScriptDriven());
		_vars->setMovieScriptDriven(0);
	}

	if (_vars->getMovieStartFrameVar()) {
		movie->setStartFrameVar(_vars->getMovieStartFrameVar());
		_vars->setMovieStartFrameVar(0);
	}

	if (_vars->getMovieEndFrameVar()) {
		movie->setEndFrameVar(_vars->getMovieEndFrameVar());
		_vars->setMovieEndFrameVar(0);
	}

	if (_vars->getMovieStartFrame()) {
		movie->setStartFrame(_vars->getMovieStartFrame());
		_vars->setMovieStartFrame(0);
	}

	if (_vars->getMovieEndFrame()) {
		movie->setEndFrame(_vars->getMovieEndFrame());
		_vars->setMovieEndFrame(0);
	}

	if (_vars->getMovieNextFrameGetVar()) {
		movie->setNextFrameReadVar(_vars->getMovieNextFrameGetVar());
		_vars->setMovieNextFrameGetVar(0);
	}

	if (_vars->getMovieNextFrameSetVar()) {
		movie->setNextFrameWriteVar(_vars->getMovieNextFrameSetVar());
		_vars->setMovieNextFrameSetVar(0);
	}

	if (_vars->getMoviePlayingVar()) {
		movie->setPlayingVar(_vars->getMoviePlayingVar());
		_vars->setMoviePlayingVar(0);
	}

	if (_vars->getMovieOverridePosU()) {
		movie->setPosU(_vars->getMovieOverridePosU());
		_vars->setMovieOverridePosU(0);
	}

	if (_vars->getMovieOverridePosV()) {
		movie->setPosV(_vars->getMovieOverridePosV());
		_vars->setMovieOverridePosV(0);
	}

	if (_vars->getMovieUVar()) {
		movie->setPosUVar(_vars->getMovieUVar());
		_vars->setMovieUVar(0);
	}

	if (_vars->getMovieVVar()) {
		movie->setPosVVar(_vars->getMovieVVar());
		_vars->setMovieVVar(0);
	}

	if (_vars->getMovieOverrideCondition()) {
		movie->setCondition(_vars->getMovieOverrideCondition());
		_vars->setMovieOverrideCondition(0);
	}

	if (_vars->getMovieConditionBit()) {
		movie->setConditionBit(_vars->getMovieConditionBit());
		_vars->setMovieConditionBit(0);
	}

	_movies.push_back(movie);
}

void Myst3Engine::playSimpleMovie(uint16 id) {
	SimpleMovie movie = SimpleMovie(this, _archive, id);

	if (_vars->getMovieSynchronized()) {
		movie.setSynchronized(_vars->getMovieSynchronized());
		_vars->setMovieSynchronized(0);
	}

	if (_vars->getMovieStartFrame()) {
		movie.setStartFrame(_vars->getMovieStartFrame());
		_vars->setMovieStartFrame(0);
	}

	if (_vars->getMovieEndFrame()) {
		movie.setEndFrame(_vars->getMovieEndFrame());
		_vars->setMovieEndFrame(0);
	}

	_drawables.push_back(&movie);

	while (movie.update()) {
		processInput(true);
		drawFrame();
	}

	_drawables.pop_back();
}

void Myst3Engine::addSpotItem(uint16 id, uint16 condition, bool fade) {
	_node->loadSpotItem(*_archive, id, condition, fade);
}

} // end of namespace Myst3
