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

#ifndef TSAGE_RINGWORLD_SCENES10_H
#define TSAGE_RINGWORLD_SCENES10_H

#include "common/scummsys.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class SceneObject9150 : public SceneObject {
public:
	int _timer, _signalFlag;

	void synchronize(Serializer &s) override {
		SceneObject::synchronize(s);
		s.syncAsSint16LE(_timer);
		s.syncAsSint16LE(_signalFlag);
	}
	Common::String getClassName() override { return "SceneObject9150"; }
};

class Scene2 : public Scene {
public :
	int _sceneState;

	Scene2();
	void synchronize(Serializer &s) override {
		Scene::synchronize(s);
		s.syncAsSint16LE(_sceneState);
	}
};

class Object9350 : public SceneObject {
public:
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void draw() override;
};

class Scene9100 : public Scene {
	/* Items */
	class SceneHotspot1 : public NamedHotspot {
	public:
		void doAction(int action) override;
	};
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneHotspot1  _hotspotSlave;
	NamedHotspot _hotspotSoiledClothes;
	NamedHotspot _hotspotCleanedClothes;
	NamedHotspot _hotspotIsland;
	NamedHotspot _hotspotBoulders;
	NamedHotspot _hotspotTrees;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9150 : public Scene2 {
	class Object3 : public SceneObject9150 {
	public:
		void signal() override;
		void dispatch() override;
	};
public:
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SceneObject _object1;
	SceneObject _object2;
	Object3 _object3;
	NamedHotspot _sceneHotspot1;
	NamedHotspot _sceneHotspot2;
	NamedHotspot _sceneHotspot3;
	NamedHotspot _sceneHotspot4;
	NamedHotspot _sceneHotspot5;
	NamedHotspot _sceneHotspot6;
	NamedHotspot _sceneHotspot7;
	NamedHotspot _sceneHotspot8;
	NamedHotspot _sceneHotspot9;
	NamedHotspot _sceneHotspot10;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9200 : public Scene2 {
	class SceneHotspot1 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	Action _action1;
	SpeakerGText _speakerGText;
	SpeakerGR _speakerGR;
	SpeakerQText _speakerQText;
	ASound _soundHandler;
	SceneHotspot1 _hotspot1;
	NamedHotspot _hotspot2;
	NamedHotspot _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
	void process(Event &event) override;
};

class Scene9300 : public Scene {
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	NamedHotspot _hotspot1;
	NamedHotspot _hotspot2;
	NamedHotspot _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;
	NamedHotspot _hotspot9;
	NamedHotspot _hotspot10;
	NamedHotspot _hotspot11;
	NamedHotspot _hotspot12;
	NamedHotspot _hotspot13;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9350 : public Scene2 {
public:
	SequenceManager _sequenceManager;
	Object9350 _object1;
	SceneObject _object2;
	NamedHotspot _sceneHotspot1;
	NamedHotspot _sceneHotspot2;
	NamedHotspot _sceneHotspot3;
	NamedHotspot _sceneHotspot4;
	NamedHotspot _sceneHotspot5;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9360 : public Scene2 {
public:
	SequenceManager _sequenceManager;
	Action _action1;
	Object9350 _object1;
	NamedHotspot _hotspot1;
	NamedHotspot _hotspot2;
	NamedHotspot _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9400 : public Scene2 {
	class SceneHotspot7 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class SceneHotspot8 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};
public:
	Scene9400();
	SequenceManager _sequenceManager;
	Action _action1;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SpeakerOText _speakerOText;
	SpeakerOR _speakerOR;
	SpeakerQText _speakerQText;
	NamedHotspot _hotspot1;
	NamedHotspot _hotspot2;
	NamedHotspot _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	ASound _soundHandler;
	bool _hittingAnvil;
	SceneHotspot7 _hotspot7;
	SceneHotspot8 _hotspot8;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene9450 : public Scene2 {
	class Object2 : public SceneObject {
	public:
		void signal() override;
	};

	class Object3 : public SceneObject9150 {
	public:
		void dispatch() override;
	};

	class Hotspot1 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot3 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};
public:
	SceneObject _object1;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	Object2 _object2;
	SequenceManager _sequenceManager3;
	Object3 _object3;
	Hotspot1 _hotspot1;
	NamedHotspot _hotspot2;
	Hotspot3 _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;
	NamedHotspot _hotspot9;
	NamedHotspot _hotspot10;
	NamedHotspot _hotspot11;
	NamedHotspot _hotspot12;
	NamedHotspot _hotspot13;
	NamedHotspot _hotspot14;
	NamedHotspot _hotspot15;
	NamedHotspot _hotspot16;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9500 : public Scene2 {
	class Hotspot1 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot2 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot3 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot4 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

public:
	SequenceManager _sequenceManager;
	SceneObject _candle;
	SceneObject _object2;
	SceneObject _object3;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	Hotspot3 _hotspot3;
	Hotspot4 _hotspot4;
	Hotspot4 _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;
	NamedHotspot _hotspot9;
	NamedHotspot _hotspot10;
	NamedHotspot _hotspot11;
	NamedHotspot _hotspot12;
	NamedHotspot _hotspot13;
	NamedHotspot _hotspot14;
	NamedHotspot _hotspot15;
	NamedHotspot _hotspot16;
	NamedHotspot _hotspot17;
	NamedHotspot _hotspot18;
	NamedHotspot _hotspot19;
	NamedHotspot _hotspot20;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
	void process(Event &event) override;
};

class Scene9700 : public Scene2 {
	SequenceManager _sequenceManager;
	SceneObject _object1;
	NamedHotspot _sceneHotspot1;
	NamedHotspot _sceneHotspot2;
	NamedHotspot _sceneHotspot3;
	NamedHotspot _sceneHotspot4;
	NamedHotspot _sceneHotspot5;
	NamedHotspot _sceneHotspot6;
	GfxButton _gfxButton1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene9750 : public Scene {
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene9850 : public Scene {
	class Object6 : public SceneObject{
	public:
		void doAction(int action) override;
	};

	class Object7 : public SceneObjectExt{
	public:
		void doAction(int action) override;
	};

	class Hotspot12 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot14 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot16 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot17 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot18 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot19 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};

	class Hotspot20 : public NamedHotspot{
	public:
		void doAction(int action) override;
	};
public:
	SequenceManager _sequenceManager;
	SceneObject _objDoor;
	SceneObject _objCloak;
	SceneObject _objJacket;
	SceneObject _objTunic2;
	SceneObject _objLever;
	Object6 _objScimitar;
	Object7 _objSword;
	ASound _soundHandler;
	NamedHotspot _hotspot1;
	NamedHotspot _hotspot2;
	NamedHotspot _hotspot3;
	NamedHotspot _hotspot4;
	NamedHotspot _hotspot5;
	NamedHotspot _hotspot6;
	NamedHotspot _hotspot7;
	NamedHotspot _hotspot8;
	NamedHotspot _hotspot9;
	NamedHotspot _hotspot10;
	NamedHotspot _hotspot11;
	Hotspot12 _hotspot12;
	NamedHotspot _hotspot13;
	Hotspot14 _hotspot14;
	NamedHotspot _hotspot15;
	Hotspot16 _hotspot16;
	Hotspot17 _hotspot17;
	Hotspot18 _hotspot18;
	Hotspot19 _hotspot19;
	Hotspot20 _spotLever;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene9900 : public Scene {
	class strAction1 : public Action {
	public:
		SceneObject _object9;
		ScenePalette _palette1;

		void signal() override;
	};

	class strAction2 : public Action {
	public:
		SceneText _txtArray1[2];
		SceneText _txtArray2[2];
		int _lineNum, _txtArray1Index, _var3;

		void signal() override;
		void dispatch() override;
		void synchronize(Serializer &s) override;
	};

	class strAction3 : public Action {
	public:
		SceneObject _object10;
		ScenePalette _palette2;
		ScenePalette _palette3;

		void signal() override;
	};

public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObjectExt _object8;
	strAction1 _strAction1;
	strAction2 _strAction2;
	strAction3 _strAction3;
	SpeakerMR _speakerMR;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
	void process(Event &event) override;
};

class Scene9999 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};

public:

	Action1 _action1;
	Action2 _action2;
	Action _action3;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
