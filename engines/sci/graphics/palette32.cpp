/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/system.h"
#include "graphics/palette.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/util.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/screen.h"

namespace Sci {

#pragma mark HunkPalette

HunkPalette::HunkPalette(byte *rawPalette) :
	_version(0),
	// NOTE: The header size in palettes is garbage. In at least KQ7 2.00b and
	// Phant1, the 999.pal sets this value to 0. In most other palettes it is
	// set to 14, but the *actual* size of the header structure used in SSCI is
	// 13, which is reflected by `kHunkPaletteHeaderSize`.
	// _headerSize(rawPalette[0]),
	_numPalettes(rawPalette[10]),
	_data(nullptr) {
	assert(_numPalettes == 0 || _numPalettes == 1);
	if (_numPalettes) {
		_data = rawPalette;
		_version = getEntryHeader().version;
	}
}

void HunkPalette::setVersion(const uint32 version) {
	if (_numPalettes != _data[10]) {
		error("Invalid HunkPalette");
	}

	if (_numPalettes) {
		const EntryHeader header = getEntryHeader();
		if (header.version != _version) {
			error("Invalid HunkPalette");
		}

		WRITE_SCI11ENDIAN_UINT32(getPalPointer() + kEntryVersionOffset, version);
		_version = version;
	}
}

const HunkPalette::EntryHeader HunkPalette::getEntryHeader() const {
	const byte *const data = getPalPointer();

	EntryHeader header;
	header.startColor = data[10];
	header.numColors = READ_SCI11ENDIAN_UINT16(data + 14);
	header.used = data[16];
	header.sharedUsed = data[17];
	header.version = READ_SCI11ENDIAN_UINT32(data + kEntryVersionOffset);

	return header;
}

const Palette HunkPalette::toPalette() const {
	Palette outPalette;

	for (int16 i = 0; i < ARRAYSIZE(outPalette.colors); ++i) {
		outPalette.colors[i].used = false;
		outPalette.colors[i].r = 0;
		outPalette.colors[i].g = 0;
		outPalette.colors[i].b = 0;
	}

	if (_numPalettes) {
		const EntryHeader header = getEntryHeader();
		byte *data = getPalPointer() + kEntryHeaderSize;

		int16 end = header.startColor + header.numColors;
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
#pragma mark GfxPalette32

GfxPalette32::GfxPalette32(ResourceManager *resMan)
	: _resMan(resMan),
	// Palette versioning
	_version(1),
	_needsUpdate(false),
	_currentPalette(),
	_sourcePalette(),
	_nextPalette(),
	// Clut
	_clutTable(nullptr),
	// Palette varying
	_varyStartPalette(nullptr),
	_varyTargetPalette(nullptr),
	_varyFromColor(0),
	_varyToColor(255),
	_varyLastTick(0),
	_varyTime(0),
	_varyDirection(0),
	_varyTargetPercent(0),
	_varyNumTimesPaused(0),
	// Palette cycling
	_cyclers(),
	_cycleMap() {
	_varyPercent = _varyTargetPercent;
	for (int i = 0, len = ARRAYSIZE(_fadeTable); i < len; ++i) {
		_fadeTable[i] = 100;
	}

	loadPalette(999);
}

GfxPalette32::~GfxPalette32() {
#ifdef ENABLE_SCI3_GAMES
	unloadClut();
#endif
	varyOff();
	cycleAllOff();
}

inline void mergePaletteInternal(Palette *const to, const Palette *const from) {
	// The last color is always white, so it is not copied.
	// (Some palettes try to set the last color, which causes
	// churning in the palettes when they are merged)
	for (int i = 0, len = ARRAYSIZE(to->colors) - 1; i < len; ++i) {
		if (from->colors[i].used) {
			to->colors[i] = from->colors[i];
		}
	}
}

void GfxPalette32::submit(const Palette &palette) {
	const Palette oldSourcePalette(_sourcePalette);
	mergePaletteInternal(&_sourcePalette, &palette);

	if (!_needsUpdate && _sourcePalette != oldSourcePalette) {
		++_version;
		_needsUpdate = true;
	}
}

void GfxPalette32::submit(HunkPalette &hunkPalette) {
	if (hunkPalette.getVersion() == _version) {
		return;
	}

	const Palette oldSourcePalette(_sourcePalette);
	const Palette palette = hunkPalette.toPalette();
	mergePaletteInternal(&_sourcePalette, &palette);

	if (!_needsUpdate && oldSourcePalette != _sourcePalette) {
		++_version;
		_needsUpdate = true;
	}

	hunkPalette.setVersion(_version);
}

bool GfxPalette32::loadPalette(const GuiResourceId resourceId) {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);

	if (!palResource) {
		return false;
	}

	HunkPalette palette(palResource->data);
	submit(palette);
	return true;
}

int16 GfxPalette32::matchColor(const uint8 r, const uint8 g, const uint8 b) {
	int16 bestIndex = 0;
	int bestDifference = 0xFFFFF;
	int difference;

	for (int i = 0, channelDifference; i < g_sci->_gfxRemap32->getStartColor(); ++i) {
		difference = _currentPalette.colors[i].r - r;
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

void GfxPalette32::updateHardware(const bool updateScreen) {
	if (_currentPalette == _nextPalette) {
		return;
	}

	byte bpal[3 * 256];

	for (int i = 0; i < ARRAYSIZE(_currentPalette.colors) - 1; ++i) {
		_currentPalette.colors[i] = _nextPalette.colors[i];

		// NOTE: If the brightness option in the user configuration file is set,
		// SCI engine adjusts palette brightnesses here by mapping RGB values to values
		// in some hard-coded brightness tables. There is no reason on modern hardware
		// to implement this, unless it is discovered that some game uses a non-standard
		// brightness setting by default

		// All color entries MUST be copied, not just "used" entries, otherwise
		// uninitialised memory from bpal makes its way into the system palette.
		// This would not normally be a problem, except that games sometimes use
		// unused palette entries. e.g. Phant1 title screen references palette
		// entries outside its own palette, so will render garbage colors where
		// the game expects them to be black
		bpal[i * 3    ] = _currentPalette.colors[i].r;
		bpal[i * 3 + 1] = _currentPalette.colors[i].g;
		bpal[i * 3 + 2] = _currentPalette.colors[i].b;
	}

	if (g_sci->getPlatform() != Common::kPlatformMacintosh) {
		// The last color must always be white
		bpal[255 * 3    ] = 255;
		bpal[255 * 3 + 1] = 255;
		bpal[255 * 3 + 2] = 255;
	} else {
		bpal[255 * 3    ] = 0;
		bpal[255 * 3 + 1] = 0;
		bpal[255 * 3 + 2] = 0;
	}

	g_system->getPaletteManager()->setPalette(bpal, 0, 256);
	if (updateScreen) {
		g_sci->getEventManager()->updateScreen();
	}
}

void GfxPalette32::applyAll() {
	applyVary();
	applyCycles();
	applyFade();
}

#pragma mark -
#pragma mark Colour look-up

#ifdef ENABLE_SCI3_GAMES
bool GfxPalette32::loadClut(uint16 clutId) {
	// loadClut() will load a color lookup table from a clu file and set
	// the palette found in the file. This is to be used with Phantasmagoria 2.

	unloadClut();

	Common::String filename = Common::String::format("%d.clu", clutId);
	Common::File clut;

	if (!clut.open(filename) || clut.size() != 0x10000 + 236 * 3)
		return false;

	// Read in the lookup table
	// It maps each RGB565 color to a palette index
	_clutTable = new byte[0x10000];
	clut.read(_clutTable, 0x10000);

	Palette pal;
	memset(&pal, 0, sizeof(Palette));

	// Setup 1:1 mapping
	for (int i = 0; i < 256; i++) {
		pal.mapping[i] = i;
	}

	// Now load in the palette
	for (int i = 1; i <= 236; i++) {
		pal.colors[i].used = 1;
		pal.colors[i].r = clut.readByte();
		pal.colors[i].g = clut.readByte();
		pal.colors[i].b = clut.readByte();
	}

	set(&pal, true);
	setOnScreen();
	return true;
}

byte GfxPalette32::matchClutColor(uint16 color) {
	// Match a color in RGB565 format to a palette index based on the loaded CLUT
	assert(_clutTable);
	return _clutTable[color];
}

void GfxPalette32::unloadClut() {
	// This will only unload the actual table, but not reset any palette
	delete[] _clutTable;
	_clutTable = nullptr;
}
#endif

#pragma mark -
#pragma mark Varying

inline Palette GfxPalette32::getPaletteFromResourceInternal(const GuiResourceId resourceId) const {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, resourceId), false);

	if (!palResource) {
		error("Could not load vary palette %d", resourceId);
	}

	HunkPalette rawPalette(palResource->data);
	return rawPalette.toPalette();
}

inline void GfxPalette32::setVaryTimeInternal(const int16 percent, const int time) {
	_varyLastTick = g_sci->getTickCount();
	if (!time || _varyPercent == percent) {
		_varyDirection = 0;
		_varyTargetPercent = _varyPercent = percent;
	} else {
		_varyTime = time / (percent - _varyPercent);
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

void GfxPalette32::kernelPalVarySet(const GuiResourceId paletteId, const int16 percent, const int time, const int16 fromColor, const int16 toColor) {
	Palette palette = getPaletteFromResourceInternal(paletteId);
	setVary(&palette, percent, time, fromColor, toColor);
}

void GfxPalette32::kernelPalVaryMergeTarget(GuiResourceId paletteId) {
	Palette palette = getPaletteFromResourceInternal(paletteId);
	mergeTarget(&palette);
}

void GfxPalette32::kernelPalVarySetTarget(GuiResourceId paletteId) {
	Palette palette = getPaletteFromResourceInternal(paletteId);
	setTarget(&palette);
}

void GfxPalette32::kernelPalVarySetStart(GuiResourceId paletteId) {
	Palette palette = getPaletteFromResourceInternal(paletteId);
	setStart(&palette);
}

void GfxPalette32::kernelPalVaryMergeStart(GuiResourceId paletteId) {
	Palette palette = getPaletteFromResourceInternal(paletteId);
	mergeStart(&palette);
}

void GfxPalette32::kernelPalVaryPause(bool pause) {
	if (pause) {
		varyPause();
	} else {
		varyOn();
	}
}

void GfxPalette32::setVary(const Palette *const target, const int16 percent, const int time, const int16 fromColor, const int16 toColor) {
	setTarget(target);
	setVaryTimeInternal(percent, time);

	if (fromColor > -1) {
		_varyFromColor = fromColor;
	}
	if (toColor > -1) {
		assert(toColor < 256);
		_varyToColor = toColor;
	}
}

void GfxPalette32::setVaryPercent(const int16 percent, const int time, const int16 fromColor, const int16 fromColorAlternate) {
	if (_varyTargetPalette != nullptr) {
		setVaryTimeInternal(percent, time);
	}

	// This looks like a mistake in the actual SCI engine (both SQ6 and Lighthouse);
	// the values are always hardcoded to -1 in kPalVary, so this code can never
	// actually be executed
	if (fromColor > -1) {
		_varyFromColor = fromColor;
	}
	if (fromColorAlternate > -1) {
		_varyFromColor = fromColorAlternate;
	}
}

int16 GfxPalette32::getVaryPercent() const {
	return ABS(_varyPercent);
}

void GfxPalette32::varyOff() {
	_varyNumTimesPaused = 0;
	_varyPercent = _varyTargetPercent = 0;
	_varyFromColor = 0;
	_varyToColor = 255;
	_varyDirection = 0;

	if (_varyTargetPalette != nullptr) {
		delete _varyTargetPalette;
		_varyTargetPalette = nullptr;
	}

	if (_varyStartPalette != nullptr) {
		delete _varyStartPalette;
		_varyStartPalette = nullptr;
	}
}

void GfxPalette32::mergeTarget(const Palette *const palette) {
	if (_varyTargetPalette != nullptr) {
		mergePaletteInternal(_varyTargetPalette, palette);
	} else {
		_varyTargetPalette = new Palette(*palette);
	}
}

void GfxPalette32::varyPause() {
	_varyDirection = 0;
	++_varyNumTimesPaused;
}

void GfxPalette32::varyOn() {
	if (_varyNumTimesPaused > 0) {
		--_varyNumTimesPaused;
	}

	if (_varyTargetPalette != nullptr && _varyNumTimesPaused == 0 && _varyPercent != _varyTargetPercent) {
		if (_varyTime == 0) {
			_varyPercent = _varyTargetPercent;
		} else if (_varyTargetPercent < _varyPercent) {
			_varyDirection = -1;
		} else {
			_varyDirection = 1;
		}
	}
}

void GfxPalette32::setVaryTime(const int time) {
	if (_varyTargetPalette == nullptr) {
		return;
	}

	setVaryTimeInternal(_varyTargetPercent, time);
}

void GfxPalette32::setTarget(const Palette *const palette) {
	if (_varyTargetPalette != nullptr) {
		delete _varyTargetPalette;
	}

	_varyTargetPalette = new Palette(*palette);
}

void GfxPalette32::setStart(const Palette *const palette) {
	if (_varyStartPalette != nullptr) {
		delete _varyStartPalette;
	}

	_varyStartPalette = new Palette(*palette);
}

void GfxPalette32::mergeStart(const Palette *const palette) {
	if (_varyStartPalette != nullptr) {
		mergePaletteInternal(_varyStartPalette, palette);
	} else {
		_varyStartPalette = new Palette(*palette);
	}
}

void GfxPalette32::applyVary() {
	while (g_sci->getTickCount() - _varyLastTick > (uint32)_varyTime && _varyDirection != 0) {
		_varyLastTick += _varyTime;

		if (_varyPercent == _varyTargetPercent) {
			_varyDirection = 0;
		}

		_varyPercent += _varyDirection;
	}

	if (_varyPercent == 0 || _varyTargetPalette == nullptr) {
		for (int i = 0, len = ARRAYSIZE(_nextPalette.colors); i < len; ++i) {
			if (_varyStartPalette != nullptr && i >= _varyFromColor && i <= _varyToColor) {
				_nextPalette.colors[i] = _varyStartPalette->colors[i];
			} else {
				_nextPalette.colors[i] = _sourcePalette.colors[i];
			}
		}
	} else {
		for (int i = 0, len = ARRAYSIZE(_nextPalette.colors); i < len; ++i) {
			if (i >= _varyFromColor && i <= _varyToColor) {
				Color targetColor = _varyTargetPalette->colors[i];
				Color sourceColor;

				if (_varyStartPalette != nullptr) {
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

#pragma mark -
#pragma mark Cycling

inline void GfxPalette32::clearCycleMap(const uint16 fromColor, const uint16 numColorsToClear) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *lastEntry = _cycleMap + numColorsToClear;
	while (mapEntry < lastEntry) {
		*mapEntry++ = false;
	}
}

inline void GfxPalette32::setCycleMap(const uint16 fromColor, const uint16 numColorsToSet) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *lastEntry = _cycleMap + numColorsToSet;
	while (mapEntry < lastEntry) {
		if (*mapEntry != false) {
			error("Cycles intersect");
		}
		*mapEntry++ = true;
	}
}

inline PalCycler *GfxPalette32::getCycler(const uint16 fromColor) {
	const int numCyclers = ARRAYSIZE(_cyclers);

	for (int cyclerIndex = 0; cyclerIndex < numCyclers; ++cyclerIndex) {
		PalCycler *cycler = _cyclers[cyclerIndex];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			return cycler;
		}
	}

	return nullptr;
}

inline void doCycleInternal(PalCycler *cycler, const int16 speed) {
	int16 currentCycle = cycler->currentCycle;
	const uint16 numColorsToCycle = cycler->numColorsToCycle;

	if (cycler->direction == 0) {
		currentCycle = (currentCycle - (speed % numColorsToCycle)) + numColorsToCycle;
	} else {
		currentCycle = currentCycle + speed;
	}

	cycler->currentCycle = (uint8) (currentCycle % numColorsToCycle);
}

void GfxPalette32::setCycle(const uint8 fromColor, const uint8 toColor, const int16 direction, const int16 delay) {
	assert(fromColor < toColor);

	int cyclerIndex;
	const int numCyclers = ARRAYSIZE(_cyclers);

	PalCycler *cycler = getCycler(fromColor);

	if (cycler != nullptr) {
		clearCycleMap(fromColor, cycler->numColorsToCycle);
	} else {
		for (cyclerIndex = 0; cyclerIndex < numCyclers; ++cyclerIndex) {
			if (_cyclers[cyclerIndex] == nullptr) {
				cycler = new PalCycler;
				_cyclers[cyclerIndex] = cycler;
				break;
			}
		}
	}

	// SCI engine overrides the first oldest cycler that it finds where
	// “oldest” is determined by the difference between the tick and now
	if (cycler == nullptr) {
		const uint32 now = g_sci->getTickCount();
		uint32 minUpdateDelta = 0xFFFFFFFF;

		for (cyclerIndex = 0; cyclerIndex < numCyclers; ++cyclerIndex) {
			PalCycler *candidate = _cyclers[cyclerIndex];

			const uint32 updateDelta = now - candidate->lastUpdateTick;
			if (updateDelta < minUpdateDelta) {
				minUpdateDelta = updateDelta;
				cycler = candidate;
			}
		}

		clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
	}

	const uint16 numColorsToCycle = 1 + ((uint8) toColor) - fromColor;
	cycler->fromColor = (uint8) fromColor;
	cycler->numColorsToCycle = (uint8) numColorsToCycle;
	cycler->currentCycle = (uint8) fromColor;
	cycler->direction = direction < 0 ? PalCycleBackward : PalCycleForward;
	cycler->delay = delay;
	cycler->lastUpdateTick = g_sci->getTickCount();
	cycler->numTimesPaused = 0;

	setCycleMap(fromColor, numColorsToCycle);
}

void GfxPalette32::doCycle(const uint8 fromColor, const int16 speed) {
	PalCycler *cycler = getCycler(fromColor);
	if (cycler != nullptr) {
		cycler->lastUpdateTick = g_sci->getTickCount();
		doCycleInternal(cycler, speed);
	}
}

void GfxPalette32::cycleOn(const uint8 fromColor) {
	PalCycler *cycler = getCycler(fromColor);
	if (cycler != nullptr && cycler->numTimesPaused > 0) {
		--cycler->numTimesPaused;
	}
}

void GfxPalette32::cyclePause(const uint8 fromColor) {
	PalCycler *cycler = getCycler(fromColor);
	if (cycler != nullptr) {
		++cycler->numTimesPaused;
	}
}

void GfxPalette32::cycleAllOn() {
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr && cycler->numTimesPaused > 0) {
			--cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleAllPause() {
	// NOTE: The original engine did not check for null pointers in the
	// palette cyclers pointer array.
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr) {
			// This seems odd, because currentCycle is 0..numColorsPerCycle,
			// but fromColor is 0..255. When applyAllCycles runs, the values
			// end up back in range
			cycler->currentCycle = cycler->fromColor;
		}
	}

	applyAllCycles();

	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr) {
			++cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleOff(const uint8 fromColor) {
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			clearCycleMap(fromColor, cycler->numColorsToCycle);
			delete cycler;
			_cyclers[i] = nullptr;
			break;
		}
	}
}

void GfxPalette32::cycleAllOff() {
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr) {
			clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
			delete cycler;
			_cyclers[i] = nullptr;
		}
	}
}

void GfxPalette32::applyAllCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(Color) * 256);

	for (int cyclerIndex = 0, numCyclers = ARRAYSIZE(_cyclers); cyclerIndex < numCyclers; ++cyclerIndex) {
		PalCycler *cycler = _cyclers[cyclerIndex];
		if (cycler != nullptr) {
			cycler->currentCycle = (uint8) ((((int) cycler->currentCycle) + 1) % cycler->numColorsToCycle);
			// Disassembly was not fully evaluated to verify this is exactly the same
			// as the code from applyCycles, but it appeared to be at a glance
			for (int j = 0; j < cycler->numColorsToCycle; j++) {
				_nextPalette.colors[cycler->fromColor + j] = paletteCopy[cycler->fromColor + (cycler->currentCycle + j) % cycler->numColorsToCycle];
			}
		}
	}
}

void GfxPalette32::applyCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(Color) * 256);

	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler == nullptr) {
			continue;
		}

		if (cycler->delay != 0 && cycler->numTimesPaused == 0) {
			while ((cycler->delay + cycler->lastUpdateTick) < g_sci->getTickCount()) {
				doCycleInternal(cycler, 1);
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

// NOTE: There are some game scripts (like SQ6 Sierra logo and main menu) that call
// setFade with numColorsToFade set to 256, but other parts of the engine like
// processShowStyleNone use 255 instead of 256. It is not clear if this is because
// the last palette entry is intentionally left unmodified, or if this is a bug
// in the engine. It certainly seems confused because all other places that accept
// color ranges typically receive values in the range of 0–255.
void GfxPalette32::setFade(uint16 percent, uint8 fromColor, uint16 numColorsToFade) {
	if (fromColor > numColorsToFade) {
		return;
	}

	assert(numColorsToFade <= ARRAYSIZE(_fadeTable));

	for (int i = fromColor; i < numColorsToFade; i++)
		_fadeTable[i] = percent;
}

void GfxPalette32::fadeOff() {
	setFade(100, 0, 256);
}

void GfxPalette32::applyFade() {
	for (int i = 0; i < ARRAYSIZE(_fadeTable); ++i) {
		if (_fadeTable[i] == 100)
			continue;

		Color &color = _nextPalette.colors[i];

		color.r = MIN(255, (uint16)color.r * _fadeTable[i] / 100);
		color.g = MIN(255, (uint16)color.g * _fadeTable[i] / 100);
		color.b = MIN(255, (uint16)color.b * _fadeTable[i] / 100);
	}
}

} // End of namespace Sci
