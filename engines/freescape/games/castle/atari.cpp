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

#include "freescape/copylock.h"
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
// M.PRG is a GEMDOS executable (magic 0x601A) whose DATA segment holds a
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
// The Atari ST release wraps the packed stream in a GEMDOS executable, while the
// Amiga compilation stores it on its own, hence the offset.
Common::SeekableReadStream *CastleEngine::decompressCastle(Common::SeekableReadStream *source, uint32 packedOffset) {
	int fileSize = source->size();
	byte *buffer = (byte *)malloc(fileSize);
	source->read(buffer, fileSize);

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

Common::SeekableReadStream *CastleEngine::decompressAtari(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename))
		error("Failed to open '%s'", filename.toString().c_str());

	// The original file is wrapped in a Copylock protection, which is removed
	// here; a file that was already decrypted by hand is taken as it is
	Common::SeekableReadStream *unwrapped = Copylock::unwrap(&file);
	Common::SeekableReadStream *source = unwrapped ? unwrapped : (Common::SeekableReadStream *)&file;

	byte header[6];
	source->read(header, sizeof(header));
	source->seek(0);
	if (READ_BE_UINT16(header) != 0x601a)
		error("'%s' is not a GEMDOS executable", filename.toString().c_str());

	// the packed stream follows the TEXT segment, i.e. it is the DATA segment
	Common::SeekableReadStream *result = decompressCastle(source, 0x1c + READ_BE_UINT32(header + 2));
	delete unwrapped;
	return result;
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

// Castle Master, located by matching the shared asset bytes against the Amiga
// "x" file.
const CastleAtariLayout kAtariCastleLayout = {
	0x27946, 178, 0x28410, 0x27928, 0x2f32a, 0x32594, 0x33694,
	3, 0x49284, 0x4a364, 0x04f24, 0x55d40, 0x55ed0,
	0x55f20, 0x569d0, 0x56bb0, 0x56ed0, 0x57ef0, 0x594a0, 0x5974a,
	0x59ae4, 0x59b04, 0x10fb6, 0x31adc, 0x5a994, 0x58918
};

// "The Crypt", the second disc of "Castle Master & The Crypt". It carries the
// interface artwork and the music module byte for byte, 0xf512 lower in the
// image; what differs is the world, and it has no riddles (the one left in the
// data is Castle Master's).
const CastleAtariLayout kAtariCryptLayout = {
	0x273ec, 165, 0, 0x2731a, 0x2e366, 0x315d0, 0x326d0,
	0, 0, 0x3ae52, 0x04914, 0x4682e, 0x469be,
	0x46a0e, 0x474be, 0x4769e, 0x479be, 0x489de, 0x49f8e, 0x4a238,
	0x4a5d2, 0x4a5f2, 0x109a6, 0x30b18, 0x4b482, 0x49406
};

// L.PRG, the loader program that launches The Crypt, copies a palette and then
// 1000 * 32 bytes straight to the screen, from program $6f4 and $770.
void CastleEngine::loadAtariLoadingScreen() {
	Common::File file;
	if (!file.open("L.PRG"))
		return;

	byte palette[16][3];
	file.seek(0x710);
	for (int i = 0; i < 16; i++) {
		uint16 color = file.readUint16BE();
		for (int c = 0; c < 3; c++) {
			byte v = (color >> (8 - 4 * c)) & 7;
			palette[i][c] = v * 255 / 7;
		}
	}

	file.seek(0x78c);
	_title = loadFrameFromPlanesInterleaved(&file, 20, 200);
	_title->convertToInPlace(_gfx->_texturePixelFormat, (byte *)palette, 16);
}

void CastleEngine::loadAssetsAtariFullGame() {
	// The player provides the Copylock-decrypted executable as "M.PRG" ("C.PRG"
	// for The Crypt, which carries no Copylock); it is still Huffman-packed, so
	// decompress it to obtain the real game binary. The Atari ST build shares
	// the Amiga data *format* (68000, big-endian) at different offsets.
	const CastleAtariLayout *layout = isCastleMaster2() ? &kAtariCryptLayout : &kAtariCastleLayout;
	Common::SeekableReadStream *file = decompressAtari(isCastleMaster2() ? "C.PRG" : "M.PRG");

	if (isCastleMaster2())
		loadAtariLoadingScreen();

	_viewArea = Common::Rect(40, 29, 280, 154);
	loadMessagesVariableSize(file, layout->messages, layout->messageCount);
	if (layout->riddles)
		loadRiddles(file, layout->riddles, 19);

	// Font: 90 characters, 8x8, 4 interleaved bitplanes (identical bytes to the
	// Amiga build, so the Amiga 16-colour palette applies).
	file->seek(layout->fonts);
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

	// Castle Master has 87 rooms followed by 3 trailing areas and then the
	// global area 255; The Crypt lists all 49 of its areas, 255 included.
	load8bitBinary(file, layout->areaDB, 16);
	for (int i = 0; i < layout->extraAreas; i++) {
		Area *newArea = load8bitArea(file, 16);
		if (newArea) {
			if (!_areaMap.contains(newArea->getAreaID()))
				_areaMap[newArea->getAreaID()] = newArea;
			else
				error("Repeated area ID: %d", newArea->getAreaID());
		} else
			error("Invalid area %d?", i);
	}

	loadPalettes(file, layout->palettes);

	// COLOR15 cycling table, terminated by 0xFFFF.
	file->seek(layout->colorCycling);
	while (true) {
		uint16 val = file->readUint16BE();
		if (val == 0xFFFF)
			break;
		_gfx->_colorCyclingTable.push_back(val);
	}

	if (layout->area255) {
		file->seek(layout->area255);
		_areaMap[255] = load8bitArea(file, 16);
	}

	// In-game border frame (the game title + castle walls + bottom UI bar
	// surrounding the 3D viewport). 320x200, stored as Atari ST
	// word-interleaved bitplanes; identical artwork to the Amiga build (which
	// keeps it in vertical-planar form), so the Amiga palette applies.
	file->seek(layout->border);
	_border = loadFrameFromPlanesInterleaved(file, 20, 200);
	_border->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Mountains panorama (63 words x 22 rows, interleaved) - same bytes/format
	// as the Amiga build.
	file->seek(layout->mountains);
	_background = loadFrameFromPlanesInterleaved(file, 63, 22);
	_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Spirit meter, strength-weight and key/eye sprites (shared with the Amiga
	// build, relocated in the Atari binary).
	file->seek(layout->spiritMeterBg);
	_spiritsMeterIndicatorBackgroundFrame = loadFrameFromPlanesInterleaved(file, 5, 10);
	_spiritsMeterIndicatorBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	file->seek(layout->spiritMeter);
	_spiritsMeterIndicatorFrame = loadFrameFromPlanesInterleaved(file, 1, 10);
	_spiritsMeterIndicatorFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	// Weight discs of the strength barbell: 4 frames of 1 word x 15 rows,
	// followed by the 5 word x 3 row shaft.
	file->seek(layout->weights);
	for (int i = 0; i < 4; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 1, 15);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_strenghtWeightsFrames.push_back(frame);
	}

	file->seek(layout->bar);
	_strenghtBarFrame = loadFrameFromPlanesInterleaved(file, 5, 3);
	_strenghtBarFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);

	loadThunderFramesAmiga(file, layout->thunder);

	// Ten collected-key sprites, 2 words x 16 rows each. The blit routine
	// indexes them by key ID with a 0x100 stride (prog $483a).
	file->seek(layout->keys);
	for (int i = 0; i < 10; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 2, 16);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_keysBorderFrames.push_back(frame);
	}

	// Flag animation: 5 frames x 2 words x 11 rows.
	file->seek(layout->flag);
	for (int i = 0; i < 5; i++) {
		Graphics::ManagedSurface *frame = loadFrameFromPlanesInterleaved(file, 2, 11);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)kAmigaCastlePalette, 16);
		_flagFrames.push_back(frame);
	}

	// Riddle frames: a 16-word transparency mask followed by the top/background/
	// bottom frames, masked and drawn with the riddle palette.
	file->seek(layout->riddleMask);
	uint16 riddleMask[16];
	for (int i = 0; i < 16; i++)
		riddleMask[i] = file->readUint16BE();

	file->seek(layout->riddleTop);
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
		file->seek(layout->gatePixels);
		file->read(pixelData, sizeof(pixelData));
		file->seek(layout->gateMask);
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

	// Same command table as the Amiga, and the bank the Amiga ships as the
	// external "cmsnds2" is embedded here instead
	_sound = loadSoundsAtariCastle(file, layout->soundTable, 36, layout->soundBank);

	// The full Atari ST binary embeds the same ProTracker module used by the
	// Amiga full game; The Crypt ships that module again, unchanged.
	uint32 modSize = getProTrackerModuleSize(file, layout->mod);
	if (modSize > 0) {
		_modData.resize(modSize);
		file->seek(layout->mod);
		file->read(_modData.data(), modSize);
	}

	delete file;
}

} // End of namespace Freescape
