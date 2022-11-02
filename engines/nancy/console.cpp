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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/events.h"

#include "audio/audiostream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/console.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

NancyConsole::NancyConsole() : GUI::Debugger() {
	registerCmd("load_cal", WRAP_METHOD(NancyConsole, Cmd_loadCal));
	registerCmd("cif_hexdump", WRAP_METHOD(NancyConsole, Cmd_cifHexDump));
	registerCmd("cif_export", WRAP_METHOD(NancyConsole, Cmd_cifExport));
	registerCmd("cif_list", WRAP_METHOD(NancyConsole, Cmd_cifList));
	registerCmd("cif_info", WRAP_METHOD(NancyConsole, Cmd_cifInfo));
	registerCmd("chunk_hexdump", WRAP_METHOD(NancyConsole, Cmd_chunkHexDump));
	registerCmd("chunk_list", WRAP_METHOD(NancyConsole, Cmd_chunkList));
	registerCmd("show_image", WRAP_METHOD(NancyConsole, Cmd_showImage));
	registerCmd("play_video", WRAP_METHOD(NancyConsole, Cmd_playVideo));
	registerCmd("play_audio", WRAP_METHOD(NancyConsole, Cmd_playAudio));
	registerCmd("load_scene", WRAP_METHOD(NancyConsole, Cmd_loadScene));
	registerCmd("scene_id", WRAP_METHOD(NancyConsole, Cmd_sceneID));
}

NancyConsole::~NancyConsole() {
}

void NancyConsole::postEnter() {
	GUI::Debugger::postEnter();
	if (!_videoFile.empty()) {
		Video::VideoDecoder *dec = new AVFDecoder;

		if (dec->loadFile(_videoFile)) {
			dec->start();
			Common::EventManager *ev = g_system->getEventManager();
			while (!g_nancy->shouldQuit() && !dec->endOfVideo()) {
				Common::Event event;
				if (ev->pollEvent(event)) {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == Nancy::InputManager::kNancyActionLeftClick) {
						break;
					}
				}

				if (dec->needsUpdate()) {
					const Graphics::Surface *frame = dec->decodeNextFrame();
					if (frame) {
						g_nancy->_graphicsManager->debugDrawToScreen(*frame);
					}
				}

				g_system->delayMillis(10);
			}

			g_nancy->_graphicsManager->redrawAll();
		} else {
			debugPrintf("Failed to load '%s'\n", _videoFile.c_str());
		}

		_videoFile.clear();
		delete dec;
	}

	if (!_imageFile.empty()) {
		Graphics::Surface surf;
		if (g_nancy->_resource->loadImage(_imageFile, surf)) {
			g_nancy->_graphicsManager->debugDrawToScreen(surf);
			surf.free();

			Common::EventManager *ev = g_system->getEventManager();
			while (!g_nancy->shouldQuit()) {
				Common::Event event;
				if (ev->pollEvent(event)) {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == Nancy::InputManager::kNancyActionLeftClick) {
						break;
					}

					g_system->updateScreen();
				}

				g_system->delayMillis(10);
			}

			g_nancy->_graphicsManager->redrawAll();
		} else {
			debugPrintf("Failed to load image '%s'\n", _imageFile.c_str());
		}

		_imageFile.clear();
	}

	// After calling the console, action end events get sent to it and the input manager
	// can still think a keyboard button is being held when it is not; clearing all inputs fixes that
	g_nancy->_input->forceCleanInput();
}

bool NancyConsole::Cmd_cifHexDump(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Dumps the specified resource to standard output\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	uint size;
	byte *buf = g_nancy->_resource->loadCif((argc == 2 ? "ciftree" : argv[2]), argv[1], size);
	if (!buf) {
		debugPrintf("Failed to load resource '%s'\n", argv[1]);
		return true;
	}

	Common::hexdump(buf, size);
	delete[] buf;
	return true;
}

bool NancyConsole::Cmd_cifExport(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Exports the specified resource to .cif file\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	if (!g_nancy->_resource->exportCif((argc == 2 ? "ciftree" : argv[2]), argv[1]))
		debugPrintf("Failed to export '%s'\n", argv[1]);

	return true;
}

bool NancyConsole::Cmd_cifList(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("List resources of a certain type\n");
		debugPrintf("Types - 0: all, 2: image, 3: script\n");
		debugPrintf("Usage: %s type [cal]\n", argv[0]);
		return true;
	}

	Common::Array<Common::String> list;
	g_nancy->_resource->list((argc == 2 ? "ciftree" : argv[2]), list, atoi(argv[1]));
	for (uint i = 0; i < list.size(); i++) {
		debugPrintf("%-38s", list[i].c_str());
		if ((i % 2) == 1 && i + 1 != list.size())
			debugPrintf("\n");
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_cifInfo(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Prints information about a resource\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	debugPrintf("%s", g_nancy->_resource->getCifDescription((argc == 2 ? "ciftree" : argv[2]), argv[1]).c_str());
	return true;
}

bool NancyConsole::Cmd_chunkHexDump(int argc, const char **argv) {
	if (argc < 3 || argc > 4) {
		debugPrintf("Hexdumps an IFF chunk\n");
		debugPrintf("Usage: %s iffname chunkname [index]\n", argv[0]);
		return true;
	}

	IFF iff(argv[1]);
	if (!iff.load()) {
		debugPrintf("Failed to load IFF '%s'\n", argv[1]);
		return true;
	}

	const byte *buf;
	uint size;

	char idStr[4] = { ' ', ' ', ' ', ' ' };
	uint len = strlen(argv[2]);
	memcpy(idStr, argv[2], (len <= 4 ? len : 4));
	uint32 id = READ_BE_UINT32(idStr);
	uint index = 0;

	if (argc == 4)
		index = atoi(argv[3]);

	buf = iff.getChunk(id, size, index);
	if (!buf) {
		debugPrintf("Failed to find chunk '%s' (index %d) in IFF '%s'\n", argv[2], index, argv[1]);
		return true;
	}

	Common::hexdump(buf, size);
	return true;
}

bool NancyConsole::Cmd_chunkList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("List chunks inside an IFF\n");
		debugPrintf("Usage: %s iffname\n", argv[0]);
		return true;
	}

	IFF iff(argv[1]);
	if (!iff.load()) {
		debugPrintf("Failed to load IFF '%s'\n", argv[1]);
		return true;
	}

	Common::Array<Common::String> list;
	iff.list(list);
	for (uint i = 0; i < list.size(); i++) {
		debugPrintf("%-6s", list[i].c_str());
		if ((i % 13) == 12 && i + 1 != list.size())
			debugPrintf("\n");
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_showImage(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Draws an image on the screen\n");
		debugPrintf("Usage: %s name [cal]\n", argv[0]);
		return true;
	}

	_imageFile = argv[1];
	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_loadCal(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Loads a .cal file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	if (!g_nancy->_resource->loadCifTree(argv[1], "cal"))
		debugPrintf("Failed to load '%s.cal'\n", argv[1]);
	return true;
}

bool NancyConsole::Cmd_playVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Plays a video\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	_videoFile = argv[1];
	_videoFile += ".avf";
	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_playAudio(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Plays an audio file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	Common::File *f = new Common::File;
	if (!f->open(Common::String(argv[1]) + ".his")) {
		debugPrintf("Failed to open '%s.his'\n", argv[1]);
		return true;
	}

	Audio::AudioStream *stream = SoundManager::makeHISStream(f, DisposeAfterUse::YES);

	if (!stream) {
		debugPrintf("Failed to load '%s.his'\n", argv[1]);
		delete f;
		return true;
	}
	Audio::SoundHandle handle;
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &handle, stream);
	return true;
}

bool NancyConsole::Cmd_loadScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Loads a scene\n");
		debugPrintf("Usage: %s sceneID\n", argv[0]);
		return true;
	}

	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	Common::String sceneName = Common::String::format("S%s", argv[1]);
	IFF iff(sceneName);
	if (!iff.load()) {
		debugPrintf("Invalid scene S%s\n", argv[1]);
		return true;
	}

	NancySceneState.changeScene((uint16)atoi(argv[1]), 0, 0, false);
	NancySceneState._state = State::Scene::kLoad;
	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_sceneID(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	debugPrintf("Scene: %u, Frame: %i \n", NancySceneState.getSceneInfo().sceneID, NancySceneState.getSceneInfo().frameID);
	return true;
}

} // End of namespace Nancy
