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
#include "ultima/ultima8/gumps/container_gump.h"

#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/slider_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/split_item_process.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ContainerGump)

ContainerGump::ContainerGump()
	: ItemRelativeGump(), _displayDragging(false), _draggingShape(0),
	  _draggingFrame(0), _draggingFlags(0), _draggingX(0), _draggingY(0) {

}

ContainerGump::ContainerGump(const Shape *shape, uint32 frameNum, uint16 owner,
                             uint32 flags, int32 layer)
	: ItemRelativeGump(0, 0, 5, 5, owner, flags, layer),
	  _displayDragging(false), _draggingShape(0), _draggingFrame(0),
	  _draggingFlags(0), _draggingX(0), _draggingY(0) {
	_shape = shape;
	_frameNum = frameNum;
}

ContainerGump::~ContainerGump() {
}

void ContainerGump::InitGump(Gump *newparent, bool take_focus) {
	UpdateDimsFromShape();

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);

	// make every item enter the fast area
	Container *c = getContainer(_owner);

	if (!c) return; // Container gone!?

	Std::list<Item *> &contents = c->_contents;
	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->enterFastArea();
	}


	// Position isn't like in the original
	// U8 puts a container gump slightly to the left of an object
}

void ContainerGump::getItemCoords(Item *item, int32 &itemx, int32 &itemy) {
	item->getGumpLocation(itemx, itemy);

	if (itemx == 0xFF && itemy == 0xFF) {
		// randomize position
		// TODO: maybe try to put it somewhere where it doesn't overlap others?

		itemx = getRandom() % _itemArea.width();
		itemy = getRandom() % _itemArea.height();

		item->setGumpLocation(itemx, itemy);
	}

	itemx += _itemArea.left;
	itemy += _itemArea.top;
}


void ContainerGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// paint self
	ItemRelativeGump::PaintThis(surf, lerp_factor, scaled);

	Container *c = getContainer(_owner);

	if (!c) {
		// Container gone!?
		Close();
		return;
	}

	Std::list<Item *> &contents = c->_contents;
	int32 gameframeno = Kernel::get_instance()->getFrameNum();

	//!! TODO: check these painting commands (flipped? translucent?)
	bool paintEditorItems = Ultima8Engine::get_instance()->isPaintEditorItems();

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item *item = *iter;
		item->setupLerp(gameframeno);

		if (!paintEditorItems && item->getShapeInfo()->is_editor())
			continue;

		int32 itemx, itemy;
		getItemCoords(item, itemx, itemy);
		const Shape *s = item->getShapeObject();
		assert(s);
		surf->Paint(s, item->getFrame(), itemx, itemy);
	}


	if (_displayDragging) {
		int32 itemx, itemy;
		itemx = _draggingX + _itemArea.left;
		itemy = _draggingY + _itemArea.top;
		Shape *s = GameData::get_instance()->getMainShapes()->
		           getShape(_draggingShape);
		assert(s);
		surf->PaintInvisible(s, _draggingFrame, itemx, itemy, false, (_draggingFlags & Item::FLG_FLIPPED) != 0);
	}

}

// Find object (if any) at (mx,my)
// (mx,my) are relative to parent
uint16 ContainerGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	ParentToGump(mx, my);

	Container *c = getContainer(_owner);

	if (!c)
		return 0; // Container gone!?

	bool paintEditorItems = Ultima8Engine::get_instance()->isPaintEditorItems();

	Std::list<Item *> &contents = c->_contents;
	Std::list<Item *>::reverse_iterator iter;

	// iterate backwards, since we're painting from begin() to end()
	for (iter = contents.rbegin(); iter != contents.rend(); ++iter) {
		Item *item = *iter;
		if (!paintEditorItems && item->getShapeInfo()->is_editor())
			continue;

		int32 itemx, itemy;
		getItemCoords(item, itemx, itemy);
		const Shape *s = item->getShapeObject();
		assert(s);
		const ShapeFrame *frame = s->getFrame(item->getFrame());

		if (frame->hasPoint(mx - itemx, my - itemy)) {
			// found it
			return item->getObjId();
		}
	}

	// didn't find anything, so return self
	return getObjId();
}

// get item coords relative to self
bool ContainerGump::GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
                                      int32 lerp_factor) {
	Item *item = getItem(itemid);
	if (!item) return false;
	Item *parent = item->getParentAsContainer();
	if (!parent) return false;
	if (parent->getObjId() != _owner) return false;

	//!!! need to use lerp_factor

	int32 itemx, itemy;
	getItemCoords(item, itemx, itemy);

	gx = itemx;
	gy = itemy;

	return false;
}

// we don't want our position to depend on Gump of parent container
// so change the default ItemRelativeGump behaviour
void ContainerGump::GetItemLocation(int32 lerp_factor) {
	Item *it = getItem(_owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	int32 gx, gy;
	Item *topitem = it;

	Container *p = it->getParentAsContainer();
	if (p) {
		while (p->getParentAsContainer()) {
			p = p->getParentAsContainer();
		}

		topitem = p;
	}

	Gump *gump = GetRootGump()->FindGump<GameMapGump>();
	assert(gump);
	gump->GetLocationOfItem(topitem->getObjId(), gx, gy, lerp_factor);

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gy = gy - it->getShapeInfo()->_z * 8 - 16;
	gump->GumpToScreenSpace(gx, gy);

	// Convert the screenspace coords into the coords of us
	if (_parent) _parent->ScreenSpaceToGump(gx, gy);

	// Set x and y, and center us over it
	_ix = gx - _dims.width() / 2;
	_iy = gy - _dims.height();
}

void ContainerGump::Close(bool no_del) {
	// close any gumps belonging to contents
	// and make every item leave the fast area
	Container *c = getContainer(_owner);
	if (!c) return; // Container gone!?

	Std::list<Item *> &contents = c->_contents;
	Std::list<Item *>::iterator iter = contents.begin();
	while (iter != contents.end()) {
		Item *item = *iter;
		++iter;
		Gump *g = getGump(item->getGump());
		if (g) {
			g->Close(); //!! what about no_del?
		}
		item->leaveFastArea();  // Can destroy the item
	}

	Item *o = getItem(_owner);
	if (o)
		o->clearGump(); //!! is this the appropriate place?

	ItemRelativeGump::Close(no_del);
}

Container *ContainerGump::getTargetContainer(Item *item, int mx, int my) {
	int32 px = mx, py = my;
	GumpToParent(px, py);
	Container *targetcontainer = getContainer(TraceObjId(px, py));

	if (targetcontainer && targetcontainer->getObjId() == item->getObjId())
		targetcontainer = nullptr;

	if (targetcontainer) {
		const ShapeInfo *targetinfo = targetcontainer->getShapeInfo();
		if ((targetcontainer->getObjId() == item->getObjId()) ||
		        targetinfo->is_land() ||
		        targetcontainer->hasFlags(Item::FLG_IN_NPC_LIST)) {
			targetcontainer = nullptr;
		}
	}

	if (!targetcontainer)
		targetcontainer = getContainer(_owner);

	return targetcontainer;
}


Gump *ContainerGump::onMouseDown(int button, int32 mx, int32 my) {
	Gump *handled = Gump::onMouseDown(button, mx, my);
	if (handled) return handled;

	// only interested in left clicks
	if (button == Shared::BUTTON_LEFT)
		return this;

	return nullptr;
}

void ContainerGump::onMouseClick(int button, int32 mx, int32 my) {
	if (button == Shared::BUTTON_LEFT) {
		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << Std::endl;
			return;
		}

		uint16 objID = TraceObjId(mx, my);

		Item *item = getItem(objID);
		if (item) {
			item->dumpInfo();

			// call the 'look' event
			item->callUsecodeEvent_look();
		}
	}
}

void ContainerGump::onMouseDouble(int button, int32 mx, int32 my) {
	if (button == Shared::BUTTON_LEFT) {
		if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << Std::endl;
			return;
		}

		uint16 objID = TraceObjId(mx, my);

		if (objID == getObjId()) {
			objID = _owner; // use container when double click on self
		}

		Item *item = getItem(objID);
		if (item) {
			item->dumpInfo();

			MainActor *avatar = getMainActor();
			if (objID == _owner || avatar->canReach(item, 128)) { // CONSTANT!
				// call the 'use' event
				item->use();
			} else {
				Mouse::get_instance()->flashCrossCursor();
			}
		}
	}
}


bool ContainerGump::StartDraggingItem(Item *item, int mx, int my) {
	// probably don't need to check if item can be moved, since it shouldn't
	// be in a container otherwise

	Container *c = getContainer(_owner);
	assert(c);

	// check if the container the item is in is in range
	MainActor *avatar = getMainActor();
	if (!avatar->canReach(c, 128)) return false;

	int32 itemx, itemy;
	getItemCoords(item, itemx, itemy);

	Mouse::get_instance()->setDraggingOffset(mx - itemx, my - itemy);

	return true;
}

bool ContainerGump::DraggingItem(Item *item, int mx, int my) {
	Container *c = getContainer(_owner);
	assert(c);

	// check if the container the item is in is in range
	MainActor *avatar = getMainActor();
	if (!avatar->canReach(c, 128)) {
		_displayDragging = false;
		return false;
	}

	int32 dox, doy;
	Mouse::get_instance()->getDraggingOffset(dox, doy);
	Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_TARGET);
	_displayDragging = true;

	_draggingShape = item->getShape();
	_draggingFrame = item->getFrame();
	_draggingFlags = item->getFlags();

	// determine target location and set dragging_x/y

	_draggingX = mx - _itemArea.left - dox;
	_draggingY = my - _itemArea.top - doy;

	const Shape *sh = item->getShapeObject();
	assert(sh);
	const ShapeFrame *fr = sh->getFrame(_draggingFrame);
	assert(fr);

	if (_draggingX - fr->_xoff < 0 ||
	        _draggingX - fr->_xoff + fr->_width > _itemArea.width() ||
	        _draggingY - fr->_yoff < 0 ||
	        _draggingY - fr->_yoff + fr->_height > _itemArea.height()) {
		_displayDragging = false;
		return false;
	}

	// check if item will fit (weight/volume/adding container to itself)
	Container *target = getTargetContainer(item, mx, my);
	if (!target || !target->CanAddItem(item, true)) {
		_displayDragging = false;
		return false;
	}

	return true;
}

void ContainerGump::DraggingItemLeftGump(Item *item) {
	_displayDragging = false;
}


void ContainerGump::StopDraggingItem(Item *item, bool moved) {
	if (!moved) return; // nothing to do
}

void ContainerGump::DropItem(Item *item, int mx, int my) {
	_displayDragging = false;

	int32 px = mx, py = my;
	GumpToParent(px, py);
	// see what the item is being dropped on
	Item *targetitem = getItem(TraceObjId(px, py));
	Container *targetcontainer = dynamic_cast<Container *>(targetitem);


	if (item->getShapeInfo()->hasQuantity() &&
	        item->getQuality() > 1) {
		// more than one, so see if we should ask if we should split it up

		Item *splittarget = nullptr;

		// also try to combine
		if (targetitem && item->canMergeWith(targetitem)) {
			splittarget = targetitem;
		}

		if (!splittarget) {
			// create new item
			splittarget = ItemFactory::createItem(
			                  item->getShape(), item->getFrame(), 0,
			                  item->getFlags() & (Item::FLG_DISPOSABLE | Item::FLG_OWNED | Item::FLG_INVISIBLE | Item::FLG_FLIPPED | Item::FLG_FAST_ONLY | Item::FLG_LOW_FRICTION), item->getNpcNum(), item->getMapNum(),
			                  item->getExtFlags() & (Item::EXT_SPRITE | Item::EXT_HIGHLIGHT | Item::EXT_TRANSPARENT), true);
			if (!splittarget) {
				perr << "ContainerGump failed to create item ("
				     << item->getShape() << "," << item->getFrame()
				     << ") while splitting" << Std::endl;
				return;
			}


			if (targetcontainer) {
				splittarget->moveToContainer(targetcontainer);
				splittarget->randomGumpLocation();
			} else {
				splittarget->moveToContainer(getContainer(_owner));
				splittarget->setGumpLocation(_draggingX, _draggingY);
			}
		}

		SliderGump *slidergump = new SliderGump(100, 100,
		                                        0, item->getQuality(),
		                                        item->getQuality());
		slidergump->InitGump(0);
		slidergump->CreateNotifier(); // manually create notifier
		Process *notifier = slidergump->GetNotifyProcess();
		SplitItemProcess *splitproc = new SplitItemProcess(item, splittarget);
		Kernel::get_instance()->addProcess(splitproc);
		splitproc->waitFor(notifier);

		return;
	}

	if (targetitem && item->getShapeInfo()->hasQuantity()) {
		// try to combine items
		if (item->canMergeWith(targetitem)) {
			targetitem->setQuality(targetitem->getQuality() +
			                       item->getQuality());
			targetitem->callUsecodeEvent_combine();

			// combined, so delete item
			item->destroy();
			return;
		}
	}

	targetcontainer = getTargetContainer(item, mx, my);
	assert(targetcontainer);

	if (targetcontainer->getObjId() != _owner) {
		if (item->getParent() == targetcontainer->getObjId()) {
			// already in this container, so move item to let it be drawn
			// on top of all other items
			targetcontainer->moveItemToEnd(item);
		} else {
			item->moveToContainer(targetcontainer);
			item->randomGumpLocation();
		}
	} else {
		// add item to self

		if (item->getParent() == _owner) {
			targetcontainer->moveItemToEnd(item);
		} else {
			item->moveToContainer(targetcontainer);
		}

		int32 dox, doy;
		Mouse::get_instance()->getDraggingOffset(dox, doy);
		_draggingX = mx - _itemArea.left - dox;
		_draggingY = my - _itemArea.top - doy;
		item->setGumpLocation(_draggingX, _draggingY);
	}
}

void ContainerGump::saveData(Common::WriteStream *ws) {
	ItemRelativeGump::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_itemArea.left));
	ws->writeUint32LE(static_cast<uint32>(_itemArea.top));
	ws->writeUint32LE(static_cast<uint32>(_itemArea.width()));
	ws->writeUint32LE(static_cast<uint32>(_itemArea.height()));
}

bool ContainerGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ItemRelativeGump::loadData(rs, version)) return false;

	int32 iax = static_cast<int32>(rs->readUint32LE());
	int32 iay = static_cast<int32>(rs->readUint32LE());
	int32 iaw = static_cast<int32>(rs->readUint32LE());
	int32 iah = static_cast<int32>(rs->readUint32LE());
	_itemArea.moveTo(iax, iay);
	_itemArea.setWidth(iaw);
	_itemArea.setHeight(iah);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
