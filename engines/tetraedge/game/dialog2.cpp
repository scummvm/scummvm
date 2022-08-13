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

#include "tetraedge/game/dialog2.h"
#include "tetraedge/te/te_button_layout.h"

namespace Tetraedge {

Dialog2::Dialog2() {
	_music.onStopSignal().add(this, &Dialog2::onSoundFinished);
	_minimumTimeTimer.alarmSignal().add(this, &Dialog2::onMinimumTimeTimer);
}

bool Dialog2::isDialogPlaying() {
	TeButtonLayout *lockbtn = _gui.buttonLayout("dialogLockButton");

	if (lockbtn)
		return lockbtn->visible();

	return false;
}

void Dialog2::launchNextDialog() {
	error("TODO: Implement Dialog2::launchNextDialog.");
}

void Dialog2::load() {
	setName("dialog2");
	setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 usersz = userSize();
	setSize(TeVector3f32(1.0f, 1.0f, usersz.z()));
	size(); // refresh size.. seems to do nothing with result?
	_music.repeat(false);
	_gui.load("menus/dialog.lua");
	TeButtonLayout *dialogLockBtn = _gui.buttonLayoutChecked("dialogLockButton");

	dialogLockBtn->setVisible(false);
	addChild(dialogLockBtn);

	TeButtonLayout *dialogBtn = _gui.buttonLayoutChecked("dialog");
	dialogBtn->onMouseClickValidated().add(this, &Dialog2::onSkipButton);

	TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
	if (!dialogAnimUp || !dialogAnimDown)
		error("Dialog2::load: didn't get dialogAnimUp or dialogAnimationDown");

	dialogAnimUp->_callbackObj = dialogBtn;
	dialogAnimUp->_callbackMethod = &TeLayout::setAnchor;
	dialogAnimUp->onFinished().add(this, &Dialog2::onAnimationUpFinished);

	dialogAnimDown->_callbackObj = dialogBtn;
	dialogAnimDown->_callbackMethod = &TeLayout::setAnchor;
	dialogAnimDown->onFinished().add(this, &Dialog2::onAnimationDownFinished);
}

void Dialog2::loadFromBackup() {
	// seems to do nothing useful? just iterates the children..
}

bool Dialog2::onAnimationDownFinished() {
	Common::String param(_animDownFinishedResultString);
	launchNextDialog();
	_onAnimationDownFinishedSignal.call(param);
	return false;
}

bool Dialog2::onAnimationUpFinished() {
	// Seems like this just prints a debug value??
	TeButtonLayout *dialogButton = _gui.buttonLayout("dialog");
	dialogButton->anchor();
	return false;
}

bool Dialog2::onMinimumTimeTimer() {
	_minimumTimeTimer.stop();
	if (!_music.isPlaying())
		startDownAnimation();
	return false;
}

bool Dialog2::onSkipButton() {
	const TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	if (!dialogAnimUp->_runTimer._stopped) {
		const TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
		if (dialogAnimDown->_runTimer._stopped) {
			startDownAnimation();
			_music.stop();
		}
	}
	return false;
}

bool Dialog2::onSoundFinished() {
	if (_minimumTimeTimer._stopped)
		startDownAnimation();
	return false;
}

void Dialog2::pushDialog(const Common::String &param_1, const Common::String &param_2, const Common::String &param_3, int param_4) {
	error("TODO: Implement Dialog2::pushDialog");
}

void Dialog2::pushDialog(const Common::String &param_1, const Common::String &param_2, const Common::String &param_3,
						 const Common::String &param_4, const Common::String &param_5, float param_6) {
	error("TODO: Implement Dialog2::pushDialog");
}

//void saveToBackup(TiXmlNode *node)

void Dialog2::startDownAnimation() {
	_minimumTimeTimer.stop();
	TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
	dialogAnimDown->play();
}

void Dialog2::unload() {
	TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	dialogAnimUp->stop();
	TeCurveAnim2<TeLayout,TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
	dialogAnimDown->stop();
	_music.close();
	_gui.unload();
	error("TODO: Finish Dialog2::unload");
	//_dialogDataList.clear();
	//_minimumTimeTimer.stop();
}

} // end namespace Tetraedge
