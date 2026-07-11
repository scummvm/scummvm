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

#include "comfy/comfy.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/ptr.h"

namespace Comfy {

static uint16 getLanguageId(const Common::String &directory) {
	if (directory.equalsIgnoreCase("HEBREW"))
		return 1;

	if (directory.equalsIgnoreCase("ENGLISH"))
		return 2;

	if (directory.equalsIgnoreCase("GERMAN"))
		return 3;

	if (directory.equalsIgnoreCase("FRENCH"))
		return 4;

	if (directory.equalsIgnoreCase("SPANISH"))
		return 5;

	if (directory.equalsIgnoreCase("DUTCH"))
		return 6;

	if (directory.equalsIgnoreCase("PORTUGES"))
		return 7;

	if (directory.equalsIgnoreCase("ENGUK"))
		return 8;

	if (directory.equalsIgnoreCase("ITALIAN"))
		return 9;

	if (directory.equalsIgnoreCase("DANISH"))
		return 10;

	if (directory.equalsIgnoreCase("NORWEGIA"))
		return 11;

	if (directory.equalsIgnoreCase("SWEDISH"))
		return 12;

	if (directory.equalsIgnoreCase("JAPANESE"))
		return 14;

	if (directory.equalsIgnoreCase("TURKISH"))
		return 15;

	return 0;
}

void ComfyEngine::gameConfigInit() {
	if (!strcmp(_gameDescription->gameId, "comfyland")) {
		_gameDirectory = Common::Path("COMFY1");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "boo")) {
		_gameDirectory = Common::Path("BOO");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "first")) {
		_gameDirectory = Common::Path("FIRST");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "match")) {
		_gameDirectory = Common::Path("MATCH");
		_introDirectory = Common::Path("MINTRO");
	} else if (!strcmp(_gameDescription->gameId, "colors")) {
		_gameDirectory = Common::Path("COLORS");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "concert")) {
		_gameDirectory = Common::Path("CONCERT");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "friends")) {
		_gameDirectory = Common::Path("FRIENDS");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "panther")) {
		_gameDirectory = Common::Path("PANTHER");
		_introDirectory = Common::Path("MINTRO");
	}

	findLanguageDirectories();
	ConfMan.registerDefault("comfy_language", 2);
	ConfMan.registerDefault("comfy_screen_update_interleaved", false);
	ConfMan.registerDefault("comfy_input_device", 0);
}

void ComfyEngine::findLanguageDirectories() {
	uint languageCount = 0;
	_multiLanguage = false;
	for (uint i = 0; i < ARRAYSIZE(_languageDirectories); i++)
		_languageDirectories[i] = Common::Path();

	Common::FSNode root(ConfMan.getPath("path"));
	Common::FSList gameDirectories;
	if (!root.getChildren(gameDirectories, Common::FSNode::kListDirectoriesOnly))
		return;

	Common::FSNode gameDirectory;
	for (Common::FSList::iterator it = gameDirectories.begin(); it != gameDirectories.end(); it++) {
		if (it->getName().equalsIgnoreCase(_gameDirectory.toString())) {
			gameDirectory = *it;
			break;
		}
	}

	if (!gameDirectory.exists())
		return;

	Common::FSList directories;
	if (!gameDirectory.getChildren(directories, Common::FSNode::kListDirectoriesOnly))
		return;

	for (Common::FSList::iterator it = directories.begin(); it != directories.end(); it++) {
		uint16 language = getLanguageId(it->getName());
		if (language) {
			_languageDirectories[language] = Common::Path(it->getName());
			languageCount++;
		}
	}

	_multiLanguage = languageCount > 1;
}

bool ComfyEngine::iniReadGameConfig() {
	_language = ConfMan.getInt("comfy_language");
	_renderInterleaved = ConfMan.getBool("comfy_screen_update_interleaved");
	_inputDeviceMode = ConfMan.getInt("comfy_input_device");
	if (_inputDeviceMode > 2)
		_inputDeviceMode = 0;

	return !_gameDirectory.empty();
}

bool ComfyEngine::iniGetGameDataPath(uint16 sceneId) {
	if (sceneId == 0) {
		if (_introDirectory.empty())
			return false;

		pathSetGameDataDir(_introDirectory);
		return true;
	}

	if (sceneId == 0x63) {
		pathSetGameDataDir(_gameDirectory.join("SETLANG"));
		return true;
	}

	Common::Path languageDirectory = getLanguageDirectory(sceneId);
	if (languageDirectory.empty())
		return false;

	pathSetGameDataDir(_gameDirectory.join(languageDirectory));
	return true;
}

void ComfyEngine::iniWriteLanguage(uint16 language) {
	_language = language;
	ConfMan.setInt("comfy_language", language);
	ConfMan.flushToDisk();
}

Common::Path ComfyEngine::getLanguageDirectory(uint16 language) {
	if (language >= ARRAYSIZE(_languageDirectories))
		return Common::Path();

	return _languageDirectories[language];
}

void ComfyEngine::pathSetGameDataDir(const Common::Path &path) {
	_gameDataPath = path;
}

Common::Path ComfyEngine::pathBuild(const Common::Path &filename, bool useGamePath) {
	if (useGamePath && !_gameDataPath.empty())
		return _gameDataPath.join(filename);

	return filename;
}

Common::SeekableReadStream *ComfyEngine::pathFOpen(const Common::Path &filename, bool useGamePath) {
	Common::File *file = new Common::File();
	if (!file->open(pathBuild(filename, useGamePath))) {
		delete file;
		return nullptr;
	}

	return file;
}

bool ComfyEngine::readAssetFile(const Common::Path &filename, Common::Array<byte> &data) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(filename, true));
	if (!stream || stream->size() < 0 || uint64(stream->size()) > UINT32_MAX)
		return false;

	uint32 size = stream->size();
	data.resize(size);
	if (size && stream->read(&data[0], size) != size) {
		data.clear();
		return false;
	}

	return true;
}

bool ComfyEngine::comfyObjOpen() {
	if (!readAssetFile(Common::Path("COMFY.OBJ"), _comfyObjData) || _comfyObjData.size() < 0x14)
		return false;

	_stringCount = READ_LE_UINT16(&_comfyObjData[0x08]);
	_sceneCount = READ_LE_UINT16(&_comfyObjData[0x0A]);
	_keyBitCount = READ_LE_UINT16(&_comfyObjData[0x0C]);
	_picDataSize = READ_LE_UINT32(&_comfyObjData[0x0E]);
	_resourceHandleCount = READ_LE_UINT16(&_comfyObjData[0x12]);
	_usesAnimFile = (_resourceHandleCount & 0x8000) != 0;
	_resourceHandleCount &= 0x7FFF;
	return keyBitAllocate(_keyBitCount);
}

bool ComfyEngine::picFileOpen() {
	if (!readAssetFile(Common::Path("PICFILE.DAT"), _picFileData) || _picFileData.size() < 2)
		return false;

	uint16 count = READ_LE_UINT16(&_picFileData[0]);
	uint32 tableSize = 2 + uint32(count) * 0x11;
	if (tableSize > _picFileData.size())
		return false;

	_spriteHeaders.resize(count);
	_spriteResources.resize(count);
	for (uint i = 0; i < count; i++) {
		byte *entry = &_picFileData[2 + i * 0x11];
		SpriteObjectHeader &header = _spriteHeaders[i];
		header.fileOffset = READ_LE_UINT32(entry);
		header.dataSize = READ_LE_UINT16(entry + 0x04);
		header.width = READ_LE_UINT16(entry + 0x06);
		header.height = READ_LE_UINT16(entry + 0x08);
		header.hotspotX = int16(READ_LE_UINT16(entry + 0x0A));
		header.hotspotY = int16(READ_LE_UINT16(entry + 0x0C));
		header.reserved = entry[0x0E];
		header.tiledSize = READ_LE_UINT16(entry + 0x0F);
		_spriteResources[i].header = header;
		_spriteResources[i].id = i;
		_spriteResources[i].pixels.clear();
		_spriteResources[i].loaded = false;

		if (header.fileOffset > _picFileData.size() ||
				header.dataSize > _picFileData.size() - header.fileOffset)
			return false;
	}

	return true;
}

bool ComfyEngine::midiFileOpen() {
	if (!readAssetFile(Common::Path("MIDIFILE.DAT"), _midiFileData) || _midiFileData.size() < 4)
		return false;

	if (_midiFileData[0] != 'C' || (_midiFileData[1] != 'M' && _midiFileData[1] != 'm'))
		return false;

	_midiEntryCount = READ_LE_UINT16(&_midiFileData[2]);
	_midiFileMode = _midiFileData[1] == 'm' ? 1 : 0;
	return true;
}

bool ComfyEngine::assetsLoad() {
	assetsUnload();
	if (!comfyObjOpen() || !picFileOpen() || !midiFileOpen()) {
		assetsUnload();
		return false;
	}

	_stringTable.resize(_stringCount + 1);
	memset(&_stringTable[0], 0, _stringTable.size() * sizeof(uint16));
	_sceneHandles.resize(_sceneCount + 1);
	memset(&_sceneHandles[0], 0, _sceneHandles.size() * sizeof(uint16));
	_midiHandles.resize(_resourceHandleCount + 1);
	memset(&_midiHandles[0], 0, _midiHandles.size() * sizeof(uint16));
	_actors.resize(COMFY_ACTOR_COUNT);
	memset(&_actors[0], 0, _actors.size() * sizeof(Actor));
	memset(_actorPcTable, 0, sizeof(_actorPcTable));
	for (uint i = 1; i + 1 < _actors.size(); i++)
		actorWriteWord(_actors[i], kActorNextLink, i + 1);

	actorWriteWord(_actors[0], kActorNextLink, _actors.size() > 1 ? 1 : 0);
	for (uint i = 1; i < COMFY_ACTOR_PC_TABLE_COUNT; i++)
		_actorPcTable[i] = uint32(i + 1) << 24;

	_actorPcTable[COMFY_ACTOR_PC_TABLE_COUNT - 1] = 0;
	_actorPcTable[0] = 1 << 24;
	uint32 stringBytes = _stringTable.size() * sizeof(uint16);
	uint32 handleBytes = _sceneHandles.size() * sizeof(uint16);
	uint32 actorBytes = _actors.size() * sizeof(Actor);
	uint32 keyBytes = (_keyBitCount + 1) / 8 + 1;
	_sceneMidiInstanceOffset = 0;
	_sceneEntryListOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES;
	_sceneActorPcOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES + 0x167;
	_sceneStringTableOffset = _sceneActorPcOffset + COMFY_SCENE_ACTOR_PC_BYTES;
	_sceneHandlesOffset = _sceneStringTableOffset + stringBytes;
	_sceneActorsOffset = _sceneHandlesOffset + handleBytes;
	_sceneKeyBitsOffset = _sceneActorsOffset + actorBytes;
	uint32 sceneBytes = (_sceneKeyBitsOffset + keyBytes + 1) & ~1U;
	_sceneMemoryBlock.resize(sceneBytes);
	memset(&_sceneMemoryBlock[0], 0, _sceneMemoryBlock.size());
	memcpy(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable, sizeof(_actorPcTable));
	_sceneFrameData.resize(COMFY_SCENE_FRAME_BYTES);
	memset(&_sceneFrameData[0], 0, _sceneFrameData.size());
	memset(_sceneEntryOffsets, 0, sizeof(_sceneEntryOffsets));
	_numFrames = _picDataSize / COMFY_TILE_SIZE + (_picDataSize % COMFY_TILE_SIZE ? 2 : 1);
	_numObjects = _spriteHeaders.size();
	_numSprites = sceneBytes;
	_envNumSprites = _numSprites;
	_objectCacheEntries.resize(_spriteHeaders.size());
	_frameCacheEntries.resize(_numFrames + 1);
	if (!_objectCacheEntries.empty())
		memset(&_objectCacheEntries[0], 0xFF, _objectCacheEntries.size() * sizeof(SpriteCacheEntry));

	if (!_frameCacheEntries.empty())
		memset(&_frameCacheEntries[0], 0xFF, _frameCacheEntries.size() * sizeof(SpriteCacheEntry));
	memset(&_spriteConversionLoads, 0, sizeof(_spriteConversionLoads));
	_scenePoolData.resize(0x1FC000);
	memset(&_scenePoolData[0], 0, _scenePoolData.size());
	_scenePoolCursor = 0;
	_scenePoolEvictCursor = 0;
	uint32 environmentBytes = ((_envNumSprites * 5 + 0x3FF) / 0x400) * 0x400;
	_environmentData.resize(environmentBytes);
	if (environmentBytes)
		memset(&_environmentData[0], 0, environmentBytes);

	_activeSceneCount = _resourceHandleCount;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
	Common::ScopedPtr<Common::SeekableReadStream> message(pathFOpen(Common::Path("MESSAGE"), true));
	_usesWcomfy99ScriptOps = bool(message);
	_pendingScene = 0;
	_musicEventMask = 0;
	_musicEventFlag = 0;
	_musicEnabled = true;
	memset(_vocQueue, 0, sizeof(_vocQueue));
	_soundEventIndex = 0;
	_soundEventMaximum = 0;
	_soundEventSubIndex = 0xFFFF;
	_soundEventPreviousSubIndex = 0xFFFF;
	_scriptFault = false;

	return true;
}

void ComfyEngine::assetsUnload() {
	keyBitFree();
	_comfyObjData.clear();
	_picFileData.clear();
	_midiFileData.clear();
	_spriteHeaders.clear();
	_spriteResources.clear();
	_stringTable.clear();
	_sceneHandles.clear();
	_midiHandles.clear();
	_actors.clear();
	_sceneMemoryBlock.clear();
	_scenePoolData.clear();
	_environmentData.clear();
	_sceneFrameData.clear();
	_objectCacheEntries.clear();
	_frameCacheEntries.clear();
	memset(&_spriteConversionLoads, 0, sizeof(_spriteConversionLoads));
	memset(_sceneEntryOffsets, 0, sizeof(_sceneEntryOffsets));
	memset(_actorPcTable, 0, sizeof(_actorPcTable));
	_usesWcomfy99ScriptOps = false;
	_pendingScene = 0;
	_currentActor = 0;
	_scriptFault = false;
	_stringCount = 0;
	_sceneCount = 0;
	_keyBitCount = 0;
	_resourceHandleCount = 0;
	_midiEntryCount = 0;
	_picDataSize = 0;
	_numFrames = 0;
	_numObjects = 0;
	_numSprites = 0;
	_envNumSprites = 0;
	_activeSceneCount = 0;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
	_midiFileMode = 0;
	_usesAnimFile = false;
}

} // End of namespace Comfy
