#include "twp/lighting.h"
#include "graphics/opengl/debug.h"
#include "graphics/opengl/system_headers.h"

namespace Twp {

Lighting::Lighting() {
	const char *vshader = R"(#version 110
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

	const char *fshader = R"(#version 110
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
	init(vshader, fshader);

	GL_CALL(_contentSizeLoc = glGetUniformLocation(program, "u_contentSize"));
	GL_CALL(_spriteOffsetLoc = glGetUniformLocation(program, "u_spriteOffset"));
	GL_CALL(_spritePosInSheetLoc = glGetUniformLocation(program, "u_spritePosInSheet"));
	GL_CALL(_spriteSizeRelToSheetLoc = glGetUniformLocation(program, "u_spriteSizeRelToSheet"));
	GL_CALL(_numberLightsLoc = glGetUniformLocation(program, "u_numberLights"));
	GL_CALL(_ambientColorLoc = glGetUniformLocation(program, "u_ambientColor"));
}

Lighting::~Lighting() {}

void Lighting::setSpriteSheetFrame(const SpriteSheetFrame& frame, const Texture& texture) {
	_contentSize = frame.sourceSize;
	//_spriteOffset = {-frame.frame.width() / 2.f, -frame.frame.height() / 2.f};
	_spriteOffset = {0, (float)frame.frame.height()};
	_spritePosInSheet = {(float)(frame.frame.left) / texture.width, (float)(frame.frame.top) / texture.height};
	_spriteSizeRelToSheet = {(float)(frame.sourceSize.getX()) / texture.width, (float)(frame.sourceSize.getY()) / texture.height};
}

void Lighting::applyUniforms() {
	GL_CALL(glUniform1i(_numberLightsLoc, 0));
	float ambientColor[] = {1,1,1};
	GL_CALL(glUniform3fv(_ambientColorLoc, 1, ambientColor));
	GL_CALL(glUniform2fv(_contentSizeLoc, 1, _contentSize.getData()));
	GL_CALL(glUniform2fv(_spriteOffsetLoc, 1, _spriteOffset.getData()));
	GL_CALL(glUniform2fv(_spritePosInSheetLoc, 1, _spritePosInSheet.getData()));
	GL_CALL(glUniform2fv(_spriteSizeRelToSheetLoc, 1, _spriteSizeRelToSheet.getData()));
}

} // namespace Twp
