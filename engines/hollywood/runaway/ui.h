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

#ifndef HOLLYWOOD_RUNAWAY_UI_H
#define HOLLYWOOD_RUNAWAY_UI_H

#include "common/random.h"
#include "hollywood/runaway/font.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"

namespace Hollywood {

class RunawayUI {
public:
	struct Command {
		enum Type { kNone, kRoom, kExamineItem, kCombine, kSkipSpeech };
		Type type = kNone;
		RunawayVerb verb = kVerbWalk;
		Common::Point mouse;
		byte item = 0, target = 0;
	};
	explicit RunawayUI(RunawayInventory &inventory) : _inventory(inventory), _random("runaway-ui") {}
	~RunawayUI();
	bool load();
	void update(uint32 now, Common::Point mouse, byte roomHotspot, bool blocked, bool speaking);
	Command click(bool right);
	void toggleInventory();
	bool cancel();
	bool inventoryOpen() const { return _inventoryOpen; }
	bool busy() const { return _pickupItem != 0; }
	RunawayVerb verb() const { return _verb; }
	byte hovered() const { return _hovered; }
	void draw(Graphics::ManagedSurface &screen, const Common::U32String &roomCaption,
		const Common::U32String &committedCaption, const Common::U32String &subtitle, bool showSubtitles);

private:
	bool loadSurface(uint chunk, int width, int height, Graphics::ManagedSurface &surface);
	Common::Rect itemRect(uint slot) const;
	byte itemAt(Common::Point mouse) const;
	bool overButton() const;
	bool inViewport() const;
	void setInventoryOpen(bool open);
	void setVerb(RunawayVerb verb);
	void updateCursor();
	Common::U32String inventoryCaption() const;
	void drawInventory(Graphics::ManagedSurface &screen);

	RunawayInventory &_inventory;
	Common::RandomSource _random;
	RunawayFont _font;
	Graphics::ManagedSurface _cursors, _waitCursor, _cursor, _inventoryButton, _returnButton;
	Graphics::ManagedSurface _inventoryBackground, _inventoryTexture, _icons;
	RunawayAnimation _portrait, _idlePortrait;
	Common::Point _mouse;
	RunawayVerb _verb = kVerbWalk, _previousVerb = kVerbWalk;
	byte _hovered = 0, _buttonFrame = 0, _pickupItem = 0;
	bool _inventoryOpen = false, _enteredInventory = false, _returnAfterPickup = false;
	bool _blocked = false, _speaking = false, _cursorPushed = false;
	uint32 _now = 0, _buttonTick = 0, _verbStarted = 0, _portraitTick = 0;
	uint32 _blinkUntil = 0, _idleStarted = 0, _lastInteraction = 0, _pickupStarted = 0;
	uint _mouth = 0;
	int _cursorKey = -1;
};

} // End of namespace Hollywood

#endif
