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

#include "twp/hud.h"
#include "twp/twp.h"
#include "math/vector2d.h"
#include "graphics/opengl/debug.h"
#include "graphics/opengl/system_headers.h"
#include "common/config-manager.h"

namespace Twp {

HudShader::HudShader() {
	const char *verbVtxShader = R"(#version 110
	attribute vec2 a_position;
	attribute vec4 a_color;
	attribute vec2 a_texCoords;

	uniform vec4 u_shadowColor;
	uniform vec4 u_normalColor;
	uniform vec4 u_highlightColor;
	uniform vec2 u_ranges;
	uniform mat4 u_transform;

	varying vec4 v_color;
	varying vec2 v_texCoords;
	varying vec4 v_shadowColor;
	varying vec4 v_normalColor;
	varying vec4 v_highlightColor;
	varying vec2 v_ranges;

	void main() {
		v_color = a_color;
		v_texCoords = a_texCoords;
		v_shadowColor = u_shadowColor;
		v_normalColor = u_normalColor;
		v_highlightColor = u_highlightColor;
		v_ranges = u_ranges;
		vec4 worldPosition = vec4(a_position, 0.0, 1.0);
		vec4 normalizedPosition = u_transform * worldPosition;
		gl_Position = normalizedPosition;
	})";

const char* verbFgtShader = R"(#version 110
	varying vec4 v_color;
	varying vec2 v_texCoords;
	varying vec4 v_shadowColor;
	varying vec4 v_normalColor;
	varying vec4 v_highlightColor;
	varying vec2 v_ranges;
	uniform sampler2D u_texture;

	void main() {
		float shadows = v_ranges.x;
		float highlights = v_ranges.y;
		vec4 texColor = texture2D(u_texture, v_texCoords);
		if (texColor.g <= shadows) {
			texColor *= v_shadowColor;
		} else if (texColor.g >= highlights) {
			texColor *= v_highlightColor;
		} else {
			texColor *= v_normalColor;
		}
		texColor *= v_color;
		gl_FragColor = texColor;
	})";
	init(verbVtxShader, verbFgtShader);

	GL_CALL(_rangesLoc = glGetUniformLocation(program, "u_ranges"));
	GL_CALL(_shadowColorLoc = glGetUniformLocation(program, "u_shadowColor"));
	GL_CALL(_normalColorLoc = glGetUniformLocation(program, "u_normalColor"));
	GL_CALL(_highlightColorLoc = glGetUniformLocation(program, "u_highlightColor"));
}

HudShader::~HudShader() {}

void HudShader::applyUniforms() {
	GL_CALL(glUniform2f(_rangesLoc, 0.8f, 0.8f));
	GL_CALL(glUniform4f(_shadowColorLoc, _shadowColor.rgba.r, _shadowColor.rgba.g, _shadowColor.rgba.b, _shadowColor.rgba.a));
	GL_CALL(glUniform4f(_normalColorLoc, _normalColor.rgba.r, _normalColor.rgba.g, _normalColor.rgba.b, _normalColor.rgba.a));
	GL_CALL(glUniform4f(_highlightColorLoc, _highlightColor.rgba.r, _highlightColor.rgba.g, _highlightColor.rgba.b, _highlightColor.rgba.a));
}

Hud::Hud() : Node("hud") {
	_zOrder = 100;
	for (int i = 0; i < NUMACTORS; i++) {
		ActorSlot *slot = &_actorSlots[i];
		slot->actor = nullptr;
	}
}

ActorSlot *Hud::actorSlot(Object *actor) {
	for (int i = 0; i < NUMACTORS; i++) {
		ActorSlot *slot = &_actorSlots[i];
		if (slot->actor == actor) {
			return slot;
		}
	}
	return nullptr;
}

void Hud::drawSprite(const SpriteSheetFrame &sf, Texture *texture, Color color, Math::Matrix4 trsf) {
	Math::Vector3d pos(sf.spriteSourceSize.left, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY(), 0.f);
	trsf.translate(pos);
	g_engine->getGfx().drawSprite(sf.frame, *texture, color, trsf);
}

void Hud::drawCore(Math::Matrix4 trsf) {
	ActorSlot *slot = this->actorSlot(_actor);
	if (!slot)
		return;

	// draw HUD background
	SpriteSheet *gameSheet = g_engine->_resManager.spriteSheet("GameSheet");
	bool classic = ConfMan.getBool("hudSentence");
	const SpriteSheetFrame &backingFrame = gameSheet->frameTable[classic ? "ui_backing_tall" : "ui_backing"];
	Texture *gameTexture = g_engine->_resManager.texture(gameSheet->meta.image);
	float alpha = 0.33f; // prefs(UiBackingAlpha);
	g_engine->getGfx().drawSprite(backingFrame.frame, *gameTexture, Color(0, 0, 0, alpha), trsf);

	bool verbHlt = ConfMan.getBool("invertVerbHighlight");
	Color verbHighlight = verbHlt ? Color() : slot->verbUiColors.verbHighlight;
	Color verbColor = verbHlt ? slot->verbUiColors.verbHighlight : Color();

	// draw actor's verbs
	SpriteSheet *verbSheet = g_engine->_resManager.spriteSheet("VerbSheet");
	Texture *verbTexture = g_engine->_resManager.texture(verbSheet->meta.image);
	// let lang = prefs(Lang);
	Common::String lang = "en";
	bool retroVerbs = ConfMan.getBool("retroVerbs");
	Common::String verbSuffix = retroVerbs ? "_retro" : "";

	Shader *saveShader = g_engine->getGfx().getShader();
	// g_engine->getGfx().use(&_shader);
	// _shader._shadowColor = slot->verbUiColors.verbNormalTint;
	// _shader._normalColor = slot->verbUiColors.verbHighlight;
	// _shader._highlightColor = slot->verbUiColors.verbHighlightTint;

	bool isOver = false;
	for (int i = 1; i < 22; i++) {
		const Verb &verb = slot->verbs[i];
		if (verb.image.size() > 0) {
			const SpriteSheetFrame &verbFrame = verbSheet->frameTable[Common::String::format("%s%s_%s", verb.image.c_str(), verbSuffix.c_str(), lang.c_str())];
			bool over = verbFrame.spriteSourceSize.contains(_mousePos.getX(), _mousePos.getY());
			if (over)
				isOver = true;
			Color color = (over || (verb.id.id == _defaultVerbId)) ? verbHighlight : verbColor;
			if (_mouseClick && over) {
				_verb = verb;
			}
			drawSprite(verbFrame, verbTexture, color, trsf);
		}
	}
	// g_engine->getGfx().use(saveShader);
	_over = isOver;
}

void Hud::update(Math::Vector2d pos, Object *hotspot, bool mouseClick) {
	_mousePos = Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
	_defaultVerbId = !hotspot ? 0 : hotspot->defaultVerbId();
	_mouseClick = mouseClick;
}

} // namespace Twp
