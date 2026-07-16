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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_WORLD_MAP_H
#define RIPPER_WORLD_MAP_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;

class WorldMap {
public:
	explicit WorldMap(RipperEngine *engine);

	bool initialize(ResourceManager &resources);
	bool run(Common::String &targetScript);
	uint locationCount() const { return _locations.size(); }
	bool isLocationAvailable(uint locationIndex) const;
	const Common::String &locationName(uint locationIndex) const;

private:
	struct TravelLocation {
		Common::String name;
		Common::String targetPattern;
		Common::Array<uint16> entryIndices;
		bool available;

		TravelLocation() : available(false) {}
	};

	bool captureDisplay();
	void restoreDisplay();
	void refreshLocations();
	void draw() const;
	void drawText(byte *screen, uint pitch, int x, int y, const Common::String &text,
		byte color) const;
	int findVisibleLocation(const Common::Point &point) const;
	void updateFirstVisible();
	uint resolveChapter() const;
	Common::String resolveTargetScript(uint locationIndex) const;
	const Common::String &resourceString(uint resourceId) const;

	RipperEngine *_engine;
	BitmapAssetFrame _background;
	BitmapFontAsset _font;
	Common::Array<Common::String> _gameText;
	Common::Array<TravelLocation> _locations;
	Common::Array<uint> _visibleLocations;
	Common::Array<byte> _savedPixels;
	Common::Array<byte> _savedPalette;
	uint _selectedVisible;
	uint _firstVisible;
	int _pressedVisible;
	bool _initialized;
};

} // End of namespace Ripper

#endif // RIPPER_WORLD_MAP_H
