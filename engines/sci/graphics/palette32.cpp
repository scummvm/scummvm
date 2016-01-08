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

#include "sci/sci.h"
#include "sci/graphics/palette32.h"

namespace Sci {
	
GfxPalette32::GfxPalette32(ResourceManager *resMan, GfxScreen *screen)
	: GfxPalette(resMan, screen), _clutTable(0), _cyclers(), _cycleMap() {}

GfxPalette32::~GfxPalette32() {
	unloadClut();
	cycleAllOff();
}

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
	_clutTable = 0;
}

inline void GfxPalette32::_clearCycleMap(const uint16 fromColor, const uint16 numColorsToClear) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *lastEntry = _cycleMap + numColorsToClear;
	while (mapEntry < lastEntry) {
		*mapEntry++ = false;
	}
}

inline void GfxPalette32::_setCycleMap(const uint16 fromColor, const uint16 numColorsToSet) {
	bool *mapEntry = _cycleMap + fromColor;
	const bool *lastEntry = _cycleMap + numColorsToSet;
	while (mapEntry < lastEntry) {
		if (*mapEntry != false) {
			error("Cycles intersect");
		}
		*mapEntry++ = true;
	}
}

inline PalCycler *GfxPalette32::_getCycler(const uint16 fromColor) {
	const int numCyclers = ARRAYSIZE(_cyclers);

	for (int cyclerIndex = 0; cyclerIndex < numCyclers; ++cyclerIndex) {
		PalCycler *cycler = _cyclers[cyclerIndex];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			return cycler;
		}
	}

	return nullptr;
}

inline void _doCycle(PalCycler *cycler, const int16 speed) {
	int16 currentCycle = cycler->currentCycle;
	const uint16 numColorsToCycle = cycler->numColorsToCycle;

	if (cycler->direction == 0) {
		currentCycle = (currentCycle - (speed % numColorsToCycle)) + numColorsToCycle;
	} else {
		currentCycle = currentCycle + speed;
	}

	cycler->currentCycle = (uint8) (currentCycle % numColorsToCycle);
}

inline void _applyCycleToPalette(PalCycler *cycler, Palette *palette) {
	const int16 currentCycle = cycler->currentCycle;
	const uint16 numColorsToCycle = cycler->numColorsToCycle;

	Color *tempPalette = new Color[numColorsToCycle];
	Color *sourceColor = palette->colors + cycler->fromColor;
	memcpy(tempPalette, sourceColor, sizeof(tempPalette));

	Color *targetColor = sourceColor;
	for (int numColorsCycled = 0; numColorsCycled < numColorsToCycle; ++numColorsCycled) {
		Color sourceColor = *(tempPalette + ((currentCycle + numColorsCycled) % numColorsToCycle));
		*(targetColor + numColorsCycled) = sourceColor;
	}

	delete[] tempPalette;
}

void GfxPalette32::applyAllCycles() {
	for (int cyclerIndex = 0, numCyclers = ARRAYSIZE(_cyclers); cyclerIndex < numCyclers; ++cyclerIndex) {
		PalCycler *cycler = _cyclers[cyclerIndex];
		if (cycler != nullptr) {
			cycler->currentCycle = (uint8) ((((int) cycler->currentCycle) + 1) % cycler->numColorsToCycle);
			// Disassembly was not fully evaluated to verify this is exactly the same
			// as the code from applyCycles, but it appeared to be at a glance
			_applyCycleToPalette(cycler, &_sysPalette);
		}
	}
}

void GfxPalette32::applyCycles() {
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler == nullptr) {
			continue;
		}

		if (cycler->delay != 0 && cycler->numTimesPaused == 0) {
			while ((cycler->delay + cycler->lastUpdateTick) < g_sci->getTickCount()) {
				_doCycle(cycler, 1);
				cycler->lastUpdateTick += cycler->delay;
			}
		}

		_applyCycleToPalette(cycler, &_sysPalette);
	}
}

int16 GfxPalette32::setCycle(const uint16 fromColor, const uint16 toColor, const int16 direction, const int16 delay) {
	assert(fromColor < toColor);

	int cyclerIndex;
	const int numCyclers = ARRAYSIZE(_cyclers);

	PalCycler *cycler = _getCycler(fromColor);

	if (cycler != nullptr) {
		debug("Resetting existing cycler");
		_clearCycleMap(fromColor, cycler->numColorsToCycle);
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

		_clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
	}

	const uint16 numColorsToCycle = 1 + ((uint8) toColor) - fromColor;
	cycler->fromColor = (uint8) fromColor;
	cycler->numColorsToCycle = (uint8) numColorsToCycle;
	cycler->currentCycle = (uint8) fromColor;
	cycler->direction = direction < 0 ? PalCycleBackward : PalCycleForward;
	cycler->delay = delay;
	cycler->lastUpdateTick = g_sci->getTickCount();
	cycler->numTimesPaused = 0;

	_setCycleMap(fromColor, numColorsToCycle);

	// TODO: Validate that this is the correct return value according
	// to disassembly
	return 0;
}

void GfxPalette32::doCycle(const uint16 fromColor, const int16 speed) {
	PalCycler *cycler = _getCycler(fromColor);
	if (cycler != nullptr) {
		cycler->lastUpdateTick = g_sci->getTickCount();
		_doCycle(cycler, speed);
	}
}

void GfxPalette32::cycleOn(const uint16 fromColor) {
	PalCycler *cycler = _getCycler(fromColor);
	if (cycler != nullptr && cycler->numTimesPaused > 0) {
		--cycler->numTimesPaused;
	}
}

void GfxPalette32::cyclePause(const uint16 fromColor) {
	PalCycler *cycler = _getCycler(fromColor);
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

void GfxPalette32::cycleOff(const uint16 fromColor) {
	for (int i = 0, len = ARRAYSIZE(_cyclers); i < len; ++i) {
		PalCycler *cycler = _cyclers[i];
		if (cycler != nullptr && cycler->fromColor == fromColor) {
			_clearCycleMap(fromColor, cycler->numColorsToCycle);
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
			_clearCycleMap(cycler->fromColor, cycler->numColorsToCycle);
			delete cycler;
			_cyclers[i] = nullptr;
		}
	}
}

void GfxPalette32::applyFade() {
	// TODO: Create and update a _nextPalette, not a single _sysPalette, to
	// conform to the way the actual SCI32 engine works (writes to a
	// next-frame-palette and then copies to the current palette on frameout)
	Color *color = _sysPalette.colors;
	uint8 *fadeAmount = _fadeTable;
	for (int i = 0; i < 256; ++i) {
		if (*fadeAmount == 100) {
			continue;
		}

		color->r = ((int) color->r * ((int) *fadeAmount)) / 100;
		color->g = ((int) color->r * ((int) *fadeAmount)) / 100;
		color->b = ((int) color->r * ((int) *fadeAmount)) / 100;
	}
}

void GfxPalette32::setFade(uint8 percent, uint16 fromColor, uint16 toColor) {
	uint8 *fadeAmount = _fadeTable;
	for (int i = fromColor, len = toColor - fromColor + 1; i < len; ++i) {
		*fadeAmount++ = percent;
	}
}

void GfxPalette32::fadeOff() {
	setFade(100, 0, 255);
}

}
