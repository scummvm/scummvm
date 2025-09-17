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

#include "common/config-manager.h"
#include "graphics/cursorman.h"
#include "ultima/ultima.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/palette.h"

namespace Ultima {
namespace Ultima8 {

Mouse *Mouse::_instance = nullptr;

Mouse::Mouse() : _lastMouseFrame(-1), _flashingCursorTime(0), _mouseOverGump(0),
		_dragging(DRAG_NOT), _dragging_objId(0), _draggingItem_startGump(0),
		_draggingItem_lastGump(0) {
	_instance = this;

	_cursors.push(MOUSE_NONE);
	CursorMan.showMouse(false);

	// The original game switches cursors from small -> medium -> large on
	// rectangles - in x, ~30px and ~130px away from the avatar (center) on
	// the 320px screen, and approximately the same proportions in y.
	//
	// These cursors correspond to the player movement of step -> walk -> run.
	// 
	// Modern players may be in a window so give them a little bit more
	// space to make the large cursor without having to hit the edge.

	// Walk & run threshold range of 0-255
	ConfMan.registerDefault("walk_threshold", 50);
	ConfMan.registerDefault("run_threshold", 160);

	_walkThreshold = CLIP<int>(ConfMan.getInt("walk_threshold"), 0, 255);
	_runThreshold = CLIP<int>(ConfMan.getInt("run_threshold"), 0, 255);
}

Mouse::~Mouse() {
	_instance = nullptr;
}

bool Mouse::buttonDown(MouseButton button) {
	assert(button != MOUSE_LAST);
	bool handled = false;

	MButton &mbutton = _mouseButton[button];

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	Gump *mousedowngump = desktopGump->onMouseDown(button, _mousePos.x, _mousePos.y);
	if (mousedowngump) {
		mbutton._downGump = mousedowngump->getObjId();
		handled = true;
	} else {
		mbutton._downGump = 0;
	}

	mbutton._lastDown = mbutton._curDown;
	mbutton._curDown = g_system->getMillis();
	mbutton._downPoint = _mousePos;
	mbutton.setState(MBS_DOWN);
	mbutton.clearState(MBS_HANDLED);

	uint32 timeout = getDoubleClickTime();
	if (mbutton.isUnhandledDoubleClick(timeout)) {
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

	return handled;
}

bool Mouse::buttonUp(MouseButton button) {
	assert(button != MOUSE_LAST);
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

	if (button == BUTTON_LEFT && _dragging != Mouse::DRAG_NOT) {
		stopDragging(_mousePos.x, _mousePos.y);
		handled = true;
	}

	return handled;
}

void Mouse::popAllCursors() {
	_cursors.clear();
	_cursors.push(MOUSE_NONE);
	update();
}

bool Mouse::isMouseDownEvent(MouseButton button) const {
	return _mouseButton[button].isState(MBS_DOWN);
}

int Mouse::getMouseLength(int mx, int my) const {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		if (proc->hasMovementFlags(AvatarMoverProcess::MOVE_STEP))
			return 0;
		if (proc->hasMovementFlags(AvatarMoverProcess::MOVE_RUN))
			return 2;
	}

	RenderSurface *screen = engine->getRenderScreen();
	Common::Rect32 dims = screen->getSurfaceDims();

	// Reference point is the center of the screen
	int dx = abs(mx - dims.width() / 2);
	int dy = abs((dims.height() / 2) - my);
	int xmed = dims.width() / 2 * _runThreshold / 255;
	int ymed = dims.height() / 2 * _runThreshold / 255;

	if (dx > xmed || dy > ymed)
		return 2;

	// For short cursor, reference point is near the avatar's feet
	dy = abs((dims.height() / 2 + 14) - my); //! constant
	int xshort = dims.width() / 2 * _walkThreshold / 255;
	int yshort = dims.height() / 2 * _walkThreshold / 255;

	if (dx > xshort || dy > yshort)
		return 1;

	return 0;
}

Direction Mouse::getMouseDirectionWorld(int mx, int my) const {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	Common::Rect32 dims = screen->getSurfaceDims();

	// For now, reference point is (near) the center of the screen
	int dx = mx - dims.width() / 2;
	int dy = (dims.height() / 2 + 14) - my; //! constant

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
	if (_cursors.empty())
		return -1;

	MouseCursor cursor = _cursors.top();

	if (_flashingCursorTime > 0) {
		if (g_system->getMillis() < _flashingCursorTime + 250)
			cursor = MOUSE_CROSS;
		else
			_flashingCursorTime = 0;
	}


	switch (cursor) {
	case MOUSE_NORMAL: {
		if (GAME_IS_CRUSADER)
			return -1;

		bool combat = false;
		bool combatRun = false;
		const MainActor *av = getMainActor();
		if (av) {
			combat = av->isInCombat();
			combatRun = av->hasActorFlags(Actor::ACT_COMBATRUN);
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
		int offset = 25;
		if (!combat || combatRun) //combat mouse is off if running
			offset = getMouseLength() * 8;
		return frame + offset;
	}

	//!! constants...
	case MOUSE_NONE:
		return -1;
	case MOUSE_TARGET:
		return 34;
	case MOUSE_WAIT:
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
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	Common::Rect32 dims = screen->getSurfaceDims();

	if (mx < dims.left)
		mx = dims.left;
	else if (mx >= dims.right)
		mx = dims.right - 1;

	if (my < dims.top)
		my = dims.top;
	else if (my >= dims.bottom)
		my = dims.bottom - 1;

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
		if (_mouseButton[BUTTON_LEFT].isState(MBS_DOWN)) {
			int startx = _mouseButton[BUTTON_LEFT]._downPoint.x;
			int starty = _mouseButton[BUTTON_LEFT]._downPoint.y;
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
	update();
}

void Mouse::flashCrossCursor() {
	_flashingCursorTime = g_system->getMillis();
}

void Mouse::pushMouseCursor(MouseCursor cursor) {
	_cursors.push(cursor);
	update();
}

void Mouse::popMouseCursor() {
	_cursors.pop();
	update();
}

void Mouse::startDragging(int startx, int starty) {
	setDraggingOffset(0, 0); // initialize

	Gump *desktopGump = Ultima8Engine::get_instance()->getDesktopGump();
	_dragging_objId = desktopGump->TraceObjId(startx, starty);

	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);

	// for a Gump, notify the Gump's parent that we started _dragging:
	if (gump) {
		debugC(kDebugObject, "Dragging gump %u (class=%s)", _dragging_objId, gump->GetClassType()._className);

		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int32 px = startx, py = starty;
		parent->ScreenSpaceToGump(px, py);
		if (gump->IsDraggable() && gump->onDragStart(px, py))
			_dragging = DRAG_OK;
		else {
			_dragging_objId = 0;
			return;
		}
	} else if (item) {
		// for an Item, notify the gump the item is in that we started _dragging
		debugC(kDebugObject, "Dragging item %u (class=%s)", _dragging_objId, item->GetClassType()._className);

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

	pushMouseCursor(MOUSE_NORMAL);

	// pause the kernel
	Kernel::get_instance()->pause();

	_mouseButton[BUTTON_LEFT].setState(MBS_HANDLED);

	if (_dragging == DRAG_INVALID) {
		setMouseCursor(MOUSE_CROSS);
	}
}

void Mouse::moveDragging(int mx, int my) {
	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);

	setMouseCursor(MOUSE_NORMAL);

	if (gump) {
		// for a gump, notify Gump's parent that it was dragged
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int32 px = mx, py = my;
		parent->ScreenSpaceToGump(px, py);
		gump->onDrag(px, py);
	} else if (item) {
		// for an item, notify the gump it's on
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
		warning("Unknown object id on mouse drag");
	}

	if (_dragging == DRAG_TEMPFAIL) {
		setMouseCursor(MOUSE_CROSS);
	}
}


void Mouse::stopDragging(int mx, int my) {
	debugC(kDebugObject, "Dropping object %u", _dragging_objId);

	Gump *gump = getGump(_dragging_objId);
	Item *item = getItem(_dragging_objId);
	// for a Gump: notify parent
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int32 px = mx, py = my;
		parent->ScreenSpaceToGump(px, py);
		gump->onDragStop(px, py);
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

uint32 Mouse::getDoubleClickTime() const {
	uint32 timeout = g_system->getDoubleClickTime();
	return timeout > 0 ? timeout : 400;
}

void Mouse::handleDelayedEvents() {
	uint32 now = g_system->getMillis();
	uint32 timeout = getDoubleClickTime();

	for (int button = 0; button < MOUSE_LAST; ++button) {
		if (!_mouseButton[button].isState(MBS_DOWN) &&
			_mouseButton[button].isUnhandledPastTimeout(now, timeout)) {
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

void Mouse::update() {
	GameData *gamedata = GameData::get_instance();
	if (!gamedata)
		return;

	const Shape *mouse = gamedata->getMouse();
	if (mouse) {
		int frame = getMouseFrame();
		if (frame != _lastMouseFrame) {
			_lastMouseFrame = frame;

			if (frame >= 0 && (uint)frame < mouse->frameCount()) {
				const ShapeFrame *f = mouse->getFrame(frame);
				CursorMan.replaceCursor(f->getSurface(), f->_xoff, f->_yoff, f->_keycolor);
				CursorMan.replaceCursorPalette(mouse->getPalette()->data(), 0, 256);
				CursorMan.showMouse(true);
			} else {
				CursorMan.showMouse(false);
			}
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
