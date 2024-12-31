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
#include "tetraedge/game/game.h"
#include "tetraedge/game/notifier.h"
#include "tetraedge/te/te_layout.h"

namespace Tetraedge {

Notifier::Notifier() {
}

static const char *notifyLayoutName() {
	return g_engine->gameIsAmerzone() ? "notify" : "notifier";
}

void Notifier::launchNextnotifier() {
	TeCurveAnim2<Te3DObject2, TeColor> *colorAnim = _gui.colorLinearAnimation("fadeIn");
	assert(colorAnim);
	if (colorAnim->_runTimer.running())
		return;

	colorAnim = _gui.colorLinearAnimation("fadeOut");
	if (!colorAnim->_runTimer.running()) {
		colorAnim = _gui.colorLinearAnimation("visible");
		bool abort = true;
		if (!colorAnim->_runTimer.running()) {
			abort = _notifierDataArray.empty();
		}
		if (abort)
			return;
	}

	unload();
	load();

	if (_notifierDataArray.empty())
		return;

	Common::String textformat = _gui.value("textFormat").toString();
	Common::String formattedName;
	if (!textformat.empty())
		formattedName = Common::String::format(textformat.c_str(), _notifierDataArray[0]._name.c_str());
	else
		formattedName = _notifierDataArray[0]._name;

	TeITextLayout *text = _gui.textLayout("text");
	text->setText(formattedName);

	if (!_notifierDataArray[0]._imgpath.empty()) {
		assert(!g_engine->gameIsAmerzone());
		_gui.spriteLayoutChecked("image")->load(_notifierDataArray[0]._imgpath);
	}

	_gui.layoutChecked(notifyLayoutName())->setVisible(true);

	colorAnim = _gui.colorLinearAnimation("fadeIn");
	colorAnim->_callbackObj = _gui.layoutChecked("sprite");
	colorAnim->play();

	colorAnim = g_engine->gameIsAmerzone() ? nullptr : _gui.colorLinearAnimation("fadeInImage");
	if (colorAnim) {
		colorAnim->_callbackObj = _gui.layoutChecked("image");
		colorAnim->play();
	}

	_notifierDataArray.remove_at(0);
}

void Notifier::load() {
	const char *luaPath = g_engine->gameIsAmerzone() ? "GUI/Notify.lua" : "menus/Notifier.lua";
	_gui.load(luaPath);
	TeLayout *notifierLayout = _gui.layoutChecked(notifyLayoutName());
	g_engine->getGame()->addNoScale2Child(notifierLayout);
	notifierLayout->setVisible(false);

	TeCurveAnim2<Te3DObject2, TeColor> *fadeIn = _gui.colorLinearAnimation("fadeIn");
	fadeIn->onFinished().add(this, &Notifier::onFadeInFinished);

	TeCurveAnim2<Te3DObject2, TeColor> *visible = _gui.colorLinearAnimation("visible");
	visible->onFinished().add(this, &Notifier::onVisibleFinished);

	TeCurveAnim2<Te3DObject2, TeColor> *fadeOut = _gui.colorLinearAnimation("fadeOut");
	fadeOut->onFinished().add(this, &Notifier::onFadeOutFinished);
}

bool Notifier::onFadeInFinished() {
	TeCurveAnim2<Te3DObject2, TeColor> *colorAnim = _gui.colorLinearAnimation("visible");
	colorAnim->_callbackObj = _gui.layout("sprite");
	colorAnim->play();

	colorAnim = g_engine->gameIsAmerzone() ? nullptr : _gui.colorLinearAnimation("visibleImage");
	if (colorAnim) {
		colorAnim->_callbackObj = _gui.layout("image");
		colorAnim->play();
	}

	return false;
}

bool Notifier::onFadeOutFinished() {
	TeLayout *notifierLayout = _gui.layoutChecked(notifyLayoutName());
	notifierLayout->setVisible(false);
	launchNextnotifier();
	return false;
}

bool Notifier::onVisibleFinished() {
	TeCurveAnim2<Te3DObject2, TeColor> *colorAnim = _gui.colorLinearAnimation("fadeOut");
	colorAnim->_callbackObj = _gui.layout("sprite");
	colorAnim->play();

	colorAnim = g_engine->gameIsAmerzone() ? nullptr : _gui.colorLinearAnimation("fadeOutImage");
	if (colorAnim) {
		colorAnim->_callbackObj = _gui.layout("image");
		colorAnim->play();
	}
	return false;
}

void Notifier::push(const Common::String &name, const Common::Path &imgpath) {
	notifierData n = {name, imgpath};
	_notifierDataArray.push_back(n);
	launchNextnotifier();
}

void Notifier::unload() {
	TeLayout *layout = _gui.layoutChecked(notifyLayoutName());
	g_engine->getGame()->removeNoScale2Child(layout);
	_gui.unload();
}



} // end namespace Tetraedge
