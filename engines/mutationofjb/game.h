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

#include "common/scummsys.h"

namespace Common
{
	class ReadStream;
}

namespace MutationOfJB
{

static const uint8 MAX_STR_LENGTH = 0x14;

struct Door {
	char _name[MAX_STR_LENGTH + 1];
	uint8  _destSceneId;
	uint16 _destX;
	uint16 _destY;
	uint16 _x;
	uint8  _y;
	uint16 _width;
	uint8  _height;
	uint16 _walkToX;
	uint8  _walkToY;
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Object {
	uint8  _AC;
	uint8  _FA;
	uint8  _FR;
	uint8  _NA;
	uint8  _FS;
	uint8  _unknown;
	uint8  _CA;
	uint16 _x;
	uint8  _y;
	uint16 _XL;
	uint8  _YL;
	uint16 _WX;
	uint8  _WY;
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Static {
	uint8  _active;
	char _name[MAX_STR_LENGTH + 1];
	uint16 _x;
	uint8  _y;
	uint16 _width;
	uint8  _height;
	uint16 _walkToX;
	uint8  _walkToY;
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Bitmap {
	uint8  _unknown;
	uint8  _isVisible;
	uint16 _x1;
	uint8  _y1;
	uint16 _x2;
	uint8  _y2;

	bool loadFromStream(Common::ReadStream &stream);
};


struct SceneInfo {
	uint8 _startup;
	uint8 _unknown001;
	uint8 _unknown002;
	uint8 _unknown003;
	uint8 _DL;

	uint8 _noDoors;
	Door _doors[5];

	uint8 _noObjects;
	Object _objects[9];

	uint8 _noStatics;
	Static _statics[15];

	Bitmap _bitmaps[10];

	uint8 _obstacleY1;
	uint8 _unknown386;
	uint8 _palRotStart;
	uint8 _palRotEnd;
	uint8 _palRotPeriod;
	uint8 _unknown38A[80];

	bool loadFromStream(Common::ReadStream &stream);
};

struct GameData
{
	GameData();

	SceneInfo _scenes[45];
	uint8 _currentScene;

	bool loadFromStream(Common::ReadStream &stream);
};

}
