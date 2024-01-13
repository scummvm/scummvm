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

	const char* bwShader = R"(#version 110
varying vec2 v_texCoords;
varying vec4 v_color;
uniform sampler2D u_texture;
void main() {
	vec4 texColor = texture2D(u_texture, v_texCoords);
	vec4 col = v_color * texColor;
	float gray = dot(col.xyz, vec3(0.299, 0.587, 0.114));
	gl_FragColor = vec4(gray, gray, gray, col.a);
})";

const char* ghostShader = R"(#version 110
// Work in progress ghost shader.. Too over the top at the moment, it'll make you sick.

varying vec4 v_color;
varying vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float iGlobalTime;
uniform float iFade;
uniform float wobbleIntensity;
uniform vec3 shadows;
uniform vec3 midtones;
uniform vec3 highlights;

const float speed = 0.1;
const float emboss = 0.70;
const float intensity = 0.6;
const int steps = 4;
const float frequency = 9.0;

float colour(vec2 coord) {
	float col = 0.0;

	float timeSpeed = iGlobalTime * speed;
	vec2 adjc = coord;
	adjc.x += timeSpeed; // adjc0.x += fcos*timeSpeed;
	float sum0 = cos(adjc.x * frequency) * intensity;
	col += sum0;

	adjc = coord;
	float fcos = 0.623489797;
	float fsin = 0.781831503;
	adjc.x += fcos * timeSpeed;
	adjc.y -= fsin * timeSpeed;
	float sum1 = cos((adjc.x * fcos - adjc.y * fsin) * frequency) * intensity;
	col += sum1;

	adjc = coord;
	fcos = -0.900968909;
	fsin = 0.433883607;
	adjc.x += fcos * timeSpeed;
	adjc.y -= fsin * timeSpeed;
	col += cos((adjc.x * fcos - adjc.y * fsin) * frequency) * intensity;

	// do same in reverse.
	col += sum1;
	col += sum0;

	return cos(col);
}

vec3 rgb2hsv(vec3 c) {
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float rand(vec2 Input) {
	float dt = dot(Input, vec2(12.9898, 78.233));
	float sn = mod(dt, 3.14);
	return fract(sin(sn) * 43758.5453123);
}

float color_balance(float col, float l, vec3 change) {
	// NOTE: change = (shadow, midtones, highlights)

	float sup = 83.0;  // shadow upper bounds
	float mup = 166.0; // midtones upper bounds

	float value = col * 255.0;
	l = l * 100.0;

	if (l < sup) {
		// shadow
		float f = (sup - l + 1.0) / (sup + 1.0);
		value += change.x * f;
	} else if (l < mup) {
		// midtone
		float mrange = (mup - sup) / 2.0;
		float mid = mrange + sup;
		float diff = mid - l;
		diff = -diff;
		if (diff < 0.0) {
			float f = 1.0 - (diff + 1.0) / (mrange + 1.0);
			value += change.y * f;
		}
	} else {
		// highlight
		float f = (l - mup + 1.0) / (255.0 - mup + 1.0);
		value += change.z * f;
	}
	value = min(255.0, max(0.0, value));
	return value / 255.0;
}

vec2 rgb2cv(vec3 RGB) {
	vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0 / 3.0) : vec4(RGB.gb, 0.0, -1.0 / 3.0);
	vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	return vec2(C, Q.x);
}

float rgbToLuminance(vec3 RGB) {
	float cMax = max(max(RGB.x, RGB.y), RGB.z);
	float cMin = min(min(RGB.x, RGB.y), RGB.z);

	return (cMax + cMin) * 0.5;
}

void main(void) {
	vec2 c1 = v_texCoords;
	float cc1 = colour(c1);
	vec2 offset;

	c1.x += (0.001 * wobbleIntensity); // appx 12 pixels horizontal
	offset.x = emboss * (cc1 - colour(c1));

	c1.x = v_texCoords.x;
	c1.y += (0.002 * wobbleIntensity); // appx 12 pixels verticle
	offset.y = emboss * (cc1 - colour(c1));

	// TODO: The effect should be centered around Franklyns position in the room, not the center
	// if ( emitFromCenter == 1)
	{
		vec2 center = vec2(0.5, 0.5);
		float distToCenter = distance(center, v_texCoords);
		offset *= distToCenter * 2.0;
	}

	c1 = v_texCoords;
	c1 += (offset * iFade);

	vec3 col = vec3(0, 0, 0);
	if (c1.x >= 0.0 && c1.x < (1.0 - 0.003125)) {
		col = texture2D(u_texture, c1).rgb;
		float intensity = rgbToLuminance(col); //(col.r + col.g + col.b) * 0.333333333;

		// Exponential Shadows
		float shadowsBleed = 1.0 - intensity;
		shadowsBleed *= shadowsBleed;
		shadowsBleed *= shadowsBleed;

		// Exponential midtones
		float midtonesBleed = 1.0 - abs(-1.0 + intensity * 2.0);
		midtonesBleed *= midtonesBleed;
		midtonesBleed *= midtonesBleed;

		// Exponential Hilights
		float hilightsBleed = intensity;
		hilightsBleed *= hilightsBleed;
		hilightsBleed *= hilightsBleed;

		vec3 colorization = col.rgb + shadows * shadowsBleed +
							midtones * midtonesBleed +
							highlights * hilightsBleed;

		colorization = mix(col, colorization, iFade);

		// col = lerp(col, colorization, _Amount);
		col = min(vec3(1.0), max(vec3(0.0), colorization));
	}
	gl_FragColor = v_color * vec4(col, texture2D(u_texture, c1).a);
})";

const char* sepiaShader = R"(#version 110

varying vec4 v_color;
varying vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float sepiaFlicker;
uniform float RandomValue[5];
uniform float TimeLapse;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 1.0) * x); }
float snoise(vec2 v) {
	const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
						0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
						-0.577350269189626, // -1.0 + 2.0 * C.x
						0.024390243902439); // 1.0 / 41.0

	// First corner
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);

	// Other corners
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));

	vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = m * m;
	m = m * m;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

	// Compute final noise value at P
	vec3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

void main(void) {
	const float RADIUS = 0.75;
	const float SOFTNESS = 0.45;
	const float ScratchValue = 0.3;

	vec4 texColor = texture2D(u_texture, v_texCoords);
	vec4 col = v_color * texColor;
	float gray = dot(col.rgb, vec3(0.299, 0.587, 0.114));
	vec2 dist = vec2(v_texCoords.x - 0.5, v_texCoords.y - 0.5);
	vec3 sepiaColor = vec3(gray) * vec3(0.9, 0.8, 0.6); // vec3(1.2, 1.0, 0.8);
	float len = dot(dist, dist);
	float vignette = smoothstep(RADIUS, RADIUS - SOFTNESS, len);
	//   float vignette = (1.0 - len);
	col.rgb = mix(col.rgb, sepiaColor, 0.80) * vignette * sepiaFlicker; // Want to keep SOME of the original color, so only use 80% sepia
	//   col.rgb = vec3( vignette ) * sepiaFlicker;

	for (int i = 0; i < 1; i++) {
		if (RandomValue[i] < ScratchValue) {
			// Pick a random spot to show scratches
			float dist = 1.0 / ScratchValue;
			float d = distance(v_texCoords, vec2(RandomValue[i] * dist, RandomValue[i] * dist));
			if (d < 0.4) {
				// Generate the scratch
				float xPeriod = 8.0;
				float yPeriod = 1.0;
				float pi = 3.141592;
				float phase = TimeLapse;
				float turbulence = snoise(v_texCoords * 2.5);
				float vScratch = 0.5 + (sin(((v_texCoords.x * xPeriod + v_texCoords.y * yPeriod + turbulence)) * pi + phase) * 0.5);
				vScratch = clamp((vScratch * 10000.0) + 0.35, 0.0, 1.0);

				col.rgb *= vScratch;
			}
		}
	}
	gl_FragColor = col;
})";

FadeShader::FadeShader() {
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
	if (effect == RoomEffect::Sepia) {
		Shader *shader = g_engine->getGfx().getShader();
		shader->setUniform("RandomValue", randomValue);
		shader->setUniform("TimeLapse", timeLapse);
	}
	//   } else if (effect == RoomEffect::Vhs) {
	//     Shader* shader = g_engine->getGfx().getShader();
	//     shader->setUniform("iGlobalTime", iGlobalTime);
	//     shader->setUniform("iNoiseThreshold", iNoiseThreshold);
	//   } else
	else if (effect == RoomEffect::Ghost) {
		Shader *shader = g_engine->getGfx().getShader();
		shader->setUniform("iGlobalTime", iGlobalTime);
		shader->setUniform("iFade", iFade);
		shader->setUniform("wobbleIntensity", wobbleIntensity);
		shader->setUniform("shadows", shadows);
		shader->setUniform("midtones", midtones);
		shader->setUniform("highlights", highlights);
	}
}

} // namespace Twp
