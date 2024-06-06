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

#ifndef __GDI_GR_DISPATCHER_H__
#define __GDI_GR_DISPATCHER_H__
#define LOGPALETTE int
#define HPALETTE int

#include "qdengine/core/system/graphics/gr_dispatcher.h"


namespace QDEngine {

class GDI_grDispatcher : public grDispatcher {
public:
	GDI_grDispatcher();
	~GDI_grDispatcher();

	bool Flush(int x, int y, int sx, int sy);
	bool StretchFlush(int x_dest, int y_dest, int sx_dest, int sy_dest, int x_src, int y_src, int sx_src, int sy_src);

	bool init(int sx, int sy, grPixelFormat pixel_format, void *hwnd, bool fullscreen = false);

	bool InitScreen(int sx, int sy);
	bool Finit();

	bool is_in_fullscreen_mode() const {
		return false;
	}
	bool is_mode_supported(int sx, int sy, grPixelFormat pixel_format) const {
		return true;
	}
	bool is_mode_supported(grPixelFormat pixel_format) const {
		return true;
	}

	bool palette_mode() const {
		return palette_mode_;
	}
	bool set_palette(const char *pal, int start_col, int pal_size);

private:

	void *dibPtr;
	void *dibHandle;

	bool palette_mode_;
	LOGPALETTE *sys_palette_;
	HPALETTE sys_pal_handle_;
	LOGPALETTE *palette_;
	HPALETTE pal_handle_;
};

} // namespace QDEngine

#endif /* __GDI_GR_DISPATCHER_H__ */
