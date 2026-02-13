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

#ifndef COLONY_COLONY_H
#define COLONY_COLONY_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"
#include "common/array.h"
#include "common/rect.h"
#include "colony/gfx.h"

namespace Colony {

#define BASEOBJECT 20
#define MENUM 101

struct Locate {
	uint8 ang;
	uint8 look;
	int lookx;
	int delta;
	int xloc;
	int yloc;
	int xindex;
	int yindex;
	int xmx, xmn;
	int zmx, zmn;
	int32 power[3];
	int type;
	int dx, dy;
	int dist;
};

struct Thing {
	int type;
	int visible;
	int alive;
	Common::Rect clip;
	int count;
	Locate where;
	int opcode;
	int counter;
	int time;
	int grow;
	// void (*make)(); // To be implemented as virtual functions or member function pointers
	// void (*think)();
};

class ColonyEngine : public Engine {
public:
	ColonyEngine(OSystem *syst, const ADGameDescription *gd);
	virtual ~ColonyEngine();

	Common::Error run() override;

	void initTrig();
	void loadMap(int mnum);
	void corridor();
	void quadrant();
	void perspective(int pnt[2], int rox, int roy);
	void rot_init(int x, int y);

private:
	const ADGameDescription *_gameDescription;

	uint8 _wall[32][32];
	uint8 _mapData[31][31][5][5];
	uint8 _robotArray[32][32];
	uint8 _foodArray[32][32];
	
	Locate _me;
	Common::Array<Thing> _objects;
	int _level;
	int _robotNum;

	Gfx *_gfx;
	
	int _rox, _roy;
	int _tsin, _tcos;
	int _sint[256];
	int _cost[256];
	int _rtable[11585];
	int _centerX, _centerY;
	int _width, _height;
	bool _flip;
	int _mouseSensitivity;
	bool _change;

	int _frntxWall, _frntyWall;
	int _sidexWall, _sideyWall;
	int _frntx, _frnty;
	int _sidex, _sidey;
	int _front, _side;
	int _direction;

	int _drX[34][34];
	int _drY[34][34];

	Common::Rect _clip;
	Common::Rect _screenR;

	void drawend(int xstart, int ystart, int xFrontLeft, int yFrontLeft);
};

} // End of namespace Colony

#endif // COLONY_COLONY_H
