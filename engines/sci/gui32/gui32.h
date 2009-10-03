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
 * $URL$
 * $Id$
 *
 */

namespace Sci {

class SciGUI32 : public SciGUI {
public:
	SciGUI32(OSystem *system, EngineState *s);
	~SciGUI32();

	void init(bool oldGfxFunctions);

	int16 getTimeTicks();
	void wait(int16 ticks);
	void setPort(uint16 portPtr);
	void setPortPic(Common::Rect rect, int16 picTop, int16 picLeft);
	reg_t getPort();
	void globalToLocal(int16 *x, int16 *y);
	void localToGlobal(int16 *x, int16 *y);
	reg_t newWindow(Common::Rect rect1, Common::Rect rect2, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	void disposeWindow(uint16 windowPtr, int16 arg2);

	void display(const char *text, int argc, reg_t *argv);

	void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	void textFonts(int argc, reg_t *argv);
	void textColors(int argc, reg_t *argv);

	void drawPicture(sciResourceId pictureId, uint16 showStyle, uint16 flags, int16 EGApaletteNo);
	void drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo);
	void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool inverse);
	void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool inverse);

	void graphFillBoxForeground(Common::Rect rect);
	void graphFillBoxBackground(Common::Rect rect);	
	void graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control);
	void graphDrawLine(Common::Rect rect, int16 color, int16 priority, int16 control);
	reg_t graphSaveBox(Common::Rect rect, uint16 flags);
	void graphRestoreBox(reg_t handle);

	void paletteSet(int resourceNo, int flags);
	virtual int16 paletteFind(int r, int g, int b);
	void paletteAnimate(int fromColor, int toColor, int speed);

	void moveCursor(int16 x, int16 y);

private:
	OSystem *_system;
	EngineState *s;
	bool _usesOldGfxFunctions;

	bool activated_icon_bar;	// FIXME: Avoid non-const global vars
	int port_origin_x;	// FIXME: Avoid non-const global vars
	int port_origin_y;	// FIXME: Avoid non-const global vars
};

} // End of namespace Sci
