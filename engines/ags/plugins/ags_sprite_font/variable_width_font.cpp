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

#include "ags/plugins/ags_sprite_font/variable_width_font.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

void VariableWidthFont::SetGlyph(int character, int x, int y, int width, int height) {
	characters[character].X = x;
	characters[character].Y = y;
	characters[character].Width = width;
	characters[character].Height = height;
	characters[character].Character = character;
}

void VariableWidthFont::SetLineHeightAdjust(int LineHeight, int SpacingHeight, int SpacingOverride) {
	LineHeightAdjust = LineHeight;
	LineSpacingAdjust = SpacingHeight;
	LineSpacingOverride = SpacingOverride;
}

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
