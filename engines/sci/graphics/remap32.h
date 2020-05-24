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

#ifndef SCI_GRAPHICS_REMAP32_H
#define SCI_GRAPHICS_REMAP32_H

#include "common/algorithm.h"
#include "common/array.h"
#include "common/scummsys.h"
#include "sci/graphics/helpers.h"

namespace Sci {
class GfxPalette32;

enum RemapType {
	kRemapNone = 0,
	kRemapByRange = 1,
	kRemapByPercent = 2,
	kRemapToGray = 3,
	kRemapToPercentGray = 4
};

#pragma mark -
#pragma mark SingleRemap

/**
 * SingleRemap objects each manage one remapping operation.
 */
class SingleRemap {
public:
	SingleRemap() : _type(kRemapNone) {}

	/**
	 * The type of remap.
	 */
	RemapType _type;

	/**
	 * The first color that should be shifted by a range remap.
	 */
	uint8 _from;

	/**
	 * The last color that should be shifted a range remap.
	 */
	uint8 _to;

	/**
	 * The direction and amount that the colors should be shifted in a range
	 * remap.
	 */
	int16 _delta;

	/**
	 * The difference in brightness that should be applied by a brightness
	 * (percent) remap.
	 *
	 * This value may be be greater than 100, in which case the color will be
	 * oversaturated.
	 */
	int16 _percent;

	/**
	 * The amount of desaturation that should be applied by a saturation (gray)
	 * remap, where 0 is full saturation and 100 is full desaturation.
	 */
	uint8 _gray;

	/**
	 * The final array used by CelObj renderers to composite remapped pixels to
	 * the screen buffer.
	 *
	 * Here is how it works:
	 *
	 * The source bitmap being rendered will have pixels within the remap range
	 * (236-245 or 236-254), and the target buffer will have colors in the
	 * non-remapped range (0-235).
	 *
	 * To arrive at the correct color, first the source pixel is used to look up
	 * the correct SingleRemap for that pixel. Then, the final composited color
	 * is looked up in this array using the target's pixel color. In other
	 * words,
	 * `target = _remaps[remapEndColor - source].remapColors[target]`.
	 *
	 * Mac SSCI includes entry 236 in the non-remapped range at the cost of one
	 * remap entry and so the Mac remap range starts at 237. The final entry in
	 * this array, and the others in this class, is not used by PC games.
	 */
	uint8 _remapColors[237];

	/**
	 * Resets this SingleRemap's color information to default values.
	 */
	void reset();

	/**
	 * Recalculates and reapplies remap colors to the `_remapColors` array.
	 */
	bool update();

private:
	/**
	 * The previous brightness value. Used to determine whether or not
	 * `_idealColors` needs to be updated.
	 */
	int16 _lastPercent;

	/**
	 * The previous saturation value. Used to determine whether or not
	 * `_idealColors` needs to be updated.
	 */
	uint8 _lastGray;

	/**
	 * The colors from the current GfxPalette32 palette before this SingleRemap
	 * is applied.
	 */
	Color _originalColors[237];

	/**
	 * Map of colors that changed in `_originalColors` when this SingleRemap was
	 * updated. This map is transient and gets reset to `false` after the
	 * SingleRemap finishes updating.
	 */
	bool _originalColorsChanged[237];

	/**
	 * The ideal target RGB color values for each generated remap color.
	 */
	Color _idealColors[237];

	/**
	 * Map of colors that changed in `_idealColors` when this SingleRemap was
	 * updated. This map is transient and gets reset to `false` after the
	 * SingleRemap finishes applying.
	 */
	bool _idealColorsChanged[237];

	/**
	 * When applying a SingleRemap, finding an appropriate color in the palette
	 * is the responsibility of a distance function. Once a match is found, the
	 * distance of that match is stored here so that the next time the
	 * SingleRemap is applied, it can check the distance from the previous
	 * application and avoid triggering an expensive redraw of the entire screen
	 * if the new palette value only changed slightly.
	 */
	int _matchDistances[237];

	/**
	 * Computes the final target values for a range remap and applies them
	 * directly to the `_remaps` map.
	 *
	 * @note Was ByRange in SSCI.
	 */
	bool updateRange();

	/**
	 * Computes the intermediate target values for a brightness remap and
	 * applies them indirectly via the `apply` method.
	 *
	 * @note Was ByPercent in SSCI.
	 */
	bool updateBrightness();

	/**
	 * Computes the intermediate target values for a saturation remap and
	 * applies them indirectly via the `apply` method.
	 *
	 * @note Was ToGray in SSCI.
	 */
	bool updateSaturation();

	/**
	 * Computes the intermediate target values for a saturation + brightness
	 * bitmap and applies them indirectly via the `apply` method.
	 *
	 * @note Was ToPercentGray in SSCI.
	 */
	bool updateSaturationAndBrightness();

	/**
	 * Computes and applies the final values to the `_remaps` map.
	 *
	 * @note In SSCI, a boolean array of changed values was passed into this
	 * method, but this was done by creating arrays on the stack in the caller.
	 * Instead of doing this, we simply add another member property
	 * `_idealColorsChanged` and use that instead.
	 */
	bool apply();

	/**
	 * Calculates the square distance of two colors.
	 *
	 * @note In SSCI this method is Rgb24::Dist, but it is only used by
	 * SingleRemap.
	 */
	int colorDistance(const Color &a, const Color &b) const;

	/**
	 * Finds the closest index in the next palette matching the given RGB color.
	 * Returns -1 if no match can be found that is closer than
	 * `minimumDistance`.
	 *
	 * @note In SSCI, this method is SOLPalette::Match, but this particular
	 * signature is only used by SingleRemap.
	 */
	int16 matchColor(const Color &color, const int minimumDistance, int &outDistance, const bool *const blockedIndexes) const;
};

#pragma mark -
#pragma mark GfxRemap32

/**
 * This class provides color remapping support for SCI32 games.
 */
class GfxRemap32 : public Common::Serializable {
public:
	GfxRemap32();

	void saveLoadWithSerializer(Common::Serializer &s) override;

	inline uint8 getRemapCount() const { return _numActiveRemaps; }
	inline uint8 getStartColor() const { return _remapStartColor; }
	inline uint8 getEndColor() const { return _remapEndColor; }
	inline uint8 getBlockedRangeStart() const { return _blockedRangeStart; }
	inline int16 getBlockedRangeCount() const { return _blockedRangeCount; }

	/**
	 * Turns off remapping of the given color. If `color` is 0, all remaps are
	 * turned off.
	 */
	void remapOff(const uint8 color);

	/**
	 * Turns off all color remaps.
	 */
	void remapAllOff();

	/**
	 * Configures a SingleRemap for the remap color `color`. The SingleRemap
	 * will shift palette colors between `from` and `to` (inclusive) by `delta`
	 * palette entries when the remap is applied.
	 */
	void remapByRange(const uint8 color, const int16 from, const int16 to, const int16 delta);

	/**
	 * Configures a SingleRemap for the remap color `color` to modify the
	 * brightness of remapped colors by `percent`.
	 */
	void remapByPercent(const uint8 color, const int16 percent);

	/**
	 * Configures a SingleRemap for the remap color `color` to modify the
	 * saturation of remapped colors by `gray`.
	 */
	void remapToGray(const uint8 color, const int8 gray);

	/**
	 * Configures a SingleRemap for the remap color `color` to modify the
	 * brightness of remapped colors by `percent`, and saturation of remapped
	 * colors by `gray`.
	 */
	void remapToPercentGray(const uint8 color, const int16 gray, const int16 percent);

	/**
	 * Prevents GfxRemap32 from using the given range of palette entries as
	 * potential remap targets.
	 *
	 * @NOTE Was DontMapToRange in SSCI.
	 */
	void blockRange(const uint8 from, const int16 count);

	/**
	 * Determines whether or not the given color has an active remapper. If it
	 * does not, it is treated as a skip color and the pixel is not drawn.
	 *
	 * @note SSCI uses a boolean array to decide whether a pixel is remapped,
	 * but it is possible to get the same information from `_remaps`, as this
	 * function does. Presumably, the separate array was created for performance
	 * reasons, since this is called a lot in the most critical section of the
	 * renderer.
	 */
	inline bool remapEnabled(uint8 color) const {
		const uint8 index = _remapEndColor - color;
		// At least KQ7 DOS uses remap colors that are outside the valid remap
		// range; in these cases, just treat those pixels as skip pixels (which
		// is how they would be treated in SSCI)
		if (index >= _remaps.size()) {
			return false;
		}
		return (_remaps[index]._type != kRemapNone);
	}

	/**
	 * Calculates the correct color for a target by looking up the target color
	 * in the SingleRemap that controls the given sourceColor. If there is no
	 * remap for the given color, it will be treated as a skip color.
	 */
	inline uint8 remapColor(const uint8 sourceColor, const uint8 targetColor) const {
		const uint8 index = _remapEndColor - sourceColor;
		assert(index < _remaps.size());
		const SingleRemap &singleRemap = _remaps[index];
		assert(singleRemap._type != kRemapNone);
		// SSCI never really properly handled attempts to draw to a target with
		// pixels above the remap color maximum. In RAMA, the cursor views have
		// a remap color outlining the cursor, and so get drawn into a target
		// surface filled with a skip color of 255. In SSCI, this causes the
		// remapped color to be read from some statically allocated, never
		// written memory and so always ends up being 0 (black).
		if (targetColor >= _remapStartColor) {
			return 0;
		}
		return singleRemap._remapColors[targetColor];
	}

	/**
	 * Updates all active remaps in response to a palette change or a remap
	 * settings change.
	 *
	 * `paletteChanged` is true if the next palette in GfxPalette32 has been
	 * previously modified by other palette operations.
	 */
	bool remapAllTables(const bool paletteUpdated);

private:
	typedef Common::Array<SingleRemap> SingleRemapsList;

	/**
	 * The first index of the remap area in the system palette.
	 */
	uint8 _remapStartColor;

	/**
	 * The last index of the remap area in the system palette.
	 */
	uint8 _remapEndColor;

	/**
	 * The number of currently active remaps.
	 */
	uint8 _numActiveRemaps;

	/**
	 * The list of SingleRemaps.
	 */
	SingleRemapsList _remaps;

	/**
	 * If true, indicates that one or more SingleRemaps were reconfigured and
	 * all remaps need to be recalculated.
	 */
	bool _needsUpdate;

	/**
	 * The first color that is blocked from being used as a remap target color.
	 */
	uint8 _blockedRangeStart;

	/**
	 * The size of the range of blocked colors. If zero, all colors are
	 * potential targets for remapping.
	 */
	int16 _blockedRangeCount;
};
} // End of namespace Sci
#endif
