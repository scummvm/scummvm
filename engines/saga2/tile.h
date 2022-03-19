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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TILE_H
#define SAGA2_TILE_H

#include "common/memstream.h"
#include "saga2/fta.h"
#include "saga2/tileload.h"
#include "saga2/annoy.h"
#include "saga2/terrain.h"
#include "saga2/property.h"
#include "saga2/tcoords.h"

namespace Saga2 {

/* ===================================================================== *
   Tile ID's and asociated functions
 * ===================================================================== */

class   TileBank;
typedef TileBank    *TileBankPtr;           // pointer to tile bank


inline TileID makeTileID(int bank, int num) {
	return (TileID)((bank << 10) | num);
}

inline void TileID2Bank(TileID t, int16 &bank, int16 &num) {
	bank = (int16)(t >> 10);
	num = (int16)(t & 0x3ff);
}

/* ===================================================================== *
   Inline functions
 * ===================================================================== */

//  Given a U between 0 and 3, and a V between 0 and 3, compute
//  a terrain mask for the subtile at (U,V)

inline int16 calcSubTileMask(int u, int v) {
	return (int16)(1 << ((u << 2) + v));
}

/* ===================================================================== *
   Tile Attributes
 * ===================================================================== */

struct TileAttrs {


	//  Height above base of tile below which terrain has effect
	uint8           terrainHeight;

	//  Visual information
	uint8           height;                 // height of tile bitmap

	//  Terrain information

	uint16          terrainMask;            // 16 terrain selector bits
	uint8           fgdTerrain,
	                bgdTerrain;

	//  Reserved bytes
	uint8           reserved0[8];         // auto-terrain data

	//  Masking information

	uint8           maskRule,               // which tile masking rule to use
	                altMask;                // for tiles with special masks

	//  Altitude information

	uint8           cornerHeight[4];

	//  Animation information

	uint8           cycleRange;             // cycle range for tile
	uint8           tileFlags;              // various flags for tile

	//  Reserved bytes
	uint16          reserved1;

	int32 testTerrain(int16 mask) {
		int32       terrain = 0;

		if (terrainMask & mask)  terrain |= (1 << fgdTerrain);
		if (~terrainMask & mask) terrain |= (1 << bgdTerrain);
		return terrain;
	}
};

enum tile_flags {
	//  This tile has been used in at least one activity group
	tileInGroup     = (1 << 0),

	//  Indicates that an activity group should be placed in lieu
	//  of the tile.
	tileAutoGroup   = (1 << 1),

	//  Indicates that the tile is sensitive to being walked on
	tileWalkSense   = (1 << 2),

	//  Indicates that tile has been recently modified
	tileModified    = (1 << 3)
};

/* ===================================================================== *
   Terrain types
 * ===================================================================== */

enum terrainTypes {
	terrNumNormal   = 0,
	terrNumEasy,
	terrNumRough,
	terrNumStone,
	terrNumWood,
	terrNumHedge,
	terrNumTree,
	terrNumWater,
	terrNumFall,
	terrNumRamp,
	terrNumStair,
	terrNumLadder,
	terrNumObject,
	terrNumActive,
	terrNumSlash,
	terrNumBash,
	terrNumIce,
	terrNumCold,
	terrNumHot,
	terrNumFurniture
};

enum terrainBits {
	terrainNormal       = (1 << terrNumNormal), // clear terrain
	terrainEasy         = (1 << terrNumEasy),   // easy terrain (path)
	terrainRough        = (1 << terrNumRough),  // rough terrain (shrub)
	terrainStone        = (1 << terrNumStone),  // stone obstacle
	terrainWood         = (1 << terrNumWood),   // wood obstacle
	terrainHedge        = (1 << terrNumHedge),  // penetrable obstacle
	terrainTree         = (1 << terrNumTree),   // tree obstacle
	terrainWater        = (1 << terrNumWater),  // water (depth given by height)
	terrainFall         = (1 << terrNumFall),   // does not support things
	terrainRamp         = (1 << terrNumRamp),   // low friction slope
	terrainStair        = (1 << terrNumStair),  // high friction slope
	terrainLadder       = (1 << terrNumLadder), // vertical climb
	terrainObject       = (1 << terrNumObject), // collision with other object
	terrainActive       = (1 << terrNumActive), // tile is sensitive to walking on
	terrainSlash        = (1 << terrNumSlash),  // Slide Down Slope Left
	terrainBash         = (1 << terrNumBash),   // Slide Down Slope Left
	terrainIce          = (1 << terrNumIce),
	terrainCold         = (1 << terrNumCold),
	terrainHot          = (1 << terrNumHot),
	terrainFurniture    = (1 << terrNumFurniture)
};

//  A combination mask of all the terrain types which can have
//  sloped surfaces. (Water is a negative sloped surface)

const int           terrainSurface  = terrainNormal
                                      | terrainEasy
                                      | terrainRough
                                      | terrainWater
                                      | terrainRamp
                                      | terrainCold
                                      | terrainStair;

const int           terrainSolidSurface
    = terrainNormal
      | terrainEasy
      | terrainRough
      | terrainRamp
      | terrainCold
      | terrainStair;

//  A combination mask of all terrain types which can have
//  raised surfaces.

const int           terrainRaised   = terrainStone
                                      | terrainWood
                                      | terrainTree
                                      | terrainHedge
                                      | terrainFurniture;

const int           terrainSupportingRaised = terrainStone
        | terrainWood
        | terrainFurniture;

const int           terrainImpassable = terrainStone
                                        | terrainWood
                                        | terrainTree
                                        | terrainHedge
                                        | terrainFurniture;

const int           terrainSlow     = terrainRough
                                      | terrainWater
                                      | terrainLadder;

const int           terrainAverage  = terrainNormal
                                      | terrainRamp
                                      | terrainStair;

const int           terrainInsubstantial = terrainFall
        | terrainLadder
        | terrainSlash
        | terrainBash;

const int           terrainTransparent = terrainSurface
        | terrainInsubstantial;


/* ===================================================================== *
   Describes an individual tile
 * ===================================================================== */

struct TileInfo {
	uint32          offset;                 // offset in tile list
	TileAttrs       attrs;                  // tile attributes

	int32 combinedTerrainMask() {
		return (1 << attrs.fgdTerrain) | (1 << attrs.bgdTerrain);
	}

	bool hasProperty(const TileProperty &tileProp) {
		return tileProp.operator()(this);
	}

	static TileInfo *tileAddress(TileID id);
	static TileInfo *tileAddress(TileID id, uint8 **imageData);
};

/* ===================================================================== *
   Describes a bank of tiles
 * ===================================================================== */

class TileBank {
public:
	uint32          _numTiles;               // number of tiles in list
	TileInfo        *_tileArray;         // variable-sized array

	TileBank() {
		_numTiles = 0;
		_tileArray = nullptr;
	}

	TileBank(Common::SeekableReadStream *stream);
	~TileBank();

	TileInfo *tile(uint16 index) {
		return &_tileArray[index];
	}
};


/* ===================================================================== *
   TileRef: This structure is used whenever a tile is positioned on a
   map or TAG. It contains the tile, the tile height, and various flags.
 * ===================================================================== */

struct TileRef {
	TileID tile;		// which tile
	uint8 flags;		// tile flags
	uint8 tileHeight;	// height of tile above platform
};

enum tileRefFlags {
	trTileTAG = (1 << 0),		// this tile part of a TAG
	trTileHidden = (1 << 1),	// tile hidden when covered
	trTileFlipped = (1 << 2),	// draw tile flipped horizontal
	trTileSensitive = (1 << 3)	// tile is triggerable (TAG only)
};

typedef TileRef *TileRefPtr, **TileRefHandle;

void drawMainDisplay();

/* ===================================================================== *
   TileCycleData: This structure is used to define continously cycling
   tiles such as waves on the ocean or a flickering torch.
 * ===================================================================== */

class TileCycleData {
public:
	int32           counter;                // cycling counter
	uint8           pad;                    // odd-byte pad
	uint8           numStates,              // number of animated states
	                currentState,           // current state of animation
	                cycleSpeed;             // speed of cycling (0=none)

	TileID          cycleList[16];        // array of tiles

	void load(Common::SeekableReadStream *stream) {
		counter = stream->readSint32LE();
		pad = stream->readByte();
		numStates = stream->readByte();
		currentState = stream->readByte();
		cycleSpeed = stream->readByte();

		for (int i = 0; i < 16; ++i)
			cycleList[i] = stream->readUint16LE();
	}
};

typedef TileCycleData
*CyclePtr,              // pointer to cycle data
* *CycleHandle;         // handle to cycle data

const int maxCycleRanges = 128;             // 128 should do for now...

/* ===================================================================== *
   ActiveTileItem: This is the base class for all of the behavioral
   objects which can be placed on a tilemap.
 * ===================================================================== */

enum ActiveItemTypes {
	activeTypeGroup = 0,
	activeTypeInstance
};

//  A pointer to the array of active item state arrays
extern byte **stateArray;

class ActiveItemList;

class ActiveItem;

#include "common/pack-start.h"

struct ActiveItemData {
	uint32 nextHashDummy;	// next item in hash chain
	uint16 scriptClassID;          // associated script object
	uint16 associationOffset;      // offset into association table
	uint8 numAssociations;        // number of associated items
	uint8 itemType;               // item type code.

	union {
		struct {
			uint16  grDataOffset;           // offset to group data
			uint8   numStates,              // number of animated states
			        uSize,                  // dimensions of group
			        vSize,
			        animArea,               // uSize * vSize
			        triggerWeight,          // sensitivity
			        pad;
			uint16  reserved0;
			uint16  reserved1;
		} group;

		struct {
			int16   groupID;                // id of defining group
			int16   u, v, h;
			uint16  stateIndex;             // for state-based anims.
			uint16  scriptFlags;
			uint16  targetU,                // U-coord of target
			        targetV;                // V-coord of target
			uint8   targetZ,                // Z-coord of target
			        worldNum;               // Add 0xf000 to get world Object ID
		} instance;
	};

	ActiveItem *aItem; // active item this ActiveItemData is a part of
} PACKED_STRUCT;

#include "common/pack-end.h"

class ActiveItem {
public:
	ActiveItem *_nextHash;		// next item in hash chain
	int _index;
	ActiveItemList *_parent;
	ActiveItemData _data;

	enum {
		activeItemLocked    = (1 << 8),     // The door is locked
		activeItemOpen      = (1 << 9),     // The door is open (not used)
		activeItemExclusive = (1 << 10)     // Script semaphore
	};

	ActiveItem(ActiveItemList *parent, int ind, Common::SeekableReadStream *stream);

	//  Return the map number of this active item
	int16 getMapNum();

	//  Return the address of an active item, given its ID
	static ActiveItem *activeItemAddress(ActiveItemID id);

	//  Return this active items ID
	ActiveItemID thisID();

	//  Return this active items ID
	ActiveItemID thisID(int16 mapNum);

	//  Return a pointer to this TAI's group
	ActiveItem *getGroup() {
		assert(_data.itemType == activeTypeInstance);
		return  activeItemAddress(ActiveItemID(getMapNum(), _data.instance.groupID));
	}

	enum BuiltInBehaviorType {
		builtInNone = 0,                    // TAG handled by SAGA
		builtInLamp,                        // TAG has lamp behavior
		builtInDoor,                        // TAG has door behavior
		builtInTransporter                  // TAG has transporter behavior
	};

	//  Return the state number of this active item instance
	uint8 getInstanceState(int16 mapNum) {
		return stateArray[mapNum][_data.instance.stateIndex];
	}

	//  Set the state number of this active item instance
	void setInstanceState(int16 mapNum, uint8 state) {
		stateArray[mapNum][_data.instance.stateIndex] = state;
	}

	uint8 builtInBehavior() {
		return (uint8)(_data.instance.scriptFlags >> 13);
	}

	//  Access to the locked bit
	bool isLocked() {
		return (bool)(_data.instance.scriptFlags & activeItemLocked);
	}
	void setLocked(bool val) {
		if (val)
			_data.instance.scriptFlags |= activeItemLocked;
		else
			_data.instance.scriptFlags &= ~activeItemLocked;
	}

	//  Access to the exclusion semaphore
	bool isExclusive() {
		return (bool)(_data.instance.scriptFlags & activeItemExclusive);
	}
	void setExclusive(bool val) {
		if (val)
			_data.instance.scriptFlags |= activeItemExclusive;
		else
			_data.instance.scriptFlags &= ~activeItemExclusive;
	}

	uint8 lockType() {
		return (uint8)_data.instance.scriptFlags;
	}

	//  ActiveItem instance methods
	bool use(ObjectID enactor);
	bool trigger(ObjectID enactor, ObjectID objID);
	bool release(ObjectID enactor, ObjectID objID);
	bool acceptLockToggle(ObjectID enactor, uint8 keyCode);

	bool inRange(const TilePoint &loc, int16 range);

	//  ActiveItem group methods
	bool use(ActiveItem *ins, ObjectID enactor);
	bool trigger(ActiveItem *ins, ObjectID enactor, ObjectID objID);
	bool release(ActiveItem *ins, ObjectID enactor, ObjectID objID);
	bool acceptLockToggle(ActiveItem *ins, ObjectID enactor, uint8 keyCode);

	bool inRange(ActiveItem *ins, const TilePoint &loc, int16 range) {
		return      loc.u >= ins->_data.instance.u - range
		            &&  loc.v >= ins->_data.instance.v - range
		            &&  loc.u <  ins->_data.instance.u + _data.group.uSize + range
		            &&  loc.v <  ins->_data.instance.v + _data.group.vSize + range;
	}

	ObjectID getInstanceContext();
	Location getInstanceLocation();

	static void playTAGNoise(ActiveItem *ai, int16 tagNoiseID);

};

typedef ActiveItem  *ActiveItemPtr,
        *ActiveItemHandle;

struct WorldMapData;

class ActiveItemList {
public:
	int _count;
	ActiveItem **_items;
	WorldMapData *_parent;

	ActiveItemList(WorldMapData *parent, int count, Common::SeekableReadStream *stream);
	~ActiveItemList();
};

#if 0

/* ===================================================================== *
   TileHitZone: This object represents a large region which overlays the
   tile map without affecting it's appearance. It does not contain any
   tiles or animation information, it is a behavioral region only.
   It can be much larger and more complex in shape than an activity group.
 * ===================================================================== */

class TileHitZone : public ActiveItem {
public:

	//  REM: Allow discontiguous regions??
	int16           numVertices;
	XArray<Point16> vertexList;

	int16 type() {
		return activeTypeHitZone;
	}
};

class ObjectClass : public ActiveItem {
public:
	// A general type of object

	int16 type() {
		return activeTypeObjectType;
	}
};

class ObjectInstance : public ActiveItem {
public:
	TileGroupID     classID;                // ID of object class

	// An instance of a specific object.

	uint16          u, v, h;                // where the instance lies
	uint8           facing;                 // which direction it's facing

	int16 type() {
		return activeTypeObject;
	}
};
#endif


/* ============================================================================ *
   TileActivityTask class
 * ============================================================================ */

//  This class handles the built-in movement of active terrain items. It
//  includes things like opening/closing doors, and toggling lamps.
//
//  Since most things in the game aren't moving at a given point, the
//  variables for simulating motion don't need to always be present.

class TileActivityTask {
	friend class    TileActivityTaskList;
	friend class    ActiveItem;

	uint8           activityType;           // open or close
	uint8           targetState;
	ActiveItem      *tai;                   // the tile activity instance
	ThreadID        script;                 // script to wake up when task done

	enum activityTypes {
		activityTypeNone,                   // no activity

		activityTypeOpen,                   // open door
		activityTypeClose,                  // close door

		activityTypeScript                  // scriptable activity
	};

	void remove();                   // tile activity task is finished.

public:

	//  Functions to create a new tile activity task.
	static void openDoor(ActiveItem &activeInstance);
	static void closeDoor(ActiveItem &activeInstance);
	static void doScript(ActiveItem &activeInstance, uint8 finalState, ThreadID id);

	static void updateActiveItems();

	static void initTileActivityTasks();

	static TileActivityTask *find(ActiveItem *tai);
	static bool setWait(ActiveItem *tai, ThreadID script);
};

/* ============================================================================ *
   TileActivityTaskList class
 * ============================================================================ */

class TileActivityTaskList {
	friend class    TileActivityTask;

public:
	Common::List<TileActivityTask *> _list;

	//  Constructor -- initial construction
	TileActivityTaskList();

	//  Reconstruct the TileActivityTaskList from an archive buffer
	TileActivityTaskList(Common::SeekableReadStream *stream);

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);

	//  Cleanup this list
	void cleanup();

	// get new tile activity task
	TileActivityTask    *newTask(ActiveItem *activeInstance);
};

void moveActiveTerrain(int32 deltaTime);

/* ===================================================================== *
   A structure to record special return values from tileSlopeHeight
 * ===================================================================== */

struct StandingTileInfo {
	TileInfo        *surfaceTile;
	ActiveItemPtr   surfaceTAG;
	TileRef         surfaceRef;
	int16           surfaceHeight;
};

/* ======================================================================= *
   Platform struct
 * ======================================================================= */

const int           maxPlatforms = 8;

struct Platform {
	uint16          height,                 // height above ground
	                highestPixel;           // tallest tile upper extent
	uint16          flags;                  // platform flags
	TileRef         tiles[kPlatformWidth][kPlatformWidth];

	void load(Common::SeekableReadStream *stream) {
		height = stream->readUint16LE();
		highestPixel = stream->readUint16LE();
		flags = stream->readUint16LE();

		for (int j = 0; j < kPlatformWidth; ++j) {
			for (int i = 0; i < kPlatformWidth; ++i) {
				tiles[j][i].tile = stream->readUint16LE();
				tiles[j][i].flags = stream->readByte();
				tiles[j][i].tileHeight = stream->readByte();
			}
		}
	}

	TileRef &getTileRef(const TilePoint p) {
		return tiles[p.u][p.v];
	}

	TileRef &getTileRef(int16 u, int16 v) {
		return tiles[u][v];
	}

	//  fetch the REAL tile terrain info
	TileInfo *fetchTile(int16           mapNum,
	                    const TilePoint &pt,
	                    const TilePoint &origin,
	                    int16           &height,
	                    int16           &trFlags);

	//  Fetch the tile and the active item it came from...
	TileInfo *fetchTAGInstance(
	    int16           mapNum,
	    const TilePoint &pt,
	    const TilePoint &origin,
	    StandingTileInfo &sti);

	//  fetch the REAL tile terrain info and image data
	TileInfo *fetchTile(int16           mapNum,
	                    const TilePoint &pt,
	                    const TilePoint &origin,
	                    uint8           **imageData,
	                    int16           &height,
	                    int16           &trFlags);

	//  Fetch the tile and image data and the active item it came from...
	TileInfo *fetchTAGInstance(
	    int16           mapNum,
	    const TilePoint &pt,
	    const TilePoint &origin,
	    uint8           **imageData,
	    StandingTileInfo &sti);

	uint16   roofRipID() {
		return (uint16)(flags & 0x0FFF);
	}
};

typedef Platform    *PlatformPtr,
        * *PlatformHandle;

enum platformFlags {
	plCutaway = (1 << 0),                   // remove when player underneath

	//  Cutaway directions: When platform is cut away, also cut
	//  away any adjacent platforms in these directions.
	plVisible = (1 << 15),                      // platform is visible
	plModified = (1 << 14),                     // platform has been changed
	plCutUPos = (1 << 13),
	plCutUNeg = (1 << 13),
	plCutVPos = (1 << 13),
	plCutVNeg = (1 << 13)
};

#ifdef OLDPLATFLAAGS
enum platformFlags {
	plCutaway = (1 << 0),                   // remove when player underneath

	//  Cutaway directions: When platform is cut away, also cut
	//  away any adjacent platforms in these directions.

	plCutUPos = (1 << 1),
	plCutUNeg = (1 << 2),
	plCutVPos = (1 << 3),
	plCutVNeg = (1 << 4),

	plVisible = (1 << 5),                   // platform is visible
	plEnabled = (1 << 6)                    // enforce platform terrain.
};
#endif

/* ======================================================================= *
   PlatformCacheEntry struct
 * ======================================================================= */

struct PlatformCacheEntry {
	uint16          platformNum,            // original platform num
	                layerNum;               // index of this plat in mt.
	MetaTileID      metaID;                 // pointer to parent metatile
	Platform        pl;                     // actual platform data

	enum {
		kPlatformCacheSize = 256
	};
};

/* ======================================================================= *
   RipTable struct
 * ======================================================================= */

//  An object roof ripping "z-buffer" type structure

typedef int16       RipTableID;

struct RipTable {
	MetaTileID  metaID;
	uint16      ripID;
	int16       zTable[kPlatformWidth][kPlatformWidth];
	int _index;

	enum {
		kRipTableSize = 25
	};

	//  Constructor
	RipTable() : metaID(NoMetaTile), ripID(0), _index(-1) {
		for (int i = 0; i < kPlatformWidth; i++)
			for (int j = 0; j < kPlatformWidth; j++)
				zTable[i][j] = 0;
	}

	//  Return a pointer to a rip table, given the rip table's ID
	static RipTable *ripTableAddress(RipTableID id);

	//  Return the rip table's ID
	RipTableID thisID();
};

typedef RipTable    *RipTablePtr;
typedef RipTableID  *RipTableIDPtr,
                    **RipTableIDHandle;

typedef uint16 metaTileNoise;

/* ======================================================================= *
   MetaTile struct
 * ======================================================================= */

//  A "Metatile" is a larger tile made up of smaller tiles.

class MetaTileList;

class MetaTile {
public:
	uint16          _highestPixel;           // more drawing optimization
	BankBits        _banksNeeded;            // which banks are needed
	uint16          _stack[maxPlatforms];    // pointer to platforms
	uint32          _properties;             // more drawing optimization
	int             _index;
	MetaTileList   *_parent;

	MetaTile(MetaTileList *parent, int ind, Common::SeekableReadStream *stream);

	//  Return a pointer to a meta tile given its ID
	static MetaTile *metaTileAddress(MetaTileID id);

	//  Return this meta tile's ID
	MetaTileID thisID(int16 mapNum);

	//  Return a pointer to the specified platform
	Platform *fetchPlatform(int16 mapNum, int16 index);

	//  Return a pointer to this metatile's current object ripping
	//  table
	RipTable *ripTable(int16 mapNum);

	//  Return a reference to this meta tile's rip table ID
	RipTableID &ripTableID(int16 mapNum);

	metaTileNoise HeavyMetaMusic();

	bool hasProperty(
	    const MetaTileProperty &metaTileProp,
	    int16 mapNum,
	    const TilePoint &mCoords) {
		return metaTileProp(this, mapNum, mCoords);
	}
};

typedef MetaTile    *MetaTilePtr,
        * *MetaTileHandle;

class MetaTileList {
public:
	int _count;
	MetaTile **_tiles;

	MetaTileList(int count, Common::SeekableReadStream *stream);
	~MetaTileList();
};

/* ===================================================================== *
   MapHeader struct
 * ===================================================================== */

struct MapHeader {
	int16           size;                   // size of map
	int16           edgeType;               // edge type of map
	uint16          *mapData;           // start of map array

	MapHeader(Common::SeekableReadStream *stream);
	~MapHeader();
};

enum mapEdgeTypes {
	edgeTypeBlack = 0,
	edgeTypeFill0,
	edgeTypeFill1,
	edgeTypeRepeat,
	edgeTypeWrap
};

typedef MapHeader   *MapPtr,
        * *MapHandle;

/* ===================================================================== *
   WorldMapData struct
 * ===================================================================== */

const uint16            metaTileVisited = (1 << 15);

struct WorldMapData {
	ObjectID            worldID;            //  The number of this map

	MapPtr              map;                //  Map data
	MetaTileList        *metaList;          //  MetaTile list
	TileRefPtr          activeItemData;     //  ActiveItem tileRefs
	ActiveItemList      *activeItemList;    //  ActiveItem list
	uint16              *assocList;          //  Associations
	RipTableIDPtr       ripTableIDList;     //  MetaTile object ripping

	ActiveItem          *instHash[513];   //  ActiveItem hash table

	int16               metaCount,          //  Number of MetaTiles
	                    activeCount;        //  Number of ActiveItems
	int16               mapSize;            //  Size of map in meta tiles
	int32               mapHeight;          //  Height of map in Y


	//  Lookup metatile on map.
	MetaTilePtr lookupMeta(TilePoint coords);

	//  Build active item instance hash table
	void buildInstanceHash();

	//  Return a pointer to an active item instance based upon the
	//  group ID and the MetaTile's coordinates
	ActiveItem *findHashedInstance(TilePoint &tp, int16 group);
};

/* ===================================================================== *
   MetaTileIterator class
 * ===================================================================== */

class MetaTileIterator {
	TilePoint   mCoords;
	TileRegion  region;

	int16       mapNum;

	bool iterate();

public:
	MetaTileIterator(int16 map, const TileRegion &reg) : mapNum(map) {
		region.min.u = reg.min.u >> kPlatShift;
		region.max.u = (reg.max.u + kPlatMask) >> kPlatShift;
		region.min.v = reg.min.v >> kPlatShift;
		region.max.v = (reg.max.v + kPlatMask) >> kPlatShift;
		region.min.z = region.max.z = 0;
	}

	MetaTile *first(TilePoint *loc = NULL);
	MetaTile *next(TilePoint *loc = NULL);

	int16 getMapNum() {
		return mapNum;
	}
};

/* ===================================================================== *
   TileIterator class
 * ===================================================================== */

class TileIterator {
	MetaTileIterator    metaIter;
	MetaTile            *mt;
	int16               platIndex;
	Platform            *platform;
	TilePoint           tCoords,
	                    origin;
	TileRegion          region,
	                    tCoordsReg;

	bool iterate();

public:
	TileIterator(int16 mapNum, const TileRegion &reg) :
		metaIter(mapNum, reg),
		region(reg) {
			mt = nullptr;
			platIndex = 0;
			platform = nullptr;
	}

	TileInfo *first(TilePoint *loc, StandingTileInfo *stiResult = NULL);
	TileInfo *next(TilePoint *loc, StandingTileInfo *stiResult = NULL);
};

/* ===================================================================== *
   Exports
 * ===================================================================== */

extern StaticTilePoint viewCenter;             // coordinates of view on map

//  These two variables define which sectors overlap the view rect.

extern uint16       rippedRoofID;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

//  Initialize map data
void initMaps();

//  Cleanup map data
void cleanupMaps();

void setCurrentMap(int mapNum);              // set which map is current

//  Initialize the platform cache
void initPlatformCache();

//  Initialize the tile activity task list
void initTileTasks();

void saveTileTasks(Common::OutSaveFile *outS);
void loadTileTasks(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the tile activity task list
void cleanupTileTasks();

TilePoint getClosestPointOnTAI(ActiveItem *TAI, GameObject *obj);

void initActiveItemStates();
void saveActiveItemStates(Common::OutSaveFile *outS);
void loadActiveItemStates(Common::InSaveFile *in);
void cleanupActiveItemStates();

void initTileCyclingStates();
void saveTileCyclingStates(Common::OutSaveFile *outS);
void loadTileCyclingStates(Common::InSaveFile *in);
void cleanupTileCyclingStates();

void initAutoMap();
void saveAutoMap(Common::OutSaveFile *outS);
void loadAutoMap(Common::InSaveFile *in, int32 chunkSize);
inline void cleanupAutoMap() { /* nothing to do */ }

//  Determine if a platform is ripped
inline bool platformRipped(Platform *pl) {
	if (rippedRoofID != 0)
		return pl->roofRipID() == rippedRoofID;

	return false;
}

//  Compute visible area in U/V coords
TilePoint XYToUV(const Point32 &pt);
void TileToScreenCoords(const TilePoint &tp, Point16 &p);
void TileToScreenCoords(const TilePoint &tp, StaticPoint16 &p);

//  Determine height of point on a tile based on four corner heights
int16 ptHeight(const TilePoint &tp, uint8 *cornerHeight);


/* --------------------------------------------------------------------- *
   Prototypes for TERRAIN.CPP moved to terrain.h
 * --------------------------------------------------------------------- */


//  Determine which roof is above object
uint16 objRoofID(GameObject *obj);
uint16 objRoofID(GameObject *obj, int16 objMapNum, const TilePoint &objCoords);

//  Determine if roof over an object is ripped
bool objRoofRipped(GameObject *obj);

//  Determine if two objects are both under the same roof
bool underSameRoof(GameObject *obj1, GameObject *obj2);

//  Determine the distance between a point and a line
uint16 lineDist(
    const TilePoint &p1,
    const TilePoint &p2,
    const TilePoint &m);

/* ============================================================================ *
   Misc prototypes
 * ============================================================================ */

//  Converts Local XY to UV coordinates
StaticTilePoint pickTilePos(Point32 pos, const TilePoint &protagPos);
StaticTilePoint pickTile(Point32 pos,
                   const TilePoint &protagPos,
                   StaticTilePoint *floor = NULL,
                   ActiveItemPtr *pickTAI = NULL);

//  Function to select a nearby site
TilePoint selectNearbySite(
    ObjectID        worldID,
    const TilePoint &startingCoords,
    int32           minDist,
    int32           maxDist,
    bool            offScreenOnly = false);     // true if we want it off-screen

void drawMetaTiles(gPixelMap &drawMap);

} // end of namespace Saga2

#endif
