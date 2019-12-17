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
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/shape_archive.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/gumps/scaler_gump.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(Gump, Object)

Gump::Gump()
	: Object(), parent(0), children() {
}

Gump::Gump(int inX, int inY, int Width, int Height, uint16 inOwner,
           uint32 inFlags, int32 inLayer) :
	Object(), owner(inOwner), parent(0), x(inX), y(inY),
	dims(0, 0, Width, Height), flags(inFlags), layer(inLayer), index(-1),
	shape(0), framenum(0), children(), focus_child(0), notifier(0),
	process_result(0) {
	assignObjId(); // gumps always get an objid
}

Gump::~Gump() {
	// Get rid of focus
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;

	// Delete all children
	std::list<Gump *>::iterator it = children.begin();
	std::list<Gump *>::iterator end = children.end();

	while (it != end) {
		Gump *g = *it;
		it = children.erase(it);
		delete g;
	}
}

void Gump::InitGump(Gump *newparent, bool take_focus) {
	if (newparent)
		newparent->AddChild(this, take_focus);
	else
		Ultima8Engine::get_instance()->addGump(this);

	if (owner && !notifier) CreateNotifier();
}

void Gump::SetShape(FrameID frame, bool adjustsize) {
	shape = GameData::get_instance()->getShape(frame);
	framenum = frame.framenum;

	if (adjustsize && shape) {
		ShapeFrame *sf = shape->getFrame(framenum);
		dims.w = sf->width;
		dims.h = sf->height;
	}
}


void Gump::CreateNotifier() {
	assert(notifier == 0);

	// Create us a GumpNotifyProcess
	GumpNotifyProcess *p = new GumpNotifyProcess(owner);
	p->setGump(this);
	notifier = Kernel::get_instance()->addProcess(p);
}

void Gump::SetNotifyProcess(GumpNotifyProcess *proc) {
	assert(notifier == 0);
	notifier = proc->getPid();
}

GumpNotifyProcess *Gump::GetNotifyProcess() {
	return p_dynamic_cast<GumpNotifyProcess *>(Kernel::get_instance()->
	        getProcess(notifier));
}


void Gump::Close(bool no_del) {
	GumpNotifyProcess *p = GetNotifyProcess();
	if (p) {
		p->notifyClosing(process_result);
	}
	notifier = 0;

	if (!parent) {
		flags |= FLAG_CLOSING;
		if (!no_del) delete this;
	} else {
		flags |= FLAG_CLOSING;
		if (!no_del) flags |= FLAG_CLOSE_AND_DEL;
	}
}

void Gump::RenderSurfaceChanged() {
	// Iterate all children
	std::list<Gump *>::reverse_iterator it = children.rbegin();
	std::list<Gump *>::reverse_iterator end = children.rend();

	while (it != end) {
		(*it)->RenderSurfaceChanged();
		++it;
	}
}

void Gump::run() {
	// Iterate all children
	std::list<Gump *>::iterator it = children.begin();
	std::list<Gump *>::iterator end = children.end();

	while (it != end) {
		Gump *g = *it;

		// Run the child if it's not closing
		if (!(g->flags & FLAG_CLOSING))
			g->run();

		// If closing, we can kill it
		if (g->flags & FLAG_CLOSING) {
			it = children.erase(it);
			FindNewFocusChild();
			if (g->flags & FLAG_CLOSE_AND_DEL) delete g;
		} else {
			++it;
		}
	}
}

void Gump::CloseItemDependents() {
	// Close it, and return
	if (flags & FLAG_ITEM_DEPENDENT) {
		Close();
		return;
	}

	// Pass the MapChanged message to all the children
	std::list<Gump *>::iterator it = children.begin();
	std::list<Gump *>::iterator end = children.end();

	while (it != end) {
		Gump *g = *it;

		// Pass to child if it's not closing
		if (!(g->flags & FLAG_CLOSING)) g->CloseItemDependents();

		// If closing, we can kill it
		if (g->flags & FLAG_CLOSING) {
			it = children.erase(it);
			FindNewFocusChild();
			if (g->flags & FLAG_CLOSE_AND_DEL) delete g;
		} else {
			++it;
		}
	}
}

bool Gump::GetMouseCursor(int mx, int my, Shape &shape_, int32 &frame) {
	ParentToGump(mx, my);

	bool ret = false;

	// This reverse iterates the children
	std::list<Gump *>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it)
	{
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		// It's got the point
		if (g->PointOnGump(mx, my))
			ret = g->GetMouseCursor(mx, my, shape_, frame);

		if (ret) break;
	}

	return ret;
}

void Gump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Don't paint if hidden
	if (IsHidden()) return;

	// Get old Origin
	int ox = 0, oy = 0;
	surf->GetOrigin(ox, oy);

	// Set the new Origin
	int nx = 0, ny = 0;
	GumpToParent(nx, ny);
	surf->SetOrigin(ox + nx, oy + ny);

	// Get Old Clipping Rect
	Pentagram::Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	Pentagram::Rect new_rect = dims;
	new_rect.Intersect(old_rect);
	surf->SetClippingRect(new_rect);

	// Paint This
	PaintThis(surf, lerp_factor, scaled);

	// Paint children
	PaintChildren(surf, lerp_factor, scaled);

	// Reset The Clipping Rect
	surf->SetClippingRect(old_rect);

	// Reset The Origin
	surf->SetOrigin(ox, oy);
}

void Gump::PaintThis(RenderSurface *surf, int32 /*lerp_factor*/, bool /*scaled*/) {
	if (shape) {
#if 0
		Pentagram::Rect sr;
		ShapeFrame *f = shape->getFrame(framenum);
		sr.h = f->height;
		sr.w = f->height;
		sr.x = -f->xoff;
		sr.y = -f->yoff;

		if (surf->CheckClipped(sr))
#endif
			surf->Paint(shape, framenum, 0, 0);
#if 0
		else
			surf->PaintNoClip(shape, framenum, 0, 0);
#endif
	}
}

void Gump::PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Iterate all children
	std::list<Gump *>::iterator it = children.begin();
	std::list<Gump *>::iterator end = children.end();

	while (it != end) {
		Gump *g = *it;
		// Paint if not closing
		if (!(g->flags & FLAG_CLOSING))
			g->Paint(surf, lerp_factor, scaled);

		++it;
	}
}

void Gump::PaintCompositing(RenderSurface *surf, int32 lerp_factor,
                            int32 sx, int32 sy) {
	// Don't paint if hidden
	if (IsHidden()) return;

	// Get old Origin
	int ox = 0, oy = 0;
	surf->GetOrigin(ox, oy);

	// FIXME - Big accuracy problems here with the origin and clipping rect

	// Set the new Origin
	surf->SetOrigin(0, 0);

	// Get Old Clipping Rect
	Pentagram::Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	int cx = dims.x, cy = dims.y, cw = dims.w, ch = dims.h;
	GumpRectToScreenSpace(cx, cy, cw, ch, ROUND_OUTSIDE);
	Pentagram::Rect new_rect(cx, cy, cw, ch);
	new_rect.Intersect(old_rect);
	surf->SetClippingRect(new_rect);

	// Iterate all children
	std::list<Gump *>::reverse_iterator it = children.rbegin();
	std::list<Gump *>::reverse_iterator end = children.rend();

	while (it != end) {
		Gump *g = *it;
		// Paint if not closing
		if (!g->IsClosing())
			g->PaintCompositing(surf, lerp_factor, sx, sy);

		++it;
	}

	// Paint This
	PaintComposited(surf, lerp_factor, sx, sy);

	// Reset The Clipping Rect
	surf->SetClippingRect(old_rect);

	// Reset The Origin
	surf->SetOrigin(ox, oy);
}

void Gump::PaintComposited(RenderSurface * /*surf*/, int32 /*lerp_factor*/, int32 /*scalex*/, int32 /*scaley*/) {
}

Gump *Gump::FindGump(int mx, int my) {
	int gx = mx, gy = my;
	ParentToGump(gx, gy);
	Gump *gump = 0;

	// Iterate all children
	std::list<Gump *>::reverse_iterator it = children.rbegin();
	std::list<Gump *>::reverse_iterator end = children.rend();

	while (it != end && !gump) {
		Gump *g = *it;
		gump = g->FindGump(gx, gy);
		++it;
	}

	// it's over a child
	if (gump) return gump;

	// it's over this gump
	if (PointOnGump(mx, my)) return this;

	return 0;
}

void Gump::setRelativePosition(Gump::Position pos, int xoffset, int yoffset) {
	if (parent) {
		Pentagram::Rect rect;
		parent->GetDims(rect);

		switch (pos) {
		case CENTER:
			Move(rect.w / 2 - dims.w / 2 + xoffset,
			     rect.h / 2 - dims.h / 2 + yoffset);
			break;
		case TOP_LEFT:
			Move(xoffset, yoffset);
			break;
		case TOP_RIGHT:
			Move(rect.w - dims.w + xoffset, yoffset);
			break;
		case BOTTOM_LEFT:
			Move(xoffset, rect.h - dims.h + yoffset);
			break;
		case BOTTOM_RIGHT:
			Move(rect.w - dims.w + xoffset, rect.h - dims.h + yoffset);
			break;
		case TOP_CENTER:
			Move(rect.w / 2 - dims.w / 2 + xoffset, yoffset);
			break;
		case BOTTOM_CENTER:
			Move(rect.w / 2 - dims.w / 2 + xoffset, rect.h - dims.h + yoffset);
			break;
		default:
			break;
		}
	}
}

bool Gump::PointOnGump(int mx, int my) {
	int gx = mx, gy = my;
	ParentToGump(gx, gy);

	// First check again rectangle
	if (!dims.InRect(gx, gy)) {
		return false;
	}

	if (!shape) {
		// no shape? Then if it's in the rectangle it's on the gump.
		return true;
	}

	ShapeFrame *sf = shape->getFrame(framenum);
	assert(sf);
	if (sf->hasPoint(gx, gy)) {
		return true;
	}

	// reverse-iterate children
	std::list<Gump *>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it) {
		Gump *g = *it;

		// It's got the point
		if (g->PointOnGump(gx, gy)) return true;
	}

	return false;
}

// Convert a screen space point to a gump point
void Gump::ScreenSpaceToGump(int &sx, int &sy, PointRoundDir r) {
	// This is a recursive operation. We get each
	// parent to convert the point into their local
	// coords.
	if (parent) parent->ScreenSpaceToGump(sx, sy, r);

	ParentToGump(sx, sy, r);
}

// Convert a gump point to a screen space point
void Gump::GumpToScreenSpace(int &gx, int &gy, PointRoundDir r) {
	// This is a recursive operation. We get each
	// gump to convert the point to their parent

	GumpToParent(gx, gy, r);

	if (parent) parent->GumpToScreenSpace(gx, gy, r);
}

// Convert a parent relative point to a gump point
void Gump::ParentToGump(int &px, int &py, PointRoundDir) {
	px -= x;
	px += dims.x;
	py -= y;
	py += dims.y;
}

// Convert a gump point to parent relative point
void Gump::GumpToParent(int &gx, int &gy, PointRoundDir) {
	gx -= dims.x;
	gx += x;
	gy -= dims.y;
	gy += y;
}

// Transform a rectangle to screenspace from gumpspace
void Gump::GumpRectToScreenSpace(int &gx, int &gy, int &gw, int &gh,
                                 RectRoundDir r) {
	PointRoundDir tl = (r == ROUND_INSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);
	PointRoundDir br = (r == ROUND_OUTSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);

	int x1 = gx, y1 = gy;
	int x2 = gx + gw, y2 = gy + gh;
	GumpToScreenSpace(x1, y1, tl);
	GumpToScreenSpace(x2, y2, br);
	gx = x1;
	gy = y1;
	if (gw != 0) gw = x2 - x1;
	if (gh != 0) gh = y2 - y1;
}

// Transform a rectangle to gumpspace from screenspace
void Gump::ScreenSpaceToGumpRect(int &sx, int &sy, int &sw, int &sh,
                                 RectRoundDir r) {
	PointRoundDir tl = (r == ROUND_INSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);
	PointRoundDir br = (r == ROUND_OUTSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);

	int x1 = sx, y1 = sy;
	int x2 = sx + sw, y2 = sy + sh;
	ScreenSpaceToGump(x1, y1, tl);
	ScreenSpaceToGump(x2, y2, br);
	sx = x1;
	sy = y1;
	if (sw != 0) sw = x2 - x1;
	if (sh != 0) sh = y2 - y1;
}

uint16 Gump::TraceObjId(int mx, int my) {
	// Convert to local coords
	int gx = mx, gy = my;
	ParentToGump(gx, gy);

	uint16 objId_ = 0;

	// reverse-iterate children
	std::list<Gump *>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		// It's got the point
		if (g->PointOnGump(gx, gy)) objId_ = g->TraceObjId(gx, gy);

		if (objId_ && objId_ != 65535) break;
	}

//	if (!objId_ || objId_ == 65535)
//		if (PointOnGump(mx,my))
//			objId_ = getObjId();

	return objId_;
}

bool Gump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
                             int32 lerp_factor) {
	gx = 0;
	gy = 0;
	return false;
}

// Find a child gump of the specified type
Gump *Gump::FindGump(const RunTimeClassType &t, bool recursive,
                     bool no_inheritance) {
	// If that is our type, then return us!
	if (GetClassType() == t) return this;
	else if (!no_inheritance && IsOfType(t)) return this;

	// Iterate all children
	std::list<Gump *>::iterator  it = children.begin();
	std::list<Gump *>::iterator  end = children.end();

	for (; it != end; ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		if (g->GetClassType() == t) return g;
		else if (!no_inheritance && g->IsOfType(t)) return g;
	}

	if (!recursive) return 0;

	// Recursive Iterate all children
	it = children.begin();
	end = children.end();

	for (; it != end; ++it) {
		Gump *g = (*it);

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		g = g->FindGump(t, recursive, no_inheritance);

		if (g) return g;
	}

	return 0;
}

// Makes this gump the focus
void Gump::MakeFocus() {
	// By default we WONT do anything
	if (parent) {
		if (parent->focus_child) parent->focus_child->OnFocus(false);
		parent->focus_child = this;
	}
	OnFocus(true);
}

void Gump::FindNewFocusChild() {
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;

	// Now add the gump to use as the new focus
	std::list<Gump *>::reverse_iterator	it = children.rbegin();

	if (it != children.rend()) {
		(*it)->MakeFocus();
	}
}


// Adds a child to the list
void Gump::AddChild(Gump *gump, bool take_focus) {
	if (!gump) return;

	// Remove it if required
	Gump *old_parent = gump->GetParent();
	if (old_parent) old_parent->RemoveChild(gump);

	// Now add the gump in the correct spot
	std::list<Gump *>::iterator  it = children.begin();
	std::list<Gump *>::iterator  end = children.end();

	for (; it != end; ++it) {
		Gump *other = *it;

		// Why don't we check for FLAG_CLOSING here?
		// Because we want to make sure that the sort order is always valid

		// If we are same layer as focus and we wont take it, we will not be
		// placed in front of it
		if (!take_focus && other == focus_child && other->layer == gump->layer)
			break;

		// Lower layers get placed before higher layers
		if (other->layer > gump->layer) break;
	}

	// Now add it
	children.insert(it, gump);
	gump->parent = this;

	// Make the gump the focus if needed
	if (take_focus || !focus_child) {
		if (focus_child) focus_child->OnFocus(false);
		gump->OnFocus(true);
		focus_child = gump;
	}
}

// Remove a gump from the list
void Gump::RemoveChild(Gump *gump) {
	if (!gump) return;

	// Remove it
	children.remove(gump);
	gump->parent = 0;

	// Remove focus, the give upper most gump the focus
	if (gump == focus_child) {
		FindNewFocusChild();
	}
}

void Gump::MoveChildToFront(Gump *gump) {
	if (!gump) return;

	children.remove(gump);

	std::list<Gump *>::iterator  it = children.begin();
	std::list<Gump *>::iterator  end = children.end();
	for (; it != end; ++it) {
		Gump *other = *it;

		// Lower layers get placed before higher layers
		if (other->layer > gump->layer) break;
	}

	children.insert(it, gump);
}


Gump *Gump::GetRootGump() {
	if (!parent) return this;
	return parent->GetRootGump();
}


bool Gump::StartDraggingChild(Gump *gump, int mx, int my) {
	return false;
}

void Gump::DraggingChild(Gump *gump, int mx, int my) {
	CANT_HAPPEN();
}

void Gump::StopDraggingChild(Gump *gump) {
	CANT_HAPPEN();
}

//
// Input handling
//

Gump *Gump::OnMouseDown(int button, int mx, int my) {
	// Convert to local coords
	ParentToGump(mx, my);

	Gump *handled = 0;

	// Iterate children backwards
	std::list<Gump *>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING || g->IsHidden()) continue;

		// It's got the point
		if (g->PointOnGump(mx, my)) handled = g->OnMouseDown(button, mx, my);

		if (handled) break;
	}

	return handled;
}

Gump *Gump::OnMouseMotion(int mx, int my) {
	// Convert to local coords
	ParentToGump(mx, my);

	Gump *handled = 0;

	// Iterate children backwards
	std::list<Gump *>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING || g->IsHidden()) continue;

		// It's got the point
		if (g->PointOnGump(mx, my)) handled = g->OnMouseMotion(mx, my);

		if (handled) break;
	}

	// All gumps need to handle mouse motion
	if (!handled) handled = this;

	return handled;
}

//
// KeyInput
//
bool Gump::OnKeyDown(int key, int mod) {
	bool handled = false;
	if (focus_child) handled = focus_child->OnKeyDown(key, mod);
	return handled;
}

bool Gump::OnKeyUp(int key) {
	bool handled = false;
	if (focus_child) handled = focus_child->OnKeyUp(key);
	return handled;
}

bool Gump::OnTextInput(int unicode) {
	bool handled = false;
	if (focus_child) handled = focus_child->OnTextInput(unicode);
	return handled;
}

bool Gump::mustSave(bool toplevel) {
	// DONT_SAVE flag
	if (flags & FLAG_DONT_SAVE)
		return false;

	if (toplevel) {
		// don't save gumps with parents, unless parent is a core gump
		if (parent && !(parent->flags & FLAG_CORE_GUMP))
			return false;
	}

	return true;
}

void Gump::saveData(ODataSource *ods) {
	Object::saveData(ods);

	ods->write2(owner);
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(dims.x));
	ods->write4(static_cast<uint32>(dims.y));
	ods->write4(static_cast<uint32>(dims.w));
	ods->write4(static_cast<uint32>(dims.h));
	ods->write4(flags);
	ods->write4(static_cast<uint32>(layer));
	ods->write4(static_cast<uint32>(index));

	uint16 flex = 0;
	uint32 shapenum = 0;
	if (shape) {
		shape->getShapeId(flex, shapenum);
	}
	ods->write2(flex);
	ods->write4(shapenum);

	ods->write4(framenum);
	if (focus_child)
		ods->write2(focus_child->getObjId());
	else
		ods->write2(0);
	ods->write2(notifier);
	ods->write4(process_result);

	unsigned int childcount = 0;
	std::list<Gump *>::iterator it;
	for (it = children.begin(); it != children.end(); ++it) {
		if (!(*it)->mustSave(false)) continue;
		childcount++;
	}

	// write children:
	ods->write4(childcount);
	for (it = children.begin(); it != children.end(); ++it) {
		if (!(*it)->mustSave(false)) continue;

		(*it)->save(ods);
	}
}

bool Gump::loadData(IDataSource *ids, uint32 version) {
	if (!Object::loadData(ids, version)) return false;

	owner = ids->read2();
	x = static_cast<int32>(ids->read4());
	y = static_cast<int32>(ids->read4());

	int dx = static_cast<int32>(ids->read4());
	int dy = static_cast<int32>(ids->read4());
	int dw = static_cast<int32>(ids->read4());
	int dh = static_cast<int32>(ids->read4());
	dims.Set(dx, dy, dw, dh);

	flags = ids->read4();
	layer = static_cast<int32>(ids->read4());
	index = static_cast<int32>(ids->read4());

	shape = 0;
	ShapeArchive *flex = GameData::get_instance()->getShapeFlex(ids->read2());
	uint32 shapenum = ids->read4();
	if (flex) {
		shape = flex->getShape(shapenum);
	}

	framenum = ids->read4();
	uint16 focusid = ids->read2();
	focus_child = 0;
	notifier = ids->read2();
	process_result = ids->read4();

	// read children
	uint32 childcount = ids->read4();
	for (unsigned int i = 0; i < childcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(ids, version);
		Gump *child = p_dynamic_cast<Gump *>(obj);
		if (!child) return false;

		AddChild(child, false);

		if (child->getObjId() == focusid)
			focus_child = child;

	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
