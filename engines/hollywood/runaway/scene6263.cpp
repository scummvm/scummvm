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
 */


#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Window", 252, 492, 0, {301, 302}, {0, 1}},
	{"Boiler", 223, 483, 1, {303, 304}, {0, 0}},
	{"Lever", 287, 505, 1, {305, 306}, {0, 1}},
	{"Wheel", 287, 505, 1, {307, 308}, {0, 1}},
	{"Gauges", 287, 505, 1, {311, 312}, {0, 0}},
	{"Lever", 0, 0, 0, {314, 315}, {0, 0}},
	{"Lever", 0, 0, 1, {316, 317}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6263, 301, 21, 0}, // Examine the window.
	{6263, 311, 31, 0}, // Identify the pressure and water gauges.
	{6263, 312, 32, 0}, // Leave the gauges alone.
	{6263, 314, 34, 0}, // Identify the driving lever.
	{6263, 317, 37, 0}  // Refuse to drain the tank.
};

const RunawayItemAction kItemActions[] = {
	{6263, 14, 2, 309},       // Put firewood into the boiler.
	{6263, 15, 2, 313},       // Light the boiler with the blowtorch.
	{6263, 0, 2, 310, false} // Reject other objects as fuel.
};

const byte kSteamLeverFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34
};

const byte kPressureWheelFrames[] = {
	35, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
	62, 63, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45,
	44, 43, 42, 41, 40, 39, 38, 37, 36, 35,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6263 = {
	6263, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {60, 60, 60}},
	Common::Point(252, 492),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

bool RunawayRoomScene::loadScene6263() {
	if (!_ambient.load(_archive, 30, 101) || !_overlay.load(_archive, 27, 1)) {
		return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_locomotiveGauges); ++i) {
		if (!_locomotiveGauges[i].load(_archive, 31 + i, 31)) {
			return false;
		}
	}
	return true;
}

void RunawayRoomScene::resume6263() {
	_ambientFrame = _town.boiler >= 2 ? 41 : 0;
	_ambientTick = _now;
	if (_town.boiler >= 2) {
		_audio.playEffect(53, 178, 1, true);
	}
	if (!_entering) {
		return;
	}
	if (_entryScene == 6264) {
		beginRoomAnimation(401, 26, 64, kSteamLeverFrames + 40, ARRAYSIZE(kSteamLeverFrames) - 40, true, 60);
	} else {
		beginRoomAnimation(400, 24, 24, nullptr, 0, true, 60);
	}
}

void RunawayRoomScene::update6263() {
	if (_town.boiler < 2 || _now - _ambientTick < 80) {
		return;
	}
	_ambientTick = _now;
	_ambientFrame = _ambientFrame < 40 ? _ambientFrame + 1 : 41 + _random.getRandomNumber(59);
	for (uint i = 0; i < 4; i += 3) {
		const int step = _random.getRandomBit() ? 1 : -1;
		_locomotiveGaugeFrames[i] = CLIP<int>(_locomotiveGaugeFrames[i] + step, 0, 30);
	}
}

void RunawayRoomScene::drawScene6263(Graphics::ManagedSurface &viewport) {
	_ambient.draw(viewport, _ambientFrame, 0, 0);
	_locomotiveGaugeFrames[1] = _town.boiler == 3 ? 30 : _town.boiler == 2 ? 15 : 0;
	_locomotiveGaugeFrames[2] = _town.waterTank * 6;
	if (drawingRoomAnimation()) {
		if (_roomAction == 313 && _town.boiler == 2) {
			_locomotiveGaugeFrames[1] = CLIP<int>((int)_actionStep - 41, 0, 15);
		} else if (_roomAction == 308) {
			_locomotiveGaugeFrames[1] = CLIP<int>((int)_actionStep - 23 + 15, 15, 30);
		}
	}
	for (uint i = 0; i < ARRAYSIZE(_locomotiveGauges); ++i) {
		_locomotiveGauges[i].draw(viewport, _locomotiveGaugeFrames[i], 0, 0);
	}
	if (!drawingRoomAnimation() || (_roomAction != 306 && _roomAction != 308 && _roomAction != 401)) {
		_overlay.draw(viewport, 0, 0, 0);
	}
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6263(uint16 action) {
	switch (action) {
	case 302: // Climb back out through the window.
		if (!_town.cabExited) {
			say(22);
		}
		beginRoomAnimation(302, 25, 29, nullptr, 0, true, 60);
		break;
	case 1302:
		_town.cabExited = 1;
		leaveTownRoom(6261);
		break;
	case 303: // Examine the boiler and its fuel.
		say(23, MIN<byte>(_town.boiler, 2));
		break;
	case 304: // Try to open the boiler.
		say(24, _town.boiler >= 2);
		break;
	case 305: // Identify the steam outlet lever.
		say(25, _town.steamLever);
		if (!_town.steamLever) {
			_town.steamLever = 1;
		}
		break;
	case 306: // Release steam, if the boiler has enough pressure.
		if (!_town.steamLever) {
			execute6263(305);
		}
		beginRoomAnimation(306, 26, 64, kSteamLeverFrames,
			_town.boiler == 3 && !_town.locomotiveKey ? 41 : ARRAYSIZE(kSteamLeverFrames), true, 60);
		break;
	case 1306:
		if (_town.boiler == 3 && !_town.locomotiveKey) {
			leaveTownRoom(6262);
		} else {
			say(26, _town.boiler < 2 ? 0 : _town.boiler == 2 ? 1 : 3);
		}
		break;
	case 307: // Identify the pressure wheel.
		say(27, _town.pressureWheel != 0);
		if (!_town.pressureWheel) {
			_town.pressureWheel = 1;
		}
		break;
	case 308: // Increase the boiler pressure once.
		if (!_town.pressureWheel) {
			execute6263(307);
		}
		if (_town.boiler == 2) {
			say(28, 1);
			beginRoomAnimation(308, 26, 64, kPressureWheelFrames, ARRAYSIZE(kPressureWheelFrames), true, 60);
		} else {
			say(28, _town.boiler == 3 ? 3 : 0);
		}
		break;
	case 1308:
		_town.pressureWheel = 2;
		_town.boiler = 3;
		say(28, 2);
		break;
	case 309: // Load the sheriff's firewood into the boiler.
		say(29);
		beginRoomAnimation(309, 28, 162);
		_actionFrames.insert_at(0, 0);
		_inventory.select(0);
		break;
	case 1309:
		_town.boiler = 1;
		_inventory.remove(14);
		break;
	case 310: // Reject unsuitable fuel.
		say(30, _town.boiler >= 2);
		break;
	case 313: // Light a fueled boiler only when the water tank is full.
		if (!_town.boiler) {
			say(33);
		} else if (_town.boiler >= 2) {
			say(33, 5);
		} else if (_town.waterTank < 5) {
			say(33, _town.waterTank ? 2 : 1);
		} else {
			say(33, 3);
			byte frames[92];
			for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
				frames[i] = i < 24 ? (i ? i - 1 : 0) : i < 70 ? 22 : 91 - i;
			}
			beginRoomAnimation(313, 29, 23, frames, ARRAYSIZE(frames));
			_inventory.select(0);
		}
		break;
	case 1313:
		say(33, 4);
		break;
	case 315: // Refuse to drive the derailed locomotive.
		say(35, _town.boiler >= 2);
		break;
	case 316: // Identify the tank's drainage lever.
		say(36, _town.drainSeen);
		_town.drainSeen = 1;
		break;
	case 1400: // Finish entering the cab.
		_position = Common::Point(252, 492);
		_walkPath.reset(_position);
		_facing = 4;
		if (!_town.cabVisited) {
			say(20);
			_town.cabVisited = 1;
		}
		break;
	case 1401: // Release the lever after the exterior steam shot.
		_position = Common::Point(287, 505);
		_walkPath.reset(_position);
		_facing = 1;
		say(26, 2);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::locomotiveAnimationFrame() {
	if (_room == 6260) {
		if (_roomAction == 304 && _actionStep == 20) {
			_audio.playEffect(20);
		} else if (_roomAction == 401 && _actionStep == 32) {
			_audio.playEffect(22);
		} else if (_roomAction == 313 && _actionStep == 37) {
			_audio.playEffect(21, 178);
		} else if (_roomAction == 402) {
			if (_actionStep == 3) {
				_audio.playEffect(56, 230);
			} else if (_actionStep == 17) {
				_audio.playEffect(57, 255, 1);
			}
		}
		return;
	}
	if (_roomAction == 400 && _actionStep == 13) {
		_audio.playEffect(23, 204);
	} else if (_roomAction == 302 && _actionStep == 10) {
		_audio.playEffect(81, 204);
	} else if ((_roomAction == 306 && (_actionStep == 16 || _actionStep == 43)) ||
			(_roomAction == 401 && _actionStep == 3)) {
		_audio.playEffect(54);
	} else if (_roomAction == 308 && _actionStep == 23) {
		_audio.playEffect(55);
	} else if (_roomAction == 309) {
		if (_actionStep == 12) {
			_audio.playEffect(24);
		} else if (_actionStep == 53) {
			_audio.playEffect(25);
		}
	} else if (_roomAction == 313) {
		if (_actionStep == 23) {
			_audio.playEffect(52, 255, 0, true);
		} else if (_actionStep == 35) {
			_town.boiler = 2;
			_ambientFrame = 0;
			_ambientTick = _now;
			_audio.playEffect(53, 178, 1, true);
		} else if (_actionStep == 70) {
			_audio.stopEffect(0);
		}
	}
}

} // End of namespace Hollywood
