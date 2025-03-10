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
#include "common/system.h"
#include "graphics/paletteman.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/resource/resource.h"
#include "sci/util.h"
#include "sci/engine/features.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/screen.h"

namespace Sci {

#pragma mark HunkPalette

HunkPalette::HunkPalette(const SciSpan<const byte> &rawPalette) :
	_version(0),
	// The header size in palettes is garbage. In at least KQ7 2.00b and Phant1,
	// the 999.pal sets this value to 0. In most other palettes it is set to 14,
	// but the *actual* size of the header structure used in SSCI is 13, which
	// is reflected by `kHunkPaletteHeaderSize`.
	_numPalettes(rawPalette.getUint8At(kNumPaletteEntriesOffset)),
	_data() {
	assert(_numPalettes == 0 || _numPalettes == 1);
	if (_numPalettes) {
		_data = rawPalette;
		_version = getEntryHeader().version;
	}
}

void HunkPalette::write(SciSpan<byte> &out, const Palette &palette) {
	const uint8 numPalettes = 1;
	const uint16 paletteOffset = kHunkPaletteHeaderSize + 2 * numPalettes;

	out[kNumPaletteEntriesOffset] = numPalettes;
	out[kHunkPaletteHeaderSize + 2] = paletteOffset;

	SciSpan<byte> entry = out.subspan(paletteOffset);
	entry[kEntryStartColorOffset] = 0;
	entry.setUint16SEAt(kEntryNumColorsOffset, ARRAYSIZE(palette.colors));
	entry[kEntryUsedOffset] = 1;
	entry[kEntrySharedUsedOffset] = 0;
	entry.setUint32SEAt(kEntryVersionOffset, 1);

	SciSpan<byte> paletteData = entry.subspan(kEntryHeaderSize);
	for (uint i = 0; i < ARRAYSIZE(palette.colors); ++i) {
		*paletteData++ = palette.colors[i].used;
		*paletteData++ = palette.colors[i].r;
		*paletteData++ = palette.colors[i].g;
		*paletteData++ = palette.colors[i].b;
	}
}

void HunkPalette::setVersion(const uint32 version) const {
	if (_numPalettes != _data.getUint8At(kNumPaletteEntriesOffset)) {
		error("Invalid HunkPalette");
	}

	if (_numPalettes) {
		const EntryHeader header = getEntryHeader();
		if (header.version != _version) {
			error("Invalid HunkPalette");
		}

		byte *palette = const_cast<byte *>(getPalPointer().getUnsafeDataAt(kEntryVersionOffset, sizeof(uint32)));
		WRITE_SCI11ENDIAN_UINT32(palette, version);
		_version = version;
	}
}

const HunkPalette::EntryHeader HunkPalette::getEntryHeader() const {
	const SciSpan<const byte> data(getPalPointer());

	EntryHeader header;
	header.startColor = data.getUint8At(kEntryStartColorOffset);
	header.numColors = data.getUint16SEAt(kEntryNumColorsOffset);
	header.used = data.getUint8At(kEntryUsedOffset);
	header.sharedUsed = data.getUint8At(kEntrySharedUsedOffset);
	header.version = data.getUint32SEAt(kEntryVersionOffset);

	return header;
}

const Palette HunkPalette::toPalette() const {
	Palette outPalette;

	// Set outPalette structures to 0
	for (int16 i = 0; i < ARRAYSIZE(outPalette.mapping); ++i) {
		outPalette.mapping[i] = 0;
	}
	outPalette.timestamp = 0;
	for (int16 i = 0; i < ARRAYSIZE(outPalette.colors); ++i) {
		outPalette.colors[i].used = false;
		outPalette.colors[i].r = 0;
		outPalette.colors[i].g = 0;
		outPalette.colors[i].b = 0;
	}
	for (int16 i = 0; i < ARRAYSIZE(outPalette.intensity); ++i) {
		outPalette.intensity[i] = 0;
	}

	if (_numPalettes) {
		const EntryHeader header = getEntryHeader();
		const uint32 dataSize = header.numColors * (/* RGB */ 3 + (header.sharedUsed ? 0 : 1));
		const byte *data = getPalPointer().getUnsafeDataAt(kEntryHeaderSize, dataSize);

		const int16 end = header.startColor + header.numColors;
		assert(end <= 256);

		if (header.sharedUsed) {
			for (int16 i = header.startColor; i < end; ++i) {
				outPalette.colors[i].used = header.used;
				outPalette.colors[i].r = *data++;
				outPalette.colors[i].g = *data++;
				outPalette.colors[i].b = *data++;
			}
		} else {
			for (int16 i = header.startColor; i < end; ++i) {
				outPalette.colors[i].used = *data++;
				outPalette.colors[i].r = *data++;
				outPalette.colors[i].g = *data++;
				outPalette.colors[i].b = *data++;
			}
		}
	}

	return outPalette;
}

#pragma mark -
#pragma mark Gamma correction tables

static const uint8 gammaTables[GfxPalette32::numGammaTables][256] = {
	{ 0, 2, 3, 5, 6, 7, 9, 10,
	11, 13, 14, 15, 16, 18, 19, 20,
	21, 22, 23, 25, 26, 27, 28, 29,
	30, 32, 33, 34, 35, 36, 37, 38,
	39, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 54, 55, 56,
	57, 58, 59, 60, 61, 62, 63, 64,
	65, 66, 67, 68, 69, 70, 71, 72,
	74, 75, 76, 77, 78, 79, 80, 81,
	82, 83, 84, 85, 86, 87, 88, 89,
	90, 91, 92, 93, 94, 95, 96, 97,
	98, 99, 100, 101, 102, 103, 104, 105,
	106, 107, 108, 109, 110, 111, 112, 113,
	114, 115, 116, 117, 118, 119, 120, 121,
	122, 123, 124, 125, 126, 127, 128, 128,
	129, 130, 131, 132, 133, 134, 135, 136,
	137, 138, 139, 140, 141, 142, 143, 144,
	145, 146, 147, 148, 149, 150, 151, 152,
	153, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 171, 172, 173, 174,
	175, 176, 177, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 186, 187, 188, 189,
	190, 191, 192, 193, 194, 195, 196, 197,
	198, 199, 199, 200, 201, 202, 203, 204,
	205, 206, 207, 208, 209, 210, 211, 211,
	212, 213, 214, 215, 216, 217, 218, 219,
	220, 221, 222, 222, 223, 224, 225, 226,
	227, 228, 229, 230, 231, 232, 232, 233,
	234, 235, 236, 237, 238, 239, 240, 241,
	242, 242, 243, 244, 245, 246, 247, 248,
	249, 250, 251, 251, 252, 253, 254, 255 },

	{ 0, 3, 5, 6, 8, 10, 11, 13,
	14, 16, 17, 19, 20, 22, 23, 24,
	26, 27, 28, 30, 31, 32, 33, 35,
	36, 37, 38, 40, 41, 42, 43, 44,
	46, 47, 48, 49, 50, 51, 53, 54,
	55, 56, 57, 58, 59, 60, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71,
	73, 74, 75, 76, 77, 78, 79, 80,
	81, 82, 83, 84, 85, 86, 87, 88,
	89, 90, 91, 92, 93, 94, 95, 96,
	97, 99, 100, 101, 102, 103, 104, 105,
	106, 107, 108, 108, 109, 110, 111, 112,
	113, 114, 115, 116, 117, 118, 119, 120,
	121, 122, 123, 124, 125, 126, 127, 128,
	129, 130, 131, 132, 133, 134, 135, 136,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	151, 152, 153, 154, 155, 156, 157, 158,
	159, 160, 161, 162, 162, 163, 164, 165,
	166, 167, 168, 169, 170, 171, 172, 172,
	173, 174, 175, 176, 177, 178, 179, 180,
	180, 181, 182, 183, 184, 185, 186, 187,
	188, 188, 189, 190, 191, 192, 193, 194,
	195, 196, 196, 197, 198, 199, 200, 201,
	202, 202, 203, 204, 205, 206, 207, 208,
	209, 209, 210, 211, 212, 213, 214, 215,
	215, 216, 217, 218, 219, 220, 221, 221,
	222, 223, 224, 225, 226, 227, 227, 228,
	229, 230, 231, 232, 233, 233, 234, 235,
	236, 237, 238, 238, 239, 240, 241, 242,
	243, 243, 244, 245, 246, 247, 248, 249,
	249, 250, 251, 252, 253, 254, 254, 255 },

	{ 0, 4, 6, 8, 10, 12, 14, 16,
	18, 19, 21, 23, 24, 26, 27, 29,
	30, 32, 33, 35, 36, 37, 39, 40,
	41, 43, 44, 45, 47, 48, 49, 50,
	52, 53, 54, 55, 57, 58, 59, 60,
	61, 62, 64, 65, 66, 67, 68, 69,
	71, 72, 73, 74, 75, 76, 77, 78,
	79, 81, 82, 83, 84, 85, 86, 87,
	88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 102, 103, 104,
	105, 106, 107, 108, 109, 110, 111, 112,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 146, 147, 148, 149,
	150, 151, 152, 153, 154, 155, 156, 156,
	157, 158, 159, 160, 161, 162, 163, 163,
	164, 165, 166, 167, 168, 169, 170, 170,
	171, 172, 173, 174, 175, 176, 177, 177,
	178, 179, 180, 181, 182, 183, 183, 184,
	185, 186, 187, 188, 188, 189, 190, 191,
	192, 193, 194, 194, 195, 196, 197, 198,
	199, 199, 200, 201, 202, 203, 203, 204,
	205, 206, 207, 208, 208, 209, 210, 211,
	212, 212, 213, 214, 215, 216, 217, 217,
	218, 219, 220, 221, 221, 222, 223, 224,
	225, 225, 226, 227, 228, 229, 229, 230,
	231, 232, 233, 233, 234, 235, 236, 237,
	237, 238, 239, 240, 240, 241, 242, 243,
	244, 244, 245, 246, 247, 247, 248, 249,
	250, 251, 251, 252, 253, 254, 254, 255 },

	{ 0, 5, 9, 11, 14, 16, 19, 21,
	23, 25, 26, 28, 30, 32, 33, 35,
	37, 38, 40, 41, 43, 44, 46, 47,
	49, 50, 52, 53, 54, 56, 57, 58,
	60, 61, 62, 64, 65, 66, 67, 69,
	70, 71, 72, 73, 75, 76, 77, 78,
	79, 80, 82, 83, 84, 85, 86, 87,
	88, 89, 91, 92, 93, 94, 95, 96,
	97, 98, 99, 100, 101, 102, 103, 104,
	105, 106, 107, 108, 109, 110, 111, 112,
	113, 114, 115, 116, 117, 118, 119, 120,
	121, 122, 123, 124, 125, 126, 127, 128,
	129, 130, 131, 132, 133, 134, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 144, 145, 146, 147, 148, 149, 150,
	151, 152, 152, 153, 154, 155, 156, 157,
	158, 158, 159, 160, 161, 162, 163, 164,
	164, 165, 166, 167, 168, 169, 169, 170,
	171, 172, 173, 174, 174, 175, 176, 177,
	178, 179, 179, 180, 181, 182, 183, 183,
	184, 185, 186, 187, 187, 188, 189, 190,
	191, 191, 192, 193, 194, 195, 195, 196,
	197, 198, 199, 199, 200, 201, 202, 202,
	203, 204, 205, 205, 206, 207, 208, 209,
	209, 210, 211, 212, 212, 213, 214, 215,
	215, 216, 217, 218, 218, 219, 220, 221,
	221, 222, 223, 224, 224, 225, 226, 227,
	227, 228, 229, 230, 230, 231, 232, 232,
	233, 234, 235, 235, 236, 237, 238, 238,
	239, 240, 240, 241, 242, 243, 243, 244,
	245, 245, 246, 247, 248, 248, 249, 250,
	250, 251, 252, 252, 253, 254, 255, 255 },

	{ 0, 9, 14, 18, 21, 24, 27, 29,
	32, 34, 37, 39, 41, 43, 45, 47,
	48, 50, 52, 54, 55, 57, 59, 60,
	62, 63, 65, 66, 68, 69, 71, 72,
	73, 75, 76, 77, 79, 80, 81, 83,
	84, 85, 86, 88, 89, 90, 91, 92,
	94, 95, 96, 97, 98, 99, 100, 102,
	103, 104, 105, 106, 107, 108, 109, 110,
	111, 112, 113, 114, 115, 116, 117, 118,
	119, 120, 121, 122, 123, 124, 125, 126,
	127, 128, 129, 130, 131, 132, 133, 134,
	135, 136, 137, 137, 138, 139, 140, 141,
	142, 143, 144, 145, 145, 146, 147, 148,
	149, 150, 151, 151, 152, 153, 154, 155,
	156, 156, 157, 158, 159, 160, 161, 161,
	162, 163, 164, 165, 165, 166, 167, 168,
	169, 169, 170, 171, 172, 173, 173, 174,
	175, 176, 176, 177, 178, 179, 179, 180,
	181, 182, 182, 183, 184, 185, 185, 186,
	187, 188, 188, 189, 190, 191, 191, 192,
	193, 194, 194, 195, 196, 196, 197, 198,
	199, 199, 200, 201, 201, 202, 203, 203,
	204, 205, 206, 206, 207, 208, 208, 209,
	210, 210, 211, 212, 212, 213, 214, 214,
	215, 216, 216, 217, 218, 218, 219, 220,
	220, 221, 222, 222, 223, 224, 224, 225,
	226, 226, 227, 228, 228, 229, 230, 230,
	231, 231, 232, 233, 233, 234, 235, 235,
	236, 237, 237, 238, 238, 239, 240, 240,
	241, 242, 242, 243, 243, 244, 245, 245,
	246, 247, 247, 248, 248, 249, 250, 250,
	251, 251, 252, 253, 253, 254, 254, 255 },

	{ 0, 16, 23, 28, 32, 36, 39, 42,
	45, 48, 50, 53, 55, 58, 60, 62,
	64, 66, 68, 70, 71, 73, 75, 77,
	78, 80, 81, 83, 84, 86, 87, 89,
	90, 92, 93, 94, 96, 97, 98, 100,
	101, 102, 103, 105, 106, 107, 108, 109,
	111, 112, 113, 114, 115, 116, 117, 118,
	119, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 145, 146, 147, 148, 149,
	150, 151, 151, 152, 153, 154, 155, 156,
	156, 157, 158, 159, 160, 160, 161, 162,
	163, 164, 164, 165, 166, 167, 167, 168,
	169, 170, 170, 171, 172, 173, 173, 174,
	175, 176, 176, 177, 178, 179, 179, 180,
	181, 181, 182, 183, 183, 184, 185, 186,
	186, 187, 188, 188, 189, 190, 190, 191,
	192, 192, 193, 194, 194, 195, 196, 196,
	197, 198, 198, 199, 199, 200, 201, 201,
	202, 203, 203, 204, 204, 205, 206, 206,
	207, 208, 208, 209, 209, 210, 211, 211,
	212, 212, 213, 214, 214, 215, 215, 216,
	217, 217, 218, 218, 219, 220, 220, 221,
	221, 222, 222, 223, 224, 224, 225, 225,
	226, 226, 227, 228, 228, 229, 229, 230,
	230, 231, 231, 232, 233, 233, 234, 234,
	235, 235, 236, 236, 237, 237, 238, 238,
	239, 240, 240, 241, 241, 242, 242, 243,
	243, 244, 244, 245, 245, 246, 246, 247,
	247, 248, 248, 249, 249, 250, 250, 251,
	251, 252, 252, 253, 253, 254, 254, 255 }
};

#pragma mark -
#pragma mark GfxPalette32

	GfxPalette32::GfxPalette32(ResourceManager *resMan)
	: _resMan(resMan),

	// Palette versioning
	_version(1),
	_needsUpdate(false),
#ifdef USE_RGB_COLOR
	_hardwarePalette(),
#endif
	_currentPalette(),
	_sourcePalette(),
	_nextPalette(),

	// Palette varying
	_varyStartPalette(nullptr),
	_varyTargetPalette(nullptr),
	_varyFromColor(0),
	_varyToColor(255),
	_varyLastTick(0),
	_varyTime(0),
	_varyDirection(0),
	_varyPercent(0),
	_varyTargetPercent(0),
	_varyNumTimesPaused(0),

	// Palette cycling
	_cycleMap(),

	// Gamma correction
	_gammaLevel(g_sci->_features->useMacGammaLevel() ? 2 : -1),
	_gammaChanged(false) {

	for (int i = 0, len = ARRAYSIZE(_fadeTable); i < len; ++i) {
		_fadeTable[i] = 100;
	}

	loadPalette(999);
}

bool GfxPalette32::loadPalette(const GuiResourceId resourceId) {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);

	if (!palResource) {
		return false;
	}

	const HunkPalette palette(*palResource);
	submit(palette);
	return true;
}

int16 GfxPalette32::matchColor(const uint8 r, const uint8 g, const uint8 b) {
	int16 bestIndex = 0;
	int bestDifference = 0xFFFFF;

	for (int i = 0, channelDifference; i < g_sci->_gfxRemap32->getStartColor(); ++i) {
		int difference = _currentPalette.colors[i].r - r;
		difference *= difference;
		if (bestDifference <= difference) {
			continue;
		}

		channelDifference = _currentPalette.colors[i].g - g;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}

		channelDifference = _currentPalette.colors[i].b - b;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}
		bestDifference = difference;
		bestIndex = i;
	}

	return bestIndex;
}

uint8 GfxPalette32::getPlatformBlack() const {
	return (g_sci->getPlatform() == Common::kPlatformMacintosh) ? 255 : 0;
}

uint8 GfxPalette32::getPlatformWhite() const {
	return (g_sci->getPlatform() == Common::kPlatformMacintosh) ? 0 : 255;
}

void GfxPalette32::submit(const Palette &palette) {
	// If `_needsUpdate` is already set, there is no need to test whether
	// this palette submission causes a change to `_sourcePalette` since it is
	// going to be updated already anyway
	if (_needsUpdate) {
		mergePalette(_sourcePalette, palette);
	} else {
		const Palette oldSourcePalette(_sourcePalette);
		mergePalette(_sourcePalette, palette);

		if (_sourcePalette != oldSourcePalette) {
			++_version;
			_needsUpdate = true;
		}
	}
}

void GfxPalette32::submit(const HunkPalette &hunkPalette) {
	if (hunkPalette.getVersion() == _version) {
		return;
	}

	submit(hunkPalette.toPalette());
	hunkPalette.setVersion(_version);
}

bool GfxPalette32::updateForFrame() {
	applyAll();
	_needsUpdate = false;
	return g_sci->_gfxRemap32->remapAllTables(_nextPalette != _currentPalette);
}

void GfxPalette32::updateFFrame() {
	for (int i = 0; i < ARRAYSIZE(_nextPalette.colors); ++i) {
		_nextPalette.colors[i] = _sourcePalette.colors[i];
	}
	_needsUpdate = false;
	g_sci->_gfxRemap32->remapAllTables(_nextPalette != _currentPalette);
}

void GfxPalette32::updateHardware() {
	if (_currentPalette == _nextPalette && !_gammaChanged) {
		return;
	}

#ifdef USE_RGB_COLOR
	uint8 *bpal = _hardwarePalette;
#else
	uint8 bpal[256 * 3];
#endif

	// HACK: There are resources in a couple of Windows-only games that seem to
	// include bogus palette entries above 236. SSCI does a lot of extra work
	// when running in Windows to shift palettes and rewrite view & pic pixel
	// data on-the-fly to account for the way Windows palettes work, which
	// seems to end up masking the fact that there is some bad palette data.
	// Since only one demo and one game seem to have this problem, we instead
	// "fix" the problem here by ignoring attempts to send high palette entries
	// to the backend. This makes those high pixels render black, which seems to
	// match what would happen in the original interpreter, and saves us from
	// having to clutter up the engine with a bunch of palette shifting garbage.
	//
	// This workaround also handles Mac games, as they use 236 for black to avoid
	//  conflicting with the operating system's palette which uses 0 for white.
	int maxIndex = ARRAYSIZE(_currentPalette.colors) - 2;
	if (g_sci->getGameId() == GID_HOYLE5 ||
		(g_sci->getGameId() == GID_GK2 && g_sci->isDemo()) ||
		g_sci->getPlatform() == Common::kPlatformMacintosh) {
		maxIndex = 235;
	}

	for (int i = 0; i <= maxIndex; ++i) {
		_currentPalette.colors[i] = _nextPalette.colors[i];

		// All color entries MUST be copied, not just "used" entries, otherwise
		// uninitialised memory from bpal makes its way into the system palette.
		// This would not normally be a problem, except that games sometimes use
		// unused palette entries. e.g. Phant1 title screen references palette
		// entries outside its own palette, so will render garbage colors where
		// the game expects them to be black
		if (_gammaLevel == -1) {
			bpal[i * 3    ] = _currentPalette.colors[i].r;
			bpal[i * 3 + 1] = _currentPalette.colors[i].g;
			bpal[i * 3 + 2] = _currentPalette.colors[i].b;
		} else {
			bpal[i * 3    ] = gammaTables[_gammaLevel][_currentPalette.colors[i].r];
			bpal[i * 3 + 1] = gammaTables[_gammaLevel][_currentPalette.colors[i].g];
			bpal[i * 3 + 2] = gammaTables[_gammaLevel][_currentPalette.colors[i].b];
		}
	}

#ifndef USE_RGB_COLOR
	// When creating a raw palette on the stack, any skipped area of the palette
	// needs to be blacked out or else it will contain garbage memory
	memset(bpal + (maxIndex + 1) * 3, 0, (255 - maxIndex - 1) * 3);
#endif

	// The last color must always be white
	bpal[255 * 3    ] = 255;
	bpal[255 * 3 + 1] = 255;
	bpal[255 * 3 + 2] = 255;

	// If the system is in a high color mode, which can happen during video
	// playback, attempting to send the palette to OSystem is illegal and will
	// result in a crash
	if (g_system->getScreenFormat().bytesPerPixel == 1) {
		g_system->getPaletteManager()->setPalette(bpal, 0, 256);
	}

	_gammaChanged = false;
}

Palette GfxPalette32::getPaletteFromResource(const GuiResourceId resourceId) const {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);

	if (!palResource) {
		error("Could not load vary palette %d", resourceId);
	}

	const HunkPalette rawPalette(*palResource);
	return rawPalette.toPalette();
}

void GfxPalette32::mergePalette(Palette &to, const Palette &from) {
	// All colors MUST be copied, even index 255, despite the fact that games
	// cannot actually change index 255 (it is forced to white when generating
	// the hardware palette in updateHardware). While this causes some
	// additional unnecessary source palette invalidations, not doing it breaks
	// some badly programmed rooms, like room 6400 in Phant1 (see Trac#9788).
	// (Note, however, that that specific glitch is fully fixed by ignoring a
	// bad palette in the CelObjView constructor)
	for (int i = 0; i < ARRAYSIZE(to.colors); ++i) {
		if (from.colors[i].used) {
			to.colors[i] = from.colors[i];
		}
	}
}

void GfxPalette32::applyAll() {
	applyVary();
	applyCycles();
	applyFade();
}

#pragma mark -
#pragma mark Varying

void GfxPalette32::setVary(const Palette &target, const int16 percent, const int32 ticks, const int16 fromColor, const int16 toColor) {
	setTarget(target);
	setVaryTime(percent, ticks);

	if (fromColor > -1) {
		_varyFromColor = fromColor;
	}
	if (toColor > -1) {
		assert(toColor < 256);
		_varyToColor = toColor;
	}
}

void GfxPalette32::setVaryPercent(const int16 percent, const int32 ticks) {
	if (_varyTargetPalette) {
		setVaryTime(percent, ticks);
	}

	// SSCI had two additional parameters for this function to change the
	// `_varyFromColor`, but they were always hardcoded to be ignored
}

void GfxPalette32::setVaryTime(const int32 time) {
	if (_varyTargetPalette) {
		setVaryTime(_varyTargetPercent, time);
	}
}

void GfxPalette32::setVaryTime(const int16 percent, const int32 ticks) {
	_varyLastTick = g_sci->getTickCount();
	if (!ticks || _varyPercent == percent) {
		_varyDirection = 0;
		_varyTargetPercent = _varyPercent = percent;
	} else {
		_varyTime = ticks / (percent - _varyPercent);
		_varyTargetPercent = percent;

		if (_varyTime > 0) {
			_varyDirection = 1;
		} else if (_varyTime < 0) {
			_varyDirection = -1;
			_varyTime = -_varyTime;
		} else {
			_varyDirection = 0;
			_varyTargetPercent = _varyPercent = percent;
		}
	}
}

void GfxPalette32::varyOff() {
	_varyNumTimesPaused = 0;
	_varyPercent = _varyTargetPercent = 0;
	_varyFromColor = 0;
	_varyToColor = 255;
	_varyDirection = 0;
	_varyTargetPalette.reset();
	_varyStartPalette.reset();
}

void GfxPalette32::varyPause() {
	_varyDirection = 0;
	++_varyNumTimesPaused;
}

void GfxPalette32::varyOn() {
	if (_varyNumTimesPaused > 0) {
		--_varyNumTimesPaused;
	}

	if (_varyTargetPalette && _varyNumTimesPaused == 0) {
		if (_varyPercent != _varyTargetPercent && _varyTime != 0) {
			_varyDirection = (_varyTargetPercent - _varyPercent > 0) ? 1 : -1;
		} else {
			_varyPercent = _varyTargetPercent;
		}
	}
}

void GfxPalette32::setTarget(const Palette &palette) {
	_varyTargetPalette.reset(new Palette(palette));
}

void GfxPalette32::setStart(const Palette &palette) {
	_varyStartPalette.reset(new Palette(palette));
}

void GfxPalette32::mergeStart(const Palette &palette) {
	if (_varyStartPalette) {
		mergePalette(*_varyStartPalette, palette);
	} else {
		_varyStartPalette.reset(new Palette(palette));
	}
}

void GfxPalette32::mergeTarget(const Palette &palette) {
	if (_varyTargetPalette) {
		mergePalette(*_varyTargetPalette, palette);
	} else {
		_varyTargetPalette.reset(new Palette(palette));
	}
}

void GfxPalette32::applyVary() {
	const uint32 now = g_sci->getTickCount();
	while ((int32)(now - _varyLastTick) > _varyTime && _varyDirection != 0) {
		_varyLastTick += _varyTime;

		if (_varyPercent == _varyTargetPercent) {
			_varyDirection = 0;
		}

		_varyPercent += _varyDirection;
	}

	if (_varyPercent == 0 || !_varyTargetPalette) {
		for (int i = 0; i < ARRAYSIZE(_nextPalette.colors); ++i) {
			if (_varyStartPalette && i >= _varyFromColor && i <= _varyToColor) {
				_nextPalette.colors[i] = _varyStartPalette->colors[i];
			} else {
				_nextPalette.colors[i] = _sourcePalette.colors[i];
			}
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(_nextPalette.colors); ++i) {
			if (i >= _varyFromColor && i <= _varyToColor) {
				Color targetColor = _varyTargetPalette->colors[i];
				Color sourceColor;

				if (_varyStartPalette) {
					sourceColor = _varyStartPalette->colors[i];
				} else {
					sourceColor = _sourcePalette.colors[i];
				}

				Color computedColor;

				int color;
				color = targetColor.r - sourceColor.r;
				computedColor.r = ((color * _varyPercent) / 100) + sourceColor.r;
				color = targetColor.g - sourceColor.g;
				computedColor.g = ((color * _varyPercent) / 100) + sourceColor.g;
				color = targetColor.b - sourceColor.b;
				computedColor.b = ((color * _varyPercent) / 100) + sourceColor.b;
				computedColor.used = sourceColor.used;

				_nextPalette.colors[i] = computedColor;
			}
			else {
				_nextPalette.colors[i] = _sourcePalette.colors[i];
			}
		}
	}
}

void GfxPalette32::kernelPalVarySet(const GuiResourceId paletteId, const int16 percent, const int32 ticks, const int16 fromColor, const int16 toColor) {
	Palette palette;

	if (getSciVersion() == SCI_VERSION_3 && paletteId == 0xFFFF) {
		palette = _currentPalette;
		assert(fromColor >= 0 && fromColor < 256);
		assert(toColor >= 0 && toColor < 256);
		// While palette varying is normally inclusive of `toColor`, the
		// palette inversion code in SSCI excludes `toColor`, and RAMA room
		// 6201 requires this or else parts of the game's UI get inverted
		for (int i = fromColor; i < toColor; ++i) {
			palette.colors[i].r = ~palette.colors[i].r;
			palette.colors[i].g = ~palette.colors[i].g;
			palette.colors[i].b = ~palette.colors[i].b;
		}
	} else {
		palette = getPaletteFromResource(paletteId);
	}

	setVary(palette, percent, ticks, fromColor, toColor);
}

void GfxPalette32::kernelPalVaryMergeTarget(const GuiResourceId paletteId) {
	const Palette palette = getPaletteFromResource(paletteId);
	mergeTarget(palette);
}

void GfxPalette32::kernelPalVarySetTarget(const GuiResourceId paletteId) {
	const Palette palette = getPaletteFromResource(paletteId);
	setTarget(palette);
}

void GfxPalette32::kernelPalVarySetStart(const GuiResourceId paletteId) {
	const Palette palette = getPaletteFromResource(paletteId);
	setStart(palette);
}

void GfxPalette32::kernelPalVaryMergeStart(const GuiResourceId paletteId) {
	const Palette palette = getPaletteFromResource(paletteId);
	mergeStart(palette);
}

void GfxPalette32::kernelPalVaryPause(const bool pause) {
	if (pause) {
		varyPause();
	} else {
		varyOn();
	}
}

#pragma mark -
#pragma mark Cycling

void GfxPalette32::setCycle(const uint8 fromColor, const uint8 toColor, const int16 direction, const int16 delay) {
	assert(fromColor < toColor);

	PalCycler *cycler = getCycler(fromColor);

	if (cycler != nullptr) {
		clearCycleMap(fromColor, cycler->numColorsToCycle);
	} else {
		for (int i = 0; i < kNumCyclers; ++i) {
			if (!_cyclers[i]) {
				cycler = new PalCycler;
				_cyclers[i].reset(cycler);
				break;
			}
		}
	}

	// If there are no free cycler slots, SSCI overrides the first oldest cycler
	// that it finds, where "oldest" is determined by the difference between the
	// tick and now
	if (cycler == nullptr) {
		const uint32 now = g_sci->getTickCount();
		uint32 minUpdateDelta = 0xFFFFFFFF;

		for (int i = 0; i < kNumCyclers; ++i) {
			PalCyclerOwner &candidate = _cyclers[i];

			const uint32 updateDelta = now - candidate->lastUpdateTick;
			if (updateDelta < minUpdateDelta) {
				minUpdateDelta = updateDelta;
				cycler = candidate.get();
			}
		}

		clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
	}

	uint16 numColorsToCycle = toColor - fromColor;
	if (g_sci->_features->hasMidPaletteCode()) {
		numColorsToCycle += 1;
	}
	cycler->fromColor = fromColor;
	cycler->numColorsToCycle = numColorsToCycle;
	cycler->currentCycle = fromColor;
	cycler->direction = direction < 0 ? kPalCycleBackward : kPalCycleForward;
	cycler->delay = delay;
	cycler->lastUpdateTick = g_sci->getTickCount();
	cycler->numTimesPaused = 0;

	setCycleMap(fromColor, numColorsToCycle);
}

void GfxPalette32::doCycle(const uint8 fromColor, const int16 speed) {
	PalCycler *const cycler = getCycler(fromColor);
	if (cycler != nullptr) {
		cycler->lastUpdateTick = g_sci->getTickCount();
		updateCycler(*cycler, speed);
	}
}

void GfxPalette32::cycleOn(const uint8 fromColor) {
	PalCycler *const cycler = getCycler(fromColor);
	if (cycler != nullptr && cycler->numTimesPaused > 0) {
		--cycler->numTimesPaused;
	}
}

void GfxPalette32::cyclePause(const uint8 fromColor) {
	PalCycler *const cycler = getCycler(fromColor);
	if (cycler != nullptr) {
		++cycler->numTimesPaused;
	}
}

void GfxPalette32::cycleAllOn() {
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler && cycler->numTimesPaused > 0) {
			--cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleAllPause() {
	// SSCI did not check for null pointers in the palette cyclers pointer array
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler) {
			// This seems odd, because currentCycle is 0..numColorsPerCycle,
			// but fromColor is 0..255. When applyAllCycles runs, the values
			// end up back in range
			cycler->currentCycle = cycler->fromColor;
		}
	}

	applyAllCycles();

	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler) {
			++cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleOff(const uint8 fromColor) {
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler && cycler->fromColor == fromColor) {
			clearCycleMap(fromColor, cycler->numColorsToCycle);
			_cyclers[i].reset();
			break;
		}
	}
}

void GfxPalette32::cycleAllOff() {
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler) {
			clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
			_cyclers[i].reset();
		}
	}
}

void GfxPalette32::updateCycler(PalCycler &cycler, const int16 speed) {
	int16 currentCycle = cycler.currentCycle;
	const uint16 numColorsToCycle = cycler.numColorsToCycle;

	if (cycler.direction == kPalCycleBackward) {
		currentCycle = (currentCycle - (speed % numColorsToCycle)) + numColorsToCycle;
	} else {
		currentCycle = currentCycle + speed;
	}

	cycler.currentCycle = currentCycle % numColorsToCycle;
}

void GfxPalette32::clearCycleMap(const uint16 fromColor, const uint16 numColorsToClear) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *const lastEntry = _cycleMap + numColorsToClear;
	while (mapEntry < lastEntry) {
		*mapEntry++ = false;
	}
}

void GfxPalette32::setCycleMap(const uint16 fromColor, const uint16 numColorsToSet) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *const lastEntry = _cycleMap + numColorsToSet;
	while (mapEntry < lastEntry) {
		if (*mapEntry != false) {
			error("Cycles intersect");
		}
		*mapEntry++ = true;
	}
}

PalCycler *GfxPalette32::getCycler(const uint16 fromColor) {
	for (int cyclerIndex = 0; cyclerIndex < kNumCyclers; ++cyclerIndex) {
		PalCyclerOwner &cycler = _cyclers[cyclerIndex];
		if (cycler && cycler->fromColor == fromColor) {
			return cycler.get();
		}
	}

	return nullptr;
}

void GfxPalette32::applyAllCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(paletteCopy));

	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (cycler) {
			cycler->currentCycle = (cycler->currentCycle + 1) % cycler->numColorsToCycle;
			for (int j = 0; j < cycler->numColorsToCycle; j++) {
				_nextPalette.colors[cycler->fromColor + j] = paletteCopy[cycler->fromColor + (cycler->currentCycle + j) % cycler->numColorsToCycle];
			}
		}
	}
}

void GfxPalette32::applyCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(paletteCopy));

	const uint32 now = g_sci->getTickCount();
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCyclerOwner &cycler = _cyclers[i];
		if (!cycler) {
			continue;
		}

		if (cycler->delay != 0 && cycler->numTimesPaused == 0) {
			while ((cycler->delay + cycler->lastUpdateTick) < now) {
				updateCycler(*cycler, 1);
				cycler->lastUpdateTick += cycler->delay;
			}
		}

		for (int j = 0; j < cycler->numColorsToCycle; j++) {
			_nextPalette.colors[cycler->fromColor + j] = paletteCopy[cycler->fromColor + (cycler->currentCycle + j) % cycler->numColorsToCycle];
		}
	}
}

#pragma mark -
#pragma mark Fading

void GfxPalette32::setFade(const uint16 percent, const uint8 fromColor, uint16 toColor) {
	if (fromColor > toColor) {
		return;
	}

	// Some game scripts (like SQ6 Sierra logo and main menu) incorrectly call
	// setFade with toColor set to 256
	if (toColor > 255) {
		toColor = 255;
	}

	for (int i = fromColor; i <= toColor; i++) {
		_fadeTable[i] = percent;
	}
}

void GfxPalette32::fadeOff() {
	setFade(100, 0, 255);
}

void GfxPalette32::applyFade() {
	for (int i = 0; i < ARRAYSIZE(_fadeTable); ++i) {
		if (_fadeTable[i] == 100) {
			continue;
		}

		Color &color = _nextPalette.colors[i];

		color.r = MIN(255, (uint16)color.r * _fadeTable[i] / 100);
		color.g = MIN(255, (uint16)color.g * _fadeTable[i] / 100);
		color.b = MIN(255, (uint16)color.b * _fadeTable[i] / 100);
	}
}

} // End of namespace Sci
