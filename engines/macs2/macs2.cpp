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
#include "adlib.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/util.h"
#include "gameobjects.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "macs2/console.h"
#include "macs2/debugtools.h"
#include "macs2/detection.h"
#include "view1.h"

namespace Macs2 {

Macs2Engine *g_engine;

Graphics::ManagedSurface Macs2Engine::readRLEImage(int64 offs, Common::MemoryReadStream *stream) {
	// TODO: Should we pass the stream as pointer or reference?
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

	return result;
}

int previewNumFrames(int64 offs, Common::File &file) {
	file.seek(offs);
	int64 numBytes = file.readUint32LE();
	// byte* data = new byte[numBytes];
	// file.read(data, numBytes);
	// TODO: Extract the frames
	// Skip ahead to the start of the first frame
	file.seek(20, SEEK_CUR);
	numBytes -= 20;
	int numFrames = 0;
	while (numBytes > 0) {
		file.seek(6, SEEK_CUR);
		numBytes -= 6;
		uint16 x = file.readUint16LE();
		uint16 y = file.readUint16LE();
		file.seek(x * y, SEEK_CUR);
		numBytes -= 4 + 4 + x * y;
		numFrames++;
	}
	file.seek(offs);
	return numFrames;
}

void Macs2Engine::readResourceFile() {
	{
		// Extra scope in order to make sure no code tries to read from the file directly.
		Common::File file;
		if (!file.open("RESOURCE.MCS"))
			error("readResourceFile(): Error reading MCS file");

		int64 size = file.size();
		byte *fileData = new byte[size];
		file.read(fileData, size);

		_fileStream = new Common::MemoryReadStream(fileData, size);
	}

	// Full implementation here

	// Read the image resources (33 cursor/icon entries)
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

	_fileStream->seek(0xC, SEEK_SET);
	Scenes::instance()._currentActorIndex = _fileStream->readUint16LE();
	uint16 firstSceneIndex = _fileStream->readUint16LE();
	Scenes::instance()._currentSceneIndex = firstSceneIndex;
	Scenes::instance()._currentSceneScript = Scenes::instance().readSceneScript(firstSceneIndex, _fileStream);
	Scenes::instance()._currentSceneStrings = Scenes::instance().readSceneStrings(firstSceneIndex, _fileStream);
	Scenes::instance()._currentSceneSpecialAnimOffsets = Scenes::instance().readSpecialAnimsOffsets(firstSceneIndex, _fileStream);
	_scriptExecutor->setScript(Scenes::instance()._currentSceneScript);

	// Load object data (512 entries max, matching original loadResourceFile)
	for (int i = 1; i < 0x200; i++) {
		// The formula for the seek lives at l0037_0936
		// The global [0752h] is loaded with 3000h bytes read from offset Ch + 4h in the file
		// Regarding the 4h offset: Before the 3000h bytes, we have the values of the two globals 0776 and 077C
		uint32 addressOffset = 0x17F4 + (0xC + 0x04) + i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);
		uint32 objectOffset = _fileStream->readUint32LE();
		if (objectOffset == 0) {
			break;
		}

		_fileStream->seek(objectOffset, SEEK_SET);
		GameObject *gameObject = new GameObject();
		gameObject->_index = i;
		gameObject->_dataOffset = objectOffset;

		// This loading happens around the l0037_082D: mark
		uint16 x = _fileStream->readUint16LE();
		uint16 y = _fileStream->readUint16LE();
		uint16 sceneIndex = _fileStream->readUint16LE();
		uint16 orientation = _fileStream->readUint16LE();
		uint16 unknown = _fileStream->readUint16LE();
		gameObject->Position = Common::Point(x, y);
		gameObject->SceneIndex = sceneIndex;
		gameObject->Orientation = orientation;
		gameObject->Unknown = unknown;

		for (int j = 1; j < 0x15; j++) {
			// Per-slot data in file: 2 bytes unknown1, 2 bytes sourceKey, 4 bytes dataSize, data, 2 bytes speed, 1 byte mirrorFlag, 1 byte (discarded)
			_fileStream->readUint16LE(); // unknown1
			uint16 unknown2 = _fileStream->readUint16LE();
			uint32 dataSize = _fileStream->readUint32LE();
			uint8 *data = new uint8[dataSize];
			_fileStream->read(data, dataSize);
			gameObject->Blobs.push_back(Common::Array<uint8>(data, dataSize));
			gameObject->BlobSourceKeys.push_back(unknown2);
			// Per-animation walk speed (+0x30 in runtime)
			uint16 blobSpeed = _fileStream->readUint16LE();
			gameObject->BlobSpeeds.push_back(blobSpeed);
			// Mirror flag (+0x32 in runtime)
			uint16 unknown5 = _fileStream->readByte();
			_fileStream->readByte(); // slot loaded flag (runtime-only, discarded from file)
			gameObject->BlobMirrorFlags.push_back(unknown5 != 0);

			// In order to get to l0037_0BBA: where the blob will be mirrored,
			// the bytes at +Eh and +Fh must be != 0
			// +Fh is set related to the inner loop - I think it means that
			// the blob is empty
			// +Eh is read here
			if (unknown5 != 0) {
				debug("Object %.4x need to mirror blob %4.x", i, j);
				if (dataSize > 0) {
					BackgroundAnimationBlob::mirrorAnimBlob(gameObject->Blobs.back());
				}
			}

			// Seek forward for the next 2+1+1 bytes reads
			// _fileStream->seek(0x4, SEEK_CUR);
		}
		// Read the object script (resource offset table + script bytecode)
		// Binary: scene table at +0x17F8 holds the script data file offset for each object
		addressOffset = 0x17F8 + (0xC + 0x04) + i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);

		objectOffset = _fileStream->readUint32LE();
		// Binary loadSceneObjects checks both +0x17F4 and +0x17F6 (high word); zero means no data
		if (objectOffset == 0) {
			break;
		}
		_fileStream->seek(objectOffset, SEEK_SET);
		// Resource offset table at +0x18D equivalent in file (128 bytes = 32 dword offsets).
		// Binary loadSceneObjects reads this into runtime+0x18D.
		for (int r = 0; r < 32; r++) {
			gameObject->_resourceOffsets[r] = _fileStream->readUint32LE();
		}
		uint16 scriptLength = _fileStream->readUint16LE();
		gameObject->Script.resize(scriptLength);
		_fileStream->read(gameObject->Script.data(), scriptLength);

		GameObjects::instance()._objects.push_back(gameObject);
	}


}

void Macs2Engine::readExecutable() {
	Common::MemoryReadStream *exeFileStream;
	// TODO: Memory management
	{
		// Extra scope in order to make sure no code tries to read from the file directly.
		Common::File file;
		if (!file.open("MCSEXEC.EXE"))
			error("readExecutable(): Error reading executable file");

		int64 size = file.size();
		byte *fileData = new byte[size];
		file.read(fileData, size);

		exeFileStream = new Common::MemoryReadStream(fileData, size);
	}

	_adlib->ReadDataFromExecutable(exeFileStream);

	exeFileStream->seek(0x0001B610, SEEK_SET);
	inventoryIconIndices.resize(6);
	exeFileStream->read(inventoryIconIndices.data(), 12);
}

void Macs2Engine::readBackgroundAnimations(Common::MemoryReadStream *stream) {
	// Offset 50F5 in scene data
	// TODO: Remove the non-blob implementation
	_numBackgroundAnimations = stream->readUint16LE();

	_backgroundAnimations = new BackgroundAnimation[_numBackgroundAnimations];
	_backgroundAnimationsBlobs.resize(_numBackgroundAnimations);

	for (int i = 0; i < _numBackgroundAnimations; i++) {
		BackgroundAnimationBlob &currentBlob = _backgroundAnimationsBlobs[i];

		BackgroundAnimation &current = _backgroundAnimations[i];
		// Local offset +0h
		current._x = stream->readUint16LE();
		currentBlob._x = current._x;
		// Local offset +2n
		current._y = stream->readUint16LE();
		currentBlob._y = current._y;

		// current.numFrames = previewNumFrames(file.pos(), file);
		uint32 numBytes = stream->readUint32LE();

		currentBlob._blob.resize(numBytes);
		int64 pos = stream->pos();
		stream->read(currentBlob._blob.data(), numBytes);
		stream->seek(pos, SEEK_SET);

		// Skip to the intermediary data
		// Game loading code puts this at a pointer stored in local offset +8h
		stream->seek(10, SEEK_CUR);
		uint16 nextNumBytes = stream->readUint16LE();
		stream->seek(nextNumBytes, SEEK_CUR);
		current._numFrames = stream->readUint16LE();
		current._frameIndex = 0;
		current._frames = new AnimFrame[current._numFrames];
		for (int j = 0; j < current._numFrames; j++) {
			// Skip to width and height
			stream->seek(6, SEEK_CUR);
			current._frames[j].readFromStream(stream);
		}
		// TODO: This allows us to skip over a faulty implementation, but
		// probably missing some valid data or loading wrong data
		// file.seek(endPos);
		// TODO: Figure out the trailing values?
		// Data at offset +Ch
		stream->readUint16LE(); // unknown1
		// Data at offset +Fh
		stream->readByte(); // unknown2
		// Local variable [bp-5h]
		stream->readByte(); // unknown3

		// Initialize the blob
		// TODO: There is a lot more going on in the function that does this. It is around
		// -- Caller (1): 01e7:7a5b
		// --Caller(2) : 01e7 : 8820
		Macs2::BackgroundAnimationBlob::advanceAnimFrame(currentBlob._blob, true, 0x64 + current._numFrames);
	}
}

void Macs2Engine::readImageResources(Common::MemoryReadStream *stream) {
	// l0037_3355:
	for (int i = 0; i < 0x21; i++) {
		uint32 length = stream->readUint32LE();
		if (length == 0) {
			continue;
		}
		AnimFrame frame;
		// Move forward to skip the first word
		stream->seek(0x2, SEEK_CUR);
		frame.readFromStream(stream);
		_imageResources.push_back(frame);
		debug("W: %u, H: %u", frame._width, frame._height);
	}
}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			 _gameDescription(gameDesc), _randomSource("Macs2") {
	g_engine = this;
	_scriptExecutor = new Script::ScriptExecutor();
	_scriptExecutor->_engine = this;
	_adlib = new Adlib();

	// We have a fixed 0x10 number of entries
	_hotspotOverrides.resize(0x11);
	for (uint i = 0; i < _hotspotOverrides.size(); i++) {
		_hotspotOverrides[i] = 0xFFFF;
	}
	_pathfindingValueRemaps.resize(0x100);
	for (uint i = 0; i < _pathfindingValueRemaps.size(); i++) {
		_pathfindingValueRemaps[i] = 0;
	}
}

Macs2Engine::~Macs2Engine() {
	stopInputRecording();
	clearCurrentSoundData();
	_adlib->Deinit();
}

void Macs2Engine::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) const {
#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan && ConfMan.getBool("tts_enabled")) {
		ttsMan->say(text, action);
	}
#endif
}

void Macs2Engine::changeScene(uint32 newSceneIndex, bool executeScript) {
	// Release old scene resources
	if (_backgroundAnimations != nullptr) {
		for (int i = 0; i < _numBackgroundAnimations; i++) {
			delete[] _backgroundAnimations[i]._frames;
		}
		delete[] _backgroundAnimations;
		_backgroundAnimations = nullptr;
	}
	_backgroundAnimationsBlobs.clear();
	_numBackgroundAnimations = 0;
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
	// _bgImageShip.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	uint8 *data = new uint8[0x320];

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
				_bgImageShip.setPixel(x, y, value);
				remainingPixels--;
				x++;
			} else {
				// We need to decode the RLE data
				const uint8 &runlength = dataPointer[0];
				dataPointer++;
				const uint8 &encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength && x < 320; i++) {
					_bgImageShip.setPixel(x++, y, encodedValue);
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

	// Continuing with data, even if we don't know all uses yet
	// Common::Array<uint8> unknownData1;
	// unknownData1.resize(0x100);
	_fileStream->read(_shadingTable, 0x100);

	_fileStream->readByte(); // unknownByte1
	_fileStream->readByte(); // unknownByte2
	_fileStream->readByte(); // unknownByte3

	// Offset 1013h
	Graphics::ManagedSurface unknownRLE1 = readRLEImage(_fileStream->pos(), _fileStream);
	// Confirmed: depth map at scene offset 0x1013
	_depthMap.blitFrom(unknownRLE1);

	// Offset 2017h
	Graphics::ManagedSurface unknownRLE2 = readRLEImage(_fileStream->pos(), _fileStream);
	// Walkability/pathfinding map at scene offset 0x2017
	_pathfindingMap.blitFrom(unknownRLE2);

	// Offset 301Bh
	Graphics::ManagedSurface unknownRLE3 = readRLEImage(_fileStream->pos(), _fileStream);

	// Offset 401Fh
	// This is the first map used in 0037:10C4 for the lookup of interacted hotspots
	Graphics::ManagedSurface bgMap = readRLEImage(_fileStream->pos(), _fileStream);
	_map.copyFrom(bgMap);

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
	_hotspotColorTable.resize(0x20 / 2);
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

	_bgAnimMode = _fileStream->readUint16LE();
	_bgAnimParam = _fileStream->readUint16LE();

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

	// Refresh characters
	View1 *currentView = (View1 *)findView("View1");

	// Refresh the surface
	currentView->_backgroundSurface.copyFrom(_bgImageShip);
	currentView->_paletteDirty = true;
	currentView->clearStringBox(false);
	currentView->_drawnStringBox.clear();
	currentView->_continueScriptAfterUI = false;
	currentView->currentSpeechActData = SpeechActData();
	currentView->_isShowingInventory = false;
	currentView->_activeInventoryItem = nullptr;
	currentView->_isShowingMainMenu = false;
	currentView->clearOverlayTextEntries();
	_scriptExecutor->_inventoryActionFlag = false;
	_scriptExecutor->_inventoryCombineFlag = false;

	// Stop all characters from sending leftover events
	for (auto currentCharacter : currentView->_characters) {
		currentCharacter->_executeScriptOnFinishLerp = false;
	}
	currentView->_characters.clear();
	for (auto currentObject : GameObjects::instance()._objects) {
		if (currentObject->SceneIndex == newSceneIndex) {
			Character *c = new Character();
			c->_gameObject = currentObject;
			currentView->_characters.push_back(c);
		}
	}

	// Load the script and execute it
	Scenes::instance()._lastSceneIndex = Scenes::instance()._currentSceneIndex;
	Scenes::instance()._currentSceneIndex = newSceneIndex;
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
		// Start the execution
		_scriptExecutor->run(true);
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
// Bounds: x<0 || x>=320 || y<0 || y>=200 → return 0
// Lookup: scene[y*4 + 0x2017] → row pointer, then byte at [rowPtr + x]
// Values 0xC8..0xEF: override range - checks scene[value*5 + 0x4EA5]:
//   If override disabled (flag==0): returns 0xFF
//   If override enabled (flag!=0): returns scene[value*5 + 0x4EA6]
uint16 Macs2Engine::getWalkabilityAt(int16 y, int16 x) {
	if (x < 0 || x >= 320 || y < 0 || y >= 200) {
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

uint8 Macs2Engine::getPathfindingOverride2(uint16 index) {
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
			if (adj == prevNode) continue;

			// Check visited stack
			bool alreadyVisited = false;
			for (int j = 1; j < visitedCount; j++) {
				if (visitedStack[j] == adj) {
					alreadyVisited = true;
					break;
				}
			}
			if (alreadyVisited) continue;

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

void Macs2Engine::nextCursorMode() {
	// Cycle through the 4 main cursor modes from the action bar:
	// Talk(0x13) -> Look(0x14) -> Use(0x15) -> Walk(0x16) -> Talk
	switch (_scriptExecutor->_mouseMode) {
	case Script::MouseMode::Talk:
		_scriptExecutor->_mouseMode = Script::MouseMode::Look;
		break;
	case Script::MouseMode::Look:
		_scriptExecutor->_mouseMode = Script::MouseMode::Use;
		break;
	case Script::MouseMode::Use:
		_scriptExecutor->_mouseMode = Script::MouseMode::Walk;
		break;
	default:
		_scriptExecutor->_mouseMode = Script::MouseMode::Talk;
		break;
	}
}

void Macs2Engine::setCursorMode(Script::MouseMode newMode) {
	_scriptExecutor->_mouseMode = newMode;
}

void Macs2Engine::dumpStream(Common::MemoryReadStream *s, uint16 len) {
	int64 start_pos = s->pos();
	Common::String result;
	for (int i = 0; i < len; i++) {
		uint8 v = s->readByte();
		result += Common::String::format("%.2X", v);
	}
	debug(result.c_str());
	s->seek(start_pos, SEEK_SET);
}

uint16 Macs2Engine::getHotspotAtPoint(const Common::Point &p) {
	uint16 result = 0;
	// TODO: Abstract the screen sizes
	if (p.x < 0 || p.x > 320 || p.y < 0 || p.y > 200) {
		return result;
	}

	uint8 firstLookup = _map.getPixel(p.x, p.y);
	uint16 numHotspots = _numHotspots;

	uint8 i = 1;
	if (i > numHotspots) {
		return result;
	}

	Common::Array<uint16> a = _hotspotColorTable;

	do {
		uint16 lookup = a[i - 1];
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
	// Bounds check: returns 0 if out of screen (verified: getWalkabilityAt at 1008:0e8c)
	if (p.x < 0 || p.x >= 320 || p.y < 0 || p.y >= 200) {
		return 0;
	}
	uint8 value = _pathfindingMap.getPixel(p.x, p.y);
	if (value < 0xC8 || value > 0xEF) {
		return value;
	}

	if (value >= _pathfindingValueRemaps.size()) {
		return 0xFF;
	}

	const uint16 remappedValue = _pathfindingValueRemaps[value];
	return remappedValue != 0 ? remappedValue : 0xFF;
}

int Macs2Engine::measureString(Common::String &s) {
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

int Macs2Engine::measureStringsVertically(Common::StringArray sa) {
	// This is implemented around l0037_B318:
	return sa.size() * (maxGlyphHeight + 2);
}

int Macs2Engine::measureStrings(Common::StringArray sa) {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(measureString(*iter), max);
	}
	return max;
}

Common::StringArray Macs2Engine::decodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings) {
	Common::StringArray result(numStrings);
	dumpStream(stream, 64);
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

	return result;
}

uint32 Macs2Engine::getFeatures() const {
	return _gameDescription->flags;
}

void Macs2Engine::loadAnimationFromSceneData(uint16 objectIndex, uint16 slotIndex, uint8 arrayIndex, bool decodeBlob) {
	if (arrayIndex == 0 || arrayIndex > _sceneResourceOffsets.size()) {
		warning("Invalid animation array index %u for object %u slot %u", arrayIndex, objectIndex, slotIndex);
		return;
	}

	GameObject *go = GameObjects::instance().getObjectByIndex(objectIndex);
	if (go == nullptr) {
		warning("Tried to load animation for invalid object %u", objectIndex);
		return;
	}

	if (slotIndex == 0 || slotIndex > 0x15) {
		warning("Invalid animation slot %u for object %u", slotIndex, objectIndex);
		return;
	}

	// We need to account for the game starting indices at 1
	uint32 address = _sceneResourceOffsets[arrayIndex - 1];
	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	_fileStream->seek(address + 0x4 + 0xC);
	Common::Array<uint8> data;
	data.resize(size);
	_fileStream->read(data.data(), size);
	Common::Array<uint8> *targetBlob = nullptr;
	if (slotIndex == 0x15) {
		targetBlob = &go->overloadAnimation;
		go->overloadAnimationSourceKey = static_cast<uint16>(address >> 16);
		go->overloadAnimationMirrored = false;
	} else if ((uint)(slotIndex - 1) < go->Blobs.size()) {
		targetBlob = &go->Blobs[slotIndex - 1];
		if ((uint)(slotIndex - 1) < go->BlobSourceKeys.size())
			go->BlobSourceKeys[slotIndex - 1] = static_cast<uint16>(address >> 16);
		if ((uint)(slotIndex - 1) < go->BlobMirrorFlags.size())
			go->BlobMirrorFlags[slotIndex - 1] = false;
	}

	if (targetBlob == nullptr) {
		warning("Object %u has no storage for animation slot %u", objectIndex, slotIndex);
		return;
	}

	*targetBlob = data;
	if (decodeBlob) {
		BackgroundAnimationBlob::advanceAnimFrame(*targetBlob, true, 2);
	}
}

void Macs2Engine::loadSongFromSceneData(uint8 dataIndex) {

	uint32 address = _sceneResourceOffsets[dataIndex - 1];
	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	Common::Array<uint8> data;
	data.resize(size);
	_fileStream->read(data.data(), size);
	_adlib->PlaySongData(data);
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
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_currentSoundHandle, audioStream);
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

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	// Initialize Adlib
	_adlib->Init();

	// Set the engine's debugger console
	setDebugger(new Console());

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
	int64 stride = file.pos();
	_ascii = file.readByte();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data = new byte[_width * _height];
	file.read(_data, _width * _height);
	stride = file.pos() - stride;
}

void GlyphData::readFromMemory(Common::MemoryReadStream *stream) {
	int64 stride = stream->pos();
	_ascii = stream->readByte();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data = new byte[_width * _height];
	stream->read(_data, _width * _height);
	stride = stream->pos() - stride;
}

void AnimFrame::readFromeFile(Common::File &file) {
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data = new byte[_width * _height];
	file.read(_data, _width * _height);
}

void AnimFrame::readFromStream(Common::MemoryReadStream *stream) {
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data = new byte[_width * _height];
	stream->read(_data, _width * _height);
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

Sprite AnimFrame::asSprite() {
	// TODO: Shows that the separation makes little sense
	Sprite result;
	result._data.resize(_width * _height);
	result._data.assign(_data, _data + _width * _height);
	result._width = _width;
	result._height = _height;
	return result;
}

AnimFrame BackgroundAnimationBlob::getFrame(uint32 index) {
	AnimationReader animReader(_blob);
	uint16 numAnimations = animReader.readNumAnimations();
	debug("Number of animation frames for background object: %.4x", numAnimations);

	// TODO: Check consistency between 0 and 1 based indexing
	animReader.seekToAnimation((index - 1) % numAnimations);
	// testReader.SeekToAnimation(0);
	// Skip ahead to the width and height
	animReader._readStream->seek(6, SEEK_CUR);

	AnimFrame result;
	result.readFromStream(animReader._readStream);
	return result;
}

AnimFrame BackgroundAnimationBlob::getCurrentFrame() {
	// Mode 0: get current frame without advancing (advancement happens in View1::tick)
	uint16 offset = advanceAnimFrame(_blob, false, 0x0);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(_blob.data(), _blob.size());
	offset += 6;
	stream->seek(offset);
	AnimFrame result;
	result.readFromStream(stream);
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

	uint16 bp22 = stream.readUint16LE(); // +0: unknown (preserved on save)
	uint16 bp6 = stream.readUint16LE();  // +2: current sequence position
	uint16 bp8 = stream.readUint16LE();  // +4: repeat counter
	uint16 bp0A = stream.readUint16LE(); // +6: loop start position
	uint16 bp10 = stream.readUint16LE(); // +8: delay counter
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
	Common::MemoryReadStream header(blob.data(), blob.size());
	header.seek(0xA);
	uint16 headerSize = header.readUint16LE();
	uint32 countOffset = 0xC + headerSize;
	if (countOffset + 2 > blob.size())
		return;
	uint16 frameCount = READ_LE_UINT16(&blob[countOffset]);
	// First frame starts at headerSize + 0xE from blob start
	uint32 framePos = headerSize + 0xE;
	for (uint16 f = 0; f < frameCount; f++) {
		if (framePos + 10 > blob.size())
			break;
		// Frame layout: +0(2) +2(2) +4(2unknown) +6(width,2) +8(height,2) +10(pixels)
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
// Reads word at blob+10 and adds 1.
uint16 BackgroundAnimationBlob::getAnimFrameCount(Common::Array<uint8> &blob) {
	Common::MemoryReadStream stream(blob.data(), blob.size());
	stream.seek(0xA);
	uint16 count = stream.readUint16LE();
	return count + 1;
}

int MacsAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int numSamplesRead = 0;
	for (int i = 0; i < numSamples; i++) {
		if (_pos >= _data.size()) {
			return numSamplesRead;
		}
		buffer[i] = static_cast<int16>((static_cast<int>(_data[_pos]) - 128) << 8);
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

} // End of namespace Macs2

