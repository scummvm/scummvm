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
#include "common/random.h"
#include "common/rect.h"
#include "graphics/framelimiter.h"
#include "common/rendermode.h"
#include "colony/renderer.h"
#include "colony/sound.h"
#include "graphics/cursor.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"


namespace Colony {

enum ColonyAction {
	kActionNone,
	kActionMoveForward,
	kActionMoveBackward,
	kActionStrafeLeft,
	kActionStrafeRight,
	kActionRotateLeft,
	kActionRotateRight,
	kActionLookLeft,
	kActionLookRight,
	kActionLookBehind,
	kActionToggleMouselook,
	kActionToggleDashboard,
	kActionToggleWireframe,
	kActionToggleFullscreen,
	kActionSkipIntro,
	kActionEscape,
	kActionFire
};

enum GameMode {
	kModeColony = 2,
	kModeBattle = 1
};

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
	kDirSouth = 3,
	kDirCenter = 4
};

enum RobotType {
	kRobEye = 1,
	kRobPyramid = 2,
	kRobCube = 3,
	kRobUPyramid = 4,
	kRobFEye = 5,
	kRobFPyramid = 6,
	kRobFCube = 7,
	kRobFUPyramid = 8,
	kRobSEye = 9,
	kRobSPyramid = 10,
	kRobSCube = 11,
	kRobSUPyramid = 12,
	kRobMEye = 13,
	kRobMPyramid = 14,
	kRobMCube = 15,
	kRobMUPyramid = 16,
	kRobQueen = 17,
	kRobDrone = 18,
	kRobSoldier = 19,
	kRobSnoop = 20
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
	kObjBench = 41,
	kObjPToilet = 43,
	kObjCBench = 44,
	kObjProjector = 45,
	kObjReactor = 46,
	kObjFWall = 48,
	kObjCWall = 49,
	kObjBBed = 42
};

enum ObjColor {
	kColorClear = 0,
	kColorBlack = 1,
	kColorDkGray = 9,
	kColorLtGreen = 11,
	kColorBath = 17,
	kColorWater = 18,
	kColorSilver = 19,
	kColorReactor = 20,
	kColorBlanket = 21,
	kColorSheet = 22,
	kColorBed = 23,
	kColorBox = 24,
	kColorBench = 25,
	kColorChair = 26,
	kColorChairBase = 27,
	kColorCouch = 28,
	kColorConsole = 29,
	kColorTV = 30,
	kColorTVScreen = 31,
	kColorDrawer = 32,
	kColorDesk = 37,
	kColorDeskTop = 38,
	kColorDeskChair = 39,
	kColorMac = 40,
	kColorMacScreen = 41,
	kColorCryo = 33,
	kColorCryoGlass = 34,
	kColorCryoBase = 35,
	kColorForklift = 49,
	kColorTread1 = 50,
	kColorTread2 = 51,
	kColorPot = 52,
	kColorPlant = 53,
	kColorPower = 54,
	kColorPBase = 55,
	kColorPSource = 56,
	kColorTable = 61,
	kColorTableBase = 62,
	kColorPStand = 63,
	kColorPLens = 64,
	kColorProjector = 65,
	kColorTele = 66,
	kColorTeleDoor = 67,
	kColorWall = 77,
	kColorRainbow1 = 80,
	kColorRainbow2 = 81,
	kColorRainbow3 = 82,
	kColorRainbow4 = 83,
	// Robot colors
	kColorCube = 36,
	kColorDrone = 42,
	kColorClaw1 = 43,
	kColorClaw2 = 44,
	kColorEyes = 45,
	kColorEye = 46,
	kColorIris = 47,
	kColorPupil = 48,
	kColorPyramid = 57,
	kColorQueen = 58,
	kColorTopSnoop = 59,
	kColorBottomSnoop = 60,
	kColorUPyramid = 68,
	kColorShadow = 74,
	// Animated reactor/power suit colors (Mac: c_hcore1..c_hcore4, c_ccore, c_color0..c_color3)
	kColorHCore1 = 100,
	kColorHCore2 = 101,
	kColorHCore3 = 102,
	kColorHCore4 = 103,
	kColorCCore = 104,
	// Semantic robot colors that need platform- or level-specific mapping.
	kColorEyeball = 105,
	kColorEyeIris = 106,
	kColorMiniEyeIris = 107,
	kColorDroneEye = 108,
	kColorSoldierBody = 109,
	kColorSoldierEye = 110,
	kColorQueenBody = 111,
	kColorQueenEye = 112,
	kColorQueenWingRed = 113
};

enum {
	kColonyDebugMove = 1 << 0,
	kColonyDebugRender = 1 << 1,
	kColonyDebugAnimation = 1 << 2,
	kColonyDebugMap = 1 << 3,
	kColonyDebugSound = 1 << 4,
	kColonyDebugUI = 1 << 5,
	kColonyDebugCombat = 1 << 6,
};

// Mac menu action IDs (matching original Mac Colony menu structure)
enum MenuAction {
	kMenuActionAbout = 1,
	kMenuActionNew,
	kMenuActionOpen,
	kMenuActionSave,
	kMenuActionSaveAs,
	kMenuActionQuit,
	kMenuActionSound,
	kMenuActionCrosshair,
	kMenuActionPolyFill,
	kMenuActionCursorShoot
};

static const int kBaseObject = 20;
static const int kMeNum = 101;

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

// PATCH.C: Tracks object relocations across levels (forklift carry/drop).
struct PatchEntry {
	struct { uint8 level, xindex, yindex; } from;
	struct { uint8 level, xindex, yindex; int xloc, yloc; uint8 ang; } to;
	uint8 type;
	uint8 mapdata[5];
};

// Temporary location reference for carrying objects.
struct PassPatch {
	uint8 level, xindex, yindex;
	int xloc, yloc;
	uint8 ang;
};

// Per-level persistence: wall state changes (airlock locks) and visit flags.
struct LevelData {
	uint8 visit;
	uint8 queen;
	uint8 object[kBaseObject + 1];
	uint8 count;           // airlock open count (termination check)
	uint8 size;            // number of saved wall changes (max 10)
	uint8 location[10][3]; // [x, y, direction] of each changed wall
	uint8 data[10][5];     // saved wall feature bytes (5 per location)
};

struct MacColor {
	uint16 fg[3];
	uint16 bg[3];
	uint16 pattern;
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
	~Image() { delete[] data; }
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

class Debugger;

class ColonyEngine : public Engine {
	friend class Debugger;
public:
	ColonyEngine(OSystem *syst, const ADGameDescription *gd);
	virtual ~ColonyEngine();

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	void pauseEngineIntern(bool pause) override;
	Common::Platform getPlatform() const { return _gameDescription->platform; }
	bool isSoundEnabled() const { return _soundOn; }
	const Graphics::Surface *getSavedScreen() const { return _savedScreen; }

	void initTrig();
	void loadMacColors();
	void loadMap(int mnum);
	void startNewGame();
	void corridor();
	void quadrant();
	bool hasInteractiveWallFeature(int cx, int cy, int dir) const;
	void clampToWalls(Locate *p);
	int checkwallMoveTo(int xnew, int ynew, int xind2, int yind2, Locate *pobject, uint8 trailCode);
	int checkwallTryFeature(int xnew, int ynew, int xind2, int yind2, Locate *pobject, int dir);
	int checkwall(int xnew, int ynew, Locate *pobject);
	void cCommand(int xnew, int ynew, bool allowInteraction);
	bool scrollInfo(const Graphics::Font *macFont = nullptr);
	bool checkSkipRequested();
	bool waitForInput();
	void checkCenter();
	void fallThroughHole();
	void playTunnelEffect(bool falling);

	void doText(int entry, int center);
	void inform(const char *text, bool hold);
	void printMessage(const char *text[], bool hold);
	void makeMessageRect(Common::Rect &r);
	int runMacEndgameDialog(const Common::String &message);

private:
	const ADGameDescription *_gameDescription;

	uint8 _wall[32][32];
	uint8 _mapData[31][31][5][5];
	uint8 _robotArray[32][32];
	uint8 _foodArray[32][32];
	uint8 _dirXY[32][32];

	Locate _me;
	Common::Array<Thing> _objects;
	int _level;
	int _robotNum;
	int _dynamicObjectBase = 0;

	Renderer *_gfx;
	Sound *_sound;
	Graphics::FrameLimiter *_frameLimiter;
	Common::RenderMode _renderMode;
	Graphics::Surface *_savedScreen = nullptr;


	int _tsin, _tcos;
	int _sint[256];
	int _cost[256];
	int _centerX, _centerY;
	int _width, _height;
	int _mouseSensitivity;
	bool _mouseLocked;
	bool _soundOn = true;
	bool _showDashBoard;
	bool _crosshair;
	bool _cursorShoot = false;
	bool _insight;
	bool _hasKeycard;
	bool _unlocked;
	int _weapons;
	bool _wireframe;
	bool _widescreen;
	bool _fullscreen;
	int _speedShift; // 1-5, movement speed = 1 << (_speedShift - 1)

	// Continuous movement flags (set/cleared by keymapper action events)
	bool _moveForward;
	bool _moveBackward;
	bool _strafeLeft;
	bool _strafeRight;
	bool _rotateLeft;
	bool _rotateRight;

	Common::RandomSource _randomSource;
	Common::Point _mousePos;
	uint8 _decode1[4];
	uint8 _decode2[4];
	uint8 _decode3[4];
	uint8 _animDisplay[6];
	int _coreState[2];
	int _coreHeight[2];
	int _corePower[3];
	int _epower[3];   // log2 display levels for power bars (from qlog)
	int _coreIndex;
	int _orbit = 0;
	int _armor = 0;
	bool _gametest = false;
	uint32 _blackoutColor = 0;
	uint32 _lastClickTime = 0;
	uint32 _displayCount = 0; // Frame counter for COLOR wall animation (Mac: count)
	int _foodCount = 32;      // Mac display.c: periodic power drain counter (FCOUNT=32)
	uint32 _lastHotfootTime = 0;  // Time-gate for HOTFOOT damage (~8fps)
	uint32 _lastAnimUpdate = 0;
	uint32 _lastWarningChimeTime = 0;
	int _action0, _action1;
	int _creature;
	bool _allGrow = false;
	bool _suppressCollisionSound = false;

	// Battle state (battle.c)
	int _gameMode = kModeColony;
	Locate _bfight[16];           // 16 battle enemies
	Locate _battleEnter;          // entrance structure
	Locate _battleShip;           // shuttle
	Locate _battleProj;           // enemy projectile
	bool _projon = false;         // projectile active
	int _pcount = 0;              // projectile countdown
	int _mountains[256];          // mountain height profile
	int _battledx = 0;            // mountain parallax divisor (Width/59)
	int _battleRound = 0;         // AI round-robin counter
	Locate *_battlePwh[100];      // visible object pointers (for hit detection)
	int _battleMaxP = 0;          // count of visible objects
	Locate _pyramids[4][4][15];   // pyramid obstacles: 4x4 quadrants, 15 each

	// PATCH.C: object relocation + wall state persistence
	Common::Array<PatchEntry> _patches;
	PassPatch _carryPatch[2];   // [0]=forklift, [1]=carried object
	int _carryType;             // type of object being carried
	int _fl;                    // 0=not in forklift, 1=in forklift empty, 2=carrying object
	LevelData _levelData[8];   // per-level wall state persistence

	MacColor _macColors[145];
	bool _hasMacColors;
	Graphics::Cursor *_macCrossCursor = nullptr;
	Graphics::Cursor *_macArrowCursor = nullptr;
	int _lastLoggedCursorMode = -1;

	// Mac menu bar (MacWindowManager overlay)
	Graphics::MacWindowManager *_wm;
	Graphics::MacMenu *_macMenu;
	Graphics::ManagedSurface *_menuSurface;
	int _menuBarHeight;
	void initMacMenus();
	void loadMacCursorResources();
	void handleMenuAction(int action);
	static void menuCommandsCallback(int action, Common::String &text, void *data);

	int _frntxWall, _frntyWall;
	int _sidexWall, _sideyWall;
	int _frntx, _frnty;
	int _sidex, _sidey;
	int _front, _side;
	int _direction;

	Common::Rect _clip;
	Common::Rect _screenR;
	Common::Rect _dashBoardRect;
	Common::Rect _compassRect;   // DOS: compOval (after shrink); Mac: moveWindow
	Common::Rect _headsUpRect;   // DOS: floorRect; Mac: minimap inside moveWindow
	Common::Rect _powerRect;     // DOS: powerRect; Mac: infoWindow

	// DOS dashboard layout (from original MetaWINDOW pix_per_Qinch values)
	int _pQx;           // pixels per quarter-inch X (24 for EGA 640x350)
	int _pQy;           // pixels per quarter-inch Y (18 for EGA 640x350)
	int _powerWidth;     // width of each of the 3 power bar columns
	int _powerHeight;    // pixel height per power bar unit (max 5)

	// Cached decoded PICT surfaces for dashboard panels (Mac color mode)
	Graphics::Surface *_pictPower = nullptr;      // PICT -32755 (normal) or -32760 (trouble)
	Graphics::Surface *_pictPowerNoArmor = nullptr; // PICT -32761 (no armor, color)
	Graphics::Surface *_pictCompass = nullptr;     // PICT -32757
	int _pictPowerID = 0;   // Track which PICT is cached
	Graphics::Surface *loadPictSurface(int resID);
	void drawPictAt(Graphics::Surface *surf, int destX, int destY);

	uint8 wallAt(int x, int y) const;
	const uint8 *mapFeatureAt(int x, int y, int direction) const;
	bool _visibleCell[32][32];
	void computeVisibleCells();
	void drawStaticObjects();

public:
	struct PrismPartDef {
		int pointCount;
		const int (*points)[3];
		int surfaceCount;
		const int (*surfaces)[8];
	};

private:
	void draw3DPrism(Thing &obj, const PrismPartDef &def, bool useLook, int colorOverride = -1, bool accumulateBounds = false, bool forceVisible = false);
	void draw3DLeaf(const Thing &obj, const PrismPartDef &def);
	void draw3DSphere(Thing &obj, int pt0x, int pt0y, int pt0z,
	                  int pt1x, int pt1y, int pt1z, uint32 fillColor, uint32 outlineColor, bool accumulateBounds = false);
	void drawPrismOval3D(Thing &thing, const PrismPartDef &def, bool useLook, int colorOverride);
	bool drawStaticObjectPrisms3D(Thing &obj);
	void initRobots();
	void renderCorridor3D();
	void drawWallFeatures3D();
	void drawWallFeature3D(int cellX, int cellY, int direction);
	void drawCellFeature3D(int cellX, int cellY);
	void getWallFace3D(int cellX, int cellY, int direction, float corners[4][3]);
	void getCellFace3D(int cellX, int cellY, bool ceiling, float corners[4][3]);

	int occupiedObjectAt(int x, int y, const Locate *pobject);
	void interactWithObject(int objNum);

	// shoot.c: shooting and power management
	void setPower(int p0, int p1, int p2);
	void cShoot();
	void destroyRobot(int num);
	void doShootCircles(int cx, int cy);
	void doBurnHole(int cx, int cy, int radius);
	void meGetShot();

	// battle.c: outdoor battle system (OpenGL 3D)
	void battleInit();
	void battleSet();
	void battleThink();
	void enterColonyFromBattle(int mapNum, int xloc, int yloc);
	void battleCommand(int xnew, int ynew);
	void battleShoot();
	void battleProjCommand(int xcheck, int ycheck);
	void renderBattle();
	void draw3DBattlePrism(const PrismPartDef &def, int worldX, int worldY, uint8 ang, int zShift = 0);
	void battleBackdrop();
	void battleDrawPyramids();
	void battleDrawTanks();

	// PATCH.C: object relocation + wall state persistence
	void resetObjectSlot(int slot, int type, int xloc, int yloc, uint8 ang);
	bool createObject(int type, int xloc, int yloc, uint8 ang);
	void saveLevelState();
	void doPatch();
	void saveWall(int x, int y, int direction);
	void getWall();
	void newPatch(int type, const PassPatch &from, const PassPatch &to, const uint8 *mapdata);
	bool patchMapTo(const PassPatch &to, uint8 *mapdata);
	bool patchMapFrom(const PassPatch &from, uint8 *mapdata);
	void exitForklift();
	void dropCarriedObject();
	bool setDoorState(int x, int y, int direction, int state);
	int openAdjacentDoors(int x, int y);
	int goToDestination(const uint8 *map, Locate *pobject);
	int tryPassThroughFeature(int fromX, int fromY, int direction, Locate *pobject);
	void playTunnelAirlockEffect();
	void syncMacMenuChecks();
	void updateMouseCapture(bool recenter = true);
	Common::Point getAimPoint() const;
	void updateViewportLayout();
	void drawDashboardStep1();
	void drawDashboardMac();
	void drawDOSBarGraph(int x, int y, int height);
	void updateDOSPowerBars();
	static int qlog(int32 x);
	void drawMiniMapMarker(int x, int y, int halfSize, uint32 color, bool isMac);
	bool hasRobotAt(int x, int y) const;
	bool hasFoodAt(int x, int y) const;
	void drawMiniMap(uint32 lineColor);
	void drawCrosshair();
	bool clipLineToRect(int &x1, int &y1, int &x2, int &y2, const Common::Rect &clip) const;
	void wallLine(const float corners[4][3], float u1, float v1, float u2, float v2, uint32 color);
	void wallPolygon(const float corners[4][3], const float *u, const float *v, int count, uint32 color);
	void wallChar(const float corners[4][3], uint8 cnum);

	struct TextIndex {
		uint32 offset;
		uint16 ch;
		uint16 lines;
	};

	// Animation system
	Common::Array<Sprite *> _cSprites;
	Common::Array<ComplexSprite *> _lSprites;
	Image *_backgroundMask;
	Image *_backgroundFG;
	Common::Rect _backgroundClip;
	Common::Rect _backgroundLocate;
	bool _backgroundActive;
	Common::MacResManager *_resMan;
	Common::MacResManager *_colorResMan;
	byte _topBG[8];
	byte _bottomBG[8];
	int16 _divideBG;
	Common::String _animationName;
	Common::Array<int16> _animBMColors;
	bool _animationRunning;
	int _animationResult;
	bool _doorOpen;
	int _elevatorFloor;
	int _airlockX = -1;
	int _airlockY = -1;
	int _airlockDirection = -1;
	bool _airlockTerminate = false;

	void playIntro();
	bool makeStars(const Common::Rect &r, int btn);
	bool makeBlackHole();
	bool timeSquare(const Common::String &str, const Graphics::Font *macFont = nullptr);
	bool drawPict(int resID);
	bool loadAnimation(const Common::String &name);
	void deleteAnimation();
	void takeOff();
	void gameOver(bool kill);
	int countSavedCryos() const;
	void playAnimation();
	void updateAnimation();
	void drawAnimation();
	void drawComplexSprite(int index, int ox, int oy);
	void drawAnimationImage(Image *img, Image *mask, int x, int y, uint32 fillColor = 0xFFFFFFFF);
	uint32 resolveAnimColor(int16 bmEntry) const;
	Image *loadImage(Common::SeekableReadStream &file);
	void unpackBytes(Common::SeekableReadStream &file, byte *dst, uint32 len);
	Common::Rect readRect(Common::SeekableReadStream &file);
	int16 readSint16(Common::SeekableReadStream &s);
	uint16 readUint16(Common::SeekableReadStream &s);
	uint32 readUint32(Common::SeekableReadStream &s);
	int whichSprite(const Common::Point &p);
	void handleAnimationClick(int item);
	void handleDeskClick(int item);
	void handleVanityClick(int item);
	void handleSlidesClick(int item);
	void handleTeleshowClick(int item);
	void handleKeypadClick(int item);
	void handleSuitClick(int item);
	void handleDoorClick(int item);
	void handleAirlockClick(int item);
	void handleElevatorClick(int item);
	void handleControlsClick(int item);
	void dolSprite(int index);
	void moveObject(int index);
	void setObjectState(int num, int state);
	int objectState(int num) const;
	void setObjectOnOff(int num, bool on);
	void refreshAnimationDisplay();
	void crypt(uint8 sarray[6], int i, int j, int k, int l);
	void terminateGame(bool blowup);

	// think.c / shoot.c: colony robot AI, egg growth, and egg eating
	void cThink();
	void cubeThink(int num);
	void pyramidThink(int num);
	void upyramidThink(int num);
	void eyeThink(int num);
	void queenThink(int num);
	void droneThink(int num);
	void snoopThink(int num);
	void eggThink(int num);
	int getColonyActiveRobotLimit() const;
	void copyOverflowObjectToSlot(int num);
	bool layEgg(int type, int xindex, int yindex);
	void moveThink(int num);
	void bigGrow(int num);
	void growRobot(int num);
	int scanForPlayer(int num);
	void robotShoot(int num);
	void meEat();
	void respawnObject(int num, int type);
};

} // End of namespace Colony

#endif // COLONY_COLONY_H
