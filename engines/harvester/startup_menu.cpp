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

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "engines/metaengine.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
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
static const byte kQuickTipActionColor = 0xc3;

static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const char *const kOptionsVolumeBitmapPath = "1:/GRAPHIC/OTHER/VOLUME.BM";
static const char *const kOptionsIndicatorBitmapPath = "1:/GRAPHIC/OTHER/INDICATR.BM";
static const char *const kOptionsPreviewSoundPath = "1:/SOUND/EFFECTS/WHIP2.WAV";
static const char *const kSaveGameBitmapPath = "1:/GRAPHIC/OTHER/SAVEGAME.BM";
static const char *const kSaveGamePalettePath = "1:/GRAPHIC/PAL/SAVEGAME.PAL";

static const int kOptionsSliderMinX = 0x16f;
static const int kOptionsSliderMaxX = 0x22b;
static const int kOptionsSliderStep = 0x13;
static const int kOptionsVolumeBitmapX = 0x168;
static const int kOptionsVolumeBitmapY = 0x68;
static const int kOptionsIndicatorY = 0x6c;
static const int kOptionsSliderHeight = 0x1e;
static const int kOptionsItemCount = 7;
static const int kStartupOptionMaxLevel = 9;

static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;
static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kQuickTipTextWidth = 280;
static const int kSaveSlotCount = 25;
static const int kSaveSlotLabelX = 8;
static const int kSaveSlotNameX = 0x50;
static const int kSaveSlotStartY = 0x19;
static const int kSaveSlotStride = 0x11;
static const int kSaveSlotTextWidth = 0x27b - kSaveSlotNameX;
static const int kSaveStatusTextY = 0x1b8;
static const int kSaveSlotMaxCharacters = 63;

struct RoomMenuTextConfig {
	Common::Array<Common::String> optionItems;
	Common::String yesLabel = "YES";
	Common::String noLabel = "NO";
	Common::String clickLabel = "CLICK";
};

static void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y);
static int getNativeRoomMenuLineHeight(const Graphics::Font &selectedFont);

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

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kStartupOptionMaxLevel)
		return kStartupOptionMaxLevel;

	return level;
}

static Common::Rect quickTipsExitRect() {
	return Common::Rect(180, 280, 238, 291);
}

static Common::Rect quickTipsNextRect() {
	return Common::Rect(420, 280, 492, 291);
}

static Common::Rect quickTipsToggleRect() {
	return Common::Rect(258, 280, 366, 291);
}

static Common::Rect textEntryInputRect() {
	return Common::Rect(176, 212, 464, 234);
}

static Common::Rect textEntryOkRect() {
	return Common::Rect(224, 278, 286, 292);
}

static Common::Rect textEntryCancelRect() {
	return Common::Rect(344, 278, 424, 292);
}

static Common::Rect saveSlotRect(int slotIndex) {
	const int top = kSaveSlotStartY + slotIndex * kSaveSlotStride;
	return Common::Rect(8, top, 0x27b + 1, top + kSaveSlotStride);
}

static Common::Rect saveConfirmRect() {
	return Common::Rect(3, 0x1cc, 0x13d + 1, 0x1dc + 1);
}

static Common::Rect saveCancelRect() {
	return Common::Rect(0x141, 0x1cc, 0x27b + 1, 0x1dc + 1);
}

static Common::Rect optionsSliderRect(int sliderIndex, int lineHeight) {
	const int y = kOptionsIndicatorY + sliderIndex * lineHeight;
	return Common::Rect(kOptionsSliderMinX, y, kOptionsSliderMaxX + 1, y + kOptionsSliderHeight + 1);
}

static int resolveOptionsSliderIndexFromMouseX(int mouseX) {
	const int relativeX = mouseX - kOptionsSliderMinX;
	return clampStartupOptionLevel((relativeX + (kOptionsSliderStep / 2)) / kOptionsSliderStep);
}

static bool loadMenuTextConfig(HarvesterEngine &engine, RoomMenuTextConfig &config) {
	config = RoomMenuTextConfig();
	config.optionItems.resize(kOptionsItemCount);
	config.optionItems[0] = "SOUND FX";
	config.optionItems[1] = "MUSIC";
	config.optionItems[2] = "GAMMA";
	config.optionItems[3] = "TEXT";
	config.optionItems[4] = "GORE";
	config.optionItems[5] = "QUICK TIPS";
	config.optionItems[6] = "PASSWORD";

	ResourceManager *resources = engine.getResources();
	if (!resources)
		return false;

	Common::Array<byte> data;
	if (!resources->loadFile(kMenuPath, data))
		return false;

	Common::MemoryReadStream stream(data.data(), data.size());
	Common::INIFile menu;
	menu.setDefaultSectionName(kMenuSectionName);
	menu.requireKeyValueDelimiter();
	menu.suppressValuelessLineWarning();
	if (!menu.loadFromStream(stream))
		return false;

	for (uint i = 0; i < config.optionItems.size(); ++i) {
		Common::String value;
		if (menu.getKey(Common::String::format("options_menu_%u", i + 1), kMenuSectionName, value) && !value.empty())
			config.optionItems[i] = value;
	}

	Common::String value;
	if (menu.getKey("yes", kMenuSectionName, value) && !value.empty())
		config.yesLabel = value;
	if (menu.getKey("no", kMenuSectionName, value) && !value.empty())
		config.noLabel = value;
	if (menu.getKey("click", kMenuSectionName, value) && !value.empty())
		config.clickLabel = value;

	return true;
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap.width = READ_LE_UINT32(data.data());
	bitmap.height = READ_LE_UINT32(data.data() + 4);
	const uint32 pixelCount = bitmap.width * bitmap.height;
	if (bitmap.width == 0 || bitmap.height == 0 || data.size() < 12 + pixelCount)
		return false;

	bitmap.pixels.resize(pixelCount);
	memcpy(bitmap.pixels.data(), data.data() + 12, pixelCount);
	return true;
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

static void applyMenuPalette(Graphics::Screen &screen, const HarvesterEngine &engine,
		const byte *palette, float brightness) {
	if (!palette)
		return;

	byte displayPalette[256 * 3];
	buildHarvesterDisplayPalette(palette, brightness * engine.getStartupGammaBrightnessScale(), displayPalette);
	screen.setPalette(displayPalette);
}

static void renderHelpScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap, const byte *palette) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	applyMenuPalette(*screen, engine, palette, 1.0f);
	screen->fillRect(screen->getBounds(), 0);
	blitBitmap(*screen, bitmap, 0, 0);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

static Common::String buildTextModeSuffix(const StartupScript &startupScript, const RoomMenuTextConfig &config) {
	switch (startupScript.getDialogueTextMode()) {
	case kStartupDialogueTextNone:
		return Common::String::format(" - %s", config.noLabel.c_str());
	case kStartupDialogueTextClick:
		return Common::String::format(" - %s", config.clickLabel.c_str());
	case kStartupDialogueTextYes:
	default:
		return Common::String::format(" - %s", config.yesLabel.c_str());
	}
}

static Common::String buildOptionsMenuItemLabel(const StartupScript &startupScript,
		const RoomMenuTextConfig &config, int index) {
	if (index < 0 || index >= (int)config.optionItems.size())
		return Common::String();

	switch (index) {
	case 3:
		return config.optionItems[index] + buildTextModeSuffix(startupScript, config);
	case 4:
		return config.optionItems[index] + (startupScript.isGoreEnabled() ? " - On" : " - Off");
	case 6:
		return config.optionItems[index] + (startupScript.getParentalPassword().empty() ? " - Off" : " - On");
	default:
		return config.optionItems[index];
	}
}

static bool appendTextEntryCharacter(Common::String &text, const Graphics::Font &font,
		uint32 ascii, uint maxCharacters) {
	if (ascii < 32 || ascii > 126 || ascii == '~' || text.size() >= maxCharacters)
		return false;

	const Common::String candidate = text + (char)ascii;
	if (font.getStringWidth(candidate) > textEntryInputRect().width() - 10)
		return false;

	text = candidate;
	return true;
}

static bool appendInlineSaveCharacter(Common::String &text, const Graphics::Font &font, uint32 ascii) {
	if (ascii < 32 || ascii > 126 || ascii == '~' || text.size() >= kSaveSlotMaxCharacters)
		return false;

	const Common::String candidate = text + (char)ascii;
	if (font.getStringWidth(candidate) > kSaveSlotTextWidth)
		return false;

	text = candidate;
	return true;
}

static Common::String buildDefaultSaveSlotLabel(int slotIndex) {
	return Common::String::format("GAME %02d", slotIndex + 1);
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

static void renderOptionsMenuScreen(HarvesterEngine &engine, const IndexedBitmap &backdrop,
		const byte *palette, float paletteBrightness,
		const Graphics::Font &selectedFont, const Graphics::Font &unselectedFont,
		const StartupArt &art, const RoomMenuTextConfig &config,
		const IndexedBitmap &volumeBar, const IndexedBitmap &indicator, int selectedItem) {
	Graphics::Screen *screen = engine.getScreen();
	StartupScript *startupScript = engine.getStartupScript();
	if (!screen || !startupScript)
		return;

	applyMenuPalette(*screen, engine, palette, paletteBrightness);
	blitBitmap(*screen, backdrop, 0, 0);
	blitTransparentBitmap(*screen, art.getLogoBitmap(), kLogoX, kLogoY);

	const int lineHeight = getNativeRoomMenuLineHeight(selectedFont);
	for (int i = 0; i < 3; ++i) {
		blitTransparentBitmap(*screen, volumeBar, kOptionsVolumeBitmapX, kOptionsVolumeBitmapY + i * lineHeight);
	}

	const int levels[3] = {
		startupScript->getFxVolumeLevel(),
		startupScript->getMusicVolumeLevel(),
		startupScript->getGammaLevel()
	};
	for (int i = 0; i < 3; ++i) {
		blitTransparentBitmap(*screen, indicator, kOptionsSliderMinX + levels[i] * kOptionsSliderStep,
			kOptionsIndicatorY + i * lineHeight);
	}

	for (int i = 0; i < (int)config.optionItems.size(); ++i) {
		const Common::String label = buildOptionsMenuItemLabel(*startupScript, config, i);
		const Graphics::Font &font = (i == selectedItem) ? selectedFont : unselectedFont;
		const int width = font.getStringWidth(label);
		const int x = (screen->w - width) / 2;
		const int y = kMenuStartY + i * lineHeight;
		font.drawString(screen, label, x, y, width, 0);
	}

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

static void renderQuickTipsOverlay(HarvesterEngine &engine, const IndexedBitmap &backdrop,
		const byte *palette, float paletteBrightness,
		const Common::String &tipText) {
	Graphics::Screen *screen = engine.getScreen();
	const StartupArt *art = engine.getStartupArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	StartupScript *startupScript = engine.getStartupScript();
	if (!screen || !art || !font || !startupScript)
		return;

	applyMenuPalette(*screen, engine, palette, paletteBrightness);
	blitBitmap(*screen, backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);
	blitBitmap(*screen, art->getTipsBitmap(), kQuickTipsOverlayX, kQuickTipsOverlayY);

	drawWrappedShadowedText(*screen, *font, tipText, kQuickTipTextX, kQuickTipTextY, kQuickTipTextWidth,
		kTextColorNormal);
	const Common::String toggleLabel = startupScript->resolveTextValue(
		startupScript->isQuickTipsEnabled() ? "Show_Tips_ON" : "Show_Tips_OFF");
	drawShadowedString(*screen, *font, "Exit", quickTipsExitRect().left, quickTipsExitRect().top,
		quickTipsExitRect().width(), kQuickTipActionColor);
	drawShadowedString(*screen, *font, "Next", quickTipsNextRect().left, quickTipsNextRect().top,
		quickTipsNextRect().width(), kQuickTipActionColor);
	drawShadowedString(*screen, *font, toggleLabel, quickTipsToggleRect().left, quickTipsToggleRect().top,
		quickTipsToggleRect().width(), kQuickTipActionColor);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

static void renderTextEntryDialog(HarvesterEngine &engine, const IndexedBitmap &backdrop,
		const byte *palette, float paletteBrightness, const Graphics::Font &titleFont,
		const Graphics::Font &entryFont, const Common::String &title, const Common::String &text,
		bool cursorVisible, bool okHover, bool cancelHover) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	applyMenuPalette(*screen, engine, palette, paletteBrightness);
	blitBitmap(*screen, backdrop, 0, 0);

	screen->fillRect(Common::Rect(132, 148, 508, 320), kPanelFillColor);
	drawShadowedString(*screen, titleFont, title, 132, 168, 376, kTextColorNormal,
		Graphics::kTextAlignCenter);
	screen->fillRect(textEntryInputRect(), kTransparentPaletteIndex);
	screen->frameRect(textEntryInputRect(), kTextColorDim);

	Common::String displayText = text;
	if (cursorVisible)
		displayText += "_";
	const Common::Rect inputRect = textEntryInputRect();
	const Common::Rect okRect = textEntryOkRect();
	const Common::Rect cancelRect = textEntryCancelRect();
	drawShadowedString(*screen, entryFont, displayText, inputRect.left + 5, inputRect.top + 4,
		inputRect.width() - 10, kTextColorNormal);
	drawShadowedString(*screen, entryFont, "OK", okRect.left, okRect.top,
		okRect.width(), okHover ? kTextColorHover : kTextColorNormal, Graphics::kTextAlignCenter);
	drawShadowedString(*screen, entryFont, "Cancel", cancelRect.left, cancelRect.top,
		cancelRect.width(), cancelHover ? kTextColorHover : kTextColorNormal,
		Graphics::kTextAlignCenter);

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

static void renderSaveGameMenuScreen(HarvesterEngine &engine, const IndexedBitmap &background,
		const byte *palette, float paletteBrightness, const Graphics::Font &selectedFont,
		const Graphics::Font &unselectedFont, const Common::Array<Common::String> &slotTitles,
		int activeSlot, bool hoverSave, bool hoverCancel, const Common::String &statusMessage,
		int editingSlot = -1, const Common::String *editingText = nullptr) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	applyMenuPalette(*screen, engine, palette, paletteBrightness);
	blitBitmap(*screen, background, 0, 0);

	for (int i = 0; i < kSaveSlotCount; ++i) {
		const bool selected = i == activeSlot;
		const Graphics::Font &labelFont = selected ? selectedFont : unselectedFont;
		const Graphics::Font &nameFont = (i == editingSlot || selected) ? selectedFont : unselectedFont;
		const int y = kSaveSlotStartY + i * kSaveSlotStride;
		const Common::String slotLabel = buildDefaultSaveSlotLabel(i);
		labelFont.drawString(screen, slotLabel, kSaveSlotLabelX, y, 64, 0);

		Common::String displayTitle = slotTitles[i];
		if (i == editingSlot && editingText)
			displayTitle = *editingText + "_";
		nameFont.drawString(screen, displayTitle, kSaveSlotNameX, y, kSaveSlotTextWidth, 0);
	}

	drawShadowedString(*screen, selectedFont, "SAVE", saveConfirmRect().left, saveConfirmRect().top,
		saveConfirmRect().width(), hoverSave ? kTextColorHover : kTextColorNormal,
		Graphics::kTextAlignCenter);
	drawShadowedString(*screen, selectedFont, "CANCEL", saveCancelRect().left, saveCancelRect().top,
		saveCancelRect().width(), hoverCancel ? kTextColorHover : kTextColorNormal,
		Graphics::kTextAlignCenter);
	if (!statusMessage.empty()) {
		drawShadowedString(*screen, unselectedFont, statusMessage, 20, kSaveStatusTextY,
			screen->w - 40, kTextColorNormal, Graphics::kTextAlignCenter);
	}

	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
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
		return -1;

	selection = (selection - 1) / MAX<int>(1, getNativeRoomMenuLineHeight(selectedFont));
	if (selection < 0 || selection >= (int)itemCount)
		return -1;

	return selection;
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

Common::Error StartupMenuSystem::runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, StartupFlow &startupFlow) {
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
					if (selectedItem >= 0 && _menuItems[selectedItem].equalsIgnoreCase("OPTIONS")) {
						Common::Error optionsError = runOptionsMenu(backdrop, palette, paletteBrightness, startupFlow);
						if (optionsError.getCode() != Common::kNoError)
							return optionsError;
						needsRedraw = true;
					} else if (selectedItem >= 0 && _menuItems[selectedItem].equalsIgnoreCase("HELP")) {
						Common::Error helpError = runHelpScreen(palette, paletteBrightness, startupFlow);
						if (helpError.getCode() != Common::kNoError)
							return helpError;
						needsRedraw = true;
					} else if (selectedItem >= 0 && _menuItems[selectedItem].equalsIgnoreCase("SAVE GAME")) {
						Common::Error saveError = runSaveGameMenu(palette, paletteBrightness, startupFlow);
						if (saveError.getCode() != Common::kNoError)
							return saveError;
						needsRedraw = true;
					} else {
						debug(1, "Harvester: room menu item '%s' selected but not implemented",
							selectedItem >= 0 ? _menuItems[selectedItem].c_str() : "");
					}
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (_menuItems.empty())
					break;

				selectedItem = getRoomMenuItemAt(_mousePos);
				if (selectedItem == -1)
					break;

				if (_menuItems[selectedItem].equalsIgnoreCase("OPTIONS")) {
					Common::Error optionsError = runOptionsMenu(backdrop, palette, paletteBrightness, startupFlow);
					if (optionsError.getCode() != Common::kNoError)
						return optionsError;
					needsRedraw = true;
				} else if (_menuItems[selectedItem].equalsIgnoreCase("HELP")) {
					Common::Error helpError = runHelpScreen(palette, paletteBrightness, startupFlow);
					if (helpError.getCode() != Common::kNoError)
						return helpError;
					needsRedraw = true;
				} else if (_menuItems[selectedItem].equalsIgnoreCase("SAVE GAME")) {
					Common::Error saveError = runSaveGameMenu(palette, paletteBrightness, startupFlow);
					if (saveError.getCode() != Common::kNoError)
						return saveError;
					needsRedraw = true;
				} else {
					debug(1, "Harvester: room menu item '%s' clicked but not implemented",
						_menuItems[selectedItem].c_str());
				}
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

Common::Error StartupMenuSystem::runSaveGameMenu(const byte *palette, float paletteBrightness,
		StartupFlow &startupFlow) {
	(void)palette;
	(void)paletteBrightness;
	ResourceManager *resources = _engine.getResources();
	MetaEngine *metaEngine = _engine.getMetaEngine();
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	const CftFontResource *unselectedFontResource = findStartupFontByName(_engine, "HARVFNT2");
	if (!resources || !metaEngine || !selectedFontResource || !unselectedFontResource)
		return Common::kReadingFailed;
	if (!_engine.canSaveGameStateCurrently())
		return Common::kNoError;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return Common::kReadingFailed;

	IndexedBitmap background;
	byte savePalette[256 * 3];
	if (!loadBitmapResource(*resources, kSaveGameBitmapPath, background) ||
			!loadPaletteResource(*resources, kSaveGamePalettePath, savePalette)) {
		return Common::kReadingFailed;
	}

	Common::Array<Common::String> slotTitles;
	slotTitles.resize(kSaveSlotCount);
	auto reloadSlotTitles = [&]() {
		for (int i = 0; i < kSaveSlotCount; ++i)
			slotTitles[i].clear();

		const SaveStateList saves = metaEngine->listSaves(ConfMan.getActiveDomainName().c_str());
		for (const SaveStateDescriptor &save : saves) {
			const int slot = save.getSaveSlot();
			if (slot < 0 || slot >= kSaveSlotCount)
				continue;
			slotTitles[slot] = save.getDescription().encode();
		}
	};
	reloadSlotTitles();

	auto resolveHoveredSlot = [&]() -> int {
		for (int slot = 0; slot < kSaveSlotCount; ++slot) {
			if (saveSlotRect(slot).contains(_mousePos))
				return slot;
		}
		return -1;
	};
	auto runInlineSlotEditor = [&](int slotIndex, Common::String &editedTitle,
			bool &confirmed) -> Common::Error {
		confirmed = false;
		bool needsRedraw = true;
		startupFlow.resetCursorAnimationSequence();
		Graphics::FrameLimiter entryLimiter(g_system, 60);

		while (!_engine.shouldQuit()) {
			const bool hoverSave = saveConfirmRect().contains(_mousePos);
			const bool hoverCancel = saveCancelRect().contains(_mousePos);
			if (needsRedraw) {
				renderSaveGameMenuScreen(_engine, background, savePalette, 1.0f,
					selectedFont, unselectedFont, slotTitles, slotIndex, hoverSave, hoverCancel,
					Common::String(), slotIndex, &editedTitle);
				needsRedraw = false;
			}

			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (startupFlow.handleSystemEvent(event, result))
					return result;

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					needsRedraw = true;
					break;
				case Common::EVENT_LBUTTONDOWN:
					if (saveConfirmRect().contains(_mousePos)) {
						confirmed = true;
						return Common::kNoError;
					}
					if (saveCancelRect().contains(_mousePos))
						return Common::kNoError;
					break;
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
						return Common::kNoError;
					if (event.kbd.keycode == Common::KEYCODE_RETURN ||
							event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
						confirmed = true;
						return Common::kNoError;
					}
					if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
						if (!editedTitle.empty()) {
							editedTitle.deleteLastChar();
							needsRedraw = true;
						}
						break;
					}
					if (appendInlineSaveCharacter(editedTitle, selectedFont, event.kbd.ascii))
						needsRedraw = true;
					break;
				default:
					break;
				}
			}

			if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
				(void)runtimeEntities->syncCursorEntityPosition(_mousePos);

			entryLimiter.delayBeforeSwap();
			entryLimiter.startFrame();
		}

		return Common::kNoError;
	};

	int activeSlot = 0;
	Common::String statusMessage;
	bool needsRedraw = true;
	startupFlow.resetCursorAnimationSequence();
	Graphics::FrameLimiter limiter(g_system, 60);

	auto activateSelectedSlot = [&](bool &saved) -> Common::Error {
		saved = false;
		Common::String editedTitle = slotTitles[activeSlot];
		bool confirmed = false;
		Common::Error entryError = runInlineSlotEditor(activeSlot, editedTitle, confirmed);
		if (entryError.getCode() != Common::kNoError)
			return entryError;
		if (!confirmed)
			return Common::kNoError;

		Common::Error saveError = _engine.saveGameState(activeSlot, editedTitle);
		if (saveError.getCode() == Common::kNoError) {
			saved = true;
			return Common::kNoError;
		}

		statusMessage = saveError.getDesc();
		reloadSlotTitles();
		needsRedraw = true;
		return Common::kNoError;
	};

	while (!_engine.shouldQuit()) {
		const bool hoverSave = saveConfirmRect().contains(_mousePos);
		const bool hoverCancel = saveCancelRect().contains(_mousePos);
		if (needsRedraw) {
			renderSaveGameMenuScreen(_engine, background, savePalette, 1.0f,
				selectedFont, unselectedFont, slotTitles, activeSlot, hoverSave, hoverCancel, statusMessage);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredSlot = resolveHoveredSlot();
				if (hoveredSlot != -1 && hoveredSlot != activeSlot)
					activeSlot = hoveredSlot;
				needsRedraw = true;
				break;
			}
			case Common::EVENT_LBUTTONDOWN: {
				const int hoveredSlot = resolveHoveredSlot();
				bool saved = false;
				statusMessage.clear();
				if (hoveredSlot != -1) {
					activeSlot = hoveredSlot;
					Common::Error saveError = activateSelectedSlot(saved);
					if (saveError.getCode() != Common::kNoError)
						return saveError;
					if (saved)
						return Common::kNoError;
					break;
				}
				if (saveConfirmRect().contains(_mousePos)) {
					Common::Error saveError = activateSelectedSlot(saved);
					if (saveError.getCode() != Common::kNoError)
						return saveError;
					if (saved)
						return Common::kNoError;
				} else if (saveCancelRect().contains(_mousePos)) {
					return Common::kNoError;
				}
				needsRedraw = true;
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;
				if (event.kbd.keycode == Common::KEYCODE_UP) {
					activeSlot = (activeSlot + kSaveSlotCount - 1) % kSaveSlotCount;
					statusMessage.clear();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					activeSlot = (activeSlot + 1) % kSaveSlotCount;
					statusMessage.clear();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					bool saved = false;
					statusMessage.clear();
					Common::Error saveError = activateSelectedSlot(saved);
					if (saveError.getCode() != Common::kNoError)
						return saveError;
					if (saved)
						return Common::kNoError;
					needsRedraw = true;
				}
				break;
			default:
				break;
			}
		}

		if (RuntimeEntityManager *runtimeEntities = _engine.getRuntimeEntities())
			(void)runtimeEntities->syncCursorEntityPosition(_mousePos);

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error StartupMenuSystem::runOptionsMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, StartupFlow &startupFlow) {
	const StartupArt *art = _engine.getStartupArt();
	StartupScript *startupScript = _engine.getStartupScript();
	ResourceManager *resources = _engine.getResources();
	const CftFontResource *selectedFontResource = findStartupFontByName(_engine, "HARVFONT");
	const CftFontResource *unselectedFontResource = findStartupFontByName(_engine, "HARVFNT2");
	if (!art || !startupScript || !resources || !selectedFontResource || !unselectedFontResource)
		return Common::kReadingFailed;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return Common::kReadingFailed;

	RoomMenuTextConfig config;
	(void)loadMenuTextConfig(_engine, config);

	IndexedBitmap volumeBar;
	IndexedBitmap indicator;
	if (!loadBitmapResource(*resources, kOptionsVolumeBitmapPath, volumeBar) ||
			!loadBitmapResource(*resources, kOptionsIndicatorBitmapPath, indicator)) {
		return Common::kReadingFailed;
	}

	const int lineHeight = getNativeRoomMenuLineHeight(selectedFont);
	const Graphics::Font *bodyFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	const Graphics::Font &entryFont = bodyFont ? *bodyFont : static_cast<const Graphics::Font &>(unselectedFont);
	int selectedItem = 0;
	int capturedSlider = -1;
	bool showingQuickTips = false;
	bool needsRedraw = true;
	uint quickTipIndex = startupFlow._quickTips.empty() ? 0 : _engine.getRandomNumber(startupFlow._quickTips.size() - 1);

	auto persistConfig = [&]() {
		(void)startupScript->saveConfig();
	};

	auto updateSlider = [&](int sliderIndex) {
		const int newLevel = resolveOptionsSliderIndexFromMouseX(_mousePos.x);
		switch (sliderIndex) {
		case 0:
			if (newLevel != startupScript->getFxVolumeLevel()) {
				_engine.setStartupFxVolumeLevel(newLevel);
				_engine.playStartupSingleSound(kOptionsPreviewSoundPath);
				persistConfig();
				needsRedraw = true;
			}
			break;
		case 1:
			if (newLevel != startupScript->getMusicVolumeLevel()) {
				_engine.setStartupMusicVolumeLevel(newLevel);
				persistConfig();
				needsRedraw = true;
			}
			break;
		case 2:
			if (newLevel != startupScript->getGammaLevel()) {
				_engine.setStartupGammaLevel(newLevel);
				persistConfig();
				needsRedraw = true;
			}
			break;
		default:
			break;
		}
	};

	auto cycleTextMode = [&]() {
		switch (startupScript->getDialogueTextMode()) {
		case kStartupDialogueTextNone:
			startupScript->setDialogueTextMode(kStartupDialogueTextYes);
			break;
		case kStartupDialogueTextYes:
			startupScript->setDialogueTextMode(kStartupDialogueTextClick);
			break;
		case kStartupDialogueTextClick:
		default:
			startupScript->setDialogueTextMode(kStartupDialogueTextNone);
			break;
		}
		persistConfig();
		needsRedraw = true;
	};

	auto toggleGore = [&]() {
		startupScript->setGoreEnabled(!startupScript->isGoreEnabled());
		ConfMan.setBool("gore", startupScript->isGoreEnabled());
		persistConfig();
		needsRedraw = true;
	};

	auto togglePassword = [&]() {
		if (startupScript->getParentalPassword().empty()) {
			const Common::String password = runModalTextEntryDialog(backdrop, palette, paletteBrightness,
				selectedFont, entryFont, "ENTER PASSWORD", Common::String(), startupFlow);
			if (!password.empty()) {
				startupScript->setParentalPassword(password);
				persistConfig();
				needsRedraw = true;
			}
		} else {
			startupScript->setParentalPassword(Common::String());
			persistConfig();
			needsRedraw = true;
		}
	};

	auto activateSelectedItem = [&]() -> Common::Error {
		switch (selectedItem) {
		case 3:
			cycleTextMode();
			break;
		case 4:
			toggleGore();
			break;
		case 5:
			if (!startupFlow._quickTips.empty()) {
				showingQuickTips = true;
				needsRedraw = true;
			}
			break;
		case 6:
			togglePassword();
			break;
		default:
			break;
		}

		return Common::kNoError;
	};

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			if (showingQuickTips) {
				renderQuickTipsOverlay(_engine, backdrop, palette, paletteBrightness,
					startupFlow._quickTips[quickTipIndex]);
			} else {
				renderOptionsMenuScreen(_engine, backdrop, palette, paletteBrightness,
					selectedFont, unselectedFont, *art, config, volumeBar, indicator, selectedItem);
			}
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			if (showingQuickTips) {
				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					needsRedraw = true;
					break;
				case Common::EVENT_LBUTTONDOWN:
					if (quickTipsExitRect().contains(_mousePos)) {
						showingQuickTips = false;
						needsRedraw = true;
					} else if (quickTipsNextRect().contains(_mousePos)) {
						quickTipIndex = (quickTipIndex + 1) % startupFlow._quickTips.size();
						needsRedraw = true;
					} else if (quickTipsToggleRect().contains(_mousePos)) {
						startupScript->setQuickTipsEnabled(!startupScript->isQuickTipsEnabled());
						persistConfig();
						needsRedraw = true;
					}
					break;
				case Common::EVENT_RBUTTONDOWN:
					showingQuickTips = false;
					needsRedraw = true;
					break;
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						showingQuickTips = false;
						needsRedraw = true;
					}
					break;
				default:
					break;
				}
				continue;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				if (capturedSlider != -1) {
					selectedItem = capturedSlider;
					updateSlider(capturedSlider);
					needsRedraw = true;
				} else {
					const int hoveredItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
						config.optionItems.size(), _mousePos);
					if (hoveredItem != -1 && hoveredItem != selectedItem) {
						selectedItem = hoveredItem;
						needsRedraw = true;
					}
				}
				break;
			}
			case Common::EVENT_LBUTTONDOWN: {
				bool handled = false;
				for (int sliderIndex = 0; sliderIndex < 3; ++sliderIndex) {
					if (optionsSliderRect(sliderIndex, lineHeight).contains(_mousePos)) {
						selectedItem = sliderIndex;
						capturedSlider = sliderIndex;
						updateSlider(sliderIndex);
						needsRedraw = true;
						handled = true;
						break;
					}
				}
				if (handled)
					break;

				const int clickedItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
					config.optionItems.size(), _mousePos);
				if (clickedItem == -1)
					break;

				selectedItem = clickedItem;
				needsRedraw = true;
				if (selectedItem >= 3) {
					Common::Error activateError = activateSelectedItem();
					if (activateError.getCode() != Common::kNoError)
						return activateError;
				}
				break;
			}
			case Common::EVENT_LBUTTONUP:
				capturedSlider = -1;
				break;
			case Common::EVENT_RBUTTONDOWN:
				return Common::kNoError;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					return Common::kNoError;
				case Common::KEYCODE_UP:
					selectedItem = (selectedItem + (int)config.optionItems.size() - 1) % (int)config.optionItems.size();
					needsRedraw = true;
					break;
				case Common::KEYCODE_DOWN:
					selectedItem = (selectedItem + 1) % (int)config.optionItems.size();
					needsRedraw = true;
					break;
				case Common::KEYCODE_LEFT:
					if (selectedItem < 3) {
						_mousePos.x = MAX<int>(kOptionsSliderMinX, _mousePos.x - kOptionsSliderStep);
						updateSlider(selectedItem);
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_RIGHT:
					if (selectedItem < 3) {
						_mousePos.x = MIN<int>(kOptionsSliderMaxX, _mousePos.x + kOptionsSliderStep);
						updateSlider(selectedItem);
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER: {
					Common::Error activateError = activateSelectedItem();
					if (activateError.getCode() != Common::kNoError)
						return activateError;
					break;
				}
				case Common::KEYCODE_t:
					selectedItem = 3;
					cycleTextMode();
					break;
				case Common::KEYCODE_g:
					selectedItem = 4;
					toggleGore();
					break;
				case Common::KEYCODE_q:
					selectedItem = 5;
					if (!startupFlow._quickTips.empty()) {
						showingQuickTips = true;
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_p:
					selectedItem = 6;
					togglePassword();
					break;
				default:
					break;
				}
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

Common::Error StartupMenuSystem::runHelpScreen(const byte *palette, float paletteBrightness, StartupFlow &startupFlow) {
	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;

	IndexedBitmap mouseHelp;
	IndexedBitmap keysHelp;
	byte mouseHelpPalette[256 * 3];
	byte keysHelpPalette[256 * 3];
	if (!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/MOUSHELP.BM", mouseHelp) ||
			!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/KEYSHELP.BM", keysHelp) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/MOUSHELP.PAL", mouseHelpPalette) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/KEYSHELP.PAL", keysHelpPalette)) {
		return Common::kReadingFailed;
	}

	int page = 0;
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderHelpScreen(_engine, page == 0 ? mouseHelp : keysHelp,
				page == 0 ? mouseHelpPalette : keysHelpPalette);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (Graphics::Screen *screen = _engine.getScreen()) {
					applyMenuPalette(*screen, _engine, palette, paletteBrightness);
					screen->fillRect(screen->getBounds(), 0);
				}
				return Common::kNoError;
			case Common::EVENT_LBUTTONDOWN:
				page ^= 1;
				needsRedraw = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					if (Graphics::Screen *screen = _engine.getScreen()) {
						applyMenuPalette(*screen, _engine, palette, paletteBrightness);
						screen->fillRect(screen->getBounds(), 0);
					}
					return Common::kNoError;
				}
				page ^= 1;
				needsRedraw = true;
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

Common::String StartupMenuSystem::runModalTextEntryDialog(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const Graphics::Font &titleFont, const Graphics::Font &entryFont,
		const Common::String &title, const Common::String &initialText, StartupFlow &startupFlow) {
	Graphics::FrameLimiter limiter(g_system, 60);
	Common::String text = initialText;
	const uint maxCharacters = 30;
	bool needsRedraw = true;
	bool cursorVisible = true;
	uint32 cursorToggleTicks = g_system->getMillis() + 400;

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderTextEntryDialog(_engine, backdrop, palette, paletteBrightness, titleFont, entryFont,
				title, text, cursorVisible, textEntryOkRect().contains(_mousePos), textEntryCancelRect().contains(_mousePos));
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (startupFlow.handleSystemEvent(event, result))
				return Common::String();

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (textEntryOkRect().contains(_mousePos))
					return text;
				if (textEntryCancelRect().contains(_mousePos))
					return Common::String();
				break;
			case Common::EVENT_RBUTTONDOWN:
				return Common::String();
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::String();
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER)
					return text;
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
					if (!text.empty()) {
						text.deleteLastChar();
						needsRedraw = true;
					}
					break;
				}
				if (appendTextEntryCharacter(text, entryFont, event.kbd.ascii, maxCharacters))
					needsRedraw = true;
				break;
			default:
				break;
			}
		}

		const uint32 now = g_system->getMillis();
		if ((int32)(now - cursorToggleTicks) >= 0) {
			cursorVisible = !cursorVisible;
			cursorToggleTicks = now + 400;
			needsRedraw = true;
		}

		if (_engine.getRuntimeEntities() && _engine.getRuntimeEntities()->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::String();
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
