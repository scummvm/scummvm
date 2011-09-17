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
		_vars(0), _node(0), _scene(0), _archive(0) {
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Track Script Execution");
	DebugMan.addDebugChannel(kDebugNode, "Node", "Track Node Changes");
}

Myst3Engine::~Myst3Engine() {
	DebugMan.clearAllDebugChannels();

	delete _scene;
	delete _archive;
	delete _db;
	delete _scriptEngine;
	delete _console;
	delete _vars;
}

Common::Error Myst3Engine::run() {
	const int w = 800;
	const int h = 600;

	_console = new Console(this);
	_scriptEngine = new Script(this);
	_db = new Database("M3.exe");
	_vars = new Variables(this);
	_scene = new Scene();
	_archive = new Archive();

	_system->setupScreen(w, h, false, true);

	_scene->init(w, h);

	// Var init script
	runScriptsFromNode(1000, 101);

	goToNode(1, 245); // LEIS
	
	for(;;) {
		// Process events
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Check for "Hard" quit"
			if (event.type == Common::EVENT_QUIT) {
				return Common::kNoError;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				if (_viewType == kCube) {
					_scene->updateCamera(event.relMouse);
				}
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				NodePtr nodeData = _db->getNodeData(_currentNode);
				if (_viewType == kCube) {
					Common::Point mouse = _scene->getMousePos();

					for (uint j = 0; j < nodeData->hotspots.size(); j++) {
						if (nodeData->hotspots[j].isPointInRectsCube(mouse)) {
							_scriptEngine->run(&nodeData->hotspots[j].script);

						}
					}
				} else if (_viewType == kFrame) {
					static const uint originalWidth = 640;
					static const uint originalHeight = 480;
					static const uint frameHeight = 360;

					Common::Point mouse = _system->getEventManager()->getMousePos();
					Common::Point scaledMouse = Common::Point(
							mouse.x * originalWidth / _system->getWidth(),
							CLIP<uint>(mouse.y * originalHeight / _system->getHeight()
									- (originalHeight - 360) / 2, 0, frameHeight));

					for (uint j = 0; j < nodeData->hotspots.size(); j++) {
						if (nodeData->hotspots[j].isPointInRectsFrame(scaledMouse)) {
							_scriptEngine->run(&nodeData->hotspots[j].script);
						}
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
		
		_scene->clear();

		if (_viewType == kFrame) {
			_scene->setupCameraFrame();
		} else {
			_scene->setupCameraCube();
		}
		_node->draw();

		_system->updateScreen();
		_system->delayMillis(10);
	}

	_node->unload();
	delete _node;

	_archive->close();

	return Common::kNoError;
}

void Myst3Engine::goToNode(uint16 nodeID, uint8 roomID) {
	if (_node) {
		_node->unload();
		delete _node;
		_node = 0;
	}

	loadNode(nodeID, roomID);
}

void Myst3Engine::loadNode(uint16 nodeID, uint8 roomID, uint32 ageID) {
	_scriptEngine->run(&_db->getNodeInitScript());

	if (nodeID)
		_currentNode = _vars->valueOrVarValue(nodeID);

	if (roomID)
		_currentRoom = _vars->valueOrVarValue(roomID);

	if (ageID)
		_currentAge = _vars->valueOrVarValue(ageID);

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
	NodePtr nodeData = _db->getNodeData(_currentNode, _currentRoom, _currentAge);

	NodePtr nodeDataInit = _db->getNodeData(32765);
	if (nodeDataInit)
		runScriptsFromNode(32765);

	for (uint j = 0; j < nodeData->scripts.size(); j++) {
		if (_vars->evaluate(nodeData->scripts[j].condition)) {
			_scriptEngine->run(&nodeData->scripts[j].script);
		}
	}
}

void Myst3Engine::loadNodeCubeFaces(uint16 nodeID) {
	_viewType = kCube;

	_system->showMouse(false);

	_node = new NodeCube();
	_node->load(*_archive, nodeID);
}

void Myst3Engine::loadNodeFrame(uint16 nodeID) {
	_viewType = kFrame;

	_system->showMouse(true);

	_node = new NodeFrame();
	_node->load(*_archive, nodeID);
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

} // end of namespace Myst3
