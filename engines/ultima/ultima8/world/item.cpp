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
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/usecode/uc_stack.h"
#include "ultima/ultima8/misc/direction.h"
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
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/gumps/slider_gump.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/missile_tracker.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Item, Object)

Item::Item()
	: shape(0), frame(0), x(0), y(0), z(0),
	  flags(0), quality(0), npcnum(0), mapnum(0),
	  extendedflags(0), parent(0),
	  cachedShape(0), cachedShapeInfo(0), gump(0), gravitypid(0),
	  last_setup(0) {

}


Item::~Item() {

}

void Item::dumpInfo() {
	pout << "Item " << getObjId() << " (class "
	     << GetClassType().class_name << ", shape "
	     << getShape() << ", " << getFrame() << ", (";

	if (parent) {
		int32 gx, gy;
		getGumpLocation(gx, gy);
		pout << gx << "," << gy;
	} else {
		pout << x << "," << y << "," << z;
	}

	pout << ") q:" << getQuality()
	     << ", m:" << getMapNum() << ", n:" << getNpcNum()
	     << ", f:" << Std::hex << getFlags() << ", ef:"
	     << getExtFlags() << ")" << Std::dec << Std::endl;
}

Container *Item::getParentAsContainer() const {
	// No parent, no container
	if (!parent) return 0;

	Container *p = getContainer(parent);

	if (!p) {
		perr << "Item " << getObjId() << " parent (" << parent << ") is an invalid Container ObjID" << Std::endl;
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
	x = X;
	y = Y;
	z = Z;
}

void Item::move(int32 X, int32 Y, int32 Z) {
	bool no_lerping = false;
	CurrentMap *map = World::get_instance()->getCurrentMap();
	int mapChunkSize = map->getChunkSize();

	if (getObjId() == 1 && Z < 0) {
		perr.Print("Warning: moving avatar below Z=0. (%d,%d,%d)\n", X, Y, Z);
	}

	// It's currently in the ethereal void, remove it from there
	if (flags & FLG_ETHEREAL) {

		// Remove us from the ethereal void
		World::get_instance()->etherealRemove(objid);
	}

	// Remove from container (if contained or equiped)
	if (flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		if (parent) {
			// If we are flagged as Ethereal, we are already removed
			if (!(flags & FLG_ETHEREAL)) {
				Container *p = getParentAsContainer();
				if (p) p->removeItem(this);
			}
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no parent" << Std::endl;

		// Clear our owner.
		parent = 0;

		// No lerping when going from a container to somewhere else
		no_lerping = true;
	}
	// Item needs to be removed if it in the map, and it is moving to a
	// different chunk
	else if ((extendedflags & EXT_INCURMAP) &&
	         ((x / mapChunkSize != X / mapChunkSize) ||
	          (y / mapChunkSize != Y / mapChunkSize))) {

		// Remove us from the map
		map->removeItem(this);
	}

	// Unset all the various flags that no longer apply
	flags &= ~(FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL);

	// Set the location
	x = X;
	y = Y;
	z = Z;

	// Add it to the map if needed
	if (!(extendedflags & EXT_INCURMAP)) {
		// Disposable fast only items get put at the end
		// While normal items get put at start
		if (flags & (FLG_DISPOSABLE | FLG_FAST_ONLY))
			map->addItemToEnd(this);
		else
			map->addItem(this);
	}

	// Call just moved
	callUsecodeEvent_justMoved();

	// Are we moving somewhere fast
	bool dest_fast = map->isChunkFast(X / mapChunkSize, Y / mapChunkSize);

	// No lerping for this move
	if (no_lerping) extendedflags |= EXT_LERP_NOPREV;

	// If the destination is not in the fast area, and we are in
	// the fast area, we need to call leaveFastArea, as long as we aren't
	// being followed by the camera. We also disable lerping in such a case
	if (!dest_fast && (flags & Item::FLG_FASTAREA)) {
		extendedflags |= EXT_LERP_NOPREV;
		if (extendedflags & EXT_CAMERA)
			CameraProcess::GetCameraProcess()->ItemMoved();
		else
			leaveFastArea();

		return; //we are done
	}
	// Or if the dest is fast, and we are not, we need to call enterFastArea
	else if (dest_fast && !(flags & Item::FLG_FASTAREA)) {
		extendedflags |= EXT_LERP_NOPREV;
		enterFastArea();
	}

	// If we are being followed, notify the camera that we moved
	// Note that we don't need to
	if (extendedflags & EXT_CAMERA)
		CameraProcess::GetCameraProcess()->ItemMoved();
}

bool Item::moveToContainer(Container *container, bool checkwghtvol) {
	// Null container, report an error message
	if (!container) {
		perr << "NULL container passed to Item::moveToContainer" << Std::endl;
		return false;
	}

	// Already there, do nothing, but only if not ethereal
	bool ethereal_same = false;
	if (container->getObjId() == parent) {
		// If we are ethereal we'd like to know if we are just being moved back
		if (flags & FLG_ETHEREAL) ethereal_same = true;
		else return true;
	}

	// Eh, can't do it
	if (!container->CanAddItem(this, checkwghtvol)) return false;

	// It's currently in the ethereal void
	if (flags & FLG_ETHEREAL) {

		// Remove us from the ethereal void
		World::get_instance()->etherealRemove(objid);
	}

	// Remove from container (if contained or equiped)
	if (flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		if (parent) {
			// If we are flagged as Ethereal, we are already removed
			if (!(flags & FLG_ETHEREAL)) {
				Container *p = getParentAsContainer();
				if (p) p->removeItem(this);
			}
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no parent" << Std::endl;

		// Clear our owner.
		parent = 0;
	}
	// Item needs to be removed if it in the map
	else if (extendedflags & EXT_INCURMAP) {

		// Remove us from the map
		World::get_instance()->getCurrentMap()->removeItem(this);
	}

	// Unset all the various flags that no longer apply
	flags &= ~(FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL);

	// Set location to 0,0,0 if we aren't an ethereal item moving back
	if (!ethereal_same) x = y = 0;
	z = 0;

	// Add the item, don't bother with checking weight or vol since we already
	// know if it will fit or not
	container->addItem(this, false);

	// Set our owner.
	parent = container->getObjId();

	// Set us contained
	flags |= FLG_CONTAINED;

	// If moving to avatar, mark as OWNED
	Item *p = this;
	while (p->getParentAsContainer())
		p = p->getParentAsContainer();
	// In Avatar's inventory?
	if (p->getObjId() == 1)
		setFlagRecursively(FLG_OWNED);

	// No lerping when moving to a container
	extendedflags |= EXT_LERP_NOPREV;

	// Call just moved
	callUsecodeEvent_justMoved();

	// For a container, it's fast if it's got a gump open
	bool dest_fast = (container->flags & FLG_GUMP_OPEN) != 0;

	// If the destination is not in the fast area, and we are in
	// the fast area, we need to call leaveFastArea
	if (!dest_fast && (flags & Item::FLG_FASTAREA))
		leaveFastArea();
	// Or if the dest is fast, and we are not, we need to call enterFastArea
	else if (dest_fast && !(flags & Item::FLG_FASTAREA))
		enterFastArea();

	// Done
	return true;
}

void Item::moveToEtherealVoid() {
	// It's already Ethereal
	if (flags & FLG_ETHEREAL) return;

	// Add it to the ethereal void
	World::get_instance()->etherealPush(objid);

	// It's owned by something removed it from the something, but keep flags
	if (flags & (FLG_CONTAINED | FLG_EQUIPPED)) {

		if (parent) {
			Container *p = getParentAsContainer();
			if (p) p->removeItem(this);
		} else
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no parent" << Std::endl;
	} else if (extendedflags & EXT_INCURMAP) {
		World::get_instance()->getCurrentMap()->removeItem(this);
	}

	// Set the ETHEREAL Flag
	flags |=  FLG_ETHEREAL;
}

void Item::returnFromEtherealVoid() {
	// It's not Ethereal
	if (!(flags & FLG_ETHEREAL)) return;

	// Ok, we can do 2 things here
	// If an item has the contained or Equipped flags set, we return it to it's owner
	if (flags & (FLG_CONTAINED | FLG_EQUIPPED)) {
		Container *p = getParentAsContainer();
		if (!p) {
			perr << "Item " << getObjId() << " FLG_CONTAINED or FLG_EQUIPPED set but item has no valid parent" << Std::endl;
			CANT_HAPPEN();
		}
		moveToContainer(p);
	}
	// or we return it to the world
	else {
		move(x, y, z);
	}

}

void Item::movedByPlayer() {
	// owned-by-avatar items can't be stolen
	if (flags & FLG_OWNED) return;

	// Otherwise, player is stealing.
	// See if anybody is around to notice.
	Item *avatar = getItem(1);
	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	currentmap->areaSearch(&itemlist, script, sizeof(script),
	                       avatar, SCREEN_WIDTH, false);

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Actor *actor = getActor(itemlist.getuint16(i));
		if (actor && !actor->isDead())
			actor->callUsecodeEvent_AvatarStoleSomething(getObjId());
	}
}

int32 Item::getZ() const {
	return z;
}

void Item::getLocationAbsolute(int32 &X, int32 &Y, int32 &Z) const {
	if (parent) {
		Item *p = getParentAsContainer();

		if (p) {
			p->getLocationAbsolute(X, Y, Z);
			return;
		}
	}

	X = x;
	Y = y;
	Z = z;
}

void Item::getGumpLocation(int32 &X, int32 &Y) const {
	if (!parent) return;

	X = y & 0xFF;
	Y = (y >> 8) & 0xFF;
}

void Item::setGumpLocation(int32 X, int32 Y) {
	if (!parent) return;

	y = (X & 0xFF) + ((Y & 0xFF) << 8);
}

void Item::randomGumpLocation() {
	if (!parent) return;

	// This sets the coordinates to (255,255) and lets the ContainerGump
	// randomize the position when it is next opened.
	y = 0xFFFF;
}

void Item::getCentre(int32 &X, int32 &Y, int32 &Z) const {
	// constants!
	ShapeInfo *shapeinfo = getShapeInfo();
	if (flags & FLG_FLIPPED) {
		X = x - shapeinfo->y * 16;
		Y = y - shapeinfo->x * 16;
	} else {
		X = x - shapeinfo->x * 16;
		Y = y - shapeinfo->y * 16;
	}

	Z = z + shapeinfo->z * 4;
}

Pentagram::Box Item::getWorldBox() const {
	int32 xd, yd, zd;
	getFootpadWorld(xd, yd, zd);
	return Pentagram::Box(x, y, z, xd, yd, zd);
}

bool Item::overlaps(Item &item2) const {
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

bool Item::overlapsxy(Item &item2) const {
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

bool Item::isOn(Item &item2) const {
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

bool Item::canExistAt(int32 x_, int32 y_, int32 z_, bool needsupport) const {
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	Item *support;
	bool valid = cm->isValidPosition(x_, y_, z_, getShape(), getObjId(),
	                                 &support, 0);
	return valid && (!needsupport || support);
}

int Item::getDirToItemCentre(Item &item2) const {
	int32 xv, yv, zv;
	getCentre(xv, yv, zv);

	int32 i2x, i2y, i2z;
	item2.getCentre(i2x, i2y, i2z);

	return Get_WorldDirection(i2y - yv, i2x - xv);
}

int Item::getRange(Item &item2, bool checkz) const {
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

ShapeInfo *Item::getShapeInfoFromGameInstance() const {
	return GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
}

Shape *Item::getShapeObject() const {
	if (!cachedShape) cachedShape = GameData::get_instance()->getMainShapes()->getShape(shape);
	return cachedShape;
}

uint16 Item::getFamily() {
	return static_cast<uint16>(getShapeInfo()->family);
}

uint32 Item::getWeight() {
	uint32 weight = getShapeInfo()->weight;

	switch (getShapeInfo()->family) {
	case ShapeInfo::SF_QUANTITY:
		return ((getQuality() * weight) + 9) / 10;
	case ShapeInfo::SF_REAGENT:
		return getQuality() * weight;
	default:
		return weight * 10;
	}
}

uint32 Item::getTotalWeight() {
	return getWeight();
}

uint32 Item::getVolume() {
	// invisible items (trap markers and such) don't take up volume
	if (getFlags() & FLG_INVISIBLE) return 0;


	uint32 volume = getShapeInfo()->volume;

	switch (getShapeInfo()->family) {
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

bool Item::checkLoopScript(const uint8 *script, uint32 scriptsize) {
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

		case LS_TOKEN_Q: // item quality
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

		case LS_TOKEN_SHAPE: // item shape
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

		case LS_TOKEN_FRAME: // item frame
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
	if (parent) {
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
	map->sweepTest(start, end, dims, getShapeInfo()->flags, objid,
	               false, &collisions);

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
	if (teleport || parent) {
		// If teleporting and not force, work out if we can reach the end
		if (!force) {
			for (it = collisions.begin(); it != collisions.end(); it++) {
				// Uh oh, we hit something, can't move
				if (it->end_time == 0x4000 && !it->touching && it->blocking) {
					if (hititem) *hititem = it->item;
					if (dirs) *dirs = it->dirs;
					return 0;
				}
			}
		}

		// Trigger all the required events
		bool we_were_released = false;
		for (it = collisions.begin(); it != collisions.end(); it++) {
			Item *item = getItem(it->item);

			// Hitting us at the start and end, don't do anything
			if (!parent && it->hit_time == 0x0000 &&
			        it->end_time == 0x4000) {
				continue;
			}
			// Hitting us at the end (call hit on us, got hit on them)
			else if (it->end_time == 0x4000) {
				if (objid == 1 && guiapp->isShowTouchingItems())
					item->setExtFlag(Item::EXT_HIGHLIGHT);

				item->callUsecodeEvent_gotHit(objid, hitforce);
				callUsecodeEvent_hit(item->getObjId(), hitforce);
			}
			// Hitting us at the start (call release on us and them)
			else if (!parent && it->hit_time == 0x0000) {
				if (objid == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
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
				if (it->blocking && !it->touching) {
					if (hititem) *hititem = it->item;
					if (dirs) *dirs = it->dirs;
					hit = it->hit_time;
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
			Item *item = getItem(it->item);
			if (!item) continue;

			// Did we go past the endpoint of the move?
			if (it->hit_time > hit) break;

			uint16 proc_gothit = 0, proc_rel = 0;

			// If hitting at start, we should have already
			// called gotHit and hit.
			if ((!it->touching || it->touching_floor) && it->hit_time >= 0) {
				if (objid == 1 && guiapp->isShowTouchingItems())
					item->setExtFlag(Item::EXT_HIGHLIGHT);

				proc_gothit = item->callUsecodeEvent_gotHit(objid, hitforce);
				callUsecodeEvent_hit(item->getObjId(), hitforce);
			}

			// If not hitting at end, we will need to call release
			if (it->end_time < hit) {
				if (objid == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
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

unsigned int Item::countNearby(uint32 shape_, uint16 range) {
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	UCList itemlist(2);
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(shape_));
	currentmap->areaSearch(&itemlist, script, sizeof(script),
	                       this, range, false);
	return itemlist.getSize();
}


uint32 Item::callUsecodeEvent(uint32 event, const uint8 *args, int argsize) {
	uint32  class_id = shape;

	// Non-monster NPCs use objid/npcnum + 1024
	// Note: in the original, a non-monster NPC is specified with
	// the FAST_ONLY flag. However, this causes some summoned monster which
	// do not receive the FAST_ONLY flag to behave strangely. (Confirmed that
	// happens in the original as well.) -wjp 20050128
	if (objid < 256 && (extendedflags & EXT_PERMANENT_NPC))
		class_id = objid + 1024;

	// CHECKME: to make Pentagram behave as much like the original as possible,
	// don't call any usecode if the original would call the wrong class
	if (objid < 256 && !(extendedflags & EXT_PERMANENT_NPC) &&
	        !(flags & FLG_FAST_ONLY))
		return 0;

	// UnkEggs have quality+0x47F
	if (getFamily() == ShapeInfo::SF_UNKEGG)
		class_id = quality + 0x47F;

	Usecode *u = GameData::get_instance()->getMainUsecode();
	uint32 offset = u->get_class_event(class_id, event);
	if (!offset) return 0; // event not found

#ifdef DEBUG
	if (UCMachine::get_instance()->trace_event()) {
		pout.printf("Item: %d calling usecode event %d @ %04X:%04X\n",
		            objid, event, class_id, offset);
	}
#endif

	// FIXME: Disabled usecode except for Use events in crusader for now
	if (GAME_IS_CRUSADER && event != 1) return 0;

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

uint32 Item::callUsecodeEvent_combine() {                       // event C
	return callUsecodeEvent(0xC);   // CONSTANT
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

uint32 Item::use() {
	Actor *actor = p_dynamic_cast<Actor *>(this);
	if (actor) {
		if (actor->isDead()) {
			// dead actor, so open/close the dead-body-gump
			if (getFlags() & FLG_GUMP_OPEN) {
				closeGump();
			} else {
				openGump(12); // CONSTANT!!
			}
			return 0;
		}
	}

	return callUsecodeEvent_use();
}

void Item::destroy(bool delnow) {
	if (flags & FLG_ETHEREAL) {
		// Remove us from the ether
		World::get_instance()->etherealRemove(objid);
	} else if (parent) {
		// we're in a container, so remove self from parent
		//!! need to make sure this works for equipped items too...
		Container *p = getParentAsContainer();
		if (p) p->removeItem(this);
	} else if (extendedflags & EXT_INCURMAP) {
		// remove self from CurrentMap
		World::get_instance()->getCurrentMap()->removeItemFromList(this, x, y);
	}

	if (extendedflags & Item::EXT_CAMERA)
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
	if (last_setup && gametick == last_setup)
		return;

	// Are we lerping or are we not? Default we lerp.
	bool no_lerp = false;

	// No lerping this frame if Shape Changed, or No lerp is set,
	// or no last setup, or last setup more than 1 tick ago
	if ((last_setup == 0) || (l_next.shape != shape) ||
	        (extendedflags & EXT_LERP_NOPREV) ||
	        (gametick - last_setup) > 1 || flags & FLG_CONTAINED)
		no_lerp = true;

	// Update the time we were just setup
	last_setup = gametick;

	// Clear the flag
	extendedflags &= ~EXT_LERP_NOPREV;

	// Animate it, if needed
	if ((gametick % 3) == (objid % 3)) animateItem();

	// Setup the prev values for lerping
	if (!no_lerp) l_prev = l_next;

	// Setup next
	if (flags & FLG_CONTAINED) {
		l_next.x = ix = y & 0xFF;
		l_next.y = iy = (y >> 8) & 0xFF;
		l_next.z = iz = 0;
	} else {
		l_next.x = ix = x;
		l_next.y = iy = y;
		l_next.z = iz = z;
	}
	l_next.shape = shape;
	l_next.frame = frame;

	// Setup prev values for not lerping
	if (no_lerp) l_prev = l_next;
}

// Animate the item
void Item::animateItem() {
	ShapeInfo *info = getShapeInfo();
	Shape *shp = getShapeObject();

	if (!info->animtype) return;

	int anim_data = info->animdata;
	//bool dirty = false;

	if ((static_cast<int>(last_setup) % 6) != (objid % 6) && info->animtype != 1)
		return;

	switch (info->animtype) {
	case 2:
		// 50 % chance
		if (getRandom() & 1) break;

	case 1:
	case 3:
		// 50 % chance
		if (anim_data == 1 && (getRandom() & 1)) break;
		frame ++;
		if (anim_data < 2) {
			if (shp && frame == shp->frameCount()) frame = 0;
		} else {
			unsigned int num = (frame - 1) / anim_data;
			if (frame == ((num + 1)*anim_data)) frame = num * anim_data;
		}
		//dirty = true;
		break;

	case 4:
		if (!(getRandom() % anim_data)) break;
		frame ++;
		if (shp && frame == shp->frameCount()) frame = 0;
		//dirty = true;
		break;


	case 5:
		callUsecodeEvent_anim();
		//dirty = true;
		break;

	case 6:
		if (anim_data < 2) {
			if (frame == 0) break;
			frame ++;
			if (shp && frame == shp->frameCount()) frame = 1;
		} else {
			if (!(frame % anim_data)) break;
			frame ++;
			unsigned int num = (frame - 1) / anim_data;
			if (frame == ((num + 1)*anim_data)) frame = num * anim_data + 1;
		}
		//dirty = true;
		break;

	default:
		pout << "type " << info->animtype << " data " << anim_data << Std::endl;
		break;
	}
	//return dirty;
}


// Called when an item has entered the fast area
void Item::enterFastArea() {
	//!! HACK to get rid of endless SFX loops
	if (shape == 0x2c8) return;

	// Call usecode
	if (!(flags & FLG_FASTAREA)) {

		Actor *actor = p_dynamic_cast<Actor *>(this);
		if (actor && actor->isDead()) {
			// dead actor, don't call the usecode
		} else {
			callUsecodeEvent_enterFastArea();
		}
	}

	// We're fast!
	flags |= FLG_FASTAREA;
}

// Called when an item is leaving the fast area
void Item::leaveFastArea() {
	// Call usecode
	if ((!(flags & FLG_FAST_ONLY) || getShapeInfo()->is_noisy()) &&
	        (flags & FLG_FASTAREA))
		callUsecodeEvent_leaveFastArea();

	// If we have a gump open, close it (unless we're in a container)
	if (!parent && (flags & FLG_GUMP_OPEN)) {
		Gump *g = Ultima8Engine::get_instance()->getGump(gump);
		if (g) g->Close();
	}

	// Unset the flag
	flags &= ~FLG_FASTAREA;

	// CHECKME: what do we need to do exactly?
	// currently,  destroy object

	// Kill us if we are fast only, unless we're in a container
	if ((flags & FLG_FAST_ONLY) && !getParent()) {
		// destroy contents if container
		Container *c = p_dynamic_cast<Container *>(this);
		if (c) c->destroyContents();

		destroy();
		// NB: destroy() creates an DestroyItemProcess to actually
		// delete the item in this case.
	}
	// If we have a gravity process, move us to the ground
	else if (gravitypid) {
		Process *p = Kernel::get_instance()->getProcess(gravitypid);
		if (p) {
			p->terminateDeferred();
			gravitypid = 0;
			collideMove(x, y, 0, true, false);
		}
	}
}

uint16 Item::openGump(uint32 gumpshape) {
	if (flags & FLG_GUMP_OPEN) return 0;
	assert(gump == 0);
	Shape *shapeP = GameData::get_instance()->getGumps()->getShape(gumpshape);

	ContainerGump *cgump;

	if (getObjId() != 1) { //!! constant
		cgump = new ContainerGump(shapeP, 0, objid, Gump::FLAG_ITEM_DEPENDENT |
		                          Gump::FLAG_DRAGGABLE);
	} else {
		cgump = new PaperdollGump(shapeP, 0, objid, Gump::FLAG_ITEM_DEPENDENT |
		                          Gump::FLAG_DRAGGABLE);
	}
	//!!TODO: clean up the way this is set
	//!! having the itemarea associated with the shapeP through the
	//!! GumpShapeFlex maybe
	cgump->setItemArea(GameData::get_instance()->
	                   getGumps()->getGumpItemArea(gumpshape));
	cgump->InitGump(0);
	flags |= FLG_GUMP_OPEN;
	gump = cgump->getObjId();

	return gump;
}

void Item::closeGump() {
	if (!(flags & FLG_GUMP_OPEN)) return;

	Gump *g = Ultima8Engine::get_instance()->getGump(gump);
	assert(g);
	g->Close();

	// can we already clear gump here, or do we need to wait for the gump
	// to really close??
	clearGump();
}


void Item::clearGump() {
	gump = 0;
	flags &= ~FLG_GUMP_OPEN;
}

int32 Item::ascend(int delta) {
//	pout << "Ascend: objid=" << getObjId() << ", delta=" << delta << Std::endl;

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

		item->getLocation(ix, iy, iz);

		if (item->canExistAt(ix, iy, iz + delta)) {
			item->move(ix, iy, iz + delta); // automatically un-etherealizes item
		} else {
			// uh oh...
			// CHECKME: what do we do here?
			item->move(ix, iy, iz);
			if (delta < 0) item->fall();
		}
	}

	return dist;
}

GravityProcess *Item::ensureGravityProcess() {
	GravityProcess *p = 0;
	if (gravitypid) {
		p = p_dynamic_cast<GravityProcess *>(
		        Kernel::get_instance()->getProcess(gravitypid));
	} else {
		p = new GravityProcess(this, 0);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}
	assert(p);
	return p;
}

void Item::fall() {
	if (flags & FLG_HANGING || getShapeInfo()->is_fixed()) {
		// can't fall
		return;
	}

	GravityProcess *p = ensureGravityProcess();
	p->setGravity(4); //!! constant
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


void Item::explode() {
	Process *p = new SpriteProcess(578, 20, 34, 1, 1, //!! constants
	                               x, y, z);
	Kernel::get_instance()->addProcess(p);

	int sfx = (getRandom() % 2) ? 31 : 158;
	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc) audioproc->playSFX(sfx, 0x60, 0, 0);

	int32 xv, yv, zv;
	getLocation(xv, yv, zv);

	destroy(); // delete self
	// WARNING: we are deleted at this point

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
		int dir = Get_WorldDirection(xv - xv, yv - yv); //!! CHECKME
		item->receiveHit(0, dir, 6 + (getRandom() % 6),
		                 WeaponInfo::DMG_BLUNT | WeaponInfo::DMG_FIRE);
	}
}

uint16 Item::getDamageType() {
	ShapeInfo *si = getShapeInfo();
	if (si->weaponinfo) {
		return si->weaponinfo->damage_type;
	}

	return 0;
}

void Item::receiveHit(uint16 other, int dir, int damage, uint16 type) {
	// first, check if the item has a 'gotHit' usecode event
	if (callUsecodeEvent_gotHit(other, 0)) //!! TODO: what should the 0 be??
		return;

	// explosive?
	if (getShapeInfo()->is_explode()) {
		explode(); // warning: deletes this
		return;
	}

	// breakable?
	if (getFamily() == ShapeInfo::SF_BREAKABLE) {
		// CHECKME: anything else?
		destroy();
		return;
	}

	if (getShapeInfo()->is_fixed() || getShapeInfo()->weight == 0) {
		// can't move
		return;
	}

	// nothing special, so just hurl the item
	// TODO: hurl item in direction, with speed depending on damage
	hurl(-16 * x_fact[dir], -16 * y_fact[dir], 16, 4); //!! constants
}

bool Item::canDrag() {
	ShapeInfo *si = getShapeInfo();
	if (si->is_fixed()) return false;
	if (si->weight == 0) return false;

	Actor *actor = p_dynamic_cast<Actor *>(this);
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
		if (it->item == item) continue;
		if (it->item == other && !usingAlternatePos) {
			other_hit_time = it->hit_time;
			continue;
		}

		// only touching?
		if (it->touching) continue;

		// hit something
		if (it->blocking && it->hit_time < blocked_time) {
			blocked_time = it->hit_time;
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
	               objid, false, &collisions);
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
	               objid, false, &collisions);
	if (checkLineOfSightCollisions(collisions, usingAlternatePos,
	                               getObjId(), other->getObjId()))
		return true;

	// if that fails, try line of sight between eye level and top of 2nd
	end[2] = otherZ + otherZd;

	collisions.clear();
	map->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
	               objid, false, &collisions);
	return checkLineOfSightCollisions(collisions, usingAlternatePos,
	                                  getObjId(), other->getObjId());
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

	// special cases: necromancy reagents, shape 395
	// blood: frame 0-5
	// bone: frame 6-7
	// wood: frame 8
	// dirt: frame 9
	// ex.hood: frame 10-12
	// blackmoor: frame 14-15
	if (CoreApp::get_instance()->getGameInfo()->type == GameInfo::GAME_U8) {
		if (getShape() != 395) return false;

		if (frame1 <= 5 && frame2 <= 5)
			return true;
		if (frame1 >= 6 && frame1 <= 7 && frame2 >= 6 && frame2 <= 7)
			return true;
		if (frame1 >= 10 && frame1 <= 12 && frame2 >= 10 && frame2 <= 12)
			return true;
		if (frame1 >= 14 && frame1 <= 15 && frame2 >= 14 && frame2 <= 15)
			return true;
	}
	return false;
}


void Item::saveData(ODataSource *ods) {
	Object::saveData(ods);
	ods->write2(static_cast<uint16>(extendedflags));
	ods->write2(flags);
	ods->write2(static_cast<uint16>(shape));
	ods->write2(static_cast<uint16>(frame));
	ods->write2(static_cast<uint16>(x));
	ods->write2(static_cast<uint16>(y));
	ods->write2(static_cast<uint16>(z));
	ods->write2(quality);
	ods->write2(npcnum);
	ods->write2(mapnum);
	if (getObjId() != 0xFFFF) {
		// these only make sense in currently loaded items
		ods->write2(gump);
		ods->write2(gravitypid);
	}
	if ((flags & FLG_ETHEREAL) && (flags & (FLG_CONTAINED | FLG_EQUIPPED)))
		ods->write2(parent);
}

bool Item::loadData(IDataSource *ids, uint32 version) {
	if (!Object::loadData(ids, version)) return false;

	extendedflags = ids->read2();
	flags = ids->read2();
	shape = ids->read2();
	frame = ids->read2();
	x = ids->read2();
	y = ids->read2();
	z = ids->read2();

	quality = ids->read2();
	npcnum = ids->read2();
	mapnum = ids->read2();
	if (getObjId() != 0xFFFF) {
		gump = ids->read2();
		gravitypid = ids->read2();
	} else {
		gump = gravitypid = 0;
	}

	if ((flags & FLG_ETHEREAL) && (flags & (FLG_CONTAINED | FLG_EQUIPPED)))
		parent = ids->read2();
	else
		parent = 0;

	//!! hackish...
	if (extendedflags & EXT_INCURMAP) {
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
	return x;
}

uint32 Item::I_getY(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);
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

	if (item->flags & FLG_FLIPPED)
		return x - item->getShapeInfo()->y * 16;
	else
		return x - item->getShapeInfo()->x * 16;
}

uint32 Item::I_getCY(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	if (item->flags & FLG_FLIPPED)
		return y - item->getShapeInfo()->x * 16;
	else
		return y - item->getShapeInfo()->y * 16;
}

uint32 Item::I_getCZ(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	return z + item->getShapeInfo()->z * 4;
}

uint32 Item::I_getPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UC_PTR(ptr);
	if (!item) return 0;

	int32 x, y, z;
	item->getLocationAbsolute(x, y, z);

	WorldPoint point;
	point.setX(x);
	point.setY(y);
	point.setZ(z);

	UCMachine::get_instance()->assignPointer(ptr, point.buf, 5);

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

	//! What do we do if item has no parent?
	//! What do we do with equipped items?

	if (item->getParent())
		return item->getParent();
	else
		return 0;
}

uint32 Item::I_getRootContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	Container *parent = item->getParentAsContainer();

	//! What do we do if item has no parent?
	//! What do we do with equipped items?

	if (!parent) return 0;

	while (parent->getParentAsContainer()) {
		parent = parent->getParentAsContainer();
	}

	return parent->getObjId();
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

uint32 Item::I_getFamily(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFamily();
}

uint32 Item::I_getTypeFlag(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(typeflag);
	if (!item) return 0;

	ShapeInfo *info = item->getShapeInfo();

	if (GAME_IS_U8 && typeflag >= 64)
		perr << "Invalid TypeFlag greater than 63 requested (" << typeflag << ") by Usecode" << Std::endl;
	if (GAME_IS_CRUSADER && typeflag >= 72)
		perr << "Invalid TypeFlag greater than 63 requested (" << typeflag << ") by Usecode" << Std::endl;

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

uint32 Item::I_orStatus(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags |= mask;
	return 0;
}

uint32 Item::I_andStatus(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags &= mask;
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
	Gump *gump = new BarkGump(item->getObjId(), str, shapenum);

	if (item->getObjId() < 256) { // CONSTANT!
		GumpNotifyProcess *notifyproc;
		notifyproc = new ActorBarkNotifyProcess(item->getObjId());
		Kernel::get_instance()->addProcess(notifyproc);
		gump->SetNotifyProcess(notifyproc);
	}

	gump->InitGump(0);

	return gump->GetNotifyProcess()->getPid();
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
	ARG_SINT16(unk);
	if (!item) return 0;

	return item->callUsecodeEvent_gotHit(hitter, unk);
}


uint32 Item::I_enterFastArea(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_enterFastArea();
}

uint32 Item::I_ask(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // currently unused.
	ARG_LIST(answers);

	if (!answers) return 0;

	// Use AskGump
	Gump *gump = new AskGump(1, answers);
	gump->InitGump(0);
	return gump->GetNotifyProcess()->getPid();
}

uint32 Item::I_legalCreateAtPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_WORLDPOINT(point);

	// check if item can exist
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(point.getX(), point.getY(), point.getZ(),
	                                 shape, 0, 0, 0);
	if (!valid)
		return 0;

	Item *newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0, true);
	if (!newitem) {
		perr << "I_legalCreateAtPoint failed to create item (" << shape
		     << "," << frame << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newitem->getObjId();
	newitem->move(point.getX(), point.getY(), point.getZ());

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

uint32 Item::I_getFamilyOfType(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(shape);

	return GameData::get_instance()->getMainShapes()->
	       getShapeInfo(shape)->family;
}

uint32 Item::I_push(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

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

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPeek();
	Item *item = getItem(objid);
	if (!item) {
		w->etherealRemove(objid);
		return 0; // top item was invalid
	}

	item->returnFromEtherealVoid();

#if 0
	perr << "Popping item to original location: " << item->getShape() << "," << item->getFrame() << Std::endl;
#endif

	//! Anything else?

	return objid;
}

uint32 Item::I_popToCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	World *w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPeek();
	Item *item = getItem(objid);
	if (!item) {
		w->etherealRemove(objid);
		return 0; // top item was invalid
	}

	item->move(x, y, z);

#if 0
	perr << "Popping item into map: " << item->getShape() << "," << item->getFrame() << " at (" << x << "," << y << "," << z << ")" << Std::endl;
#endif

	//! Anything else?

	return objid;
}

uint32 Item::I_popToContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << Std::endl;
		return 0;
	}

	World *w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPeek();
	Item *item = getItem(objid);
	if (!item) {
		w->etherealRemove(objid);
		return 0; // top item was invalid
	}

	item->moveToContainer(container);

	//! Anything else?

	return objid;
}

uint32 Item::I_popToEnd(const uint8 *args, unsigned int /*argsize*/) {
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << Std::endl;
		return 0;
	}

	World *w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPeek();
	Item *item = getItem(objid);
	if (!item) {
		w->etherealRemove(objid);
		return 0; // top item was invalid
	}

	item->moveToContainer(container);

	//! Anything else?

	//! This should probably be different from I_popToContainer, but
	//! how exactly?

	return objid;
}

uint32 Item::I_move(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	if (!item) return 0;

	//! What should this do to ethereal items?

	item->move(x, y, z);
	//item->collideMove(x, y, z, true, true);
	return 0;
}

uint32 Item::I_legalMoveToPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(force); // 0/1
	ARG_UINT16(unknown2); // always 0

	//! What should this do to ethereal items?

//	if (item->canExistAt(point.getX(), point.getY(), point.getZ())) {
//		item->move(point.getX(), point.getY(), point.getZ());
//		return 1;
//	} else {
	return item->collideMove(point.getX(), point.getY(), point.getZ(), false, force == 1) == 0x4000;
//	}
}

uint32 Item::I_legalMoveToContainer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_CONTAINER_FROM_PTR(container);
	ARG_UINT16(unknown); // always 0

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
	ARG_UINT16(mapnum);
	if (!item) return 0;

	item->setMapNum(mapnum);
	return 0;
}

uint32 Item::I_getNpcNum(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getNpcNum();
}

uint32 Item::I_getDirToCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	return Get_WorldDirection(y - iy, x - ix);
}

uint32 Item::I_getDirFromCoords(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	int32 ix, iy, iz;
	item->getLocationAbsolute(ix, iy, iz);

	return Get_WorldDirection(iy - y, ix - x);
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

	return Get_WorldDirection(i2y - iy, i2x - ix);
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

	return (Get_WorldDirection(i2y - iy, i2x - ix) + 4) % 8;
}

uint32 Item::I_hurl(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(xs);
	ARG_SINT16(ys);
	ARG_SINT16(zs);
	ARG_SINT16(grav);
	if (!item) return 0;

	item->hurl(xs, ys, zs, grav);

	return item->gravitypid;
}

uint32 Item::I_shoot(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(speed); // either 0x20 (fish) or 0x40 (death disk, dart)
	ARG_UINT16(gravity); // either 2 (fish) or 1 (death disk, dart)
	if (!item) return 0;

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

	UCProcess *current = p_dynamic_cast<UCProcess *>(Kernel::get_instance()->getRunningProcess());
	assert(current);

//	pout << "SliderGump: min=" << minval << ", max=" << maxval << ", step=" << step << Std::endl;

	SliderGump *gump = new SliderGump(100, 100, minval, maxval, minval, step);
	gump->InitGump(0); // modal gump
	gump->setUsecodeNotify(current);

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
	if (!item) return 0;
	return item->getShapeInfo()->is_explode() ? 1 : 0;
}

uint32 Item::I_receiveHit(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(other);
	ARG_SINT16(dir);
	ARG_SINT16(damage); // force of the hit
	ARG_UINT16(type); // hit type
	if (!item) return 0;

	item->receiveHit(other, dir, damage, type);

	return 0;
}

uint32 Item::I_explode(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->explode();
	return 0;
}

uint32 Item::I_igniteChaos(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_NULL8();

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

	if (item->canReach(other, range))
		return 1;
	else
		return 0;
}


uint32 Item::I_getRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(other);
	if (!item) return 0;
	if (!other) return 0;

	return item->getRange(*other);
}

uint32 Item::I_isCrusTypeNPC(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(sh);

	if (sh == 0x7FE) return 1;

	ShapeInfo *info;
	info = GameData::get_instance()->getMainShapes()->getShapeInfo(sh);
	if (!info) return 0;

	if (info->flags & ShapeInfo::SI_CRUS_NPC)
		return 1;
	else
		return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
