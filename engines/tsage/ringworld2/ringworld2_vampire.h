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
			void synchronize(Serializer &s) override;

			void init(int indx);
			void process(Event &event) override;
			bool startAction(CursorType action, Event &event) override;
		};

		SceneActor _areaActor;
		KeypadButton _buttons[16];

		int _buttonIndex;

		KeypadWindow();
		void synchronize(Serializer &s) override;
		void remove() override;
		void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) override;
		void setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum) override;
	};

	class Keypad : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actors */
	class Door : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Scrolls : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Gem : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Vampire : public SceneActor {
	public:
		Common::Point _deadPosition;
		int _deltaX;
		int _deltaY;
		int _vampireMode;

		Vampire();
		void synchronize(Serializer &s) override;

		void signal() override;
		bool startAction(CursorType action, Event &event) override;
	};

	/* Exits */
	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class UpExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class EastExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class DownExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class WestExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class ShaftExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class DoorExit : public SceneExit {
	public:
		void changeScene() override;
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
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
