/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_SERVICES_FONT_PROVIDER_H
#define STARK_SERVICES_FONT_PROVIDER_H

#include "common/hash-str.h"
#include "common/ptr.h"

namespace Common {
class INIFile;
}

namespace Graphics {
class Font;
}

namespace Stark {

/**
 * The font provider offers a set of predefined fonts for the game to use
 */
class FontProvider {
public:
	FontProvider();
	~FontProvider();

	enum FontType {
		kSmallFont,
		kBigFont,
		kCustomFont
	};

	/**
	 * Request a font matching the specified parameters
	 */
	const Graphics::Font *getScaledFont(FontType type, int32 customFontIndex);

	/**
	 * Get the height of the font matching the specified parameters
	 */
	uint getScaledFontHeight(FontType type, int32 customFontIndex);
	uint getOriginalFontHeight(FontType type, int32 customFontIndex);

	/** Load all the fonts to memory */
	void initFonts();

private:
	struct FontHolder {
		Common::String _name;

		uint32 _originalHeight;
		uint32 _scaledHeight;

		Common::SharedPtr<Graphics::Font> _font;

		FontHolder() : _originalHeight(0), _scaledHeight(0) {}
		FontHolder(FontProvider *fontProvider, const Common::String &name, uint32 height);

	};

	void readFontEntry(const Common::INIFile *gui, FontHolder &holder, const char *nameKey, const char *sizeKey);
	FontHolder *getFontHolder(FontType type, int32 customFontIndex);

	FontHolder _smallFont;
	FontHolder _bigFont;
	FontHolder _customFonts[8];

	Common::StringMap _ttfFileMap;
};

} // End of namespace Stark

#endif // STARK_SERVICES_FONT_PROVIDER_H
