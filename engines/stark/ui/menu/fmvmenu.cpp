/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/menu/fmvmenu.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/location.h"

#include "engines/stark/visual/text.h"

namespace Stark {

const Color FMVWidget::_textColorHovered = Color(0x1E, 0x1E, 0x96);
const Color FMVWidget::_textColorDefault = Color(0x00, 0x00, 0x00);

// Hard-coded parameters in case cannot retrieve the format rectangle
Common::Point FMVMenuScreen::_formatRectPos(202, 61);
int FMVMenuScreen::_fontHeight(16);
uint FMVMenuScreen::_fmvPerPage(18);

FMVMenuScreen::FMVMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryFMV", Screen::kScreenFMVMenu),
		_fmvWidgets() {
}

FMVMenuScreen::~FMVMenuScreen() {
	freeFMVWidgets();
}

void FMVMenuScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(FMVMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(FMVMenuScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"PreviousPage",
			CLICK_HANDLER(FMVMenuScreen, prevPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"NextPage",
			CLICK_HANDLER(FMVMenuScreen, nextPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	// Acquire data for FMVWidget from the format rectangle

	int formatRectHeight = 379;

	Resources::Location *location = StarkStaticProvider->getLocation();
	Gfx::RenderEntry *formatRect = location->getRenderEntryByName("FormatRectangle");

	if (formatRect) {
		_formatRectPos = formatRect->getPosition();
		formatRectHeight = formatRect->getText()->getTargetHeight();

		// The format rectangle contains one line,
		// which can be used to retrieve the font's height
		Common::Rect textRect = formatRect->getText()->getRect();
		_fontHeight = textRect.bottom - textRect.top;

		_fmvPerPage = formatRectHeight / (_fontHeight + 4);
	}

	_maxPage = StarkDiary->countFMV() / _fmvPerPage;
	changePage(0);
}

void FMVMenuScreen::close() {
	freeFMVWidgets();
	StaticLocationScreen::close();
}

void FMVMenuScreen::onScreenChanged() {
	StaticLocationScreen::onScreenChanged();
	for (uint i = 0; i < _fmvWidgets.size(); ++i) {
		_fmvWidgets[i]->onScreenChanged();
	}
}

void FMVMenuScreen::onMouseMove(const Common::Point &pos) {
	StaticLocationScreen::onMouseMove(pos);
	for (uint i = 0; i < _fmvWidgets.size(); ++i) {
		_fmvWidgets[i]->onMouseMove(pos);
	}
}

void FMVMenuScreen::onClick(const Common::Point &pos) {
	StaticLocationScreen::onClick(pos);
	for (uint i = 0; i < _fmvWidgets.size(); ++i) {
		if (_fmvWidgets[i]->isMouseInside(pos)) {
			_fmvWidgets[i]->onClick();
			return;
		}
	}
}

void FMVMenuScreen::onRender() {
	StaticLocationScreen::onRender();
	for (uint i = 0; i < _fmvWidgets.size(); ++i) {
		_fmvWidgets[i]->render();
	}
}

void FMVMenuScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void FMVMenuScreen::freeFMVWidgets() {
	for (uint i = 0; i < _fmvWidgets.size(); ++i) {
		delete _fmvWidgets[i];
	}
	_fmvWidgets.clear();
}

void FMVMenuScreen::loadFMVWidgets(uint page) {
	uint start = page * _fmvPerPage;
	uint end = start + _fmvPerPage;
	end = end < StarkDiary->countFMV() ? end : StarkDiary->countFMV();

	for (uint i = start; i < end; ++i) {
		_fmvWidgets.push_back(new FMVWidget(_gfx, i));
	}
}

void FMVMenuScreen::changePage(uint page) {
	assert(page <= _maxPage);

	freeFMVWidgets();
	loadFMVWidgets(page);

	_widgets[kWidgetPrevious]->setVisible(page > 0);
	_widgets[kWidgetNext]->setVisible(page < _maxPage);

	_page = page;
}

FMVWidget::FMVWidget(Gfx::Driver *gfx, uint fmvIndex) :
		_filename(StarkDiary->getFMVFilename(fmvIndex)),
		_title(gfx) {
	_title.setText(StarkDiary->getFMVTitle(fmvIndex));
	_title.setColor(_textColorDefault);
	_title.setFont(FontProvider::kCustomFont, 3);

	Common::Rect rect = _title.getRect();
	_width = rect.right - rect.left;

	_position.x = FMVMenuScreen::_formatRectPos.x;
	_position.y = FMVMenuScreen::_formatRectPos.y + 
	              (fmvIndex % FMVMenuScreen::_fmvPerPage) * (FMVMenuScreen::_fontHeight + 4);
}

void FMVWidget::onClick() {
	StarkUserInterface->requestFMVPlayback(_filename);
}

bool FMVWidget::isMouseInside(const Common::Point &mousePos) const {
	return mousePos.x >= _position.x && mousePos.x <= _position.x + _width &&
		   mousePos.y >= _position.y && mousePos.y <= _position.y + FMVMenuScreen::_fontHeight;
}

} // End of namespace Stark
