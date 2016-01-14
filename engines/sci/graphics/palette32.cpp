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
#include "sci/graphics/palette32.h"
#include "sci/graphics/screen.h"

namespace Sci {
	
GfxPalette32::GfxPalette32(ResourceManager *resMan, GfxScreen *screen)
	: GfxPalette(resMan, screen),
	_clutTable(nullptr),
	// Palette cycling
	_cyclers(), _cycleMap(),
	// Palette varying
	_sourcePalette(_sysPalette), _nextPalette(_sysPalette),
	_varyTime(0), _varyDirection(0), _varyTargetPercent(0),
	_varyTargetPalette(nullptr), _varyStartPalette(nullptr),
	_varyFromColor(0), _varyToColor(255), _varyNumTimesPaused(0),
	_varyPercent(_varyTargetPercent), _varyLastTick(0),
	// Palette versioning
	_version(1), _versionUpdated(false) {
		memset(_fadeTable, 100, sizeof(_fadeTable));

		// NOTE: In SCI engine, the palette manager constructor loads
		// the default palette, but in ScummVM this initialisation
		// is performed by SciEngine::run; see r49523 for details
	}

GfxPalette32::~GfxPalette32() {
	unloadClut();
	varyOff();
	cycleAllOff();
}

inline void mergePaletteInternal(Palette *const to, const Palette *const from) {
	for (int i = 0; i < ARRAYSIZE(to->colors); ++i) {
		if (from->colors[i].used) {
			to->colors[i] = from->colors[i];
		}
	}
}

void GfxPalette32::submit(Palette &palette) {
	// TODO: The resource manager in SCI32 retains raw data of palettes from
	// the ResourceManager (ResourceMgr) through SegManager (MemoryMgr), and
	// the version number for submitted palettes is set in the raw palette
	// data in memory as an int at an offset
	// `rawData + *rawData[0x0a] * 2 + 31`. However, ScummVM does not retain
	// resource data like this, so this versioning code, while accurate to
	// the original engine, does not do much.
	// (Hopefully this was an optimisation mechanism in SCI engine and not a
	// clever thing to keep the same palette submitted many times from
	// overwriting other palette entries.)
	if (palette.timestamp == _version) {
		return;
	}

	Palette oldSourcePalette(_sourcePalette);
	mergePaletteInternal(&_sourcePalette, &palette);

	if (!_versionUpdated && _sourcePalette != oldSourcePalette) {
		++_version;
		_versionUpdated = true;
	}

	// Technically this information is supposed to be persisted through a
	// HunkPalette object; right now it would just be lost once the temporary
	// palette was destroyed.
	palette.timestamp = _version;
}

bool GfxPalette32::kernelSetFromResource(GuiResourceId resourceId, bool force) {
	// TODO: In SCI32, palettes that come from resources come in as
	// HunkPalette objects, not SOLPalette objects. The HunkPalettes
	// have some extra persistence stuff associated with them, such that
	// when they are passed to GfxPalette32::submit, they would get the
	// version number of GfxPalette32 assigned to them.
	Palette palette;

	if (createPaletteFromResourceInternal(resourceId, &palette)) {
		submit(palette);
		return true;
	}

	return false;
}

// In SCI32 engine this method is SOLPalette::Match(Rgb24 *)
// and is called as PaletteMgr.Current().Match(color)
int16 GfxPalette32::kernelFindColor(uint16 r, uint16 g, uint16 b) {
	// SQ6 SCI32 engine takes the 16-bit r, g, b arguments from the
	// VM and puts them into al, ah, dl. For compatibility, make sure
	// to discard any high bits here too
	r = r & 0xFF;
	g = g & 0xFF;
	b = b & 0xFF;
	int16 bestIndex = 0;
	int bestDifference = 0xFFFFF;
	int difference;

	// SQ6 DOS really does check only the first 236 entries
	for (int i = 0, channelDifference; i < 236; ++i) {
		difference = _sysPalette.colors[i].r - r;
		difference *= difference;
		if (bestDifference <= difference) {
			continue;
		}

		channelDifference = _sysPalette.colors[i].g - g;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}

		channelDifference = _sysPalette.colors[i].b - b;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}
		bestDifference = difference;
		bestIndex = i;
	}

	return bestIndex;
}

// TODO: set is overridden for the time being to send palettes coming from
// various draw methods like GfxPicture::drawSci32Vga and GfxView::draw to
// _nextPalette instead of _sysPalette. In the SCI32 engine, CelObj palettes
// (which are stored as Hunk palettes) are submitted by GraphicsMgr::FrameOut
// to PaletteMgr::Submit by way of calls to CelObj::SubmitPalette.
// GfxPalette::set is very similar to GfxPalette32::submit, except that SCI32
// does not do any fancy best-fit merging and so does not accept arguments
// like `force` and `forceRealMerge`.
void GfxPalette32::set(Palette *newPalette, bool force, bool forceRealMerge) {
	submit(*newPalette);
}

// In SCI32 engine this method is SOLPalette::Match(Rgb24 *, int, int *, int *)
// and is used by Remap
// TODO: Anything that calls GfxPalette::matchColor(int, int, int) is going to
// match using an algorithm from SCI16 engine right now. This needs to be
// corrected in the future so either nothing calls
// GfxPalette::matchColor(int, int, int), or it is fixed to match the other
// SCI32 algorithms.
int16 GfxPalette32::matchColor(const byte r, const byte g, const byte b, const int defaultDifference, int &lastCalculatedDifference, const bool *const matchTable) {
	int16 bestIndex = -1;
	int bestDifference = 0xFFFFF;
	int difference = defaultDifference;

	// SQ6 DOS really does check only the first 236 entries
	for (int i = 0, channelDifference; i < 236; ++i) {
		if (matchTable[i] == 0) {
			continue;
		}

		difference = _sysPalette.colors[i].r - r;
		difference *= difference;
		if (bestDifference <= difference) {
			continue;
		}
		channelDifference = _sysPalette.colors[i].g - g;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}
		channelDifference = _sysPalette.colors[i].b - b;
		difference += channelDifference * channelDifference;
		if (bestDifference <= difference) {
			continue;
		}
		bestDifference = difference;
		bestIndex = i;
	}

	// NOTE: This value is only valid if the last index to
	// perform a difference calculation was the best index
	lastCalculatedDifference = difference;
	return bestIndex;
}

void GfxPalette32::updateForFrame() {
	applyAll();
	_versionUpdated = false;
	// TODO: Implement remapping
	// g_sci->_gfxFrameout->remapAllTables(_nextPalette != _sysPalette);
}

void GfxPalette32::updateHardware() {
	if (_sysPalette == _nextPalette) {
		// TODO: This condition has never been encountered yet
		debug("Skipping hardware update because palettes are identical");
		return;
	}

	byte bpal[3 * 256];

	for (int i = 0; i < ARRAYSIZE(_sysPalette.colors); ++i) {
		_sysPalette.colors[i] = _nextPalette.colors[i];

		// NOTE: If the brightness option in the user configuration file is set,
		// SCI engine adjusts palette brightnesses here by mapping RGB values to values
		// in some hard-coded brightness tables. There is no reason on modern hardware
		// to implement this, unless it is discovered that some game uses a non-standard
		// brightness setting by default
		if (_sysPalette.colors[i].used) {
			bpal[i * 3    ] = _sysPalette.colors[i].r;
			bpal[i * 3 + 1] = _sysPalette.colors[i].g;
			bpal[i * 3 + 2] = _sysPalette.colors[i].b;
		}
	}

	g_system->getPaletteManager()->setPalette(bpal, 0, 256);
	g_sci->getEventManager()->updateScreen();
}

void GfxPalette32::applyAll() {
	applyVary();
	applyCycles();
	applyFade();
}

//
// Clut
//

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

//
// Palette vary
//

inline bool GfxPalette32::createPaletteFromResourceInternal(const GuiResourceId paletteId, Palette *const out) const {
	Resource *palResource = _resMan->findResource(ResourceId(kResourceTypePalette, paletteId), false);

	if (!palResource) {
		return false;
	}

	createFromData(palResource->data, palResource->size, out);
	return true;
}

inline Palette GfxPalette32::getPaletteFromResourceInternal(const GuiResourceId paletteId) const {
	Palette palette;
	if (!createPaletteFromResourceInternal(paletteId, &palette)) {
		error("Could not load vary target %d", paletteId);
	}
	return palette;
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

// TODO: This gets called *a lot* in at least the first scene
// of SQ6. Optimisation would not be the worst idea in the world.
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
	return abs(_varyPercent);
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

//
// Palette cycling
//

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
		int maxUpdateDelta = -1;
		// Optimization: Unlike actual SCI (SQ6) engine, we call
		// getTickCount only once and store it, instead of calling it
		// twice on each iteration through the loop
		const uint32 now = g_sci->getTickCount();

		for (cyclerIndex = 0; cyclerIndex < numCyclers; ++cyclerIndex) {
			PalCycler *candidate = _cyclers[cyclerIndex];

			const int32 updateDelta = now - candidate->lastUpdateTick;
			if (updateDelta >= maxUpdateDelta) {
				maxUpdateDelta = updateDelta;
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
	// TODO: The SCI SQ6 cycleAllPause function does not seem to perform
	// nullptr checking?? This would definitely cause null pointer
	// dereference in SCI code. I have not seen anything actually call
	// this function yet, so it is possible it is just unused and broken
	// in SCI SQ6. This assert exists so that if this function is called,
	// it is noticed and can be rechecked in the actual engine.
	// Obviously this code *does* do nullptr checks instead of crashing. :)
	assert(0);
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

//
// Palette fading
//

void GfxPalette32::setFade(uint8 percent, uint8 fromColor, uint16 numColorsToFade) {
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

		color.r = (int16)color.r * _fadeTable[i] / 100;
		color.g = (int16)color.g * _fadeTable[i] / 100;
		color.b = (int16)color.b * _fadeTable[i] / 100;
	}
}
}
