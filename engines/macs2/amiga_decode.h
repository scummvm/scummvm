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

#ifndef MACS2_AMIGA_DECODE_H
#define MACS2_AMIGA_DECODE_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Macs2 {

/**
 * Amiga MXOO object body layout (after the 12-byte MXOO header):
 *   +0x00..0x0B  padding
 *   +0x0C        signature 0x0101
 *   +0x0E        21 x uint32BE slot offsets (0 / 0xFFFFFFFF = empty)
 *   +0x62        uint32BE offset of extra/portrait section (often end of anims)
 *
 * Each anim slot is planar 6-plane frame data with a short BE header.
 * Script bytecode is identical to DOS (LE operands). Strings are plaintext
 * with uint16BE length prefixes (no XOR cipher).
 *
 * Game object index = Amiga OO resource id + 1 (OO_0000 -> object 1).
 */
struct AmigaMxooInfo {
	uint32 scriptOffset = 0;
	uint32 stringOffset = 0;
	uint32 slotOffsets[21];
	uint32 extraOffset = 0;
	uint32 bodyOffset = 12; // start of body within MXOO
	uint32 bodySize = 0;

	AmigaMxooInfo() {
		for (uint i = 0; i < 21; i++)
			slotOffsets[i] = 0xFFFFFFFF;
	}
};

struct AmigaAnimSlotInfo {
	uint16 frameCount = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint16 seqPos = 0;
	uint16 repeatCounter = 0;
	uint16 loopStart = 0;
	uint16 headerHint = 0;
	uint32 pixelOffset = 0; // absolute offset in MXOO of first planar byte
	uint32 headerSize = 0;  // bytes from slot start to pixel data
	bool valid = false;
};

bool parseAmigaMxoo(const byte *mxoo, uint32 mxooSize, AmigaMxooInfo &out);
bool inspectAmigaAnimSlot(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeOffset, AmigaAnimSlotInfo &out);

/** Decode one frame of planar Amiga anim data to chunky 8bpp (color planes 0..4). */
bool decodeAmigaPlanarFrame(const byte *planar, uint16 width, uint16 height, uint16 frameIndex,
							uint16 frameCount, Common::Array<byte> &outPixels);

/**
 * Convert an Amiga anim slot into a DOS-compatible animation blob so the
 * existing AnimBlobView / renderer path can consume it.
 */
bool convertAmigaAnimSlotToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeOffset,
								   Common::Array<byte> &outBlob);

/** Extract script bytecode (without the MXOO script section header). */
bool extractAmigaScript(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outScript);

/**
 * Extract Amiga string entries (u16BE length + plaintext), skipping the
 * MXOO string-section header. Offsets used by scripts are relative to this block.
 */
bool extractAmigaStringBlock(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outStrings);

/** Convert a simple planar MXOO sprite (cursor/icon) into a 1-frame DOS anim blob. */
bool convertAmigaSimpleSpriteToDosBlob(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outBlob);

/**
 * Convert Amiga dialogue portrait atlas (body+0x62) into a DOS anim blob.
 * Layout: 240x80x6 separated planar = three 80x80 faces (D5=0xF0, D3/D4/D6=0x50,
 * 6 plane blit in animateDialoguePortrait @ 0022f79c). Color from planes 0..4.
 * Pixels keep Amiga COLOR indices 0..31 (playfield copper); demo portraits use
 * only COLOR17..31 from the copper high bank / MXIN chrome ramp.
 */
bool convertAmigaPortraitAtlasToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeExtraOffset,
										Common::Array<byte> &outBlob);

/** Decompressed MXMM chunk0 screen buffer size (6x8000 planes + copper block). */
enum : uint32 {
	kAmigaSceneScreenSize = 54432, // 0xD4A0
	kAmigaSceneCopperOffset = 0xBB80,
	kAmigaSceneCopperSize = 0x1920, // 16 base colors + 200x16 line colors
	kAmigaScenePlaneBytes = 8000 // 40x200
};
enum : uint16 {
	kAmigaSceneWidth = 320,
	kAmigaSceneHeight = 200,
	kAmigaScenePlanes = 6 // BPLCON0 = 0x6200 -> EHB
};

/**
 * Decode MXMM scene package chunk0 into chunky 8bpp 320x200 pixels and an RGB8
 * palette (up to 256 entries).
 *
 * Palette layout (matches Amiga copper / sprite drawing):
 * - indices 0..31: COLOR00..31 from the copper base block + first scanline
 * - indices 32..63: Extra HalfBrite of 0..31 (BPLCON0 0x6200)
 * - indices 64..*: extra colors needed for per-scanline copper differences
 *
 * Character/OO sprites use planes 0..4 against COLOR00..31, so 0..31 must stay
 * stable Amiga hardware colors.
 */
bool decodeAmigaMxmmSceneBackground(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outPixels,
									byte outPaletteRgb[768],
									uint &outColorCount);

/**
 * Extract scene script + string block from an MXMM package trailer.
 *
 * After the size-prefixed chunks (BG / maps / MXCC / ...), the trailer is:
 *   [optional u32 zero markers]
 *   u32BE scriptSize
 *   u32BE unknown (ignored)
 *   script[scriptSize]          - DOS-identical LE bytecode
 *   [optional] u32BE stringBytes + u16BE-length plaintext entries
 *
 * Ghidra: load_scene_mxmm reads this after planar BG / map chunks.
 * Script-visible scene ids are MM_resource_id + 1 (changeScene subtracts 1
 * before MM lookup; curScene is set to mmId+1 after load).
 */
bool extractAmigaMxmmSceneScript(const byte *mxmm, uint32 mxmmSize,
								 Common::Array<byte> &outScript,
								 Common::Array<byte> &outStrings);

/**
 * Extract scene walk / palette scalars from the MXMM trailer tables.
 *
 * Ghidra load_scene_mxmm @ 00221de8 reads 10 bytes after pathfinding (0xA2) and
 * hotspot (0x22) tables into DAT_002376da..e2 (five BE u16s), matching DOS
 * scene+0x51FD..0x5205:
 *   [0] walk depth threshold Y
 *   [1] walk depth scale factor
 *   [2] walk base speed percent
 *   [3] scene palette mode
 *   [4] palette darken percent
 *
 * Without these, ScummVM's DOS walkAlongPath formula collapses to 1 px/frame
 * when the Amiga native path never loads MCS scene metadata.
 */
bool extractAmigaMxmmSceneWalkParams(const byte *mxmm, uint32 mxmmSize,
									 uint16 &outWalkDepthThresholdY,
									 uint16 &outWalkDepthScaleFactor,
									 uint16 &outWalkBaseSpeedPct,
									 uint16 &outScenePaletteMode,
									 uint16 &outPaletteDarkenPercent);

/**
 * One pathfinding graph node from the MXMM trailer 0xA2 table
 * (Ghidra load_scene_mxmm @ 00221d9a -> 00248794).
 *
 * Layout (BE), matching DOS MCS 16x10-byte nodes:
 *   u16 x, u16 y, u8 adj[4], u16 unused
 * Prefixed by u16BE active node count (DOS scene+0x51F7).
 */
struct AmigaPathfindingNode {
	uint16 x = 0;
	uint16 y = 0;
	byte adjacent[4] = {0, 0, 0, 0};
	uint16 numConnections = 0;
};

/**
 * Extract pathfinding node graph from the MXMM trailer (0xA2 bytes after a
 * skipped u16). Always yields 16 node slots; outNumPoints is the active count.
 * Without this, Amiga scenes keep _numPathfindingPoints==0, calculatePath fails,
 * walks cancel early, and waitForWalk completes at the stuck position.
 */
bool extractAmigaMxmmScenePathfinding(const byte *mxmm, uint32 mxmmSize,
									  uint16 &outNumPoints,
									  Common::Array<AmigaPathfindingNode> &outNodes);

/**
 * Decode MXMM map chunks (320x200) after chunk0.
 * Native order: Map0=depth (g_pSceneMap0Buffer), Map1=walkability
 * (g_pSceneMap1Buffer). Map2 holds MXCC (hotspot RLE), not a 64000 shadow -
 * use extractAmigaMxmmMxccHotspotMap for that. A third 64000 PP20 (rare) is
 * returned as outShadow when present.
 */
bool extractAmigaMxmmSceneMaps(const byte *mxmm, uint32 mxmmSize,
							   Common::Array<byte> &outPathfinding,
							   Common::Array<byte> &outDepth,
							   Common::Array<byte> &outShadow);

/**
 * Extract hotspot color table from the MXMM trailer (0x22 bytes after pathfinding).
 * Layout matches DOS scene+0x50C3: u16BE numHotspots + 0x20 bytes color words
 * (color in the first byte of each pair / low byte once stored as uint16 on LE).
 * MM_0004: num=7, colors 249,254,250,251,253,252,248 - same as DOS scene 5.
 */
bool extractAmigaMxmmSceneHotspotColors(const byte *mxmm, uint32 mxmmSize,
										uint16 &outNumHotspots,
										Common::Array<uint16> &outColorTable);

/**
 * Decode MXCC chunk (Map2 / g_pSceneMap2Buffer) to a 320x200 hotspot pixel map.
 * Ghidra decodeMxccRunLengthAt @ 00233590: per-row RLE with marker at MXCC+5,
 * row offsets at +0x0A (200xu16BE), row data at +0x19A + offset.
 * MM_0004 decodes pixel-identical to DOS scene 5 hotspot RLE.
 * No separate 4th 64000 map chunk exists in the Amiga demo format.
 */
bool extractAmigaMxmmMxccHotspotMap(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outHotspotMap);

/**
 * True if MXMM has a non-empty MXAA overlay-anim blob (native tickMxaaOverlayAnims).
 * Demo MM_0004/MM_0040 have size 0 - no consumer wired yet.
 */
bool amigaMxmmHasMxaaOverlay(const byte *mxmm, uint32 mxmmSize);

/**
 * One decoded MXFF glyph (chunky 8bpp, color 0 = transparent).
 * Ghidra drawText @ 00224492: charmap at +0x0E, widths at +0x8C, atlas at +0x10A.
 */
struct AmigaMxffGlyph {
	byte ascii = 0;
	uint16 width = 0;
	uint16 height = 0;
	Common::Array<byte> pixels; // width * height
};

/**
 * Decode native Amiga MXFF font (FF_0000) into chunky glyphs for GlyphData.
 *
 * Header (BE): magic MXFF, ver=1 at +4, blit height at +6, atlas rows at +8,
 * atlas width-pixels at +0xA (row bytes = value>>3), planes=6 at +0xC.
 * Glyph index map: +0x0E (0x7E bytes for chars 0x20..).
 * Advance widths: +0x8C (0x7E bytes).
 * Planar atlas: +0x10A, rowBytes * atlasRows * 6 planes; cell X = glyphIndex * 16.
 */
bool decodeAmigaMxffFont(const byte *mxff, uint32 mxffSize, Common::Array<AmigaMxffGlyph> &outGlyphs);

/**
 * Extract unsigned 8-bit PCM (+ optional rate) from an MXOS container for MacsAudioStream.
 * Returns false if magic/version/layout is not recognized.
 * outRateHz is set from Paula period when present (NTSC clock / period), else 8000.
 */
bool extractAmigaMxosPcm(const byte *mxos, uint32 mxosSize, Common::Array<byte> &outPcm, uint16 &outRateHz);

} // End of namespace Macs2

#endif // MACS2_AMIGA_DECODE_H
