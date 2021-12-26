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

#include "ags/plugins/ags_sprite_font/ags_sprite_font_clifftop.h"
#include "ags/plugins/ags_sprite_font/sprite_font_renderer_clifftop.h"
#include "ags/plugins/ags_sprite_font/variable_width_sprite_font_clifftop.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

void AGSSpriteFontClifftopGames::AGS_EngineStartup(IAGSEngine *engine) {
	// Use custom font renderers
	// They need to be set before calling AGSSpriteFont::AGS_EngineStartup()
	engine->PrintDebugConsole("AGSSpriteFont: Init fixed width renderer");
	_fontRenderer = new SpriteFontRenderer(engine);

	engine->PrintDebugConsole("AGSSpriteFont: Init vari width renderer");
	_vWidthRenderer = new VariableWidthSpriteFontRendererClifftop(engine);

	AGSSpriteFont::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SetLineHeightAdjust, AGSSpriteFontClifftopGames::SetLineHeightAdjust);
}

void AGSSpriteFontClifftopGames::SetLineHeightAdjust(ScriptMethodParams &params) {
	PARAMS4(int, fontNum, int, LineHeight, int, SpacingHeight, int, SpacingOverride);

	_vWidthRenderer->SetLineHeightAdjust(fontNum, LineHeight, SpacingHeight, SpacingOverride);
}

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
