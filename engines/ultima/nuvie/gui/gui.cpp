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

/* This is a C++ class for handling a GUI, and associated widgets */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_types.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

const int GUI::mouseclick_delay = 300; /* SB-X */


/* Number of widget elements to allocate at once */
#define WIDGET_ARRAYCHUNK   32

GUI *GUI::gui = NULL;

GUI:: GUI(Configuration *c, Screen *s) {
	Graphics::ManagedSurface *sdl_surface;

	gui = this;
	config = c;
	screen = s;
	numwidgets = 0;
	maxwidgets = 0;
	widgets = NULL;
	display = 1;
	running = 0;

	screen_scale_factor = screen->get_scale_factor();

	dragging = false;
	full_redraw = true;
	focused_widget = locked_widget = NULL;
	block_input = false;

	sdl_surface = screen->get_sdl_surface();

	selected_color = new GUI_Color(10, 10, 50);
	selected_color->map_color(sdl_surface);

	gui_font = new GUI_Font();
	gui_drag_manager = new GUI_DragManager(screen);
}

GUI:: ~GUI() {
	if (widgets != NULL) {
		for (int i = 0; i < numwidgets; ++i) {
			delete widgets[i];
		}
		free(widgets);
	}

	delete selected_color;

	delete gui_font;
	delete gui_drag_manager;
}

/* Add a widget to the GUI.
   The widget will be automatically deleted when the GUI is deleted.
 */
int
GUI:: AddWidget(GUI_Widget *widget) {
	int i;

	/* Look for deleted widgets */
	for (i = 0; i < numwidgets; ++i) {
		if (widgets[i]->Status() == WIDGET_DELETED) {
			delete widgets[i];
			break;
		}
	}
	if (i == numwidgets) {
		/* Expand the widgets array if necessary */
		if (numwidgets == maxwidgets) {
			GUI_Widget **newarray;
			int maxarray;

			maxarray = maxwidgets + WIDGET_ARRAYCHUNK;
			if ((newarray = (GUI_Widget **)realloc(widgets,
			                                       maxarray * sizeof(*newarray))) == NULL) {
				return (-1);
			}
			widgets = newarray;
			maxwidgets = maxarray;
		}
		++numwidgets;
	}
	widgets[i] = widget;
	widget->PlaceOnScreen(screen, gui_drag_manager, 0, 0);

	return (0);
}

/* remove widget from gui system but don't delete it */
bool GUI::removeWidget(GUI_Widget *widget) {
	int i;

	for (i = 0; i < numwidgets; ++i) {
		if (widgets[i] == widget) {
			for (int j = i + 1; j < numwidgets; ++j) { //shuffle remaining widgets down.
				widgets[j - 1] = widgets[j];
			}

			--numwidgets;
			force_full_redraw();
			Display();
			return true;
		}
	}

	return false;
}

void GUI::CleanupDeletedWidgets(bool redraw) {
	/* Garbage collection */
	if (locked_widget && locked_widget->Status() == WIDGET_DELETED)
		locked_widget = 0;
	if (focused_widget && focused_widget->Status() == WIDGET_DELETED)
		focused_widget = 0;

	for (int i = 0; i < numwidgets;) {
		if (widgets[i]->Status() == WIDGET_DELETED) {
			delete widgets[i];

			for (int j = i + 1; j < numwidgets; ++j) //shuffle remaining widgets down.
				widgets[j - 1] = widgets[j];

			--numwidgets;
			if (redraw) {
				// CHECKME: is it really necessary to redraw after each deletion?
				force_full_redraw();
				Display();
			}
		} else
			++i;
	}
}

bool GUI::moveWidget(GUI_Widget *widget, uint32 dx, uint32 dy) {
	if (!widget)
		return false;

	widget->MoveRelative(dx, dy);

	if (widget->Status() == WIDGET_VISIBLE)
		widget->Redraw();//force_full_redraw();

	return true;
}

void GUI::force_full_redraw() {
	full_redraw = true;
}

void GUI::Display() {
	int i;
	bool complete_redraw = false;

	//  hack for now to make everyhing under the cursor draw until I find a better
	//  way of doing this...
	if (dragging || full_redraw)
		complete_redraw = true;

	for (i = 0; i < numwidgets; ++i) {
		if (widgets[i]->Status() == WIDGET_VISIBLE) {
			widgets[i]->Display(complete_redraw);
			//screen->update(widgets[i]->area.left,widgets[i]->area.top,widgets[i]->area.width(),widgets[i]->area.height());
		}
	}
	//SDL_UpdateRect(screen, 0, 0, 0, 0);

	int mx, my;
	screen->get_mouse_location(&mx, &my);

	gui_drag_manager->draw(mx, my);

	if (full_redraw)
		full_redraw = false;
}

/* Function to handle a GUI status */
void
GUI:: HandleStatus(GUI_status status) {
	switch (status) {
	case GUI_QUIT:
		running = 0;
		break;
	case GUI_REDRAW:
		display = 1;
		break;
	case GUI_DRAG_AND_DROP:
		dragging = true;
		break;
	default:
		break;
	}
}

/* Handle an event, passing it to widgets until they return a status */
GUI_status GUI:: HandleEvent(Common::Event *event) {
	int i;
	int hit;
	GUI_status status = GUI_PASS;

	if (screen_scale_factor != 1) {
		if (Shared::isMouseDownEvent(event->type) || Shared::isMouseUpEvent(event->type)) {
			event->mouse.x /= screen_scale_factor;
			event->mouse.y /= screen_scale_factor;
		}
		if (event->type == Common::EVENT_MOUSEMOVE) {
			event->mouse.x /= screen_scale_factor;
			event->mouse.y /= screen_scale_factor;
		}
	}

	if (dragging) { //&& !block_input)
		if (Shared::isMouseUpEvent(event->type)) { //FIX for button up that doesn't hit a widget.
			for (hit = false, i = numwidgets - 1; (i >= 0) && (hit == false); --i) {
				if (widgets[i]->Status() == WIDGET_VISIBLE && widgets[i]->is_drop_target() && widgets[i]->HitRect(event->mouse.x, event->mouse.y)) {
					gui_drag_manager->drop((GUI_DragArea *)widgets[i], event->mouse.x, event->mouse.y);
					dragging = false;
					Display(); // redraw the widget to get rid of the drop graphic.
					break;
				}
			}
		}
	} else if (!block_input) {
		if (event->type == Common::EVENT_JOYAXIS_MOTION ||
				event->type == Common::EVENT_JOYBUTTON_DOWN ||
				event->type == Common::EVENT_JOYBUTTON_UP) {
			event->kbd.keycode = Game::get_game()->get_keybinder()->get_key_from_joy_events(event);
			if (event->kbd.keycode == Common::KEYCODE_INVALID) { // isn't mapped, is in deadzone, or axis didn't return to center before moving again
				HandleStatus(status);
				CleanupDeletedWidgets(status != GUI_QUIT);
				return status; // pretend nothing happened
			}
			event->type = Common::EVENT_KEYDOWN;
			event->kbd.flags = 0;
		}

		switch (event->type) {
		/* SDL_QUIT events quit the GUI */
		// case SDL_QUIT:
		//   status = GUI_QUIT;
		//   break;

		/* Keyboard and mouse events go to widgets */

		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_MBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MBUTTONUP:
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
		case Common::EVENT_WHEELDOWN:
		case Common::EVENT_WHEELUP:
//			 /* Go through widgets, topmost first */
//			 status = GUI_PASS;
//			 for (i=numwidgets-1; (i>=0)&&(status==GUI_PASS); --i) {
//				 if ( widgets[i]->Status() == WIDGET_VISIBLE ) {
//				   status = widgets[i]->HandleEvent(event);
//				 }
//			 }
//			 break;
			/* Send everything to locked widget. */
			if (locked_widget && locked_widget->Status() == WIDGET_VISIBLE) {
				status = locked_widget->HandleEvent(event);
				if (status == GUI_PASS) // can't bypass the lock
					status = GUI_YUM;
			}
			/* Go through widgets, focused first, then from the
			   top.*/
			else {
				status = GUI_PASS;
				if (focused_widget && focused_widget->Status() == WIDGET_VISIBLE) {
					status = focused_widget->HandleEvent(event);
				}
				for (i = numwidgets - 1; (i >= 0) && (status == GUI_PASS); --i) {
					if (widgets[i]->Status() == WIDGET_VISIBLE
					        && widgets[i] != focused_widget) {  // don't send to focused twice
						status = widgets[i]->HandleEvent(event);
					}
				}
			}
			break;

		/* Ignore unhandled events */
		default:
			status = GUI_PASS;
			break;
		}
	}

	HandleStatus(status);

	CleanupDeletedWidgets(status != GUI_QUIT);

	return status;
}

/* Run the GUI.
   This returns when either a widget requests a quit, the idle
   function requests a quit, or the SDL window has been closed.
 */
void GUI::Run(GUI_IdleProc idle, int once, int multitaskfriendly) {
	int i;
	Common::Event event;

	/* If there's nothing to do, return immediately */
	if ((numwidgets == 0) && (idle == NULL)) {
		return;
	}

	running = 1;
	if (! once) {
		display = 1;
	}
	do {
		CleanupDeletedWidgets();

		/* Display widgets if necessary */
		if (display) {
			Display();
			display = 0;
		}

///////////////////////////////////////////////////////////////// Polling is time consuming - instead:
		if (multitaskfriendly && (idle == NULL)) {
			SDL_WaitEvent(&event);
			HandleEvent(&event);
		} else
/////////////////////////////////////////////////////////////////
			/* Handle events, or run idle functions */
			if (SDL_PollEvent(&event)) {
				/* Handle all pending events */
				do {
					HandleEvent(&event);
				} while (SDL_PollEvent(&event));
			} else {
				if (idle != NULL) {
					HandleStatus(idle());
				}
				for (i = numwidgets - 1; i >= 0; --i) {
					HandleStatus(widgets[i]->Idle());
				}
			}
		//ERIC SDL_Delay(10);
	} while (running && ! once);
}

GUI_Font *GUI::get_font() {
	return gui_font;
}


// SB-X
void GUI::Idle() {
	if (locked_widget) {
		locked_widget->Idle();
		return;
	}

	for (int i = numwidgets - 1; i >= 0; --i) {
		HandleStatus(widgets[i]->Idle());
	}
}

bool GUI::set_focus(GUI_Widget *widget) {
	/*
	    for(int i = 0; i < numwidgets; ++i)
	      {
	        if(!widget || (widgets[i] == widget)) // must be managed by GUI
	            {
	*/
	focused_widget = widget;
	return true;
//            }
//      }

// return false;
}

void GUI::lock_input(GUI_Widget *widget) {
	for (int i = 0; i < numwidgets; ++i)
		if (!widget || (widgets[i] == widget)) // must be managed by GUI
			locked_widget = widget;
}

Std::string GUI::get_data_dir() {
	Std::string datadir;

	config->value("config/datadir", datadir, "");

	return datadir;
}

} // End of namespace Nuvie
} // End of namespace Ultima
