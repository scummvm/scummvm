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
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/item_sorter.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/missile_tracker.h"
#include "ultima/ultima8/misc/direction.h"

#include "ultima/ultima8/world/gravity_process.h" // hack...
#include "ultima/ultima8/kernel/object_manager.h" // hack...
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"

// map dumping
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "image/png.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(GameMapGump)

bool GameMapGump::_highlightItems = false;

GameMapGump::GameMapGump() :
	Gump(), _displayDragging(false), _displayList(0), _draggingShape(0),
		_draggingFrame(0), _draggingFlags(0) {
	_displayList = new ItemSorter();
}

GameMapGump::GameMapGump(int x, int y, int width, int height) :
		Gump(x, y, width, height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP, LAYER_GAMEMAP),
		_displayList(0), _displayDragging(false), _draggingShape(0), _draggingFrame(0),
		_draggingFlags(0) {
	// Offset the gump. We want 0,0 to be the centre
	_dims.moveTo(-_dims.width() / 2, -_dims.height() / 2);

	pout << "Create _displayList ItemSorter object" << Std::endl;
	_displayList = new ItemSorter();
}

GameMapGump::~GameMapGump() {
	delete _displayList;
}

void GameMapGump::GetCameraLocation(int32 &lx, int32 &ly, int32 &lz,
                                    int lerp_factor) {
	CameraProcess *camera = CameraProcess::GetCameraProcess();
	if (!camera) {
		CameraProcess::GetCameraLocation(lx, ly, lz);
	} else {
		camera->GetLerped(lx, ly, lz, lerp_factor);
	}
}

void GameMapGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	World *world = World::get_instance();
	if (!world) return; // Is it possible the world doesn't exist?

	CurrentMap *map = world->getCurrentMap();
	if (!map) return;   // Is it possible the map doesn't exist?


	// Get the camera location
	int32 lx, ly, lz;
	GetCameraLocation(lx, ly, lz, lerp_factor);

	CameraProcess *camera = CameraProcess::GetCameraProcess();

	uint16 roofid = 0;
	int zlimit = 1 << 16; // should be high enough

	if (!camera) {
		// Check roof
		//!! This is _not_ the right place for this...
		int32 ax, ay, az, axd, ayd, azd;
		const Actor *av = getMainActor();
		av->getLocation(ax, ay, az);
		av->getFootpadWorld(axd, ayd, azd);
		map->isValidPosition(ax, ay, az, 32, 32, 8, 0, 1, 0, &roofid);
	} else
		roofid = camera->FindRoof(lerp_factor);

	const Item *roof = getItem(roofid);
	if (roof) {
		zlimit = roof->getZ();
	}

	_displayList->BeginDisplayList(surf, lx, ly, lz);

	uint32 gametick = Kernel::get_instance()->getFrameNum();

	bool paintEditorItems = Ultima8Engine::get_instance()->isPaintEditorItems();

	// Get all the required items
	for (int cy = 0; cy < MAP_NUM_CHUNKS; cy++) {
		for (int cx = 0; cx < MAP_NUM_CHUNKS; cx++) {
			// Not fast, ignore
			if (!map->isChunkFast(cx, cy)) continue;

			const Std::list<Item *> *items = map->getItemList(cx, cy);

			if (!items) continue;

			Std::list<Item *>::const_iterator it = items->begin();
			Std::list<Item *>::const_iterator end = items->end();
			for (; it != end; ++it) {
				Item *item = *it;
				if (!item) continue;

				item->setupLerp(gametick);
				item->doLerp(lerp_factor);

				if (item->getZ() >= zlimit && !item->getShapeInfo()->is_draw())
					continue;
				if (!paintEditorItems && item->getShapeInfo()->is_editor())
					continue;
				if (item->hasFlags(Item::FLG_INVISIBLE)) {
					// special case: invisible avatar _is_ drawn
					// HACK: unless EXT_TRANSPARENT is also set.
					// (Used for hiding the avatar when drawing a full area map)

					if (item->getObjId() == 1) {
						if (item->hasExtFlags(Item::EXT_TRANSPARENT))
							continue;

						int32 x_, y_, z_;
						item->getLerped(x_, y_, z_);
						_displayList->AddItem(x_, y_, z_, item->getShape(), item->getFrame(), item->getFlags() & ~Item::FLG_INVISIBLE, item->getExtFlags() | Item::EXT_TRANSPARENT, 1);
					}

					continue;
				}
				_displayList->AddItem(item);
			}
		}
	}

	// Dragging:

	if (_displayDragging) {
		_displayList->AddItem(_draggingPos[0], _draggingPos[1], _draggingPos[2],
		                      _draggingShape, _draggingFrame,
		                      _draggingFlags, Item::EXT_TRANSPARENT);
	}


	_displayList->PaintDisplayList(_highlightItems);
}

// Trace a click, and return ObjId
uint16 GameMapGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	ParentToGump(mx, my);
	return _displayList->Trace(mx, my, 0, _highlightItems);
}

uint16 GameMapGump::TraceCoordinates(int mx, int my, int32 coords[3],
                                     int offsetx, int offsety, Item *item) {
	int32 dxd = 0, dyd = 0, dzd = 0;
	if (item)
		item->getFootpadWorld(dxd, dyd, dzd);

	int32 cx, cy, cz;
	GetCameraLocation(cx, cy, cz);

	ItemSorter::HitFace face;
	ObjId trace = _displayList->Trace(mx, my, &face);

	Item *hit = getItem(trace);
	if (!hit) // strange...
		return 0;

	int32 hx, hy, hz;
	int32 hxd, hyd, hzd;
	hit->getLocation(hx, hy, hz);
	hit->getFootpadWorld(hxd, hyd, hzd);

	// adjust mx (if dragged item wasn't 'picked up' at its origin)
	mx -= offsetx;
	my -= offsety;

	// mx = (coords[0]-cx-coords[1]+cy)/4
	// my = (coords[0]-cx+coords[1]-cy)/8 - coords[2] + cz

	// the below expressions solve these two equations to two of the coords,
	// while fixing the other coord

	switch (face) {
	case ItemSorter::Z_FACE:
		coords[0] = 2 * mx + 4 * (my + hz + hzd) + cx - 4 * cz;
		coords[1] = -2 * mx + 4 * (my + hz + hzd) + cy - 4 * cz;
		coords[2] = hz + hzd;
		break;
	case ItemSorter::X_FACE:
		coords[0] = hx + dxd;
		coords[1] = -4 * mx + hx + dxd - cx + cy;
		coords[2] = -my + (hx + dxd) / 4 - mx / 2 - cx / 4 + cz;
		break;
	case ItemSorter::Y_FACE:
		coords[0] = 4 * mx + hy + dyd + cx - cy;
		coords[1] = hy + dyd;
		coords[2] = -my + mx / 2 + (hy + dyd) / 4 - cy / 4 + cz;
		break;
	}

	return trace;
}

bool GameMapGump::GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
                                    int32 lerp_factor) {
	Item *item = getItem(itemid);

	if (!item) return false;

	while (item->getParentAsContainer()) item = item->getParentAsContainer();

	int32 ix, iy, iz;

	// Hacks be us. Force the item into the fast area
	item->setupLerp(Kernel::get_instance()->getFrameNum());
	item->doLerp(lerp_factor);
	item->getLerped(ix, iy, iz);

	// Get the camera's location
	int32 cx, cy, cz;
	CameraProcess *cam = CameraProcess::GetCameraProcess();
	if (!cam) CameraProcess::GetCameraLocation(cx, cy, cz);
	else cam->GetLerped(cx, cy, cz, lerp_factor);

	// Screenspace bounding box bottom x coord (RNB x coord)
	gx = (ix - iy) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	gy = (ix + iy) / 8 - iz;

	// Screenspace bounding box bottom x coord (RNB x coord)
	gx -= (cx - cy) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	gy -= (cx + cy) / 8 - cz;

	return true;
}

Gump *GameMapGump::onMouseDown(int button, int32 mx, int32 my) {
	int32 sx = mx, sy = my;
	ParentToGump(sx, sy);
	GumpToScreenSpace(sx, sy);

	AvatarMoverProcess *amp = Ultima8Engine::get_instance()->getAvatarMoverProcess();
	if (button == Shared::BUTTON_RIGHT || button == Shared::BUTTON_LEFT) {
		amp->onMouseDown(button, sx, sy);
	}

	if (button == Shared::BUTTON_LEFT || button == Shared::BUTTON_RIGHT ||
	        button == Shared::BUTTON_MIDDLE) {
		// we take all clicks
		return this;
	}

	return nullptr;
}

void GameMapGump::onMouseUp(int button, int32 mx, int32 my) {
	AvatarMoverProcess *amp = Ultima8Engine::get_instance()->getAvatarMoverProcess();
	if (button == Shared::BUTTON_RIGHT || button == Shared::BUTTON_LEFT) {
		amp->onMouseUp(button);
	}
}

void GameMapGump::onMouseClick(int button, int32 mx, int32 my) {
	MainActor *avatar = getMainActor();
	switch (button) {
	case Shared::BUTTON_LEFT: {
		if (avatar->isInCombat()) break;

		if (Mouse::get_instance()->isMouseDownEvent(Shared::BUTTON_RIGHT)) break;

		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << Std::endl;
			break;
		}

		uint16 objID = TraceObjId(mx, my);
		Item *item = getItem(objID);
		if (item) {
			int32 xv, yv, zv;
			item->getLocation(xv, yv, zv);
			item->dumpInfo();

			// call the 'look' event
			item->callUsecodeEvent_look();
		}
		break;
	}
	case Shared::BUTTON_MIDDLE: {
		uint16 objID = TraceObjId(mx, my);
		Item *item = getItem(objID);
		if (item) {
			int32 xv, yv, zv;
			item->getLocation(xv, yv, zv);
			item->dumpInfo();

#if 1
			Actor *devon = getActor(1);
			PathfinderProcess *pfp = new PathfinderProcess(devon, xv, yv, zv);
//			PathfinderProcess* pfp = new PathfinderProcess(devon, objID, false);
			Kernel::get_instance()->addProcess(pfp);
#elif 0
			if (dynamic_cast<Actor *>(item)) {
				dynamic_cast<Actor *>(item)->die(0);
			} else {
				item->destroy();
			}
#elif 0
			UCList uclist(2);
			LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
			World *world = World::get_instance();
			world->getCurrentMap()->surfaceSearch(&uclist, script,
			                                      sizeof(script),
			                                      item, true, false, true);
			for (uint32 i = 0; i < uclist.getSize(); i++) {
				Item *item2 = getItem(uclist.getuint16(i));
				if (!item2) continue;
				item2->setExtFlag(Item::EXT_HIGHLIGHT);
			}
#elif 0
			item->receiveHit(1, 0, 1024, 0);
#elif 0
			item->clearFlag(Item::FLG_HANGING);
#endif
		}
	}
	default:
		break;
	}
}

void GameMapGump::onMouseDouble(int button, int32 mx, int32 my) {
	MainActor *avatar = getMainActor();
	switch (button) {
	case Shared::BUTTON_LEFT: {
		if (avatar->isInCombat()) break;

		if (Mouse::get_instance()->isMouseDownEvent(Shared::BUTTON_RIGHT)) break;

		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << Std::endl;
			break;
		}

		uint16 objID = TraceObjId(mx, my);
		Item *item = getItem(objID);
		if (item) {
			int32 xv, yv, zv;
			item->getLocation(xv, yv, zv);
			item->dumpInfo();

			int range = 128; // CONSTANT!
			if (GAME_IS_CRUSADER) {
				range = 512;
			}

			if (dynamic_cast<Actor *>(item) ||
			        avatar->canReach(item, range)) {
				// call the 'use' event
				item->use();
			} else {
				Mouse::get_instance()->flashCrossCursor();
			}
		}
		break;
	}
	default:
		break;
	}
}

void GameMapGump::IncSortOrder(int count) {
	if (count > 0) _displayList->IncSortLimit();
	else _displayList->DecSortLimit();
}

bool GameMapGump::StartDraggingItem(Item *item, int mx, int my) {
//	ParentToGump(mx, my);

	if (!item->canDrag()) return false;

	MainActor *avatar = getMainActor();
	if (!avatar->canReach(item, 128)) return false;  // CONSTANT!

	// get item offset
	int32 itemx = 0;
	int32 itemy = 0;
	GetLocationOfItem(item->getObjId(), itemx, itemy);
	Mouse::get_instance()->setDraggingOffset(mx - itemx, my - itemy);

	return true;
}

bool GameMapGump::DraggingItem(Item *item, int mx, int my) {
	// determine target location and set dragging_x/y/z
	int32 dox, doy;
	Mouse::get_instance()->getDraggingOffset(dox, doy);

	_draggingShape = item->getShape();
	_draggingFrame = item->getFrame();
	_draggingFlags = item->getFlags();
	_displayDragging = true;

	// determine if item can be dropped here

	ObjId trace = TraceCoordinates(mx, my, _draggingPos, dox, doy, item);
	if (!trace)
		return false;

	MainActor *avatar = getMainActor();
	if (trace == 1) { // dropping on self
		ObjId bp = avatar->getEquip(7); // !! constant
		Container *backpack = getContainer(bp);
		return  backpack->CanAddItem(item, true);
	}

	bool throwing = false;
	if (!avatar->canReach(item, 128, // CONSTANT!
	                      _draggingPos[0], _draggingPos[1], _draggingPos[2])) {
		// can't reach, so see if we can throw
		int throwrange = item->getThrowRange();
		if (throwrange && avatar->canReach(item, throwrange, _draggingPos[0],
		                                   _draggingPos[1], _draggingPos[2])) {
			int speed = 64 - item->getTotalWeight() + avatar->getStr();
			if (speed < 1) speed = 1;
			int32 ax, ay, az;
			avatar->getLocation(ax, ay, az);
			MissileTracker t(item, ax, ay, az,
			                 _draggingPos[0], _draggingPos[1], _draggingPos[2],
			                 speed, 4);
			if (t.isPathClear())
				throwing = true;
			else
				return false;
		} else {
			return false;
		}
	}

	if (!item->canExistAt(_draggingPos[0], _draggingPos[1], _draggingPos[2]))
		return false;

	if (throwing)
		Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_TARGET);

	return true;
}

void GameMapGump::DraggingItemLeftGump(Item *item) {
	_displayDragging = false;
}


void GameMapGump::StopDraggingItem(Item *item, bool moved) {
	_displayDragging = false;

	if (!moved) return; // nothing to do

	// make items on top of item fall and call release on supporting items
	item->grab();
}

void GameMapGump::DropItem(Item *item, int mx, int my) {
	int32 dox, doy;
	Mouse::get_instance()->getDraggingOffset(dox, doy);

	_displayDragging = false;
	Actor *avatar = getMainActor();

	ObjId trace = TraceCoordinates(mx, my, _draggingPos, dox, doy, item);
	if (trace == 1) { // dropping on self
		ObjId bp = avatar->getEquip(7); // !! constant
		Container *backpack = getContainer(bp);
		if (backpack && item->moveToContainer(backpack)) {
			pout << "Dropped item in backpack" << Std::endl;
			item->randomGumpLocation();
			return;
		}
	}

	if (!avatar->canReach(item, 128, // CONSTANT!
	                      _draggingPos[0], _draggingPos[1], _draggingPos[2])) {
		// can't reach, so throw
		pout << "Throwing item to (" << _draggingPos[0] << ","
		     << _draggingPos[1] << "," << _draggingPos[2] << ")" << Std::endl;
		int speed = 64 - item->getTotalWeight() + avatar->getStr();
		if (speed < 1) speed = 1;
		int32 ax, ay, az;
		avatar->getLocation(ax, ay, az);
		// CHECKME: correct position to throw from?
		// CHECKME: correct events triggered when doing this move?
		item->move(ax, ay, az + 24);
		int32 tx, ty;
		tx = _draggingPos[0];
		ty = _draggingPos[1];
		int inaccuracy = 4 * (30 - avatar->getDex());
		if (inaccuracy < 20) inaccuracy = 20; // just in case dex > 25
		tx += (getRandom() % inaccuracy) - (getRandom() % inaccuracy);
		ty += (getRandom() % inaccuracy) - (getRandom() % inaccuracy);
		MissileTracker t(item, tx, ty, _draggingPos[2],
		                 speed, 4);
		t.launchItem();

		// FIXME: When doing this animation, sometimes items will
		//        get stuck on the avatar. Why?
#if 0
		avatar->doAnim(Animation::stand,
		               Get_WorldDirection(_draggingPos[1] - ay,
		                                  _draggingPos[0] - ax));
#endif
	} else {
		pout << "Dropping item at (" << _draggingPos[0] << ","
		     << _draggingPos[1] << "," << _draggingPos[2] << ")" << Std::endl;

		// CHECKME: collideMove and grab (in StopDraggingItem)
		// both call release on supporting items.

		item->collideMove(_draggingPos[0], _draggingPos[1], _draggingPos[2],
		                  true, true); // teleport item
		item->fall();
	}
}

void GameMapGump::RenderSurfaceChanged() {
	// Resize the desktop gump to match the parent
	Rect new_dims;
	_parent->GetDims(new_dims);
	_dims.setWidth(new_dims.width());
	_dims.setHeight(new_dims.height());

	// Offset the gump. We want 0,0 to be the centre
	_dims.moveTo(-_dims.width() / 2, -_dims.height() / 2);

	Gump::RenderSurfaceChanged();
}

void GameMapGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save GameMapGump");
}

bool GameMapGump::loadData(Common::ReadStream *rs, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load GameMapGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
