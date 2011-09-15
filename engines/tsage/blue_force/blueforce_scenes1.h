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
 */

#ifndef TSAGE_BLUEFORCE_SCENES1_H
#define TSAGE_BLUEFORCE_SCENES1_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene100: public SceneExt {
	/* Support classes */
	class Text: public SceneText {
	public:
		virtual Common::String getClassName() { return "BF100Text"; }
		virtual void dispatch();
	};

	/* Actions */
	class Action1: public ActionExt {
	private:
		void setTextStrings(const Common::String &msg1, const Common::String &msg2, Action *action);
	public:
		Text _sceneText1;
		SceneText _sceneText2;
		int _textHeight;

		virtual Common::String getClassName() { return "BF100Action1"; }
		virtual void synchronize(Serializer &s) {
			ActionExt::synchronize(s);
			s.syncAsSint16LE(_textHeight);
		}
		virtual void signal();
	};
	class Action2: public ActionExt {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	ScenePalette _scenePalette;
	NamedObject _object1, _object2, _object3, _object4, _object5;
	int _index;

	Scene100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene109: public PalettedScene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};

	/* Texts */
	class Text: public SceneText {
	public:
		Action *_action;
		uint32 _frameNumber;
		int _diff;
	public:
		Text();
		void setup(const Common::String &msg, Action *action);

		virtual Common::String getClassName() { return "BF109Text"; }
		virtual void synchronize(Serializer &s);
		virtual void dispatch();
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SequenceManager _sequenceManager4, _sequenceManager5, _sequenceManager6;
	SequenceManager _sequenceManager7, _sequenceManager8;
	SceneObject _object1, _object2, _protaginist2, _protaginist1, _object5;
	SceneObject _drunk, _object7, _bartender, _object9, _object10;
	Text _text;
	Action1 _action1;
	Action _action2, _action3;
public:
	Scene109();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene190: public SceneExt {
	/* Objects */
	class Object4: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	FollowerObject _object1;
	NamedObject _object2, _object3;
	Object4 _object4;
	Item1 _item1;
	Item2 _item2;
	NamedHotspot _item3, _item4, _item5, _item6;
	NamedHotspot _item7, _item8, _item9, _item10;
	Exit _exit;
	Action1 _action1;
	ASoundExt _sound;
	SpeakerGameText _speaker;
	bool _fieldB52;

	Scene190();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void synchronize(Serializer &s) {
		SceneExt::synchronize(s);
		s.syncAsSint16LE(_fieldB52);
	}
};	

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
