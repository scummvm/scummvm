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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_WINDOW_H
#define TONY_WINDOW_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "tony/adv.h"

namespace Tony {

typedef uint32 HWND;
struct DDSURFACEDESC {
};

class RMSnapshot {
private:
	// Buffer used to convert to RGB
	static byte rgb[RM_SX *RM_SY * 3];
public:
	/**
	 * Take a screenshot
	 */
	void grabScreenshot(byte *lpBuf, int dezoom = 1, uint16 *lpDestBuf = NULL);
};


class RMWindow {
private:
	bool lock();
	void unlock();
	void plotSplices(const byte *lpBuf, const Common::Point &center, int x, int y);
	void plotLines(const byte *lpBuf, const Common::Point &center, int x, int y);

protected:
	int fps, fcount;
	int lastsecond, lastfcount;

	int mskRed, mskGreen, mskBlue;
	bool _wiping;
	bool _showDirtyRects;

	bool _bGrabScreenshot;
	bool _bGrabThumbnail;
	bool _bGrabMovie;
	uint16 *_wThumbBuf;
	uint16 *_precalcTable;

	void createBWPrecalcTable();
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	void wipeEffect(Common::Rect &rcBoundEllipse);
	void getNewFrameWipe(byte *lpBuf, Common::Rect &rcBoundEllipse);

public:
	RMWindow();
	~RMWindow();

	/**
	 * Initialization
	 */
	void init(/*HINSTANCE hInst*/);
	void initDirectDraw();
	void close();

	/**
	 * Drawing
	 */
	void repaint();

	/**
	 * Switch between windowed and fullscreen
	 */
	void switchFullscreen(bool bFull) {}

	/**
	 * Reads the next frame
	 */
	void getNewFrame(RMGfxTargetBuffer &lpBuf, Common::Rect *rcBoundEllipse);

	/**
	 * Request a thumbnail be grabbed during the next frame
	 */
	void grabThumbnail(uint16 *buf);

	int getFps() const {
		return fps;
	}
	void showDirtyRects(bool v) { _showDirtyRects = v; }
};

} // End of namespace Tony

#endif /* TONY_WINDOW_H */
