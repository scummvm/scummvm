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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"

namespace Hadesch {

enum {
	kBackgroundZ = 10000,
	kCreditsZ = 1000
};

class CreditsHandler : public Handler {
public:
	CreditsHandler(bool inOptions) {
		_inOptions = inOptions;
	}

	void handleClick(const Common::String &name) override {
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

		switch(eventId) {
		case 31001:
			if (_inOptions)
				g_vm->enterOptions();
			else
				g_vm->moveToRoom(g_vm->getPreviousRoomId());
			break;
		}
	}

	void frameCallback() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		int timeElapsed = (g_vm->getCurrentTime() - startTime);
		room->selectFrame("h2030ba0", kCreditsZ, 0,
				  Common::Point(0, 481
						- timeElapsed * 6151 / 136000
					  ));
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->disableHeroBelt();
		room->disableMouse();
		room->addStaticLayer("h2030pa0", kBackgroundZ);
		room->playVideo("c2590ma0", 0, 31001);
		room->selectFrame("h2030ba0", kCreditsZ, 0,
				  Common::Point(0, 481));
		startTime = g_vm->getCurrentTime();
	}
private:
	int startTime;
	bool _inOptions;
};

Common::SharedPtr<Hadesch::Handler> makeCreditsHandler(bool inOptions) {
	return Common::SharedPtr<Hadesch::Handler>(new CreditsHandler(inOptions));
}

}
