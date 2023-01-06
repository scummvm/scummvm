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
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

BonusMenu::BonusMenu() {
}

void BonusMenu::enter(const Common::String &scriptName) {
	bool loaded = load(scriptName);
	if (!loaded)
		error("BonusMenu::enter: failed to load %s", scriptName.c_str());
	Application *app = g_engine->getApplication();
	app->frontLayout().addChild(layoutChecked("menu"));

	buttonLayoutChecked("quitButton")->onMouseClickValidated().add(this, &BonusMenu::onQuitButton);

	int btnNo = 0;
	while (true) {
		const Common::String btnNoStr = Common::String::format("%d", btnNo);
		TeButtonLayout *btn = buttonLayout(btnNoStr);
		if (!btn)
			break;
		SaveButton *saveBtn = new SaveButton(btn, btnNoStr);
		_saveButtons.push_back(saveBtn);

		// TODO: Finish this.

		btnNo++;
	}

	btnNo = 0;
	while( true ) {
		const Common::String btnNoStr = Common::String::format("slot%d", btnNo);
		TeLayout *l = layout(btnNoStr);
		if (!l)
			break;

		if (btnNo < (int)_saveButtons.size()) {
			l->addChild(_saveButtons[btnNo]);
		}
		btnNo = btnNo + 1;
	}

	TeButtonLayout *slideBtn = buttonLayoutChecked("slideButton");
	slideBtn->onButtonChangedToStateDownSignal().add(this, &BonusMenu::onSlideButtonDown);

	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.add(this, &BonusMenu::onMouseMove);

	_pageNo = 0;

	TeButtonLayout *leftBtn = buttonLayout("leftButton");
	if (leftBtn)
		leftBtn->onMouseClickValidated().add(this, &BonusMenu::onLeftButton);

	TeButtonLayout *rightBtn = buttonLayout("rightButton");
	if (rightBtn)
		rightBtn->onMouseClickValidated().add(this, &BonusMenu::onRightButton);

	// TODO: more stuff here with "text" values (also finish loop above)
	warning("TODO: Finish BonusMenu::enter(%s)", scriptName.c_str());

	TeButtonLayout *pictureBtn = buttonLayout("fullScreenPictureButton");
	if (pictureBtn) {
		pictureBtn->onMouseClickValidated().add(this, &BonusMenu::onPictureButton);
	}
}

void BonusMenu::enter() {
	error("TODO: implement BonusMenu::enter()");
}

void BonusMenu::leave() {
	for (auto *s : _saveButtons) {
		delete s;
	}
	_saveButtons.clear();
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.remove(this, &BonusMenu::onMouseMove);
	TeLuaGUI::unload();
}

bool BonusMenu::onLeftButton() {
	TeCurveAnim2<TeLayout, TeVector3f32> *slideAnim = layoutPositionLinearAnimation("slideAnimation");

	if (!slideAnim->_runTimer.running() && _pageNo != 0) {
		TeLayout *slotsLayout = layout("slots");
		TeVector3f32 slotsPos = slotsLayout->userPosition();
		slideAnim->_startVal = slotsPos;
		slotsPos.x() += value("slideTranslation").toFloat64();
		slideAnim->_endVal = slotsPos;

		slideAnim->_callbackObj = layoutChecked("slots");
		slideAnim->_callbackMethod = &TeLayout::setPosition;
		slideAnim->play();

		_pageNo--;

		buttonLayoutChecked("slideButton")->reset();

		warning("TODO: Finish BonusMenu::onLeftButton");
		// TODO: Set values depending on whether saves exist (lines 95-120)
	}

	return false;
}

bool BonusMenu::onMouseMove(const Common::Point &pt) {
	TeButtonLayout *slideLayout = buttonLayout("slideButton");
	if (slideLayout->state() == TeButtonLayout::BUTTON_STATE_DOWN) {
		TeCurveAnim2<TeLayout, TeVector3f32> *slideAnim = layoutPositionLinearAnimation("slideAnimation");
		if (!slideAnim->_runTimer.running()) {
			warning("TODO: implement BonusMenu::onMouseMove");
		}
	}

	return false;
}

bool BonusMenu::onPictureButton() {
	TeButtonLayout *btn = buttonLayoutChecked("menu");
	btn->setVisible(true);

	Application *app = g_engine->getApplication();
	TeSpriteLayout *pictureLayout = spriteLayoutChecked("fullScreenPictureLayout");
	app->frontLayout().removeChild(pictureLayout);
	pictureLayout->setVisible(true);

	return true;
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
	TeCurveAnim2<TeLayout, TeVector3f32> *slideAnim = layoutPositionLinearAnimation("slideAnimation");

	if (!slideAnim->_runTimer.running() && _pageNo < (int)_saveButtons.size() - 1) {
		TeLayout *slotsLayout = layout("slots");
		TeVector3f32 slotsPos = slotsLayout->userPosition();
		slideAnim->_startVal = slotsPos;
		slotsPos.x() -= value("slideTranslation").toFloat64();
		slideAnim->_endVal = slotsPos;

		slideAnim->_callbackObj = layoutChecked("slots");
		slideAnim->_callbackMethod = &TeLayout::setPosition;
		slideAnim->play();

		_pageNo++;

		buttonLayoutChecked("slideButton")->reset();

		// TODO: Set values depending on whether saves exist (lines 95-120)
		warning("TODO: Finish BonusMenu::onRightButton");
	}
	return false;
}

bool BonusMenu::onSlideButtonDown() {
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	TeVector2s32 mousepos = inputmgr->lastMousePos();
	_slideBtnStartMousePos = mousepos;
	buttonLayoutChecked("slideButton")->setClickPassThrough(true);
	return false;
}

BonusMenu::SaveButton::SaveButton(TeButtonLayout *btn, const Common::String &name) {
	setName(name);
	btn->setEnable(true);
	// TODO: Add child something here?
	btn->onMouseClickValidated().add(this, &BonusMenu::SaveButton::onLoadSave);
}

Common::String BonusMenu::SaveButton::path() const {
	return Common::String("Backup/") + name() + ".xml";
}

bool BonusMenu::SaveButton::onLoadSave() {
	error("TODO: implement BonusMenu::SaveButton::onLoadSave");
}

} // end namespace Tetraedge
