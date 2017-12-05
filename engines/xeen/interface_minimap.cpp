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

#include "xeen/interface_minimap.h"
#include "xeen/xeen.h"

namespace Xeen {

void InterfaceMinimap::drawMinimap() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Resources &res = *g_vm->_resources;
	Windows &windows = *g_vm->_windows;
	Window &window1 = windows[1];

	if (windows[2]._enabled || windows[10]._enabled)
		return;
	if (!party._automapOn && !party._wizardEyeActive) {
		// Draw the Might & Magic logo
		res._globalSprites.draw(window1, 5, Common::Point(232, 9));
		return;
	}

	int v, frame;
	int frame2 = 0; // _overallFrame * 2;
	bool eyeActive = party._wizardEyeActive;
	if (party._automapOn)
		party._wizardEyeActive = false;

	if (map._isOutdoors) {
		res._globalSprites.draw(window1, 15, Common::Point(237, 12));

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);
				frame = map.mazeDataCurrent()._surfaceTypes[v];

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}
			}
		}

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);
				frame = map.mazeData()._wallTypes[v];

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame + 16, Common::Point(xp, yp));
				}
			}
		}

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);

				if (v != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, v + 32, Common::Point(xp, yp));
				}
			}
		}

		// Draw the direction arrow
		res._globalSprites.draw(window1, party._mazeDirection + 1,
			Common::Point(267, 36));
	} else {
		frame2 = (frame2 + 2) % 8;

		// First draw the default surface bases for each cell to show
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);

				if (v != INVALID_CELL && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, 0, Common::Point(xp, yp));
				}
			}
		}

		// Draw correct surface bases for revealed tiles
		for (int rowNum = 0, yp = 17, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 242, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);
				int surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];

				if (v != INVALID_CELL && map._currentSurfaceId &&
					(map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, surfaceId + 36, Common::Point(xp, yp));
				}
			}
		}

		v = map.mazeLookup(Common::Point(party._mazePosition.x - 4, party._mazePosition.y + 4), 0xffff, 0);
		if (v != INVALID_CELL && map._currentSurfaceId &&
			(map._currentSteppedOn || party._wizardEyeActive)) {
			map._tileSprites.draw(window1,
				map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
				Common::Point(232, 9));
		}

		// Handle drawing surface sprites partially clipped at the left edge
		for (int rowNum = 0, yp = 17, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, --yDiff, yp += 8) {
			v = map.mazeLookup(
				Common::Point(party._mazePosition.x - 4, party._mazePosition.y + yDiff),
				0, 0xffff);

			if (v != INVALID_CELL && map._currentSurfaceId &&
				(map._currentSteppedOn || party._wizardEyeActive)) {
				map._tileSprites.draw(window1,
					map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
					Common::Point(232, yp));
			}
		}

		// Handle drawing surface sprites partially clipped at the top edge
		for (int colNum = 0, xp = 242, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, ++xDiff, xp += 8) {
			v = map.mazeLookup(
				Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + 4),
				0, 0xffff);

			if (v != INVALID_CELL && map._currentSurfaceId &&
				(map._currentSteppedOn || party._wizardEyeActive)) {
				map._tileSprites.draw(window1,
					map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
					Common::Point(xp, 9));
			}
		}

		//
		for (int idx = 0, xp = 237, yp = 60, xDiff = -3; idx < MINIMAP_SIZE;
			++idx, ++xDiff, xp += 10, yp -= 8) {
			v = map.mazeLookup(
				Common::Point(party._mazePosition.x - 4, party._mazePosition.y - 3 + idx),
				12, 0xffff);

			switch (v) {
			case 1:
				frame = 18;
				break;
			case 3:
				frame = 22;
				break;
			case 4:
			case 13:
				frame = 16;
				break;
			case 5:
			case 8:
				frame = 2;
				break;
			case 6:
				frame = 30;
				break;
			case 7:
				frame = 32;
				break;
			case 9:
				frame = 24;
				break;
			case 10:
				frame = 28;
				break;
			case 11:
				frame = 14;
				break;
			case 12:
				frame = frame2 + 4;
				break;
			case 14:
				frame = 24;
				break;
			case 15:
				frame = 26;
				break;
			default:
				frame = -1;
				break;
			}

			if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive))
				map._tileSprites.draw(window1, frame, Common::Point(222, yp));

			v = map.mazeLookup(
				Common::Point(party._mazePosition.x - 3 + idx, party._mazePosition.y + 4),
				0);

			switch (v) {
			case 1:
				frame = 19;
				break;
			case 2:
				frame = 35;
				break;
			case 3:
				frame = 23;
				break;
			case 4:
			case 13:
				frame = 17;
				break;
			case 5:
			case 8:
				frame = 3;
				break;
			case 6:
				frame = 31;
				break;
			case 7:
				frame = 33;
				break;
			case 9:
				frame = 21;
				break;
			case 10:
				frame = 29;
				break;
			case 11:
				frame = 15;
				break;
			case 12:
				frame = frame2 + 5;
				break;
			case 14:
				frame = 25;
				break;
			case 15:
				frame = 27;
				break;
			default:
				frame = -1;
				break;
			}

			if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive))
				map._tileSprites.draw(window1, frame, Common::Point(xp, 4));
		}

		// Draw the front/back walls of cells in the minimap
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE;
			++rowNum, --yDiff, yp += 8) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE;
				++colNum, ++xDiff, xp += 10) {
				if (colNum == 4 && rowNum == 4) {
					// Center of the minimap. Draw the direction arrow
					res._globalSprites.draw(window1, party._mazeDirection + 1,
						Common::Point(272, 40));
				}

				v = map.mazeLookup(Common::Point(party._mazePosition.x + xDiff,
					party._mazePosition.y + yDiff), 12, 0xffff);
				switch (v) {
				case 1:
					frame = 18;
					break;
				case 3:
					frame = 22;
					break;
				case 4:
				case 13:
					frame = 16;
					break;
				case 5:
				case 8:
					frame = 2;
					break;
				case 6:
					frame = 30;
					break;
				case 7:
					frame = 32;
					break;
				case 9:
					frame = 20;
					break;
				case 10:
					frame = 28;
					break;
				case 11:
					frame = 14;
					break;
				case 12:
					frame = frame2 + 4;
					break;
				case 14:
					frame = 24;
					break;
				case 15:
					frame = 26;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}

				v = map.mazeLookup(Common::Point(party._mazePosition.x + xDiff,
					party._mazePosition.y + yDiff), 12, 0xffff);
				switch (v) {
				case 1:
					frame = 19;
					break;
				case 2:
					frame = 35;
					break;
				case 3:
					frame = 23;
					break;
				case 4:
				case 13:
					frame = 17;
					break;
				case 5:
				case 8:
					frame = 3;
					break;
				case 6:
					frame = 31;
					break;
				case 7:
					frame = 33;
					break;
				case 9:
					frame = 21;
					break;
				case 10:
					frame = 29;
					break;
				case 11:
					frame = 15;
					break;
				case 12:
					frame = frame2 + 5;
					break;
				case 14:
					frame = 25;
					break;
				case 15:
					frame = 27;
					break;
				default:
					frame = -1;
					break;
				}

				if (v == -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}
			}
		}

		// Draw the top of blocked/wall cells on the map
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);

				if (v == INVALID_CELL || (!map._currentSteppedOn && !party._wizardEyeActive)) {
					map._tileSprites.draw(window1, 1, Common::Point(xp, yp));
				}
			}
		}
	}

	// Draw outer rectangle around the automap
	res._globalSprites.draw(window1, 6, Common::Point(223, 3));
	party._wizardEyeActive = eyeActive;
}

} // End of namespace Xeen
