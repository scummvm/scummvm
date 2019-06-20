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

#include "common/system.h"

namespace HDB {

enum {
	kMaxAnimFrames = 8,
	kMaxDeathFrames = 12,
	kMaxInventory = 10,
	kMaxDeliveries = 5,
	kMaxWaypoints = 10,
	kPlayerMoveSpeed = 4,
	kEnemyMoveSpeed = 2,
	kPushMoveSpeed = (kPlayerMoveSpeed >> 1)
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

struct AIStateDef {
	AIState state;
	const char name[64];
};

struct AIEntity {
	AIType type;
	AIState state;
	AIDir dir;

	void *draw;											// Current frame to draw

	void (*aiInit)(AIEntity *e);						// func ptr to init routine
	void (*aiInit2)(AIEntity *e);						// func ptr to init2 routine - graphic init only (this for LoadGame functionality)
	void (*aiAction)(AIEntity *e);						// func ptr to action routine
	void (*aiUse)(AIEntity *e);							// func ptr to use routine
	void (*aiDraw)(AIEntity *e, int x, int y);			// func ptr to extra drawing routine (only for special stuff) - pass in mapx, mapy

	char		luaFuncInit[32];						// Lua function for Init (always called after entity's init). These are ptrs into the map header.
	char		luaFuncAction[32];						// Lua function for Action
	char		luaFuncUse[32];							// Lua function for Use
	uint16		level;									// which floor level we're on
	uint16		value1, value2;							// extra values we might need
	AIDir		dir2;									// this is from TED

	uint16		x, y;
	uint16		drawXOff, drawYOff;					// might need a drawing offset
	uint16		onScreen;								// FLAG: is this entity onscreen?
	uint16		moveSpeed;								// movement speed of this entity
	int16		xVel, yVel;								// movement values
	uint16		tileX, tileY;
	uint16		goalX, goalY;							// where we're trying to go - TILE COORDS
	uint16		touchpX, touchpY, touchpTile, touchpWait;		// ACTION index a touchplate is using, which you're on
	uint32		stunnedWait;							// if we're stunned, this is the delay before being normal again
	uint16		sequence;								// to use for specially-coded sequences
	char		entityName[32];						// the name of the entity, as registered by the Lua init function for the entity
	char		printedName[32];						// the name of the entity/item, the way it should be printed

	uint16		animFrame;								// which frame we're on
	uint16		animDelay;								// changes every frame; based on anim_cycle at start
	uint16		animCycle;								// delay between frame animations

	union {
		uint16 blinkFrames;
		uint16 int1;
	};
	void *blinkGfx[kMaxAnimFrames];

	union {
		uint16 special1Frames;
		uint16 int2;
	};
	void *special1Gfx[kMaxAnimFrames];

	uint16		standdownFrames;
	Tile		*standdownGfx[kMaxAnimFrames];

	uint16		standupFrames;
	Tile		*standupGfx[kMaxAnimFrames];

	uint16		standleftFrames;
	Tile		*standleftGfx[kMaxAnimFrames];

	uint16		standrightFrames;
	Tile		*standrightGfx[kMaxAnimFrames];

	uint16		moveupFrames;
	Tile		*moveupGfx[kMaxAnimFrames];

	uint16		movedownFrames;
	Tile		*movedownGfx[kMaxAnimFrames];

	uint16		moveleftFrames;
	Tile		*moveleftGfx[kMaxAnimFrames];

	uint16		moverightFrames;
	Tile		*moverightGfx[kMaxAnimFrames];
};

struct AIEntTypeInfo {
	AIType type;
	const char *luaName;
	AIStateDef *stateDef;
	void (*initFunc)(AIEntity *e);
	void (*initFunc2)(AIEntity *e);
};

extern AIEntTypeInfo aiEntList[];

struct InvEnt {
	uint16 keep;
	AIEntity ent;
};

struct Waypoint {
	int x, y, level;
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
	char *id;
	AIEntity *e;

	CineCommand() : cmdType(C_NO_COMMAND), x(0.0), y(0.0), x2(0.0), y2(0.0), xv(0.0), yv(0.0),
				start(0), end(0), delay(0), speed(0), title(NULL), string(NULL), id(NULL), e(NULL) {}
};

class AI {
public:
	AI();
	~AI();

	bool init();
	void clearPersistent();
	void restartSystem();

	// Entity Functions
	AIEntity *spawn(AIType type, AIDir dir, int x, int y, char *funcInit, char *funcAction, char *funcUse, AIDir dir2, int level, int value1, int value2, int callInit);
	bool cacheEntGfx(AIEntity *e, bool init);
	void stopEntity(AIEntity *e);
	AIEntity *locateEntity(const char *luaName);
	void removeEntity(AIEntity *e);
	void setEntityGoal(AIEntity *e, int x, int y);
	void initAllEnts();
	bool getTableEnt(AIType type);
	bool walkThroughEnt(AIType type);
	void getItemSound(AIType type);
	void lookAtEntity(AIEntity *e);

	// Player Functions
	AIEntity *getPlayer() {
		warning("STUB: AI::getPlayer: dummyplayer not supported");
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
	void processCines();
	void cineStart(bool abortable, const char *abortFunc);
	void cineLockPlayer();
	void cineUnlockPlayer();
	void cineSetCamera(int x, int y);
	void cineResetCamera();
	void cineMoveCamera(int x, int y, int speed);
	void cineWait(int seconds);
	void cineWaitUntilDone();
	void cineSetEntity(const char *entName, int x, int y, int level);
	void cineMoveEntity(const char *entName, int x, int y, int level, int speed);
	void cineUse(const char *entName);
	void cineFadeIn(bool isBlack, int steps);
	void cineFadeOut(bool isBlack, int steps);

	// Waypoint & Movement Functions
	void lookAtXY(int x, int y);
	void clearWaypoints();

	// Inventory Functions
	bool addToInventory(AIEntity *e);

	// Cinematic Variables
	Common::Array<CineCommand *> _cine;

private:

	Common::Array<AIEntity *> *_ents;
	AIEntity *_player;

	// Player Variables
	bool _playerDead;
	bool _playerInvisible;	// While on RailRider for example
	bool _playerOnIce;
	bool _playerEmerging;
	bool _playerRunning;

	uint16		pushupFrames;
	Tile		*pushupGfx[kMaxAnimFrames];

	uint16		pushdownFrames;
	Tile		*pushdownGfx[kMaxAnimFrames];

	uint16		pushleftFrames;
	Tile		*pushleftGfx[kMaxAnimFrames];

	uint16		pushrightFrames;
	Tile		*pushrightGfx[kMaxAnimFrames];

	Tile		*getGfx[5];			// only 1 frame in each direction (+1 for DIR_NONE at start)

	uint16		dyingFrames;
	Tile		*dyingGfx[kMaxDeathFrames];

	Tile		*goodjobGfx;			// only 1 frame

	uint16		horrible1Frames;
	Tile		*horrible1Gfx[kMaxDeathFrames];
	uint16		horrible2Frames;
	Tile		*horrible2Gfx[kMaxDeathFrames];
	uint16		horrible3Frames;
	Tile		*horrible3Gfx[kMaxDeathFrames];
	uint16		horrible4Frames;
	Tile		*horrible4Gfx[kMaxDeathFrames];
	uint16		plummetFrames;
	Tile		*plummetGfx[kMaxDeathFrames];

	uint16		clubUpFrames;
	Tile		*clubUpGfx[kMaxAnimFrames];
	uint16		clubDownFrames;
	Tile		*clubDownGfx[kMaxAnimFrames];
	uint16		clubLeftFrames;
	Tile		*clubLeftGfx[kMaxAnimFrames];
	uint16		clubRightFrames;
	Tile		*clubRightGfx[kMaxAnimFrames];

	uint16		stunUpFrames;
	Tile		*stunUpGfx[kMaxAnimFrames];
	uint16		stunDownFrames;
	Tile		*stunDownGfx[kMaxAnimFrames];
	uint16		stunLeftFrames;
	Tile		*stunLeftGfx[kMaxAnimFrames];
	uint16		stunRightFrames;
	Tile		*stunRightGfx[kMaxAnimFrames];
	Tile		*stun_lightningGfx[kMaxAnimFrames];
	Tile		*stunnedGfx[kMaxAnimFrames];

	uint16		slugUpFrames;
	Tile		*slugUpGfx[kMaxAnimFrames];
	uint16		slugDownFrames;
	Tile		*slugDownGfx[kMaxAnimFrames];
	uint16		slugLeftFrames;
	Tile		*slugLeftGfx[kMaxAnimFrames];
	uint16		slugRightFrames;
	Tile		*slugRightGfx[kMaxAnimFrames];

	uint16		slugAttackFrames;
	Tile		*slugAttackGfx[kMaxAnimFrames];

	// Player Resources and Deliveries

	int _numGems;
	int _numGooCups;
	int _numMonkeystones;

	InvEnt _inventory[kMaxInventory];
	int _numInventory;

	Waypoint _waypoints[kMaxWaypoints];
	int _numWaypoints;
	Tile *_waypointGfx[4]; // Animating waypoint gfx

	// Cinematics Variables
	bool _cineAbortable;
	bool _cineAborted;
	const char *_cineAbortFunc;
	bool _cineActive;
	bool _playerLock;
	bool _cameraLock;
	double _cameraX, _cameraY;

};

} // End of Namespace

#endif // !HDB_AI_H
