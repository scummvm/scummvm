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
#include "common/config-manager.h"

#include "audio/audiostream.h"
#include "image/bmp.h"
#include "video/bink_decoder.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/console.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/cif.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

NancyConsole::NancyConsole() : GUI::Debugger() {
	registerCmd("load_cal", WRAP_METHOD(NancyConsole, Cmd_loadCal));
	registerCmd("cif_export", WRAP_METHOD(NancyConsole, Cmd_cifExport));
	registerCmd("ciftree_export", WRAP_METHOD(NancyConsole, Cmd_ciftreeExport));
	registerCmd("cif_list", WRAP_METHOD(NancyConsole, Cmd_cifList));
	registerCmd("cif_info", WRAP_METHOD(NancyConsole, Cmd_cifInfo));
	registerCmd("chunk_export", WRAP_METHOD(NancyConsole, Cmd_chunkExport));
	registerCmd("chunk_hexdump", WRAP_METHOD(NancyConsole, Cmd_chunkHexDump));
	registerCmd("chunk_list", WRAP_METHOD(NancyConsole, Cmd_chunkList));
	registerCmd("show_image", WRAP_METHOD(NancyConsole, Cmd_showImage));
	registerCmd("export_image", WRAP_METHOD(NancyConsole, Cmd_exportImage));
	registerCmd("play_video", WRAP_METHOD(NancyConsole, Cmd_playVideo));
	registerCmd("play_sound", WRAP_METHOD(NancyConsole, Cmd_playSound));
	registerCmd("load_scene", WRAP_METHOD(NancyConsole, Cmd_loadScene));
	registerCmd("scene_id", WRAP_METHOD(NancyConsole, Cmd_sceneID));
	registerCmd("list_actionrecords", WRAP_METHOD(NancyConsole, Cmd_listActionRecords));
	registerCmd("scan_ar_type", WRAP_METHOD(NancyConsole, Cmd_scanForActionRecordType));
	registerCmd("get_eventflags", WRAP_METHOD(NancyConsole, Cmd_getEventFlags));
	registerCmd("set_eventflags", WRAP_METHOD(NancyConsole, Cmd_setEventFlags));
	registerCmd("get_inventory", WRAP_METHOD(NancyConsole, Cmd_getInventory));
	registerCmd("set_inventory", WRAP_METHOD(NancyConsole, Cmd_setInventory));
	registerCmd("get_player_time", WRAP_METHOD(NancyConsole, Cmd_getPlayerTime));
	registerCmd("set_player_time", WRAP_METHOD(NancyConsole, Cmd_setPlayerTime));
	registerCmd("get_difficulty", WRAP_METHOD(NancyConsole, Cmd_getDifficulty));
	registerCmd("set_difficulty", WRAP_METHOD(NancyConsole, Cmd_setDifficulty));
	registerCmd("sound_info", WRAP_METHOD(NancyConsole, Cmd_soundInfo));
	registerCmd("debug_hotspots", WRAP_METHOD(NancyConsole, Cmd_showHotspots));
}

NancyConsole::~NancyConsole() {}

void NancyConsole::postEnter() {
	GUI::Debugger::postEnter();
	if (!_videoFile.empty()) {
		Common::Path withExt = _videoFile;
		Video::VideoDecoder *dec = new AVFDecoder();

		if (!dec->loadFile(withExt.append(".avf"))) {
			// No AVF found, try Bink
			delete dec;
			dec = new Video::BinkDecoder();

			if (!dec->loadFile(withExt.append(".bik"))) {
				debugPrintf("Failed to load video '%s'\n", _videoFile.toString(Common::Path::kNativeSeparator).c_str());
				delete dec;
				dec = nullptr;
			}
		}

		if (dec) {
			Graphics::ManagedSurface surf;

			if (!_paletteFile.empty()) {
				GraphicsManager::loadSurfacePalette(surf, _paletteFile);
			}

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
						GraphicsManager::copyToManaged(*frame, surf, !_paletteFile.empty());
						g_nancy->_graphics->debugDrawToScreen(surf);
					}
				}

				g_system->delayMillis(10);
			}

			g_nancy->_graphics->redrawAll();
		}

		_videoFile.clear();
		_paletteFile.clear();
		delete dec;
	}

	if (!_imageFile.empty()) {
		Graphics::ManagedSurface surf;
		if (g_nancy->_resource->loadImage(_imageFile, surf)) {
			if (!_paletteFile.empty()) {
				GraphicsManager::loadSurfacePalette(surf, _paletteFile);
			}

			g_nancy->_graphics->debugDrawToScreen(surf);

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

			g_nancy->_graphics->redrawAll();
		} else {
			debugPrintf("Failed to load image '%s'\n", _imageFile.toString().c_str());
		}

		_imageFile.clear();
		_paletteFile.clear();
	}

	// After calling the console, action end events get sent to it and the input manager
	// can still think a keyboard button is being held when it is not; clearing all inputs fixes that
	g_nancy->_input->forceCleanInput();
}

bool NancyConsole::Cmd_cifExport(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Exports the specified resource to .cif file\n");
		debugPrintf("Usage: %s <name> [cal]\n", argv[0]);
		return true;
	}

	if (!g_nancy->_resource->exportCif((argc == 2 ? "" : argv[2]), argv[1]))
		debugPrintf("Failed to export '%s'\n", argv[1]);

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_ciftreeExport(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Exports the specified resources to a ciftree\n");
		debugPrintf("Usage: %s <tree name> <files...>\n", argv[0]);
		return true;
	}

	Common::Array<Common::Path> files;

	for (int i = 2; i < argc; ++i) {
		files.push_back(argv[i]);
	}

	if (!g_nancy->_resource->exportCifTree(argv[1], files))
		debugPrintf("Failed to export '%s'\n", argv[1]);

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_cifList(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("List resources of a certain type\n");
		debugPrintf("Types - 0: all, 2: image, 3: script\n");
		debugPrintf("Usage: %s <type> [cal]\n", argv[0]);
		return true;
	}

	Common::Array<Common::Path> list;
	g_nancy->_resource->list((argc == 2 ? "" : argv[2]), list, (CifInfo::ResType)atoi(argv[1]));

	Common::StringArray listStr;
	listStr.resize(list.size());
	for (unsigned int i = 0; i < list.size(); i++) {
		listStr[i] = list[i].toString();
	}
	debugPrintColumns(listStr);

	return true;
}

bool NancyConsole::Cmd_cifInfo(int argc, const char **argv) {
	if (argc < 2 || argc > 3) {
		debugPrintf("Prints information about a resource\n");
		debugPrintf("Usage: %s <name> [cal]\n", argv[0]);
		return true;
	}

	debugPrintf("%s", g_nancy->_resource->getCifDescription((argc == 2 ? "ciftree" : argv[2]), argv[1]).c_str());
	return true;
}

bool NancyConsole::Cmd_chunkExport(int argc, const char **argv) {
	if (argc < 3 || argc > 4) {
		debugPrintf("Exports an IFF chunk\n");
		debugPrintf("Usage: %s <iffname> <chunkname> [index]\n", argv[0]);
		return true;
	}

	IFF *iff = g_nancy->_resource->loadIFF(argv[1]);
	if (!iff) {
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

	buf = iff->getChunk(id, size, index);
	if (!buf) {
		debugPrintf("Failed to find chunk '%s' (index %d) in IFF '%s'\n", argv[2], index, argv[1]);
		delete iff;
		return true;
	}

	Common::DumpFile dumpfile;
	Common::String filename = g_nancy->getGameId();
	filename += '_';
	filename += argv[1];
	filename += '_';
	filename += argv[2];
	filename += ".dat";
	dumpfile.open(Common::Path(filename));
	dumpfile.write(buf, size);
	dumpfile.close();
	delete iff;
	return true;
}

bool NancyConsole::Cmd_chunkHexDump(int argc, const char **argv) {
	if (argc < 3 || argc > 4) {
		debugPrintf("Hexdumps an IFF chunk\n");
		debugPrintf("Usage: %s <iffname> <chunkname> [index]\n", argv[0]);
		return true;
	}

	IFF *iff = g_nancy->_resource->loadIFF(argv[1]);
	if (!iff) {
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

	buf = iff->getChunk(id, size, index);
	if (!buf) {
		debugPrintf("Failed to find chunk '%s' (index %d) in IFF '%s'\n", argv[2], index, argv[1]);
		return true;
	}

	Common::hexdump(buf, size);
	delete iff;
	return true;
}

bool NancyConsole::Cmd_chunkList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("List chunks inside an IFF\n");
		debugPrintf("Usage: %s <iffname>\n", argv[0]);
		return true;
	}

	IFF *iff = g_nancy->_resource->loadIFF(argv[1]);
	if (!iff) {
		debugPrintf("Failed to load IFF '%s'\n", argv[1]);
		return true;
	}

	Common::Array<Common::String> list;
	iff->list(list);
	for (uint i = 0; i < list.size(); i++) {
		debugPrintf("%-6s", list[i].c_str());
		if ((i % 13) == 12 && i + 1 != list.size())
			debugPrintf("\n");
	}

	debugPrintf("\n");
	delete iff;

	return true;
}

bool NancyConsole::Cmd_showImage(int argc, const char **argv) {
	if (g_nancy->getGameType() == kGameTypeVampire) {
		if (argc != 3) {
			debugPrintf("Draws an image on the screen\n");
			debugPrintf("Usage: %s <name> <paletteFile>\n", argv[0]);
			return true;
		}

		_imageFile = argv[1];
		_paletteFile = argv[2];
		return cmdExit(0, nullptr);
	} else {
		if (argc != 2) {
			debugPrintf("Draws an image on the screen\n");
			debugPrintf("Usage: %s <name>\n", argv[0]);
			return true;
		}

		_imageFile = argv[1];
		return cmdExit(0, nullptr);
	}
}

bool NancyConsole::Cmd_exportImage(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Exports an image to a file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	Graphics::ManagedSurface surf;
	if (g_nancy->_resource->loadImage(argv[1], surf)) {
		Common::DumpFile f;
		if (!f.open(Common::Path(argv[1]).appendInPlace(".bmp"))) {
			debugPrintf("Couldn't open file for writing!");

			return true;
		}
		Image::writeBMP(f, surf);
	} else {
		debugPrintf("File doesn't exist!\n");
	}

	return true;
}

bool NancyConsole::Cmd_playVideo(int argc, const char **argv) {
	if (g_nancy->getGameType() == kGameTypeVampire) {
		if (argc != 3) {
			debugPrintf("Plays a video\n");
			debugPrintf("Usage: %s <name> <paletteFile>\n", argv[0]);
			return true;
		}

		_videoFile = argv[1];
		_paletteFile = argv[2];
		return cmdExit(0, nullptr);
	} else {
		if (argc != 2) {
			debugPrintf("Plays a video\n");
			debugPrintf("Usage: %s <name>\n", argv[0]);
			return true;
		}

		_videoFile = argv[1];
		return cmdExit(0, nullptr);
	}
}

bool NancyConsole::Cmd_loadCal(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Loads a .cal file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	if (!g_nancy->_resource->readCifTree(argv[1], "cal", 3))
		debugPrintf("Failed to load '%s.cal'\n", argv[1]);
	return true;
}

bool NancyConsole::Cmd_playSound(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Plays an audio file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	Common::File *f = new Common::File;
	if (!f->open(Common::Path(argv[1]).appendInPlace(".his"))) {
		debugPrintf("Failed to open '%s.his'\n", argv[1]);
		delete f;
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
		debugPrintf("Usage: %s <sceneID>\n", argv[0]);
		return true;
	}

	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	Common::Path sceneName(Common::String::format("S%s", argv[1]));
	IFF *iff = g_nancy->_resource->loadIFF(sceneName);
	if (!iff) {
		debugPrintf("Invalid scene S%s\n", argv[1]);
		return true;
	}

	SceneChangeDescription scene;
	scene.sceneID = (uint16)atoi(argv[1]);
	NancySceneState.changeScene(scene);
	NancySceneState._state = State::Scene::kLoad;
	delete iff;
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

void NancyConsole::printActionRecord(const Nancy::Action::ActionRecord *record, bool noDependencies) {
	debugPrintf("\n%s\n\ttype: %i, %s\n\texecType: %s",
		record->_description.c_str(),
		record->_type,
		record->getRecordTypeName().c_str(),
		record->_execType == Nancy::Action::ActionRecord::kRepeating ? "kRepeating" : "kOneShot");

	if (!noDependencies && record->_dependencies.children.size()) {
		debugPrintf("\n\tDependencies:");

		recursePrintDependencies(record->_dependencies);
	}
}

void NancyConsole::recursePrintDependencies(const Nancy::Action::DependencyRecord &record) {
	using namespace Nancy::Action;

	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	for (const DependencyRecord &dep : record.children) {
		debugPrintf("\n\t\t");
		switch (dep.type) {
		case DependencyType::kNone :
			debugPrintf("kNone");
			break;
		case DependencyType::kInventory :
			debugPrintf("kInventory, item %u, %s, %s",
				dep.label,
				inventoryData->itemDescriptions[dep.label].name.c_str(),
				dep.condition == g_nancy->_true ? "true" : "false");
			break;
		case DependencyType::kEvent :
			debugPrintf("kEvent, flag %u, %s, %s",
				dep.label,
				g_nancy->getStaticData().eventFlagNames[dep.label >= 1000 ? dep.label - 1000 : dep.label].c_str(),
				dep.condition == g_nancy->_true ? "true" : "false");
			break;
		case DependencyType::kLogic :
			debugPrintf("kLogic, flag %u, %s",
				dep.label,
				dep.condition == g_nancy->_true ? "used" : "not used");
			break;
		case DependencyType::kElapsedGameTime :
			debugPrintf("kElapsedGameTime, %i hours, %i minutes, %i seconds, %i milliseconds",
				dep.hours,
				dep.minutes,
				dep.seconds,
				dep.milliseconds);
			break;
		case DependencyType::kElapsedSceneTime :
			debugPrintf("kElapsedSceneTime, %i hours, %i minutes, %i seconds, %i milliseconds",
				dep.hours,
				dep.minutes,
				dep.seconds,
				dep.milliseconds);
			break;
		case DependencyType::kElapsedPlayerTime :
			debugPrintf("kPlayerTime, player time %s %i hours, %i minutes, %i seconds, %i milliseconds",
				dep.condition == 0 ? "greater than" : (dep.condition == 1 ? "less than" : "equals"),
				dep.hours,
				dep.minutes,
				dep.seconds,
				dep.milliseconds);
			break;
		case DependencyType::kSceneCount :
			debugPrintf("kSceneCount, scene ID %i, hit count %s %i",
				dep.hours,
				dep.milliseconds == 1 ? ">" : dep.milliseconds == 2 ? "<" : "==",
				dep.minutes);
			break;
		case DependencyType::kElapsedPlayerDay :
			debugPrintf("kElapsedPlayerDay");
			break;
		case DependencyType::kCursorType :
			debugPrintf("kCursorType, item %u, %s",
				dep.label,
				inventoryData->itemDescriptions[dep.label].name.c_str());
			break;
		case DependencyType::kPlayerTOD :
			debugPrintf("kPlayerTOD, %s",
				dep.label == 0 ? "kPlayerDay" : dep.label == 1 ? "kPLayerNight" : "kPLayerDuskDawn");
			break;
		case DependencyType::kTimerLessThanDependencyTime :
			debugPrintf("kTimerLessThanDependencyTime");
			break;
		case DependencyType::kTimerGreaterThanDependencyTime :
			debugPrintf("kTimerGreaterThanDependencyTime");
			break;
		case DependencyType::kDifficultyLevel :
			debugPrintf("kDifficultyLevel, level %i", dep.condition);
			break;
		case DependencyType::kClosedCaptioning :
			debugPrintf("kClosedCaptioning, %s", dep.condition == 2 ? "true" : "false");
			break;
		case DependencyType::kSound :
			debugPrintf("kSound, channel %i", dep.condition);
			break;
		case DependencyType::kOpenParenthesis :
			debugPrintf("((((((((\n");
			recursePrintDependencies(dep);
			debugPrintf("\n))))))))");
			break;
		case DependencyType::kRandom :
			debugPrintf("kRandom, chance %i", dep.condition);
			break;
		default:
			debugPrintf("unknown type %u", (uint)dep.type);
			break;
		}

		debugPrintf("\n\t\t\torFlag == %s", dep.orFlag == true ? "true" : "false");
	}
}

bool NancyConsole::Cmd_listActionRecords(int argc, const char **argv) {
	using namespace Nancy::Action;

	if (argc == 1) {
		// Print the current scene
		if (g_nancy->_gameFlow.curState != NancyState::kScene) {
			debugPrintf("Not in the kScene state\n");
			return true;
		}

		Common::Array<ActionRecord *> &records = NancySceneState.getActionManager()._records;

		debugPrintf("Scene %u has %u action records:\n\n", NancySceneState.getSceneInfo().sceneID, records.size());

		for (uint i = 0; i < records.size(); ++i) {
			ActionRecord *rec = records[i];
			debugPrintf("Record %u:\n", i);
			printActionRecord(rec);
			debugPrintf("\n\n");
		}
	} else if (argc == 2) {
		// Print a different scene. We need to load all records into a temporary array and read from it
		Common::String s = argv[1];

		Common::Array<ActionRecord *> records;
		Common::Queue<uint> unknownTypes;
		Common::Queue<Common::String> unknownDescs;
		Common::SeekableReadStream *chunk;
		IFF *sceneIFF = g_nancy->_resource->loadIFF(Common::Path("S" + s));
		if (!sceneIFF) {
			debugPrintf("Invalid scene S%s\n", argv[1]);
			return true;
		}

		while (chunk = sceneIFF->getChunkStream("ACT", records.size()), chunk != nullptr) {
			ActionRecord *rec = ActionManager::createAndLoadNewRecord(*chunk);
			if (rec == nullptr) {
				chunk->seek(0);
				char descBuf[0x30];
				chunk->read(descBuf, 0x30);
				descBuf[0x2F] = '\0';
				byte ARType = chunk->readByte();
				unknownDescs.push(descBuf);
				unknownTypes.push(ARType);
			}
			records.push_back(rec);
			delete chunk;
		}

		for (uint i = 0; i < records.size(); ++i) {
			ActionRecord *rec = records[i];
			debugPrintf("Record %u:\n", i);

			if (rec == nullptr) {
				// For unknown record types, we want to print the typeID and description
				debugPrintf("\nUnknown or changed type %u, description:\n%s", unknownTypes.pop(), unknownDescs.pop().c_str());
			} else {
				printActionRecord(rec);
			}

			debugPrintf("\n\n");
		}

		for (uint i = 0; i < records.size(); ++i) {
			delete records[i];
		}

		delete sceneIFF;
	} else {
		debugPrintf("Invalid input\n");
	}

	return true;
}

bool NancyConsole::Cmd_scanForActionRecordType(int argc, const char **argv) {
	if (argc < 2 || argc % 2) {
		debugPrintf("Scans all IFFs for ActionRecords of the provided type\n");
		debugPrintf("Optionally also scans inside the AR's data for matching bytes\n");
		debugPrintf("Warning: can be quite slow, especially on archived game versions\n");
		debugPrintf("Usage: %s <typeID> {[byte offset] [byte value]}...\n", argv[0]);
		return true;
	}

	Common::Array<uint64> vals;
	vals.push_back(0x30);

	uint64 insertVal = 0;

	for (int i = 1; i < argc; ++i) {
		Common::String s(argv[i]);
		insertVal = s.asUint64();

		if (insertVal != 0 || s.firstChar() == '0') {
			if (i % 2) {
				if (insertVal > 255) {
					debugPrintf("Invalid input: %u is a byte, value cannot be over 255!\n", (uint32)insertVal);
					return true;
				}
				vals.push_back(insertVal);
			} else {
				vals.push_back(insertVal + 0x32);
			}

		} else {
			debugPrintf("Invalid input: %s\n", argv[i]);
			return true;
		}
	}

	Common::Array<Common::Path> list;
	// Action records only appear in the ciftree and promotree
	g_nancy->_resource->list("ciftree", list, CifInfo::kResTypeScript);
	g_nancy->_resource->list("promotree", list, CifInfo::kResTypeScript);

	char descBuf[0x30];

	Common::ArchiveMemberList searchManList;
	SearchMan.listMatchingMembers(searchManList, "*.iff");
	for (auto &i : searchManList) {
		list.push_back(i->getPathInArchive());
	}

	for (Common::Path &cifName : list) {
		Common::String name = cifName.baseName();
		if (name.hasSuffixIgnoreCase(".iff")) {
			name = name.substr(0, name.size() - 4);
		}

		// Only check inside scenes
		if (name.matchString("S#") ||
			name.matchString("S##") ||
			name.matchString("S###") ||
			name.matchString("S####")) {

			IFF *iff = g_nancy->_resource->loadIFF(cifName);
			if (iff) {
				uint num = 0;
				Common::SeekableReadStream *chunk = nullptr;
				while (chunk = iff->getChunkStream("ACT", num), chunk != nullptr) {
					bool isSatisfied = true;
					for (uint i = 0; i < vals.size(); i += 2) {
						if ((int64)vals[i] >= chunk->size()) {
							isSatisfied = false;
							break;
						}

						chunk->seek(vals[i]);
						if (chunk->readByte() != vals[i + 1]) {
							isSatisfied = false;
							break;
						}
					}

					if (isSatisfied) {
						chunk->seek(0);
						chunk->read(descBuf, 0x30);
						descBuf[0x2F] = '\0';
						debugPrintf("%s: ACT chunk %u, %s\n", cifName.toString().c_str(), num, descBuf);
					}

					++num;
					delete chunk;
				}

				delete iff;
			}
		}
	}

	return true;
}

bool NancyConsole::Cmd_getEventFlags(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	uint numEventFlags = g_nancy->getStaticData().numEventFlags;

	debugPrintf("Total number of event flags: %u\n", numEventFlags);

	if (argc == 1) {
		for (uint i = 0; i < numEventFlags; ++i) {
			debugPrintf("\nFlag %u, %s, %s",
				i,
				g_nancy->getStaticData().eventFlagNames[i].c_str(),
				NancySceneState.getEventFlag(i, g_nancy->_true) == true ? "true" : "false");
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			int flagID = atoi(argv[i]);
			if (flagID < 0 || flagID >= (int)numEventFlags) {
				debugPrintf("\nInvalid flag %s", argv[i]);
				continue;
			}
			debugPrintf("\nFlag %u, %s, %s",
				flagID,
				g_nancy->getStaticData().eventFlagNames[flagID].c_str(),
				NancySceneState.getEventFlag(flagID, g_nancy->_true) == true ? "true" : "false");

		}
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_setEventFlags(int argc, const char **argv) {
	if (argc < 2 || argc % 2 == 0) {
		debugPrintf("Sets one or more event flags to the provided value.\n");
		debugPrintf("Usage: %s <flagID> <true/false>...\n", argv[0]);
		return true;
	}

	for (int i = 1; i < argc; i += 2) {
		int flagID = atoi(argv[i]);
		if (flagID < 0 || flagID >= (int)g_nancy->getStaticData().numEventFlags) {
			debugPrintf("Invalid flag %s\n", argv[i]);
			continue;
		}

		if (Common::String(argv[i + 1]).compareTo("true") == 0) {
			NancySceneState.setEventFlag(flagID, g_nancy->_true);
			debugPrintf("Set flag %i, %s, to true\n",
				flagID,
				g_nancy->getStaticData().eventFlagNames[flagID].c_str());
		} else if (Common::String(argv[i + 1]).compareTo("false") == 0) {
			NancySceneState.setEventFlag(flagID, g_nancy->_false);
			debugPrintf("Set flag %i, %s, to false\n",
				flagID,
				g_nancy->getStaticData().eventFlagNames[flagID].c_str());
		} else {
			debugPrintf("Invalid value %s\n", argv[i + 1]);
			continue;
		}
	}

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_getInventory(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	uint numItems = g_nancy->getStaticData().numItems;
	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	debugPrintf("Total number of inventory items: %u\n", numItems);

	if (argc == 1) {
		for (uint i = 0; i < numItems; ++i) {
			byte keep = inventoryData->itemDescriptions[i].keepItem;
			debugPrintf("\nItem %u, %s, %s, %s",
				i,
				inventoryData->itemDescriptions[i].name.c_str(),
				keep == 0 ? "UseThenLose" : keep == 1 ? "KeepAlways" : "ReturnToInventory",
				NancySceneState.hasItem(i) == g_nancy->_true ? "true" : "false");
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			byte keep = inventoryData->itemDescriptions[i].keepItem;
			int flagID = atoi(argv[i]);
			if (flagID < 0 || flagID >= (int)numItems) {
				debugPrintf("\nInvalid flag %s", argv[i]);
				continue;
			}
			debugPrintf("\nItem %u, %s, %s, %s",
				flagID,
				inventoryData->itemDescriptions[flagID].name.c_str(),
				keep == 0 ? "UseThenLose" : keep == 1 ? "KeepAlways" : "ReturnToInventory",
				NancySceneState.hasItem(i) == g_nancy->_true ? "true" : "false");

		}
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_setInventory(int argc, const char **argv) {
	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (argc < 2 || argc % 2 == 0) {
		debugPrintf("Sets one or more inventory items to the provided value.\n");
		debugPrintf("Usage: %s <itemID> <true/false>...\n", argv[0]);
		return true;
	}

	for (int i = 1; i < argc; i += 2) {
		int itemID = atoi(argv[i]);
		if (itemID < 0 || itemID >= (int)g_nancy->getStaticData().numItems) {
			debugPrintf("Invalid item %s\n", argv[i]);
			continue;
		}

		if (Common::String(argv[i + 1]).compareTo("true") == 0) {
			NancySceneState.addItemToInventory(itemID);
			debugPrintf("Added item %i, %s, to inventory\n",
				itemID,
				inventoryData->itemDescriptions[itemID].name.c_str());
		} else if (Common::String(argv[i + 1]).compareTo("false") == 0) {
			NancySceneState.removeItemFromInventory(itemID, false);
			debugPrintf("Removed item %i, %s, from inventory\n",
				itemID,
				inventoryData->itemDescriptions[itemID].name.c_str());
		} else {
			debugPrintf("Invalid value %s\n", argv[i + 1]);
			continue;
		}
	}

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_getPlayerTime(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	Time time = NancySceneState._timers.playerTime;
	debugPrintf("Player time: %u days, %u hours, %u minutes; %u\n",
		time.getDays(),
		time.getHours(),
		time.getMinutes(),
		(uint32)time);
	return true;
}

bool NancyConsole::Cmd_setPlayerTime(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (argc != 2 && argc != 4) {
		debugPrintf("Sets the in-game time.\n");
		debugPrintf("Usage: %s <milliseconds> or %s <days> <hours> <minutes>\n", argv[0], argv[0]);
		return true;
	}

	Time &time = NancySceneState._timers.playerTime;

	if (argc == 2) {
		time = atoi(argv[1]);
	} else {
		time = 	atoi(argv[1]) * 86400000 +	// days
				atoi(argv[2]) * 3600000 +	// hours
				atoi(argv[3]) * 60000;		// minutes
	}

	debugPrintf("Set player time to: %u days, %u hours, %u minutes; %u\n",
		time.getDays(),
		time.getHours(),
		time.getMinutes(),
		(uint32)time);

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_getDifficulty(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	debugPrintf("Difficulty: %u\n", NancySceneState.getDifficulty());

	return true;
}

bool NancyConsole::Cmd_setDifficulty(int argc, const char **argv) {
	if (g_nancy->_gameFlow.curState != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (argc != 2) {
		debugPrintf("Set the game difficulty.\n");
		debugPrintf("Usage: %s <difficulty>\n", argv[0]);
		return true;
	}

	int diff = atoi(argv[1]);

	if (diff < 0 || diff > 2) {
		debugPrintf("Invalid difficulty %s\n", argv[1]);
		return true;
	}

	NancySceneState.setDifficulty(diff);
	debugPrintf("Set difficulty to %i\n", diff);

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_soundInfo(int argc, const char **argv) {
	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		const Math::Vector3d &pos = NancySceneState.getSceneSummary().listenerPosition;
		const Math::Vector3d &ori = g_nancy->_sound->_orientation;
		debugPrintf("3D listener position: %f, %f, %f\n", pos.x(), pos.y(), pos.z());
		debugPrintf("3D listener orientation: %f, %f, %f\n\n", ori.x(), ori.y(), ori.z());
	}

	Common::Array<byte> channelIDs;
	if (argc == 1) {
		debugPrintf("Currently playing sounds:\n\n");

		for (uint i = 0; i < g_nancy->getStaticData().soundChannelInfo.numChannels; ++i) {
			channelIDs.push_back(i);
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			channelIDs.push_back(atoi(argv[i]));
		}
	}

	for (byte channelID : channelIDs) {
		const auto &chan = g_nancy->_sound->_channels[channelID];

		if (g_nancy->_sound->isSoundPlaying(channelID)) {
			debugPrintf("Channel %u, filename %s\n", channelID, chan.name.c_str());
			debugPrintf("Source rate %i, playing at %i\n", chan.stream->getRate(), g_nancy->_sound->_mixer->getChannelRate(chan.handle));
			debugPrintf("Volume: %u, pan: %i, numLoops: %u\n\n", chan.volume, g_nancy->_sound->_mixer->getChannelBalance(chan.handle), chan.numLoops);

			if (chan.playCommands != SoundManager::kPlaySequential) {
				debugPrintf("\tPlay commands 0x%08x\n", chan.playCommands);

				if (chan.effectData) {
					debugPrintf("\tPosition: %f, %f, %f, ", chan.position.x(), chan.position.y(), chan.position.z());
					debugPrintf("delta: %f, %f, %f\n\n", chan.positionDelta.x(), chan.positionDelta.y(), chan.positionDelta.z());
				}
			}
		}
	}

	return true;
}

bool NancyConsole::Cmd_showHotspots(int argc, const char **argv) {
	ConfMan.setBool("debug_hotspots", !ConfMan.getBool("debug_hotspots", Common::ConfigManager::kTransientDomain), Common::ConfigManager::kTransientDomain);

	return cmdExit(0, nullptr);
}

} // End of namespace Nancy
