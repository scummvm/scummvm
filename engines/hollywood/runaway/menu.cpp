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
#include "hollywood/runaway/menu.h"

namespace Hollywood {
namespace {

const Graphics::PixelFormat kMenuFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
const uint16 kNormal = 0xbd32;
const uint16 kHighlight = 0xff19;
const uint16 kDisabled = 0x6b4d;
Common::Rect returnRect() {
	return Common::Rect(612, 0, 712, 74);
}

} // End of anonymous namespace

RunawayMenu::~RunawayMenu() {
	close();
}

bool RunawayMenu::load() {
	Graphics::ManagedSurface cursors;
	if (!_font.load(true) || !_captionFont.load() ||
			!loadRunawaySurface(132, 1024, 600, _background) ||
			!loadRunawaySurface(130, 700, 74, _returnButton) ||
			!loadRunawaySurface(118, 48, 48, _logo) || !loadRunawaySurface(114, 904, 540, cursors))
		return false;
	_cursor.create(60, 60, kMenuFormat);
	_cursor.copyRectToSurface(cursors, 0, 0, Common::Rect(60, 60));
	_thumbnail.create(204, 120, kMenuFormat);
	return true;
}

void RunawayMenu::open(const Graphics::ManagedSurface &screen, const Settings &settings) {
	close();
	_settings = settings;
	_settings.volume = CLIP(_settings.volume, 0, 256);
	_confirmQuit = _dragging = false;
	// The original options screen samples every fifth room pixel.
	for (int y = 0; y < _thumbnail.h; ++y) {
		uint16 *dst = (uint16 *)_thumbnail.getBasePtr(0, y);
		const uint16 *src = (const uint16 *)screen.getBasePtr(0, kRunawayViewportTop + y * 5);
		for (int x = 0; x < _thumbnail.w; ++x)
			dst[x] = src[x * 5];
	}
	const uint16 key = *(const uint16 *)_cursor.getPixels();
	CursorMan.pushCursor(_cursor.getPixels(), 60, 60, 30, 30, key, false, &kMenuFormat);
	_cursorPushed = true;
	CursorMan.showMouse(true);
}

void RunawayMenu::close() {
	if (_cursorPushed) {
		CursorMan.popCursor();
		_cursorPushed = false;
	}
	_dragging = false;
}

Common::Rect RunawayMenu::labelRect(RunawayStrings::TextId id, int center, int y) const {
	const int width = _font.getStringWidth(_strings.text(id));
	return Common::Rect(center - width / 2 - 10, y, center + (width + 1) / 2 + 10, y + 28);
}

Common::U32String RunawayMenu::confirmation() const {
	Common::U32String text = _strings.text(RunawayStrings::kConfirmQuit);
	text += U" [ ";
	text += _strings.text(RunawayStrings::kYes);
	text += U" / ";
	text += _strings.text(RunawayStrings::kNo);
	text += U" ]";
	return text;
}

Common::Rect RunawayMenu::answerRect(bool yes) const {
	Common::U32String prefix = _strings.text(RunawayStrings::kConfirmQuit);
	prefix += U" [ ";
	if (!yes) {
		prefix += _strings.text(RunawayStrings::kYes);
		prefix += U" / ";
	}
	const int x = (1024 - _captionFont.getStringWidth(confirmation())) / 2 + _captionFont.getStringWidth(prefix);
	return Common::Rect(x, 684, x + _captionFont.getStringWidth(_strings.text(yes ? RunawayStrings::kYes : RunawayStrings::kNo)), 712);
}

RunawayMenu::Control RunawayMenu::controlAt(Common::Point mouse) const {
	if (_confirmQuit) {
		if (answerRect(true).contains(mouse))
			return kYes;
		return answerRect(false).contains(mouse) ? kNo : kNoControl;
	}
	if (returnRect().contains(mouse) || labelRect(RunawayStrings::kPlay, 183, 460).contains(mouse))
		return kPlay;
	if (labelRect(RunawayStrings::kQuit, 183, 502).contains(mouse))
		return kExit;
	const int ys[] = {161, 329};
	for (uint i = 0; i < ARRAYSIZE(ys); ++i) {
		if (Common::Rect(716, ys[i], 846, ys[i] + 28).contains(mouse))
			return i ? kSubtitlesOn : kMusicOn;
		if (Common::Rect(855, ys[i], 995, ys[i] + 28).contains(mouse))
			return i ? kSubtitlesOff : kMusicOff;
	}
	const int dx = mouse.x - 525, dy = mouse.y - 516;
	if (dx * dx + dy * dy <= 52 * 52)
		return kVolume;
	return kNoControl;
}

RunawayMenu::Command RunawayMenu::update(Common::Point mouse) {
	_mouse = mouse;
	if (_dragging)
		return adjustVolume(_dragVolume + (mouse.x - _dragX) * 2 - _settings.volume);
	return kNone;
}

RunawayMenu::Command RunawayMenu::adjustVolume(int delta) {
	if (_confirmQuit)
		return kNone;
	const int volume = CLIP(_settings.volume + delta, 0, 256);
	if (volume == _settings.volume)
		return kNone;
	_settings.volume = volume;
	return kSettingsChanged;
}

RunawayMenu::Command RunawayMenu::click(bool right) {
	if (right)
		return cancel();
	switch (controlAt(_mouse)) {
	case kPlay:
		return kResume;
	case kExit:
		_confirmQuit = true;
		_dragging = false;
		return kNone;
	case kMusicOn:
	case kMusicOff:
		_settings.music = controlAt(_mouse) == kMusicOn;
		return kSettingsChanged;
	case kSubtitlesOn:
	case kSubtitlesOff:
		_settings.subtitles = controlAt(_mouse) == kSubtitlesOn;
		return kSettingsChanged;
	case kVolume:
		_dragging = true;
		_dragX = _mouse.x;
		_dragVolume = _settings.volume;
		return kNone;
	case kYes:
		return kQuit;
	case kNo:
		return cancel();
	default:
		return kNone;
	}
}

RunawayMenu::Command RunawayMenu::cancel() {
	_dragging = false;
	if (_confirmQuit) {
		_confirmQuit = false;
		return kNone;
	}
	return kResume;
}

RunawayMenu::Command RunawayMenu::key(uint32 ascii) {
	if (ascii >= 'a' && ascii <= 'z')
		ascii -= 'a' - 'A';
	if (_confirmQuit) {
		if (ascii == _strings.text(RunawayStrings::kYes)[0])
			return kQuit;
		if (ascii == _strings.text(RunawayStrings::kNo)[0])
			return cancel();
	} else {
		if (ascii == _strings.text(RunawayStrings::kPlay)[0])
			return kResume;
		if (ascii == _strings.text(RunawayStrings::kQuitKey)[0]) {
			_confirmQuit = true;
			_dragging = false;
		}
	}
	return kNone;
}

void RunawayMenu::drawLabel(Graphics::ManagedSurface &screen, RunawayStrings::TextId id,
		int center, int y, bool enabled, Control control) {
	const Common::Rect rect = labelRect(id, center, y);
	const bool highlighted = enabled && control != kNoControl && controlAt(_mouse) == control;
	_font.drawString(screen.surfacePtr(), _strings.text(id), rect.left, y, rect.width(),
		enabled ? (highlighted ? kHighlight : kNormal) : kDisabled, Graphics::kTextAlignCenter);
}

void RunawayMenu::drawToggle(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int y,
		bool enabled, bool selected, Control on, Control off) {
	_font.drawString(screen.surfacePtr(), _strings.text(id), 415, y, 300, enabled ? kNormal : kDisabled);
	for (uint i = 0; i < 2; ++i) {
		const Control control = i ? off : on;
		const bool active = enabled && (selected == (i == 0));
		const bool hovered = enabled && controlAt(_mouse) == control;
		const RunawayStrings::TextId label = id == RunawayStrings::kScrolling ?
			(i ? RunawayStrings::kSlow : RunawayStrings::kFast) : (i ? RunawayStrings::kOff : RunawayStrings::kOn);
		_font.drawString(screen.surfacePtr(), _strings.text(label), 756 + i * 139, y, 126,
			enabled ? ((active || hovered) ? kHighlight : kNormal) : kDisabled);
		if (active)
			_captionFont.drawChar(screen.surfacePtr(), 0xb7, 726 + i * 139, y, kHighlight);
	}
}

void RunawayMenu::draw(Graphics::ManagedSurface &screen) {
	screen.clear();
	screen.copyRectToSurface(_background, 0, kRunawayViewportTop, Common::Rect(1024, 600));
	screen.copyRectToSurface(_thumbnail, 85, 169, Common::Rect(204, 120));
	// Layout from native 0x4230b0. Unimplemented options stay disabled.
	drawLabel(screen, RunawayStrings::kLoad, 183, 334, false);
	drawLabel(screen, RunawayStrings::kSave, 183, 376, false);
	drawLabel(screen, RunawayStrings::kDelete, 183, 418, false);
	drawLabel(screen, RunawayStrings::kPlay, 183, 460, true, kPlay);
	drawLabel(screen, RunawayStrings::kQuit, 183, 502, true, kExit);
	drawToggle(screen, RunawayStrings::kMusic, 161, true, _settings.music, kMusicOn, kMusicOff);
	drawToggle(screen, RunawayStrings::kAntialias, 203, false, false);
	drawToggle(screen, RunawayStrings::kScrolling, 245, false, false);
	drawToggle(screen, RunawayStrings::kPointerSound, 287, false, false);
	drawToggle(screen, RunawayStrings::kSubtitles, 329, true, _settings.subtitles, kSubtitlesOn, kSubtitlesOff);
	drawLabel(screen, RunawayStrings::kCredits, 525, 392, false);
	drawLabel(screen, RunawayStrings::kSaveSettings, 802, 404, false);
	drawLabel(screen, RunawayStrings::kDefaultSettings, 802, 446, false);
	drawLabel(screen, RunawayStrings::kVolume, 525, 582, true, kVolume);
	drawLabel(screen, RunawayStrings::kBrightness, 802, 582, false);
	_font.drawString(screen.surfacePtr(), Common::String::format("%d / 100", (_settings.volume * 100 + 128) / 256),
		465, 616, 120, kNormal, Graphics::kTextAlignCenter);
	const uint frame = returnRect().contains(_mouse) ? 6 : 5;
	screen.copyRectToSurface(_returnButton, 612, 0, Common::Rect(frame * 100, 0, (frame + 1) * 100, 74));
	if (_confirmQuit) {
		_captionFont.drawString(screen.surfacePtr(), confirmation(), 0, 684, 1024, kNormal, Graphics::kTextAlignCenter);
		const Control hovered = controlAt(_mouse);
		if (hovered == kYes || hovered == kNo) {
			const Common::Rect rect = answerRect(hovered == kYes);
			_captionFont.drawString(screen.surfacePtr(), _strings.text(hovered == kYes ? RunawayStrings::kYes : RunawayStrings::kNo),
				rect.left, rect.top, rect.width(), kHighlight);
		}
	} else if (returnRect().contains(_mouse)) {
		_captionFont.drawString(screen.surfacePtr(), _strings.text(RunawayStrings::kResume), 0, 684, 1024, kNormal, Graphics::kTextAlignCenter);
	}
	screen.copyRectToSurface(_logo, 976, 720, Common::Rect(48, 48));
}

} // End of namespace Hollywood
