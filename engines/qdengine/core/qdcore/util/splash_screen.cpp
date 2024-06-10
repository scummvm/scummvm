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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "image/bmp.h"
#include "common/formats/winexe.h"
#include "common/formats/winexe_pe.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/util/splash_screen.h"
#include "qdengine/qdengine.h"

namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool SplashScreen::create(int bitmapResID) {
	if (!create_window()) return false;

	Common::PEResources r;
    Common::WinResourceID resid(bitmapResID);
    Image::BitmapDecoder decoder;

    if (r.loadFromEXE("shveik.exe")) {
        Common::SeekableReadStream *stream = r.getResource(Common::kWinBitmap, resid);
        if (decoder.loadStream(*stream)) {
            splash_hwnd_->copyRectToSurface(decoder.getSurface(), 24,  0, 0, decoder.getSurface()->w, decoder.getSurface()->h);
            g_engine->_screen->updateScreen();
        }
    }
	return true;
}

bool SplashScreen::create(const char *bitmap_file) {
	if (!create_window()) return false;

	bitmap_handle_ = LoadImage(GetModuleHandle(NULL), bitmap_file, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	if (!bitmap_handle_) return false;

	SendMessage((HWND)splash_hwnd_, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap_handle_);
	return true;
}

bool SplashScreen::set_mask(int mask_resid) {
	HANDLE hbitmap = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(mask_resid), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	if (!hbitmap) return false;

	apply_mask(hbitmap);

	DeleteObject(hbitmap);
	return true;
}

bool SplashScreen::set_mask(const char *mask_file) {
	HANDLE hbitmap = LoadImage(GetModuleHandle(NULL), mask_file, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if (!hbitmap) return false;

	apply_mask(hbitmap);

	DeleteObject(hbitmap);
	return true;
}

bool SplashScreen::destroy() {
	if (splash_hwnd_) {
		hide();
		DestroyWindow((HWND)splash_hwnd_);
		splash_hwnd_ = NULL;
	}

	if (bitmap_handle_) {
		DeleteObject(bitmap_handle_);
		bitmap_handle_ = NULL;
	}

	return true;
}

void SplashScreen::show() {
	RECT rect;
	GetClientRect((HWND)splash_hwnd_, &rect);

	int x = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;

	SetWindowPos((HWND)splash_hwnd_, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);

	ShowWindow((HWND)splash_hwnd_, SW_SHOWNORMAL);
	UpdateWindow((HWND)splash_hwnd_);

	start_time_ = xclock();
}

void SplashScreen::wait(int time) {
	if (xclock() - start_time_ < time)
		Sleep(time - (xclock() - start_time_));
}

void SplashScreen::hide() {
	ShowWindow((HWND)splash_hwnd_, SW_HIDE);
}

bool SplashScreen::create_window() {
	destroy();

    splash_hwnd_ = new Graphics::Surface();
    splash_hwnd_->create(300, 300, g_engine->_pixelformat);
	if (!splash_hwnd_)
		return false;

	return true;
}

void SplashScreen::apply_mask(void *mask_handle) {
	BITMAP bmp;
	GetObject((HGDIOBJ)mask_handle, sizeof(bmp), &bmp);

	HRGN rgn = CreateRectRgn(0, 0, 0, 0);

	int sx = bmp.bmWidth;
	int sy = abs(bmp.bmHeight);

	int ssx = bmp.bmWidth;
	if (ssx % 4) ssx += 4 - (ssx % 4);

	int bpp = bmp.bmBitsPixel / 8;
	ssx *= bpp;

	for (int y = 0; y < sy; y ++) {
		int x, x0 = 0;
		const char *p = (const char *)bmp.bmBits + y * ssx;

		for (x = 0; x < sx; x ++) {
			bool flag = false;
			for (int i = 0; i < bpp; i++) {
				if (p[i])
					flag = true;
			}

			if (!flag) {
				if (x0 == x) {
					x0++;
				} else {
					HRGN rgn_add = CreateRectRgn(x0, y, x, y + 1);
					CombineRgn(rgn, rgn, rgn_add, RGN_OR);
					x0 = x + 1;
				}
			}

			p += bpp;
		}
		if (x0 != x) {
			HRGN rgn_add = CreateRectRgn(x0, y, x, y + 1);
			CombineRgn(rgn, rgn, rgn_add, RGN_OR);
		}
	}

	SetWindowRgn((HWND)splash_hwnd_, rgn, true);
}

} // namespace QDEngine
