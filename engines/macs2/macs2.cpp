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
	ReadImageResources(_fileStream);
	// Font 1 follows immediately after the 33 image resource entries.
	// Original: 4-byte size field (skipped) + 2-byte glyph count + glyph data.
	uint32 font1SizeField = _fileStream->readUint32LE(); // skip size field
	(void)font1SizeField;
	uint16 font1GlyphCount = _fileStream->readUint16LE();
	maxGlyphHeight = 0;
	for (uint i = 0; i < font1GlyphCount; i++) {
		_glyphs[i].ReadFromMemory(_fileStream);
		maxGlyphHeight = MAX(_glyphs[i].Height, maxGlyphHeight);
	}
	numGlyphs = font1GlyphCount;

	_fileStream->seek(0xC, SEEK_SET);
	Scenes::instance().CurrentActorIndex = _fileStream->readUint16LE();
	uint16 firstSceneIndex = _fileStream->readUint16LE();
	Scenes::instance().CurrentSceneIndex = firstSceneIndex;
	Scenes::instance().CurrentSceneScript = Scenes::instance().ReadSceneScript(firstSceneIndex, _fileStream);
	Scenes::instance().CurrentSceneStrings = Scenes::instance().ReadSceneStrings(firstSceneIndex, _fileStream);
	Scenes::instance().CurrentSceneSpecialAnimOffsets = Scenes::instance().ReadSpecialAnimsOffsets(firstSceneIndex, _fileStream);
	_scriptExecutor->SetScript(Scenes::instance().CurrentSceneScript);

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
		gameObject->Index = i;
		gameObject->DataOffset = objectOffset;

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
			// We're at l0037_0A3E here
			// TODO: Compare places and read values with the game
			_fileStream->readUint16LE(); // unknown1 // _fileStream->readUint16LE();
			uint16 unknown2 = _fileStream->readUint16LE();
			uint32 dataSize = _fileStream->readUint32LE();
			uint8 *data = new uint8[dataSize];
			_fileStream->read(data, dataSize);
			// TODO: Place this data in the game object and create the game object
			gameObject->Blobs.push_back(Common::Array<uint8>(data, dataSize));
			gameObject->BlobSourceKeys.push_back(unknown2);
			// Load three values here
			// l0037_0B62:
			// Data at offset +Ch - per-animation walk speed
			uint16 blobSpeed = _fileStream->readUint16LE();
			gameObject->BlobSpeeds.push_back(blobSpeed);
			// Data at offset +Eh
			uint16 unknown5 = _fileStream->readByte();
			// Local variable [bp-5h]
			_fileStream->readByte(); // unknown6
			gameObject->BlobMirrorFlags.push_back(unknown5 != 0);

			// In order to get to l0037_0BBA: where the blob will be mirrored,
			// the bytes at +Eh and +Fh must be != 0
			// +Fh is set related to the inner loop - I think it means that
			// the blob is empty
			// +Eh is read here
			if (unknown5 != 0) {
				debug("Object %.4x need to mirror blob %4.x", i, j);
			}

			// Seek forward for the next 2+1+1 bytes reads
			// _fileStream->seek(0x4, SEEK_CUR);
		}
		// Read the object script
		// The offset is calculated at l0037_0C9D - also see above for adjustmnets
		addressOffset = 0x17F8 + (0xC + 0x04) + i * 0xC;
		_fileStream->seek(addressOffset, SEEK_SET);

		objectOffset = _fileStream->readUint32LE();
		// TODO: Not sure if this can happen or should be checked
		if (objectOffset == 0) {
			break;
		}
		_fileStream->seek(objectOffset, SEEK_SET);
		// TODO: We read 80h bytes - to check where these are used - script variables?
		_fileStream->seek(0x80, SEEK_CUR);
		uint16 scriptLength = _fileStream->readUint16LE();
		gameObject->Script.resize(scriptLength);
		_fileStream->read(gameObject->Script.data(), scriptLength);

		GameObjects::instance().Objects.push_back(gameObject);
	}

	// Test implementations below

	_fileStream->seek(0x23BE09);

	_bgImageShip.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	uint8 *lengthData = new uint8[2];
	uint8 *data = new uint8[320];

	// TODO: Consider if it can be that the data is more than this. Maybe the tooling of the engine can make bad calls and
	// try to RLE something which would be better not RLE encoded.

	for (int y = 0; y < 200; y++) {
		// TODO: Use the proper read function, it seems to be available
		_fileStream->read(lengthData, 2);
		uint16 length = lengthData[1] << 8 | lengthData[0];
		_fileStream->read(data, length);
		uint16 remainingPixels = 320;
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
				for (int i = 0; i < runlength; i++) {
					_bgImageShip.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	// Load the palette
	_fileStream->seek(0x00248BCB);
	_fileStream->read(_pal, 256 * 3);
	// Make a copy that will not be color corrected, for fading
	memcpy(_palVanilla, _pal, 256 * 3);

	// Adjust the palette
	for (int i = 0; i < 256 * 3; i++) {
		_pal[i] = (_pal[i] * 259 + 33) >> 6;
	}

	// Load the pathfinding points (16 entries at scene+0x5023, 10 bytes each = 0xA0 bytes)
	_fileStream->seek(0x0024BF72);

	for (int i = 0; i < 16; i++) {
		PathfindingPoint current;
		current.Position.x = _fileStream->readUint16LE();
		current.Position.y = _fileStream->readUint16LE();
		_fileStream->seek(4, SEEK_CUR);
		uint16 numConnections = _fileStream->readUint16LE();
		_fileStream->seek(-6, SEEK_CUR);
		for (int j = 0; j < numConnections; j++) {
			current.adjacentPoints.push_back(_fileStream->readByte());
		}
		_fileStream->seek(4 - numConnections + 2, SEEK_CUR);
		pathfindingPoints.push_back(current);
	}
	_fileStream->seek(0x0024BF72);
	for (int i = 0; i < 16; i++) {
		_pathfindingPoints[i * 2] = _fileStream->readUint16LE();
		_pathfindingPoints[i * 2 + 1] = _fileStream->readUint16LE();
		// 4 bytes: adjacent point indices, 2 bytes: connection count
		Common::Array<uint8> indices;

		for (int j = 0; j < 4; j++) {
			indices.push_back(_fileStream->readByte());
		}
		_fileStream->readUint16LE(); // numConnections (already read above in pathfindingPoints)
	}

	// Iterate over 0 to 199 for each row
	// Load the amount of bytes for the row
	// Load the row data and do run-length decoding
	// Save into the right row of the surface

	// Load the data for a character

	_fileStream->seek(0x6FB2);
	_charASCII = _fileStream->readByte();
	_charWidth = _fileStream->readUint16LE();
	_charHeight = _fileStream->readUint16LE();
	_charData = new byte[_charWidth * _charHeight];
	_fileStream->read(_charData, _charWidth * _charHeight);

	// Font 1 already loaded above after ReadImageResources

	// Load the animation frames
	// Load 6 flag/decoration animation frames from fixed resource file offset
	_fileStream->seek(0x006A5941);
	for (int i = 0; i < 6; i++) {
		_animFrames[i].ReadFromStream(_fileStream);
		// There are 6 empty bytes until the next one
		_fileStream->seek(6, SEEK_CUR);
	}

	// Load the data for a border part
	AnimFrame &borderFrame = imageResources[imageResources.size() - 2];
	_borderSprite.Data.assign(borderFrame.Data, borderFrame.Data + borderFrame.Width * borderFrame.Height);
	_borderSprite.Width = borderFrame.Width;
	_borderSprite.Height = borderFrame.Height;
	_borderData = borderFrame.Data;

	// TODO: Get rid of these
	_borderWidth = _borderSprite.Width;
	_borderHeight = _borderSprite.Height;

	// And the highlight part
	_fileStream->seek(0x6962);

	_borderHighlightWidth = _fileStream->readUint16LE();
	_borderHighlightHeight = _fileStream->readUint16LE();
	_borderHighlightData = new byte[_borderHighlightWidth * _borderHighlightHeight];
	_fileStream->read(_borderHighlightData, _borderHighlightWidth * _borderHighlightHeight);
	_borderHighlightSprite.Width = _borderHighlightWidth;
	_borderHighlightSprite.Height = _borderHighlightHeight;
	_borderHighlightSprite.Data = Common::Array<uint8>(_borderHighlightData, _borderHighlightWidth * _borderHighlightHeight);

	AnimFrame &shadowFrame = imageResources[imageResources.size() - 3];
	_borderShadowSprite.Data.assign(shadowFrame.Data, shadowFrame.Data + shadowFrame.Width * shadowFrame.Height);
	_borderShadowSprite.Width = shadowFrame.Width;
	_borderShadowSprite.Height = shadowFrame.Height;

	// The flag animation frames
	_fileStream->seek(0x00250D47);
	_flagData = new byte *[3];
	_flagWidths = new uint16[3];
	_flagHeights = new uint16[3];
	_flagWidths[0] = _fileStream->readUint16LE();
	_flagHeights[0] = _fileStream->readUint16LE();
	_flagData[0] = new byte[_flagWidths[0] * _flagHeights[0]];
	_fileStream->read(_flagData[0], _flagWidths[0] * _flagHeights[0]);

	_fileStream->seek(0x00250B3C);
	_flagWidths[1] = _fileStream->readUint16LE();
	_flagHeights[1] = _fileStream->readUint16LE();
	_flagData[1] = new byte[_flagWidths[1] * _flagHeights[1]];
	_fileStream->read(_flagData[1], _flagWidths[1] * _flagHeights[1]);

	_fileStream->seek(0x00250931);
	_flagWidths[2] = _fileStream->readUint16LE();
	_flagHeights[2] = _fileStream->readUint16LE();
	_flagData[2] = new byte[_flagWidths[2] * _flagHeights[2]];
	_fileStream->read(_flagData[2], _flagWidths[2] * _flagHeights[2]);

	// Load the strings for the scene
	_fileStream->seek(0x000D2F22);
	numBytesStrings = _fileStream->readUint16LE();
	stringsData = new byte[numBytesStrings];
	_fileStream->read(stringsData, numBytesStrings);
	// _stringsStream = new Common::MemoryReadStream(stringsData, numBytesStrings);

	// Load the background map
	// _map = readRLEImage(0x0024BD9B, file);
	// _map = readRLEImage(0x00248FCE, file);
	// Next on is the actual map
	_map = readRLEImage(0x0024B0DF, _fileStream);
	// Depth map (scene data offset 0x1013, used by drawSpriteTransparent for depth testing)
	_depthMap = readRLEImage(0x00248FCE, _fileStream);

	// This is the walkability map - TBC if that's really it and how it works
	_pathfindingMap = readRLEImage(0x00249CC1, _fileStream);

	// Load the data for the mouse cursor
	// Original loads 33 entries from the same position as ReadImageResources.
	// File layout: 0xC header + 4 indices + 0x3000 scene table + 0x300 palette + 0x800 shading = 0x3B10
	_fileStream->seek(0xC + 0x4 + 0x3000 + 0x300 + 0x800);
	constexpr int numCursors = 33; // Original loads 0x21 (33) cursor/icon image entries
	_cursorData = new byte *[numCursors];
	_cursorWidths = new uint16[numCursors];
	_cursorHeights = new uint16[numCursors];
	for (int i = 0; i < numCursors; i++) {
		// Original format: 4-byte blob size, then blob data.
		// Blob layout: +0 unknown(2), +2 width(2), +4 height(2), +6 pixels(w*h)
		uint32 blobSize = _fileStream->readUint32LE();
		if (blobSize == 0) {
			_cursorData[i] = nullptr;
			_cursorWidths[i] = 0;
			_cursorHeights[i] = 0;
			continue;
		}
		byte *blob = new byte[blobSize];
		_fileStream->read(blob, blobSize);
		if (blobSize >= 6) {
			_cursorWidths[i] = READ_LE_UINT16(blob + 2);
			_cursorHeights[i] = READ_LE_UINT16(blob + 4);
			uint32 pixelSize = _cursorWidths[i] * _cursorHeights[i];
			if (pixelSize > 0 && pixelSize + 6 <= blobSize) {
				_cursorData[i] = new byte[pixelSize];
				memcpy(_cursorData[i], blob + 6, pixelSize);
			} else {
				_cursorData[i] = nullptr;
				_cursorWidths[i] = 0;
				_cursorHeights[i] = 0;
			}
		} else {
			_cursorData[i] = nullptr;
			_cursorWidths[i] = 0;
			_cursorHeights[i] = 0;
		}
		delete[] blob;
	}

	// Load a frame of animation from the protagonist
	// Crawling towards the left
	// _fileStream->seek(0x000C95A8);
	// Standing towards the right
	// _fileStream->seek(0x001CC5D9);
	// Also standing towards the right
	// _fileStream->seek(0x000D0B26);

	_fileStream->seek(0x006AB0FD);

	_guyWidth = _fileStream->readUint16LE();
	_guyHeight = _fileStream->readUint16LE();
	_guyData = new byte[_guyWidth * _guyHeight];
	_fileStream->read(_guyData, _guyWidth * _guyHeight);

	// Load the shading table
	_fileStream->seek(0x00248ECB);
	_shadingTable = new byte[256];
	_fileStream->read(_shadingTable, 256);

	// Load the objects data
	_fileStream->seek(0x6a5913);

	// Load the stick
	_fileStream->seek(0x00708410);
	_stick.ReadFromStream(_fileStream);

	/*
	_fileStream->seek(0x000d4fbd);
	// TODO: Figure out where the number comes from
	byte *adlibData = new byte[15610];
	_fileStream->read(adlibData, 15610);
	_adlib->data = new Common::MemoryReadStream(adlibData, 15610); */
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

void Macs2Engine::ReadBackgroundAnimations(Common::MemoryReadStream *stream) {
	// Offset 50F5 in scene data
	// TODO: Remove the non-blob implementation
	_numBackgroundAnimations = stream->readUint16LE();

	_backgroundAnimations = new BackgroundAnimation[_numBackgroundAnimations];
	_backgroundAnimationsBlobs.resize(_numBackgroundAnimations);

	for (int i = 0; i < _numBackgroundAnimations; i++) {
		BackgroundAnimationBlob &currentBlob = _backgroundAnimationsBlobs[i];

		BackgroundAnimation &current = _backgroundAnimations[i];
		// Local offset +0h
		current.X = stream->readUint16LE();
		currentBlob.X = current.X;
		// Local offset +2n
		current.Y = stream->readUint16LE();
		currentBlob.Y = current.Y;

		// current.numFrames = previewNumFrames(file.pos(), file);
		uint32 numBytes = stream->readUint32LE();

		currentBlob.Blob.resize(numBytes);
		int64 pos = stream->pos();
		stream->read(currentBlob.Blob.data(), numBytes);
		stream->seek(pos, SEEK_SET);

		// Skip to the intermediary data
		// Game loading code puts this at a pointer stored in local offset +8h
		stream->seek(10, SEEK_CUR);
		uint16 nextNumBytes = stream->readUint16LE();
		stream->seek(nextNumBytes, SEEK_CUR);
		current.numFrames = stream->readUint16LE();
		current.FrameIndex = 0;
		current.Frames = new AnimFrame[current.numFrames];
		for (int j = 0; j < current.numFrames; j++) {
			// Skip to width and height
			stream->seek(6, SEEK_CUR);
			current.Frames[j].ReadFromStream(stream);
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
		Macs2::BackgroundAnimationBlob::advanceAnimFrame(currentBlob.Blob, true, 0x64 + current.numFrames);
	}
}

void Macs2Engine::ReadImageResources(Common::MemoryReadStream *stream) {
	// l0037_3355:
	for (int i = 0; i < 0x21; i++) {
		uint32 length = stream->readUint32LE();
		if (length == 0) {
			continue;
		}
		AnimFrame frame;
		// Move forward to skip the first word
		stream->seek(0x2, SEEK_CUR);
		frame.ReadFromStream(stream);
		imageResources.push_back(frame);
		debug("W: %u, H: %u", frame.Width, frame.Height);
	}
}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			 _gameDescription(gameDesc), _randomSource("Macs2") {
	g_engine = this;
	_scriptExecutor = new Script::ScriptExecutor();
	_scriptExecutor->_engine = this;
	_adlib = new Adlib();

	DebugMan.addDebugChannel(DEBUG_RLE, "rle", "Verbose RLE decoding log");
	DebugMan.addDebugChannel(DEBUG_SV, "sv", "Verbose script debugging log");
	// We have a fixed 0x10 number of entries
	HotspotOverrides.resize(0x11);
	for (uint i = 0; i < HotspotOverrides.size(); i++) {
		HotspotOverrides[i] = 0xFFFF;
	}
	pathfindingValueRemaps.resize(0x100);
	for (uint i = 0; i < pathfindingValueRemaps.size(); i++) {
		pathfindingValueRemaps[i] = 0;
	}
}

Macs2Engine::~Macs2Engine() {
	stopInputRecording();
	clearCurrentSoundData();
	_adlib->Deinit();
}

void Macs2Engine::changeScene(uint32 newSceneIndex, bool executeScript) {
	// Release old scene resources
	if (_backgroundAnimations != nullptr) {
		for (int i = 0; i < _numBackgroundAnimations; i++) {
			delete[] _backgroundAnimations[i].Frames;
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
	_fileStream->seek(bgImageOffset, SEEK_SET);

	// TODO: Copy-pasted code here
	// _bgImageShip.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	uint8 *data = new uint8[0x320];

	// TODO: Consider if it can be that the data is more than this. Maybe the tooling of the engine can make bad calls and
	// try to RLE something which would be better not RLE encoded.

	for (int y = 0; y < 200; y++) {
		// TODO: Use the proper read function, it seems to be available
		uint16 length = _fileStream->readUint16LE();
		_fileStream->read(data, length);
		debugC(DEBUG_RLE, "RLE: Row %.4x with %.4x bytes of data.", y, length);
		int16 remainingPixels = 320; // signed: see readRLEImage (matches decodeRLERows 1008:0666)
		uint8 *dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8 &value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				_bgImageShip.setPixel(x, y, value);
				remainingPixels--;
				debugC(DEBUG_RLE, "RLE : Literal pixel %.2x, remaining row data %.4x bytes.", value, remainingPixels);
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
				debugC(DEBUG_RLE, "RLE: Encoded pixel %.2x for %.2x reps, remaining row data %.4x bytes.", encodedValue, runlength, remainingPixels);
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
	// at 1, and add 5019 to the address, and we multiply by 0xA to get the data
	// of the specific point
	array5023.clear();
	// We will address this array as words, so we are not using a byte array but a word array
	array5023.resize(0xa0);
	_fileStream->read(array5023.data(), 0xa0);

	word50D3 = _fileStream->readUint16LE();

	array50D5.clear();
	array50D5.resize(0x20 / 2);
	_fileStream->read(array50D5.data(), 0x20);

	// TODO: Remove the now superfluous one
	ReadBackgroundAnimations(_fileStream);

	// Offset 51F7h
	numPathfindingPoints = _fileStream->readUint16LE();

	// Offset 51F9h
	_fileStream->readUint16LE();

	// Offset 51FBh
	_fileStream->readUint16LE();

	// Offset 51FDh - 5201h
	word51FD = _fileStream->readUint16LE();
	word51FF = _fileStream->readUint16LE();
	word5201 = _fileStream->readUint16LE();

	word5203 = _fileStream->readUint16LE();
	word5205 = _fileStream->readUint16LE();

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
	array520D.resize(0x80 / 4);
	_fileStream->read(array520D.data(), 0x80);

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
	currentView->activeInventoryItem = nullptr;
	currentView->isShowingMainMenu = false;
	currentView->clearOverlayTextEntries();
	_scriptExecutor->inventoryActionFlag = false;
	_scriptExecutor->inventoryCombineFlag = false;
	_scriptExecutor->overlayTextStageActive = false;

	// Stop all characters from sending leftover events
	for (auto currentCharacter : currentView->characters) {
		currentCharacter->ExecuteScriptOnFinishLerp = false;
	}
	currentView->characters.clear();
	for (auto currentObject : GameObjects::instance().Objects) {
		if (currentObject->SceneIndex == newSceneIndex) {
			Character *c = new Character();
			c->GameObject = currentObject;
			currentView->characters.push_back(c);
		}
	}

	// Load the script and execute it
	Scenes::instance().LastSceneIndex = Scenes::instance().CurrentSceneIndex;
	Scenes::instance().CurrentSceneIndex = newSceneIndex;
	Scenes::instance().CurrentSceneScript = Scenes::instance().ReadSceneScript(newSceneIndex, _fileStream);
	Scenes::instance().CurrentSceneStrings = Scenes::instance().ReadSceneStrings(newSceneIndex, _fileStream);
	Scenes::instance().CurrentSceneSpecialAnimOffsets = Scenes::instance().ReadSpecialAnimsOffsets(newSceneIndex, _fileStream);
	_scriptExecutor->SetScript(Scenes::instance().CurrentSceneScript);

	if (executeScript) {
		// Start the execution
		_scriptExecutor->Run(true);
	}
}

bool Macs2Engine::loadOverlayFont(uint8 resourceIndex, uint16 executingObjectID) {
	// Original (1008:d749): looks up file offset from scene/object resource table
	// at scene+0x5209+index*4 (same table as loadIndexedResource/array520D),
	// seeks to offset+0x10, then calls loadFontData.
	if (resourceIndex == 0)
		return false;

	const int64 oldPos = _fileStream->pos();
	uint32 address = 0;

	if (executingObjectID == 0) {
		if (resourceIndex > array520D.size()) {
			_fileStream->seek(oldPos, SEEK_SET);
			return false;
		}
		address = array520D[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::GetObjectByIndex(executingObjectID);
		if (object == nullptr || object->DataOffset == 0) {
			_fileStream->seek(oldPos, SEEK_SET);
			return false;
		}
		_fileStream->seek(object->DataOffset + 0x189 + (resourceIndex - 1) * 4, SEEK_SET);
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
		_overlayGlyphs[i].ReadFromMemory(_fileStream);
		maxOverlayGlyphHeight = MAX(maxOverlayGlyphHeight, _overlayGlyphs[i].Height);
	}
	_fileStream->seek(oldPos, SEEK_SET);
	return true;
}

bool Macs2Engine::FindGlyph(char c, GlyphData &out) const {
	for (int i = 0; i < numGlyphs; i++) {
		if (_glyphs[i].ASCII == c) {
			out = _glyphs[i];
			return true;
		}
	}
	return false;
}

uint16 Macs2Engine::getWalkabilityAt(uint16 x, uint16 y) {
	if (x >= 320 || y >= 200) {
		return 0;
	}
	uint16 value = _pathfindingMap.getPixel(x, y);
	if (value >= 0xC8 && value <= 0xEF) {
		// Pathfinding override range: check override table
		uint16 overrideResult;
		if (GetPathfindingOverride(value, overrideResult)) {
			return overrideResult;
		}
		return 0xFF; // Override disabled = non-walkable
	}
	return value;
}

void Macs2Engine::snapToWalkablePosition(Common::Point &target, const Common::Point &charPos) {
	// Simplified snapToWalkablePosition (1008:9be2):
	// If target is non-walkable, scan downward then upward to find walkable pixel.
	// Then apply gradient push away from nearby walls.
	if (getWalkabilityAt(target.x, target.y) < 200)
		return; // Already walkable

	// Scan downward
	for (int y = target.y; y < 200; y++) {
		if (getWalkabilityAt(target.x, y) < 200) {
			target.y = y;
			goto applyPush;
		}
	}
	// Scan upward
	for (int y = target.y; y >= 0; y--) {
		if (getWalkabilityAt(target.x, y) < 200) {
			target.y = y;
			goto applyPush;
		}
	}
	// Scan X toward character
	if (charPos.x < target.x) {
		for (int x = target.x; x >= 0; x--) {
			if (getWalkabilityAt(x, target.y) < 200) {
				target.x = x;
				goto applyPush;
			}
		}
	} else {
		for (int x = target.x; x < 320; x++) {
			if (getWalkabilityAt(x, target.y) < 200) {
				target.x = x;
				goto applyPush;
			}
		}
	}
	// All failed — fall back to character position
	target = charPos;
	return;

applyPush:
	// Gradient-based wall push (sample ±1, ±2 pixels)
	int pushX = 0, pushY = 0;
	if (getWalkabilityAt(target.x + 1, target.y) >= 200)
		pushX--;
	if (getWalkabilityAt(target.x - 1, target.y) >= 200)
		pushX++;
	if (getWalkabilityAt(target.x, target.y + 1) >= 200)
		pushY--;
	if (getWalkabilityAt(target.x, target.y - 1) >= 200)
		pushY++;
	if (getWalkabilityAt(target.x + 2, target.y) >= 200)
		pushX--;
	if (getWalkabilityAt(target.x - 2, target.y) >= 200)
		pushX++;
	if (getWalkabilityAt(target.x, target.y + 2) >= 200)
		pushY--;
	if (getWalkabilityAt(target.x, target.y - 2) >= 200)
		pushY++;

	while (pushX != 0 || pushY != 0) {
		if (pushX < 0 && getWalkabilityAt(target.x - 1, target.y) < 200) {
			target.x--;
			pushX++;
		}
		if (pushX > 0 && getWalkabilityAt(target.x + 1, target.y) < 200) {
			target.x++;
			pushX--;
		}
		if (pushY < 0 && getWalkabilityAt(target.x, target.y - 1) < 200) {
			target.y--;
			pushY++;
		}
		if (pushY > 0 && getWalkabilityAt(target.x, target.y + 1) < 200) {
			target.y++;
			pushY--;
		}
		// Safety: break if push can't move
		if (pushX < 0 && getWalkabilityAt(target.x - 1, target.y) >= 200)
			pushX = 0;
		if (pushX > 0 && getWalkabilityAt(target.x + 1, target.y) >= 200)
			pushX = 0;
		if (pushY < 0 && getWalkabilityAt(target.x, target.y - 1) >= 200)
			pushY = 0;
		if (pushY > 0 && getWalkabilityAt(target.x, target.y + 1) >= 200)
			pushY = 0;
	}
}

bool Macs2Engine::GetPathfindingOverride(uint16 index, uint16 &result) {
	for (auto current : PathfindingOverrides) {
		if (current.Index == index && current.Active) {
			result = current.OverrideValue;
			return true;
		}
	}
	return false;
}
void Macs2Engine::SetPathfindingOverride(uint16 index, uint16 overrideValue) {
	RemovePathfindingOverride(index);
	PathfindingAreaOverride override;
	override.Active = true;
	override.Index = index;
	override.OverrideValue = overrideValue;
	PathfindingOverrides.push_back(override);
}

uint8 Macs2Engine::GetPathfindingOverride2(uint16 index) {
	if (index < AREA_OVERRIDE_MIN || index > AREA_OVERRIDE_MAX) {
		return 0;
	}
	return _areaOverrides[index - AREA_OVERRIDE_MIN];
}

void Macs2Engine::RemovePathfindingOverride(uint16 index) {
	for (uint i = 0; i < PathfindingOverrides.size(); i++) {
		PathfindingAreaOverride &current = PathfindingOverrides[i];
		if (current.Index == index) {
			PathfindingOverrides.remove_at(i);
			return;
		}
	}
};

bool Macs2Engine::isPathWalkable(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	// TODO: need to figure out which algorithm the game uses exactly

	// Trace a line between p1 and p2
	// Look up in the map if we are walkable

	// Bresenham's line algorithm, as described by Wikipedia
	const bool steep = ABS(y2 - y1) > ABS(x2 - x1);

	if (steep) {
		SWAP(x1, y1);
		SWAP(x2, y2);
	}

	const int delta_x = ABS(x2 - x1);
	const int delta_y = ABS(y2 - y1);
	const int delta_err = delta_y;
	int x = x1;
	int y = y1;
	int err = 0;

	const int x_step = (x1 < x2) ? 1 : -1;
	const int y_step = (y1 < y2) ? 1 : -1;

	uint16 walkability = 0;
	int stopFlag = 0;

	if (steep) {
		walkability = getWalkabilityAt(y, x);
		stopFlag = false; // (*plotProc)(y, x, data);
	} else {
		walkability = getWalkabilityAt(x, y);
		stopFlag = false; // (*plotProc)(x, y, data);
	}
	if (walkability > 0x0C7) {
		return false;
	}

	while (x != x2 && !stopFlag) {
		x += x_step;
		err += delta_err;
		if (2 * err > delta_x) {
			y += y_step;
			err -= delta_x;
		}
		if (steep) {
			walkability = getWalkabilityAt(y, x);
			stopFlag = false; // (*plotProc)(y, x, data);
		} else {
			walkability = getWalkabilityAt(x, y);
			stopFlag = false; // (*plotProc)(x, y, data);
		}
		if (walkability > 0x0C7) {
			return false;
		}
	}
	// return stopFlag;

	return true;
}

void Macs2Engine::NextCursorMode() {
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

void Macs2Engine::SetCursorMode(Script::MouseMode newMode) {
	_scriptExecutor->_mouseMode = newMode;
}

void Macs2Engine::DumpStream(Common::MemoryReadStream *s, uint16 len) {
	int64 start_pos = s->pos();
	Common::String result;
	for (int i = 0; i < len; i++) {
		uint8 v = s->readByte();
		result += Common::String::format("%.2X", v);
	}
	debug(result.c_str());
	s->seek(start_pos, SEEK_SET);
}

uint16 Macs2Engine::GetInteractedBackgroundHotspot(const Common::Point &p) {
	uint16 result = 0;
	// TODO: Abstract the screen sizes
	if (p.x < 0 || p.x > 320 || p.y < 0 || p.y > 200) {
		return result;
	}

	uint8 firstLookup = _map.getPixel(p.x, p.y);
	uint16 numHotspots = word50D3;

	uint8 i = 1;
	if (i > numHotspots) {
		return result;
	}

	Common::Array<uint16> a = array50D5;

	do {
		uint16 lookup = a[i - 1];
		if (lookup == firstLookup) {
			if (HotspotOverrides[i] != 0xFFFF) {
				return 0x800 + HotspotOverrides[i];
			}
			return 0x800 + i;
		}
		i++;
	} while (i <= numHotspots);
	return 0;
}

void Macs2Engine::ScheduleRun(bool initScene) {
	runScheduled = true;
	scheduledRunIsInitScene = initScene;
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

	if (value >= pathfindingValueRemaps.size()) {
		return 0xFF;
	}

	const uint16 remappedValue = pathfindingValueRemaps[value];
	return remappedValue != 0 ? remappedValue : 0xFF;
}

int Macs2Engine::MeasureString(Common::String &s) {
	int sum = 0;
	GlyphData currentGlyph;
	bool found = false;
	uint16 widestGlyph = 0;
	for (auto current = s.begin(); current != s.end(); current++) {
		found = FindGlyph(*current, currentGlyph);
		if (found) {
			widestGlyph = MAX(widestGlyph, currentGlyph.Width);
		}
	}

	for (auto current = s.begin(); current != s.end(); current++) {
		found = FindGlyph(*current, currentGlyph);
		if (!found) {
			sum += widestGlyph;
		} else {
			sum += currentGlyph.Width + 1;
		}
	}
	return sum;
}

int Macs2Engine::MeasureStringsVertically(Common::StringArray sa) {
	// This is implemented around l0037_B318:
	return sa.size() * (maxGlyphHeight + 2);
}

int Macs2Engine::MeasureStrings(Common::StringArray sa) {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(MeasureString(*iter), max);
	}
	return max;
}

Common::StringArray Macs2Engine::DecodeStrings(Common::MemoryReadStream *stream, int offset, int numStrings) {
	Common::StringArray result(numStrings);
	DumpStream(stream, 64);
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
	if (arrayIndex == 0 || arrayIndex > array520D.size()) {
		warning("Invalid animation array index %u for object %u slot %u", arrayIndex, objectIndex, slotIndex);
		return;
	}

	GameObject *go = GameObjects::instance().GetObjectByIndex(objectIndex);
	if (go == nullptr) {
		warning("Tried to load animation for invalid object %u", objectIndex);
		return;
	}

	if (slotIndex == 0 || slotIndex > 0x15) {
		warning("Invalid animation slot %u for object %u", slotIndex, objectIndex);
		return;
	}

	// We need to account for the game starting indices at 1
	uint32 address = array520D[arrayIndex - 1];
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

	uint32 address = array520D[dataIndex - 1];
	_fileStream->seek(address);
	uint32 size = _fileStream->readUint32LE();
	// TODO: Also delete this one
	Common::Array<uint8> *data = new Common::Array<uint8>();
	data->resize(size);
	_fileStream->read(data->data(), size);
	StreamHandler *sh = new StreamHandler(data);
	_adlib->SetSong(sh);
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
	audioStream->pos = 2; // Skip 2-byte header (original: size = stored_size - 2)
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

	GameObjects::instance().Init();
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
		int dosSlot = slot - 100;
		Common::String name = Common::String::format("SAVEGAME.%d", dosSlot);
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(name);
		if (!f) {
			name = Common::String::format("savegame.%d", dosSlot);
			f = g_system->getSavefileManager()->openForLoading(name);
		}
		if (!f)
			return Common::kReadingFailed;
		Common::Error err = loadOriginalSave(f);
		delete f;
		return err;
	}
	return Engine::loadGameState(slot);
}

Common::Error Macs2Engine::syncGame(Common::Serializer &s) {
	static const uint32 SAVE_VERSION = 1;
	uint32 version = SAVE_VERSION;
	s.syncAsUint32LE(version);
	if (s.isLoading() && version > SAVE_VERSION)
		return Common::kReadingFailed;

	// Core indices
	s.syncAsSint32LE(Scenes::instance().CurrentActorIndex);
	s.syncAsSint32LE(Scenes::instance().CurrentSceneIndex);

	View1 *currentView = (View1 *)findView("View1");
	if (s.isLoading()) {
		currentView->started = true;
		changeScene(Scenes::instance().CurrentSceneIndex, false);
	}

	// Script variables (2048 × 4 bytes = 0x2000 bytes, matching original)
	for (uint i = 0; i < _scriptExecutor->_variables.size(); i++) {
		s.syncAsUint16LE(_scriptExecutor->_variables[i].a);
		s.syncAsUint16LE(_scriptExecutor->_variables[i].b);
	}

	// Sound/music state
	s.syncAsByte(_scriptExecutor->soundSystemActive);
	s.syncAsByte(_scriptExecutor->soundEnabled);
	s.syncAsByte(_scriptExecutor->musicEnabled);
	s.syncAsUint16LE(_scriptExecutor->activeMusicSlot);
	s.syncAsUint16LE(_scriptExecutor->musicControlMode);
	s.syncAsUint16LE(_scriptExecutor->musicControlParam);
	s.syncAsUint16LE(_scriptExecutor->musicControlVolume);

	// Script executor state
	uint16 mouseMode = (uint16)_scriptExecutor->_mouseMode;
	s.syncAsUint16LE(mouseMode);
	if (s.isLoading())
		_scriptExecutor->_mouseMode = (Script::MouseMode)mouseMode;

	s.syncAsUint16LE(_scriptExecutor->_interactedObjectID);
	s.syncAsUint16LE(_scriptExecutor->_interactedOtherObjectID);
	s.syncAsByte(_scriptExecutor->scriptSkippable);
	s.syncAsByte(_scriptExecutor->pickupInProgress);
	s.syncAsUint16LE(_scriptExecutor->pickupActorObjectID);
	s.syncAsUint16LE(_scriptExecutor->pickupTargetObjectID);
	s.syncAsByte(_scriptExecutor->isRepeatRun);
	s.syncAsByte(_scriptExecutor->inventoryCheckResult);
	s.syncAsByte(_scriptExecutor->animBlobRangeTestResult);
	s.syncAsByte(_scriptExecutor->inventoryActionFlag);
	s.syncAsByte(_scriptExecutor->inventoryCombineFlag);

	// Area overrides (matching original's 0x200 byte block at DAT_1020_202a)
	for (int i = 0; i < AREA_OVERRIDE_COUNT; i++) {
		s.syncAsUint16LE(_areaOverrides[i]);
	}

	// All 512 objects: position, scene, orientation, unknown
	for (auto obj : GameObjects::instance().Objects) {
		s.syncAsUint16LE(obj->Position.x);
		s.syncAsUint16LE(obj->Position.y);
		s.syncAsUint16LE(obj->SceneIndex);
		s.syncAsUint16LE(obj->Orientation);
		s.syncAsUint16LE(obj->Unknown);
		s.syncAsByte(obj->HasBoundsAttachment);
		s.syncAsUint16LE(obj->BoundsAttachmentObjectID);
		s.syncAsUint16LE(obj->BoundsAttachmentValue1);
		s.syncAsUint16LE(obj->BoundsAttachmentValue2);
		s.syncAsUint16LE(obj->BoundsAttachmentValue3);
		s.syncAsByte(obj->IsClickable);
		s.syncAsByte(obj->IsVisible);
		s.syncAsUint16LE(obj->RuntimeValue217);
		s.syncAsUint16LE(obj->RuntimeValue219);
		s.syncAsByte(obj->RuntimeFlag22F);
		s.syncAsByte(obj->useOverloadAnimation);
		s.syncAsUint16LE(obj->overloadAnimTriggerDirection);
	}

	// Characters in current scene
	uint32 numCharacters = 0;
	if (s.isSaving())
		numCharacters = currentView->characters.size();
	s.syncAsUint32LE(numCharacters);
	if (s.isLoading())
		currentView->characters.clear();
	for (uint32 i = 0; i < numCharacters; i++) {
		uint32 characterIndex = 0;
		if (s.isSaving())
			characterIndex = currentView->characters[i]->GameObject->Index;
		s.syncAsUint32LE(characterIndex);
		if (s.isLoading()) {
			Character *c = new Character();
			c->GameObject = GameObjects::instance().Objects[characterIndex - 1];
			currentView->characters.push_back(c);
		}
	}

	// Rebuild inventory on load
	if (s.isLoading()) {
		currentView->SetInventorySource(GameObjects::instance().GetProtagonistObject());
		currentView->UpdateCursor();
		currentView->_paletteDirty = true;
	}

	return Common::kNoError;
}

Common::Error Macs2Engine::loadOriginalSave(Common::SeekableReadStream *stream) {
	// Validate 12-byte magic "AHFFMSGM0100"
	char magic[12];
	stream->read(magic, 12);
	if (memcmp(magic, "AHFFMSGM0100", 12) != 0)
		return Common::kReadingFailed;

	// Skip 21-byte slot name (Pascal string)
	stream->skip(21);

	// Current actor index and scene index (2 bytes each, LE)
	Scenes::instance().CurrentActorIndex = stream->readUint16LE();
	Scenes::instance().CurrentSceneIndex = stream->readUint16LE();

	View1 *currentView = (View1 *)findView("View1");
	currentView->started = true;
	changeScene(Scenes::instance().CurrentSceneIndex, false);

	// Script variables: 0x2000 bytes = 2048 vars × (uint16 a + uint16 b)
	for (uint i = 0; i < _scriptExecutor->_variables.size(); i++) {
		_scriptExecutor->_variables[i].a = stream->readUint16LE();
		_scriptExecutor->_variables[i].b = stream->readUint16LE();
	}

	// Sound system active (2 bytes)
	_scriptExecutor->soundSystemActive = stream->readUint16LE() != 0;

	// Script state fields (matching original loadGameFromFile order):
	// 0xf88: 1 byte - scriptIsExecuting
	stream->readByte(); // g_wScriptIsExecuting - we don't restore this
	// 0xf8a: 2 bytes - script position low
	stream->readUint16LE(); // script position - not directly applicable
	// 0xf90-0xf9a: 6 × 2 bytes - script state words
	stream->readUint16LE(); // 0xf90
	stream->readUint16LE(); // 0xf92 - executingScriptObjectId
	// _executingScriptObjectID is private and resets on load
	stream->readUint16LE(); // 0xf94
	stream->readUint16LE(); // 0xf96
	stream->readUint16LE(); // 0xf98
	stream->readUint16LE(); // 0xf9a

	// g_wRepeatRunFlag: 1 byte
	_scriptExecutor->isRepeatRun = stream->readByte() != 0;
	// g_wFrameWaitCounter: 2 bytes
	stream->readUint16LE(); // frame wait - not directly used
	// g_wWalkTargetObjectIndex: 2 bytes
	stream->readUint16LE(); // walk target
	// g_wPickupInProgress: 2 bytes
	_scriptExecutor->pickupInProgress = stream->readUint16LE() != 0;
	// 0xfd0: 2 bytes
	stream->readUint16LE();
	// 0xfea: 2 bytes
	stream->readUint16LE();
	// 0xfec: 1 byte
	stream->readByte();
	// g_wWalkTargetObjectIndex (again): 2 bytes
	stream->readUint16LE();
	// PTR_LOOP_1020_1018: 2 bytes (cursor mode related)
	uint16 mouseMode = stream->readUint16LE();
	if (mouseMode >= 0x13 && mouseMode <= 0x1A)
		_scriptExecutor->_mouseMode = (Script::MouseMode)mouseMode;
	else
		_scriptExecutor->_mouseMode = Script::MouseMode::Walk;
	// 0x101a-0x101e: 3 × 2 bytes
	stream->readUint16LE();
	stream->readUint16LE();
	stream->readUint16LE();
	// 0x1020: 1 byte
	stream->readByte();
	// g_wInteractedObjectId: 2 bytes
	_scriptExecutor->_interactedObjectID = stream->readUint16LE();
	// g_wInteractedInventoryItemId: 2 bytes
	_scriptExecutor->_interactedOtherObjectID = stream->readUint16LE();
	// g_wScriptSkippable: 1 byte
	_scriptExecutor->scriptSkippable = stream->readByte() != 0;
	// g_wPickupActorObjectId: 2 bytes
	_scriptExecutor->pickupActorObjectID = stream->readUint16LE();
	// g_wPickupTargetObjectId: 2 bytes
	_scriptExecutor->pickupTargetObjectID = stream->readUint16LE();
	// g_wIsRepeatRun: 2 bytes
	_scriptExecutor->isRepeatRun = stream->readUint16LE() != 0;
	// g_wInventoryCheckResult: 1 byte
	_scriptExecutor->inventoryCheckResult = stream->readByte() != 0;
	// g_wAnimBlobRangeTestResult: 1 byte
	_scriptExecutor->animBlobRangeTestResult = stream->readByte() != 0;
	// g_wInventoryActionFlag: 1 byte
	_scriptExecutor->inventoryActionFlag = stream->readByte() != 0;
	// g_wInventoryCombineFlag: 1 byte
	_scriptExecutor->inventoryCombineFlag = stream->readByte() != 0;
	// g_wSoundSystemState: 2 bytes
	uint16 soundState = stream->readUint16LE();
	_scriptExecutor->soundEnabled = (soundState & 1) != 0;
	_scriptExecutor->musicEnabled = (soundState & 2) != 0;

	// Area overrides: 0x200 bytes
	// Original has 256 uint16 values; we only use AREA_OVERRIDE_COUNT
	for (int i = 0; i < 256; i++) {
		uint16 val = stream->readUint16LE();
		if (i < AREA_OVERRIDE_COUNT)
			_areaOverrides[i] = val;
	}

	// Scene data: skip (pathfinding overrides, animation offsets, sound buffers)
	// These are scene-specific runtime data that gets rebuilt by changeScene
	// We skip the rest of the file as it contains runtime scene data and
	// per-object animation blobs that are rebuilt from the resource file.

	// All 512 objects: 5 × uint16 each (position.x, position.y, scene, orientation, unknown)
	// The original iterates objects 1..0x200 and reads their base fields
	// We need to seek past the scene data first. The scene data size varies,
	// so we read the object data from a known structure.
	// Unfortunately the scene data between area overrides and objects is variable-length
	// (depends on background animations count and PCM sound sizes).
	// For a robust implementation, we skip the variable scene/sound data and
	// just read the object positions which come at the end.

	// Since the variable-length data makes seeking complex, we'll just
	// skip the rest for original saves - the essential state (scene, variables,
	// object positions) would need the full binary-compatible parser.
	// For now, we restore what we can and let changeScene rebuild the rest.

	// Rebuild view state
	currentView->characters.clear();
	for (auto obj : GameObjects::instance().Objects) {
		if (obj->SceneIndex == (uint16)Scenes::instance().CurrentSceneIndex) {
			Character *c = new Character();
			c->GameObject = obj;
			currentView->characters.push_back(c);
		}
	}
	currentView->SetInventorySource(GameObjects::instance().GetProtagonistObject());
	currentView->UpdateCursor();
	currentView->_paletteDirty = true;

	return Common::kNoError;
}

bool Macs2Engine::tick() {
	_scriptExecutor->tick();
	if (runScheduled) {
		runScheduled = false;
		bool shouldRunInit = scheduledRunIsInitScene;
		scheduledRunIsInitScene = false;
		_scriptExecutor->isRepeatRun = true;
		_scriptExecutor->Run(shouldRunInit);
	}
	return Events::tick();
}

void GlyphData::ReadFromeFile(Common::File &file) {
	int64 stride = file.pos();
	ASCII = file.readByte();
	Width = file.readUint16LE();
	Height = file.readUint16LE();
	Data = new byte[Width * Height];
	file.read(Data, Width * Height);
	stride = file.pos() - stride;
}

void GlyphData::ReadFromMemory(Common::MemoryReadStream *stream) {
	int64 stride = stream->pos();
	ASCII = stream->readByte();
	Width = stream->readUint16LE();
	Height = stream->readUint16LE();
	Data = new byte[Width * Height];
	stream->read(Data, Width * Height);
	stride = stream->pos() - stride;
}

void AnimFrame::ReadFromeFile(Common::File &file) {
	Width = file.readUint16LE();
	Height = file.readUint16LE();
	Data = new byte[Width * Height];
	file.read(Data, Width * Height);
}

void AnimFrame::ReadFromStream(Common::MemoryReadStream *stream) {
	Width = stream->readUint16LE();
	Height = stream->readUint16LE();
	Data = new byte[Width * Height];
	stream->read(Data, Width * Height);
}

bool AnimFrame::PixelHit(const Common::Point &point) const {
	if (point.x < 0 || point.x >= Width || point.y < 0 || point.y >= Height) {
		return false;
	}
	return Data[point.y * Width + point.x] != 0;
}

Common::Point AnimFrame::GetBottomMiddleOffset(uint16 scale) const {
	if (scale == 100) {
		return Common::Point(Width / 2, Height);
	}
	return Common::Point(
		Width * scale / 200, // scaled width / 2
		Height * scale / 100);
}

Sprite AnimFrame::AsSprite() {
	// TODO: Shows that the separation makes little sense
	Sprite result;
	result.Data.resize(Width * Height);
	result.Data.assign(Data, Data + Width * Height);
	result.Width = Width;
	result.Height = Height;
	return result;
}

AnimFrame BackgroundAnimationBlob::GetFrame(uint32 index) {
	AnimationReader animReader(Blob);
	uint16 numAnimations = animReader.readNumAnimations();
	debug("Number of animation frames for background object: %.4x", numAnimations);

	// TODO: Check consistency between 0 and 1 based indexing
	animReader.SeekToAnimation((index - 1) % numAnimations);
	// testReader.SeekToAnimation(0);
	// Skip ahead to the width and height
	animReader.readStream->seek(6, SEEK_CUR);

	AnimFrame result;

	result.ReadFromStream(animReader.readStream);
	return result;
	// TODO: Think about proper memory management
}

AnimFrame BackgroundAnimationBlob::GetCurrentFrame() {
	// Mode 0: get current frame without advancing (advancement happens in View1::tick)
	uint16 offset = advanceAnimFrame(Blob, false, 0x0);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(Blob.data(), Blob.size());
	offset += 6;
	stream->seek(offset);
	AnimFrame result;
	result.ReadFromStream(stream);
	return result;
}

uint16 BackgroundAnimationBlob::advanceAnimFrame(Common::Array<uint8> &blob, bool bpp6, uint16 bpp8) {
	uint16 s = blob.size();
	if (s == 0x767) {
		s = 1;
	}
	Common::MemorySeekableReadWriteStream stream(blob.data(), blob.size());

	/* Arguments:
	Argument - Byte value: [bp+06]: 00 - Determines if we save the result
	Argument - Value between 0 and something around A4: [bp+08]: 0000 - The important one
	Argument - Address: [bp+0a]: 99e0:022f
	Argument - Address: [bp+0e]: 99e2:022f
	Argument - Address: [bp+12]: 0000:04bf
	*/

	// bp-22h
	uint16 bp22 = stream.readUint16LE();
	// bp-6h
	uint16 bp6 = stream.readUint16LE();
	// bp-8h
	uint16 bp8 = stream.readUint16LE();
	// bp-0Ah
	uint16 bp0A = stream.readUint16LE();
	// bp-10h
	uint16 bp10 = stream.readUint16LE();
	// bp-0Eh
	uint16 bp0E = stream.readUint16LE() + 1;

	stream.seek(bp6 - 1, SEEK_CUR);
	uint8 bp0C = stream.readByte();
	if (bpp8 == 0x1) {
		// l00B7_14B4:
		bp8 = 0x00;
		bp10 = 0x00;
		bp6 = 0x01;

	} else if (bpp8 >= 0x65) {
		if (bpp8 <= 0xA4) {
			bp6 = bpp8 - 0x64;
			bp8 = bp10 = 0x00;
			if (bp6 > bp0E) {
				bp6 = 0x01;
			}
		}
	}
	// l00B7_14EF:
	// TODO: Do I have the comparisons right between bp6 and bp0E throughout the function?
	if (bp6 >= bp0E) {
		// l00B7_14F7:
		bp6 = 1;
	}

	// l00B7_14FD:
	// TODO: Look end condition
	while (true) {
		if (bp6 >= bp0E) {
			// l00B7_14F7:
			bp6 = 1;
		}
		// l00B7_150A:
		stream.seek(0x0B, SEEK_SET);
		stream.seek(bp6, SEEK_CUR);
		bp0C = stream.readByte();
		if (bp0C == 0x01) {
			// l00B7_151F:
			bp6++;
			bp8 = stream.readByte();
			bp6++;
			bp0A = bp6;
		} else if (bp0C == 0x02) {
			// l00B7_1539:
			// TODO: Continue here
			bp6++;
			bp10 = stream.readByte();
			bp6++;
		} else if (bp0C == 0x03) {
			// l00B7_154B:
			// TODO: No idea why we would increment first
			bp6++;
			bp6 = stream.readByte();
		} else {
			break;
		}
	}
	// l00B7_1554:
	uint16 cx = bp0C - 0xA;
	stream.seek(0xB, SEEK_SET);
	stream.seek(bp0E, SEEK_CUR);
	uint16 bp24 = stream.readUint16LE();
	if (cx > bp24) {
		// l00B7_156A:
		cx = 1;
	}
	// l00B7_156D:
	for (; cx > 1; cx--) {
		// TODO: Check if the logic for the loop works out like this
		stream.readUint16LE(); // bp1A
		stream.readUint16LE(); // bp1C
		stream.seek(0x2, SEEK_CUR);
		uint16 bp16 = stream.readUint16LE();
		uint16 bp18 = stream.readUint16LE();
		// This is the amount of bytes of the frame (width * height)
		uint16 bx = bp16 * bp18;
		stream.seek(bx, SEEK_CUR);
	}

	// l00B7_158D:
	// TODO: Due to difference in implementation of the loop, I don't need to rewind,
	// but am probably doing a
	// stream.seek(-6, SEEK_CUR);
	uint16 bp12 = stream.pos();
	// TODO: Check if indendation is right here
	if (bpp8 == 0x02) {
		if (bp0C >= 0xA) {
			// l00B7_15A2:
			bp6++;
			if (bp10 > 0) {
				bp10--;
			}
			// l00B7_15AE:
			if (bp10 == 0) {
				// l00B7_15B4:
				if (bp8 > 0) {
					// l00B7_15BA:
					bp8--;
					bp6 = bp0A;
				}
			}
		}
	}
	// l00B7_15C3:
	if (bp6 >= bp0E) {
		bp6 = 1;
	}
	// l00B7_15D0:
	if (bpp6) {
		// l00B7_15D6:
		stream.seek(0, SEEK_SET);
		stream.writeUint16LE(bp22);
		stream.writeUint16LE(bp6);
		stream.writeUint16LE(bp8);
		stream.writeUint16LE(bp0A);
		stream.writeUint16LE(bp10);
	}

	// l00B7_15EF:
	// TODO: Check what these writes are
	/*
	* mov	ax,[bp-1Ah]
	les	di,[bp+0Eh]
	mov	es:[di],ax
	mov	ax,[bp-1Ch]
	les	di,[bp+0Ah]
	mov	es:[di],ax
	*/
	// TODO: Check what value exactly the original returns,
	// if we have bp+6h==1, we will mess up our posiition
	// return stream.pos();
	return bp12;
}

uint16 BackgroundAnimationBlob::getAnimFrameCount(Common::Array<uint8> &blob) {
	// [bp-2h]
	uint16 result;
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(blob.data(), blob.size());
	stream->seek(0xA);
	uint16 bp4 = stream->readUint16LE();
	bp4++;
	stream->seek(0xA + bp4 - 2);
	uint8 bp6 = stream->readByte();
	if (bp4 == 0x3) {
		// l00B7_16AF:
		result = 0;
	}
	// l00B7_16B4:
	if (bp6 != 0x3) {
		// l00B7_16BA:
		result = 0;
	}
	// l00B7_16BF:
	// TODO: I only realized this after finishing the function, but it looks like
	// the calculations in between are actually superfluous
	result = bp4;
	return result;
}

int MacsAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int numSamplesRead = 0;
	for (int i = 0; i < numSamples; i++) {
		if (pos >= _data.size()) {
			return numSamplesRead;
		}
		buffer[i] = static_cast<int16>((static_cast<int>(_data[pos]) - 128) << 8);
		numSamplesRead++;
		pos++;
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
	return pos >= _data.size();
}

bool MacsAudioStream::seek(const Audio::Timestamp &where) {
	const int64 targetPos = where.msecs() * getRate() / 1000;
	if (targetPos < 0 || targetPos > _data.size()) {
		return false;
	}

	pos = targetPos;
	return true;
}

Audio::Timestamp MacsAudioStream::getLength() const {
	return Audio::Timestamp(0, _data.size(), getRate());
}

} // End of namespace Macs2

/*
private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
		{
			string[] hexValuesSplit = InputTextBox.Text.Split(' ');
			string result = String.Empty;
			byte index = 1;
			foreach (string hex in hexValuesSplit)
			{
				byte value;
				// Convert the number expressed in base-16 to an integer.
				try
				{
					value = Convert.ToByte(hex, 16);
				}
				catch
				{
					return;
				}
				byte x = (byte)(index * index * 0x0c);
				byte y = (byte)(value ^ index);
				byte r = (byte)(x ^ y);
				result += (char)r;
				index++;
			}
			OutputTextBlock.Text = result;
		}

*/
