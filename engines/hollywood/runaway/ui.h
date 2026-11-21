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
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/detection.h"
#include "hollywood/runaway/font.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/inventory.h"

namespace Hollywood {

class RunawayUI {
public:
	struct Command {
		enum Type { kNone, kRoom, kExamineItem, kCombine, kSkipSpeech, kOpenMenu, kChooseDialogue };
		Type type = kNone;
		RunawayVerb verb = kVerbWalk;
		Common::Point mouse;
		byte item = 0, target = 0;
	};
	RunawayUI(Audio::Mixer *mixer, RunawayInventory &inventory, const RunawayStrings &strings) :
		_audio(mixer), _inventory(inventory), _strings(strings), _random("runaway-ui") {}
	~RunawayUI();
	bool load(RunawayGameType game, byte chapter);
	bool loadChapter(byte chapter);
	void update(uint32 now, Common::Point mouse, byte roomHotspot, bool blocked, bool speaking,
		bool inventoryPresentation = false, byte speechFrame = 0, bool roomHasVerbs = true,
		byte actionCursor = 0, bool preferLook = false);
	void setDialogueChoices(const Common::Array<Common::U32String> &choices);
	Command click(bool right);
	void toggleInventory();
	bool cancel();
	bool inventoryOpen() const { return _inventoryOpen; }
	bool busy() const { return _pickupItem != 0; }
	RunawayVerb verb() const { return _verb; }
	byte hovered() const { return _hovered; }
	void captureState(RunawaySaveState &state) const;
	void restoreState(const RunawaySaveState &state, uint32 now);
	void setPointerSound(bool enabled) { _pointerSound = enabled; }
	void drawSubtitle(Graphics::ManagedSurface &screen, const Common::U32String &text) const;
	void draw(Graphics::ManagedSurface &screen, const Common::U32String &roomCaption,
		const Common::U32String &committedCaption, const Common::U32String &subtitle, bool showSubtitles);

private:
	Common::Rect itemRect(uint slot) const;
	byte itemAt(Common::Point mouse) const;
	bool overButton() const;
	bool overOptions() const;
	bool inViewport() const;
	void setInventoryOpen(bool open);
	void setVerb(RunawayVerb verb, byte actionCursor = 0);
	void updateCursor();
	Common::U32String inventoryCaption() const;
	void drawInventory(Graphics::ManagedSurface &screen);

	RunawayAudio _audio;
	RunawayInventory &_inventory;
	const RunawayStrings &_strings;
	bool _museum = false;
	Common::RandomSource _random;
	RunawayFont _font;
	Graphics::ManagedSurface _cursors, _demoLogo, _cursor, _inventoryButton, _returnButton, _optionsButton;
	Graphics::ManagedSurface _inventoryBackground, _inventoryTexture, _icons;
	RunawayAnimation _portrait, _idlePortrait;
	Common::Point _mouse;
	Common::Array<Common::Array<Common::U32String> > _choices;
	Common::Array<Common::Rect> _choiceRects;
	RunawayVerb _verb = kVerbWalk;
	byte _cursorType = 0, _previousCursor = 0, _actionCursor = 0;
	bool _preferLook = false;
	byte _hovered = 0, _buttonFrame = 0, _optionsFrame = 0, _pickupItem = 0;
	bool _inventoryOpen = false, _enteredInventory = false, _returnAfterPickup = false;
	bool _inventoryPresentation = false;
	bool _roomHasVerbs = true;
	bool _pointerSound = true;
	bool _pickupSoundPlayed = false;
	bool _blocked = false, _speaking = false, _cursorPushed = false;
	uint32 _now = 0, _buttonTick = 0, _verbStarted = 0, _portraitTick = 0;
	uint32 _blinkUntil = 0, _idleStarted = 0, _lastInteraction = 0, _pickupStarted = 0;
	uint _mouth = 0;
	int _cursorKey = -1;
};

} // End of namespace Hollywood

#endif
