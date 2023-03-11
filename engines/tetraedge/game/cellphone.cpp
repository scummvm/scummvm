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
#include "tetraedge/te/te_core.h"

#include "tetraedge/game/cellphone.h"
#include "tetraedge/te/te_text_layout.h"

namespace Tetraedge {

Cellphone::Cellphone() : _nextNumber(0) {
}

bool Cellphone::addNumber(const Common::String &num) {
	for (const Common::String &addedNum : _addedNumbers) {
		if (addedNum == num)
			return false;
	}

	TeTextLayout *layout = new TeTextLayout();
	const Common::String namePrefix("numRepertoire");
	layout->setName(namePrefix + num);
	layout->setSizeType(RELATIVE_TO_PARENT);
	layout->setAnchor(TeVector3f32(0.5f, 0.0f, 0.0f));
	// WORKAROUND: Original uses (1.0, 1.0, 1.0) here but then the text area is too high.
	layout->setSize(TeVector3f32(1.0f, 0.6f, 0.0f));
	layout->setPosition(TeVector3f32(0.5f, 0.08f, 0.0f));
	layout->setTextSizeType(1);
	layout->setTextSizeProportionalToWidth(46);
	Common::String val("Unknown");
	const Common::String *locNum = g_engine->getCore()->loc()->text(num);
	if (locNum)
		val = *locNum;

	layout->setText(_gui.value("textAttributs").toString() + val);
	layout->setVisible(true);
	_textLayoutArray.push_back(layout);
	_addedNumbers.push_back(num);

	TeSpriteLayout *sprite = _gui.spriteLayoutChecked("numRepertoire");
	sprite->addChild(layout);
	return true;
}

void Cellphone::currentPage(int offset) {
	if (_textLayoutArray.empty())
		return;

	_nextNumber = offset;
	TeLayout *repertoire = _gui.layoutChecked("numRepertoire");
	for (int i = 0; i < repertoire->childCount(); i++) {
		repertoire->child(i)->setVisible(i == offset);
	}
}

void Cellphone::enter() {
	_gui.buttonLayoutChecked("background")->setVisible(true);
	currentPage(_nextNumber);
}

void Cellphone::leave() {
	if (!_gui.loaded())
		return;

	_gui.buttonLayoutChecked("background")->setVisible(false);
	for (TeTextLayout *text : _textLayoutArray) {
		text->deleteLater();
	}
	_textLayoutArray.clear();
	_addedNumbers.clear();
}

void Cellphone::load() {
	_nextNumber = 0;
	TeButtonLayout *btnlayout;
	_gui.load("menus/cellphone.lua");
	btnlayout = _gui.buttonLayoutChecked("haut");
	btnlayout->onMouseClickValidated().add(this, &Cellphone::onPreviousNumber);
	btnlayout = _gui.buttonLayoutChecked("bas");
	btnlayout->onMouseClickValidated().add(this, &Cellphone::onNextNumber);
	btnlayout = _gui.buttonLayoutChecked("appeler");
	btnlayout->onMouseClickValidated().add(this, &Cellphone::onCallNumberValidated);
	btnlayout = _gui.buttonLayoutChecked("fermer");
	btnlayout->onMouseClickValidated().add(this, &Cellphone::onCloseButtonValidated);
	btnlayout = _gui.buttonLayoutChecked("background");
	btnlayout->setVisible(false);
}

void Cellphone::loadFromBackup(const Common::XMLParser::ParserNode *node) {
	error("TODO: implement Cellphone::loadFromBackup");
	/*
	 basic algorithm:
	child = node->lastChild;
	while (child != nullptr) {
		if (child->type == ELEMENT) {
			if (if child->userData == "Number") {
				addNumber(this, child->getAttribute("num"));
			}
		}
	child = child->prev;
	}*/
}

bool Cellphone::onCallNumberValidated() {
	_onCallNumberSignal.call(_addedNumbers[_nextNumber]);
	return false;
}

bool Cellphone::onCloseButtonValidated() {
	_gui.buttonLayoutChecked("background")->setVisible(false);
	return false;
}

bool Cellphone::onNextNumber() {
	uint numoffset = _nextNumber + 1;
	if (numoffset < _textLayoutArray.size()) {
		currentPage(numoffset);
	}
	return false;
}

bool Cellphone::onPreviousNumber() {
	int numoffset = _nextNumber - 1;
	if (numoffset >= 0)
		currentPage(numoffset);
	return false;
}

void Cellphone::saveToBackup(Common::XMLParser::ParserNode *xmlnode) {
	error("TODO: implement Cellphone::saveToBackup");
}

void Cellphone::setVisible(bool visible) {
	_gui.buttonLayoutChecked("background")->setVisible(visible);
}

void Cellphone::unload() {
	leave();
	_gui.unload();
}

Common::Error Cellphone::syncState(Common::Serializer &s) {
	Common::Array<Common::String> numbers = _addedNumbers;
	uint numElems = numbers.size();
	s.syncAsUint32LE(numElems);
	if (numElems > 1000)
		error("Unexpected number of elems syncing cellphone");
	numbers.resize(numElems);
	for (uint i = 0; i < numElems; i++) {
		s.syncString(numbers[i]);
	}
	if (s.isLoading()) {
		if (!_addedNumbers.empty())
			leave();

		for (const auto &num : numbers)
			addNumber(num);
	}
	return Common::kNoError;
}

} // end namespace Tetraedge
