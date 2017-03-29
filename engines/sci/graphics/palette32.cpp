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
#include "sci/engine/features.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/screen.h"

namespace Sci {

#pragma mark HunkPalette

HunkPalette::HunkPalette(const SciSpan<const byte> &rawPalette) :
	_version(0),
	// NOTE: The header size in palettes is garbage. In at least KQ7 2.00b and
	// Phant1, the 999.pal sets this value to 0. In most other palettes it is
	// set to 14, but the *actual* size of the header structure used in SSCI is
	// 13, which is reflected by `kHunkPaletteHeaderSize`.
	// _headerSize(rawPalette[0]),
	_numPalettes(rawPalette.getUint8At(10)),
	_data() {
	assert(_numPalettes == 0 || _numPalettes == 1);
	if (_numPalettes) {
		_data = rawPalette;
		_version = getEntryHeader().version;
	}
}

void HunkPalette::setVersion(const uint32 version) const {
	if (_numPalettes != _data.getUint8At(10)) {
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
	header.startColor = data.getUint8At(10);
	header.numColors = data.getUint16SEAt(14);
	header.used = data.getUint8At(16);
	header.sharedUsed = data.getUint8At(17);
	header.version = data.getUint32SEAt(kEntryVersionOffset);

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
#pragma mark GfxPalette32

GfxPalette32::GfxPalette32(ResourceManager *resMan)
	: _resMan(resMan),

	// Palette versioning
	_version(1),
	_needsUpdate(false),
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

void GfxPalette32::submit(const Palette &palette) {
	const Palette oldSourcePalette(_sourcePalette);
	mergePalette(_sourcePalette, palette);

	if (!_needsUpdate && _sourcePalette != oldSourcePalette) {
		++_version;
		_needsUpdate = true;
	}
}

void GfxPalette32::submit(const HunkPalette &hunkPalette) {
	if (hunkPalette.getVersion() == _version) {
		return;
	}

	const Palette oldSourcePalette(_sourcePalette);
	const Palette palette = hunkPalette.toPalette();
	mergePalette(_sourcePalette, palette);

	if (!_needsUpdate && oldSourcePalette != _sourcePalette) {
		++_version;
		_needsUpdate = true;
	}

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

void GfxPalette32::updateHardware(const bool updateScreen) {
	if (_currentPalette == _nextPalette) {
		return;
	}

	byte bpal[3 * 256];

	for (int i = 0; i < ARRAYSIZE(_currentPalette.colors) - 1; ++i) {
		_currentPalette.colors[i] = _nextPalette.colors[i];

		// NOTE: If the brightness option in the user configuration file is set,
		// SCI engine adjusts palette brightnesses here by mapping RGB values to
		// values in some hard-coded brightness tables. There is no reason on
		// modern hardware to implement this, unless it is discovered that some
		// game uses a non-standard brightness setting by default

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
		g_system->updateScreen();
	}
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
	// The last color is always white in SCI, so it is not copied. (Some
	// palettes, particularly in KQ7, try to set the last color, which causes
	// unnecessary palette updates since the last color is forced by SSCI to a
	// specific value)
	for (int i = 0; i < ARRAYSIZE(to.colors) - 1; ++i) {
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
	if (_varyTargetPalette != nullptr) {
		setVaryTime(percent, ticks);
	}

	// NOTE: SSCI had two additional parameters for this function to change the
	// `_varyFromColor`, but they were always hardcoded to be ignored
}

void GfxPalette32::setVaryTime(const int32 time) {
	if (_varyTargetPalette != nullptr) {
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

	if (_varyTargetPalette != nullptr) {
		delete _varyTargetPalette;
		_varyTargetPalette = nullptr;
	}

	if (_varyStartPalette != nullptr) {
		delete _varyStartPalette;
		_varyStartPalette = nullptr;
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

	if (_varyTargetPalette != nullptr && _varyNumTimesPaused == 0) {
		if (_varyPercent != _varyTargetPercent && _varyTime != 0) {
			_varyDirection = (_varyTargetPercent - _varyPercent > 0) ? 1 : -1;
		} else {
			_varyPercent = _varyTargetPercent;
		}
	}
}

void GfxPalette32::setTarget(const Palette &palette) {
	delete _varyTargetPalette;
	_varyTargetPalette = new Palette(palette);
}

void GfxPalette32::setStart(const Palette &palette) {
	delete _varyStartPalette;
	_varyStartPalette = new Palette(palette);
}

void GfxPalette32::mergeStart(const Palette &palette) {
	if (_varyStartPalette != nullptr) {
		mergePalette(*_varyStartPalette, palette);
	} else {
		_varyStartPalette = new Palette(palette);
	}
}

void GfxPalette32::mergeTarget(const Palette &palette) {
	if (_varyTargetPalette != nullptr) {
		mergePalette(*_varyTargetPalette, palette);
	} else {
		_varyTargetPalette = new Palette(palette);
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

	if (_varyPercent == 0 || _varyTargetPalette == nullptr) {
		for (int i = 0; i < ARRAYSIZE(_nextPalette.colors); ++i) {
			if (_varyStartPalette != nullptr && i >= _varyFromColor && i <= _varyToColor) {
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

void GfxPalette32::kernelPalVarySet(const GuiResourceId paletteId, const int16 percent, const int32 ticks, const int16 fromColor, const int16 toColor) {
	const Palette palette = getPaletteFromResource(paletteId);
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
			if (_cyclers[i] == nullptr) {
				_cyclers[i] = cycler = new PalCycler;
				break;
			}
		}
	}

	// If there are no free cycler slots, SCI engine overrides the first oldest
	// cycler that it finds, where "oldest" is determined by the difference
	// between the tick and now
	if (cycler == nullptr) {
		const uint32 now = g_sci->getTickCount();
		uint32 minUpdateDelta = 0xFFFFFFFF;

		for (int i = 0; i < kNumCyclers; ++i) {
			PalCycler *const candidate = _cyclers[i];

			const uint32 updateDelta = now - candidate->lastUpdateTick;
			if (updateDelta < minUpdateDelta) {
				minUpdateDelta = updateDelta;
				cycler = candidate;
			}
		}

		clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
	}

	uint16 numColorsToCycle = toColor - fromColor;
	if (g_sci->_features->hasNewPaletteCode()) {
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
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr && cycler->numTimesPaused > 0) {
			--cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleAllPause() {
	// NOTE: The original engine did not check for null pointers in the
	// palette cyclers pointer array.
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr) {
			// This seems odd, because currentCycle is 0..numColorsPerCycle,
			// but fromColor is 0..255. When applyAllCycles runs, the values
			// end up back in range
			cycler->currentCycle = cycler->fromColor;
		}
	}

	applyAllCycles();

	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr) {
			++cycler->numTimesPaused;
		}
	}
}

void GfxPalette32::cycleOff(const uint8 fromColor) {
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			clearCycleMap(fromColor, cycler->numColorsToCycle);
			delete cycler;
			_cyclers[i] = nullptr;
			break;
		}
	}
}

void GfxPalette32::cycleAllOff() {
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr) {
			clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
			delete cycler;
			_cyclers[i] = nullptr;
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
		PalCycler *cycler = _cyclers[cyclerIndex];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			return cycler;
		}
	}

	return nullptr;
}

void GfxPalette32::applyAllCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(Color) * 256);

	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler != nullptr) {
			cycler->currentCycle = (((int) cycler->currentCycle) + 1) % cycler->numColorsToCycle;
			for (int j = 0; j < cycler->numColorsToCycle; j++) {
				_nextPalette.colors[cycler->fromColor + j] = paletteCopy[cycler->fromColor + (cycler->currentCycle + j) % cycler->numColorsToCycle];
			}
		}
	}
}

void GfxPalette32::applyCycles() {
	Color paletteCopy[256];
	memcpy(paletteCopy, _nextPalette.colors, sizeof(Color) * 256);

	const uint32 now = g_sci->getTickCount();
	for (int i = 0; i < kNumCyclers; ++i) {
		PalCycler *const cycler = _cyclers[i];
		if (cycler == nullptr) {
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
