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

#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "hollywood/runaway/scene6390.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {

const RunawayRoom kRunawayScene6390 = {
	6390, 1024, 600, nullptr, 0, {}, Common::Point(462, 312),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6390::Scene6390(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6390::loadCustomResources() {
	return _dialAnimation.load(_archive, 3, 100) && _handAnimation.load(_archive, 4, 18) &&
		_leverAnimation.load(_archive, 5, 2) && _pointerAnimation.load(_archive, 6, 241);
}

bool Scene6390::applyCustomBackgroundPatches() {
	return _entryScene != 6391 || applyPatch(7);
}

void Scene6390::runCustomEntrySequence() {
	if (_entryScene == 6391) {
		say(9, 1);
	} else {
		say(8, _town.safeOpened ? 2 : _town.safeDialVisited ? 1 : 0);
	}
	_town.safeDialVisited = 1;
}

Common::U32String Scene6390::actionCaption(byte id, RunawayVerb verb, byte item) const {
	if (_sliderOpen) {
		if (_dialTarget < 0) {
			return _strings.sceneActionCaption(_room, 8, 0, false);
		}
		return Common::U32String::format(_strings.sceneActionCaption(_room, 5, 0, false,
			_heldDirection == 1 ? "right" : "left"), (uint)_dialTarget);
	}
	return _strings.sceneActionCaption(_room, id, 0, false);
}

bool Scene6390::busy() const {
	return _sliderOpen || _autoTurn || RunawayRoomScene::busy();
}

Common::U32String Scene6390::hotspotName(byte id) const {
	return actionCaption(id, kVerbUse, 0);
}

void Scene6390::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (busy()) {
		return;
	}
	const byte id = hotspotAt(mouse);
	if (!id) { // Leave the close-up by clicking outside the controls.
		leaveDial();
	} else if (id <= 2) { // Turn one step, repeating while the button is held.
		_heldDirection = id;
		turnDial(_heldDirection);
		_repeatAt = _now + 350;
	} else { // Drag along the scale to select a number and direction.
		_sliderOpen = true;
		updateSlider(mouse);
	}
}

void Scene6390::updateSlider(Common::Point mouse) {
	_pointer = CLIP<int>((mouse.x - 32) / 4 + 1, 0, 240);
	_dialTarget = -1;
	if (mouse.x >= 72 && mouse.x < 472) {
		_dialTarget = (mouse.x - 72) / 4;
		_heldDirection = 2;
	} else if (mouse.x >= 552 && mouse.x < 952) {
		_dialTarget = (mouse.x - 552) / 4;
		_heldDirection = 1;
	}
	_caption = actionCaption(0, kVerbUse, 0);
}

void Scene6390::turnDial(byte direction) {
	if (_town.safeProgress == 3) {
		return;
	}
	const bool listening = _entryScene == 6391;
	if (listening && ((_town.safeProgress == 1 && direction == 1) || (_town.safeProgress == 2 && direction == 2))) {
		say(9, _town.safeProgress == 1 ? 5 : 6);
		_heldDirection = 0;
		return;
	}
	if (direction == 1) { // Turn right, decreasing the displayed number.
		_hand = _hand < 6 || _hand >= 14 ? 6 : _hand + 1;
		_town.safeDial = (_town.safeDial + 99) % 100;
	} else { // Turn left, increasing the displayed number.
		_hand = _hand <= 6 || _hand > 14 ? 14 : _hand - 1;
		_town.safeDial = (_town.safeDial + 1) % 100;
	}
	_audio.playEffect(listening ? 49 : 48, listening ? 255 : 77);
	static const byte combination[] = {85, 29, 54};
	const byte requiredDirection = _town.safeProgress == 1 ? 2 : 1;
	if (direction != requiredDirection) {
		_town.safeProgress = 0;
	} else if ((listening || _town.safeOpened) && _town.safeDial == combination[_town.safeProgress]) {
		++_town.safeProgress;
		_heldDirection = 0;
		if (listening) {
			_audio.playEffect(50);
			say(9, 1 + _town.safeProgress);
		}
	}
}

void Scene6390::advanceCustomGameplayLoop(uint32 delta) {
	Common::EventManager *events = g_system->getEventManager();
	const bool pressed = events->getButtonState() & Common::EventManager::LBUTTON;
	if (_sliderOpen) {
		updateSlider(events->getMousePos());
		if (!pressed) {
			_sliderOpen = false;
			_caption.clear();
			_autoTurn = _dialTarget >= 0 && _dialTarget != _town.safeDial;
			_repeatAt = _now;
		}
		return;
	}
	if (!_autoTurn && !pressed) {
		_heldDirection = 0;
	}
	if (RunawayRoomScene::busy()) {
		_heldDirection = 0;
		_autoTurn = false;
		return;
	}
	if (_town.safeProgress == 3) {
		leaveDial();
	} else if (_heldDirection && (int32)(_now - _repeatAt) >= 0) {
		turnDial(_heldDirection);
		_repeatAt = _now + 80;
		if (_autoTurn && (!_heldDirection || _town.safeDial == _dialTarget)) {
			_autoTurn = false;
			_heldDirection = 0;
		}
	}
}

void Scene6390::leaveDial() {
	_heldDirection = 0;
	leaveTownRoom(_entryScene == 6391 ? 6252 : 6251);
}

bool Scene6390::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	_dialAnimation.draw(viewport, _town.safeDial, 0, 0);
	_handAnimation.draw(viewport, _hand, 0, 0);
	_leverAnimation.draw(viewport, _sliderOpen, 0, 0);
	if (_sliderOpen) {
		_pointerAnimation.draw(viewport, _pointer, 0, 0);
	}
	return true;
}

} // End of namespace Runaway
} // End of namespace Hollywood
