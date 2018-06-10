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
		StaticLocationScreen(gfx, cursor, "DiaryPages", Screen::kScreenDiaryPages) {
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
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new StaticLocationWidget(
			"Next",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);

	_widgets.push_back(new DiaryWidget(0));
}

void DiaryPagesScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

DiaryWidget::DiaryWidget(uint diaryIndex):
		StaticLocationWidget(nullptr, nullptr, nullptr) {
	Resources::Location *location = StarkStaticProvider->getLocation();
	Resources::Layer *layer = location->getLayerByName(StarkDiary->getDiary(diaryIndex));

	// Diary page layer contains only one item, the text
	_renderEntry = layer->listRenderEntries()[0];
}

} // End of namespace Stark
