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

#ifndef __DDRAW_GR_DISPATCHER_H__
#define __DDRAW_GR_DISPATCHER_H__
#include "qdengine/core/system/graphics/gr_dispatcher.h"


namespace QDEngine {

class DDraw_VideoMode {
public:
	DDraw_VideoMode(int sx, int sy, grPixelFormat pf) : screen_sx_(sx),
		screen_sy_(sy), pixel_format_(pf) { }
	~DDraw_VideoMode() { }

	int screen_sx() const {
		return screen_sx_;
	}
	int screen_sy() const {
		return screen_sy_;
	}
	grPixelFormat pixel_format() const {
		return pixel_format_;
	}

	int bits_per_pixel() const {
		switch (pixel_format_) {
		case GR_RGB565:
		case GR_ARGB1555:
			return 16;
		case GR_RGB888:
			return 24;
		case GR_ARGB8888:
			return 32;
		}
		return 0;
	}

private:
	int screen_sx_;
	int screen_sy_;
	grPixelFormat pixel_format_;
};

typedef std::vector<DDraw_VideoMode> DDraw_VideoModeVector;

class DDraw_grDispatcher : public grDispatcher {
public:
	DDraw_grDispatcher();
	~DDraw_grDispatcher();

	bool Flush(int x, int y, int sx, int sy);
	bool StretchFlush(int x_dest, int y_dest, int sx_dest, int sy_dest, int x_src, int y_src, int sx_src, int sy_src);

	bool Finit();

	bool init(int sx, int sy, grPixelFormat pixel_format, void *hwnd, bool fullscreen = false);

	bool is_in_fullscreen_mode() const {
		return fullscreen_;
	}

	bool is_mode_supported(int sx, int sy, grPixelFormat pixel_format) const;
	bool is_mode_supported(grPixelFormat pixel_format) const;

	bool get_current_mode(int &sx, int &sy, grPixelFormat &pixel_format) const;

	bool enum_display_modes();

private:
	bool fullscreen_;
#if 0
	LPDIRECTDRAW ddobj_;
	LPDIRECTDRAWSURFACE prim_surface_;
	LPDIRECTDRAWSURFACE back_surface_;
#endif
	DDSURFACEDESC back_surface_obj_;

	DDraw_VideoModeVector video_modes_;
};

} // namespace QDEngine

#endif /* __DDRAW_GR_DISPATCHER_H__ */
