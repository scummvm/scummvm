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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Immutable WBASE resource model.

#ifndef HOPKINS_BASE_DATA_H
#define HOPKINS_BASE_DATA_H

#include "hopkins/base_types.h"

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"

namespace Hopkins {

class BaseData {
public:
	BaseData();

	static bool hasRequiredResources(Common::String *missingResources = nullptr);
	static void appendAudioResourceReport(Common::String &missingResources);

	bool load(Common::String &errorMessage);

	uint16 mapCodeAt(int mapPos) const;
	uint16 mapCodeXY(int x, int y) const;
	uint16 objectCodeAt(int mapPos) const;

	const byte *palette() const { return _palette; }
	const BaseBitmap &wallBitmap(uint id) const;
	const BaseBitmap &objectBitmap(uint id) const;
	const BaseBitmap &weaponFrame(uint id) const;
	const BaseBitmap &fontFrame(uint id) const;

	int32 sinQ16(int angle) const;
	int32 cosQ16(int angle) const;
	int32 longTanQ16(int angle) const;
	int32 longInvTanQ16(int angle) const;
	int32 invCos(int angle) const;
	int32 invSin(int angle) const;
	int32 longCosQ16(int angle) const;
	int32 xNextQ16(int angle) const;
	int32 yNextQ16(int angle) const;
	int32 viewCosQ16(int column) const;
	int32 floorCos(int column) const;
	int16 distanceHeight(int distance) const;
	int32 adjustTable(int distance) const;

private:
	bool loadMap(Common::String &errorMessage);
	bool loadPalette(Common::String &errorMessage);
	bool loadInfo(Common::String &errorMessage);
	bool loadBitmaps(Common::String &errorMessage);
	bool loadBbm(const Common::Path &filename, BaseBitmap &bitmap, Common::String &errorMessage);
	bool loadSpr(const Common::Path &filename, Common::Array<BaseBitmap> &frames, Common::String &errorMessage);
	void buildDerivedTables();

	uint16 _map[kBaseMapCellCount];
	uint16 _objectMap[kBaseMapCellCount];
	byte _palette[256 * 3];

	Common::Array<int32> _trig[7];
	Common::Array<int32> _xNext;
	Common::Array<int32> _yNext;
	int32 _viewCos[kBaseViewWidth];
	int32 _floorCos[kBaseViewWidth + 1];
	int16 _distanceHeight[kBaseMaximumDistance + 1];
	int32 _adjust[kBaseMaximumDistance + 1];

	Common::Array<BaseBitmap> _wallBitmaps;
	Common::Array<BaseBitmap> _objectBitmaps;
	Common::Array<BaseBitmap> _weaponFrames;
	Common::Array<BaseBitmap> _fontFrames;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_DATA_H
