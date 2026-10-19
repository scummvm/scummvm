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

#include "harvester/menu/menu_render.h"

#include "common/algorithm.h"
#include "graphics/fontman.h"
#include "harvester/cft_font.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/script/script.h"
#include "harvester/text.h"

namespace Harvester {
namespace MenuRendering {

static const int kInventoryX = 64;
static const int kInventoryY = 48;
static const int kLogoX = 160;
static const int kLogoY = 0;
const int kMenuStartY = 100;
const int kMenuLineSpacing = 28;
const byte kTextColorNormal = 255;
static const byte kTextColorHover = 251;
static const byte kTextColorDim = 248;
static const byte kShadowColor = 0;
static const byte kPanelFillColor = 1;
static const byte kQuickTipActionColor = 0xc3;
const int kOptionsSliderMinX = 0x16f;
const int kOptionsSliderStep = 0x13;
static const int kOptionsVolumeBitmapX = 0x168;
static const int kOptionsVolumeBitmapY = 0x68;
const int kOptionsIndicatorY = 0x6c;
static const int kQuickTipsOverlayX = 167;
static const int kQuickTipsOverlayY = 200;
static const int kQuickTipsHeaderY = 202;
static const int kQuickTipTextX = 180;
static const int kQuickTipTextY = 228;
static const int kEnglishQuickTipTextWidth = 280;
static const int kLocalizedQuickTipTextWidth = 300;
static const int kLocalizedQuickTipActionY = 308;
static const int kQuickTipExitX = 180;
static const int kQuickTipToggleX = 258;
static const int kQuickTipNextRightInset = 8;
static const int kConfirmDialogX = 167;
static const int kConfirmDialogY = 200;
static const int kConfirmPromptTextX = 0xea;
static const int kConfirmPromptTextY = 0xd6;
static const int kConfirmChoiceTextY = 0xf8;
static const int kConfirmYesTextX = 0xdc;
static const int kConfirmNoTextX = 0x17c;
static const int kPasswordEntryX = 0xdc;
static const int kPasswordEntryY = 0xdc;
const int kPasswordEntryWidth = 0x226;

void applyMenuPalette(Graphics::Screen &screen, const HarvesterEngine &engine,
		const byte *palette, float brightness) {
	if (!palette)
		return;

	byte displayPalette[256 * 3];
	buildHarvesterDisplayPalette(palette, brightness * engine.getGammaBrightnessScale(), displayPalette);
	screen.setPalette(displayPalette);
}

void renderCdChangePromptScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap,
		const byte *palette) {
	Graphics::Screen *screen = engine.getScreen();
	if (!screen)
		return;

	applyMenuPalette(*screen, engine, palette, 1.0f);
	screen->fillRect(screen->getBounds(), 0);
	if (bitmap.isValid())
		screen->copyRectToSurface(bitmap.pixels.data(), bitmap.width, 0, 0, bitmap.width, bitmap.height);
	if (engine.getRuntimeEntities())
		engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void renderHelpScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap, const byte *palette) {
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

static Common::String buildTextModeSuffix(const Script &script, const MenuTextConfig &config) {
	switch (script.getDialogueTextMode()) {
	case kStartupDialogueTextNone:
		return Common::String::format(" - %s", config.noLabel.c_str());
	case kStartupDialogueTextClick:
		return Common::String::format(" - %s", config.clickLabel.c_str());
	case kStartupDialogueTextYes:
	default:
		return Common::String::format(" - %s", config.yesLabel.c_str());
	}
}

static Common::String buildOptionsMenuItemLabel(const Script &script,
		const MenuTextConfig &config, int index) {
	if (index < 0 || index >= static_cast<int>(config.optionItems.size()))
		return Common::String();

	switch (index) {
	case 3:
		return config.optionItems[index] + buildTextModeSuffix(script, config);
	case 4:
		return Common::String::format("%s - %s", config.optionItems[index].c_str(),
			(script.isGoreEnabled() ? config.onLabel : config.offLabel).c_str());
	case 6:
		return Common::String::format("%s - %s", config.optionItems[index].c_str(),
			(script.getParentalPassword().empty() ? config.offLabel : config.onLabel).c_str());
	default:
		return config.optionItems[index];
	}
}

void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y) {
	if (!bitmap.isValid())
		return;

	screen.copyRectToSurface(bitmap.pixels.data(), bitmap.width, x, y, bitmap.width, bitmap.height);
}

void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, const TextLayout &layout, byte color, Graphics::TextAlign align) {
	const int x = layout.x;
	const int y = layout.y;
	const int width = layout.width;

	font.drawString(&screen, text, x + 1, y + 1, width, kShadowColor, align);
	font.drawString(&screen, text, x, y, width, color, align);
}

static void drawSinglePassString(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, int x, int y) {
	if (text.empty())
		return;

	font.drawString(&screen, text, x, y, font.getStringWidth(text), 0);
}

static void drawWrappedShadowedText(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, const TextLayout &layout, byte color) {
	const int x = layout.x;
	const int y = layout.y;
	const int width = layout.width;

	Common::Array<Common::String> lines;
	font.wordWrapText(text, width, lines);
	for (uint i = 0; i < lines.size(); ++i)
		drawShadowedString(screen, font, lines[i],
			TextLayout(x, y + static_cast<int>(i) * (font.getFontHeight() + 2), width), color);
}

static void splitMenuConfigLines(const Common::String &text, Common::Array<Common::String> &lines) {
	lines.clear();
	Common::String currentLine;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] == '/') {
			lines.push_back(Common::move(currentLine));
			currentLine.clear();
			continue;
		}
		currentLine += text[i];
	}
	lines.push_back(Common::move(currentLine));
}

void Renderer::renderOptionsMenu(const FontPair &fonts, const Art &art,
		const MenuTextConfig &config, const SliderBitmaps &sliders, int selectedItem, bool drawCursor) const {
	Graphics::Screen *screen = _engine.getScreen();
	Script *script = _engine.getScript();
	if (!screen || !script)
		return;

	applyMenuPalette(*screen, _engine, _palette, _paletteBrightness);
	blitBitmap(*screen, _backdrop, 0, 0);
	blitTransparentBitmap(*screen, art.getLogoBitmap(), kLogoX, kLogoY);

	const int lineHeight = getNativeRoomMenuLineHeight(fonts.primary);
	for (int i = 0; i < static_cast<int>(config.optionItems.size()); ++i) {
		const Common::String label = buildOptionsMenuItemLabel(*script, config, i);
		const Graphics::Font &font = (i == selectedItem) ? fonts.primary : fonts.secondary;
		const int width = font.getStringWidth(label);
		const int x = (screen->w - width) / 2;
		const int y = kMenuStartY + i * lineHeight;
		font.drawString(screen, label, x, y, width, 0);
	}

	// Native run_main_menu spawns the option text entities before adding the
	// VOLUME/INDICATR bitmap entities, so the bars clip the trailing overlap on
	// the first three rows instead of the text painting over them.
	for (int i = 0; i < 3; ++i)
		blitTransparentBitmap(*screen, sliders.bar, kOptionsVolumeBitmapX, kOptionsVolumeBitmapY + i * lineHeight);

	const int levels[3] = {
		script->getFxVolumeLevel(),
		script->getMusicVolumeLevel(),
		script->getGammaLevel()
	};
	for (int i = 0; i < 3; ++i) {
		blitTransparentBitmap(*screen, sliders.indicator, kOptionsSliderMinX + levels[i] * kOptionsSliderStep,
			kOptionsIndicatorY + i * lineHeight);
	}

	if (drawCursor && _engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void Renderer::renderPasswordPrompt(const FontPair &fonts, const Art &art,
		const Common::String &title, const Common::String &text, bool cursorVisible, bool drawLogo) const {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen)
		return;

	if (_palette)
		applyMenuPalette(*screen, _engine, _palette, _paletteBrightness);
	blitBitmap(*screen, _backdrop, 0, 0);
	if (drawLogo)
		blitTransparentBitmap(*screen, art.getLogoBitmap(), kLogoX, kLogoY);

	const int titleWidth = fonts.primary.getStringWidth(title);
	const int titleX = (screen->w - titleWidth) / 2;
	fonts.primary.drawString(screen, title, titleX, 0xa0, titleWidth, 0);

	Common::String displayText = text;
	if (cursorVisible)
		displayText += '~';
	fonts.secondary.drawString(screen, displayText, kPasswordEntryX, kPasswordEntryY,
		kPasswordEntryWidth, 0);

	screen->makeAllDirty();
	screen->update();
}

void Renderer::renderQuickTipsOverlay(const MenuTextConfig &config,
		const QuickTipsLayout &layout, const Common::String &tipText) const {
	Graphics::Screen *screen = _engine.getScreen();
	const Art *art = _engine.getArt();
	if (!screen || !art)
		return;

	applyMenuPalette(*screen, _engine, _palette, _paletteBrightness);
	blitBitmap(*screen, _backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);
	drawQuickTipsPanel(_engine, config, layout, tipText);

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void Renderer::renderConfirmPrompt(const Graphics::Font &promptFont,
		const Graphics::Font &yesFont, const Graphics::Font &noFont,
		const IndexedBitmap &textbox, const Common::String &promptText, const MenuTextConfig &config) const {
	Graphics::Screen *screen = _engine.getScreen();
	const Art *art = _engine.getArt();
	if (!screen || !art)
		return;

	applyMenuPalette(*screen, _engine, _palette, _paletteBrightness);
	blitBitmap(*screen, _backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);
	blitTransparentBitmap(*screen, textbox, kConfirmDialogX, kConfirmDialogY);
	Common::Array<Common::String> promptLines;
	splitMenuConfigLines(promptText, promptLines);
	for (uint i = 0; i < promptLines.size(); ++i) {
		drawSinglePassString(*screen, promptFont, promptLines[i], kConfirmPromptTextX,
			kConfirmPromptTextY + static_cast<int>(i) * (promptFont.getFontHeight() + 2));
	}
	drawSinglePassString(*screen, yesFont, config.yesLabel, kConfirmYesTextX, kConfirmChoiceTextY);
	drawSinglePassString(*screen, noFont, config.noLabel, kConfirmNoTextX, kConfirmChoiceTextY);

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

int getNativeRoomMenuLineHeight(const Graphics::Font &selectedFont) {
	return selectedFont.getFontHeight() + 2;
}

} // End of namespace MenuRendering

bool resolveQuickTipsLayout(HarvesterEngine &engine, const MenuTextConfig &config,
		QuickTipsLayout &layout) {
	using namespace MenuRendering;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	const Art *art = engine.getArt();
	const Script *script = engine.getScript();
	if (!font || !art || !script)
		return false;

	if (!config.hasQuickTipsHeader()) {
		layout.exitRect = Common::Rect(180, 280, 238, 291);
		layout.nextRect = Common::Rect(420, 280, 492, 291);
		layout.toggleRect = Common::Rect(258, 280, 366, 291);
		return true;
	}

	const IndexedBitmap *panel = art->getQuickTipsTextboxBitmap();
	if (!panel || !panel->isValid())
		return false;

	const Common::String &toggleLabel = script->isQuickTipsEnabled()
		? config.quickTipsOnLabel : config.quickTipsOffLabel;
	auto makeLabelRect = [font](const Common::String &label, int x) {
		const int width = MAX<int>(1, font->getStringWidth(label));
		const int height = MAX<int>(1, font->getFontHeight());
		return Common::Rect(x, kLocalizedQuickTipActionY,
			x + width, kLocalizedQuickTipActionY + height);
	};

	layout.exitRect = makeLabelRect(config.quickTipsExitLabel, kQuickTipExitX);
	layout.toggleRect = makeLabelRect(toggleLabel, kQuickTipToggleX);
	const int nextX = kQuickTipsOverlayX + static_cast<int>(panel->width)
		- font->getStringWidth(config.quickTipsNextLabel) - kQuickTipNextRightInset;
	layout.nextRect = makeLabelRect(config.quickTipsNextLabel, nextX);
	return true;
}

void drawQuickTipsPanel(HarvesterEngine &engine, const MenuTextConfig &config,
		const QuickTipsLayout &layout, const Common::String &tipText) {
	using namespace MenuRendering;

	Graphics::Screen *screen = engine.getScreen();
	const Art *art = engine.getArt();
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	const Script *script = engine.getScript();
	if (!screen || !art || !font || !script)
		return;

	const IndexedBitmap *panel = config.hasQuickTipsHeader()
		? art->getQuickTipsTextboxBitmap() : &art->getTipsBitmap();
	if (!panel || !panel->isValid())
		return;

	blitBitmap(*screen, *panel, kQuickTipsOverlayX, kQuickTipsOverlayY);
	if (config.hasQuickTipsHeader()) {
		drawShadowedString(*screen, *font, config.quickTipsHeader,
			TextLayout(kQuickTipsOverlayX, kQuickTipsHeaderY, panel->width),
			kQuickTipActionColor, Graphics::kTextAlignCenter);
	}

	const int tipTextWidth = config.hasQuickTipsHeader()
		? kLocalizedQuickTipTextWidth : kEnglishQuickTipTextWidth;
	drawWrappedShadowedText(*screen, *font, tipText,
		TextLayout(kQuickTipTextX, kQuickTipTextY, tipTextWidth), kTextColorNormal);
	const Common::String &toggleLabel = script->isQuickTipsEnabled()
		? config.quickTipsOnLabel : config.quickTipsOffLabel;
	drawShadowedString(*screen, *font, config.quickTipsExitLabel,
		TextLayout(layout.exitRect.left, layout.exitRect.top, layout.exitRect.width()), kQuickTipActionColor);
	drawShadowedString(*screen, *font, config.quickTipsNextLabel,
		TextLayout(layout.nextRect.left, layout.nextRect.top, layout.nextRect.width()), kQuickTipActionColor);
	drawShadowedString(*screen, *font, toggleLabel,
		TextLayout(layout.toggleRect.left, layout.toggleRect.top, layout.toggleRect.width()), kQuickTipActionColor);
}

void MenuSystem::renderMainMenuScreen(int selectedItem, const Common::String &statusMessage,
		bool drawCursor, const Common::Array<Common::String> &menuItems) const {
	using namespace MenuRendering;

	Graphics::Screen *screen = _engine.getScreen();
	const Art *art = _engine.getArt();
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

	drawShadowedString(*screen, *titleFont, "Harvester", TextLayout(panel.left, 110, panel.width()),
		kTextColorNormal, Graphics::kTextAlignCenter);

	for (uint i = 0; i < menuItems.size(); ++i) {
		const int itemY = kMenuStartY + static_cast<int>(i) * kMenuLineSpacing;
		const byte color = (static_cast<int>(i) == selectedItem) ? kTextColorHover : kTextColorNormal;
		drawShadowedString(*screen, *bodyFont, menuItems[i],
			TextLayout(panel.left, itemY, panel.width()), color, Graphics::kTextAlignCenter);
	}

	if (!statusMessage.empty()) {
		drawWrappedShadowedText(*screen, *bodyFont, statusMessage,
			TextLayout(panel.left + 24, 352, panel.width() - 48), kTextColorNormal);
	}
	drawShadowedString(*screen, *bodyFont, "Use mouse or arrow keys. Enter activates. Esc returns to launcher.",
		TextLayout(panel.left, 404, panel.width()), kTextColorDim, Graphics::kTextAlignCenter);

	if (drawCursor && _engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

void MenuSystem::renderBackdropMenuScreen(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const Common::Array<Common::String> &menuItems,
		int selectedItem) const {
	using namespace MenuRendering;

	Graphics::Screen *screen = _engine.getScreen();
	const Art *art = _engine.getArt();
	const Text *text = _engine.getText();
	const CftFontResource *selectedFontResource = text ? text->findFont("HARVFONT") : nullptr;
	const CftFontResource *unselectedFontResource = text ? text->findFont("HARVFNT2") : nullptr;
	if (!screen || !art || !selectedFontResource || !unselectedFontResource || !backdrop.isValid())
		return;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return;

	applyMenuPalette(*screen, _engine, palette, paletteBrightness);
	blitBitmap(*screen, backdrop, 0, 0);
	blitTransparentBitmap(*screen, art->getLogoBitmap(), kLogoX, kLogoY);

	const int lineSpacing = getNativeRoomMenuLineHeight(selectedFont);
	for (uint i = 0; i < menuItems.size(); ++i) {
		const Graphics::Font *font = (static_cast<int>(i) == selectedItem)
			? static_cast<const Graphics::Font *>(&selectedFont)
			: static_cast<const Graphics::Font *>(&unselectedFont);
		const int width = font->getStringWidth(menuItems[i]);
		const int x = (screen->w - width) / 2;
		const int y = kMenuStartY + static_cast<int>(i) * lineSpacing;
		font->drawString(screen, menuItems[i], x, y, width, 0);
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

} // End of namespace Harvester
