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
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "image/bmp.h"

#include "engines/nancy/movieplayer.h"
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

#ifdef ENABLE_NANCY_V4
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/scenecontext.h"
#endif

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
#ifdef ENABLE_NANCY_V4
	registerCmd("list_records", WRAP_METHOD(NancyConsole, Cmd_listRecords));
	registerCmd("script_export", WRAP_METHOD(NancyConsole, Cmd_scriptExport));
#endif
	registerCmd("actionrecord_export", WRAP_METHOD(NancyConsole, Cmd_actionRecordExport));
	registerCmd("scan_ar_type", WRAP_METHOD(NancyConsole, Cmd_scanForActionRecordType));
	registerCmd("list_includes", WRAP_METHOD(NancyConsole, Cmd_listIncludes));
	registerCmd("find_include", WRAP_METHOD(NancyConsole, Cmd_findInclude));
	registerCmd("get_eventflags", WRAP_METHOD(NancyConsole, Cmd_getEventFlags));
	registerCmd("set_eventflags", WRAP_METHOD(NancyConsole, Cmd_setEventFlags));
	registerCmd("get_inventory", WRAP_METHOD(NancyConsole, Cmd_getInventory));
	registerCmd("set_inventory", WRAP_METHOD(NancyConsole, Cmd_setInventory));
	registerCmd("get_money", WRAP_METHOD(NancyConsole, Cmd_getMoney));
	registerCmd("set_money", WRAP_METHOD(NancyConsole, Cmd_setMoney));
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
		MoviePlayer player;

		if (!player.loadFile(_videoFile)) {
			debugPrintf("Failed to load video '%s'\n", _videoFile.toString(Common::Path::kNativeSeparator).c_str());
		} else {
			Graphics::ManagedSurface surf;

			if (!_paletteFile.empty()) {
				GraphicsManager::loadSurfacePalette(surf, _paletteFile);
			}

			player.start();
			Common::EventManager *ev = g_system->getEventManager();
			while (!g_nancy->shouldQuit() && !player.endOfVideo()) {
				Common::Event event;
				if (ev->pollEvent(event)) {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == InputManager::kNancyActionLeftClick) {
						break;
					}
				}

				if (player.needsUpdate()) {
					const Graphics::Surface *frame = player.decodeNextFrame();
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
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == InputManager::kNancyActionLeftClick) {
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
	if (argc != 2) {
		debugPrintf("Exports the specified resource to .cif file\n");
		debugPrintf("Usage: %s <name>\n", argv[0]);
		return true;
	}

	if (!g_nancy->_resource->exportCif(argv[1]))
		debugPrintf("Failed to export '%s'\n", argv[1]);

	return true;
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

	return true;
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

	if (g_nancy->_sound->isCommonSound(argv[1])) {
		g_nancy->_sound->playSound(argv[1]);
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

	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	// Nancy19 and later scenes are scripts, and one that can't be loaded leaves
	// the player where they are
	if (g_nancy->getGameType() < kGameTypeNancy19) {
		Common::Path sceneName(Common::String::format("S%s", argv[1]));
		IFF *iff = g_nancy->_resource->loadIFF(sceneName);
		if (!iff) {
			debugPrintf("Invalid scene S%s\n", argv[1]);
			return true;
		}

		delete iff;
	}

	SceneChangeDescription scene;
	scene.sceneID = (uint16)atoi(argv[1]);
	NancySceneState.changeScene(scene);
	NancySceneState.setState(State::Scene::kLoad);
	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_sceneID(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	debugPrintf("Scene: %u, Frame: %i \n", NancySceneState.getSceneInfo().sceneID, NancySceneState.getSceneInfo().frameID);
	return true;
}

void NancyConsole::printActionRecord(const Action::ActionRecord *record, bool noDependencies) {
	debugPrintf("\n%s\n\ttype: %i, %s\n\texecType: %s",
		record->_description.c_str(),
		record->_type,
		record->getRecordTypeName().c_str(),
		record->_execType == Action::ActionRecord::kRepeating ? "kRepeating" : "kOneShot");

	Common::String extraInfo = record->getRecordExtraInfo();
	if (!extraInfo.empty())
		debugPrintf("\n\textraInfo: %s", extraInfo.c_str());

	if (!noDependencies && record->_dependencies.children.size()) {
		debugPrintf("\n\tDependencies:");

		recursePrintDependencies(record->_dependencies);
	}
}

// Nancy15 gives every playable character their own inventory, so the console
// has to name the one it reports on. The PCUI chunk holds the image names the
// characters are known by, e.g. "PUI_CRE_Nancy".
static Common::String playerCharacterName(uint characterIndex) {
	const PCUI *pcui = GetEngineData(PCUI);
	if (pcui && characterIndex < pcui->characters.size()) {
		const Common::String &imageName = pcui->characters[characterIndex].imageName;
		size_t lastSeparator = imageName.findLastOf('_');
		if (lastSeparator != Common::String::npos) {
			return imageName.substr(lastSeparator + 1);
		}

		if (!imageName.empty()) {
			return imageName;
		}
	}

	return Common::String::format("character %u", characterIndex);
}

static uint numPlayerCharacters() {
	if (g_nancy->getGameType() < kGameTypeNancy15) {
		return 1;
	}

	const PCUI *pcui = GetEngineData(PCUI);
	return pcui ? MIN<uint>(pcui->characters.size(), kMaxPlayerCharacters) : 1;
}

// The name of an inventory item, from wherever the game keeps it: the inventory
// data, Nancy17's item table, or Nancy16's list of names next to the event flags
static Common::String inventoryItemName(uint itemID) {
	const INV *inventoryData = GetEngineData(INV);
	if (inventoryData) {
		return itemID < inventoryData->itemDescriptions.size() ? inventoryData->itemDescriptions[itemID].name : "Invalid";
	}

	const INVD *itemTable = GetEngineData(INVD);
	if (itemTable) {
		for (const INVD::Item &item : itemTable->items) {
			if (item.id == (int16)itemID) {
				return item.name;
			}
		}

		return "Invalid";
	}

	const EVNT *names = (const EVNT *)g_nancy->getEngineData("EVNT");
	return names && itemID < names->itemNames.size() ? names->itemNames[itemID] : "Invalid";
}

void NancyConsole::recursePrintDependencies(const Action::DependencyRecord &record) {
	using namespace Action;

	for (const DependencyRecord &dep : record.children) {
		debugPrintf("\n\t\t");
		switch (dep.type) {
		case DependencyType::kNone:
			debugPrintf("kNone");
			break;
		case DependencyType::kInventory:
			debugPrintf("kInventory, item %u, %s, %s",
				dep.label,
				inventoryItemName((uint16)dep.label).c_str(),
				dep.condition == g_nancy->_true ? "true" : "false");

			// Nancy15+ can ask about a character other than the one being played
			if (g_nancy->getGameType() >= kGameTypeNancy15 && dep.hours >= 0 &&
					dep.hours != kPlayerCharacterActive) {
				debugPrintf(", %s", playerCharacterName(dep.hours).c_str());
			}

			break;
		case DependencyType::kEvent:
			debugPrintf("kEvent, flag %u, %s, %s",
				dep.label,
				g_nancy->getEventFlagName(dep.label).c_str(),
				dep.condition == g_nancy->_true ? "true" : "false");
			break;
		case DependencyType::kLogic:
			debugPrintf("kLogic, flag %u, %s",
				dep.label,
				dep.condition == g_nancy->_true ? "used" : "not used");
			break;
		case DependencyType::kElapsedGameTime:
			debugPrintf("kElapsedGameTime, %i hours, %i minutes, %i seconds, %i milliseconds",
				dep.hours,
				dep.minutes,
				dep.seconds,
				dep.milliseconds);
			break;
		case DependencyType::kElapsedSceneTime:
			debugPrintf("kElapsedSceneTime, %i hours, %i minutes, %i seconds, %i milliseconds",
				dep.hours,
				dep.minutes,
				dep.seconds,
				dep.milliseconds);
			break;
		case DependencyType::kElapsedPlayerTime:
			if (dep.condition == 3) {
				// A range keeps its end in the seconds and milliseconds fields
				debugPrintf("kPlayerTime, player time between %i:%02i and %i:%02i",
					dep.hours, dep.minutes, dep.seconds, dep.milliseconds);
			} else {
				debugPrintf("kPlayerTime, player time %s %i:%02i",
					dep.condition == 0 ? "at or after" : (dep.condition == 1 ? "at or before" : "equals"),
					dep.hours, dep.minutes);
			}
			break;
		case DependencyType::kSceneCount:
			debugPrintf("kSceneCount, scene ID %i, hit count %s %i",
				dep.hours,
				dep.milliseconds == 1 ? ">" : dep.milliseconds == 2 ? "<" : "==",
				dep.minutes);
			break;
		case DependencyType::kElapsedPlayerDay:
			debugPrintf("kElapsedPlayerDay");
			break;
		case DependencyType::kCursorType:
			debugPrintf("kCursorType, item %u, %s",
				dep.label,
				inventoryItemName((uint16)dep.label).c_str());
			break;
		case DependencyType::kPlayerTOD:
			debugPrintf("kPlayerTOD, %s",
				dep.label == 0 ? "kPlayerDay" : dep.label == 1 ? "kPLayerNight" : "kPLayerDuskDawn");
			break;
		case DependencyType::kTimerLessThanDependencyTime:
			if (g_nancy->getGameType() >= kGameTypeNancy14) {
				// Repurposed as a value-table test in Nancy14
				static const char *const comparisons[] = { "==", ">", ">=", "<", "<=" };
				const char *comparison = dep.condition < ARRAYSIZE(comparisons) ? comparisons[dep.condition] : "?";
				const TableData *table = State::Scene::hasInstance() ?
					(const TableData *)NancySceneState.getPuzzleData(TableData::getTag()) : nullptr;

				if (g_nancy->getGameType() >= kGameTypeNancy16 && table) {
					// Nancy16 compares against another value when the hours field is 1; values
					// are indexed with a byte, and the last one is the day. Show what they hold.
					if (dep.hours == 1) {
						debugPrintf("kValueTest, value %u (now %i) %s value %u (now %i)", (byte)dep.label,
							table->getValueOrZero((byte)dep.label), comparison,
							(byte)dep.milliseconds, table->getValueOrZero((byte)dep.milliseconds));
					} else {
						debugPrintf("kValueTest, value %u (now %i) %s %i", (byte)dep.label,
							table->getValueOrZero((byte)dep.label), comparison, dep.milliseconds);
					}
				} else {
					debugPrintf("kValueTest, value %u %s %i", dep.label, comparison, dep.milliseconds);
				}
			} else {
				debugPrintf("kTimerLessThanDependencyTime");
			}
			break;
		case DependencyType::kTimerGreaterThanDependencyTime:
			debugPrintf("kTimerGreaterThanDependencyTime");
			break;
		case DependencyType::kDifficultyLevel:
			debugPrintf("kDifficultyLevel, level %i", dep.condition);
			break;
		case DependencyType::kClosedCaptioning:
			debugPrintf("kClosedCaptioning, %s", dep.condition == 2 ? "true" : "false");
			break;
		case DependencyType::kSound:
			debugPrintf("kSound, channel %i, %s", dep.label, dep.condition == 1 ? "playing" : "not playing");
			break;
		case DependencyType::kOpenParenthesis:
			debugPrintf("((((((((\n");
			recursePrintDependencies(dep);
			debugPrintf("\n))))))))");
			break;
		case DependencyType::kRandom:
			debugPrintf("kRandom, chance %i", dep.condition);
			break;
		case DependencyType::kDefaultAR:
			if (g_nancy->getGameType() >= kGameTypeNancy14) {
				debugPrintf("kDefaultAR, no record of this type (or types %i, %i, %i, %i) executed in this scene",
					dep.hours, dep.minutes, dep.seconds, dep.milliseconds);
			} else {
				debugPrintf("kDefaultAR, previous record did not execute");
			}

			break;
		default:
			debugPrintf("unknown type %u", (uint)dep.type);
			break;
		}

		debugPrintf("\n\t\t\torFlag == %s", dep.orFlag == true ? "true" : "false");
	}
}

#ifdef ENABLE_NANCY_V4
// The records of the scripted scenes: those of every stream, or with the fields
// the scripts gave them, those of one stream or one record of it
bool NancyConsole::Cmd_listRecords(int argc, const char **argv) {
	if (argc > 3) {
		debugPrintf("Lists the records of the scene scripts, in Nancy19 and later.\n");
		debugPrintf("Usage: %s [stream] [record name or kind]\n", argv[0]);
		debugPrintf("Without a stream, the records of every stream are listed; with one, also their fields.\n");
		debugPrintf("The main scene is the stream %s.\n", State::SceneStreamManager::getMainStreamName());
		return true;
	}

	if (g_nancy->getGameType() < kGameTypeNancy19) {
		debugPrintf("This game has action records; use list_actionrecords\n");
		return true;
	}

	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	State::SceneStreamManager &streams = NancySceneState.getStreams();
	Common::Array<Common::String> streamNames;
	if (argc >= 2) {
		streamNames.push_back(argv[1]);
	} else {
		streams.getStreamNames(streamNames, true);
	}

	const bool showsFields = argc >= 2;
	const Common::String wantedRecord = argc == 3 ? argv[2] : "";

	for (const Common::String &streamName : streamNames) {
		Script::SceneContext *context = nullptr;
		if (streamName.equalsIgnoreCase(State::SceneStreamManager::getMainStreamName())) {
			context = NancySceneState.getScriptScene();
		} else {
			State::SceneStream *stream = streams.find(streamName);
			context = stream ? stream->getScriptScene() : nullptr;
		}

		if (!context) {
			debugPrintf("No running stream %s\n", streamName.c_str());
			continue;
		}

		const Common::Array<Script::Record *> &records = context->getRecords();
		debugPrintf("%s, scene %s: %u records\n", streamName.c_str(), context->getSceneName().c_str(), records.size());

		for (uint i = 0; i < records.size(); ++i) {
			Script::Record *record = records[i];
			const Common::String globalName = context->findGlobalName(record);

			// "ND.MovieAR" is a Movie
			Common::String typeName = record->getTypeName();
			if (typeName.hasPrefix("ND.")) {
				typeName.erase(0, 3);
			}

			if (typeName.hasSuffix("AR")) {
				typeName.erase(typeName.size() - 2);
			}

			// A record is asked for by its name, or by its kind
			if (!wantedRecord.empty() && !globalName.equalsIgnoreCase(wantedRecord) && !typeName.equalsIgnoreCase(wantedRecord)) {
				continue;
			}

			const char *state = "inactive";
			if (record->isDone()) {
				state = "done";
			} else if (record->isActive()) {
				state = record->hasStarted() ? "running" : "active";
			}

			debugPrintf("  %u: %s %s (%s, z %g)\n", i, typeName.c_str(),
				globalName.empty() ? "-" : globalName.c_str(), state, record->getZ());

			if (showsFields) {
				Common::Array<Common::String> fields;
				record->describeFields(fields);
				for (const Common::String &field : fields) {
					debugPrintf("      %s\n", field.c_str());
				}
			}
		}

		debugPrintf("\n");
	}

	return true;
}

// Writes a script as it is, compiled, to a file
bool NancyConsole::Cmd_scriptExport(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Exports a script of Nancy19 and later\n");
		debugPrintf("Usage: %s <script name or scene number>\n", argv[0]);
		debugPrintf("The scripts are compiled Lua 5.1, which a decompiler like unluac turns into Lua code.\n");
		return true;
	}

	// Scenes are named after their number
	Common::String name = argv[1];
	if (Common::isDigit(name.firstChar())) {
		name = "s" + name;
	}

	Common::SeekableReadStream *script = g_nancy->_resource->loadScript(Common::Path(name));
	if (!script) {
		debugPrintf("Failed to find script '%s'\n", name.c_str());
		return true;
	}

	Common::String filename = Common::String::format("%s_%s.luac", g_nancy->getGameId(), name.c_str());
	Common::DumpFile dumpfile;
	if (!dumpfile.open(Common::Path(filename))) {
		debugPrintf("Failed to write '%s'\n", filename.c_str());
		delete script;
		return true;
	}

	dumpfile.writeStream(script);
	dumpfile.close();
	delete script;
	debugPrintf("Wrote %s\n", filename.c_str());
	return true;
}
#endif

bool NancyConsole::Cmd_listActionRecords(int argc, const char **argv) {
	using namespace Action;

	if (argc == 1) {
		// Print the current scene
		if (g_nancy->getState() != NancyState::kScene) {
			debugPrintf("Not in the kScene state\n");
			return true;
		}

		Common::Array<ActionRecord *> &records = NancySceneState.getActionManager()._records;

		debugPrintf("Scene %u has %u action records:\n\n", NancySceneState.getSceneInfo().sceneID, records.size());

		for (uint i = 0; i < records.size(); ++i) {
			ActionRecord *rec = records[i];
			if (rec->_includeSource.empty()) {
				debugPrintf("Record %u:\n", i);
			} else {
				debugPrintf("Record %u (from %s):\n", i, rec->_includeSource.c_str());
			}
			printActionRecord(rec);
			debugPrintf("\n\n");
		}
	} else if (argc == 2) {
		// Print a different scene, or an included script given by name. We need to
		// load all records into a temporary array and read from it
		Common::String s = argv[1];
		if (Common::isDigit(s.firstChar())) {
			s = "S" + s;
		}

		Common::Array<ActionRecord *> records;
		Common::Queue<uint> unknownTypes;
		Common::Queue<Common::String> unknownDescs;
		Common::SeekableReadStream *chunk;
		IFF *sceneIFF = g_nancy->_resource->loadIFF(Common::Path(s));
		if (!sceneIFF) {
			debugPrintf("Invalid scene or script %s\n", s.c_str());
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
			Common::String source = sceneIFF->getChunkSource("ACT", i);
			if (source.empty()) {
				debugPrintf("Record %u:\n", i);
			} else {
				debugPrintf("Record %u (from %s):\n", i, source.c_str());
			}

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

bool NancyConsole::Cmd_actionRecordExport(int argc, const char **argv) {
	using namespace Action;

	if (argc < 2) {
		debugPrintf("Exports an action record of the current or a specified scene to a file\n");
		debugPrintf("Usage: %s <actionRecordID> <sceneID>\n", argv[0]);
		return true;
	}

	int recordId = atoi(argv[1]);
	uint16 sceneId = 0;

	if (argc == 2) {
		// Export the record from the current scene
		if (g_nancy->getState() != NancyState::kScene) {
			debugPrintf("Not in the kScene state\n");
			return true;
		}

		sceneId = NancySceneState.getSceneInfo().sceneID;
	} else if (argc == 3) {
		// Export a record from a different scene. We need to load all records into a temporary array and read from it
		sceneId = (uint16)atoi(argv[2]);
	}

	Common::String s = Common::String::format("S%u", sceneId);

	IFF *sceneIFF = g_nancy->_resource->loadIFF(Common::Path(s));
	if (!sceneIFF) {
		debugPrintf("Invalid scene S%s\n", argv[1]);
		return true;
	}

	Common::SeekableReadStream *chunk = sceneIFF->getChunkStream("ACT", recordId);
	if (chunk) {
		char descBuf[48];
		chunk->read(descBuf, 48);
		descBuf[47] = '\0';
		Common::String desc(descBuf);
		desc.replace('/', '-');
		desc.replace('\\', '-');
		desc.replace('>', '_');
		desc.replace('<', '_');
		byte ARType = chunk->readByte();
		chunk->skip(1); // execType

		Common::DumpFile f;
		Common::String filename = Common::String::format("%s_ar_%d_scene_%d_%d_%s.dat", g_nancy->getGameId(), ARType, sceneId, recordId, desc.c_str());
		f.open(Common::Path(filename));
		f.writeStream(chunk, chunk->size() - 50);
		f.close();
		debugPrintf("Exported record %d (%s) from scene S%u to %s\n", recordId, descBuf, sceneId, filename.c_str());
	} else {
		debugPrintf("Invalid record ID %d\n", recordId);
	}
	delete chunk;

	delete sceneIFF;

	return true;
}

// The name of an IFF, without any .iff extension
static Common::String getIFFName(const Common::Path &path) {
	Common::String name = path.baseName();
	if (name.hasSuffixIgnoreCase(".iff")) {
		name = name.substr(0, name.size() - 4);
	}

	return name;
}

// Lists all scene IFFs (S#, S##, ...) in the ciftree, the promotree, and loose .iff files
static void listSceneIFFs(Common::Array<Common::Path> &sceneList) {
	Common::Array<Common::Path> list;
	// Action records only appear in the ciftree and promotree
	g_nancy->_resource->list("ciftree", list, CifInfo::kResTypeScript);
	g_nancy->_resource->list("promotree", list, CifInfo::kResTypeScript);

	Common::ArchiveMemberList searchManList;
	SearchMan.listMatchingMembers(searchManList, "*.iff");
	for (auto &i : searchManList) {
		list.push_back(i->getPathInArchive());
	}

	for (Common::Path &path : list) {
		Common::String name = getIFFName(path);
		if (name.matchString("S#") ||
			name.matchString("S##") ||
			name.matchString("S###") ||
			name.matchString("S####")) {
			sceneList.push_back(path);
		}
	}
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
	listSceneIFFs(list);

	char descBuf[0x30];

	for (Common::Path &cifName : list) {
		IFF *iff = g_nancy->_resource->loadIFF(cifName);
		if (!iff) {
			continue;
		}

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

				Common::String source = iff->getChunkSource("ACT", num);
				if (source.empty()) {
					debugPrintf("%s: ACT chunk %u, %s\n", cifName.toString().c_str(), num, descBuf);
				} else {
					debugPrintf("%s: ACT chunk %u, %s (from %s)\n", cifName.toString().c_str(), num, descBuf, source.c_str());
				}
			}

			++num;
			delete chunk;
		}

		delete iff;
	}

	return true;
}

bool NancyConsole::Cmd_listIncludes(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Lists the files included (via USE chunks) by the current or a specified scene\n");
		debugPrintf("Usage: %s [sceneID]\n", argv[0]);
		return true;
	}

	uint sceneID = 0;
	if (argc == 2) {
		sceneID = atoi(argv[1]);
	} else if (g_nancy->getState() == NancyState::kScene) {
		sceneID = NancySceneState.getSceneInfo().sceneID;
	} else {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	IFF *sceneIFF = g_nancy->_resource->loadIFF(Common::Path(Common::String::format("S%u", sceneID)));
	if (!sceneIFF) {
		debugPrintf("Invalid scene S%u\n", sceneID);
		return true;
	}

	const Common::Array<Common::String> &includes = sceneIFF->getIncludes();
	if (includes.empty()) {
		debugPrintf("Scene S%u has no includes\n", sceneID);
	} else {
		debugPrintf("Scene S%u includes:\n", sceneID);
		for (const Common::String &include : includes) {
			debugPrintf("\t%s\n", include.c_str());
		}
	}

	delete sceneIFF;
	return true;
}

bool NancyConsole::Cmd_findInclude(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Lists the scenes that include (via USE chunks) the given file,\n");
		debugPrintf("or every included file and the scenes using it if none is given\n");
		debugPrintf("Warning: can be quite slow, especially on archived game versions\n");
		debugPrintf("Usage: %s [filename]\n", argv[0]);
		return true;
	}

	Common::Array<Common::Path> list;
	listSceneIFFs(list);

	// Included file name -> including scenes, in first-seen order
	Common::Array<Common::String> includeNames;
	Common::HashMap<Common::String, Common::Array<Common::String>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> users;

	for (Common::Path &cifName : list) {
		IFF *iff = g_nancy->_resource->loadIFF(cifName);
		if (!iff) {
			continue;
		}

		for (const Common::String &include : iff->getIncludes()) {
			if (argc == 2 && !include.equalsIgnoreCase(argv[1])) {
				continue;
			}

			if (!users.contains(include)) {
				includeNames.push_back(include);
			}

			users[include].push_back(getIFFName(cifName));
		}

		delete iff;
	}

	if (includeNames.empty()) {
		if (argc == 2) {
			debugPrintf("No scene includes %s\n", argv[1]);
		} else {
			debugPrintf("No scene includes any files\n");
		}

		return true;
	}

	for (const Common::String &include : includeNames) {
		const Common::Array<Common::String> &scenes = users[include];
		debugPrintf("%s is included by %u scene(s):\n", include.c_str(), scenes.size());
		for (uint i = 0; i < scenes.size(); ++i) {
			debugPrintf("%-7s", scenes[i].c_str());
			if ((i % 10) == 9 && i + 1 != scenes.size()) {
				debugPrintf("\n");
			}
		}

		debugPrintf("\n\n");
	}

	return true;
}

bool NancyConsole::Cmd_getEventFlags(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	uint numEventFlags = g_nancy->getStaticData().numEventFlags;

	// Event flags are addressed by label. Nancy3 and later number them from 1000
	// (and Nancy12+ splits them into a 1xxx generic and a 2xxx game-specific range),
	// so the label of the flag stored at index i is i + baseLabel.
	int baseLabel = g_nancy->getGameType() >= kGameTypeNancy3 ? 1000 : 0;

	debugPrintf("Total number of event flags: %u\n", numEventFlags);

	if (argc == 1) {
		for (uint i = 0; i < numEventFlags; ++i) {
			int label = baseLabel + (int)i;
			debugPrintf("\nFlag %d, %s, %s",
				label,
				g_nancy->getEventFlagName(label).c_str(),
				NancySceneState.getEventFlag(label, g_nancy->_true) ? "true" : "false");
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			int label = baseLabel + atoi(argv[i]);
			if (label - baseLabel < 0 || label - baseLabel >= (int)numEventFlags) {
				debugPrintf("\nInvalid flag %s", argv[i]);
				continue;
			}
			debugPrintf("\nFlag %d, %s, %s",
				label,
				g_nancy->getEventFlagName(label).c_str(),
				NancySceneState.getEventFlag(label, g_nancy->_true) ? "true" : "false");

		}
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_setEventFlags(int argc, const char **argv) {
	if (argc < 2 || argc % 2 == 0) {
		debugPrintf("Sets one or more event flags to the provided value.\n");
		debugPrintf("Usage: %s <flagID or name> <true/false>...\n", argv[0]);
		return true;
	}

	// Event flags are addressed by label (numbered from 1000 in Nancy3 and later)
	int baseLabel = g_nancy->getGameType() >= kGameTypeNancy3 ? 1000 : 0;
	auto *eventFlags = GetEngineData(EVNT);

	for (int i = 1; i < argc; i += 2) {
		int label = -1;
		if (Common::isDigit(argv[i][0])) {
			label = baseLabel + atoi(argv[i]);
			if (label - baseLabel >= (int)g_nancy->getStaticData().numEventFlags) {
				label = -1;
			}
		} else {
			// By name. From Nancy12 on, the flags of the game are the ones of the
			// EVNT chunk, numbered from 2000.
			const uint firstLabel = eventFlags ? (uint)EVNT::kFirstEventFlag : (uint)baseLabel;
			const uint numFlags = eventFlags ? eventFlags->eventFlagNames.size() : g_nancy->getStaticData().numEventFlags;
			for (uint j = 0; j < numFlags; ++j) {
				if (g_nancy->getEventFlagName(firstLabel + j).equalsIgnoreCase(argv[i])) {
					label = firstLabel + j;
					break;
				}
			}
		}

		if (label < 0) {
			debugPrintf("Invalid flag %s\n", argv[i]);
			continue;
		}

		if (Common::String(argv[i + 1]).compareTo("true") == 0) {
			NancySceneState.setEventFlag(label, g_nancy->_true);
			debugPrintf("Set flag %d, %s, to true\n",
				label,
				g_nancy->getEventFlagName(label).c_str());
		} else if (Common::String(argv[i + 1]).compareTo("false") == 0) {
			NancySceneState.setEventFlag(label, g_nancy->_false);
			debugPrintf("Set flag %d, %s, to false\n",
				label,
				g_nancy->getEventFlagName(label).c_str());
		} else {
			debugPrintf("Invalid value %s\n", argv[i + 1]);
			continue;
		}
	}

	return cmdExit(0, nullptr);
}

void NancyConsole::printInventoryItem(uint itemID) {
	debugPrintf("\nItem %u, %s", itemID, inventoryItemName(itemID).c_str());

	// Only the games with inventory data say what happens to a used item
	auto *inventoryData = GetEngineData(INV);
	if (inventoryData) {
		byte keep = inventoryData->itemDescriptions[itemID].keepItem;
		debugPrintf(", %s", keep == 0 ? "UseThenLose" : keep == 1 ? "KeepAlways" : keep == 2 ? "ReturnToInventory" : "NewSceneView");
	}

	uint numCharacters = numPlayerCharacters();
	if (numCharacters == 1) {
		debugPrintf(", %s", NancySceneState.hasItem(itemID) == g_nancy->_true ? "true" : "false");
		return;
	}

	// Every character keeps their own copy of the item flags, and scene
	// dependencies can ask about any of them, so list them all
	for (uint i = 0; i < numCharacters; ++i) {
		debugPrintf(", %s: %s",
			playerCharacterName(i).c_str(),
			NancySceneState.hasCharacterItem(i, itemID) == g_nancy->_true ? "true" : "false");
	}
}

bool NancyConsole::Cmd_getInventory(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	uint numItems = g_nancy->getStaticData().numItems;

	debugPrintf("Total number of inventory items: %u\n", numItems);

	if (numPlayerCharacters() > 1) {
		debugPrintf("Playing as %s\n", playerCharacterName(g_nancy->getPlayerCharacter()).c_str());
	}

	if (argc == 1) {
		for (uint i = 0; i < numItems; ++i) {
			printInventoryItem(i);
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			int itemID = atoi(argv[i]);
			if (itemID < 0 || itemID >= (int)numItems) {
				debugPrintf("\nInvalid item %s", argv[i]);
				continue;
			}

			printInventoryItem(itemID);
		}
	}

	debugPrintf("\n");

	return true;
}

bool NancyConsole::Cmd_setInventory(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	// Without -c the items go to whoever is being played, which is the only
	// inventory the games before Nancy15 have
	uint characterIndex = g_nancy->getPlayerCharacter();
	int firstItemArg = 1;

	if (argc > 1 && Common::String(argv[1]).equalsIgnoreCase("-c")) {
		if (argc < 3) {
			debugPrintf("Missing character index after -c\n");
			return true;
		}

		int requestedCharacter = atoi(argv[2]);
		if (requestedCharacter < 0 || requestedCharacter >= (int)numPlayerCharacters()) {
			debugPrintf("Invalid character %s\n", argv[2]);
			return true;
		}

		characterIndex = requestedCharacter;
		firstItemArg = 3;
	}

	if (argc < firstItemArg + 2 || (argc - firstItemArg) % 2 != 0) {
		debugPrintf("Sets one or more inventory items to the provided value.\n");
		debugPrintf("Usage: %s [-c <characterIndex>] <itemID> <true/false>...\n", argv[0]);
		debugPrintf("-c picks the player character to give the items to (Nancy15+); the character being played is the default.\n");
		return true;
	}

	Common::String targetDescription = numPlayerCharacters() > 1 ?
		Common::String::format("the inventory of %s", playerCharacterName(characterIndex).c_str()) :
		Common::String("inventory");

	for (int i = firstItemArg; i < argc; i += 2) {
		int itemID = atoi(argv[i]);
		if (itemID < 0 || itemID >= (int)g_nancy->getStaticData().numItems) {
			debugPrintf("Invalid item %s\n", argv[i]);
			continue;
		}

		if (Common::String(argv[i + 1]).compareTo("true") == 0) {
			NancySceneState.addItemToCharacterInventory(characterIndex, itemID);
			debugPrintf("Added item %i, %s, to %s\n",
				itemID,
				inventoryItemName(itemID).c_str(),
				targetDescription.c_str());
		} else if (Common::String(argv[i + 1]).compareTo("false") == 0) {
			NancySceneState.removeItemFromCharacterInventory(characterIndex, itemID);
			debugPrintf("Removed item %i, %s, from %s\n",
				itemID,
				inventoryItemName(itemID).c_str(),
				targetDescription.c_str());
		} else {
			debugPrintf("Invalid value %s\n", argv[i + 1]);
			continue;
		}
	}

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_getMoney(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (!hasMoneyResource()) {
		debugPrintf("This game doesn't have a coin purse\n");
		return true;
	}

	auto *resourceData = GetEngineData(UIRC);
	if (!resourceData || resourceData->items.empty()) {
		debugPrintf("No UI resource data loaded\n");
		return true;
	}

	// Money is UI resource 0; the purse formats it using that record's settings
	const UIRC::ItemRecord &item = resourceData->items[0];
	uint numCharacters = numPlayerCharacters();

	if (numCharacters == 1) {
		int32 value = NancySceneState.getUIResource(0);
		debugPrintf("Money: %s (%d)\n", formatUIResourceValue(item, value).c_str(), value);
		return true;
	}

	// From Nancy15 every player character carries their own money
	debugPrintf("Playing as %s\n", playerCharacterName(g_nancy->getPlayerCharacter()).c_str());

	for (uint i = 0; i < numCharacters; ++i) {
		int32 value = NancySceneState.getUIResource(0, (byte)i);
		debugPrintf("%s: %s (%d)\n",
			playerCharacterName(i).c_str(),
			formatUIResourceValue(item, value).c_str(),
			value);
	}

	return true;
}

bool NancyConsole::Cmd_setMoney(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (!hasMoneyResource()) {
		debugPrintf("This game doesn't have a coin purse\n");
		return true;
	}

	auto *resourceData = GetEngineData(UIRC);
	if (!resourceData || resourceData->items.empty()) {
		debugPrintf("No UI resource data loaded\n");
		return true;
	}

	// Without -c the money goes to whoever is being played, which is the only
	// purse the games before Nancy15 have
	uint characterIndex = g_nancy->getPlayerCharacter();
	int valueArg = 1;

	if (argc > 1 && Common::String(argv[1]).equalsIgnoreCase("-c")) {
		if (argc < 3) {
			debugPrintf("Missing character index after -c\n");
			return true;
		}

		int requestedCharacter = atoi(argv[2]);
		if (requestedCharacter < 0 || requestedCharacter >= (int)numPlayerCharacters()) {
			debugPrintf("Invalid character %s\n", argv[2]);
			return true;
		}

		characterIndex = requestedCharacter;
		valueArg = 3;
	}

	const UIRC::ItemRecord &item = resourceData->items[0];

	if (argc != valueArg + 1) {
		debugPrintf("Sets the money a player character carries.\n");
		debugPrintf("Usage: %s [-c <characterIndex>] <value>\n", argv[0]);
		debugPrintf("-c picks the player character to give the money to (Nancy15+); the character being played is the default.\n");
		debugPrintf("The value is a whole number in the purse's smallest unit, so 1234 means %s\n",
			formatUIResourceValue(item, 1234).c_str());
		return true;
	}

	int32 value = atoi(argv[valueArg]);

	if (value < 0) {
		debugPrintf("Invalid value %s\n", argv[valueArg]);
		return true;
	}

	// Nancy14 added a maximum that empties the purse instead of capping it,
	// so setting a value above it would leave the character with nothing
	if (g_nancy->getGameType() >= kGameTypeNancy14 && value > (int32)item.maxValue) {
		debugPrintf("Value %d is above the maximum of %u, which would empty the purse\n", value, item.maxValue);
		return true;
	}

	NancySceneState.setUIResource(0, value, (byte)characterIndex);

	if (numPlayerCharacters() > 1) {
		debugPrintf("Set the money %s carries to %s\n",
			playerCharacterName(characterIndex).c_str(),
			formatUIResourceValue(item, value).c_str());
	} else {
		debugPrintf("Set money to %s\n", formatUIResourceValue(item, value).c_str());
	}

	return cmdExit(0, nullptr);
}

bool NancyConsole::Cmd_getPlayerTime(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	Time time = NancySceneState.getPlayerTime();
	debugPrintf("Player time: %u days, %u hours, %u minutes; %u\n",
		time.getDays(),
		time.getHours(),
		time.getMinutes(),
		(uint32)time);

	auto *bootSummary = GetEngineData(BSUM);
	if (bootSummary && bootSummary->endOfDayFlag != kEvNoEvent) {
		// Games with an end of day keep the day separately from the clock
		debugPrintf("Day: %d\n", NancySceneState._timers.playerDay);
	}

	return true;
}

bool NancyConsole::Cmd_setPlayerTime(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	if (argc != 2 && argc != 4) {
		debugPrintf("Sets the in-game time.\n");
		debugPrintf("Usage: %s <milliseconds> or %s <days> <hours> <minutes>\n", argv[0], argv[0]);
		return true;
	}

	Time &time = NancySceneState._timers.playerTime;
	auto *bootSummary = GetEngineData(BSUM);

	if (argc == 2) {
		time = atoi(argv[1]);
	} else if (bootSummary && bootSummary->endOfDayFlag != kEvNoEvent) {
		// Games with an end of day keep the day separately from the clock
		NancySceneState.setPlayerDay(atoi(argv[1]));
		time = 	atoi(argv[2]) * 3600000 +	// hours
				atoi(argv[3]) * 60000;		// minutes
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
	if (g_nancy->getState() != NancyState::kScene) {
		debugPrintf("Not in the kScene state\n");
		return true;
	}

	debugPrintf("Difficulty: %u\n", NancySceneState.getDifficulty());

	return true;
}

bool NancyConsole::Cmd_setDifficulty(int argc, const char **argv) {
	if (g_nancy->getState() != NancyState::kScene) {
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
		const Math::Vector3d &ori = g_nancy->_sound->getOrientation();
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
		debugPrintf("%s", g_nancy->_sound->getChannelInfo(channelID).c_str());
	}

	return true;
}

bool NancyConsole::Cmd_showHotspots(int argc, const char **argv) {
	ConfMan.setBool("debug_hotspots", !ConfMan.getBool("debug_hotspots", Common::ConfigManager::kTransientDomain), Common::ConfigManager::kTransientDomain);

	return cmdExit(0, nullptr);
}

} // End of namespace Nancy
