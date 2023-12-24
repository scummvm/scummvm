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

#include "twp/shaders.h"
#include "twp/twp.h"
#include "graphics/opengl/debug.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

FadeShader::FadeShader() {
	const char *vsrc = R"(#version 110
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
	const char *fadeShader = R"(#version 110
#ifdef GL_ES
precision highp float;
#endif

	varying vec4 v_color;
	varying vec2 v_texCoords;
	uniform sampler2D u_texture;
	uniform sampler2D u_texture2;

	uniform float u_timer;
	uniform float u_fade;
	uniform int u_fadeToSep;
	uniform float u_movement;

	void main() {
		const float RADIUS = 0.75;
		const float SOFTNESS = 0.45;
		const float ScratchValue = 0.3;
		vec2 uv = v_texCoords;
		float pi2 = (3.142 * 2.0);
		float intervals = 4.0;
		uv.x += sin((u_timer + uv.y) * (pi2 * intervals)) * u_movement;
		vec4 texColor1 = v_color * texture2D(u_texture, uv);
		vec4 texColor2 = v_color * texture2D(u_texture2, uv);
		if (u_fadeToSep != 0) {
			float gray = dot(texColor2.rgb, vec3(0.299, 0.587, 0.114));
			vec2 dist = vec2(uv.x - 0.5, uv.y - 0.5);
			vec3 sepiaColor = vec3(gray, gray, gray) * vec3(0.9, 0.8, 0.6);
			float len = dot(dist, dist);
			float vignette = smoothstep(RADIUS, RADIUS - SOFTNESS, len);
			vec3 sep = mix(texColor2.rgb, sepiaColor, 0.80) * vignette;
			gl_FragColor.rgb = (texColor1.rgb * (1.0 - u_fade)) + (sep * u_fade);
		} else {
			gl_FragColor.rgb = (texColor1.rgb * (1.0 - u_fade)) + (texColor2.rgb * u_fade);
		}
		gl_FragColor.a = 1.0;
	})";
	init(vsrc, fadeShader);

	GL_CALL(_textureLoc[0] = glGetUniformLocation(program, "u_texture"));
	GL_CALL(_textureLoc[1] = glGetUniformLocation(program, "u_texture2"));
	GL_CALL(_timerLoc = glGetUniformLocation(program, "u_timer"));
	GL_CALL(_fadeLoc = glGetUniformLocation(program, "u_fade"));
	GL_CALL(_fadeToSepLoc = glGetUniformLocation(program, "u_fadeToSep"));
	GL_CALL(_movementLoc = glGetUniformLocation(program, "u_movement"));
}

FadeShader::~FadeShader() {}

void FadeShader::applyUniforms() {
	float movement = (sin(M_PI * _fade) * _movement);
	GL_CALL(glUniform1f(_timerLoc, _elapsed));
	GL_CALL(glUniform1f(_fadeLoc, _fade));
	GL_CALL(glUniform1i(_fadeToSepLoc, _fadeToSepia ? 1 : 0));
	GL_CALL(glUniform1f(_movementLoc, movement));
}

int FadeShader::getNumTextures() { return 2; }

int FadeShader::getTexture(int index) {
	switch (index) {
	case 0:
		return _texture1->id;
	case 1:
		return _texture2->id;
	}
	return 0;
}

int FadeShader::getTextureLoc(int index) { return _textureLoc[index]; }

void ShaderParams::updateShader() {
	// TODO
	//   if (effect == RoomEffect::Sepia) {
	//     Shader* shader = g_engine->getGfx().getShader();
	//     shader->setUniform("RandomValue", randomValue);
	//     shader->setUniform("TimeLapse", timeLapse);
	//   } else if (effect == RoomEffect::Vhs) {
	//     Shader* shader = g_engine->getGfx().getShader();
	//     shader->setUniform("iGlobalTime", iGlobalTime);
	//     shader->setUniform("iNoiseThreshold", iNoiseThreshold);
	//   } else if (effect == RoomEffect::Ghost) {
	//     Shader* shader = g_engine->getGfx().getShader();
	//     shader->setUniform("iGlobalTime", iGlobalTime);
	//     shader->setUniform("iFade", iFade);
	//     shader->setUniform("wobbleIntensity", wobbleIntensity);
	//     shader->setUniform("shadows", shadows);
	//     shader->setUniform("midtones", midtones);
	//     shader->setUniform("highlights", highlights);
	//   }
}

} // namespace Twp
