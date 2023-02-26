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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/dialog2.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/character.h"
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
	Game *game = g_engine->getGame();
	if (_dialogs.empty()) {
		game->showMarkers(false);
		_gui.buttonLayoutChecked("dialogLockButton")->setVisible(false);
		return;
	}

	TeButtonLayout *dialog = _gui.buttonLayoutChecked("dialog");
	if (dialog->anchor().y() >= 1.0) {
		TeCurveAnim2<TeLayout, TeVector3f32> *anim = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
		anim->stop();
		anim->play();
	} else {
		dialog->setSizeType(CoordinatesType::ABSOLUTE);
		TeButtonLayout *lockBtn = _gui.buttonLayoutChecked("dialogLockButton");
		dialog->setSize(lockBtn->size());
		_currentDialogData = _dialogs.front();
		_dialogs.remove_at(0);
		const Common::String formatStr = _gui.value("textFormat").toString();
		Common::String formattedVal = Common::String::format(formatStr.c_str(), _currentDialogData._stringVal.c_str());
		_gui.textLayout("text")->setText(formattedVal);
		_music.load(_currentDialogData._sound.toString());
		_music.setChannelName("dialog");
		_music.play();
		if (!_currentDialogData._charname.empty()) {
			Character *c = game->scene().character(_currentDialogData._charname);
			if (!c) {
				warning("[Dialog2::launchNextDialog] Character's \"%s\" doesn't exist", _currentDialogData._charname.c_str());
			} else {
				if (_currentDialogData._animBlend == 0.0f) {
					if (!c->setAnimation(_currentDialogData._animfile, false, true))
						error("[Dialog2::launchNextDialog] Character's animation \"%s\" doesn't exist for the character\"%s\"",
								_currentDialogData._animfile.c_str(), _currentDialogData._charname.c_str());
				} else {
					if (!c->blendAnimation(_currentDialogData._animfile, _currentDialogData._animBlend, false, true))
						error("[Dialog2::launchNextDialog] Character's animation \"%s\" doesn't exist for the character\"%s\"",
								_currentDialogData._animfile.c_str(), _currentDialogData._charname.c_str());
				}
			}
		}
		lockBtn->setVisible(true);
		TeCurveAnim2<TeLayout, TeVector3f32> *anim = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
		anim->stop();
		anim->play();
		_minimumTimeTimer.start();
		_minimumTimeTimer.setAlarmIn(1500000);
	}
}

void Dialog2::load() {
	setName("dialog2");
	setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 usersz = userSize();
	setSize(TeVector3f32(1.0f, 1.0f, usersz.z()));
	size(); // refresh size? seems to do nothing with result
	_music.repeat(false);
	_gui.load("menus/dialog.lua");
	size(); // refresh size? seems to do nothing with result
	TeButtonLayout *dialogLockBtn = _gui.buttonLayoutChecked("dialogLockButton");

	dialogLockBtn->setVisible(false);
	addChild(dialogLockBtn);
	size(); // refresh size? seems to do nothing with result again.

	TeButtonLayout *dialogBtn = _gui.buttonLayoutChecked("dialog");

	dialogBtn->onMouseClickValidated().add(this, &Dialog2::onSkipButton);

	TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
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
	Common::String param(_currentDialogData._name);
	launchNextDialog();
	_onAnimationDownFinishedSignal.call(param);
	return false;
}

bool Dialog2::onAnimationUpFinished() {
	// Seems like this just prints a debug value??
	//TeButtonLayout *dialogButton = _gui.buttonLayout("dialog");
	//dialogButton->anchor();
	return false;
}

bool Dialog2::onMinimumTimeTimer() {
	_minimumTimeTimer.stop();
	if (!_music.isPlaying())
		startDownAnimation();
	return false;
}

bool Dialog2::onSkipButton() {
	const TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	if (!dialogAnimUp->_runTimer.running()) {
		const TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
		if (!dialogAnimDown->_runTimer.running()) {
			startDownAnimation();
			_music.stop();
		}
	}
	// Divergence from original: don't let clicks through on skip operation.
	return true;
}

bool Dialog2::onSoundFinished() {
	if (!_minimumTimeTimer.running())
		startDownAnimation();
	return false;
}

void Dialog2::pushDialog(const Common::String &name, const Common::String &textVal, const Common::String &sound, int param_4) {
	error("TODO: Implement Dialog2::pushDialog 3 param");
}

void Dialog2::pushDialog(const Common::String &name, const Common::String &textVal, const Common::String &sound,
						 const Common::String &charname, const Common::String &animfile, float animBlend) {
	DialogData data;
	data._name = name;
	data._stringVal = textVal;
	data._charname = charname;
	data._animfile = animfile;
	data._sound = Common::Path("sounds/Dialogs").join(sound);
	data._animBlend = animBlend;
	if (sound.empty()) {
		data._sound = Common::Path("sounds/dialogs/silence5s.ogg");
	}
	_dialogs.push_back(data);
	if (_dialogs.size() == 1) {
		Game *game = g_engine->getGame();
		game->showMarkers(true);
	}
	if (!_music.isPlaying())
		launchNextDialog();
}

//void saveToBackup(TiXmlNode *node)

void Dialog2::startDownAnimation() {
	_minimumTimeTimer.stop();
	TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
	dialogAnimDown->play();
}

void Dialog2::unload() {
	if (!_gui.loaded())
		return;
	TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimUp = _gui.layoutAnchorLinearAnimation("dialogAnimationUp");
	dialogAnimUp->stop();
	TeCurveAnim2<TeLayout, TeVector3f32> *dialogAnimDown = _gui.layoutAnchorLinearAnimation("dialogAnimationDown");
	dialogAnimDown->stop();
	_music.close();
	_gui.unload();

	_dialogs.clear();
	_minimumTimeTimer.stop();
}

} // end namespace Tetraedge
