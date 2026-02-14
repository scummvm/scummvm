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

enum WallFeatureType {
	kWallFeatureNone = 0,
	kWallFeatureDoor = 2,
	kWallFeatureWindow = 3,
	kWallFeatureShelves = 4,
	kWallFeatureUpStairs = 5,
	kWallFeatureDnStairs = 6,
	kWallFeatureChar = 7,
	kWallFeatureGlyph = 8,
	kWallFeatureElevator = 9,
	kWallFeatureTunnel = 10,
	kWallFeatureAirlock = 11,
	kWallFeatureColor = 12
};

enum MapDirection {
	kDirNorth = 0,
	kDirEast = 1,
	kDirWest = 2,
	kDirSouth = 3
};

enum ObjectType {
	kObjDesk = 21,
	kObjPlant = 22,
	kObjCChair = 23,
	kObjBed = 24,
	kObjTable = 25,
	kObjCouch = 26,
	kObjChair = 27,
	kObjTV = 28,
	kObjScreen = 29,
	kObjConsole = 30,
	kObjPowerSuit = 31,
	kObjForkLift = 32,
	kObjCryo = 33,
	kObjBox1 = 34,
	kObjBox2 = 35,
	kObjTeleport = 36,
	kObjDrawer = 37,
	kObjTub = 38,
	kObjSink = 39,
	kObjToilet = 40,
	kObjPToilet = 43,
	kObjProjector = 45,
	kObjReactor = 46,
	kObjFWall = 48,
	kObjCWall = 49,
	kObjBBed = 42
};

#define BASEOBJECT 20
#define MENUM 101

struct Locate {
	uint8 ang;
	uint8 look;
	int8  lookY;
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

struct Image {
	int16 width;
	int16 height;
	int16 align;
	int16 rowBytes;
	int8 bits;
	int8 planes;
	byte *data;

	Image() : width(0), height(0), align(0), rowBytes(0), bits(0), planes(0), data(nullptr) {}
	~Image() { free(data); }
};

struct Sprite {
	Image *fg;
	Image *mask;
	Common::Rect clip;
	Common::Rect locate;
	bool used;

	Sprite() : fg(nullptr), mask(nullptr), used(false) {}
	~Sprite() { delete fg; delete mask; }
};

struct ComplexSprite {
	struct SubObject {
		int16 spritenum;
		int16 xloc, yloc;
	};
	Common::Array<SubObject> objects;
	Common::Rect bounds;
	bool visible;
	int16 current;
	int16 xloc, yloc;
	int16 acurrent;
	int16 axloc, ayloc;
	uint8 type;
	uint8 frozen;
	uint8 locked;
	int16 link;
	int16 key;
	int16 lock;
	bool onoff;

	ComplexSprite() : visible(false), current(0), xloc(0), yloc(0), acurrent(0), axloc(0), ayloc(0), type(0), frozen(0), locked(0), link(0), key(0), lock(0), onoff(true) {}
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
	int checkwall(int xnew, int ynew, Locate *pobject);
	void cCommand(int xnew, int ynew, bool allowInteraction);
	void scrollInfo();

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

	Renderer *_gfx;

	int _tsin, _tcos;
	int _sint[256];
	int _cost[256];
	int _centerX, _centerY;
	int _width, _height;
	int _mouseSensitivity;
	bool _showDashBoard;
	bool _crosshair;
	bool _insight;
	bool _hasKeycard;
	bool _unlocked;
	int _weapons;
	bool _wireframe;
	bool _widescreen;

	int _frntxWall, _frntyWall;
	int _sidexWall, _sideyWall;
	int _frntx, _frnty;
	int _sidex, _sidey;
	int _front, _side;
	int _direction;

	Common::Rect _clip;
	Common::Rect _screenR;
	Common::Rect _dashBoardRect;
	Common::Rect _compassRect;
	Common::Rect _headsUpRect;
	Common::Rect _powerRect;

	uint8 wallAt(int x, int y) const;
	const uint8 *mapFeatureAt(int x, int y, int direction) const;
	void drawStaticObjects();
	uint32 objectColor(int type) const;

public:
	struct PrismPartDef {
		int pointCount;
		const int (*points)[3];
		int surfaceCount;
		const int (*surfaces)[8];
	};

private:
	void draw3DPrism(const Thing &obj, const PrismPartDef &def, bool useLook, uint32 color);
	bool drawStaticObjectPrisms3D(const Thing &obj, uint32 baseColor);
	void renderCorridor3D();
	void drawWallFeatures3D();
	void drawWallFeature3D(int cellX, int cellY, int direction);
	void getWallFace3D(int cellX, int cellY, int direction, float corners[4][3]);

	int occupiedObjectAt(int x, int y, const Locate *pobject);
	void interactWithObject(int objNum);
	bool setDoorState(int x, int y, int direction, int state);
	int openAdjacentDoors(int x, int y);
	bool tryPassThroughFeature(int fromX, int fromY, int direction, Locate *pobject);
	void updateViewportLayout();
	void drawDashboardStep1();
	void drawCrosshair();
	bool clipLineToRect(int &x1, int &y1, int &x2, int &y2, const Common::Rect &clip) const;

	// Animation system
	Common::Array<Sprite *> _cSprites;
	Common::Array<ComplexSprite *> _lSprites;
	Image *_backgroundMask;
	Image *_backgroundFG;
	Common::Rect _backgroundClip;
	Common::Rect _backgroundLocate;
	bool _backgroundActive;
	byte _topBG[8];
	byte _bottomBG[8];
	int16 _divideBG;

	bool loadAnimation(const Common::String &name);
	void deleteAnimation();
	void playAnimation();
	void drawAnimation();
	void drawComplexSprite(int index, int ox, int oy);
	void drawAnimationImage(Image *img, Image *mask, int x, int y);
	Image *loadImage(Common::SeekableReadStream &file);
	void unpackBytes(Common::SeekableReadStream &file, byte *dst, uint32 len);
	Common::Rect readRect(Common::SeekableReadStream &file);
	int whichSprite(const Common::Point &p);
	void handleAnimationClick(int item);
};

} // End of namespace Colony

#endif // COLONY_COLONY_H
