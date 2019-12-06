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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/paperdoll_gump.h"

#include "ultima8/graphics/shape.h"
#include "ultima8/graphics/shape_frame.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/games/game_data.h"
#include "ultima8/graphics/main_shape_archive.h"
#include "ultima8/graphics/fonts/font.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/graphics/fonts/rendered_text.h"
#include "ultima8/graphics/gump_shape_archive.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/gumps/mini_stats_gump.h"
#include "ultima8/ultima8.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(PaperdollGump, ContainerGump)



// lots of CONSTANTS...
struct equipcoords_struct {
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

struct statcords_struct {

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

const Pentagram::Rect backpack_rect(49, 25, 10, 25);

static const int statbuttonshape = 38;
static const int statbuttonx = 81;
static const int statbuttony = 84;


PaperdollGump::PaperdollGump()
	: ContainerGump() {
	for (int i = 0; i < 14; ++i) // ! constant
		cached_text[i] = 0;
}

PaperdollGump::PaperdollGump(Shape *shape_, uint32 framenum_, uint16 owner_,
                             uint32 Flags_, int32 layer_)
	: ContainerGump(shape_, framenum_, owner_, Flags_, layer_) {
	statbuttongid = 0;
	for (int i = 0; i < 14; ++i) // ! constant
		cached_text[i] = 0;
}

PaperdollGump::~PaperdollGump() {
	for (int i = 0; i < 14; ++i) { // ! constant
		delete cached_text[i];
		cached_text[i] = 0;
	}
}

void PaperdollGump::InitGump(Gump *newparent, bool take_focus) {
	ContainerGump::InitGump(newparent, take_focus);

	FrameID button_up(GameData::GUMPS, statbuttonshape, 0);
	FrameID button_down(GameData::GUMPS, statbuttonshape, 1);

	Gump *widget = new ButtonWidget(statbuttonx, statbuttony,
	                                button_up, button_down);
	statbuttongid = widget->getObjId();
	widget->InitGump(this);
}

void PaperdollGump::Close(bool no_del) {
	// NOTE: this does _not_ call its direct parent's Close function
	// because we do not want to close the Gumps of our contents.

	// Make every item leave the fast area
	Container *c = getContainer(owner);
	if (!c) return; // Container gone!?

	std::list<Item *> &contents = c->contents;
	std::list<Item *>::iterator iter = contents.begin();
	while (iter != contents.end()) {
		Item *item = *iter;
		++iter;
		item->leaveFastArea();  // Can destroy the item
	}

	Item *o = getItem(owner);
	if (o)
		o->clearGump(); //!! is this the appropriate place?

	ItemRelativeGump::Close(no_del);
}

void PaperdollGump::PaintStat(RenderSurface *surf, unsigned int n,
                              std::string text, int val) {
	assert(n < 7); // constant!

	Pentagram::Font *font, *descfont;

	font = FontManager::get_instance()->getGameFont(statfont);
	descfont = FontManager::get_instance()->getGameFont(statdescfont);
	char buf[16]; // enough for uint32
	unsigned int remaining;

	if (!cached_text[2 * n])
		cached_text[2 * n] = descfont->renderText(text, remaining,
		                     statdescwidth, statheight,
		                     Pentagram::Font::TEXT_RIGHT);
	cached_text[2 * n]->draw(surf, statcoords[n].xd, statcoords[n].y);

	if (!cached_text[2 * n + 1] || cached_val[n] != val) {
		delete cached_text[2 * n + 1];
		sprintf(buf, "%d", val);
		cached_text[2 * n + 1] = font->renderText(buf, remaining,
		                         statwidth, statheight,
		                         Pentagram::Font::TEXT_RIGHT);
		cached_val[n] = val;
	}
	cached_text[2 * n + 1]->draw(surf, statcoords[n].x, statcoords[n].y);
}

void PaperdollGump::PaintStats(RenderSurface *surf, int32 lerp_factor) {
	Actor *a = getActor(owner);
	assert(a);

	PaintStat(surf, 0, _TL_("STR"), a->getStr());
	PaintStat(surf, 1, _TL_("INT"), a->getInt());
	PaintStat(surf, 2, _TL_("DEX"), a->getDex());
	PaintStat(surf, 3, _TL_("ARMR"), a->getArmourClass());
	PaintStat(surf, 4, _TL_("HITS"), a->getHP());
	PaintStat(surf, 5, _TL_("MANA"), a->getMana());
	PaintStat(surf, 6, _TL_("WGHT"), a->getTotalWeight() / 10);
}

void PaperdollGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// paint self
	ItemRelativeGump::PaintThis(surf, lerp_factor, scaled);

	Actor *a = getActor(owner);

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
		itemx += itemarea.x;
		itemy += itemarea.y;
		Shape *s = item->getShapeObject();
		assert(s);
		surf->Paint(s, frame, itemx, itemy);
	}

	if (display_dragging) {
		int32 itemx, itemy;
		itemx = dragging_x + itemarea.x;
		itemy = dragging_y + itemarea.y;
		Shape *s = GameData::get_instance()->getMainShapes()->
		           getShape(dragging_shape);
		assert(s);
		surf->PaintInvisible(s, dragging_frame, itemx, itemy, false, (dragging_flags & Item::FLG_FLIPPED) != 0);
	}
}

// Find object (if any) at (mx,my)
// (mx,my) are relative to parent
uint16 PaperdollGump::TraceObjId(int mx, int my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (objId_ && objId_ != 65535) return objId_;

	ParentToGump(mx, my);

	Actor *a = getActor(owner);

	if (!a) return 0; // Container gone!?

	for (int i = 1; i <= 6; ++i) {
		Item *item = getItem(a->getEquip(i));
		if (!item) continue;
		int32 itemx, itemy;

		itemx = equipcoords[i].x;
		itemy = equipcoords[i].y;
		itemx += itemarea.x;
		itemy += itemarea.y;
		Shape *s = item->getShapeObject();
		assert(s);
		ShapeFrame *frame = s->getFrame(item->getFrame() + 1);

		if (frame->hasPoint(mx - itemx, my - itemy)) {
			// found it
			return item->getObjId();
		}
	}

	// try backpack
	if (backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		if (a->getEquip(7)) // constants
			return a->getEquip(7);
	}

	// didn't find anything, so return self
	return getObjId();
}

// get item coords relative to self
bool PaperdollGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
                                      int32 lerp_factor) {

	Item *item = getItem(itemid);
	Item *parent_ = item->getParentAsContainer();
	if (!parent_) return false;
	if (parent_->getObjId() != owner) return false;

	//!!! need to use lerp_factor

	if (item->getShape() == 529) { //!! constant
		gx = backpack_rect.x;
		gy = backpack_rect.y;
	} else {
		int equiptype = item->getZ();
		assert(equiptype >= 0 && equiptype <= 6); //!! constants
		gx = equipcoords[equiptype].x;
		gy = equipcoords[equiptype].y;
	}
	gx += itemarea.x;
	gy += itemarea.y;

	return true;
}

bool PaperdollGump::StartDraggingItem(Item *item, int mx, int my) {
	// can't drag backpack
	if (item->getShape() == 529) { //!! constant
		return false;
	}

	bool ret = ContainerGump::StartDraggingItem(item, mx, my);

	// set dragging offset to center of item
	Shape *s = item->getShapeObject();
	assert(s);
	ShapeFrame *frame = s->getFrame(item->getFrame());
	assert(frame);

	Ultima8Engine::get_instance()->setDraggingOffset(frame->width / 2 - frame->xoff,
	        frame->height / 2 - frame->yoff);

	return ret;
}


bool PaperdollGump::DraggingItem(Item *item, int mx, int my) {
	if (!itemarea.InRect(mx, my)) {
		display_dragging = false;
		return false;
	}

	Actor *a = getActor(owner);
	assert(a);

	bool over_backpack = false;
	Container *backpack = getContainer(a->getEquip(7)); // constant!

	if (backpack && backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		over_backpack = true;
	}

	display_dragging = true;

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();

	int equiptype = item->getShapeInfo()->equiptype;
	// determine target location and set dragging_x/y
	if (!over_backpack && equiptype) {
		// check if item will fit (weight/volume/etc...)
		if (!a->CanAddItem(item, true)) {
			display_dragging = false;
			return false;
		}

		dragging_frame++;
		dragging_x = equipcoords[equiptype].x;
		dragging_y = equipcoords[equiptype].y;
	} else {
		// drop in backpack

		if (!backpack->CanAddItem(item, true)) {
			display_dragging = false;
			return false;
		}

		dragging_x = backpack_rect.x + backpack_rect.w / 2;
		dragging_y = backpack_rect.y + backpack_rect.h / 2;
	}

	return true;
}

void PaperdollGump::DropItem(Item *item, int mx, int my) {
	display_dragging = false;

	Actor *a = getActor(owner);
	assert(a);

	bool over_backpack = false;
	Container *backpack = getContainer(a->getEquip(7)); // constant!

	if (backpack && backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		over_backpack = true;
	}

	int equiptype = item->getShapeInfo()->equiptype;
	if (!over_backpack && equiptype) {
		item->moveToContainer(a);
	} else {
		item->moveToContainer(backpack);
		item->randomGumpLocation();
	}
}

void PaperdollGump::ChildNotify(Gump *child, uint32 message) {
	if (child->getObjId() == statbuttongid &&
	        message == ButtonWidget::BUTTON_CLICK) {
		// check if there already is an open MiniStatsGump
		Gump *desktop = Ultima8Engine::get_instance()->getDesktopGump();
		Gump *statsgump = desktop->FindGump(MiniStatsGump::ClassType);

		if (!statsgump) {
			Gump *gump = new MiniStatsGump(0, 0);
			gump->InitGump(0);
			gump->setRelativePosition(BOTTOM_RIGHT, -5, -5);
		} else {
			// check if it is off-screen. If so, move it back
			Pentagram::Rect rect;
			desktop->GetDims(rect);
			Pentagram::Rect sr;
			statsgump->GetDims(sr);
			sr.x += 2;
			sr.w -= 4;
			sr.y += 2;
			sr.h -= 4;
			statsgump->GumpRectToScreenSpace(sr.x, sr.y, sr.w, sr.h);
			if (!sr.Overlaps(rect))
				statsgump->setRelativePosition(BOTTOM_RIGHT, -5, -5);
		}
	}
}


void PaperdollGump::saveData(ODataSource *ods) {
	ContainerGump::saveData(ods);

	ods->write2(statbuttongid);
}

bool PaperdollGump::loadData(IDataSource *ids, uint32 version) {
	if (!ContainerGump::loadData(ids, version)) return false;

	statbuttongid = ids->read2();

	return true;
}

} // End of namespace Ultima8
