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

#include "engines/stark/ui/menu/diarypages.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/location.h"
#include "engines/stark/resources/layer.h"

#include "engines/stark/gfx/renderentry.h"

namespace Stark {

DiaryPagesScreen::DiaryPagesScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryPages", Screen::kScreenDiaryPages),
		_page(0) {
}

DiaryPagesScreen::~DiaryPagesScreen() {
}

void DiaryPagesScreen::open() {
	StaticLocationScreen::open();
	
	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(DiaryPagesScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"GoBack",
			CLICK_HANDLER(DiaryPagesScreen, backHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(DiaryPagesScreen, prevPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"Next",
			CLICK_HANDLER(DiaryPagesScreen, nextPageHandler),
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_page = StarkDiary->getPageIndex();

	if (StarkDiary->countDiary() > 0) {
		_widgets.push_back(new DiaryWidget(_page));
		_widgets[kWidgetBack]->setVisible(_page > 0);
		_widgets[kWidgetNext]->setVisible(_page < StarkDiary->countDiary() - 1);
	} else {
		_widgets[kWidgetBack]->setVisible(false);
		_widgets[kWidgetNext]->setVisible(false);
	}
}

void DiaryPagesScreen::close() {
	StarkDiary->setPageIndex(_page);
	StarkDiary->setDiaryAllRead();
	StaticLocationScreen::close();
}

void DiaryPagesScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

void DiaryPagesScreen::changePage(uint page) {
	assert(page < StarkDiary->countDiary());

	delete _widgets.back();
	_widgets.pop_back();
	_widgets.push_back(new DiaryWidget(page));

	_widgets[kWidgetBack]->setVisible(page > 0);
	_widgets[kWidgetNext]->setVisible(page < StarkDiary->countDiary() - 1);

	_page = page;
}

DiaryWidget::DiaryWidget(uint diaryIndex):
		StaticLocationWidget(nullptr, nullptr, nullptr) {
	Resources::Location *location = StarkStaticProvider->getLocation();
	Resources::Layer *layer = location->getLayerByName(StarkDiary->getDiary(diaryIndex));

	if (!layer) {
		debug("Unable to retrieve diary in layer %s.", StarkDiary->getDiary(diaryIndex).c_str());
		return;
	}

	// Diary page layer contains only one item, the text
	_renderEntry = layer->listRenderEntries()[0];
}

} // End of namespace Stark
