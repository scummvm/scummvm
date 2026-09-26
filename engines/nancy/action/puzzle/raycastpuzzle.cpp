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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/action/puzzle/raycastpuzzle.h"
#include "engines/nancy/state/scene.h"

#include "common/stack.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

enum WallFlags {
	kWall 				= 0x01000000,
	kVertical			= 0x02000000,
	kHorizontal			= 0x04000000,
	kDoor				= 0x08000000,
	kHasBlankWalls		= 0x10000000,
	kTransparentWall 	= 0x20000000,
};

static const uint16 wallLightmapValues[8] = {
	1 << 0 | 0 << 4 | 0 << 8,
	1 << 0 | 0 << 4 | 0 << 8,
	2 << 0 | 1 << 4 | 0 << 8,
	4 << 0 | 2 << 4 | 0 << 8,
	5 << 0 | 3 << 4 | 2 << 8,
	5 << 0 | 4 << 4 | 3 << 8,
	6 << 0 | 5 << 4 | 4 << 8,
	7 << 0 | 6 << 4 | 5 << 8
};

static const byte floorCeilingLightmapValues[8] = {
	0 << 0 | 0 << 4,
	1 << 0 | 0 << 4,
	2 << 0 | 0 << 4,
	3 << 0 | 1 << 4,
	4 << 0 | 2 << 4,
	5 << 0 | 3 << 4,
	6 << 0 | 4 << 4,
	7 << 0 | 5 << 4
};

void clampRotation(int32 &rot) {
	rot = rot 
< 0 ? rot + 4096 : rot;
	rot = rot > 4095 ? rot - 4096 : rot;
}

class RaycastLevelBuilder {
public:
	RaycastLevelBuilder(uint width, uint height, uint verticalHeight);

	void fillCells();
	void fillWalls();
	void fillLocalWallAndInfo();
	void writeThemesAndExitFloor();
	uint writeTheme(uint startX, uint startY, uint quadrant);
	void writeTransparentWalls(uint startX, uint startY, uint themeID);
	void writeObjectWalls(uint startX, uint startY, uint themeID);
	void writeDoors(uint startX, uint startY, uint themeID);
	void writeLightSwitch(uint startX, uint startY, uint quadrant);
	void writeExitFloorTexture(uint themeID);

	Common::Array<uint32> _wallMap, _infoMap;
	Common::Array<int16> _floorMap, _ceilingMap;
	Common::Array<uint16> _wallLightMap, _floorCeilingLightMap, _heightMap;

	uint _inputWidth, _inputHeight, _inputNumCells;
	uint _halfWidth, _halfHeight, _halfNumCells;
	uint _fullWidth, _fullHeight, _fullNumCells;
	uint _verticalHeight;
	float _objectsBaseDensity;

	uint _startX, _startY;

	Common::Array<uint16> _cells;
	Common::Array<byte> _walls;

	const RCLB *_themeData;
};

RaycastLevelBuilder::RaycastLevelBuilder(uint width, uint height, uint verticalHeight) {
	_themeData = GetEngineData(RCLB);
	assert(_themeData);

	_verticalHeight = verticalHeight;

	_inputWidth = width;
	_inputHeight = height;
	_inputNumCells = width * height;

	_halfWidth = width * 2 + 1;
	_halfHeight = height * 2 + 1;
	_halfNumCells = _halfWidth * _halfHeight;

	_fullWidth = _halfWidth * 2;
	_fullHeight = _halfHeight * 2;
	_fullNumCells = _fullWidth * _fullHeight;

	_objectsBaseDensity = (float)_fullNumCells / 1764.0;

	_cells.resize(_inputNumCells, 0xF);
	_walls.resize(_halfNumCells, 0);
	_wallMap.resize(_fullNumCells, 0);
	_floorMap.resize(_fullNumCells, -1);
	_ceilingMap.resize(_fullNumCells, -1);
	_infoMap.resize(_fullNumCells, 0);
	_wallLightMap.resize(_fullNumCells, 0);
	_floorCeilingLightMap.resize(_fullNumCells, 0);
	_heightMap.resize(_fullNumCells, verticalHeight * 128);

	f
illCells();
	fillWalls();
	fillLocalWallAndInfo();
	writeThemesAndExitFloor();
}

void RaycastLevelBuilder::fillCells() {
	Common::Stack<Common::Point> cellStack;
	Common::Array<byte> switchStack;

	Common::Point curCell(0, 0);

	for (uint i = 1; i < _inputNumCells;) {
		switchStack.clear();

		if ( (curCell.y > 0) && ((_cells[(curCell.y - 1) * _inputWidth + curCell.x] & 0xF) == 0xF) ) {
			switchStack.push_back(2);
		}

		if ( (curCell.y + 1 <= (int)_inputHeight - 1) && ((_cells[(curCell.y + 1) * _inputWidth + curCell.x] & 0xF) == 0xF) ) {
			switchStack.push_back(3);
		}

		if ( (curCell.x > 0) && ((_cells[curCell.y * _inputWidth + curCell.x - 1] & 0xF) == 0xF) ) {
			switchStack.push_back(0);
		}

		if ( (curCell.x + 1 <= (int)_inputWidth - 1) && ((_cells[curCell.y * _inputWidth + curCell.x + 1] & 0xF) == 0xF) ) {
			switchStack.push_back(1);
		}

		uint numOptions = switchStack.size();
		if (numOptions) {
			switch (switchStack[g_nancy->_randomSource->getRandomNumber(numOptions - 1)]) {
			case 0:
				_cells[curCell.y * _inputWidth + curCell.x] 		&= 0xFFF7;
				_cells[curCell.y * _inputWidth + curCell.x - 1] 	&= 0xFFFB;
				cellStack.push(curCell);
				--curCell.x;
				break;
			case 1:
				_cells[curCell.y * _inputWidth + curCell.x] 		&= 0xFFFB;
				_cells[curCell.y * _inputWidth + curCell.x + 1] 	&= 0xFFF7;
				cellStack.push(curCell);
				++curCell.x;
				break;
			case 2:
				_cells[curCell.y * _inputWidth + curCell.x] 		&= 0xFFFD;
				_cells[(curCell.y - 1) * _inputWidth + curCell.x] 	&= 0xFFFE;
				cellStack.push(curCell);
				--curCell.y;
				break;
			case 3:
				_cells[curCell.y * _inputWidth + curCell.x] 		&= 0xFFFE;
				_cells[(curCell.y + 1) * _inputWidth + curCell.x] 	&= 0xFFFD;
				cellStack.push(curCell);
				++curCell.y;
				break;
			}

			++i;
		} else {
			curCell = cellStack.pop();
		}
	}
}

void RaycastLevelBuilder::fillWalls() {
	// Surround the whole map with walls
	for (uint y = 0; y < _halfHeight; ++y) {
		if (y == 0 || y == _halfHeight 
- 1) {
			for (uint x = 0; x < _halfWidth; ++x) {
				_walls[y * _halfWidth + x] |= 1;
			}
		} else {
			_walls[y * _halfWidth] |= 1;
			_walls[y * _halfWidth + _halfWidth - 1] |= 1;
		}
	}

	uint y = 1;
	for (uint j = 0; j < _inputHeight; ++j) {
		uint x = 1;
		for (uint i = 0; i < _inputWidth; ++i) {
			if (j == _inputHeight - 1 && i == _inputHeight - 1) {
				_walls[y * _halfWidth + x] |= 2;
			}

			if (j == 0 && i == 0) {
				_walls[y * _halfWidth + x] |= 4;
			}

			if (_cells[j * _inputWidth + i] & 4) {
				_walls[y * _halfWidth + x + 1] |= 1;
			}

			if (_cells[j * _inputWidth + i] & 1) {
				_walls[(y + 1) * _halfWidth + x] |= 1;
			}

			_walls[(y + 1) * _halfWidth + x + 1] |= 1;
			x += 2;
		}

		y += 2;
	}
}

void RaycastLevelBuilder::fillLocalWallAndInfo() {
	uint y = 0;
	for (uint j = 0; j < _halfHeight; ++j) {
		uint x = 0;
		for (uint i = 0; i < _halfWidth; ++i) {
			byte curCell = _walls[j * _halfWidth + i];

			if (curCell & 2) { // end point flag
				_infoMap[y * _fullWidth + x] = 1;
				_infoMap[y * _fullWidth + x + 1] = 1;
				_infoMap[(y + 1) * _fullWidth + x] = 1;
				_infoMap[(y + 1) * _fullWidth + x + 1] = 1;
			}

			if (curCell & 4) { // start point flag
				_startX = x;
				_startY = y;
			}

			if (curCell & 1) {
				_wallMap[y * _fullWidth + x] = 1;
				_wallMap[y * _fullWidth + x + 1] = 1;
				_wallMap[(y + 1) * _fullWidth + x] = 1;
				_wallMap[(y + 1) * _fullWidth + x + 1] = 1;
			}

			x += 2;
		}

		y += 2;
	}
}

void RaycastLevelBuilder::writeThemesAndExitFloor() {
	writeTheme(0, 0, 1);
	writeTheme(_halfWidth, 0, 2);
	writeTheme(0, _halfHeight, 3);
	uint exitThemeID = writeTheme(_halfWidth, _halfHeight, 4);

	writeExitFloorTexture(exitThemeID);
}

uint RaycastLevelBuilder::writeTheme(uint startX, uint startY, uint quadrant) {
	uint themeID = g_nancy->_randomSource->getRandomNumber(_themeData->themes.size() - 1);
	const RCLB::Theme &theme = _themeData->themes[themeID];

	uint themeHalfWidth, themeHalfHeight;

	themeHalfWidth = _i
nputWidth + 1;
	themeHalfHeight = _inputHeight + 1;
	themeHalfWidth = themeHalfWidth < 2 ? 1 : themeHalfWidth;
	themeHalfHeight = themeHalfHeight < 2 ? 1 : themeHalfHeight;

	for (uint i = 0; i < 4; ++i) {
		uint selectedWallIDs = theme.wallIDs[g_nancy->_randomSource->getRandomNumber(theme.wallIDs.size() - 1)];
		uint halfY = 0;
		for (uint fullY = startY + (themeHalfHeight * (i % 2)); halfY < themeHalfHeight && fullY < _fullHeight; ++fullY) {
			uint halfX = 0;
			for (uint fullX = startX + (themeHalfWidth * (i / 2)); halfX < themeHalfWidth && fullX < _fullWidth; ++fullX) {
				if (_wallMap[fullY * _fullWidth + fullX] == 1) {
					_wallMap[fullY * _fullWidth + fullX] = selectedWallIDs | kWall;
					_wallLightMap[fullY * _fullWidth + fullX] = wallLightmapValues[theme.generalLighting] | (quadrant << 0xC);
				}
				++halfX;
			}
			++halfY;
		}
	}

	uint selectedFloorID = theme.floorIDs[g_nancy->_randomSource->getRandomNumber(theme.floorIDs.size() - 1)];
	uint selectedCeilingID = theme.ceilingIDs[g_nancy->_randomSource->getRandomNumber(theme.ceilingIDs.size() - 1)];

	for (uint y = 0; y < _halfHeight; ++y) {
		for (uint x = 0; x < _halfWidth; ++x) {
			_floorMap[(startY + y) * _fullWidth + startX + x] = selectedFloorID;
			_ceilingMap[(startY + y) * _fullWidth + startX + x] = selectedCeilingID;
			_floorCeilingLightMap[(startY + y) * _fullWidth + startX + x] = floorCeilingLightmapValues[theme.generalLighting] | (quadrant << 0xC);
		}
	}

	if (theme.transparentWallDensity > 0) {
		writeTransparentWalls(startX, startY, themeID);
	}

	if (theme.objectWallDensity > 0) {
		writeObjectWalls(startX, startY, themeID);
	}

	if (theme.doorDensity > 0) {
		writeDoors(startX, startY, themeID);
	}

	if (theme.hasLightSwitch) {
		// This is called with quadrant, NOT themeID
		writeLightSwitch(startX, startY, quadrant);
	}

	return themeID;
}

void RaycastLevelBuilder::writeTransparentWalls(uint startX, uint startY, uint themeID) {
	const RCLB::Theme &theme = _themeData->themes[the
meID];
	uint numWallsToWrite = (int)((float)theme.objectWallDensity * _objectsBaseDensity);

	for (uint numWrittenWalls = 0; numWrittenWalls < numWallsToWrite;) {
		bool vertical = g_nancy->_randomSource->getRandomBit();
		bool foundWallLocation = false;

		uint x = 0;
		uint y = 0;
		uint otherX = 0;
		uint otherY = 0;

		for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundWallLocation; ++checkedCells) {
			x = g_nancy->_randomSource->getRandomNumberRng(MAX<uint>(4, startX), MIN(startX + _halfWidth, _fullWidth - 5));
			y = g_nancy->_randomSource->getRandomNumberRng(MAX<uint>(4, startY), MIN(startY + _halfHeight, _fullHeight - 5));

			if (_wallMap[y * _fullWidth + x] & kWall) {
				if (vertical) {
					if ( 	!(_wallMap[(y - 1) * _fullWidth + x] & kWall) 	||
							(_wallMap[(y - 2) * _fullWidth + x])				||
							(_wallMap[(y + 1) * _fullWidth + x]) ) {

						if (	(_wallMap[(y + 1) * _fullWidth + x] & kWall)	&&
								!(_wallMap[(y + 2) * _fullWidth + x])				&&
								!(_wallMap[(y - 1) * _fullWidth + x]) ) {

							otherY = y + 1;
							otherX = x;

							if (	(_wallMap[y * _fullWidth + x - 1] & kWall)		&&
									(_wallMap[y * _fullWidth + x + 1] & kWall)		&&
									(_wallMap[otherY * _fullWidth + x - 1] & kWall)	&&
									(_wallMap[otherY * _fullWidth + x + 1] & kWall) ) {

								foundWallLocation = true;
							}
						}
					} else {
						otherY = y - 1;
						otherX = x;

						if (	(_wallMap[y * _fullWidth + x - 1] & kWall)		&&
								(_wallMap[y * _fullWidth + x + 1] & kWall)		&&
								(_wallMap[otherY * _fullWidth + x - 1] & kWall)	&&
								(_wallMap[otherY * _fullWidth + x + 1] & kWall) ) {

							foundWallLocation = true;
						}
					}
				} else {
					if ( 	!(_wallMap[y * _fullWidth + x - 1] & kWall)	||
							(_wallMap[y * _fullWidth + x - 2])				||
							(_wallMap[y * _fullWidth + x + 1]) ) {

						if (	(_wallMap[y * _fullWidth + x + 1] & kWall)	&&
								!(_wallMap[y * _fullWidth + x + 2])				&&
								!(_wall
Map[y * _fullWidth + x - 1]) ) {

							otherY = y;
							otherX = x + 1;

							if (	(_wallMap[(y - 1) * _fullWidth + x] & kWall)		&&
									(_wallMap[(y + 1) * _fullWidth + x] & kWall)		&&
									(_wallMap[(y - 1) * _fullWidth + otherX] & kWall)	&&
									(_wallMap[(y + 1) * _fullWidth + otherX] & kWall) ) {

								foundWallLocation = true;
							}
						}
					} else {
						otherY = y;
						otherX = x - 1;

						if (	(_wallMap[(y - 1) * _fullWidth + x] & kWall)		&&
								(_wallMap[(y + 1) * _fullWidth + x] & kWall)		&&
								(_wallMap[(y - 1) * _fullWidth + otherX] & kWall)	&&
								(_wallMap[(y + 1) * _fullWidth + otherX] & kWall) ) {

							foundWallLocation = true;
						}
					}
				}
			}
		}

		// No more suitable locations, exit
		if (!foundWallLocation) {
			break;
		}

		uint selectedTransparentWallIDs = theme.transparentwallIDs[theme.transparentwallIDs.size() - 1];

		if (vertical) {
			selectedTransparentWallIDs |= (kVertical | kHasBlankWalls | kTransparentWall);
		} else {
			selectedTransparentWallIDs |= (kHorizontal | kHasBlankWalls | kTransparentWall);
		}

		_wallMap[y * _fullWidth + x] = selectedTransparentWallIDs;
		_wallMap[otherY * _fullWidth + otherX] = selectedTransparentWallIDs;

		++numWrittenWalls;
	}
}

void RaycastLevelBuilder::writeObjectWalls(uint startX, uint startY, uint themeID) {
	const RCLB::Theme &theme = _themeData->themes[themeID];
	uint numWallsToWrite = (int)((float)theme.objectWallDensity * _objectsBaseDensity);

	uint textureVerticalHeight = _verticalHeight * 128; // 128 is a constant inside RayCast

	for (uint numWrittenWalls = 0; numWrittenWalls < numWallsToWrite;) {
		bool vertical = g_nancy->_randomSource->getRandomBit();
		bool foundWallLocation = false;

		uint x = 0;
		uint y = 0;

		for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundWallLocation; ++checkedCells) {
			x = g_nancy->_randomSource->getRandomNumberRng(MAX(startX, 1U), MIN(startX + _halfWidth, _fullWidth - 2));
			y =
 g_nancy->_randomSource->getRandomNumberRng(MAX(startY, 1U), MIN(startY + _halfHeight, _fullHeight - 2));

			if (_wallMap[y * _fullWidth + x] & kWall) {
				if (vertical) {
					if (	!(_wallMap[(y - 1) * _fullWidth + x] & kWall) 		||
							!(_wallMap[y * _fullWidth + x - 1] & kWall) 		||
							!(_wallMap[y * _fullWidth + x + 1] & kWall) 		||
							!(_wallMap[(y - 1) * _fullWidth + x - 1] & kWall) 	||
							!(_wallMap[(y - 1) * _fullWidth + x + 1] & kWall) 	||
							_wallMap[(y + 1) * _fullWidth + x] ) {

						if (	(_wallMap[(y + 1) * _fullWidth + x] & kWall)		&&
								(_wallMap[y * _fullWidth + x - 1] & kWall)			&&
								(_wallMap[y * _fullWidth + x + 1] & kWall)			&&
								(_wallMap[(y + 1) * _fullWidth + x - 1] & kWall) 	&&
								(_wallMap[(y + 1) * _fullWidth + x + 1] & kWall)	&&
								!_wallMap[(y - 1) * _fullWidth + x] ) {

							foundWallLocation = true;
						}
					} else {
						foundWallLocation = true;
					}
				} else {
					if (	!(_wallMap[y * _fullWidth + x - 1] & kWall) 		||
							!(_wallMap[(y - 1) * _fullWidth + x] & kWall) 		||
							!(_wallMap[(y + 1) * _fullWidth + x] & kWall) 		||
							!(_wallMap[(y - 1) * _fullWidth + x - 1] & kWall) 	||
							!(_wallMap[(y + 1) * _fullWidth + x - 1] & kWall) 	||
							_wallMap[y * _fullWidth + x + 1] ) {

						if (	(_wallMap[y * _fullWidth + x + 1] & kWall)			&&
								(_wallMap[(y - 1) * _fullWidth + x] & kWall)		&&
								(_wallMap[(y + 1) * _fullWidth + x] & kWall)		&&
								(_wallMap[(y - 1) * _fullWidth + x + 1] & kWall) 	&&
								(_wallMap[(y + 1) * _fullWidth + x + 1] & kWall)	&&
								!_wallMap[y * _fullWidth + x - 1] ) {

							foundWallLocation = true;
						}
					} else {
						foundWallLocation = true;
					}
				}
			}
		}

		// No more suitable locations, exit
		if (!foundWallLocation) {
			break;
		}

		// Found a suitable location, pick random object wall
		uint r = g_nancy->_randomSource->getRandomNumber(theme.objectwallIDs.size() - 1);
		uint32 selectedObje
ctWalls = theme.objectwallIDs[r];
		uint16 selectedWallsHeight = theme.objectWallHeights[r];

		byte lowWall, midWall, highWall;
		lowWall = selectedObjectWalls & 0xFF;
		midWall = (selectedObjectWalls >> 8) & 0xFF;
		highWall = (selectedObjectWalls >> 16) & 0xFF;

		if (textureVerticalHeight - 1 <= selectedWallsHeight) {
			selectedWallsHeight = textureVerticalHeight - 1;
		}

		selectedObjectWalls &= 0xFFFFFF;

		if (lowWall == 0 || (midWall == 0 && selectedWallsHeight > 128) || (highWall == 0 && selectedWallsHeight > 256 )) {
			selectedObjectWalls |= kHasBlankWalls;
		}

		if (vertical) {
			selectedObjectWalls |= 0x22000000;
		} else {
			selectedObjectWalls |= 0x24000000;
		}

		// Place the selected object wall on the map
		_wallMap[y * _fullWidth + x] = selectedObjectWalls;
		_heightMap[y * _fullWidth + x] = selectedWallsHeight;

		// Subtract 2 from all lightmap values when an object wall is added
		uint16 lightmapValue = _wallLightMap[y * _fullWidth + x];

		lowWall = lightmapValue & 0xF;
		midWall = (lightmapValue >> 4) & 0xF;
		highWall = (lightmapValue >> 8) & 0xF;

		lowWall = (int)lowWall - 2 < 0 ? 0 : lowWall - 2;
		midWall = (int)midWall - 2 < 0 ? 0 : midWall - 2;
		highWall = (int)highWall - 2 < 0 ? 0 : highWall - 2;

		_wallLightMap[y * _fullWidth + x] = (lightmapValue & 0xF000) | (highWall << 8) | (midWall << 4) | lowWall;

		lightmapValue = _floorCeilingLightMap[y * _fullWidth + x];

		byte floor, ceiling;
		floor = lightmapValue & 0xF;
		ceiling = (lightmapValue >> 4) & 0xF;

		floor = (int)floor - 2 < 0 ? 0 : floor - 2;
		ceiling = (int)ceiling - 2 < 0 ? 0 : ceiling - 2;

		_floorCeilingLightMap[y * _fullWidth + x] = (lightmapValue & 0xF000) | (ceiling << 4) | floor;

		++numWrittenWalls;
	}
}

void RaycastLevelBuilder::writeDoors(uint startX, uint startY, uint themeID) {
	const RCLB::Theme &theme = _themeData->themes[themeID];
	uint numDoorsToWrite = (int)((float)theme.doorDensity * _objectsBaseDensity);

	for (uint numWrittenWalls = 0; numWr
ittenWalls < numDoorsToWrite;) {
		bool vertical = g_nancy->_randomSource->getRandomBit();
		bool foundDoorLocation = false;

		uint x = 0;
		uint y = 0;
		uint otherX = 0;
		uint otherY = 0;

		for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundDoorLocation; ++checkedCells) {
			x = g_nancy->_randomSource->getRandomNumberRng(startX, MIN(startX + _halfWidth, _fullWidth - 1));
			y = g_nancy->_randomSource->getRandomNumberRng(startY, MIN(startY + _halfHeight, _fullHeight - 1));

			if (!_wallMap[y * _fullWidth + x]) {
				if (vertical) {
					if (	!(_wallMap[y * _fullWidth + x - 1])				&&
							(_wallMap[y * _fullWidth + x + 1] & kWall)	&&
							(_wallMap[y * _fullWidth + x - 2] & kWall)	&&
							!(_wallMap[(y - 1) * _fullWidth + x])			&&
							!(_wallMap[(y - 2) * _fullWidth + x])			&&
							!(_wallMap[(y + 1) * _fullWidth + x])			&&
							!(_wallMap[(y + 2) * _fullWidth + x])			&&
							!(_wallMap[(y - 1) * _fullWidth + x - 1])		&&
							!(_wallMap[(y - 2) * _fullWidth + x - 1])		&&
							!(_wallMap[(y + 1) * _fullWidth + x - 1])		&&
							!(_wallMap[(y + 2) * _fullWidth + x - 1]) ) {

						otherX = x - 1;
						otherY = y;

						if (	!(_infoMap[y * _fullWidth + x])			&&
								!(_infoMap[y * _fullWidth + otherX])	&&
								(y != _startY || x != _startX)			&&
								(y != _startY || otherX != _startX) ) {

							foundDoorLocation = true;
						}
					} else if (	!(_wallMap[y * _fullWidth + x + 1])					&&
								(_wallMap[y * _fullWidth + x - 1] & kWall)		&&
								(_wallMap[y * _fullWidth + x + 2] & kWall)		&&
								!(_wallMap[(y - 1) * _fullWidth + x])				&&
								!(_wallMap[(y - 2) * _fullWidth + x])				&&
								!(_wallMap[(y + 1) * _fullWidth + x])				&&
								!(_wallMap[(y + 2) * _fullWidth + x])				&&
								!(_wallMap[(y - 1) * _fullWidth + x + 1])			&&
								!(_wallMap[(y - 2) * _fullWidth + x + 1])			&&
								!(_wallMap[(y + 1) * _fullWidth + x + 1])			&&
								!(_wallMap[(y + 2) * _fullWidth + x + 1]) ) {

		
				otherX = x + 1;
						otherY = y;

						if (	!(_infoMap[y * _fullWidth + x])			&&
								!(_infoMap[y * _fullWidth + otherX])	&&
								(y != _startY || x != _startX)			&&
								(y != _startY || otherX != _startX) ) {

							foundDoorLocation = true;
						}
					}
				} else {
					if (	!(_wallMap[(y - 1) * _fullWidth + x])				&&
							(_wallMap[(y + 1) * _fullWidth + x] & kWall)	&&
							(_wallMap[(y - 2) * _fullWidth + x] & kWall)	&&
							!(_wallMap[y * _fullWidth + x - 1])					&&
							!(_wallMap[y * _fullWidth + x - 2])					&&
							!(_wallMap[y * _fullWidth + x + 1])					&&
							!(_wallMap[y * _fullWidth + x + 2])					&&
							!(_wallMap[(y - 1) * _fullWidth + x - 1])			&&
							!(_wallMap[(y - 1) * _fullWidth + x - 2])			&&
							!(_wallMap[(y - 1) * _fullWidth + x + 1])			&&
							!(_wallMap[(y - 1) * _fullWidth + x + 2]) ) {

						otherY = y - 1;
						otherX = x;

						if (	!(_infoMap[y * _fullWidth + x])			&&
								!(_infoMap[otherY * _fullWidth + x])	&&
								(y != _startY || x != _startX)			&&
								(otherY != _startY || x != _startX) ) {

							foundDoorLocation = true;
						}
					} else if (	!(_wallMap[(y + 1) * _fullWidth + x])				&&
								(_wallMap[(y - 1) * _fullWidth + x] & kWall)	&&
								(_wallMap[(y + 2) * _fullWidth + x] & kWall)	&&
								!(_wallMap[y * _fullWidth + x - 1])					&&
								!(_wallMap[y * _fullWidth + x - 2])					&&
								!(_wallMap[y * _fullWidth + x + 1])					&&
								!(_wallMap[y * _fullWidth + x + 2])					&&
								!(_wallMap[(y + 1) * _fullWidth + x - 1])			&&
								!(_wallMap[(y + 1) * _fullWidth + x - 2])			&&
								!(_wallMap[(y + 1) * _fullWidth + x + 1])			&&
								!(_wallMap[(y + 1) * _fullWidth + x + 2]) ) {

						otherY = y + 1;
						otherX = x;

						if (	!(_infoMap[y * _fullWidth + x])			&&
								!(_infoMap[otherY * _fullWidth + x])	&&
								(y != _startY || x != _startX)			&&
								(otherY != _startY || x != _startX) ) {

							foundDoorLocation = true;

						}
					}
				}
			}
		}

		// No more suitable locations, exit
		if (!foundDoorLocation) {
			break;
		}

		uint32 selectedDoorIDs = theme.doorIDs[g_nancy->_randomSource->getRandomNumber(theme.doorIDs.size() - 1)];
		bool leftOrTop = g_nancy->_randomSource->getRandomBit();

		uint32 doorX, doorY;
		uint lightmapValue;

		if (vertical) {
			selectedDoorIDs |= (kDoor | kVertical | kHasBlankWalls | kTransparentWall);
			doorY = y;

			if (leftOrTop) {
				if (x < otherX) {
					doorX = x;
				} else {
					doorX = otherX;
				}

				_wallMap[y * _fullWidth + doorX] = selectedDoorIDs;
				_wallMap[y * _fullWidth + doorX + 1] = _wallMap[y * _fullWidth + doorX + 2];
				lightmapValue = _wallLightMap[y * _fullWidth + doorX - 1];
				_wallLightMap[y * _fullWidth + doorX] = lightmapValue;
				_wallLightMap[y * _fullWidth + doorX + 1] = _wallLightMap[y * _fullWidth + doorX + 2];
			} else {
				if (x > otherX) {
					doorX = x;
				} else {
					doorX = otherX;
				}

				_wallMap[y * _fullWidth + doorX] = selectedDoorIDs;
				_wallMap[y * _fullWidth + doorX - 1] = _wallMap[y * _fullWidth + doorX - 2];
				lightmapValue = _wallLightMap[y * _fullWidth + doorX + 1];
				_wallLightMap[y * _fullWidth + doorX] = lightmapValue;
				_wallLightMap[y * _fullWidth + doorX - 1] = _wallLightMap[y * _fullWidth + doorX - 2];
			}
		} else {
			selectedDoorIDs |= (kDoor | kHorizontal | kHasBlankWalls | kTransparentWall);
			doorX = x;

			if (leftOrTop) {
				if (y > otherY) {
					doorY = y;
				} else {
					doorY = otherY;
				}

				_wallMap[doorY * _fullWidth + x] = selectedDoorIDs;
				_wallMap[(doorY - 1) * _fullWidth + x] = _wallMap[(doorY - 2) * _fullWidth + x];
				lightmapValue = (uint)_wallLightMap[(doorY + 1) * _fullWidth + x];
				_wallLightMap[doorY * _fullWidth + x] = _wallLightMap[(doorY + 1) * _fullWidth + x];
				_wallLightMap[(doorY - 1) * _fullWidth + x] = _wallLightMap[(doorY - 2) * _fullWidth + x];
			} else {
				if (y < otherY) {
					doorY = y;
				} else {
	
				doorY = otherY;
				}

				_wallMap[doorY * _fullWidth + x] = selectedDoorIDs;
				_wallMap[(doorY + 1) * _fullWidth + x] = _wallMap[(doorY + 2) * _fullWidth + x];
				lightmapValue = _wallLightMap[(doorY - 1) * _fullWidth + x];
				_wallLightMap[doorY * _fullWidth + x] = _wallLightMap[(doorY - 1) * _fullWidth + x];
				_wallLightMap[(doorY + 1) * _fullWidth + x] = _wallLightMap[(doorY + 2) * _fullWidth + x];
			}
		}

		// Subtract 2 from all lightmap values when a door is added
		// This looks extremely ugly but the original devs must've added it for a reason
		byte lowWall, midWall, highWall;
		lowWall = lightmapValue & 0xF;
		midWall = (lightmapValue >> 4) & 0xF;
		highWall = (lightmapValue >> 8) & 0xF;

		lowWall = (int)lowWall - 2 < 0 ? 0 : lowWall - 2;
		midWall = (int)midWall - 2 < 0 ? 0 : midWall - 2;
		highWall = (int)highWall - 2 < 0 ? 0 : highWall - 2;

		_wallLightMap[y * _fullWidth + x] = (lightmapValue & 0xF000) | (highWall << 8) | (midWall << 4) | lowWall;

		lightmapValue = _floorCeilingLightMap[doorY * _fullWidth + doorX];

		byte floor, ceiling;
		floor = lightmapValue & 0xF;
		ceiling = (lightmapValue >> 4) & 0xF;

		floor = (int)floor - 2 < 0 ? 0 : floor - 2;
		ceiling = (int)ceiling - 2 < 0 ? 0 : ceiling - 2;

		_floorCeilingLightMap[doorY * _fullWidth + doorX] = (lightmapValue & 0xF000) | (ceiling << 4) | floor;
	}
}

void RaycastLevelBuilder::writeLightSwitch(uint startX, uint startY, uint switchID) {
	bool foundSwitchLocation = false;

	for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundSwitchLocation; ++checkedCells) {
		uint x = g_nancy->_randomSource->getRandomNumberRng(startX, MIN(startX + _halfWidth, _fullWidth - 1));
		uint y = g_nancy->_randomSource->getRandomNumberRng(startY, MIN(startY + _halfHeight, _fullHeight - 1));

		if (!(_wallMap[y * _fullWidth + x]) && !(_infoMap[y * _fullWidth + x]) && (y != _startY || x != _startX)) {
			foundSwitchLocation = true;
		}

		if (foundSwitchLocation) {
			_infoMap[y * _fullW
idth + x] = (switchID << 8) | 2;

			uint lightmapValue = _floorCeilingLightMap[y * _fullWidth + x];

			// Ceiling remains unchanged
			byte floor, ceiling;
			floor = lightmapValue & 0xF;
			ceiling = (lightmapValue >> 4) & 0xF;
			floor = (int)floor - 2 < 0 ? 0 : floor - 2;

			_floorCeilingLightMap[y * _fullWidth + x] = (lightmapValue & 0xF000) | (ceiling << 4) | floor;
			_floorMap[y * _fullWidth + x] = _themeData->lightSwitchID;
		}
	}
}

void RaycastLevelBuilder::writeExitFloorTexture(uint themeID) {
	const RCLB::Theme &theme = _themeData->themes[themeID];
	int16 selectedFloorID = theme.exitFloorIDs[g_nancy->_randomSource->getRandomNumber(theme.exitFloorIDs.size() - 1)];
	uint addToID = 0;

	for (uint y = 0; y < _fullHeight; ++y) {
		for (uint x = 0; x < _fullWidth; ++x) {
			if (_infoMap[y * _fullWidth + x] == 1) {
				uint lightmapValue = _floorCeilingLightMap[y * _fullWidth + x];

				// Ceiling remains unchanged
				byte floor, ceiling;
				floor = lightmapValue & 0xF;
				ceiling = (lightmapValue >> 4) & 0xF;

				floor = (int)floor - 2 < 0 ? 0 : floor - 2;

				_floorCeilingLightMap[y * _fullWidth + x] = (lightmapValue & 0xF000) | (ceiling << 4) | floor;
				_floorMap[y * _fullWidth + x] = selectedFloorID + addToID;

				++addToID;

				if (addToID == 4) {
					return;
				}
			}
		}
	}
}

void RaycastPuzzle::validateMap() {
	for (uint y = 0; y < _mapFullHeight; ++y) {
		for (uint x = 0; x < _mapFullWidth; ++x) {
			if (_wallMap[y * _mapFullWidth + x] == 1) {
				error("wallMap not complete at coordinates x = %d, y = %d", x, y);
			}

			if (_floorMap[y * _mapFullWidth + x] == -1) {
				error("floorMap not complete at coordinates x = %d, y = %d", x, y);
			}

			if (_ceilingMap[y * _mapFullWidth + x] == -1) {
				error("wallMap not complete at coordinates x = %d, y = %d", x, y);
			}

			// Find light switches
			if ((_infoMap[y * _mapFullWidth + x] & 0xFF) == 2) {
				_lightSwitchIDs.push_back((_infoMap[y * _mapFullWidth + x] >> 8) & 0xFF);
				_lightS
witchPositions.push_back(Common::Point(x, y));
				_lightSwitchStates.push_back(false);
			}
		}
	}
}

class RaycastDeferredLoader : public DeferredLoader {
public:
	RaycastDeferredLoader(RaycastPuzzle &owner, uint width, uint height, uint verticalHeight) :
		_owner(owner),
		_builder(width, height, verticalHeight),
		_loadState(kInitDrawSurface),
		_x(0), _y(0),
		_isDone(false) {}
	virtual ~RaycastDeferredLoader() {}

	bool _isDone;

private:
	bool loadInner() override;

	enum State { kInitDrawSurface, kInitPlayerLocationRotation, kCopyData, kInitMap, kInitTables1, kInitTables2, kLoadTextures };

	State _loadState;

	RaycastPuzzle &_owner;
	RaycastLevelBuilder _builder;

	uint16 _x, _y;
};

bool RaycastDeferredLoader::loadInner() {
	switch(_loadState) {
	case kInitDrawSurface : {
		auto *viewportData = GetEngineData(VIEW);
		assert(viewportData);

		Common::Rect viewport = viewportData->bounds;
		_owner.moveTo(viewport);
		_owner._drawSurface.create(viewport.width(), viewport.height(), g_nancy->_graphics->getInputPixelFormat());
		_owner.setTransparent(true);

		_loadState = kInitPlayerLocationRotation;
		break;
	}
	case kInitPlayerLocationRotation :
		if (	_builder._wallMap[_builder._startY * _builder._fullWidth + _builder._startX + 1] == 0 &&
					_builder._wallMap[_builder._startY * _builder._fullWidth + _builder._startX + 2] == 0) {
			_owner._playerRotation = 0;
		} else if (	_builder._wallMap[(_builder._startY - 1) * _builder._fullWidth + _builder._startX] == 0 &&
					_builder._wallMap[(_builder._startY - 2) * _builder._fullWidth + _builder._startX] == 0) {
			_owner._playerRotation = 1024;
		} else if (	_builder._wallMap[_builder._startY * _builder._fullWidth + _builder._startX - 1] == 0 &&
					_builder._wallMap[_builder._startY * _builder._fullWidth + _builder._startX - 2] == 0) {
			_owner._playerRotation = 2048;
		} else if (	_builder._wallMap[(_builder._startY + 1) * _builder._fullWidth + _builder._startX] == 0 &&
					_builder._wallMap[(_builder._sta
rtY + 2) * _builder._fullWidth + _builder._startX] == 0) {
			_owner._playerRotation = 3072;
		} else {
			_owner._playerRotation = 512;
		}

		_owner._playerX = _builder._startX * 128 + 64;
		_owner._playerY = _builder._startY * 128 + 64;

		_loadState = kCopyData;
		break;
	case kCopyData :
		_owner._wallMap.swap(_builder._wallMap);
		_owner._infoMap.swap(_builder._infoMap);
		_owner._floorMap.swap(_builder._floorMap);
		_owner._ceilingMap.swap(_builder._ceilingMap);
		_owner._heightMap.swap(_builder._heightMap);
		_owner._wallLightMap.swap(_builder._wallLightMap);
		_owner._floorCeilingLightMap.swap(_builder._floorCeilingLightMap);
		_owner._wallLightMapBackup = _owner._wallLightMap;
		_owner._floorCeilingLightMapBackup = _owner._floorCeilingLightMap;
		_owner._mapFullWidth = _builder._fullWidth;
		_owner._mapFullHeight = _builder._fullHeight;

		_loadState = kInitMap;
		break;
	case kInitMap : {
		_owner.drawMap();
		_owner._map.setVisible(false);

		_loadState = kInitTables1;
		break;
	}
	case kInitTables1 : {
		Common::Rect selectedBounds = _owner._puzzleData->screenViewportSizes[_owner._puzzleData->viewportSizeUsed];
		auto *viewportData = GetEngineData(VIEW);
		assert(viewportData);

		_owner._wallCastColumnAngles.resize(viewportData->screenPosition.width());
		uint center = selectedBounds.left + (selectedBounds.width() >> 1);
		for (uint i = 0; i < _owner._wallCastColumnAngles.size(); ++i) {
			int32 &angle = _owner._wallCastColumnAngles[i];
			angle = (int32)(atan(((float)i - (float)center) / (float)_owner._fov) * _owner._rotationSingleStep);
			clampRotation(angle);
		}

		_owner._leftmostAngle = _owner._wallCastColumnAngles[selectedBounds.left];
		_owner._rightmostAngle = _owner._wallCastColumnAngles[selectedBounds.right];

		_loadState = kInitTables2;
		break;
	}
	case kInitTables2 : {
		auto *viewportData = GetEngineData(VIEW);
		assert(viewportData);

		_owner._sinTable.resize(4096);
		_owner._cosTable.resize(4096);

		for (uint i = 0; i < 4096; ++i) 
{
			double f = (i * _owner._pi * 2) / 4096;
			_owner._cosTable[i] = cos(f);
			_owner._sinTable[i] = sin(f);
		}

		_owner._maxWorldDistance = sqrt(((128 * _owner._mapFullWidth) - 1) * ((128 * _owner._mapFullHeight) - 1) * 2);
		_owner._depthBuffer.resize(viewportData->bounds.width());
		_owner._zBuffer.resize(viewportData->bounds.width() * viewportData->bounds.height(), 0);
		_owner._lastZDepth = 0;

		_loadState = kLoadTextures;
		break;
	}
	case kLoadTextures: {
		bool shouldBreak = false;

		for (; _y < _owner._mapFullHeight; ++_y) {
			if (_x >= _owner._mapFullWidth) {
				_x = 0;
			}

			for (; _x < _owner._mapFullWidth && !shouldBreak; ++_x) {
				uint32 wallMapVal = _owner._wallMap[_y * _owner._mapFullHeight + _x];

				for (uint i

... [Content truncated]