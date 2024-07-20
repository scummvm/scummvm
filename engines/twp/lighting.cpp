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

#include "math/utils.h"
#include "twp/lighting.h"
#include "twp/room.h"
#include "twp/twp.h"
#include "twp/walkboxnode.h"

namespace Twp {

static const char *fshader = R"(
#ifdef GL_ES
	precision highp float;
#endif

varying vec2 v_texCoords;
varying vec4 v_color;

uniform sampler2D u_texture;

uniform vec2 u_contentSize;
uniform vec3 u_ambientColor;
uniform vec2 u_spritePosInSheet;
uniform vec2 u_spriteSizeRelToSheet;
uniform vec2 u_spriteOffset;

uniform int u_numberLights;
uniform vec3 u_lightPos[50];
uniform vec3 u_lightColor[50];
uniform float u_brightness[50];
uniform float u_cutoffRadius[50];
uniform float u_halfRadius[50];
uniform float u_coneCosineHalfConeAngle[50];
uniform float u_coneFalloff[50];
uniform vec2 u_coneDirection[50];

void main(void) {
	vec4 texColor = texture2D(u_texture, v_texCoords);

	vec2 spriteTexCoord = (v_texCoords - u_spritePosInSheet) / u_spriteSizeRelToSheet; // [0..1]
	vec2 pixelPos = spriteTexCoord * u_contentSize + u_spriteOffset;                   // [0..origSize]
	vec2 curPixelPosInLocalSpace = vec2(pixelPos.x, -pixelPos.y);

	vec3 diffuse = vec3(0, 0, 0);
	for (int i = 0; i < u_numberLights; i++) {
		vec2 lightVec = curPixelPosInLocalSpace.xy - u_lightPos[i].xy;
		float coneValue = dot(normalize(-lightVec), u_coneDirection[i]);
		if (coneValue >= u_coneCosineHalfConeAngle[i]) {
			float intercept = u_cutoffRadius[i] * u_halfRadius[i];
			float dx_1 = 0.5 / intercept;
			float dx_2 = 0.5 / (u_cutoffRadius[i] - intercept);
			float offset = 0.5 + intercept * dx_2;

			float lightDist = length(lightVec);
			float falloffTermNear = clamp((1.0 - lightDist * dx_1), 0.0, 1.0);
			float falloffTermFar = clamp((offset - lightDist * dx_2), 0.0, 1.0);
			float falloffSelect = step(intercept, lightDist);
			float falloffTerm = (1.0 - falloffSelect) * falloffTermNear + falloffSelect * falloffTermFar;
			float spotLight = u_brightness[i] * falloffTerm;

			vec3 ltdiffuse = vec3(u_brightness[i] * falloffTerm) * u_lightColor[i];

			float coneRange = 1.0 - u_coneCosineHalfConeAngle[i];
			float halfConeRange = coneRange * u_coneFalloff[i];
			float conePos = 1.0 - coneValue;
			float coneFalloff = 1.0;
			if (conePos > halfConeRange) {
				coneFalloff = 1.0 - ((conePos - halfConeRange) / (coneRange - halfConeRange));
			}

			diffuse += ltdiffuse * coneFalloff;
			;
		}
	}
	vec4 finalCol = texColor * v_color;
	vec3 finalLight = (diffuse + u_ambientColor);
	finalLight = min(finalLight, vec3(1, 1, 1));
	gl_FragColor = vec4(finalCol.rgb * finalLight, finalCol.a);
})";

static const char *debug_fshader = R"(
varying vec2 v_texCoords;
varying vec4 v_color;

uniform sampler2D u_texture;

uniform vec2 u_contentSize;
uniform vec3 u_ambientColor;
uniform vec2 u_spritePosInSheet;
uniform vec2 u_spriteSizeRelToSheet;
uniform vec2 u_spriteOffset;

uniform int u_numberLights;
uniform vec3 u_lightPos[50];
uniform vec3 u_lightColor[50];
uniform float u_brightness[50];
uniform float u_cutoffRadius[50];
uniform float u_halfRadius[50];
uniform float u_coneCosineHalfConeAngle[50];
uniform float u_coneFalloff[50];
uniform vec2 u_coneDirection[50];

void main(void) {
	vec4 texColor = texture2D(u_texture, v_texCoords);

	vec2 spriteTexCoord = (v_texCoords - u_spritePosInSheet) / u_spriteSizeRelToSheet; // [0..1]
	vec2 pixelPos = spriteTexCoord * u_contentSize + u_spriteOffset;                   // [0..origSize]
	vec2 curPixelPosInLocalSpace = vec2(pixelPos.x, -pixelPos.y);

	vec3 diffuse = vec3(0, 0, 0);
	for (int i = 0; i < u_numberLights; i++) {
		vec2 lightVec = curPixelPosInLocalSpace.xy - u_lightPos[i].xy;
		float coneValue = dot(normalize(-lightVec), u_coneDirection[i]);
		if (coneValue >= u_coneCosineHalfConeAngle[i]) {
			float intercept = u_cutoffRadius[i] * u_halfRadius[i];
			float dx_1 = 0.5 / intercept;
			float dx_2 = 0.5 / (u_cutoffRadius[i] - intercept);
			float offset = 0.5 + intercept * dx_2;

			float lightDist = length(lightVec);
			float falloffTermNear = clamp((1.0 - lightDist * dx_1), 0.0, 1.0);
			float falloffTermFar = clamp((offset - lightDist * dx_2), 0.0, 1.0);
			float falloffSelect = step(intercept, lightDist);
			float falloffTerm = (1.0 - falloffSelect) * falloffTermNear + falloffSelect * falloffTermFar;
			float spotLight = u_brightness[i] * falloffTerm;

			vec3 ltdiffuse = vec3(u_brightness[i] * falloffTerm) * u_lightColor[i];

			float coneRange = 1.0 - u_coneCosineHalfConeAngle[i];
			float halfConeRange = coneRange * u_coneFalloff[i];
			float conePos = 1.0 - coneValue;
			float coneFalloff = 1.0;
			if (conePos > halfConeRange) {
				coneFalloff = 1.0 - ((conePos - halfConeRange) / (coneRange - halfConeRange));
			}

			diffuse += ltdiffuse * coneFalloff;
			;
		}
	}
	vec3 finalLight = (diffuse);
	vec4 finalCol = texColor * v_color;
	finalCol.rgb = finalCol.rgb * u_ambientColor;
	gl_FragColor = vec4(finalCol.rgb + diffuse, finalCol.a);
})";

static const char *vshader = R"(
uniform mat4 u_transform;
attribute vec2 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoords;
varying vec4 v_color;
varying vec2 v_texCoords;
void main() {
	gl_Position = u_transform * vec4(a_position, 0.0, 1.0);
	v_color = a_color;
	v_texCoords = a_texCoords;
})";

Lighting::Lighting() {
	init("lighting", vshader, fshader);
}

Lighting::~Lighting() = default;

void Lighting::setSpriteOffset(const Math::Vector2d &offset) {
	_spriteOffset = offset;
}

void Lighting::setSpriteSheetFrame(const SpriteSheetFrame &frame, const Texture &texture, bool flipX) {
	_contentSize = flipX ? Math::Vector2d(frame.sourceSize.getX(), frame.sourceSize.getY()) : frame.sourceSize;
	_spritePosInSheet = {(float)(frame.frame.left) / texture.width, (float)(frame.frame.top) / texture.height};
	_spriteSizeRelToSheet = flipX ? Math::Vector2d((float)(-frame.sourceSize.getX()) / texture.width, (float)(frame.sourceSize.getY()) / texture.height) : Math::Vector2d((float)(frame.sourceSize.getX()) / texture.width, (float)(frame.sourceSize.getY()) / texture.height);
}

void Lighting::update(const Lights &lights) {
	if (_currentDebug != _debug) {
		init("lighting", vshader, _debug ? debug_fshader : fshader);
		_currentDebug = _debug;
		g_twp->_lightingNode->setVisible(_debug);
	}
	_ambientLight = lights._ambientLight;
	u_numberLights = 0;
	for (int i = 0; i < MIN(MAX_LIGHTS, lights._numLights); ++i) {
		const Light &light = lights._lights[i];
		if (!light.on)
			continue;
		const float direction = light.coneDirection - 90.f;
		u_lightPos[u_numberLights * 3 + 0] = light.pos.getX();
		u_lightPos[u_numberLights * 3 + 1] = light.pos.getY();
		u_lightPos[u_numberLights * 3 + 2] = 1.f;
		u_coneDirection[u_numberLights * 2 + 0] = cos(Math::deg2rad(direction));
		u_coneDirection[u_numberLights * 2 + 1] = sin(Math::deg2rad(direction));
		u_coneCosineHalfConeAngle[u_numberLights] = cos(Math::deg2rad(light.coneAngle / 2.f));
		u_coneFalloff[u_numberLights] = light.coneFalloff;
		u_lightColor[u_numberLights * 3 + 0] = light.color.rgba.r;
		u_lightColor[u_numberLights * 3 + 1] = light.color.rgba.g;
		u_lightColor[u_numberLights * 3 + 2] = light.color.rgba.b;
		u_brightness[u_numberLights] = light.brightness;
		u_cutoffRadius[u_numberLights] = MAX(1.0f, light.cutOffRadius);
		u_halfRadius[u_numberLights] = MAX(0.01f, MIN(0.99f, light.halfRadius));
		u_numberLights++;
	}
}

void Lighting::applyUniforms() {
	setUniform3("u_ambientColor", _ambientLight);
	setUniform("u_numberLights", u_numberLights);

	if (u_numberLights > 0) {
		setUniform3("u_lightPos", u_lightPos, MAX_LIGHTS);
		setUniform2("u_coneDirection", u_coneDirection, MAX_LIGHTS);
		setUniform("u_coneCosineHalfConeAngle", u_coneCosineHalfConeAngle, MAX_LIGHTS);
		setUniform("u_coneFalloff", u_coneFalloff, MAX_LIGHTS);
		setUniform3("u_lightColor", u_lightColor, MAX_LIGHTS);
		setUniform("u_brightness", u_brightness, MAX_LIGHTS);
		setUniform("u_cutoffRadius", u_cutoffRadius, MAX_LIGHTS);
		setUniform("u_halfRadius", u_halfRadius, MAX_LIGHTS);
	}

	setUniform("u_contentSize", _contentSize);
	setUniform("u_spriteOffset", _spriteOffset);
	setUniform("u_spritePosInSheet", _spritePosInSheet);
	setUniform("u_spriteSizeRelToSheet", _spriteSizeRelToSheet);
}

} // namespace Twp
