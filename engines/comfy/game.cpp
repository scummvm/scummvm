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
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "concert")) {
		_gameDirectory = Common::Path("CONCERT");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "friends")) {
		_gameDirectory = Common::Path("FRIENDS");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "panther")) {
		_gameDirectory = Common::Path("PANTHER");
		_introDirectory.clear(); // No intro
	}

	findLanguageDirectories();
	ConfMan.registerDefault("comfy_language", 2);
	ConfMan.registerDefault("comfy_language_chosen", false);
	ConfMan.registerDefault("force_language_setup", false);
	ConfMan.registerDefault("comfy_screen_update_interleaved", false);
	ConfMan.registerDefault("comfy_input_device", 0);
}

void ComfyEngine::findLanguageDirectories() {
	uint languageCount = 0;
	_multiLanguage = false;
	for (uint i = 0; i < ARRAYSIZE(_languageDirectories); i++)
		_languageDirectories[i] = Common::Path();

	Common::FSNode root(ConfMan.getPath("path"));
	Common::FSNode gameDirectory;
	if (root.getName().equalsIgnoreCase(_gameDirectory.toString())) {
		gameDirectory = root;
	} else {
		Common::FSList gameDirectories;
		if (!root.getChildren(gameDirectories, Common::FSNode::kListDirectoriesOnly))
			return;

		for (Common::FSList::iterator it = gameDirectories.begin(); it != gameDirectories.end(); it++) {
			if (it->getName().equalsIgnoreCase(_gameDirectory.toString())) {
				gameDirectory = *it;
				break;
			}
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
	if (getLanguageDirectory(_language).empty()) {
		for (uint language = 1; language < ARRAYSIZE(_languageDirectories); language++) {
			if (!_languageDirectories[language].empty()) {
				_language = language;
				break;
			}
		}
	}

	_renderInterleaved = ConfMan.getBool("comfy_screen_update_interleaved");
	_inputDeviceMode = ConfMan.getInt("comfy_input_device");
	if (_inputDeviceMode > 4)
		_inputDeviceMode = 0;

	_inputDevicePreference = _inputDeviceMode;
	_wcomfy99Sensitivity = ConfMan.hasKey("SENSITIVITY") ? ConfMan.getInt("SENSITIVITY") : 0;

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
	_hasDataPath = !_gameDataPath.empty();
}

Common::Path ComfyEngine::pathBuild(const Common::Path &filename, bool useGamePath) {
	if (useGamePath && _hasDataPath)
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

	_stringCount = assetsReadLe16At(_comfyObjData, 0x08);
	_sceneCount = assetsReadLe16At(_comfyObjData, 0x0A);
	_keyBitCount = assetsReadLe16At(_comfyObjData, 0x0C);
	_picDataSize = assetsReadLe32At(_comfyObjData, 0x0E);
	_resourceHandleCount = assetsReadLe16At(_comfyObjData, 0x12);
	_usesAnimFile = (_resourceHandleCount & 0x8000) != 0;
	_resourceHandleCount &= 0x7FFF;
	_comfyObjOpen = true;
	return true;
}

bool ComfyEngine::picFileOpen() {
	if (!readAssetFile(Common::Path("PICFILE.DAT"), _picFileData) || _picFileData.size() < 4)
		return false;

	uint16 count = assetsReadLe16At(_picFileData, 0);
	uint32 tableSize = 2 + uint32(count) * 0x11;
	if (tableSize > _picFileData.size())
		return false;

	_spriteHeaders.resize(count);
	_spriteResources.resize(count);
	for (uint i = 0; i < count; i++) {
		_spriteHeaders[i] = SpriteObjectHeader();
		_spriteResources[i].header = SpriteObjectHeader();
		_spriteResources[i].id = i;
		_spriteResources[i].pixels.clear();
		_spriteResources[i].loaded = false;
	}

	_picFileDatOpen = true;
	return true;
}

bool ComfyEngine::midiFileOpen() {
	if (!readAssetFile(Common::Path("MIDIFILE.DAT"), _midiFileData) || _midiFileData.size() < 4)
		return false;

	if (_midiFileData[0] != 'C' || (_midiFileData[1] != 'M' && _midiFileData[1] != 'm'))
		return false;

	_midiEntryCount = assetsReadLe16At(_midiFileData, 2);
	_midiFileMode = _midiFileData[1] == 'm' ? 1 : 0;
	return true;
}

uint32 ComfyEngine::assetsAlignEven32(uint32 value) {
	return value + (value & 1);
}

uint16 ComfyEngine::assetsReadLe16At(Common::Array<byte> &data, uint32 offset) {
	return offset <= data.size() && 2 <= data.size() - offset ? READ_LE_UINT16(&data[offset]) : 0;
}

uint32 ComfyEngine::assetsReadLe32At(Common::Array<byte> &data, uint32 offset) {
	return offset <= data.size() && 4 <= data.size() - offset ? READ_LE_UINT32(&data[offset]) : 0;
}

int32 ComfyEngine::assetsXmsCopy(byte *destination, uint32 destinationSize, uint32 destinationOffset,
		const byte *source, uint32 sourceSize, uint32 sourceOffset, uint32 size) {
	if (!size)
		return 0;

	if (!destination || !source || destinationOffset > destinationSize || size > destinationSize - destinationOffset ||
			sourceOffset > sourceSize || size > sourceSize - sourceOffset)
		return -1;

	memcpy(destination + destinationOffset, source + sourceOffset, size);
	return 0;
}

void ComfyEngine::midiFileReadEntries(byte *destination) {
	if (!destination || _midiFileData.size() <= 4)
		return;

	uint32 size = uint32(_midiEntryCount) * 6;
	memcpy(destination, &_midiFileData[4], MIN<uint32>(size, _midiFileData.size() - 4));
}

byte *ComfyEngine::soundReadTiledData() {
	return _vocFileData.size() > 4 ? &_vocFileData[4] : nullptr;
}

void ComfyEngine::soundGetTileParams(uint16 *entryCount, uint16 *entrySize) {
	if (entryCount)
		*entryCount = _soundEntryCount;

	if (entrySize)
		*entrySize = 8;
}

bool ComfyEngine::selPoolInit() {
	if (_selectorPoolInitialized)
		return true;

	for (uint i = 0; i < ARRAYSIZE(_selectorPoolEntries); i++)
		_selectorPoolEntries[i] = i + 1;

	_selectorRing = 0;
	_selectorPoolInitialized = true;
	return true;
}

void ComfyEngine::selPoolFree() {
	if (!_selectorPoolInitialized)
		return;

	memset(_selectorPoolEntries, 0, sizeof(_selectorPoolEntries));
	_selectorRing = 0;
	_selectorPoolInitialized = false;
}

bool ComfyEngine::sceneLoadAndInit(uint16 sceneCount, uint16 actorCount, uint16 keyBitCount,
		uint16 handleCount, uint32 &numSprites) {
	_stringTable.resize(_stringCount + 1);
	memset(&_stringTable[0], 0, _stringTable.size() * sizeof(uint16));
	_sceneHandles.resize(sceneCount + 1);
	memset(&_sceneHandles[0], 0, _sceneHandles.size() * sizeof(uint16));
	_midiHandles.resize(handleCount + 1);
	memset(&_midiHandles[0], 0, _midiHandles.size() * sizeof(uint16));
	_actors.resize(actorCount + 1);
	memset(&_actors[0], 0, _actors.size() * sizeof(Actor));
	if (!keyBitAllocate(keyBitCount))
		return false;

	memset(_actorPcTable, 0, sizeof(_actorPcTable));
	for (uint i = 0; i < 0xC7 && i < ARRAYSIZE(_actorPcTable); i++)
		_actorPcTable[i] = uint32(i + 1) << 24;

	if (0xC7 < ARRAYSIZE(_actorPcTable))
		_actorPcTable[0xC7] = 0;

	uint32 stringBytes = _stringTable.size() * sizeof(uint16);
	uint32 handleBytes = _sceneHandles.size() * sizeof(uint16);
	uint32 actorBytes = _actors.size() * (_engineVersion == kEngineVersion3 ? COMFY_ACTOR_SIZE_V3 : COMFY_ACTOR_SIZE);
	uint32 keyBytes = (uint32(keyBitCount) + 1) / 8 + 1;
	_sceneMidiInstanceOffset = 0;
	if (_engineVersion == kEngineVersion3) {
		_sceneEntryListOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES + COMFY_SCENE_VOC_STATE_BYTES_V3 +
				COMFY_ANM_STATE_BYTES;
		_sceneActorPcOffset = _sceneEntryListOffset + COMFY_SCENE_STATE_BYTES_V3;
		_sceneStringTableOffset = _sceneActorPcOffset + COMFY_SCENE_ACTOR_PC_BYTES;
	} else {
		_sceneEntryListOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES;
		_sceneActorPcOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES + midiGetVersion();
		_sceneStringTableOffset = _sceneActorPcOffset + COMFY_SCENE_ACTOR_PC_BYTES;
	}

	_sceneHandlesOffset = _sceneStringTableOffset + stringBytes;
	_sceneActorsOffset = _sceneHandlesOffset + handleBytes;
	_sceneKeyBitsOffset = _sceneActorsOffset + actorBytes;
	_sceneAnimStateOffset = _engineVersion == kEngineVersion3 ?
			COMFY_SCENE_MIDI_INSTANCE_BYTES + COMFY_SCENE_VOC_STATE_BYTES_V3 : _sceneKeyBitsOffset + keyBytes;
	uint32 sceneBytes = assetsAlignEven32(_sceneKeyBitsOffset + keyBytes);
	_sceneMemoryBlock.resize(sceneBytes);
	memset(&_sceneMemoryBlock[0], 0, sceneBytes);
	numSprites = sceneBytes;
	_numSprites = numSprites;
	_envNumSprites = numSprites;
	_activeSceneCount = handleCount;

	if (_engineVersion == kEngineVersion3) {
		midiInitInstanceAt();
		if (!soundInit())
			return false;

		if (_usesAnimFile && !animFileOpen())
			return false;
	}

	if (!sceneOpen(_sceneEntryListOffset))
		return false;

	for (uint i = 0; i + 1 < _actors.size(); i++) {
		actorWriteU16(_actors[i], kActorNextLink, i + 1);
		actorWriteU16(_actors[i], kActorSceneHandle, 0);
	}

	actorWriteU16(_actors[_actors.size() - 1], kActorNextLink, 0);
	actorWriteU16(_actors[_actors.size() - 1], kActorSceneHandle, 0);
	actorSetFrame(0);
	return true;
}

bool ComfyEngine::assetsLoad(uint32 budget, byte *scenePtr) {
	if (!selPoolInit())
		return false;

	if (!comfyObjOpen() || !picFileOpen() || !midiFileOpen()) {
		return false;
	}

	if (_engineVersion != kEngineVersion3 && !soundInit())
		return false;

	_picFileMapped = true;

	_numFrames = _picDataSize / COMFY_TILE_SIZE + (_picDataSize % COMFY_TILE_SIZE ? 2 : 1);
	_numObjects = _spriteHeaders.size();
	uint32 numSprites = 0;
	if (!sceneLoadAndInit(_sceneCount, 0x78, _keyBitCount, _resourceHandleCount, numSprites))
		return false;

	_soundLoaded = true;

	uint32 environmentBytes = ((_envNumSprites * 5 + 0x3FF) / 0x400) * 0x400;
	_environmentData.resize(environmentBytes);
	if (environmentBytes)
		memset(&_environmentData[0], 0, environmentBytes);
	_xmsEnvAllocated = !_environmentData.empty();

	uint32 midiHeaderBytes = assetsAlignEven32(uint32(_midiEntryCount) * 6);
	uint16 soundEntryCount = 0;
	uint16 soundEntrySize = 0;
	soundGetTileParams(&soundEntryCount, &soundEntrySize);
	uint32 soundHeaderBytes = assetsAlignEven32(uint32(soundEntryCount) * soundEntrySize);
	uint32 objectTableBytes = assetsAlignEven32(uint32(_numObjects) * 0x11);
	uint32 headerTotalBytes = midiHeaderBytes + soundHeaderBytes + objectTableBytes;
	uint16 headerPages = (headerTotalBytes + 0x400) / 0x400;
	_headerXmsData.resize(uint32(headerPages) * 0x400);
	if (!_headerXmsData.empty())
		memset(&_headerXmsData[0], 0, _headerXmsData.size());

	videoInit();

	_objectCacheEntries.resize(_spriteHeaders.size());
	_frameCacheEntries.resize(_numFrames + 1);
	if (!_objectCacheEntries.empty())
		memset(&_objectCacheEntries[0], 0xFF, _objectCacheEntries.size() * sizeof(SpriteCacheEntry));

	if (!_frameCacheEntries.empty())
		memset(&_frameCacheEntries[0], 0xFF, _frameCacheEntries.size() * sizeof(SpriteCacheEntry));

	_spriteConversionLoads = ResourceLoadList();

	for (uint i = 0; i < _spriteResources.size(); i++)
		spriteInvalidateHostCache(_spriteResources[i]);

	if (_engineVersion != kEngineVersion3 && _usesAnimFile && !animFileOpen())
		return false;

	if (_usesAnimFile) {
		_animFrameBuffer.resize(COMFY_ANMFRAME_BYTES);
		memset(&_animFrameBuffer[0], 0, _animFrameBuffer.size());
	}

	uint32 xmsLimit = _engineVersion == kEngineVersion3 ? 0x7D000 : 0x0800 << 10;
	_scenePoolSize = xmsLimit > budget ? xmsLimit - budget : 0;
	_scenePoolSize &= 0xFFFFFFFE;
	_scenePoolData.resize(_scenePoolSize);
	if (_scenePoolSize)
		memset(&_scenePoolData[0], 0, _scenePoolSize);

	_scenePoolCursor = 0;
	_scenePoolEvictCursor = 0;
	if (_engineVersion != kEngineVersion3 && _scenePoolSize >= 2)
		WRITE_LE_UINT16(&_scenePoolData[0], 0x8000);

	uint32 xmsOffset = 0;
	uint32 fileOffset = 2;
	uint32 remaining = objectTableBytes;
	_headerXmsObjectTableBase = xmsOffset;
	_headerXmsObjectTableBytes = objectTableBytes;
	while (remaining) {
		uint32 chunk = MIN<uint32>(remaining, 0x8000);
		if (assetsXmsCopy(&_headerXmsData[0], _headerXmsData.size(), xmsOffset,
				&_picFileData[0], _picFileData.size(), fileOffset, chunk) < 0)
			return false;

		xmsOffset += chunk;
		fileOffset += chunk;
		remaining -= chunk;
	}

	_headerXmsMidiEntriesBase = xmsOffset;
	_headerXmsMidiEntriesBytes = midiHeaderBytes;
	if (midiHeaderBytes && scenePtr) {
		memset(scenePtr, 0, midiHeaderBytes);
		midiFileReadEntries(scenePtr);

		if (assetsXmsCopy(&_headerXmsData[0], _headerXmsData.size(), xmsOffset,
				scenePtr, midiHeaderBytes, 0, midiHeaderBytes) < 0)
			return false;

		xmsOffset += midiHeaderBytes;
	}

	_headerXmsSoundHeadersBase = xmsOffset;
	_headerXmsSoundHeadersBytes = soundHeaderBytes;
	if (soundHeaderBytes) {
		byte *soundTiledData = soundReadTiledData();
		uint32 available = _vocFileData.size() > 4 ? MIN<uint32>(soundHeaderBytes, _vocFileData.size() - 4) : 0;
		if (available && assetsXmsCopy(&_headerXmsData[0], _headerXmsData.size(), xmsOffset,
				soundTiledData, available, 0, available) < 0)
			return false;

		xmsOffset += soundHeaderBytes;
	}

	_activeSceneCount = _resourceHandleCount;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
	Common::ScopedPtr<Common::SeekableReadStream> message(pathFOpen(Common::Path("MESSAGE"), true));
	_usesWcomfy99ScriptOps = bool(message);
	_scriptFault = false;

	return true;
}

void ComfyEngine::assetsUnload(byte freeAudio) {
	if (_engineVersion == kEngineVersion3) {
		_objectCacheEntries.clear();
		_frameCacheEntries.clear();
		_scenePoolData.clear();
		spriteInvalidateHostCache(_frameSpriteResource);
		_frameSpriteResource.id = 0;
		_environmentData.clear();
		_headerXmsData.clear();

		if (_picFileMapped)
			soundShutdown();

		_picFileData.clear();
		_comfyObjData.clear();

		if (_sceneOpen)
			sceneShutdown();

		animFileShutdown();
		selPoolFree();
	} else {
		animFileShutdown();
		_spriteHeaders.clear();
		_spriteResources.clear();
		_stringTable.clear();
		_sceneHandles.clear();
		_midiHandles.clear();
		_actors.clear();
		_sceneMemoryBlock.clear();
		keyBitFree();
		_objectCacheEntries.clear();
		_frameCacheEntries.clear();
		_scenePoolData.clear();
		_environmentData.clear();
		_headerXmsData.clear();

		if (_picFileMapped)
			soundShutdown();

		_picFileData.clear();
		_comfyObjData.clear();

		if (_soundLoaded && _sceneOpen)
			sceneShutdown();

		selPoolFree();
	}

	_spriteHeaders.clear();
	_spriteResources.clear();
	_stringTable.clear();
	_sceneHandles.clear();
	_midiHandles.clear();
	_actors.clear();
	_sceneMemoryBlock.clear();
	keyBitFree();
	_midiFileData.clear();
	_spriteConversionLoads = ResourceLoadList();
	memset(_sceneEntryOffsets, 0, sizeof(_sceneEntryOffsets));
	memset(_actorPcTable, 0, sizeof(_actorPcTable));
	_usesWcomfy99ScriptOps = false;
	_currentActor = 0;
	_scriptFault = false;
	_stringCount = 0;
	_sceneCount = 0;
	_keyBitCount = 0;
	_resourceHandleCount = 0;
	_midiEntryCount = 0;
	_picDataSize = 0;
	_picFileDatOpen = false;
	_comfyObjOpen = false;
	_xmsEnvAllocated = false;
	_soundLoaded = false;
	_picFileMapped = false;
	_numFrames = 0;
	_numObjects = 0;
	_numSprites = 0;
	_envNumSprites = 0;
	_activeSceneCount = 0;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
	_sceneMidiInstanceOffset = 0;
	_sceneEntryListOffset = 0;
	_sceneActorPcOffset = 0;
	_sceneStringTableOffset = 0;
	_sceneHandlesOffset = 0;
	_sceneActorsOffset = 0;
	_sceneKeyBitsOffset = 0;
	_sceneAnimStateOffset = 0;
	_midiFileMode = 0;
	_usesAnimFile = false;
	_sceneEntryListActive = false;
	_scenePoolCursor = 0;
	_scenePoolEvictCursor = 0;
	_scenePoolSize = 0;
	_headerXmsObjectTableBase = 0;
	_headerXmsMidiEntriesBase = 0;
	_headerXmsSoundHeadersBase = 0;
	_headerXmsObjectTableBytes = 0;
	_headerXmsMidiEntriesBytes = 0;
	_headerXmsSoundHeadersBytes = 0;
	// The original consumes one UI character here; ScummVM has no teardown prompt to acknowledge.
	(void)freeAudio;
}

} // End of namespace Comfy
