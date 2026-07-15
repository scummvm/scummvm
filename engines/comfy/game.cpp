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

	if (directory.equalsIgnoreCase("FINISH") || directory.equalsIgnoreCase("FINNISH"))
		return 13;

	if (directory.equalsIgnoreCase("JAPANESE"))
		return 14;

	if (directory.equalsIgnoreCase("TURKISH"))
		return 15;

	return 0;
}

void ComfyEngine::gameConfigInit() {
	if (!strcmp(_game->desc.gameId, "comfyland")) {
		_gameDirectory = Common::Path("COMFY1");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_game->desc.gameId, "boo")) {
		_gameDirectory = Common::Path("BOO");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_game->desc.gameId, "first")) {
		_gameDirectory = Common::Path("FIRST");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_game->desc.gameId, "match")) {
		_gameDirectory = Common::Path("MATCH");
		_introDirectory = Common::Path("MINTRO");
	} else if (!strcmp(_game->desc.gameId, "colors")) {
		_gameDirectory = Common::Path("COLORS");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_game->desc.gameId, "concert")) {
		_gameDirectory = Common::Path("CONCERT");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_game->desc.gameId, "friends")) {
		_gameDirectory = Common::Path("FRIENDS");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_game->desc.gameId, "panther")) {
		_gameDirectory = Common::Path("PANTHER");
		_introDirectory.clear(); // No intro
	}

	findLanguageDirectories();
	ConfMan.registerDefault("comfy_language", 2);
	ConfMan.registerDefault("comfy_language_chosen", false);
	ConfMan.registerDefault("force_language_setup", false);
	ConfMan.registerDefault("skip_intro", false);
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

	for (uint language = 1; language < ARRAYSIZE(_languageDirectories); language++) {
		Common::String key = Common::String::format("language%u", language);
		ConfMan.set(key, _languageDirectories[language].empty() ? "?????" :
			_languageDirectories[language].toString());
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
	if (!stream || stream->size() < 0 || (uint64)stream->size() > UINT32_MAX)
		return false;

	uint32 size = stream->size();
	data.resize(size);
	if (size && stream->read(&data[0], size) != size) {
		data.clear();
		return false;
	}

	return true;
}

uint32 ComfyEngine::assetsAlignEven32(uint32 value) {
	return value + (value & 1);
}

void ComfyEngine::midiFileGetTileParams(uint16 *entryCount, uint16 *entrySize) {
	if (entryCount)
		*entryCount = _midiEntryCount;

	if (entrySize)
		*entrySize = 6;
}

void ComfyEngine::midiFileReadEntries(byte *destination) {
	if (!destination || !_midiFileStream || !_midiFileStream->seek(4))
		return;

	uint32 size = (uint32)_midiEntryCount * 6;
	_midiFileStream->read(destination, size);
}

byte *ComfyEngine::soundReadTiledData() {
	return _vocFile ? objFileReadTiled(4, (uint32)_soundEntryCount * 8, _vocFile) : nullptr;
}

void ComfyEngine::soundGetTileParams(uint16 *entryCount, uint16 *entrySize) {
	if (entryCount)
		*entryCount = _soundEntryCount;

	if (entrySize)
		*entrySize = 8;
}

void ComfyEngine::stringTableSetCount(uint16 count) {
	_stringCount = count + 1;
}

uint16 ComfyEngine::stringTableGetSize() {
	return _stringCount * sizeof(uint16);
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
	return sceneFrameInitTables(sceneCount, actorCount, keyBitCount, handleCount, numSprites);
}

bool ComfyEngine::sceneFrameInitTables(uint16 sceneCount, uint16 actorCount, uint16 keyBitCount,
		uint16 handleCount, uint32 &numSprites) {
	_stringTable.resize(_stringCount);
	memset(&_stringTable[0], 0, _stringTable.size() * sizeof(uint16));
	_sceneHandles.resize(sceneCount + 1);
	memset(&_sceneHandles[0], 0, _sceneHandles.size() * sizeof(uint16));
	_midiHandles.resize(handleCount + 1);
	memset(&_midiHandles[0], 0, _midiHandles.size() * sizeof(uint16));
	_actors.resize(actorCount + 1);
	for (uint i = 0; i < _actors.size(); i++)
		_actors[i] = Actor();

	if (!keyBitAllocate(keyBitCount))
		return false;

	memset(_actorPcTable, 0, sizeof(_actorPcTable));
	for (uint i = 0; i < 0xC7 && i < ARRAYSIZE(_actorPcTable); i++)
		_actorPcTable[i] = (uint32)(i + 1) << 24;

	if (0xC7 < ARRAYSIZE(_actorPcTable))
		_actorPcTable[0xC7] = 0;

	uint32 stringBytes = stringTableGetSize();
	uint32 handleBytes = _sceneHandles.size() * sizeof(uint16);
	uint32 actorRecordSize = COMFY_ACTOR_SIZE_V3;
	if (_engineVersion == 1)
		actorRecordSize = COMFY_ACTOR_SIZE_V1;
	else if (_engineVersion == 2)
		actorRecordSize = COMFY_ACTOR_SIZE_V2;

	uint32 actorBytes = _actors.size() * actorRecordSize;
	uint32 keyBytes = ((uint32)keyBitCount + 1) / 8 + 1;
	_sceneMidiInstanceOffset = 0;
	if (_engineVersion == 3) {
		_sceneEntryListOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES + COMFY_SCENE_VOC_STATE_BYTES_V3 +
				COMFY_ANM_STATE_BYTES;
		_sceneActorPcOffset = _sceneEntryListOffset + COMFY_SCENE_STATE_BYTES_V3;
		_sceneStringTableOffset = _sceneActorPcOffset + COMFY_SCENE_ACTOR_PC_BYTES;
	} else if (_isPanther) {
		_sceneSoundStateOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES;
		_sceneAnimStateOffset = _sceneSoundStateOffset + COMFY_PANTHER_SOUND_STATE_BYTES;
		_sceneEntryListOffset = _sceneAnimStateOffset + COMFY_PANTHER_ANM_STATE_BYTES;
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
	if (_engineVersion == 3)
		_sceneAnimStateOffset = COMFY_SCENE_MIDI_INSTANCE_BYTES + COMFY_SCENE_VOC_STATE_BYTES_V3;
	else if (!_isPanther)
		_sceneAnimStateOffset = _sceneKeyBitsOffset + keyBytes;
	uint32 sceneBytes = assetsAlignEven32(_sceneKeyBitsOffset + keyBytes);
	_sceneMemoryBlock.resize(sceneBytes);
	memset(&_sceneMemoryBlock[0], 0, sceneBytes);
	numSprites = sceneBytes;
	_numSprites = numSprites;
	_envNumSprites = numSprites;
	_activeSceneCount = handleCount;

	if (_isPanther || _engineVersion == 3)
		midiInitInstanceAt();
	else
		midiInitInstance();

	if (_isPanther || _engineVersion == 3) {
		if (!soundInit())
			return false;

		if (_usesAnimFile && !animFileOpen())
			return false;
	}

	if (!sceneOpen(_sceneEntryListOffset))
		return false;

	for (uint i = 0; i + 1 < _actors.size(); i++) {
		_actors[i].nextLink = i + 1;
		_actors[i].sceneHandle = 0;
	}

	_actors[_actors.size() - 1].nextLink = 0;
	_actors[_actors.size() - 1].sceneHandle = 0;
	actorSetFrame(0);
	return true;
}

bool ComfyEngine::assetsLoad(uint32 budget, byte *scenePtr) {
	if (!selPoolInit())
		return false;

	if (!memCompactAndCheck(0x200000))
		return false;

	uint32 comfyCacheBytes = _engineVersion == 3 ? 0x32000 : 0xA000;
	_comfyObjFile = objFileLoadSoundData(Common::Path("COMFY.OBJ"), 0x0800, comfyCacheBytes);
	if (!_comfyObjFile)
		return false;

	byte value[4];
	objFileReadField(value, 0x08, 2, _comfyObjFile);
	stringTableSetCount(READ_LE_UINT16(value));
	objFileReadField(value, 0x0A, 2, _comfyObjFile);
	_sceneCount = READ_LE_UINT16(value);
	objFileReadField(value, 0x0C, 2, _comfyObjFile);
	_keyBitCount = READ_LE_UINT16(value);
	objFileReadField(value, 0x0E, 4, _comfyObjFile);
	_picDataSize = READ_LE_UINT32(value);
	objFileReadField(value, 0x12, 2, _comfyObjFile);
	_resourceHandleCount = READ_LE_UINT16(value);
	_usesAnimFile = (_resourceHandleCount & 0x8000) != 0;
	_resourceHandleCount &= 0x7FFF;
	_comfyObjOpen = true;
	_numFrames = (uint16)(_picDataSize / COMFY_TILE_SIZE + (_picDataSize % COMFY_TILE_SIZE ? 2 : 1));
	if (_engineVersion == 3) {
		uint16 preloadCount = MIN<uint16>(_numFrames, 0x64);
		for (uint16 i = 0; i < preloadCount; i++)
			objFileReadTiled((uint32)i << 11, 8, _comfyObjFile);
	}

	_picFile = objFileLoadSoundData(Common::Path("PICFILE.DAT"), 0x0800, 0x5000);
	if (!_picFile)
		return false;

	byte objectCount[2];
	objFileReadField(objectCount, 0, sizeof(objectCount), _picFile);
	_numObjects = READ_LE_UINT16(objectCount);
	soundBufFree(_picFile);

	if (!_isPanther && _engineVersion != 3 && !soundInit())
		return false;

	_picFileMapped = true;
	uint32 numSprites = 0;
	if (!sceneLoadAndInit(_sceneCount, 0x78, _keyBitCount, _resourceHandleCount, numSprites))
		return false;

	_soundLoaded = true;
	if (!memIsReady())
		return false;

	uint16 environmentPages = (uint16)((_envNumSprites * 5 + 0x3FF) / 0x400);
	int32 environmentHandle = memAllocTrackFar(environmentPages);
	if (environmentHandle < 0)
		return false;

	_xmsEnvHandle = (uint16)environmentHandle;
	_xmsEnvAllocated = true;

	uint16 midiEntryCount = 0;
	uint16 midiEntrySize = 0;
	midiFileGetTileParams(&midiEntryCount, &midiEntrySize);
	uint32 midiHeaderBytes = assetsAlignEven32((uint32)midiEntryCount * midiEntrySize);
	uint16 soundEntryCount = 0;
	uint16 soundEntrySize = 0;
	soundGetTileParams(&soundEntryCount, &soundEntrySize);
	uint32 soundHeaderBytes = assetsAlignEven32((uint32)soundEntryCount * soundEntrySize);
	uint32 objectTableBytes = assetsAlignEven32((uint32)_numObjects * 0x11);
	uint32 headerTotalBytes = midiHeaderBytes + soundHeaderBytes + objectTableBytes;
	uint16 headerPages = (uint16)((headerTotalBytes + 0x400) / 0x400);
	int32 headerHandle = memAllocTrackFar(headerPages);
	if (headerHandle < 0)
		return false;

	_xmsHeaderHandle = (uint16)headerHandle;

	videoInit();

	_spriteHeaders.resize(_numObjects);
	_spriteResources.resize(_numObjects);
	for (uint i = 0; i < _numObjects; i++) {
		_spriteHeaders[i] = SpriteObjectHeader();
		_spriteResources[i].header = SpriteObjectHeader();
		_spriteResources[i].id = i;
		_spriteResources[i].pixels.clear();
		_spriteResources[i].loaded = false;
	}

	_objectCacheEntries.resize(_numObjects);
	_frameCacheEntries.resize(_numFrames + 1);
	if (!_objectCacheEntries.empty())
		memset(&_objectCacheEntries[0], 0xFF, _objectCacheEntries.size() * sizeof(SpriteCacheEntry));

	if (!_frameCacheEntries.empty())
		memset(&_frameCacheEntries[0], 0xFF, _frameCacheEntries.size() * sizeof(SpriteCacheEntry));

	_spriteConversionLoads = ResourceLoadList();

	for (uint i = 0; i < _spriteResources.size(); i++)
		spriteInvalidateHostCache(_spriteResources[i]);

	// Panther hardcodes the original background-buffer flag to one; WCOMFY v3 hardcodes it to zero.
	if (_isPanther) {
		_backgroundFramebuf.resize(framebufferBytes());
		memset(&_backgroundFramebuf[0], 0, _backgroundFramebuf.size());
		_backgroundFrame = 0;
	}

	if (!_isPanther && _engineVersion != 3 && _usesAnimFile && !animFileOpen())
		return false;

	if (_usesAnimFile) {
		_animFrameBuffer.resize(framebufferBytes());
		memset(&_animFrameBuffer[0], 0, _animFrameBuffer.size());
	}

	uint32 xmsLimit = memGetXmsLimitFar();
	_scenePoolSize = xmsLimit > budget ? xmsLimit - budget : 0;
	_scenePoolSize &= 0xFFFFFFFE;
	_scenePoolData.resize(_scenePoolSize);
	if (_scenePoolSize)
		memset(&_scenePoolData[0], 0, _scenePoolSize);

	_scenePoolCursor = 0;
	_scenePoolEvictCursor = 0;
	if (_engineVersion != 3 && _scenePoolSize >= 2)
		WRITE_LE_UINT16(&_scenePoolData[0], 0x8000);

	uint32 freeKB = memGetFreeKBThunk();
	if (!_isPanther && _engineVersion != 3 && freeKB < 0x12C)
		freeKB = memGetFreeKBThunk() * 0x5A / 0x64;

	if (freeKB <= 0x12C)
		return false;

	uint32 picCacheBytes = freeKB << 10;
	if (_isPanther || _engineVersion == 3)
		picCacheBytes = (freeKB - 1) << 10;

	_picFile = objFileLoadSoundData(Common::Path("PICFILE.DAT"), 0x8000, picCacheBytes);
	if (!_picFile)
		return false;

	_picFileDatOpen = true;
	uint32 xmsOffset = 0;
	uint32 fileOffset = 2;
	uint32 remaining = objectTableBytes;
	_headerXmsObjectTableBase = xmsOffset;
	_headerXmsObjectTableBytes = objectTableBytes;
	while (remaining) {
		uint32 chunk = MIN<uint32>(remaining, 0x8000);
		byte *source = objFileReadTiled(fileOffset, chunk, _picFile);
		if (!source)
			return false;

		XmsMove move;
		move.length = chunk;
		move.destinationHandle = _xmsHeaderHandle;
		move.destinationOffset = xmsOffset;
		move.sourceMemory = source;
		if (xmsTransfer(move) < 0)
			return false;

		xmsOffset += chunk;
		fileOffset += chunk;
		remaining -= chunk;
	}

	_headerXmsMidiEntriesBase = xmsOffset;
	_headerXmsMidiEntriesBytes = midiHeaderBytes;
	if (midiHeaderBytes) {
		if (!scenePtr)
			return false;

		memset(scenePtr, 0, midiHeaderBytes);
		midiFileReadEntries(scenePtr);

		XmsMove move;
		move.length = midiHeaderBytes;
		move.destinationHandle = _xmsHeaderHandle;
		move.destinationOffset = xmsOffset;
		move.sourceMemory = scenePtr;
		if (xmsTransfer(move) < 0)
			return false;

		xmsOffset += midiHeaderBytes;
	}

	_headerXmsSoundHeadersBase = xmsOffset;
	_headerXmsSoundHeadersBytes = soundHeaderBytes;
	if (soundHeaderBytes) {
		byte *soundTiledData = soundReadTiledData();
		if (!soundTiledData)
			return false;

		XmsMove move;
		move.length = soundHeaderBytes;
		move.destinationHandle = _xmsHeaderHandle;
		move.destinationOffset = xmsOffset;
		move.sourceMemory = soundTiledData;
		if (xmsTransfer(move) < 0)
			return false;

		xmsOffset += soundHeaderBytes;
	}

	_activeSceneCount = _resourceHandleCount;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
	_usesWcomfy99ScriptOps = _engineVersion == 3;
	_scriptFault = false;

	return true;
}

void ComfyEngine::assetsUnload(byte freeAudio) {
	_objectCacheEntries.clear();
	_frameCacheEntries.clear();
	_scenePoolData.clear();
	spriteInvalidateHostCache(_frameSpriteResource);
	_frameSpriteResource.id = 0;
	if (!_backgroundFramebuf.empty()) {
		_backgroundFramebuf.clear();
		_backgroundFrame = 0;
	}

	if (_xmsEnvAllocated)
		memFreeTrackFar(_xmsEnvHandle);

	if (_xmsHeaderHandle)
		memFreeTrackFar(_xmsHeaderHandle);

	if (_picFileMapped)
		soundShutdown();

	if (_picFileDatOpen)
		soundBufFree(_picFile);

	if (_comfyObjOpen)
		soundBufFree(_comfyObjFile);

	if (_soundLoaded)
		sceneShutdown();

	if (_usesAnimFile && (_isPanther || _engineVersion == 3))
		animFileShutdown();

	selPoolFree();
	_spriteHeaders.clear();
	_spriteResources.clear();
	_stringTable.clear();
	_sceneHandles.clear();
	_midiHandles.clear();
	_actors.clear();
	_sceneMemoryBlock.clear();
	keyBitFree();
	soundBufFree(_vocFile);
	soundBufFree(_picFile);
	soundBufFree(_comfyObjFile);
	delete _midiFileStream;
	_midiFileStream = nullptr;
	xmsReset();
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
	_xmsEnvHandle = 0;
	_xmsHeaderHandle = 0;
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
	_sceneSoundStateOffset = 0;
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
