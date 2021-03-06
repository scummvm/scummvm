/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_FONT_H
#define AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_FONT_H

#include "ags/plugins/ags_sprite_font/character_entry.h"
#include "ags/plugins/agsplugin.h"
#include "ags/lib/std/map.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

class VariableWidthFont {
public:
	VariableWidthFont(void);
	~VariableWidthFont(void);
	void SetGlyph(int character, int x, int y, int width, int height);
	void SetLineHeightAdjust(int LineHeight, int SpacingHeight, int SpacingOverride);
	int SpriteNumber;
	int FontReplaced;
	int Spacing;
	int LineHeightAdjust;
	int LineSpacingAdjust;
	int LineSpacingOverride;
	std::map<char, CharacterEntry> characters;
};

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3

#endif
