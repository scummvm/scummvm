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

#include "common/config-manager.h"
#include "twp/twp.h"
#include "twp/hud.h"
#include "twp/resmanager.h"

namespace Twp {

Verb::Verb() = default;

Verb::Verb(VerbId verbId, const Common::String &img, const Common::String &f, const Common::String &t, const Common::String &k, int fl)
	: id(verbId), image(img), fun(f), text(t), key(k), flags(fl) {
}

VerbUiColors::VerbUiColors() = default;

VerbUiColors::VerbUiColors(const Color &s, const Color &vbNormal, const Color &vbNormalTint, const Color &vbHiglight, const Color &vbHiglightTint, const Color &dlgNormal, const Color &dlgHighlt, const Color &invFrame, const Color &inventoryBack, const Color &retroNml, const Color &retroHighlt)
	: sentence(s), verbNormal(vbNormal), verbNormalTint(vbNormalTint), verbHighlight(vbHiglight), verbHighlightTint(vbHiglightTint), dialogNormal(dlgNormal), dialogHighlight(dlgHighlt), inventoryFrame(invFrame), inventoryBackground(inventoryBack), retroNormal(retroNml), retroHighlight(retroHighlt) {
}

ActorSlot::ActorSlot() = default;

HudShader::HudShader() = default;

void HudShader::init() {
	const char *v_source = R"(
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
		gl_Position = u_transform * vec4(a_position, 0.0, 1.0);

		v_color = a_color;
		v_texCoords = a_texCoords;
		v_shadowColor = u_shadowColor;
		v_normalColor = u_normalColor;
		v_highlightColor = u_highlightColor;
		v_ranges = u_ranges;
	})";

	const char *f_source = R"(
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
	Shader::init("hud", v_source, f_source);
}

HudShader::~HudShader() = default;

void HudShader::applyUniforms() {
	setUniform("u_ranges", Math::Vector2d(0.8f, 0.8f));
	setUniform4("u_shadowColor", _shadowColor);
	setUniform4("u_normalColor", _normalColor);
	setUniform4("u_highlightColor", _highlightColor);
}

Hud::Hud() : Node("hud") {
	_zOrder = 100;
	for (auto &_actorSlot : _actorSlots) {
		ActorSlot *slot = &_actorSlot;
		slot->actor = nullptr;
	}
}

void Hud::init() {
	_shader.init();
}

ActorSlot *Hud::actorSlot(Common::SharedPtr<Object> actor) {
	for (auto &_actorSlot : _actorSlots) {
		ActorSlot *slot = &_actorSlot;
		if (slot->actor == actor) {
			return slot;
		}
	}
	return nullptr;
}

void Hud::drawSprite(const SpriteSheetFrame &sf, Texture *texture, const Color &color, const Math::Matrix4 &t) {
	Math::Matrix4 trsf(t);
	Math::Vector3d pos(sf.spriteSourceSize.left, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY(), 0.f);
	trsf.translate(pos);
	g_twp->getGfx().drawSprite(sf.frame, *texture, color, trsf);
}

void Hud::drawCore(const Math::Matrix4 &trsf) {
	ActorSlot *slot = actorSlot(_actor);
	if (!slot)
		return;

	// draw HUD background
	SpriteSheet *gameSheet = g_twp->_resManager->spriteSheet("GameSheet");
	bool classic = ConfMan.getBool("hudSentence");
	const SpriteSheetFrame &backingFrame = gameSheet->getFrame(classic ? "ui_backing_tall" : "ui_backing");
	Texture *gameTexture = g_twp->_resManager->texture(gameSheet->meta.image);
	float alpha = 0.33f; // prefs(UiBackingAlpha);
	g_twp->getGfx().drawSprite(backingFrame.frame, *gameTexture, Color(0, 0, 0, alpha * getAlpha()), trsf);

	bool verbHlt = ConfMan.getBool("invertVerbHighlight");
	Color verbHighlight = verbHlt ? Color() : slot->verbUiColors.verbHighlight;
	Color verbColor = verbHlt ? slot->verbUiColors.verbHighlight : Color();

	// draw actor's verbs
	SpriteSheet *verbSheet = g_twp->_resManager->spriteSheet("VerbSheet");
	Texture *verbTexture = g_twp->_resManager->texture(verbSheet->meta.image);
	Common::String lang = ConfMan.get("language");
	bool retroVerbs = ConfMan.getBool("retroVerbs");
	Common::String verbSuffix = retroVerbs ? "_retro" : "";

	Shader *saveShader = g_twp->getGfx().getShader();
	g_twp->getGfx().use(&_shader);
	_shader._shadowColor = slot->verbUiColors.verbNormalTint;
	_shader._normalColor = slot->verbUiColors.verbHighlight;
	_shader._highlightColor = slot->verbUiColors.verbHighlightTint;

	bool isOver = false;
	for (int i = 1; i < MAX_VERBS; i++) {
		const Verb &verb = slot->verbs[i];
		if (verb.image.size() > 0) {
			const SpriteSheetFrame &verbFrame = verbSheet->getFrame(Common::String::format("%s%s_%s", verb.image.c_str(), verbSuffix.c_str(), lang.c_str()));
			bool over = verbFrame.spriteSourceSize.contains(_mousePos.getX(), _mousePos.getY());
			isOver |= over;
			Color color = (over || (verb.id.id == _defaultVerbId)) ? verbHighlight : verbColor;
			if (_mouseClick && over) {
				_verb = verb;
			}
			drawSprite(verbFrame, verbTexture, Color::withAlpha(color, getAlpha()), trsf);
		}
	}
	g_twp->getGfx().use(saveShader);
	_over = isOver;
}

void Hud::update(float elapsed, const Math::Vector2d &pos, Common::SharedPtr<Object> hotspot, bool mouseClick) {
	if (_active) {
		_mousePos = Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
		_defaultVerbId = !hotspot ? 0 : hotspot->defaultVerbId();
		_mouseClick = mouseClick;
	}

	_fadeTime += elapsed;

	if (_fadeTime > 2.f) {
		_fadeTime = 2.f;
		if (!_fadeIn) {
			setVisible(false);
		}
	}

	if (_fadeIn) {
		float alpha = MIN(_fadeTime, 2.0f) / 2.0f;
		setAlpha(alpha);
	} else {
		float alpha = MAX(2.0f - _fadeTime, 0.0f) / 2.0f;
		setAlpha(alpha);
	}
}

void Hud::setVisible(bool visible) {
	if (_fadeIn != visible) {
		_fadeIn = visible;
		if (visible) {
			Node::setVisible(visible);
		}
		_fadeTime = 0;
	}
}

} // namespace Twp
