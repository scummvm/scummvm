/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_FONTMAN_H
#define GRAPHICS_FONTMAN_H

#include "common/sys.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "graphics/font.h"


namespace Graphics {

class FontManager : public Common::Singleton<FontManager> {
public:
	enum FontUsage {
		kOSDFont,
		kConsoleFont,
		kGUIFont,
		kBigGUIFont
	};

	/**
	 * Retrieve a font object based on its 'name'.
	 *
	 * @param name	the name of the font to be retrieved.
	 * @return a pointer to a font, or 0 if no suitable font was found.
	 */
	const Font *getFontByName(const Common::String &name) const;

	/**
	 * Associates a font object with an 'name'
	 *
	 * @param name	the name of the font
	 * @param font	the font object
	 * @return true on success, false on failure
	 */
	bool assignFontToName(const Common::String &name, const Font *font) { _fontMap[name] = font; return true; }

	/**
	 * Removes binding from name to font
	 *
	 * @param name	name which should be removed
	 */
	void removeFontName(const Common::String &name) { _fontMap.erase(name); }

	/**
	 * Retrieve a font object based on what it is supposed
	 * to be used for
	 *
	 * @param usage	a FontUsage enum value indicating what the font will be used for.
	 * @return a pointer to a font, or 0 if no suitable font was found.
	 */
	const Font *getFontByUsage(FontUsage usage) const;

	//const Font *getFontBySize(int size???) const;


private:
	friend class Common::Singleton<SingletonBaseType>;
	FontManager();

	Common::HashMap<Common::String, const Font *> _fontMap;
};


} // End of namespace Graphics

/** Shortcut for accessing the font manager. */
#define FontMan		(Graphics::FontManager::instance())

#endif
