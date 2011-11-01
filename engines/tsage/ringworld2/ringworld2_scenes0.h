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

#ifndef TSAGE_RINGWORLD2_SCENES0_H
#define TSAGE_RINGWORLD2_SCENES0_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene100: public SceneExt {
	/* Objects */
	class Door: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Table: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class StasisNegator: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Object10: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class SteppingDisks: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Terminal: public NamedHotspot{
	public:
		bool startAction(CursorType action, Event &event);
	};

public:
	NamedHotspot _background, _duct, _bed, _desk;
	Terminal _terminal;
	SceneActor _object1, _object2, _object3, _object4, _object5;
	SceneActor _object6;
	Door _door;
	Table _table;
	StasisNegator _stasisNegator;
	Object10 _object10;
	SteppingDisks _steppingDisks;
	SequenceManager _sequenceManager1, _sequenceManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene125: public SceneExt {
	/* Objects */
	class Object5: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Icon: public SceneActor {
	public:
		int _lookLineNum, _field98;
		bool _pressed;
		SceneObject _object1, _object2;
		SceneText _sceneText1, _sceneText2;

		Icon();
		virtual Common::String getClassName() { return "Scene125_Icon"; }
		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void synchronize(Serializer &s);
		virtual void process(Event &event);

		void setIcon(int id);
		void showIcon();
		void hideIcon();
	};

	/* Items */
	class Item4: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	ScenePalette _palette;
	ASoundExt _sound1;
	NamedHotspot _background, _item2, _item3;
	Item4 _item4;
	SceneActor _object1, _object2, _object3, _object4, _object5, _object6, _object7;
	Icon _icon1, _icon2, _icon3, _icon4, _icon5,  _icon6;
	SequenceManager _sequenceManager;
	SceneText _sceneText;
	int _consoleMode, _iconFontNumber, _logIndex, _databaseIndex, _infodiskIndex;
	int _soundCount, _soundIndex;
	int _soundIndexes[10];

	Scene125();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();

	void consoleAction(int id);
	void setDetails(int resNum, int lineNum);
	void stop();
	Common::String parseMessage(const Common::String &msg);
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
