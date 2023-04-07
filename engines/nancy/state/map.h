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

#include "engines/nancy/sound.h"
#include "engines/nancy/video.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/animatedbutton.h"
#include "engines/nancy/ui/ornaments.h"

namespace Nancy {

class NancyEngine;

namespace UI {
class Button;
}

namespace State {

class Map : public State, public Common::Singleton<Map> {
public:
	enum State { kInit, kLoad, kRun, kExit };
	Map();
	virtual ~Map() = default;

	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

	const SoundDescription &getSound();

protected:
	class MapViewport : public Nancy::RenderObject {
	public:
		MapViewport() : RenderObject(6) {}
		virtual ~MapViewport() = default;

		void init() override;
		void updateGraphics() override;

		void loadVideo(const Common::String &filename, const Common::String &palette = Common::String());
		void playVideo() { _decoder.start(); }
		void unloadVideo() { _decoder.close(); }

		AVFDecoder _decoder;
	private:
	};

	virtual void init() = 0;
	virtual void load();
	virtual void run() = 0;
	virtual void registerGraphics();

	void setLabel(int labelID);

	MAP *_mapData;

	MapViewport _viewport;
	RenderObject _label;
	RenderObject _closedLabel;
	RenderObject _background;

	State _state;
	uint16 _mapID;
	int16 _pickedLocationID;
	Common::Array<bool> _activeLocations;
	Common::Array<Common::Rect> _locationLabelDests;
};

class TVDMap : public Map {
	friend class MapGlobe;

public:
	TVDMap();
	virtual ~TVDMap() = default;

private:
	class MapGlobe : public Nancy::UI::AnimatedButton {
	public:
		MapGlobe(uint zOrder, TVDMap *owner) : AnimatedButton(zOrder), _gargoyleEyes(zOrder), _owner(owner) {}
		virtual ~MapGlobe() = default;

		void init() override;
		void registerGraphics() override;
		void onClick() override;
		void onTrigger() override;

	private:
		TVDMap *_owner;
		RenderObject _gargoyleEyes;
	};

	void init() override;
	void load() override;
	void run() override;
	void registerGraphics() override;

	bool onStateExit(const NancyState::NancyState nextState) override;

	MapGlobe _globe;
	UI::ViewportOrnaments _ornaments;
};

class Nancy1Map : public Map {
public:
	Nancy1Map();
	virtual ~Nancy1Map();

private:
	void init() override;
	void load() override;
	void run() override;
	void registerGraphics() override;

	bool onStateExit(const NancyState::NancyState next) override;

	UI::Button *_button;
};

#define NancyMapState Nancy::State::Map::instance()

} // End of namespace State
} // End of namespace Nancy

namespace Common {

template<>
Nancy::State::Map *Singleton<Nancy::State::Map>::makeInstance();

} // End of namespace Common

#endif // NANCY_STATE_MAP_H
