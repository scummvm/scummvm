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

#include "harvester/startup_menu.h"

#include "common/events.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_flow.h"
#include "harvester/startup_script.h"
#include "harvester/startup_text.h"

namespace Harvester {

namespace {

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kLogoX = 160;
static const int kLogoY = 0;
static const int kMenuStartY = 100;
static const int kMenuLineSpacing = 28;

static const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kTextColorDim = 248;
static const byte kShadowColor = 0;
static const byte kPanelFillColor = 1;
static const byte kTransparentPaletteIndex = 0;

static const CftFontResource *findStartupFontByName(const HarvesterEngine &engine, const char *fontName) {
	const StartupText *startupText = engine.getStartupText();
	if (!startupText || !fontName)
		return nullptr;

	for (const CftFontResource &font : startupText->getFonts()) {
		if (font.name.equalsIgnoreCase(fontName))
			return &font;
	}

	return nullptr;
}

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

static void blitTransparentBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	int srcX = 0;
	int srcY = 0;
	int destX = x;
	int destY = y;
	int width = (int)bitmap.width;
	int height = (int)bitmap.height;

	if (destX < 0) {
		srcX = -destX;
		width += destX;
		destX = 0;
	}
	if (destY < 0) {
		srcY = -destY;
		height += destY;
		destY = 0;
	}
	if (destX >= screen.w || destY >= screen.h || width <= 0 || height <= 0)
		return;

	width = MIN<int>(width, screen.w - destX);
	height = MIN<int>(height, screen.h - destY);
	if (width <= 0 || height <= 0)
		return;

	const byte *src = bitmap.pixels.data() + srcY * bitmap.width + srcX;
	byte *dst = (byte *)screen.getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, screen.pitch, bitmap.width, width, height,
		screen.format.bytesPerPixel, kTransparentPaletteIndex);
}

static void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font, const Common::String &text,
		int x, int y, int width, byte color, Graphics::TextAlign align = Graphics::kTextAlignLeft) {
	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void drawWrappedShadowedText(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, int x, int y, int width, byte color) {
	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);
	for (uint i = 0; i < lines.size(); ++i)
		drawShadowedString(screen, font, lines[i], x, y + (int)i * (font.getFontHeight() + 2), width, color);
}

static int getNativeRoomMenuLineHeight(const Graphics::Font &selectedFont) {
	return selectedFont.getFontHeight() + 2;
}

static int getNativeRoomMenuSelectionFromMouse(const Graphics::Font &selectedFont, uint itemCount,
		const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;

	int selection = mousePos.y - kMenuStartY;
	if (selection < 1)
		selection = 1;

	return (selection - 1) / MAX<int>(1, getNativeRoomMenuLineHeight(selectedFont));
}

} // End of anonymous namespace

StartupMenuSystem::StartupMenuSystem(HarvesterEngine &engine, Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems)
	: _engine(engine), _mousePos(mousePos), _menuItems(menuItems) {
}

Common::Error StartupMenuSystem::runMainMenuStub(StartupFlow &startupFlow) {
	Graphics::FrameLimiter limiter(g_system, 60);
	int selectedItem = _menuItems.empty() ? -1 : 0;
	Common::String statusMessage = "Main menu stub active.";
	bool needsRedraw = true;

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderMainMenuStub(selectedItem, statusMessage);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getMenuItemAt(_mousePos);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (_menuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + _menuItems.size() - 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
						return Common::kNoError;

					if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
						startupFlow.clearPendingMainMenuReturn();
						_engine.getStartupScript()->resetRuntimeState();
						Common::Error roomError = startupFlow.runRoomLoop("START");
						_engine.stopStartupMusic();
						_engine.stopStartupSound();
						if (roomError.getCode() == Common::kReadingFailed) {
							statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
							needsRedraw = true;
						} else if (roomError.getCode() != Common::kNoError) {
							return roomError;
						} else if (startupFlow.takePendingMainMenuReturn()) {
							statusMessage = "Returned to main menu after startup death sequence.";
							needsRedraw = true;
						} else {
							statusMessage = "Resolved START room handoff from HARVEST.SCR.";
							needsRedraw = true;
						}
					} else {
						statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
							_menuItems[selectedItem].c_str());
						needsRedraw = true;
					}
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				if (_menuItems[selectedItem].equalsIgnoreCase("QUIT GAME"))
					return Common::kNoError;

				if (_menuItems[selectedItem].equalsIgnoreCase("NEW GAME")) {
					startupFlow.clearPendingMainMenuReturn();
					_engine.getStartupScript()->resetRuntimeState();
					Common::Error roomError = startupFlow.runRoomLoop("START");
					_engine.stopStartupMusic();
					_engine.stopStartupSound();
					if (roomError.getCode() == Common::kReadingFailed) {
						statusMessage = "Unable to resolve START room setup from HARVEST.SCR.";
						needsRedraw = true;
					} else if (roomError.getCode() != Common::kNoError) {
						return roomError;
					} else if (startupFlow.takePendingMainMenuReturn()) {
						statusMessage = "Returned to main menu after startup death sequence.";
						needsRedraw = true;
					} else {
						statusMessage = "Resolved START room handoff from HARVEST.SCR.";
						needsRedraw = true;
					}
				} else {
					statusMessage = Common::String::format("%s is stubbed; room setup and gameplay loop are next.",
						_menuItems[selectedItem].c_str());
					needsRedraw = true;
				}
				break;
			default:
				break;
			}
		}

		if (startupFlow.tickRuntimeEntities())
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupMenuSystem::runRoomMenuStub(const IndexedBitmap &backdrop, StartupFlow &startupFlow) {
	Graphics::FrameLimiter limiter(g_system, 60);
	int selectedItem = _menuItems.empty() ? -1 : 0;
	bool needsRedraw = true;
	startupFlow.resetCursorAnimationSequence();

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderRoomMenuStub(backdrop, selectedItem);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getRoomMenuItemAt(_mousePos);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (_menuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + _menuItems.size() - 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % _menuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					debug(1, "Harvester: room menu item '%s' selected but not implemented",
						selectedItem >= 0 ? _menuItems[selectedItem].c_str() : "");
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getRoomMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				debug(1, "Harvester: room menu item '%s' clicked but not implemented",
					_menuItems[selectedItem].c_str());
				break;
			default:
				break;
			}
		}

		RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities();
		if (runtimeEntities && runtimeEntities->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void StartupMenuSystem::renderMainMenuStub(int selectedItem, const Common::String &statusMessage) const {
	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const Graphics::Font *titleFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!screen || !art || !titleFont || !bodyFont)
		return;

	byte displayPalette[256 * 3];
	buildHarvesterDisplayPalette(art->getWaitPalette(), 1.0f, displayPalette);
	screen->setPalette(displayPalette);
	screen->fillRect(screen->getBounds(), 0);
	blitBitmap(*screen, art->getInventoryBitmap(), kInventoryX, kInventoryY);
	blitBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const Common::Rect panel(96, 96, 544, 432);
	screen->fillRect(panel, kPanelFillColor);

	drawShadowedString(*screen, *titleFont, "Startup Main Menu Stub", panel.left, 110, panel.width(),
		kTextColorNormal, Graphics::kTextAlignCenter);

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int itemY = kMenuStartY + (int)i * kMenuLineSpacing;
		const byte color = ((int)i == selectedItem) ? kTextColorHover : kTextColorNormal;
		drawShadowedString(*screen, *bodyFont, _menuItems[i], panel.left, itemY, panel.width(), color,
			Graphics::kTextAlignCenter);
	}

	drawShadowedString(*screen, *bodyFont, "This is the next startup handoff after quick tips.", panel.left, 330,
		panel.width(), kTextColorDim, Graphics::kTextAlignCenter);
	drawWrappedShadowedText(*screen, *bodyFont, statusMessage, panel.left + 24, 360, panel.width() - 48,
		kTextColorNormal);
	drawShadowedString(*screen, *bodyFont, "Use mouse or arrow keys. Enter activates. Esc returns to launcher.",
		panel.left, 404, panel.width(), kTextColorDim, Graphics::kTextAlignCenter);

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void StartupMenuSystem::renderRoomMenuStub(const IndexedBitmap &backdrop, int selectedItem) const {
	Graphics::Screen *screen = _engine.getScreen();
	const StartupArt *art = _engine.getStartupArt();
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	const CftFontResource *unselectedFontResource = findStartupFontByName(_engine, "HARVFNT2");
	if (!screen || !art || !selectedFontResource || !unselectedFontResource || !backdrop.isValid())
		return;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return;

	blitBitmap(*screen, backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const int lineSpacing = getNativeRoomMenuLineHeight(selectedFont);
	for (uint i = 0; i < _menuItems.size(); ++i) {
		const Graphics::Font *font = ((int)i == selectedItem)
			? static_cast<const Graphics::Font *>(&selectedFont)
			: static_cast<const Graphics::Font *>(&unselectedFont);
		const int width = font->getStringWidth(_menuItems[i]);
		const int x = (screen->w - width) / 2;
		const int y = kMenuStartY + (int)i * lineSpacing;
		font->drawString(screen, _menuItems[i], x, y, width, 0);
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

int StartupMenuSystem::getMenuItemAt(const Common::Point &mousePos) const {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return -1;

	for (uint i = 0; i < _menuItems.size(); ++i) {
		const int width = font->getStringWidth(_menuItems[i]);
		const int x = (640 - width) / 2;
		const int y = kMenuStartY + (int)i * kMenuLineSpacing;
		const Common::Rect bounds(x - 12, y - 2, x + width + 12, y + font->getFontHeight() + 2);
		if (bounds.contains(mousePos))
			return (int)i;
	}

	return -1;
}

int StartupMenuSystem::getRoomMenuItemAt(const Common::Point &mousePos) const {
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	if (!selectedFontResource)
		return getMenuItemAt(mousePos);

	HarvesterCftFont selectedFont(*selectedFontResource);
	if (!selectedFont.isValid())
		return getMenuItemAt(mousePos);

	return getNativeRoomMenuSelectionFromMouse(selectedFont, _menuItems.size(), mousePos);
}

} // End of namespace Harvester
