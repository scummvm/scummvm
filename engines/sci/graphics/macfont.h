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

#ifndef SCI_GRAPHICS_MACFONT_H
#define SCI_GRAPHICS_MACFONT_H

#include "common/hashmap.h"
#include "sci/graphics/helpers.h"   // for GuiResourceId

namespace Common {
class MacResManager;
}
namespace Graphics {
class MacFontManager;
class Font;
}

namespace Sci {

/**
 * GfxMacFontManager handles loading Mac fonts and mapping them to SCI fonts.
 *
 * Most Mac SCI1/1.1 games use native Mac fonts to draw their controls.
 * This was done by altering the kernel functions used by the control scripts
 * such as kDrawControl and kTextSize. All dialog text and buttons use these
 * functions so this affects most text in SCI games. Scripts that draw text with
 * kDisplay are unaffected and continue to use SCI fonts.
 *
 * The Mac game window could be set to three sizes: 100% (small), 150% (medium),
 * or 200% (large). These percentages were relative to the internal resolution
 * of the game; usually 320x200. The game's screen was stretched to the window
 * and the Mac fonts were drawn directly to the window using Mac's Toolbox API.
 * The Mac interpreter chose the font size based on the window size. At 200% the
 * text was relatively high-resolution and looked quite crisp and distinct.
 *
 * The Mac fonts were originally included in the resource fork of the game's
 * executable. The resource fork also contained a small table that specified
 * the mapping of SCI font ids to Mac font ids, along with a default Mac font,
 * and the Mac font sizes to use for the small, medium, and large window.
 *
 * Sierra switched to using the Palatino system font in QFG1VGA and LSL6.
 * The font mapping table still existed, but the interpreter was hard-coded to
 * always use Palatino values instead.
 *
 * GfxMacFontManager handles both cases by accepting the loaded Mac executable
 * of games that include their own fonts. If no executable is provided then it
 * attempts to use Palatino from classicmacfonts.dat along with the hard-coded
 * values from Sierra's interpreter.
 *
 * GfxMacFontManager only exposes the small and large fonts. When Mac fonts are
 * present and high resolution graphics are enabled, the game is upscaled to
 * 200% and the large font is used. If high resolution graphics are disabled
 * then the small font is used with no upscaling. Either way, the small font is
 * always used for the calculations that determine the size of the text area.
 *
 * TODO: Add KQ5 support. It did things differently and it only had two window
 * sizes. The mapping table changed and it appears to have its own sizing logic.
 * Unfortunately, KQ5's interpreter doesn't include function names.
 */
class GfxMacFontManager {
public:
	GfxMacFontManager(Common::MacResManager *macExecutable = nullptr);
	~GfxMacFontManager();

	bool hasFonts();
	bool usesSystemFonts();
	const Graphics::Font *getSmallFont(GuiResourceId sciFontId);
	const Graphics::Font *getLargeFont(GuiResourceId sciFontId);

private:
	bool initFromFontTable(Common::MacResManager *macExecutable);
	const Graphics::Font *getMacFont(int macFontId, int size);

	struct MacFontItem {
		const Graphics::Font *smallFont;
		const Graphics::Font *largeFont;
	};

	bool _usesSystemFonts;
	Graphics::MacFontManager *_macFontManager;
	Common::HashMap<GuiResourceId, MacFontItem *> _macFonts;
	MacFontItem *_defaultFont;
};

} // End of namespace Sci

#endif
