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

#ifndef SCI_GRAPHICS_PALETTE32_H
#define SCI_GRAPHICS_PALETTE32_H

#include "sci/graphics/palette.h"

enum PalCyclerDirection {
	PalCycleBackward = 0,
	PalCycleForward = 1
};

struct PalCycler {
	/**
	 * The color index of the palette cycler. This value is effectively used as the ID for the
	 * cycler.
	 */
	uint8 fromColor;

	/**
	 * The number of palette slots which are cycled by the palette cycler.
	 */
	uint16 numColorsToCycle;

	/**
	 * The position of the cursor in its cycle.
	 */
	uint8 currentCycle;

	/**
	 * The direction of the cycler.
	 */
	PalCyclerDirection direction;

	/**
	 * The cycle tick at the last time the cycler’s currentCycle was updated.
	 * 795 days of game time ought to be enough for everyone? :)
	 */
	uint32 lastUpdateTick;

	/**
	 * The amount of time in ticks each cycle should take to complete. In other words,
	 * the higher the delay, the slower the cycle animation. If delay is 0, the cycler
	 * does not automatically cycle and needs to be pumped manually with DoCycle.
	 */
	int16 delay;

	/**
	 * The number of times this cycler has been paused.
	 */
	uint16 numTimesPaused;
};

namespace Sci {
	class GfxPalette32 : public GfxPalette {
	public:
		GfxPalette32(ResourceManager *resMan, GfxScreen *screen);
		~GfxPalette32();

	private:
		// SCI2 (SQ6) defines 10 cyclers
		PalCycler *_cyclers[10];
		/**
		 * The cycle map is used to detect overlapping cyclers. According to SCI engine code, when two cyclers overlap,
		 * a fatal error has occurred and the engine will display an error and then exit.
		 */
		bool _cycleMap[256];
		inline void _clearCycleMap(uint16 fromColor, uint16 numColorsToClear);
		inline void _setCycleMap(uint16 fromColor, uint16 numColorsToClear);
		inline PalCycler *_getCycler(uint16 fromColor);

		/**
		 * The fade table records the expected intensity level of each pixel in the palette that will be displayed on
		 * the next frame.
		 */
		byte _fadeTable[256];
		byte *_clutTable;

	public:
		void applyAllCycles();
		void applyCycles();
		void applyFade();

		bool loadClut(uint16 clutId);
		byte matchClutColor(uint16 color);
		void unloadClut();

		int16 setCycle(uint16 fromColor, uint16 toColor, int16 direction, int16 delay);
		void doCycle(uint16 fromColor, int16 speed);
		void cycleOn(uint16 fromColor);
		void cyclePause(uint16 fromColor);
		void cycleAllOn();
		void cycleAllPause();
		void cycleOff(uint16 fromColor);
		void cycleAllOff();

		void setFade(uint8 percent, uint16 fromColor, uint16 toColor);
		void fadeOff();
	};
}

#endif
