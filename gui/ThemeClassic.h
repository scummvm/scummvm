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
 * $URL$
 * $Id$
 */

#ifndef GUI_THEMECLASSIC_H
#define GUI_THEMECLASSIC_H

#include "gui/theme.h"

namespace GUI {

// enable this to disable transparency support for the classic theme
//#define CT_NO_TRANSPARENCY

class ThemeClassic : public Theme {
public:
	ThemeClassic(OSystem *system, const Common::String &config = "classic", const Common::ConfigFile *cfg = 0);
	virtual ~ThemeClassic();

	bool init();
	void deinit();

	void refresh();

	void enable();
	void disable();

	void openDialog(bool topDialog, ShadingStyle = kShadingNone);
	void closeAllDialogs();

	void clearAll();
	void updateScreen();

	void resetDrawArea();

	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }

	void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state);
	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state);

	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state);
	void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans);
	void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state);
	void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state);
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align);
	void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state);
	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state);
	void restoreBackground(Common::Rect r, bool special = false);
	bool addDirtyRect(Common::Rect r, bool save = false, bool special = false);

	int getTabSpacing() const;
	int getTabPadding() const;

private:
	void box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB, bool skipLastRow = false);
	void box(int x, int y, int width, int height);

	OverlayColor getColor(State state);

	OSystem *_system;
	Graphics::Surface _screen;

#ifndef CT_NO_TRANSPARENCY
	struct DialogState {
		Graphics::Surface screen;
	} *_dialog;

	void blendScreenToDialog();
#endif
	bool _enableBlending;

	bool _forceRedraw;
	bool _initOk;
	bool _enabled;

	Common::String _fontName;
	const Graphics::Font *_font;
	OverlayColor _color, _shadowcolor;
	OverlayColor _bgcolor;
	OverlayColor _textcolor;
	OverlayColor _textcolorhi;

	enum {
		kColor = 0,
		kShadowColor = 1,
		kBGColor = 2,
		kTextColor = 3,
		kTextColorHi = 4,
		kMaxColors = 5
	};
	uint8 _colors[kMaxColors][3];

	void setupConfig();
	bool loadConfig();
};

} // end of namespace GUI

#endif

