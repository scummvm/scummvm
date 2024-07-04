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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/graphics/gr_dispatcher.h"
#include "qdengine/core/system/input/mouse_input.h"
#include "qdengine/core/system/input/keyboard_input.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

LRESULT CALLBACK grWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* --------------------------- DEFINITION SECTION --------------------------- */

void *grDispatcher::default_mouse_cursor_;
char *grDispatcher::wnd_class_name_ = "grWndClass";

LRESULT CALLBACK GR_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	grDispatcher *p = grDispatcher::instance(hwnd);
	bool hittest = false;

		warning("GR_WindowProc: %d", uMsg);
#if 0
	switch (uMsg) {
	case WM_SYSKEYDOWN:
		if ((int)wParam != VK_F4)
			return 0;
		break;
	case WM_NCHITTEST:
		hittest = true;
		break;
	case WM_CHAR:
		if (grDispatcher::handle_char_input(int(wParam)))
			return 0;
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_MAXIMIZE) {
			if (p && p->maximize_handler())
				(*p->maximize_handler())();
			return 0;
		}
		break;
#ifndef _QUEST_EDITOR
	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		DestroyWindow(hwnd);
		return 0;
#endif
	case WM_DESTROY:
		if (!grDispatcher::instance() || !grDispatcher::instance()->is_in_reinit_mode())
			PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		if (wParam)
			grDispatcher::activate(true);
		else
			grDispatcher::activate(false);
		break;
	case WM_SETCURSOR:
		if (p) {
#ifndef _QUEST_EDITOR
			SetCursor((HCURSOR)p->mouse_cursor());
			return 1;
#else
			return 0;
#endif
		}
		break;
	case WM_PAINT:
		if (p) {
			PAINTSTRUCT pc;
			HDC hdc = BeginPaint(hwnd, &pc);
			p->Flush(pc.rcPaint.left, pc.rcPaint.top, pc.rcPaint.right - pc.rcPaint.left, pc.rcPaint.bottom - pc.rcPaint.top);
			EndPaint(hwnd, &pc);
			return 0;
		}
		break;
	}

	LRESULT res = DefWindowProc(hwnd, uMsg, wParam, lParam);

	if (hittest && p) {
		if (res == HTCLIENT && p->is_mouse_hidden())
			p->set_null_mouse_cursor();
		else
			p->set_default_mouse_cursor();
	}
	return res;
#endif
}

grDispatcher *grDispatcher::instance(void *hwnd) {
	warning("STUB: grDispatcher::instance()");
#if 0
	return reinterpret_cast<grDispatcher *>(GetWindowLong((HWND)hwnd, GWL_USERDATA));
#endif
}

bool grDispatcher::sys_finit() {
	warning("STUB: grDispatcher::sys_finit()");
#if 0
	UnregisterClass(grDispatcher::wnd_class_name(), GetModuleHandle(0));
	DestroyCursor((HCURSOR)default_mouse_cursor_);
#endif
	return true;
}
} // namespace QDEngine
