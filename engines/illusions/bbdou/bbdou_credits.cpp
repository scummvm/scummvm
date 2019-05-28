/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_credits.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/textdrawer.h"
#include "illusions/time.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

BbdouCredits::BbdouCredits(IllusionsEngine_BBDOU *vm)
	: _vm(vm) {
}

BbdouCredits::~BbdouCredits() {
}

void BbdouCredits::start(uint32 endSignalPropertyId, float speedModifier) {
	_endSignalPropertyId = endSignalPropertyId;
	_currFontId = 0x120004;
	_currLineIndex = 1;
	_split = false;
	// convertTextData();
	initCreditsItems();
	createCreditsThread(speedModifier);
}

void BbdouCredits::stop() {
	freeCreditsItems();
}

void BbdouCredits::drawNextLine() {
	uint leftIndex, rightIndex;

	if (!readNextLine(leftIndex, rightIndex)) {
		_vm->_scriptResource->_properties.set(_endSignalPropertyId, true);
		return;
	}

	if (leftIndex) {
		const char *leftText = getText(leftIndex);
		if (leftText && strlen(leftText) != 0) {
			uint32 objectId = getNextFreeObjectId();
			int alignment = rightIndex ? 1 : 2;
			drawTextToControl(objectId, leftText, alignment);
		}
	}

	if (rightIndex) {
		const char *rightText = getText(rightIndex);
		if (rightText && strlen(rightText) != 0) {
			uint32 objectId = getNextFreeObjectId();
			drawTextToControl(objectId, rightText, 4);
		}
	}

}

void charToWChar(const char *text, uint16 *wtext, uint size) {
	while (*text != 0 && size > 1) {
		*wtext++ = (byte)*text++;
		/*
		byte c = (byte)*text++;
		if (c > 127) c = 32;
		*wtext = c;
		debug("%04X", *wtext);
		++wtext;
		*/
		--size;
	}
	*wtext++ = 0;
}

void BbdouCredits::drawTextToControl(uint32 objectId, const char *text, uint alignment) {
	uint16 wtext[128];
	charToWChar(text, wtext, ARRAYSIZE(wtext));

	Control *control = _vm->getObjectControl(objectId);
	FontResource *font = _vm->_dict->findFont(_currFontId);
	TextDrawer textDrawer;
	WidthHeight dimensions;
	uint16 *outText;
	control->getActorFrameDimensions(dimensions);
	control->fillActor(0);
	textDrawer.wrapText(font, wtext, &dimensions, Common::Point(0, 0), alignment, outText);
	textDrawer.drawText(_vm->_screen, control->_actor->_surface, 0, 0);
	control->_actor->_flags |= Illusions::ACTOR_FLAG_4000;

}

bool BbdouCredits::readNextLine(uint &leftIndex, uint &rightIndex) {
	bool done = false;
	int textLines = 0;
	leftIndex = 0;
	rightIndex = 0;

	do {
		uint lineIndex = _currLineIndex++;
		const char *text = getText(lineIndex);
		if (text[0] == '@') {
			const char *command = text + 1;
			if (!strcmp(command, "end"))
				done = true;
			if (!strcmp(command, "bold"))
				_currFontId = 0x120005;
			else if (!strcmp(command, "normal"))
				_currFontId = 0x120004;
			else if (!strcmp(command, "center"))
				_split = false;
			else if (!strcmp(command, "split"))
				_split = true;
			else
				done = true;
		} else {
			++textLines;
			if (!_split) {
				leftIndex = lineIndex;
				done = true;
			} else if (textLines > 1) {
				rightIndex = lineIndex;
				done = true;
			} else {
				leftIndex = lineIndex;
			}
		}
	} while (!done);

	return textLines > 0;
}

void BbdouCredits::initCreditsItems() {
	for (uint i = 0; i < kCreditsItemsCount; ++i) {
		uint32 objectId = _vm->_controls->newTempObjectId();
		_vm->_controls->placeActor(0x00050188, Common::Point(320, 480), 0x00060BE1, objectId, 0);
		Control *control = _vm->_dict->getObjectControl(objectId);
		control->startSequenceActor(0x60BE2, 2, 0);
		_items[i].isUsed = false;
		_items[i].objectId = objectId;
	}
}

void BbdouCredits::freeCreditsItems() {
	for (uint i = 0; i < kCreditsItemsCount; ++i) {
		Control *control = _vm->_dict->getObjectControl(_items[i].objectId);
		control->disappearActor();
	}
}

uint32 BbdouCredits::getNextFreeObjectId() {
	for (uint i = 0; i < kCreditsItemsCount; ++i) {
		if (!_items[i].isUsed) {
			_items[i].isUsed = true;
			return _items[i].objectId;
		}
	}
	return 0;
}

void BbdouCredits::removeText(uint32 objectId) {
	for (uint i = 0; i < kCreditsItemsCount; ++i) {
		if (_items[i].objectId == objectId) {
			_items[i].isUsed = false;
			resetObjectPos(objectId);
		}
	}
}

void BbdouCredits::resetObjectPos(uint32 objectId) {
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->setActorPosition(Common::Point(320, 480));
}

void BbdouCredits::createCreditsThread(float speedModifier) {
	uint32 tempThreadId = _vm->newTempThreadId();
	CreditsThread *creditsThread = new CreditsThread(_vm, this, tempThreadId, speedModifier);
	_vm->_threads->startThread(creditsThread);
}

void BbdouCredits::updateTexts(int yIncr) {
	for (uint i = 0; i < kCreditsItemsCount; ++i) {
		if (_items[i].isUsed) {
			Control *control = _vm->_dict->getObjectControl(_items[i].objectId);
			Common::Point pt = control->getActorPosition();
			pt.y += yIncr;
			control->setActorPosition(pt);
			if (pt.y <= 0)
				removeText(_items[i].objectId);
		}
	}
}

// CreditsThread

CreditsThread::CreditsThread(IllusionsEngine_BBDOU *vm, BbdouCredits *credits, uint32 threadId, float speedModifier)
	: Thread(vm, threadId, 0, 0), _speedModifier(speedModifier), _lastFraction(0.0), _credits(credits) {
	_type = kTTSpecialThread;
	_lastUpdateTime = getCurrentTime();
}

int CreditsThread::onUpdate() {
	uint32 currTime = getCurrentTime();
	float fltDelta = (currTime - _lastUpdateTime) * _speedModifier + _lastFraction;
	int delta = (int)fltDelta;
	_lastFraction = fltDelta - delta;
	if (delta != 0)
		_credits->updateTexts(-delta);
	_lastUpdateTime = currTime;
	return 2;
}

void CreditsThread::onNotify() {
	_lastUpdateTime = getCurrentTime();
}

void CreditsThread::onResume() {
	onNotify();
}

void CreditsThread::onTerminated() {
	_credits->stop();
}

} // End of namespace Illusions
