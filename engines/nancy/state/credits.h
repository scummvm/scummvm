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

#ifndef NANCY_STATE_CREDITS_H
#define NANCY_STATE_CREDITS_H

#include "common/singleton.h"

#include "engines/nancy/time.h"
#include "engines/nancy/commontypes.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {

namespace State {

class Credits : public State, public Common::Singleton<Credits> {
public:
	enum State { kInit, kRun };
	Credits() : _state(kInit), _background(), _text(), _pixelsToScroll(0) {}

	// State API
	void process() override;
	void onStateExit() override { destroy(); };

protected:
	void init();
	void run();

	class CreditsText : public RenderObject {
		friend class Credits;
	public:
		CreditsText() : RenderObject(1) {}
		virtual ~CreditsText() = default;
	};

	State _state;
	UI::FullScreenImage _background;
	CreditsText _text;
	Time _nextUpdateTime;
	Graphics::ManagedSurface _fullTextSurface;

	Time _updateTime; // 0x54
	uint16 _pixelsToScroll; // 0x56
	SoundDescription _sound; // 0x58, kMenu?
};

#define NancyCreditsState Nancy::State::Credits::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_CREDITS_H
