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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/anim_dat.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/usecode/uc_stack.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/actors/actor_bark_notify_process.h"
#include "ultima/ultima8/gumps/container_gump.h"
#include "ultima/ultima8/gumps/paperdoll_gump.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/world/world_point.h"
#include "ultima/ultima8/world/gravity_process.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/gumps/slider_gump.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/snap_process.h"
#include "ultima/ultima8/world/super_sprite_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/missile_tracker.h"
#include "ultima/ultima8/world/crosshair_process.h"
#include "ultima/ultima8/world/actors/anim_action.h"

namespace Ultima {
namespace Ultima8 {

static const uint32 SNAP_EGG_SHAPE = 0x4fe;
static const uint32 BULLET_SPLASH_SHAPE = 0x1d9;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Item)

Item::Item()
	: _shape(0), _frame(0), _x(0), _y(0), _z(0),
	  _flags(0), _quality(0), _npcNum(0), _mapNum(0),
	  _extendedFlags(0), _parent(0),
	  _cachedShape(nullptr), _cachedShapeInfo(nullptr),
	  _gump(0), _gravityPid(0), _lastSetup(0),
	  _ix(0), _iy(0), _iz(0), _damagePoints(1) {
}


Item::~Item() {
}

void Item::dumpInfo() const {
	pout << "Item " << getObjId() << " (class "
	     << GetClassType()._className << ", shape "
		 << getShape();

	const char *ucname = GameData::get_instance()->getMainUsecode()->get_class_name(_shape);
	if (ucname != nullptr) {
		pout << " (uc:" << ucname << ")";
	}

	pout << ", " << getFrame() << ", (";

	if (_parent) {
		int32 gx, gy;
		getGumpLocation(gx, gy);
		pout << gx << "," << gy;
	} else {
		pout << _x << "," << _y << "," << _z;
	}

	pout << ") q:" << getQuality()
	     << ", m:" << getMapNum() << ", n:" << getNpcNum()
	     << ", f:0x" << Std::hex << getFlags() << ", ef:0x"
		 << getExtFlags();

	const ShapeInfo *info = getShapeInfo();
	if (info) {
		pout << " shapeinfo f:" << info->_flags << ", fam:"
			 << info->_family << ", et:" << info->_equipType;
	}

	pout << ")" << Std::dec << Std::endl;
}

Container *Item::getParentAsContainer() const {
	// No parent, no container
	if (!_parent)
		return nullptr;

	Container *p = getContainer(_parent);

	if (!p) {
		perr << "Item " << getObjId() << " _parent (" << _parent << ") is an invalid Container ObjID" << Std::endl;
		CANT_HAPPEN();
	}

	return p;
}

Item *Item::getTopItem() {
	Container *parentItem = getParentAsContainer();

	if (!parentItem) return this;

	while (parentItem->getParentAsContainer()) {
		parentItem = parentItem->getParentAsContainer();
	}

	return parentItem;
}

void Item::setLocation(int32 X, int32 Y, int32 Z) {
	_x = X;
	_y = Y;
	_z = Z;
}

void Item::move(const Point3 &pt) {
	move(pt.x, pt.y, pt.z);
}

void Item::move(int32 X, int32 Y, int32 Z) {
	bool no_lerping = false;
	CurrentMap *map = World::get_instance()->getCurrentMap();
	int mapChunkSize = map->getChunkSize();

	if (getObjId() == 1 && Z < 0) {
		perr.Print("Warning: moving avatar below Z=0. (%d,%d,%d)\n", X, Y, Z);
	}

	// TODO: In Crusader, if we are moving the avatar the game also checks whether
	// there is an active "shield zap" sprite that needs moving at the same time.

	// It's currently in the ethereal void, remove it from there
	if (_flags & FLG_ETHEREAL) {

		// Remove us from the ethereal void
		World::get_instance()->etherealRemove(_objId);
	}

	// Remove from container (if contained or equiped)
	if (_flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		if (_parent) {
			// If we are flagged as Ethereal, we are already removed
			if (!(_flags & FLG_ETHEREAL)) {
				Container *p = getParentAsContainer();
				if (p) p->removeItem(this);
			}
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no _parent" << Std::endl;

		// Clear our owner.
		_parent = 0;

		// No lerping when going from a container to somewhere else
		no_lerping = true;
	}
	// Item needs to be removed if it in the map, and it is moving to a
	// different chunk
	else if ((_extendedFlags & EXT_INCURMAP) &&
	         ((_x / mapChunkSize != X / mapChunkSize) ||
	          (_y / mapChunkSize != Y / mapChunkSize))) {

		// Remove us from the map
		map->removeItem(this);
	}

	// Unset all the various _flags that no longer apply
	_flags &= ~(FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL);

	// Set the location
	_x = X;
	_y = Y;
	_z = Z;

	// Add it to the map if needed
	if (!(_extendedFlags & EXT_INCURMAP)) {
		// Disposable fast only items get put at the end
		// While normal items get put at start
		if (_flags & (FLG_DISPOSABLE | FLG_FAST_ONLY))
			map->addItemToEnd(this);
		else
			map->addItem(this);
	}

	// Call just moved
	callUsecodeEvent_justMoved();

	// Are we moving somewhere fast
	bool dest_fast = map->isChunkFast(X / mapChunkSize, Y / mapChunkSize);

	// No lerping for this move
	if (no_lerping) _extendedFlags |= EXT_LERP_NOPREV;

	// If the destination is not in the fast area, and we are in
	// the fast area, we need to call leaveFastArea, as long as we aren't
	// being followed by the camera. We also disable lerping in such a case
	if (!dest_fast && (_flags & Item::FLG_FASTAREA)) {
		_extendedFlags |= EXT_LERP_NOPREV;
		if (_extendedFlags & EXT_CAMERA)
			CameraProcess::GetCameraProcess()->ItemMoved();
		else
			leaveFastArea();

		return; //we are done
	}
	// Or if the dest is fast, and we are not, we need to call enterFastArea
	else if (dest_fast && !(_flags & Item::FLG_FASTAREA)) {
		_extendedFlags |= EXT_LERP_NOPREV;
		enterFastArea();
	}

	// If we are being followed, notify the camera that we moved
	// Note that we don't need to
	if (_extendedFlags & EXT_CAMERA)
		CameraProcess::GetCameraProcess()->ItemMoved();

	if (_extendedFlags & EXT_TARGET)
		TargetReticleProcess::get_instance()->itemMoved(this);
}

bool Item::moveToContainer(Container *container, bool checkwghtvol) {
	// Null container, report an error message
	if (!container) {
		perr << "NULL container passed to Item::moveToContainer" << Std::endl;
		return false;
	}

	// Already there, do nothing, but only if not ethereal
	bool ethereal_same = false;
	if (container->getObjId() == _parent) {
		// If we are ethereal we'd like to know if we are just being moved back
		if (_flags & FLG_ETHEREAL) ethereal_same = true;
		else return true;
	}

	// Eh, can't do it
	if (!container->CanAddItem(this, checkwghtvol)) return false;

	// It's currently in the ethereal void
	if (_flags & FLG_ETHEREAL) {

		// Remove us from the ethereal void
		World::get_instance()->etherealRemove(_objId);
	}

	// Remove from container (if contained or equiped)
	if (_flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		if (_parent) {
			// If we are flagged as Ethereal, we are already removed
			if (!(_flags & FLG_ETHEREAL)) {
				Container *p = getParentAsContainer();
				if (p) p->removeItem(this);
			}
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no _parent" << Std::endl;

		// Clear our owner.
		_parent = 0;
	}
	// Item needs to be removed if it in the map
	else if (_extendedFlags & EXT_INCURMAP) {

		// Remove us from the map
		World::get_instance()->getCurrentMap()->removeItem(this);
	}

	// Unset all the various _flags that no longer apply
	_flags &= ~(FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL);

	// Set location to 0,0,0 if we aren't an ethereal item moving back
	if (!ethereal_same) _x = _y = 0;
	_z = 0;

	// Add the item, don't bother with checking weight or vol since we already
	// know if it will fit or not
	container->addItem(this, false);

	// Set our owner.
	_parent = container->getObjId();

	// Set us contained
	_flags |= FLG_CONTAINED;

	// If moving to avatar, mark as OWNED
	Item *p = this;
	while (p->getParentAsContainer())
		p = p->getParentAsContainer();
	// In Avatar's inventory?
	if (p->getObjId() == 1)
		setFlagRecursively(FLG_OWNED);

	// No lerping when moving to a container
	_extendedFlags |= EXT_LERP_NOPREV;

	// Call just moved
	callUsecodeEvent_justMoved();

	// For a container, it's fast if it's got a _gump open
	bool dest_fast = (container->_flags & FLG_GUMP_OPEN) != 0;

	// If the destination is not in the fast area, and we are in
	// the fast area, we need to call leaveFastArea
	if (!dest_fast && (_flags & Item::FLG_FASTAREA))
		leaveFastArea();
	// Or if the dest is fast, and we are not, we need to call enterFastArea
	else if (dest_fast && !(_flags & Item::FLG_FASTAREA))
		enterFastArea();

	// Done
	return true;
}

void Item::moveToEtherealVoid() {
	// It's already Ethereal
	if (_flags & FLG_ETHEREAL) return;

	// Add it to the ethereal void
	World::get_instance()->etherealPush(_objId);

	// It's owned by something removed it from the something, but keep _flags
	if (_flags & (FLG_CONTAINED | FLG_EQUIPPED)) {

		if (_parent) {
			Container *p = getParentAsContainer();
			if (p) p->removeItem(this);
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no _parent" << Std::endl;
	} else if (_extendedFlags & EXT_INCURMAP) {
		World::get_instance()->getCurrentMap()->removeItem(this);
	}

	// Set the ETHEREAL Flag
	_flags |=  FLG_ETHEREAL;
}

void Item::returnFromEtherealVoid() {
	// It's not Ethereal
	if (!(_flags & FLG_ETHEREAL)) return;

	// Ok, we can do 2 things here
	// If an item has the contained or Equipped _flags set, we return it to it's owner
	if (_flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		Container *p = getParentAsContainer();
		if (!p) {
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no valid _parent" << Std::endl;
			CANT_HAPPEN();
		}
		moveToContainer(p);
	}
	// or we return it to the world
	else {
		move(_x, _y, _z);
	}

}

void Item::movedByPlayer() {
	// owned-by-avatar items can't be stolen
	if (_flags & FLG_OWNED) return;

	// Otherwise, player is stealing.
	// See if anybody is around to notice.
	Item *avatar = getItem(1);
	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	currentmap->areaSearch(&itemlist, script, sizeof(script), avatar, 640, 0);

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Actor *actor = getActor(itemlist.getuint16(i));
		if (actor && !actor->isDead())
			actor->callUsecodeEvent_AvatarStoleSomething(getObjId());
	}
}

int32 Item::getZ() const {
	return _z;
}

void Item::getLocationAbsolute(int32 &X, int32 &Y, int32 &Z) const {
	if (_parent) {
		Item *p = getParentAsContainer();

		if (p) {
			p->getLocationAbsolute(X, Y, Z);
			return;
		}
	}

	X = _x;
	Y = _y;
	Z = _z;
}

void Item::getGumpLocation(int32 &X, int32 &Y) const {
	if (!_parent) return;

	X = _y & 0xFF;
	Y = (_y >> 8) & 0xFF;
}

void Item::setGumpLocation(int32 X, int32 Y) {
	if (!_parent) return;

	_y = (X & 0xFF) + ((Y & 0xFF) << 8);
}

void Item::randomGumpLocation() {
	if (!_parent) return;

	// This sets the coordinates to (255,255) and lets the ContainerGump
	// randomize the position when it is next opened.
	_y = 0xFFFF;
}

void Item::getCentre(int32 &X, int32 &Y, int32 &Z) const {
	// constants!
	const ShapeInfo *shapeinfo = getShapeInfo();
	if (_flags & FLG_FLIPPED) {
		X = _x - shapeinfo->_y * 16;
		Y = _y - shapeinfo->_x * 16;
	} else {
		X = _x - shapeinfo->_x * 16;
		Y = _y - shapeinfo->_y * 16;
	}

	Z = _z + shapeinfo->_z * 4;
}

Box Item::getWorldBox() const {
	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	return Box(_x, _y, _z, xd, yd, zd);
}

void Item::setShape(uint32 shape) {
	_shape = shape;
	_cachedShapeInfo = nullptr;
	_cachedShape = nullptr;
	// FIXME: In Crusader, here we should check if the shape
	// changed from targetable to not-targetable, or vice-versa
}

bool Item::overlaps(const Item &item2) const {
	int32 x1a, y1a, z1a, x1b, y1b, z1b;
	int32 x2a, y2a, z2a, x2b, y2b, z2b;
	getLocation(x1b, y1b, z1a);
	item2.getLocation(x2b, y2b, z2a);

	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	x1a = x1b - xd;
	y1a = y1b - yd;
	z1b = z1a + zd;

	item2.getFootpadWorld(xd, yd, zd);
	x2a = x2b - xd;
	y2a = y2b - yd;
	z2b = z2a + zd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	if (z1b <= z2a || z2b <= z1a) return false;
	return true;
}

bool Item::overlapsxy(const Item &item2) const {
	int32 x1a, y1a, z1a, x1b, y1b;
	int32 x2a, y2a, z2a, x2b, y2b;
	getLocation(x1b, y1b, z1a);
	item2.getLocation(x2b, y2b, z2a);

	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	x1a = x1b - xd;
	y1a = y1b - yd;

	item2.getFootpadWorld(xd, yd, zd);
	x2a = x2b - xd;
	y2a = y2b - yd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	return true;
}

bool Item::isOn(const Item &item2) const {
	int32 x1a, y1a, z1a, x1b, y1b;
	int32 x2a, y2a, z2a, x2b, y2b, z2b;
	getLocation(x1b, y1b, z1a);
	item2.getLocation(x2b, y2b, z2a);

	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	x1a = x1b - xd;
	y1a = y1b - yd;

	item2.getFootpadWorld(xd, yd, zd);
	x2a = x2b - xd;
	y2a = y2b - yd;
	z2b = z2a + zd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	if (z2b == z1a) return true;
	return false;
}

bool Item::isCompletelyOn(const Item &item2) const {
	if (hasFlags(FLG_CONTAINED) || item2.hasFlags(FLG_CONTAINED))
		return false;

	int32 x1a, y1a, z1a, x1b, y1b;
	int32 x2a, y2a, z2a, x2b, y2b, z2b;
	getLocation(x1b, y1b, z1a);
	item2.getLocation(x2b, y2b, z2a);

	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	x1a = x1b - xd;
	y1a = y1b - yd;

	item2.getFootpadWorld(xd, yd, zd);
	x2a = x2b - xd;
	y2a = y2b - yd;
	z2b = z2a + zd;

	return ((x1b <= x2b && x2a <= x1a) &&
			(y1b <= y2b && y2a <= y1a) &&
			(z2b == z1a));
}

bool Item::isCentreOn(const Item &item2) const {
	int32 x1c, y1c, z1c;
	int32 x2a, y2a, z2a, x2b, y2b, z2b;
	item2.getLocation(x2b, y2b, z2a);

	getCentre(x1c, y1c, z1c);

	int32 xd, yd, zd;
	item2.getFootpadWorld(xd, yd, zd);
	x2a = x2b - xd;
	y2a = y2b - yd;
	z2b = z2a + zd;

	if (x1c <= x2a || x2b <= x1c) return false;
	if (y1c <= y2a || y2b <= y1c) return false;
	if (z2b == getZ()) return true;
	return false;
}

bool Item::isOnScreen() const {
	GameMapGump *game_map = Ultima8Engine::get_instance()->getGameMapGump();

	if (!game_map)
		return false;

	Rect game_map_dims;
	int32 screenx = -1;
	int32 screeny = -1;
	game_map->GetLocationOfItem(_objId, screenx, screeny);
	game_map->GetDims(game_map_dims);
	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);

	if (game_map_dims.contains(screenx, screeny) &&
	    game_map_dims.contains(screenx + xd, screeny + yd)) {
		return true;
	}

	return false;
}

bool Item::canExistAt(int32 x, int32 y, int32 z, bool needsupport) const {
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	const Item *support;
	bool valid = cm->isValidPosition(x, y, z, getShape(), getObjId(),
	                                 &support, 0);
	return valid && (!needsupport || support);
}

Direction Item::getDirToItemCentre(const Item &item2) const {
	int32 xv, yv, zv;
	getCentre(xv, yv, zv);

	int32 i2x, i2y, i2z;
	item2.getCentre(i2x, i2y, i2z);

	return Direction_GetWorldDir(i2y - yv, i2x - xv, dirmode_8dirs);
}

Direction Item::getDirToItemCentre(const Point3 &pt) const {
	int32 xv, yv, zv;
	getCentre(xv, yv, zv);

	return Direction_GetWorldDir(pt.y - yv, pt.x - xv, dirmode_8dirs);
}


int Item::getRange(const Item &item2, bool checkz) const {
	int32 thisX, thisY, thisZ;
	int32 otherX, otherY, otherZ;
	int32 thisXd, thisYd, thisZd;
	int32 otherXd, otherYd, otherZd;
	int32 thisXmin, thisYmin, thisZmax;
	int32 otherXmin, otherYmin, otherZmax;

	getLocationAbsolute(thisX, thisY, thisZ);
	item2.getLocationAbsolute(otherX, otherY, otherZ);
	getFootpadWorld(thisXd, thisYd, thisZd);
	item2.getFootpadWorld(otherXd, otherYd, otherZd);

	thisXmin = thisX - thisXd;
	thisYmin = thisY - thisYd;
	thisZmax = thisZ + thisZd;

	otherXmin = otherX - otherXd;
	otherYmin = otherY - otherYd;
	otherZmax = otherZ + otherZd;

	int32 range = 0;

	if (thisXmin - otherX > range)
		range = thisYmin - otherY;
	if (otherXmin - thisX > range)
		range = thisXmin - otherX;
	if (thisYmin - otherY > range)
		range = otherXmin - thisX;
	if (otherYmin - thisY > range)
		range = otherYmin - thisY;
	if (checkz && thisZ - otherZmax > range)
		range = thisZ - otherZmax;
	if (checkz && otherZ - thisZmax > range)
		range = otherZ - thisZmax;

	return range;
}

int Item::getRangeIfVisible(const Item &item2) const {
	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();
	int32 start[3];
	int32 end[3];
	int32 dims[3] = {1, 1, 1};
	Std::list<CurrentMap::SweepItem> hitItems;
	getCentre(start[0], start[1], start[2]);
	item2.getCentre(end[0], end[1], end[2]);

	int xdiff = abs(start[0] - end[0]);
	int ydiff = abs(start[1] - end[1]);
	int zdiff = abs(start[2] - end[2]);

	map->sweepTest(start, end, dims, getShapeInfo()->_flags, _objId, true, &hitItems);

	if (hitItems.size() > 0) {
		for (Std::list<CurrentMap::SweepItem>::const_iterator it = hitItems.begin();
			 it != hitItems.end();
			 it++) {
			int objId = it->_item;
			if (it->_blocking && objId != _objId && objId != item2.getObjId()) {
				//int out[3];
				//it->GetInterpolatedCoords(out, start, end);
				//warning("found blocking item %d at %d %d %d.", objId, out[0], out[1], out[2]);
				return 0;
			}
		}
	}

	int distance = MAX(MAX(xdiff, ydiff), zdiff);
	return distance;
}

const ShapeInfo *Item::getShapeInfoFromGameInstance() const {
	return GameData::get_instance()->getMainShapes()->getShapeInfo(_shape);
}

const Shape *Item::getShapeObject() const {
	if (!_cachedShape) _cachedShape = GameData::get_instance()->getMainShapes()->getShape(_shape);
	return _cachedShape;
}

uint16 Item::getFamily() const {
	const ShapeInfo *info = getShapeInfo();
	if (!info)
		return 0;
	return static_cast<uint16>(info->_family);
}

uint32 Item::getWeight() const {
	uint32 weight = getShapeInfo()->_weight;

	switch (getShapeInfo()->_family) {
	case ShapeInfo::SF_QUANTITY:
		return ((getQuality() * weight) + 9) / 10;
	case ShapeInfo::SF_REAGENT:
		return getQuality() * weight;
	default:
		return weight * 10;
	}
}

uint32 Item::getTotalWeight() const {
	return getWeight();
}

uint32 Item::getVolume() const {
	// invisible items (trap markers and such) don't take up volume
	if (hasFlags(FLG_INVISIBLE))
		return 0;


	uint32 volume = getShapeInfo()->_volume;

	switch (getShapeInfo()->_family) {
	case ShapeInfo::SF_QUANTITY:
		return ((getQuality() * volume) + 99) / 100;
	case ShapeInfo::SF_REAGENT:
		return ((getQuality() * volume) + 9) / 10;
	case ShapeInfo::SF_CONTAINER:
		return (volume == 0) ? 1 : volume;
	default:
		return volume;
	}
}

bool Item::checkLoopScript(const uint8 *script, uint32 scriptsize) const {
	// if really necessary this could be made static to prevent news/deletes
	DynamicUCStack stack(0x40); // 64bytes should be plenty of room

	unsigned int i = 0;

	uint16 ui16a, ui16b;

	stack.push2(1); // default to true if script is empty

	while (i < scriptsize) {
		switch (script[i]) {
		case LS_TOKEN_FALSE: // false
			stack.push2(0);
			break;

		case LS_TOKEN_TRUE: // true
			stack.push2(1);
			break;

		case LS_TOKEN_END: // end
			ui16a = stack.pop2();
			return (ui16a != 0);

		case LS_TOKEN_INT: // int
			//! check for i out of bounds
			ui16a = script[i + 1] + (script[i + 2] << 8);
			stack.push2(ui16a);
			i += 2;
			break;

		case LS_TOKEN_AND: // and
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 && ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_OR: // or
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 || ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_NOT: // not
			ui16a = stack.pop2();
			if (ui16a != 0)
				stack.push2(0);
			else
				stack.push2(1);
			break;

		case LS_TOKEN_STATUS: // item status
			stack.push2(getFlags());
			break;

		case LS_TOKEN_Q: // item _quality
			stack.push2(getQuality());
			break;

		case LS_TOKEN_NPCNUM: // npc num
			stack.push2(getNpcNum());
			break;

		case LS_TOKEN_EQUAL: // equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a == ui16b)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_GREATER: // greater than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b > ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_LESS: // less than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b < ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_GEQUAL: // greater or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b >= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_LEQUAL: // smaller or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b <= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_FAMILY: // item family
			stack.push2(getFamily());
			break;

		case LS_TOKEN_SHAPE: // item _shape
			stack.push2(static_cast<uint16>(getShape()));
			break;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z': {
			bool match = false;
			int count = script[i] - '@';
			for (int j = 0; j < count; j++) {
				//! check for i out of bounds
				if (getShape() == static_cast<uint32>(script[i + 1] + (script[i + 2] << 8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;

		case LS_TOKEN_FRAME: // item _frame
			stack.push2(static_cast<uint16>(getFrame()));
			break;

		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': {
			bool match = false;
			int count = script[i] - '`';
			for (int j = 0; j < count; j++) {
				//! check for i out of bounds
				if (getFrame() == static_cast<uint32>(script[i + 1] + (script[i + 2] << 8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;

		default:
			perr.Print("Unknown loopscript opcode %02X\n", script[i]);
		}

		i++;
	}
	perr.Print("Didn't encounter $ in loopscript\n");
	return false;
}


int32 Item::collideMove(int32 dx, int32 dy, int32 dz, bool teleport, bool force, ObjId *hititem, uint8 *dirs) {
	Ultima8Engine *guiapp = Ultima8Engine::get_instance();
	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();

	if (hititem) *hititem = 0;
	if (dirs) *dirs = 0;

	int32 end[3] = { dx, dy, dz };

	int32 start[3];
	if (_parent) {
		// If we are moving from a container, only check the destination
		start[0] = end[0];
		start[1] = end[1];
		start[2] = end[2];
	} else {
		// Otherwise check from where we are to where we want to go
		getLocation(start[0], start[1], start[2]);
	}

	int32 dims[3];
	getFootpadWorld(dims[0], dims[1], dims[2]);

	// Do the sweep test
	Std::list<CurrentMap::SweepItem> collisions;
	Std::list<CurrentMap::SweepItem>::iterator it;
	map->sweepTest(start, end, dims, getShapeInfo()->_flags, _objId, false, &collisions);

	// Ok, now to work out what to do


	// the force of the hit, used for the gotHit/hit usecode calls
	int deltax = ABS(start[0] - end[0]) / 4;
	int deltay = ABS(start[1] - end[1]) / 4;
	int deltaz = ABS(start[2] - end[2]);
	int maxdirdelta = deltay;
	if (deltay > maxdirdelta) maxdirdelta = deltay;
	if (deltaz > maxdirdelta) maxdirdelta = deltaz;
	int hitforce = (deltax + deltay + deltaz + maxdirdelta) / 2;

	// if we are contained, we always teleport
	if (teleport || _parent) {
		// If teleporting and not force, work out if we can reach the end
		if (!force) {
			for (it = collisions.begin(); it != collisions.end(); it++) {
				// Uh oh, we hit something, can't move
				if (it->_endTime == 0x4000 && !it->_touching && it->_blocking) {
					if (hititem) *hititem = it->_item;
					if (dirs) *dirs = it->_dirs;
					return 0;
				}
			}
		}

		// Trigger all the required events
		bool we_were_released = false;
		for (it = collisions.begin(); it != collisions.end(); it++) {
			Item *item = getItem(it->_item);
			if (!item)
				continue; // shouldn't happen..

			// Hitting us at the start and end, don't do anything
			if (!_parent && it->_hitTime == 0x0000 &&
			        it->_endTime == 0x4000) {
				continue;
			}
			// Hitting us at the end (call hit on us, got hit on them)
			else if (it->_endTime == 0x4000) {
				if (_objId == 1 && guiapp->isShowTouchingItems())
					item->setExtFlag(Item::EXT_HIGHLIGHT);

				item->callUsecodeEvent_gotHit(_objId, hitforce);
				callUsecodeEvent_hit(item->getObjId(), hitforce);
			}
			// Hitting us at the start (call release on us and them)
			else if (!_parent && it->_hitTime == 0x0000) {
				if (_objId == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
				we_were_released = true;
				item->callUsecodeEvent_release();
			}
		}

		// Call release on us
		if (we_were_released) callUsecodeEvent_release();

		// Move US!
		move(end[0], end[1], end[2]);

		// We reached the end
		return 0x4000;
	} else {
		int32 hit = 0x4000;

		// If not force, work out if we can reach the end
		// if not, need to do 'stuff'
		// We don't care about items hitting us at the start
		if (!force) {
			for (it = collisions.begin(); it != collisions.end(); it++) {
				if (it->_blocking && !it->_touching) {
					if (hititem) *hititem = it->_item;
					if (dirs) *dirs = it->_dirs;
					hit = it->_hitTime;
					break;
				}
			}
			if (hit < 0) hit = 0;

			if (hit != 0x4000) {
#if 0
				pout << " Hit time: " << hit << Std::endl;
				pout << "    Start: " << start[0] << ", " << start[1] << ", " << start[2] << Std::endl;
				pout << "      End: " << end[0] << ", " << end[1] << ", " << end[2] << Std::endl;
#endif
				it->GetInterpolatedCoords(end, start, end);
#if 0
				pout << "Collision: " << end[0] << ", " << end[1] << ", " << end[2] << Std::endl;
#endif
			}
		}

		// Trigger all the required events
		bool we_were_released = false;
		for (it = collisions.begin(); it != collisions.end(); it++) {
			Item *item = getItem(it->_item);
			if (!item) continue;

			// Did we go past the endpoint of the move?
			if (it->_hitTime > hit) break;

			uint16 proc_gothit = 0, proc_rel = 0;

			// If hitting at start, we should have already
			// called gotHit and hit.
			if ((!it->_touching || it->_touchingFloor) && it->_hitTime >= 0) {
				if (_objId == 1 && guiapp->isShowTouchingItems())
					item->setExtFlag(Item::EXT_HIGHLIGHT);

				proc_gothit = item->callUsecodeEvent_gotHit(_objId, hitforce);
				callUsecodeEvent_hit(item->getObjId(), hitforce);
			}

			// If not hitting at end, we will need to call release
			if (it->_endTime < hit) {
				if (_objId == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
				we_were_released = true;
				proc_rel = item->callUsecodeEvent_release();
			}

			// We want to make sure that release is called AFTER gotHit.
			if (proc_rel && proc_gothit) {
				Process *p = Kernel::get_instance()->getProcess(proc_rel);
				p->waitFor(proc_gothit);
			}
		}

		// Call release on us
		if (we_were_released) callUsecodeEvent_release();

		// Move US!
		move(end[0], end[1], end[2]);

		return hit;
	}

	return 0;
}

uint16 Item::fireWeapon(int32 x, int32 y, int32 z, Direction dir, int firetype, char findtarget) {
	int32 ix, iy, iz;
	getLocation(ix, iy, iz);

	if (!GAME_IS_CRUSADER)
		return 0;

	if (GAME_IS_REGRET)
		warning("Item::fireWeapon: TODO: Update for Regret (different firetypes)");

	ix += x;
	iy += y;
	iz += z;

	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	const FireType *firetypedat = GameData::get_instance()->getFireType(firetype);

	if (!firetypedat)
		return 0;

	int damage = firetypedat->getRandomDamage();

	const Item *blocker = nullptr;
	// CHECKME: the original doesn't exclude the source like this,
	// but it seems obvious we have to or NPCs shoot themselves?
	bool isvalid = currentmap->isValidPosition(ix, iy, iz, BULLET_SPLASH_SHAPE, _objId, nullptr, nullptr, &blocker);
	if (!isvalid && blocker) {
		Item *block = getItem(blocker->getObjId());
		Point3 blockpt;
		block->getLocation(blockpt);
		Direction damagedir = Direction_GetWorldDir(blockpt.y - iy, blockpt.x - ix, dirmode_8dirs);
		block->receiveHit(getObjId(), damagedir, damage, firetype);
		int splashdamage = firetypedat->getRandomDamage();
		firetypedat->applySplashDamageAround(blockpt, splashdamage, block, this);

		firetypedat->makeBulletSplashShapeAndPlaySound(ix, iy, iz);
		return 0;
	} else {
		int spriteframe = 0;
		switch (firetype) {
		case 3:
		case 9:
		case 10:
			spriteframe = dir + 0x11;
			break;
		case 5:
			spriteframe = dir + 1;
			break;
		case 6:
			spriteframe = 0x46;
			break;
		case 0xe:
			spriteframe = 0x47 + getRandom() % 5;
			break;
		case 0xf:
			spriteframe = 0x4c;
			break;
		}

		// HACK: this should be fixed to use inheritence so the behavior
		// is clean for both Item and Actor.
		DirectionMode dirmode = dirmode_8dirs;
		const Actor *thisactor = dynamic_cast<Actor *>(this);
		if (thisactor) {
			// TODO: Get damage for active inventory item
			dirmode = thisactor->animDirMode(thisactor->getLastAnim());
		}

		Item *target = nullptr;
		if (findtarget) {
			if (this != getControlledActor()) {
				target = getControlledActor();
			} else {
				target = currentmap->findBestTargetItem(ix, iy, dir, dirmode);
			}
		}

		int32 tx = -1;
		int32 ty = 0;
		int32 tz = 0;
		if (target) {
			target->getCentre(tx, ty, tz);
			tz = target->getTargetZRelativeToAttackerZ(getZ());
		}

		// TODO: check if we need the equivalent of FUN_1130_0299 here..
		// maybe not? It seems to reset the target reticle etc which we
		// handle differently.

		int numshots = firetypedat->getNumShots();
		uint16 spriteprocpid = 0;
		for (int i = 0; i < numshots; i++) {
			SuperSpriteProcess *ssp;
			CrosshairProcess *chp = CrosshairProcess::get_instance();
			assert(chp);
			Item *crosshair = getItem(chp->getItemNum());
			int32 ssx, ssy, ssz;
			if (tx != -1) {
				// Shoot toward the target
				ssx = tx;
				ssy = ty;
				ssz = tz;
			} else if (this == getControlledActor() && crosshair) {
				// Shoot toward the crosshair
				crosshair->getLocation(ssx, ssy, ssz);
				ssz = iz;
			} else {
				// Just send the projectile off into the distance
				// CHECKME: This is not how the game does it - it has different
				// tables (at 1478:129b and 1478:12ac). Check the logic here.
				ssx = ix + Direction_XFactor(dir) * 0x500;
				ssy = iy + Direction_YFactor(dir) * 0x500;
				ssz = iz;
			}

			uint16 targetid = (target ? target->getObjId() : 0);
			ssp = new SuperSpriteProcess(BULLET_SPLASH_SHAPE, spriteframe,
										 ix, iy, iz, ssx, ssy, ssz, firetype,
										 damage, _objId, targetid, findtarget);
			Kernel::get_instance()->addProcess(ssp);
			spriteprocpid = ssp->getPid();
		}
		return spriteprocpid;
	}
}

uint16 Item::fireDistance(Item *other, Direction dir, int16 xoff, int16 yoff, int16 zoff) {
	if (!other)
		return 0;

	//
	// We pick what animation the actor would do to fire, then
	// pick the frame(s) where they fire in that anim.
	//
	// Then, check if the target can be hit using the attackx/attacky/attackz offsets.
	// The offsets are checked in priority order:
	// * First fire frame in anim
	// * Second fire frame
	// * if there are no fire frames, use the parameter offsets
	//
	int16 xoff2 = 0;
	int16 yoff2 = 0;
	int16 zoff2 = 0;
	bool other_offsets = false;
	Actor *a = dynamic_cast<Actor *>(this);
	if (a) {
		Animation::Sequence anim;
		bool kneeling = a->hasActorFlags(Actor::ACT_KNEELING);
		bool smallwpn = true;
		MainActor *ma = dynamic_cast<MainActor *>(this);
		Item *wpn = getItem(a->getActiveWeapon());
		if (wpn && wpn->getShapeInfo()->_weaponInfo) {
			smallwpn = wpn->getShapeInfo()->_weaponInfo->_small;
		}

		if (kneeling) {
			if (smallwpn)
				anim = Animation::kneelAndFireSmallWeapon;
			else
				anim = Animation::kneelAndFireLargeWeapon;
		} else {
			// TODO: fire2 seems to be different in Regret, check me.
			if (ma || smallwpn)
				anim = Animation::attack;
			else
				anim = Animation::fire2;
		}

		bool first_offsets = false;
		const AnimAction *action = GameData::get_instance()->getMainShapes()->getAnim(_shape, static_cast<int32>(anim));
		unsigned int nframes = action ? action->getSize() : 0;
		for (unsigned int i = 0; i < nframes; i++) {
			const AnimFrame &frame = action->getFrame(dir, i);
			if (frame.is_cruattack()) {
				if (!first_offsets) {
					xoff = frame.cru_attackx();
					yoff = frame.cru_attacky();
					zoff = frame.cru_attackz();
					first_offsets = true;
				} else {
					xoff2 = frame.cru_attackx();
					yoff2 = frame.cru_attacky();
					zoff2 = frame.cru_attackz();
					other_offsets = true;
					break;
				}
			}
		}
	}

	int32 x, y, z;
	getLocation(x, y, z);

	int32 ox, oy, oz;
	other->getLocation(ox, oy, oz);

	int32 dist = 0;

	CurrentMap *cm = World::get_instance()->getCurrentMap();
	if (!cm)
		return 0;

	for (int i = 0; i < (other_offsets ? 2 : 1) && dist == 0; i++) {
		int32 cx = x + (i == 0 ? xoff : xoff2);
		int32 cy = y + (i == 0 ? yoff : yoff2);
		int32 cz = z + (i == 0 ? zoff : zoff2);
		const Item *blocker = nullptr;
		bool valid = cm->isValidPosition(cx, cy, cz, BULLET_SPLASH_SHAPE,
										 getObjId(), nullptr, nullptr, &blocker);
		if (!valid) {
			if (blocker->getObjId() == other->getObjId())
				dist = MAX(abs(_x - ox), abs(_y - oy));
		} else {
			int32 ocx, ocy, ocz;
			other->getCentre(ocx, ocy, ocz);
			ocz = other->getTargetZRelativeToAttackerZ(getZ());
			const int32 start[3] = {cx, cy, cz};
			const int32 end[3] = {ocx, ocy, ocz};
			const int32 dims[3] = {2, 2, 2};

			Std::list<CurrentMap::SweepItem> collisions;
			Std::list<CurrentMap::SweepItem>::iterator it;
			cm->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
						   _objId, false, &collisions);
			for (it = collisions.begin(); it != collisions.end(); it++) {
				if (it->_item == getObjId())
					continue;
				if (it->_item != other->getObjId())
					break;
				int32 out[3];
				it->GetInterpolatedCoords(out, start, end);
				dist = MAX(abs(_x - out[0]), abs(_y - out[1]));
				break;
			}
		}
	}
	return dist / 32;
}

int32 Item::getTargetZRelativeToAttackerZ(int32 otherz) {
	int32 tsx, tsy, tsz;
	getFootpadData(tsx, tsy, tsz);

	int32 tz = getZ() + tsz * 8;

	if (tsz < 3) {
		if (tsz)
			tz -= 8;
	} else {
		int32 targetz = tz;
		tz -= 16;
		if (otherz - targetz < -0x2f) {
			tz += 8;
		} else if (otherz - targetz > 0x2f) {
			if (tsz == 6) {
				tz -= 16;
			} else if (tsz >= 7) {
				tz -= 24;
			}
		}
	}
	return tz;
}


unsigned int Item::countNearby(uint32 shape, uint16 range) {
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	UCList itemlist(2);
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(shape));
	currentmap->areaSearch(&itemlist, script, sizeof(script),
	                       this, range, false);
	return itemlist.getSize();
}


uint32 Item::callUsecodeEvent(uint32 event, const uint8 *args, int argsize) {
	uint32  class_id = _shape;

	// Non-monster NPCs use _objId/_npcNum + 1024 (2048 in crusader)
	// Note: in the original, a non-monster NPC is specified with
	// the FAST_ONLY flag. However, this causes some summoned monster which
	// do not receive the FAST_ONLY flag to behave strangely. (Confirmed that
	// happens in the original as well.) -wjp 20050128
	if (_objId < 256 && (_extendedFlags & EXT_PERMANENT_NPC)) {
		if (GAME_IS_U8)
			class_id = _objId + 1024;
		else
			class_id = _objId + 2048;
	}

	// CHECKME: to make Pentagram behave as much like the original as possible,
	// don't call any usecode if the original would call the wrong class
	if (GAME_IS_U8 && _objId < 256 && !(_extendedFlags & EXT_PERMANENT_NPC) &&
	        !(_flags & FLG_FAST_ONLY))
		return 0;

	// UnkEggs have class quality + 0x47F in U8, +0x900 in Crusader
	if (getFamily() == ShapeInfo::SF_UNKEGG)
		class_id = _quality + (GAME_IS_U8 ? 0x47F : 0x900);

	Usecode *u = GameData::get_instance()->getMainUsecode();
	uint32 offset = u->get_class_event(class_id, event);
	if (!offset) return 0; // event not found

	debug(10, "Item: %d (shape %d) calling usecode event %d @ %04X:%04X",
			_objId, _shape, event, class_id, offset);

	return callUsecode(static_cast<uint16>(class_id),
	                   static_cast<uint16>(offset),
	                   args, argsize);
}

uint32 Item::callUsecodeEvent_look() {                          // event 0
	return callUsecodeEvent(0); // CONSTANT
}

uint32 Item::callUsecodeEvent_use() {                           // event 1
	return callUsecodeEvent(1); // CONSTANT
}

uint32 Item::callUsecodeEvent_anim() {                          // event 2
	return callUsecodeEvent(2); // CONSTANT
}

uint32 Item::callUsecodeEvent_cachein() {                       // event 4
	return callUsecodeEvent(4); // CONSTANT
}

uint32 Item::callUsecodeEvent_hit(uint16 hitter, int16 hitforce) { // event 5
	DynamicUCStack  arg_stack(4);
	arg_stack.push2(hitforce);
	arg_stack.push2(hitter);
	return callUsecodeEvent(5, arg_stack.access(), 4);  // CONSTANT 5
}

uint32 Item::callUsecodeEvent_gotHit(uint16 hitter, int16 hitforce) { // event 6
	DynamicUCStack  arg_stack(4);
	arg_stack.push2(hitforce);
	arg_stack.push2(hitter);
	return callUsecodeEvent(6, arg_stack.access(), 4);  // CONSTANT 6
}

uint32 Item::callUsecodeEvent_hatch() {                         // event 7
	return callUsecodeEvent(7);     // CONSTANT
}

uint32 Item::callUsecodeEvent_schedule(uint32 time) {           // event 8
	DynamicUCStack  arg_stack(4);
	arg_stack.push4(time);
	return callUsecodeEvent(8, arg_stack.access(), 4);  // CONSTANT 8
}

uint32 Item::callUsecodeEvent_release() {                       // event 9
	return callUsecodeEvent(9);     // CONSTANT
}

uint32 Item::callUsecodeEvent_equip() {                         // event A
	return callUsecodeEvent(0xA); // CONSTANT
}

uint32 Item::callUsecodeEvent_equipWithParam(ObjId param) {     // event A
	DynamicUCStack  arg_stack(2);
	arg_stack.push2(param);
	return callUsecodeEvent(0xA, arg_stack.access(), 2);
}

uint32 Item::callUsecodeEvent_unequip() {                       // event B
	return callUsecodeEvent(0xB); // CONSTANT
}

uint32 Item::callUsecodeEvent_unequipWithParam(ObjId param) {   // event B
	DynamicUCStack  arg_stack(2);
	arg_stack.push2(param);
	return callUsecodeEvent(0xB, arg_stack.access(), 2);
}

uint32 Item::callUsecodeEvent_combine() {                       // event C
	return callUsecodeEvent(0xC);   // CONSTANT
}

uint32 Item::callUsecodeEvent_calledFromAnim() {                // event E
	return callUsecodeEvent(0xE);   // CONSTANT
}

uint32 Item::callUsecodeEvent_enterFastArea() {                 // event F
	return callUsecodeEvent(0xF);   // CONSTANT
}

uint32 Item::callUsecodeEvent_leaveFastArea() {                 // event 10
	return callUsecodeEvent(0x10);  // CONSTANT
}

uint32 Item::callUsecodeEvent_cast(uint16 unk) {                // event 11
	DynamicUCStack  arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x11, arg_stack.access(), 2); // CONSTANT 0x11
}

uint32 Item::callUsecodeEvent_justMoved() {                     // event 12
	return callUsecodeEvent(0x12);  // CONSTANT
}

uint32 Item::callUsecodeEvent_AvatarStoleSomething(uint16 unk) { // event 13
	DynamicUCStack  arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x13, arg_stack.access(), 2); // CONSTANT 0x13
}

uint32 Item::callUsecodeEvent_guardianBark(int16 unk) {         // event 15
	DynamicUCStack  arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x15, arg_stack.access(), 2); // CONSTANT 0x15
}

uint32 Item::callUsecodeEvent_unhatch() {                     // event 15
	return callUsecodeEvent(0x15);  // CONSTANT
}

uint32 Item::use() {
	Actor *actor = dynamic_cast<Actor *>(this);
	if (actor) {
		if (actor->isDead()) {
			if (GAME_IS_U8) {
				// dead actor, so open/close the dead-body-gump
				if (hasFlags(FLG_GUMP_OPEN)) {
					closeGump();
				} else {
					openGump(12); // CONSTANT!!
				}
			}
			return 0;
		}
	}

	return callUsecodeEvent_use();
}

void Item::destroy(bool delnow) {
	if (_flags & FLG_ETHEREAL) {
		// Remove us from the ether
		World::get_instance()->etherealRemove(_objId);
	} else if (_parent) {
		// we're in a container, so remove self from _parent
		//!! need to make sure this works for equipped items too...
		Container *p = getParentAsContainer();
		if (p) p->removeItem(this);
	} else if (_extendedFlags & EXT_INCURMAP) {
		// remove self from CurrentMap
		World::get_instance()->getCurrentMap()->removeItemFromList(this, _x, _y);
	}

	if (GAME_IS_CRUSADER) {
		// Ensure sounds for this object are stopped
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio)
			audio->stopSFX(-1, _objId);
		if (_shape == SNAP_EGG_SHAPE) {
			SnapProcess *snap = SnapProcess::get_instance();
			if (snap)
				snap->removeEgg(this);
		}
	}

	if (_extendedFlags & Item::EXT_CAMERA)
		CameraProcess::SetCameraProcess(0);

	// Do we want to delete now or not?
	if (!delnow) {
		Process *dap = new DestroyItemProcess(this);
		Kernel::get_instance()->addProcess(dap);
		return;
	}

	clearObjId();
	delete this; // delete self.
}

//
// Item::setupLerp()
//
// Desc: Setup the lerped info for this frame
//
void Item::setupLerp(int32 gametick) {
	// Don't need to set us up
	if (_lastSetup && gametick == _lastSetup)
		return;

	// Are we lerping or are we not? Default we lerp.
	bool no_lerp = false;

	// No lerping this _frame if Shape Changed, or No lerp is set,
	// or no last setup, or last setup more than 1 tick ago
	if ((_lastSetup == 0) || (_lNext._shape != _shape) ||
	        (_extendedFlags & EXT_LERP_NOPREV) ||
	        (gametick - _lastSetup) > 1 || _flags & FLG_CONTAINED)
		no_lerp = true;

	// Update the time we were just setup
	_lastSetup = gametick;

	// Clear the flag
	_extendedFlags &= ~EXT_LERP_NOPREV;

	// Animate it, if needed
	const ShapeInfo *info = getShapeInfo();
	if (info->_animType &&
			((gametick % info->_animSpeed) == (_objId % info->_animSpeed)))
		animateItem();

	// Setup the prev values for lerping
	if (!no_lerp) _lPrev = _lNext;

	// Setup next
	if (_flags & FLG_CONTAINED) {
		_lNext._x = _ix = _y & 0xFF;
		_lNext._y = _iy = (_y >> 8) & 0xFF;
		_lNext._z = _iz = 0;
	} else {
		_lNext._x = _ix = _x;
		_lNext._y = _iy = _y;
		_lNext._z = _iz = _z;
	}
	_lNext._shape = _shape;
	_lNext._frame = _frame;

	// Setup prev values for not lerping
	if (no_lerp) _lPrev = _lNext;
}

// Animate the item
void Item::animateItem() {
	const ShapeInfo *info = getShapeInfo();

	if (!info->_animType)
		return;

	int anim_data = info->_animData;
	int speed = info->_animSpeed;

	if ((static_cast<int>(_lastSetup) % speed * 2) != (_objId % speed * 2) && info->_animType != 1)
		return;

	const Shape *shp = getShapeObject();

	switch (info->_animType) {
	case 2:
		// 50 % chance
		if (getRandom() & 1) break;
		// Intentional fall-through

	case 1:
	case 3:
		// 50 % chance
		if (anim_data == 1 && (getRandom() & 1)) break;
		_frame ++;
		if (anim_data < 2) {
			if (shp && _frame == shp->frameCount()) _frame = 0;
		} else {
			// Data represents frame count for the loop
			unsigned int num = (_frame - 1) / anim_data;
			if (_frame == ((num + 1)*anim_data)) _frame = num * anim_data;
		}
		break;

	case 4:
		if (!(getRandom() % anim_data)) break;
		_frame ++;
		if (shp && _frame == shp->frameCount()) _frame = 0;
		break;


	case 5:
		callUsecodeEvent_anim();
		break;

	case 6:
		if (anim_data < 2) {
			if (_frame == 0) break;
			_frame ++;
			if (shp && _frame == shp->frameCount()) _frame = 1;
		} else {
			if (!(_frame % anim_data)) break;
			_frame ++;
			unsigned int num = (_frame - 1) / anim_data;
			if (_frame == ((num + 1)*anim_data)) _frame = num * anim_data + 1;
		}
		break;

	default:
		pout << "type " << info->_animType << " data " << anim_data << Std::endl;
		break;
	}
}


// Called when an item has entered the fast area
void Item::enterFastArea() {
	//!! HACK to get rid of endless SFX loops
	if (_shape == 0x2c8 && GAME_IS_U8)
		return;

	// Call usecode
	if (!(_flags & FLG_FASTAREA)) {
		Actor *actor = dynamic_cast<Actor *>(this);
		// Crusader special-cases a few shapes even when they are dead.
		bool call_even_if_dead = (_shape == 0x576 || _shape == 0x596 ||
								  _shape == 0x59c || _shape == 0x58f) && GAME_IS_CRUSADER;
		if (actor && actor->isDead() && !call_even_if_dead) {
			// dead actor, don't call the usecode
		} else {
			if (actor && GAME_IS_CRUSADER) {
				actor->clearLastActivityNo();
				actor->clearInCombat();
			}
			callUsecodeEvent_enterFastArea();
		}
	}

	if (!hasFlags(FLG_BROKEN) && GAME_IS_CRUSADER) {
		const ShapeInfo *si = getShapeInfo();
		if ((si->_flags & ShapeInfo::SI_CRU_TARGETABLE) || (si->_flags & ShapeInfo::SI_OCCL)) {
			World::get_instance()->getCurrentMap()->addTargetItem(this);
		}
		if (_shape == SNAP_EGG_SHAPE) {
			SnapProcess *snap = SnapProcess::get_instance();
			if (snap)
				snap->addEgg(this);
		}
	}

	// We're fast!
	_flags |= FLG_FASTAREA;
}

// Called when an item is leaving the fast area
void Item::leaveFastArea() {
	// Call usecode
	if ((!(_flags & FLG_FAST_ONLY) || getShapeInfo()->is_noisy()) &&
	        (_flags & FLG_FASTAREA))
		callUsecodeEvent_leaveFastArea();

	// If we have a gump open, close it (unless we're in a container)
	if (!_parent && (_flags & FLG_GUMP_OPEN)) {
		Gump *g = Ultima8Engine::get_instance()->getGump(_gump);
		if (g) g->Close();
	}

	if (_objId == 1) {
		debug(6, "avatar leaving fast area");
	}

	// Unset the flag
	_flags &= ~FLG_FASTAREA;

	if (!hasFlags(FLG_BROKEN) && GAME_IS_CRUSADER) {
		World::get_instance()->getCurrentMap()->removeTargetItem(this);
		if (_shape == SNAP_EGG_SHAPE) {
			SnapProcess *snap = SnapProcess::get_instance();
			if (snap)
				snap->removeEgg(this);
		}
	}

	// CHECKME: what do we need to do exactly?
	// currently,  destroy object

	// Kill us if we are fast only, unless we're in a container
	if ((_flags & FLG_FAST_ONLY) && !getParent()) {
		// destroy contents if container
		Container *c = dynamic_cast<Container *>(this);
		if (c) c->destroyContents();

		destroy();
		// NB: destroy() creates an DestroyItemProcess to actually
		// delete the item in this case.
	}
	// If we have a gravity process, move us to the ground
	else if (_gravityPid) {
		Process *p = Kernel::get_instance()->getProcess(_gravityPid);
		if (p) {
			p->terminateDeferred();
			_gravityPid = 0;
			collideMove(_x, _y, 0, true, false);
		}
	}
}

uint16 Item::openGump(uint32 gumpshape) {
	if (_flags & FLG_GUMP_OPEN) return 0;
	assert(_gump == 0);
	const Shape *shapeP = GameData::get_instance()->getGumps()->getShape(gumpshape);

	ContainerGump *cgump;

	if (getObjId() != 1) { //!! constant
		cgump = new ContainerGump(shapeP, 0, _objId, Gump::FLAG_ITEM_DEPENDENT |
		                          Gump::FLAG_DRAGGABLE);
	} else {
		cgump = new PaperdollGump(shapeP, 0, _objId, Gump::FLAG_ITEM_DEPENDENT |
		                          Gump::FLAG_DRAGGABLE);
	}
	//!!TODO: clean up the way this is set
	//!! having the itemarea associated with the shapeP through the
	//!! GumpShapeFlex maybe
	cgump->setItemArea(GameData::get_instance()->
	                   getGumps()->getGumpItemArea(gumpshape));
	cgump->InitGump(0);
	_flags |= FLG_GUMP_OPEN;
	_gump = cgump->getObjId();

	return _gump;
}

void Item::closeGump() {
	if (!(_flags & FLG_GUMP_OPEN)) return;

	Gump *g = Ultima8Engine::get_instance()->getGump(_gump);
	if (g)
		g->Close();

	// can we already clear gump here, or do we need to wait for the gump
	// to really close??
	clearGump();
}


void Item::clearGump() {
	_gump = 0;
	_flags &= ~FLG_GUMP_OPEN;
}

int32 Item::ascend(int delta) {
//	pout << "Ascend: _objId=" << getObjId() << ", delta=" << delta << Std::endl;

	if (delta == 0) return 0x4000;

	// * gather all items on top of this item (recursively?)
	// * etherealize all those items to get them out of the way
	// * move self up/down
	// * attempt to rematerialize the items up/down
	// (going through etherealness to avoid having to sort the supported items)

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	World *world = World::get_instance();
	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
	                                      this, true, false, false);
	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *item = getItem(uclist.getuint16(i));
		if (!item) continue;
		if (item->getShapeInfo()->is_fixed()) continue;

		item->moveToEtherealVoid();
	}

	// move self
	int32 xv, yv, zv;
	getLocation(xv, yv, zv);
	int dist = collideMove(xv, yv, zv + delta, false, false);
	delta = (delta * dist) / 0x4000;

//	pout << "Ascend: dist=" << dist << Std::endl;

	// move other items
	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *item = getItem(uclist.getuint16(i));
		if (!item) continue;
		if (item->getShapeInfo()->is_fixed()) continue;

		item->getLocation(_ix, _iy, _iz);

		if (item->canExistAt(_ix, _iy, _iz + delta)) {
			item->move(_ix, _iy, _iz + delta); // automatically un-etherealizes item
		} else {
			// uh oh...
			// CHECKME: what do we do here?
			item->move(_ix, _iy, _iz);
			if (delta < 0) item->fall();
		}
	}

	return dist;
}

GravityProcess *Item::ensureGravityProcess() {
	GravityProcess *p;
	if (_gravityPid) {
		p = dynamic_cast<GravityProcess *>(
		        Kernel::get_instance()->getProcess(_gravityPid));
	} else {
		p = new GravityProcess(this, 0);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}
	assert(p);
	return p;
}

void Item::fall() {
	const ShapeInfo *info = getShapeInfo();
	if (_flags & FLG_HANGING || info->is_fixed() ||
		(info->_weight == 0 && GAME_IS_CRUSADER)) {
		// can't fall
		return;
	}

	int gravity = GAME_IS_CRUSADER ? 2 : 4; //!! constants

	GravityProcess *p = ensureGravityProcess();
	p->setGravity(gravity);
}

void Item::grab() {
	// CHECKME: is the fall/release timing correct?

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	World *world = World::get_instance();
	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
	                                      this, true, false, true);

	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *item = getItem(uclist.getuint16(i));
		if (!item) continue;
		item->fall();
	}

	uclist.free();

	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
	                                      this, false, true, false);

	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *item = getItem(uclist.getuint16(i));
		if (!item) continue;
		item->callUsecodeEvent_release();
	}

}


void Item::hurl(int xs, int ys, int zs, int grav) {
	GravityProcess *p = ensureGravityProcess();
	p->setGravity(grav);
	p->move(xs, ys, zs);
}


void Item::explode(int explosion_type, bool destroy_item, bool cause_damage) {
	Process *p;

	if (GAME_IS_CRUSADER) {
		setFlag(FLG_BROKEN);
		// TODO: original game puts them at cx/cy/cz, but that looks wrong..
		int32 cx, cy, cz;
		getCentre(cx, cy, cz);
		static const int expshapes[] = {0x31C, 0x31F, 0x326, 0x320, 0x321, 0x324, 0x323, 0x325};
		int rnd = getRandom();
		int spriteno;
		// NOTE: The game does some weird 32-bit stuff to decide what
		// shapenum to use.  Just simplified to a random.
		switch (explosion_type) {
		case 0:
			spriteno = expshapes[rnd % 2];
			break;
		case 1:
			spriteno = expshapes[2 + rnd % 3];
			break;
		case 2:
		default:
			spriteno = expshapes[5 + rnd % 3];
			break;
		}
		p = new SpriteProcess(spriteno, 0, 39, 1, 1, //!! constants
	                               _x, _y, cz);
	} else {
		p = new SpriteProcess(578, 20, 34, 1, 1, //!! constants
	                               _x, _y, _z);
	}
	Kernel::get_instance()->addProcess(p);

	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc) {
		int sfx;
		if (GAME_IS_CRUSADER) {
			sfx = (getRandom() % 2) ? 28 : 108;
			audioproc->stopSFX(-1, _objId);
		} else {
			sfx = (getRandom() % 2) ? 31 : 158;
		}
		audioproc->playSFX(sfx, 0x60, 0, 0);
	}

	int32 xv, yv, zv;
	getLocation(xv, yv, zv);

	if (destroy_item) {
		destroy(); // delete self
		// WARNING: we are deleted at this point
	}

	if (!cause_damage)
		return;

	if (GAME_IS_U8) {
		UCList itemlist(2);
		LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
		CurrentMap *currentmap = World::get_instance()->getCurrentMap();
		currentmap->areaSearch(&itemlist, script, sizeof(script), 0,
							   160, false, xv, yv); //! CHECKME: 128?

		for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
			Item *item = getItem(itemlist.getuint16(i));
			if (!item) continue;
			if (getRange(*item, true) > 160) continue; // check vertical distance

			item->getLocation(xv, yv, zv);
			Direction dir = Direction_GetWorldDir(xv - xv, yv - yv, dirmode_8dirs); //!! CHECKME
			item->receiveHit(0, dir, 6 + (getRandom() % 6),
							 WeaponInfo::DMG_BLUNT | WeaponInfo::DMG_FIRE);
		}
	} else {
		Point3 pt;
		getLocation(pt);
		// Note: same FireType number used in both Remorse and Regret
		const FireType *firetypedat = GameData::get_instance()->getFireType(4);
		if (firetypedat) {
			int damage = firetypedat->getRandomDamage();
			firetypedat->applySplashDamageAround(pt, damage, this, this);
		} else {
			warning("couldn't explode properly - no firetype 4 data");
		}
	}
}

uint16 Item::getDamageType() const {
	const ShapeInfo *si = getShapeInfo();
	if (si->_weaponInfo) {
		return si->_weaponInfo->_damageType;
	}

	return 0;
}

void Item::receiveHit(uint16 other, Direction dir, int damage, uint16 type) {
	if (GAME_IS_U8)
		receiveHitU8(other, dir, damage, type);
	else
		receiveHitCru(other, dir, damage, type);
}

void Item::receiveHitU8(uint16 other, Direction dir, int damage, uint16 type) {
	// first, check if the item has a 'gotHit' usecode event
	if (callUsecodeEvent_gotHit(other, 0)) //!! TODO: what should the 0 be??
		return;

	// explosive?
	if (getShapeInfo()->is_u8_explode()) {
		explode(0, true); // warning: deletes this
		return;
	}

	// breakable?
	if (getFamily() == ShapeInfo::SF_BREAKABLE) {
		// CHECKME: anything else?
		destroy();
		return;
	}

	if (getShapeInfo()->is_fixed() || getShapeInfo()->_weight == 0) {
		// can't move
		return;
	}

	// nothing special, so just hurl the item
	// TODO: hurl item in direction, with speed depending on damage
	hurl(-16 * Direction_XFactor(dir), -16 * Direction_YFactor(dir), 16, 4); //!! constants
}


void Item::receiveHitCru(uint16 other, Direction dir, int damage, uint16 type) {
	damage = scaleReceivedDamageCru(damage, type);
	const ShapeInfo *shapeInfo = getShapeInfo();
	if (!shapeInfo)
		return;
	const DamageInfo *damageInfo = shapeInfo->_damageInfo;

	// TODO: work out how this flag is decided.
	uint8 shouldCallUsecode = 1;

	if (shouldCallUsecode)
		callUsecodeEvent_gotHit(0x4000, (type << 8) | (damage & 0xff));

	if (damageInfo) {
		bool wasbroken = damageInfo->applyToItem(this, damage);
		if (wasbroken) {
			Kernel::get_instance()->killProcesses(_objId, 0xc, true);
		}
	}

	if (shapeInfo->is_fixed() || shapeInfo->_weight == 0) {
		// can't move
		return;
	}

	int xhurl = 10 + getRandom() % 15;
	int yhurl = 10 + getRandom() % 15;

	// nothing special, so just hurl the item
	hurl(-xhurl * Direction_XFactor(dir), -yhurl * Direction_YFactor(dir), 16, 4); //!! constants
}


bool Item::canDrag() {
	const ShapeInfo *si = getShapeInfo();
	if (si->is_fixed()) return false;
	if (si->_weight == 0) return false;

	Actor *actor = dynamic_cast<Actor *>(this);
	if (actor) {
		// living actors can't be moved
		if (!actor->isDead()) return false;
	}

	// CHECKME: might need more checks here (weight?)

	return true;
}

int Item::getThrowRange() {
	if (!canDrag()) return 0;

	Actor *avatar = getMainActor();

	int range = 64 - getTotalWeight() + avatar->getStr();
	if (range < 1) range = 1;
	range = (range * range) / 2;

	return range;
}

static bool checkLineOfSightCollisions(
    const Std::list<CurrentMap::SweepItem> &collisions,
    bool usingAlternatePos, ObjId item, ObjId other) {
	Std::list<CurrentMap::SweepItem>::const_iterator it;
	int32 other_hit_time = 0x4000;
	int32 blocked_time = 0x4000;
	for (it = collisions.begin(); it != collisions.end(); it++) {
		// ignore self and other
		if (it->_item == item) continue;
		if (it->_item == other && !usingAlternatePos) {
			other_hit_time = it->_hitTime;
			continue;
		}

		// only touching?
		if (it->_touching) continue;

		// hit something
		if (it->_blocking && it->_hitTime < blocked_time) {
			blocked_time = it->_hitTime;
		}
	}

	// 'other' must be the first item that is hit.
	return (blocked_time >= other_hit_time);
}

bool Item::canReach(Item *other, int range,
                    int32 otherX, int32 otherY, int32 otherZ) {
	// get location and dimensions of self and other (or their root containers)
	int32 thisX, thisY, thisZ;
	int32 thisXd, thisYd, thisZd;
	int32 otherXd, otherYd, otherZd;
	int32 thisXmin, thisYmin;
	int32 otherXmin, otherYmin;

	bool usingAlternatePos = (otherX != 0);

	getLocationAbsolute(thisX, thisY, thisZ);
	other = other->getTopItem();
	if (otherX == 0)
		other->getLocationAbsolute(otherX, otherY, otherZ);

	getFootpadWorld(thisXd, thisYd, thisZd);
	other->getFootpadWorld(otherXd, otherYd, otherZd);

	thisXmin = thisX - thisXd;
	thisYmin = thisY - thisYd;

	otherXmin = otherX - otherXd;
	otherYmin = otherY - otherYd;

	// if items are further away than range in any direction, return false
	if (thisXmin - otherX > range) return false;
	if (otherXmin - thisX > range) return false;
	if (thisYmin - otherY > range) return false;
	if (otherYmin - thisY > range) return false;


	// if not, do line of sight between origins of items
	int32 start[3];
	int32 end[3];
	int32 dims[3] = { 2, 2, 2 };

	start[0] = thisX;
	start[1] = thisY;
	start[2] = thisZ;
	end[0] = otherX;
	end[1] = otherY;
	end[2] = otherZ;
	if (otherZ > thisZ && otherZ < thisZ + thisZd)
		start[2] = end[2]; // bottom of other between bottom and top of this

	Std::list<CurrentMap::SweepItem> collisions;
	Std::list<CurrentMap::SweepItem>::iterator it;
	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();
	map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
	               _objId, false, &collisions);
	if (checkLineOfSightCollisions(collisions, usingAlternatePos,
	                               getObjId(), other->getObjId()))
		return true;

	// if that fails, try line of sight between centers
	start[0] = thisX - thisXd / 2; // xy center of this
	start[1] = thisY - thisYd / 2;
	start[2] = thisZ;
	if (thisZd > 16)
		start[2] += thisZd - 8; // eye height

	end[0] = otherX - otherXd / 2; // xyz center of other
	end[1] = otherY - otherYd / 2;
	end[2] = otherZ + otherZd / 2;

	collisions.clear();
	map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
	               _objId, false, &collisions);
	if (checkLineOfSightCollisions(collisions, usingAlternatePos,
	                               getObjId(), other->getObjId()))
		return true;

	// if that fails, try line of sight between eye level and top of 2nd
	end[2] = otherZ + otherZd;

	collisions.clear();
	map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
	               _objId, false, &collisions);
	return checkLineOfSightCollisions(collisions, usingAlternatePos,
	                                  getObjId(), other->getObjId());
}


/**
 * A helper function to check if both frames are in the
 * same range (inclusive) for the merge check below */
static inline bool bothInRange(uint32 x, uint32 y, uint32 lo, uint32 hi) {
	return (x >= lo && x <= hi && y >= lo && y <= hi);
}

bool Item::canMergeWith(Item *other) {
	// can't merge with self
	if (other->getObjId() == getObjId()) return false;

	if (other->getShape() != getShape()) return false;

	int family = getFamily();
	if (family == ShapeInfo::SF_QUANTITY) return true;

	if (family != ShapeInfo::SF_REAGENT) return false;

	uint32 frame1 = getFrame();
	uint32 frame2 = other->getFrame();
	if (frame1 == frame2) return true;

	// special cases:
	// necromancy reagents (shape 395)
	// 		blood: frame 0-5
	// 		bone: frame 6-7
	// 		wood: frame 8
	// 		dirt: frame 9
	// 		ex.hood: frame 10-12
	// 		blackmoor: frame 14-15
	// 		dead man's elbow: frame 16-20
	// sorcery reagents (shape 398).
	// Disabled because the usecode doesn't support saying how many there are.
	//		volcanic ash: frame 0-1
	//		pumice: frame 2-5
	//		obsidian: 6-9
	//		iron: 10-13
	//		brimstone: 14-17
	// 		daemon bones: 18-20
	// 3. ether reagents (shape 399) (also not supported in usecode)
	//
	if (GAME_IS_U8) {
		if (getShape() == 395) {
			if (bothInRange(frame1, frame2, 0, 5))
				return true;
			if (bothInRange(frame1, frame2, 6, 7))
				return true;
			if (bothInRange(frame1, frame2, 10, 12))
				return true;
			if (bothInRange(frame1, frame2, 14, 15))
				return true;
			if (bothInRange(frame1, frame2, 16, 20))
				return true;
		}
		/*if (getShape() == 398) {
			if (bothInRange(frame1, frame2, 0, 1))
				return true;
			if (bothInRange(frame1, frame2, 2, 5))
				return true;
			if (bothInRange(frame1, frame2, 6, 9))
				return true;
			if (bothInRange(frame1, frame2, 10, 13))
				return true;
			if (bothInRange(frame1, frame2, 14, 17))
				return true;
			if (bothInRange(frame1, frame2, 18, 20))
				return true;
		}*/
	}
	return false;
}

bool Item::isRobotCru() const {
	uint32 shape = getShape();
	return (shape == 0x4c8 || shape == 0x338 || shape == 0x45d ||
			shape == 0x2cb || shape == 0x4e6 || shape == 899 ||
			shape == 0x385);
}

int Item::scaleReceivedDamageCru(int damage, uint16 type) const {
	uint8 difficulty = World::get_instance()->getGameDifficulty();
	const Actor *actor = dynamic_cast<const Actor *>(this);
	//
	// For difficulty 1 and 2, we scale damage to others *up* and damage
	// to avatar *down*.
	//
	if (!actor || (this != getMainActor() && this != getControlledActor())) {
		if (difficulty == 1) {
			damage *= 5;
		} else if (difficulty == 2) {
			damage *= 3;
		}
	} else {
		if (difficulty == 1) {
			damage /= 5;
		} else if (difficulty == 2) {
			damage /= 3;
		}
	}

	if (isRobotCru() && (type == 1 || type == 2 || type == 0xb || type == 0xd)) {
		damage /= 3;
	}

	damage = CLIP(damage, 1, 0xfa);
	return damage;
}


void Item::saveData(Common::WriteStream *ws) {
	Object::saveData(ws);
	ws->writeUint16LE(static_cast<uint16>(_extendedFlags));
	ws->writeUint16LE(_flags);
	ws->writeUint16LE(static_cast<uint16>(_shape));
	ws->writeUint16LE(static_cast<uint16>(_frame));
	ws->writeUint16LE(static_cast<uint16>(_x));
	ws->writeUint16LE(static_cast<uint16>(_y));
	ws->writeUint16LE(static_cast<uint16>(_z));
	ws->writeUint16LE(_quality);
	ws->writeUint16LE(_npcNum);
	ws->writeUint16LE(_mapNum);
	if (getObjId() != 0xFFFF) {
		// these only make sense in currently loaded items
		ws->writeUint16LE(_gump);
		ws->writeUint16LE(_gravityPid);
	}
	if ((_flags & FLG_ETHEREAL) && (_flags & (FLG_CONTAINED | FLG_EQUIPPED)))
		ws->writeUint16LE(_parent);
}

bool Item::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Object::loadData(rs, version)) return false;

	_extendedFlags = rs->readUint16LE();
	_flags = rs->readUint16LE();
	_shape = rs->readUint16LE();
	_frame = rs->readUint16LE();
	_x = rs->readUint16LE();
	_y = rs->readUint16LE();
	_z = rs->readUint16LE();

	_quality = rs->readUint16LE();
	_npcNum = rs->readUint16LE();
	_mapNum = rs->readUint16LE();
	if (getObjId() != 0xFFFF) {
		_gump = rs->readUint16LE();
		_gravityPid = rs->readUint16LE();
	} else {
		_gump = _gravityPid = 0;
	}

	if ((_flags & FLG_ETHEREAL) && (_flags & (FLG_CONTAINED | FLG_EQUIPPED)))
		_parent = rs->readUint16LE();
	else
		_parent = 0;

	//!! hackish...
	if (_extendedFlags & EXT_INCURMAP) {
		World::get_instance()->getCurrentMap()->addItem(this);
	}

	return true;
}


uint32 Item::I_touch(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item);

	// Guess: this is used to make sure an item is painted in the original.
	// Our renderer is different, making this intrinsic unnecessary.

	return 0;
}

uint32 Item::I_getX(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);
	if (GAME_IS_CRUSADER)
		return x / 2;
	else
		return x;
}

uint32 Item::I_getY(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);
	if (GAME_IS_CRUSADER)
		return y / 2;
	else
		return y;
}

uint32 Item::I_getZ(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);
	return z;
}

uint32 Item::I_getCX(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	int mul = 16;
	if (GAME_IS_CRUSADER) {
		x /= 2;
		mul /= 2;
	}

	if (item->_flags & FLG_FLIPPED)
		return x - item->getShapeInfo()->_y * mul;
	else
		return x - item->getShapeInfo()->_x * mul;
}

uint32 Item::I_getCY(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	int mul = 16;
	if (GAME_IS_CRUSADER) {
		y /= 2;
		mul /= 2;
	}

	if (item->_flags & FLG_FLIPPED)
		return y - item->getShapeInfo()->_x * mul;
	else
		return y - item->getShapeInfo()->_y * mul;
}

uint32 Item::I_getCZ(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	return z + item->getShapeInfo()->_z * 4;
}

uint32 Item::I_getPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UC_PTR(ptr);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	if (GAME_IS_CRUSADER) {
		x /= 2;
		y /= 2;
	}

	WorldPoint point;
	point.setX(x);
	point.setY(y);
	point.setZ(z);

	UCMachine::get_instance()->assignPointer(ptr, point._buf, 5);

	return 0;
}

uint32 Item::I_getShape(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getShape();
}

uint32 Item::I_setShape(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	if (!item) return 0;

	item->setShape(shape);
	return 0;
}

uint32 Item::I_getFrame(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFrame();
}

uint32 Item::I_setFrame(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(frame);
	if (!item) return 0;

	item->setFrame(frame);
	return 0;
}

uint32 Item::I_getQuality(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUALITY)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getUnkEggType(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_UNKEGG) {
		if (GAME_IS_U8) {
			return item->getQuality();
		} else {
			return item->getQuality() & 0xFF;
		}
	} else {
		return 0;
	}
}

uint32 Item::I_getQuantity(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
	        item->getFamily() == ShapeInfo::SF_REAGENT)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	//! What do we do if item has no _parent?
	//! What do we do with equipped items?

	if (item->getParent())
		return item->getParent();
	else
		return 0;
}

uint32 Item::I_getRootContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	Container *_parent = item->getParentAsContainer();

	//! What do we do if item has no _parent?
	//! What do we do with equipped items?

	if (!_parent) return 0;

	while (_parent->getParentAsContainer()) {
		_parent = _parent->getParentAsContainer();
	}

	return _parent->getObjId();
}

uint32 Item::I_getQ(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getQuality();
}

uint32 Item::I_getQLo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getQuality() & 0xFF;
}

uint32 Item::I_getQHi(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return (item->getQuality() >> 8) & 0xFF;
}

uint32 Item::I_setQ(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	item->setQuality(q);
	return 0;
}

uint32 Item::I_setQLo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	uint16 iq = item->getQuality() & 0xFF00;

	item->setQuality(iq | (q & 0xFF));
	return 0;
}

uint32 Item::I_setQHi(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	uint16 iq = item->getQuality() & 0x00FF;

	item->setQuality(iq | ((q << 8) & 0xFF00));
	return 0;
}

uint32 Item::I_setQuality(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() != ShapeInfo::SF_GENERIC)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_setQuantity(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
	        item->getFamily() == ShapeInfo::SF_REAGENT)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_setQAndCombine(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	item->setQuality(q);
	item->callUsecodeEvent_combine();

	return 0;
}

uint32 Item::I_getFamily(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFamily();
}

uint32 Item::I_getTypeFlag(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(typeflag);
	if (!item) return 0;

	const ShapeInfo *info = item->getShapeInfo();

	if (GAME_IS_U8 && typeflag >= 64)
		perr << "Invalid TypeFlag greater than 63 requested (" << typeflag << ") by Usecode" << Std::endl;
	if (GAME_IS_CRUSADER && typeflag >= 72)
		perr << "Invalid TypeFlag greater than 72 requested (" << typeflag << ") by Usecode" << Std::endl;

	if (info->getTypeFlag(typeflag))
		return 1;
	else
		return 0;
}

uint32 Item::I_getStatus(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFlags();
}

uint32 Item::I_orStatus(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->setFlag(mask);
	return 0;
}

uint32 Item::I_andStatus(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->_flags &= mask;
	return 0;
}

uint32 Item::I_ascend(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(delta);
	if (!item) return 0;

	int dist = item->ascend(delta);

	if (dist == 0x4000)
		return 1;
	else
		return 0;
}

uint32 Item::I_getWeight(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getWeight();
}

uint32 Item::I_getWeightIncludingContents(const uint8 *args,
        unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getTotalWeight();
}

uint32 Item::I_bark(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	if (id_item == 666) item = getItem(1);
	if (!item) return 0;    // Hack!

	uint32 shapenum = item->getShape();
	if (id_item == 666) shapenum = 666; // Hack for guardian barks
	Gump *_gump = new BarkGump(item->getObjId(), str, shapenum);

	if (item->getObjId() < 256) { // CONSTANT!
		GumpNotifyProcess *notifyproc;
		notifyproc = new ActorBarkNotifyProcess(item->getObjId());
		Kernel::get_instance()->addProcess(notifyproc);
		_gump->SetNotifyProcess(notifyproc);
	}

	_gump->InitGump(0);

	return _gump->GetNotifyProcess()->getPid();
}

uint32 Item::I_look(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_look();
}

uint32 Item::I_use(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_use();
}

uint32 Item::I_gotHit(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(hitter);
	ARG_SINT16(force);
	if (!item) return 0;

	return item->callUsecodeEvent_gotHit(hitter, force);
}

uint32 Item::I_equip(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	// Note: The U8 version (no param) is never actually called in the usecode.
	assert(argsize > 4);

	ARG_UINT16(val);
	return item->callUsecodeEvent_equipWithParam(val);
}

uint32 Item::I_unequip(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	// Note: The U8 version (no param) is never actually called in the usecode.
	assert(argsize > 4);

	ARG_UINT16(val)
	return item->callUsecodeEvent_unequipWithParam(val);
}

uint32 Item::I_enterFastArea(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_enterFastArea();
}

uint32 Item::I_cast(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;
	ARG_UINT16(arg);

	return item->callUsecodeEvent_cast(arg);
}

uint32 Item::I_avatarStoleSomething(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	// Abort if npc && dead to match original game behavior
	Actor *actor = dynamic_cast<Actor *>(item);
	if (actor && actor->isDead())
		return 0;

	ARG_UINT16(arg);

	return item->callUsecodeEvent_AvatarStoleSomething(arg);
}

uint32 Item::I_ask(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // currently unused.
	ARG_LIST(answers);

	if (!answers) return 0;

	// Use AskGump
	Gump *_gump = new AskGump(1, answers);
	_gump->InitGump(0);
	return _gump->GetNotifyProcess()->getPid();
}

uint32 Item::I_legalCreateAtPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_WORLDPOINT(point);

	int32 x = point.getX();
	int32 y = point.getY();
	int32 z = point.getZ();

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	// check if item can exist
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, shape, 0, 0, 0);
	if (!valid)
		return 0;

	Item *newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0, true);
	if (!newitem) {
		perr << "I_legalCreateAtPoint failed to create item (" << shape
		     << "," << frame << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newitem->getObjId();
	newitem->move(x, y, z);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return 1;
}

uint32 Item::I_legalCreateAtCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	// check if item can exist
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, shape, 0, 0, 0);
	if (!valid)
		return 0;

	// if yes, create it
	Item *newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0, true);
	if (!newitem) {
		perr << "I_legalCreateAtCoords failed to create item (" << shape
		     << "," << frame << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newitem->getObjId();
	newitem->move(x, y, z);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return 1;
}

uint32 Item::I_legalCreateInCont(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_CONTAINER_FROM_ID(container);
	ARG_UINT16(unknown); // ?

	uint8 buf[2];
	buf[0] = 0;
	buf[1] = 0;
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	// Create an item and try to add it to the given container.
	// If it fits, return id; otherwise return 0.

	Item *newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0, true);
	if (!newitem) {
		perr << "I_legalCreateInCont failed to create item (" << shape
		     << "," << frame << ")." << Std::endl;
		return 0;
	}

	// also need to check weight, volume maybe??
	if (newitem->moveToContainer(container)) {
		uint16 objID = newitem->getObjId();

		buf[0] = static_cast<uint8>(objID);
		buf[1] = static_cast<uint8>(objID >> 8);
		UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

		return 1;
	} else {
		perr << "I_legalCreateInCont failed to add item to container ("
		     << container->getObjId() << ")" << Std::endl;
		// failed to add; clean up
		newitem->destroy();

		return 0;
	}
}

uint32 Item::I_destroy(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item || item->getObjId() == 1) return 0;

	item->destroy();

	return 0;
}

uint32 Item::I_getFootpadData(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UC_PTR(xptr);
	ARG_UC_PTR(yptr);
	ARG_UC_PTR(zptr);
	if (!item) return 0;

	// TODO: Data is packed differently in Crusader - check that this still works.

	uint8 buf[2];
	int32 x, y, z;
	item->getFootpadData(x, y, z);

	buf[0] = static_cast<uint8>(x);
	buf[1] = static_cast<uint8>(x >> 8);
	UCMachine::get_instance()->assignPointer(xptr, buf, 2);

	buf[0] = static_cast<uint8>(y);
	buf[1] = static_cast<uint8>(y >> 8);
	UCMachine::get_instance()->assignPointer(yptr, buf, 2);

	buf[0] = static_cast<uint8>(z);
	buf[1] = static_cast<uint8>(z >> 8);
	UCMachine::get_instance()->assignPointer(zptr, buf, 2);

	return 0;
}

uint32 Item::I_overlaps(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->overlaps(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_overlapsXY(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->overlapsxy(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_isOn(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->isOn(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_isCompletelyOn(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->isCompletelyOn(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_isCentreOn(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->isCentreOn(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_isInNpc(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item)
		return 0;

	const Container *container = item->getParentAsContainer();
	while (container) {
		const Actor *actor = dynamic_cast<const Actor *>(container);
		if (actor)
			return 1;
		container = container->getParentAsContainer();
	}
	return 0;
}

uint32 Item::I_getFamilyOfType(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(shape);

	return GameData::get_instance()->getMainShapes()->
	       getShapeInfo(shape)->_family;
}

uint32 Item::I_push(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item)
		return 0;

	#if 0
		perr << "Pushing item to ethereal void: " << item->getShape() << "," << item->getFrame() << Std::endl;
	#endif

	item->moveToEtherealVoid();

	return 0;
}

uint32 Item::I_create(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr (????)
	ARG_UINT16(shape);
	ARG_UINT16(frame);

	Item *newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0, true);
	if (!newitem) {
		perr << "I_create failed to create item (" << shape
		     << "," << frame << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newitem->getObjId();

#if 0
	pout << "Item::create: created item " << objID << " (" << shape
	     << "," << frame << ")" << Std::endl;
#endif

	newitem->moveToEtherealVoid();

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return 1;
}

uint32 Item::I_pop(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused

	World *w = World::get_instance();

	if (w->etherealEmpty())
		return 0; // no items left on stack

	uint16 _objId = w->etherealPeek();
	Item *item = getItem(_objId);
	if (!item) {
		w->etherealRemove(_objId);
		return 0; // top item was invalid
	}

	item->returnFromEtherealVoid();

#if 0
	perr << "Popping item to original location: " << item->getShape() << "," << item->getFrame() << Std::endl;
#endif

	//! Anything else?

	return _objId;
}

uint32 Item::I_popToCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	World *w = World::get_instance();

	if (w->etherealEmpty())
		return 0; // no items left on stack

	uint16 objId = w->etherealPeek();
	Item *item = getItem(objId);
	if (!item) {
		w->etherealRemove(objId);
		return 0; // top item was invalid
	}

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	item->move(x, y, z);

#if 0
	perr << "Popping item into map: " << item->getShape() << "," << item->getFrame() << " at (" << x << "," << y << "," << z << ")" << Std::endl;
#endif

	//! Anything else?

	return objId;
}

uint32 Item::I_popToContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << Std::endl;
		return 0;
	}

	World *w = World::get_instance();

	if (w->etherealEmpty())
		return 0; // no items left on stack

	uint16 _objId = w->etherealPeek();
	Item *item = getItem(_objId);
	if (!item) {
		w->etherealRemove(_objId);
		return 0; // top item was invalid
	}

	item->moveToContainer(container);

	//! Anything else?

	return _objId;
}

uint32 Item::I_popToEnd(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << Std::endl;
		return 0;
	}

	World *w = World::get_instance();

	if (w->etherealEmpty())
		return 0; // no items left on stack

	uint16 _objId = w->etherealPeek();
	Item *item = getItem(_objId);
	if (!item) {
		w->etherealRemove(_objId);
		return 0; // top item was invalid
	}

	item->moveToContainer(container);

	//! Anything else?

	//! This should probably be different from I_popToContainer, but
	//! how exactly?

	return _objId;
}

uint32 Item::I_move(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	if (!item)
		return 0;

	//! What should this do to ethereal items?
	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	#if 0
		perr << "Moving item: " << item->getShape() << "," << item->getFrame() << " to (" << x << "," << y << "," << z << ")" << Std::endl;
	#endif

	item->move(x, y, z);
	//item->collideMove(x, y, z, true, true);
	return 0;
}

uint32 Item::I_legalMoveToPoint(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(force); // 0/1
	ARG_UINT16(unknown2); // always 0

	int32 x = point.getX();
	int32 y = point.getY();
	int32 z = point.getZ();

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	if (!item)
		return 0;
	//! What should this do to ethereal items?
	if (item->collideMove(x, y, z, false, force == 1) == 0x4000)
		return 1;
	return 0;
}

uint32 Item::I_legalMoveToContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_CONTAINER_FROM_PTR(container);
	ARG_UINT16(unknown); // always 0
	if (!item || !container) return 0; // shouldn't happen?

	// try to move item to container checking weight and volume
	return item->moveToContainer(container, true);
}

uint32 Item::I_getEtherealTop(const uint8 * /*args*/, unsigned int /*argsize*/) {
	World *w = World::get_instance();
	if (w->etherealEmpty()) return 0; // no items left on stack
	return w->etherealPeek();
}


//!!! is this correct?
uint32 Item::I_getMapArray(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getMapNum();
}

//!!! is this correct?
uint32 Item::I_setMapArray(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mapNum);
	if (!item) return 0;

	item->setMapNum(mapNum);
	return 0;
}

uint32 Item::I_getNpcNum(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getNpcNum();
}

uint32 Item::I_setNpcNum(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(npcNum);
	if (!item) return 0;

	item->setNpcNum(npcNum);
	return 0;
}

uint32 Item::I_getDirToCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	return Direction_ToUsecodeDir(Direction_GetWorldDir(y - iy, x - ix, dirmode_8dirs));
}

uint32 Item::I_getDirFromCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	return Direction_ToUsecodeDir(Direction_GetWorldDir(iy - y, ix - x, dirmode_8dirs));
}

uint32 Item::I_getDirToItem(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	int32 i2x, i2y, i2z;
	item2->getLocationAbsolute(i2x, i2y, i2z);

	return Direction_ToUsecodeDir(Direction_GetWorldDir(i2y - iy, i2x - ix, dirmode_8dirs));
}

uint32 Item::I_getDirFromItem(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	int32 i2x, i2y, i2z;
	item2->getLocationAbsolute(i2x, i2y, i2z);

	return Direction_ToUsecodeDir(Direction_Invert(Direction_GetWorldDir(i2y - iy, i2x - ix, dirmode_8dirs)));
}

uint32 Item::I_getDirFromTo16(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x1);
	ARG_UINT16(y1);
	ARG_UINT16(x2);
	ARG_UINT16(y2);

	if (x1 == x2 && y1 == y2)
		return 16;

	return Direction_ToUsecodeDir(Direction_GetWorldDir(y2 - y1, x2 - x1, dirmode_16dirs));
}

uint32 Item::I_getClosestDirectionInRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x1);
	ARG_UINT16(y1);
	ARG_UINT16(x2);
	ARG_UINT16(y2);
	ARG_UINT16(ndirs);
	ARG_UINT16(mind);
	ARG_UINT16(maxd);

	Direction mindir = Direction_FromUsecodeDir(mind);
	Direction maxdir = Direction_FromUsecodeDir(maxd);
	DirectionMode mode = (ndirs == 16 ? dirmode_16dirs : dirmode_8dirs);
	Direction result = Direction_GetWorldDirInRange(y2 - y1, x2 - x1, mode, mindir, maxdir);
	return Direction_ToUsecodeDir(result);
}

uint32 Item::I_hurl(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(xs);
	ARG_SINT16(ys);
	ARG_SINT16(zs);
	ARG_SINT16(grav);
	if (!item) return 0;

	item->hurl(xs, ys, zs, grav);

	return item->_gravityPid;
}

uint32 Item::I_shoot(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(speed); // either 0x20 (fish) or 0x40 (death disk, dart)
	ARG_UINT16(gravity); // either 2 (fish) or 1 (death disk, dart)
	if (!item) return 0;

	assert(GAME_IS_U8);
	MissileTracker tracker(item, point.getX(), point.getY(), point.getZ(),
	                       speed, gravity);
	tracker.launchItem();

	return 0;
}

uint32 Item::I_fall(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->fall();

	return 0;
}

uint32 Item::I_grab(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->grab();

	return 0;
}

uint32 Item::I_getSliderInput(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); //    ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(minval);
	ARG_SINT16(maxval);
	ARG_SINT16(step);

	UCProcess *current = dynamic_cast<UCProcess *>(Kernel::get_instance()->getRunningProcess());
	assert(current);

//	pout << "SliderGump: min=" << minval << ", max=" << maxval << ", step=" << step << Std::endl;

	SliderGump *_gump = new SliderGump(100, 100, minval, maxval, minval, step);
	_gump->InitGump(0); // modal _gump
	_gump->setUsecodeNotify(current);

	current->suspend();

	return 0;
}

uint32 Item::I_openGump(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(gumpshape);
	if (!item) return 0;

	item->openGump(gumpshape);
	return 0;
}

uint32 Item::I_closeGump(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->closeGump();
	return 0;
}

uint32 Item::I_guardianBark(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(num);
	if (!item) return 0;

	assert(GAME_IS_U8);

	return item->callUsecodeEvent_guardianBark(num);
}

uint32 Item::I_getSurfaceWeight(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	World *world = World::get_instance();
	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
	                                      item, true, false, true);

	uint32 weight = 0;
	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *other = getItem(uclist.getuint16(i));
		if (!other) continue;
		weight += other->getTotalWeight();
	}

	return weight;
}

uint32 Item::I_isExplosive(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	assert(GAME_IS_U8); // explode bit has different meaning in Cru.
	if (!item) return 0;
	return item->getShapeInfo()->is_u8_explode() ? 1 : 0;
}

uint32 Item::I_receiveHit(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(other);
	ARG_SINT16(dir);
	ARG_SINT16(damage); // force of the hit
	ARG_UINT16(type); // hit type
	if (!item) return 0;

	item->receiveHit(other, Direction_FromUsecodeDir(dir), damage, type);

	return 0;
}

uint32 Item::I_explode(const uint8 *args, unsigned int argsize) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int exptype = 0;
	bool destroy_item = true;
	if (argsize > 4) {
		ARG_UINT16(etype)
		ARG_UINT16(destroy)
		exptype = etype;
		destroy_item = (destroy != 0);
	}

	item->explode(exptype, destroy_item);
	return 0;
}

uint32 Item::I_igniteChaos(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_NULL8();

	assert(GAME_IS_U8);

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(592)); // all oilflasks (CONSTANT!)
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	currentmap->areaSearch(&itemlist, script, sizeof(script), 0,
	                       160, false, x, y); //! CHECKME: 160?

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Item *item = getItem(itemlist.getuint16(i));
		if (!item) continue;
		item->use();
	}

	return 0;
}

uint32 Item::I_canReach(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(other);
	ARG_SINT16(range);

	// TODO: add cheat to make this always return 1

	if (item && other && item->canReach(other, range))
		return 1;
	else
		return 0;
}


uint32 Item::I_getRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(other);
	if (!item) return 0;
	if (!other) return 0;

	assert(GAME_IS_U8);

	return item->getRange(*other);
}

uint32 Item::I_getRangeIfVisible(const uint8 *args, unsigned int /*argsize*/) {
	// TODO: This is not exactly the same as the implementation in Cruasder,
	// but it should work?
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(other);

	if (!item || !other)
		return 0;

	// Somewhat arbitrary maths in here to replicate Crusader behavior.
	int range = item->getRangeIfVisible(*other) / 16;
	if ((range & 0xf) != 0)
		range++;

	if (range <= 48) {
		return range;
	}
	return 0;
}

uint32 Item::I_isCrusTypeNPC(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(sh);

	if (sh == 0x7FE) return 1;

	const ShapeInfo *info;
	info = GameData::get_instance()->getMainShapes()->getShapeInfo(sh);
	if (!info) return 0;

	if (info->_flags & ShapeInfo::SI_CRU_NPC)
		return 1;
	else
		return 0;
}

uint32 Item::I_setBroken(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item)
		return 0;

	World::get_instance()->getCurrentMap()->removeTargetItem(item);
	item->setFlag(FLG_BROKEN);
	return 0;
}

uint32 Item::I_inFastArea(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);

	if (!item)
		return 0;

	return item->hasFlags(FLG_FASTAREA);
}

uint32 Item::I_isOnScreen(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->isOnScreen();
}

uint32 Item::I_fireWeapon(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(x);
	ARG_SINT16(y);
	ARG_SINT16(z);
	ARG_UINT16(dir);
	ARG_UINT16(firetype);
	ARG_UINT16(unkflag);

	if (!item) return 0;

	return item->fireWeapon(x * 2, y * 2, z, Direction_FromUsecodeDir(dir), firetype, unkflag);
}

uint32 Item::I_fireDistance(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(other);
	ARG_SINT16(dir);
	ARG_SINT16(xoff);
	ARG_SINT16(yoff);
	ARG_SINT16(zoff);

	Item *otheritem = getItem(other);

	if (!item || !otheritem) return 0;

	return item->fireDistance(otheritem, Direction_FromUsecodeDir(dir), xoff * 2, yoff * 2, zoff);
}

} // End of namespace Ultima8
} // End of namespace Ultima
