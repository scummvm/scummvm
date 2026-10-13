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

#include "hollywood/runaway/font.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {

class RunawayMenu {
public:
	struct Settings {
		bool music = true;
		bool subtitles = true;
		int volume = 256;
	};
	enum Command { kNone, kResume, kSettingsChanged, kQuit };

	explicit RunawayMenu(const RunawayStrings &strings) : _strings(strings) {}
	~RunawayMenu();
	bool load();
	void open(const Graphics::ManagedSurface &screen, const Settings &settings);
	void close();
	Command update(Common::Point mouse);
	Command click(bool right);
	void release() { _dragging = false; }
	Command cancel();
	Command key(uint32 ascii);
	Command adjustVolume(int delta);
	void draw(Graphics::ManagedSurface &screen);
	const Settings &settings() const { return _settings; }

private:
	enum Control { kNoControl, kPlay, kExit, kMusicOn, kMusicOff, kSubtitlesOn, kSubtitlesOff, kVolume, kYes, kNo };
	Control controlAt(Common::Point mouse) const;
	Common::Rect labelRect(RunawayStrings::TextId id, int center, int y) const;
	Common::U32String confirmation() const;
	Common::Rect answerRect(bool yes) const;
	void drawLabel(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int center, int y,
		bool enabled, Control control = kNoControl);
	void drawToggle(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int y,
		bool enabled, bool selected, Control on = kNoControl, Control off = kNoControl);

	const RunawayStrings &_strings;
	RunawayFont _font, _captionFont;
	Graphics::ManagedSurface _background, _returnButton, _logo, _cursor, _thumbnail;
	Settings _settings;
	Common::Point _mouse;
	int _dragX = 0, _dragVolume = 0;
	bool _confirmQuit = false, _dragging = false;
	bool _cursorPushed = false;
};

} // End of namespace Hollywood

#endif
