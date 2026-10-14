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
#include "graphics/cursorman.h"
#include "hollywood/runaway/strings.h"
#include "hollywood/runaway/ui.h"

namespace Hollywood {
namespace {

const Graphics::PixelFormat kUIFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
const uint kCursorWidth = 60;
const uint kButtonX = 312;

void shadeBevelPixel(Graphics::ManagedSurface &screen, int x, int y, int amount) {
	uint16 *pixel = (uint16 *)screen.getBasePtr(x, y);
	int r = *pixel >> 11, g = (*pixel >> 5) & 63, b = *pixel & 31;
	// Native 0x40c600 shades the texture, one step on horizontal edges and
	// two on vertical edges, keeping all three channels away from clipping.
	const int direction = amount < 0 ? -1 : 1;
	for (int i = 0; i < ABS(amount); ++i) {
		if ((direction < 0 && r && g && b) || (direction > 0 && r < 31 && g < 31 && b < 31)) {
			r += direction;
			g += direction;
			b += direction;
		}
	}
	*pixel = (r << 11) | (g << 5) | b;
}

} // End of anonymous namespace

RunawayUI::~RunawayUI() {
	if (_cursorPushed)
		CursorMan.popCursor();
}

bool RunawayUI::load() {
	Common::Array<byte> animation;
	if (!_font.load() || !loadRunawaySurface(114, 904, 540, _cursors) ||
			!loadRunawaySurface(118, 48, 48, _demoLogo) || !loadRunawaySurface(121, 80, 42 * 80, _icons) ||
			!loadRunawaySurface(127, 700, 74, _inventoryButton) || !loadRunawaySurface(129, 700, 74, _returnButton) ||
			!loadRunawaySurface(128, 700, 74, _optionsButton) ||
			!loadRunawaySurface(131, 1024, 600, _inventoryBackground) || !loadRunawaySurface(133, 768, 576, _inventoryTexture) ||
			!readRunawayGlobalChunk(137, animation) || !_portrait.load(animation, 104) ||
			!readRunawayGlobalChunk(141, animation) || !_idlePortrait.load(animation, 32))
		return false;
	_cursor.create(kCursorWidth, kCursorWidth, kUIFormat);
	const uint16 key = *(const uint16 *)_cursors.getPixels();
	_cursor.fillRect(Common::Rect(0, 0, kCursorWidth, kCursorWidth), key);
	CursorMan.pushCursor(_cursor.getPixels(), kCursorWidth, kCursorWidth, 30, 30, key, false, &kUIFormat);
	_cursorPushed = true;
	return true;
}

Common::Rect RunawayUI::itemRect(uint slot) const {
	// Original layout table at 0x464508: the grid grows symmetrically within
	// the 768-pixel inventory texture, with 80-pixel icons on a 96-pixel pitch.
	const uint count = _inventory.size();
	const uint columns = count <= 4 ? MAX<uint>(1, count) : count <= 8 ? 4 : count <= 15 ? 5 : count <= 24 ? 6 : 7;
	const uint rows = (count + columns - 1) / columns;
	const int x = 356 - (columns - 1) * 48 + (slot % columns) * 96;
	const int y = kRunawayViewportTop + 260 - (rows - 1) * 48 + (slot / columns) * 96;
	return Common::Rect(x, y, x + 80, y + 80);
}

byte RunawayUI::itemAt(Common::Point mouse) const {
	for (uint slot = 0; slot < _inventory.size(); ++slot) {
		if (itemRect(slot).contains(mouse))
			return _inventory.itemAt(slot);
	}
	return 0;
}

bool RunawayUI::overButton() const {
	return Common::Rect(kButtonX, 0, kButtonX + 100, 74).contains(_mouse);
}

bool RunawayUI::overOptions() const {
	return Common::Rect(612, 0, 712, 74).contains(_mouse);
}

bool RunawayUI::inViewport() const {
	return _mouse.x >= 0 && _mouse.x < kRunawayScreenWidth &&
		_mouse.y >= kRunawayViewportTop && _mouse.y < kRunawayViewportTop + kRunawayViewportHeight;
}

void RunawayUI::setVerb(RunawayVerb verb) {
	if (_verb != verb) {
		_previousVerb = _verb;
		_verb = verb;
		_verbStarted = _now;
	}
}

void RunawayUI::setInventoryOpen(bool open) {
	_inventoryOpen = open;
	_enteredInventory = inViewport();
	_hovered = 0;
	_idleStarted = 0;
	_lastInteraction = _now;
	setVerb(kVerbWalk);
}

void RunawayUI::toggleInventory() {
	if (!_blocked && !_speaking && !busy())
		setInventoryOpen(!_inventoryOpen);
}

bool RunawayUI::cancel() {
	if (busy() || _inventoryPresentation)
		return true;
	if (_inventory.selected()) {
		_inventory.select(0);
		return true;
	}
	if (_inventoryOpen) {
		setInventoryOpen(false);
		return true;
	}
	return false;
}

void RunawayUI::update(uint32 now, Common::Point mouse, byte roomHotspot, bool blocked, bool speaking,
		bool inventoryPresentation, byte speechFrame) {
	_now = now;
	_mouse = mouse;
	_blocked = blocked;
	_speaking = speaking;
	// Native 0x40e8e0 uses the same twelve mouth/eye frames as Brian in
	// the room, on the same speech clock, including continuation lines.
	_mouth = MIN<byte>(11, speechFrame);
	if (_inventoryPresentation != inventoryPresentation) {
		_inventoryPresentation = inventoryPresentation;
		setInventoryOpen(inventoryPresentation);
	}
	const byte added = _inventory.takeNewItem();
	if (added) {
		_returnAfterPickup = !_inventoryOpen;
		setInventoryOpen(true);
		_pickupItem = added;
		_pickupStarted = now;
		_pickupSoundPlayed = false;
	}
	// Native 0x40d390 plays S03 cue 1 when the twelfth pickup frame
	// reveals the item in its inventory slot.
	if (_pickupItem && !_pickupSoundPlayed && now - _pickupStarted >= 12 * 60) {
		_audio.playEffect(1);
		_pickupSoundPlayed = true;
	}
	if (_pickupItem && now - _pickupStarted >= 18 * 60) {
		_pickupItem = 0;
		if (_returnAfterPickup && !_speaking)
			setInventoryOpen(false);
	}
	if (_returnAfterPickup && !_pickupItem && !_speaking) {
		_returnAfterPickup = false;
		setInventoryOpen(false);
	}
	if (_inventoryOpen && !busy() && !_speaking && _inventory.selected()) {
		if (inViewport())
			_enteredInventory = true;
		else if (_enteredInventory)
			setInventoryOpen(false);
	}
	const byte hovered = _inventoryOpen ? itemAt(mouse) : roomHotspot;
	if (_hovered != hovered) {
		_hovered = hovered;
		setVerb(hovered ? (_inventoryOpen ? kVerbUse : kVerbLook) : kVerbWalk);
	}
	if (now - _buttonTick >= 20) {
		const uint steps = MIN<uint>(6, (now - _buttonTick) / 20);
		_buttonTick = now;
		if (_blocked || _speaking || busy())
			_buttonFrame = 0;
		else if (overButton())
			_buttonFrame = 6;
		else if (mouse.y < kRunawayViewportTop)
			_buttonFrame = MIN<uint>(5, _buttonFrame + steps);
		else
			_buttonFrame = MAX<int>(0, MIN<int>(5, _buttonFrame) - steps);
		if (overOptions())
			_optionsFrame = 6;
		else if (mouse.y < kRunawayViewportTop)
			_optionsFrame = MIN<uint>(5, _optionsFrame + steps);
		else
			_optionsFrame = MAX<int>(0, MIN<int>(5, _optionsFrame) - steps);
	}
	if (_inventoryOpen && now - _portraitTick >= 60) {
		_portraitTick = now;
		if (_speaking) {
			_blinkUntil = 0;
			_lastInteraction = now;
			_idleStarted = 0;
		} else if (!busy() && _random.getRandomNumber(49) == 0) {
			_blinkUntil = now + 180;
		}
		if (!busy() && !_speaking && !_idleStarted && now - _lastInteraction >= 30000)
			_idleStarted = now;
		if (_idleStarted && now - _idleStarted >= 32 * 60) {
			_idleStarted = 0;
			_lastInteraction = now;
		}
	}
	updateCursor();
}

RunawayUI::Command RunawayUI::click(bool right) {
	Command command;
	_lastInteraction = _now;
	_idleStarted = 0;
	if (!right && overOptions()) {
		command.type = Command::kOpenMenu;
		return command;
	}
	if (_speaking) {
		command.type = Command::kSkipSpeech;
		return command;
	}
	if (_blocked || busy())
		return command;
	if (right) {
		if (_inventory.selected())
			_inventory.select(0);
		else if (_hovered)
			setVerb(_verb == kVerbLook ? kVerbUse : kVerbLook);
		return command;
	}
	if (overButton()) {
		toggleInventory();
		return command;
	}
	if (!inViewport())
		return command;
	command.mouse = _mouse;
	command.verb = _verb;
	command.item = _inventory.selected();
	command.target = _hovered;
	if (!_inventoryOpen) {
		command.type = Command::kRoom;
		if (command.item) {
			command.verb = kVerbUse;
			if (_hovered)
				_inventory.select(0);
		}
	} else if (_hovered) {
		if (command.item == _hovered) {
			_inventory.select(0);
		} else if (command.item) {
			command.type = Command::kCombine;
			_inventory.select(0);
		} else if (_verb == kVerbLook) {
			command.type = Command::kExamineItem;
		} else {
			_inventory.select(_hovered);
		}
	}
	return command;
}

void RunawayUI::updateCursor() {
	const bool waiting = _blocked || _speaking || busy();
	// Native 0x411990 hides the cursor during actions and dialogue.
	// RESOURCE.000 chunk 118 is the corner logo, not a busy cursor.
	CursorMan.showMouse(!waiting);
	if (waiting)
		return;
	const byte item = _inventory.selected();
	uint tile = _verb;
	uint shade = 0;
	if (item) {
		tile = _inventory.cursor(item) + (_hovered != 0);
	} else {
		const uint phase = (_now - _verbStarted) / 20;
		if (phase < 10) {
			// Four cels collapse the old verb to a ball; four expand the new one.
			tile = !phase ? (uint)_previousVerb : phase < 5 ? _previousVerb * 4 + 8 + phase :
				phase == 5 ? 8 : _verb * 4 + 18 - phase;
		} else {
			const byte pulse[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 8, 7, 6, 5, 4, 3, 2, 1};
			shade = pulse[(_now / 50) % ARRAYSIZE(pulse)];
		}
	}
	const int key = tile * 10 + shade;
	if (key == _cursorKey)
		return;
	_cursorKey = key;
	const uint16 transparent = *(const uint16 *)_cursors.getPixels();
	_cursor.fillRect(Common::Rect(0, 0, 60, 60), transparent);
	const uint sx = (tile % 15) * 60;
	const uint sy = (tile / 15) * 60;
	for (uint y = 0; y < 60; ++y) {
		const uint16 *src = (const uint16 *)_cursors.getBasePtr(sx, sy + y);
		uint16 *dst = (uint16 *)_cursor.getBasePtr(0, y);
		for (uint x = 0; x < 60; ++x) {
			if (src[x] == transparent)
				continue;
			const uint r = (src[x] >> 11) & 31, g = (src[x] >> 5) & 63, b = src[x] & 31;
			dst[x] = ((r * (31 - shade) / 31) << 11) |
				((g * (31 - shade) / 31) << 5) | (b * (31 - shade) / 31);
		}
	}
	CursorMan.replaceCursor(_cursor.getPixels(), 60, 60, 30, 30, transparent, false, &kUIFormat);
}

Common::U32String RunawayUI::inventoryCaption() const {
	const byte selected = _inventory.selected();
	if (selected) {
		if (_hovered == selected)
			return _strings.text(RunawayStrings::kPutAway) + _inventory.name(selected);
		Common::U32String caption = _strings.text(RunawayStrings::kUse) + _inventory.name(selected);
		if (_hovered)
			caption += _strings.text(RunawayStrings::kWith) + _inventory.name(_hovered);
		return caption;
	}
	if (!_hovered)
		return Common::U32String();
	return _strings.text(_verb == kVerbLook ? RunawayStrings::kLook : RunawayStrings::kTake) + _inventory.name(_hovered);
}

void RunawayUI::drawInventory(Graphics::ManagedSurface &screen) {
	screen.copyRectToSurface(_inventoryBackground, 0, kRunawayViewportTop,
		Common::Rect(0, 0, 1024, 600));
	screen.copyRectToSurface(_inventoryTexture, 12, kRunawayViewportTop + 12,
		Common::Rect(0, 0, 768, 576));
	for (uint slot = 0; slot < _inventory.size(); ++slot) {
		const byte item = _inventory.itemAt(slot);
		const Common::Rect rect = itemRect(slot);
		for (int edge = 0; edge < 4; ++edge) {
			for (int pixel = edge + 1; pixel < 88 - edge; ++pixel) {
				shadeBevelPixel(screen, rect.left - 4 + pixel, rect.top - 4 + edge, -1);
				shadeBevelPixel(screen, rect.left + 83 - pixel, rect.top + 83 - edge, 1);
				shadeBevelPixel(screen, rect.left - 4 + edge, rect.top + 83 - pixel, -2);
				shadeBevelPixel(screen, rect.left + 83 - edge, rect.top - 4 + pixel, 2);
			}
		}
		if (item == _inventory.selected() || (item == _pickupItem && _now - _pickupStarted < 12 * 60)) {
			screen.fillRect(rect, 0);
		} else {
			const uint y = (_inventory.icon(item) - 1) * 80;
			screen.copyRectToSurface(_icons, rect.left, rect.top, Common::Rect(0, y, 80, y + 80));
		}
	}
	if (_pickupItem) {
		_portrait.draw(screen, 64 + MIN<uint>(17, (_now - _pickupStarted) / 60), 0, kRunawayViewportTop);
	} else if (_idleStarted) {
		_idlePortrait.draw(screen, MIN<uint>(31, (_now - _idleStarted) / 60), 0, kRunawayViewportTop);
	} else {
		const uint frame = _speaking ? _mouth : (int32)(_blinkUntil - _now) > 0 ? 6 : 0;
		_portrait.draw(screen, frame, 0, kRunawayViewportTop);
	}
}

void RunawayUI::draw(Graphics::ManagedSurface &screen, const Common::U32String &roomCaption,
		const Common::U32String &committedCaption, const Common::U32String &subtitle, bool showSubtitles) {
	if (_inventoryOpen)
		drawInventory(screen);
	const Graphics::ManagedSurface &button = _inventoryOpen ? _returnButton : _inventoryButton;
	screen.copyRectToSurface(button, kButtonX, 0, Common::Rect(_buttonFrame * 100, 0, (_buttonFrame + 1) * 100, 74));
	screen.copyRectToSurface(_optionsButton, 612, 0, Common::Rect(_optionsFrame * 100, 0, (_optionsFrame + 1) * 100, 74));
	if (_speaking) {
		if (showSubtitles) {
			Common::Array<Common::U32String> lines;
			_font.wordWrapText(subtitle, 984, lines);
			for (uint i = 0; i < MIN<uint>(3, lines.size()); ++i)
				_font.drawString(screen.surfacePtr(), lines[i], 20, 684 + i * 28, 984,
					kUIFormat.RGBToColor(255, 223, 206), Graphics::kTextAlignCenter);
		}
	} else {
		Common::U32String caption;
		uint16 color = kUIFormat.RGBToColor(189, 166, 148);
		if (!_inventoryOpen && !committedCaption.empty()) {
			caption = committedCaption;
			color = kUIFormat.RGBToColor(255, 223, 206);
		} else if (!_blocked && !busy()) {
			caption = overOptions() ? _strings.text(RunawayStrings::kOptions) :
				overButton() ? _strings.text(_inventoryOpen ? RunawayStrings::kResume : RunawayStrings::kInventory) :
				_inventoryOpen ? inventoryCaption() : roomCaption;
		}
		_font.drawString(screen.surfacePtr(), caption, 20, 684, 984, color, Graphics::kTextAlignCenter);
	}
	// Native 0x4020b0 copies the demo logo to the bottom-right corner.
	screen.copyRectToSurface(_demoLogo, 976, 720, Common::Rect(0, 0, 48, 48));
}

} // End of namespace Hollywood
