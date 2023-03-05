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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/credits.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/application.h"

namespace Tetraedge {

Credits::Credits() : _animCounter(0), _returnToOptions(false) {
}

void Credits::enter(bool returnToOptions) {
	_returnToOptions = returnToOptions;
	_animCounter = 0;
	_timer.start();
	// TODO: set _field0x50 = 0;
	_gui.load("menus/credits/credits.lua");
	Application *app = g_engine->getApplication();
	app->frontLayout().addChild(_gui.layoutChecked("menu"));

	Common::String musicPath = _gui.value("musicPath").toString();
	if (!app->music().isPlaying() || app->music().path() != musicPath) {
		app->music().stop();
		app->music().load(musicPath);
		app->music().play();
		app->music().volume(1.0f);
	}

	TeButtonLayout *bgbtn = _gui.buttonLayout("background");
	if (bgbtn) {
		bgbtn->onMouseClickValidated().add(this, &Credits::onQuitButton);
	}

	TeCurveAnim2<TeLayout, TeVector3f32> *posAnim = _gui.layoutPositionLinearAnimation("scrollTextPositionAnim");
	if (!posAnim)
		error("Credits gui - couldn't find scrollTextPositionAnim");
	posAnim->onFinished().add(this, &Credits::onAnimFinished);
	posAnim->_callbackObj = _gui.layoutChecked("text");
	posAnim->_callbackMethod = &TeLayout::setPosition;
	posAnim->play();

	TeCurveAnim2<TeLayout, TeVector3f32> *anchorAnim = _gui.layoutAnchorLinearAnimation("scrollTextAnchorAnim");
	if (!anchorAnim)
		error("Credits gui - couldn't find scrollTextAnchorAnim");

	anchorAnim->_callbackObj = _gui.layoutChecked("text");
	anchorAnim->_callbackMethod = &TeLayout::setAnchor;
	anchorAnim->play();

	if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
		TeCurveAnim2<TeLayout, TeVector3f32> *bgPosAnim = _gui.layoutPositionLinearAnimation("scrollBackgroundPositionAnim");
		if (!bgPosAnim)
			error("Credits gui - couldn't find scrollBackgroundPositionAnim");

		bgPosAnim->_callbackObj = _gui.layoutChecked("backgroundSprite");
		bgPosAnim->_callbackMethod = &TeLayout::setAnchor;
		bgPosAnim->play();
	}

	_curveAnim._runTimer.pausable(false);
	_curveAnim.stop();
	_curveAnim._startVal = TeColor(0xff, 0xff, 0xff, 0);
	_curveAnim._endVal = TeColor(0xff, 0xff, 0xff, 0xff);
	Common::Array<float> curve;
	curve.push_back(0.0f);
	curve.push_back(0.0f);
	curve.push_back(0.0f);
	curve.push_back(0.0f);
	curve.push_back(1.0f);
	_curveAnim._repeatCount = 1;
	_curveAnim.setCurve(curve);
	_curveAnim._duration = 12000;

	if (g_engine->gameType() == TetraedgeEngine::kSyberia) {
		TeLayout *backgrounds = _gui.layoutChecked("Backgrounds");
		if (_animCounter < backgrounds->childCount()) {
			TeSpriteLayout *bgchild = dynamic_cast<TeSpriteLayout *>(backgrounds->child(_animCounter));
			if (!bgchild)
				error("Child of backgrounds is not a TeSpriteLayout");
			_curveAnim._callbackObj = bgchild;
			_curveAnim._callbackMethod = &TeLayout::setColor;
			_curveAnim.play();
			bgchild->setVisible(true);
			const Common::String bgAnimName = bgchild->name() + "Anim";
			TeCurveAnim2<TeLayout, TeVector3f32> *bgPosAnim = _gui.layoutPositionLinearAnimation(bgAnimName);
			if (!bgPosAnim)
				error("Couldn't find bg position anim %s", bgAnimName.c_str());
			bgPosAnim->_callbackObj = bgchild;
			bgPosAnim->_callbackMethod = &TeLayout::setPosition;
			bgPosAnim->play();
		}
	}
	_curveAnim.onFinished().add(this, &Credits::onBackgroundAnimFinished);
}

void Credits::leave() {
	_curveAnim.stop();
	for (auto &anim : _gui.layoutPositionLinearAnimations()) {
		anim._value->stop();
	}
	if (_gui.loaded()) {
		Application *app = g_engine->getApplication();
		app->captureFade();
		app->frontLayout().removeChild(_gui.layoutChecked("menu"));
		_timer.stop();
		_gui.unload();
		if (_returnToOptions) {
			error("TODO: Implement returning to options menu");
		} else {
			// WORKAROUND: Ensure game is left before opening menu to
			// stop inventory button appearing in menu.
			g_engine->getGame()->leave(true);
			app->mainMenu().enter();
		}
		app->fade();
		_curveAnim.onFinished().remove(this, &Credits::onBackgroundAnimFinished);
	}
}

bool Credits::onAnimFinished() {
	leave();
	return false;
}

bool Credits::onBackgroundAnimFinished() {
	_animCounter++;
	TeLayout *backgrounds = _gui.layoutChecked("Backgrounds");
	if (_animCounter < backgrounds->childCount()) {
		TeSpriteLayout *bgchild = dynamic_cast<TeSpriteLayout *>(backgrounds->child(_animCounter));
		if (!bgchild)
			error("Children of credits Backgrounds should be Sprites.");
		_curveAnim._callbackObj = bgchild;
		_curveAnim._callbackMethod = &TeLayout::setColor;
		_curveAnim.play();
		bgchild->setVisible(true);
		const Common::String bgAnimName = bgchild->name() + "Anim";
		TeCurveAnim2<TeLayout, TeVector3f32> *bgPosAnim = _gui.layoutPositionLinearAnimation(bgAnimName);
		if (!bgPosAnim)
			error("Couldn't find bg position anim %s", bgAnimName.c_str());
		bgPosAnim->_callbackObj = bgchild;
		bgPosAnim->_callbackMethod = &TeLayout::setPosition;
		bgPosAnim->play();
	}
	return false;
}

bool Credits::onPadButtonUp(uint button) {
	// Original calls this function here but it seems unnecessary?
	//TeLayout *buttonsLayout = _gui.layout("buttons");
	if (button & 2) // TODO; which button is 2?
		leave();
	return false;
}

bool Credits::onQuitButton() {
	TeCurveAnim2<TeLayout, TeVector3f32> *anim1 = _gui.layoutPositionLinearAnimation("scrollTextPositionAnim");
	anim1->stop();
	TeCurveAnim2<TeLayout, TeVector3f32> *anim2 = _gui.layoutAnchorLinearAnimation("scrollTextAnchorAnim");
	anim2->stop();
	leave();
	return true;
}

} // end namespace Tetraedge
