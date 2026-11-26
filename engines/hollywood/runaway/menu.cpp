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
#include "gui/saveload.h"
#include "hollywood/runaway/menu.h"
#include "hollywood/resource.h"

namespace Hollywood {
namespace {

const Graphics::PixelFormat kMenuFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
// Native menu palette entries 3, 2, 23, 4 and 0, rounded to RGB565.
const uint16 kNormal = 0xbd32, kHighlight = 0xfef9, kDisabled = 0x4186, kUnselected = 0x8bed, kShortcut = 0xffff;

Common::Rect returnRect() { return Common::Rect(612, 0, 712, 74); }
Common::Rect inventoryRect() { return Common::Rect(312, 0, 412, 74); }

void drawShortcut(Graphics::ManagedSurface &screen, const RunawayFont &font, const Common::U32String &text,
		int x, int y, uint32 key) {
	if (!key)
		return;
	uint32 previous = 0;
	for (uint i = 0; i < text.size(); ++i) {
		x += font.getKerningOffset(previous, text[i]);
		if (text[i] == key) {
			font.drawChar(screen.surfacePtr(), text[i], x, y, kShortcut);
			return;
		}
		x += font.getCharWidth(text[i]);
		previous = text[i];
	}
}

byte buttonFrame(byte frame, bool hovered, bool visible, int steps) {
	if (hovered)
		return 6;
	return visible ? MIN<int>(5, MIN<byte>(frame, 5) + steps) : MAX<int>(0, MIN<byte>(frame, 5) - steps);
}

void drawThumbnail(Graphics::ManagedSurface &screen, const Graphics::Surface &source, const Common::Rect &rect, bool dim = false) {
	if (!source.w || !source.h)
		return;
	for (int y = 0; y < rect.height(); ++y) {
		uint16 *out = (uint16 *)screen.getBasePtr(rect.left, rect.top + y);
		for (int x = 0; x < rect.width(); ++x) {
			byte r, g, b;
			source.format.colorToRGB(source.getPixel(x * source.w / rect.width(), y * source.h / rect.height()), r, g, b);
			out[x] = screen.format.RGBToColor(r, g, b);
			if (dim)
				out[x] = (out[x] >> 1) & 0x7bef;
		}
	}
}

uint16 bevelColor(uint16 pixel, int delta) {
	int r = pixel >> 11, g = (pixel >> 5) & 63, b = pixel & 31;
	for (int i = 0; i < ABS(delta); ++i) {
		if (delta < 0 && r && g && b) {
			--r;
			--g;
			--b;
		} else if (delta > 0 && r < 31 && g < 31 && b < 31) {
			++r;
			++g;
			++b;
		}
	}
	return (r << 11) | (g << 5) | b;
}

void drawSavePanel(Graphics::ManagedSurface &screen, const Graphics::ManagedSurface &background) {
	// Native 0x420390 shades four pixels of the menu background around the panel.
	screen.fillRect(Common::Rect(370, 100, 1008, 668), 0);
	for (int inset = 0; inset < 4; ++inset) {
		for (int x = inset + 1; x < 638 - inset; ++x) {
			screen.setPixel(370 + x, 100 + inset, bevelColor(background.getPixel(370 + x, 16 + inset), -1));
			screen.setPixel(1007 - x, 667 - inset, bevelColor(background.getPixel(1007 - x, 583 - inset), 1));
		}
		for (int y = inset + 1; y < 568 - inset; ++y) {
			screen.setPixel(370 + inset, 667 - y, bevelColor(background.getPixel(370 + inset, 583 - y), -2));
			screen.setPixel(1007 - inset, 100 + y, bevelColor(background.getPixel(1007 - inset, 16 + y), 2));
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

bool RunawayMenu::load(RunawayGameType game) {
	const bool museum = game == kRunawayMuseumDemo;
	_museum = museum;
	_fullGame = game == kRunawayFullGame;
	_audio.setMuseum(museum);
	Graphics::ManagedSurface cursors;
	Common::Array<byte> animation;
	if (!_font.load(true, museum, _strings.language()) || !_captionFont.load(false, museum, _strings.language()))
		return false;
	if (museum) {
		if (!loadRunawaySurface(Common::Path("panopc2.fnd"), 1024, 600, _background) ||
				!loadRunawaySurface(Common::Path("jugar_der.fnd"), 700, 74, _returnButton) ||
				!loadRunawaySurface(Common::Path("bolsa3.fnd"), 700, 74, _inventoryButton) ||
				!loadRunawaySurface(Common::Path("jugar_izq.fnd"), 700, 74, _inventoryReturnButton) ||
				!loadRunawaySurface(Common::Path("Mosca.spf"), 48, 48, _logo) ||
				!loadRunawaySurface(Common::Path("raton10.spf"), 904, 360, cursors))
			return false;
	} else if (!loadRunawaySurface(132, 1024, 600, _background) ||
			!loadRunawaySurface(130, 700, 74, _returnButton) ||
			!loadRunawaySurface(127, 700, 74, _inventoryButton) ||
			!loadRunawaySurface(129, 700, 74, _inventoryReturnButton) ||
			(game != kRunawayFullGame && !loadRunawaySurface(118, 48, 48, _logo)) ||
			!loadRunawaySurface(game == kRunawayFullGame ? 112 : 114, 904, 540, cursors) ||
			!readRunawayGlobalChunk(155, animation) || !_volumeDial.load(animation, 103))
		return false;
	_cursor.create(60, 60, kMenuFormat);
	_cursor.copyRectToSurface(cursors, 0, 0, Common::Rect(60, 60));
	_thumbnail.create(204, 120, kMenuFormat);
	if (game == kRunawayFullGame) {
		ChunkArchive archive;
		if (!archive.open(Common::Path("RESOURCE.A03")) || !loadRunawaySurface(128, 700, 74, _optionsButton)) {
			return false;
		}
		for (uint i = 0; i < ARRAYSIZE(_chapters); ++i) {
			if (!loadRunawaySurface(archive, i, 204, 120, _chapters[i][1])) {
				return false;
			}
			_chapters[i][0].create(204, 120, kMenuFormat);
			_chapters[i][0].clear();
			_chapters[i][0].transBlitFrom(_chapters[i][1], 0, false, 128);
		}
	}
	return true;
}

void RunawayMenu::open(const Graphics::ManagedSurface &screen, const Settings &settings, bool canSave, bool canLoad,
		byte chapters, bool inventoryOpen) {
	close();
	_chapterCount = MIN<byte>(chapters, 6);
	_selectedChapter = -1;
	_optionsFrame = 0;
	_inventoryFrame = 0;
	_inventoryOpen = inventoryOpen;
	_mouse = Common::Point(-1, -1);
	_settings = settings;
	_settings.volume = CLIP(_settings.volume, 0, 256);
	_canSave = canSave && !_chapterCount;
	_canLoad = canLoad;
	_page = _chapterCount ? kChapters : kOptions;
	_confirmation = kNoConfirmation;
	_notice.clear();
	_saves.clear();
	captureThumbnail(screen);
	const uint16 key = *(const uint16 *)_cursor.getPixels();
	CursorMan.pushCursor(_cursor.getPixels(), 60, 60, 30, 30, key, &kMenuFormat);
	_cursorPushed = true;
	CursorMan.showMouse(true);
}

void RunawayMenu::setChapterCount(byte chapters) {
	chapters = MIN<byte>(chapters, 6);
	if (_chapterCount != chapters) {
		_chapterCount = chapters;
		_selectedChapter = _chapterCount ? chapterAt(_mouse) : -1;
	}
}

void RunawayMenu::captureThumbnail(const Graphics::ManagedSurface &screen) {
	// Native 0x41fd50 averages 5x5 blocks of the room image.
	for (int y = 0; y < _thumbnail.h; ++y) {
		uint16 *dst = (uint16 *)_thumbnail.getBasePtr(0, y);
		for (int x = 0; x < _thumbnail.w; ++x) {
			uint r = 0, g = 0, b = 0;
			for (int row = 0; row < 5; ++row) {
				const uint16 *src = (const uint16 *)screen.getBasePtr(x * 5, kRunawayViewportTop + y * 5 + row);
				for (int column = 0; column < 5; ++column) {
					r += src[column] >> 11;
					g += (src[column] >> 5) & 63;
					b += src[column] & 31;
				}
			}
			dst[x] = ((r / 25) << 11) | ((g / 25) << 5) | (b / 25);
		}
	}
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
	const int width = _font.getStringWidth(text(id));
	return Common::Rect(center - width / 2 - 10, y, center + (width + 1) / 2 + 10, y + 28);
}

uint32 RunawayMenu::shortcut(Control control) const {
	RunawayStrings::TextId id;
	switch (control) {
	case kLoad: id = RunawayStrings::kLoad; break;
	case kSave: id = RunawayStrings::kSave; break;
	case kDelete: id = RunawayStrings::kDeleteKey; break;
	case kPlay: id = RunawayStrings::kPlay; break;
	case kExit: id = RunawayStrings::kQuitKey; break;
	default: return 0;
	}
	const Common::U32String label = text(id);
	return label.empty() ? 0 : label[0];
}

RunawayStrings::TextId RunawayMenu::toggleLabel(RunawayStrings::TextId id, bool on) const {
	if (id == RunawayStrings::kScrolling)
		return on ? RunawayStrings::kFast : RunawayStrings::kSlow;
	if (id == RunawayStrings::kVideoDefinition)
		return on ? RunawayStrings::kHigh : RunawayStrings::kLow;
	return on ? RunawayStrings::kOn : RunawayStrings::kOff;
}

Common::Rect RunawayMenu::toggleRect(RunawayStrings::TextId id, int y, bool on) const {
	const int x = on ? 746 : 885;
	return Common::Rect(x - 30, y, x + _font.getStringWidth(text(toggleLabel(id, on))) + 21, y + 29);
}

Common::U32String RunawayMenu::caption() const {
	if (!_notice.empty())
		return _notice;
	if (_dragging == kVolume)
		return _strings.menuHelp("dragVolume");
	if (_confirmation == kConfirmDelete)
		return _strings.menuHelp("confirmDelete");
	Control control = controlAt(_mouse);
	if (_confirmation == kConfirmExit)
		control = kExit;
	else if (_confirmation == kConfirmSettings)
		control = kStoreSettings;
	else if (_confirmation == kConfirmDefaults)
		control = kDefaults;
	const char *help = nullptr;
	switch (control) {
	case kPlay: return text(RunawayStrings::kResume);
	case kOpenInventory: return text(_inventoryOpen ? RunawayStrings::kResume : RunawayStrings::kInventory);
	case kLoad: help = "load"; break;
	case kSave: help = "save"; break;
	case kDelete: help = "delete"; break;
	case kExit: help = "quit"; break;
	case kMusicOn: help = "musicOn"; break;
	case kMusicOff: help = "musicOff"; break;
	case kAntialiasOn: help = "antialiasOn"; break;
	case kAntialiasOff: help = "antialiasOff"; break;
	case kScrollFast: help = "scrollFast"; break;
	case kScrollSlow: help = "scrollSlow"; break;
	case kPointerOn: help = "pointerOn"; break;
	case kPointerOff: help = "pointerOff"; break;
	case kSubtitlesOn: help = "subtitlesOn"; break;
	case kSubtitlesOff: help = "subtitlesOff"; break;
	case kVolume: help = "volume"; break;
	case kShowCredits: help = "credits"; break;
	case kStoreSettings: help = "saveSettings"; break;
	case kDefaults: help = "defaultSettings"; break;
	default: break;
	}
	if (!help && _page == kSaves)
		help = _browseMode == kBrowseLoad ? "chooseLoad" : _browseMode == kBrowseDelete ? "chooseDelete" : "save";
	return help ? _strings.menuHelp(help) : Common::U32String();
}

Common::U32String RunawayMenu::confirmation() const {
	Common::U32String prompt = text(RunawayStrings::kConfirmQuit);
	prompt += U" [ ";
	prompt += text(RunawayStrings::kYes);
	prompt += U" / ";
	prompt += text(RunawayStrings::kNo);
	prompt += U" ]";
	return prompt;
}

Common::Rect RunawayMenu::answerRect(bool yes) const {
	Common::U32String prefix = text(RunawayStrings::kConfirmQuit);
	prefix += U" [ ";
	if (!yes) {
		prefix += text(RunawayStrings::kYes);
		prefix += U" / ";
	}
	const int x = (1024 - _captionFont.getStringWidth(confirmation())) / 2 + _captionFont.getStringWidth(prefix);
	return Common::Rect(x, confirmationY(), x + _captionFont.getStringWidth(text(yes ? RunawayStrings::kYes : RunawayStrings::kNo)), confirmationY() + 28);
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
	if (_canSave && inventoryRect().contains(mouse))
		return kOpenInventory;
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
			return (i == 0 && !_canLoad) || (i == 1 && !_canSave) || (i == 2 && _museum) ? kNoControl : controls[i];
	}
	const RunawayStrings::TextId toggles[] = {RunawayStrings::kMusic, RunawayStrings::kAntialias,
		RunawayStrings::kScrolling, RunawayStrings::kPointerSound, RunawayStrings::kSubtitles};
	const bool selected[] = {_settings.music, _settings.antialias, _settings.fastScrolling, _settings.pointerSound, _settings.subtitles};
	for (uint i = 0; i < (_museum ? 3U : 5U); ++i) {
		if (_fullGame && i == 3)
			continue; // Native video definition selects DirectDraw's display depth.
		const int y = 161 + i * 42;
		if (!selected[i] && toggleRect(toggles[i], y, true).contains(mouse))
			return (Control)(kMusicOn + i * 2);
		if (selected[i] && toggleRect(toggles[i], y, false).contains(mouse))
			return (Control)(kMusicOff + i * 2);
	}
	const int dx = mouse.x - 525, dy = mouse.y - 516;
	if (dx * dx + dy * dy <= 52 * 52 || labelRect(RunawayStrings::kVolume, 525, 582).contains(mouse))
		return kVolume;
	if (!_museum && labelRect(RunawayStrings::kCredits, 525, 392).contains(mouse))
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
	if (mouse != _mouse && _page == kChapters) {
		_selectedChapter = chapterAt(mouse);
	}
	_mouse = mouse;
	if (_dragging == kVolume)
		return adjustVolume(_dragValue + (mouse.x - _dragX) * 256 / 100 - _settings.volume);
	return kNone;
}

void RunawayMenu::tick(uint32 now) {
	if (_page != kCredits) {
		const int steps = MIN<uint>(5, now / 20 - _now / 20);
		const bool enabled = _confirmation == kNoConfirmation && _dragging == kNoControl;
		_optionsFrame = buttonFrame(_optionsFrame, enabled && returnRect().contains(_mouse), enabled && _mouse.y < kRunawayViewportTop, steps);
		_inventoryFrame = buttonFrame(_inventoryFrame, enabled && inventoryRect().contains(_mouse), enabled && _mouse.y < kRunawayViewportTop, steps);
	}
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
		entry.setDescription(text(RunawayStrings::kNewSave));
		_saves.push_back(entry);
		_firstSave = MAX<int>(0, ((int)_saves.size() - 1) / 3 * 3 - 9);
	}
	if (_saves.empty())
		_notice = text(RunawayStrings::kNoSaves);
	_focusedSave = mode == kBrowseSave && newSlot >= 0 ? (int)_saves.size() - 1 : saveAt(_mouse);
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
		_notice = text(RunawayStrings::kDefaultsRestored);
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
	if (_dragging != kNoControl)
		return kNone;
	if (_page == kChapters) {
		if (returnRect().contains(_mouse)) {
			return cancel();
		}
		_selectedChapter = chapterAt(_mouse);
		return _selectedChapter >= 0 ? kStartChapter : kNone;
	}
	_notice.clear();
	if (_page == kSaves && _confirmation == kNoConfirmation && saveAt(_mouse) >= 0)
		return selectSave(saveAt(_mouse));
	const Control control = controlAt(_mouse);
	switch (control) {
	case kPlay:
		if (returnRect().contains(_mouse)) {
			if (!_chapterCount)
				return kResume;
			_page = kChapters;
			return kNone;
		}
		return cancel();
	case kOpenInventory: return kInventory;
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
			_notice = text(RunawayStrings::kCreditsFailed);
		break;
	case kPrevious: return adjust(1);
	case kNext: return adjust(-1);
	case kAcceptName:
		return kSaveGame;
	case kYes: return accept();
	case kNo: return cancel();
	default: break;
	}
	return kNone;
}

RunawayMenu::Command RunawayMenu::cancel() {
	const bool dragging = _dragging != kNoControl;
	release();
	if (dragging)
		return kNone;
	_notice.clear();
	if (_confirmation != kNoConfirmation) {
		_confirmation = kNoConfirmation;
		return kNone;
	}
	if (_page == kName) {
		_page = kSaves;
		return kNone;
	}
	if (_page == kChapters) {
		Graphics::ManagedSurface screen(kRunawayScreenWidth, kRunawayScreenHeight, kMenuFormat);
		drawChapters(screen);
		captureThumbnail(screen);
		_page = kOptions;
		return kNone;
	}
	if (_page != kOptions) {
		_page = kOptions;
		_audio.stopMusic();
		CursorMan.showMouse(true);
		return kNone;
	}
	if (_chapterCount) {
		_page = kChapters;
		return kNone;
	}
	return kResume;
}

RunawayMenu::Command RunawayMenu::key(uint32 ascii) {
	if (_dragging != kNoControl)
		return kNone;
	if (_page == kName && _confirmation == kNoConfirmation) {
		if (ascii >= 32 && ascii != 127 && _name.size() < 60)
			_name += ascii;
		return kNone;
	}
	if (ascii >= 'a' && ascii <= 'z')
		ascii -= 'a' - 'A';
	if (_confirmation != kNoConfirmation) {
		if (ascii == text(RunawayStrings::kYes)[0])
			return accept();
		if (ascii == text(RunawayStrings::kNo)[0])
			return cancel();
	} else if (_page == kOptions) {
		if (ascii == 'I' && _canSave)
			return kInventory;
		if (ascii == shortcut(kPlay)) {
			return cancel();
		}
		if (ascii == shortcut(kExit))
			_confirmation = kConfirmExit;
		else if (ascii == shortcut(kLoad) && _canLoad)
			return kBrowseLoad;
		else if (ascii == shortcut(kSave) && _canSave)
			return kBrowseSave;
		else if (!_museum && ascii == shortcut(kDelete))
			return kBrowseDelete;
	}
	return kNone;
}

RunawayMenu::Command RunawayMenu::key(const Common::KeyState &keyState) {
	if (keyState.keycode == Common::KEYCODE_ESCAPE || keyState.keycode == Common::KEYCODE_F5)
		return cancel();
	if (_dragging != kNoControl)
		return kNone;
	if (_page == kCredits)
		return (keyState.keycode == Common::KEYCODE_SPACE || keyState.keycode == Common::KEYCODE_RETURN ||
			keyState.keycode == Common::KEYCODE_KP_ENTER) ? cancel() : kNone;
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
			return kSaveGame;
		}
		if (keyState.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META))
			return kNone;
		return key(keyState.ascii);
	}
	if (keyState.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META))
		return kNone;
	if (_confirmation == kNoConfirmation) {
		if (_page == kChapters) {
			int next = MAX(0, _selectedChapter);
			switch (keyState.keycode) {
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_UP:
				--next;
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_DOWN:
				++next;
				break;
			case Common::KEYCODE_HOME:
				next = 0;
				break;
			case Common::KEYCODE_END:
				next = _chapterCount;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_SPACE:
				return _selectedChapter >= 0 ? kStartChapter : kNone;
			default:
				return kNone;
			}
			_selectedChapter = CLIP(next, 0, (int)_chapterCount);
			return kNone;
		}
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
			if (keyState.keycode == Common::KEYCODE_RETURN || keyState.keycode == Common::KEYCODE_KP_ENTER || keyState.keycode == Common::KEYCODE_SPACE) {
				return cancel();
			}
		}
	}
	return key(keyState.ascii);
}

Common::String RunawayMenu::saveDescription() const {
	Common::String description = _name.encode();
	description.trim();
	if (description.empty()) {
		GUI::SaveLoadChooser chooser(true);
		return chooser.createDefaultSaveDescription(_selectedSlot);
	}
	return description;
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
			_notice = text(RunawayStrings::kNoSaves);
	} else {
		_page = kOptions;
		_notice = text(RunawayStrings::kGameSaved);
	}
}

void RunawayMenu::drawLabel(Graphics::ManagedSurface &screen, RunawayStrings::TextId id,
		int center, int y, bool enabled, Control control) {
	const Common::Rect rect = labelRect(id, center, y);
	const bool browsing = (_page == kSaves || _page == kName) &&
		((_browseMode == kBrowseLoad && control == kLoad) || (_browseMode == kBrowseSave && control == kSave) ||
		 (_browseMode == kBrowseDelete && control == kDelete));
	const bool confirmed = (_confirmation == kConfirmExit && control == kExit) ||
		(_confirmation == kConfirmSettings && control == kStoreSettings) ||
		(_confirmation == kConfirmDefaults && control == kDefaults);
	enabled = (enabled || browsing) && (_confirmation == kNoConfirmation || confirmed || browsing) &&
		(_dragging == kNoControl || _dragging == control);
	const bool highlighted = enabled && control != kNoControl && (confirmed || browsing || _dragging == control || controlAt(_mouse) == control);
	_font.drawMenuString(screen.surfacePtr(), text(id), rect.left, y, rect.width(),
		enabled ? (highlighted ? kHighlight : kNormal) : kDisabled, Graphics::kTextAlignCenter);
	if (enabled)
		drawShortcut(screen, _font, text(id), center - _font.getStringWidth(text(id)) / 2, y, shortcut(control));
}

void RunawayMenu::drawToggle(Graphics::ManagedSurface &screen, RunawayStrings::TextId id, int y,
		bool selected, Control on, Control off, bool enabled) {
	enabled = enabled && _confirmation == kNoConfirmation && _dragging == kNoControl;
	const Control hovered = enabled ? controlAt(_mouse) : kNoControl;
	const bool highlighted = hovered != kNoControl && (hovered == on || hovered == off);
	_font.drawMenuString(screen.surfacePtr(), text(id), 415, y, 300, enabled ? (highlighted ? kHighlight : kNormal) : kDisabled);
	for (uint i = 0; i < 2; ++i) {
		const Control control = i ? off : on;
		const bool active = selected == (i == 0);
		const RunawayStrings::TextId label = toggleLabel(id, i == 0);
		const uint16 color = !enabled ? kDisabled : highlighted && hovered == control ? kHighlight : active ? kNormal : kUnselected;
		_font.drawMenuString(screen.surfacePtr(), text(label), 756 + i * 139, y, 129, color);
		_font.drawMenuString(screen.surfacePtr(), Common::U32String(1, 0xb7), 726 + i * 139, y, 28, active ? (enabled ? kNormal : kDisabled) : 0);
	}
}

void RunawayMenu::drawSaves(Graphics::ManagedSurface &screen) {
	drawSavePanel(screen, _background);
	if (_page == kName) {
		const Common::Rect preview(588, 252, 792, 372);
		if (_selectedSlot == _newSlot)
			screen.blitFrom(_thumbnail, Common::Point(preview.left, preview.top));
		else if (_focusedSave >= 0 && _saves[_focusedSave].getThumbnail())
			drawThumbnail(screen, *_saves[_focusedSave].getThumbnail(), preview);
		screen.frameRect(preview, kHighlight);
		_captionFont.drawMenuString(screen.surfacePtr(), text(RunawayStrings::kEnterSaveName), 386, 414, 610, kNormal);
		Common::U32String display = _name;
		while (!display.empty() && _captionFont.getStringWidth(display) > 590)
			display.erase(0, 1);
		display += U"_";
		_captionFont.drawMenuString(screen.surfacePtr(), display, 386, 460, 610, kHighlight);
		drawLabel(screen, RunawayStrings::kSave, 690, 518, true, kAcceptName);
		return;
	}
	for (uint i = 0; i < 12 && _firstSave + i < _saves.size(); ++i) {
		const SaveStateDescriptor &entry = _saves[_firstSave + i];
		const Common::Rect rect = saveRect(i);
		const bool selected = (_confirmation != kNoConfirmation || _page == kName) ?
			entry.getSaveSlot() == _selectedSlot : _focusedSave == (int)(_firstSave + i);
		if (entry.getSaveSlot() == _newSlot)
			drawThumbnail(screen, _thumbnail, rect, !selected);
		else if (entry.getThumbnail())
			drawThumbnail(screen, *entry.getThumbnail(), rect, !selected);
		_captionFont.drawMenuString(screen.surfacePtr(), Common::U32String::format("%u", _firstSave + i + 1),
			rect.left, rect.top + 116, rect.width(), selected ? kHighlight : kNormal, Graphics::kTextAlignCenter);
	}
	if (_firstSave > 0)
		_captionFont.drawChar(screen.surfacePtr(), 0xa4, 340, 355, controlAt(_mouse) == kPrevious ? kHighlight : kNormal);
	if (_firstSave + 12 < (int)_saves.size())
		_captionFont.drawChar(screen.surfacePtr(), 0xf0, 340, 385, controlAt(_mouse) == kNext ? kHighlight : kNormal);
}

Common::Rect RunawayMenu::chapterRect(uint chapter) const {
	// Original scene 1000 centers its thumbnails according to the unlocked chapter.
	static const Common::Point positions[6][7] = {
		{{410, 150}, {410, 290}},
		{{410, 150}, {298, 290}, {522, 290}},
		{{410, 150}, {186, 290}, {410, 290}, {634, 290}},
		{{410, 80}, {186, 220}, {410, 220}, {634, 220}, {410, 360}},
		{{410, 80}, {186, 220}, {410, 220}, {634, 220}, {298, 360}, {522, 360}},
		{{410, 80}, {186, 220}, {410, 220}, {634, 220}, {186, 360}, {410, 360}, {634, 360}}
	};
	assert(_chapterCount >= 1 && _chapterCount <= 6 && chapter <= _chapterCount);
	const Common::Point position = positions[_chapterCount - 1][chapter];
	return Common::Rect(position.x, position.y + kRunawayViewportTop, position.x + 204, position.y + kRunawayViewportTop + 120);
}

int RunawayMenu::chapterAt(Common::Point mouse) const {
	for (uint i = 0; i <= _chapterCount; ++i) {
		if (chapterRect(i).contains(mouse)) {
			return i;
		}
	}
	return -1;
}

void RunawayMenu::drawChapters(Graphics::ManagedSurface &screen) {
	screen.clear();
	for (uint i = 0; i <= _chapterCount; ++i) {
		const Common::Rect rect = chapterRect(i);
		screen.blitFrom(_chapters[i][i == (uint)_selectedChapter], Common::Point(rect.left, rect.top));
	}
	screen.copyRectToSurface(_optionsButton, 612, 0, Common::Rect(_optionsFrame * 100, 0, (_optionsFrame + 1) * 100, 74));
	Common::U32String caption;
	if (returnRect().contains(_mouse)) {
		caption = text(RunawayStrings::kOptions);
	} else if (_selectedChapter == 0) {
		caption = text(RunawayStrings::kPlayBeginning);
	} else if (_selectedChapter > 0) {
		caption = Common::U32String::format(text(RunawayStrings::kPlayChapter), _selectedChapter);
	}
	_captionFont.drawMenuString(screen.surfacePtr(), caption, 0, 684, 1024, kNormal, Graphics::kTextAlignCenter);
}

void RunawayMenu::draw(Graphics::ManagedSurface &screen) {
	if (_page == kChapters) {
		drawChapters(screen);
		return;
	}
	if (_page == kCredits) {
		_credits.draw(screen);
		return;
	}
	screen.clear();
	screen.copyRectToSurface(_background, 0, kRunawayViewportTop, Common::Rect(1024, 600));
	screen.copyRectToSurface(_thumbnail, 85, 169, Common::Rect(204, 120));
	drawLabel(screen, RunawayStrings::kLoad, 183, 334, _page == kOptions && _canLoad, kLoad);
	drawLabel(screen, RunawayStrings::kSave, 183, 376, _page == kOptions && _canSave, kSave);
	drawLabel(screen, RunawayStrings::kDelete, 183, 418, _page == kOptions && !_museum, kDelete);
	drawLabel(screen, RunawayStrings::kPlay, 183, 460, _page == kOptions, kPlay);
	drawLabel(screen, RunawayStrings::kQuit, 183, 502, _page == kOptions, kExit);
	if (_page == kSaves || _page == kName) {
		drawSaves(screen);
	} else {
		drawToggle(screen, RunawayStrings::kMusic, 161, _settings.music, kMusicOn, kMusicOff);
		drawToggle(screen, RunawayStrings::kAntialias, 203, _settings.antialias, kAntialiasOn, kAntialiasOff);
		drawToggle(screen, RunawayStrings::kScrolling, 245, _settings.fastScrolling, kScrollFast, kScrollSlow);
		if (!_museum) {
			if (_fullGame)
				drawToggle(screen, RunawayStrings::kVideoDefinition, 287, false, kNoControl, kNoControl, false);
			else
				drawToggle(screen, RunawayStrings::kPointerSound, 287, _settings.pointerSound, kPointerOn, kPointerOff);
			drawToggle(screen, RunawayStrings::kSubtitles, 329, _settings.subtitles, kSubtitlesOn, kSubtitlesOff);
		}
		drawLabel(screen, RunawayStrings::kCredits, 525, 392, !_museum, kShowCredits);
		drawLabel(screen, RunawayStrings::kSaveSettings, 802, 404, true, kStoreSettings);
		drawLabel(screen, RunawayStrings::kDefaultSettings, 802, 446, true, kDefaults);
		if (_museum) {
			const Common::String volume = Common::String::format("%d%%", (_settings.volume * 100 + 128) / 256);
			_font.drawMenuString(screen.surfacePtr(), volume, 473, 502, 104, kNormal, Graphics::kTextAlignCenter);
		} else {
			_volumeDial.draw(screen, _dragging == kVolume ? 102 : 101, 0, kRunawayViewportTop);
			_volumeDial.draw(screen, (_settings.volume * 100 + 128) / 256, 0, kRunawayViewportTop);
		}
		drawLabel(screen, RunawayStrings::kVolume, 525, 582, true, kVolume);
		drawLabel(screen, RunawayStrings::kBrightness, 802, 582, false);
	}
	screen.copyRectToSurface(_returnButton, 612, 0, Common::Rect(_optionsFrame * 100, 0, (_optionsFrame + 1) * 100, 74));
	if (_canSave && (_page == kOptions || _page == kSaves)) {
		const Graphics::ManagedSurface &button = _inventoryOpen ? _inventoryReturnButton : _inventoryButton;
		screen.copyRectToSurface(button, 312, 0, Common::Rect(_inventoryFrame * 100, 0, (_inventoryFrame + 1) * 100, 74));
	}
	const bool highlightCaption = !_notice.empty() || _dragging == kVolume ||
		(_page == kSaves && _confirmation == kNoConfirmation && controlAt(_mouse) != kPlay && controlAt(_mouse) != kOpenInventory);
	_captionFont.drawMenuString(screen.surfacePtr(), caption(), 0, 684, 1024,
		highlightCaption ? kHighlight : kNormal, Graphics::kTextAlignCenter);
	if (_page == kSaves && _focusedSave >= 0 && _focusedSave < (int)_saves.size()) {
		Common::U32String description = Common::U32String::format("%u ~ ", _focusedSave + 1);
		description += Common::U32String(_saves[_focusedSave].getDescription(), Common::kUtf8);
		_captionFont.drawMenuString(screen.surfacePtr(), description, 0, 712, 1024, kShortcut, Graphics::kTextAlignCenter);
	}
	if (_confirmation != kNoConfirmation) {
		_captionFont.drawMenuString(screen.surfacePtr(), confirmation(), 0, confirmationY(), 1024, kHighlight, Graphics::kTextAlignCenter);
		for (uint i = 0; i < 2; ++i) {
			const Common::Rect rect = answerRect(i == 0);
			const Common::U32String answer = text(i == 0 ? RunawayStrings::kYes : RunawayStrings::kNo);
			if (!answer.empty())
				drawShortcut(screen, _captionFont, answer, rect.left, rect.top, answer[0]);
		}
	}
	if (_logo.w)
		screen.copyRectToSurface(_logo, 976, 720, Common::Rect(48, 48));
}

} // End of namespace Hollywood
