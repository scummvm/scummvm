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
 */

#include "ultima/ultima8/gumps/paperdoll_gump.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/fonts/font.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/gfx/fonts/rendered_text.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/mini_stats_gump.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(PaperdollGump)



// lots of CONSTANTS...
const struct equipcoords_struct {
	int x, y;
} equipcoords[] = {
	{ 0, 0 },
	{ 24, 60 }, // shield
	{ 36, 50 }, // arm
	{ 40, 26 }, // head
	{ 40, 63 }, // body
	{ 40, 92 }, // legs
	{ 16, 18 }  // weapon
};

const struct statcords_struct {

	int xd, x, y;
} statcoords[] = {
	{ 90, 130, 24 },
	{ 90, 130, 33 },
	{ 90, 130, 42 },
	{ 90, 130, 51 },
	{ 90, 130, 60 },
	{ 90, 130, 69 },
	{ 90, 130, 78 }
};

static const int statdescwidth = 29;
static const int statwidth = 15;
static const int statheight = 8;
static const int statfont = 7;
static const int statdescfont = 0;
static const int statbuttonshape = 38;
static const int statbuttonx = 81;
static const int statbuttony = 84;


PaperdollGump::PaperdollGump() : ContainerGump(), _statButtonId(0),
		_draggingArmourClass(0), _draggingWeight(0),
		_backpackRect(49, 25, 59, 50) {
	Common::fill(_cachedText, _cachedText + 14, (RenderedText *)nullptr);
	Common::fill(_cachedVal, _cachedVal + 7, 0);
}

PaperdollGump::PaperdollGump(const Shape *shape, uint32 frameNum, uint16 owner,
		uint32 Flags, int32 layer)
		: ContainerGump(shape, frameNum, owner, Flags, layer),
		_statButtonId(0), _draggingArmourClass(0), _draggingWeight(0),
		_backpackRect(49, 25, 59, 50) {
	_statButtonId = 0;

	Common::fill(_cachedText, _cachedText + 14, (RenderedText *)nullptr);
	Common::fill(_cachedVal, _cachedVal + 7, 0);
}

PaperdollGump::~PaperdollGump() {
	for (int i = 0; i < 14; ++i) { // ! constant
		delete _cachedText[i];
	}
}

void PaperdollGump::InitGump(Gump *newparent, bool take_focus) {
	ContainerGump::InitGump(newparent, take_focus);

	FrameID button_up(GameData::GUMPS, statbuttonshape, 0);
	FrameID button_down(GameData::GUMPS, statbuttonshape, 1);

	Gump *widget = new ButtonWidget(statbuttonx, statbuttony,
	                                button_up, button_down);
	_statButtonId = widget->getObjId();
	widget->InitGump(this);
}

void PaperdollGump::Close(bool no_del) {
	// NOTE: this does _not_ call its direct parent's Close function
	// because we do not want to close the Gumps of our contents.

	// Make every item leave the fast area
	Container *c = getContainer(_owner);
	if (!c) return; // Container gone!?

	Std::list<Item *> &contents = c->_contents;
	Std::list<Item *>::iterator iter = contents.begin();
	while (iter != contents.end()) {
		Item *item = *iter;
		++iter;
		item->leaveFastArea();  // Can destroy the item
	}

	Item *o = getItem(_owner);
	if (o)
		o->clearGump(); //!! is this the appropriate place?

	ItemRelativeGump::Close(no_del);
}

void PaperdollGump::PaintStat(RenderSurface *surf, unsigned int n,
							  Std::string text, int val) {
	assert(n < 7); // constant!

	Font *font, *descfont;

	font = FontManager::get_instance()->getGameFont(statfont);
	descfont = FontManager::get_instance()->getGameFont(statdescfont);
	char buf[16]; // enough for uint32
	unsigned int remaining;

	if (!_cachedText[2 * n])
		_cachedText[2 * n] = descfont->renderText(text, remaining,
		                     statdescwidth, statheight,
		                     Font::TEXT_RIGHT);
	_cachedText[2 * n]->draw(surf, statcoords[n].xd, statcoords[n].y);

	if (!_cachedText[2 * n + 1] || _cachedVal[n] != val) {
		delete _cachedText[2 * n + 1];
		Common::sprintf_s(buf, "%d", val);
		_cachedText[2 * n + 1] = font->renderText(buf, remaining,
		                         statwidth, statheight,
		                         Font::TEXT_RIGHT);
		_cachedVal[n] = val;
	}
	_cachedText[2 * n + 1]->draw(surf, statcoords[n].x, statcoords[n].y);
}

void PaperdollGump::PaintStats(RenderSurface *surf, int32 lerp_factor) {
	Actor *a = getActor(_owner);
	assert(a);

	int armour = a->getArmourClass();
	int weight = a->getTotalWeight();
	if (_displayDragging) {
		armour += _draggingArmourClass;
		weight += _draggingWeight;
	}

	PaintStat(surf, 0, _TL_("STR"), a->getStr());
	PaintStat(surf, 1, _TL_("INT"), a->getInt());
	PaintStat(surf, 2, _TL_("DEX"), a->getDex());
	PaintStat(surf, 3, _TL_("ARMR"), armour);
	PaintStat(surf, 4, _TL_("HITS"), a->getHP());
	PaintStat(surf, 5, _TL_("MANA"), a->getMana());
	PaintStat(surf, 6, _TL_("WGHT"), weight / 10);
}

void PaperdollGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// paint self
	ItemRelativeGump::PaintThis(surf, lerp_factor, scaled);

	Actor *a = getActor(_owner);

	if (!a) {
		// Actor gone!?
		Close();
		return;
	}

	PaintStats(surf, lerp_factor);

	for (int i = 6; i >= 1; --i) { // constants
		Item *item = getItem(a->getEquip(i));
		if (!item) continue;
		int32 itemx, itemy;
		uint32 frame = item->getFrame() + 1;

		itemx = equipcoords[i].x;
		itemy = equipcoords[i].y;
		itemx += _itemArea.left;
		itemy += _itemArea.top;
		const Shape *s = item->getShapeObject();
		assert(s);
		surf->Paint(s, frame, itemx, itemy);
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
uint16 PaperdollGump::TraceObjId(int32 mx, int32 my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	ParentToGump(mx, my);

	Actor *a = getActor(_owner);

	if (!a)
		return 0; // Container gone!?

	for (int i = 1; i <= 6; ++i) {
		Item *item = getItem(a->getEquip(i));
		if (!item) continue;
		int32 itemx, itemy;

		itemx = equipcoords[i].x;
		itemy = equipcoords[i].y;
		itemx += _itemArea.left;
		itemy += _itemArea.top;
		const Shape *s = item->getShapeObject();
		assert(s);
		const ShapeFrame *frame = s->getFrame(item->getFrame() + 1);

		if (frame->hasPoint(mx - itemx, my - itemy)) {
			// found it
			return item->getObjId();
		}
	}

	// try backpack
	if (_backpackRect.contains(mx - _itemArea.left, my - _itemArea.top)) {
		ObjId bp = a->getEquip(ShapeInfo::SE_BACKPACK);
		if (bp)
			return bp;
	}

	// didn't find anything, so return self
	return getObjId();
}

// get item coords relative to self
bool PaperdollGump::GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
									  int32 lerp_factor) {

	Item *item = getItem(itemid);
	if (!item)
		return false; // item gone - shouldn't happen?
	Item *parent = item->getParentAsContainer();
	if (!parent || parent->getObjId() != _owner)
		return false;

	//!!! need to use lerp_factor

	if (item->getShape() == 529) { //!! constant
		gx = _backpackRect.left;
		gy = _backpackRect.top;
	} else {
		int equiptype = item->getZ();
		assert(equiptype >= 0 && equiptype <= 6); //!! constants
		gx = equipcoords[equiptype].x;
		gy = equipcoords[equiptype].y;
	}
	gx += _itemArea.left;
	gy += _itemArea.top;

	return true;
}

bool PaperdollGump::StartDraggingItem(Item *item, int mx, int my) {
	// can't drag backpack
	if (item->getShape() == 529) { //!! constant
		return false;
	}

	bool ret = ContainerGump::StartDraggingItem(item, mx, my);

	// set dragging offset to center of item
	const Shape *s = item->getShapeObject();
	assert(s);
	const ShapeFrame *frame = s->getFrame(item->getFrame());
	assert(frame);

	Mouse::get_instance()->setDraggingOffset(frame->_width / 2 - frame->_xoff,
	        frame->_height / 2 - frame->_yoff);

	// Remove equipment and clear owner on drag start for better drag feedback
	// NOTE: This original game appears to equip/unequip the item during drag instead of on drop
	if (_owner == item->getParent() && item->hasFlags(Item::FLG_EQUIPPED)) {
		Actor *a = getActor(_owner);
		if (a && a->removeItem(item)) {
			item->setParent(0);
		}
	}

	return ret;
}


bool PaperdollGump::DraggingItem(Item *item, int mx, int my) {
	if (!_itemArea.contains(mx, my)) {
		_displayDragging = false;
		return false;
	}

	Actor *a = getActor(_owner);
	assert(a);

	bool over_backpack = false;
	Container *backpack = getContainer(a->getEquip(7)); // constant!

	if (backpack && _backpackRect.contains(mx - _itemArea.left, my - _itemArea.top)) {
		over_backpack = true;
	}

	_displayDragging = true;

	_draggingShape = item->getShape();
	_draggingFrame = item->getFrame();
	_draggingFlags = item->getFlags();
	_draggingArmourClass = 0;
	_draggingWeight = 0;

	Container *root = item->getRootContainer();
	if (!root || root->getObjId() != _owner)
		_draggingWeight = item->getWeight();

	const ShapeInfo *si = item->getShapeInfo();
	int equiptype = si->_equipType;
	// determine target location and set dragging_x/y
	if (!over_backpack && equiptype) {
		// check if item will fit (weight/volume/etc...)
		if (!a->CanAddItem(item, true)) {
			_displayDragging = false;
			return false;
		}

		if (si->_armourInfo) {
			_draggingArmourClass += si->_armourInfo[_draggingFrame]._armourClass;
		}
		if (si->_weaponInfo) {
			_draggingArmourClass += si->_weaponInfo->_armourBonus;
		}

		_draggingFrame++;
		_draggingX = equipcoords[equiptype].x;
		_draggingY = equipcoords[equiptype].y;
	} else {
		// drop in backpack
		if (backpack && !backpack->CanAddItem(item, true)) {
			_displayDragging = false;
			return false;
		}

		_draggingX = _backpackRect.left + _backpackRect.width() / 2;
		_draggingY = _backpackRect.top + _backpackRect.height() / 2;
	}

	return true;
}

void PaperdollGump::DropItem(Item *item, int mx, int my) {
	_displayDragging = false;
	_draggingArmourClass = 0;
	_draggingWeight = 0;

	Actor *a = getActor(_owner);
	assert(a);

	bool over_backpack = false;
	Container *backpack = getContainer(a->getEquip(7)); // constant!

	if (backpack && _backpackRect.contains(mx - _itemArea.left, my - _itemArea.top)) {
		over_backpack = true;
	}

	int equiptype = item->getShapeInfo()->_equipType;
	if (!over_backpack && equiptype) {
		item->moveToContainer(a);
	} else {
		item->moveToContainer(backpack);
		item->randomGumpLocation();
	}
}

void PaperdollGump::ChildNotify(Gump *child, uint32 message) {
	if (child->getObjId() == _statButtonId &&
		(message == ButtonWidget::BUTTON_CLICK || message == ButtonWidget::BUTTON_DOUBLE)) {
		// check if there already is an open MiniStatsGump
		Gump *desktop = Ultima8Engine::get_instance()->getDesktopGump();
		Gump *statsgump = desktop->FindGump<MiniStatsGump>();

		if (!statsgump) {
			Gump *gump = new MiniStatsGump(0, 0);
			gump->InitGump(0);
			gump->setRelativePosition(BOTTOM_RIGHT, -5, -5);
		} else {
			// check if it is off-screen. If so, move it back
			Rect rect;
			desktop->GetDims(rect);
			Rect sr;
			statsgump->GetDims(sr);
			sr.grow(-2);
			statsgump->GumpRectToScreenSpace(sr);
			if (!sr.intersects(rect))
				statsgump->setRelativePosition(BOTTOM_RIGHT, -5, -5);
		}
	}
}


void PaperdollGump::saveData(Common::WriteStream *ws) {
	ContainerGump::saveData(ws);

	ws->writeUint16LE(_statButtonId);
}

bool PaperdollGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ContainerGump::loadData(rs, version)) return false;

	_statButtonId = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
