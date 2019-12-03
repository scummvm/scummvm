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
#include "ultima8/gumps/console_gump.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

#include "ultima8/kernel/gui_app.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ConsoleGump, Gump)

using Pentagram::istring;

ConsoleGump::ConsoleGump()
	: Gump() {
	con.AddConsoleCommand("ConsoleGump::toggle",
	                      ConsoleGump::ConCmd_toggle);
}

ConsoleGump::ConsoleGump(int X, int Y, int Width, int Height) :
	Gump(X, Y, Width, Height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP, LAYER_CONSOLE),
	scroll_state(NORMAL_DISPLAY), scroll_frame(8) {
	con.ClearCommandBuffer();

	// Resize it
	con.CheckResize(Width);

	con.AddConsoleCommand("ConsoleGump::toggle",
	                      ConsoleGump::ConCmd_toggle);
}

ConsoleGump::~ConsoleGump() {
	con.RemoveConsoleCommand(ConsoleGump::ConCmd_toggle);
}

void ConsoleGump::RenderSurfaceChanged() {
	// Resize the desktop gump to match the RenderSurface
	Pentagram::Rect new_dims;
	parent->GetDims(new_dims);
	con.CheckResize(new_dims.w);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	Gump::RenderSurfaceChanged();
}

void ConsoleGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	int h = dims.h;
	uint32 next_frame = scroll_frame + 1;
	Gump::PaintThis(surf, lerp_factor, scaled);

	switch (scroll_state) {
	case NOTIFY_OVERLAY:
#ifdef DEBUG
		con.DrawConsoleNotify(surf);
#endif
		return;
	case WAITING_TO_SHOW:
		return;

	case SCROLLING_TO_HIDE:
		if (scroll_frame == 0)
			return;
		// Just reverse next_frame and fall through
		next_frame = scroll_frame - 1;
	case SCROLLING_TO_SHOW:
		if (next_frame > 8)
			break;

		// Take in part from Item::doLerp()
		if (lerp_factor == 256)
			h = (h * next_frame) >> 3;
		else if (lerp_factor == 0)
			h = (h * scroll_frame) >> 3;
		else
			h = (h * ((scroll_frame * (256 - lerp_factor))
			          + (next_frame * lerp_factor))) >> 11;

		if (h > dims.h)
			h = dims.h;
		else if (h < 0)
			h = 0;
	default:
		break;
	}

	//surf->FillBlended(0x60000000,0,0,dims.w,h);

	// This line shows some draw lag when uncommented
	//surf->Fill32(0x00FFFFFF,0,h-1,dims.w,h);
	con.DrawConsole(surf, h);
}

void ConsoleGump::ToggleConsole() {
	switch (scroll_state) {
	case WAITING_TO_HIDE:
	case SCROLLING_TO_HIDE:
		scroll_state = SCROLLING_TO_SHOW;
		break;

	case NOTIFY_OVERLAY:
		scroll_state = WAITING_TO_SHOW;
		break;

	case WAITING_TO_SHOW:
	case SCROLLING_TO_SHOW:
		scroll_state = SCROLLING_TO_HIDE;
		break;

	case NORMAL_DISPLAY:
		scroll_state = WAITING_TO_HIDE;
		GUIApp::get_instance()->leaveTextMode(this);
		con.ClearCommandBuffer();
		break;

	default:
		break;
	}
}


void ConsoleGump::HideConsole() {
	switch (scroll_state) {
	case WAITING_TO_SHOW:
	case SCROLLING_TO_SHOW:
		scroll_state = SCROLLING_TO_HIDE;
		break;

	case NORMAL_DISPLAY:
		scroll_state = WAITING_TO_HIDE;
		GUIApp::get_instance()->leaveTextMode(this);
		con.ClearCommandBuffer();
		break;

	default:
		break;
	}
}


void ConsoleGump::ShowConsole() {
	switch (scroll_state) {
	case WAITING_TO_HIDE:
	case SCROLLING_TO_HIDE:
		scroll_state = SCROLLING_TO_SHOW;
		break;

	case NOTIFY_OVERLAY:
		scroll_state = WAITING_TO_SHOW;
		break;

	default:
		break;
	}
}

bool ConsoleGump::ConsoleIsVisible() {
	return scroll_state != NOTIFY_OVERLAY;
}

void ConsoleGump::run() {
	Gump::run();

	con.setFrameNum(Kernel::get_instance()->getFrameNum());

	switch (scroll_state) {
	case WAITING_TO_HIDE:
		scroll_state = SCROLLING_TO_HIDE;
		break;

	case SCROLLING_TO_HIDE:
		if (scroll_frame > 0) {
			--scroll_frame;
			break;
		}
		scroll_frame = 0;
		scroll_state = NOTIFY_OVERLAY;
		break;

	case WAITING_TO_SHOW:
		scroll_state = SCROLLING_TO_SHOW;
		break;

	case SCROLLING_TO_SHOW:
		if (scroll_frame < 8) {
			++scroll_frame;
			break;
		}
		scroll_frame = 8;
		scroll_state = NORMAL_DISPLAY;
		GUIApp::get_instance()->enterTextMode(this);
		con.ClearCommandBuffer();
		break;

	default:
		break;
	}
}

void ConsoleGump::ConCmd_toggle(const Console::ArgvType &argv) {
	ConsoleGump *consoleGump = GUIApp::get_instance()->getConsoleGump();
	consoleGump->ToggleConsole();
}

void ConsoleGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ConsoleGump");
}

bool ConsoleGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to save ConsoleGump");

	return false;
}

bool ConsoleGump::OnTextInput(int unicode) {
	bool handled = false;
	if (scroll_state == NORMAL_DISPLAY) {

		con.AddCharacterToCommandBuffer(unicode);
		handled = true;
	}
	return handled;
}


bool ConsoleGump::OnKeyDown(int key, int mod) {
	bool handled = false;
	if (scroll_state == NORMAL_DISPLAY) {
		switch (key) {
		// Command completion
		case Common::KEYCODE_TAB:
			con.AddCharacterToCommandBuffer(Console::Tab);
			break;

		case Common::KEYCODE_ESCAPE:
			ToggleConsole();
			break;

		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			con.AddCharacterToCommandBuffer(Console::Enter);
			break;

		case Common::KEYCODE_BACKSPACE:
			con.DeleteCommandBufferChars(-1);
			break;

		case Common::KEYCODE_DELETE:
			con.DeleteCommandBufferChars(1);
			break;

		case Common::KEYCODE_PAGEUP:
			con.ScrollConsole(-3);
			break;

		case Common::KEYCODE_PAGEDOWN:
			con.ScrollConsole(3);
			break;

		case Common::KEYCODE_UP:
			con.ScrollCommandHistory(-1);
			break;

		case Common::KEYCODE_DOWN:
			con.ScrollCommandHistory(1);
			break;

		case Common::KEYCODE_LEFT:
			con.MoveCommandCursor(-1);
			break;

		case Common::KEYCODE_RIGHT:
			con.MoveCommandCursor(1);
			break;

		case Common::KEYCODE_INSERT:
			con.ToggleCommandInsert();
			break;

		case Common::KEYCODE_KP0:
		case Common::KEYCODE_KP1:
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_KP3:
		case Common::KEYCODE_KP4:
		case Common::KEYCODE_KP5:
		case Common::KEYCODE_KP6:
		case Common::KEYCODE_KP7:
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_KP9:
			OnTextInput(key - Common::KEYCODE_KP0 + '0');
			break;

		default:
			break;
		}
		handled = true;
	}
	return handled;
}

void ConsoleGump::OnFocus(bool gain) {
	/*
	if (scroll_state == NORMAL_DISPLAY) {
	    if (gain)
	        GUIApp::get_instance()->enterTextMode(this);
	    else
	        GUIApp::get_instance()->leaveTextMode(this);
	}
	*/

}

} // End of namespace Ultima8
