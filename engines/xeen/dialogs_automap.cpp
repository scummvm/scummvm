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

#include "xeen/dialogs_automap.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {


void AutoMapDialog::show(XeenEngine *vm) {
	AutoMapDialog *dlg = new AutoMapDialog(vm);
	dlg->execute();
	delete dlg;
}

void AutoMapDialog::execute() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int frame2 = intf._overallFrame * 2;
	int varSI = 1;
	bool frameEndFlag;

	int yDiffStart = 8;
	int xDiffStart = 248;
	Common::Point pt = party._mazePosition;
	Common::Point arrowPt;
	SpriteResource globalSprites;
	globalSprites.load("global.icn");

	if (pt.x < 8 && map.mazeData()._surroundingMazes._west == 0) {
		arrowPt.x = pt.x * 10 + 4;
	} else if (pt.x > 23) {
		arrowPt.x = pt.x * 10 + 100;
		pt.x = 23;
	} else if (pt.x > 8 && map.mazeData()._surroundingMazes._east == 0) {
		arrowPt.x = pt.x * 10 + 4;
		pt.x = 7;
	} else {
		arrowPt.x = 74;
	}

	if (pt.y < 8 && map.mazeData()._surroundingMazes._south == 0) {
		arrowPt.y = ((15 - pt.y) << 3) + 13;
		pt.y = 8;
	} else if (pt.y > 24) {
		arrowPt.y = ((15 - (pt.y - 24)) << 3) + 13;
		pt.y = 24;
	} else if (pt.y >= 8 && map.mazeData()._surroundingMazes._north == 0) {
		arrowPt.y = ((15 - pt.y) << 3) + 13;
		pt.y = 8;
	} else {
		arrowPt.y = 69;
	}

	screen._windows[5].open();
	MazeData &mazeData = map.mazeDataCurrent();
	bool drawFlag = true;
	int v;
	
	events.updateGameCounter();
	do {
		if (drawFlag)
			intf.draw3d(false);
		screen._windows[5].writeString("\n");

		if (map._isOutdoors) {
			// Draw outdoors map
			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16; 
					++yCtr, --yDiff, yp += 8) {
				for (int xp = 80, xDiff = xDiffStart + 1; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0);

					if (map._currentSteppedOn) {
						map._tileSprites.draw(screen, map.mazeDataCurrent()._surfaceTypes[v],
							Common::Point(xp, yp));
					}
				}
			}

			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16;
				++yCtr, --yDiff, yp += 8) {
				for (int xp = 80, xDiff = xDiffStart + 1; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 4);
					int wallType = map.mazeDataCurrent()._wallTypes[v];

					if (wallType && map._currentSteppedOn)
						map._tileSprites.draw(screen, wallType, Common::Point(xp, yp));
				}
			}

			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16;
				++yCtr, --yDiff, yp += 8) {
				for (int xCtr = 0, xp = 80, xDiff = xDiffStart + 1; xp < 240; 
						++xCtr, ++xDiff, xp += 10) {
					if (xCtr == (arrowPt.x / 10) && yCtr == (14 - (arrowPt.y)) && frameEndFlag)
						map._tileSprites.draw(screen, party._mazeDirection + 1,
							Common::Point(arrowPt.x + 81, arrowPt.y + 29));
					
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 12);					
					int frame;
					switch (v) {
					case SURFTYPE_WATER:
						frame = 18;
						break;
					case SURFTYPE_DIRT:
						frame = 34;
						break;
					case SURFTYPE_GRASS:
						frame = 22;
						break;
					case SURFTYPE_SNOW:
					case SURFTYPE_SEWER:
						frame = 16;
						break;
					case SURFTYPE_SWAMP:
					case SURFTYPE_ROAD:
						frame = 2;
					case SURFTYPE_LAVA:
						frame = 30;
						break;
					case SURFTYPE_DESERT:
						frame = 32;
						break;
					case SURFTYPE_DWATER:
						frame = 20;
						break;
					case SURFTYPE_TFLR:
						frame = 28;
						break;
					case SURFTYPE_SKY:
						frame = 14;
						break;
					case SURFTYPE_CROAD:
						frame = frame2 + 4;
						break;
					case SURFTYPE_CLOUD:
						frame = 24;
						break;
					case SURFTYPE_SCORCH:
						frame = 26;
						break;
					default:
						frame = -1;
						break;
					}
					
					if (frame != -1 && map._currentSteppedOn)
						map._tileSprites.draw(screen, frame, Common::Point(xp, yp));

					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0);
					switch (v) {
					case SURFTYPE_WATER:
						frame = 19;
						break;
					case SURFTYPE_DIRT:
						frame = 35;
						break;
					case SURFTYPE_GRASS:
						frame = 23;
						break;
					case SURFTYPE_SNOW:
					case SURFTYPE_SEWER:
						frame = 17;
						break;
					case SURFTYPE_SWAMP:
					case SURFTYPE_ROAD:
						frame = 3;
						break;
					case SURFTYPE_LAVA:
						frame = 31;
						break;
					case SURFTYPE_DESERT:
						frame = 33;
						break;
					case SURFTYPE_DWATER:
						frame = 21;
						break;
					case SURFTYPE_TFLR:
						frame = 29;
						break;
					case SURFTYPE_SKY:
						frame = 15;
						break;
					case SURFTYPE_CROAD:
						frame = frame2 + 5;
						break;
					case SURFTYPE_CLOUD:
						frame = 25;
						break;
					case SURFTYPE_SCORCH:
						frame = 27;
						break;
					default:
						frame = -1;
						break;
					}
					
					if (frame != -1 && map._currentSteppedOn)
						map._tileSprites.draw(screen, frame, Common::Point(xp, yp));
				}
			}

			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16;
				++yCtr, --yDiff, yp += 8) {
				for (int xp = 80, xDiff = xDiffStart + 1; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v != INVALID_CELL && map._currentSteppedOn)
						map._tileSprites.draw(screen, 1, Common::Point(xp, yp));
				}
			}
		} else {
			// Draw indoors map
			frame2 = (frame2 + 2) % 8;

			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16;
				++yCtr, --yDiff, yp += 8) {
				for (int xp = 80, xDiff = xDiffStart + 1; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v != INVALID_CELL && map._currentSteppedOn)
						map._tileSprites.draw(screen, 0, Common::Point(xp, yp));
				}
			}

			for (int yDiff = yDiffStart - 1, yp = 38; yp < 171; --yDiff, yp += 8) {
				v = map.mazeLookup(Common::Point(pt.x - 8, yDiff), 0, 0xffff);

				if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
					map._tileSprites.draw(screen, 36 + map.mazeData()._surfaceTypes[v],
						Common::Point(75, yp));
			}

			v = map.mazeLookup(Common::Point(pt.x - 8, pt.y + 8), 0, 0xffff);
			if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
				map._tileSprites.draw(screen, 36 + map.mazeData()._surfaceTypes[v],
				Common::Point(75, 35));

			for (int xp = 85, xDiff = xDiffStart + 1; xp < 245; xp += 10, ++xDiff) {
				v = map.mazeLookup(Common::Point(xDiff, pt.y + 8), 0, 0xffff);

				if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
					map._tileSprites.draw(screen, 36 + map.mazeData()._surfaceTypes[v],
					Common::Point(xp, 35));
			}

			for (int xp = 80, yp = 158, xDiff = pt.x - 7, yDiff = pt.y - 8; xp < 250; 
					xp += 10, yp += 8, ++xDiff, ++yDiff) {
				v = map.mazeLookup(Common::Point(pt.x - 8, yDiff), 12);

				int frame;
				switch (v) {
				case SURFTYPE_WATER:
					frame = 18;
					break;
				case SURFTYPE_GRASS:
					frame = 24;
					break;
				case SURFTYPE_SNOW:
				case SURFTYPE_SEWER:
					frame = 16;
					break;
				case SURFTYPE_SWAMP:
				case SURFTYPE_ROAD:
					frame = 2;
					break;
				case SURFTYPE_LAVA:
					frame = 30;
					break;
				case SURFTYPE_DESERT:
					frame = 32;
					break;
				case SURFTYPE_DWATER:
					frame = 20;
					break;
				case SURFTYPE_TFLR:
					frame = 28;
					break;
				case SURFTYPE_SKY:
					frame = 14;
					break;
				case SURFTYPE_CROAD:
					frame = frame2 + 4;
					break;
				case SURFTYPE_CLOUD:
					frame = 24;
					break;
				case SURFTYPE_SCORCH:
					frame = 26;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && map._currentSteppedOn)
					map._tileSprites.draw(screen, frame, Common::Point(70, yp));

				v = map.mazeLookup(Common::Point(xDiff, pt.y + 8), 0);

				switch (v) {
				case SURFTYPE_WATER:
					frame = 19;
					break;
				case SURFTYPE_DIRT:
					frame = 35;
					break;
				case SURFTYPE_GRASS:
					frame = 23;
					break;
				case SURFTYPE_SNOW:
				case SURFTYPE_SEWER:
					frame = 17;
					break;
				case SURFTYPE_SWAMP:
				case SURFTYPE_ROAD:
					frame = 3;
					break;
				case SURFTYPE_LAVA:
					frame = 31;
					break;
				case SURFTYPE_DESERT:
					frame = 33;
					break;
				case SURFTYPE_DWATER:
					frame = 21;
					break;
				case SURFTYPE_TFLR:
					frame = 29;
					break;
				case SURFTYPE_SKY:
					frame = 15;
					break;
				case SURFTYPE_CROAD:
					frame = frame2 + 5;
					break;
				case SURFTYPE_CLOUD:
					frame = 25;
					break;
				case SURFTYPE_SCORCH:
					frame = 27;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && map._currentSteppedOn)
					map._tileSprites.draw(screen, frame, Common::Point(xp, 30));
			}

			for (int yCtr = 0, yDiff = yDiffStart - 1, yp = 38; yCtr < 16;
				++yCtr, --yDiff, yp += 8) {
				for (int xp = 80, xDiff = xDiffStart + 1; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v != INVALID_CELL && map._currentSteppedOn)
						map._tileSprites.draw(screen, 0, Common::Point(xp, yp));
				}
			}
		}

		screen._windows[5].frame();
		if (!map._isOutdoors) {
			map._tileSprites.draw(screen, 52, Common::Point(76, 30));
		} else if (frameEndFlag) {
			globalSprites.draw(screen, party._mazeDirection + 1,
				Common::Point(arrowPt.x + 76, arrowPt.y + 25));
		}

		if (events.timeElapsed() > 5) {
			// Set the flag to make the basic arrow blinking effect
			frameEndFlag = true;
			events.updateGameCounter();
		}

		screen._windows[5].writeString(Common::String::format(MAP_TEXT,
			map._mazeName.c_str(), party._mazePosition.x,
			party._mazePosition.y, DIRECTION_TEXT[party._mazeDirection]));
		screen._windows[5].update();
		screen._windows[3].update();

		events.pollEvents();
	} while (!_vm->shouldQuit() && !events.isKeyMousePressed());

	events.clearEvents();
	screen._windows[5].close();
}

} // End of namespace Xeen
