/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 */

#ifndef FONTMAN_H
#define FONTMAN_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/str.h"
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
	//const Font *getFontByName(const Common::String &name) const;

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
};


} // End of namespace Graphics

/** Shortcut for accessing the font manager. */
#define FontMan		(Graphics::FontManager::instance())

#endif
