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

#ifndef MADS_SPRITES_H
#define MADS_SPRITES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/assets.h"
#include "mads/msurface.h"

namespace MADS {

enum SpriteFlags {
	IMG_STATIC = 0,			// Item should remain fixed on the screen
	IMG_UPDATE = 1,			// Item needs to be redrawn
	IMG_ERASE = -1,			// Erase image and remove it
	IMG_REFRESH = -2,		// Full refresh
	IMG_OVERPRINT = -3,		// Interface overprint
	IMG_DELTA = -4,			// Delta change
	IMG_FULL_UPDATE = -5,	// Interface refresh
	IMG_UPDATE_ONLY = -20	// Update the active screen area only
};

class MADSEngine;

struct BGR8 {
	uint8 b, g, r;
};

typedef struct {
	int32	x;			// x position relative	to GrBuff(0, 0)
	int32	y;			// y position relative	to GrBuff(0, 0)
	int32	scale_x;	// x scale factor (can be negative for reverse draw)
	int32	scale_y;	// y scale factor (can't be negative)
	uint8*	depth_map;	// depth code array for destination (doesn't care if srcDepth is 0)
	BGR8*	Pal;		// palette for shadow draw (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8*	ICT;		// Inverse Color Table (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8	depth;		// depth code for source (0 if no depth processing)
} DrawRequestX;

typedef struct {
	uint32 Pack;
	uint32 Stream;
	long   hot_x;
	long   hot_y;
	uint32 Width;
	uint32 Height;
	uint32 Comp;
	uint32 Reserved[8];
	uint8* data;
} RendCell;

#define SS_HEADER_NUM_FIELDS 14
struct SpriteSeriesHeader {
	uint32 header;
	uint32 size;
	uint32 packing;
	uint32 frameRate;
	uint32 pixSpeed;
	uint32 maxWidth;
	uint32 maxHeight;
	uint32 reserved3;
	uint32 reserved4;
	uint32 reserved5;
	uint32 reserved6;
	uint32 reserved7;
	uint32 reserved8;
	uint32 count;
};

#define SF_HEADER_NUM_FIELDS 15
struct SpriteFrameHeader {
	uint32 pack;
	uint32 stream;
	uint32 x;
	uint32 y;
	uint32 width;
	uint32 height;
	uint32 comp;
	uint32 reserved1;
	uint32 reserved2;
	uint32 reserved3;
	uint32 reserved4;
	uint32 reserved5;
	uint32 reserved6;
	uint32 reserved7;
	uint32 reserved8;
};

class MSprite : public MSurface {
private:
	void loadSprite(Common::SeekableReadStream *source, const Common::Array<RGB6> &palette);
public:
	MSprite();
	MSprite(Common::SeekableReadStream *source, const Common::Array<RGB6> &palette,
		const Common::Rect &bounds);
	~MSprite() override;

	Common::Point _offset;
	int _transparencyIndex;

	byte getTransparencyIndex() const;
};

class SpriteSlotSubset {
public:
	int _spritesIndex;
	int _frameNumber;
	Common::Point _position;
	int _depth;
	int _scale;
};

class SpriteSlot : public SpriteSlotSubset {
private:
	static MADSEngine *_vm;
	friend class SpriteSlots;
public:
	SpriteFlags _flags;
	int _seqIndex;
public:
	SpriteSlot();
	SpriteSlot(SpriteFlags type, int seqIndex);

	void setup(int dirtyAreaIndex);
	bool operator==(const SpriteSlotSubset &other) const;
	void copy(const SpriteSlotSubset &other);
};

class SpriteSlots : public Common::Array<SpriteSlot> {
private:
	MADSEngine *_vm;
public:
	SpriteSlots(MADSEngine *vm);

	/**
	* Clears any pending slot data and schedules a full screen refresh.
	* @param flag		Also reset sprite list
	*/
	void reset(bool flag = true);

	/**
	* Delete a sprite entry
	* @param index		Specifies the index in the array
	*/
	void deleteEntry(int index);

	/**
	 * Setup dirty areas for the sprite slots
	 */
	void setDirtyAreas();

	/**
	* Adds a full screen refresh to the sprite slots
	*/
	void fullRefresh(bool clearAll = false);

	/**
	 * Delete a timer entry with the given Id
	 */
	void deleteTimer(int seqIndex);

	/**
	 * Add a new slot entry and return it's index
	 */
	int add();

	/**
	 * Draw any sprites into the background of the scene
	 */
	void drawBackground();

	/**
	* Draw any sprites into the foreground of the scene
	*/
	void drawSprites(MSurface *s);

	void cleanUp();
};

class SpriteSets : public Common::Array<SpriteAsset *> {
private:
	MADSEngine *_vm;
public:
	SpriteAsset *_uiSprites;
public:
	/**
	 * Constructor
	 */
	SpriteSets(MADSEngine *vm) : _vm(vm), _uiSprites(nullptr) {}

	/**
	 * Destructor
	 */
	~SpriteSets();

	/**
	 * Clears the current list, freeing any laoded assets
	 */
	void clear();

	/**
	* Add a sprite asset to the list
	*/
	int add(SpriteAsset *asset, int idx = 0);

	/**
	 * Adds a sprite asset to the list by name
	 */
	int addSprites(const Common::String &resName, int flags = 0);

	/**
	 * Remove an asset from the list
	 */
	void remove(int idx);

	SpriteAsset *&operator[](int idx);
};

} // End of namespace MADS

#endif /* MADS_SPRITES_H */
