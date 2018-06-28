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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SLUDGE_TALK_H
#define SLUDGE_TALK_H

#include "sludge/sprites.h"

namespace Sludge {

struct ObjectType;

struct SpeechLine {
	Common::String textLine;
	int x;
};

typedef Common::List<SpeechLine *> SpeechLineList;

struct SpeechStruct {
	OnScreenPerson *currentTalker;
	SpeechLineList allSpeech;
	int speechY, lastFile, lookWhosTalking;
	SpritePalette talkCol;
};

class SpeechManager {
public:
	SpeechManager(SludgeEngine *vm) : _vm(vm) { init(); }
	~SpeechManager() { kill(); }

	void init();
	void kill();

	int wrapSpeech(const Common::String &theText, int objT, int sampleFile, bool);
	void display();

	int isThereAnySpeechGoingOn();
	bool isCurrentTalker(OnScreenPerson *person) { return person == _speech->currentTalker; }
	int getLastSpeechSound();

	// setters & getters
	void setObjFontColour(ObjectType *t);
	void setSpeechSpeed(float speed) { _speechSpeed = speed; }
	float getSpeechSpeed() { return _speechSpeed; }
	void setSpeechMode(int speechMode) { _speechMode = speechMode; }

	// load & save
	void save(Common::WriteStream *stream);
	bool load(Common::SeekableReadStream *stream);

	// freeze & restore
	void freeze(FrozenStuffStruct *frozenStuff);
	void restore(FrozenStuffStruct *frozenStuff);

private:
	SludgeEngine *_vm;
	int _speechMode;
	SpeechStruct *_speech;
	float _speechSpeed;

	void addSpeechLine(const Common::String &theLine, int x, int &offset);
	int wrapSpeechXY(const Common::String &theText, int x, int y, int wrap, int sampleFile);
	int wrapSpeechPerson(const Common::String &theText, OnScreenPerson &thePerson, int sampleFile, bool animPerson);
};

} // End of namespace Sludge

#endif
