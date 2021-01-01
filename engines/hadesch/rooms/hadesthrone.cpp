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

class HadesThroneHandler : public Handler {
public:
	HadesThroneHandler() {
	}

	void handleClick(const Common::String &name) override {
	}

	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		Persistent *persistent = g_vm->getPersistent();

		switch(eventId) {
		case 29001:
			persistent->_quest = kEndGame;
			persistent->clearInventory();
			persistent->_doQuestIntro = true;
			g_vm->moveToRoom(kWallOfFameRoom);
			break;
		}
	}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playVideo("movie", 500, 29001);
		room->disableHeroBelt();
		room->playMusicLoop("V6010eA0");
		room->disableMouse();
	}
};

Common::SharedPtr<Hadesch::Handler> makeHadesThroneHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new HadesThroneHandler());
}

}
