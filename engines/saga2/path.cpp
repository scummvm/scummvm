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

#include "saga2/saga2.h"
#include "saga2/dispnode.h"
#include "saga2/tile.h"
#include "saga2/motion.h"
#include "saga2/player.h"
#include "saga2/cmisc.h"
#include "saga2/priqueue.h"

namespace Saga2 {

//  Defines to turn on visual debugging aids for path finder.
#define VISUAL1     0
//1 for paths considered
#define VISUAL2     0
//2 unused
#define VISUAL3     0
//3 unused
#define VISUAL4     0
//4 for fetchTileSection
#define VISUAL5     0
//5 for fetchSubMeta
#define VISUAL6     0
//6 for selectNearbySite
#define VISUAL7     0
//7 for checkPath

#if VISUAL1 || VISUAL2 || VISUAL4 || VISUAL5 || VISUAL6 || VISUAL7
void TPLine(const TilePoint &start, const TilePoint &stop);
#endif

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern WorldMapData         *mapList;

/* ===================================================================== *
   Types
 * ===================================================================== */

const int           searchCenter = 13,      // origin point of array
                    searchDiameter = searchCenter * 2;

const int           straightBaseCost    = 4,
                    diagBaseCost        = 6,
                    straightEasyCost    = straightBaseCost + 2,
                    straightNormalCost  = straightBaseCost + 4,
                    straightHardCost    = straightBaseCost + 8,
                    diagEasyCost        = diagBaseCost + 3,
                    diagNormalCost      = diagBaseCost + 6,
                    diagHardCost        = diagBaseCost + 12;

const int           subMetaSize         = 4,
                    subMetaShift        = 2,
                    subMetaMask         = subMetaSize - 1;



static StaticTilePoint tDirTable[8] = {
	{ 4,  4, 0},
	{ 0,  4, 0},
	{-4,  4, 0},
	{-4,  0, 0},
	{-4, -4, 0},
	{ 0, -4, 0},
	{ 4, -4, 0},
	{ 4,  0, 0}
};

static StaticTilePoint tDirTable2[8] = {
	{ 1,  1, 0},
	{ 0,  1, 0},
	{-1,  1, 0},
	{-1,  0, 0},
	{-1, -1, 0},
	{ 0, -1, 0},
	{ 1, -1, 0},
	{ 1,  0, 0}
};

static StaticTilePoint tDirTable3[8] = {
	{ 16,  16, 0},
	{  0,  16, 0},
	{-16,  16, 0},
	{-16,   0, 0},
	{-16, -16, 0},
	{  0, -16, 0},
	{ 16, -16, 0},
	{ 16,   0, 0}
};



/* ===================================================================== *
   The PathTileRegion class
 * ===================================================================== */

struct PathTileInfo {
	TileInfo    *surfaceTile;
	int16       surfaceHeight;

	PathTileInfo() : surfaceTile(nullptr), surfaceHeight(0) {}
};

typedef PathTileInfo        PathTilePosInfo[maxPlatforms];

typedef PathTilePosInfo     PathTilePosArray[searchDiameter + 4][searchDiameter + 4];

typedef uint8 PathSubMetaFlags[((((searchDiameter + subMetaMask + 4)
				 * (searchDiameter + subMetaMask + 4))
					>>  subMetaShift) + 7) >> 3];


//  This class manages an array containing terrain information, which
//  will be loaded on the fly as the path finder needs it.
class PathTileRegion {

	friend class PathRequest;

	int16           _mapNum;         //  The map number of the terrain data

	TilePoint       _origin,         //  The base tile coords of the array
	                _area,           //  The size of the array in tiles
	                _subMetaArea;    //  The number of submetatiles overlaying
	//  the array area
	PathTilePosInfo *_array;         //  The pointer to the array
	uint8           *_subMetaFlags;  //  A bit array indicating which
	//  submetatiles have been loaded

	void fetchSubMeta(const TilePoint &subMeta);

public:

	PathTileRegion() {
		_mapNum = 0;
		_array = nullptr;
		_subMetaFlags = nullptr;
	}

	void init(
	    int16           map,
	    const TilePoint &org,
	    const TilePoint &a,
	    PathTilePosInfo *arr,
	    uint8           *flagArray);

	void fetchTileSection(const TilePoint &org, const TilePoint &a);
	PathTilePosInfo *tilePos(const TilePoint &pos) {
		assert(pos.u >= _origin.u && (pos.u - _origin.u) < _area.u);
		assert(pos.v >= _origin.v && (pos.v - _origin.v) < _area.v);
		return &_array[(pos.u - _origin.u) * _area.v + pos.v - _origin.v];
	}
};


//  Initialize the path tile region
void PathTileRegion::init(
    int16           map,
    const TilePoint &org,
    const TilePoint &a,
    PathTilePosInfo *arr,
    uint8           *flagArray) {
	_mapNum = map;
	_origin = org;
	_area = a;
	_array = arr;
	_subMetaFlags = flagArray;

	_origin.z = _area.z = _subMetaArea.z = 0;
	_subMetaArea.u = (_area.u + (_origin.u & subMetaMask) + subMetaMask)
	                >> subMetaShift;
	_subMetaArea.v = (_area.v + (_origin.v & subMetaMask) + subMetaMask)
	                >> subMetaShift;

	//  clear all of the submetatile flags
	memset(_subMetaFlags, 0, (_subMetaArea.u * _subMetaArea.v + 7) >> 3);

	//  nullptr the tile pointers in the array
	int16           _arraySize = _area.u * _area.v;
	PathTilePosInfo *tiPtr = _array;
	for (; _arraySize > 0; _arraySize--, tiPtr++) {
		PathTilePosInfo &ptpi = *tiPtr;
		for (int i = 0; i < maxPlatforms; i++)
			ptpi[i].surfaceTile = nullptr;
	}
}


//  This function will load the submetatiles which overlay a specified
//  region into the array, if those submetatiles have not already been
//  loaded.
void PathTileRegion::fetchTileSection(const TilePoint &org,
                                      const TilePoint &a) {
	int16       flagIndex;
	int       u, v;
	TilePoint   secSubMetaOrigin,
	            secSubMetaArea,
	            relSubMeta;

#if VISUAL4
	TilePoint   pt1, pt2;
	pt1 = pt2 = org << kTileUVShift;
	pt1.z = pt2.z = 0;
	pt2.u += (a.u << kTileUVShift);
	TPLine(pt1, pt2);
	pt1.u = pt2.u;
	pt1.v += (a.v << kTileUVShift);
	TPLine(pt1, pt2);
	pt2.v = pt1.v;
	pt2.u -= (a.u << kTileUVShift);
	TPLine(pt1, pt2);
	pt1.u = pt2.u;
	pt1.v -= (a.v << kTileUVShift);
	TPLine(pt1, pt2);
#endif

	//  Determine the origin and area of the specified tile section
	//  in submetatile coordinates
	secSubMetaOrigin.u = (org.u >> subMetaShift);
	secSubMetaOrigin.v = (org.v >> subMetaShift);
	secSubMetaArea.u = (a.u + (org.u & subMetaMask) + subMetaMask)
	                   >> subMetaShift;
	secSubMetaArea.v = (a.v + (org.v & subMetaMask) + subMetaMask)
	                   >> subMetaShift;

	for (u = 0; u < secSubMetaArea.u; u++) {
		relSubMeta.u = secSubMetaOrigin.u - (_origin.u >> subMetaShift) + u;
		relSubMeta.v = secSubMetaOrigin.v - (_origin.v >> subMetaShift);
		flagIndex = relSubMeta.u * _subMetaArea.v + relSubMeta.v;

		for (v = 0; v < secSubMetaArea.v; v++, flagIndex++) {
			//  Check the submetatile flag in the bit array
			if (!(_subMetaFlags[flagIndex >> 3] & (1 << (flagIndex & 7)))) {
				//  Load the submetatile and set its flag
				fetchSubMeta(TilePoint(secSubMetaOrigin.u + u,
				                       secSubMetaOrigin.v + v,
				                       0));
				_subMetaFlags[flagIndex >> 3] |= 1 << (flagIndex & 7);
			}
		}
	}
}


//  This function will load a submeta tile in the array
void PathTileRegion::fetchSubMeta(const TilePoint &subMeta) {
	WorldMapData    *map = &mapList[_mapNum];

	TilePoint       mCoords;
	MetaTile        *mt;

	mCoords.u = subMeta.u >> 1;
	mCoords.v = subMeta.v >> 1;
	mCoords.z = 0;
	mt = map->lookupMeta(mCoords);

#if VISUAL5
	TilePoint   pt1, pt2;
	pt1 = pt2 = subMeta << (subMetaShift + kTileUVShift);
	pt1.z = pt2.z = 0;
	pt2.u += (subMetaSize << kTileUVShift);
	TPLine(pt1, pt2);
	pt1.u = pt2.u;
	pt1.v += (subMetaSize << kTileUVShift);
	TPLine(pt1, pt2);
	pt2.v = pt1.v;
	pt2.u -= (subMetaSize << kTileUVShift);
	TPLine(pt1, pt2);
	pt1.u = pt2.u;
	pt1.v -= (subMetaSize << kTileUVShift);
	TPLine(pt1, pt2);
#endif

	if (mt) {
		TileRegion  tileReg;
		TilePoint   offset;

		tileReg.min.u = (subMeta.u << subMetaShift) - _origin.u;
		offset.u =  tileReg.min.u + subMetaSize < _area.u
		            ?   subMetaSize
		            :   _area.u - tileReg.min.u;
		tileReg.min.v = (subMeta.v << subMetaShift) - _origin.v;
		offset.v =  tileReg.min.v + subMetaSize < _area.v
		            ?   subMetaSize
		            :   _area.v - tileReg.min.v;

		if (tileReg.min.u < 0) {
			offset.u += tileReg.min.u;
			tileReg.min.u = 0;
		}
		if (tileReg.min.v < 0) {
			offset.v += tileReg.min.v;
			tileReg.min.v = 0;
		}

		//  Compute tile region relative to metatile
		tileReg.min.u = (tileReg.min.u + _origin.u) & kPlatMask;
		tileReg.max.u = tileReg.min.u + offset.u;
		tileReg.min.v = (tileReg.min.v + _origin.v) & kPlatMask;
		tileReg.max.v = tileReg.min.v + offset.v;

		assert(tileReg.max.u <= kPlatformWidth);
		assert(tileReg.max.v <= kPlatformWidth);

		//  Compute the offset of base tile in metatile to origin
		offset.u = ((subMeta.u >> 1) << kPlatShift) - _origin.u;
		offset.v = ((subMeta.v >> 1) << kPlatShift) - _origin.v;

		for (int i = 0; i < maxPlatforms; i++) {
			uint16      tpFlags = 0;
			Platform    *p;
			int       u, v;
			TileRef     *tr;
			int16       height;

			if ((p = mt->fetchPlatform(_mapNum, i)) == nullptr)
				continue;

			if (!(p->flags & plVisible)) continue;

			for (u = tileReg.min.u; u < tileReg.max.u; u++) {
				PathTilePosInfo *arrRow = &_array[(u + offset.u) * _area.v];

				assert(u >= 0);
				assert(u < kPlatformWidth);

				for (v = tileReg.min.v; v < tileReg.max.v; v++) {
					int16   flagIndex = ((u & subMetaMask) << subMetaShift) | (v & subMetaMask);

					assert(v >= 0);
					assert(v < kPlatformWidth);

					if (!(tpFlags & (1 << flagIndex))) {
						tpFlags |= (1 << flagIndex);

						tr = &p->tiles[u][v];
						height = tr->tileHeight << 3;

						if (tr->flags & trTileTAG) {
							ActiveItem  *groupItem,
							            *instanceItem;
							int16       state = 0;
							int16       tagU, tagV;
							int            tempU, tempV;
							TilePoint   absPos;
							TileRegion  subMetaTag;
							TileRef     *stateData;

							assert((uint16)tr->tile <= activeItemIndexNullID);
							groupItem = ActiveItem::activeItemAddress(
							                ActiveItemID(_mapNum, tr->tile));

							tagU = u - ((tr->flags >> 1) & 0x07);
							tagV = v - ((tr->flags >> 4) & 0x07);

							subMetaTag.min.u = tagU;
							subMetaTag.max.u = tagU + groupItem->_data.group.uSize;
							subMetaTag.min.v = tagV;
							subMetaTag.max.v = tagV + groupItem->_data.group.vSize;

							if (subMetaTag.min.u < tileReg.min.u)
								subMetaTag.min.u = tileReg.min.u;
							if (subMetaTag.max.u > tileReg.max.u)
								subMetaTag.max.u = tileReg.max.u;
							if (subMetaTag.min.v < tileReg.min.v)
								subMetaTag.min.v = tileReg.min.v;
							if (subMetaTag.max.v > tileReg.max.v)
								subMetaTag.max.v = tileReg.max.v;

							//  Abspos is the absolute position of the
							//  group on the tile map.

							absPos.u = (mCoords.u << kPlatShift) | tagU;
							absPos.v = (mCoords.v << kPlatShift) | tagV;
							absPos.z = height;

							//  Look up the group instance in the hash.
							instanceItem = map->findHashedInstance(absPos, tr->tile);
							if (instanceItem) state = instanceItem->getInstanceState(_mapNum);

							stateData = &(map->activeItemData)[
							                groupItem->_data.group.grDataOffset
							                +   state * groupItem->_data.group.animArea];

							for (tempU = subMetaTag.min.u; tempU < subMetaTag.max.u; tempU++) {
								TileRef         *rowData = &stateData[(tempU - tagU) * groupItem->_data.group.vSize];
								PathTilePosInfo *tempArrRow = &_array[(tempU + offset.u) * _area.v];

								for (tempV = subMetaTag.min.v; tempV < subMetaTag.max.v; tempV++) {
									flagIndex = ((tempU & subMetaMask) << subMetaShift) + (tempV & subMetaMask);

									tpFlags |= (1 << flagIndex);

									if (instanceItem) tr = &rowData[tempV - tagV];
#if DEBUG
									else {
										static  TileRef dummyRef = { 1, 0, 0 };
										tr = &dummyRef;
									}
#endif
									tempArrRow[tempV + offset.v][i].surfaceTile =
									    TileInfo::tileAddress(tr->tile);
									tempArrRow[tempV + offset.v][i].surfaceHeight =
									    height + (tr->tileHeight << 3);
								}
							}
						} else {
							arrRow[v + offset.v][i].surfaceTile =
							    TileInfo::tileAddress(tr->tile);
							arrRow[v + offset.v][i].surfaceHeight =
							    height;
						}
					}
				}
			}
		}
	}
}


/* ===================================================================== *
   The PathArray and QueueItem types
 * ===================================================================== */

//  Represents a single cell on the path array

struct PathCell {
	uint8           direction;              // direction into cell
	int8            platformDelta;          // platform difference of cell
	int16           height;                 // height above floor
	int16           cost;                   // cost to get there
};

//  Virtual array of cells representing the path region we will search.
class PathArray {
public:

	enum {
		chunkTileDiameter = 4,
		regionChunkDiameter = (searchDiameter + chunkTileDiameter - 1) / chunkTileDiameter
	};

private:
	//  Search sub-region class.
	struct PathArrayChunk {
		uint16      mask;       //  Mask indicating which cells in chunk are
		//  allocated
		//  The cell array
		PathCell    array[chunkTileDiameter][chunkTileDiameter];

		PathArrayChunk() : mask(0) {}
	};

	//  Master array of chunk pointers
	PathArrayChunk  *_array[maxPlatforms][regionChunkDiameter][regionChunkDiameter];
public:

	//  Exception class
	class CellAllocationFailure {};

	//  Constructor
	PathArray();

	//  Destructor
	~PathArray();

	//  Make a new cell or access an existing cell.  If the specified
	//  cell already exists *newCell will be set to false, else it will
	//  be true.  If it fails to allocate a new cell it will throw
	//  a CellAllocationFailure.
	PathCell *makeCell(int plat, int uCoord, int vCoord, bool *newCell);

	//  Get a pointer to an existing cell.  If the specified cell has
	//  not been created, it will return nullptr.
	PathCell *getCell(int plat, int uCoord, int vCoord);

	//  Delete an existing cell
	void deleteCell(int plat, int uCoord, int vCoord);

	//  Delete all existing cells
	void reset();
};

//  Constructor
PathArray::PathArray() {
	int     plat, chunkU, chunkV;

	for (plat = 0; plat < maxPlatforms; plat++) {
		for (chunkU = 0; chunkU < regionChunkDiameter; chunkU++) {
			for (chunkV = 0; chunkV < regionChunkDiameter; chunkV++)
				_array[plat][chunkU][chunkV] = nullptr;
		}
	}
}

//  Destructor
PathArray::~PathArray() {
	reset();
}


//  Make a new cell or access an existing cell.  If the specified
//  cell already exists *newCell will be set to false, else it will
//  be true.  If it fails to allocate a new cell it will throw
//  a CellAllocationFailure.
PathCell *PathArray::makeCell(int plat, int uCoord, int vCoord, bool *newCell) {
	assert(plat >= 0 && plat < maxPlatforms);
	assert(uCoord >= 0 && uCoord < searchDiameter);
	assert(vCoord >= 0 && vCoord < searchDiameter);
	assert(newCell != nullptr);

	//  Compute the chunk coords
	int             chunkUCoord = uCoord >> 2,
	                chunkVCoord = vCoord >> 2;

	//  Get a pointer to the chunk pointer in the array
	PathArrayChunk  **chunkPtrPtr = &_array[plat][chunkUCoord][chunkVCoord];

	//  Get existing chunk or allocate a new one
	if (*chunkPtrPtr != nullptr || (*chunkPtrPtr = new PathArrayChunk) !=  nullptr) {
		PathArrayChunk  *chunkPtr = *chunkPtrPtr;
		uint16          chunkCellMask;

		//  Compute the coordinates of the cell relative to the chunk
		uCoord &= chunkTileDiameter - 1;
		vCoord &= chunkTileDiameter - 1;

		//  Compute the bit mask of the cell within the chunk
		chunkCellMask = 1 << ((uCoord << 2) | vCoord);

		//  Determine if this is a new cell
		*newCell = (chunkPtr->mask & chunkCellMask) == 0;

		//  Mark the cell as allocated
		chunkPtr->mask |= chunkCellMask;

		return &chunkPtr->array[uCoord][vCoord];
	} else {
		//  Failed to allocate cell so throw exception
		error("Cell Allocation failure");
	}
}

//  Get a pointer to an existing cell.  If the specified cell has
//  not been created, it will return nullptr.
PathCell *PathArray::getCell(int plat, int uCoord, int vCoord) {
	assert(plat >= 0 && plat < maxPlatforms);
	assert(uCoord >= 0 && uCoord < searchDiameter);
	assert(vCoord >= 0 && vCoord < searchDiameter);

	//  Compute the chunk coords
	int             chunkUCoord = uCoord >> 2,
	                chunkVCoord = vCoord >> 2;
	uint16          chunkCellMask;

	PathArrayChunk  *chunkPtr = _array[plat][chunkUCoord][chunkVCoord];

	if (chunkPtr == nullptr) return nullptr;

	//  Compute the coordinates of the cell relative to the chunk
	uCoord &= chunkTileDiameter - 1;
	vCoord &= chunkTileDiameter - 1;

	//  Compute the bit mask of the cell within the chunk
	chunkCellMask = 1 << ((uCoord << 2) | vCoord);

	//  Determine if cell has been allocated
	if ((chunkPtr->mask & chunkCellMask) == 0)
		return nullptr;

	return &chunkPtr->array[uCoord][vCoord];
}

void PathArray::deleteCell(int plat, int uCoord, int vCoord) {
	assert(plat >= 0 && plat < maxPlatforms);
	assert(uCoord >= 0 && uCoord < searchDiameter);
	assert(vCoord >= 0 && vCoord < searchDiameter);

	//  Compute the chunk coords
	int             chunkUCoord = uCoord >> 2,
	                chunkVCoord = vCoord >> 2;
	uint16          chunkCellMask;

	PathArrayChunk  *chunkPtr = _array[plat][chunkUCoord][chunkVCoord];

	if (chunkPtr == nullptr)
		return;

	//  Compute the coordinates of the cell relative to the chunk
	uCoord &= chunkTileDiameter - 1;
	vCoord &= chunkTileDiameter - 1;

	//  Compute the bit mask of the cell within the chunk
	chunkCellMask = 1 << ((uCoord << 2) | vCoord);

	//  Clear the bit
	chunkPtr->mask &= ~chunkCellMask;
}

//  Delete all existing cells
void PathArray::reset() {
	int     plat, chunkU, chunkV;

	for (plat = 0; plat < maxPlatforms; plat++) {
		for (chunkU = 0; chunkU < regionChunkDiameter; chunkU++) {
			for (chunkV = 0; chunkV < regionChunkDiameter; chunkV++) {
				PathArrayChunk      **chunkPtrPtr;

				chunkPtrPtr = &_array[plat][chunkU][chunkV];

				if (*chunkPtrPtr != nullptr) {
					delete *chunkPtrPtr;
					*chunkPtrPtr = nullptr;
				}
			}
		}
	}
}

//  Represents an entry in the queue

struct QueueItem {
	int16           z;                      // height over terrain
	uint8           u, v;                   // relative coords of cell
	uint8           platform;               // platform number of cell
	Direction       direction;              // direction out of cell
	uint8           pad;
	int16           cost;                   // Cost to get to this cell

	QueueItem() {
		z = 0;
		u = v = 0;
		platform = 0;
		pad = 0;
		cost = 0;
		direction = 0;
	}

	operator int() {
		return cost;
	}
};


/* ===================================================================== *
   The MaskComputer class
 * ===================================================================== */

//  Represents the subtile mask of an object at a single point
struct PointMask {
	TilePoint   size;
	TilePoint   offset;
	uint16      mask[16];
};


//  Represents the subtile masks of each point on a path in a single
//  direction
class DirMask {
	friend class DirMaskGroup;

	PointMask _pathPt[4];

public:
	PointMask &operator[](int16 index) {
		return _pathPt[index];
	}
};


//  Represents the subtile masks of each path in all eight direction
class DirMaskGroup {
	friend class MaskComputer;

	uint8   _crossSection;
	DirMask _dMask[8];

	void computeMask(uint8 objSection);

public:
	DirMaskGroup() : _crossSection(0) {}
	DirMask &operator[](int16 index) {
		return _dMask[index];
	}
};


//  The class which will compute all of the subtile masks for an object
class MaskComputer {

private:
	DirMaskGroup    _array[8],
	                *_ptrArray[8];
	int16           _arraySize;

public:
	MaskComputer() : _arraySize(0) {
		for (int i = 0; i < 8; i++)
			_ptrArray[i] = nullptr;
	}

	DirMaskGroup *computeMask(uint8 objSection);
};


//  return a word with the bit (minBit) to bit (maxBit-1) all set,
//  and all other bits reset.

inline uint32 makeMask16(int minBit, int maxBit) {
	return (1 << maxBit) - (1 << minBit);
}

void DirMaskGroup::computeMask(uint8 objSection) {
	bool        diagExpand;
	int       dir;
	struct {
		int16   min;
		int16   max;
	}           area;

	_crossSection = objSection;

	//  Calculate the area in subtiles the object occupies.  Since U and
	//  V coordinates will alway equal each other, there is no need to
	//  calculate both.
	area.min = ((kTileUVSize / 2) - objSection) >> kSubTileShift;
	area.max = ((kTileUVSize / 2) + objSection + kSubTileMask) >> kSubTileShift;

	//  Determine if the cross section is wide enough that the diaginal
	//  masks need to be expanded outward one subtile
	diagExpand = (objSection - 1) & 0x2;

	for (dir = 0; dir < 8; dir++) {
		int           ptNum;
		TileRegion  baseMaskArea;

		//  Calculate the base mask from which all of the point masks
		//  will be derived
		baseMaskArea.min.u = baseMaskArea.min.v = area.min;
		baseMaskArea.max.u = baseMaskArea.max.v = area.max;

		if (!(dir & 0x1) && diagExpand) {
			switch (dir >> 1) {
			case 0:
				baseMaskArea.min.u--;
				baseMaskArea.max.v--;
				break;
			case 1:
				baseMaskArea.max.u++;
				baseMaskArea.min.v--;
				break;
			case 2:
				baseMaskArea.max.u++;
				baseMaskArea.max.v++;
				break;
			case 3:
				baseMaskArea.min.u--;
				baseMaskArea.max.v++;
			}
		}

		for (ptNum = 0; ptNum < 4; ptNum++) {
			int       u,
			          v;
			TileRegion  ptMaskArea;
			uint16      tempMask[16];
			PointMask   *ptMask = &_dMask[dir]._pathPt[ptNum];

			//  Compute the point mask area
			ptMaskArea.min = baseMaskArea.min + tDirTable2[dir] * (ptNum + 1);
			ptMaskArea.max = baseMaskArea.max + tDirTable2[dir] * (ptNum + 1);

			ptMask->offset.u = ptMaskArea.min.u >> kTileSubShift;
			ptMask->offset.v = ptMaskArea.min.v >> kTileSubShift;

			ptMaskArea.max.u -= ptMaskArea.min.u & ~kSubTileMask;
			ptMaskArea.min.u &= kSubTileMask;
			ptMaskArea.max.v -= ptMaskArea.min.v & ~kSubTileMask;
			ptMaskArea.min.v &= kSubTileMask;

			ptMask->size.u = (ptMaskArea.max.u + kTileSubMask) >> kTileSubShift;
			ptMask->size.v = (ptMaskArea.max.v + kTileSubMask) >> kTileSubShift;

			memset(tempMask, 0, sizeof(tempMask));

			uint16  vMask = makeMask16(ptMaskArea.min.v, ptMaskArea.max.v);
			for (u = ptMaskArea.min.u; u < ptMaskArea.max.u; u++)
				tempMask[u] = vMask;

			for (u = 0; u < ptMask->size.u; u++) {
				uint16  *srcMask = &tempMask[u << 2];
				uint16  *destMask = &ptMask->mask[u << 2];

				for (v = 0; v < ptMask->size.v; v++) {
					switch (v) {
					case 0:
						destMask[0] = (srcMask[0] & 0x000f)       |
						                (srcMask[1] & 0x000f) <<  4 |
						                (srcMask[2] & 0x000f) <<  8 |
						                (srcMask[3] & 0x000f) << 12;
						break;

					case 1:
						destMask[1] = (srcMask[0] & 0x00f0) >>  4 |
						                (srcMask[1] & 0x00f0)       |
						                (srcMask[2] & 0x00f0) <<  4 |
						                (srcMask[3] & 0x00f0) <<  8;
						break;

					case 2:

						destMask[2] = (srcMask[0] & 0x0f00) >>  8 |
						                (srcMask[1] & 0x0f00) >>  4 |
						                (srcMask[2] & 0x0f00)       |
						                (srcMask[3] & 0x0f00) <<  4;
						break;

					case 3:
						destMask[3] = (srcMask[0] & 0xf000) >> 12 |
						                (srcMask[1] & 0xf000) >>  8 |
						                (srcMask[2] & 0xf000) >>  4 |
						                (srcMask[3] & 0xf000);
					}
				}
			}
		}
	}
}

DirMaskGroup *MaskComputer::computeMask(uint8 _crossSection) {
	DirMaskGroup    *maskGroup;
	int             i;

	//  Check if this mask group has already been computed
	for (i = 0; i < _arraySize; i++) {
		maskGroup = _ptrArray[i];

		if (maskGroup->_crossSection == _crossSection) {
			//  This mask group has already been computed
			if (i > 0) {
				//  Move the reference to this mask group up one position
				_ptrArray[i] = _ptrArray[i - 1];
				_ptrArray[i - 1] = maskGroup;
			}

			return maskGroup;
		}
	}

	if (_arraySize < ARRAYSIZE(_array)) {
		//  Allocate a new place for this mask group
		maskGroup = _ptrArray[_arraySize] = &_array[_arraySize];
		_arraySize++;
	} else
		//  Discard last referenced mask group in array
		maskGroup = _ptrArray[ARRAYSIZE(_array) - 1];

	//  Compute the new group of masks
	maskGroup->computeMask(_crossSection);

	return maskGroup;
}


uint32 tileTerrain(
    PathTilePosInfo *tilePos,
    int16           mask,
    int16           minZ,
    int16           maxZ) {
	uint32          terrain = 0;

	for (int i = 0; i < maxPlatforms; i++) {
		int32           height, tileMinZ, tileMaxZ;
		TileInfo        *ti;

		ti = (*tilePos)[i].surfaceTile;

		if (ti) {
			height = (*tilePos)[i]. surfaceHeight;
			TileAttrs &attrs = ti->attrs;
			tileMinZ = tileMaxZ = height;
			int32   combinedMask = ti->combinedTerrainMask();

			if (combinedMask & terrainRaised)
				tileMaxZ += attrs.terrainHeight;
			if (combinedMask & terrainWater)
				tileMinZ -= attrs.terrainHeight;

			if (tileMinZ <  maxZ
			        &&  tileMaxZ >= minZ) {
				uint32  terrainResult = 0,
				        tileFgdTerrain = (1 << attrs.fgdTerrain),
				        tileBgdTerrain = (1 << attrs.bgdTerrain);

				//  If only checking the top of raised terrain treat it
				//  as if it were normal terrain.
				if (minZ + kMaxStepHeight >= tileMaxZ) {
					if (tileFgdTerrain & terrainSupportingRaised)
						tileFgdTerrain = terrainNormal;
					if (tileBgdTerrain & terrainSupportingRaised)
						tileBgdTerrain = terrainNormal;
				}

				if (mask & attrs.terrainMask)
					terrainResult |= tileFgdTerrain;

				if (mask & ~attrs.terrainMask)
					terrainResult |= tileBgdTerrain;

				//  This prevents actors from walking through
				//  catwalks and other surfaces which have no bottom.

				if ((terrainResult & terrainSolidSurface)
				        &&  height > minZ + kMaxStepHeight) {
					terrainResult |= terrainStone;
				}

				terrain |= terrainResult;
			}
		}
	}
	return terrain;
}


const uint32 terrainSink = terrainInsubstantial | terrainSupportingRaised | terrainWater;

int16 tileSlopeHeight(
    PathTileRegion  &tileArr,
    const TilePoint &pt,
    GameObject      *obj,
    PathTileInfo    *ptiResult,
    uint8           *platformResult) {
	//  Calculate coordinates of tile and subtile
	TilePoint       tileCoords = pt >> kTileUVShift,
	                subTile(
	                    (pt.u >> kSubTileShift) & kSubTileMask,
	                    (pt.v >> kSubTileShift) & kSubTileMask,
	                    0);

	PathTileInfo    highestTile,
	                lowestTile;
	int             supportHeight,
	                highestSupportHeight,
	                lowestSupportHeight;
	uint32
	highestSupportPlatform = 0,
	lowestSupportPlatform = 0;
	bool            highestTileFlag,
	                lowestTileFlag;
	PathTilePosInfo &tilePosInfo = *tileArr.tilePos(tileCoords);

	highestSupportHeight = -100;
	lowestSupportHeight = 0x7FFF;
	highestTileFlag = false;
	lowestTileFlag = false;
	int objProtHt = obj->proto()->height;

	//  Search each platform until we find a tile which is under
	//  the character.

	for (int i = 0; i < maxPlatforms; i++) {
		PathTileInfo    *pti = ((PathTileInfo *)(&tilePosInfo)) + i; // &tilePosInfo[i];
		TileInfo        *ti = pti->surfaceTile;

		if (ti) {
			int     height = pti->surfaceHeight;
			TileAttrs &attrs = ti->attrs;
			int16   tileBase = height;
			int32 subTileTerrain =
			    attrs.testTerrain(calcSubTileMask(subTile.u,
			                                      subTile.v));
			if (subTileTerrain & terrainSink) {
				if (subTileTerrain & terrainInsubstantial)
					continue;
				else if (subTileTerrain & terrainSupportingRaised)
					// calculate height of raised surface
					supportHeight = height +
					                attrs.terrainHeight;
				else {
					// calculate depth of water
					supportHeight = height -
					                attrs.terrainHeight;
					tileBase = supportHeight;
				}
			} else
				// calculate height of unraised surface
				supportHeight = height +
				                ptHeight(TilePoint(pt.u & kTileUVMask,
				                                   pt.v & kTileUVMask,
				                                   0),
				                         attrs.cornerHeight);

			//  See if the tile is a potential supporting surface
			if (tileBase < pt.z + objProtHt
			        &&  supportHeight >= highestSupportHeight
			        && (ti->combinedTerrainMask() & (terrainSurface | terrainRaised))) {
				highestTileFlag = true;
				highestTile = *pti;
				highestSupportHeight = supportHeight;
				highestSupportPlatform = i;
			} else if (!highestTileFlag &&
			           supportHeight <= lowestSupportHeight &&
			           (ti->combinedTerrainMask() & (terrainSurface | terrainRaised))) {
				lowestTileFlag = true;
				lowestTile = *pti;
				lowestSupportHeight = supportHeight;
				lowestSupportPlatform = i;
			}
		}
	}

	if (highestTileFlag) {
		if (ptiResult) *ptiResult = highestTile;
		if (platformResult) *platformResult = highestSupportPlatform;
		return highestSupportHeight;
	}
	if (lowestTileFlag) {
		if (ptiResult) *ptiResult = lowestTile;
		if (platformResult) *platformResult = lowestSupportPlatform;
		return lowestSupportHeight;
	}

	if (ptiResult) {
		ptiResult->surfaceTile = nullptr;
		ptiResult->surfaceHeight = 0;
	}
	if (platformResult) *platformResult = 0;

	return 0;
}

/* ===================================================================== *
   PathRequest Class
 * ===================================================================== */

enum PathResult {
	pathNotDone,
	pathDone,
	pathAborted
};

//  This if the base class for all PathRequests
class PathRequest {
	friend void addPathRequestToQueue(PathRequest *pr);

protected:
	Actor           *_actor;                 // actor path applies to.
	int16           _smartness;              // how intelligent?
	MotionTask      *_mTask;                 // which motion task started this
	uint8           _flags;

	enum pathFlags {
		aborted     = (1 << 0),             // path request has been aborted

		completed   = (1 << 1),             // pathfinder has found best path
		run         = (1 << 2)
	};

	enum {
		kPathSize = 16
	};

	//  These static members are initialized when the path request
	//  becomes the current active request being serviced.
	static TilePoint *_path;
	static int16      _pathLength;

	static StaticTilePoint  _baseCoords,
	       _baseTileCoords,
	       _centerPt,       // The current center coordinates
	       _bestLoc;        // The best cell coordinates,
	// currently visited

	static uint8            _centerPlatform,
	       _bestPlatform;

	static int16            _fetchRadius;

	static int32            _firstTick,
	       _lastTick,
	       _timeLimit;

	static DirMaskGroup     *_dirMasks;

	//  Calculates the center point given the base coordinate of the
	//  cell array and a queue item which contains cell coordinates.
	static void calcCenterPt(const TilePoint &baseTileCoords_, const QueueItem &qi) {
		_centerPt.u = ((baseTileCoords_.u + qi.u) << kTileUVShift)
		             +   kTileUVSize / 2;
		_centerPt.v = ((baseTileCoords_.v + qi.v) << kTileUVShift)
		             +   kTileUVSize / 2;
		_centerPt.z = qi.z;

		_centerPlatform = qi.platform;
	}

	//  Constructor
	PathRequest(Actor *a, int16 howSmart);

public:
	static PathTileRegion   *_tileArray;

	virtual ~PathRequest() {
		if (_path)
			delete[] _path;

		_path = nullptr;
	}

	void requestAbort() {
		_flags |= aborted;
	}

	virtual void initialize();
	virtual void finish();           // completion method
	virtual void abortReq();                // abnormal termination method

	PathResult findPath();

	//  Set and evaluate a new center location.
	virtual bool setCenter(
	    const TilePoint &_baseTileCoords,
	    const QueueItem &qi) = 0;

	//  Determine if path request will allow a move to the specified
	//  location
	virtual bool validMove(const TilePoint &testPt) = 0;

	//  Virtual function for evaluating the cost of moving on stairs.
	virtual int16 evaluateStairs(
	    const TilePoint &testPt,
	    Direction moveDir,
	    Direction stairDir,
	    int16 baseAltitute,
	    int16 upperAltitude) = 0;

	//  Virtual function for evaluating the cost of moving from the
	//  current center location to the specified coordinates.
	virtual int16 evaluateMove(const TilePoint &testPt, uint8 testPlatform) = 0;

	// NEW added by Evan 12/3
	virtual bool _timeLimitExceeded();

};

/* ===================================================================== *
   DestinationPathRequest Class
 * ===================================================================== */

//  This class is used to request a path leading to a specified
//  _destination.
class DestinationPathRequest : public PathRequest {
protected:
	TilePoint           _destination;    // The _destination of the path
	uint8               _destPlatform;   // The _destination platform

	//  These static members are initialized when the path request
	//  becomes the current active request being serviced.
	static StaticTilePoint _targetCoords;   // The current _destination coordinates
	// quantized to the nearest tile
	// center.
	static uint8        _targetPlatform;
	static int16        _bestDist,       // The distance from the target of
	       // the best cell visited so far.
	       _centerCost;     // The distance from the target of
	// the current center coordinates.
public:
	DestinationPathRequest(Actor *a, int16 howSmart);

	//  Initialize the static data members for this path request.
	void initialize() override;

	//  Set and evaluate a new center location.
	bool setCenter(
	    const TilePoint &_baseTileCoords,
	    const QueueItem &qi) override;

	bool validMove(const TilePoint &testPt) override;

	//  Evaluate the cost of travelling on these stairs
	int16 evaluateStairs(
	    const TilePoint &testPt,
	    Direction moveDir,
	    Direction stairDir,
	    int16 baseAltitude,
	    int16 upperAltitude) override;

	//  Evaluate the cost of the specified move
	int16 evaluateMove(const TilePoint &testPt, uint8 testPlatform) override;
};

/* ===================================================================== *
   WanderPathRequest Class
 * ===================================================================== */

class WanderPathRequest : public PathRequest {
protected:
	bool                _tethered;       //  Flag indicating if there is a
	//  tether on this path

	//  Tether coordinates
	int16               _tetherMinU,
	                    _tetherMinV,
	                    _tetherMaxU,
	                    _tetherMaxV;

	//  These static members are initialized when the path request
	//  becomes the current active request being serviced.
	static StaticTilePoint _startingCoords; // The actor's location at the
	// beginning of the service.
	static int16        _bestDist,       // The distance from the target of
	       // the best cell visited so far.
	       _centerCost;     // The distance from the target of
	// the current center coordinates.

public:
	//  Constructor
	WanderPathRequest(Actor *a, int16 howSmart);

	//  Initialize the static data members
	void initialize() override;

	//  Set and evaluate a new center location.
	bool setCenter(
	    const TilePoint &_baseTileCoords,
	    const QueueItem &qi) override;

	//  Determine if point is within the tether region if there is a
	//  tether
	bool validMove(const TilePoint &testPt) override;

	//  Evaluate the cost of moving on the specified stairs.
	int16 evaluateStairs(
	    const TilePoint &testPt,
	    Direction moveDir,
	    Direction stairDir,
	    int16 baseAltitude,
	    int16 upperAltitude) override;

	//  Evaluate the cost of moving from the current center location
	//  to the specified location.
	int16 evaluateMove(const TilePoint &testPt, uint8 testPlatform) override;
};

/* ===================================================================== *
                            Globals
 * ===================================================================== */

PathRequest                 *currentRequest = nullptr;

static PathTilePosArray     *pathTileArray;
static PathSubMetaFlags     _subMetaFlags;

static MaskComputer         *maskComp;

static PriorityQueue<QueueItem, 192> *queue;
static PathArray            *cellArray;

static TileRegion           *objectVolumeArray;

struct VolumeLookupNode {
	VolumeLookupNode        *next;
	TileRegion              *volume;
};

static VolumeLookupNode     volumeLookupNodePool[256];
static VolumeLookupNode     *volumeLookupTable[searchDiameter][searchDiameter];

TilePoint *PathRequest::_path = nullptr;
int16           PathRequest::_pathLength;

StaticTilePoint PathRequest::_baseCoords = {0, 0, 0},
                PathRequest::_baseTileCoords = {0, 0, 0},
                PathRequest::_centerPt = {0, 0, 0},      // The current center coordinates
                PathRequest::_bestLoc = {0, 0, 0};       // The best cell coordinates,
// currently visited
uint8           PathRequest::_centerPlatform,
                PathRequest::_bestPlatform;

int16           PathRequest::_fetchRadius;

int32           PathRequest::_firstTick,
                PathRequest::_timeLimit;

DirMaskGroup    *PathRequest::_dirMasks = nullptr;

PathTileRegion  *PathRequest::_tileArray = nullptr;

StaticTilePoint DestinationPathRequest::_targetCoords = {0, 0, 0};
uint8           DestinationPathRequest::_targetPlatform;
int16           DestinationPathRequest::_bestDist,
                DestinationPathRequest::_centerCost;

StaticTilePoint WanderPathRequest::_startingCoords = {0, 0, 0};
int16           WanderPathRequest::_bestDist,
                WanderPathRequest::_centerCost;

/* ===================================================================== *
   PathQueue member functions
 * ===================================================================== */

static void push(
    const   TilePoint &tp,
    uint8   platform,
    int     cost,
    int     direction,
    int8    platformDelta) {
	assert(cellArray != nullptr);

	PathCell        *cellPtr;
	bool            newCell;
	QueueItem       newItem;

	//  Don't search beyond map edges
	if (tp.u < 1 || tp.u >= searchDiameter - 1
	        ||  tp.v < 1 || tp.v >= searchDiameter - 1)
		return;

	cellPtr = cellArray->makeCell(platform, tp.u, tp.v, &newCell);

	assert(cellPtr != nullptr);

	//  If the cell is already visited, only
	//  update it if it was less cost to get here.
	if (!newCell && cellPtr->cost <= cost) return;

	newItem.u = tp.u;                       // coords of this point
	newItem.v = tp.v;                       //
	newItem.z = tp.z;
	newItem.platform = platform;
	newItem.cost = cost;
	newItem.direction = direction;
	newItem.pad = 0;

	if (queue->insert(newItem)) {
		cellPtr->direction = direction;
		cellPtr->platformDelta = platformDelta;
		cellPtr->cost = cost;
		cellPtr->height = tp.z;
	} else {
		if (newCell)
			cellArray->deleteCell(platform, tp.u, tp.v);
	}
}


/* ===================================================================== *
   Member Functions
 * ===================================================================== */

/* ===================================================================== *
   PathRequest Class
 * ===================================================================== */

PathRequest::PathRequest(Actor *a, int16 howSmart) {
	_actor       = a;
	_smartness   = howSmart;
	_mTask       = _actor->_moveTask;
	_flags       = _mTask->_flags & MotionTask::kMfRequestRun ? run : 0;

	if (_path == nullptr)
		_path = new TilePoint[kPathSize]();

	_mTask->_pathFindTask = this;
}

void PathRequest::initialize() {
	ProtoObj        *proto = _actor->proto();
	TilePoint       startingCoords = _actor->getLocation();
	int             uCoord, vCoord;
	int             objectVolumes = 0;
	int             nextAvailableLookupNode = 0;
	int           curTileRegU,
	              curTileRegV,
	              minTileRegU,
	              minTileRegV,
	              maxTileRegU,
	              maxTileRegV;
	uint8 pCross = proto->crossSection;

	_firstTick = gameTime,
	_timeLimit = /*flags & run ? ticksPerSecond / 4 :*/ ticksPerSecond;

	_fetchRadius =
	    ((kTileUVSize / 2 + pCross) >> kTileUVShift) + 1;

	_dirMasks = maskComp->computeMask(pCross);

	//  Set the best location to the starting location
	_bestLoc.set(Nowhere.u, Nowhere.v, Nowhere.z);

	//  Calculate where search cells will be projected onto map
	_baseTileCoords.u = (startingCoords.u >> kTileUVShift) - searchCenter;
	_baseTileCoords.v = (startingCoords.v >> kTileUVShift) - searchCenter;
	_baseTileCoords.z = 0;

	_baseCoords.u = _baseTileCoords.u << kTileUVShift;
	_baseCoords.v = _baseTileCoords.v << kTileUVShift;
	_baseCoords.z = 0;

	//  Clear the priority queue
	queue->clear();

	//  Initialize the tile array
	_tileArray->init(
	    _actor->getMapNum(),
	    TilePoint(
	        _baseTileCoords.u - 2,
	        _baseTileCoords.v - 2,
	        0),
	    TilePoint(
	        searchDiameter + 4,
	        searchDiameter + 4,
	        0),
	    (PathTilePosInfo *)pathTileArray,
	    _subMetaFlags);

	for (uCoord = 0; uCoord < searchDiameter; uCoord++) {
		for (vCoord = 0; vCoord < searchDiameter; vCoord++)
			volumeLookupTable[uCoord][vCoord] = nullptr;
	}

	RegionalObjectIterator  iter(
	    currentWorld,
	    _baseCoords,
	    TilePoint(
	        _baseCoords.u
	        + (searchCenter << kTileUVShift) * 2,
	        _baseCoords.v
	        + (searchCenter << kTileUVShift) * 2,
	        0));
	GameObject *obj = nullptr;

	for (iter.first(&obj);
	        obj != nullptr;
	        iter.next(&obj)) {
		TilePoint       objLoc = obj->getLocation() - _baseCoords;
		ProtoObj        *objProto = obj->proto();
		TileRegion      *objRegion = &objectVolumeArray[objectVolumes];
		uint8 poCross = objProto->crossSection;

		//  Obviously, we shouldn't block ourselves.
		if (obj == _actor || obj->isInvisible()) continue;

		//  Dead corpses and invisible actors are not considered
		//  obstacles.
		if (isActor(obj)
		        && (((Actor *)obj)->isDead()
		            || ((Actor *)obj)->hasEffect(kActorInvisible)))
			continue;

		//  Compute the objects volume
		objRegion->min.u = objLoc.u - poCross;
		objRegion->min.v = objLoc.v - poCross;
		objRegion->min.z = objLoc.z;
		objRegion->max.u = objLoc.u + poCross;
		objRegion->max.v = objLoc.v + poCross;
		objRegion->max.z = objLoc.z + objProto->height;

		//  Compute the tile region which this object overlays
		minTileRegU =   MAX(objRegion->min.u >> kTileUVShift, 0);
		maxTileRegU =   MIN(
		                    (objRegion->max.u + kTileUVMask) >> kTileUVShift,
		                    searchDiameter);
		minTileRegV =   MAX(objRegion->min.v >> kTileUVShift, 0);
		maxTileRegV =   MIN(
		                    (objRegion->max.v + kTileUVMask) >> kTileUVShift,
		                    searchDiameter);

		for (curTileRegU = minTileRegU;
		        curTileRegU < maxTileRegU;
		        curTileRegU++) {
			assert(curTileRegU >= 0 && curTileRegU < searchDiameter);

			for (curTileRegV = minTileRegV;
			        curTileRegV < maxTileRegV;
			        curTileRegV++) {
				assert(curTileRegV >= 0 && curTileRegV < searchDiameter);

				VolumeLookupNode    *node;
				VolumeLookupNode    **tablePtrPtr;

				//  Get the next lookup node
				node = &volumeLookupNodePool[nextAvailableLookupNode++];
				tablePtrPtr = &volumeLookupTable[curTileRegU][curTileRegV];

				//  Link into lookup table
				node->volume = objRegion;
				node->next = *tablePtrPtr;
				*tablePtrPtr = node;

				if (nextAvailableLookupNode
				        >=  ARRAYSIZE(volumeLookupNodePool))
					goto big_break;
			}
		}

		if (++objectVolumes >= kObjectVolumeArraySize) break;
	}
big_break:

	//  Compute the actor's tile region
	minTileRegU = (startingCoords.u - pCross) >> kTileUVShift;
	minTileRegV = (startingCoords.v - pCross) >> kTileUVShift;
	maxTileRegU = (startingCoords.u + pCross + kTileUVMask)
	              >>  kTileUVShift;
	maxTileRegV = (startingCoords.v + pCross + kTileUVMask)
	              >>  kTileUVShift;

	for (curTileRegU = minTileRegU;
	        curTileRegU < maxTileRegU;
	        curTileRegU++) {
		for (curTileRegV = minTileRegV;
		        curTileRegV < maxTileRegV;
		        curTileRegV++) {
			TilePoint       quantizedCoords,
			                offsetVector;
			uint8           platform;
			int16           dist,
			                zDist,
			                cost;

			//  Quantize this tile position to the tile center
			quantizedCoords.u = (curTileRegU << kTileUVShift) + kTileUVSize / 2;
			quantizedCoords.v = (curTileRegV << kTileUVShift) + kTileUVSize / 2;
			quantizedCoords.z = startingCoords.z;
			quantizedCoords.z = tileSlopeHeight(
			                        quantizedCoords,
			                        _actor,
			                        nullptr,
			                        &platform);

			//  If the height difference is too great skip this tile
			//  position
			if (ABS(quantizedCoords.z - startingCoords.z) > kMaxStepHeight)
				continue;

			//  Compute initial cost based upon the distance from the
			//  starting location
			offsetVector = quantizedCoords - startingCoords;
			dist = offsetVector.quickHDistance();
			zDist = ABS(offsetVector.z);
			cost = dist + zDist;

			//  Push this point
			push(
			    TilePoint(
			        curTileRegU - _baseTileCoords.u,
			        curTileRegV - _baseTileCoords.v,
			        quantizedCoords.z),
			    platform,
			    cost,
			    kDirInvalid,
			    0);
		}
	}
}

void PathRequest::finish() {
	Direction           prevDir;
	int16               prevHeight = 0;
	TilePoint           *resultSteps = _path,
	                    coords;
	int16               stepCount = 0;
	TilePoint           *res;
	PathCell            *cell;

	static TilePoint tempResult[32];

	debugC(2, kDebugPath, "Finishing Path Request: %p", (void *)this);

	if (_bestLoc != Nowhere) {
		cell = cellArray->getCell(_bestPlatform, _bestLoc.u, _bestLoc.v);
		assert(cell != nullptr);

		if (cell->direction != kDirInvalid) {
			res = &tempResult[ARRAYSIZE(tempResult)];

			prevDir = kDirInvalid;

			for (;;) {
				int16       reverseDir;

				cell = cellArray->getCell(_bestPlatform, _bestLoc.u, _bestLoc.v);
				assert(cell != nullptr);

				if (cell->direction != kDirInvalid) {
					if (cell->direction != prevDir
					        ||  ABS(cell->height - prevHeight) > kMaxStepHeight) {
						if (res <= tempResult) break;

						coords.u =
						    (_bestLoc.u << kTileUVShift)
						    +   _baseCoords.u
						    +   kTileUVSize / 2;
						coords.v =
						    (_bestLoc.v << kTileUVShift)
						    +   _baseCoords.v
						    +   kTileUVSize / 2;
						coords.z = cell->height;
						*--res = coords;

						prevDir = cell->direction;
						prevHeight = cell->height;
					}

					reverseDir = (cell->direction + 4) & 0x07;
					_bestLoc += tDirTable2[reverseDir];
					assert(_bestLoc.u >= 0 && _bestLoc.u < searchDiameter);
					assert(_bestLoc.v >= 0 && _bestLoc.v < searchDiameter);
					_bestPlatform -= cell->platformDelta;
					assert(_bestPlatform < maxPlatforms);
				} else
					break;
			}

			if (resultSteps) {
				while (stepCount < kPathSize
				        &&  res < &tempResult[ARRAYSIZE(tempResult)]) {
					*resultSteps++ = *res++;
					stepCount++;
				}
			}
		} else
			//  if pathfinder didn't get anywhere, we're done
			_flags |= completed;
	}

	_pathLength = stepCount;

	if (_mTask->_pathFindTask == this && _mTask->isWalk()) {
		memcpy(_mTask->_pathList, _path, _pathLength * sizeof(_path[0]));
		_mTask->_pathCount = _pathLength;
		_mTask->_pathIndex = 0;
		_mTask->_flags |= MotionTask::kMfReset;
		if (_flags & completed) _mTask->_flags |= MotionTask::kMfFinalPath;
		_mTask->_pathFindTask = nullptr;
	}
}

void PathRequest::abortReq() {
	debugC(4, kDebugPath, "Aborting Path Request: %p", (void *)this);

	if (_mTask->_pathFindTask == this)
		_mTask->_pathFindTask = nullptr;
}


PathResult PathRequest::findPath() {
	assert(cellArray != nullptr);

	static const uint8 costTable[] = {4, 10, 12, 16, 12, 10, 4, 0, 4, 10, 12, 16, 12, 10, 4, 0};

	ProtoObj        *proto = _actor->proto();
	QueueItem       qi;
	uint8 pCross = proto->crossSection;

	debugC(4, kDebugPath, "Finding Path for %p: pCross = %d", (void *)this, pCross);

	if (_flags & aborted) return pathAborted;

	int32 lastTick_ = gameTime;

	while (queue->remove(qi)) {
		assert(cellArray->getCell(qi.platform, qi.u, qi.v) != nullptr);
		assert(qi.u >= 1 && qi.u < searchDiameter - 1);
		assert(qi.v >= 1 && qi.v < searchDiameter - 1);

		TilePoint   centerTileCoords;
		StaticTilePoint *tDir;
		int32       i,
		            dir,
		            endDir;


		//  Limit the total path length to avoid maze-solving
		if (qi.cost > _smartness) continue;

		//  Set a new center and determine if we're done
		if (setCenter(_baseTileCoords, qi)) return pathDone;

		//  Calculate the coordinates of the center in tiles
		centerTileCoords.u = qi.u + _baseTileCoords.u;
		centerTileCoords.v = qi.v + _baseTileCoords.v;
		centerTileCoords.z = 0;

		if (qi.direction == kDirInvalid) {
			//  If this is the starting position, check all directions
			i = dir = 0;
			tDir = tDirTable2;
			endDir = 8;

			_tileArray->fetchTileSection(
			    TilePoint(qi.u - _fetchRadius, qi.v - _fetchRadius, 0)
			    +   _baseTileCoords,
			    TilePoint(
			        (_fetchRadius << 1) + 1,
			        (_fetchRadius << 1) + 1,
			        0));
		} else {
			//  Check only the forward directions
			i = dir = (qi.direction + 6) & 0x7;
			tDir = &tDirTable2[dir];
			endDir = i + 5;

			switch (qi.direction) {
			case 0:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u + _fetchRadius,
				        qi.v - _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint(1, _fetchRadius << 1, 0));
					// fall through
			case 1:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius,
				        qi.v + _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint((_fetchRadius << 1) + 1, 1, 0));
				break;
			case 2:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius + 1,
				        qi.v + _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint(_fetchRadius << 1, 1, 0));
					// fall through
			case 3:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius,
				        qi.v - _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint(1, (_fetchRadius << 1) + 1, 0));
				break;
			case 4:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius,
				        qi.v - _fetchRadius + 1,
				        0)
				    +   _baseTileCoords,
				    TilePoint(1, _fetchRadius << 1, 0));
					// fall through
			case 5:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius,
				        qi.v - _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint((_fetchRadius << 1) + 1, 1, 0));
				break;
			case 6:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u - _fetchRadius,
				        qi.v - _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint(_fetchRadius << 1, 1,  0));
					// fall through
			case 7:
				_tileArray->fetchTileSection(
				    TilePoint(
				        qi.u + _fetchRadius,
				        qi.v - _fetchRadius,
				        0)
				    +   _baseTileCoords,
				    TilePoint(1, (_fetchRadius << 1) + 1, 0));
			}

		}

		for (;
		        i < endDir;
		        tDir = &tDirTable2[(dir = (++i & 0x7))]) {
			if (!validMove(_centerPt + tDirTable3[dir]))
				continue;

			PathTileInfo    pti;
			TilePoint       testPt = _centerPt;
			uint8           testPlatform;
			uint32          terrain = 0;
			int32           cost;
			DirMask         &dMask = (*_dirMasks)[dir];
			int16           prevZ = _centerPt.z;

			for (int d = 0; d < 4; d++) {
				int       u, v;
				uint8       maskU, maskV;
				PointMask   &ptMask = dMask[d];
				TileRegion  maskReg,
				            actorVolume;

				maskReg.min.u = centerTileCoords.u + ptMask.offset.u;
				maskReg.max.u = maskReg.min.u + ptMask.size.u;
				maskReg.min.v = centerTileCoords.v + ptMask.offset.v;
				maskReg.max.v = maskReg.min.v + ptMask.size.v;

				testPt.u += tDirTable[dir].u;
				testPt.v += tDirTable[dir].v;
				testPt.z =  tileSlopeHeight(
				                *_tileArray,
				                testPt,
				                _actor,
				                &pti,
				                &testPlatform);

				//  Determine if elevation change is too great
				if (ABS(testPt.z - prevZ) <= kMaxStepHeight)
					prevZ = testPt.z;
				else
					goto big_continue;

				//  Compute the actor's volume at this point relative to
				//  the base of the search region
				actorVolume.min.u =     testPt.u
				                        -   _baseCoords.u
				                        -   pCross;
				actorVolume.max.u = actorVolume.min.u + pCross * 2;
				actorVolume.min.v =     testPt.v
				                        -   _baseCoords.v
				                        -   pCross;
				actorVolume.max.v = actorVolume.min.v + pCross * 2;
				actorVolume.min.z = testPt.z;
				actorVolume.max.z = testPt.z + proto->height;
				int16 aph = _actor->proto()->height;

				for (u = maskReg.min.u, maskU = 0; u < maskReg.max.u; u++, maskU++) {
					PathTilePosInfo *arrRow =
					    &_tileArray->_array[(u - _tileArray->_origin.u) * _tileArray->_area.v];

					for (v = maskReg.min.v, maskV = 0; v < maskReg.max.v; v++, maskV++) {
						VolumeLookupNode    *node;

						//  Lookup any potentially intersecting object
						//  volumes
						for (node = volumeLookupTable[u - _baseTileCoords.u][v - _baseTileCoords.v];
						        node != nullptr;
						        node = node->next) {
							TileRegion *trv = node->volume;
							//  Check for volume intersection
							if (trv->min.u < actorVolume.max.u
							        &&  actorVolume.min.u < trv->max.u
							        &&  trv->min.v < actorVolume.max.v
							        &&  actorVolume.min.v < trv->max.v
							        &&  trv->min.z < actorVolume.max.z
							        &&      actorVolume.min.z + kMaxStepHeight
							        <   trv->max.z)
								goto big_continue;
						}

						terrain |=  tileTerrain(
						                &arrRow[v - _tileArray->_origin.v],
						                ptMask.mask[(maskU << 2) | maskV],
						                testPt.z,
						                testPt.z + aph);
					}
				}
			}

			if (terrain & (terrainImpassable | terrainRaised)) continue;

			//  determine the height of the center of the tile
			//  that we're testing.

			//  Assign costs based on the direction of travel

			if (terrain & terrainSlow) {
				cost = dir & 1 ? straightHardCost : diagHardCost;
			} else if (terrain & terrainAverage) {
				cost = dir & 1 ? straightNormalCost : diagNormalCost;
			} else {
				cost = dir & 1 ? straightEasyCost : diagEasyCost;
			}

			//  We must treat stairs as a special case

			if (pti.surfaceTile != nullptr
			        && (pti.surfaceTile->combinedTerrainMask() & terrainStair)) {
				uint8   *cornerHeight = pti.surfaceTile->attrs.cornerHeight;
				uint8   stairDir;
				int16   stairHeight;

				//  Determine the direction and upper altitude of the
				//  stairs

				if (*((uint16 *)&cornerHeight[0]) == 0) {
					stairDir = 1;
					stairHeight = pti.surfaceHeight + cornerHeight[2];
				} else if (*((uint16 *)&cornerHeight[1]) == 0) {
					stairDir = 3;
					stairHeight = pti.surfaceHeight + cornerHeight[3];
				} else if (*((uint16 *)&cornerHeight[2]) == 0) {
					stairDir = 5;
					stairHeight = pti.surfaceHeight + cornerHeight[0];
				} else if (cornerHeight[0] == 0 && cornerHeight[3] == 0) {
					stairDir = 7;
					stairHeight = pti.surfaceHeight + cornerHeight[1];
				} else
					continue;

				//  Do not go onto the stair at a right angle

				if (stairDir == 1 || stairDir == 5) {
					if (dir == 3 || dir == 7)
						continue;
				} else if (stairDir == 3 || stairDir == 7) {
					if (dir == 1 || dir == 5)
						continue;
				}

				//  Add any additional costs for travelling on these
				//  stairs.
				cost += evaluateStairs(
				            testPt,
				            dir,
				            stairDir,
				            pti.surfaceHeight,
				            stairHeight);

			}

			//  Assign additional costs based on having to travel
			//  uphill or having to jump down steep slopes
			else if (testPt.z > _centerPt.z
			         ||  testPt.z < _centerPt.z - 12) {
				cost += ((testPt.z - _centerPt.z)
				         * (testPt.z - _centerPt.z)) >> 5;
			}

			//  If the drop-off is too much, then don't go there!
			//  (i.e. don't jmup off of cliffs). Also we can
			//  only climb steps below a certain height.

//			if (   testPt.z < _centerPt.z - kMaxJumpStep
//				||    testPt.z > _centerPt.z + kMaxStepHeight)
//					continue;

			//  Turns are expensive, the sharper turns are more so.

			cost += costTable[
			            7
			            +   dir
			            - (qi.direction != kDirInvalid
			               ?   qi.direction
			               :   _actor->_currentFacing)];

#if VISUAL1
			TPLine(_centerPt, testPt);
#endif
			//  Determine the final cost of moving to this cell
			cost = clamp(0L,
			             (int32)qi.cost
			             + (int32)cost
			             + (int32)evaluateMove(testPt, testPlatform),
			             (int32)maxint16);

			//  Cost should never be less than previous cost
			cost = MAX<int16>(cost, qi.cost);

			//  Push the new point onto the queue->

			push(
			    TilePoint(
			        qi.u + tDir->u,
			        qi.v + tDir->v,
			        testPt.z),
			    testPlatform,
			    cost,
			    dir,
			    testPlatform - _centerPlatform);
			assert(cellArray->getCell(_centerPlatform, qi.u, qi.v) != nullptr);

big_continue:
			;
		}

		if ((gameTime - lastTick_) >= 4) {         // JEFFKLUDGE
			if (_timeLimitExceeded())
				return pathDone;
		}
	}

	return pathDone;
}

//-------------------------------------------------------------------
// Path finder time management:
//   Originally all requests had a time limit of 72 and IQ of 400
//   I have rewired it to use an IQ of 400 for player actors, and
//   100 for NPCs. That IQ also affects the time limit now. Players
//   will generally get (72/2)+(400/10) or 76 ticks. NPCs will
//   usually get (72/2)+(100/10) or 46 ticks.


bool PathRequest::_timeLimitExceeded() {
#ifdef OLD_PATHFINDER_TIME_MGMT
	return (gameTime - _firstTick >= _timeLimit);
#else
	int32 cutoff = _smartness / (queue->getCount() ? 5 : 8);
	return (gameTime - _firstTick >= cutoff);
#endif
}


/* ===================================================================== *
   DestinationPathRequest Class
 * ===================================================================== */

DestinationPathRequest::DestinationPathRequest(Actor *a, int16 howSmart) :
	PathRequest(a, howSmart) {
	//  Quantize the target destination to the nearest tile center.
	_mTask->_finalTarget.u = (_mTask->_finalTarget.u & ~kTileUVMask) + kTileUVSize / 2;
	_mTask->_finalTarget.v = (_mTask->_finalTarget.v & ~kTileUVMask) + kTileUVSize / 2;
	_mTask->_finalTarget.z =  tileSlopeHeight(
	                            _mTask->_finalTarget,
	                            a,
	                            nullptr,
	                            &_destPlatform);

	_destination = _mTask->_finalTarget;
}

//  Initialize the static data members
void DestinationPathRequest::initialize() {
	debugC(2, kDebugPath, "Initializing Path Request: %p", (void *)this);

	PathRequest::initialize();

	//  Initialize _bestDist to the highest possible value.
	_bestDist = maxint16;

	//  Quantize the target coordinates to the nearest tile center.
	_targetCoords.u = (_destination.u & ~kTileUVMask) + kTileUVSize / 2;
	_targetCoords.v = (_destination.v & ~kTileUVMask) + kTileUVSize / 2;
	_targetCoords.z = _destination.z;
	_targetPlatform = _destPlatform;
}

//  Set and evaluate a new center location.
bool DestinationPathRequest::setCenter(
    const TilePoint &baseTileCoords_,
    const QueueItem &qi) {
	int16       dist,
	            zDist,
	            platDiff;
	TilePoint   targetDelta;

	//  Calculate the center coordinates.
	calcCenterPt(baseTileCoords_, qi);

	//  Determine the target vector in order to calculate distance.
	targetDelta = (_targetCoords - _centerPt);
	dist = targetDelta.quickHDistance();
	zDist = ABS(targetDelta.z);
	platDiff = ABS(_centerPlatform - _targetPlatform);
	_centerCost = dist + zDist * (platDiff + 1);

	//  Determine if this location is closer than any location we have
	//  previously visited.
	if (_centerCost < _bestDist) {
		//  Save closest point encountered.

		_bestLoc.u = qi.u;
		_bestLoc.v = qi.v;
		_bestLoc.z = qi.z;
		_bestPlatform = qi.platform;
		_bestDist = _centerCost;

		//  If we're at target square, then we're done!
		if (dist == 0 && zDist <= kMaxStepHeight) {
			_flags |= PathRequest::completed;

			//  Return true to indicate that the path finding is done.
			return true;
		}
	}

	return false;
}

bool DestinationPathRequest::validMove(const TilePoint &) {
	return true;
}

//  Evaluate the cost of moving on the specified stairs in the specified
//  direction
int16 DestinationPathRequest::evaluateStairs(
    const TilePoint &testPt,    //  Coordinates of location we are testing
    Direction moveDir,          //  Direction of movement
    Direction stairDir,         //  Direction of up stairs
    int16 baseAltitude,         //  Altitude of bottom of the stairs
    int16 upperAltitude) {      //  Altitude of the top of the stairs
	int16   cost = 0;

	//  Determine if the stairs are going towards the
	//  altitude of the target coordinates.  If not, assign
	//  additional costs.

	if (_targetCoords.z >= upperAltitude) {
		if (moveDir != stairDir) {
			cost = ((testPt.z - _centerPt.z)
			        * (testPt.z - _centerPt.z)) >> 4;
		}
	} else if (_targetCoords.z <= baseAltitude) {
		if (moveDir == stairDir) {
			cost = ((testPt.z - _centerPt.z)
			        * (testPt.z - _centerPt.z)) >> 4;
		}
	}

	return cost;
}

//  Evaluate the cost of moving to the specified point from the
//  current center location.
int16 DestinationPathRequest::evaluateMove(
    const TilePoint &testPt,
    uint8           testPlatform) {
	int16       dist,
	            zDist,
	            platDiff;
	TilePoint   targetDelta;

	//  Determine the target vector of the specified coordinates, in
	//  order to calculate the distance.
	targetDelta = _targetCoords - testPt;
	dist = targetDelta.quickHDistance();
	zDist = ABS(targetDelta.z);
	platDiff = ABS(testPlatform - _targetPlatform);

	return (dist + zDist * (platDiff + 1) - _centerCost) >> 2;
}

/* ===================================================================== *
   WanderPathRequest Class
 * ===================================================================== */

WanderPathRequest::WanderPathRequest(
    Actor *a,
    int16 howSmart) :
	PathRequest(a, howSmart) {
	if (_mTask->_flags & MotionTask::kMfTethered) {
		_tethered = true;
		_tetherMinU = _mTask->_tetherMinU;
		_tetherMinV = _mTask->_tetherMinV;
		_tetherMaxU = _mTask->_tetherMaxU;
		_tetherMaxV = _mTask->_tetherMaxV;
	} else {
		_tethered = false;
		_tetherMinU = _tetherMinV = _tetherMaxU = _tetherMaxV = 0;
	}
}

//  Initialize the static data members
void WanderPathRequest::initialize() {
	PathRequest::initialize();

	//  Initialize _bestDist to zero.
	_bestDist = 0;
	_startingCoords.set(_actor->getLocation().u,
	                    _actor->getLocation().v,
	                    _actor->getLocation().z);
}


//  Set and evaluate a new center location.
bool WanderPathRequest::setCenter(
    const TilePoint &baseTileCoords_,
    const QueueItem &qi) {
	int16       dist,
	            zDist;
	TilePoint   movementDelta;

	//  Calculate the center coordinates.
	calcCenterPt(baseTileCoords_, qi);

	//  Determine the movement vector in order to calculate distance.
	movementDelta = (_startingCoords - _centerPt);
	dist = movementDelta.quickHDistance();
	zDist = ABS(movementDelta.z);
	_centerCost = dist + zDist;

	//  Determine if this location is farther than any location we have
	//  previously visited.
	if (_centerCost > _bestDist) {
		//  Save farthest point encountered.

		_bestLoc.u = qi.u;
		_bestLoc.v = qi.v;
		_bestLoc.z = qi.z;
		_bestPlatform = qi.platform;
		_bestDist = _centerCost;
	}

	return false;
}

bool WanderPathRequest::validMove(const TilePoint &testPt) {
	return      !_tethered
	            || (testPt.u >= _tetherMinU
	                &&  testPt.u < _tetherMaxU
	                &&  testPt.v >= _tetherMinV
	                &&  testPt.v < _tetherMaxV);
}

//  There will be no additional costs for travelling on stairs
int16 WanderPathRequest::evaluateStairs(
    const TilePoint &,
    Direction,
    Direction,
    int16,
    int16) {
	return 0;
}

//  Evaluate the cost of moving to the specified point from the
//  current center location.
int16 WanderPathRequest::evaluateMove(const TilePoint &testPt, uint8) {
	int16       dist,
	            zDist;
	TilePoint   movementDelta;

	//  Determine the movement vector of the specified coordinates, in
	//  order to calculate the distance.
	movementDelta = _startingCoords - testPt;
	dist = movementDelta.quickHDistance();
	zDist = ABS(movementDelta.z) >> 1;

	return (_centerCost - (dist + zDist)) >> 1;
}

void runPathFinder() {
	if (currentRequest == nullptr && !g_vm->_pathQueue.empty()) {
		currentRequest = g_vm->_pathQueue.front();
		g_vm->_pathQueue.pop_front();
		currentRequest->initialize();
	}

	if (currentRequest != nullptr) {
		PathResult  result;

		result = currentRequest->findPath();

		if (result != pathNotDone) {
			if (result == pathDone)
				currentRequest->finish();
			else
				currentRequest->abortReq();

			delete currentRequest;
			currentRequest = nullptr;

			cellArray->reset();
		}
	}
}

void addPathRequestToQueue(PathRequest *pr) {
	Actor           *a = pr->_actor;
	Actor           *centerActor = getCenterActor();

	if (a == centerActor)
		g_vm->_pathQueue.push_front(pr);
	else {
		if (isPlayerActor(a)) {
			Common::List<PathRequest *>::iterator it;

			for (it = g_vm->_pathQueue.begin(); it != g_vm->_pathQueue.end(); ++it) {
				Actor       *prActor = (*it)->_actor;

				if (prActor != centerActor || !isPlayerActor(prActor))
					break;
			}

			if (it != g_vm->_pathQueue.end())
				g_vm->_pathQueue.insert(it, pr);
			else
				g_vm->_pathQueue.push_back(pr);
		} else
			g_vm->_pathQueue.push_back(pr);
	}
}

void RequestPath(MotionTask *mTask, int16 _smartness) {
	DestinationPathRequest      *pr;
	Actor                       *a = (Actor *)mTask->_object;

	if ((pr = new DestinationPathRequest(a, _smartness)) != nullptr)
		addPathRequestToQueue(pr);
}

void RequestWanderPath(MotionTask *mTask, int16 _smartness) {
	WanderPathRequest           *pr;
	Actor                       *a = (Actor *)mTask->_object;

	if ((pr = new WanderPathRequest(a, _smartness)) != nullptr)
		addPathRequestToQueue(pr);
}

void abortPathFind(MotionTask *mTask) {
	if (mTask->_pathFindTask) {
		PathRequest     *pr = mTask->_pathFindTask;

		if (pr == currentRequest)
			pr->requestAbort();
		else
			g_vm->_pathQueue.remove(pr);

		mTask->_pathFindTask = nullptr;
	}
}

/* ===================================================================== *
   A special pathfinder to select a random site reachable from another one.
 * ===================================================================== */

/*  Specs:

    1.  Find a locations which is reachable from 'center', and which
        is between minDist and maxDist tiles away (preferably midway
        between them).
    2.  Terrain cost and direction of travel need not be considered.
    3.  Don't put creatures on top of houses, but do put them
        on top of hills.
    4.  To simplify even further, checking in diagonal directions
        is not needed.
    5.  This routine should probably run synchronously, and therefore
        cannot share the data structures of the real pathfinder.
*/

enum cellStates {
	cellUnvisited = 0,
	cellOccupied = (1 << 0),
	cellVisited = (1 << 1)
};

typedef uint8       SimpleCellArray[searchDiameter][searchDiameter];

static PriorityQueue<QueueItem, 128> *squeue;

static void spush(const TilePoint &tp, int cost, int direction) {
	QueueItem       newItem;

	//  Don't search beyond map edges

	if (tp.u < 1 || tp.u >= searchDiameter - 1
	        ||  tp.v < 1 || tp.v >= searchDiameter - 1)
		return;

	newItem.u = tp.u;                       // coords of this point
	newItem.v = tp.v;                       //
	newItem.z = tp.z;
	newItem.cost = cost;
	newItem.direction = direction;
	newItem.platform = 0;

	squeue->insert(newItem);
}

/*  ordering of bits:

    u3v3 u3v2 u3v1 u3v0                 +U
    u2v3 u2v2 u2v1 u2v0    +V <---> -V   |
    u1v3 u1v2 u1v1 u1v0                  |
    u0v3 u0v2 u0v1 u0v0                 -U

     15      14      13      12      11      10      09      08
    ----    ----    ----    ----    ----    ----    ----    ----
    u3v3    u3v2    u3v1    u3v0    u2v3    u2v2    u2v1    u2v0

     07      06      05      04      03      02      01      00
    ----    ----    ----    ----    ----    ----    ----    ----
    u1v3    u1v2    u1v1    u1v0    u0v3    u0v2    u0v1    u0v0
*/

/*  0000 0110 0000 0000 <--transition masks
    0110 0110 0111 1110
    0110 0110 0111 1110
    0110 0000 0000 0000
     -U   +U   -V   +V

    0666 6660 0770 0ee0
*/

static const uint16
posUMask = 0x6660,
negUMask = 0x0666,
posVMask = 0x0770,
negVMask = 0x0ee0;

uint16 sTerrainMasks[8] = {
	posUMask, negUMask,             //  kDirUpLeft (U+)
	negVMask, posVMask,             //  kDirDownLeft (V-)
	negUMask, posUMask,             //  kDirDownRight (U-)
	posVMask, negVMask,             //  kDirUpRight (V+)
};

TilePoint selectNearbySite(
    ObjectID        worldID,
    const TilePoint &startingCoords,
    int32           minDist,
    int32           maxDist,
    bool            offScreenOnly) {        // true if we want it off-screen
	assert(isWorld(worldID));

	TilePoint       _baseCoords,
	                _baseTileCoords,
	                _centerPt,               // The current center coordinates
	                _bestLoc;                // The best cell coordinates,
	// currently visited
	int16           _mapNum = GameWorld::IDtoMapNum(worldID);

	int32           bestRating = -100,
	                bestPossible = (maxDist - minDist) / 2;

	QueueItem       qi;

	//  Allocate the array of cells
	SimpleCellArray *cellArray1 = (SimpleCellArray *)malloc(sizeof * cellArray1);

	//  Nowhere indicates failure of the algorithm.
	_bestLoc = Nowhere;

	//  Calculate where search cells will be projected onto map
	_baseTileCoords.u = (startingCoords.u >> kTileUVShift) - searchCenter;
	_baseTileCoords.v = (startingCoords.v >> kTileUVShift) - searchCenter;
	_baseTileCoords.z = 0;

	_baseCoords.u = _baseTileCoords.u << kTileUVShift;
	_baseCoords.v = _baseTileCoords.v << kTileUVShift;
	_baseCoords.z = 0;

	//  Clear the search array and the queue
	memset(cellArray1, cellUnvisited, sizeof(*cellArray1));
	squeue->clear();

	//  Iterate through all actors in the region and mark areas
	//  as occupied.
	RegionalObjectIterator  iter(
	    (GameWorld *)GameObject::objectAddress(worldID),
	    _baseCoords,
	    TilePoint(
	        _baseCoords.u
	        + (searchCenter << kTileUVShift) * 2,
	        _baseCoords.v
	        + (searchCenter << kTileUVShift) * 2,
	        0));
	GameObject *obj = nullptr;

	for (iter.first(&obj);
	        obj != nullptr;
	        iter.next(&obj)) {
		TilePoint       objLoc = obj->getLocation();
		ProtoObj        *objProto = obj->proto();

		//  If the object is higher than the actor's head, or
		//  low enough to step over, then ignore it.
		if (objLoc.z >= startingCoords.z + 80
		        ||  objLoc.z + objProto->height <= startingCoords.z + kMaxStepHeight / 2) {
			continue;
		}

		//  Calculate which tile actor is standing on.
		objLoc = (objLoc - _baseCoords) >> kTileUVShift;

		//  If that tile is in the search area, then mark it.
		if (objLoc.u >= 0 && objLoc.u < searchDiameter
		        &&  objLoc.v >= 0 && objLoc.v < searchDiameter) {
			(*cellArray1)[objLoc.u][objLoc.v] = cellOccupied;
		}
	}

	//  Push the starting location in the center of the array.

	spush(TilePoint(searchCenter, searchCenter, startingCoords.z),
	      1,                              // initial cost is 1
	      0);                             // facing irrelevant

	while (squeue->remove(qi)) {
		TilePoint   centerTileCoords,
		            distVector;
		StaticTilePoint *tDir;
		int16       dir;
		int32       distFromCenter,
		            rating;

		//  Calculate the distance between the current point and
		//  the center of the search
		distFromCenter =
		    quickDistance(
		        Point32(qi.u - searchCenter, qi.v - searchCenter));
//				max( ABS( qi.u - searchCenter ), ABS( qi.v - searchCenter ) );

		//  Calculate the "goodness" of this cell -- is it in the
		//  middle of the band between min and max?
		rating = MIN(distFromCenter - minDist, maxDist - distFromCenter);

		//  Calculate the coordinates of the center in tiles
		centerTileCoords.u = qi.u + _baseTileCoords.u;
		centerTileCoords.v = qi.v + _baseTileCoords.v;
		centerTileCoords.z = 0;

		_centerPt.u = (centerTileCoords.u << kTileUVShift) + kTileUVSize / 2;
		_centerPt.v = (centerTileCoords.v << kTileUVShift) + kTileUVSize / 2;
		_centerPt.z = qi.z;

		//  If this is the best cell found so far, and it is not
		//  occupied, then mark it as the best cell.
		if (rating > bestRating
		        &&  !((*cellArray1)[qi.u][qi.v] & cellOccupied)) {
			bool    cellOK = true;

			//  if this point is on-screen, we might want to reject it...
			if (offScreenOnly) {
				Point16 screenCoords;           // screen coordinates

				//  Convert to XY coords.
				TileToScreenCoords(_centerPt, screenCoords);

				//  If the point is on-screen, then reject it.
				//  (We want the monsters to walk in from off-screen,
				//  not 'pop in').
				if (screenCoords.x >= -16 && screenCoords.x <= kTileRectWidth + 16
				        &&  screenCoords.y >= -16 && screenCoords.y <= kTileRectHeight + 80) {
					cellOK = false;
				}
			}

			//  Save closest point encountered.
			if (cellOK) {
				_bestLoc.u = qi.u;
				_bestLoc.v = qi.v;
				_bestLoc.z = qi.z;
				bestRating = rating;

				if (rating >= bestPossible) break;
			}
		}

		if (distFromCenter >= maxDist) continue;

		for (dir = kDirUpLeft;
		        dir <= kDirUpRight;
		        dir += 2) {
			uint32          terrain;
			uint8           *cell;
			TilePoint       testPt;
			StandingTileInfo sti;
			TilePoint       fromSubPt,
			                toSubPt;
			bool            traversable = true;
			int16           i;

			uint16          *moveMask = &sTerrainMasks[dir - 1];

			tDir = &tDirTable2[dir];
			cell = &(*cellArray1)[qi.u + tDir->u][qi.v + tDir->v];

			//  Only visit each cell once. Do this before terrain
			//  is checked, to save time.
			if (*cell & cellVisited) continue;

			testPt = _centerPt + tDirTable3[dir];

			//  Get info about the terrain at that point
			terrain =   tileTerrain(_mapNum,
			                        centerTileCoords,
			                        moveMask[0],
			                        _centerPt.z + 8,
			                        _centerPt.z + 68);

			//  Reject if we can't move
			if (terrain & (terrainImpassable | terrainRaised)) {
				//  But only if this is isn't the center point
				if (distFromCenter > 0) continue;
			}

			//  Get the height of the terrain at the new point
			testPt.z =  tileSlopeHeight(testPt, _mapNum, 68, &sti);

			//  If it's too high to step, then don't continue
//			if (testPt.z - qi.z > kMaxStepHeight) continue;
			fromSubPt = _centerPt;
			for (i = 0; i < kTileSubSize; i++) {
				int16       deltaZ;

				//  Next sub tile
				toSubPt = fromSubPt + tDirTable[dir];
				toSubPt.z = tileSlopeHeight(toSubPt, _mapNum, 68);

				deltaZ = toSubPt.z - fromSubPt.z;

				//  If it's too high to step, then don't continue
				if (deltaZ > kMaxStepHeight || deltaZ < -(kMaxStepHeight * 2)) {
					traversable = false;
					break;
				}

				fromSubPt = toSubPt;
			}

			if (!traversable) continue;


			//  Get info about terrain at new point
			terrain =   tileTerrain(_mapNum,
			                        centerTileCoords + *tDir,
			                        moveMask[1],
			                        testPt.z + 8,
			                        testPt.z + 68);

			//  Reject if terrain allows no entry
			if (terrain & (terrainImpassable | terrainRaised)) continue;

#if VISUAL6
			TPLine(_centerPt, testPt);
#endif

			*cell |= cellVisited;

			//  Push the new point onto the queue->
			//  (Cost is random so as to allow site selection to
			//  be somewhat non-deterministic).
			spush(TilePoint(qi.u + tDir->u,
			                qi.v + tDir->v,
			                testPt.z),
			      qi.cost + g_vm->_rnd->getRandomNumber(3),
			      dir);
		}
	}

	free(cellArray1);

	return  _bestLoc != Nowhere
	        ?   TilePoint(
	            ((_bestLoc.u + _baseTileCoords.u) << kTileUVShift) + kTileUVSize / 2,
	            ((_bestLoc.v + _baseTileCoords.v) << kTileUVShift) + kTileUVSize / 2,
	            _bestLoc.z)
	        :   Nowhere;
}

bool checkPath(
    ObjectID            worldID,
    uint8               height,
    const TilePoint     &startingPt,
    const TilePoint     &destPt) {
	TilePoint       startingCoords = startingPt,
	                destCoords = destPt,
	                startingTileCoords,
	                destTileCoords;
	TilePoint       _baseCoords,
	                _baseTileCoords,
	                _centerPt;               // The current center coordinates
	int           minTileRegU,
	              minTileRegV,
	              maxTileRegU,
	              maxTileRegV,
	              curTileRegU,
	              curTileRegV;

	int16           _mapNum = GameWorld::IDtoMapNum(worldID);

	QueueItem       qi;

	StandingTileInfo sti;

	startingTileCoords.u = startingCoords.u >> kTileUVShift;
	startingTileCoords.v = startingCoords.v >> kTileUVShift;
	startingTileCoords.z = 0;

	destTileCoords.u = destCoords.u >> kTileUVShift;
	destTileCoords.v = destCoords.v >> kTileUVShift;
	destTileCoords.z = 0;

	//  Quantize _destination coords to nearest tile center
	destCoords.u = (destTileCoords.u << kTileUVShift) + kTileUVSize / 2;
	destCoords.v = (destTileCoords.v << kTileUVShift) + kTileUVSize / 2;
	destCoords.z = tileSlopeHeight(destCoords, _mapNum, height);

	//  Determine if _destination is outside the search region
	if (destTileCoords.u < startingTileCoords.u - searchCenter
	        ||  destTileCoords.u >= startingTileCoords.u + searchCenter
	        ||  destTileCoords.v < startingTileCoords.v - searchCenter
	        ||  destTileCoords.v >= startingTileCoords.v + searchCenter)
		return false;

	//  Allocate the array of cells
	SimpleCellArray *cellArray1 = (SimpleCellArray *)malloc(sizeof(*cellArray1));
	if (cellArray1 == nullptr)
		return false;

	//  Calculate where search cells will be projected onto map
	_baseTileCoords.u = startingTileCoords.u - searchCenter;
	_baseTileCoords.v = startingTileCoords.v - searchCenter;
	_baseTileCoords.z = 0;

	_baseCoords.u = _baseTileCoords.u << kTileUVShift;
	_baseCoords.v = _baseTileCoords.v << kTileUVShift;
	_baseCoords.z = 0;

	//  Clear the search array and the queue
	memset(cellArray1, cellUnvisited, sizeof(* cellArray1));
	squeue->clear();

	//  Push the starting location in the center of the array.
	minTileRegU = (startingCoords.u - kTileUVSize / 2) >> kTileUVShift;
	minTileRegV = (startingCoords.v - kTileUVSize / 2) >> kTileUVShift;
	maxTileRegU = (startingCoords.u + kTileUVSize / 2 + kTileUVMask)
	              >>  kTileUVShift;
	maxTileRegV = (startingCoords.v + kTileUVSize / 2 + kTileUVMask)
	              >>  kTileUVShift;

	for (curTileRegU = minTileRegU;
	        curTileRegU < maxTileRegU;
	        curTileRegU++) {
		for (curTileRegV = minTileRegV;
		        curTileRegV < maxTileRegV;
		        curTileRegV++) {
			TilePoint       quantizedCoords,
			                offsetVector;
			int16           dist,
			                zDist,
			                cost;

			//  Quantize this tile position to the tile center
			quantizedCoords.u = (curTileRegU << kTileUVShift) + kTileUVSize / 2;
			quantizedCoords.v = (curTileRegV << kTileUVShift) + kTileUVSize / 2;
			quantizedCoords.z = startingCoords.z;
			quantizedCoords.z = tileSlopeHeight(quantizedCoords, _mapNum, height);

			//  If the height difference is too great skip this tile
			//  position
			if (ABS(quantizedCoords.z - startingCoords.z) > kMaxStepHeight)
				continue;

			//  Compute initial cost based upon the distance from the
			//  starting location
			offsetVector = quantizedCoords - startingCoords;
			dist = offsetVector.quickHDistance();
			zDist = ABS(offsetVector.z);
			cost = dist + zDist;

			//  Push this point
			spush(
			    TilePoint(
			        curTileRegU - _baseTileCoords.u,
			        curTileRegV - _baseTileCoords.v,
			        quantizedCoords.z),
			    cost + 1,
			    0);
		}
	}

	while (squeue->remove(qi)) {
		TilePoint   centerTileCoords;
		StaticTilePoint *tDir;
		int16       centerDistFromDest;
		int       dir;

		//  Calculate the coordinates of the center in tiles
		centerTileCoords.u = qi.u + _baseTileCoords.u;
		centerTileCoords.v = qi.v + _baseTileCoords.v;
		centerTileCoords.z = 0;

		_centerPt.u = (centerTileCoords.u << kTileUVShift) + kTileUVSize / 2;
		_centerPt.v = (centerTileCoords.v << kTileUVShift) + kTileUVSize / 2;
		_centerPt.z = qi.z;

		centerDistFromDest = (_centerPt - destCoords).quickHDistance();

		for (dir = kDirUpLeft;
		        dir <= kDirUpRight;
		        dir += 2) {
			uint32          terrain;
			uint8           *cell;
			TilePoint       testPt,
			                testTileCoords,
			                fromSubPt,
			                toSubPt;
			int16           testDistFromDest,
			                deltaDistFromDest;
			int           i;
			bool            traversable = true;

			uint16          *moveMask = &sTerrainMasks[dir - 1];

			tDir = &tDirTable2[dir];

			testTileCoords.u = centerTileCoords.u + tDir->u;
			testTileCoords.v = centerTileCoords.v + tDir->v;
			testTileCoords.z = 0;

			cell = &(*cellArray1)[qi.u + tDir->u][qi.v + tDir->v];

			//  Only visit each cell once..
			if (*cell & cellVisited) continue;

			testPt = _centerPt + tDirTable3[dir];

			testDistFromDest = (testPt - destCoords).quickHDistance();
			deltaDistFromDest = testDistFromDest - centerDistFromDest;

			//  Get info about the terrain at that point
			terrain =   tileTerrain(_mapNum,
			                        centerTileCoords,
			                        moveMask[0],
			                        _centerPt.z + 8,
			                        _centerPt.z + height);

			//  Reject if we can't move
			if (terrain & (terrainImpassable | terrainRaised)) continue;

			//  Get the height of the terrain at the new point
			testPt.z =  tileSlopeHeight(testPt, _mapNum, height, &sti);

			fromSubPt = _centerPt;
			for (i = 0; i < kTileSubSize; i++) {
				int16       deltaZ;

				//  Next sub tile
				toSubPt = fromSubPt + tDirTable[dir];
				toSubPt.z = tileSlopeHeight(toSubPt, _mapNum, height);

				deltaZ = toSubPt.z - fromSubPt.z;

				//  If it's too high to step, then don't continue
				if (deltaZ > kMaxStepHeight || deltaZ < -(kMaxStepHeight * 2)) {
					traversable = false;
					break;
				}

				fromSubPt = toSubPt;
			}

			if (!traversable) continue;

			//  Get info about terrain at new point
			terrain =   tileTerrain(_mapNum,
			                        centerTileCoords + *tDir,
			                        moveMask[1],
			                        testPt.z + 8,
			                        testPt.z + height);

			//  Reject if terrain allows no entry
			if (terrain & (terrainImpassable | terrainRaised)) continue;

#if VISUAL7
			TPLine(_centerPt, testPt);
#endif

			*cell |= cellVisited;

			//  If we're there, we're done
			if (testTileCoords == destTileCoords) {
				free(cellArray1);

				//  If the resulting height is significantly different
				//  from the _destination height, assume we're on a
				//  different level and return false.
				return ABS(testPt.z - destCoords.z) <= kMaxStepHeight;
			}


			//  Push the new point onto the queue->
			spush(TilePoint(qi.u + tDir->u,
			                qi.v + tDir->v,
			                testPt.z),
			      qi.cost + (deltaDistFromDest + kTileUVSize) / 4,
			      dir);
		}
	}

	free(cellArray1);

	//  If we're here we've haven't found a path
	return false;
}


/* ===================================================================== *
   Path finder management functions
 * ===================================================================== */

void initPathFinder() {
	queue = new PriorityQueue<QueueItem, 192>;
	squeue = new PriorityQueue<QueueItem, 128>;
	objectVolumeArray = new TileRegion[128];

	pathTileArray = (PathTilePosArray *)malloc( sizeof *pathTileArray);
	maskComp = new MaskComputer;
	cellArray = new PathArray;
	PathRequest::_tileArray = new PathTileRegion;
}

void cleanupPathFinder() {
	if (pathTileArray) {
		free(pathTileArray);
		pathTileArray = nullptr;
	}
	if (maskComp) {
		delete maskComp;
		maskComp = nullptr;
	}
	if (cellArray != nullptr) {
		delete cellArray;
		cellArray = nullptr;
	}

	delete queue;
	delete squeue;
	delete[] objectVolumeArray;
	delete PathRequest::_tileArray;
}

} // end of namespace Saga2
