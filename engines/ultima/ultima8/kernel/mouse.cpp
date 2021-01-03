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

#include "graphics/cursorman.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/main_actor.h"

namespace Ultima {
namespace Ultima8 {

Mouse *Mouse::_instance = nullptr;

Mouse::Mouse() : _flashingCursorTime(0), _mouseOverGump(0),
		_dragging(DRAG_NOT), _dragging_objId(0), _draggingItem_startGump(0),
		_draggingItem_lastGump(0) {
	_instance = this;
}

Mouse::~Mouse() {
	_instance = nullptr;
}

void Mouse::setup() {
	pushMouseCursor();
}

bool Mouse::buttonDown(Shared::MouseButton button) {
	assert(button != Shared::MOUSE_LAST);
	bool handled = false;
	uint32 now = g_system->getMillis();

	MButton &mbutton = _mouseButton[button];

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	Gump *mousedowngump = desktopGump->onMouseDown(button, _mousePos.x, _mousePos.y);
	if (mousedowngump) {
		mbutton._downGump = mousedowngump->getObjId();
		handled = true;
	} else {
		mbutton._downGump = 0;
	}

	mbutton._curDown = now;
	mbutton._downPoint = _mousePos;
	mbutton.setState(MBS_DOWN);
	mbutton.clearState(MBS_HANDLED);

	if (mbutton.isUnhandledDoubleClick()) {
		if (_dragging == Mouse::DRAG_NOT) {
			Gump *gump = getGump(mbutton._downGump);
			if (gump) {
				int32 mx2 = _mousePos.x, my2 = _mousePos.y;
				Gump *parent = gump->GetParent();
				if (parent) parent->ScreenSpaceToGump(mx2, my2);
				gump->onMouseDouble(button, mx2, my2);
			}
			mbutton.setState(MBS_HANDLED);
			mbutton._lastDown = 0;
		}
	}
	mbutton._lastDown = now;

	return handled;
}

bool Mouse::buttonUp(Shared::MouseButton button) {
	assert(button != Shared::MOUSE_LAST);
	bool handled = false;

	_mouseButton[button].clearState(MBS_DOWN);

	// Need to store the last down position of the mouse
	// when the button is released.
	_mouseButton[button]._downPoint = _mousePos;

	// Always send mouse up to the gump
	Gump *gump = getGump(_mouseButton[button]._downGump);
	if (gump) {
		int32 mx2 = _mousePos.x, my2 = _mousePos.y;
		Gump *parent = gump->GetParent();
		if (parent)
			parent->ScreenSpaceToGump(mx2, my2);
		gump->onMouseUp(button, mx2, my2);
		handled = true;
	}

	if (button == Shared::BUTTON_LEFT && _dragging != Mouse::DRAG_NOT) {
		stopDragging(_mousePos.x, _mousePos.y);
		handled = true;
	}

	return handled;
}

void Mouse::popAllCursors() {
	_cursors.clear();
	CursorMan.popAllCursors();
}

bool Mouse::isMouseDownEvent(Shared::MouseButton button) const {
	return _mouseButton[button].isState(MBS_DOWN);
}

int Mouse::getMouseLength(int mx, int my) const {
	Rect dims;
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	screen->GetSurfaceDims(dims);

	// For now, reference point is (near) the center of the screen
	int dx = abs(mx - dims.width() / 2);
	int dy = abs((dims.height() / 2 + (dims.height() * 14 / 200)) - my); //! constant

	//
	// The original game switches cursors from small -> medium -> large on
	// rectangles - in x, ~30px and ~130px away from the avatar (center) on
	// the 320px screen, and approximately the same proportions in y.
	//
	// Modern players may be in a window so give them a little bit more
	// space to make the large cursor without having to hit the edge.
	//
	int xshort = (dims.width() * 30 / 320);
	int xmed = (dims.width() * 100 / 320);
	int yshort = (dims.height() * 30 / 320);
	int ymed = (dims.height() * 100 / 320);

	// determine length of arrow
	if (dx > xmed || dy > ymed) {
		return 2;
	} else if (dx > xshort || dy > yshort) {
		return 1;
	} else {
		return 0;
	}
}

Direction Mouse::getMouseDirectionWorld(int mx, int my) const {
	Rect dims;
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	screen->GetSurfaceDims(dims);

	// For now, reference point is (near) the center of the screen
	int dx = mx - dims.width() / 2;
	int dy = (dims.height() / 2 + (dims.height() * 14 / 200)) - my; //! constant

	return Direction_Get(dy * 2, dx, dirmode_8dirs);
}

Direction Mouse::getMouseDirectionScreen(int mx, int my) const {
	return Direction_OneRight(getMouseDirectionWorld(mx, my), dirmode_8dirs);
}

int Mouse::getMouseFrame() {
	// Ultima 8 mouse cursors:

	// 0-7 = short (0 = up, 1 = up-right, 2 = right, ...)
	// 8-15 = medium
	// 16-23 = long
	// 24 = blue dot
	// 25-32 = combat
	// 33 = red dot
	// 34 = target
	// 35 = pentagram
	// 36 = skeletal hand
	// 38 = quill
	// 39 = magnifying glass
	// 40 = red cross

	MouseCursor cursor = _cursors.top();

	if (_flashingCursorTime > 0) {
		if (g_system->getMillis() < _flashingCursorTime + 250)
			cursor = MOUSE_CROSS;
		else
			_flashingCursorTime = 0;
	}


	switch (cursor) {
	case MOUSE_NORMAL: {
		bool combat = false;
		MainActor *av = getMainActor();
		if (av) {
			combat = av->isInCombat();
		}

		// Calculate frame based on direction
		Direction mousedir = getMouseDirectionScreen();
		int frame = mouseFrameForDir(mousedir);

		/** length --- frame offset
		 *    0              0
		 *    1              8
		 *    2             16
		 *  combat          25
		 **/
		int offset = getMouseLength() * 8;
		if (combat && offset != 16) //combat mouse is off if running
			offset = 25;
		return frame + offset;
	}
					 //!! constants...
	case MOUSE_NONE:
		return -1;
	case MOUSE_POINTER:
		return -2;
	case MOUSE_TARGET:
		return 34;
	case MOUSE_PENTAGRAM:
		return 35;
	case MOUSE_HAND:
		return 36;
	case MOUSE_QUILL:
		return 38;
	case MOUSE_MAGGLASS:
		return 39;
	case MOUSE_CROSS:
		return 40;
	default:
		return -1;
	}
}

int Mouse::mouseFrameForDir(Direction mousedir) const {
	switch (mousedir) {
		case dir_north:		return 0;
		case dir_northeast: return 1;
		case dir_east:		return 2;
		case dir_southeast: return 3;
		case dir_south:		return 4;
		case dir_southwest: return 5;
		case dir_west:		return 6;
		case dir_northwest: return 7;
		default:			return 0;
	}
}

void Mouse::setMouseCoords(int mx, int my) {
	Rect dims;
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	screen->GetSurfaceDims(dims);

	if (mx < dims.left)
		mx = dims.left;
	else if (mx > dims.width())
		mx = dims.width();

	if (my < dims.top)
		my = dims.top;
	else if (my > dims.height())
		my = dims.height();

	_mousePos.x = mx;
	_mousePos.y = my;

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	Gump *gump = desktopGump->onMouseMotion(mx, my);
	if (gump && _mouseOverGump != gump->getObjId()) {
		Gump *oldGump = getGump(_mouseOverGump);
		Std::list<Gump *> oldgumplist;
		Std::list<Gump *> newgumplist;

		// create lists of parents of old and new 'mouseover' gumps
		if (oldGump) {
			while (oldGump) {
				oldgumplist.push_front(oldGump);
				oldGump = oldGump->GetParent();
			}
		}
		Gump *newGump = gump;
		while (newGump) {
			newgumplist.push_front(newGump);
			newGump = newGump->GetParent();
		}

		Std::list<Gump *>::iterator olditer = oldgumplist.begin();
		Std::list<Gump *>::iterator newiter = newgumplist.begin();

		// strip common prefix from lists
		while (olditer != oldgumplist.end() &&
			newiter != newgumplist.end() &&
			*olditer == *newiter) {
			++olditer;
			++newiter;
		}

		// send events to remaining gumps
		for (; olditer != oldgumplist.end(); ++olditer)
			(*olditer)->onMouseLeft();

		_mouseOverGump = gump->getObjId();

		for (; newiter != newgumplist.end(); ++newiter)
			(*newiter)->onMouseOver();
	}

	if (_dragging == DRAG_NOT) {
		if (_mouseButton[Shared::BUTTON_LEFT].isState(MBS_DOWN)) {
			int startx = _mouseButton[Shared::BUTTON_LEFT]._downPoint.x;
			int starty = _mouseButton[Shared::BUTTON_LEFT]._downPoint.y;
			if (ABS(startx - mx) > 2 ||
				ABS(starty - my) > 2) {
				startDragging(startx, starty);
			}
		}
	}

	if (_dragging == DRAG_OK || _dragging == DRAG_TEMPFAIL) {
		moveDragging(mx, my);
	}
}

void Mouse::setMouseCursor(MouseCursor cursor) {
	_cursors.pop();
	_cursors.push(cursor);
}

void Mouse::flashCrossCursor() {
	_flashingCursorTime = g_system->getMillis();
}

void Mouse::pushMouseCursor() {
	_cursors.push(MOUSE_NORMAL);
}

void Mouse::popMouseCursor() {
	_cursors.pop();
}

void Mouse::startDragging(int startx, int starty) {
	setDraggingOffset(0, 0); // initialize

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	_dragging_objId = desktopGump->TraceObjId(startx, starty);

	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);

	// for a Gump, notify the Gump's parent that we started _dragging:
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int32 px = startx, py = starty;
		parent->ScreenSpaceToGump(px, py);
		if (gump->IsDraggable() && parent->StartDraggingChild(gump, px, py))
			_dragging = DRAG_OK;
		else {
			_dragging_objId = 0;
			return;
		}
	} else if (item) {
		// for an Item, notify the gump the item is in that we started _dragging

		// find gump item was in
		gump = desktopGump->FindGump(startx, starty);
		int32 gx = startx, gy = starty;
		gump->ScreenSpaceToGump(gx, gy);
		bool ok = !Ultima8Engine::get_instance()->isAvatarInStasis() &&
			gump->StartDraggingItem(item, gx, gy);
		if (!ok) {
			_dragging = DRAG_INVALID;
		} else {
			_dragging = DRAG_OK;

			// this is the gump that'll get StopDraggingItem
			_draggingItem_startGump = gump->getObjId();

			// this is the gump the item is currently over
			_draggingItem_lastGump = gump->getObjId();
		}
	} else {
		_dragging = DRAG_INVALID;
	}

#if 0
	Object *obj = ObjectManager::get_instance()->getObject(_dragging_objId);
	perr << "Dragging object " << _dragging_objId << " (class=" << (obj ? obj->GetClassType().class_name : "NULL") << ")" << Std::endl;
#endif

	pushMouseCursor();
	setMouseCursor(MOUSE_NORMAL);

	// pause the kernel
	Kernel::get_instance()->pause();

	_mouseButton[Shared::BUTTON_LEFT].setState(MBS_HANDLED);

	if (_dragging == DRAG_INVALID) {
		setMouseCursor(MOUSE_CROSS);
	}
}

void Mouse::moveDragging(int mx, int my) {
	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);

	setMouseCursor(MOUSE_NORMAL);

	// for a gump, notify Gump's parent that it was dragged
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int32 px = mx, py = my;
		parent->ScreenSpaceToGump(px, py);
		parent->DraggingChild(gump, px, py);
	} else {
		// for an item, notify the gump it's on
		if (item) {
			Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
			gump = desktopGump->FindGump(mx, my);
			assert(gump);

			if (gump->getObjId() != _draggingItem_lastGump) {
				// item switched gump, so notify previous gump item left
				Gump *last = getGump(_draggingItem_lastGump);
				if (last) last->DraggingItemLeftGump(item);
			}
			_draggingItem_lastGump = gump->getObjId();
			int32 gx = mx, gy = my;
			gump->ScreenSpaceToGump(gx, gy);
			bool ok = gump->DraggingItem(item, gx, gy);
			if (!ok) {
				_dragging = DRAG_TEMPFAIL;
			} else {
				_dragging = DRAG_OK;
			}
		} else {
			CANT_HAPPEN();
		}
	}

	if (_dragging == DRAG_TEMPFAIL) {
		setMouseCursor(MOUSE_CROSS);
	}
}


void Mouse::stopDragging(int mx, int my) {
	//	perr << "Dropping object " << _dragging_objId << Std::endl;

	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);
	// for a Gump: notify parent
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		parent->StopDraggingChild(gump);
	} else if (item) {
		// for an item: notify gumps
		if (_dragging != DRAG_INVALID) {
			Gump *startgump = getGump(_draggingItem_startGump);
			assert(startgump); // can't have disappeared
			bool moved = (_dragging == DRAG_OK);

			if (_dragging != DRAG_OK) {
				Gump *last = getGump(_draggingItem_lastGump);
				if (last && last != startgump)
					last->DraggingItemLeftGump(item);
			}

			startgump->StopDraggingItem(item, moved);
		}

		if (_dragging == DRAG_OK) {
			item->movedByPlayer();

			Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
			gump = desktopGump->FindGump(mx, my);
			int32 gx = mx, gy = my;
			gump->ScreenSpaceToGump(gx, gy);
			gump->DropItem(item, gx, gy);
		}
	} else {
		assert(_dragging == DRAG_INVALID);
	}

	_dragging = DRAG_NOT;

	Kernel::get_instance()->unpause();

	popMouseCursor();
}

void Mouse::handleDelayedEvents() {
	for (int button = 0; button < Shared::MOUSE_LAST; ++button) {
		if (!(_mouseButton[button]._state & (MBS_HANDLED | MBS_DOWN)) &&
			!_mouseButton[button].lastWithinDblClkTimeout()) {
			Gump *gump = getGump(_mouseButton[button]._downGump);
			if (gump) {
				int32 mx = _mouseButton[button]._downPoint.x;
				int32 my = _mouseButton[button]._downPoint.y;
				Gump *parent = gump->GetParent();
				if (parent)
					parent->ScreenSpaceToGump(mx, my);
				
				gump->onMouseClick(button, mx, my);
			}

			_mouseButton[button]._downGump = 0;
			_mouseButton[button].setState(MBS_HANDLED);
		}
	}
}

Gump *Mouse::getMouseOverGump() const {
	return getGump(_mouseOverGump);
}

void Mouse::paint() {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	GameData *gamedata = GameData::get_instance();

	if (!gamedata)
		return;

	const Shape *mouse = gamedata->getMouse();
	if (mouse) {
		int frame = getMouseFrame();
		if (frame >= 0) {
			screen->Paint(mouse, frame, _mousePos.x, _mousePos.y, true);
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
