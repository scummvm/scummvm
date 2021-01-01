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
static const char *kNewButton = "newbutton";
static const char *kRestoreButton = "restorebutton";
static const char *kQuitButton = "quitbutton";

enum {
	kBackgroundZ = 10000,
	kWaterFallZ = 9000,
	kButtonZ = 2101,
	kLogoZ = 1101
};

class OlympusHandler : public Handler {
public:
	void handleClick(const Common::String &hotname) override {
		if (hotname == "new") {
			g_vm->newGame();
			return;
		}

		if (hotname == "restore") {
			g_vm->enterOptions();
			return;
		}

		if (hotname == "quit") {
			g_vm->quit();
			return;
		}
	}
	void handleMouseOver(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (name == "new") {
			room->selectFrame(kNewButton, kButtonZ, 6);
			return;
		}
		if (name == "restore") {
			room->selectFrame(kRestoreButton, kButtonZ, 6);
			return;
		}
		if (name == "quit") {
			room->selectFrame(kQuitButton, kButtonZ, 6);
			return;
		}
	}
	void handleMouseOut(const Common::String &name) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (name == "new") {
			room->selectFrame(kNewButton, kButtonZ, 5);
			return;
		}
		if (name == "restore") {
			room->selectFrame(kRestoreButton, kButtonZ, 5);
			return;
		}
		if (name == "quit") {
			room->selectFrame(kQuitButton, kButtonZ, 5);
			return;
		}
	}
	void handleEvent(int eventId) override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		switch (eventId) {
		case 21001:
			room->playSFX("o1010ea0");
			break;
		case 21002:
			room->enableMouse();
			room->selectFrame("logo", kLogoZ, 0);
			room->playAnimLoop("waterfall1", kWaterFallZ);
			room->playAnimLoop("waterfall2", kWaterFallZ);
			room->playAnimLoop("waterfall3", kWaterFallZ);
			room->playAnimLoop("waterfall4", kWaterFallZ);
			room->playAnim(kNewButton, kButtonZ, PlayAnimParams::keepLastFrame().partial(0, 5));
			room->playAnim(kQuitButton, kButtonZ, PlayAnimParams::keepLastFrame().partial(0, 5));
			if (g_vm->hasAnySaves())
				room->playAnim(kRestoreButton, kButtonZ, PlayAnimParams::keepLastFrame().partial(0, 5));
			else
				room->disableHotzone("restore");
			break;
		}
	}

	~OlympusHandler() override {}

	void prepareRoom() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->loadHotZones("Olympus.HOT");
		room->addStaticLayer("background", kBackgroundZ);
		room->disableMouse();
		if (g_vm->getPreviousRoomId() == kOptionsRoom) {
			room->playSFX("o1010ea0", 21002);
		} else {
			room->playVideo("movie", 201, 21002);
			g_vm->addTimer(21001, 40000);
		}
		room->disableHeroBelt();
	}
};

Common::SharedPtr<Hadesch::Handler> makeOlympusHandler() {
	return Common::SharedPtr<Hadesch::Handler>(new OlympusHandler());
}

}
