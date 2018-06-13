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

#include "engines/stark/ui/menu/dialogmenu.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/diary.h"

namespace Stark {

DialogScreen::DialogScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "DiaryLog", Screen::kScreenDialog) {
}

DialogScreen::~DialogScreen() {
}

void DialogScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"Return",
			CLICK_HANDLER(DialogScreen, backHandler),
			nullptr));

	_widgets.push_back(new StaticLocationWidget(
			"Back",
			CLICK_HANDLER(DialogScreen, backHandler),
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexBack",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexNext",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogBack",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"Index",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogNext",
			nullptr,
			nullptr));
	
	for (uint i = 1; i < _widgets.size(); ++i) {
		_widgets[i]->setupSounds(0, 1);
	}
			
	_widgets.push_back(new StaticLocationWidget(
			"IndexFrame",
			nullptr,
			nullptr));
			
	_widgets.push_back(new StaticLocationWidget(
			"LogFrame",
			nullptr,
			nullptr));
}

void DialogScreen::backHandler() {
	StarkUserInterface->backPrevScreen();
}

} // End of namespace Stark
