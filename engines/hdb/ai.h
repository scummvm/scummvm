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

#ifndef HDB_AI_H
#define HDB_AI_H

#define STARS_MONKEYSTONE_7			0xfe257d	// magic value in the config file for the unlocking of the Monkeystone secret #7
#define STARS_MONKEYSTONE_7_FAKE	0x11887e	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_14		0x3341fe	// <same> for the Monkeystone #14
#define STARS_MONKEYSTONE_14_FAKE	0x1cefd0	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_21		0x77ace3	// <same> for the Monkeystone #21
#define STARS_MONKEYSTONE_21_FAKE	0x3548fe	// fake value that means it hasn't been unlocked

namespace HDB {

enum {
	kMaxAnimFrames = 8,
	kMaxAnimTFrames = 16,
	kMaxDeathFrames = 12,
	kMaxLevel2Ents = 60,
	kMaxInventory = 10,
	kMaxDeliveries = 5,
	kMaxWaypoints = 10,
	kMaxActions = 20,
	kMaxTeleporters = 20,
	kMaxAutoActions = 30,
	kMaxLuaEnts = 50,
	kMaxCallbacks = 20,
	kMaxFairystones = 5,
	kMaxGatePuddles = 8,
	kMaxBridges = 10,
	kDelay5Seconds = 5 * kGameFPS,
	kPlayerMoveSpeed = 4,
	kEnemyMoveSpeed = 2,
	kPushMoveSpeed = (kPlayerMoveSpeed >> 1),
	kPlayerTouchPWait = 16,
	kMaxCineGfx = 10,
	kRunToggleDelay = 2,
	kMsgDelay = 3,
	kYouGotX = -1,
	kNumSaveSlots = 8,
	kAutoSaveSlot = 0
};

enum AIType {
	AI_NONE,
	AI_GUY,
	AI_DOLLY,
	AI_SPACEDUDE,
	AI_SERGEANT,
	AI_SCIENTIST,
	AI_WORKER,
	AI_DEADWORKER,
	AI_ACCOUNTANT,
	AI_RAILRIDER,
	AI_RAILRIDER_ON,
	AI_VORTEXIAN,
	AI_CHICKEN,
	AI_GEM_ATTACK,
	AI_SLUG_ATTACK,

	AI_LASER,
	AI_LASERBEAM,
	AI_DIVERTER,
	AI_FOURFIRER,
	AI_OMNIBOT,
	AI_TURNBOT,
	AI_SHOCKBOT,
	AI_RIGHTBOT,
	AI_PUSHBOT,
	AI_LISTENBOT,
	AI_MAINTBOT,
	AI_OMNIBOT_MISSILE,
	AI_DEADEYE,
	AI_MEERKAT,
	AI_FATFROG,
	AI_GOODFAIRY,
	AI_BADFAIRY,
	AI_ICEPUFF,
	AI_BUZZFLY,
	AI_DRAGON,
	AI_GATEPUDDLE,

	AI_CRATE,
	AI_LIGHTBARREL,
	AI_HEAVYBARREL,
	AI_BOOMBARREL,
	AI_FROGSTATUE,

	AI_MAGIC_EGG,
	AI_ICE_BLOCK,

	ITEM_CELL,
	ITEM_ENV_WHITE,
	ITEM_ENV_RED,
	ITEM_ENV_BLUE,
	ITEM_ENV_GREEN,
	ITEM_TRANSCEIVER,
	ITEM_CLUB,
	ITEM_ROBOSTUNNER,
	ITEM_SLUGSLINGER,
	ITEM_MONKEYSTONE,
	ITEM_GEM_WHITE,
	ITEM_GEM_BLUE,
	ITEM_GEM_RED,
	ITEM_GEM_GREEN,
	ITEM_GOO_CUP,
	ITEM_TEACUP,
	ITEM_COOKIE,
	ITEM_BURGER,
	ITEM_PDA,
	ITEM_BOOK,
	ITEM_CLIPBOARD,
	ITEM_NOTE,
	ITEM_KEYCARD_WHITE,
	ITEM_KEYCARD_BLUE,
	ITEM_KEYCARD_RED,
	ITEM_KEYCARD_GREEN,
	ITEM_KEYCARD_PURPLE,
	ITEM_KEYCARD_BLACK,
	ITEM_CABKEY,
	ITEM_DOLLYTOOL1,
	ITEM_DOLLYTOOL2,
	ITEM_DOLLYTOOL3,
	ITEM_DOLLYTOOL4,
	ITEM_SEED,
	ITEM_SODA,
	ITEM_ROUTER,
	ITEM_SLICER,
	ITEM_CHICKEN,
	ITEM_PACKAGE,

	INFO_FAIRY_SRC,
	INFO_FAIRY_SRC2,
	INFO_FAIRY_SRC3,
	INFO_FAIRY_SRC4,
	INFO_FAIRY_SRC5,
	INFO_FAIRY_DEST,
	INFO_FAIRY_DEST2,
	INFO_FAIRY_DEST3,
	INFO_FAIRY_DEST4,
	INFO_FAIRY_DEST5,
	INFO_TRIGGER,
	INFO_SET_MUSIC,
	INFO_PROMOTE,
	INFO_DEMOTE,
	INFO_LUA,
	INFO_HERE,
	INFO_ARROW_TURN,
	INFO_ARROW_STOP,
	INFO_ARROW_4WAY,
	INFO_TELEPORTER1,
	INFO_TELEPORTER2,
	INFO_TELEPORTER3,
	INFO_TELEPORTER4,
	INFO_TELEPORTER5,
	INFO_TELEPORTER6,
	INFO_TELEPORTER7,
	INFO_TELEPORTER8,
	INFO_TELEPORTER9,
	INFO_TELEPORTER10,
	INFO_TELEPORTER11,
	INFO_TELEPORTER12,
	INFO_TELEPORTER13,
	INFO_TELEPORTER14,
	INFO_TELEPORTER15,
	INFO_TELEPORTER16,
	INFO_TELEPORTER17,
	INFO_TELEPORTER18,
	INFO_TELEPORTER19,
	INFO_TELEPORTER20,
	INFO_LEVELEXIT,
	INFO_ACTION1,
	INFO_ACTION2,
	INFO_ACTION3,
	INFO_ACTION4,
	INFO_ACTION5,
	INFO_ACTION6,
	INFO_ACTION7,
	INFO_ACTION8,
	INFO_ACTION9,
	INFO_ACTION10,
	INFO_ACTION11,
	INFO_ACTION12,
	INFO_ACTION13,
	INFO_ACTION14,
	INFO_ACTION15,
	INFO_ACTION16,
	INFO_ACTION17,
	INFO_ACTION18,
	INFO_ACTION19,
	INFO_ACTION20,
	INFO_ACTION_AUTO,

	INFO_QMARK,
	INFO_DEBUG,
	END_AI_TYPES

};

enum AIDir {
	DIR_NONE,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
};

enum Death {
	DEATH_NORMAL,
	DEATH_FRIED,
	DEATH_DROWNED,
	DEATH_GRABBED,
	DEATH_SHOCKED,
	DEATH_PANICZONE,
	DEATH_PLUMMET
};

enum AIState {
	STATE_NONE,
	STATE_STANDDOWN,
	STATE_STANDUP,
	STATE_STANDLEFT,
	STATE_STANDRIGHT,
	STATE_BLINK,
	STATE_MOVEUP,
	STATE_MOVEDOWN,
	STATE_MOVELEFT,
	STATE_MOVERIGHT,
	STATE_DYING,
	STATE_DEAD,
	STATE_HORRIBLE1,
	STATE_HORRIBLE2,
	STATE_HORRIBLE3,
	STATE_HORRIBLE4,
	STATE_GOODJOB,
	STATE_PLUMMET,

	STATE_PUSHUP,				// these are only used for the player
	STATE_PUSHDOWN,
	STATE_PUSHLEFT,
	STATE_PUSHRIGHT,
	STATE_GRABUP,				// player grabbing something
	STATE_GRABDOWN,
	STATE_GRABLEFT,
	STATE_GRABRIGHT,
	STATE_ATK_CLUB_UP,			// player attacking frames
	STATE_ATK_CLUB_DOWN,
	STATE_ATK_CLUB_LEFT,
	STATE_ATK_CLUB_RIGHT,
	STATE_ATK_STUN_DOWN,
	STATE_ATK_STUN_UP,
	STATE_ATK_STUN_LEFT,
	STATE_ATK_STUN_RIGHT,
	STATE_ATK_SLUG_DOWN,
	STATE_ATK_SLUG_UP,
	STATE_ATK_SLUG_LEFT,
	STATE_ATK_SLUG_RIGHT,

	STATE_FLOATING,				// floating in stuff (can walk on)
	STATE_FLOATDOWN,
	STATE_FLOATUP,
	STATE_FLOATLEFT,
	STATE_FLOATRIGHT,
	STATE_MELTED,				// melted into slag (can walk on)
	STATE_SLIDING,				// sliding across a floor
	STATE_SHOCKING,				// for Shockbot floor-shock anim
	STATE_EXPLODING,			// boom barrel explosion!

	STATE_USEDOWN,				// crazy maintenance bot!
	STATE_USEUP,
	STATE_USELEFT,
	STATE_USERIGHT,

	STATE_MEER_MOVE,			// for the Meerkat
	STATE_MEER_APPEAR,
	STATE_MEER_BITE,
	STATE_MEER_DISAPPEAR,
	STATE_MEER_LOOK,

	STATE_ICEP_PEEK,			// for the Icepuff
	STATE_ICEP_APPEAR,
	STATE_ICEP_THROWDOWN,
	STATE_ICEP_THROWRIGHT,
	STATE_ICEP_THROWLEFT,
	STATE_ICEP_DISAPPEAR,

	STATE_LICKDOWN,				// for the Fatfrog
	STATE_LICKLEFT,
	STATE_LICKRIGHT,

	STATE_DIVERTER_BL,			// for Diverters
	STATE_DIVERTER_BR,
	STATE_DIVERTER_TL,
	STATE_DIVERTER_TR,

	STATE_KISSRIGHT,			// for Dolly
	STATE_KISSLEFT,
	STATE_ANGRY,
	STATE_PANIC,
	STATE_LAUGH,
	STATE_DOLLYUSERIGHT,

	STATE_YELL,					// for Sarge

	STATE_ENDSTATES
};

enum AnimSpeed {
	ANIM_SLOW,
	ANIM_NORMAL,
	ANIM_FAST
};

enum CineType {
	C_NO_COMMAND,
	C_STOPCINE,
	C_LOCKPLAYER,
	C_UNLOCKPLAYER,
	C_SETCAMERA,
	C_MOVECAMERA,
	C_WAIT,
	C_WAITUNTILDONE,
	C_MOVEENTITY,
	C_DIALOG,
	C_ANIMENTITY,
	C_RESETCAMERA,
	C_SETENTITY,
	C_STARTMAP,
	C_MOVEPIC,
	C_MOVEMASKEDPIC,
	C_DRAWPIC,
	C_DRAWMASKEDPIC,
	C_FADEIN,
	C_FADEOUT,
	C_SPAWNENTITY,
	C_PLAYSOUND,
	C_CLEAR_FG,
	C_SET_FG,
	C_SET_BG,
	C_FUNCTION,
	C_ENTITYFACE,
	C_USEENTITY,
	C_REMOVEENTITY,
	C_SETANIMFRAME,
	C_TEXTOUT,
	C_CENTERTEXTOUT,
	C_PLAYVOICE,

	C_ENDLIST
};

enum CallbackType {
	NO_FUNCTION,
	AI_BARREL_EXPLOSION_END,
	CALLBACK_DOOR_OPEN_CLOSE,
	CALLBACK_AUTODOOR_OPEN_CLOSE,

	CALLBACK_END
};

struct AIStateDef {
	AIState state;
	const char *name;
};

struct AIEntity {
	AIType type;
	AIState state;
	AIDir dir;

	Tile *draw;											// Current frame to draw

	void (*aiInit)(AIEntity *e);						// func ptr to init routine
	void (*aiInit2)(AIEntity *e);						// func ptr to init2 routine - graphic init only (this for LoadGame functionality)
	void (*aiAction)(AIEntity *e);						// func ptr to action routine
	void (*aiUse)(AIEntity *e);							// func ptr to use routine
	void (*aiDraw)(AIEntity *e, int x, int y);			// func ptr to extra drawing routine (only for special stuff) - pass in mapx, mapy

	char		luaFuncInit[32];						// Lua function for Init (always called after entity's init). These are ptrs into the map header.
	char		luaFuncAction[32];						// Lua function for Action
	char		luaFuncUse[32];							// Lua function for Use
	int16		level;									// which floor level we're on
	int16		value1, value2;							// extra values we might need
	AIDir		dir2;									// this is from TED

	int16		x, y;
	int16		drawXOff, drawYOff;					// might need a drawing offset
	int16		onScreen;								// FLAG: is this entity onscreen?
	int16		moveSpeed;								// movement speed of this entity
	int16		xVel, yVel;								// movement values
	int16		tileX, tileY;
	int16		goalX, goalY;							// where we're trying to go - TILE COORDS
	int16		touchpX, touchpY, touchpTile, touchpWait;		// ACTION index a touchplate is using, which you're on
	int32		stunnedWait;							// if we're stunned, this is the delay before being normal again
	int16		sequence;								// to use for specially-coded sequences
	char		entityName[32];						// the name of the entity, as registered by the Lua init function for the entity
	char		printedName[32];						// the name of the entity/item, the way it should be printed

	int16		animFrame;								// which frame we're on
	int16		animDelay;								// changes every frame; based on anim_cycle at start
	int16		animCycle;								// delay between frame animations

	union {
		uint16 blinkFrames;
		int16 int1;
	};
	Tile *blinkGfx[kMaxAnimFrames];

	union {
		uint16 special1Frames;
		int16 int2;
	};
	Tile *special1Gfx[kMaxAnimFrames];

	int16		standdownFrames;
	Tile		*standdownGfx[kMaxAnimFrames];

	int16		standupFrames;
	Tile		*standupGfx[kMaxAnimFrames];

	int16		standleftFrames;
	Tile		*standleftGfx[kMaxAnimFrames];

	int16		standrightFrames;
	Tile		*standrightGfx[kMaxAnimFrames];

	int16		moveupFrames;
	Tile		*moveupGfx[kMaxAnimFrames];

	int16		movedownFrames;
	Tile		*movedownGfx[kMaxAnimFrames];

	int16		moveleftFrames;
	Tile		*moveleftGfx[kMaxAnimFrames];

	int16		moverightFrames;
	Tile		*moverightGfx[kMaxAnimFrames];

	void reset() {
		luaFuncInit[0] = 0;
		luaFuncAction[0] = 0;
		luaFuncUse[0] = 0;
		entityName[0] = 0;
		printedName[0] = 0;

		type = AI_NONE;
		state = STATE_NONE;
		dir = DIR_NONE;

		draw = nullptr;

		aiInit = aiInit2 = nullptr;
		aiAction = nullptr;
		aiUse = nullptr;
		aiDraw = nullptr;

		level = 0;
		value1 = value2 = 0;
		dir2 = DIR_NONE;

		x = y = 0;
		drawXOff = drawYOff = 0;
		onScreen = 0;
		moveSpeed = 0;
		xVel = yVel = 0;
		tileX = tileY = 0;
		goalX = goalY = 0;
		touchpX = touchpY = touchpTile = touchpWait = 0;
		stunnedWait = 0;
		sequence = 0;

		animCycle = 0;
		animDelay = 0;
		animFrame = 0;

		blinkFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			blinkGfx[i] = nullptr;
		}

		special1Frames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			special1Gfx[i] = nullptr;
		}

		standdownFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			standdownGfx[i] = nullptr;
		}

		standupFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			standupGfx[i] = nullptr;
		}

		standleftFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			standleftGfx[i] = nullptr;
		}

		standrightFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			standrightGfx[i] = nullptr;
		}

		movedownFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			movedownGfx[i] = nullptr;
		}

		moveupFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			moveupGfx[i] = nullptr;
		}

		moveleftFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			moveleftGfx[i] = nullptr;
		}

		moverightFrames = 0;
		for (int i = 0; i < kMaxAnimFrames; i++) {
			moverightGfx[i] = nullptr;
		}
	}

	AIEntity() {
		reset();
	}

	~AIEntity() {
	}

	void save(Common::OutSaveFile *out);
	void load(Common::InSaveFile *in);
};

// Structs for Function Table Lookup for SaveGames
typedef void(*FuncPtr)(AIEntity *);
typedef void(*EntFuncPtr)(AIEntity *, int, int);

struct AIEntTypeInfo {
	AIType type;
	const char *luaName;
	AIStateDef *stateDef;
	void (*initFunc)(AIEntity *e);
	void (*initFunc2)(AIEntity *e);
};

struct FuncLookUp {
	void(*function)(AIEntity *e);
	const char *funcName;
};

extern AIEntTypeInfo aiEntList[];
extern FuncLookUp aiFuncList[];

struct AIEntLevel2 {
	uint16 x;
	uint16 y;
	Tile *draw;
	AIEntity *e;
	void(*aiDraw)(AIEntity *e, int x, int y);
	uint32 stunnedWait;

	AIEntLevel2() : x(0), y(0), draw(nullptr), e(nullptr), aiDraw(nullptr), stunnedWait(0) {}
};

struct AnimTarget {
	uint16 x, y;
	uint16 start, end;
	int16 vel;
	uint16 animCycle;
	uint16 animFrame;
	bool killAuto; // Keep it alive if its an Auto?
	bool inMap;
	Tile *gfxList[kMaxAnimTFrames];

	AnimTarget() : x(0), y(0), start(0), end(0), vel(0), animCycle(0), animFrame(0), killAuto(false), inMap(false) {
		for (int i = 0; i < kMaxAnimTFrames; i++) {
			gfxList[i] = nullptr;
		}
	}
};

struct InvEnt {
	uint16 keep;
	AIEntity ent;

	void reset() {
		keep = 0;
		ent.reset();
	}

	InvEnt() {
		reset();
	}
};

struct DlvEnt {
	char itemTextName[32];
	char itemGfxName[32];
	Tile *itemGfx;

	char destTextName[32];
	char destGfxName[32];
	Tile *destGfx;

	char id[32];

	DlvEnt() : itemGfx(nullptr), destGfx(nullptr) {
		itemTextName[0] = 0;
		itemGfxName[0] = 0;
		destTextName[0] = 0;
		destGfxName[0] = 0;
	}

	~DlvEnt() {
		itemGfx = nullptr;
		destGfx = nullptr;
	}
};

struct Waypoint {
	int x, y, level;

	void reset() {
		x = 0;
		y = 0;
		level = 0;
	}

	Waypoint() {
		reset();
	}
};

struct LuaT {
	uint16 x, y;
	uint16 value1, value2;
	char luaFuncInit[32];
	char luaFuncAction[32];
	char luaFuncUse[32];

	LuaT() : x(0), y(0), value1(0), value2(0) {
		luaFuncInit[0] = 0;
		luaFuncAction[0] = 0;
		luaFuncUse[0] = 0;
	}
};

struct ActionInfo {
	uint16 x1, y1;
	uint16 x2, y2;
	char luaFuncInit[32];
	char luaFuncUse[32];
	char entityName[32];

	ActionInfo() : x1(0), y1(0), x2(0), y2(0) {
		luaFuncInit[0] = 0;
		luaFuncUse[0] = 0;
		entityName[0] = 0;
	}
};

struct TeleInfo {
	uint16 x1, y1;
	uint16 x2, y2;
	AIDir dir1;
	AIDir dir2;
	uint16 level1, level2;
	uint16 usable1, usable2;
	uint16 anim1, anim2;
	char luaFuncUse1[32];
	char luaFuncUse2[32];

	TeleInfo() : x1(0), y1(0), x2(0), y2(0), dir1(DIR_NONE), dir2(DIR_NONE), level1(0), level2(0), usable1(0), usable2(0), anim1(0), anim2(0) {
		luaFuncUse1[0] = 0;
		luaFuncUse2[0] = 0;
	}
};

struct SingleTele {
	uint16 x, y, level, usable, anim;
	AIDir dir;

	SingleTele() : x(0), y(0), level(0), usable(0), anim(0), dir(DIR_NONE) {}
};

struct AutoAction {
	uint16 x, y;
	bool activated;
	char luaFuncInit[32];
	char luaFuncUse[32];
	char entityName[32];

	AutoAction() : x(0), y(0), activated(false) {
		luaFuncInit[0] = 0;
		luaFuncUse[0] = 0;
		entityName[0] = 0;
	}
};

struct ArrowPath {
	uint16 type;
	AIDir dir;
	uint16 tileX, tileY;

	ArrowPath() : type(0), dir(DIR_NONE), tileX(0), tileY(0) {}
};

struct HereT {
	uint16 x, y;
	char entName[32];
};

struct Trigger {
	char id[32];
	uint16 x, y;
	uint16 value1, value2;
	char luaFuncInit[32];
	char luaFuncUse[32];

	Trigger() : x(0), y(0), value1(0), value2(0) {
		id[0] = 0;
		luaFuncInit[0] = 0;
		luaFuncUse[0] = 0;
	}
};

struct CallbackDef {
	CallbackType type;
	void(*function)(int x, int y);
};

struct Callback {
	CallbackType type;
	uint16 x, y;
	uint16 delay;

	Callback() : type(NO_FUNCTION), x(0), y(0), delay(0) {}
};

struct Fairystone {
	uint16 srcX, srcY;
	uint16 destX, destY;

	Fairystone() : srcX(0), srcY(0), destX(0), destY(0) {}
};

struct Bridge {
	uint16 x, y;
	AIDir dir;
	uint16 delay;
	uint16 anim;

	void reset() {
		x = 0;
		y = 0;
		dir = DIR_NONE;
		delay = 0;
		anim = 0;
	}

	Bridge() {
		reset();
	}
};

struct CineCommand {
	CineType cmdType;
	double x, y;
	double x2, y2;
	double xv, yv;
	int start, end;
	uint32	delay;
	int	speed;
	const char *title;
	const char *string;
	const char *id;
	AIEntity *e;
	Picture *pic;

	CineCommand() : cmdType(C_NO_COMMAND), x(0.0), y(0.0), x2(0.0), y2(0.0), xv(0.0), yv(0.0),
				start(0), end(0), delay(0), speed(0), title(nullptr), string(nullptr), id(nullptr), e(nullptr), pic(nullptr) {}
};

struct CineBlit {
	double x, y;
	Picture *pic;
	const char *name;
	const char *id;
	bool masked;

	CineBlit() : x(0), y(0), pic(nullptr), name(nullptr), id(nullptr), masked(false) {}
};

#define onEvenTile(x, y)		( !(x & 31) && !(y & 31) )
#define hitPlayer(x, y)			( e->onScreen && g_hdb->_ai->checkPlayerCollision( x, y, 4 ) && !g_hdb->_ai->playerDead() )
#define cycleFrames( e, max ) \
	{ \
		if ( e->animDelay-- < 1 ) \
		{ \
			e->animDelay = e->animCycle; \
			e->animFrame++; \
			if ( e->animFrame >= max ) \
				e->animFrame = 0; \
		} \
	}
#define spawnBlocking(x, y, level)	g_hdb->_ai->spawn(AI_NONE, DIR_NONE, x, y, nullptr, nullptr, nullptr, DIR_NONE, level, 0, 0, 0)

class AI {
public:
	AI();
	~AI();

	void init();
	void clearPersistent();
	void restartSystem();
	const char *funcLookUp(void(*function)(AIEntity *e));
	FuncPtr funcLookUp(const char *function);
	void save(Common::OutSaveFile *out);
	void loadSaveFile(Common::InSaveFile *in);
	void initAnimInfo();

	// Entity Functions
	AIEntity *spawn(AIType type, AIDir dir, int x, int y, const char *funcInit, const char *funcAction, const char *funcUse, AIDir dir2, int level, int value1, int value2, int callInit);
	bool cacheEntGfx(AIEntity *e, bool initFlag);
	void stopEntity(AIEntity *e);
	AIEntity *locateEntity(const char *luaName);
	AIEntity *findEntity(int x, int y);
	AIEntity *findEntityIgnore(int x, int y, AIEntity *ignore);
	AIEntity *findEntityType(AIType type, int x, int y);
	void getEntityXY(const char *entName, int *x, int *y);
	bool useLuaEntity(const char *initName);
	void removeLuaEntity(const char *initName);
	void animLuaEntity(const char *initName, AIState st);
	void setLuaAnimFrame(const char *initName, AIState st, int frame);
	int checkForTouchplate(int x, int y);
	void removeEntity(AIEntity *e);
	void setEntityGoal(AIEntity *e, int x, int y);
	void initAllEnts();
	void killPlayer(Death method);
	void stunEnemy(AIEntity *e, int time);
	int metalOrFleshSND(AIEntity *e);
	int tileDistance(AIEntity *e1, AIEntity *e2) {
		return abs(e1->tileX - e2->tileX) + abs(e1->tileY - e2->tileY);
	}

	void animateEntity(AIEntity *e);
	void animEntFrames(AIEntity *e);
	void drawEnts(int x, int y, int w, int h);
	void drawLevel2Ents();
	void animGrabbing();
	void entityFace(const char *luaName, int dir);

	void moveEnts();

	bool findPath(AIEntity *e);
	AIEntity *legalMove(int tileX, int tileY, int level, int *result);
	AIEntity *legalMoveOverWater(int tileX, int tileY, int level, int *result);
	AIEntity *legalMoveOverWaterIgnore(int tileX, int tileY, int level, int *result, AIEntity *ignore);
	void addAnimateTarget(int x, int y, int start, int end, AnimSpeed speed, bool killAuto, bool inMap, const char *tileName);
	void animateTargets();

	void addBridgeExtend(int x, int y, int bridgeType);
	void animateBridges();
	void addToFairystones(int index, int tileX, int tileY, int sourceOrDest);
	int checkFairystones(int tileX, int tileY);
	void getFairystonesSrc(int index, int *tileX, int *tileY) {
		*tileX = _fairystones[index].srcX;
		*tileY = _fairystones[index].srcY;
	}

	AIEntity *playerCollision(int topBorder, int bottomBorder, int leftBorder, int rightBorder);
	bool checkPlayerTileCollision(int x, int y);
	bool checkPlayerCollision(int x, int y, int border);
	void clearDiverters();
	void laserScan();

	// List functions
	void addToActionList(int actionIndex, int x, int y, char *funcLuaInit, char *funcLuaUse);
	bool checkActionList(AIEntity *e, int x, int y, bool lookAndGrab);
	void addToHereList(const char *entName, int x, int y);
	HereT *findHere(int x, int y);
	void addToAutoList(int x, int y, const char *luaFuncInit, const char *luaFuncUse);
	void autoDeactivate(int x, int y);
	bool activateAction(AIEntity *e, int x, int y, int targetX, int targetY);
	bool checkAutoList(AIEntity *e, int x, int y);
	bool autoActive(int x, int y);
	void addCallback(CallbackType type, int x, int y, int delay);
	void processCallbackList();
	void addToLuaList(int x, int y, int value1, int value2, char *luaFuncInit, char *luaFuncAction, char *luaFuncUse);
	bool checkLuaList(AIEntity *e, int x, int y);
	bool luaExistAtXY(int x, int y);
	void addToTeleportList(int teleIndex, int x, int y, int dir, int level, int anim, int usable, const char *luaFuncUse);
	bool checkTeleportList(AIEntity *e, int x, int y);
	bool findTeleporterDest(int tileX, int tileY, SingleTele *info);
	void addToPathList(int x, int y, int type, AIDir dir);
	ArrowPath *findArrowPath(int x, int y);
	void addToTriggerList(char *luaFuncInit, char *luaFuncUse, int x, int y, int value1, int value2, char *id);
	bool checkTriggerList(char *entName, int x, int y);
	void killTrigger(const char *id);

	void floatEntity(AIEntity *e, AIState state);
	bool checkFloating(int x, int y);

	bool getTableEnt(AIType type);
	bool walkThroughEnt(AIType type);
	void getItemSound(AIType type);
	void lookAtEntity(AIEntity *e);

	// Player Functions

	void movePlayer(uint16 buttons);
	void playerUse();
	void setPlayerWeapon(AIType w, Tile *gfx) {
		_weaponSelected = w;
		_weaponGfx = gfx;
	}
	AIType getPlayerWeapon() {
		return _weaponSelected;
	}
	Tile *getPlayerWeaponGfx() {
		return _weaponGfx;
	}
	Tile *getPlayerWeaponSelGfx() {
		return _weaponSelGfx;
	}

	AIEntity *getPlayer() {
		if (!_player)
			return &_dummyPlayer;
		return _player;
	}

	void getPlayerXY(int *x, int *y) {
		if (_player) {
			*x = _player->x;
			*y = _player->y;
		} else {
			*x = *y = 0;
		}
	}

	void setPlayerXY(int x, int y) {
		if (_player) {
			_player->x = x;
			_player->tileX = x / kTileWidth;
			_player->y = y;
			_player->tileY = y / kTileHeight;
			_player->xVel = _player->yVel = 0;
		}
	}

	void assignPlayer(AIEntity *p) {
		_player = p;
	}

	bool playerDead() {
		return _playerDead;
	}

	bool playerOnIce() {
		return _playerOnIce;
	}

	bool playerLocked() {
		return _playerLock;
	}

	void setPlayerLock(bool status) {
		_playerLock = status;
	}

	void setPlayerInvisible(bool status) {
		_playerInvisible = status;
	}

	bool playerRunning() {
		return _playerRunning;
	}

	void togglePlayerRunning() {
		_playerRunning = !_playerRunning;
	}

	// Cinematic Functions
	bool cinematicsActive() {
		return _cineActive;
	}
	bool cineAbortable() {
		return _cineAbortable;
	}
	void processCines();
	void cineCleanup();
	void cineAbort();
	void cineAddToBlitList(const char *id, Picture *pic, int x, int y, bool masked);
	Picture *cineFindInBlitList(const char *name);
	void cineRemoveFromBlitList(const char *name);
	void cineAddToFreeList(Picture *pic);
	void cineFreeGfx();
	void cineStart(bool abortable, const char *abortFunc);
	void cineStop(const char *funcNext);
	void cineStartMap(const char *mapName);
	void cineLockPlayer();
	void cineUnlockPlayer();
	void cineSetCamera(int x, int y);
	void cineResetCamera();
	void cineMoveCamera(int x, int y, int speed);
	void cineWait(int seconds);
	void cineWaitUntilDone();
	void cineSetEntity(const char *entName, int x, int y, int level);
	void cineMoveEntity(const char *entName, int x, int y, int level, int speed);
	void cineSpawnEntity(AIType t, AIDir d, int x, int y, const char *func_init, const char *func_action,
					const char *func_use, AIDir d2, int level, int value1, int value2);
	void cineRemoveEntity(const char *entName);
	void cineAnimEntity(const char *entName, AIState state, int loop);
	void cineSetAnimFrame(const char *entName, AIState state, int frame);
	void cineEntityFace(const char *luaName, double dir);
	void cineDialog(const char *title, const char *string, int seconds);
	void cineTextOut(const char *text, int x, int y, int timer);
	void cineCenterTextOut(const char *text, int y, int timer);
	void cineDrawPic(const char *id, const char *pic, int x, int y);
	void cineDrawMaskedPic(const char *id, const char *pic, int x, int y);
	void cineMovePic(const char *id, const char *pic, int x1, int y1, int x2, int y2, int speed);
	void cineMoveMaskedPic(const char *id, const char *pic, int x1, int y1, int x2, int y2, int speed);
	void cineUse(const char *entName);
	void cinePlaySound(int index);
	void cinePlayVoice(int index, int actor);
	void cineFadeIn(bool isBlack, int steps);
	void cineFadeOut(bool isBlack, int steps);
	void cineClearForeground(int x, int y);
	void cineSetBackground(int x, int y, int index);
	void cineSetForeground(int x, int y, int index);
	void cineFunction(const char *func);

	// Waypoint & Movement Functions
	void lookAtXY(int x, int y);
	void addWaypoint(int px, int py, int x, int y, int level);
	void removeFirstWaypoint();
	void clearWaypoints();
	bool traceStraightPath(int x1, int y1, int *x2, int *y2, int *lvl);
	Tile *getStandFrameDir(AIEntity *e);
	void drawWayPoints();
	int waypointsLeft() {
		return _numWaypoints;
	}

	// Inventory Functions
	bool addToInventory(AIEntity *e);
	void purgeInventory();
	void clearInventory();
	int getInvAmount() {
		return _numInventory;
	}
	int getGemAmount() {
		return _numGems;
	}
	int getMonkeystoneAmount() {
		return _numMonkeystones;
	}
	int getGooCupAmount() {
		return _numGooCups;
	}
	void setGemAmount(int amt) {
		_numGems = amt;
	}
	int getInvMax() {
		return _numInventory;
	}
	AIType getInvItemType(int which) {
		return _inventory[which].ent.type;
	}
	Tile *getInvItemGfx(int which) {
		return _inventory[which].ent.standdownGfx[0];
	}

	AIEntity *getInvItem(int which);
	int queryInventory(const char *string);
	bool removeInvItem(const char *string, int amount);
	int queryInventoryType(AIType which);
	int queryInventoryTypeSlot(AIType which);
	bool removeInvItemType(AIType which, int amount);
	bool addItemToInventory(AIType type, int amount, const char *funcInit, const char *funcAction, const char *funcUse);
	void keepInvItem(AIType type);
	void printYouGotMsg(const char *name);

	// Delivery Functions
	void newDelivery(const char *itemTextName, const char *itemGfxName, const char *destTextName, const char *destGfxName, const char *id);
	int getDeliveriesAmount() {
		return _numDeliveries;
	}
	DlvEnt *getDeliveryItem(int which) {
		return &_deliveries[which];
	}
	bool completeDelivery(const char *id);

	// Gate Puddles
	int _gatePuddles;
	void addGatePuddle(int amount) {
		_gatePuddles += amount;
	}
	int getGatePuddles() {
		return _gatePuddles;
	}

	// Platform-Specific Constants
	int _youGotY;

	// Player Variables
	bool _playerDead;
	bool _playerInvisible;	// While on RailRider for example
	bool _playerOnIce;
	bool _playerEmerging;
	bool _playerRunning;

	uint16		_pushupFrames;
	Tile		*_pushupGfx[kMaxAnimFrames];

	uint16		_pushdownFrames;
	Tile		*_pushdownGfx[kMaxAnimFrames];

	uint16		_pushleftFrames;
	Tile		*_pushleftGfx[kMaxAnimFrames];

	uint16		_pushrightFrames;
	Tile		*_pushrightGfx[kMaxAnimFrames];

	Tile		*_getGfx[5];			// only 1 frame in each direction (+1 for DIR_NONE at start)

	uint16		_dyingFrames;
	Tile		*_dyingGfx[kMaxDeathFrames];

	Tile		*_goodjobGfx;			// only 1 frame

	uint16		_horrible1Frames;
	Tile		*_horrible1Gfx[kMaxDeathFrames];
	uint16		_horrible2Frames;
	Tile		*_horrible2Gfx[kMaxDeathFrames];
	uint16		_horrible3Frames;
	Tile		*_horrible3Gfx[kMaxDeathFrames];
	uint16		_horrible4Frames;
	Tile		*_horrible4Gfx[kMaxDeathFrames];
	uint16		_plummetFrames;
	Tile		*_plummetGfx[kMaxDeathFrames];

	uint16		_clubUpFrames;
	Picture		*_clubUpGfx[kMaxAnimFrames];
	uint16		_clubDownFrames;
	Picture		*_clubDownGfx[kMaxAnimFrames];
	uint16		_clubLeftFrames;
	Picture		*_clubLeftGfx[kMaxAnimFrames];
	uint16		_clubRightFrames;
	Picture		*_clubRightGfx[kMaxAnimFrames];

	uint16		_stunUpFrames;
	Tile		*_stunUpGfx[kMaxAnimFrames];
	uint16		_stunDownFrames;
	Tile		*_stunDownGfx[kMaxAnimFrames];
	uint16		_stunLeftFrames;
	Tile		*_stunLeftGfx[kMaxAnimFrames];
	uint16		_stunRightFrames;
	Tile		*_stunRightGfx[kMaxAnimFrames];
	Tile		*_stunLightningGfx[kMaxAnimFrames];
	Tile		*_stunnedGfx[kMaxAnimFrames];

	uint16		_slugUpFrames;
	Tile		*_slugUpGfx[kMaxAnimFrames];
	uint16		_slugDownFrames;
	Tile		*_slugDownGfx[kMaxAnimFrames];
	uint16		_slugLeftFrames;
	Tile		*_slugLeftGfx[kMaxAnimFrames];
	uint16		_slugRightFrames;
	Tile		*_slugRightGfx[kMaxAnimFrames];

	uint16		_slugAttackFrames;
	Picture		*_slugAttackGfx[kMaxAnimFrames];

	Tile		*_weaponSelGfx;
	Tile		*_weaponGfx;
	AIType		_weaponSelected;

	// Player Resources and Deliveries

	int _numGems;
	int _numGooCups;
	int _numMonkeystones;

	// Special Tiles that are usable
	// These variables hold the tile-indices set
	// in ai-init.cpp

	int	_useSwitchOff;		// the door opening switch
	int	_useSwitchOn;		// state, when opened
	int	_useHolderEmpty;	// cell holding switch
	int	_useHolderFull;		// state, when full
	int	_useSwitch2Off;		// another switch
	int	_useSwitch2On;		// state, when opened
	int	_useMailsorter;		// mailsorter entity
	int	_useAskcomp;		// askcomp entitiy
	int	_useTeleporter;		// teleporter entity
	int	_useHandswitchOn;	// 2-sided handswitch
	int	_useHandswitchOff;	// 2-sided handswitch

	int	_targetDoorN;		// horz SILVER door
	int	_targetDoorP;		// horz BLUE door
	int	_targetDoorS;		// horz RED door
	int	_targetDoorNv;		// vert SILVER door
	int	_targetDoorPv;		// vert BLUE door
	int	_targetDoorSv;		// vert RED door

	int	_targetDoor2N;		// horz SILVER door
	int	_targetDoor2P;		// horz BLUE door
	int	_targetDoor2S;		// horz RED door
	int	_targetDoor2Nv;		// vert SILVER door
	int	_targetDoor2Pv;		// vert BLUE door
	int	_targetDoor2Sv;		// vert RED door

	int	_target2DoorN;		// horz SILVER door
	int	_target2DoorP;		// horz BLUE door
	int	_target2DoorS;		// horz RED door
	int	_target2DoorNv;		// vert SILVER door
	int	_target2DoorPv;		// vert BLUE door
	int	_target2DoorSv;		// vert RED door

	int	_target3DoorN;		// horz SILVER door
	int	_target3DoorP;		// horz BLUE door
	int	_target3DoorS;		// horz RED door
	int	_target3DoorNv;		// vert SILVER door
	int	_target3DoorPv;		// vert BLUE door
	int	_target3DoorSv;		// vert RED door

	int	_targetBridgeU;		// bridge extending UP
	int	_targetBridgeD;		// bridge extending DOWN
	int	_targetBridgeL;		// bridge extending LEFT
	int	_targetBridgeR;		// bridge extending RIGHT

	int	_targetBridgeMidLR;	// bridge grating plank LEFT/RIGHT
	int	_targetBridgeMidUD;	// bridge grating plank UP/DOWN
	int	_touchplateOn;		// touchplate ON
	int	_touchplateOff;
	int	_templeTouchpOn;	// touchplate ON
	int	_templeTouchpOff;
	int	_blockpole;			// blockpole

	int	_kcHolderWhiteOff;	// keycard holders
	int	_kcHolderWhiteOn;
	int	_kcHolderBlueOff;
	int	_kcHolderBlueOn;
	int	_kcHolderRedOff;
	int	_kcHolderRedOn;
	int	_kcHolderGreenOff;
	int	_kcHolderGreenOn;
	int	_kcHolderPurpleOff;
	int	_kcHolderPurpleOn;
	int	_kcHolderBlackOff;
	int	_kcHolderBlackOn;

	AIEntLevel2 _entsLevel2[kMaxLevel2Ents];
	int _numLevel2Ents;

	InvEnt _inventory[kMaxInventory];
	int _numInventory;

	DlvEnt _deliveries[kMaxDeliveries];
	int _numDeliveries;

	Waypoint _waypoints[kMaxWaypoints];
	int _numWaypoints;
	Tile *_waypointGfx[4]; // Animating waypoint gfx
	Tile *_debugQMark;

	LuaT _luaList[kMaxLuaEnts];
	int _numLuaList;

	ActionInfo _actions[kMaxActions];

	TeleInfo _teleporters[kMaxTeleporters];
	int _numTeleporters;
	// Virtual Player
	AIEntity _dummyPlayer, _dummyLaser;
	bool _laserRescan, _laserOnScreen;

	AutoAction _autoActions[kMaxAutoActions];

	Callback _callbacks[kMaxCallbacks];

	Fairystone _fairystones[kMaxFairystones];

	Bridge _bridges[kMaxBridges];
	int _numBridges;

	Common::Array<ArrowPath *> *_arrowPaths;
	Common::Array<HereT *> *_hereList;
	Common::Array<Trigger *> *_triggerList;

	// Cinematic Variables
	Common::Array<CineCommand *> _cine;

	Picture *_cineFreeList[kMaxCineGfx];
	int _numCineFreeList;

	CineBlit *_cineBlitList[kMaxCineGfx];
	int _numCineBlitList;

	int _stunAnim;
	uint32 _stunTimer;

	// Bots Gfx

	Picture *_icepSnowballGfxDown;		// ICEPUFF's snowball moving down
	Picture *_icepSnowballGfxLeft;		// ICEPUFF's snowball moving left
	Picture *_icepSnowballGfxRight;		// ICEPUFF's snowball moving right

	Tile *_tileFroglickMiddleUD;
	Tile *_tileFroglickWiggleUD[3];
	Tile *_tileFroglickMiddleLR;
	Tile *_tileFroglickWiggleLeft[3];
	Tile *_tileFroglickWiggleRight[3];

	Picture *_gfxDragonAsleep;
	Picture *_gfxDragonFlap[2];
	Picture *_gfxDragonBreathe[3];

	Tile *_gfxLaserbeamUD[4];
	Tile *_gfxLaserbeamUDTop[4];
	Tile *_gfxLaserbeamUDBottom[4];
	Tile *_gfxLaserbeamLR[4];
	Tile *_gfxLaserbeamLRLeft[4];
	Tile *_gfxLaserbeamLRRight[4];

private:

	// Action Functions

	// Checks for the existence of a closed/open door
	bool isClosedDoor(int x, int y);
	bool isOpenDoor(int x, int y);

	// MAIN FUNCTION : handles all animation of targeted tiles & changing the state of the "switch"
	bool useTarget(int x, int y, int targetX, int targetY, int newTile, int *worked);

	// Black Door Switch
	bool useSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile);
	bool useSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile);
	bool useSwitch2(AIEntity *e, int x, int y, int targetX, int targetY);

	// Colored Keycard Switch
	bool useLockedSwitch(AIEntity *e, int x, int y, int targetX, int targetY, int onTile, AIType item, const char *keyerror);
	bool useLockedSwitchOn(AIEntity *e, int x, int y, int targetX, int targetY, int offTile, AIType item);

	// Purple Cell Holder Switch
	bool useCellHolder(AIEntity *e, int x, int y, int targetX, int targetY);

	// Touchplate
	bool useTouchplate(AIEntity *e, int x, int y, int targetX, int targetY, int type);
	bool useTouchplateOn(AIEntity *e, int x, int y, int targetX, int targetY, int type);

	// Normal Door
	bool useDoorOpenClose(AIEntity *e, int x, int y);
	bool useAutoDoorOpenClose(AIEntity *e, int x, int y);

	// Any Type Door
	bool useDoorOpenCloseBot(AIEntity *e, int x, int y);

	Common::Array<AIEntity *> *_ents;
	Common::Array<AIEntity *> *_floats;
	Common::Array<AnimTarget *> _animTargets;
	AIEntity *_player;

	// Cinematics Variables
	bool _cineAbortable;
	bool _cineAborted;
	const char *_cineAbortFunc;
	bool _cineActive;
	bool _playerLock;
	bool _cameraLock;
	double _cameraX, _cameraY;

};

const char *AIType2Str(AIType v);
const char *AIState2Str(AIState v);

} // End of Namespace

#endif // !HDB_AI_H
