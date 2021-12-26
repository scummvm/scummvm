/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_FONT_H
#define AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_FONT_H

#include "ags/plugins/ags_sprite_font/character_entry.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/lib/std/map.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

class VariableWidthFont {
public:
	int SpriteNumber = 0;
	int FontReplaced = 0;
	int Spacing = 0;
	int LineHeightAdjust = 0;
	int LineSpacingAdjust = 0;
	int LineSpacingOverride = 0;
	std::map<char, CharacterEntry> characters;

public:
	void SetGlyph(int character, int x, int y, int width, int height);
	void SetLineHeightAdjust(int LineHeight, int SpacingHeight, int SpacingOverride);
};

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3

#endif
