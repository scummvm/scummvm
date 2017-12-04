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
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	int frame2 = intf._overallFrame * 2;
	bool frameEndFlag = false;

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

	windows[5].open();
//	MazeData &mazeData = map.mazeDataCurrent();
	bool drawFlag = true;
	int v;

	events.updateGameCounter();
	do {
		if (drawFlag)
			intf.draw3d(false, true);
		windows[5].writeString("\n");

		if (map._isOutdoors) {
			// Draw outdoors map
			for (int yp = 38, yDiff = pt.y + 7; yp < 166; --yDiff, yp += 8) {
				for (int xp = 80, xDiff = pt.x - 7; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0);

					if (map._currentSteppedOn) {
						map._tileSprites.draw(0, map.mazeDataCurrent()._surfaceTypes[v],
							Common::Point(xp, yp));
					}
				}
			}

			for (int yp = 38, yDiff = pt.y + 7; yp < 166; --yDiff, yp += 8) {
				for (int xp = 80, xDiff = pt.x - 7; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 4);
					int wallType = map.mazeDataCurrent()._wallTypes[v];

					if (wallType && map._currentSteppedOn)
						map._tileSprites.draw(0, wallType, Common::Point(xp, yp));
				}
			}

			for (int yp = 38, yDiff = pt.y + 7; yp < 166; yp += 8, --yDiff) {
				for (int xp = 80, xDiff = -7; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 8);

					if (v && map._currentSteppedOn)
						map._tileSprites.draw(0, 1, Common::Point(xp, yp));
				}
			}
		} else {
			// Draw indoors map
			frame2 = (frame2 + 2) % 8;

			// Draw default ground for all the valid explored areas
			for (int yp = 38, yDiff = pt.y + 7; yp < 166; yp += 8, --yDiff) {
				for (int xp = 80, xDiff = pt.x - 7; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v != INVALID_CELL && map._currentSteppedOn)
						map._tileSprites.draw(0, 0, Common::Point(xp, yp));
				}
			}

			// Draw thinner ground tiles on the left edge of the map
			for (int yp = 43, yDiff = pt.y + 7; yp < 171; yp += 8, --yDiff) {
				v = map.mazeLookup(Common::Point(pt.x - 8, yDiff), 0, 0xffff);

				if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
					map._tileSprites.draw(0, 36 + map.mazeData()._surfaceTypes[
						map._currentSurfaceId], Common::Point(75, yp));
			}

			// Draw thin tile portion on top-left corner of map
			v = map.mazeLookup(Common::Point(pt.x - 8, pt.y + 8), 0, 0xffff);
			if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
				map._tileSprites.draw(0, 36 + map.mazeData()._surfaceTypes[
					map._currentSurfaceId], Common::Point(75, 35));

			// Draw any thin tiles at the very top of the map
			for (int xp = 85, xDiff = pt.x - 7; xp < 245; xp += 10, ++xDiff) {
				v = map.mazeLookup(Common::Point(xDiff, pt.y + 8), 0, 0xffff);

				if (v != INVALID_CELL && map._currentSurfaceId != 0 && map._currentSteppedOn)
					map._tileSprites.draw(0, 36 + map.mazeData()._surfaceTypes[
						map._currentSurfaceId], Common::Point(xp, 35));
			}

			// Draw the default ground tiles
			for (int yp = 43, yDiff = pt.y + 7; yp < 171; yp += 8, --yDiff) {
				for (int xp = 85, xDiff = pt.x - 7; xp < 245; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v != INVALID_CELL && map._currentSurfaceId && map._currentSteppedOn)
						map._tileSprites.draw(0, map.mazeData()._surfaceTypes[
							map._currentSurfaceId], Common::Point(xp, yp));
				}
			}

			// Draw walls on left and top edges of map
			for (int xp = 80, yp = 158, xDiff = pt.x - 7, yDiff = pt.y - 8; xp < 250;
					xp += 10, yp -= 8, ++xDiff, ++yDiff) {
				// Draw walls on left edge of map
				v = map.mazeLookup(Common::Point(pt.x - 8, yDiff), 12);

				int frame;
				switch (v) {
				case SURFTYPE_DIRT:
					frame = 18;
					break;
				case SURFTYPE_SNOW:
					frame = 22;
					break;
				case SURFTYPE_SWAMP:
				case SURFTYPE_CLOUD:
					frame = 16;
					break;
				case SURFTYPE_LAVA:
				case SURFTYPE_DWATER:
					frame = 2;
					break;
				case SURFTYPE_DESERT:
					frame = 30;
					break;
				case SURFTYPE_ROAD:
					frame = 32;
					break;
				case SURFTYPE_TFLR:
					frame = 20;
					break;
				case SURFTYPE_SKY:
					frame = 28;
					break;
				case SURFTYPE_CROAD:
					frame = 14;
					break;
				case SURFTYPE_SEWER:
					frame = frame2 + 4;
					break;
				case SURFTYPE_SCORCH:
					frame = 24;
					break;
				case SURFTYPE_SPACE:
					frame = 26;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && map._currentSteppedOn)
					map._tileSprites.draw(0, frame, Common::Point(70, yp));

				// Draw walls on top edge of map
				v = map.mazeLookup(Common::Point(xDiff, pt.y + 8), 0);

				switch (v) {
				case SURFTYPE_DIRT:
					frame = 19;
					break;
				case SURFTYPE_GRASS:
					frame = 35;
					break;
				case SURFTYPE_SNOW:
					frame = 23;
					break;
				case SURFTYPE_SWAMP:
				case SURFTYPE_CLOUD:
					frame = 17;
					break;
				case SURFTYPE_LAVA:
				case SURFTYPE_DWATER:
					frame = 3;
					break;
				case SURFTYPE_DESERT:
					frame = 31;
					break;
				case SURFTYPE_ROAD:
					frame = 33;
					break;
				case SURFTYPE_TFLR:
					frame = 21;
					break;
				case SURFTYPE_SKY:
					frame = 29;
					break;
				case SURFTYPE_CROAD:
					frame = 15;
					break;
				case SURFTYPE_SEWER:
					frame = frame2 + 5;
					break;
				case SURFTYPE_SCORCH:
					frame = 25;
					break;
				case SURFTYPE_SPACE:
					frame = 27;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && map._currentSteppedOn)
					map._tileSprites.draw(0, frame, Common::Point(xp, 30));
			}

			// Draw any walls on the cells
			for (int yCtr = 0, yp = 38, yDiff = pt.y + 7; yCtr < 16; ++yCtr, yp += 8, --yDiff) {
				for (int xCtr = 0, xp = 80, xDiff = pt.x - 7; xCtr < 16; ++xCtr, xp += 10, ++xDiff) {
					// Draw the arrow if at the correct position
					if ((arrowPt.x / 10) == xCtr && (14 - (arrowPt.y / 10)) == yCtr && frameEndFlag) {
						globalSprites.draw(0, party._mazeDirection + 1,
							Common::Point(arrowPt.x + 81, arrowPt.y + 29));
					}

					v = map.mazeLookup(Common::Point(xDiff, yDiff), 12);
					int frame;
					switch (v) {
					case SURFTYPE_DIRT:
						frame = 18;
						break;
					case SURFTYPE_GRASS:
						frame = 34;
						break;
					case SURFTYPE_SNOW:
						frame = 22;
						break;
					case SURFTYPE_SWAMP:
					case SURFTYPE_CLOUD:
						frame = 16;
						break;
					case SURFTYPE_LAVA:
					case SURFTYPE_DWATER:
						frame = 2;
						break;
					case SURFTYPE_DESERT:
						frame = 30;
						break;
					case SURFTYPE_ROAD:
						frame = 32;
						break;
					case SURFTYPE_TFLR:
						frame = 20;
						break;
					case SURFTYPE_SKY:
						frame = 28;
						break;
					case SURFTYPE_CROAD:
						frame = 14;
						break;
					case SURFTYPE_SEWER:
						frame = frame2 + 4;
						break;
					case SURFTYPE_SCORCH:
						frame = 24;
						break;
					case SURFTYPE_SPACE:
						frame = 26;
						break;
					default:
						frame = -1;
						break;
					}

					if (frame != -1 && map._currentSteppedOn)
						map._tileSprites.draw(0, frame, Common::Point(xp, yp));

					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0);
					switch (v) {
					case SURFTYPE_DIRT:
						frame = 19;
						break;
					case SURFTYPE_GRASS:
						frame = 35;
						break;
					case SURFTYPE_SNOW:
						frame = 23;
						break;
					case SURFTYPE_SWAMP:
					case SURFTYPE_CLOUD:
						frame = 17;
						break;
					case SURFTYPE_LAVA:
					case SURFTYPE_DWATER:
						frame = 3;
						break;
					case SURFTYPE_DESERT:
						frame = 31;
						break;
					case SURFTYPE_ROAD:
						frame = 33;
						break;
					case SURFTYPE_TFLR:
						frame = 21;
						break;
					case SURFTYPE_SKY:
						frame = 29;
						break;
					case SURFTYPE_CROAD:
						frame = 15;
						break;
					case SURFTYPE_SEWER:
						frame = frame2 + 5;
						break;
					case SURFTYPE_SCORCH:
						frame = 25;
						break;
					case SURFTYPE_SPACE:
						frame = 27;
						break;
					default:
						frame = -1;
						break;
					}

					if (frame != -1 && map._currentSteppedOn)
						map._tileSprites.draw(0, frame, Common::Point(xp, yp));
				}
			}

			// Draw overlay on cells that haven't been stepped on yet
			for (int yDiff = pt.y + 7, yp = 38; yp < 166; --yDiff, yp += 8) {
				for (int xp = 80, xDiff = pt.x - 7; xp < 240; xp += 10, ++xDiff) {
					v = map.mazeLookup(Common::Point(xDiff, yDiff), 0, 0xffff);

					if (v == INVALID_CELL || !map._currentSteppedOn)
						map._tileSprites.draw(0, 1, Common::Point(xp, yp));
				}
			}
		}

		windows[5].frame();
		if (!map._isOutdoors) {
			map._tileSprites.draw(0, 52, Common::Point(76, 30));
		} else if (frameEndFlag) {
			globalSprites.draw(0, party._mazeDirection + 1,
				Common::Point(arrowPt.x + 76, arrowPt.y + 25));
		}

		if (events.timeElapsed() > 5) {
			// Set the flag to make the basic arrow blinking effect
			frameEndFlag = !frameEndFlag;
			events.updateGameCounter();
		}

		windows[5].writeString(Common::String::format(Res.MAP_TEXT,
			map._mazeName.c_str(), party._mazePosition.x,
			party._mazePosition.y, Res.DIRECTION_TEXT[party._mazeDirection]));
		windows[5].update();
		windows[3].update();

		events.pollEvents();
		drawFlag = false;
	} while (!_vm->shouldQuit() && !events.isKeyMousePressed());

	events.clearEvents();
	windows[5].close();
}

} // End of namespace Xeen
