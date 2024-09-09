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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/detection.h"
#include "saga2/fta.h"
#include "saga2/blitters.h"
#include "saga2/sprite.h"
#include "saga2/tcoords.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

const int           maxWeaponSpriteSets = 40;

const uint32        spriteGroupID   = MKTAG('S', 'P', 'R', 'I'),
                    frameGroupID    = MKTAG('F', 'R', 'M', 'L'),
                    poseGroupID     = MKTAG('P', 'O', 'S', 'E'),
                    schemeGroupID   = MKTAG('S', 'C', 'H', 'M'),
                    objectSpriteID  = MKTAG('O', 'B', 'J', 'S'),
                    mentalSpriteID  = MKTAG('M', 'E', 'N', 'T'),
                    weaponSpriteBaseID  = MKTAG('W', 'P', 'N', 0),
                    missileSpriteID = MKTAG('M', 'I', 'S', 'S');

extern uint16 rippedRoofID;
extern void drawTileMask(
    const Point16 &sPos,
    gPixelMap &map,
    TilePoint loc,
    uint16 roofID = rippedRoofID);

//  Color map ranges
extern uint8        *ColorMapRanges;

/* ===================================================================== *
   Exports
 * ===================================================================== */

/* ===================================================================== *
   Locals
 * ===================================================================== */

//  Remap table for colors which are not remapped.

const uint8 fixedColors[] = {
	0, 10, 12, 14, 16, 18, 21, 24,
	101, 104, 130, 132, 197, 199, 228, 230
};

SpriteSet           *objectSprites,    // object sprites
                    *mentalSprites,   // intangible object sprites
                    *weaponSprites[maxWeaponSpriteSets], // weapon sprites
                    *missileSprites;  // missile sprites

hResContext         *spriteRes,         // sprite resource handle
                    *frameRes,          // framelist resource handle
                    *poseRes,           // poselist resource handle
                    *schemeRes;         // schemelist resource handle

//  An array of 32 actor appearances
static ActorAppearance appearanceTable[32];

/* ===================================================================== *
   Quick memory routines
 * ===================================================================== */

static uint8        *quickMemBase,
       *quickMemPtr;

int32               quickMemSize;

void initQuickMem(int32 size) {
	quickMemBase = new uint8[size]();
	if (quickMemBase == nullptr)
		error("Error: Memory allocation size %d failed!", size);
	quickMemSize = size;
	quickMemPtr = quickMemBase;
}

void cleanupQuickMem() {
	if (quickMemBase)
		delete[] quickMemBase;
	quickMemBase = nullptr;
}

void *getQuickMem(int32 size) {
	if (quickMemPtr + size > quickMemBase + quickMemSize) {
		error("Error: QuickMem allocation failed, size %d", size);
	}
	quickMemPtr += size;
	return quickMemPtr - size;
}

void freeQuickMem(void *ptr) {
	quickMemPtr = ptr ? (uint8 *)ptr : quickMemBase;
}

/* ===================================================================== *
   Sprite rendering routines
 * ===================================================================== */

void DrawCompositeMaskedSprite(
    gPort           &port,                  // destination gPort
    SpriteComponent *scList,                // list of components
    int16           numParts,               // number of components
    const Point16   &destPoint,             // where to render to
    const TilePoint &loc,                   // location on map
    int16           effects,                // effects flags
    bool            *obscured) {            // set if object obscured by terrain
	SpriteComponent *sc;                    // sprite component
	int16           xMax = 0,               // extent of composite
	                xMin = 0,
	                yMax = 0,
	                yMin = 0;
	Rect16          clip;                   // clip rect of port
	gPixelMap       compMap,                // pixel map for composite
	                sprMap;                 // sprite map
	Point16         org;
	int             x, y;

	port.getClip(clip);

	//  First, determine the enclosing rectangle which
	//  surrounds all of the sprites.

	sc = scList;

	for (int i = 0; i < numParts; i++, sc++) {
		Sprite      *sp = sc->sp;
		int16       left,
		            right,
		            bottom,
		            top;

		//  Compute the rectangle of the sprites

		if (sc->flipped)
			left = destPoint.x + sc->offset.x - sp->size.x - sp->offset.x;
		else left = destPoint.x + sc->offset.x + sp->offset.x;

		top    = destPoint.y + sc->offset.y + sp->offset.y;
		right  = left + sp->size.x;
		bottom = top + sp->size.y;

		if (i == 0) {
			xMin = left;
			xMax = right;
			yMin = top;
			yMax = bottom;
		} else {
			if (left   < xMin) xMin = left;
			if (right  > xMax) xMax = right;
			if (top    < yMin) yMin = top;
			if (bottom > yMax) yMax = bottom;
		}
	}

	//  If the composite area is outside the screen rect, then
	//  nothing needs to be done.

	if (xMax <= clip.x
	        || yMax <= clip.y
	        || xMin >= clip.x + clip.width
	        || yMin >= clip.y + clip.height)
		return;

	//  Justify the x coords to the nearest tile boundary

	xMin = xMin & ~31;
	xMax = (xMax + 31) & ~31;

	//  Build a temporary bitmap to composite the sprite within

	compMap._size.x = xMax - xMin;
	compMap._size.y = yMax - yMin;
	compMap._data = (uint8 *)getQuickMem(compMap.bytes());
	memset(compMap._data, 0, compMap.bytes());

	//  Calculate the offset from the upper-left corner of
	//  our composite map to the origin point where the sprites
	//  should be drawn.

	org.x = destPoint.x - xMin;
	org.y = destPoint.y - yMin;

	//  First, determine the enclosing rectangle which
	//  surrounds all of the sprites.
	sc = scList;
	for (int i = 0; i < numParts; i++, sc++) {
		Sprite      *sp = sc->sp;

		//  Create a temp map for the sprite to unpack in

		sprMap._size = sp->size;
		if (sprMap._size.x <= 0 || sprMap._size.y <= 0) continue;
		sprMap._data = (uint8 *)getQuickMem(compMap.bytes());

		//  Unpack the sprite into the temp map

		unpackSprite(&sprMap, sp->data, sp->dataSize);

		//  Blit the temp map onto the composite map

		if (sc->flipped) {
			compositePixelsRvs(
			    &compMap,
			    &sprMap,
			    org.x + sc->offset.x - sp->offset.x,
			    org.y + sc->offset.y + sp->offset.y,
			    sc->colorTable);
		} else {
			compositePixels(
			    &compMap,
			    &sprMap,
			    org.x + sc->offset.x + sp->offset.x,
			    org.y + sc->offset.y + sp->offset.y,
			    sc->colorTable);
		}
		freeQuickMem(sprMap._data);
	}

	//  do terrain masking
	if (effects & kSprFXTerrainMask) {
		if (!(effects & kSprFXGhostIfObscured)) {
			drawTileMask(
			    Point16(xMin, yMin),
			    compMap,
			    loc);
		} else {
			gPixelMap       tempMap;
			int32           compMapBytes = compMap.bytes(),
			                visiblePixels;
			bool            isObscured;

			tempMap._size = compMap._size;
			tempMap._data = (uint8 *)getQuickMem(compMapBytes);

			memcpy(tempMap._data, compMap._data, compMapBytes);

			drawTileMask(
			    Point16(xMin, yMin),
			    compMap,
			    loc);

			visiblePixels = 0;
			for (int i = 0; i < compMapBytes; i++) {
				if (compMap._data[i] != 0) {
					visiblePixels++;
					if (visiblePixels > 10) break;
				}
			}

			isObscured = visiblePixels <= 10;
			if (isObscured) {
				memcpy(compMap._data, tempMap._data, compMapBytes);
				effects |= kSprFXGhosted;
			}

			if (obscured != nullptr) *obscured = isObscured;

			freeQuickMem(tempMap._data);
		}
	}

	//  Check if location is underwater
	if (loc.z < 0) {
		uint8   *submergedArea = &compMap._data[(-loc.z < compMap._size.y ?
		                                        (compMap._size.y + loc.z)
		                                        * compMap._size.x :
		                                        0)];

		uint16  submergedSize = &compMap._data[compMap.bytes()] -
		                        submergedArea;

		memset(submergedArea, 0, submergedSize);
	}

	//  Add in "ghost" effects
	if (effects & kSprFXGhosted) {
		uint32  *dstRow = (uint32 *)compMap._data;

		uint32  mask = (yMin & 1) ? 0xff00ff00 : 0x00ff00ff;

		for (y = 0; y < compMap._size.y; y++) {
			for (x = 0; x < compMap._size.x; x += 4) {
				*dstRow++ &= mask;
			}
			mask = ~mask;
		}
	}

	//  Blit to the port

	TBlit(port._map, &compMap, xMin, yMin);

	freeQuickMem(compMap._data);
}

void DrawSprite(
    gPort           &port,                  // destination gPort
    const Point16   &destPoint,             // where to render to
    Sprite          *sp) {                  // sprite pointer
	gPixelMap       sprMap;                 // sprite map

	//  Create a temp map for the sprite to unpack in
	sprMap._size = sp->size;
	sprMap._data = (uint8 *)getQuickMem(sprMap.bytes());

	//  Unpack the sprite into the temp map
	unpackSprite(&sprMap, sp->data, sp->dataSize);

	//  Blit to the port
	port.setMode(kDrawModeMatte);
	port.bltPixels(sprMap,
	               0, 0,
	               destPoint.x + sp->offset.x,
	               destPoint.y + sp->offset.y,
	               sprMap._size.x, sprMap._size.y);

	freeQuickMem(sprMap._data);
}

//  Draw a single sprite with no masking, but with color mapping.

void DrawColorMappedSprite(
    gPort           &port,                  // destination gPort
    const Point16   &destPoint,             // where to render to
    Sprite          *sp,                    // sprite pointer
    uint8           *colorTable) {          // color remapping table
	gPixelMap       sprMap,                 // sprite map
	                sprReMap;               // remapped sprite map

	//  Create a temp map for the sprite to unpack in
	sprMap._size = sp->size;
	sprMap._data = (uint8 *)getQuickMem(sprMap.bytes());
	sprReMap._size = sp->size;
	sprReMap._data = (uint8 *)getQuickMem(sprReMap.bytes());

	//  Unpack the sprite into the temp map
	unpackSprite(&sprMap, sp->data, sp->dataSize);

	memset(sprReMap._data, 0, sprReMap.bytes());

	//  remap the sprite to the color table given
	compositePixels(
	    &sprReMap,
	    &sprMap,
	    0,
	    0,
	    colorTable);

	//  Blit to the port
	port.setMode(kDrawModeMatte);
	port.bltPixels(sprReMap,
	               0, 0,
	               destPoint.x + sp->offset.x,
	               destPoint.y + sp->offset.y,
	               sprReMap._size.x, sprReMap._size.y);

	freeQuickMem(sprReMap._data);
	freeQuickMem(sprMap._data);
}

//  Draw a single sprite with no masking, but with color mapping.

void ExpandColorMappedSprite(
    gPixelMap       &map,                   // destination gPixelMap
    Sprite          *sp,                    // sprite pointer
    uint8           *colorTable) {          // color remapping table
	gPixelMap       sprMap,                 // sprite map
	                sprReMap;               // remapped sprite map

	//  Create a temp map for the sprite to unpack in
	sprMap._size = sp->size;
	sprMap._data = (uint8 *)getQuickMem(sprMap.bytes());

	//  Unpack the sprite into the temp map
	unpackSprite(&sprMap, sp->data, sp->dataSize);

	//  remap the sprite to the color table given
	compositePixels(
	    &map,
	    &sprMap,
	    0,
	    0,
	    colorTable);

	freeQuickMem(sprMap._data);
}

//  Unpacks a sprite for a moment and returns the value of a
//  specific pixel in the sprite. This is used to do hit testing
//  against sprites.

uint8 GetSpritePixel(
    Sprite          *sp,                    // sprite pointer
    int16           flipped,                // true if sprite was flipped
    const Point16   &testPoint) {           // where to render to
	gPixelMap       sprMap;                 // sprite map
	uint8           result;

	//  Create a temp map for the sprite to unpack in
	sprMap._size = sp->size;
	sprMap._data = (uint8 *)getQuickMem(sprMap.bytes());

	//  Unpack the sprite into the temp map
	unpackSprite(&sprMap, sp->data, sp->dataSize);

	//  Map the coords to the bitmap and return the pixel
	if (flipped) {
		result = sprMap._data[testPoint.y * sprMap._size.x
		                                  + sprMap._size.x - testPoint.x];
	} else {
		result = sprMap._data[testPoint.y * sprMap._size.x + testPoint.x];
	}
	freeQuickMem(sprMap._data);

	return result;
}


//  Return the number of visible pixels in a sprite after terrain masking
uint16 visiblePixelsInSprite(
    Sprite          *sp,                    // sprite pointer
    bool            flipped,                // is sprite flipped
    ColorTable      colors,                 // sprite's color table
    Point16         drawPos,                // XY position of sprite
    TilePoint       loc,                    // UVZ coordinates of sprite
    uint16          roofID) {               // ID of ripped roof

	Point16     org;
	int16       xMin,                   // extent of sprite
	            xMax,
	            yMin,
	            yMax;
	gPixelMap   sprMap,                 // sprite map
	            compMap;
	uint16      compBytes,
	            i,
	            visiblePixels;

	//  Determine the extent of the sprite
	xMin = drawPos.x + sp->offset.x;
	yMin = drawPos.y + sp->offset.y;
	xMax = xMin + sp->size.x;
	yMax = yMin + sp->size.y;

	//  Justify the x coords to the nearest tile boundary
	xMin &= ~31;
	xMax = (xMax + 31) & ~31;

	//  Build a temporary bitmap to composite the sprite within
	compMap._size.x = xMax - xMin;
	compMap._size.y = yMax - yMin;
	compMap._data = (uint8 *)getQuickMem(compBytes = compMap.bytes());
	memset(compMap._data, 0, compBytes);

	//  Build bitmap in which to unpack the sprite
	sprMap._size = sp->size;
	sprMap._data = (uint8 *)getQuickMem(sprMap.bytes());

	unpackSprite(&sprMap, sp->data, sp->dataSize);

	org.x = drawPos.x - xMin;
	org.y = drawPos.y - yMin;

	//  Blit the sprite onto the composite map
	if (!flipped) {
		compositePixels(
		    &compMap,
		    &sprMap,
		    org.x + sp->offset.x,
		    org.y + sp->offset.y,
		    colors);
	} else {
		compositePixelsRvs(
		    &compMap,
		    &sprMap,
		    org.x - sp->offset.x,
		    org.y + sp->offset.y,
		    colors);
	}

	//  do terrain masking
	drawTileMask(
	    Point16(xMin, yMin),
	    compMap,
	    loc,
	    roofID);

	//  count the visible pixels in the composite map
	for (i = 0, visiblePixels = 0; i < compBytes; i++)
		if (compMap._data[i]) visiblePixels++;

#if DEBUG*0
	WriteStatusF(8, "Visible pixels = %u", visiblePixels);
#endif

	freeQuickMem(sprMap._data);

	freeQuickMem(compMap._data);

	return visiblePixels;
}

/* ===================================================================== *
   Color table assembly
 * ===================================================================== */

void buildColorTable(
    uint8           *colorTable,            // color table to build
    uint8           *colorOptions,          // colors ranges chosen
    int16           numOptions) {
	uint32           *src,
	                 *dst;

	memcpy(colorTable, fixedColors, sizeof fixedColors);
	dst = (uint32 *)(colorTable + sizeof fixedColors);

	while (numOptions--) {
		src = (uint32 *)&ColorMapRanges[*colorOptions * 8];
		colorOptions++;
		*dst++ = *src++;
		*dst++ = *src++;
	}
}

/* ===================================================================== *
   Load actor appearance
 * ===================================================================== */

void ActorAppearance::loadSpriteBanks(int16 banksNeeded) {
	int16           bank;

	WriteStatusF(2, "Loading Banks: %x", banksNeeded);

	//  Make this one the most recently used entry
	g_vm->_appearanceLRU.push_back(this);

	//  Load in additional sprite banks if requested...
	for (bank = 0; bank < (long)ARRAYSIZE(_spriteBanks); bank++) {
		//  Load the sprite handle...
		if (_spriteBanks[bank] == nullptr && (banksNeeded & (1 << bank))) {
			Common::SeekableReadStream *stream = loadResourceToStream(spriteRes, _id + MKTAG(0, 0, 0, bank), "sprite bank");
			if (stream) {
				_spriteBanks[bank] = new SpriteSet(stream);
				delete stream;
			}
		}
	}
}

ActorAnimation::ActorAnimation(Common::SeekableReadStream *stream) {
	for (int i = 0; i < kNumPoseFacings; i++)
		start[i] = stream->readUint16LE();

	for (int i = 0; i < kNumPoseFacings; i++)
		count[i] = stream->readUint16LE();

	for (int i = 0; i < kNumPoseFacings; i++)
		debugC(2, kDebugLoading, "anim%d: start: %d count: %d", i, start[i], count[i]);
}

ActorPose::ActorPose() {
	flags = 0;

	actorFrameIndex = actorFrameBank = leftObjectIndex = rightObjectIndex = 0;
}


ActorPose::ActorPose(Common::SeekableReadStream *stream) {
	load(stream);
}

void ActorPose::load(Common::SeekableReadStream *stream) {
	flags = stream->readUint16LE();

	actorFrameIndex = stream->readByte();
	actorFrameBank = stream->readByte();
	leftObjectIndex = stream->readByte();
	rightObjectIndex = stream->readByte();

	leftObjectOffset.load(stream);
	rightObjectOffset.load(stream);
}

void ActorPose::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint16LE(flags);

	out->writeByte(actorFrameIndex);
	out->writeByte(actorFrameBank);
	out->writeByte(leftObjectIndex);
	out->writeByte(rightObjectIndex);

	leftObjectOffset.write(out);
	rightObjectOffset.write(out);
}


ColorScheme::ColorScheme(Common::SeekableReadStream *stream) {
	for (int i = 0; i < 11; ++i)
		bank[i] = stream->readByte();

	speechColor = stream->readByte();

	for (int i = 0; i < 32; ++i)
		name[i] = stream->readSByte();
}

ColorSchemeList::ColorSchemeList(int count, Common::SeekableReadStream *stream) {
	_count = count;

	_schemes = (ColorScheme **)malloc(_count * sizeof(ColorScheme *));
	for (int i = 0; i < _count; ++i)
		_schemes[i] = new ColorScheme(stream);
}

ColorSchemeList::~ColorSchemeList() {
	for (int i = 0; i < _count; ++i)
		delete _schemes[i];

	free(_schemes);
}

ActorAppearance *LoadActorAppearance(uint32 id, int16 banksNeeded) {
	int16           bank;
	int schemeListSize;
	Common::SeekableReadStream *stream;

	//  Search the table for either a matching appearance,
	//  or for an empty one.
	for (Common::List<ActorAppearance *>::iterator it = g_vm->_appearanceLRU.begin(); it != g_vm->_appearanceLRU.end(); ++it) {
		if ((*it)->_id == id                    // If has same ID
		        && (*it)->_poseList != nullptr) {      // and frames not dumped
			// then use this one!
			(*it)->_useCount++;
			(*it)->loadSpriteBanks(banksNeeded);
			return *it;
		}
	}

	//  If we couldn't find an extact match, search for an
	//  empty one.
	ActorAppearance *aa = nullptr;
	//  Search from LRU end of list.
	for (Common::List<ActorAppearance *>::iterator it = g_vm->_appearanceLRU.begin(); it != g_vm->_appearanceLRU.end(); ++it) {
		if ((*it)->_useCount == 0)  {	// If not in use
			aa = *it;					// then use this one!
			break;
		}
	}

	//  If none available, that's fatal...
	if (aa == nullptr) {
		error("All ActorAppearance records are in use!");
	}

	//  Dump the sprites being stored
	for (bank = 0; bank < (long)ARRAYSIZE(aa->_spriteBanks); bank++) {
		if (aa->_spriteBanks[bank])
			delete aa->_spriteBanks[bank];
		aa->_spriteBanks[bank] = nullptr;
	}

	if (aa->_poseList) {
		for (uint i = 0; i < aa->_poseList->numPoses; i++)
			delete aa->_poseList->poses[i];

		free(aa->_poseList->poses);

		for (uint i = 0; i < aa->_poseList->numAnimations; i++)
			delete aa->_poseList->animations[i];

		free(aa->_poseList->animations);

		delete aa->_poseList;
	}
	aa->_poseList = nullptr;

	if (aa->_schemeList) {
		delete aa->_schemeList;
	}
	aa->_schemeList = nullptr;

	//  Set ID and use count
	aa->_id = id;
	aa->_useCount = 1;

	//  Load in new frame lists and sprite banks
	aa->loadSpriteBanks(banksNeeded);

	Common::SeekableReadStream *poseStream = loadResourceToStream(poseRes, id, "pose list");

	if (poseStream == nullptr) {
		warning("LoadActorAppearance: Could not load pose list");
	} else {
		ActorAnimSet *as = new ActorAnimSet;
		aa->_poseList = as;
		as->numAnimations = poseStream->readUint32LE();
		as->poseOffset = poseStream->readUint32LE();

		// compute number of ActorPoses
		uint32 poseBytes = poseStream->size() - as->poseOffset;
		const int poseSize = 14;

		debugC(1, kDebugLoading, "Pose List: bytes: %ld numAnimations: %d  poseOffset: %d calculated offset: %d numPoses: %d",
			long(poseStream->size()), as->numAnimations, as->poseOffset, 8 + as->numAnimations * 32, poseBytes / poseSize);

		if (poseBytes % poseSize != 0)
			warning("Incorrect number of poses, %d bytes more", poseBytes % poseSize);

		as->numPoses = poseBytes / poseSize;

		as->animations = (ActorAnimation **)malloc(as->numAnimations * sizeof(ActorAnimation *));

		for (uint i = 0; i < as->numAnimations; i++)
			as->animations[i] = new ActorAnimation(poseStream);

		as->poses = (ActorPose **)malloc(as->numPoses * sizeof(ActorPose *));

		for (uint i = 0; i < as->numPoses; i++)
			as->poses[i] = new ActorPose(poseStream);

		delete poseStream;
	}

	if (schemeRes->seek(id) == 0) {
		warning("LoadActorAppearance: Could not load scheme list");
	} else {
		const int colorSchemeSize = 44;

		if (schemeRes->size(id) % colorSchemeSize != 0)
			warning("Incorrect number of colorschemes, %d bytes more", schemeRes->size(id) % colorSchemeSize);

		schemeListSize = schemeRes->size(id) / colorSchemeSize;
		stream = loadResourceToStream(schemeRes, id, "scheme list");
		aa->_schemeList = new ColorSchemeList(schemeListSize, stream);

		delete stream;
	}

	return aa;
}

void ReleaseActorAppearance(ActorAppearance *aa) {
	aa->_useCount--;
}

/* ===================================================================== *
   Sprite initialization routines
 * ===================================================================== */

Sprite::Sprite(Common::SeekableReadStream *stream) {
	size.load(stream);
	offset.load(stream);

	dataSize = size.x * size.y;
	data = (byte *)malloc(dataSize);
	stream->read(data, dataSize);
}

Sprite::~Sprite() {
	free(data);
}

SpriteSet::SpriteSet(Common::SeekableReadStream *stream) {
	count = stream->readUint32LE();
	sprites = (Sprite **)malloc(count * sizeof(Sprite *));

	for (uint i = 0; i < count; ++i) {
		stream->seek(4 + i * 4);
		uint32 offset = stream->readUint32LE();
		stream->seek(offset);
		sprites[i] = new Sprite(stream);
	}
}

SpriteSet::~SpriteSet() {
	for (uint i = 0; i < count; ++i) {
		if (sprites[i])
			delete sprites[i];
	}

	free(sprites);
}

void initSprites() {
	int     i;
	Common::SeekableReadStream *stream = nullptr;

	spriteRes = resFile->newContext(spriteGroupID, "sprite resources");
	if (!spriteRes->_valid)
		error("Error accessing sprite resource group.");

	frameRes = resFile->newContext(frameGroupID, "frame resources");
	assert(frameRes && frameRes->_valid);

	poseRes = resFile->newContext(poseGroupID, "pose resources");
	assert(poseRes && poseRes->_valid);

	schemeRes = resFile->newContext(schemeGroupID, "scheme resources");
	assert(schemeRes && schemeRes->_valid);

	// object sprites
	stream = loadResourceToStream(spriteRes, objectSpriteID, "object sprites");
	objectSprites = new SpriteSet(stream);
	delete stream;
	assert(objectSprites);

	if (g_vm->getGameId() == GID_FTA2) {
		// intagible object sprites
		stream = loadResourceToStream(spriteRes, mentalSpriteID, "mental sprites");
		mentalSprites = new SpriteSet(stream);
		delete stream;
		assert(mentalSprites);

		for (i = 0; i < maxWeaponSpriteSets; i++) {
			hResID weaponSpriteID;

			weaponSpriteID = weaponSpriteBaseID + MKTAG(0, 0, 0, i);

			if (spriteRes->size(weaponSpriteID) == 0) {
				weaponSprites[i] = nullptr;
				continue;
			}

			stream = loadResourceToStream(spriteRes, weaponSpriteID, "weapon sprite set");
			weaponSprites[i] = new SpriteSet(stream);
			delete stream;
		}

		stream = loadResourceToStream(spriteRes, missileSpriteID, "missile sprites");
		missileSprites = new SpriteSet(stream);
		delete stream;
	}

	initQuickMem(0x10000);

	//  Initialize actor appearance table
	for (i = 0; i < ARRAYSIZE(appearanceTable); i++) {
		ActorAppearance *aa = &appearanceTable[i];

		aa->_useCount = 0;
		g_vm->_appearanceLRU.push_front(aa);
	}
}

void cleanupSprites() {
	int         i;

	cleanupQuickMem();

	if (objectSprites)
		delete objectSprites;
	objectSprites = nullptr;

	if (mentalSprites)
		delete mentalSprites;
	mentalSprites = nullptr;

	for (i = 0; i < maxWeaponSpriteSets; i++) {
		if (weaponSprites[i]) {
			delete weaponSprites[i];
			weaponSprites[i] = nullptr;
		}
	}

	if (schemeRes) resFile->disposeContext(schemeRes);
	schemeRes = nullptr;

	if (poseRes) resFile->disposeContext(poseRes);
	poseRes = nullptr;

	if (frameRes) resFile->disposeContext(frameRes);
	frameRes = nullptr;

	if (spriteRes) resFile->disposeContext(spriteRes);
	spriteRes = nullptr;
}

} // end of namespace Saga2
