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

#include "engines/nancy/action/raycastpuzzle.h"
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
	rot = rot < 0 ? rot + 4096 : rot;
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
	void validateMap();

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

	RCLB *_themeData;
};

RaycastLevelBuilder::RaycastLevelBuilder(uint width, uint height, uint verticalHeight) {
	_themeData = g_nancy->_raycastPuzzleLevelBuilderData;

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

	fillCells();
	fillWalls();
	fillLocalWallAndInfo();
	writeThemesAndExitFloor();
	validateMap();
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
		if (y == 0 || y == _halfHeight - 1) {
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
	RCLB::Theme &theme = _themeData->themes[themeID];

	uint themeHalfWidth, themeHalfHeight;

	themeHalfWidth = _inputWidth + 1;
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
	RCLB::Theme &theme = _themeData->themes[themeID];
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
								!(_wallMap[y * _fullWidth + x - 1]) ) {

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
	RCLB::Theme &theme = _themeData->themes[themeID];
	uint numWallsToWrite = (int)((float)theme.objectWallDensity * _objectsBaseDensity);

	uint textureVerticalHeight = _verticalHeight * 128; // 128 is a constant inside RayCast

	for (uint numWrittenWalls = 0; numWrittenWalls < numWallsToWrite;) {
		bool vertical = g_nancy->_randomSource->getRandomBit();
		bool foundWallLocation = false;

		uint x = 0;
		uint y = 0;

		for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundWallLocation; ++checkedCells) {
			x = g_nancy->_randomSource->getRandomNumberRng(MAX(startX, 1U), MIN(startX + _halfWidth, _fullWidth - 2));
			y = g_nancy->_randomSource->getRandomNumberRng(MAX(startY, 1U), MIN(startY + _halfHeight, _fullHeight - 2));

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
		uint32 selectedObjectWalls = theme.objectwallIDs[r];
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
	RCLB::Theme &theme = _themeData->themes[themeID];
	uint numDoorsToWrite = (int)((float)theme.doorDensity * _objectsBaseDensity);

	for (uint numWrittenWalls = 0; numWrittenWalls < numDoorsToWrite;) {
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
								!(_wallMap[y * _fullWidth + x + 1])					&&
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

void RaycastLevelBuilder::writeLightSwitch(uint startX, uint startY, uint quadrant) {
	bool foundSwitchLocation = false;

	for (uint checkedCells = 0; checkedCells < _fullNumCells && !foundSwitchLocation; ++checkedCells) {
		uint x = g_nancy->_randomSource->getRandomNumberRng(startX, MIN(startX + _halfWidth, _fullWidth - 1));
		uint y = g_nancy->_randomSource->getRandomNumberRng(startY, MIN(startY + _halfHeight, _fullHeight - 1));

		if (!(_wallMap[y * _fullWidth + x]) && !(_infoMap[y * _fullWidth + x]) && (y != _startY || x != _startX)) {
			foundSwitchLocation = true;
		}

		if (foundSwitchLocation) {
			_infoMap[y * _fullWidth + x] = (quadrant << 8) | 2;

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
	RCLB::Theme &theme = _themeData->themes[themeID];
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

void RaycastLevelBuilder::validateMap() {
	for (uint y = 0; y < _fullHeight; ++y) {
		for (uint x = 0; x < _fullWidth; ++x) {
			if (_wallMap[y * _fullWidth + x] == 1) {
				error("wallMap not complete at coordinates x = %d, y = %d", x, y);
			}

			if (_floorMap[y * _fullWidth + x] == -1) {
				error("floorMap not complete at coordinates x = %d, y = %d", x, y);
			}

			if (_ceilingMap[y * _fullWidth + x] == -1) {
				error("wallMap not complete at coordinates x = %d, y = %d", x, y);
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

	enum State { kInitDrawSurface, kCopyData, kInitMap, kInitTables1, kInitTables2, kLoadTextures };

	State _loadState;

	RaycastPuzzle &_owner;
	RaycastLevelBuilder _builder;

	uint16 _x, _y;
};

bool RaycastDeferredLoader::loadInner() {
	switch(_loadState) {
	case kInitDrawSurface : {
		Common::Rect viewport = g_nancy->_viewportData->bounds;
		_owner.moveTo(viewport);
		_owner._drawSurface.create(viewport.width(), viewport.height(), g_nancy->_graphicsManager->getInputPixelFormat());
		_owner.setTransparent(true);

		_loadState = kCopyData;
		break;
	}
	case kCopyData :
		_owner._wallMap.swap(_builder._wallMap);
		_owner._infoMap.swap(_builder._infoMap);
		_owner._floorMap.swap(_builder._floorMap);
		_owner._ceilingMap.swap(_builder._ceilingMap);
		_owner._heightMap.swap(_builder._heightMap);
		_owner._wallLightMap.swap(_builder._wallLightMap);
		_owner._floorCeilingLightMap.swap(_builder._floorCeilingLightMap);
		_owner._mapFullWidth = _builder._fullWidth;
		_owner._mapFullHeight = _builder._fullHeight;

		_loadState = kInitMap;
		break;
	case kInitMap : {
		// TODO map is a debug feature, make sure to hide it
		// Also, fix the fact that it's rendered upside-down
		_owner._map._drawSurface.create(_owner._mapFullWidth, _owner._mapFullHeight, g_nancy->_graphicsManager->getInputPixelFormat());
		Common::Rect mapPos(g_nancy->_bootSummary->textboxScreenPosition);
		mapPos.setWidth(_owner._mapFullWidth * 2);
		mapPos.setHeight(_owner._mapFullHeight * 2);
		_owner._map.moveTo(mapPos);
		_owner._map.init();
		_owner.drawMap();

		_loadState = kInitTables1;
		break;
	}
	case kInitTables1 : {
		Common::Rect selectedBounds = _owner._puzzleData->screenViewportSizes[_owner._puzzleData->viewportSizeUsed];

		_owner._wallCastColumnAngles.resize(g_nancy->_viewportData->screenPosition.width());
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
		_owner._sinTable.resize(4096);
		_owner._cosTable.resize(4096);

		for (uint i = 0; i < 4096; ++i) {
			double f = (i * _owner._pi * 2) / 4096;
			_owner._cosTable[i] = cos(f);
			_owner._sinTable[i] = sin(f);
		}

		_owner._maxWorldDistance = sqrt(((128 * _owner._mapFullWidth) - 1) * ((128 * _owner._mapFullHeight) - 1) * 2);
		_owner._depthBuffer.resize(g_nancy->_viewportData->bounds.width());
		_owner._zBuffer.resize(g_nancy->_viewportData->bounds.width() * g_nancy->_viewportData->bounds.height(), 0);
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

				for (uint i = 0; i < 3; ++i) {
					byte textureID = wallMapVal & 0xFF;

					if (textureID & 0x80) {
						if (!_owner._specialWallTextures.contains(textureID & 0x7F)) {
							_owner.createTextureLightSourcing(&_owner._specialWallTextures[textureID & 0x7F], _owner._puzzleData->specialWallNames[(textureID & 0x7F) - 1]);
							shouldBreak = true;
						}
					} else if (textureID) {
						if (!_owner._wallTextures.contains(textureID & 0x7F)) {
							_owner.createTextureLightSourcing(&_owner._wallTextures[textureID], _owner._puzzleData->wallNames[textureID - 1]);
							shouldBreak = true;
						}
					}

					wallMapVal >>= 8;
				}

				if (shouldBreak) {
					break;
				}

				int16 floorMapVal = _owner._floorMap[_y * _owner._mapFullWidth + _x];
				int16 ceilingMapVal = _owner._ceilingMap[_y * _owner._mapFullWidth + _x];

				if (!_owner._floorTextures.contains(floorMapVal)) {
					_owner.createTextureLightSourcing(&_owner._floorTextures[floorMapVal], _owner._puzzleData->floorNames[floorMapVal]);
					shouldBreak = true;
					break;
				}

				if (!_owner._ceilingTextures.contains(ceilingMapVal)) {
					_owner.createTextureLightSourcing(&_owner._ceilingTextures[ceilingMapVal], _owner._puzzleData->ceilingNames[ceilingMapVal]);
					shouldBreak = true;
					break;
				}
			}

			if (shouldBreak) {
				break;
			}
		}

		if (!shouldBreak) {
			for (auto &a : _owner._specialWallTextures) {
				for (auto &tex : a._value) {
					tex.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
				}
			}

			// TODO these need to be set according to the start position in _infoMap
			_owner._playerRotation = 2048;
			_owner._playerX = _owner._playerY = 320;
			
			_isDone = true;
		}

		break;
	}
	}

	return _isDone;
}

void RaycastPuzzle::init() {
	_puzzleData = g_nancy->_raycastPuzzleData;
	assert(_puzzleData);

	RaycastDeferredLoader *loader = _loaderPtr.get();
	if (!loader) {
		_loaderPtr.reset(new RaycastDeferredLoader(*this, _mapWidth, _mapHeight, _wallHeight));
		auto castedPtr = _loaderPtr.dynamicCast<DeferredLoader>();
		g_nancy->addDeferredLoader(castedPtr);
	} else {
		if (loader->_isDone) {
			_loaderPtr.reset();
			registerGraphics();

			_state = kRun;
		}
	}
}

void RaycastPuzzle::registerGraphics() {
	_map.registerGraphics();
	RenderActionRecord::registerGraphics();
}

void RaycastPuzzle::readData(Common::SeekableReadStream &stream) {
	_mapWidth = stream.readUint16LE();
	_mapHeight = stream.readUint16LE();
	_wallHeight = stream.readByte();

	readFilename(stream, _switchSoundName);
	_switchSoundChannelID = stream.readUint16LE();
	readFilename(stream, _unknownSoundName);
	_unknownSoundChannelID = stream.readUint16LE();

	_dummySound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);
}

void RaycastPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		break;
	case kRun:
		// TODO check light switches
		// TODO check exit

		break;
	case kActionTrigger:
		break;
	}
}

void RaycastPuzzle::handleInput(NancyInput &input) {
	uint32 time = g_nancy->getTotalPlayTime();
	uint32 deltaTime = time - _lastMovementTime;
	_lastMovementTime = time;
	deltaTime *= 1000;
	bool mouseIsInBounds = false;

	float deltaRotation = ((float)deltaTime * 0.0006);
	float deltaPosition = ((float)deltaTime * 0.0002);

	if (NancySceneState.getViewport().convertViewportToScreen(_puzzleData->screenViewportSizes[_puzzleData->viewportSizeUsed]).contains(input.mousePos)) {
		mouseIsInBounds = true;
	}

	// Mouse was just clicked, make sure we don't rotate
	if (mouseIsInBounds && ((input.input & NancyInput::kLeftMouseButtonDown) || (input.input & NancyInput::kRightMouseButtonDown))) {
		_lastMouseX = input.mousePos.x;
	}

	// Rotate the camera according to the left-right movement of the mouse
	if (mouseIsInBounds && ((input.input & NancyInput::kLeftMouseButtonHeld) || (input.input & NancyInput::kRightMouseButtonHeld))) {
		if (input.mousePos.x < _lastMouseX) {
			_playerRotation -= (_lastMouseX - input.mousePos.x) * 5;
		}

		if (input.mousePos.x > _lastMouseX) {
			_playerRotation -= (_lastMouseX - input.mousePos.x) * 5;
		}

		_lastMouseX = input.mousePos.x;
	}

	// Rotate the camera if the arrow keys were pressed
	if (input.input & NancyInput::kMoveLeft) {
		if (input.input & NancyInput::kMoveFastModifier) {
			_playerRotation -= deltaRotation * 2;
		} else {
			_playerRotation -= deltaRotation;
		}
	}

	if (input.input & NancyInput::kMoveRight) {
		if (input.input & NancyInput::kMoveFastModifier) {
			_playerRotation += deltaRotation * 2;
		} else {
			_playerRotation += deltaRotation;
		}
	}

	clampRotation(_playerRotation);

	int32 newX = _playerX;
	int32 newY = _playerY;
	bool hasMoved = false;
	bool hasMovedSlowdown = false;

	// Move forward/backwards

	// Invert the rotation to avoid some annoying precision errors
	int32 invertedRotation = 4095 - (_playerRotation + 2048);
	clampRotation(invertedRotation);

	if (input.input & NancyInput::kMoveUp || (input.input & NancyInput::kLeftMouseButtonHeld && mouseIsInBounds)) {
		if (input.input & NancyInput::kMoveFastModifier) {
			newX = _playerX - (int32)(_sinTable[invertedRotation] * deltaPosition) * 2;
			newY = _playerY - (int32)(_cosTable[invertedRotation] * deltaPosition) * 2;
		} else {
			newX = _playerX - (int32)(_sinTable[invertedRotation] * deltaPosition);
			newY = _playerY - (int32)(_cosTable[invertedRotation] * deltaPosition);
		}

		hasMoved = true;
	}

	if (input.input & NancyInput::kMoveDown || (input.input & NancyInput::kRightMouseButtonHeld && mouseIsInBounds)) {
		if (input.input & NancyInput::kMoveFastModifier) {
			newX = _playerX + (int32)(_sinTable[invertedRotation] * deltaPosition) * 2;
			newY = _playerY + (int32)(_cosTable[invertedRotation] * deltaPosition) * 2;
		} else {
			newX = _playerX + (int32)(_sinTable[invertedRotation] * deltaPosition);
			newY = _playerY + (int32)(_cosTable[invertedRotation] * deltaPosition);
		}

		hasMoved = true;
	}

	// Perform slowdown
	if (!hasMoved && _nextSlowdownMovementTime < time && _slowdownFramesLeft > 0) {
		_slowdownDeltaX = (float)_slowdownDeltaX * 9.0 / 10.0;
		_slowdownDeltaY = (float)_slowdownDeltaY * 9.0 / 10.0;
		newX += _slowdownDeltaX;
		newY += _slowdownDeltaY;
		hasMoved = true;
		hasMovedSlowdown = true;
		_nextSlowdownMovementTime = time + 40;
		--_slowdownFramesLeft;
	}

	// Perform collision
	if (hasMoved) {
		uint yGrid = newX >> 7;
		uint xGrid = newY >> 7;

		// Check neighboring cells
		uint32 cellLeft = xGrid > 0 ? _wallMap[yGrid * _mapFullWidth + xGrid - 1] : 1;
		uint32 cellTop = yGrid > 0 ? _wallMap[(yGrid - 1) * _mapFullWidth + xGrid] : 1;
		uint32 cellRight = xGrid < _mapFullWidth ? _wallMap[yGrid * _mapFullWidth + xGrid + 1] : 1;
		uint32 cellBottom = yGrid < _mapFullHeight ? _wallMap[(yGrid + 1) * _mapFullWidth + xGrid] : 1;

		// Allow passage through doors
		cellLeft = (cellLeft & kDoor) ? 0 : cellLeft;
		cellTop = (cellTop & kDoor) ? 0 : cellTop;
		cellRight = (cellRight & kDoor) ? 0 : cellRight;
		cellBottom = (cellBottom & kDoor) ? 0 : cellBottom;

		int collisionSize = 48;

		int32 xCell = newX & 0x7F;
		int32 yCell = newY & 0x7F;

		if (cellLeft && yCell < collisionSize) {
			newY = (newY & 0xFF80) + collisionSize;
		}

		if (cellTop && xCell < collisionSize) {
			newX = (newX & 0xFF80) + collisionSize;
		}

		if (cellRight && yCell > (128 - collisionSize)) {
			newY = (newY & 0xFF80) + (128 - collisionSize);
		}

		if (cellBottom && xCell > (128 - collisionSize)) {
			newX = (newX & 0xFF80) + (128 - collisionSize);
		}

		yGrid = newX >> 7;
		xGrid = newY >> 7;
		yCell = newX & 0x7F;
		xCell = newY & 0x7F;

		cellLeft = xGrid > 0 ? _wallMap[yGrid * _mapFullWidth + xGrid - 1] : 1;
		cellTop = yGrid > 0 ? _wallMap[(yGrid - 1) * _mapFullWidth + xGrid] : 1;
		cellRight = xGrid < _mapFullWidth ? _wallMap[yGrid * _mapFullWidth + xGrid + 1] : 1;
		cellBottom = yGrid < _mapFullHeight ? _wallMap[(yGrid + 1) * _mapFullWidth + xGrid] : 1;

		uint32 cellTopLeft = (xGrid > 0 && yGrid > 0) ? _wallMap[(yGrid - 1) * _mapFullWidth + xGrid - 1] : 1;
		uint32 cellTopRight = (xGrid < _mapFullWidth && yGrid > 0) ? _wallMap[(yGrid - 1) * _mapFullWidth + xGrid + 1] : 1;
		uint32 cellBottomLeft = (xGrid > 0 && yGrid < _mapFullHeight) ? _wallMap[(yGrid + 1) * _mapFullWidth + xGrid - 1] : 1;
		uint32 cellBottomRight = (xGrid < _mapFullWidth && yGrid < _mapFullHeight) ? _wallMap[(yGrid + 1) * _mapFullWidth + xGrid + 1] : 1;

		cellLeft = (cellLeft & kDoor) ? 0 : cellLeft;
		cellTop = (cellTop & kDoor) ? 0 : cellTop;
		cellRight = (cellRight & kDoor) ? 0 : cellRight;
		cellBottom = (cellBottom & kDoor) ? 0 : cellBottom;

		cellTopLeft = (cellTopLeft & kDoor) ? 0 : cellTopLeft;
		cellTopRight = (cellTopRight & kDoor) ? 0 : cellTopRight;
		cellBottomLeft = (cellBottomLeft & kDoor) ? 0 : cellBottomLeft;
		cellBottomRight = (cellBottomRight & kDoor) ? 0 : cellBottomRight;

		collisionSize = 21;

		// Make sure the player doesn't clip diagonally into a wall
		// TODO this is still wonky
		if (cellTopLeft && !cellLeft && !cellTop && (yCell < collisionSize) && (xCell < collisionSize)) {
			return;
		}

		if (cellTopRight && !cellRight && !cellTop && (yCell < collisionSize) && (xCell > (128 - collisionSize))) {
			return;
		}

		if (cellBottomLeft && !cellLeft && !cellBottom && (yCell > (128 - collisionSize)) && (xCell < collisionSize)) {
			return;
		}

		if (cellBottomRight && !cellRight && !cellBottom && (yCell > (128 - collisionSize)) && (xCell > (128 - collisionSize))) {
			return;
		}

		if (!hasMovedSlowdown) {
			_slowdownDeltaX = newX - _playerX;
			_slowdownDeltaY = newY - _playerY;
			_nextSlowdownMovementTime = time + 40;
			_slowdownFramesLeft = 10;
		}

		_playerX = newX;
		_playerY = newY;

		debug("x = %u, y = %u", _playerX, _playerY);
	}
}

void RaycastPuzzle::updateGraphics() {
	if (_state == kRun) {
		drawMaze();
	}
}

void RaycastPuzzle::drawMap() {
	_map._drawSurface.clear();

	uint16 *pixelPtr;
	Graphics::PixelFormat &format = _map._drawSurface.format;

	uint16 wallColor = format.RGBToColor(_puzzleData->wallColor[0], _puzzleData->wallColor[1], _puzzleData->wallColor[2]);
	uint16 uColor6 = format.RGBToColor(_puzzleData->uColor6[0], _puzzleData->uColor6[1], _puzzleData->uColor6[2]);
	uint16 uColor7 = format.RGBToColor(_puzzleData->uColor7[0], _puzzleData->uColor7[1], _puzzleData->uColor7[2]);
	uint16 uColor8 = format.RGBToColor(_puzzleData->uColor8[0], _puzzleData->uColor8[1], _puzzleData->uColor8[2]);
	uint16 transparentWallColor = format.RGBToColor(_puzzleData->transparentWallColor[0], _puzzleData->transparentWallColor[1], _puzzleData->transparentWallColor[2]);
	uint16 lightSwitchColor = format.RGBToColor(_puzzleData->lightSwitchColor[0], _puzzleData->lightSwitchColor[1], _puzzleData->lightSwitchColor[2]);
	uint16 uColor10 = format.RGBToColor(_puzzleData->uColor10[0], _puzzleData->uColor10[1], _puzzleData->uColor10[2]);
	uint16 doorColor = format.RGBToColor(_puzzleData->doorColor[0], _puzzleData->doorColor[1], _puzzleData->doorColor[2]);
	uint16 exitColor = format.RGBToColor(_puzzleData->exitColor[0], _puzzleData->exitColor[1], _puzzleData->exitColor[2]);

	for (uint y = 0; y < _mapFullHeight; ++y) {
		pixelPtr = (uint16 *)_map._drawSurface.getBasePtr(0, y);
		for (uint x = 0; x < _mapFullWidth; ++x) {
			uint32 wallMapCell = _wallMap[y * _mapFullHeight + x];
			uint32 infoMapCell = _infoMap[y * _mapFullHeight + x];
			if (wallMapCell & kWall) {
				*pixelPtr = wallColor;
			}

			if ((wallMapCell != 0) && !(wallMapCell & kWall)) {
				*pixelPtr = uColor6;
			}

			if ((wallMapCell & kVertical) || (wallMapCell & kHorizontal)) {
				*pixelPtr = uColor7;
			}

			if (wallMapCell & kHasBlankWalls) {
				*pixelPtr = uColor8;
			}

			if (wallMapCell & kTransparentWall) {
				*pixelPtr = transparentWallColor;
			}

			if ((infoMapCell & 0xFF) == 2) {
				*pixelPtr = lightSwitchColor;
			}

			if ((infoMapCell & 0xFF) == 3) {
				*pixelPtr = uColor10;
			}

			if (wallMapCell & kDoor) {
				*pixelPtr = doorColor;
			}

			if ((infoMapCell & 0xFF) == 1) {
				*pixelPtr = exitColor;
			}

			++pixelPtr;
		}
	}

	_map.setVisible(true);
}

void RaycastPuzzle::loadTextures() {
	// TODO this is slow and freezes the engine for a few seconds
	
}

void RaycastPuzzle::createTextureLightSourcing(Common::Array<Graphics::ManagedSurface> *array, Common::String &textureName) {
	Graphics::PixelFormat format = g_nancy->_graphicsManager->getInputPixelFormat();
	array->resize(8);

	g_nancy->_resource->loadImage(textureName, (*array)[0]);

	uint width = (*array)[0].w;
	uint height = (*array)[0].h;

	// Keep the original texture as the first array element
	for (uint i = 1; i < 8; ++i) {
		(*array)[i].create(width, height, format);
	}

	// Make 7 copies, each one 1/8th darker than the last
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			uint offset = y * width + x;
			byte r, g, b, rStep, gStep, bStep;
			format.colorToRGB(((uint16 *)(*array)[0].getPixels())[offset], r, g, b);
			rStep = (float)r / 8.0 * 65536.0;
			gStep = (float)g / 8.0 * 65536.0;
			bStep = (float)b / 8.0 * 65536.0;
			for (uint i = 1; i < 8; ++i) {
				r -= rStep;
				g -= gStep;
				b -= bStep;
				((uint16 *)(*array)[i].getPixels())[offset] = format.RGBToColor(r, g, b);
			}
		}
	}
}

void RaycastPuzzle::drawMaze() {
	// TODO rendering needs further optimization
	Common::Rect viewBounds = _puzzleData->screenViewportSizes[_puzzleData->viewportSizeUsed];
	uint viewportCenterY = viewBounds.top + (viewBounds.height() / 2);
	uint16 transColor = (uint16)_drawSurface.getTransparentColor();
	float depth = 1.0;

	byte curZBufferDepth = _lastZDepth + 1;

	_drawSurface.clear(_drawSurface.getTransparentColor());

	// Draw walls
	for (int x = viewBounds.left; x <= viewBounds.right; ++x) {
		int32 columnAngleForX = _wallCastColumnAngles[x];
		int32 rotatedColumnAngleForX = columnAngleForX + _playerRotation;
		clampRotation(rotatedColumnAngleForX);

		float rayStartX = _playerX;
		float rayStartY = _playerY;

		float angleSin = (float)_sinTable[rotatedColumnAngleForX] * -1024.0;
		float angleCos = (float)_cosTable[rotatedColumnAngleForX] * 1024.0;

		if (angleSin == 0.0) {
			angleSin = 0.001f;
		}

		if (angleCos == 0.0) {
			angleCos = 0.001f;
		}

		float angleAtan = angleCos / angleSin;

		if (angleAtan == 0.0) {
			angleAtan = 0.001f;
		}

		bool isBehindTransparentWall = false;
		int viewBottom = viewBounds.bottom;

		for (bool shouldBreak = false; shouldBreak == false;) {
			int xEdge, yEdge;
			float xDist, yDist;

			if (angleSin <= 0.0) {
				xEdge = ((uint)rayStartX & 0xFF80) - 1;
			} else {
				xEdge = ((uint)rayStartX & 0xFF80) + 128;
			}

			if (angleCos <= 0.0) {
				yEdge = ((uint)rayStartY & 0xFF80) - 1;
			} else {
				yEdge = ((uint)rayStartY & 0xFF80) + 128;
			}

			float xRayX = xEdge;
			float xRayY = ((float)xEdge - rayStartX) * angleAtan + rayStartY;
			float yRayX = ((float)yEdge - rayStartY) / angleAtan + rayStartX;
			float yRayY = yEdge;

			byte quadrant = rotatedColumnAngleForX >> 9;

			if ((quadrant & 3) == 0 || (quadrant & 3) == 3) {
				xDist = abs(((xRayY - (float)_playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				yDist = abs(((yRayY - (float)_playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
			} else {
				xDist = abs(((xRayX - (float)_playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				yDist = abs(((yRayX - (float)_playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
			}

			int xGridTile = -1;
			int yGridTile = -1;
			Common::Point xGrid(-1, -1);
			Common::Point yGrid(-1, -1);

			if (xDist < _maxWorldDistance) {
				xGrid.y = (int)xRayX >> 7;
				xGrid.x = (int)(xRayY / 128.0);

				if (xGrid.x < _mapFullWidth && xGrid.y < _mapFullHeight && xGrid.x >= 0 && xGrid.y >= 0) {
					xGridTile = _wallMap[xGrid.y * _mapFullWidth + xGrid.x];
				}
			}

			if (yDist < _maxWorldDistance) {
				yGrid.y = (int)(yRayX / 128.0);
				yGrid.x = (int)yRayY >> 7;

				if (yGrid.x < _mapFullWidth && yGrid.y < _mapFullHeight && yGrid.x >= 0 && yGrid.y >= 0) {
					yGridTile = _wallMap[yGrid.y * _mapFullWidth + yGrid.x];
				}
			}

			if (xGridTile == -1 && yGridTile == -1) {
				break;
			}

			if (yGridTile > 0 && (yGridTile & kHorizontal) != 0) {
				if (angleCos <= 0.0) {
					yEdge = ((uint)yRayY & 0xFF80) - 1;
				} else {
					yEdge = ((uint)yRayY & 0xFF80) + 128;
				}

				yRayX += (((float)yEdge - yRayY) / angleAtan) / 2.0;
				yRayY += ((float)yEdge - yRayY) / 2.0;

				if ((quadrant & 3) == 0 || (quadrant & 3) == 3) {
					yDist = abs(((yRayY - (float)_playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				} else {
					yDist = abs(((yRayX - (float)_playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				}

				if (xGridTile == yGridTile) {
					if (angleSin <= 0.0) {
						xEdge = ((uint)xRayX & 0xFF80) - 1;
					} else {
						xEdge = ((uint)xRayX & 0xFF80) + 128;
					}

					xRayY += (((float)xEdge - xRayX) * angleAtan) / 2.0;
					xRayX += ((float)xEdge - xRayX) / 2.0;

					if ((quadrant & 3) == 0 || (quadrant & 3) == 3) {
						xDist = abs(((xRayY - (float)_playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
					} else {
						xDist = abs(((xRayX - (float)_playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
					}
				}

				if (yDist < xDist && xDist - yDist < 12.0 && (((uint)yRayX & 0x7F) == 0 || ((uint)yRayX & 0x7F) == 0x7F)) {
					yDist = xDist + 1.0;
				}
			}

			if (xGridTile > 0 && (xGridTile & kVertical) != 0) {
				if (angleSin <= 0.0) {
					xEdge = ((uint)xRayX & 0xFF80) - 1;
				} else {
					xEdge = ((uint)xRayX & 0xFF80) + 128;
				}

				xRayY += (((float)xEdge - xRayX) * angleAtan) / 2.0;
				xRayX += ((float)xEdge - xRayX) / 2.0;

				if ((quadrant & 3) == 0 || (quadrant & 3) == 3) {
					xDist = abs(((float)(xRayY - _playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				} else {
					xDist = abs(((float)(xRayX - _playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
				}

				if (xGridTile == yGridTile) {
					if (angleCos <= 0.0) {
						yEdge = ((uint)yRayY & 0xFF80) - 1;
					} else {
						yEdge = ((uint)yRayY & 0xFF80) + 128;
					}

					yRayX += (((float)yEdge - yRayY) / angleAtan) / 2.0;
					yRayY += ((float)yEdge - yRayY) / 2.0;

					if ((quadrant & 3) == 0 || (quadrant & 3) == 3) {
						yDist = abs(((yRayY - (float)_playerY) / _cosTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
					} else {
						yDist = abs(((yRayX - (float)_playerX) / _sinTable[(rotatedColumnAngleForX + (quadrant * -2048)) & 0xFFF]) * _cosTable[columnAngleForX]);
					}
				}

				if (xDist < yDist && yDist - xDist < 12.0 && (((uint)xRayY & 0x7F) == 0 || ((uint)xRayY & 0x7F) == 0x7F)) {
					xDist = yDist + 1.0;
				}
			}

			byte wallIDs[3], wallIsSpecial[3];
			uint32 hitWallValue, hitWallLightValue;
			uint textureColumn, cellHeight;

			if (yDist <= xDist) {
				rayStartX = yRayX;
				rayStartY = yRayY;

				if (yGridTile > 0 && !(yGridTile & kVertical)) {
					hitWallValue = yGridTile;
					hitWallLightValue = _wallLightMap[yGrid.y * _mapFullWidth + yGrid.x];

					depth = yDist < 1.0 ? 1.0 : yDist;
					_depthBuffer[x] = depth;

					textureColumn = (uint)rayStartX & 0x7F;
					if (rotatedColumnAngleForX < 1024 || rotatedColumnAngleForX > 3072) {
						textureColumn = 127 - textureColumn;
					}

					if (!(yGridTile & kWall) || yGridTile & kHasBlankWalls || yGridTile & kTransparentWall) {
						cellHeight = _heightMap[yGrid.y * _mapFullWidth + yGrid.x];
					} else {
						shouldBreak = true;
						cellHeight = _wallHeight * 128;
					}
				} else {
					continue;
				}
			} else {
				rayStartX = xRayX;
				rayStartY = xRayY;

				if (xGridTile > 0 && !(xGridTile & kHorizontal)) {
					hitWallValue = xGridTile;
					hitWallLightValue = _wallLightMap[xGrid.y * _mapFullWidth + xGrid.x];

					depth = xDist < 1.0 ? 1.0 : xDist;
					_depthBuffer[x] = depth;

					textureColumn = (uint)rayStartY & 0x7F;
					if (rotatedColumnAngleForX > 0 && rotatedColumnAngleForX < 2048) {
						textureColumn = 127 - textureColumn;
					}

					if (!(xGridTile & kWall) || xGridTile & kHasBlankWalls || xGridTile & kTransparentWall) {
						cellHeight = _heightMap[xGrid.y * _mapFullWidth + xGrid.x];
					} else {
						shouldBreak = true;
						cellHeight = _wallHeight * 128;
					}
				} else {
					continue;
				}
			}

			// Draw the column
			byte lightValues[3];
			lightValues[0] = hitWallLightValue & 0xF;
			lightValues[1] = (hitWallLightValue >> 4) & 0xF;
			lightValues[2] = (hitWallLightValue >> 8) & 0xF;

			wallIDs[0] = hitWallValue & 0xFF;
			wallIDs[1] = (hitWallValue >> 8) & 0xFF;
			wallIDs[2] = (hitWallValue >> 16) & 0xFF;

			for (uint i = 0; i < 3; ++i) {
				wallIsSpecial[i] = wallIDs[i] & 0x80;
				wallIDs[i] &= 0x7F;
			}

			int drawnWallHeight = (int)((float)(_fov * cellHeight) / depth) + 1;
			if (drawnWallHeight == 0) {
				drawnWallHeight = 1;
			}

			int drawnWallBottom = (int)((float)(_fov * _playerAltitude) / depth + viewportCenterY) + 1;
			int drawnWallTop = drawnWallBottom - drawnWallHeight + 1;
			int numSrcPixelsToDraw = cellHeight;
			uint32 srcY = _wallHeight * 128 - 1;
			float heightRatio = (float)(cellHeight) / (float)drawnWallHeight;

			// Clip top of wall
			if (drawnWallTop < viewBounds.top) {
				drawnWallTop = viewBounds.top - drawnWallTop;
				drawnWallHeight -= drawnWallTop;
				numSrcPixelsToDraw = cellHeight - (int)((float)drawnWallTop * heightRatio) - 1;
				drawnWallTop = viewBounds.top;
			}

			// Clip bottom of wall
			if (drawnWallBottom > viewBottom) {
				drawnWallBottom -= viewBottom;
				drawnWallHeight -= drawnWallBottom;
				numSrcPixelsToDraw -= (int)((float)drawnWallBottom * heightRatio) + 1;
				srcY -= (int)((float)drawnWallBottom * heightRatio);
				drawnWallBottom = viewBottom;
			}

			if (_wallHeight != 3) {
				// Other cases not implemented since the nancy3 data only has a height of 3
				warning("Raycast rendering for _wallHeight != 3 not implemented");
			}

			if (drawnWallHeight > 1) {
				uint16 *destPixel = (uint16 *)_drawSurface.getBasePtr(x, drawnWallBottom);
				byte *zBufferDestPixel = &_zBuffer[drawnWallBottom * _drawSurface.w + x];
				byte baseLightVal = MIN<byte>(_depthBuffer[x] / 128, 7);
				uint srcYSubtractVal = (uint)(((float)numSrcPixelsToDraw / (float)drawnWallHeight) * 65536.0);

				srcY <<= 16;

				for (uint i = 0; i < 3; ++i) {
					lightValues[i] = MIN<byte>(lightValues[i] + baseLightVal, 7);
				}

				uint16 *srcPixels[3];

				for (uint i = 0; i < 3; ++i) {
					if (!wallIDs[i]) {
						srcPixels[i] = nullptr;
					} else {
						srcPixels[i] = wallIsSpecial[i] ? 	(uint16 *)_specialWallTextures[wallIDs[i]][lightValues[i]].getBasePtr(textureColumn, 0) :
															(uint16 *)_wallTextures[wallIDs[i]][lightValues[i]].getBasePtr(textureColumn, 0);
					}
				}

				if (!(hitWallValue & kHasBlankWalls) && !(hitWallValue & kTransparentWall)) {
					if (isBehindTransparentWall) {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && *zBufferDestPixel != curZBufferDepth) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					} else {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel) {
								*destPixel = *srcPixel;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							*zBufferDestPixel = curZBufferDepth;
							zBufferDestPixel -= _drawSurface.w;
						}
					}
				} else if (!(hitWallValue & kHasBlankWalls) && hitWallValue & kTransparentWall) {
					if (isBehindTransparentWall) {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && *zBufferDestPixel != curZBufferDepth && *srcPixel != transColor) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					} else {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && *srcPixel != transColor) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					}
				} else if (hitWallValue & kHasBlankWalls && hitWallValue & kTransparentWall) {
					if (isBehindTransparentWall) {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && wallIDs[2 - (sY >> 7)] != 0 && *zBufferDestPixel != curZBufferDepth && *srcPixel != transColor) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					} else {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && wallIDs[2 - (sY >> 7)] != 0 && *srcPixel != transColor) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					}
				} else {
					if (isBehindTransparentWall) {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && wallIDs[2 - (sY >> 7)] != 0 && *zBufferDestPixel != curZBufferDepth) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					} else {
						for (int y = 0; y < drawnWallHeight; ++y) {
							uint32 sY = srcY >> 16;
							uint16 *srcPixel = srcPixels[2 - (sY >> 7)] ? &srcPixels[2 - (sY >> 7)][128 * (sY & 0x7F)] : nullptr;

							if (srcPixel && wallIDs[2 - (sY >> 7)] != 0) {
								*destPixel = *srcPixel;
								*zBufferDestPixel = curZBufferDepth;
							}

							srcY -= srcYSubtractVal;
							destPixel -= _drawSurface.w;
							zBufferDestPixel -= _drawSurface.w;
						}
					}
				}
			}

			if (!(hitWallValue & kHasBlankWalls) && !(hitWallValue & kTransparentWall)) {
				if (drawnWallTop < viewBottom) {
					viewBottom = drawnWallTop;
				}
				isBehindTransparentWall = false;
			} else {
				isBehindTransparentWall = true;
			}

			if (viewBounds.bottom < viewBottom) {
				viewBottom = viewBounds.bottom;
			}
		}
	}

	// Draw floors and ceilings
	// TODO exit does not get rendered correctly
	int32 leftAngle = _playerRotation + _leftmostAngle;
	int32 rightAngle = _playerRotation + _rightmostAngle;

	clampRotation(leftAngle);
	clampRotation(rightAngle);

	for (int floorY = viewportCenterY + 5; floorY < viewBounds.bottom; ++floorY) {
		int ceilingY = viewportCenterY - (floorY - viewportCenterY) + 1;
		uint32 	floorSrcFracX, floorSrcFracY,
				ceilingSrcFracX, ceilingSrcFracY,
				floorSrcIncrementX, floorSrcIncrementY,
				ceilingSrcIncrementX, ceilingSrcIncrementY;

		uint16 *floorDest = (uint16 *)_drawSurface.getBasePtr(viewBounds.left, floorY);
		uint16 *ceilingDest = (uint16 *)_drawSurface.getBasePtr(viewBounds.left, ceilingY);

		{
			float floorSrcX, floorSrcY, ceilingSrcX, ceilingSrcY;

			float floorViewAngle = ((float)_fov / (float)(floorY - viewportCenterY)) * (float)_playerAltitude;
			float ceilingViewAngle = ((float)_fov / (float)(viewportCenterY - ceilingY)) * (float)((_wallHeight * 128) - _playerAltitude);

			floorSrcX = _cosTable[leftAngle] * (floorViewAngle / _cosTable[_leftmostAngle]) + (float)_playerY;
			floorSrcY = _sinTable[leftAngle] * -(floorViewAngle / _cosTable[_leftmostAngle]) + (float)_playerX;
			ceilingSrcX = _cosTable[leftAngle] * (ceilingViewAngle / _cosTable[_leftmostAngle]) + (float)_playerY;
			ceilingSrcY = _sinTable[leftAngle] * -(ceilingViewAngle / _cosTable[_leftmostAngle]) + (float)_playerX;

			floorSrcFracX = (uint32)(floorSrcX * 65536.0);
			floorSrcFracY = (uint32)(floorSrcY * 65536.0);

			ceilingSrcFracX = (uint32)(ceilingSrcX * 65536.0);
			ceilingSrcFracY = (uint32)(ceilingSrcY * 65536.0);

			floorViewAngle /= _cosTable[_rightmostAngle];
			ceilingViewAngle /= _cosTable[_rightmostAngle];

			floorSrcIncrementX = (uint32)(((_cosTable[rightAngle] * floorViewAngle + (float)_playerY - floorSrcX) / (float)viewBounds.width()) * 65536.0);
			floorSrcIncrementY = (uint32)(((_sinTable[rightAngle] * -(floorViewAngle) + (float)_playerX - floorSrcY) / (float)viewBounds.width()) * 65536.0);

			ceilingSrcIncrementX = (uint32)(((_cosTable[rightAngle] * ceilingViewAngle + (float)_playerY - ceilingSrcX) / (float)viewBounds.width()) * 65536.0);
			ceilingSrcIncrementY = (uint32)(((_sinTable[rightAngle] * -(ceilingViewAngle) + (float)_playerX - ceilingSrcY) / (float)viewBounds.width()) * 65536.0);
		}

		for (int x = viewBounds.left; x < viewBounds.right; ++x) {
			if (_zBuffer[floorY * _drawSurface.w + x] != curZBufferDepth) {
				byte offset = (floorSrcFracY >> 23) * _mapFullWidth + (floorSrcFracX >> 23);
				*floorDest = *(int16 *)_floorTextures[_floorMap[offset]][_floorCeilingLightMap[offset] & 0xF].getBasePtr((floorSrcFracX >> 16) & 0x7F, (floorSrcFracY >> 16) & 0x7F);
			}

			++floorDest;
			floorSrcFracX += floorSrcIncrementX;
			floorSrcFracY += floorSrcIncrementY;

			if (_zBuffer[ceilingY * _drawSurface.w + x] != curZBufferDepth) {
				byte offset = (ceilingSrcFracY >> 23) * _mapFullWidth + (ceilingSrcFracX >> 23);
				*ceilingDest = *(int16 *)_ceilingTextures[_ceilingMap[offset]][(_floorCeilingLightMap[offset] >> 4) & 0xF].getBasePtr((ceilingSrcFracX >> 16) & 0x7F, (ceilingSrcFracY >> 16) & 0x7F);
			}

			++ceilingDest;
			ceilingSrcFracX += ceilingSrcIncrementX;
			ceilingSrcFracY += ceilingSrcIncrementY;
		}
	}

	_lastZDepth += 2;
	if (_lastZDepth > 250) {
		clearZBuffer();
	}

	_needsRedraw = true;
}

void RaycastPuzzle::clearZBuffer() {
	for (uint i = 0; i < _zBuffer.size(); ++i) {
		_zBuffer[i] = 0;
	}

	_lastZDepth = 0;
}

void RaycastPuzzle::checkSwitch() {

}

} // End of namespace Action
} // End of namespace Nancy
