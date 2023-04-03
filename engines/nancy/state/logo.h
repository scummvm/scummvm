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

#ifndef NANCY_STATE_LOGO_H
#define NANCY_STATE_LOGO_H

#include "common/singleton.h"
#include "video/avi_decoder.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/state/state.h"
#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {

namespace State {

class Logo : public State, public Common::Singleton<Logo> {
public:
	Logo() : _state(kInit), _startTicks(0), _videoObj(1) { }

	// State API
	void process() override;
	void onStateExit() override;

private:
	void init();
	void playIntroVideo();
	void startSound();
	void run();
	void stop();

	enum State {
		kInit,
		kPlayIntroVideo,
		kStartSound,
		kRun,
		kStop
	};

	State _state;
	uint _startTicks;
	UI::FullScreenImage _logoImage;
	SoundDescription _msnd;
	Video::AVIDecoder _tvdVideoDecoder;
	RenderObject _videoObj;
};

#define NancyLogoState Nancy::State::Logo::instance()

} // end of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_LOGO_H
