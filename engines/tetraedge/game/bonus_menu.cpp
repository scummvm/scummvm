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

#include "tetraedge/game/bonus_menu.h"

#include "common/textconsole.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
//#include "tetraedge/te/te_input_manager"

namespace Tetraedge {

BonusMenu::BonusMenu() {
}

void BonusMenu::enter(const Common::String &scriptName) {
	error("TODO: implement me.");
}

void BonusMenu::enter() {
	error("TODO: implement me.");
}

void BonusMenu::leave() {
	for (auto *s : _saveButtons) {
		delete s;
	}
	_saveButtons.clear();
	warning("TODO: remove oMouseMove signal from inputmgr.");
	TeLuaGUI::unload();
}

bool BonusMenu::onLeftButton() {
	error("TODO: implement me.");
	return false;
}

bool BonusMenu::onMouseMove() {
	error("TODO: implement me.");
	return false;
}

bool BonusMenu::onPictureButton() {
	error("TODO: implement me.");
	return false;
}

bool BonusMenu::onQuitButton() {
	Application *app = g_engine->getApplication();
	assert(app);

	app->captureFade();
	leave();
	app->globalBonusMenu().enter();
	app->fade();
	return true;
}

bool BonusMenu::onRightButton() {
	error("TODO: implement me.");
	return false;
}

bool BonusMenu::onSideButtonDown() {
	/*
	TeInputManager *inputmgr = g_engine->getInputManager();
	 TeVector2s32 mousepos = inputmgr->getMousePos();
	 _slideBtnStartMousePos = mousepos;
	 buttonLayout("slideButton");
	 // TODO set some flag in super (TeLuaGUI)
	*/
	error("TODO: implement me.");
	return false;
}

Common::String BonusMenu::SaveButton::path() const {
	return Common::String("Backup/") + name() + ".xml";
}

bool BonusMenu::SaveButton::onLoadSave() {
	error("TODO: implement me.");
}

} // end namespace Tetraedge
