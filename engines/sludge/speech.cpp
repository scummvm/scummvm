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

#include "common/system.h"

#include "sludge/fonttext.h"
#include "sludge/graphics.h"
#include "sludge/freeze.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/speech.h"

namespace Sludge {

void SpeechManager::init() {
	_speechMode = 0;
	_speechSpeed = 1;
	_speech = new SpeechStruct;
	if (checkNew(_speech)) {
		_speech->currentTalker = NULL;
		_speech->allSpeech.clear();
		_speech->speechY = 0;
		_speech->lastFile = -1;
	}
}

void SpeechManager::kill() {
	if (!_speech)
		return;

	if (_speech->lastFile != -1) {
		_vm->_soundMan->huntKillSound(_speech->lastFile);
		_speech->lastFile = -1;
	}

	if (_speech->currentTalker) {
		_speech->currentTalker->makeSilent();
		_speech->currentTalker = nullptr;
	}

	for (SpeechLineList::iterator it = _speech->allSpeech.begin(); it != _speech->allSpeech.end(); ++it) {
		SpeechLine *killMe = *it;
		delete killMe;
		killMe = nullptr;
	}
	_speech->allSpeech.clear();
}

void SpeechManager::setObjFontColour(ObjectType *t) {
	_speech->talkCol.setColor(t->r, t->g, t->b);
}

void SpeechManager::addSpeechLine(const Common::String &theLine, int x, int &offset) {
	float cameraZoom = g_sludge->_gfxMan->getCamZoom();
	int halfWidth = (g_sludge->_txtMan->stringWidth(theLine) >> 1) / cameraZoom;
	int xx1 = x - (halfWidth);
	int xx2 = x + (halfWidth);

	// Create new speech line
	SpeechLine *newLine = new SpeechLine;
	checkNew(newLine);
	newLine->textLine.clear();
	newLine->textLine = theLine;
	newLine->x = xx1;
	_speech->allSpeech.push_front(newLine);

	// Calculate offset
	if ((xx1 < 5) && (offset < (5 - xx1))) {
		offset = 5 - xx1;
	} else if ((xx2 >= ((float) g_system->getWidth() / cameraZoom) - 5)
			&& (offset > (((float) g_system->getWidth() / cameraZoom) - 5 - xx2))) {
		offset = ((float) g_system->getWidth() / cameraZoom) - 5 - xx2;
	}
}

int SpeechManager::isThereAnySpeechGoingOn() {
	return _speech->allSpeech.empty() ? -1 : _speech->lookWhosTalking;
}

int SpeechManager::getLastSpeechSound() {
	return _vm->_soundMan->findInSoundCache(_speech->lastFile);
}

int SpeechManager::wrapSpeechXY(const Common::String &theText, int x, int y, int wrap, int sampleFile) {
	float cameraZoom = g_sludge->_gfxMan->getCamZoom();
	int fontHeight = g_sludge->_txtMan->getFontHeight();
	int cameraY = g_sludge->_gfxMan->getCamY();

	int a, offset = 0;

	kill();

	int speechTime = (theText.size() + 20) * _speechSpeed;
	if (speechTime < 1)
		speechTime = 1;
	if (sampleFile != -1) {
		if (_speechMode >= 1) {
			if (g_sludge->_soundMan->startSound(sampleFile, false)) {
				speechTime = -10;
				_speech->lastFile = sampleFile;
				if (_speechMode == 2) return -10;
			}

		}
	}
	_speech->speechY = y;

	char *tmp, *txt;
	tmp = txt = createCString(theText);
	while ((int)strlen(txt) > wrap) {
		a = wrap;
		while (txt[a] != ' ') {
			a--;
			if (a == 0) {
				a = wrap;
				break;
			}
		}
		txt[a] = 0;
		addSpeechLine(txt, x, offset);
		txt[a] = ' ';
		txt += a + 1;
		y -= fontHeight / cameraZoom;
	}
	addSpeechLine(txt, x, offset);
	y -= fontHeight / cameraZoom;
	delete []tmp;

	if (y < 0)
		_speech->speechY -= y;
	else if (_speech->speechY > cameraY + (float) (g_system->getHeight() - fontHeight / 3) / cameraZoom)
		_speech->speechY = cameraY
				+ (float) (g_system->getHeight() - fontHeight / 3) / cameraZoom;

	if (offset) {
		for (SpeechLineList::iterator it = _speech->allSpeech.begin(); it != _speech->allSpeech.end(); ++it) {
			(*it)->x += offset;
		}
	}
	return speechTime;
}

int SpeechManager::wrapSpeechPerson(const Common::String &theText, OnScreenPerson &thePerson, int sampleFile, bool animPerson) {
	int cameraX = g_sludge->_gfxMan->getCamX();
	int cameraY = g_sludge->_gfxMan->getCamY();
	int i = wrapSpeechXY(theText, thePerson.x - cameraX,
			thePerson.y - cameraY
					- (thePerson.scale * (thePerson.height - thePerson.floaty))
					- thePerson.thisType->speechGap,
			thePerson.thisType->wrapSpeech, sampleFile);
	if (animPerson) {
		thePerson.makeTalker();
		_speech->currentTalker = &thePerson;
	}
	return i;
}

int SpeechManager::wrapSpeech(const Common::String &theText, int objT, int sampleFile, bool animPerson) {
	int i;
	int cameraX = g_sludge->_gfxMan->getCamX();
	int cameraY = g_sludge->_gfxMan->getCamY();

	_speech->lookWhosTalking = objT;
	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objT);
	if (thisPerson) {
		setObjFontColour(thisPerson->thisType);
		i = wrapSpeechPerson(theText, *thisPerson, sampleFile, animPerson);
	} else {
		ScreenRegion *thisRegion = g_sludge->_regionMan->getRegionForObject(objT);
		if (thisRegion) {
			setObjFontColour(thisRegion->thisType);
			i = wrapSpeechXY(theText,
					((thisRegion->x1 + thisRegion->x2) >> 1) - cameraX,
					thisRegion->y1 - thisRegion->thisType->speechGap - cameraY,
					thisRegion->thisType->wrapSpeech, sampleFile);
		} else {
			ObjectType *temp = g_sludge->_objMan->findObjectType(objT);
			setObjFontColour(temp);
			i = wrapSpeechXY(theText, g_system->getWidth() >> 1, 10, temp->wrapSpeech,
					sampleFile);
		}
	}
	return i;
}

void SpeechManager::display() {
	float cameraZoom = g_sludge->_gfxMan->getCamZoom();
	int fontHeight = g_sludge->_txtMan->getFontHeight();
	int viewY = _speech->speechY;
	for (SpeechLineList::iterator it = _speech->allSpeech.begin(); it != _speech->allSpeech.end(); ++it) {
		g_sludge->_txtMan->pasteString((*it)->textLine, (*it)->x, viewY, _speech->talkCol);
		viewY -= fontHeight / cameraZoom;
	}
}

void SpeechManager::save(Common::WriteStream *stream) {
	stream->writeByte(_speechMode);
	stream->writeByte(_speech->talkCol.originalRed);
	stream->writeByte(_speech->talkCol.originalGreen);
	stream->writeByte(_speech->talkCol.originalBlue);

	stream->writeFloatLE(_speechSpeed);

	// Write y co-ordinate
	stream->writeUint16BE(_speech->speechY);

	// Write which character's talking
	stream->writeUint16BE(_speech->lookWhosTalking);
	if (_speech->currentTalker) {
		stream->writeByte(1);
		stream->writeUint16BE(_speech->currentTalker->thisType->objectNum);
	} else {
		stream->writeByte(0);
	}

	// Write what's being said
	for (SpeechLineList::iterator it = _speech->allSpeech.begin(); it != _speech->allSpeech.end(); ++it) {
		stream->writeByte(1);
		writeString((*it)->textLine, stream);
		stream->writeUint16BE((*it)->x);
	}
	stream->writeByte(0);
}

bool SpeechManager::load(Common::SeekableReadStream *stream) {
	// read speech mode
	_speechMode = stream->readByte();

	_speech->currentTalker = nullptr;
	kill();
	byte r = stream->readByte();
	byte g = stream->readByte();
	byte b = stream->readByte();
	_speech->talkCol.setColor(r, g, b);
	_speechSpeed = stream->readFloatLE();

	// Read y co-ordinate
	_speech->speechY = stream->readUint16BE();

	// Read which character's talking
	_speech->lookWhosTalking = stream->readUint16BE();

	if (stream->readByte()) {
		_speech->currentTalker = g_sludge->_peopleMan->findPerson(stream->readUint16BE());
	} else {
		_speech->currentTalker = NULL;
	}

	// Read what's being said
	_speech->lastFile = -1;
	while (stream->readByte()) {
		SpeechLine *newOne = new SpeechLine;
		if (!checkNew(newOne))
			return false;
		newOne->textLine = readString(stream);
		newOne->x = stream->readUint16BE();
		_speech->allSpeech.push_back(newOne);
	}
	return true;
}

void SpeechManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->speech = _speech;
	init();
}

void SpeechManager::restore(FrozenStuffStruct *frozenStuff) {
	kill();
	delete _speech;
	_speech = frozenStuff->speech;
}

} // End of namespace Sludge
