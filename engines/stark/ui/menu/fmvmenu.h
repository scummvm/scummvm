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

#ifndef STARK_UI_MENU_FMV_MENU_H
#define STARK_UI_MENU_FMV_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"
#include "engines/stark/visual/text.h"

namespace Stark {

class FMVWidget;

/**
 * The video replay menu
 */
class FMVMenuScreen : public StaticLocationScreen {
public:
	FMVMenuScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~FMVMenuScreen();

	// StaticLocationScreen API
	void open() override;
	void close() override;
	void onScreenChanged() override;

protected:
	// Window API
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRender() override;

private:
	enum WidgetIndex {
		kWidgetPrevious = 3,
		kWidgetNext = 4
	};

	Common::Array<FMVWidget *> _fmvWidgets;

	Common::Point _formatRectPos;
	int _fontHeight;
	uint _fmvPerPage;

	// Count from zero
	uint _page, _maxPage;

	void backHandler();
	void prevPageHandler() { changePage(_page - 1); }
	void nextPageHandler() { changePage(_page + 1); }

	void freeFMVWidgets();
	void loadFMVWidgets(uint page);
	void changePage(uint page);
};

/**
 * The widget for FMV entry, specifically built for FMVMenuScreen
 */
class FMVWidget {
public:
	FMVWidget(Gfx::Driver *gfx, uint fmvIndex);
	~FMVWidget() {}

	void render() { _title.render(_position); }

	bool isMouseInside(const Common::Point &mousePos) const;

	void onMouseMove(const Common::Point &mousePos) {
		setTextColor(isMouseInside(mousePos) ? _textColorHovered : _textColorDefault);
	}

	void onClick();

	void setTextColor(const Color &color) { _title.setColor(color); }

	void onScreenChanged() { _title.resetTexture(); }

private:
	const Color _textColorHovered = Color(0x1E, 0x1E, 0x96);
	const Color _textColorDefault = Color(0x00, 0x00, 0x00);

	Common::Point _formatRectPos;
	int _fontHeight;
	uint _fmvPerPage;

	const Common::String &_filename;
	VisualText _title;

	int _width;
	Common::Point _position;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_FMV_MENU_H
