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

#include "ags/plugins/ags_sprite_font/ags_sprite_font.h"
#include "ags/shared/core/platform.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

#pragma region Defines

#define MIN_EDITOR_VERSION 1
#define MIN_ENGINE_VERSION 3

#define DEFAULT_RGB_R_SHIFT_32  16
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  0
#define DEFAULT_RGB_A_SHIFT_32  24

#define abs(a)                       ((a)<0 ? -(a) : (a))
#define ChannelBlend_Normal(B,L)     ((uint8)(B))
#define ChannelBlend_Lighten(B,L)    ((uint8)((L > B) ? L:B))
#define ChannelBlend_Darken(B,L)     ((uint8)((L > B) ? B:L))
#define ChannelBlend_Multiply(B,L)   ((uint8)((B * L) / 255))
#define ChannelBlend_Average(B,L)    ((uint8)((B + L) / 2))
#define ChannelBlend_Add(B,L)        ((uint8)(min(255, (B + L))))
#define ChannelBlend_Subtract(B,L)   ((uint8)((B + L < 255) ? 0:(B + L - 255)))
#define ChannelBlend_Difference(B,L) ((uint8)(abs(B - L)))
#define ChannelBlend_Negation(B,L)   ((uint8)(255 - abs(255 - B - L)))
#define ChannelBlend_Screen(B,L)     ((uint8)(255 - (((255 - B) * (255 - L)) >> 8)))
#define ChannelBlend_Exclusion(B,L)  ((uint8)(B + L - 2 * B * L / 255))
#define ChannelBlend_Overlay(B,L)    ((uint8)((L < 128) ? (2 * B * L / 255):(255 - 2 * (255 - B) * (255 - L) / 255)))
#define ChannelBlend_SoftLight(B,L)  ((uint8)((L < 128)?(2*((B>>1)+64))*((float)L/255):(255-(2*(255-((B>>1)+64))*(float)(255-L)/255))))
#define ChannelBlend_HardLight(B,L)  (ChannelBlend_Overlay(L,B))
#define ChannelBlend_ColorDodge(B,L) ((uint8)((L == 255) ? L:min(255, ((B << 8 ) / (255 - L)))))
#define ChannelBlend_ColorBurn(B,L)  ((uint8)((L == 0) ? L:max(0, (255 - ((255 - B) << 8 ) / L))))
#define ChannelBlend_LinearDodge(B,L)(ChannelBlend_Add(B,L))
#define ChannelBlend_LinearBurn(B,L) (ChannelBlend_Subtract(B,L))
#define ChannelBlend_LinearLight(B,L)((uint8)(L < 128)?ChannelBlend_LinearBurn(B,(2 * L)):ChannelBlend_LinearDodge(B,(2 * (L - 128))))
#define ChannelBlend_VividLight(B,L) ((uint8)(L < 128)?ChannelBlend_ColorBurn(B,(2 * L)):ChannelBlend_ColorDodge(B,(2 * (L - 128))))
#define ChannelBlend_PinLight(B,L)   ((uint8)(L < 128)?ChannelBlend_Darken(B,(2 * L)):ChannelBlend_Lighten(B,(2 * (L - 128))))
#define ChannelBlend_HardMix(B,L)    ((uint8)((ChannelBlend_VividLight(B,L) < 128) ? 0:255))
#define ChannelBlend_Reflect(B,L)    ((uint8)((L == 255) ? L:min(255, (B * B / (255 - L)))))
#define ChannelBlend_Glow(B,L)       (ChannelBlend_Reflect(L,B))
#define ChannelBlend_Phoenix(B,L)    ((uint8)(min(B,L) - max(B,L) + 255))
#define ChannelBlend_Alpha(B,L,O)    ((uint8)(O * B + (1 - O) * L))
#define ChannelBlend_AlphaF(B,L,F,O) (ChannelBlend_Alpha(F(B,L),B,O))


#pragma endregion

#define STRINGIFY(s) STRINGIFY_X(s)
#define STRINGIFY_X(s) #s

const char *AGSSpriteFont::AGS_GetPluginName() {
	return "AGSSpriteFont";
}

void AGSSpriteFont::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	if (_fontRenderer == nullptr) {
		_engine->PrintDebugConsole("AGSSpriteFont: Init fixed width renderer");
		_fontRenderer = new SpriteFontRenderer(engine);
	}
	if (_vWidthRenderer == nullptr) {
		_engine->PrintDebugConsole("AGSSpriteFont: Init vari width renderer");
		_vWidthRenderer = new VariableWidthSpriteFontRenderer(engine);
	}
	// Make sure it's got the version with the features we need
	if (_engine->version < MIN_ENGINE_VERSION)
		_engine->AbortGame("Plugin needs engine version " STRINGIFY(MIN_ENGINE_VERSION) " or newer.");

	// Register functions
	_engine->PrintDebugConsole("AGSSpriteFont: Register functions");
	SCRIPT_METHOD(SetSpriteFont, AGSSpriteFont::SetSpriteFont);
	SCRIPT_METHOD(SetVariableSpriteFont, AGSSpriteFont::SetVariableSpriteFont);
	SCRIPT_METHOD(SetGlyph, AGSSpriteFont::SetGlyph);
	SCRIPT_METHOD(SetSpacing, AGSSpriteFont::SetSpacing);
	SCRIPT_METHOD(SetLineHeightAdjust, AGSSpriteFont::SetLineHeightAdjust);
}

void AGSSpriteFont::AGS_EngineShutdown() {
	delete _fontRenderer;
	delete _vWidthRenderer;
}

void AGSSpriteFont::SetSpriteFont(ScriptMethodParams &params) {
	PARAMS9(int, fontNum, int, sprite, int, rows, int, columns, int, charWidth, int, charHeight, int, charMin, int, charMax, bool, use32bit);
	_engine->PrintDebugConsole("AGSSpriteFont: SetSpriteFont");
	_fontRenderer->SetSpriteFont(fontNum, sprite, rows, columns, charWidth, charHeight, charMin, charMax, use32bit);
	_engine->ReplaceFontRenderer(fontNum, _fontRenderer);
}

void AGSSpriteFont::SetVariableSpriteFont(ScriptMethodParams &params) {
	PARAMS2(int, fontNum, int, sprite);
	_engine->PrintDebugConsole("AGSSpriteFont: SetVariableFont");
	_vWidthRenderer->SetSprite(fontNum, sprite);
	_engine->ReplaceFontRenderer(fontNum, _vWidthRenderer);
}

void AGSSpriteFont::SetGlyph(ScriptMethodParams &params) {
	PARAMS6(int, fontNum, int, charNum, int, x, int, y, int, width, int, height);
	_engine->PrintDebugConsole("AGSSpriteFont: SetGlyph");
	_vWidthRenderer->SetGlyph(fontNum, charNum, x, y, width, height);
}

void AGSSpriteFont::SetSpacing(ScriptMethodParams &params) {
	PARAMS2(int, fontNum, int, spacing);
	_engine->PrintDebugConsole("AGSSpriteFont: SetSpacing");
	_vWidthRenderer->SetSpacing(fontNum, spacing);
}

void AGSSpriteFont::SetLineHeightAdjust(ScriptMethodParams &params) {
	//PARAMS4(int, v1, int, v2, int, v3, int, v4);
	// TODO
}


} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
