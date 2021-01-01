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

#ifndef ULTIMA8_WORLD_ITEM_H
#define ULTIMA8_WORLD_ITEM_H

#include "ultima/ultima8/kernel/object.h"
#include "ultima/ultima8/graphics/shape_info.h"

#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/box.h"
#include "ultima/ultima8/misc/point3.h"
#include "ultima/ultima8/misc/direction.h"

namespace Ultima {
namespace Ultima8 {

class Container;
class ShapeInfo;
class Shape;
class Gump;
class GravityProcess;

class Item : public Object {
	friend class ItemFactory;

public:
	Item();
	~Item() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	//! Get the Container this Item is in, if any. (0 if not in a Container)
	ObjId getParent() const {
		return _parent;
	}

	//! Set the parent container of this item.
	void setParent(ObjId p) {
		_parent = p;
	}

	//! Get the Container this Item is in, if any. (NULL if not in a Container)
	Container *getParentAsContainer() const;

	//! Get the top-most Container this Item is in, or the Item itself if not
	//! in a container
	Item *getTopItem();

	//! Set item location. This strictly sets the location, and does not
	//! even update CurrentMap
	void setLocation(int32 x, int32 y, int32 z); // this only sets the loc.

	//! Move an item. This moves an item to the new location, and updates
	//! CurrentMap and fastArea if necessary.
	void move(int32 x, int32 y, int32 z);

	//! Move, but with a point struct.
	void move(const Point3 &pt);

	//! Move an item. This moves an item to a container and  updates
	//! CurrentMap and fastArea if necessary.
	//! \param container The container this item should be placed in
	//! \return true if item was moved, false if failed
	bool moveToContainer(Container *container, bool checkwghtvol = false);

	//! Move an item to the Ethereal Void
	void moveToEtherealVoid();

	//! Move an item out of the Ethereal Void to where it originally was
	void returnFromEtherealVoid();

	//! Check if moving this item is stealing; call AvatarStoleSomething if so
	void movedByPlayer();

	//! Get the location of the top-most container this Item is in, or
	//! this Item's location if not in a container.
	void getLocationAbsolute(int32 &x, int32 &y, int32 &z) const;

	//! Get this Item's location. Note that this does not return
	//! 'usable' coordinates if the Item is contained or equipped.
	inline void getLocation(int32 &x, int32 &y, int32 &z) const;

	//! Get the Item's location using a Point3 struct.
	inline void getLocation(Point3 &pt) const;

	//! Get this Item's Z coordinate.
	int32 getZ() const;

	//! Set this Item's Z coordinate
	void setZ(int32 z) {
		_z = z;
	}

	//! Get this Item's location in a ContainerGump. Undefined if the Item
	//! is not in a Container.
	void getGumpLocation(int32 &x, int32 &y) const;

	//! Set the Item's location in a ContainerGump. NOP if the Item
	//! is not in a Container.
	void setGumpLocation(int32 x, int32 y);

	//! Randomize the Item's location in a ContainerGump. Effectively
	//! this sets the coordinates to (255,255) and lets the ContainerGump
	//! randomize the position when it is next opened.
	void randomGumpLocation();

	//! Get the world coordinates of the Item's centre. Undefined if the Item
	//! is contained or equipped.
	void getCentre(int32 &x, int32 &y, int32 &z) const;

	//! Get the size of this item's 3D bounding box, in world coordinates.
	inline void getFootpadWorld(int32 &x, int32 &y, int32 &z) const;

	//! Get the size of this item's 3D bounding box, scaled as in the datafiles
	//! (i.e., the dimensions are not in the same unit as world coordinates!)
	inline void getFootpadData(int32 &x, int32 &y, int32 &z) const;

	//! Get the Box this item occupies in the world. Undef if item is contained
	Box getWorldBox() const;

	//! Get all flags
	inline uint16 getFlags() const {
		return _flags;
	}

	//! Does this item have any of the given flags mask set
	inline bool hasFlags(uint16 flags) const {
		return (_flags & flags) != 0;
	}

	//! Set the flags set in the given mask.
	void setFlag(uint32 mask) {
		_flags |= mask;
	}

	virtual void setFlagRecursively(uint32 mask) {
		setFlag(mask);
	}

	//! Clear the flags set in the given mask.
	void clearFlag(uint32 mask) {
		_flags &= ~mask;
	}

	//! Set _extendedFlags
	void setExtFlags(uint32 f) {
		_extendedFlags = f;
	}

	//! Get _extendedFlags
	inline uint32 getExtFlags() const {
		return _extendedFlags;
	}

	//! Does item have any of the given extended flags
	inline bool hasExtFlags(uint32 flags) const {
		return (_extendedFlags & flags) != 0;
	}

	//! Set the _extendedFlags set in the given mask.
	void setExtFlag(uint32 mask) {
		_extendedFlags |= mask;
	}

	//! Clear the _extendedFlags set in the given mask.
	void clearExtFlag(uint32 mask) {
		_extendedFlags &= ~mask;
	}

	//! Get this Item's shape number
	uint32 getShape() const {
		return _shape;
	}

	//! Set this Item's shape number
	void setShape(uint32 shape);

	//! Get this Item's frame number
	uint32 getFrame() const {
		return _frame;
	}

	//! Set this Item's frame number
	void setFrame(uint32 frame) {
		_frame = frame;
	}

	//! Get this Item's quality (a.k.a. 'Q')
	uint16 getQuality() const {
		return _quality;
	}

	//! Set this Item's quality (a.k.a 'Q');
	void setQuality(uint16 quality) {
		_quality = quality;
	}

	//! Get the 'NpcNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	uint16 getNpcNum() const {
		return _npcNum;
	}

	//! Set the 'NpcNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	void setNpcNum(uint16 npcnum) {
		_npcNum = npcnum;
	}

	//! Get the 'MapNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	uint16 getMapNum() const {
		return _mapNum;
	}

	//! Set the 'MapNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	void setMapNum(uint16 mapnum) {
		_mapNum = mapnum;
	}

	//! Get the ShapeInfo object for this Item. (The pointer will be cached.)
	inline const ShapeInfo *getShapeInfo() const;

	//! Get the ShapeInfo object for this Item from the game instance.
	const ShapeInfo *getShapeInfoFromGameInstance() const;

	//! Get the Shape object for this Item. (The pointer will be cached.)
	const Shape *getShapeObject() const;

	//! Get the family of the shape number of this Item. (This is a
	//! member of the ShapeInfo object.)
	uint16 getFamily() const;

	//! Check if we can merge with another item.
	bool canMergeWith(Item *other);

	//! Get the open ContainerGump for this Item, if any. (NULL if not open.)
	ObjId getGump() const {
		return _gump;
	}
	//! Call this to notify the Item's open Gump has closed.
	void clearGump(); // set gump to 0 and clear the GUMP_OPEN flag
	//! Open a gump with the given shape for this Item
	ObjId openGump(uint32 gumpshape);
	//! Close this Item's gump, if any
	void closeGump();

	//! Destroy self.
	virtual void destroy(bool delnow = false);

	//! Check if this item overlaps another item in 3D world-space
	bool overlaps(const Item &item2) const;

	//! Check if this item overlaps another item in the xy dims in 3D space
	bool overlapsxy(const Item &item2) const;

	//! Check if this item is on top of another item
	bool isOn(const Item &item2) const;

	//! Check if this item is on completely on top of another item
	bool isCompletelyOn(const Item &item2) const;

	//! Check if the centre of this item is on top of another item
	bool isCentreOn(const Item &item2) const;

	//! Check if the item is currently visible on screen
	bool isOnScreen() const;

	//! Check if this item can exist at the given coordinates
	bool canExistAt(int32 x, int32 y, int32 z, bool needsupport = false) const;

	//! Get direction from centre to another item's centre.
	//! Undefined if either item is contained or equipped.
	Direction getDirToItemCentre(const Item &item2) const;

	//! Same as above, but from a fixed point.
	Direction getDirToItemCentre(const Point3 &pt) const;

	//! get 'distance' to other item. This is the maximum of the differences
	//! between the x, y (and possibly z) coordinates of the items.
	int getRange(const Item &item2, bool checkz = false) const;

	//! get 'distance' to other item if it's visible (ie, there's nothing blocking the path)
	int getRangeIfVisible(const Item &item2) const;

	//! Check if this item can reach another item. (This includes LoS.)
	//! \param other item to be reached
	//! \param range range
	//! \param x x coordinate of other to use, If zero, use real coords.
	//! \param y y coordinate of other to use
	//! \param z z coordinate of other to use.
	bool canReach(Item *other, int range, int32 x = 0, int32 y = 0, int32 z = 0);

	//! Move the object to (x,y,z) colliding with objects in the way.
	//! \param teleport move without colliding with objects between source and
	//!        destination
	//! \param force force the object to get to the destination without being
	//!        blocked by solid objects
	//! \param hititem if non-NULL, this is set to (one of) the item(s)
	//!        blocking the movement, or to zero if nothing blocked it
	//! \param dirs if non-NULL, this is set to a bitmask of the x/y/z
	//         directions in which movement was blocked (bit 0=x,1=y,2=z)
	//! \returns 0-0x4000 representing how far it got.
	//!          0 = didn't move
	//!          0x4000 = reached destination
	//! \note This can destroy the object
	virtual int32 collideMove(int32 x, int32 y, int32 z, bool teleport, bool force,
	                  ObjId *hititem = 0, uint8 *dirs = 0);

	//! Make the item move up (delta>0) or down (delta<0),
	//! including any items on top of it
	//! \param delta distance in Z-direction to move
	//! \returns 0-0x4000 representing how far it got.
	//!          0 = didn't move
	//!          0x4000 = reached destination
	int32 ascend(int delta);

	//! Make the item fall down.
	//! This creates a GravityProcess to do the actual work if the Item
	//! doesn't already have one.
	void fall();

	//! Make any items on top of this Item fall down and notify any supporting
	//! items that we're gone by calling the 'release' event.
	//! Note that this Item has to be moved away right after calling grab(),
	//! since otherwise the items will immediately hit this Item again.
	void grab();

	//! Hurl the item in the given direction
	void hurl(int xs, int ys, int zs, int grav);

	//! Set the PID of the GravityProcess for this Item
	void setGravityPID(ProcId pid) {
		_gravityPid = pid;
	}

	//! Get the PID of the GravityProcess for this Item (or 0)
	ProcId getGravityPID() const {
		return _gravityPid;
	}

	//! Get the GravityProcess of this Item, creating it if necessary
	virtual GravityProcess *ensureGravityProcess();

	//! Get the weight of this Item
	virtual uint32 getWeight() const;

	//! Get the weight of this Item and its contents, if any
	virtual uint32 getTotalWeight() const;

	//! Get the volume this item takes up in a container
	virtual uint32 getVolume() const;

	//! explode with explosion type (0,1,2), whether to destroy the item,
	//! and whether to cause splash damage.
	void explode(int explosion_type, bool destroy_item, bool cause_damage = true);

	//! get the damage type this object does when hitting something
	virtual uint16 getDamageType() const;

	//! receive a hit
	//! \param other The item delivering the hit
	//! \param dir The direction the hit is coming from (or inverse? CHECKME!)
	//! \param damage The force of the hit. Zero for default
	//! \param type The type of damage done. Zero for default
	virtual void receiveHit(ObjId other, Direction dir, int damage, uint16 type);

	//! fire the given weapon type in the given direction from location x, y, z.
	uint16 fireWeapon(int32 x, int32 y, int32 z, Direction dir, int firetype, char findtarget);

	//! get the distance (in map tiles) if we were to fire in this direction to "other"
	//! and could hit, otherwise return 0.
	uint16 fireDistance(Item *other, Direction dir, int16 xoff, int16 yoff, int16 zoff);

	//! get damage points, used in Crusader for item damage.
	uint8 getDamagePoints() const {
		return _damagePoints;
	}

	//! set damage points, used in Crusader for item damage.
	void setDamagePoints(uint8 points) {
		_damagePoints = points;
	}

	//! count nearby objects of a given shape
	unsigned int countNearby(uint32 shape, uint16 range);

	//! can this item be dragged?
	bool canDrag();

	//! how far can this item be thrown?
	//! \return range, or 0 if item can't be thrown
	int getThrowRange();

	//! Check this Item against the given loopscript
	//! \param script The loopscript to run
	//! \param scriptsize The size (in bytes) of the loopscript
	//! \return true if the item matches, false otherwise
	bool checkLoopScript(const uint8 *script, uint32 scriptsize) const;

	uint32 callUsecodeEvent_look();                             // event 0
	uint32 callUsecodeEvent_use();                              // event 1
	uint32 callUsecodeEvent_anim();                             // event 2
	uint32 callUsecodeEvent_cachein();                          // event 4
	uint32 callUsecodeEvent_hit(ObjId hitted, int16 hitforce);  // event 5
	uint32 callUsecodeEvent_gotHit(ObjId hitter, int16 hitforce);// event 6
	uint32 callUsecodeEvent_hatch();                            // event 7
	uint32 callUsecodeEvent_schedule(uint32 time);              // event 8
	uint32 callUsecodeEvent_release();                          // event 9
	uint32 callUsecodeEvent_equip();                            // event A
	uint32 callUsecodeEvent_equipWithParam(ObjId param);        // event A
	uint32 callUsecodeEvent_unequip();                          // event B
	uint32 callUsecodeEvent_unequipWithParam(ObjId param);      // event B
	uint32 callUsecodeEvent_combine();                          // event C
	uint32 callUsecodeEvent_calledFromAnim();                   // event E
	uint32 callUsecodeEvent_enterFastArea();                    // event F
	uint32 callUsecodeEvent_leaveFastArea();                    // event 10
	uint32 callUsecodeEvent_cast(uint16 unk);                   // event 11
	uint32 callUsecodeEvent_justMoved();                        // event 12
	uint32 callUsecodeEvent_AvatarStoleSomething(uint16 unk);   // event 14
	uint32 callUsecodeEvent_guardianBark(int16 unk);            // event 15 (Ultima)
	uint32 callUsecodeEvent_unhatch();							// event 15 (Crusader)

	uint32 use();

	//! Get lerped location.
	inline void getLerped(int32 &xp, int32 &yp, int32 &zp) const {
		xp = _ix;
		yp = _iy;
		zp = _iz;
	}

	//! Do lerping for an in between frame (0-256)
	//! The result can be retrieved with getLerped(x,y,z)
	//! \param factor The lerp factor: 0 is start of move, 256 is end of move
	inline void doLerp(int32 factor) {
		// Should be noted that this does indeed limit us to 'only' 24bit coords
		// not that it matters because on disk they are unsigned 16 bit

		if (factor == 256) {
			_ix = _lNext._x;
			_iy = _lNext._y;
			_iz = _lNext._z;
		} else if (factor == 0) {
			_ix = _lPrev._x;
			_iy = _lPrev._y;
			_iz = _lPrev._z;
		} else {
#if 1
			// This way while possibly slower is more accurate
			_ix = ((_lPrev._x * (256 - factor) + _lNext._x * factor) >> 8);
			_iy = ((_lPrev._y * (256 - factor) + _lNext._y * factor) >> 8);
			_iz = ((_lPrev._z * (256 - factor) + _lNext._z * factor) >> 8);
#else
			_ix = _lPrev.x + (((_lNext.x - _lPrev.x) * factor) >> 8);
			_iy = _lPrev.y + (((_lNext.y - _lPrev.y) * factor) >> 8);
			_iz = _lPrev.z + (((_lNext.z - _lPrev.z) * factor) >> 8);
#endif
		}
	}

	//! Setup the lerped info for this gametick and animate the item
	void setupLerp(int32 gametick);

	//! The item has entered the fast area
	virtual void enterFastArea();

	//! The item has left the fast area
	//! \note This can destroy the object
	virtual void leaveFastArea();

	//! dump some info about this item to pout
	void dumpInfo() const override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	// Intrinsics
	INTRINSIC(I_touch);
	INTRINSIC(I_getX);
	INTRINSIC(I_getY);
	INTRINSIC(I_getZ);
	INTRINSIC(I_getCX);
	INTRINSIC(I_getCY);
	INTRINSIC(I_getCZ);
	INTRINSIC(I_getPoint);
	INTRINSIC(I_getShape);
	INTRINSIC(I_setShape);
	INTRINSIC(I_getFrame);
	INTRINSIC(I_setFrame);
	INTRINSIC(I_getQuality);
	INTRINSIC(I_getUnkEggType);
	INTRINSIC(I_getQuantity);
	INTRINSIC(I_getContainer);
	INTRINSIC(I_getRootContainer);
	INTRINSIC(I_getQ);
	INTRINSIC(I_getQHi);
	INTRINSIC(I_getQLo);
	INTRINSIC(I_setQ);
	INTRINSIC(I_setQHi);
	INTRINSIC(I_setQLo);
	INTRINSIC(I_setQuality);
	INTRINSIC(I_setQuantity);
	INTRINSIC(I_setQAndCombine);
	INTRINSIC(I_getFamily);
	INTRINSIC(I_getTypeFlag);
	INTRINSIC(I_getStatus);
	INTRINSIC(I_orStatus);
	INTRINSIC(I_andStatus);
	INTRINSIC(I_getFootpadData);
	INTRINSIC(I_overlaps);
	INTRINSIC(I_overlapsXY);
	INTRINSIC(I_isOn);
	INTRINSIC(I_isCompletelyOn);
	INTRINSIC(I_isCentreOn);
	INTRINSIC(I_isInNpc);
	INTRINSIC(I_ascend);
	INTRINSIC(I_getWeight);
	INTRINSIC(I_getWeightIncludingContents);
	INTRINSIC(I_getVolume);
	INTRINSIC(I_bark);
	INTRINSIC(I_getMapArray);
	INTRINSIC(I_setMapArray);
	INTRINSIC(I_getNpcNum);
	INTRINSIC(I_setNpcNum);
	INTRINSIC(I_getDirToCoords);
	INTRINSIC(I_getDirFromCoords);
	INTRINSIC(I_getDirToItem);
	INTRINSIC(I_getDirFromItem);
	INTRINSIC(I_getDirFromTo16);
	INTRINSIC(I_getClosestDirectionInRange);
	INTRINSIC(I_look);
	INTRINSIC(I_use);
	INTRINSIC(I_gotHit);
	INTRINSIC(I_enterFastArea);
	INTRINSIC(I_cast);
	INTRINSIC(I_ask);
	INTRINSIC(I_getSliderInput);
	INTRINSIC(I_openGump);
	INTRINSIC(I_closeGump);
	INTRINSIC(I_create);
	INTRINSIC(I_legalCreateAtPoint);
	INTRINSIC(I_legalCreateAtCoords);
	INTRINSIC(I_legalCreateInCont);
	INTRINSIC(I_push);
	INTRINSIC(I_pop);
	INTRINSIC(I_popToCoords);
	INTRINSIC(I_popToContainer);
	INTRINSIC(I_popToEnd);
	INTRINSIC(I_destroy);
	INTRINSIC(I_move);
	INTRINSIC(I_legalMoveToPoint);
	INTRINSIC(I_legalMoveToContainer);
	INTRINSIC(I_hurl);
	INTRINSIC(I_shoot);
	INTRINSIC(I_fall);
	INTRINSIC(I_grab);
	INTRINSIC(I_igniteChaos);
	INTRINSIC(I_getFamilyOfType);
	INTRINSIC(I_getEtherealTop);
	INTRINSIC(I_guardianBark);
	INTRINSIC(I_getSurfaceWeight);
	INTRINSIC(I_isExplosive);
	INTRINSIC(I_receiveHit);
	INTRINSIC(I_explode);
	INTRINSIC(I_canReach);
	INTRINSIC(I_getRange);
	INTRINSIC(I_getRangeIfVisible);
	INTRINSIC(I_isCrusTypeNPC);
	INTRINSIC(I_setBroken);
	INTRINSIC(I_inFastArea);
	INTRINSIC(I_equip);
	INTRINSIC(I_unequip);
	INTRINSIC(I_avatarStoleSomething);
	INTRINSIC(I_isOnScreen);
	INTRINSIC(I_fireWeapon);
	INTRINSIC(I_fireDistance);

private:
	uint32 _shape;   // DO NOT modify this directly! Always use setShape()!

protected:
	uint32 _frame;

	int32 _x, _y, _z; // world coordinates
	uint16 _flags;
	uint16 _quality;
	uint16 _npcNum;
	uint16 _mapNum;

	uint32 _extendedFlags; // pentagram's own flags

	ObjId _parent; // objid container this item is in (or 0 for top-level items)

	mutable const Shape *_cachedShape;
	mutable const ShapeInfo *_cachedShapeInfo;

	// This is stuff that is used for displaying and interpolation
	struct Lerped {
		Lerped() : _x(0), _y(0), _z(0), _shape(0), _frame(0) {};
		int32 _x, _y, _z;
		uint32 _shape, _frame;
	};

	Lerped  _lPrev;         // Previous state (relative to camera)
	Lerped  _lNext;         // Next (current) state (relative to camera)
	int32   _ix, _iy, _iz;  // Interpolated position in camera space

	ObjId _gump;             // Item's gump
	ProcId _gravityPid;      // Item's GravityTracker (or 0)

	uint8 _damagePoints;	// Damage points, used for item damage in Crusader

	//! True if this is a Robot shape (in a fixed list)
	bool isRobotCru() const;

	//! Scale a received damage value based on the current difficulty level
	//! and the type of object this is.
	int scaleReceivedDamageCru(int damage, uint16 type) const;

private:

	//! Call a Usecode Event. Use the separate functions instead!
	uint32 callUsecodeEvent(uint32 event, const uint8 *args = 0, int argsize = 0);

	//! The gametick setupLerp was last called on
	int32 _lastSetup;

	//! Animate the item (called by setupLerp)
	void animateItem();

	//! The U8 version of receiveHit
	void receiveHitU8(ObjId other, Direction dir, int damage, uint16 type);

	//! The Crusader version of receiveHit
	void receiveHitCru(ObjId other, Direction dir, int damage, uint16 type);

	//! Get the right Z which an attacker should aim for, given the attacker's z.
	//! (Crusader only)
	int32 getTargetZRelativeToAttackerZ(int32 attackerz);

public:
	enum statusflags {
		FLG_DISPOSABLE   = 0x0002,  //!< Item is discarded on map change
		FLG_OWNED        = 0x0004,  //!< Item is owned by avatar
		FLG_CONTAINED    = 0x0008,  //!< Item is in a container
		FLG_INVISIBLE    = 0x0010,  //!< Item is invisible
		FLG_FLIPPED      = 0x0020,  //!< Item is flipped horizontally
		FLG_IN_NPC_LIST  = 0x0040,  //!< Item is a NPC
		FLG_FAST_ONLY    = 0x0080,  //!< Item is discarded when leaving fast area
		FLG_GUMP_OPEN    = 0x0100,  //!< Item has a gump open
		FLG_EQUIPPED     = 0x0200,  //!< Item is equipped
		FLG_BOUNCING     = 0x0400,  //!< Item has bounced
		FLG_ETHEREAL     = 0x0800,  //!< Item is in the ethereal list - confirmed same meaning in crusader
		FLG_HANGING      = 0x1000,  //!< Item is suspended in the air
		FLG_FASTAREA     = 0x2000,  //!< Item is in the fast area
		FLG_LOW_FRICTION = 0x4000,  //!< Item has low friction
		FLG_BROKEN       = 0x8000   //!< Item is broken - Crusader only - broken items are not targetable.
	};

	enum extflags {
		EXT_FIXED        = 0x0001,  //!< Item came from FIXED
		EXT_INCURMAP     = 0x0002,  //!< Item is in a CurrentMap display list
		EXT_LERP_NOPREV  = 0x0008,  //!< Item can't be lerped this frame
		EXT_HIGHLIGHT    = 0x0010,  //!< Item should be Painted highlighted
		EXT_CAMERA       = 0x0020,  //!< Item is being followed by the camera
		EXT_SPRITE       = 0x0040,  //!< Item is a sprite
		EXT_TRANSPARENT  = 0x0080,  //!< Item should be painted transparent
		EXT_PERMANENT_NPC = 0x0100, //!< Item is a permanent NPC
		EXT_TARGET 		 = 0x0200,  //!< Item is the current reticle target in Crusader
		EXT_FEMALE       = 0x8000	//!< Item is Crusader Female NPC (controls sfx)
	};
};

inline const ShapeInfo *Item::getShapeInfo() const {
	if (!_cachedShapeInfo)
		_cachedShapeInfo = getShapeInfoFromGameInstance();
	return _cachedShapeInfo;
}

inline void Item::getFootpadData(int32 &X, int32 &Y, int32 &Z) const {
	const ShapeInfo *si = getShapeInfo();
	Z = si->_z;

	if (_flags & Item::FLG_FLIPPED) {
		X = si->_y;
		Y = si->_x;
	} else {
		X = si->_x;
		Y = si->_y;
	}
}

// like getFootpadData, but scaled to world coordinates
inline void Item::getFootpadWorld(int32 &X, int32 &Y, int32 &Z) const {
	const ShapeInfo *si = getShapeInfo();
	si->getFootpadWorld(X, Y, Z, _flags & Item::FLG_FLIPPED);
}

inline void Item::getLocation(int32 &X, int32 &Y, int32 &Z) const {
	X = _x;
	Y = _y;
	Z = _z;
}

inline void Item::getLocation(Point3 &pt) const {
	pt.x = _x;
	pt.y = _y;
	pt.z = _z;
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
