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

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap32.h"

namespace Sci {

#pragma mark SingleRemap

void SingleRemap::reset() {
	_lastPercent = 100;
	_lastGray = 0;

	const uint8 remapStartColor = g_sci->_gfxRemap32->getStartColor();
	const Palette &currentPalette = g_sci->_gfxPalette32->getCurrentPalette();
	for (uint i = 0; i < remapStartColor; ++i) {
		const Color &color = currentPalette.colors[i];
		_remapColors[i] = i;
		_originalColors[i] = color;
		_originalColorsChanged[i] = true;
		_idealColors[i] = color;
		_idealColorsChanged[i] = false;
		_matchDistances[i] = 0;
	}
}

bool SingleRemap::update() {
	switch (_type) {
	case kRemapNone:
		break;
	case kRemapByRange:
		return updateRange();
	case kRemapByPercent:
		return updateBrightness();
	case kRemapToGray:
		return updateSaturation();
	case kRemapToPercentGray:
		return updateSaturationAndBrightness();
	default:
		error("Illegal remap type %d", _type);
	}

	return false;
}

bool SingleRemap::updateRange() {
	const uint8 remapStartColor = g_sci->_gfxRemap32->getStartColor();
	bool updated = false;

	for (uint i = 0; i < remapStartColor; ++i) {
		uint8 targetColor;
		if (_from <= i && i <= _to) {
			targetColor = i + _delta;
		} else {
			targetColor = i;
		}

		if (_remapColors[i] != targetColor) {
			updated = true;
			_remapColors[i] = targetColor;
		}

		_originalColorsChanged[i] = true;
	}

	return updated;
}

bool SingleRemap::updateBrightness() {
	const uint8 remapStartColor = g_sci->_gfxRemap32->getStartColor();
	const Palette &nextPalette = g_sci->_gfxPalette32->getNextPalette();
	for (uint i = 1; i < remapStartColor; ++i) {
		Color color(nextPalette.colors[i]);

		if (_originalColors[i] != color) {
			_originalColorsChanged[i] = true;
			_originalColors[i] = color;
		}

		if (_percent != _lastPercent || _originalColorsChanged[i]) {
			// SSCI checked if percent was over 100 and only then clipped
			// values, but we always unconditionally ensure the result is in the
			// correct range for simplicity's sake
			color.r = MIN(255, (uint16)color.r * _percent / 100);
			color.g = MIN(255, (uint16)color.g * _percent / 100);
			color.b = MIN(255, (uint16)color.b * _percent / 100);

			if (_idealColors[i] != color) {
				_idealColorsChanged[i] = true;
				_idealColors[i] = color;
			}
		}
	}

	const bool updated = apply();
	Common::fill(_originalColorsChanged, _originalColorsChanged + remapStartColor, false);
	Common::fill(_idealColorsChanged, _idealColorsChanged + remapStartColor, false);
	_lastPercent = _percent;
	return updated;
}

bool SingleRemap::updateSaturation() {
	const uint8 remapStartColor = g_sci->_gfxRemap32->getStartColor();
	const Palette &currentPalette = g_sci->_gfxPalette32->getCurrentPalette();
	for (uint i = 1; i < remapStartColor; ++i) {
		Color color(currentPalette.colors[i]);
		if (_originalColors[i] != color) {
			_originalColorsChanged[i] = true;
			_originalColors[i] = color;
		}

		if (_gray != _lastGray || _originalColorsChanged[i]) {
			const int luminosity = (((color.r * 77) + (color.g * 151) + (color.b * 28)) >> 8) * _percent / 100;

			color.r = MIN(255, color.r - ((color.r - luminosity) * _gray / 100));
			color.g = MIN(255, color.g - ((color.g - luminosity) * _gray / 100));
			color.b = MIN(255, color.b - ((color.b - luminosity) * _gray / 100));

			if (_idealColors[i] != color) {
				_idealColorsChanged[i] = true;
				_idealColors[i] = color;
			}
		}
	}

	const bool updated = apply();
	Common::fill(_originalColorsChanged, _originalColorsChanged + remapStartColor, false);
	Common::fill(_idealColorsChanged, _idealColorsChanged + remapStartColor, false);
	_lastGray = _gray;
	return updated;
}

bool SingleRemap::updateSaturationAndBrightness() {
	const uint8 remapStartColor = g_sci->_gfxRemap32->getStartColor();
	const Palette &currentPalette = g_sci->_gfxPalette32->getCurrentPalette();
	for (uint i = 1; i < remapStartColor; i++) {
		Color color(currentPalette.colors[i]);
		if (_originalColors[i] != color) {
			_originalColorsChanged[i] = true;
			_originalColors[i] = color;
		}

		if (_percent != _lastPercent || _gray != _lastGray || _originalColorsChanged[i]) {
			const int luminosity = (((color.r * 77) + (color.g * 151) + (color.b * 28)) >> 8) * _percent / 100;

			color.r = MIN(255, color.r - ((color.r - luminosity) * _gray) / 100);
			color.g = MIN(255, color.g - ((color.g - luminosity) * _gray) / 100);
			color.b = MIN(255, color.b - ((color.b - luminosity) * _gray) / 100);

			if (_idealColors[i] != color) {
				_idealColorsChanged[i] = true;
				_idealColors[i] = color;
			}
		}
	}

	const bool updated = apply();
	Common::fill(_originalColorsChanged, _originalColorsChanged + remapStartColor, false);
	Common::fill(_idealColorsChanged, _idealColorsChanged + remapStartColor, false);
	_lastPercent = _percent;
	_lastGray = _gray;
	return updated;
}

bool SingleRemap::apply() {
	const GfxRemap32 *const gfxRemap32 = g_sci->_gfxRemap32;
	const uint8 remapStartColor = gfxRemap32->getStartColor();

	// Blocked colors are not allowed to be used as target colors for the remap
	bool blockedColors[237];
	Common::fill(blockedColors, &blockedColors[237], false);

	const bool *const paletteCycleMap = g_sci->_gfxPalette32->getCycleMap();

	const int16 blockedRangeCount = gfxRemap32->getBlockedRangeCount();
	if (blockedRangeCount) {
		const uint8 blockedRangeStart = gfxRemap32->getBlockedRangeStart();
		Common::fill(blockedColors + blockedRangeStart, blockedColors + blockedRangeStart + blockedRangeCount, true);
	}

	for (uint i = 0; i < remapStartColor; ++i) {
		if (paletteCycleMap[i]) {
			blockedColors[i] = true;
		}
	}

	// SSCI did a loop over colors here to create a new array of updated,
	// unblocked colors, but then never used it

	bool updated = false;
	for (uint i = 1; i < remapStartColor; ++i) {
		int distance;

		if (!_idealColorsChanged[i] && !_originalColorsChanged[_remapColors[i]]) {
			continue;
		}

		if (
			_idealColorsChanged[i] &&
			_originalColorsChanged[_remapColors[i]] &&
			_matchDistances[i] < 100 &&
			colorDistance(_idealColors[i], _originalColors[_remapColors[i]]) <= _matchDistances[i]
		) {
			continue;
		}

		const int16 bestColor = matchColor(_idealColors[i], _matchDistances[i], distance, blockedColors);

		if (bestColor != -1 && _remapColors[i] != bestColor) {
			updated = true;
			_remapColors[i] = bestColor;
			_matchDistances[i] = distance;
		}
	}

	return updated;
}

int SingleRemap::colorDistance(const Color &a, const Color &b) const {
	int channelDistance = a.r - b.r;
	int distance = channelDistance * channelDistance;
	channelDistance = a.g - b.g;
	distance += channelDistance * channelDistance;
	channelDistance = a.b - b.b;
	distance += channelDistance * channelDistance;
	return distance;
}

int16 SingleRemap::matchColor(const Color &color, const int minimumDistance, int &outDistance, const bool *const blockedIndexes) const {
	int16 bestIndex = -1;
	int bestDistance = 0xFFFFF;
	int distance = minimumDistance;
	const Palette &nextPalette = g_sci->_gfxPalette32->getNextPalette();

	for (uint i = 0, channelDistance; i < g_sci->_gfxRemap32->getStartColor(); ++i) {
		if (blockedIndexes[i]) {
			continue;
		}

		distance = nextPalette.colors[i].r - color.r;
		distance *= distance;
		if (bestDistance <= distance) {
			continue;
		}
		channelDistance = nextPalette.colors[i].g - color.g;
		distance += channelDistance * channelDistance;
		if (bestDistance <= distance) {
			continue;
		}
		channelDistance = nextPalette.colors[i].b - color.b;
		distance += channelDistance * channelDistance;
		if (bestDistance <= distance) {
			continue;
		}
		bestDistance = distance;
		bestIndex = i;
	}

	// This value is only valid if the last index to perform a distance
	// calculation was the best index
	outDistance = distance;
	return bestIndex;
}

#pragma mark -
#pragma mark GfxRemap32

GfxRemap32::GfxRemap32() :
	_needsUpdate(false),
	_blockedRangeStart(0),
	_blockedRangeCount(0),
	_numActiveRemaps(0) {

	// Mac SSCI has one less remap entry than PC. Mac games expand the normal
	//  range from 236 entries to 237 and uses the extra entry (236) for black
	//  instead of 0. This was done to avoid conflicting with the operating
	//  system's palette which always uses entry 0 for white.
	if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
		_remapStartColor = 237;
	} else {
		_remapStartColor = 236;
	}

	if (g_sci->_features->hasMidPaletteCode()) {
		_remaps.resize(9);
	} else {
		_remaps.resize(19);
	}

	_remapEndColor = 236 + _remaps.size() - 1;
}

void GfxRemap32::remapOff(const uint8 color) {
	if (color == 0) {
		remapAllOff();
		return;
	}

	// SSCI simply ignored invalid input values, but we at least give a warning
	// so games can be investigated for script bugs
	if (color < _remapStartColor || color > _remapEndColor) {
		warning("GfxRemap32::remapOff: %d out of remap range", color);
		return;
	}

	const uint8 index = _remapEndColor - color;
	SingleRemap &singleRemap = _remaps[index];
	singleRemap._type = kRemapNone;
	--_numActiveRemaps;
	_needsUpdate = true;
}

void GfxRemap32::remapAllOff() {
	for (uint i = 0, len = _remaps.size(); i < len; ++i) {
		_remaps[i]._type = kRemapNone;
	}

	_numActiveRemaps = 0;
	_needsUpdate = true;
}

void GfxRemap32::remapByRange(const uint8 color, const int16 from, const int16 to, const int16 delta) {
	// SSCI simply ignored invalid input values, but we at least give a warning
	// so games can be investigated for script bugs
	if (color < _remapStartColor || color > _remapEndColor) {
		warning("GfxRemap32::remapByRange: %d out of remap range", color);
		return;
	}

	if (from < 0) {
		warning("GfxRemap32::remapByRange: attempt to remap negative color %d", from);
		return;
	}

	if (to >= _remapStartColor) {
		warning("GfxRemap32::remapByRange: attempt to remap into the remap zone at %d", to);
		return;
	}

	const uint8 index = _remapEndColor - color;
	SingleRemap &singleRemap = _remaps[index];

	if (singleRemap._type == kRemapNone) {
		++_numActiveRemaps;
		singleRemap.reset();
	}

	singleRemap._from = from;
	singleRemap._to = to;
	singleRemap._delta = delta;
	singleRemap._type = kRemapByRange;
	_needsUpdate = true;
}

void GfxRemap32::remapByPercent(const uint8 color, const int16 percent) {
	// SSCI simply ignored invalid input values, but we at least give a warning
	// so games can be investigated for script bugs
	if (color < _remapStartColor || color > _remapEndColor) {
		warning("GfxRemap32::remapByPercent: %d out of remap range", color);
		return;
	}

	const uint8 index = _remapEndColor - color;
	SingleRemap &singleRemap = _remaps[index];

	if (singleRemap._type == kRemapNone) {
		++_numActiveRemaps;
		singleRemap.reset();
	}

	singleRemap._percent = percent;
	singleRemap._type = kRemapByPercent;
	_needsUpdate = true;
}

void GfxRemap32::remapToGray(const uint8 color, const int8 gray) {
	// SSCI simply ignored invalid input values, but we at least give a warning
	// so games can be investigated for script bugs
	if (color < _remapStartColor || color > _remapEndColor) {
		warning("GfxRemap32::remapToGray: %d out of remap range", color);
		return;
	}

	if (gray < 0 || gray > 100) {
		error("RemapToGray percent out of range; gray = %d", gray);
	}

	const uint8 index = _remapEndColor - color;
	SingleRemap &singleRemap = _remaps[index];

	if (singleRemap._type == kRemapNone) {
		++_numActiveRemaps;
		singleRemap.reset();
	}

	singleRemap._gray = gray;
	singleRemap._type = kRemapToGray;
	_needsUpdate = true;
}

void GfxRemap32::remapToPercentGray(const uint8 color, const int16 gray, const int16 percent) {
	// SSCI simply ignored invalid input values, but we at least give a warning
	// so games can be investigated for script bugs
	if (color < _remapStartColor || color > _remapEndColor) {
		warning("GfxRemap32::remapToPercentGray: %d out of remap range", color);
		return;
	}

	const uint8 index = _remapEndColor - color;
	SingleRemap &singleRemap = _remaps[index];

	if (singleRemap._type == kRemapNone) {
		++_numActiveRemaps;
		singleRemap.reset();
	}

	singleRemap._percent = percent;
	singleRemap._gray = gray;
	singleRemap._type = kRemapToPercentGray;
	_needsUpdate = true;
}

void GfxRemap32::blockRange(const uint8 from, const int16 count) {
	_blockedRangeStart = from;
	_blockedRangeCount = count;
}

bool GfxRemap32::remapAllTables(const bool paletteUpdated) {
	if (!_needsUpdate && !paletteUpdated) {
		return false;
	}

	bool updated = false;

	for (SingleRemapsList::iterator it = _remaps.begin(); it != _remaps.end(); ++it) {
		if (it->_type != kRemapNone) {
			updated |= it->update();
		}
	}

	_needsUpdate = false;
	return updated;
}
} // End of namespace Sci
