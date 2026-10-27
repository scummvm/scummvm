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
const uint16 kNormal = 0xbd32, kHighlight = 0xff19, kDisabled = 0x6b4d;

Common::Rect returnRect() { return Common::Rect(612, 0, 712, 74); }

void drawThumbnail(Graphics::ManagedSurface &screen, const Graphics::Surface &source, const Common::Rect &rect) {
	if (!source.w || !source.h)
		return;
	for (int y = 0; y < rect.height(); ++y) {
		uint16 *out = (uint16 *)screen.getBasePtr(rect.left, rect.top + y);
		for (int x = 0; x < rect.width(); ++x) {
			byte r, g, b;
			source.format.colorToRGB(source.getPixel(x * source.w / rect.width(), y * source.h / rect.height()), r, g, b);
			out[x] = screen.format.RGBToColor(r, g, b);
		}
	}
}

} // End of anonymous namespace

bool RunawayMenu::Settings::operator==(const Settings &other) const {
	return music == other.music && subtitles == other.subtitles && antialias == other.antialias &&
		fastScrolling == other.fastScrolling && pointerSound == other.pointerSound &&
		volume == other.volume;
}

RunawayMenu::~RunawayMenu() { close(); }

bool RunawayMenu::load() {
	Graphics::ManagedSurface cursors;
	Common::Array<byte> animation;
	if (!_font.load(true) || !_captionFont.load() ||
			!loadRunawaySurface(132, 1024, 600, _background) ||
			!loadRunawaySurface(130, 700, 74, _returnButton) ||
			!loadRunawaySurface(118, 48, 48, _logo) || !loadRunawaySurface(114, 904, 540, cursors) ||
			!readRunawayGlobalChunk(155, animation) || !_volumeDial.load(animation, 103))
		return false;
	_cursor.create(60, 60, kMenuFormat);
	_cursor.copyRectToSurface(cursors, 0, 0, Common::Rect(60, 60));
	_thumbnail.create(204, 120, kMenuFormat);
	return true;
}

void RunawayMenu::open(const Graphics::ManagedSurface &screen, const Settings &settings, bool canSave, bool canLoad) {
	close();
	_settings = settings;
	_settings.volume = CLIP(_settings.volume, 0, 256);
	_canSave = canSave;
	_canLoad = canLoad;
	_page = kOptions;
	_confirmation = kNoConfirmation;
	_notice.clear();
	_saves.clear();
	// The original options screen samples every fifth room pixel.
	for (int y = 0; y < _thumbnail.h; ++y) {
		uint16 *dst = (uint16 *)_thumbnail.getBasePtr(0, y);
		const uint16 *src = (const uint16 *)screen.getBasePtr(0, kRunawayViewportTop + y * 5);
		for (int x = 0; x < _thumbnail.w; ++x)
			dst[x] = src[x * 5];
	}
	const uint16 key = *(const uint16 *)_cursor.getPixels();
	CursorMan.pushCursor(_cursor.getPixels(), 60, 60, 30, 30, key, &kMenuFormat);
	_cursorPushed = true;
	CursorMan.showMouse(true);
}

void RunawayMenu::close() {
	_audio.stopMusic();
	_audio.stopEffect();
	if (_cursorPushed) {
		CursorMan.popCursor();
		_cursorPushed = false;
	}
	_dragging = kNoControl;
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

Common::Rect RunawayMenu::saveRect(uint index) const {
	// Native 0x41b3b0: twelve 204x120 thumbnails on a 210x139 pitch.
	const int x = 377 + index % 3 * 210, y = 107 + index / 3 * 139;
	return Common::Rect(x, y, x + 204, y + 120);
}

int RunawayMenu::saveAt(Common::Point mouse) const {
	for (uint i = 0; i < 12 && _firstSave + i < _saves.size(); ++i) {
		if (saveRect(i).contains(mouse))
			return _firstSave + i;
	}
	return -1;
}

RunawayMenu::Control RunawayMenu::controlAt(Common::Point mouse) const {
	if (_page == kCredits)
		return kNoControl;
	if (_confirmation != kNoConfirmation) {
		if (answerRect(true).contains(mouse))
			return kYes;
		return answerRect(false).contains(mouse) ? kNo : kNoControl;
	}
	if (returnRect().contains(mouse))
		return kPlay;
	if (_page == kName)
		return labelRect(RunawayStrings::kSave, 690, 518).contains(mouse) ? kAcceptName : kNoControl;
	if (_page == kSaves) {
		if (Common::Rect(332, 345, 370, 379).contains(mouse) && _firstSave > 0)
			return kPrevious;
		if (Common::Rect(332, 384, 370, 418).contains(mouse) && _firstSave + 12 < (int)_saves.size())
			return kNext;
		return kNoControl;
	}
	const RunawayStrings::TextId labels[] = {RunawayStrings::kLoad, RunawayStrings::kSave, RunawayStrings::kDelete,
		RunawayStrings::kPlay, RunawayStrings::kQuit};
	const Control controls[] = {kLoad, kSave, kDelete, kPlay, kExit};
	for (uint i = 0; i < ARRAYSIZE(labels); ++i) {
		if (labelRect(labels[i], 183, 334 + i * 42).contains(mouse))
			return (i == 0 && !_canLoad) || (i == 1 && !_canSave) ? kNoControl : controls[i];
	}
	for (uint i = 0; i < 5; ++i) {
		const int y = 161 + i * 42;
		if (Common::Rect(716, y, 846, y + 28).contains(mouse))
			return (Control)(kMusicOn + i * 2);
		if (Common::Rect(855, y, 995, y + 28).contains(mouse))
			return (Control)(kMusicOff + i * 2);
	}
	const int dx = mouse.x - 525, dy = mouse.y - 516;
	if (dx * dx + dy * dy <= 52 * 52 || labelRect(RunawayStrings::kVolume, 525, 582).contains(mouse))
		return kVolume;
	if (labelRect(RunawayStrings::kCredits, 525, 392).contains(mouse))
		return kShowCredits;
	if (labelRect(RunawayStrings::kSaveSettings, 802, 404).contains(mouse))
		return kStoreSettings;
	if (labelRect(RunawayStrings::kDefaultSettings, 802, 446).contains(mouse))
		return kDefaults;
	return kNoControl;
}

RunawayMenu::Command RunawayMenu::update(Common::Point mouse) {
	if (mouse != _mouse && _page == kSaves && _confirmation == kNoConfirmation)
		_focusedSave = saveAt(mouse);
	_mouse = mouse;
	if (_dragging == kVolume)
		return adjustVolume(_dragValue + (mouse.x - _dragX) * 256 / 100 - _settings.volume);
	return kNone;
}

void RunawayMenu::tick(uint32 now) {
	_now = now;
	if (_page == kCredits) {
		_credits.update(now);
		if (_credits.finished())
			cancel();
	}
}

void RunawayMenu::release() {
	if (_dragging == kVolume)
		_audio.stopMusic();
	_dragging = kNoControl;
}

RunawayMenu::Command RunawayMenu::adjustVolume(int delta) {
	if (_confirmation != kNoConfirmation || _page != kOptions)
		return kNone;
	const int volume = CLIP(_settings.volume + delta, 0, 256);
	if (volume == _settings.volume)
		return kNone;
	_settings.volume = volume;
	return kSettingsChanged;
}

RunawayMenu::Command RunawayMenu::adjust(int delta) {
	if (_confirmation != kNoConfirmation)
		return kNone;
	if (_page == kSaves) {
		_firstSave = CLIP(_firstSave + (delta > 0 ? -3 : 3), 0, MAX(0, (((int)_saves.size() + 2) / 3 - 4) * 3));
		_focusedSave = saveAt(_mouse);
		return kNone;
	}
	if (_page != kOptions)
		return kNone;
	return adjustVolume(delta * 2);
}

void RunawayMenu::browse(Command mode, const SaveStateList &saves, int newSlot) {
	release();
	_page = kSaves;
	_browseMode = mode;
	_saves = saves;
	_newSlot = newSlot;
	_selectedSlot = -1;
	_firstSave = 0;
	_confirmation = kNoConfirmation;
	_notice.clear();
	if (mode == kBrowseSave && newSlot >= 0) {
		SaveStateDescriptor entry;
		entry.setSaveSlot(newSlot);
		entry.setDescription(_strings.text(RunawayStrings::kNewSave));
		_saves.push_back(entry);
		_firstSave = MAX<int>(0, ((int)_saves.size() - 1) / 3 * 3 - 9);
	}
	if (_saves.empty())
		_notice = _strings.text(RunawayStrings::kNoSaves);
	_focusedSave = _saves.empty() ? -1 : mode == kBrowseSave && newSlot >= 0 ? (int)_saves.size() - 1 : 0;
}

RunawayMenu::Command RunawayMenu::accept() {
	const Confirmation confirmation = _confirmation;
	_confirmation = kNoConfirmation;
	switch (confirmation) {
	case kConfirmExit: return kQuit;
	case kConfirmDelete: return kDeleteGame;
	case kConfirmOverwrite: _page = kName; return kNone;
	case kConfirmSettings: return kSaveSettings;
	case kConfirmDefaults:
		_settings = Settings();
		_notice = _strings.text(RunawayStrings::kDefaultsRestored);
		return kRestoreDefaults;
	default: return kNone;
	}
}

RunawayMenu::Command RunawayMenu::selectSave(int index) {
	if (index < 0 || index >= (int)_saves.size() || _saves[index].getLocked())
		return kNone;
	const SaveStateDescriptor &entry = _saves[index];
	_selectedSlot = entry.getSaveSlot();
	_focusedSave = index;
	_name = Common::U32String(entry.getDescription(), Common::kUtf8);
	if (_browseMode == kBrowseLoad)
		return kLoadGame;
	if (_browseMode == kBrowseDelete) {
		if (entry.getDeletableFlag())
			_confirmation = kConfirmDelete;
	} else if (_selectedSlot == _newSlot) {
		_name.clear();
		_page = kName;
	} else if (!entry.getWriteProtectedFlag()) {
		_confirmation = kConfirmOverwrite;
	}
	return kNone;
}

RunawayMenu::Command RunawayMenu::click(bool right) {
	if (right || _page == kCredits)
		return cancel();
	_notice.clear();
	if (_page == kSaves && _confirmation == kNoConfirmation && saveAt(_mouse) >= 0)
		return selectSave(saveAt(_mouse));
	const Control control = controlAt(_mouse);
	switch (control) {
	case kPlay: return _page == kOptions ? kResume : cancel();
	case kLoad: return kBrowseLoad;
	case kSave: return kBrowseSave;
	case kDelete: return kBrowseDelete;
	case kExit: _confirmation = kConfirmExit; break;
	case kStoreSettings: _confirmation = kConfirmSettings; break;
	case kDefaults: _confirmation = kConfirmDefaults; break;
	case kMusicOn: case kMusicOff: _settings.music = control == kMusicOn; return kSettingsChanged;
	case kAntialiasOn: case kAntialiasOff: _settings.antialias = control == kAntialiasOn; return kSettingsChanged;
	case kScrollFast: case kScrollSlow: _settings.fastScrolling = control == kScrollFast; return kSettingsChanged;
	case kPointerOn: case kPointerOff: _settings.pointerSound = control == kPointerOn; return kSettingsChanged;
	case kSubtitlesOn: case kSubtitlesOff: _settings.subtitles = control == kSubtitlesOn; return kSettingsChanged;
	case kVolume:
		_dragging = control;
		_dragX = _mouse.x;
		_dragValue = _settings.volume;
		_audio.playMenuMusic();
		break;
	case kShowCredits:
		if (_credits.load()) {
			_page = kCredits;
			_credits.start(_now);
			_audio.playMenuMusic();
			CursorMan.showMouse(false);
		} else
			_notice = _strings.text(RunawayStrings::kCreditsFailed);
		break;
	case kPrevious: return adjust(1);
	case kNext: return adjust(-1);
	case kAcceptName:
		if (_name.empty())
			_name = _strings.text(RunawayStrings::kNewSave);
		return kSaveGame;
	case kYes: return accept();
	case kNo: return cancel();
	default: break;
	}
	return kNone;
}

RunawayMenu::Command RunawayMenu::cancel() {
	release();
	_notice.clear();
	if (_confirmation != kNoConfirmation) {
		_confirmation = kNoConfirmation;
		return kNone;
	}
	if (_page == kName) {
		_page = kSaves;
		return kNone;
	}
	if (_page != kOptions) {
		_page = kOptions;
		_audio.stopMusic();
		CursorMan.showMouse(true);
		return kNone;
	}
	return kResume;
}

RunawayMenu::Command RunawayMenu::key(uint32 ascii) {
	if (_page == kName && _confirmation == kNoConfirmation) {
		if (ascii >= 32 && ascii != 127 && _name.size() < 60)
			_name += ascii;
		return kNone;
	}
	if (ascii >= 'a' && ascii <= 'z')
		ascii -= 'a' - 'A';
	if (_confirmation != kNoConfirmation) {
		if (ascii == _strings.text(RunawayStrings::kYes)[0])
			return accept();
		if (ascii == _strings.text(RunawayStrings::kNo)[0])
			return cancel();
	} else if (_page == kOptions) {
		if (ascii == _strings.text(RunawayStrings::kPlay)[0])
			return kResume;
		if (ascii == _strings.text(RunawayStrings::kQuitKey)[0])
			_confirmation = kConfirmExit;
		else if (ascii == _strings.text(RunawayStrings::kLoad)[0] && _canLoad)
			return kBrowseLoad;
		else if (ascii == _strings.text(RunawayStrings::kSave)[0] && _canSave)
			return kBrowseSave;
		else if (ascii == _strings.text(RunawayStrings::kDeleteKey)[0])
			return kBrowseDelete;
	}
	return kNone;
}

RunawayMenu::Command RunawayMenu::key(const Common::KeyState &keyState) {
	if (keyState.keycode == Common::KEYCODE_ESCAPE || keyState.keycode == Common::KEYCODE_F5)
		return cancel();
	if (_page == kCredits)
		return (keyState.keycode == Common::KEYCODE_SPACE || keyState.keycode == Common::KEYCODE_RETURN) ? cancel() : kNone;
	if (_page == kName && _confirmation == kNoConfirmation) {
		if (keyState.keycode == Common::KEYCODE_a && (keyState.flags & (Common::KBD_CTRL | Common::KBD_META))) {
			_name.clear();
			return kNone;
		}
		if (keyState.keycode == Common::KEYCODE_BACKSPACE) {
			if (!_name.empty())
				_name.deleteLastChar();
			return kNone;
		}
		if (keyState.keycode == Common::KEYCODE_RETURN || keyState.keycode == Common::KEYCODE_KP_ENTER) {
			if (_name.empty())
				_name = _strings.text(RunawayStrings::kNewSave);
			return kSaveGame;
		}
		if (keyState.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META))
			return kNone;
		return key(keyState.ascii);
	}
	if (_confirmation == kNoConfirmation) {
		if (_page == kSaves && !_saves.empty()) {
			int next = _focusedSave < 0 ? _firstSave : _focusedSave;
			switch (keyState.keycode) {
			case Common::KEYCODE_LEFT: --next; break;
			case Common::KEYCODE_RIGHT: ++next; break;
			case Common::KEYCODE_UP: next -= 3; break;
			case Common::KEYCODE_DOWN: next += 3; break;
			case Common::KEYCODE_PAGEUP: next -= 12; break;
			case Common::KEYCODE_PAGEDOWN: next += 12; break;
			case Common::KEYCODE_HOME: next = 0; break;
			case Common::KEYCODE_END: next = _saves.size() - 1; break;
			case Common::KEYCODE_RETURN: case Common::KEYCODE_KP_ENTER: case Common::KEYCODE_SPACE:
				return selectSave(next);
			default: return key(keyState.ascii);
			}
			_focusedSave = CLIP(next, 0, (int)_saves.size() - 1);
			if (_focusedSave < _firstSave)
				_firstSave = _focusedSave / 3 * 3;
			else if (_focusedSave >= _firstSave + 12)
				_firstSave = _focusedSave / 3 * 3 - 9;
			_notice.clear();
			return kNone;
		}
		if (_page == kOptions) {
			if (keyState.keycode == Common::KEYCODE_LEFT || keyState.keycode == Common::KEYCODE_RIGHT)
				return adjust(keyState.keycode == Common::KEYCODE_LEFT ? -4 : 4);
			if (keyState.keycode == Common::KEYCODE_RETURN || keyState.keycode == Common::KEYCODE_SPACE)
				return kResume;
		}
	}
	return key(keyState.ascii);
}

void RunawayMenu::operationDone(bool success) {
	if (!success)
		return;
	if (_browseMode == kBrowseDelete) {
		for (uint i = 0; i < _saves.size(); ++i) {
			if (_saves[i].getSaveSlot() == _selectedSlot) {
				_saves.remove_at(i);
				break;
			}
		}
		_firstSave = MIN(_firstSave, MAX<int>(0, ((int)_saves.size() - 1) / 3 * 3 - 9));
		_focusedSave = MIN(_focusedSave, (int)_saves.size() - 1);
		_selectedSlot = -1;
		if (_saves.empty())
			_notice = _strings.text(RunawayStrings::kNoSaves);
	} else {
		_page = kOptions;
		_notice = _strings.text(RunawayStrings::kGameSaved);
	}
}

void RunawayMenu::drawLabel(Graphics::ManagedSurface &screen, RunawayStrings::TextId id,
		int center, int y, bool enabled, Control control) {
	const Common::Rect rect = labelRect(id, center, y);
	const bool highlighted = enabled && control != kNoControl && controlAt(_mouse) == control;
	_font.drawString(screen.surfacePtr(), _strings.text(id), rect.left, y, rect.width(),
		enabled ? (highlighted ? kHighlight : kNormal) : kDisabled, Graphics::kTextAlignCenter);
}

void RunawayMenu::drawToggle(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int y,
		bool selected, Control on, Control off) {
	_font.drawString(screen.surfacePtr(), _strings.text(id), 415, y, 300, kNormal);
	for (uint i = 0; i < 2; ++i) {
		const Control control = i ? off : on;
		const bool active = selected == (i == 0);
		const RunawayStrings::TextId label = id == RunawayStrings::kScrolling ?
			(i ? RunawayStrings::kSlow : RunawayStrings::kFast) : (i ? RunawayStrings::kOff : RunawayStrings::kOn);
		_font.drawString(screen.surfacePtr(), _strings.text(label), 756 + i * 139, y, 126,
			active || controlAt(_mouse) == control ? kHighlight : kNormal);
		if (active)
			_captionFont.drawChar(screen.surfacePtr(), 0xb7, 726 + i * 139, y, kHighlight);
	}
}

void RunawayMenu::drawSaves(Graphics::ManagedSurface &screen) {
	screen.fillRect(Common::Rect(373, 100, 1010, 675), 0);
	if (_page == kName) {
		const Common::Rect preview(588, 252, 792, 372);
		if (_selectedSlot == _newSlot)
			screen.blitFrom(_thumbnail, Common::Point(preview.left, preview.top));
		else if (_focusedSave >= 0 && _saves[_focusedSave].getThumbnail())
			drawThumbnail(screen, *_saves[_focusedSave].getThumbnail(), preview);
		screen.frameRect(preview, kHighlight);
		_captionFont.drawString(screen.surfacePtr(), _strings.text(RunawayStrings::kEnterSaveName), 386, 414, 610, kNormal);
		Common::U32String display = _name;
		while (!display.empty() && _captionFont.getStringWidth(display) > 590)
			display.erase(0, 1);
		display += U"_";
		_captionFont.drawString(screen.surfacePtr(), display, 386, 460, 610, kHighlight);
		drawLabel(screen, RunawayStrings::kSave, 690, 518, true, kAcceptName);
		return;
	}
	for (uint i = 0; i < 12 && _firstSave + i < _saves.size(); ++i) {
		const SaveStateDescriptor &entry = _saves[_firstSave + i];
		const Common::Rect rect = saveRect(i);
		if (entry.getSaveSlot() == _newSlot)
			screen.blitFrom(_thumbnail, Common::Point(rect.left, rect.top));
		else if (entry.getThumbnail())
			drawThumbnail(screen, *entry.getThumbnail(), rect);
		const bool selected = (_confirmation != kNoConfirmation || _page == kName) ?
			entry.getSaveSlot() == _selectedSlot : _focusedSave == (int)(_firstSave + i);
		if (selected)
			screen.frameRect(rect, kHighlight);
		_font.drawString(screen.surfacePtr(), Common::String::format("%d", entry.getSaveSlot()),
			rect.left, rect.top + 116, rect.width(), selected ? kHighlight : kNormal, Graphics::kTextAlignCenter);
	}
	if (_firstSave > 0)
		_captionFont.drawChar(screen.surfacePtr(), 0xa4, 340, 355, controlAt(_mouse) == kPrevious ? kHighlight : kNormal);
	if (_firstSave + 12 < (int)_saves.size())
		_captionFont.drawChar(screen.surfacePtr(), 0xf0, 340, 385, controlAt(_mouse) == kNext ? kHighlight : kNormal);
}

void RunawayMenu::draw(Graphics::ManagedSurface &screen) {
	if (_page == kCredits) {
		_credits.draw(screen);
		return;
	}
	screen.clear();
	screen.copyRectToSurface(_background, 0, kRunawayViewportTop, Common::Rect(1024, 600));
	screen.copyRectToSurface(_thumbnail, 85, 169, Common::Rect(204, 120));
	drawLabel(screen, RunawayStrings::kLoad, 183, 334, _page == kOptions && _canLoad, kLoad);
	drawLabel(screen, RunawayStrings::kSave, 183, 376, _page == kOptions && _canSave, kSave);
	drawLabel(screen, RunawayStrings::kDelete, 183, 418, _page == kOptions, kDelete);
	drawLabel(screen, RunawayStrings::kPlay, 183, 460, _page == kOptions, kPlay);
	drawLabel(screen, RunawayStrings::kQuit, 183, 502, _page == kOptions, kExit);
	if (_page == kSaves || _page == kName) {
		drawSaves(screen);
	} else {
		drawToggle(screen, RunawayStrings::kMusic, 161, _settings.music, kMusicOn, kMusicOff);
		drawToggle(screen, RunawayStrings::kAntialias, 203, _settings.antialias, kAntialiasOn, kAntialiasOff);
		drawToggle(screen, RunawayStrings::kScrolling, 245, _settings.fastScrolling, kScrollFast, kScrollSlow);
		drawToggle(screen, RunawayStrings::kPointerSound, 287, _settings.pointerSound, kPointerOn, kPointerOff);
		drawToggle(screen, RunawayStrings::kSubtitles, 329, _settings.subtitles, kSubtitlesOn, kSubtitlesOff);
		drawLabel(screen, RunawayStrings::kCredits, 525, 392, true, kShowCredits);
		drawLabel(screen, RunawayStrings::kSaveSettings, 802, 404, true, kStoreSettings);
		drawLabel(screen, RunawayStrings::kDefaultSettings, 802, 446, true, kDefaults);
		_volumeDial.draw(screen, _dragging == kVolume ? 102 : 101, 0, kRunawayViewportTop);
		_volumeDial.draw(screen, (_settings.volume * 100 + 128) / 256, 0, kRunawayViewportTop);
		drawLabel(screen, RunawayStrings::kVolume, 525, 582, true, kVolume);
		drawLabel(screen, RunawayStrings::kBrightness, 802, 582, false);
	}
	const uint frame = returnRect().contains(_mouse) ? 6 : 5;
	screen.copyRectToSurface(_returnButton, 612, 0, Common::Rect(frame * 100, 0, (frame + 1) * 100, 74));
	if (_confirmation != kNoConfirmation) {
		_captionFont.drawString(screen.surfacePtr(), confirmation(), 0, 684, 1024, kNormal, Graphics::kTextAlignCenter);
		const Control hovered = controlAt(_mouse);
		if (hovered == kYes || hovered == kNo) {
			const Common::Rect rect = answerRect(hovered == kYes);
			_captionFont.drawString(screen.surfacePtr(), _strings.text(hovered == kYes ? RunawayStrings::kYes : RunawayStrings::kNo),
				rect.left, rect.top, rect.width(), kHighlight);
		}
	} else {
		Common::U32String caption = _notice;
		if (caption.empty() && _page == kSaves) {
			const int index = _focusedSave;
			if (index >= 0)
				caption = Common::U32String(_saves[index].getDescription(), Common::kUtf8);
		} else if (caption.empty() && returnRect().contains(_mouse)) {
			caption = _strings.text(RunawayStrings::kResume);
		}
		_captionFont.drawString(screen.surfacePtr(), caption, 0, 684, 1024, kNormal, Graphics::kTextAlignCenter);
	}
	screen.copyRectToSurface(_logo, 976, 720, Common::Rect(48, 48));
}

} // End of namespace Hollywood
