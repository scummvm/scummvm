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

#ifndef HOLLYWOOD_RUNAWAY_MENU_H
#define HOLLYWOOD_RUNAWAY_MENU_H

#include "common/keyboard.h"
#include "engines/savestate.h"
#include "hollywood/runaway/font.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {

class RunawayMenu {
public:
	struct Settings {
		bool music = true, subtitles = true, antialias = true;
		bool fastScrolling = true, pointerSound = true;
		int volume = 192;
		bool operator==(const Settings &other) const;
		bool operator!=(const Settings &other) const { return !(*this == other); }
	};
	enum Command { kNone, kResume, kSettingsChanged, kSaveSettings, kRestoreDefaults, kQuit,
		kBrowseLoad, kBrowseSave, kBrowseDelete, kLoadGame, kSaveGame, kDeleteGame };

	RunawayMenu(Audio::Mixer *mixer, const RunawayStrings &strings) : _strings(strings), _audio(mixer) {}
	~RunawayMenu();
	bool load();
	void open(const Graphics::ManagedSurface &screen, const Settings &settings, bool canSave = false, bool canLoad = true);
	void close();
	Command update(Common::Point mouse);
	void tick(uint32 now);
	Command click(bool right);
	void release();
	Command cancel();
	Command key(uint32 ascii);
	Command key(const Common::KeyState &key);
	Command adjustVolume(int delta);
	Command adjust(int delta);
	void browse(Command mode, const SaveStateList &saves, int newSlot = -1);
	void showMessage(const Common::U32String &message) { _notice = message; }
	void operationDone(bool success);
	int selectedSlot() const { return _selectedSlot; }
	Common::String saveDescription() const { return _name.encode(); }
	bool enteringName() const { return _page == kName; }
	void draw(Graphics::ManagedSurface &screen);
	const Settings &settings() const { return _settings; }

private:
	enum Page { kOptions, kSaves, kName, kCredits };
	enum Confirmation { kNoConfirmation, kConfirmExit, kConfirmDelete, kConfirmOverwrite, kConfirmSettings, kConfirmDefaults };
	enum Control { kNoControl, kPlay, kExit, kLoad, kSave, kDelete,
		kMusicOn, kMusicOff, kAntialiasOn, kAntialiasOff, kScrollFast, kScrollSlow,
		kPointerOn, kPointerOff, kSubtitlesOn, kSubtitlesOff, kVolume,
		kShowCredits, kStoreSettings, kDefaults, kPrevious, kNext, kAcceptName, kYes, kNo };
	Control controlAt(Common::Point mouse) const;
	Common::Rect labelRect(RunawayStrings::TextId id, int center, int y) const;
	Common::U32String confirmation() const;
	Common::Rect answerRect(bool yes) const;
	Common::Rect saveRect(uint index) const;
	int saveAt(Common::Point mouse) const;
	Command selectSave(int index);
	Command accept();
	void drawSaves(Graphics::ManagedSurface &screen);
	void drawLabel(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int center, int y,
		bool enabled, Control control = kNoControl);
	void drawToggle(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int y,
		bool selected, Control on, Control off);

	const RunawayStrings &_strings;
	RunawayAudio _audio;
	RunawayFont _font, _captionFont;
	Graphics::ManagedSurface _background, _returnButton, _logo, _cursor, _thumbnail;
	RunawayAnimation _volumeDial;
	RunawayCredits _credits;
	SaveStateList _saves;
	Common::U32String _name, _notice;
	Settings _settings;
	Common::Point _mouse;
	Page _page = kOptions;
	Confirmation _confirmation = kNoConfirmation;
	Command _browseMode = kBrowseLoad;
	Control _dragging = kNoControl;
	int _dragX = 0, _dragValue = 0, _selectedSlot = -1, _firstSave = 0, _newSlot = -1;
	int _focusedSave = -1;
	uint32 _now = 0;
	bool _canSave = false, _canLoad = true, _cursorPushed = false;
};

} // End of namespace Hollywood

#endif
