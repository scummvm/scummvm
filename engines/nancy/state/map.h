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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NANCY_STATE_MAP_H
#define NANCY_STATE_MAP_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/ui/button.h"

namespace Nancy {

class NancyEngine;

namespace State {

class Map : public State, public Common::Singleton<Map> {
	friend class MapLabel;
	friend class MapButton;
public:
	enum State { kInit, kRun };
	Map();

	// State API
	virtual void process() override;
	virtual void onStateExit() override;

private:
	struct Location {
		struct SceneChange {
			uint16 sceneID = 0;
			uint16 frameID = 0;
			uint16 verticalOffset = 0;
		};

		bool isActive = false;
		Common::Rect hotspot;
		Common::Array<SceneChange> scenes;

		Common::Rect labelSrc;
		Common::Rect labelDest;
	};

	class MapLabel : public Nancy::RenderObject {
	public:
		MapLabel(RenderObject &redrawFrom, Map *parent) : Nancy::RenderObject(redrawFrom), _parent(parent) {}
		virtual ~MapLabel() = default;

		virtual void init() override;

		void setLabel(int labelID);

	protected:
		virtual uint16 getZOrder() const override { return 7; }

		Map *_parent;
	};

	class MapButton : public UI::Button {
	public:
		MapButton(RenderObject &redrawFrom, Map *parent) : Button(redrawFrom), _parent(parent) {}
		virtual ~MapButton() = default;

		virtual void init() override;
		virtual void onClick() override;

	protected:
		virtual uint16 getZOrder() const override { return 9; }

		Map *_parent;
	};

	void init();
	void run();

	void registerGraphics();

	Nancy::UI::Viewport _viewport;
	MapLabel _label;
	MapButton _button;

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
