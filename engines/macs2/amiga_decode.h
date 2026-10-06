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
#include "graphics/palette.h"

namespace Macs2 {

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
void amiga12ToVga6(uint16 rgb, byte &r6, byte &g6, byte &b6);
byte remapAmigaCopperIndexToStableUi(byte color);
bool decodeAmigaPlanarFrame(const byte *planar, uint16 width, uint16 height, uint16 frameIndex,
							uint16 frameCount, Common::Array<byte> &outPixels);
bool convertAmigaAnimSlotToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeOffset,
								   Common::Array<byte> &outBlob);
bool extractAmigaScript(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outScript);
bool extractAmigaStringBlock(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outStrings);
bool convertAmigaSimpleSpriteToDosBlob(const byte *mxoo, uint32 mxooSize, Common::Array<byte> &outBlob);
bool convertAmigaPortraitAtlasToDosBlob(const byte *mxoo, uint32 mxooSize, uint32 bodyRelativeExtraOffset,
										Common::Array<byte> &outBlob);

enum : uint32 {
	kAmigaSceneWidth = 320,
	kAmigaSceneHeight = 200,
	kAmigaScenePlanes = 6,
	kAmigaSceneScreenSize = 54432,
	kAmigaSceneCopperOffset = 0xBB80,
	kAmigaSceneCopperSize = 0x1920, // 16 base colors + 200x16 line colors
	kAmigaSceneCopperColorCount = 16,
	kAmigaSceneCopperLineBytes = kAmigaSceneCopperColorCount * 2, // 16 x u16BE
	kAmigaSceneCopperBaseBytes = kAmigaSceneCopperLineBytes, // static COLOR0..15 before per-line colors
	kAmigaScenePlaneBytes = 8000, // 40x200
	kAmigaColorRegisterCount = 32,
	kAmigaEhbPaletteCount = 64,

	/** MXMM + version u16 + pad u16 + scene id u16. */
	kAmigaMxmmHeaderSize = 10,
	kAmigaMxmmMinSize = kAmigaMxmmHeaderSize + 4,

	kAmigaMxmmWordSize = 2, // u16BE count / preamble
	kAmigaMxmmTrailerPreambleSize = kAmigaMxmmWordSize,
	kAmigaMxmmPathfindingNodeCount = 16,
	kAmigaMxmmPathfindingNodeSize = 10,
	kAmigaMxmmPathfindingMaxAdj = 4,
	kAmigaMxmmPathfindingNodeXOffset = 0,
	kAmigaMxmmPathfindingNodeYOffset = 2,
	kAmigaMxmmPathfindingNodeAdjOffset = 4,
	kAmigaMxmmPathfindingNodeConnOffset = 8,
	kAmigaMxmmPathfindingTableSize = kAmigaMxmmWordSize +
									 kAmigaMxmmPathfindingNodeCount * kAmigaMxmmPathfindingNodeSize, // 0xA2
	kAmigaMxmmHotspotColorBytes = 0x20,
	kAmigaMxmmHotspotTableSize = kAmigaMxmmWordSize + kAmigaMxmmHotspotColorBytes, // 0x22
	kAmigaMxmmWalkPaletteSize = 10, // 5 x u16BE
	kAmigaMxmmWalkDepthThresholdYOffset = 0,
	kAmigaMxmmWalkDepthScaleFactorOffset = 2,
	kAmigaMxmmWalkBaseSpeedPctOffset = 4,
	kAmigaMxmmWalkScenePaletteModeOffset = 6,
	kAmigaMxmmWalkPaletteDarkenPercentOffset = 8,
	kAmigaMxmmPathfindingToHotspotOffset = kAmigaMxmmPathfindingTableSize + kAmigaMxmmTrailerPreambleSize,
	kAmigaMxmmPathfindingToWalkOffset = kAmigaMxmmPathfindingToHotspotOffset + kAmigaMxmmHotspotTableSize,
	kAmigaMxmmTrailerTablesSize = kAmigaMxmmTrailerPreambleSize + kAmigaMxmmPathfindingTableSize +
								  kAmigaMxmmTrailerPreambleSize + kAmigaMxmmHotspotTableSize +
								  kAmigaMxmmWalkPaletteSize,

	kAmigaMxccVersionOffset = 4,
	kAmigaMxccMarkerOffset = 5,
	kAmigaMxccRowTableOffset = 0x0A,
	kAmigaMxccRowDataOffset = kAmigaMxccRowTableOffset + kAmigaSceneHeight * 2 // 0x19A
};

bool decodeAmigaMxmmSceneBackground(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outPixels,
									Graphics::Palette &outPalette,
									uint &outColorCount,
									Common::Array<byte> &outLineCopperPal);

bool extractAmigaMxmmSceneScript(const byte *mxmm, uint32 mxmmSize,
								 Common::Array<byte> &outScript,
								 Common::Array<byte> &outStrings);

bool extractAmigaMxmmSceneWalkParams(const byte *mxmm, uint32 mxmmSize,
									 uint16 &outWalkDepthThresholdY,
									 uint16 &outWalkDepthScaleFactor,
									 uint16 &outWalkBaseSpeedPct,
									 uint16 &outScenePaletteMode,
									 uint16 &outPaletteDarkenPercent);

struct AmigaPathfindingNode {
	uint16 x = 0;
	uint16 y = 0;
	byte adjacent[kAmigaMxmmPathfindingMaxAdj] = {};
	uint16 numConnections = 0;
};

bool extractAmigaMxmmScenePathfinding(const byte *mxmm, uint32 mxmmSize,
									  uint16 &outNumPoints,
									  Common::Array<AmigaPathfindingNode> &outNodes);

bool extractAmigaMxmmSceneMaps(const byte *mxmm, uint32 mxmmSize,
							   Common::Array<byte> &outPathfinding,
							   Common::Array<byte> &outDepth,
							   Common::Array<byte> &outShadow);

bool extractAmigaMxmmSceneHotspotColors(const byte *mxmm, uint32 mxmmSize,
										uint16 &outNumHotspots,
										Common::Array<uint16> &outColorTable);

bool extractAmigaMxmmMxccHotspotMap(const byte *mxmm, uint32 mxmmSize,
									Common::Array<byte> &outHotspotMap);

bool amigaMxmmHasMxaaOverlay(const byte *mxmm, uint32 mxmmSize);

struct AmigaMxffGlyph {
	byte ascii = 0;
	uint16 width = 0;
	uint16 height = 0;
	Common::Array<byte> pixels; // width * height
};

bool decodeAmigaMxffFont(const byte *mxff, uint32 mxffSize, Common::Array<AmigaMxffGlyph> &outGlyphs);

bool extractAmigaMxosPcm(const byte *mxos, uint32 mxosSize, Common::Array<byte> &outPcm, uint16 &outRateHz);

} // End of namespace Macs2

#endif // MACS2_AMIGA_DECODE_H
