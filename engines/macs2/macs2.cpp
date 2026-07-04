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

#include "macs2/macs2.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/types.h"
#include "common/util.h"
#include "engines/util.h"
#include "gameobjects.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "macs2/debugtools.h"
#include "macs2/detection.h"
#include "macs2/music.h"
#include "macs2/view1.h"

namespace Macs2 {

void resetCharacterWalkPath(Character *character) {
	if (character == nullptr || character->_gameObject == nullptr)
		return;
	const Common::Point pos = character->_gameObject->_position;
	character->_path.clear();
	character->_currentPathIndex = 0;
	character->_targetPosition = pos;
	character->_pathFinalDestination = pos;
	character->_stepDeltaX = 0;
	character->_stepDeltaY = 0;
	character->_stepError = 0;
	character->_stepDirectionSet = false;
}

Macs2Engine *g_engine;

Graphics::ManagedSurface Macs2Engine::readRLEImage(int64 offs, Common::MemoryReadStream *stream) {
	stream->seek(offs);

	Graphics::ManagedSurface result;
	result.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	// Max RLE row size: 320 pixels uncompressed = 320 bytes, but with escape sequences
	// the encoded form could be slightly larger. 1024 is more than sufficient.
	uint8 *data = new uint8[1024];

	for (int y = 0; y < 200; y++) {
		uint16 length = stream->readUint16LE();
		stream->read(data, length);
		// Signed, matching the original decodeRLERows (1008:0666): a final RLE run that
		// overshoots the row width drives this negative and terminates the row. Using an
		// unsigned counter here underflows and runs away, over-reading the encoded data and
		// corrupting this row and every row after it (spurious walkability values).
		int16 remainingPixels = 320;
		uint8 *dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8 &value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				result.setPixel(x, y, value);
				remainingPixels--;
				x++;
			} else {
				// We need to decode the RLE data
				const uint8 &runlength = dataPointer[0];
				dataPointer++;
				const uint8 &encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength && x < 320; i++) {
					result.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	delete[] data;
	return result;
}

void Macs2Engine::readResourceFile() {
	{
		// Extra scope in order to make sure no code tries to read from the file directly.
		Common::File file;
		if (!file.open("RESOURCE.MCS"))
			error("readResourceFile(): Error reading MCS file");

		int64 size = file.size();
		byte *fileData = (byte *)malloc(size);
		file.read(fileData, size);

		_fileStream = new Common::MemoryReadStream(fileData, size, DisposeAfterUse::YES);
	}

	// Full implementation here

	// File layout (from loadResourceFile at 1008:2e8d):
	//   0x00: 12-byte magic header "AHFFMCSR0100"
	//   0x0C: 2 bytes actor index + 2 bytes initial scene index (loaded below)
	//   0x10: 0x3000 bytes scene table (512 entries * 12 bytes, accessed via seek per scene)
	//   0x3010: 0x300 bytes vanilla palette (global default, overwritten per-scene by changeScene)
	//   0x3310: 0x800 bytes shading table -> sceneData+0x53D3 (64 palette entries x 32 levels)
	//   0x3B10: 33 cursor/icon image entries (4-byte size + blob data each)
	//   Then: Font 1 (4-byte size + glyph data)
	//   Then: Font 2 (4-byte size + glyph data)
	//   Then: 0x400 bytes map scene offsets -> sceneData+0x5DDB
	// We skip to the cursor images; palette/shading are loaded per-scene in changeScene.
	_fileStream->seek(0xC + 0x4 + 0x3000 + 0x300);
	_shadingTable.resize(0x800);
	_fileStream->read(_shadingTable.data(), 0x800);
	_fileStream->seek(0xC + 0x4 + 0x3000 + 0x300 + 0x800);
	readImageResources(_fileStream);
	// Font 1 follows immediately after the 33 image resource entries.
	// Original: 4-byte size field (skipped) + 2-byte glyph count + glyph data.
	uint32 font1SizeField = _fileStream->readUint32LE(); // skip size field
	(void)font1SizeField;
	uint16 font1GlyphCount = _fileStream->readUint16LE();
	maxGlyphHeight = 0;
	for (uint i = 0; i < font1GlyphCount; i++) {
		_glyphs[i].readFromMemory(_fileStream);
		maxGlyphHeight = MAX(_glyphs[i]._height, maxGlyphHeight);
	}
	numGlyphs = font1GlyphCount;

	// Font 2: clean sans-serif font used by save/load panel (scene data offset 0x1044)
	uint32 font2SizeField = _fileStream->readUint32LE();
	(void)font2SizeField;
	uint16 font2GlyphCount = _fileStream->readUint16LE();
	maxPanelGlyphHeight = 0;
	for (uint i = 0; i < font2GlyphCount && i < 256; i++) {
		_panelGlyphs[i].readFromMemory(_fileStream);
		maxPanelGlyphHeight = MAX(maxPanelGlyphHeight, _panelGlyphs[i]._height);
	}
	numPanelGlyphs = font2GlyphCount;

	// Map scene offsets: 0x400 bytes (256 entries x 4 bytes) -> scene+0x5DDB
	// First entry is the help screen image offset.
	for (int i = 0; i < 256; i++) {
		_mapSceneOffsets[i] = _fileStream->readUint32LE();
	}

	_fileStream->seek(0xC, SEEK_SET);
	Scenes::instance()._currentActorIndex = _fileStream->readUint16LE();
	uint16 firstSceneIndex = _fileStream->readUint16LE();
	Scenes::instance()._currentSceneIndex = firstSceneIndex;
	Scenes::instance()._currentSceneScript = Scenes::instance().readSceneScript(firstSceneIndex, _fileStream);
	Scenes::instance()._currentSceneStrings = Scenes::instance().readSceneStrings(firstSceneIndex, _fileStream);
	Scenes::instance()._currentSceneSpecialAnimOffsets = Scenes::instance().readSpecialAnimsOffsets(firstSceneIndex, _fileStream);
	_scriptExecutor->setScript(Scenes::instance()._currentSceneScript);

	// Load object data (512 entries max, matching original loadResourceFile)
	// Original allocates all 512 slots, then frees unused ones. We pre-fill with nullptr.
	GameObjects::instance()._objects.resize(0x200, nullptr);
	for (int i = 1; i <= 0x200; i++) {
		// The formula for the seek lives at l0037_0936
		// The global [0752h] is loaded with 3000h bytes read from offset Ch + 4h in the file
		// Regarding the 4h offset: Before the 3000h bytes, we have the values of the two globals 0776 and 077C
		uint32 addressOffset = 0x17F4 + (0xC + 0x04) + i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);
		uint32 objectOffset = _fileStream->readUint32LE();
		if (objectOffset == 0) {
			continue;
		}

		_fileStream->seek(objectOffset, SEEK_SET);
		GameObject *gameObject = new GameObject();
		gameObject->_index = i;
		gameObject->_dataOffset = objectOffset;

		// This loading happens around the l0037_082D: mark
		uint16 x = _fileStream->readUint16LE();
		uint16 y = _fileStream->readUint16LE();
		gameObject->_position = Common::Point(x, y);
		gameObject->_sceneIndex = _fileStream->readUint16LE();
		gameObject->_orientation = _fileStream->readUint16LE();
		gameObject->_verticalOffsetScale = _fileStream->readUint16LE();

		for (int j = 1; j <= 0x15; j++) {
			// Per-slot data in file: 2 bytes animID, 2 bytes sourceKey, 4 bytes dataSize, data, 2 bytes speed, 1 byte mirrorFlag, 1 byte (discarded)
			_fileStream->readUint16LE(); // runtime+0x24: animation slot ID (not used at runtime, editor metadata)
			uint16 blobSourceKey = _fileStream->readUint16LE();
			uint32 dataSize = _fileStream->readUint32LE();
			uint8 *data = new uint8[dataSize];
			_fileStream->read(data, dataSize);
			gameObject->_blobs.push_back(Common::Array<uint8>(data, dataSize));
			delete[] data;
			gameObject->_blobSourceKeys.push_back(blobSourceKey);
			// Per-slot wAnimSpeed (slot+0x0C in runtime, walk speed used by walkAlongPath)
			uint16 blobSpeed = _fileStream->readUint16LE();
			gameObject->_blobWalkSpeeds.push_back(blobSpeed);
			// Mirror flag (+0x32 in runtime)
			uint16 blobMirrorFlag = _fileStream->readByte();
			_fileStream->readByte(); // slot loaded flag (runtime-only, discarded from file)
			gameObject->_blobMirrorFlags.push_back(blobMirrorFlag != 0);

			// In order to get to l0037_0BBA: where the blob will be mirrored,
			// the bytes at +Eh and +Fh must be != 0
			// +Fh is set related to the inner loop - I think it means that
			// the blob is empty
			// +Eh is read here
			if (blobMirrorFlag != 0) {
				debugC(kDebugScript, "Object %.4x need to mirror blob %4.x", i, j);
				if (dataSize > 0) {
					BackgroundAnimationBlob::mirrorAnimBlob(gameObject->_blobs.back());
				}
			}
		}
		// Per-object rendering flags (after all 21 animation slots):
		// Binary loadObjectData reads these into runtime+0x184, +0x185, +0x186
		_fileStream->readByte();                                // runtime+0x184: hasInventoryIcon (container flag) - derived dynamically from _blobs[0x13] presence
		gameObject->_hasShading = _fileStream->readByte() != 0; // runtime+0x185: shading enabled
		gameObject->_hasScaling = _fileStream->readByte() != 0; // runtime+0x186: scaling enabled

		// Read the object script (resource offset table + script bytecode)
		// Binary: scene table at +0x17F8 holds the script data file offset for each object
		addressOffset = 0x17F8 + (0xC + 0x04) + i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);

		objectOffset = _fileStream->readUint32LE();
		// Binary loadResourceFile prunes an object slot ONLY when its DATA offset
		// (scene table +0x17F4) is zero (handled by the `continue` above). A zero
		// SCRIPT offset (+0x17F8) does NOT remove the object - it simply has no
		// script/resource table. The original keeps the slot non-null so that the
		// object set (used implicitly by save/load record ordering) stays correct.
		// Previously this did `break`, which leaked this object, left it null, and
		// aborted loading every higher-index object - corrupting the object set
		// and shifting the save-file object section.
		if (objectOffset == 0) {
			GameObjects::instance()._objects[i - 1] = gameObject;
			continue;
		}
		_fileStream->seek(objectOffset, SEEK_SET);
		// Resource offset table at +0x18D equivalent in file (128 bytes = 32 dword offsets).
		// Binary loadObjectData reads this into runtime+0x18D.
		for (int r = 0; r < 32; r++) {
			gameObject->_resourceOffsets[r] = _fileStream->readUint32LE();
		}
		uint16 scriptLength = _fileStream->readUint16LE();
		gameObject->_script.resize(scriptLength);
		_fileStream->read(gameObject->_script.data(), scriptLength);

		GameObjects::instance()._objects[i - 1] = gameObject;
	}

	// Initialize border sprites from cursor image array entries at fixed indices.
	// Original loadResourceFile (1008:2e8d) calls changeScene(g_wCurrentSceneIndex) before
	// returning, ensuring all scene data (pathfinding maps, depth map, palette, background)
	// is loaded before the game loop processes any input.
	// The original allocates the 0x75E0-byte scene data buffer (which includes space for
	// all RLE-decoded maps) before calling changeScene. Create the surfaces here.
	_sceneBackground.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_depthMap.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_pathfindingMap.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_shadowMap.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_hotspotMap.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	changeScene(Scenes::instance()._currentSceneIndex);
}

void Macs2Engine::readExecutable() {
	Common::ScopedPtr<Common::MemoryReadStream> exeFileStream;
	{
		// Extra scope in order to make sure no code tries to read from the file directly.
		Common::File file;
		if (!file.open("MCSEXEC.EXE"))
			error("readExecutable(): Error reading executable file");

		int64 size = file.size();
		byte *fileData = (byte *)malloc(size);
		file.read(fileData, size);

		exeFileStream.reset(new Common::MemoryReadStream(fileData, size, DisposeAfterUse::YES));
	}

	_adlib->readDataFromExecutable(exeFileStream.get());

	exeFileStream->seek(0x0001B610, SEEK_SET);
	inventoryIconIndices.resize(6);
	exeFileStream->read(inventoryIconIndices.data(), 12);

	exeFileStream->seek(0x0001B61C, SEEK_SET);
	containerInventoryIconIndices.resize(6);
	exeFileStream->read(containerInventoryIconIndices.data(), 12);
}

void Macs2Engine::readBackgroundAnimations(Common::MemoryReadStream *stream) {
	// changeScene (1008:2574): background animation loading at scene+0x50F5.
	// Per-entry runtime struct (0x10 bytes stride):
	//   +0x00: X position (word)
	//   +0x02: Y position (word)
	//   +0x04: blob data size (dword)
	//   +0x08: blob data pointer (far ptr, allocated at runtime)
	//   +0x0C: unknown word (read from file, not referenced at runtime)
	//   +0x0E: unknown byte (read from file, not referenced at runtime)
	//   +0x0F: unknown byte (read from file, not referenced at runtime)
	uint16 numBackgroundAnimations = stream->readUint16LE();

	_backgroundAnimations.resize(numBackgroundAnimations);
	_backgroundAnimationsBlobs.resize(numBackgroundAnimations);

	for (int i = 0; i < numBackgroundAnimations; i++) {
		BackgroundAnimationBlob &currentBlob = _backgroundAnimationsBlobs[i];
		BackgroundAnimation &current = _backgroundAnimations[i];

		// X position (+0x50E7 in scene data for entry 1)
		current._x = stream->readUint16LE();
		currentBlob._x = current._x;
		// Y position (+0x50E9)
		current._y = stream->readUint16LE();
		currentBlob._y = current._y;
		// Blob data size (+0x50EB, 4 bytes)
		uint32 numBytes = stream->readUint32LE();
		// Read raw blob data (+0x50EF points to this in runtime)
		currentBlob._blob.resize(numBytes);
		stream->read(currentBlob._blob.data(), numBytes);
		// Trailing per-animation fields (stored but not read at runtime by binary)
		currentBlob._unknown0C = stream->readUint16LE(); // +0x50F3: unknown word
		currentBlob._unknown0E = stream->readByte();     // +0x50F5: unknown byte
		currentBlob._unknown0F = stream->readByte();     // +0x50F6: unknown byte

		// Parse frames for the legacy BackgroundAnimation struct
		AnimBlobView blobView(currentBlob._blob);
		// Original uses sequence length (blob[0xA]+1) as numFrames for background animations
		uint16 numFrames = blobView.sequenceLength();
		current._frameIndex = 0;
		current._frames.resize(numFrames);
		uint16 actualFrameCount = blobView.frameCount();
		for (int j = 0; j < (int)actualFrameCount; j++) {
			AnimBlobView::FrameInfo fi;
			if (!blobView.getFrameInfo(j, fi))
				break;
			current._frames[j]._width = fi.width;
			current._frames[j]._height = fi.height;
			current._frames[j]._data.resize(fi.width * fi.height);
			memcpy(current._frames[j]._data.data(), fi.pixels, fi.width * fi.height);
		}

		// The blob's internal frame pointer is stored in the resource file data.
		// Do NOT reinitialize it - the original binary just reads the raw data.
	}
}

void Macs2Engine::readImageResources(Common::MemoryReadStream *stream) {
	// l0037_3355: Read 33 entries, preserving index alignment (zero-length = empty placeholder).
	// Binary uses g_pCursorImageArray[index] directly; indices must match.
	for (int i = 0; i < 0x21; i++) {
		uint32 length = stream->readUint32LE();
		if (length == 0) {
			AnimFrame empty;
			_imageResources.push_back(empty);
			continue;
		}
		AnimFrame frame;
		// Move forward to skip the first word
		stream->seek(0x2, SEEK_CUR);
		frame.readFromStream(stream);
		_imageResources.push_back(frame);
	}
}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			 _gameDescription(gameDesc) {
	g_engine = this;
	_scriptExecutor = new Script::ScriptExecutor();
	_scriptExecutor->_engine = this;
	_adlib = new Music();

	// We have a fixed 0x10 number of entries
	_hotspotOverrides.resize(0x11);
	for (uint i = 0; i < _hotspotOverrides.size(); i++) {
		_hotspotOverrides[i] = 0xFFFF;
	}
}

Macs2Engine::~Macs2Engine() {
#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif
	stopInputRecording();
	clearCurrentSoundData();
	_adlib->deinit();
	delete _adlib;
	delete _fileStream;
	delete _scriptExecutor;
	for (uint i = 0; i < GameObjects::instance()._objects.size(); i++) {
		delete GameObjects::instance()._objects[i];
	}
	GameObjects::instance()._objects.clear();
	delete Scenes::instance()._currentSceneScript;
	delete Scenes::instance()._currentSceneStrings;
	Scenes::instance()._currentSceneScript = nullptr;
	Scenes::instance()._currentSceneStrings = nullptr;
}

void Macs2Engine::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) const {
#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan && ConfMan.getBool("tts_enabled")) {
		ttsMan->say(text, action);
	}
#endif
}

void Macs2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_adlib && _scriptExecutor) {
		int musicVolume = ConfMan.getInt("music_volume");
		// OPL emulator is registered as kPlainSoundType; mute it at mixer level
		// when user sets music volume to 0 (OPL attenuation 0x3F is not true silence).
		_mixer->muteSoundType(Audio::Mixer::kPlainSoundType,
							  (musicVolume == 0) || (ConfMan.hasKey("mute") && ConfMan.getBool("mute")));
		_adlib->setVolume(scaledMusicVolume(_scriptExecutor->_musicControlVolume));
	}
}

uint16 Macs2Engine::scaledMusicVolume(uint16 gameAttenuation) const {
	// _masterVolume in Adlib is OPL register-level attenuation: 0 = loudest, 0x3F = silent.
	// _musicControlVolume (gameAttenuation) is also attenuation (0 = loud, 0x3F = silent).
	// Convert user's music_volume (0-255) to an attenuation (0x3F..0) and add both.
	bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	if (mute)
		return 0x3F;
	int musicVolume = ConfMan.getInt("music_volume");
	uint16 userAttenuation = 0x3F - (musicVolume * 0x3F / 255);
	uint16 total = gameAttenuation + userAttenuation;
	return (total > 0x3F) ? 0x3F : total;
}

void Macs2Engine::changeScene(uint32 newSceneIndex, bool executeScript) {
	// Release old scene resources
	_backgroundAnimations.clear();
	_backgroundAnimationsBlobs.clear();
	memset(_areaOverrides, 0, sizeof(_areaOverrides));

	// Background image
	// [0752h] is pointing to 3000h bytes data starting at Ch + 4h in the file
	// Addressing the background image starts at l0037_25A9
	_fileStream->seek(0xC + 0x4 + 0xC * newSceneIndex - 0xC, SEEK_SET);
	uint32 bgImageOffset = _fileStream->readUint32LE();
	uint32 sceneTableEntry2 = _fileStream->readUint32LE();
	uint32 sceneTableEntry3 = _fileStream->readUint32LE();
	(void)sceneTableEntry3; // strings offset, not used here
	_mapSubSceneTableFilePos = 0;
	_mapImageFileOffset = 0;
	// The map image file offset is stored in the scene data block at offset +0x3C0.
	// (sceneDataOffset2 + 0x3C0 = resource_offsets(0x80) + 0x340 of additional data).
	if (sceneTableEntry2 != 0 && sceneTableEntry2 < (uint32)_fileStream->size()) {
		_fileStream->seek(sceneTableEntry2 + 0x3C0, SEEK_SET);
		uint32 mapOffset = _fileStream->readUint32LE();
		if (mapOffset != 0 && mapOffset < (uint32)_fileStream->size()) {
			// Validate it's actually RLE data for a 320-wide image (row len typically 50-320)
			_fileStream->seek(mapOffset, SEEK_SET);
			uint16 rowLen = _fileStream->readUint16LE();
			if (rowLen >= 50 && rowLen <= 640) {
				_mapImageFileOffset = mapOffset;
				_mapSubSceneTableFilePos = sceneTableEntry2 + 0x3C0;
			}
		}
	}
	_fileStream->seek(bgImageOffset, SEEK_SET);

	// TODO: Copy-pasted code here

	uint8 data[0x320];

	for (int y = 0; y < 200; y++) {
		// TODO: Use the proper read function, it seems to be available
		uint16 length = _fileStream->readUint16LE();
		_fileStream->read(data, length);
		int16 remainingPixels = 320; // signed: see readRLEImage (matches decodeRLERows 1008:0666)
		uint8 *dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8 &value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				_sceneBackground.setPixel(x, y, value);
				remainingPixels--;
				x++;
			} else {
				// We need to decode the RLE data
				const uint8 &runlength = dataPointer[0];
				dataPointer++;
				const uint8 &encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength && x < 320; i++) {
					_sceneBackground.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	// We load the palette right afterwards - 0x300 is exactly 3 * 256d
	Common::Array<uint8> palette;
	palette.resize(0x300);
	_fileStream->read(palette.data(), 0x300);

	// TODO: Copy-pasted code here
	// Make a copy that will not be color corrected, for fading
	memcpy(_palVanilla, palette.data(), 256 * 3);
	memcpy(_pal, palette.data(), 0x300);

	// Adjust the palette
	for (int i = 0; i < 256 * 3; i++) {
		_pal[i] = (_pal[i] * 259 + 33) >> 6;
	}

	// changeScene @ 1008:2574: 0x100-byte panel remap table (scene+0x1006 area, NOT shading table)
	if (_panelRemapTable.size() != 0x100)
		_panelRemapTable.resize(0x100);
	_fileStream->read(_panelRemapTable.data(), 0x100);

	_fileStream->readByte(); // unknownByte1
	_fileStream->readByte(); // unknownByte2
	_fileStream->readByte(); // unknownByte3

	// Offset 1013h
	Graphics::ManagedSurface depthRLE = readRLEImage(_fileStream->pos(), _fileStream);
	// Confirmed: depth map at scene offset 0x1013
	_depthMap.blitFrom(depthRLE);

	// Offset 2017h
	Graphics::ManagedSurface pathfindingRLE = readRLEImage(_fileStream->pos(), _fileStream);
	// Walkability/pathfinding map at scene offset 0x2017
	_pathfindingMap.blitFrom(pathfindingRLE);

	// Offset 301Bh - Shadow/shading intensity map for character rendering
	Graphics::ManagedSurface shadowRLE = readRLEImage(_fileStream->pos(), _fileStream);
	_shadowMap.blitFrom(shadowRLE);

	// Offset 401Fh - Hotspot/interaction map (320x200, pixel value = hotspot color index)
	Graphics::ManagedSurface bgMap = readRLEImage(_fileStream->pos(), _fileStream);
	_hotspotMap.copyFrom(bgMap);

	// Pretty sure that this is the pathfinding points. We address them starting
	// Load pathfinding nodes (16 entries x 10 bytes at scene+0x5023)
	pathfindingPoints.clear();
	for (int i = 0; i < 16; i++) {
		PathfindingPoint current;
		current._index = i;
		current._position.x = _fileStream->readUint16LE();
		current._position.y = _fileStream->readUint16LE();
		uint8 adj[4];
		_fileStream->read(adj, 4);
		uint16 numConnections = _fileStream->readUint16LE();
		current._adjacentPoints.clear();
		for (uint16 j = 0; j < numConnections && j < 4; j++)
			current._adjacentPoints.push_back(adj[j]);
		pathfindingPoints.push_back(current);
	}

	_numHotspots = _fileStream->readUint16LE();

	_hotspotColorTable.clear();
	_hotspotColorTable.resize(0x20 / sizeof(uint16));
	_fileStream->read(_hotspotColorTable.data(), 0x20);

	// TODO: Remove the now superfluous one
	readBackgroundAnimations(_fileStream);

	// Offset 51F7h
	_numPathfindingPoints = _fileStream->readUint16LE();

	// Offset 51F9h
	_fileStream->readUint16LE();

	// Offset 51FBh
	_fileStream->readUint16LE();

	// Offset 51FDh - 5201h
	_walkDepthThresholdY = _fileStream->readUint16LE();
	_walkDepthScaleFactor = _fileStream->readUint16LE();
	_walkBaseSpeedPct = _fileStream->readUint16LE();

	_scenePaletteMode = _fileStream->readUint16LE();
	_paletteDarkenPercent = _fileStream->readUint16LE();

	// Seek to next place
	// TODO: Duplicated seek address calculation code
	// This addressing can be found in the l0037_2856 code block

	uint16 sceneDataOffset = newSceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	_fileStream->seek(sceneDataOffset - 0x8);
	uint32 sceneDataOffset2 = _fileStream->readUint32LE();
	_fileStream->seek(sceneDataOffset2, SEEK_SET);

	// We read 80h bytes
	_sceneResourceOffsets.resize(0x80 / 4);
	_fileStream->read(_sceneResourceOffsets.data(), 0x80);

	// TODO: There are some more data points missing from the function

	// Apply palette darkening if this scene has it (binary: sceneData+0x5203 != 1)
	applyPaletteDarkening();

	// Refresh characters
	View1 *currentView = (View1 *)findView("View1");
	if (!currentView) {
		// View system not yet initialized (first call from readResourceFile).
		// Scene data is loaded; view refresh will happen on first tick.
		return;
	}

	// Refresh the surface
	currentView->_backgroundSurface.copyFrom(_sceneBackground);
	// scriptChangeScene (1008:ad6e) manages palette via fade/instant-cut paths when
	// executeScript==false; avoid pushing _pal via draw() before fadePaletteToBlack.
	if (executeScript) {
		currentView->_paletteDirty = true;
	}
	currentView->handleTextBoxInput();
	currentView->_drawnStringBox.clear();
	currentView->_continueScriptAfterUI = false;
	currentView->currentSpeechActData = SpeechActData();
	currentView->_uiPanelState = View1::kUiPanelNone;
	currentView->_pendingPanelRequest = View1::kPanelRequestNone;
	currentView->_activeInventoryItem = nullptr;
	currentView->_uiPanelState = View1::kUiPanelNone;
	currentView->clearOverlayTextEntries();
	_scriptExecutor->_inventoryActionFlag = false;
	_scriptExecutor->_inventoryCombineFlag = false;

	for (auto currentCharacter : currentView->_characters) {
		if (currentCharacter->_gameObject != nullptr)
			_scriptExecutor->saveWalkRuntime(currentCharacter, currentCharacter->_gameObject);
		delete currentCharacter;
	}
	currentView->_characters.clear();
	currentView->flushPendingCharacterDeletes();
	// Binary changeScene (1008:2574): loadObjectData for scene objects except current actor.
	GameObject *actorObject = GameObjects::getObjectByIndex(Scenes::instance()._currentActorIndex);
	if (actorObject != nullptr && actorObject->_sceneIndex == newSceneIndex) {
		Character *actorChar = new Character();
		actorChar->_gameObject = actorObject;
		currentView->_characters.push_back(actorChar);
		_scriptExecutor->restoreWalkRuntime(actorChar, actorObject);
		resetCharacterWalkPath(actorChar);
		_scriptExecutor->saveWalkRuntime(actorChar, actorObject);
	}
	for (auto currentObject : GameObjects::instance()._objects) {
		if (currentObject == nullptr)
			continue;
		if (currentObject->_sceneIndex == newSceneIndex &&
			currentObject->_index != Scenes::instance()._currentActorIndex &&
			currentObject->_dataOffset != 0 &&
			loadObjectData(currentObject)) {
			Character *c = new Character();
			c->_gameObject = currentObject;
			currentView->_characters.push_back(c);
			_scriptExecutor->restoreWalkRuntime(c, currentObject);
			resetCharacterWalkPath(c);
			_scriptExecutor->saveWalkRuntime(c, currentObject);
		}
	}

	currentView->rebuildCharacterLookupTable();

	// Load the script and execute it
	Scenes::instance()._lastSceneIndex = Scenes::instance()._currentSceneIndex;
	Scenes::instance()._currentSceneIndex = newSceneIndex;
	// Free any object-specific stream before deleting the scene script
	_scriptExecutor->releaseObjectStream();
	delete Scenes::instance()._currentSceneScript;
	delete Scenes::instance()._currentSceneStrings;
	Scenes::instance()._currentSceneScript = Scenes::instance().readSceneScript(newSceneIndex, _fileStream);
	Scenes::instance()._currentSceneStrings = Scenes::instance().readSceneStrings(newSceneIndex, _fileStream);
	Scenes::instance()._currentSceneSpecialAnimOffsets = Scenes::instance().readSpecialAnimsOffsets(newSceneIndex, _fileStream);
	_scriptExecutor->setScript(Scenes::instance()._currentSceneScript);

	// Reset overrides before running the new scene's script (original placement:
	// memsetBytes(0, 200, sceneData+0x528D) and memsetBytes(0xffff, 0x20, sceneData+0x5BD3)
	// happen after all scene data is loaded, before script execution)
	_pathfindingOverrides.clear();
	for (uint i = 0; i < _hotspotOverrides.size(); i++) {
		_hotspotOverrides[i] = 0xFFFF;
	}

	saveAutosaveIfEnabled();

	if (executeScript) {
		_scriptExecutor->runSceneEntryScriptPasses();
	}
}

bool Macs2Engine::loadOverlayFont(uint8 resourceIndex, uint16 executingObjectID) {
	// Original (1008:d749): looks up file offset from scene/object resource table
	// at scene+0x5209+index*4 (same table as loadIndexedResource/_sceneResourceOffsets),
	// seeks to offset+0x10, then calls loadFontData.
	if (resourceIndex == 0)
		return false;

	const int64 oldPos = _fileStream->pos();
	uint32 address = 0;

	if (executingObjectID == 0) {
		if (resourceIndex > _sceneResourceOffsets.size()) {
			_fileStream->seek(oldPos, SEEK_SET);
			return false;
		}
		address = _sceneResourceOffsets[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::getObjectByIndex(executingObjectID);
		if (object == nullptr || object->_dataOffset == 0) {
			_fileStream->seek(oldPos, SEEK_SET);
			return false;
		}
		_fileStream->seek(object->_dataOffset + 0x189 + (resourceIndex - 1) * 4, SEEK_SET);
		address = _fileStream->readUint32LE();
	}

	if (address == 0) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	// Seek to address + 0x10 (original skips 16-byte resource header)
	_fileStream->seek(address + 0x10, SEEK_SET);
	uint16 glyphCount = _fileStream->readUint16LE();
	if (glyphCount == 0 || glyphCount > 256) {
		_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	numOverlayGlyphs = glyphCount;
	maxOverlayGlyphHeight = 0;
	for (uint i = 0; i < glyphCount; i++) {
		_overlayGlyphs[i].readFromMemory(_fileStream);
		maxOverlayGlyphHeight = MAX(maxOverlayGlyphHeight, _overlayGlyphs[i]._height);
	}
	_fileStream->seek(oldPos, SEEK_SET);
	return true;
}

bool Macs2Engine::findGlyph(char c, GlyphData &out) const {
	for (int i = 0; i < numGlyphs; i++) {
		if (_glyphs[i]._ascii == c) {
			out = _glyphs[i];
			return true;
		}
	}
	return false;
}

// getWalkabilityAt (1008:0e8c)
// Params: (param_1=y, param_2=x)
// Bounds: x<0 || x>=320 || y<0 || y>=200 -> return 0
// Lookup: scene[y*4 + 0x2017] -> row pointer, then byte at [rowPtr + x]
// Values 0xC8..0xEF: override range - checks scene[value*5 + 0x4EA5]:
//   If override disabled (flag==0): returns 0xFF
//   If override enabled (flag!=0): returns scene[value*5 + 0x4EA6]
uint16 Macs2Engine::getWalkabilityAt(int16 y, int16 x) {
	if (x < 0 || x >= 320 || y < 0 || y >= 200 || _pathfindingMap.w == 0) {
		return 0;
	}
	uint16 value = _pathfindingMap.getPixel(x, y);
	if (value >= 0xC8 && value <= 0xEF) {
		uint16 overrideResult;
		if (getPathfindingOverride(value, overrideResult)) {
			return overrideResult;
		}
		return 0xFF;
	}
	return value;
}

// snapToWalkablePosition (1008:9be2)
// Params: (pTargetY, pTargetX, charY, charX)
// Modifies *pTargetY and *pTargetX in place.
void Macs2Engine::snapToWalkablePosition(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX) {
	int16 savedX = *pTargetX;
	int16 savedY = *pTargetY;

	// Phase 1: Scan downward with depth constraint
	// Condition: walkability >= 200 OR (targetY - walkability) < savedY
	while (true) {
		uint16 w = getWalkabilityAt(*pTargetY, savedX);
		if (w < 200 && (*pTargetY - (int16)w >= savedY)) {
			break;
		}
		if (*pTargetY >= 199) {
			break;
		}
		*pTargetY = *pTargetY + 1;
	}

	// Phase 2: Continue scanning to bottom for best depth match
	int16 scanY = *pTargetY;
	while (scanY <= 199) {
		uint16 w = getWalkabilityAt(scanY, *pTargetX);
		if (scanY - (int16)w == savedY) {
			*pTargetY = scanY;
		}
		if (scanY == 199) {
			break;
		}
		scanY++;
	}

	// Phase 3: If at screen bottom and still non-walkable, scan upward
	if (*pTargetY == 199) {
		uint16 w = getWalkabilityAt(*pTargetY, *pTargetX);
		if (w >= 200) {
			while (w >= 200 && *pTargetY > 0) {
				*pTargetY = *pTargetY - 1;
				w = getWalkabilityAt(*pTargetY, *pTargetX);
			}
		}
	}

	// Phase 4: If still non-walkable, scan X toward character
	uint16 w = getWalkabilityAt(*pTargetY, *pTargetX);
	if (w >= 200) {
		*pTargetX = savedX;
		*pTargetY = savedY;
		if (charX < *pTargetX) {
			while (true) {
				uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
				if (w2 < 200)
					break;
				if (*pTargetX <= 0)
					break;
				*pTargetX = *pTargetX - 1;
			}
		} else {
			while (true) {
				uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
				if (w2 < 200)
					break;
				if (*pTargetX >= 319)
					break;
				*pTargetX = *pTargetX + 1;
			}
		}
		// Phase 5: If all failed, fall back to character position
		uint16 w2 = getWalkabilityAt(*pTargetY, *pTargetX);
		if (w2 >= 200) {
			*pTargetX = charX;
			*pTargetY = charY;
		}
	}

	// Phase 6: Gradient-based wall push
	int16 pushX = 0;
	int16 pushY = 0;
	if (getWalkabilityAt(*pTargetY, *pTargetX + 1) >= 200)
		pushX--;
	if (getWalkabilityAt(*pTargetY, *pTargetX - 1) >= 200)
		pushX++;
	if (getWalkabilityAt(*pTargetY + 1, *pTargetX) >= 200)
		pushY--;
	if (getWalkabilityAt(*pTargetY - 1, *pTargetX) >= 200)
		pushY++;
	if (getWalkabilityAt(*pTargetY, *pTargetX + 2) >= 200)
		pushX--;
	if (getWalkabilityAt(*pTargetY, *pTargetX - 2) >= 200)
		pushX++;
	if (getWalkabilityAt(*pTargetY + 2, *pTargetX) >= 200)
		pushY--;
	if (getWalkabilityAt(*pTargetY - 2, *pTargetX) >= 200)
		pushY++;

	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (getWalkabilityAt(*pTargetY, *pTargetX - 1) < 200) {
				*pTargetX = *pTargetX - 1;
			}
			pushX++;
		}
		if (pushX > 0) {
			if (getWalkabilityAt(*pTargetY, *pTargetX + 1) < 200) {
				*pTargetX = *pTargetX + 1;
			}
			pushX--;
		}
		if (pushY < 0) {
			if (getWalkabilityAt(*pTargetY - 1, *pTargetX) < 200) {
				*pTargetY = *pTargetY - 1;
			}
			pushY++;
		}
		if (pushY > 0) {
			if (getWalkabilityAt(*pTargetY + 1, *pTargetX) < 200) {
				*pTargetY = *pTargetY + 1;
			}
			pushY--;
		}
	}
}

bool Macs2Engine::getPathfindingOverride(uint16 index, uint16 &result) {
	for (auto current : _pathfindingOverrides) {
		if (current._index == index && current._active) {
			result = current._overrideValue;
			return true;
		}
	}
	return false;
}
void Macs2Engine::setPathfindingOverride(uint16 index, uint16 overrideValue) {
	removePathfindingOverride(index);
	PathfindingAreaOverride override;
	override._active = true;
	override._index = index;
	override._overrideValue = overrideValue;
	_pathfindingOverrides.push_back(override);
}

uint16 Macs2Engine::getPathfindingOverride2(uint16 index) {
	if (index < AREA_OVERRIDE_MIN || index > AREA_OVERRIDE_MAX) {
		return 0;
	}
	return _areaOverrides[index - AREA_OVERRIDE_MIN];
}

void Macs2Engine::removePathfindingOverride(uint16 index) {
	for (uint i = 0; i < _pathfindingOverrides.size(); i++) {
		PathfindingAreaOverride &current = _pathfindingOverrides[i];
		if (current._index == index) {
			_pathfindingOverrides.remove_at(i);
			return;
		}
	}
};

// isPathWalkable (1008:1196)
// Params: (param_1=y1, param_2=x1, param_3=y2, param_4=x2)
// Traces from (x2,y2) toward (x1,y1). Checks walkability only on major-axis steps.
// Uses unsigned 16-bit error accumulator with wrapping arithmetic.
// Returns true if entire line is walkable (all sampled pixels < 0xC8).
bool Macs2Engine::isPathWalkable(int16 y1, int16 x1, int16 y2, int16 x2) {
	uint16 error = 0;
	int16 curX = x2;
	int16 curY = y2;
	uint16 absDx = (uint16)abs((int)(x2 - x1));
	uint16 absDy = (uint16)abs((int)(y2 - y1));
	bool result = true;

	do {
		bool steppedX;
		if (error >= absDx) {
			if (y1 < y2)
				curY--;
			if (y2 < y1)
				curY++;
			error -= absDx;
			steppedX = false;
		} else {
			if (x1 < x2)
				curX--;
			if (x2 < x1)
				curX++;
			error += absDy;
			steppedX = true;
		}

		if (absDx > absDy && steppedX) {
			if (getWalkabilityAt(curY, curX) >= 0xC8)
				result = false;
		}
		if (absDx <= absDy && !steppedX) {
			if (getWalkabilityAt(curY, curX) >= 0xC8)
				result = false;
		}
	} while (curX != x1 || curY != y1);

	return result;
}

// Binary euclideanDistance (1008:1390): integer Euclidean distance approximation.
// Iterates i from 0 until i^2 >= dx^2 + dy^2. Capped at 0x500.
int Macs2Engine::euclideanDistance(const Common::Point &a, const Common::Point &b) {
	int32 dx = abs((int)(b.x - a.x));
	int32 dy = abs((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int i = 0;
	while (i < 0x500 && (int32)i * i < distSq)
		i++;
	return i;
}

// Binary walkableDistance (1008:1293): distance between two nodes IF walkable, else 0x500.
// Uses binary search on precomputed squared-distance table (scene+0x61DC) for O(log n) sqrt.
int Macs2Engine::walkableDistance(int nodeA, int nodeB) {
	const Common::Point &a = pathfindingPoints[nodeA - 1]._position;
	const Common::Point &b = pathfindingPoints[nodeB - 1]._position;
	if (!isPathWalkable(a.y, a.x, b.y, b.x))
		return 0x500;
	// Binary search for integer sqrt(dx² + dy²), matching binary at 1008:1293
	int32 dx = abs((int)(b.x - a.x));
	int32 dy = abs((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int result = 0x280;
	int step = 0x280;
	do {
		step = step >> 1;
		if ((int32)result * result >= distSq) {
			result -= step;
		} else {
			result += step;
		}
	} while (step > 1);
	return result;
}

// Binary buildPathFromNodes (1008:15a8): recursive DFS cost to reach a reachable node.
// Full recursive DFS with visited-stack cycle detection matching binary exactly.
// Terminal: returns walkableDistance(node, finalDest) when node is reachable.
// Recursive: min(computeMinCostToReachable(adj)) + walkableDistance(bestAdj, current).
int Macs2Engine::computeMinCostToReachable(int nodeIndex, int prevNode, uint16 actorIndex, const bool *reachable, int nodeCount, const Common::Point &finalDest) {
	// Static visited stack (matches binary's stack-frame approach, max 16 nodes)
	static int visitedStack[17];
	static int visitedCount = 0;

	// Push current node to visited stack
	visitedCount++;
	visitedStack[visitedCount] = nodeIndex;

	int result;
	const Common::Point &nodePos = pathfindingPoints[nodeIndex - 1]._position;

	if (reachable[nodeIndex]) {
		// Terminal: return walkable distance from this node to finalDest
		// Binary calls walkableDistance(nodePos, finalDest) = findPathNode(1008:1293)
		if (!isPathWalkable(nodePos.y, nodePos.x, finalDest.y, finalDest.x)) {
			result = 0x500;
		} else {
			int32 dx = abs((int)(finalDest.x - nodePos.x));
			int32 dy = abs((int)(finalDest.y - nodePos.y));
			int32 distSq = dx * dx + dy * dy;
			int dist = 0x280;
			int step = 0x280;
			do {
				step = step >> 1;
				if ((int32)dist * dist >= distSq) {
					dist -= step;
				} else {
					dist += step;
				}
			} while (step > 1);
			result = dist;
		}
		visitedCount--;
		return result;
	}

	int bestCost = 0x7777;
	int bestAdj = 0;
	const PathfindingPoint &pt = pathfindingPoints[nodeIndex - 1];
	int adjCount = (int)pt._adjacentPoints.size();

	if (adjCount > 0) {
		for (int i = 0; i < adjCount; i++) {
			int adj = pt._adjacentPoints[i];
			if (adj == prevNode)
				continue;

			// Check visited stack
			bool alreadyVisited = false;
			for (int j = 1; j < visitedCount; j++) {
				if (visitedStack[j] == adj) {
					alreadyVisited = true;
					break;
				}
			}
			if (alreadyVisited)
				continue;

			// Recursive call
			int cost = computeMinCostToReachable(adj, nodeIndex, actorIndex, reachable, nodeCount, finalDest);
			if (cost < bestCost) {
				bestAdj = adj;
				bestCost = cost;
			}
		}
	}

	if (bestCost < 0x7777) {
		// Add edge cost: walkable distance from bestAdj to current node
		result = bestCost + walkableDistance(bestAdj, nodeIndex);
	} else {
		result = 0x7777;
	}

	// Pop visited stack
	visitedCount--;
	return result;
}

void Macs2Engine::setCursorMode(Script::MouseMode newMode) {
	// setCursorMode (1008:3ea5): binary adjusts top-left by old/new half-extents.
	const bool cursorVisible = CursorMan.isVisible();

	_scriptExecutor->_cursorMode = newMode;
	_clipRectDirty = true;

	if (View1 *view = (View1 *)findView("View1"))
		view->updateCursor();

	if (cursorVisible)
		_needsRedraw = true;

	debugC(kDebugInput, "Cursor mode set to %i (%s)", (int)newMode,
		   newMode == Script::MouseMode::Talk ? "Talk" : newMode == Script::MouseMode::Look       ? "Look"
													 : newMode == Script::MouseMode::Use          ? "Use"
													 : newMode == Script::MouseMode::Walk         ? "Walk"
													 : newMode == Script::MouseMode::UseInventory ? "UseInventory"
													 : newMode == Script::MouseMode::PanelUse     ? "PanelUse"
													 : newMode == Script::MouseMode::PanelCursor  ? "PanelCursor"
													 : newMode == Script::MouseMode::Disabled     ? "Disabled"
																								  : "Unknown");
}

uint16 Macs2Engine::getHotspotAtPoint(const Common::Point &p) {
	uint16 result = 0;
	// TODO: Abstract the screen sizes
	if (p.x < 0 || p.x >= 320 || p.y < 0 || p.y >= 200 || _hotspotMap.w == 0) {
		return result;
	}

	uint8 firstLookup = _hotspotMap.getPixel(p.x, p.y);
	uint16 numHotspots = _numHotspots;

	uint8 i = 1;
	if (i > numHotspots) {
		return result;
	}

	Common::Array<uint16> a = _hotspotColorTable;

	do {
		if ((uint)(i - 1) >= a.size())
			break;
		// Binary compares only the low byte: *(char*)(scene + i*2 + 0x50D3)
		uint8 lookup = (uint8)a[i - 1];
		if (lookup == firstLookup) {
			if (_hotspotOverrides[i] != 0xFFFF) {
				return 0x800 + _hotspotOverrides[i];
			}
			return 0x800 + i;
		}
		i++;
	} while (i <= numHotspots);
	return 0;
}

void Macs2Engine::scheduleRun(bool initScene) {
	_runScheduled = true;
	_scheduledRunIsInitScene = initScene;
}

void Macs2Engine::startInputRecording(const Common::Path &filename) {
	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(filename)) {
		warning("Failed to open recording file %s", filename.toString().c_str());
		delete f;
		return;
	}
	// Write header matching original format: 12-byte magic "AHFFMCSR0100"
	f->write("AHFFMCSR0100", 12);
	_inputRecordStream = f;
	_inputMode = InputMode::Record;
	_inputFrameCounter = 0;
	debug("Input recording started: %s", filename.toString().c_str());
}

void Macs2Engine::startInputPlayback(const Common::Path &filename) {
	Common::File *f = new Common::File();
	if (!f->open(filename)) {
		warning("Failed to open playback file %s", filename.toString().c_str());
		delete f;
		return;
	}
	// Skip 12-byte header
	f->skip(12);
	// Read first record's frame counter to prime the playback target
	_inputPlaybackEndFrame = f->readUint16LE();
	_inputPlaybackStream = f;
	_inputMode = InputMode::Playback;
	_inputFrameCounter = 0;
	debug("Input playback started: %s (first event at frame %u)", filename.toString().c_str(), _inputPlaybackEndFrame);
}

void Macs2Engine::stopInputRecording() {
	if (_inputRecordStream) {
		_inputRecordStream->finalize();
		delete _inputRecordStream;
		_inputRecordStream = nullptr;
	}
	if (_inputPlaybackStream) {
		delete _inputPlaybackStream;
		_inputPlaybackStream = nullptr;
	}
	_inputMode = InputMode::None;
}

void Macs2Engine::recordInputFrame(uint16 mouseX, uint16 mouseY, uint16 buttons) {
	if (_inputRecordStream) {
		_inputFrameCounter++;
		_inputRecordStream->writeUint16LE(_inputFrameCounter);
		_inputRecordStream->writeUint16LE(mouseX);
		_inputRecordStream->writeUint16LE(mouseY);
		_inputRecordStream->writeUint16LE(buttons);
	}
}

bool Macs2Engine::readInputFrame(uint16 &mouseX, uint16 &mouseY, uint16 &buttons) {
	if (!_inputPlaybackStream || _inputPlaybackStream->eos())
		return false;
	// Format: each record is [frameCounter(2), mouseX(2), mouseY(2), buttons(2)]
	// Playback waits until current frame >= next record's frame counter
	if (_inputFrameCounter < _inputPlaybackEndFrame)
		return false;
	mouseX = _inputPlaybackStream->readUint16LE();
	mouseY = _inputPlaybackStream->readUint16LE();
	buttons = _inputPlaybackStream->readUint16LE();
	if (_inputPlaybackStream->eos())
		return false;
	// Read next record's frame counter (or detect end)
	_inputPlaybackEndFrame = _inputPlaybackStream->readUint16LE();
	return !_inputPlaybackStream->eos();
}

uint16 Macs2Engine::getWalkabilityAt(const Common::Point &p) {
	return getWalkabilityAt((int16)p.y, (int16)p.x);
}

int Macs2Engine::measureString(const Common::String &s) {
	int sum = 0;
	GlyphData currentGlyph;
	bool found = false;
	uint16 widestGlyph = 0;
	for (auto current = s.begin(); current != s.end(); current++) {
		found = findGlyph(*current, currentGlyph);
		if (found) {
			widestGlyph = MAX(widestGlyph, currentGlyph._width);
		}
	}

	for (auto current = s.begin(); current != s.end(); current++) {
		found = findGlyph(*current, currentGlyph);
		if (!found) {
			sum += widestGlyph;
		} else {
			sum += currentGlyph._width + 1;
		}
	}
	return sum;
}

int Macs2Engine::measureStringsVertically(const Common::StringArray &sa) {
	// This is implemented around l0037_B318:
	return sa.size() * (maxGlyphHeight + 2);
}

int Macs2Engine::measureStrings(const Common::StringArray &sa) {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(measureString(*iter), max);
	}
	return max;
}

int Macs2Engine::computeStringIndex(Common::MemoryReadStream *stream, int targetOffset) {
	stream->seek(0);
	int index = 0;
	while (stream->pos() < targetOffset && !stream->eos()) {
		uint16 len = stream->readUint16LE();
		if (len == 0)
			break;
		stream->skip(len);
		index++;
	}
	return index;
}

void Macs2Engine::loadTranslation() {
	Common::SeekableReadStream *f = SearchMan.createReadStreamForMember("macs2_translation.dat");
	if (!f) {
		warning("Cannot open macs2_translation.dat");
		return;
	}

	// Read and verify header
	char magic[4];
	f->read(magic, 4);
	if (memcmp(magic, "MCS2", 4) != 0) {
		warning("Invalid macs2_translation.dat magic");
		delete f;
		return;
	}

	uint16 version = f->readUint16LE();
	if (version != 1) {
		warning("Unsupported macs2_translation.dat version %u", version);
		delete f;
		return;
	}

	uint16 numScenes = f->readUint16LE();
	uint16 numObjects = f->readUint16LE();

	// Read index tables
	struct IndexEntry {
		uint16 id;
		uint16 numStrings;
		uint32 dataOffset;
	};

	Common::Array<IndexEntry> sceneIndex(numScenes);
	for (uint16 i = 0; i < numScenes; i++) {
		sceneIndex[i].id = f->readUint16LE();
		sceneIndex[i].numStrings = f->readUint16LE();
		sceneIndex[i].dataOffset = f->readUint32LE();
	}

	Common::Array<IndexEntry> objectIndex(numObjects);
	for (uint16 i = 0; i < numObjects; i++) {
		objectIndex[i].id = f->readUint16LE();
		objectIndex[i].numStrings = f->readUint16LE();
		objectIndex[i].dataOffset = f->readUint32LE();
	}

	// Read string data for scenes
	for (uint16 i = 0; i < numScenes; i++) {
		f->seek(sceneIndex[i].dataOffset);
		TranslationEntry entry;
		for (uint16 j = 0; j < sceneIndex[i].numStrings; j++) {
			uint16 len = f->readUint16LE();
			Common::String s;
			for (uint16 k = 0; k < len; k++)
				s += (char)f->readByte();
			entry.strings.push_back(s);
		}
		_sceneTranslations[sceneIndex[i].id] = entry;
	}

	// Read string data for objects
	for (uint16 i = 0; i < numObjects; i++) {
		f->seek(objectIndex[i].dataOffset);
		TranslationEntry entry;
		for (uint16 j = 0; j < objectIndex[i].numStrings; j++) {
			uint16 len = f->readUint16LE();
			Common::String s;
			for (uint16 k = 0; k < len; k++)
				s += (char)f->readByte();
			entry.strings.push_back(s);
		}
		_objectTranslations[objectIndex[i].id] = entry;
	}

	delete f;
	debug("Loaded macs2_translation.dat: %u scenes, %u objects", numScenes, numObjects);
}

Common::StringArray Macs2Engine::decodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings, int sceneId, int objectId) {
	Common::StringArray result(numStrings);
	stream->seek(offset);

	byte x;
	byte y;
	byte r;

	for (int i = 0; i < numStrings; i++) {
		Common::String currentLine;
		uint16 length = stream->readUint16LE();
		byte currentByte;
		for (int index = 1; index < length + 1; index++) {
			currentByte = stream->readByte();
			x = (byte)(index * index * 0x0c);
			y = (byte)(currentByte ^ index);
			r = (byte)(x ^ y);
			currentLine += (char)r;
		}
		result[i] = currentLine;
	}

	// Apply translation if available
	if (getFeatures() & GF_TRANSLATED) {
		int baseIndex = computeStringIndex(stream, offset);
		const TranslationEntry *entry = nullptr;
		if (objectId != 0 && _objectTranslations.contains(objectId)) {
			entry = &_objectTranslations[objectId];
		} else if (sceneId != 0 && _sceneTranslations.contains(sceneId)) {
			entry = &_sceneTranslations[sceneId];
		}
		if (entry) {
			for (int i = 0; i < numStrings; i++) {
				int idx = baseIndex + i;
				if (idx >= 0 && idx < (int)entry->strings.size() && !entry->strings[idx].empty()) {
					result[i] = entry->strings[idx];
				}
			}
		}
	}

	return result;
}

uint32 Macs2Engine::getFeatures() const {
	return _gameDescription->flags;
}

bool Macs2Engine::loadAnimationFromSceneData(uint16 objectIndex, uint16 slotIndex, uint8 arrayIndex, bool shouldMirror, uint16 executingScriptObjectId) {
	GameObject *go = GameObjects::instance().getObjectByIndex(objectIndex);
	if (go == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}

	uint32 address = 0;
	if (executingScriptObjectId == 0) {
		if (arrayIndex == 0 || arrayIndex > _sceneResourceOffsets.size()) {
			_scriptExecutor->setScriptError(1);
			return false;
		}
		address = _sceneResourceOffsets[arrayIndex - 1];
	} else {
		GameObject *execObj = GameObjects::getObjectByIndex(executingScriptObjectId);
		if (execObj == nullptr || arrayIndex == 0 || arrayIndex > 32) {
			_scriptExecutor->setScriptError(1);
			return false;
		}
		address = execObj->_resourceOffsets[arrayIndex - 1];
	}
	if (address == 0) {
		_scriptExecutor->setScriptError(1);
		return false;
	}

	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	_fileStream->seek(address + 0x10);
	Common::Array<uint8> data;
	data.resize(size);
	_fileStream->read(data.data(), size);

	while (go->_blobs.size() < 0x15)
		go->_blobs.push_back(Common::Array<uint8>());
	while (go->_blobSourceKeys.size() < 0x15)
		go->_blobSourceKeys.push_back(0);
	while (go->_blobMirrorFlags.size() < 0x15)
		go->_blobMirrorFlags.push_back(false);

	Common::Array<uint8> *targetBlob = nullptr;
	if (slotIndex == 0x15) {
		while (go->_blobs.size() <= 20)
			go->_blobs.push_back(Common::Array<uint8>());
		targetBlob = &go->_blobs[20];
		go->_overloadAnimationSourceKey = static_cast<uint16>(address >> 16);
		go->_overloadAnimationMirrored = shouldMirror;
	} else {
		targetBlob = &go->_blobs[slotIndex - 1];
		// Binary scriptLoadObjectAnim (1008:cb45) does NOT modify slot+0x0C (wAnimSpeed).
		go->_blobSourceKeys[slotIndex - 1] = static_cast<uint16>(address >> 16);
		go->_blobMirrorFlags[slotIndex - 1] = shouldMirror;
	}

	if (targetBlob == nullptr) {
		_scriptExecutor->setScriptError(1);
		return false;
	}

	// Binary: memFree old blob if bSlotLoaded, then alloc + read; sets slot+0x33 = 1.
	*targetBlob = data;
	if (slotIndex == 0x15)
		go->_overloadAnimation = data;
	if (shouldMirror) {
		BackgroundAnimationBlob::mirrorAnimBlob(*targetBlob);
	}
	return true;
}

void Macs2Engine::sortObjectsByDepth(uint16 objectIndex) {
	if (objectIndex < 1 || objectIndex > 0x200)
		return;

	GameObject *obj = GameObjects::getObjectByIndex(objectIndex);
	if (obj == nullptr || obj->_dataOffset == 0)
		return;

	View1 *currentView = (View1 *)findView("View1");
	if (currentView != nullptr && currentView->_activeInventoryItem != nullptr &&
		currentView->_activeInventoryItem->_index == objectIndex) {
		currentView->_activeInventoryItem = nullptr;
		if (currentView->_savedCursorMode == Script::MouseMode::UseInventory) {
			currentView->_savedCursorMode = Script::MouseMode::Use;
		}
		if (_scriptExecutor->_cursorModeBeforeWait == Script::MouseMode::UseInventory) {
			_scriptExecutor->_cursorModeBeforeWait = Script::MouseMode::Use;
		}
		if (_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory) {
			setCursorMode(Script::MouseMode::Use);
			currentView->updateCursor();
		}
	}

	clearObjectRuntime(obj);
}

void Macs2Engine::clearObjectRuntime(GameObject *obj) {
	if (obj == nullptr)
		return;

	obj->_blobs.clear();
	obj->_blobSourceKeys.clear();
	obj->_blobWalkSpeeds.clear();
	obj->_blobMirrorFlags.clear();
	obj->_script.clear();
	memset(obj->_resourceOffsets, 0, sizeof(obj->_resourceOffsets));
	obj->_overloadAnimation.clear();
	obj->_useOverloadAnimation = false;
	obj->_overloadAnimationMirrored = false;
	obj->_storedWalkRuntime = GameObject::StoredWalkRuntime();
	obj->resetDrawBounds();
}

bool Macs2Engine::loadObjectData(GameObject *obj) {
	if (obj == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	if (obj->_dataOffset == 0) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	if (_fileStream == nullptr) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}
	if (!_fileStream->seek(obj->_dataOffset + 10, SEEK_SET)) {
		_scriptExecutor->setScriptError(0x19);
		return false;
	}

	const Common::Array<Common::Array<uint8>> blobsBackup = obj->_blobs;
	const Common::Array<uint16> keysBackup = obj->_blobSourceKeys;
	const Common::Array<uint16> speedsBackup = obj->_blobWalkSpeeds;
	const Common::Array<bool> mirrorsBackup = obj->_blobMirrorFlags;
	const auto rollbackPartialLoad = [&]() {
		obj->_blobs = blobsBackup;
		obj->_blobSourceKeys = keysBackup;
		obj->_blobWalkSpeeds = speedsBackup;
		obj->_blobMirrorFlags = mirrorsBackup;
	};

	for (int j = 0; j < 0x15; j++) {
		_fileStream->readUint16LE(); // animID (editor metadata, unused at runtime)
		uint16 blobSourceKey = _fileStream->readUint16LE();
		uint32 dataSize = _fileStream->readUint32LE();

		if (_fileStream->eos() && dataSize != 0) {
			rollbackPartialLoad();
			_scriptExecutor->setScriptError(1);
			return false;
		}

		Common::Array<uint8> data;
		if (dataSize > 0) {
			if (dataSize > 0x1000000) {
				rollbackPartialLoad();
				_scriptExecutor->setScriptError(1);
				return false;
			}
			data.resize(dataSize);
			const uint32 bytesRead = _fileStream->read(data.data(), dataSize);
			if (bytesRead != dataSize) {
				rollbackPartialLoad();
				_scriptExecutor->setScriptError(1);
				return false;
			}
		}

		if (j < (int)obj->_blobs.size()) {
			obj->_blobs[j] = data;
		} else {
			obj->_blobs.push_back(data);
		}
		if (j < (int)obj->_blobSourceKeys.size()) {
			obj->_blobSourceKeys[j] = blobSourceKey;
		} else {
			obj->_blobSourceKeys.push_back(blobSourceKey);
		}

		uint16 blobSpeed = _fileStream->readUint16LE();
		if (j < (int)obj->_blobWalkSpeeds.size()) {
			obj->_blobWalkSpeeds[j] = blobSpeed;
		} else {
			obj->_blobWalkSpeeds.push_back(blobSpeed);
		}

		uint16 blobMirrorFlag = _fileStream->readByte();
		_fileStream->readByte(); // discarded byte
		if (j < (int)obj->_blobMirrorFlags.size()) {
			obj->_blobMirrorFlags[j] = blobMirrorFlag != 0;
		} else {
			obj->_blobMirrorFlags.push_back(blobMirrorFlag != 0);
		}

		if (blobMirrorFlag != 0 && dataSize > 0 && j < (int)obj->_blobs.size()) {
			BackgroundAnimationBlob::mirrorAnimBlob(obj->_blobs[j]);
		}
	}

	// Binary loadObjectData (1008:08ec): flags, runtime reset, then script/resource table.
	_fileStream->readByte(); // runtime+0x184 hasInventoryIcon (derived from slot 0x13 in C++)
	obj->_hasShading = _fileStream->readByte() != 0;
	obj->_hasScaling = _fileStream->readByte() != 0;

	if (obj->_blobs.size() > 0x11 && !obj->_blobs[0x11].empty()) {
		const uint16 frameCount = BackgroundAnimationBlob::getAnimFrameCount(obj->_blobs[0x11]);
		obj->_pickupFrameStart = (frameCount >> 1) + 1;
		obj->_pickupFrameEnd = frameCount + 1;
	} else {
		obj->_pickupFrameStart = 1;
		obj->_pickupFrameEnd = 2;
	}

	obj->_overloadAnimTriggerDirection = 0x7FFF;
	obj->_useOverloadAnimation = false;
	obj->_overloadAnimation.clear();
	obj->_snapToTarget = false;
	obj->_hasBoundsAttachment = false;
	obj->_boundsAttachmentObjectID = 0;
	obj->_boundsAttachmentValue1 = 0;
	obj->_boundsAttachmentValue2 = 0;
	obj->_boundsAttachmentValue3 = 0;
	// Binary loadObjectData (1008:08ec): runtime+0x21D = object vertical offset.
	obj->_storedWalkRuntime.motionTargetVerticalOffset = obj->_verticalOffsetScale;

	const uint32 scriptTableOffset = 0x17F8 + (0xC + 0x04) + obj->_index * 0xC;
	_fileStream->seek(scriptTableOffset, SEEK_SET);
	const uint32 scriptOffset = _fileStream->readUint32LE();
	if (scriptOffset != 0) {
		_fileStream->seek(scriptOffset, SEEK_SET);
		for (int r = 0; r < 32; r++) {
			obj->_resourceOffsets[r] = _fileStream->readUint32LE();
		}
		const uint16 scriptLength = _fileStream->readUint16LE();
		obj->_script.resize(scriptLength);
		if (scriptLength > 0) {
			_fileStream->read(obj->_script.data(), scriptLength);
		}
	}
	return true;
}

void Macs2Engine::loadSongFromSceneData(uint8 dataIndex) {
	uint32 address = _sceneResourceOffsets[dataIndex - 1];
	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	Common::Array<uint8> data;
	data.resize(size);
	_fileStream->read(data.data(), size);
	_adlib->playSongData(data);
}

void Macs2Engine::setCurrentSoundData(const Common::Array<uint8> &data) {
	stopCurrentSound();
	_currentSoundData = data;
}

void Macs2Engine::clearCurrentSoundData() {
	stopCurrentSound();
	_currentSoundData.clear();
}

void Macs2Engine::playCurrentSound() {
	if (_currentSoundData.empty())
		return;

	stopCurrentSound();
	MacsAudioStream *audioStream = new MacsAudioStream();
	audioStream->_pos = 2; // Skip 2-byte header (original: size = stored_size - 2)
	audioStream->_data = _currentSoundData;
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_currentSoundHandle, audioStream);
}

void Macs2Engine::stopCurrentSound() {
	Audio::Mixer *mixer = g_system->getMixer();
	if (mixer->isSoundHandleActive(_currentSoundHandle))
		mixer->stopHandle(_currentSoundHandle);
}

bool Macs2Engine::isCurrentSoundPlaying() const {
	return g_system->getMixer()->isSoundHandleActive(_currentSoundHandle);
}

Common::String Macs2Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error Macs2Engine::run() {
	GameObjects::instance().init();
	readResourceFile();
	readExecutable();

	// Load translation data if available
	if (getFeatures() & GF_TRANSLATED) {
		loadTranslation();
	}

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	CursorMan.showMouse(false);

	// Initialize Adlib
	_adlib->init();
	syncSoundSettings();

	// Set the engine's debugger console
	setDebugger(new GUI::Debugger());

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	callbacks.init = onImGuiInit;
	callbacks.render = onImGuiRender;
	callbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	runGame();

	return Common::kNoError;
}

Common::Error Macs2Engine::loadGameState(int slot) {
	if (slot >= 100 && slot < 110) {
		// Load original DOS save file (SAVEGAME.N)
		int dosSlot = slot - 100;
		Common::String name = Common::String::format("SAVEGAME.%d", dosSlot);
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(name);
		if (!f) {
			name = Common::String::format("savegame.%d", dosSlot);
			f = g_system->getSavefileManager()->openForLoading(name);
		}
		if (!f)
			return Common::kReadingFailed;
		Common::Serializer s(f, nullptr);
		Common::Error err = syncGame(s);
		delete f;
		return err;
	}
	return Engine::loadGameState(slot);
}

Common::Error Macs2Engine::saveOriginalGameState(int dosSlot) {
	if (dosSlot < 0 || dosSlot > 9)
		return Common::kWritingFailed;

	// Original DOS filename, uppercase first (matches loadGameState lookup order).
	Common::String name = Common::String::format("SAVEGAME.%d", dosSlot);
	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(name, false /*no compression*/);
	if (!f)
		return Common::kWritingFailed;

	Common::Serializer s(nullptr, f);
	Common::Error err = syncGame(s);
	f->finalize();
	delete f;
	return err;
}

bool Macs2Engine::tick() {
	_scriptExecutor->tick();
	if (_runScheduled) {
		_runScheduled = false;
		bool shouldRunInit = _scheduledRunIsInitScene;
		_scheduledRunIsInitScene = false;
		_scriptExecutor->_isRepeatRun = true;
		_scriptExecutor->run(shouldRunInit);
	}
	return Events::tick();
}

void GlyphData::readFromeFile(Common::File &file) {
	_ascii = file.readByte();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void GlyphData::readFromMemory(Common::MemoryReadStream *stream) {
	_ascii = stream->readByte();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

void AnimFrame::readFromeFile(Common::File &file) {
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void AnimFrame::readFromStream(Common::MemoryReadStream *stream) {
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

bool AnimFrame::pixelHit(const Common::Point &point) const {
	if (point.x < 0 || point.x >= _width || point.y < 0 || point.y >= _height) {
		return false;
	}
	return _data[point.y * _width + point.x] != 0;
}

Common::Point AnimFrame::getBottomMiddleOffset(uint16 scale) const {
	if (scale == 100) {
		return Common::Point(_width / 2, _height);
	}
	return Common::Point(
		_width * scale / 200, // scaled width / 2
		_height * scale / 100);
}

AnimFrame BackgroundAnimationBlob::getCurrentFrame() {
	// Mode 0: read current frame without advancing (draw path uses mode 2 in drawBackgroundAnimations)
	uint16 offset = advanceAnimFrame(_blob, false, 0x0);
	// offset points to per-frame header: offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels
	offset += 6; // skip offsetX, offsetY, unknown
	AnimFrame result;
	result._width = READ_LE_UINT16(&_blob[offset]);
	result._height = READ_LE_UINT16(&_blob[offset + 2]);
	result._data.resize(result._width * result._height);
	memcpy(result._data.data(), &_blob[offset + 4], result._width * result._height);
	return result;
}

// Animation blob frame sequencer (matches advanceAnimFrame at 1010:1480).
// bpp6: save flag (true=write state back to blob header)
// bpp8: mode (0=current frame, 1=reset to frame 1, 2=advance, 100+N=jump to frame N)
//
// Blob header (12 bytes):
//   +0: unknown (preserved), +2: sequence position, +4: repeat counter,
//   +6: loop start position, +8: delay counter, +10: sequence length - 1
//
// Sequence table (at blob+0xC, seqLen bytes):
//   Command 1: set repeat (next byte), record loop start
//   Command 2: set delay (next byte)
//   Command 3: jump to position (next byte)
//   Values >= 10: frame index (0-based = value - 10)
//
// Frame data (at blob+0xC+seqLen): frame count word, then per frame:
//   offsetX(2), offsetY(2), unknown(2), width(2), height(2), pixels(w*h)
//
// Returns byte offset to the start of the target frame within the blob.
uint16 BackgroundAnimationBlob::advanceAnimFrame(Common::Array<uint8> &blob, bool bpp6, uint16 bpp8) {
	Common::MemorySeekableReadWriteStream stream(blob.data(), blob.size());

	uint16 bp22 = stream.readUint16LE();     // +0: unknown (preserved on save)
	uint16 bp6 = stream.readUint16LE();      // +2: current sequence position
	uint16 bp8 = stream.readUint16LE();      // +4: repeat counter
	uint16 bp0A = stream.readUint16LE();     // +6: loop start position
	uint16 bp10 = stream.readUint16LE();     // +8: delay counter
	uint16 bp0E = stream.readUint16LE() + 1; // +10: sequence length (stored as len-1)

	if (bpp8 == 0x1) {
		// Mode 1: reset to frame 1
		bp8 = 0;
		bp10 = 0;
		bp6 = 1;
	} else if (bpp8 >= 0x65 && bpp8 <= 0xA4) {
		// Mode 100+N: jump to frame N
		bp6 = bpp8 - 0x64;
		bp8 = 0;
		bp10 = 0;
		if (bp6 > bp0E) {
			bp6 = 1;
		}
	}

	if (bp6 >= bp0E) {
		bp6 = 1;
	}

	// Parse sequence commands until we hit a frame index (>= 10)
	uint8 bp0C;
	while (true) {
		if (bp6 >= bp0E) {
			bp6 = 1;
		}
		stream.seek(0x0B + bp6, SEEK_SET);
		bp0C = stream.readByte();
		if (bp0C == 0x01) {
			// Set repeat counter, record loop start
			bp6++;
			bp8 = stream.readByte();
			bp6++;
			bp0A = bp6;
		} else if (bp0C == 0x02) {
			// Set delay counter
			bp6++;
			bp10 = stream.readByte();
			bp6++;
		} else if (bp0C == 0x03) {
			// Jump to position
			bp6 = stream.readByte();
		} else {
			break;
		}
	}

	// Seek to frame data table (past sequence table)
	uint16 cx = bp0C - 0xA;
	stream.seek(0xB + bp0E, SEEK_SET);
	uint16 frameCount = stream.readUint16LE();
	if (cx > frameCount) {
		cx = 1;
	}

	// Skip to target frame (cx is 1-based frame number)
	for (; cx > 1; cx--) {
		stream.seek(0x6, SEEK_CUR); // skip offsetX, offsetY, unknown
		uint16 w = stream.readUint16LE();
		uint16 h = stream.readUint16LE();
		stream.seek(w * h, SEEK_CUR);
	}

	uint16 bp12 = stream.pos();

	// Mode 2: advance sequence position after finding current frame
	if (bpp8 == 0x02 && bp0C >= 0xA) {
		bp6++;
		if (bp10 > 0) {
			bp10--;
		}
		if (bp10 == 0 && bp8 > 0) {
			bp8--;
			bp6 = bp0A;
		}
	}

	if (bp6 >= bp0E) {
		bp6 = 1;
	}

	// Save state back to blob header
	if (bpp6) {
		stream.seek(0, SEEK_SET);
		stream.writeUint16LE(bp22);
		stream.writeUint16LE(bp6);
		stream.writeUint16LE(bp8);
		stream.writeUint16LE(bp0A);
		stream.writeUint16LE(bp10);
	}

	return bp12;
}

// Matches binary decodeAnimBlob (1010:184d) + mirrorAnimFrame (1010:1319).
// Iterates each frame in the blob and horizontally flips its pixel data in-place.
void BackgroundAnimationBlob::mirrorAnimBlob(Common::Array<uint8> &blob) {
	if (blob.size() < 14)
		return;
	AnimBlobView blobView(blob);
	if (!blobView.isValid())
		return;
	uint16 frameCount = blobView.frameCount();
	uint32 framePos = blobView.frameDataOffset() + 2; // skip frame count word
	for (uint16 f = 0; f < frameCount; f++) {
		if (framePos + 10 > blob.size())
			break;
		uint16 width = READ_LE_UINT16(&blob[framePos + 6]);
		uint16 height = READ_LE_UINT16(&blob[framePos + 8]);
		uint32 pixelStart = framePos + 10;
		if (pixelStart + (uint32)width * height > blob.size())
			break;
		// Flip each row horizontally
		for (uint16 row = 0; row < height; row++) {
			uint8 *rowStart = &blob[pixelStart + row * width];
			for (uint16 col = 0; col < width / 2; col++) {
				SWAP(rowStart[col], rowStart[width - 1 - col]);
			}
		}
		framePos += 10 + (uint32)width * height;
	}
}

// Returns sequence length from blob header (matches getAnimFrameCount at 1010:168c).
// Note: this returns the sequence table length, NOT the actual frame count.
// Use AnimBlobView::frameCount() for the actual number of pixel frames.
uint16 BackgroundAnimationBlob::getAnimFrameCount(Common::Array<uint8> &blob) {
	AnimBlobView view(blob);
	return view.sequenceLength();
}

int MacsAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int numSamplesRead = 0;
	for (int i = 0; i < numSamples; i++) {
		if (_pos >= _data.size()) {
			return numSamplesRead;
		}
		buffer[i] = static_cast<int16>((_data[_pos] - 128) * 256);
		numSamplesRead++;
		_pos++;
	}
	return numSamplesRead;
}

bool MacsAudioStream::isStereo() const {
	return false;
}

int MacsAudioStream::getRate() const {
	return 0x1F40;
}

bool MacsAudioStream::endOfData() const {
	return _pos >= _data.size();
}

bool MacsAudioStream::seek(const Audio::Timestamp &where) {
	const int64 targetPos = where.msecs() * getRate() / 1000;
	if (targetPos < 0 || targetPos > _data.size()) {
		return false;
	}

	_pos = targetPos;
	return true;
}

Audio::Timestamp MacsAudioStream::getLength() const {
	return Audio::Timestamp(0, _data.size(), getRate());
}

void Macs2Engine::applyPaletteDarkening() {
	// Binary: sceneData+0x5203 == 1 means copy source palette as-is to display;
	// otherwise darken: display[i] = source[i] * (100 - darkenPercent) / 100.
	// _palVanilla = raw 6-bit source palette (unchanged).
	// _pal = 8-bit display palette (darkened + expanded).
	uint16 darkenPercent = (_scenePaletteMode == 1) ? 0 : _paletteDarkenPercent;
	if (darkenPercent > 100)
		darkenPercent = 100;
	uint16 brightnessFactor = 100 - darkenPercent;
	for (int i = 0; i < 256 * 3; i++) {
		uint8 darkened = (_palVanilla[i] * brightnessFactor) / 100;
		_pal[i] = (darkened * 259 + 33) >> 6;
	}
}

void Macs2Engine::applyScenePaletteEffect() {
	// applyScenePaletteEffect (1000:103e): reduce scene colors to 16 rarest indices
	// (0..0xBF) plus fixed UI palette 0xC0..0xFF, then nearest-color remap.
	uint32 histogram[256] = {};
	for (int y = 0; y < _sceneBackground.h; y++) {
		for (int x = 0; x < _sceneBackground.w; x++) {
			histogram[_sceneBackground.getPixel(x, y)]++;
		}
	}

	bool selected[256] = {};
	for (int pick = 0; pick < 16; pick++) {
		uint32 minCount = 0xFFFFFFFF;
		int minIndex = 0;
		for (int i = 0; i <= 0xBF; i++) {
			if (histogram[i] != 0xFFFFFFFF && histogram[i] < minCount) {
				minCount = histogram[i];
				minIndex = i;
			}
		}
		histogram[minIndex] = 0xFFFFFFFF;
		selected[minIndex] = true;
	}

	byte refPalette[256 * 3];
	memset(refPalette, 0, sizeof(refPalette));
	int refSlot = 0x10;
	for (int i = 0; i <= 0xBF; i++) {
		if (selected[i]) {
			refPalette[refSlot * 3 + 0] = _palVanilla[i * 3 + 0];
			refPalette[refSlot * 3 + 1] = _palVanilla[i * 3 + 1];
			refPalette[refSlot * 3 + 2] = _palVanilla[i * 3 + 2];
			refSlot++;
		}
	}
	for (int i = 0xC0; i <= 0xFF; i++) {
		refPalette[i * 3 + 0] = _palVanilla[i * 3 + 0];
		refPalette[i * 3 + 1] = _palVanilla[i * 3 + 1];
		refPalette[i * 3 + 2] = _palVanilla[i * 3 + 2];
	}

	uint8 remap[256];
	for (int paletteIndex = 0; paletteIndex < 256; paletteIndex++) {
		const byte *srcRgb = &_palVanilla[paletteIndex * 3];
		uint32 bestDistance = 0x7FFF;
		uint8 bestIndex = 0x10;
		for (int candidate = 0x10; candidate <= 0xFF; candidate++) {
			const byte *candidateRgb = &refPalette[candidate * 3];
			uint32 distance = 0;
			for (int channel = 0; channel < 3; channel++) {
				int diff = (int)srcRgb[channel] - (int)candidateRgb[channel];
				if (diff < 0)
					diff = -diff;
				distance += (uint32)diff;
			}
			if (distance < bestDistance) {
				bestDistance = distance;
				bestIndex = (uint8)candidate;
			}
		}
		remap[paletteIndex] = bestIndex;
	}

	auto remapIndex = [&](uint8 index) -> uint8 {
		return remap[index];
	};

	for (int y = 0; y < _sceneBackground.h; y++) {
		for (int x = 0; x < _sceneBackground.w; x++) {
			_sceneBackground.setPixel(x, y, remapIndex(_sceneBackground.getPixel(x, y)));
		}
	}

	for (auto &blobEntry : _backgroundAnimationsBlobs) {
		Common::Array<uint8> &blob = blobEntry._blob;
		AnimBlobView blobView(blob);
		if (!blobView.isValid())
			continue;
		const uint16 frameCount = blobView.frameCount();
		for (uint16 frame = 0; frame < frameCount; frame++) {
			AnimBlobView::FrameInfo frameInfo;
			if (!blobView.getFrameInfo(frame, frameInfo))
				break;
			const uint32 pixelCount = (uint32)frameInfo.width * frameInfo.height;
			byte *pixels = const_cast<byte *>(frameInfo.pixels);
			for (uint32 p = 0; p < pixelCount; p++) {
				pixels[p] = remapIndex(pixels[p]);
			}
		}
	}

	byte remappedVanilla[256 * 3];
	for (int i = 0; i < 256; i++) {
		const int src = remap[i];
		remappedVanilla[i * 3 + 0] = refPalette[src * 3 + 0];
		remappedVanilla[i * 3 + 1] = refPalette[src * 3 + 1];
		remappedVanilla[i * 3 + 2] = refPalette[src * 3 + 2];
	}
	memcpy(_palVanilla, remappedVanilla, 256 * 3);
	applyPaletteDarkening();

	View1 *view = (View1 *)findView("View1");
	if (view != nullptr) {
		view->_backgroundSurface.copyFrom(_sceneBackground);
		view->_paletteDirty = true;
	}
}

// Gradual palette brighten for _scenePaletteMode == 2, matching the binary
// updateBackgroundAnimations (1008:2c05).
//
// The binary decrements sceneData+0x5205 (the darken percent) by 1 each call
// while it is above 60 (0x3C), then recomputes the display palette via the
// standard darkening formula and pushes it. The pushed range depends on the
// map/help-disabled flag (1020:23B4):
//   flag != 0 (map active): update palette entries 16..255 only, preserving
//             the UI palette entries 0..15 used by the map/help overlay
//             (setPaletteRange(0xF0, 0x10, dest+0x30)).
//   flag == 0:              update all 256 entries (setPaletteRange(0x100, 0, dest)).
//
// Only mode 2 acts here. Mode 3 calls this in the binary too, but its body is
// a no-op for mode 3 (its darkening is static), so we guard on mode 2.
void Macs2Engine::updateBackgroundAnimationPalette() {
	if (_scenePaletteMode != 2)
		return;

	if (_paletteDarkenPercent > 60)
		_paletteDarkenPercent--;

	// Recompute the 8-bit display palette from the vanilla source.
	applyPaletteDarkening();

	View1 *view = (View1 *)findView("View1");
	const bool mapActive = view && view->isHelpButtonDisabled();

	if (mapActive) {
		// Preserve entries 0..15 (UI), update 16..255.
		g_system->getPaletteManager()->setPalette(_pal + 16 * 3, 16, 240);
	} else {
		g_system->getPaletteManager()->setPalette(_pal, 0, 256);
	}
}

} // End of namespace Macs2
