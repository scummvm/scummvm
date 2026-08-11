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

#include "tetraedge/game/application.h"
#include "tetraedge/game/question2.h"
#include "tetraedge/game/game.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

Question2::Question2() : _entered(false) {
}

Question2::~Question2() {
	// Should have been cleared in leave() but just in case..
	for (Answer *answer : _answers) {
		delete answer;
	}
}

void Question2::enter() {
	TeButtonLayout *backgroundButton = _gui.buttonLayoutChecked("background");
	backgroundButton->setVisible(true);
	g_engine->getGame()->showMarkers(true);
	_entered = true;
}

void Question2::leave() {
	_entered = false;
	TeLayout *background = _gui.layout("background");
	if (!background)
		return;

	background->setVisible(false);

	TeSpriteLayout *calepinLayout = _gui.spriteLayout("Calepin");
	if (!calepinLayout)
		error("Question2::leave: can't find Calepin.");

	for (Answer *answer : _answers) {
		TeLayout *alayout = answer->layout();
		if (alayout == nullptr)
			continue;
		calepinLayout->removeChild(alayout);
		answer->unload();
		// TODO: original uses TeObject::deleteLater here.. should we
		// do the same? why defer it?
		delete answer;
	}
	_answers.clear();
}

void Question2::load() {
	// TODO: set field_0xd0 = 0
	setName("dialog2");
	setSizeType(RELATIVE_TO_PARENT);
	const TeVector3f32 usersz = userSize();
	setSize(TeVector3f32(1.0, 1.0, usersz.z()));
	if (!g_engine->gameIsAmerzone())
		_gui.load("menus/answer.lua");
	else
		_gui.load("GUI/answer.lua");

	TeButtonLayout *backgroundButton = _gui.buttonLayout("background");
	if (backgroundButton) {
		addChild(backgroundButton);
		backgroundButton->setVisible(false);
	}
	size();
}

bool Question2::onAnswerValidated(Answer &answer) {
	_onAnswerSignal.call(answer._str);
	g_engine->getGame()->showMarkers(false);
	leave();
	return false;
}

void Question2::pushAnswer(const Common::String &name, const Common::String &locName, const Common::Path &path) {
	Answer *answer = new Answer();
	answer->load(name, locName, path);
	answer->_onButtonValidatedSignal.add(this, &Question2::onAnswerValidated);
	TeButtonLayout *blayout = dynamic_cast<TeButtonLayout *>(answer->layout());
	if (!blayout)
		error("No Answer button layout after loading %s!", path.toString(Common::Path::kNativeSeparator).c_str());

	blayout->setState(TeButtonLayout::BUTTON_STATE_UP);
	_answers.push_back(answer);

	float xpos;
	blayout->setSizeType(RELATIVE_TO_PARENT);
	blayout->setPositionType(RELATIVE_TO_PARENT);
	if (!path.baseName().contains("Cal_FIN.lua") && !path.baseName().contains("Cal_FIN.data")) {
		blayout->setSize(TeVector3f32(0.45f, 0.065f, 1.0f));
		xpos = 0.3f;
	} else {
		blayout->setSize(TeVector3f32(0.15f, 0.065f, 1.0f));
		xpos = 0.15f;
	}
	blayout->setPosition(TeVector3f32(xpos, _answers.size() * 0.08f + 0.06f, 1.0f));
	blayout->upLayout()->setSizeType(RELATIVE_TO_PARENT);
	blayout->upLayout()->setSize(TeVector3f32(1.0f, 1.0f, 1.0f));
	blayout->downLayout()->setSizeType(RELATIVE_TO_PARENT);
	blayout->downLayout()->setSize(TeVector3f32(1.0f, 1.0f, 1.0f));

	TeSpriteLayout *calepinLayout = _gui.spriteLayoutChecked("Calepin");
	calepinLayout->addChild(blayout);

	enter();
}

void Question2::unload() {
	leave();
	_gui.unload();
}

TeLayout *Question2::Answer::layout() {
	return _gui.layout("answer");
}

void Question2::Answer::load(const Common::String &name, const Common::String &locName, const Common::Path &path) {
	_str = name;
	_gui.load(path);
	TeButtonLayout *answerButton = _gui.buttonLayout("answer");
	if (answerButton) {
		answerButton->onMouseClickValidated().add(this, &Question2::Answer::onButtonValidated);
		answerButton->setIgnoreMouseEvents(false);
	}
}

void Question2::Answer::unload() {
	_gui.unload();
}

bool Question2::Answer::onButtonValidated() {
	_onButtonValidatedSignal.call(*this);
	return false;
}

} // end namespace Tetraedge
