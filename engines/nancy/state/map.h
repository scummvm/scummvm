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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_STATE_MAP_H
#define NANCY_STATE_MAP_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/viewport.h"

namespace Nancy {

class NancyEngine;

namespace UI {
class Button;
}

namespace State {

class Map : public State, public Common::Singleton<Map> {
public:
	enum State { kInit, kRun };
	Map();
	virtual ~Map();

	// State API
	void process() override;
	void onStateExit() override;

private:
	struct Location {
		struct SceneChange {
			uint16 sceneID = 0;
			uint16 frameID = 0;
			uint16 verticalOffset = 0;
		};

		Common::String description;

		bool isActive = false;
		Common::Rect hotspot;
		Common::Array<SceneChange> scenes;

		Common::Rect labelSrc;
		Common::Rect labelDest;
	};

	void init();
	void run();

	void registerGraphics();

	void setLabel(int labelID);

	Nancy::UI::Viewport _viewport;
	RenderObject _label;
	RenderObject _closedLabel;
	UI::Button *_button;
	SoundDescription _sound;

	State _state;
	uint16 _mapID;
	bool _mapButtonClicked;
	int16 _pickedLocationID;
	Common::Array<Location> _locations;
};

#define NancyMapState Nancy::State::Map::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_MAP_H
