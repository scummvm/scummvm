
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

#ifndef BAGEL_BAGLIB_CURSOR_H
#define BAGEL_BAGLIB_CURSOR_H

#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/boflib/error.h"

namespace Bagel {
namespace SpaceBar {

class CSystemCursor;

class CBagCursor : public CBofObject, public CBofError {
private:
	char _fileName[MAX_FNAME];
	CBofBitmap *_bitmap;
	int _x;
	int _y;
	bool _sharedPalFl;
	bool _wieldCursorFl;

protected:
	static CBagCursor *_currentCursor;
	static CSystemCursor *_systemCursor;

public:
	CBagCursor();
	CBagCursor(CBofBitmap *bmp);
	CBagCursor(const char *fileName, bool sharedPalFl = false);
	static void initialize();
	static void shutdown();
	static void showSystemCursor();
	static void hideSystemCursor() {
		_currentCursor = nullptr;
	}

	~CBagCursor();

	void setHotspot(int x, int y);

	CBofPoint getHotspot() const;

	int getX() const {
		return _x;
	}
	int getY() const {
		return _y;
	}

	ErrorCode load() {
		return load(_fileName);
	}
	ErrorCode load(CBofBitmap *bmp);
	ErrorCode load(const char *fileName, CBofPalette *pal = nullptr);

	void unLoad();

	CBofBitmap *getImage() const {
		return _bitmap;
	}
	ErrorCode setImage(CBofBitmap *bmp) {
		return load(bmp);
	}

	static CBagCursor *getCurrent() {
		return _currentCursor;
	}

	static bool isSystemCursorVisible() {
		return _currentCursor && _currentCursor->isSystemCursor();
	}

	/**
	 * Flag whether this is a wielded cursor or not
	 */
	void setWieldCursor(bool b) {
		_wieldCursorFl = b;
	}
	bool IsWieldCursor() const {
		return _wieldCursorFl;
	}

	void show() {
		setCurrent();
	}
	void hide() {
		_currentCursor = nullptr;
	}

	virtual void setCurrent();
	virtual bool isSystemCursor() const {
		return false;
	}
};

class CSystemCursor : public CBagCursor {
public:
	CSystemCursor() : CBagCursor() {
	}

	void setCurrent() override;
	bool isSystemCursor() const override {
		return true;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
