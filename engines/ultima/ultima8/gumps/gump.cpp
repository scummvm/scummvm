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
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/gumps/scaler_gump.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(Gump)

Gump::Gump() : Object(), _parent(nullptr), _owner(0),
	_x(0), _y(0), _flags(0), _layer(0), _index(-1),
	_shape(nullptr), _frameNum(0), _focusChild(nullptr),
	_notifier(0), _processResult(0) {
}

Gump::Gump(int inX, int inY, int width, int height, uint16 inOwner,
           uint32 inFlags, int32 inLayer) :
	Object(), _owner(inOwner), _parent(nullptr), _x(inX), _y(inY),
	_dims(0, 0, width, height), _flags(inFlags), _layer(inLayer), _index(-1),
	_shape(nullptr), _frameNum(0), _children(), _focusChild(nullptr),
	_notifier(0), _processResult(0) {
	assignObjId(); // gumps always get an objid
}

Gump::~Gump() {
	// Get rid of focus
	if (_focusChild) _focusChild->OnFocus(false);
	_focusChild = nullptr;

	// Delete all children
	Std::list<Gump *>::iterator it = _children.begin();
	Std::list<Gump *>::iterator end = _children.end();

	while (it != end) {
		Gump *g = *it;
		it = _children.erase(it);
		delete g;
	}
}

void Gump::InitGump(Gump *newparent, bool take_focus) {
	if (newparent)
		newparent->AddChild(this, take_focus);
	else
		Ultima8Engine::get_instance()->addGump(this);

	if (_owner && !_notifier) CreateNotifier();
}

void Gump::SetShape(FrameID frame, bool adjustsize) {
	_shape = GameData::get_instance()->getShape(frame);
	_frameNum = frame._frameNum;

	if (adjustsize && _shape) {
		UpdateDimsFromShape();
	}
}

void Gump::UpdateDimsFromShape() {
	const ShapeFrame *sf = _shape->getFrame(_frameNum);
	assert(sf);
	_dims.w = sf->_width;
	_dims.h = sf->_height;
}

void Gump::CreateNotifier() {
	assert(_notifier == 0);

	// Create us a GumpNotifyProcess
	GumpNotifyProcess *p = new GumpNotifyProcess(_owner);
	p->setGump(this);
	_notifier = Kernel::get_instance()->addProcess(p);
}

void Gump::SetNotifyProcess(GumpNotifyProcess *proc) {
	assert(_notifier == 0);
	_notifier = proc->getPid();
}

GumpNotifyProcess *Gump::GetNotifyProcess() {
	return dynamic_cast<GumpNotifyProcess *>(Kernel::get_instance()->
	        getProcess(_notifier));
}


void Gump::Close(bool no_del) {
	GumpNotifyProcess *p = GetNotifyProcess();
	if (p) {
		p->notifyClosing(_processResult);
	}
	_notifier = 0;

	if (!_parent) {
		_flags |= FLAG_CLOSING;
		if (!no_del) delete this;
	} else {
		_flags |= FLAG_CLOSING;
		if (!no_del) _flags |= FLAG_CLOSE_AND_DEL;
	}
}

void Gump::RenderSurfaceChanged() {
	// Iterate all children
	Std::list<Gump *>::reverse_iterator it = _children.rbegin();
	Std::list<Gump *>::reverse_iterator end = _children.rend();

	while (it != end) {
		(*it)->RenderSurfaceChanged();
		++it;
	}
}

void Gump::run() {
	// Iterate all children
	Std::list<Gump *>::iterator it = _children.begin();
	Std::list<Gump *>::iterator end = _children.end();

	while (it != end) {
		Gump *g = *it;

		// Run the child if it's not closing
		if (!(g->_flags & FLAG_CLOSING))
			g->run();

		// If closing, we can kill it
		if (g->_flags & FLAG_CLOSING) {
			it = _children.erase(it);
			FindNewFocusChild();
			if (g->_flags & FLAG_CLOSE_AND_DEL) delete g;
		} else {
			++it;
		}
	}
}

void Gump::CloseItemDependents() {
	// Close it, and return
	if (_flags & FLAG_ITEM_DEPENDENT) {
		Close();
		return;
	}

	// Pass the MapChanged message to all the children
	Std::list<Gump *>::iterator it = _children.begin();
	Std::list<Gump *>::iterator end = _children.end();

	while (it != end) {
		Gump *g = *it;

		// Pass to child if it's not closing
		if (!(g->_flags & FLAG_CLOSING)) g->CloseItemDependents();

		// If closing, we can kill it
		if (g->_flags & FLAG_CLOSING) {
			it = _children.erase(it);
			FindNewFocusChild();
			if (g->_flags & FLAG_CLOSE_AND_DEL) delete g;
		} else {
			++it;
		}
	}
}

bool Gump::GetMouseCursor(int32 mx, int32 my, Shape &shape_, int32 &frame) {
	ParentToGump(mx, my);

	bool ret = false;

	// This reverse iterates the children
	Std::list<Gump *>::reverse_iterator it;
	for (it = _children.rbegin(); it != _children.rend(); ++it)
	{
		Gump *g = *it;

		// Not if closing
		if (g->_flags & FLAG_CLOSING) continue;

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
	int32 ox = 0, oy = 0;
	surf->GetOrigin(ox, oy);

	// Set the new Origin
	int32 nx = 0, ny = 0;
	GumpToParent(nx, ny);
	surf->SetOrigin(ox + nx, oy + ny);

	// Get Old Clipping Rect
	Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	Rect new_rect = _dims;
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
	if (_shape) {
#if 0
		Rect sr;
		ShapeFrame *f = shape->getFrame(_frameNum);
		sr.h = f->height;
		sr.w = f->height;
		sr.x = -f->xoff;
		sr.y = -f->yoff;

		if (surf->CheckClipped(sr))
#endif
			surf->Paint(_shape, _frameNum, 0, 0);
#if 0
		else
			surf->PaintNoClip(shape, _frameNum, 0, 0);
#endif
	}
}

void Gump::PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Iterate all children
	Std::list<Gump *>::iterator it = _children.begin();
	Std::list<Gump *>::iterator end = _children.end();

	while (it != end) {
		Gump *g = *it;
		// Paint if not closing
		if (!(g->_flags & FLAG_CLOSING))
			g->Paint(surf, lerp_factor, scaled);

		++it;
	}
}

void Gump::PaintCompositing(RenderSurface *surf, int32 lerp_factor,
                            int32 sx, int32 sy) {
	// Don't paint if hidden
	if (IsHidden()) return;

	// Get old Origin
	int32 ox = 0, oy = 0;
	surf->GetOrigin(ox, oy);

	// FIXME - Big accuracy problems here with the origin and clipping rect

	// Set the new Origin
	surf->SetOrigin(0, 0);

	// Get Old Clipping Rect
	Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	int32 cx = _dims.x, cy = _dims.y, cw = _dims.w, ch = _dims.h;
	GumpRectToScreenSpace(cx, cy, cw, ch, ROUND_OUTSIDE);
	Rect new_rect(cx, cy, cw, ch);
	new_rect.Intersect(old_rect);
	surf->SetClippingRect(new_rect);

	// Iterate all children
	Std::list<Gump *>::reverse_iterator it = _children.rbegin();
	Std::list<Gump *>::reverse_iterator end = _children.rend();

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
	int32 gx = mx, gy = my;
	ParentToGump(gx, gy);
	Gump *gump = nullptr;

	// Iterate all children
	Std::list<Gump *>::reverse_iterator it = _children.rbegin();
	Std::list<Gump *>::reverse_iterator end = _children.rend();

	while (it != end && !gump) {
		Gump *g = *it;
		gump = g->FindGump(gx, gy);
		++it;
	}

	// it's over a child
	if (gump)
		return gump;

	// it's over this gump
	if (PointOnGump(mx, my))
		return this;

	return nullptr;
}

void Gump::setRelativePosition(Gump::Position pos, int xoffset, int yoffset) {
	if (_parent) {
		Rect rect;
		_parent->GetDims(rect);

		switch (pos) {
		case CENTER:
			Move(rect.w / 2 - _dims.w / 2 + xoffset,
			     rect.h / 2 - _dims.h / 2 + yoffset);
			break;
		case TOP_LEFT:
			Move(xoffset, yoffset);
			break;
		case TOP_RIGHT:
			Move(rect.w - _dims.w + xoffset, yoffset);
			break;
		case BOTTOM_LEFT:
			Move(xoffset, rect.h - _dims.h + yoffset);
			break;
		case BOTTOM_RIGHT:
			Move(rect.w - _dims.w + xoffset, rect.h - _dims.h + yoffset);
			break;
		case TOP_CENTER:
			Move(rect.w / 2 - _dims.w / 2 + xoffset, yoffset);
			break;
		case BOTTOM_CENTER:
			Move(rect.w / 2 - _dims.w / 2 + xoffset, rect.h - _dims.h + yoffset);
			break;
		default:
			break;
		}
	}
}

bool Gump::PointOnGump(int mx, int my) {
	int32 gx = mx, gy = my;
	ParentToGump(gx, gy);

	// First check again rectangle
	if (!_dims.InRect(gx, gy)) {
		return false;
	}

	if (!_shape) {
		// no shape? Then if it's in the rectangle it's on the gump.
		return true;
	}

	const ShapeFrame *sf = _shape->getFrame(_frameNum);
	assert(sf);
	if (sf->hasPoint(gx, gy)) {
		return true;
	}

	// reverse-iterate children
	Std::list<Gump *>::reverse_iterator it;
	for (it = _children.rbegin(); it != _children.rend(); ++it) {
		Gump *g = *it;

		// It's got the point
		if (g->PointOnGump(gx, gy)) return true;
	}

	return false;
}

// Convert a screen space point to a gump point
void Gump::ScreenSpaceToGump(int32 &sx, int32 &sy, PointRoundDir r) {
	// This is a recursive operation. We get each
	// parent to convert the point into their local
	// coords.
	if (_parent) _parent->ScreenSpaceToGump(sx, sy, r);

	ParentToGump(sx, sy, r);
}

// Convert a gump point to a screen space point
void Gump::GumpToScreenSpace(int32 &gx, int32 &gy, PointRoundDir r) {
	// This is a recursive operation. We get each
	// gump to convert the point to their parent

	GumpToParent(gx, gy, r);

	if (_parent) _parent->GumpToScreenSpace(gx, gy, r);
}

// Convert a parent relative point to a gump point
void Gump::ParentToGump(int32 &px, int32 &py, PointRoundDir) {
	px -= _x;
	px += _dims.x;
	py -= _y;
	py += _dims.y;
}

// Convert a gump point to parent relative point
void Gump::GumpToParent(int32 &gx, int32 &gy, PointRoundDir) {
	gx -= _dims.x;
	gx += _x;
	gy -= _dims.y;
	gy += _y;
}

// Transform a rectangle to screenspace from gumpspace
void Gump::GumpRectToScreenSpace(int32 &gx, int32 &gy, int32 &gw, int32 &gh,
                                 RectRoundDir r) {
	PointRoundDir tl = (r == ROUND_INSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);
	PointRoundDir br = (r == ROUND_OUTSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);

	int32 x1 = gx, y1 = gy;
	int32 x2 = gx + gw, y2 = gy + gh;
	GumpToScreenSpace(x1, y1, tl);
	GumpToScreenSpace(x2, y2, br);
	gx = x1;
	gy = y1;
	if (gw != 0) gw = x2 - x1;
	if (gh != 0) gh = y2 - y1;
}

// Transform a rectangle to gumpspace from screenspace
void Gump::ScreenSpaceToGumpRect(int32 &sx, int32 &sy, int32 &sw, int32 &sh,
                                 RectRoundDir r) {
	PointRoundDir tl = (r == ROUND_INSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);
	PointRoundDir br = (r == ROUND_OUTSIDE ? ROUND_BOTTOMRIGHT : ROUND_TOPLEFT);

	int32 x1 = sx, y1 = sy;
	int32 x2 = sx + sw, y2 = sy + sh;
	ScreenSpaceToGump(x1, y1, tl);
	ScreenSpaceToGump(x2, y2, br);
	sx = x1;
	sy = y1;
	if (sw != 0) sw = x2 - x1;
	if (sh != 0) sh = y2 - y1;
}

uint16 Gump::TraceObjId(int32 mx, int32 my) {
	// Convert to local coords
	int32 gx = mx, gy = my;
	ParentToGump(gx, gy);

	uint16 objId_ = 0;

	// reverse-iterate children
	Std::list<Gump *>::reverse_iterator it;
	for (it = _children.rbegin(); it != _children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->_flags & FLAG_CLOSING) continue;

		// It's got the point
		if (g->PointOnGump(gx, gy)) objId_ = g->TraceObjId(gx, gy);

		if (objId_ && objId_ != 65535) break;
	}

//	if (!objId_ || objId_ == 65535)
//		if (PointOnGump(mx,my))
//			objId_ = getObjId();

	return objId_;
}

bool Gump::GetLocationOfItem(uint16 itemid, int32 &gx, int32 &gy,
                             int32 lerp_factor) {
	gx = 0;
	gy = 0;
	return false;
}

// Find a child gump that matches the matching function 
Gump *Gump::FindGump(const FindGumpPredicate predicate, bool recursive) {
	if (predicate(this))
		return this;

	// Iterate all children
	Std::list<Gump *>::iterator  it = _children.begin();
	Std::list<Gump *>::iterator  end = _children.end();

	for (; it != end; ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->_flags & FLAG_CLOSING)
			continue;

		if (predicate(g))
			return g;
	}

	if (!recursive)
		return nullptr;

	// Recursive Iterate all children
	it = _children.begin();
	end = _children.end();

	for (; it != end; ++it) {
		Gump *g = (*it);

		// Not if closing
		if (g->_flags & FLAG_CLOSING)
			continue;

		g = g->FindGump(predicate, recursive);

		if (g)
			return g;
	}

	return nullptr;
}

// Makes this gump the focus
void Gump::MakeFocus() {
	// By default we WONT do anything
	if (_parent) {
		if (_parent->_focusChild) _parent->_focusChild->OnFocus(false);
		_parent->_focusChild = this;
	}
	OnFocus(true);
}

void Gump::FindNewFocusChild() {
	if (_focusChild)
		_focusChild->OnFocus(false);
	_focusChild = nullptr;

	// Now add the gump to use as the new focus
	Std::list<Gump *>::reverse_iterator	it = _children.rbegin();

	if (it != _children.rend()) {
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
	Std::list<Gump *>::iterator  it = _children.begin();
	Std::list<Gump *>::iterator  end = _children.end();

	for (; it != end; ++it) {
		Gump *other = *it;

		// Why don't we check for FLAG_CLOSING here?
		// Because we want to make sure that the sort order is always valid

		// If we are same layer as focus and we wont take it, we will not be
		// placed in front of it
		if (!take_focus && other == _focusChild && other->_layer == gump->_layer)
			break;

		// Lower layers get placed before higher layers
		if (other->_layer > gump->_layer) break;
	}

	// Now add it
	_children.insert(it, gump);
	gump->_parent = this;

	// Make the gump the focus if needed
	if (take_focus || !_focusChild) {
		if (_focusChild) _focusChild->OnFocus(false);
		gump->OnFocus(true);
		_focusChild = gump;
	}
}

// Remove a gump from the list
void Gump::RemoveChild(Gump *gump) {
	if (!gump) return;

	// Remove it
	_children.remove(gump);
	gump->_parent = nullptr;

	// Remove focus, the give upper most gump the focus
	if (gump == _focusChild) {
		FindNewFocusChild();
	}
}

void Gump::MoveChildToFront(Gump *gump) {
	if (!gump) return;

	_children.remove(gump);

	Std::list<Gump *>::iterator  it = _children.begin();
	Std::list<Gump *>::iterator  end = _children.end();
	for (; it != end; ++it) {
		Gump *other = *it;

		// Lower layers get placed before higher layers
		if (other->_layer > gump->_layer) break;
	}

	_children.insert(it, gump);
}


Gump *Gump::GetRootGump() {
	if (!_parent) return this;
	return _parent->GetRootGump();
}


bool Gump::StartDraggingChild(Gump *gump, int32 mx, int32 my) {
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

Gump *Gump::onMouseDown(int button, int32 mx, int32 my) {
	// Convert to local coords
	ParentToGump(mx, my);

	Gump *handled = nullptr;

	// Iterate children backwards
	Std::list<Gump *>::reverse_iterator it;
	for (it = _children.rbegin(); it != _children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->_flags & FLAG_CLOSING || g->IsHidden()) continue;

		// It's got the point
		if (g->PointOnGump(mx, my)) handled = g->onMouseDown(button, mx, my);

		if (handled) break;
	}

	return handled;
}

Gump *Gump::onMouseMotion(int32 mx, int32 my) {
	// Convert to local coords
	ParentToGump(mx, my);

	Gump *handled = nullptr;

	// Iterate children backwards
	Std::list<Gump *>::reverse_iterator it;
	for (it = _children.rbegin(); it != _children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->_flags & FLAG_CLOSING || g->IsHidden()) continue;

		// It's got the point
		if (g->PointOnGump(mx, my)) handled = g->onMouseMotion(mx, my);

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
	if (_focusChild) handled = _focusChild->OnKeyDown(key, mod);
	return handled;
}

bool Gump::OnKeyUp(int key) {
	bool handled = false;
	if (_focusChild) handled = _focusChild->OnKeyUp(key);
	return handled;
}

bool Gump::OnTextInput(int unicode) {
	bool handled = false;
	if (_focusChild) handled = _focusChild->OnTextInput(unicode);
	return handled;
}

bool Gump::mustSave(bool toplevel) const {
	// DONT_SAVE flag
	if (_flags & FLAG_DONT_SAVE)
		return false;

	if (toplevel) {
		// don't save gumps with parents, unless parent is a core gump
		if (_parent && !(_parent->_flags & FLAG_CORE_GUMP))
			return false;
	}

	return true;
}

void Gump::saveData(Common::WriteStream *ws) {
	Object::saveData(ws);

	ws->writeUint16LE(_owner);
	ws->writeUint32LE(static_cast<uint32>(_x));
	ws->writeUint32LE(static_cast<uint32>(_y));
	ws->writeUint32LE(static_cast<uint32>(_dims.x));
	ws->writeUint32LE(static_cast<uint32>(_dims.y));
	ws->writeUint32LE(static_cast<uint32>(_dims.w));
	ws->writeUint32LE(static_cast<uint32>(_dims.h));
	ws->writeUint32LE(_flags);
	ws->writeUint32LE(static_cast<uint32>(_layer));
	ws->writeUint32LE(static_cast<uint32>(_index));

	uint16 flex = 0;
	uint32 shapenum = 0;
	if (_shape) {
		_shape->getShapeId(flex, shapenum);
	}
	ws->writeUint16LE(flex);
	ws->writeUint32LE(shapenum);

	ws->writeUint32LE(_frameNum);
	if (_focusChild)
		ws->writeUint16LE(_focusChild->getObjId());
	else
		ws->writeUint16LE(0);
	ws->writeUint16LE(_notifier);
	ws->writeUint32LE(_processResult);

	unsigned int childcount = 0;
	Std::list<Gump *>::iterator it;
	for (it = _children.begin(); it != _children.end(); ++it) {
		if (!(*it)->mustSave(false)) continue;
		childcount++;
	}

	// write children:
	ws->writeUint32LE(childcount);
	for (it = _children.begin(); it != _children.end(); ++it) {
		if (!(*it)->mustSave(false)) continue;

		ObjectManager::get_instance()->saveObject(ws, *it);
	}
}

bool Gump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Object::loadData(rs, version)) return false;

	_owner = rs->readUint16LE();
	_x = static_cast<int32>(rs->readUint32LE());
	_y = static_cast<int32>(rs->readUint32LE());

	int dx = static_cast<int32>(rs->readUint32LE());
	int dy = static_cast<int32>(rs->readUint32LE());
	int dw = static_cast<int32>(rs->readUint32LE());
	int dh = static_cast<int32>(rs->readUint32LE());
	_dims.Set(dx, dy, dw, dh);

	_flags = rs->readUint32LE();
	_layer = static_cast<int32>(rs->readUint32LE());
	_index = static_cast<int32>(rs->readUint32LE());

	_shape = nullptr;
	ShapeArchive *flex = GameData::get_instance()->getShapeFlex(rs->readUint16LE());
	uint32 shapenum = rs->readUint32LE();
	if (flex) {
		_shape = flex->getShape(shapenum);
	}

	_frameNum = rs->readUint32LE();
	uint16 focusid = rs->readUint16LE();
	_focusChild = nullptr;
	_notifier = rs->readUint16LE();
	_processResult = rs->readUint32LE();

	// read children
	uint32 childcount = rs->readUint32LE();
	for (unsigned int i = 0; i < childcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(rs, version);
		Gump *child = dynamic_cast<Gump *>(obj);
		if (!child) return false;

		AddChild(child, false);

		if (child->getObjId() == focusid)
			_focusChild = child;

	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
