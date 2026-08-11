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
 */

#include "mads/nebular/bonus/bonus_text_ui.h"

#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/pixelformat.h"
#include "mads/core/mouse.h"
#include "mads/core/timer.h"
#include "mads/mads.h"

namespace MADS {
namespace RexNebular {

namespace {

static const byte kDesktopAttribute = 0x17;
static const byte kDesktopCharacter = 0xB1;
static const byte kShadowAttribute = 0x08;
static const byte kNormalAttribute = 0x47;
static const byte kDisabledAttribute = 0x48;
static const byte kSelectedAttribute = 0x74;
static const byte kHotkeyAttribute = 0x4F;
static const byte kSelectedHotkeyAttribute = 0x7F;
static const byte kTextCursorXorMask = 0x77;
static const byte kTitleAttribute = 0x1F;
static const uint32 kGoodbyeDuration = 3000;

} // namespace

BonusTextUI::BonusTextUI(const BonusExeData &text) :
		_text(text), _mouseCell(0, 0), _mouseCellValid(false),
		_restoreSystemCursor(false), _titleRect(1, 1, 25, 6),
		_mainRect(24, 9, 54, 20), _musicRect(40, 1, 70, 23),
		_viewerRect(5, 4, 76, 19), _nowPlayingRect(24, 10, 56, 15) {
}

BonusTextUI::~BonusTextUI() {
	_surface.free();
	if (_restoreSystemCursor)
		mouse_show();
}

bool BonusTextUI::init(Common::String &) {
	_surface.create(DOSTextScreen::kRasterWidth, DOSTextScreen::kRasterHeight,
			Graphics::PixelFormat::createFormatCLUT8());

	_restoreSystemCursor = !mouse_hidden;
	restorePresentation();
	return true;
}

void BonusTextUI::drawDesktop() {
	_cells.clear(kDesktopCharacter, kDesktopAttribute);
}

void BonusTextUI::drawTitlePanel() {
	_cells.drawShadow(_titleRect, kShadowAttribute);
	_cells.fill(Common::Rect(_titleRect.left + 1, _titleRect.top + 1,
			_titleRect.right - 1, _titleRect.bottom - 1), 0x20, kTitleAttribute);
	_cells.drawBox(_titleRect, kTitleAttribute, true);

	for (int line = 0; line < 3; ++line) {
		_cells.drawCenteredText(_titleRect.top + 1 + line,
				_titleRect.left + 1, _titleRect.right - 1,
				_text.title[line], kTitleAttribute, kTitleAttribute, false);
	}
}

int BonusTextUI::itemRow(const Common::Rect &rect, int itemCount, int index) {
	if (index < 0 || index >= itemCount)
		return -1;
	if (index == itemCount - 1)
		return rect.bottom - 2;
	return rect.top + 3 + index;
}

void BonusTextUI::drawMenu(const Common::Rect &rect, const Common::String &title,
		const Common::String *items, int itemCount, int selected,
		const bool *enabled) {
	_cells.drawShadow(rect, kShadowAttribute);
	_cells.fill(Common::Rect(rect.left + 1, rect.top + 1,
			rect.right - 1, rect.bottom - 1), 0x20, kNormalAttribute);
	_cells.drawBox(rect, kNormalAttribute, true);

	_cells.drawCenteredText(rect.top + 1, rect.left + 1, rect.right - 1,
			title, kNormalAttribute, kHotkeyAttribute, false);
	_cells.drawSeparator(rect, rect.top + 2, kNormalAttribute);
	_cells.drawSeparator(rect, rect.bottom - 3, kNormalAttribute);

	for (int index = 0; index < itemCount; ++index) {
		const int y = itemRow(rect, itemCount, index);
		const bool itemEnabled = !enabled || enabled[index];
		const byte attribute = !itemEnabled ? kDisabledAttribute :
				(index == selected ? kSelectedAttribute : kNormalAttribute);
		const byte hotkeyAttribute = !itemEnabled ? kDisabledAttribute :
				(index == selected ? kSelectedHotkeyAttribute : kHotkeyAttribute);

		_cells.fill(Common::Rect(rect.left + 1, y, rect.right - 1, y + 1),
				0x20, attribute);
		_cells.drawText(rect.left + 2, y, items[index], attribute,
				hotkeyAttribute, true, rect.width() - 4);
	}
}

int BonusTextUI::nextEnabled(const bool *enabled, int itemCount,
		int selected, int direction) {
	if (!enabled)
		return (selected + itemCount + direction) % itemCount;

	for (int count = 0; count < itemCount; ++count) {
		selected = (selected + itemCount + direction) % itemCount;
		if (enabled[selected])
			return selected;
	}
	return selected;
}

void BonusTextUI::drawMouseCursor() {
	if (_mouseCellValid && _cells.isValidCell(_mouseCell.x, _mouseCell.y)) {
		const byte attribute = _cells.getCell(_mouseCell.x, _mouseCell.y).attribute;
		_cells.setAttribute(_mouseCell.x, _mouseCell.y,
				attribute ^ kTextCursorXorMask);
	}
}

void BonusTextUI::present(bool forcePalette) {
	if (forcePalette)
		DOSTextScreen::installVGAPalette();
	if (!_cells.render(_surface))
		return;

	g_system->copyRectToScreen(_surface.getPixels(), _surface.pitch,
			0, 0, _surface.w, _surface.h);
	g_system->updateScreen();
}

void BonusTextUI::restorePresentation() {
	mouse_hide();
	updateMouseCell(g_system->getEventManager()->getMousePos());
	DOSTextScreen::installVGAPalette();
}

void BonusTextUI::updateMouseCell(const Common::Point &position) {
	_mouseCell.x = position.x / DOSTextScreen::kCellWidth;
	_mouseCell.y = position.y / DOSTextScreen::kCellHeight;
	_mouseCellValid = _cells.isValidCell(_mouseCell.x, _mouseCell.y);
}

int BonusTextUI::rowAtMouse(const Common::Rect &rect, int itemCount,
		int mouseX, int mouseY) {
	const int cellX = mouseX / DOSTextScreen::kCellWidth;
	const int cellY = mouseY / DOSTextScreen::kCellHeight;
	if (cellX <= rect.left || cellX >= rect.right - 1)
		return -1;

	for (int index = 0; index < itemCount; ++index) {
		if (cellY == itemRow(rect, itemCount, index))
			return index;
	}
	return -1;
}

int BonusTextUI::acceleratorChoice(const Common::String *items, int itemCount,
		int ascii) {
	if (ascii >= 'A' && ascii <= 'Z')
		ascii += 'a' - 'A';

	for (int index = 0; index < itemCount; ++index) {
		int accelerator = (byte)DOSTextScreen::accelerator(items[index]);
		if (accelerator >= 'A' && accelerator <= 'Z')
			accelerator += 'a' - 'A';
		if (accelerator && accelerator == ascii)
			return index;
	}
	return -1;
}

bool BonusTextUI::processQuitEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_QUIT &&
			event.type != Common::EVENT_RETURN_TO_LAUNCHER)
		return false;

	g_engine->quitGame();
	return true;
}

bool BonusTextUI::processGameMenuEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_MAINMENU &&
			(event.type != Common::EVENT_CUSTOM_ENGINE_ACTION_START ||
			event.customType != kActionGameMenu))
		return false;

	g_engine->openMainMenuDialog();
	restorePresentation();
	return true;
}

int BonusTextUI::runMenu(const Common::Rect &rect, const Common::String &title,
		const Common::String *items, int itemCount, int &selected,
		const bool *enabled, bool showPCSpeakerNotice) {
	restorePresentation();
	selected = CLIP<int>(selected, 0, itemCount - 1);
	if (enabled && !enabled[selected])
		selected = nextEnabled(enabled, itemCount, selected - 1, 1);
	bool dirty = true;

	while (!g_engine->shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (processQuitEvent(event))
				return -1;
			if (processGameMenuEvent(event)) {
				dirty = true;
				continue;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				updateMouseCell(event.mouse);
				const int hit = rowAtMouse(rect, itemCount,
						event.mouse.x, event.mouse.y);
				if (hit >= 0 && (!enabled || enabled[hit]))
					selected = hit;
				dirty = true;
				break;
			}
			case Common::EVENT_LBUTTONDOWN: {
				updateMouseCell(event.mouse);
				const int hit = rowAtMouse(rect, itemCount,
						event.mouse.x, event.mouse.y);
				if (hit >= 0 && (!enabled || enabled[hit])) {
					selected = hit;
					return selected;
				}
				dirty = true;
				break;
			}
			case Common::EVENT_KEYDOWN: {
				const Common::KeyCode key = event.kbd.keycode;
				if (key == Common::KEYCODE_UP) {
					selected = nextEnabled(enabled, itemCount, selected, -1);
					dirty = true;
				} else if (key == Common::KEYCODE_DOWN) {
					selected = nextEnabled(enabled, itemCount, selected, 1);
					dirty = true;
				} else if (key == Common::KEYCODE_HOME) {
					selected = nextEnabled(enabled, itemCount, -1, 1);
					dirty = true;
				} else if (key == Common::KEYCODE_END) {
					selected = nextEnabled(enabled, itemCount, 0, -1);
					dirty = true;
				} else if (key == Common::KEYCODE_RETURN ||
						key == Common::KEYCODE_KP_ENTER) {
					return selected;
				} else if (key == Common::KEYCODE_ESCAPE) {
					selected = itemCount - 1;
					return selected;
				} else {
					const int hit = acceleratorChoice(items, itemCount,
							event.kbd.ascii);
					if (hit >= 0 && (!enabled || enabled[hit])) {
						selected = hit;
						return selected;
					}
				}
				break;
			}
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionEscape) {
					selected = itemCount - 1;
					return selected;
				}
				break;
			default:
				break;
			}
		}

		if (dirty) {
			drawDesktop();
			drawTitlePanel();
			if (showPCSpeakerNotice) {
				_cells.drawCenteredText(11, 1, 39,
						"Some Musical Options do not", kTitleAttribute,
						kTitleAttribute, false);
				_cells.drawCenteredText(12, 1, 39,
						"work with PC Speaker.", kTitleAttribute,
						kTitleAttribute, false);
			}
			drawMenu(rect, title, items, itemCount, selected, enabled);
			drawMouseCursor();
			present(true);
			dirty = false;
		}
		g_system->delayMillis(10);
	}

	return -1;
}

BonusTextUI::MainChoice BonusTextUI::runMainMenu(int &selected) {
	const int result = runMenu(_mainRect, _text.mainMenuTitle,
			_text.mainMenu, ARRAYSIZE(_text.mainMenu), selected);
	return result < 0 ? kAbort : (MainChoice)result;
}

int BonusTextUI::runMusicMenu(int &selected, const bool *enabled) {
	Common::String items[17];
	for (uint index = 0; index < ARRAYSIZE(_text.musicTitles); ++index)
		items[index] = _text.musicTitles[index];
	items[16] = _text.musicExit;

	return runMenu(_musicRect, _text.musicMenuTitle,
			items, ARRAYSIZE(items), selected, enabled, enabled != nullptr);
}

void BonusTextUI::appendWrappedLine(const Common::String &source, int width,
		Common::Array<Common::String> &lines) {
	if (source.empty()) {
		lines.push_back(Common::String());
		return;
	}

	Common::String remaining = source;
	while ((int)remaining.size() > width) {
		int split = width;
		while (split > 0 && remaining[split] != ' ')
			--split;
		if (split == 0)
			split = width;

		lines.push_back(remaining.substr(0, split));
		while (split < (int)remaining.size() && remaining[split] == ' ')
			++split;
		remaining = remaining.substr(split);
	}
	lines.push_back(remaining);
}

bool BonusTextUI::showBonusText(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename))
		return false;

	Common::Array<Common::String> lines;
	const int textWidth = _viewerRect.width() - 2;
	while (!file.eos() && !file.err()) {
		const Common::String line = file.readLine();
		if (file.err())
			break;
		if (file.eos() && line.empty())
			break;
		appendWrappedLine(line, textWidth, lines);
	}
	if (lines.empty())
		lines.push_back(Common::String());
	restorePresentation();

	const int firstTextRow = _viewerRect.top + 1;
	const int linesPerPage = _viewerRect.height() - 2;
	int firstLine = 0;
	bool dirty = true;

	while (!g_engine->shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (processQuitEvent(event))
				return false;
			if (processGameMenuEvent(event)) {
				dirty = true;
				continue;
			}

			if (Common::isMouseEvent(event)) {
				updateMouseCell(event.mouse);
				dirty = true;
			}
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return true;
				if (event.kbd.keycode == Common::KEYCODE_SPACE) {
					if (firstLine + linesPerPage >= (int)lines.size())
						return true;
					firstLine += linesPerPage;
					dirty = true;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
					event.customType == kActionEscape) {
				return true;
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				if (firstLine + linesPerPage >= (int)lines.size())
					return true;
				firstLine += linesPerPage;
				dirty = true;
			} else if (event.type == Common::EVENT_RBUTTONDOWN) {
				return true;
			}
		}

		if (dirty) {
			drawDesktop();
			drawTitlePanel();
			_cells.drawShadow(_viewerRect, kShadowAttribute);
			_cells.fill(Common::Rect(_viewerRect.left + 1, _viewerRect.top + 1,
					_viewerRect.right - 1, _viewerRect.bottom - 1),
					0x20, kNormalAttribute);
			_cells.drawBox(_viewerRect, kNormalAttribute, true);

			Common::String viewerTitle = _text.bonusTextFilename;
			viewerTitle.toUppercase();
			_cells.drawTitle(_viewerRect, viewerTitle,
					kNormalAttribute, kHotkeyAttribute);
			for (int row = 0; row < linesPerPage &&
					firstLine + row < (int)lines.size(); ++row) {
				_cells.drawText(_viewerRect.left + 1, firstTextRow + row,
						lines[firstLine + row], kNormalAttribute,
						kHotkeyAttribute, false, textWidth);
			}
			_cells.drawFooter(_viewerRect, _text.continuePrompt,
					kNormalAttribute, kHotkeyAttribute);
			drawMouseCursor();
			present(true);
			dirty = false;
		}
		g_system->delayMillis(10);
	}

	return false;
}

void BonusTextUI::prepareNowPlaying(const Common::String &trackTitle) {
	restorePresentation();
	drawDesktop();
	drawTitlePanel();
	_cells.drawShadow(_nowPlayingRect, kShadowAttribute);
	_cells.fill(Common::Rect(_nowPlayingRect.left + 1,
			_nowPlayingRect.top + 1, _nowPlayingRect.right - 1,
			_nowPlayingRect.bottom - 1), 0x20, kNormalAttribute);
	_cells.drawBox(_nowPlayingRect, kNormalAttribute, true);
	_cells.drawTitle(_nowPlayingRect, _text.nowPlaying,
			kNormalAttribute, kHotkeyAttribute);
	_cells.drawCenteredText(_nowPlayingRect.top + 2,
			_nowPlayingRect.left + 1, _nowPlayingRect.right - 1,
			trackTitle, kNormalAttribute, kHotkeyAttribute, false);
	drawMouseCursor();
	present(true);
}

void BonusTextUI::waitForNowPlaying(const Common::String &trackTitle,
		Common::Functor0<bool> &isPlaying) {
	bool dirty = false;
	bool completed = false;
	long completionTime = 0;
	while (!g_engine->shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (processQuitEvent(event))
				return;
			if (processGameMenuEvent(event)) {
				dirty = true;
				continue;
			}
			if (Common::isMouseEvent(event)) {
				updateMouseCell(event.mouse);
				dirty = true;
			}
			if (event.type == Common::EVENT_KEYDOWN ||
					event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_RBUTTONDOWN ||
					(event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
					event.customType == kActionEscape))
				return;
		}

		if (!completed && !isPlaying()) {
			completed = true;
			completionTime = timer_read_60();
		}
		// The executable leaves the card up for another 60 native timer
		// ticks after command 8 reports natural completion.
		if (completed && timer_read_60() - completionTime >= 60)
			return;

		if (dirty) {
			prepareNowPlaying(trackTitle);
			dirty = false;
		}
		g_system->delayMillis(10);
	}
}

void BonusTextUI::showGoodbye() {
	restorePresentation();
	_cells.clear(0x20, 0x07);
	_cells.drawText(0, 2, _text.goodbye, 0x07, 0x07, false,
			DOSTextScreen::kColumns);
	present(true);

	const uint32 start = g_system->getMillis();
	while (!g_engine->shouldQuit() &&
			g_system->getMillis() - start < kGoodbyeDuration) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (processQuitEvent(event))
				return;
			processGameMenuEvent(event);
		}
		g_system->delayMillis(10);
	}
}

} // namespace RexNebular
} // namespace MADS
