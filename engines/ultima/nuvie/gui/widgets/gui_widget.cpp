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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"

namespace Ultima {
namespace Nuvie {

/* Widget constructors */
GUI_Widget:: GUI_Widget(void *data) {
	Init(data, 0, 0, 0, 0);
}
GUI_Widget:: GUI_Widget(void *data, int x, int y, int w, int h) {
	Init(data, x, y, w, h);
}

GUI_Widget::~GUI_Widget() {
	for (; !children.empty();) {
		GUI_Widget *child = children.front();

		children.pop_front();
		delete child;
	}

	return;
}

void GUI_Widget::Init(void *data, int x, int y, int w, int h) {
	focused = false;
	gui_drag_manager = NULL; //set from placeOnScreen method
	widget_data = data;
	screen = NULL;
	surface = NULL;
	SetRect(0, 0, w, h);
	offset_x = x;
	offset_y = y;
	Show();
	error = NULL;
	for (int n = 0; n < 3; ++n) {
		pressed[n] = 0;
	}
	parent = NULL;

	update_display = true;
	set_accept_mouseclick(false); // initializes mouseclick time; SB-X
	delayed_button = Shared::BUTTON_NONE; // optional mouseclick-delay; SB-X
	held_button = Shared::BUTTON_NONE; // optional mousedown-delay; SB-X
	mouse_moved = false;

	int mx = 0, my = 0;
	if (screen)
		screen->get_mouse_location(&mx, &my);
	mouse_over = HitRect(mx, my);
}

int GUI_Widget::AddWidget(GUI_Widget *widget) {
	children.push_back(widget);
	widget->setParent(this);

	return 0; //success.
}


/* Mark the widget as visible -- this is the default state */
void GUI_Widget::Show(void) {
	status = WIDGET_VISIBLE;
}

/* Mark the widget as hidden;  no display, no events */
void GUI_Widget::Hide(void) {
	if (has_focus()) {
		release_focus();
	}
	status = WIDGET_HIDDEN;
}

/* Mark the widget as free, so it will be deleted by the GUI */
void GUI_Widget:: Delete(void) {
	status = WIDGET_DELETED;
}

void GUI_Widget::MoveRelative(int dx, int dy) {
	Std::list<GUI_Widget *>::iterator child;

	area.translate(dx, dy);

	for (child = children.begin(); child != children.end(); child++)
		(*child)->MoveRelative(dx, dy);

	return;
}

void GUI_Widget::Move(int32 new_x, int32 new_y) {
	Std::list<GUI_Widget *>::iterator child;

	area.moveTo(new_x + offset_x, new_y + offset_y);

	for (child = children.begin(); child != children.end(); child++)
		(*child)->Move(area.left, area.top);

	return;
}

void GUI_Widget::MoveRelativeToParent(int dx, int dy) {
	Std::list<GUI_Widget *>::iterator child;

	area.left = (area.left - offset_x) + dx;
	area.top = (area.top - offset_y) + dy;

	offset_x = dx;
	offset_y = dy;

	for (child = children.begin(); child != children.end(); child++)
		(*child)->Move(area.left, area.top);

	return;
}

void GUI_Widget::grab_focus() {
	if (GUI::get_gui()->set_focus(this))
		focused = true;
}

void GUI_Widget::release_focus() {
	GUI::get_gui()->clear_focus();
	focused = false;
}

void GUI_Widget::moveToFront() {
	GUI *gui = GUI::get_gui();
	if (gui) {
		gui->removeWidget(this);
		gui->AddWidget(this);
	}
}

void GUI_Widget::PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) {
	Std::list<GUI_Widget *>::iterator child;

	if (screen != NULL)
		return;

	area.moveTo(x + offset_x, y + offset_y);

	gui_drag_manager = dm;

	SetDisplay(s);

	/* place our children relative to ourself */
	for (child = children.begin(); child != children.end(); child++)
		(*child)->PlaceOnScreen(screen, dm, area.left, area.top);
	return;
}

/* Report status to GUI */
int GUI_Widget:: Status(void) {
	return (status);
}

/* Set the bounds of the widget.
   If 'w' or 'h' is -1, that parameter will not be changed.
 */
void GUI_Widget:: SetRect(int x, int y, int w, int h) {
	area = Common::Rect(x, y, x + w, y + h);
}

void GUI_Widget:: SetRect(Common::Rect **bounds) {
	int minx, maxx;
	int miny, maxy;
	int i, v;

	maxx = 0;
	maxy = 0;
	for (i = 0; bounds[i]; ++i) {
		v = (bounds[i]->right - 1);
		if (maxx < v) {
			maxx = v;
		}
		v = (bounds[i]->bottom - 1);
		if (maxy < v) {
			maxy = v;
		}
	}
	minx = maxx;
	miny = maxy;
	for (i = 0; bounds[i]; ++i) {
		v = bounds[i]->left;
		if (minx > v) {
			minx = v;
		}
		v = bounds[i]->top;
		if (miny > v) {
			miny = v;
		}
	}
	SetRect(minx, miny, (maxx - minx + 1), (maxy - miny + 1));
}

/* Check to see if a point intersects the bounds of the widget.
 */
int GUI_Widget::HitRect(int x, int y) {
	return (HitRect(x, y, area));
}

int GUI_Widget::HitRect(int x, int y, const Common::Rect &rect) {
	int hit;

	hit = 1;
	if ((x < rect.left) || (x >= rect.right) ||
	        (y < rect.top) || (y >= rect.bottom)) {
		hit = 0;
	}
	return (hit);
}

/* Set the display surface for this widget */
void GUI_Widget::SetDisplay(Screen *s) {
	screen = s;
	surface = screen->get_sdl_surface();
}

void GUI_Widget::setParent(GUI_Widget *widget) {
	parent = widget;
}

/* Show the widget.
   If the surface needs to be locked, it will be locked
   before this call, and unlocked after it returns.

****************NO, NOT AT ALL IF I'M NOT TOO DUMB TO LOOK******
******OTHERWISE YOU COULDN'T FILLRECT in Display(), ETC!!!! ***********
 */
void GUI_Widget::Display(bool full_redraw) {
	DisplayChildren(full_redraw);
}

void GUI_Widget::DisplayChildren(bool full_redraw) {
	if (update_display)
		full_redraw = true;

	if (children.empty() == false) {
		Std::list<GUI_Widget *>::iterator child;

		/* display our children */
		for (child = children.begin(); child != children.end(); child++) {
			if ((*child)->Status() == WIDGET_VISIBLE)
				(*child)->Display(full_redraw);
		}
	}

	return;
}

/* Redraw the widget and only the widget */
void GUI_Widget::Redraw(void) {

	if (status == WIDGET_VISIBLE) {
		update_display = true;
		if (parent != NULL)
			parent->Redraw();
		//Display();
		//SDL_UpdateRects(screen,1,&area);
	}
}

/* GUI idle function -- run when no events pending */
// Idle and HandleEvent produce delayed clicks. Don't override if using those. -- SB-X
GUI_status GUI_Widget::Idle(void) {
	if (children.empty() == false) {
		Std::list<GUI_Widget *>::iterator child;
		/* idle our children */
		for (child = children.begin(); child != children.end(); child++) {
			GUI_status idleStatus = (*child)->Idle();
			if (idleStatus != GUI_PASS)
				return (idleStatus);
		}
	}
	if (delayed_button != 0 || held_button != 0)
		return (try_mouse_delayed());
	return (GUI_PASS);
}

/* Widget event handlers.
   These functions should return a status telling the GUI whether
   or not the event should be passed on to other widgets.
   These are called by the default HandleEvent function.
*/
GUI_status GUI_Widget::KeyDown(const Common::KeyState &key) {
	return (GUI_PASS);
}

GUI_status GUI_Widget::KeyUp(Common::KeyState key) {
	return (GUI_PASS);
}

GUI_status GUI_Widget::MouseDown(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}

GUI_status GUI_Widget::MouseUp(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}


GUI_status GUI_Widget::MouseMotion(int x, int y, uint8 state) {
	return (GUI_PASS);
}

GUI_status GUI_Widget::MouseWheel(sint32 x, sint32 y) {
	return (GUI_PASS);
}

/* Main event handler function.
   This function gets raw SDL events from the GUI.
 */
// Idle and HandleEvent produce delayed clicks. Don't override if using those. -- SB-X
GUI_status GUI_Widget::HandleEvent(const Common::Event *event) {
	if (status == WIDGET_HIDDEN) //we don't care for events if we are hidden.
		return GUI_PASS;

	if (children.empty() == false) {
		Std::list<GUI_Widget *>::iterator child;

		/* handle our children */
		for (child = children.begin(); child != children.end(); child++) {
			GUI_status status_ = (*child)->HandleEvent(event);
			if (status_ != GUI_PASS)
				return status_;
		}
	}

	if (delayed_button != 0 || held_button != 0) {
		GUI_status status_ = try_mouse_delayed();
		if (status_ != GUI_PASS)
			return status_;
	}

	switch (event->type) {
	case Common::EVENT_KEYDOWN:
		return (KeyDown(event->kbd.keycode));
		break;
	case Common::EVENT_KEYUP:
		return (KeyUp(event->kbd.keycode));
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN: {
		int x, y;
		Shared::MouseButton button;
		x = event->mouse.x;
		y = event->mouse.y;
		button = Shared::whichButton(event->type);
		if (focused || HitRect(x, y)) {
			set_mousedown(SDL_GetTicks(), button);
 
			if (button == Common::EVENT_WHEELUP)
				return MouseWheel(0, 1);
			else if (button == Common::EVENT_WHEELDOWN)
				return MouseWheel(0, -1);

			return(MouseDown(x, y, button));
		}
		break;
	}
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP: {
		int x, y;
		Shared::MouseButton button;
		x = event->mouse.x;
		y = event->mouse.y;
		button = Shared::whichButton(event->type);
		if (focused || HitRect(x, y))  {
			int rel_time = SDL_GetTicks();
			int last_rel_time = get_mouseup(button);
			bool do_mouseclick = get_mousedown(button);
			set_mouseup(rel_time, button);
			if (do_mouseclick && accept_mouseclick[button - 1] && (rel_time - last_rel_time < GUI::mouseclick_delay)) {
				// before a Double or Delayed click, mouseup_time is reset so another click isn't possible
				set_mouseup(0, button);
				return (MouseDouble(x, y, button));
			} else if (do_mouseclick && accept_mouseclick[button - 1])
				return (MouseClick(x, y, button));
			else
				return (MouseUp(x, y, button));
		}
		/* if widget was clicked before we must let it deactivate itself*/
		else if (ClickState(1)) {
			set_mouseup(0, button);
			return (MouseUp(-1, -1, button));
		}
		break;
	}

	case Common::EVENT_MOUSEMOVE: {
		int x, y;
		uint8 state;
		x = event->mouse.x;
		y = event->mouse.y;
		state = Events::get()->getButtonState();
		if (state > 0) // mousemotion resets Click
			mouse_moved = true;
		if (focused || HitRect(x, y)) {
			if (!mouse_over) {
				mouse_over = true;
				MouseEnter(state);
			}
			return (MouseMotion(x, y, state));
		} else {
			if (mouse_over) {
				mouse_over = false;
				MouseLeave(state);
			}
			/* if widget was clicked before we must let it react*/
			if (ClickState(1)) return (MouseMotion(-1, -1, state));
		}
	}
	break;

	case Common::EVENT_WHEELUP:
		return MouseWheel(0, 1);

	case Common::EVENT_WHEELDOWN:
		return MouseWheel(0, -1);

	default: {
		/* Pass it along.. */;
	}
	break;
	}
	return (GUI_PASS);
}

// iterate through children if present to hit the correct drag area.
bool GUI_Widget::drag_accept_drop(int x, int y, int message, void *data) {
	if (children.empty() == false) {
		Std::list<GUI_Widget *>::iterator child;

		for (child = children.begin(); child != children.end(); child++) {
			if ((*child)->HitRect(x, y)) {
				if ((*child)->drag_accept_drop(x, y, message, data))
					return true;
			}
		}
	}

	GUI::get_gui()->force_full_redraw();
	return false;
}

void GUI_Widget::drag_perform_drop(int x, int y, int message, void *data) {
	if (children.empty() == false) {
		Std::list<GUI_Widget *>::iterator child;

		for (child = children.begin(); child != children.end(); child++) {
			if ((*child)->HitRect(x, y)) {
				(*child)->drag_perform_drop(x, y, message, data);
				break;
			}
		}
	}

	return;
}

/* Mouse button was pressed and released over the widget.
 */
GUI_status GUI_Widget::MouseClick(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}

/* Mouse button was clicked twice over the widget, within a certain time period.
 */
GUI_status GUI_Widget::MouseDouble(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}

/* Mouse cursor passed out of the widget area.
 */
GUI_status GUI_Widget::MouseEnter(uint8 state) {
	return (GUI_PASS);
}

/* Mouse cursor passed into the widget area.
 */
GUI_status GUI_Widget::MouseLeave(uint8 state) {
	return (GUI_PASS);
}

/* Returns false if any widget but this one is focused or locked.
 */
bool GUI_Widget::widget_has_focus() {
	GUI_Widget *focused_widget = GUI::get_gui()->get_focused_widget();
	GUI_Widget *locked_widget = GUI::get_gui()->get_locked_widget();

	if (GUI::get_gui()->get_block_input())
		return (false);
	if (locked_widget != NULL && locked_widget != this)
		return (false);
	if (focused_widget != NULL && focused_widget != this)
		return (false);
	return (true);
}

// button 0 = all
void GUI_Widget::set_accept_mouseclick(bool set, int button) {
	if (button <= 0)
		accept_mouseclick[0] = accept_mouseclick[1] = accept_mouseclick[2] = set;
	else if (button < 4)
		accept_mouseclick[button - 1] = set;
	set_mouseup(0, button);
	set_mousedown(0, button);
}

// time 0 = reset; button 0 = all
// mousedown is always cleared
void GUI_Widget::set_mouseup(int set, int button) {
	mouse_moved = false;
	if (button <= 0) {
		mouseup[0] = mouseup[1] = mouseup[2] = set;
		mousedown[0] = mousedown[1] = mousedown[2] = 0;
	} else if (button < 4) {
		mouseup[button - 1] = set;
		mousedown[button - 1] = 0;
	}
}

// time 0 = reset; button 0 = all
// mouseup is not cleared because two mouseup times are compared for mouseclicks
void GUI_Widget::set_mousedown(int set, int button) {
	if (button <= 0) {
//        mouseup[0]=mouseup[1]=mouseup[2] = 0;
		mousedown[0] = mousedown[1] = mousedown[2] = set;
	} else if (button < 4) {
//        mouseup[button-1] = 0;
		mousedown[button - 1] = set;
	}
}

// check to see if time has passed for a MouseDelayed or MouseHeld
GUI_status GUI_Widget::try_mouse_delayed() {
	int mousedown_time = get_mousedown(held_button);
	int mouseup_time = get_mouseup(delayed_button);
	int time_to_hold = SDL_GetTicks() - mousedown_time;
	int time_to_click = SDL_GetTicks() - mouseup_time;

	if (mousedown_time != 0 && time_to_hold >= GUI::mouseclick_delay) {
		Shared::MouseButton button = held_button;
		int x, y; // position isn't saved anywhere so we get it here
		screen->get_mouse_location(&x, &y); // hopefully it hasn't changed since MouseDown
		held_button = Shared::BUTTON_NONE; // no need to clear mousedown time, MouseUp does that
		return (MouseHeld(x, y, button));
	}

	if (mouseup_time != 0 && time_to_click >= GUI::mouseclick_delay) {
		Shared::MouseButton button = delayed_button;
		int x, y; // position isn't saved anywhere so we get it here
		screen->get_mouse_location(&x, &y); // hopefully it hasn't changed since MouseClick/MouseUp
		delayed_button = Shared::BUTTON_NONE;
		// before a Double or Delayed click, mouseup time is reset
		set_mouseup(0, button);
		return (MouseDelayed(x, y, button));
	}
	return (GUI_PASS);
}

// like a MouseClick but called only after waiting for MouseDouble, if
// wait_for_mouseclick(button) was called
GUI_status GUI_Widget::MouseDelayed(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}

// like a MouseDown but called only after waiting for MouseUp, if
// wait_for_mousedown(button) was called
GUI_status GUI_Widget::MouseHeld(int x, int y, Shared::MouseButton button) {
	return (GUI_PASS);
}

} // End of namespace Nuvie
} // End of namespace Ultima
