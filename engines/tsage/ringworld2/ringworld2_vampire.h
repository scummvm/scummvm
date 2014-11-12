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

#ifndef TSAGE_RINGWORLD2_VAMPIRE_H
#define TSAGE_RINGWORLD2_VAMPIRE_H

#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene1950 : public SceneExt {
	/* Windows */
	class KeypadWindow: public ModalWindow {
	public:
		class KeypadButton : public SceneActor {
		public:
			int _buttonIndex;
			bool _pressed;
			bool _toggled;

			KeypadButton();
			void synchronize(Serializer &s);

			void init(int indx);
			virtual void process(Event &event);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _areaActor;
		KeypadButton _buttons[16];

		int _buttonIndex;

		KeypadWindow();
		virtual void synchronize(Serializer &s);
		virtual void remove();
		virtual void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Keypad : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actors */
	class Door : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Scrolls : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Gem : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Vampire : public SceneActor {
	public:
		Common::Point _deadPosition;
		int _deltaX;
		int _deltaY;
		int _vampireMode;

		Vampire();
		void synchronize(Serializer &s);

		virtual void signal();
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Exits */
	class NorthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class UpExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class EastExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class DownExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class SouthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class ShaftExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class DoorExit : public SceneExit {
	public:
		virtual void changeScene();
	};
private:
	void initArea();
	void enterArea();
	void doButtonPress(int indx);
public:
	NamedHotspot _background;
	Keypad _keypad;
	SceneActor _southDoorway;
	SceneObject _northDoorway;
	Door _door;
	Scrolls _scrolls;
	SceneActor _containmentField;
	Gem _gem;
	SceneActor _cube;
	SceneActor _pulsingLights;
	Vampire _vampire;
	KeypadWindow _KeypadWindow;
	NorthExit _northExit;
	UpExit _upExit;
	EastExit _eastExit;
	DownExit _downExit;
	SouthExit _southExit;
	WestExit _westExit;
	ShaftExit _shaftExit;
	DoorExit _doorExit;
	SequenceManager _sequenceManager;

	bool _upExitStyle;
	bool _removeFlag;
	bool _vampireActive;
	Common::Point _vampireDestPos;
	int _vampireIndex;

	Scene1950();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
