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

#ifndef NANCY_STATE_HELP_H
#define NANCY_STATE_HELP_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"

#include "engines/nancy/commontypes.h"

namespace Nancy {

namespace State {

class Help : public State, public Common::Singleton<Help> {
public:
	enum State { kInit, kBegin, kRun, kWaitForSound };
	Help() : _state(kInit), _image() {}

	// State API
	virtual void process() override;
	virtual bool onStateExit() override { destroy(); return true; };

private:
	void init();
	void begin();
	void run();
	void waitForSound();

	State _state;
	UI::FullScreenImage _image;
	Common::Rect _hotspot; // Can be an array, but isn't in nancy1
	SoundDescription _sound;
};

#define NancyHelpState Nancy::State::Help::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_HELP_H
