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
 *
 */

#ifndef GUI_THEME_DATA_H
#define GUI_THEME_DATA_H


namespace GUI {

struct TextDrawData {
	const Graphics::Font *_fontPtr;

	struct {
		uint8 r, g, b;
	} _color;
};

struct WidgetDrawData {
	/** List of all the steps needed to draw this widget */
	Common::List<Graphics::DrawStep> _steps;

	int _textDataId;
	Graphics::TextAlign _textAlignH;
	GUI::ThemeEngine::TextAlignVertical _textAlignV;

	/** Extra space that the widget occupies when it's drawn.
	    E.g. when taking into account rounded corners, drop shadows, etc
		Used when restoring the widget background */
	uint16 _backgroundOffset;

	/** Sets whether the widget is cached beforehand. */
	bool _cached;
	bool _buffer;

	/** Texture where the cached widget is stored. */
	Graphics::Surface *_surfaceCache;

	~WidgetDrawData() {
		_steps.clear();

		if (_surfaceCache) {
			_surfaceCache->free();
			delete _surfaceCache;
		}
	}
};

class ThemeItem {

public:
	ThemeItem(ThemeEngine *engine, const Common::Rect &area) :
		_engine(engine), _area(area) {}
	virtual ~ThemeItem() {}

	virtual void drawSelf(bool doDraw, bool doRestore) = 0;

protected:
	ThemeEngine *_engine;
	Common::Rect _area;
};

class ThemeItemDrawData : public ThemeItem {
public:
	ThemeItemDrawData(ThemeEngine *engine, const WidgetDrawData *data, const Common::Rect &area, uint32 dynData) :
		ThemeItem(engine, area), _dynamicData(dynData), _data(data) {}

	void drawSelf(bool draw, bool restore);

protected:
	uint32 _dynamicData;
	const WidgetDrawData *_data;
};

class ThemeItemTextData : public ThemeItem {
public:
	ThemeItemTextData(ThemeEngine *engine, const TextDrawData *data, const Common::Rect &area, const Common::String &text,
		Graphics::TextAlign alignH, GUI::ThemeEngine::TextAlignVertical alignV,
		bool ellipsis, bool restoreBg, int deltaX) :
		ThemeItem(engine, area), _data(data), _text(text), _alignH(alignH), _alignV(alignV),
		_ellipsis(ellipsis), _restoreBg(restoreBg), _deltax(deltaX) {}

	void drawSelf(bool draw, bool restore);

protected:
	const TextDrawData *_data;
	Common::String _text;
	Graphics::TextAlign _alignH;
	GUI::ThemeEngine::TextAlignVertical _alignV;
	bool _ellipsis;
	bool _restoreBg;
	int _deltax;
};

class ThemeItemBitmap : public ThemeItem {
public:
	ThemeItemBitmap(ThemeEngine *engine, const Common::Rect &area, const Graphics::Surface *bitmap, bool alpha) :
		ThemeItem(engine, area), _bitmap(bitmap), _alpha(alpha) {}

	void drawSelf(bool draw, bool restore);

protected:
	const Graphics::Surface *_bitmap;
	bool _alpha;
};

}

#endif
