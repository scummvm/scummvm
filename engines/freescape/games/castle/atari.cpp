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
#include "common/file.h"
#include "common/memstream.h"
#include "common/endian.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

// RLE output stage of the Castle Master (Atari ST) packer. The Huffman tree
// produces a byte stream that is run-length encoded as follows (both counters
// start at -1): when idle the next byte is a *control* byte; values < 0x80
// repeat the following single byte (value + 1) times, values >= 0x80 copy the
// following (value & 0x7F) + 1 bytes literally.
static void emitByteAtari(Common::MemoryWriteStreamDynamic &out, int &rep, int &lit, byte b) {
	if (rep >= 0) {
		for (int i = 0; i <= rep; i++)
			out.writeByte(b);
		rep = -1;
	} else if (lit >= 0) {
		out.writeByte(b);
		lit--;
	} else if (b < 0x80) {
		rep = b;
	} else {
		lit = b & 0x7F;
	}
}

// Decompress a Castle Master (Atari ST) self-extracting GEMDOS executable.
//
// The player is expected to provide the Copylock-decrypted file (named
// "M.PRG"): a GEMDOS executable (magic 0x601A) whose DATA segment holds a
// Huffman-tree + RLE packed stream that, when expanded, yields the actual
// Castle Master game executable (also a GEMDOS PRG).
//
// Packed stream layout (at the start of the DATA segment, i.e. file offset
// 0x1C + TEXT size):
//     u32  count        number of 32-bit words in the bitstream
//     u16  nodeTblSize  size of the Huffman node table, in bytes
//     ...  nodeTable    4-byte nodes (left s16 BE @ +0, right s16 BE @ +2)
//     ...  bitstream    count*4 bytes, consumed MSB-first as big-endian u32s
//
// Walking the tree from the root, each bit selects the left (0) / right (1)
// child word `v`: 0 <= v <= 0x201 is an internal node (continue at node v);
// otherwise it is a leaf whose high byte (unless 0xFF) and low byte are fed to
// the RLE stage, after which the walk resets to the root.
Common::SeekableReadStream *CastleEngine::decompressAtari(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename))
		error("Failed to open '%s'", filename.toString().c_str());

	int fileSize = file.size();
	byte *buffer = (byte *)malloc(fileSize);
	file.read(buffer, fileSize);
	file.close();

	if (READ_BE_UINT16(buffer) != 0x601a) {
		free(buffer);
		error("'%s' is not a GEMDOS executable (expected Copylock-decrypted M.PRG)", filename.toString().c_str());
	}

	uint32 textSize = READ_BE_UINT32(buffer + 2);
	uint32 packedOffset = 0x1c + textSize; // start of the DATA segment

	uint32 count = READ_BE_UINT32(buffer + packedOffset);
	uint16 nodeTableSize = READ_BE_UINT16(buffer + packedOffset + 4);
	const byte *nodes = buffer + packedOffset + 6;
	const byte *bitstream = nodes + nodeTableSize;

	Common::MemoryWriteStreamDynamic out(DisposeAfterUse::NO);
	int rep = -1;
	int lit = -1;
	uint32 node = 0; // byte offset of the current node within the node table
	uint32 pos = 0;

	for (uint32 w = 0; w < count; w++) {
		uint32 word = READ_BE_UINT32(bitstream + pos);
		pos += 4;
		for (int bit = 0; bit < 32; bit++) {
			uint32 msb = word >> 31;
			word = (word << 1) & 0xffffffff;
			uint32 nodeOffset = msb ? node + 2 : node;
			uint16 value = READ_BE_UINT16(nodes + nodeOffset);
			if (value < 0x8000 && value <= 0x201) {
				node = value * 4; // internal node
			} else {
				uint8 hi = (value >> 8) & 0xff;
				if (hi != 0xff)
					emitByteAtari(out, rep, lit, hi);
				emitByteAtari(out, rep, lit, value & 0xff);
				node = 0; // leaf -> back to the root
			}
		}
	}

	free(buffer);

	if (out.size() < 2 || READ_BE_UINT16(out.getData()) != 0x601a)
		error("Castle Master (Atari ST) decompression failed (no 0x601A header)");

	debugC(1, kFreescapeDebugParser, "Castle Master (Atari ST): decompressed %d bytes", (int)out.size());
	return new Common::MemoryReadStream(out.getData(), out.size(), DisposeAfterUse::YES);
}

static uint32 getProTrackerModuleSize(Common::SeekableReadStream *file, uint32 offset) {
	int64 oldPos = file->pos();
	uint32 result = 0;

	if (offset + 1084 <= (uint32)file->size()) {
		file->seek(offset + 1080);
		if (file->readUint32BE() == 0x4d2e4b2e) {
			file->seek(offset + 950);
			byte songLength = file->readByte();
			file->readByte();

			if (songLength > 0 && songLength <= 128) {
				byte highestPattern = 0;
				for (int i = 0; i < 128; i++) {
					byte pattern = file->readByte();
					if (i < songLength)
						highestPattern = MAX(highestPattern, pattern);
				}

				uint32 sampleBytes = 0;
				for (int i = 0; i < 31; i++) {
					file->seek(offset + 20 + i * 30 + 22);
					sampleBytes += file->readUint16BE() * 2;
				}

				uint32 moduleSize = 1084 + (highestPattern + 1) * 1024 + sampleBytes;
				if (offset + moduleSize <= (uint32)file->size())
					result = moduleSize;
			}
		}
	}

	file->seek(oldPos);
	return result;
}

extern byte kAmigaCastlePalette[16][3];
extern byte kAmigaCastleRiddlePalette[16][3];

void CastleEngine::loadAssetsAtariFullGame() {
	// The player provides the Copylock-decrypted executable as "M.PRG"; it is
	// still Huffman-packed, so decompress it to obtain the real game binary.
	// The Atari ST build shares the Amiga data *format* (68000, big-endian) but
	// lays everything out at different offsets. These offsets were located by
	// matching the shared world/asset bytes against the Amiga "x" file.
	Common::SeekableReadStream *file = decompressAtari("M.PRG");

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(file, 0x27946, 178);
	loadRiddles(file, 0x28410, 19);

	// Font: 90 characters, 8x8, 4 interleaved bitplanes (identical bytes to the
	// Amiga build, so the Amiga 16-colour palette applies).
	file->seek(0x2f32a);
	Common::Array<Graphics::ManagedSurface *> chars;
	Common::Array<Graphics::ManagedSurface *> charsRiddle;
	for (int i = 0; i < 90; i++) {
		Graphics::ManagedSurface *img = loadFrameFromPlanes(file, 8, 8);
		Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
		imgRiddle->copyFrom(*img);

		chars.push_back(img);
		chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

		charsRiddle.push_back(imgRiddle);
		charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	}
	_font = Font(chars);
	_font.setCharWidth(9);
	_fontRiddle = Font(charsRiddle);
	_fontRiddle.setCharWidth(9);

	// Area database: 87 rooms followed by 3 trailing areas, then the global
	// area 255.
	load8bitBinary(file, 0x33694, 16);
	for (int i = 0; i < 3; i++) {
		Area *newArea = load8bitArea(file, 16);
		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				error("Repeated area ID: %d", newArea->getAreaID());
		} else
			error("Invalid area %d?", i);
	}

	loadPalettes(file, 0x32594);

	// COLOR15 cycling table, terminated by 0xFFFF.
	file->seek(0x27928);
	while (true) {
		uint16 val = file->readUint16BE();
		if (val == 0xFFFF)
			break;
		_gfx->_colorCyclingTable.push_back(val);
	}

	file->seek(0x49284); // Global area 255
	_areaMap[255] = load8bitArea(file, 16);

	// In-game border frame (the "Castle Master" title + castle walls + bottom
	// UI bar surrounding the 3D viewport). 320x200, stored as Atari ST
	// word-interleaved bitplanes; identical artwork to the Amiga build (which
	// keeps it in vertical-planar form), so the Amiga palette applies.
	file->seek(0x4a364);
	_border = loadFrameFromPlanesInterleaved(file, 20, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Mountains panorama (63 words x 22 rows, interleaved) - same bytes/format
	// as the Amiga build.
	file->seek(0x4f24);
	_background = loadFrameFromPlanesInterleaved(file, 63, 22);
	_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Spirit meter, strength-weight and key/eye sprites (shared with the Amiga
	// build, relocated in the Atari binary).
	file->seek(0x55d40);
	_spiritsMeterIndicatorBackgroundFrame = loadFrameFromPlanesInterleaved(file, 5, 10);
	_spiritsMeterIndicatorBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file->seek(0x55ed0);
	_spiritsMeterIndicatorFrame = loadFrameFromPlanesInterleaved(file, 1, 10);
	_spiritsMeterIndicatorFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file->seek(0x569e0);
	for (int i = 0; i < 4; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 1, 14);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_strenghtWeightsFrames.push_back(frame);
	}

	file->seek(0x594a0);
	for (int i = 0; i < 12; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 1, 7);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_keysBorderFrames.push_back(frame);
	}

	// Flag animation: 5 frames x 2 words x 11 rows.
	file->seek(0x5974a);
	for (int i = 0; i < 5; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 2, 11);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_flagFrames.push_back(frame);
	}

	// Riddle frames: a 16-word transparency mask followed by the top/background/
	// bottom frames, masked and drawn with the riddle palette.
	file->seek(0x59ae4);
	uint16 riddleMask[16];
	for (int i = 0; i < 16; i++)
		riddleMask[i] = file->readUint16BE();

	file->seek(0x59b04);
	_riddleTopFrame = loadFrameFromPlanesInterleaved(file, 16, 20);
	_riddleBackgroundFrame = loadFrameFromPlanesInterleaved(file, 16, 1);
	_riddleBottomFrame = loadFrameFromPlanesInterleaved(file, 16, 8);

	Graphics::ManagedSurface *riddleFrames[] = {_riddleTopFrame, _riddleBackgroundFrame, _riddleBottomFrame};
	for (int f = 0; f < 3; f++) {
		Graphics::ManagedSurface *frame = riddleFrames[f];
		for (int y = 0; y < frame->h; y++) {
			for (int x = 0; x < frame->w; x++) {
				int col = x / 16;
				int bit = 15 - (x % 16);
				if (!(riddleMask[col] & (1 << bit)))
					frame->setPixel(x, y, 0);
			}
		}
	}
	_riddleTopFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);
	_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastleRiddlePalette, 16);

	// Castle gate that lifts up at the start of the game: 256x120, built from a
	// 3-bitplane + 1-bit-mask source (24 top tile rows repeated + 19 bottom
	// rows). Identical source bytes to the Amiga build.
	{
		static const int kTopRows = 24;
		static const int kBottomRows = 19;
		static const int kTotalSrcRows = kTopRows + kBottomRows;
		static const int kColumnsPerRow = 16;
		static const int kPixelBytesPerRow = kColumnsPerRow * 6;
		static const int kMaskBytesPerRow = kColumnsPerRow * 2;
		static const int kGateWidth = 256;
		static const int kGateHeight = 120;

		byte pixelData[kTotalSrcRows * kPixelBytesPerRow];
		byte maskData[kTotalSrcRows * kMaskBytesPerRow];
		file->seek(0x56ed0);
		file->read(pixelData, sizeof(pixelData));
		file->seek(0x57ef0);
		file->read(maskData, sizeof(maskData));

		uint32 keyColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
		uint32 paletteColors[8];
		for (int i = 0; i < 8; i++)
			paletteColors[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
				kAmigaCastlePalette[i][0], kAmigaCastlePalette[i][1], kAmigaCastlePalette[i][2]);

		_gameOverBackgroundFrame = new Graphics::ManagedSurface();
		_gameOverBackgroundFrame->create(kGateWidth, kGateHeight, _gfx->_texturePixelFormat);
		_gameOverBackgroundFrame->fillRect(Common::Rect(0, 0, kGateWidth, kGateHeight), keyColor);

		int destRow = 0;
		for (int r = kTopRows - 5; r < kTopRows; r++) {
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[r * kMaskBytesPerRow + col * 2]);
				int pOff = r * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
		for (int block = 0; block < 4; block++) {
			for (int r = 0; r < kTopRows; r++) {
				for (int col = 0; col < kColumnsPerRow; col++) {
					uint16 mask = READ_BE_UINT16(&maskData[r * kMaskBytesPerRow + col * 2]);
					int pOff = r * kPixelBytesPerRow + col * 6;
					uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
					uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
					uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
					for (int bit = 15; bit >= 0; bit--) {
						if (!(mask & (1 << bit))) {
							int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
							_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
						}
					}
				}
				destRow++;
			}
		}
		for (int r = 0; r < kBottomRows; r++) {
			int srcRow = kTopRows + r;
			for (int col = 0; col < kColumnsPerRow; col++) {
				uint16 mask = READ_BE_UINT16(&maskData[srcRow * kMaskBytesPerRow + col * 2]);
				int pOff = srcRow * kPixelBytesPerRow + col * 6;
				uint16 p0 = READ_BE_UINT16(&pixelData[pOff]);
				uint16 p1 = READ_BE_UINT16(&pixelData[pOff + 2]);
				uint16 p2 = READ_BE_UINT16(&pixelData[pOff + 4]);
				for (int bit = 15; bit >= 0; bit--) {
					if (!(mask & (1 << bit))) {
						int color = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1) | (((p2 >> bit) & 1) << 2);
						_gameOverBackgroundFrame->setPixel(col * 16 + (15 - bit), destRow, paletteColors[color]);
					}
				}
			}
			destRow++;
		}
	}

	// TODO(castle-atari): the info menu, menu buttons and movement/sound
	// indicators (drawInfoMenu) use Atari-specific artwork that does not match
	// the Amiga bytes in any plane format, so they are not loaded yet; the info
	// menu is guarded against the missing surfaces. The mouse cursor / crosshair
	// sprites also still need to be located.

	// The full Atari ST binary embeds the same ProTracker module used by the
	// Amiga full game. It starts at TEXT $10F9A / stream offset $10FB6.
	static const uint32 kAtariMusicDataOffset = 0x10fb6;
	uint32 modSize = getProTrackerModuleSize(file, kAtariMusicDataOffset);
	if (modSize > 0) {
		_modData.resize(modSize);
		file->seek(kAtariMusicDataOffset);
		file->read(_modData.data(), modSize);
	}

	delete file;
}

} // End of namespace Freescape
