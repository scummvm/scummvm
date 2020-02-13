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

#ifndef TSAGE_BLUEFORCE_SCENES5_H
#define TSAGE_BLUEFORCE_SCENES5_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene550: public SceneExt {
	/* Objects */
	class Lyle: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class CaravanDoor: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Vechile: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public ActionExt {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	Lyle _lyle;
	CaravanDoor _caravanDoor;
	Vechile _vechile;
	NamedHotspot _item1, _item2, _item3;
	SpeakerGameText _gameTextSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	Action1 _action1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene551: public Scene550 {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class DrunkStanding: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Drunk: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class PatrolCarTrunk: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class TrunkInset: public FocusObject {
	public:
		void remove() override;
		bool startAction(CursorType action, Event &event) override;
	};
	class TrunkKits: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Harrison: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Item4: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action2: public Action {
	public:
		void signal() override;
	};
public:
	SpeakerDrunk _drunkSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerGiggles _gigglesSpeaker;
	Vechile _vechile2;
	DrunkStanding _drunkStanding;
	Drunk _drunk;
	PatrolCarTrunk _patrolCarTrunk;
	TrunkInset _trunkInset;
	TrunkKits _trunkKits;
	Harrison _harrison;
	NamedObject _object11, _object12, _object13;
	SceneObject _object14, _object15;
	Item4 _item4;
	Action2 _action2;
	int _field1CD0, _field1CD2;

	Scene551();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene560: public SceneExt {
	/* Objects */
	class DeskChair: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Box: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SafeInset: public FocusObject {
		/* Items */
		class Item: public NamedHotspotExt {
		public:
			bool startAction(CursorType action, Event &event) override;
		};
	public:
		NamedObject _digit0, _digit1, _digit2;
		Item _item1, _item2, _item3, _item4, _item5, _item6;
		Visage _cursorVisage;

		Common::String getClassName() override { return "Scene560_SafeInset"; }
		void postInit(SceneObjectList *OwnerList = NULL) override;
		void remove() override;
		void signal() override;
		void process(Event &event) override;
	};
	class Nickel: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class BoxInset: public FocusObject {
		/* Items */
		class Item1: public NamedHotspot {
		public:
			bool startAction(CursorType action, Event &event) override;
		};
	public:
		Item1 _item1;

		Common::String getClassName() override { return "Scene560_BoxInset"; }
		void postInit(SceneObjectList *OwnerList = NULL) override;
		void remove() override;
	};

	/* Item groups */
	class PicturePart: public NamedHotspotExt {
	public:
		Common::String getClassName() override { return "Scene560_PicturePart"; }
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Computer: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
	class Action3: public Action {
	public:
		void signal() override;
	};
public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	SpeakerGameText _gameTextSpeaker;
	DeskChair _deskChair;
	Box _box;
	SafeInset _safeInset;
	Nickel _nickel;
	BoxInset _boxInset;
	NamedObject _object6;
	PicturePart _picture1, _picture2, _picture3, _picture4;
	Computer _computer;
	NamedHotspot _chair, _lamp, _item4, _trophy, _waterColors, _fileCabinets;
	NamedHotspot _certificate, _bookcase, _desk, _carpet, _item12, _office;
	ASound _sound1;
	bool _field380;
	bool _field11EA;
	Common::Point _destPosition;

	Scene560();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene570: public SceneExt {
	/* Custom classes */
	class PasswordEntry: public EventHandler {
	private:
		void checkPassword();
	public:
		SceneText _passwordText, _entryText;
		Common::String _passwordStr, _entryBuffer;

		PasswordEntry();
		Common::String getClassName() override { return "Scene570_PasswordEntry"; }
		void synchronize(Serializer &s) override;
		void postInit(SceneObjectList *OwnerList = NULL) override;
		void process(Event &event) override;
	};
	class Icon: public NamedObject {
	public:
		SceneText _sceneText;
		int _iconId, _folderId, _parentFolderId;
		Common::String _text;

		Icon();
		Common::String getClassName() override { return "Scene570_Custom2"; }
		void synchronize(Serializer &s) override;
		void remove() override;
		bool startAction(CursorType action, Event &event) override;

		void setDetails(int iconId, int folderId, int parentFolderId, const Common::String &msg);
	};
	class IconManager: public EventHandler {
	public:
		NamedObject _object1;
		SynchronizedList<Icon *> _list;
		int _mode, _selectedFolder, _fieldAA, _fieldAC;

		IconManager();
		void remove() override;

		void setup(int mode);
		void hideList();
		void refreshList();
		void addItem(Icon *item);
	};

	/* Objects */
	class PowerSwitch: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class PrinterIcon: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object3: public FocusObject {
	public:
		void remove() override;
	};

	/* Items */
	class FloppyDrive: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	PasswordEntry _passwordEntry;
	PowerSwitch _powerSwitch;
	PrinterIcon _printerIcon;
	Object3 _object3;
	NamedObjectExt _object4;
	Icon _folder1, _folder2, _folder3, _folder4;
	Icon _icon1, _icon2, _icon3, _icon4, _icon5;
	Icon _icon6, _icon7, _icon8, _icon9;
	IconManager _iconManager;
	FloppyDrive _floppyDrive;
	NamedHotspot _monitor, _item3, _case, _keyboard, _desk;
	NamedHotspot _item7, _printer, _window, _plant, _item11;
	ASound _sound1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene580: public SceneExt {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Door: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Lyle: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager;
	Vechile _vechile;
	Door _door;
	Lyle _lyle;
	NamedHotspot _item1;
	SpeakerGameText _gameTextSpeaker;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene590: public PalettedScene {
	/* Objects */
	class Laura: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Skip: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Exit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	Laura _laura;
	Skip _skip;
	SceneObject _object3;
	Exit _exit;
	NamedHotspot _item2, _item3, _item4, _item5, _item6, _item7;
	NamedHotspot _item8, _item9, _item10, _item11, _item12;
	SpeakerSkip _skipSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLaura _lauraSpeaker;
	ASoundExt _sound1;
	Action1 _action1;
	Action2 _action2;
	int _field17DC, _field17DE, _stripNumber, _field17E2;

	Scene590();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
