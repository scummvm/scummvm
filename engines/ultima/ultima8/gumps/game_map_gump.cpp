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
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
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

DEFINE_RUNTIME_CLASSTYPE_CODE(GameMapGump, Gump)

bool GameMapGump::highlightItems = false;

GameMapGump::GameMapGump() :
	Gump(), display_dragging(false) {
	display_list = new ItemSorter();
}

GameMapGump::GameMapGump(int X, int Y, int Width, int Height) :
	Gump(X, Y, Width, Height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP, LAYER_GAMEMAP),
	display_list(0), display_dragging(false) {
	// Offset the gump. We want 0,0 to be the centre
	dims.x -= dims.w / 2;
	dims.y -= dims.h / 2;

	pout << "Create display_list ItemSorter object" << std::endl;
	display_list = new ItemSorter();
}

GameMapGump::~GameMapGump() {
	delete display_list;
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
	int lx, ly, lz;
	GetCameraLocation(lx, ly, lz, lerp_factor);

	CameraProcess *camera = CameraProcess::GetCameraProcess();

	uint16 roofid = 0;
	int zlimit = 1 << 16; // should be high enough

	if (!camera) {
		// Check roof
		//!! This is _not_ the right place for this...
		int32 ax, ay, az, axd, ayd, azd;
		Actor *av = getMainActor();
		av->getLocation(ax, ay, az);
		av->getFootpadWorld(axd, ayd, azd);
		map->isValidPosition(ax, ay, az, 32, 32, 8, 0, 1, 0, &roofid);
	} else
		roofid = camera->FindRoof(lerp_factor);

	Item *roof = getItem(roofid);
	if (roof) {
		zlimit = roof->getZ();
	}

	display_list->BeginDisplayList(surf, lx, ly, lz);

	uint32 gametick = Kernel::get_instance()->getFrameNum();

	bool paintEditorItems = Ultima8Engine::get_instance()->isPaintEditorItems();

	// Get all the required items
	for (int cy = 0; cy < MAP_NUM_CHUNKS; cy++) {
		for (int cx = 0; cx < MAP_NUM_CHUNKS; cx++) {
			// Not fast, ignore
			if (!map->isChunkFast(cx, cy)) continue;

			const std::list<Item *> *items = map->getItemList(cx, cy);

			if (!items) continue;

			std::list<Item *>::const_iterator it = items->begin();
			std::list<Item *>::const_iterator end = items->end();
			for (; it != end; ++it) {
				Item *item = *it;
				if (!item) continue;

				item->setupLerp(gametick);
				item->doLerp(lerp_factor);

				if (item->getZ() >= zlimit && !item->getShapeInfo()->is_draw())
					continue;
				if (!paintEditorItems && item->getShapeInfo()->is_editor())
					continue;
				if (item->getFlags() & Item::FLG_INVISIBLE) {
					// special case: invisible avatar _is_ drawn
					// HACK: unless EXT_TRANSPARENT is also set.
					// (Used for hiding the avatar when drawing a full area map)

					if (item->getObjId() == 1) {
						if (item->getExtFlags() & Item::EXT_TRANSPARENT)
							continue;

						int32 x_, y_, z_;
						item->getLerped(x_, y_, z_);
						display_list->AddItem(x_, y_, z_, item->getShape(), item->getFrame(), item->getFlags() & ~Item::FLG_INVISIBLE, item->getExtFlags() | Item::EXT_TRANSPARENT, 1);
					}

					continue;
				}
				display_list->AddItem(item);
			}
		}
	}

	// Dragging:

	if (display_dragging) {
		display_list->AddItem(dragging_pos[0], dragging_pos[1], dragging_pos[2],
		                      dragging_shape, dragging_frame,
		                      dragging_flags, Item::EXT_TRANSPARENT);
	}


	display_list->PaintDisplayList(highlightItems);
}

// Trace a click, and return ObjId
uint16 GameMapGump::TraceObjId(int mx, int my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	ParentToGump(mx, my);
	return display_list->Trace(mx, my, 0, highlightItems);
}

uint16 GameMapGump::TraceCoordinates(int mx, int my, int32 coords[3],
                                     int offsetx, int offsety, Item *item) {
	int32 dxd = 0, dyd = 0, dzd = 0;
	if (item)
		item->getFootpadWorld(dxd, dyd, dzd);

	int32 cx, cy, cz;
	GetCameraLocation(cx, cy, cz);

	ItemSorter::HitFace face;
	ObjId trace = display_list->Trace(mx, my, &face);

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

bool GameMapGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
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

Gump *GameMapGump::OnMouseDown(int button, int mx, int my) {
	int sx = mx, sy = my;
	ParentToGump(sx, sy);
	GumpToScreenSpace(sx, sy);

	AvatarMoverProcess *amp = Ultima8Engine::get_instance()->getAvatarMoverProcess();
	if (button == BUTTON_RIGHT || button == BUTTON_LEFT) {
		amp->OnMouseDown(button, sx, sy);
	}

	if (button == BUTTON_LEFT || button == BUTTON_RIGHT ||
	        button == BUTTON_MIDDLE) {
		// we take all clicks
		return this;
	}

	return 0;
}

void GameMapGump::OnMouseUp(int button, int mx, int my) {
	AvatarMoverProcess *amp = Ultima8Engine::get_instance()->getAvatarMoverProcess();
	if (button == BUTTON_RIGHT || button == BUTTON_LEFT) {
		amp->OnMouseUp(button);
	}
}

void GameMapGump::OnMouseClick(int button, int mx, int my) {
	MainActor *avatar = getMainActor();
	switch (button) {
	case BUTTON_LEFT: {
		if (avatar->isInCombat()) break;

		if (Mouse::get_instance()->isMouseDown(BUTTON_RIGHT)) break;

		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl;
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
	case BUTTON_MIDDLE: {
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
			if (p_dynamic_cast<Actor *>(item)) {
				p_dynamic_cast<Actor *>(item)->die(0);
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

void GameMapGump::OnMouseDouble(int button, int mx, int my) {
	MainActor *avatar = getMainActor();
	switch (button) {
	case BUTTON_LEFT: {
		if (avatar->isInCombat()) break;

		if (Mouse::get_instance()->isMouseDown(BUTTON_RIGHT)) break;

		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}

		uint16 objID = TraceObjId(mx, my);
		Item *item = getItem(objID);
		if (item) {
			int32 xv, yv, zv;
			item->getLocation(xv, yv, zv);
			item->dumpInfo();

			if (p_dynamic_cast<Actor *>(item) ||
			        avatar->canReach(item, 128)) { // CONSTANT!
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
	if (count > 0) display_list->IncSortLimit();
	else display_list->DecSortLimit();
}

bool GameMapGump::StartDraggingItem(Item *item, int mx, int my) {
//	ParentToGump(mx, my);

	if (!item->canDrag()) return false;

	MainActor *avatar = getMainActor();
	if (!avatar->canReach(item, 128)) return false;  // CONSTANT!

	// get item offset
	int itemx, itemy;
	GetLocationOfItem(item->getObjId(), itemx, itemy);
	Mouse::get_instance()->setDraggingOffset(mx - itemx, my - itemy);

	return true;
}

bool GameMapGump::DraggingItem(Item *item, int mx, int my) {
	// determine target location and set dragging_x/y/z
	int dox, doy;
	Mouse::get_instance()->getDraggingOffset(dox, doy);

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();
	display_dragging = true;

	// determine if item can be dropped here

	ObjId trace = TraceCoordinates(mx, my, dragging_pos, dox, doy, item);
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
	                      dragging_pos[0], dragging_pos[1], dragging_pos[2])) {
		// can't reach, so see if we can throw
		int throwrange = item->getThrowRange();
		if (throwrange && avatar->canReach(item, throwrange, dragging_pos[0],
		                                   dragging_pos[1], dragging_pos[2])) {
			int speed = 64 - item->getTotalWeight() + avatar->getStr();
			if (speed < 1) speed = 1;
			int32 ax, ay, az;
			avatar->getLocation(ax, ay, az);
			MissileTracker t(item, ax, ay, az,
			                 dragging_pos[0], dragging_pos[1], dragging_pos[2],
			                 speed, 4);
			if (t.isPathClear())
				throwing = true;
			else
				return false;
		} else {
			return false;
		}
	}

	if (!item->canExistAt(dragging_pos[0], dragging_pos[1], dragging_pos[2]))
		return false;

	if (throwing)
		Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_TARGET);

	return true;
}

void GameMapGump::DraggingItemLeftGump(Item *item) {
	display_dragging = false;
}


void GameMapGump::StopDraggingItem(Item *item, bool moved) {
	display_dragging = false;

	if (!moved) return; // nothing to do

	// make items on top of item fall and call release on supporting items
	item->grab();
}

void GameMapGump::DropItem(Item *item, int mx, int my) {
	int dox, doy;
	Mouse::get_instance()->getDraggingOffset(dox, doy);

	display_dragging = false;
	Actor *avatar = getMainActor();

	ObjId trace = TraceCoordinates(mx, my, dragging_pos, dox, doy, item);
	if (trace == 1) { // dropping on self
		ObjId bp = avatar->getEquip(7); // !! constant
		Container *backpack = getContainer(bp);
		if (backpack && item->moveToContainer(backpack)) {
			pout << "Dropped item in backpack" << std::endl;
			item->randomGumpLocation();
			return;
		}
	}

	if (!avatar->canReach(item, 128, // CONSTANT!
	                      dragging_pos[0], dragging_pos[1], dragging_pos[2])) {
		// can't reach, so throw
		pout << "Throwing item to (" << dragging_pos[0] << ","
		     << dragging_pos[1] << "," << dragging_pos[2] << ")" << std::endl;
		int speed = 64 - item->getTotalWeight() + avatar->getStr();
		if (speed < 1) speed = 1;
		int32 ax, ay, az;
		avatar->getLocation(ax, ay, az);
		// CHECKME: correct position to throw from?
		// CHECKME: correct events triggered when doing this move?
		item->move(ax, ay, az + 24);
		int32 tx, ty;
		tx = dragging_pos[0];
		ty = dragging_pos[1];
		int inaccuracy = 4 * (30 - avatar->getDex());
		if (inaccuracy < 20) inaccuracy = 20; // just in case dex > 25
		tx += (getRandom() % inaccuracy) - (getRandom() % inaccuracy);
		ty += (getRandom() % inaccuracy) - (getRandom() % inaccuracy);
		MissileTracker t(item, tx, ty, dragging_pos[2],
		                 speed, 4);
		t.launchItem();

		// FIXME: When doing this animation, sometimes items will
		//        get stuck on the avatar. Why?
#if 0
		avatar->doAnim(Animation::stand,
		               Get_WorldDirection(dragging_pos[1] - ay,
		                                  dragging_pos[0] - ax));
#endif
	} else {
		pout << "Dropping item at (" << dragging_pos[0] << ","
		     << dragging_pos[1] << "," << dragging_pos[2] << ")" << std::endl;

		// CHECKME: collideMove and grab (in StopDraggingItem)
		// both call release on supporting items.

		item->collideMove(dragging_pos[0], dragging_pos[1], dragging_pos[2],
		                  true, true); // teleport item
		item->fall();
	}
}

void GameMapGump::ConCmd_toggleHighlightItems(const Console::ArgvType &argv) {
	GameMapGump::SetHighlightItems(!GameMapGump::isHighlightItems());
}

void GameMapGump::ConCmd_dumpMap(const Console::ArgvType &) {
#ifdef TODO
	// We only support 32 bits per pixel for now
	if (RenderSurface::format.s_bpp != 32) return;

	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs.
	std::string savefile = "@save/dumpmap";
	Ultima8Engine::get_instance()->saveGame(savefile, "Pre-dumpMap save");

	// Increase number of available object IDs.
	ObjectManager::get_instance()->allow64kObjects();

	// Actual size
	int32 awidth = 8192;
	int32 aheight = 8192;

	int32 xpos = 0;
	int32 ypos = 0;

	int32 left = 16384;
	int32 right = -16384;
	int32 top = 16384;
	int32 bot = -16384;

	int32 camheight = 256;

	// Work out the map limit we do this very coarsly
	// Now render the map
	for (int32 y = 0; y < 64; y++) {
		for (int32 x = 0; x < 64; x++) {
			const std::list<Item *> *list =
			    World::get_instance()->getCurrentMap()->getItemList(x, y);

			// Should iterate the items!
			// (items could extend outside of this chunk and they have height)
			if (list && list->size() != 0) {
				int32 l = (x * 512 - y * 512) / 4 - 128;
				int32 r = (x * 512 - y * 512) / 4 + 128;
				int32 t = (x * 512 + y * 512) / 8 - 256;
				int32 b = (x * 512 + y * 512) / 8;

				t -= 256; // approx. adjustment for height of items in chunk

				if (l < left) left = l;
				if (r > right) right = r;
				if (t < top) top = t;
				if (b > bot) bot = b;
			}
		}
	}

	if (right == -16384) return;

	// camera height
	bot += camheight;
	top += camheight;

	awidth = right - left;
	aheight = bot - top;

	ypos = top;
	xpos = left;

	// Buffer Size
	int32 bwidth = awidth;
	int32 bheight = 256;

	// Tile size
	int32 twidth = bwidth / 8;
	int32 theight = bheight;


	GameMapGump *g = new GameMapGump(0, 0, twidth, theight);

	// HACK: Setting both INVISIBLE and TRANSPARENT flags on the Avatar
	// will make him completely invisible.
	getMainActor()->setFlag(Item::FLG_INVISIBLE);
	getMainActor()->setExtFlag(Item::EXT_TRANSPARENT);
	World::get_instance()->getCurrentMap()->setWholeMapFast();

	RenderSurface *s = RenderSurface::CreateSecondaryRenderSurface(bwidth,
	                   bheight);
	Texture *t = s->GetSurfaceAsTexture();
	// clear buffer
	std::memset(t->buffer, 0, 4 * bwidth * bheight);


	// Write tga header
	std::string filename = "@home/mapdump";
	char buf[32];
	sprintf(buf, "%02d",  World::get_instance()->getCurrentMap()->getNum());
	filename += buf;
	filename += ".png";
	ODataSource *ds = FileSystem::get_instance()->WriteFile(filename);
	std::string pngcomment = "Map ";
	pngcomment += buf;
	pngcomment += ", dumped by Pentagram.";

	PNGWriter *pngw = new PNGWriter(ds);
	pngw->init(awidth, aheight, pngcomment);

	// Now render the map
	for (int32 y = 0; y < aheight; y += theight) {
		for (int32 x = 0; x < awidth; x += twidth) {
			// Work out 'effective' and world coords
			int32 ex = xpos + x + twidth / 2;
			int32 ey = ypos + y + theight / 2;
			int32 wx = ex * 2 + ey * 4;
			int32 wy = ey * 4 - ex * 2;

			s->SetOrigin(x, y % bheight);
			CameraProcess::SetCameraProcess(
			    new CameraProcess(wx + 4 * camheight, wy + 4 * camheight, camheight));
			g->Paint(s, 256, false);

		}

		// Write out the current buffer
		if (((y + theight) % bheight) == 0) {
			for (int i = 0; i < bwidth * bheight; ++i) {
				// Convert to correct pixel format
				uint8 r, g, b;
				UNPACK_RGB8(t->buffer[i], r, g, b);
				uint8 *buf = reinterpret_cast<uint8 *>(&t->buffer[i]);
				buf[0] = b;
				buf[1] = g;
				buf[2] = r;
				buf[3] = 0xFF;
			}

			pngw->writeRows(bheight, t);

			// clear buffer for next set
			std::memset(t->buffer, 0, 4 * bwidth * bheight);
		}
	}

	pngw->finish();
	delete pngw;

	delete ds;

	delete g;
	delete s;

	// Reload
	Ultima8Engine::get_instance()->loadGame(savefile);

	pout << "Map stored in " << filename << "." << std::endl;
#endif
}

void GameMapGump::ConCmd_incrementSortOrder(const Console::ArgvType &argv) {
	GameMapGump *gameMapGump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gameMapGump) {
		gameMapGump->IncSortOrder(1);
	}
}

void GameMapGump::ConCmd_decrementSortOrder(const Console::ArgvType &argv) {
	GameMapGump *gameMapGump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gameMapGump) {
		gameMapGump->IncSortOrder(-1);
	}
}

void GameMapGump::RenderSurfaceChanged() {
	dims.x += dims.w / 2;
	dims.y += dims.h / 2;

	// Resize the desktop gump to match the parent
	Pentagram::Rect new_dims;
	parent->GetDims(new_dims);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	dims.x -= dims.w / 2;
	dims.y -= dims.h / 2;

	Gump::RenderSurfaceChanged();
}

void GameMapGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save GameMapGump");
}

bool GameMapGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load GameMapGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
