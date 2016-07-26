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

namespace Sci {

/**
 * HunkPalette represents a raw palette resource
 * read from disk.
 */
class HunkPalette {
public:
	HunkPalette(byte *rawPalette);

	/**
	 * Gets the version of the palette.
	 */
	uint32 getVersion() const { return _version; }

	/**
	 * Sets the version of the palette.
	 */
	void setVersion(const uint32 version);

	/**
	 * Converts the hunk palette to a standard
	 * palette.
	 */
	const Palette toPalette() const;

private:
	enum {
		/**
		 * The size of the HunkPalette header.
		 */
		kHunkPaletteHeaderSize = 13,

		/**
		 * The size of a palette entry header.
		 */
		kEntryHeaderSize = 22,

		/**
		 * The offset of the hunk palette version
		 * within the palette entry header.
		 */
		kEntryVersionOffset = 18
	};

	/**
	 * The header for a palette inside the
	 * HunkPalette.
	 */
	struct EntryHeader {
		/**
		 * The start color.
		 */
		uint8 startColor;

		/**
		 * The number of palette colors in this
		 * entry.
		 */
		uint16 numColors;

		/**
		 * The default `used` flag.
		 */
		bool used;

		/**
		 * Whether or not all palette entries
		 * share the same `used` value in
		 * `defaultFlag`.
		 */
		bool sharedUsed;

		/**
		 * The palette version.
		 */
		uint32 version;
	};

	/**
	 * The version number from the last time this
	 * palette was submitted to GfxPalette32.
	 */
	uint32 _version;

	/**
	 * The number of palettes stored in the hunk
	 * palette. In SCI32 games this is always 1.
	 */
	uint8 _numPalettes;

	/**
	 * The raw palette data for this hunk palette.
	 */
	byte *_data;

	/**
	 * Returns a struct that describes the palette
	 * held by this HunkPalette. The entry header
	 * is reconstructed on every call from the raw
	 * palette data.
	 */
	const EntryHeader getEntryHeader() const;

	/**
	 * Returns a pointer to the palette data within
	 * the hunk palette.
	 */
	byte *getPalPointer() const {
		return _data + kHunkPaletteHeaderSize + (2 * _numPalettes);
	}
};

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

class GfxPalette32 {
public:
	GfxPalette32(ResourceManager *resMan);
	~GfxPalette32();

private:
	ResourceManager *_resMan;

	/**
	 * The palette revision version. Increments once per game
	 * loop that changes the source palette.
	 */
	uint32 _version;

	/**
	 * Whether or not the hardware palette needs updating.
	 */
	bool _needsUpdate;

	/**
	 * The currently displayed palette.
	 */
	Palette _currentPalette;

	/**
	 * The unmodified source palette loaded by kPalette. Additional
	 * palette entries may be mixed into the source palette by
	 * CelObj objects, which contain their own palettes.
	 */
	Palette _sourcePalette;

	/**
	 * The palette to be used when the hardware is next updated.
	 * On update, _nextPalette is transferred to _currentPalette.
	 */
	Palette _nextPalette;

	bool createPaletteFromResourceInternal(const GuiResourceId paletteId, Palette *const out) const;
	Palette getPaletteFromResourceInternal(const GuiResourceId paletteId) const;

public:
	void saveLoadWithSerializer(Common::Serializer &s);
	inline const Palette &getNextPalette() const { return _nextPalette; };
	inline const Palette &getCurrentPalette() const { return _currentPalette; };

	/**
	 * Loads a palette into GfxPalette32 with the given resource
	 * ID.
	 */
	bool loadPalette(const GuiResourceId resourceId);

	/**
	 * Finds the nearest color in the current palette matching the
	 * given RGB value.
	 */
	int16 matchColor(const uint8 r, const uint8 g, const uint8 b);

	/**
	 * Submits a palette to display. Entries marked as “used” in the
	 * submitted palette are merged into the existing entries of
	 * _sourcePalette.
	 */
	void submit(const Palette &palette);
	void submit(HunkPalette &palette);

	bool updateForFrame();
	void updateFFrame();
	void updateHardware(const bool updateScreen = true);
	void applyAll();

#pragma mark -
#pragma mark Color look-up
private:
	/**
	 * An optional lookup table used to remap RGB565 colors to a palette
	 * index. Used by Phantasmagoria 2 in 8-bit color environments.
	 */
	byte *_clutTable;

public:
	bool loadClut(uint16 clutId);
	byte matchClutColor(uint16 color);
	void unloadClut();

#pragma mark -
#pragma mark Varying
private:
	/**
	 * An optional palette used to describe the source colors used
	 * in a palette vary operation. If this palette is not specified,
	 * sourcePalette is used instead.
	 */
	Palette *_varyStartPalette;

	/**
	 * An optional palette used to describe the target colors used
	 * in a palette vary operation.
	 */
	Palette *_varyTargetPalette;

	/**
	 * The minimum palette index that has been varied from the
	 * source palette. 0–255
	 */
	uint8 _varyFromColor;

	/**
	 * The maximum palette index that is has been varied from the
	 * source palette. 0-255
	 */
	uint8 _varyToColor;

	/**
	 * The tick at the last time the palette vary was updated.
	 */
	uint32 _varyLastTick;

	/**
	 * The amount of time to elapse, in ticks, between each cycle
	 * of a palette vary animation.
	 */
	int _varyTime;

	/**
	 * The direction of change: -1, 0, or 1.
	 */
	int16 _varyDirection;

	/**
	 * The amount, in percent, that the vary color is currently
	 * blended into the source color.
	 */
	int16 _varyPercent;

	/**
	 * The target amount that a vary color will be blended into
	 * the source color.
	 */
	int16 _varyTargetPercent;

	/**
	 * The number of time palette varying has been paused.
	 */
	uint16 _varyNumTimesPaused;

public:
	void kernelPalVarySet(const GuiResourceId paletteId, const int16 percent, const int time, const int16 fromColor, const int16 toColor);
	void kernelPalVaryMergeTarget(const GuiResourceId paletteId);
	void kernelPalVarySetTarget(const GuiResourceId paletteId);
	void kernelPalVarySetStart(const GuiResourceId paletteId);
	void kernelPalVaryMergeStart(const GuiResourceId paletteId);
	void kernelPalVaryPause(bool pause);

	void setVary(const Palette *const targetPalette, const int16 percent, const int time, const int16 fromColor, const int16 toColor);
	void setVaryPercent(const int16 percent, const int time, const int16 fromColor, const int16 fromColorAlternate);
	int16 getVaryPercent() const;
	void varyOff();
	void mergeTarget(const Palette *const palette);
	void varyPause();
	void varyOn();
	void setVaryTime(const int time);
	void setTarget(const Palette *const palette);
	void setStart(const Palette *const palette);
	void mergeStart(const Palette *const palette);
	void setVaryTimeInternal(const int16 percent, const int time);
	void applyVary();

#pragma mark -
#pragma mark Cycling
private:
	// SQ6 defines 10 cyclers
	PalCycler *_cyclers[10];

	/**
	 * The cycle map is used to detect overlapping cyclers.
	 * According to SCI engine code, when two cyclers overlap,
	 * a fatal error has occurred and the engine will display
	 * an error and then exit.
	 *
	 * The cycle map is also by the color remapping system to
	 * avoid attempting to remap to palette entries that are
	 * cycling (so won't be the expected color once the cycler
	 * runs again).
	 */
	bool _cycleMap[256];
	inline void clearCycleMap(uint16 fromColor, uint16 numColorsToClear);
	inline void setCycleMap(uint16 fromColor, uint16 numColorsToClear);
	inline PalCycler *getCycler(uint16 fromColor);

public:
	void setCycle(const uint8 fromColor, const uint8 toColor, const int16 direction, const int16 delay);
	void doCycle(const uint8 fromColor, const int16 speed);
	void cycleOn(const uint8 fromColor);
	void cyclePause(const uint8 fromColor);
	void cycleAllOn();
	void cycleAllPause();
	void cycleOff(const uint8 fromColor);
	void cycleAllOff();
	void applyAllCycles();
	void applyCycles();
	inline const bool *getCycleMap() const { return _cycleMap; }

#pragma mark -
#pragma mark Fading
private:
	/**
	 * The fade table records the expected intensity level of each pixel
	 * in the palette that will be displayed on the next frame.
	 */
	uint16 _fadeTable[256];

public:
	/**
	 * Sets the intensity level for a range of palette
	 * entries. An intensity of zero indicates total
	 * darkness. Intensity may be set to over 100 percent.
	 */
	void setFade(const uint16 percent, const uint8 fromColor, const uint16 toColor);
	void fadeOff();
	void applyFade();
};

} // End of namespace Sci

#endif
