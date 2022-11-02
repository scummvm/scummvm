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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"

namespace Hadesch {

class IntroHandler : public Handler {
public:
	IntroHandler() {
	}

	void handleClick(const Common::String &name) override {
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		switch(eventId) {
		  // 32002 handles performance testing
		case 32003:
			g_vm->moveToRoom(kOlympusRoom);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playVideo("o0010ba0", 101, 32003);
		room->disableHeroBelt();
		room->disableMouse();
	}
};

Common::SharedPtr<Hadesch::Handler> makeIntroHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new IntroHandler());
}

}
