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

#ifndef SHERLOCK_SCREEN_H
#define SHERLOCK_SCREEN_H

#include "common/list.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "sherlock/surface.h"
#include "sherlock/resources.h"

namespace Sherlock {

#define PALETTE_SIZE 768
#define PALETTE_COUNT 256
#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

enum {
	INFO_BLACK		= 1,
	INFO_FOREGROUND	= 11,
	INFO_BACKGROUND	= 1,
	BORDER_COLOR	= 237,
	INV_FOREGROUND	= 14,
	INV_BACKGROUND	= 1,
	COMMAND_HIGHLIGHTED = 10,
	COMMAND_FOREGROUND = 15,
	COMMAND_BACKGROUND = 4,
	COMMAND_NULL = 248,
	BUTTON_TOP = 233,
	BUTTON_MIDDLE	= 244,
	BUTTON_BOTTOM	= 248,
	TALK_FOREGROUND = 12,
	TALK_NULL		= 16,
	PEN_COLOR		= 250
};

class SherlockEngine;

class Screen : public Surface {
private:
	SherlockEngine *_vm;
	int _fontNumber;
	Common::List<Common::Rect> _dirtyRects;
	uint32 _transitionSeed;
	ImageFile *_font;
	int _fontHeight;
	Surface _sceneSurface;

	void mergeDirtyRects();

	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);

	void writeString(const Common::String &str, const Common::Point &pt, byte color);
protected:
	virtual void addDirtyRect(const Common::Rect &r);
public:
	Surface _backBuffer1, _backBuffer2;
	Surface *_backBuffer;
	bool _fadeStyle;
	byte _cMap[PALETTE_SIZE];
	byte _sMap[PALETTE_SIZE];
public:
	Screen(SherlockEngine *vm);
	virtual ~Screen();

	void setFont(int fontNumber);

	void update();

	void getPalette(byte palette[PALETTE_SIZE]);

	void setPalette(const byte palette[PALETTE_SIZE]);

	int equalizePalette(const byte palette[PALETTE_SIZE]);

	void fadeToBlack(int speed = 2);

	void fadeIn(const byte palette[PALETTE_SIZE], int speed = 2);

	void randomTransition();

	void verticalTransition();

	void print(const Common::Point &pt, byte color, const char *formatStr, ...);
	void gPrint(const Common::Point &pt, byte color, const char *formatStr, ...);

	void restoreBackground(const Common::Rect &r);

	void slamArea(int16 xp, int16 yp, int16 width, int16 height);
	void slamRect(const Common::Rect &r);

	void flushImage(ImageFrame *frame, const Common::Point &pt,
		int16 *xp, int16 *yp, int16 *width, int16 *height);

	int stringWidth(const Common::String &str);

	int charWidth(char c);

	void vgaBar(const Common::Rect &r, int color);

	void makeButton(const Common::Rect &bounds, int textX, const Common::String &str);

	void buttonPrint(const Common::Point &pt, byte color, bool slamIt, const Common::String &str);

	void makePanel(const Common::Rect &r);

	void setDisplayBounds(const Common::Rect &r);
	void resetDisplayBounds();
	Common::Rect getDisplayBounds();

	int fontNumber() const { return _fontNumber; }

	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
