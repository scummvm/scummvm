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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/converse.h $
 * $Id: converse.h 230 2011-02-12 06:57:31Z dreammaster $
 *
 */

#ifndef TSAGE_CONVERSE_H
#define TSAGE_CONVERSE_H

#include "tsage/core.h"
#include "tsage/dialogs.h"

namespace tSage {

class StripCallback: public EventHandler {
public:
	virtual void stripCallback(int v) = 0;
};

class SequenceManager: public Action {
private:
	void setup();
	uint16 getNextValue();
	void setMessage(int resNum, int lineNum, int colour, const Common::Point &pt, int width);
	SequenceManager *globalManager();
public:
	SceneText _sceneText;
	int _resNum;
	uint _sequenceOffset;
	bool _keepActive;
	int _field24;
	int _field26;
	Common::Array<byte> _sequenceData;
	int _objectIndex;
	SceneObject *_sceneObject;
	SceneObject *_objectList[6];
	SoundHandler _SoundHandler;
public:
	SequenceManager();

	virtual Common::String getClassName() { return "SequenceManager"; }
	virtual void synchronise(Serialiser &s);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void attached(EventHandler *newOwner, EventHandler *fmt, va_list va);
};


class Speaker: public EventHandler {
public:
	Rect _fieldA;
	Action *_field18;
	Common::String _speakerName;
	int _newSceneNumber;
	int _oldSceneNumber;
	SceneObjectList _objectList;
	Rect _sceneBounds;
	SceneText _sceneText;
	int _textWidth;
	Common::Point _textPos;
	int _fontNumber;
	TextAlign _textMode;
	int _colour1, _colour2, _colour3;
	bool _hideObjects;
public:
	Speaker();

	virtual Common::String getClassName() { return "Speaker"; }
	virtual void synchronise(Serialiser &s);
	virtual void remove();
	virtual void proc12(Action *action);
	virtual void setText(const Common::String &msg);
	virtual void removeText();

	void setTextPos(const Common::Point &pt) { _textPos = pt; }
};

class SpeakerGameText: public Speaker {
public:
	SpeakerGameText();

	virtual Common::String getClassName() { return "SpeakerGameText"; }
};

class ScreenSpeaker: public Speaker {
public:
	SceneItem *_npc;
public:
	ScreenSpeaker();

	virtual Common::String getClassName() { return "ScreenSpeaker"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerGText: public Speaker {
public:
	SceneObject _sceneObject;
public:
	SpeakerGText();

	virtual Common::String getClassName() { return "SpeakerGText"; }
	virtual void setText(const Common::String &msg);
	virtual void removeText();
};	

class SpeakerOText: public SpeakerGText {
public:
	SpeakerOText();

	virtual Common::String getClassName() { return "SpeakerOText"; }
};	

class SpeakerSText: public ScreenSpeaker {
public:
	SpeakerSText();

	virtual Common::String getClassName() { return "SpeakerSText"; }
};

class SpeakerQText: public ScreenSpeaker {
public:
	SpeakerQText();

	virtual Common::String getClassName() { return "SpeakerQText"; }
};

class SpeakerAction: public Action {
public:
	virtual void signal();

	virtual Common::String getClassName() { return "SpeakerAction"; }
};

class AnimatedSpeaker: public Speaker {
public:
	SceneObject _object1;
	SceneObject _object2;
	SpeakerAction _speakerAction;
public:
	virtual Common::String getClassName() { return "AnimatedSpeaker"; }
	virtual void removeText();
};

class SpeakerQL: public AnimatedSpeaker {
public:
	SpeakerQL();

	virtual Common::String getClassName() { return "SpeakerQL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSR: public AnimatedSpeaker {
public:
	SceneObject _object3;
public:
	SpeakerSR();

	virtual Common::String getClassName() { return "SpeakerSR"; }
	void setText(const Common::String &msg);
};

class SpeakerSL: public AnimatedSpeaker {
public:
	SpeakerSL();

	virtual void setText(const Common::String &msg);
};

class SpeakerQR: public AnimatedSpeaker {
public:
	SpeakerQR();

	void setText(const Common::String &msg);
};

class ChoiceEntry {
public:
	Common::String _msg;
	Rect _bounds;

	ChoiceEntry() {}
	ChoiceEntry(const Common::String &msg, const Rect &bounds) { 
		_msg = msg;
		_bounds = bounds;
	}
};

class ConversationChoiceDialog: public ModalDialog {
public:
	int _stdColour;
	int _highlightColour;
	int _fontNumber;
	int _savedFgColour;
	int _savedFontNumber;
	Common::Array<ChoiceEntry> _choiceList;
	uint _selectedIndex;
public:
	ConversationChoiceDialog();

	void setColours(int stdColour, int highlightColour) {
		_stdColour = stdColour;
		_highlightColour = highlightColour;
	}
	void setFontNumber(int fontNum) { _fontNumber = fontNum; }
	int execute(const StringArray &choiceList);

	virtual void draw();
};

class Obj0A: public Serialisable {
public:
	int _id;
	uint _scriptOffset;

	virtual void synchronise(Serialiser &s) { 
		s.syncAsSint32LE(_id);
		s.syncAsUint32LE(_scriptOffset);
	}
};

#define OBJ44_LIST_SIZE 5

class Obj44: public Serialisable {
public:
	int _id;
	int _field2[OBJ44_LIST_SIZE];
	Obj0A _list[OBJ44_LIST_SIZE];
	uint _speakerOffset;
public:
	void load(const byte *dataP);
	virtual void synchronise(Serialiser &s);
};

class StripManager: public Action {
private:
	void reset();
	void load();
	Speaker *getSpeaker(const char *speakerName);
	int getNewIndex(int newId);
public:
	int _stripNum;
	int _obj44Index;
	int _field20;
	int _sceneNumber;
	Rect _sceneBounds;
	ConversationChoiceDialog _choiceDialog;
	Common::Array<Speaker *> _speakerList;
	StripCallback *_callbackObject;
	Speaker *_activeSpeaker;
	bool _textShown;
	bool _field2E6;
	int _field2E8;
	Common::Array<Obj44> _obj44List;
	Common::Array<byte> _script;
public:
	StripManager();
	virtual ~StripManager();

	virtual void synchronise(Serialiser &s);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);

	void start(int stripNum, EventHandler *owner, StripCallback *callback = NULL);
	void setCallback(StripCallback *callback) { _callbackObject = callback; }
	void setColours(int stdColour, int highlightColour) { _choiceDialog.setColours(stdColour, highlightColour); }
	void setFontNumber(int fontNum) { _choiceDialog.setFontNumber(fontNum); }
	void addSpeaker(Speaker *speaker);
};

} // End of namespace tSage

#endif
