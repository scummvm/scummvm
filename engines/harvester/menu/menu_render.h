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

#ifndef HARVESTER_MENU_RENDER_H
#define HARVESTER_MENU_RENDER_H

#include "graphics/font.h"
#include "harvester/menu/menu.h"

namespace Graphics {
class Screen;
}

namespace Harvester {
namespace MenuRendering {

extern const int kMenuStartY;
extern const int kMenuLineSpacing;
extern const byte kTextColorNormal;
extern const int kOptionsSliderMinX;
extern const int kOptionsSliderStep;
extern const int kOptionsIndicatorY;
extern const int kPasswordEntryWidth;

struct TextLayout {
	TextLayout(int x, int y, int width) : x(x), y(y), width(width) {}

	int x;
	int y;
	int width;
};

struct FontPair {
	FontPair(const Graphics::Font &primary, const Graphics::Font &secondary)
		: primary(primary), secondary(secondary) {}

	const Graphics::Font &primary;
	const Graphics::Font &secondary;
};

struct SliderBitmaps {
	SliderBitmaps(const IndexedBitmap &bar, const IndexedBitmap &indicator)
		: bar(bar), indicator(indicator) {}

	const IndexedBitmap &bar;
	const IndexedBitmap &indicator;
};

// Drawing calls borrow their backdrop, palette, fonts, and bitmaps synchronously.
class Renderer {
public:
	Renderer(HarvesterEngine &engine, const IndexedBitmap &backdrop,
		const byte *palette, float paletteBrightness)
		: _engine(engine), _backdrop(backdrop), _palette(palette), _paletteBrightness(paletteBrightness) {}

	void renderOptionsMenu(const FontPair &fonts, const Art &art, const MenuTextConfig &config,
		const SliderBitmaps &sliders, int selectedItem, bool drawCursor = true) const;
	void renderPasswordPrompt(const FontPair &fonts, const Art &art, const Common::String &title,
		const Common::String &text, bool cursorVisible, bool drawLogo) const;
	void renderQuickTipsOverlay(const MenuTextConfig &config, const QuickTipsLayout &layout,
		const Common::String &tipText) const;
	void renderConfirmPrompt(const Graphics::Font &promptFont,
		const Graphics::Font &yesFont, const Graphics::Font &noFont,
		const IndexedBitmap &textbox, const Common::String &promptText, const MenuTextConfig &config) const;

private:
	HarvesterEngine &_engine;
	const IndexedBitmap &_backdrop;
	const byte *_palette;
	float _paletteBrightness;
};

void applyMenuPalette(Graphics::Screen &screen, const HarvesterEngine &engine,
	const byte *palette, float brightness);
void blitBitmap(Graphics::Screen &screen, const IndexedBitmap &bitmap, int x, int y);
void drawShadowedString(Graphics::Screen &screen, const Graphics::Font &font,
	const Common::String &text, const TextLayout &layout, byte color,
	Graphics::TextAlign align = Graphics::kTextAlignLeft);
void renderCdChangePromptScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap, const byte *palette);
void renderHelpScreen(HarvesterEngine &engine, const IndexedBitmap &bitmap, const byte *palette);
int getNativeRoomMenuLineHeight(const Graphics::Font &selectedFont);

} // End of namespace MenuRendering
} // End of namespace Harvester

#endif // HARVESTER_MENU_RENDER_H
