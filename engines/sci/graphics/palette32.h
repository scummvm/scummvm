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

namespace Sci {

#pragma mark HunkPalette

/**
 * HunkPalette represents a raw palette resource read from disk. The data
 * structure of a HunkPalette allows palettes to be smaller than 256 colors. It
 * also allows multiple palettes to be stored in one HunkPalette, though in
 * SCI32 games there seems to only ever be one palette per HunkPalette.
 */
class HunkPalette {
public:
	HunkPalette(byte *rawPalette);

	/**
	 * Gets the version of the palette. Used to avoid resubmitting a HunkPalette
	 * which has already been submitted for the next frame.
	 */
	uint32 getVersion() const { return _version; }

	/**
	 * Sets the version of the palette.
	 */
	void setVersion(const uint32 version) const;

	/**
	 * Converts the hunk palette to a standard Palette.
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
		 * The offset of the hunk palette version within the palette entry
		 * header.
		 */
		kEntryVersionOffset = 18
	};

	/**
	 * The header for a palette inside the HunkPalette.
	 */
	struct EntryHeader {
		/**
		 * The start color.
		 */
		uint8 startColor;

		/**
		 * The number of palette colors in this entry.
		 */
		uint16 numColors;

		/**
		 * The default `used` flag.
		 */
		bool used;

		/**
		 * Whether or not all palette entries share the same `used` value in
		 * `defaultFlag`.
		 */
		bool sharedUsed;

		/**
		 * The palette version.
		 */
		uint32 version;
	};

	/**
	 * The version number from the last time this palette was submitted to
	 * GfxPalette32.
	 */
	mutable uint32 _version;

	/**
	 * The number of palettes stored in the hunk palette. In SCI32 games this is
	 * always 1.
	 */
	uint8 _numPalettes;

	/**
	 * The raw palette data for this hunk palette.
	 */
	byte *_data;

	/**
	 * Returns a struct that describes the palette held by this HunkPalette. The
	 * entry header is reconstructed on every call from the raw palette data.
	 */
	const EntryHeader getEntryHeader() const;

	/**
	 * Returns a pointer to the palette data within the hunk palette.
	 */
	byte *getPalPointer() const {
		return _data + kHunkPaletteHeaderSize + (2 * _numPalettes);
	}
};

#pragma mark -
#pragma mark PalCycler

enum PalCyclerDirection {
	kPalCycleBackward = 0,
	kPalCycleForward  = 1
};

/**
 * PalCycler represents a range of palette entries that are rotated on a timer.
 */
struct PalCycler {
	/**
	 * The color index of this palette cycler. This value is used as the unique
	 * key for this PalCycler object.
	 */
	uint8 fromColor;

	/**
	 * The number of palette slots which are to be cycled by this cycler.
	 */
	uint16 numColorsToCycle;

	/**
	 * The current position of the first palette entry.
	 */
	uint8 currentCycle;

	/**
	 * The direction of the cycler.
	 */
	PalCyclerDirection direction;

	/**
	 * The last tick the cycler cycled.
	 */
	uint32 lastUpdateTick;

	/**
	 * The amount of time in ticks each cycle should take to complete. In other
	 * words, the higher the delay, the slower the cycle animation. If delay is
	 * 0, the cycler does not automatically cycle and needs to be cycled
	 * manually by calling `doCycle`.
	 */
	int16 delay;

	/**
	 * The number of times this cycler has been paused.
	 */
	uint16 numTimesPaused;
};

#pragma mark -
#pragma mark GfxPalette32

class GfxPalette32 {
public:
	GfxPalette32(ResourceManager *resMan);
	~GfxPalette32();

	void saveLoadWithSerializer(Common::Serializer &s);

	/**
	 * Gets the palette that will be use for the next frame.
	 */
	inline const Palette &getNextPalette() const { return _nextPalette; };

	/**
	 * Gets the palette that is used for the current frame.
	 */
	inline const Palette &getCurrentPalette() const { return _currentPalette; };

	/**
	 * Loads a palette into GfxPalette32 with the given resource ID.
	 */
	bool loadPalette(const GuiResourceId resourceId);

	/**
	 * Finds the nearest color in the current palette matching the given RGB
	 * value.
	 */
	int16 matchColor(const uint8 r, const uint8 g, const uint8 b);

	/**
	 * Submits a palette to display. Entries marked as "used" in the submitted
	 * palette are merged into `_sourcePalette`.
	 */
	void submit(const Palette &palette);
	void submit(const HunkPalette &palette);

	/**
	 * Applies all fades, cycles, remaps, and varies for the current frame to
	 * `nextPalette`.
	 */
	bool updateForFrame();

	/**
	 * Copies all palette entries from `sourcePalette` to `nextPalette` and
	 * applies remaps. Unlike `updateForFrame`, this call does not apply fades,
	 * cycles, or varies.
	 */
	void updateFFrame();

	/**
	 * Copies all entries from `nextPalette` to `currentPalette` and updates the
	 * backend's raw palette.
	 *
	 * @param updateScreen If true, this call will also tell the backend to draw
	 * to the screen.
	 */
	void updateHardware(const bool updateScreen = true);

private:
	ResourceManager *_resMan;

	/**
	 * The palette revision version. Increments once per game loop that changes
	 * the source palette.
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
	 * The unmodified source palette loaded by kPalette. Additional palette
	 * entries may be mixed into the source palette by CelObj objects, which
	 * contain their own palettes.
	 */
	Palette _sourcePalette;

	/**
	 * The palette to be used when the hardware is next updated.
	 * On update, `_nextPalette` is transferred to `_currentPalette`.
	 */
	Palette _nextPalette;

	/**
	 * Creates and returns a new Palette object with data from the given
	 * resource ID.
	 */
	Palette getPaletteFromResource(const GuiResourceId paletteId) const;

	/**
	 * Merges used colors in the `from` palette into the `to` palette.
	 */
	void mergePalette(Palette &to, const Palette &from);

	/**
	 * Applies all varies, cycles, and fades to `_nextPalette`.
	 */
	void applyAll();

#pragma mark -
#pragma mark Varying
public:
	/**
	 * Blends the `target` palette into the current palette over `time` ticks.
	 *
	 * @param target The target palette.
	 * @param percent The amount that the target palette should be blended into
	 *                the source palette by the end of the vary.
	 * @param ticks The number of ticks that it should take for the blend to be
	 *              completed.
	 * @param fromColor The first palette entry that should be blended.
	 * @param toColor The last palette entry that should be blended.
	 */
	void setVary(const Palette &target, const int16 percent, const int32 ticks, const int16 fromColor, const int16 toColor);

	/**
	 * Gets the current vary blend amount.
	 */
	inline int16 getVaryPercent() const { return ABS(_varyPercent); }

	/**
	 * Changes the percentage of the current vary to `percent`, to be completed
	 * over `time` ticks, if there is a currently active vary target palette.
	 */
	void setVaryPercent(const int16 percent, const int32 time);

	/**
	 * Changes the amount of time, in ticks, an in-progress palette vary should
	 * take to finish.
	 */
	void setVaryTime(const int32 ticks);

	/**
	 * Changes the vary percent and time to perform the vary.
	 */
	void setVaryTime(const int16 percent, const int32 ticks);

	/**
	 * Removes the active palette vary.
	 */
	void varyOff();

	/**
	 * Pauses any active palette vary.
	 */
	void varyPause();

	/**
	 * Unpauses any paused palette vary.
	 */
	void varyOn();

	/**
	 * Sets the target palette for the blend.
	 */
	void setTarget(const Palette &palette);

	/**
	 * Sets the start palette for the blend.
	 */
	void setStart(const Palette &palette);

	/**
	 * Merges a new start palette into the existing start palette.
	 */
	void mergeStart(const Palette &palette);

	/**
	 * Merges a new target palette into the existing target palette.
	 */
	void mergeTarget(const Palette &palette);

	/**
	 * Applies any active palette vary to `_nextPalette`.
	 */
	void applyVary();

	void kernelPalVarySet(const GuiResourceId paletteId, const int16 percent, const int32 ticks, const int16 fromColor, const int16 toColor);
	void kernelPalVaryMergeTarget(const GuiResourceId paletteId);
	void kernelPalVarySetTarget(const GuiResourceId paletteId);
	void kernelPalVarySetStart(const GuiResourceId paletteId);
	void kernelPalVaryMergeStart(const GuiResourceId paletteId);
	void kernelPalVaryPause(const bool pause);

private:
	/**
	 * An optional palette used to provide source colors for a palette vary
	 * operation. If this palette is not specified, `_sourcePalette` is used
	 * instead.
	 */
	Palette *_varyStartPalette;

	/**
	 * An optional palette used to provide target colors for a palette vary
	 * operation.
	 */
	Palette *_varyTargetPalette;

	/**
	 * The minimum palette index that has been varied from the source palette.
	 */
	uint8 _varyFromColor;

	/**
	 * The maximum palette index that has been varied from the source palette.
	 */
	uint8 _varyToColor;

	/**
	 * The tick at the last time the palette vary was updated.
	 */
	uint32 _varyLastTick;

	/**
	 * The amount of time that should elapse, in ticks, between each cycle of a
	 * palette vary animation.
	 */
	int32 _varyTime;

	/**
	 * The direction of change: -1, 0, or 1.
	 */
	int16 _varyDirection;

	/**
	 * The amount, in percent, that the vary color is currently blended into the
	 * source color.
	 */
	int16 _varyPercent;

	/**
	 * The target amount that a vary color will be blended into the source
	 * color.
	 */
	int16 _varyTargetPercent;

	/**
	 * The number of times palette varying has been paused.
	 */
	uint16 _varyNumTimesPaused;

#pragma mark -
#pragma mark Cycling
public:
	inline const bool *getCycleMap() const { return _cycleMap; }

	/**
	 * Cycle palette entries between `fromColor` and `toColor`, inclusive.
	 * Palette cyclers may not overlap. `fromColor` is used in other methods as
	 * the key for looking up a cycler.
	 *
	 * @param fromColor The first color in the cycle.
	 * @param toColor The last color in the cycle.
	 * @param delay The number of ticks that should elapse between cycles.
	 * @param direction A negative `direction` will cycle backwards instead of
	 *                  forwards. The numeric value of this argument is ignored;
	 *                  only its sign is used to determine direction.
	 */
	void setCycle(const uint8 fromColor, const uint8 toColor, const int16 direction, const int16 delay);

	/**
	 * Performs a round of palette cycling.
	 *
	 * @param fromColor The color key for the cycler.
	 * @param speed The number of entries that should be cycled this round.
	 */
	void doCycle(const uint8 fromColor, const int16 speed);

	/**
	 * Unpauses the cycler starting at `fromColor`.
	 */
	void cycleOn(const uint8 fromColor);

	/**
	 * Pauses the cycler starting at `fromColor`.
	 */
	void cyclePause(const uint8 fromColor);

	/**
	 * Unpauses all cyclers.
	 */
	void cycleAllOn();

	/**
	 * Pauses all cyclers.
	 */
	void cycleAllPause();

	/**
	 * Removes the cycler starting at `fromColor`.
	 */
	void cycleOff(const uint8 fromColor);

	/**
	 * Removes all cyclers.
	 */
	void cycleAllOff();

private:
	enum {
		kNumCyclers = 10
	};

	PalCycler *_cyclers[kNumCyclers];

	/**
	 * Updates the `currentCycle` of the given `cycler` by `speed` entries.
	 */
	void updateCycler(PalCycler &cycler, const int16 speed);

	/**
	 * The cycle map is used to detect overlapping cyclers, and to avoid
	 * remapping to palette entries that are being cycled.
	 *
	 * According to SCI engine code, when two cyclers overlap, a fatal error has
	 * occurred and the engine will display an error and then exit.
	 *
	 * The color remapping system avoids attempts to remap to palette entries
	 * that are cycling because they won't be the expected color once the cycler
	 * updates the palette entries.
	 */
	bool _cycleMap[256];

	/**
	 * Marks `numColorsToClear` colors starting at `fromColor` in the cycle
	 * map as inactive.
	 */
	void clearCycleMap(const uint16 fromColor, const uint16 numColorsToClear);

	/**
	 * Marks `numColorsToClear` colors starting at `fromColor` in the cycle
	 * map as active.
	 */
	void setCycleMap(const uint16 fromColor, const uint16 numColorsToClear);

	/**
	 * Gets the cycler object that starts at the given `fromColor`, or NULL if
	 * there is no cycler for that color.
	 */
	PalCycler *getCycler(const uint16 fromColor);

	/**
	 * Advances all cyclers by one step, regardless of whether or not it is time
	 * to perform another cycle.
	 */
	void applyAllCycles();

	/**
	 * Advances, by one step, only the cyclers whose time has come to cycle.
	 */
	void applyCycles();

#pragma mark -
#pragma mark Fading
public:
	/**
	 * Sets the intensity level for a range of palette entries. An intensity of
	 * zero indicates total darkness. Intensity may also be set above 100
	 * percent to increase the intensity of a palette entry.
	 */
	void setFade(const uint16 percent, const uint8 fromColor, const uint16 toColor);

	/**
	 * Resets the intensity of all palette entries to 100%.
	 */
	void fadeOff();

	/**
	 * Applies intensity values to the palette entries in `_nextPalette`.
	 */
	void applyFade();

private:
	/**
	 * The intensity levels of each palette entry, in percent. Defaults to 100.
	 */
	uint16 _fadeTable[256];
};

} // End of namespace Sci

#endif
