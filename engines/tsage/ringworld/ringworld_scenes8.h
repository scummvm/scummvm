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

#ifndef TSAGE_RINGWORLD_SCENES8_H
#define TSAGE_RINGWORLD_SCENES8_H

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

class NamedHotspotMult : public SceneHotspot {
public:
	int _useLineNum, _lookLineNum;
	NamedHotspotMult();

	Common::String getClassName() override { return "NamedHotspotMult"; }
	void synchronize(Serializer &s) override;
};

class SceneObject7700 : public SceneObjectExt {
public:
	int _lookLineNum, _defltLineNum;

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "SceneObject7700"; }
};

class Scene7000 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void dispatch() override;
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
	class Action5 : public Action {
	public:
		void signal() override;
	};
	class Action6 : public Action {
	public:
		void signal() override;
	};
	class Action7 : public Action {
	public:
		void signal() override;
	};

	/* Objects */
	class Object1 : public SceneObject {
	public:
		void doAction(int action) override;
	};

	/* Items */
	class Hotspot1 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};

public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	SpeakerSKText _speakerSKText;
	SpeakerSKL _speakerSKL;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	Object1 _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	SceneObject _object9;
	SceneObject _object10;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Hotspot1  _hotspot1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene7100 : public Scene {
	/* Actions */
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
	class Action5 : public Action {
	public:
		void signal() override;
	};
	class Action6 : public Action {
	public:
		void signal() override;
	};
	class Action7 : public Action {
	public:
		void signal() override;
	};
	class Action8 : public Action {
	public:
		void signal() override;
	};
	class Action9 : public Action {
	public:
		void signal() override;
	};
	class Action10 : public Action {
	public:
		void signal() override;
	};
	class Action11 : public Action {
	public:
		void signal() override;
	};

public:
	ASound _soundHandler1;
	ASound _soundHandler2;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	SceneObject _object9;
	SceneObject _object10;
	SceneObject _object11;
	SceneObject _object12;
	SceneObject _object13;
	SceneObject _object14;
	SceneObject _object15;
	SceneObject _object16;
	SceneObject _object17;
	SceneObject _object18;
	SceneObject _object19;
	SceneObject _object20;
	SceneObject _object21;
	SceneObject _object22;
	SceneObject _object23;
	SceneObject _object24;
	SceneObject _object25;
	Action _action1;
	Action _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene7200 : public Scene {
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
	SceneObject _swimmer;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	SceneObject _object9;
	ASound _soundHandler;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene7300 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};

public:
	SpeakerPOR _speakerPOR;
	SpeakerPOText _speakerPOText;
	SpeakerSKText _speakerSKText;
	SpeakerQU _speakerQU;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void dispatch() override;
};

class Scene7600 : public Scene {
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
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	ASound _soundHandler1;
	ASound _soundHandler2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene7700 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
	class Action5 : public Action {
	public:
		void signal() override;
	};
	class Action6 : public Action {
	public:
		void signal() override;
	};

	class Object1 : public SceneObject7700 {
	public:
		void signal() override;
		void doAction(int action) override;
	};
	class Object3 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Object7 : public SceneObjectExt {
	public:
		void doAction(int action) override;
	};
	class Object8 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Object9 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Object10 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Object11 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Object12 : public SceneObject {
	public:
		void doAction(int action) override;
	};

	/* Items */
	class SceneHotspot1 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot2 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot3 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot4 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot5 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot6 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneItem7 : public SceneItem {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot8 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot9 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class SceneItem10 : public SceneItem {
	public:
		void doAction(int action) override;
	};
	class SceneHotspot11 : public NamedHotspotMult {
	public:
		void doAction(int action) override;
	};
public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	GfxButton _gfxButton;
	SpeakerEText _speakerEText;
	SpeakerQText _speakerQText;
	Object1 _object1;
	Object1 _object2;
	Object3 _object3;
	Object1 _object4;
	Object1 _object5;
	Object1 _object6;
	Object7 _prof;
	Object8 _object8;
	Object9 _object9;
	Object10 _object10;
	Object11 _cork;
	Object12 _emptyJar;
	SceneObject _object13;
	SceneObject _object14;
	SceneObject _object15;
	SceneObject _cloud;
	SceneObject _easterEgg1;
	SceneObject _easterEgg2;
	SceneObject _object19;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	SceneHotspot1  _sceneHotspot1;
	SceneHotspot2  _sceneHotspot2;
	SceneHotspot3  _sceneHotspot3;
	SceneHotspot4  _sceneHotspot4;
	SceneHotspot5  _sceneHotspot5;
	SceneHotspot6  _sceneHotspot6;
	SceneItem7  _sceneItem7;
	SceneHotspot8  _sceneHotspot8;
	SceneHotspot9  _sceneHotspot9;
	SceneItem10 _sceneItem10;
	SceneHotspot11 _sceneHotspot11;
	SceneHotspot11 _sceneHotspot12;
	SceneHotspot11 _sceneHotspot13;
	SceneHotspot11 _sceneHotspot14;
	SceneHotspot11 _sceneHotspot15;
	SceneHotspot11 _sceneHotspot16;
	SceneHotspot11 _sceneHotspot17;
	SceneHotspot11 _sceneHotspot18;
	SceneHotspot11 _sceneHotspot19;
	SceneHotspot11 _sceneHotspot20;
	SceneHotspot11 _sceneHotspot21;
	SceneHotspot11 _sceneHotspot22;
	SceneHotspot11 _sceneHotspot23;
	SceneHotspot11 _sceneHotspot24;
	SceneHotspot11 _sceneHotspot25;
	SceneHotspot11 _sceneHotspot26;
	SceneHotspot11 _sceneHotspot27;
	SceneHotspot11 _sceneHotspot28;
	SceneHotspot11 _sceneHotspot29;
	SceneHotspot11 _sceneHotspot30;
	SceneHotspot11 _sceneHotspot31;
	SceneHotspot11 _sceneHotspot32;
	SceneHotspot11 _sceneHotspot33;
	SceneHotspot11 _sceneHotspot34;
	SceneHotspot11 _sceneHotspot35;
	SceneHotspot11 _sceneHotspot36;
	int _seatCountLeft1, _seatCountRight, _seatCountLeft2;

	Scene7700();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
