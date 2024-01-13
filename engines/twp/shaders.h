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

#ifndef TWP_SHADERS_H
#define TWP_SHADERS_H

#include "twp/room.h"
#include "math/vector3d.h"

namespace Twp {

extern const char* vsrc;
extern const char* bwShader;
extern const char* ghostShader;
extern const char* sepiaShader;

struct ShaderParams {
    RoomEffect effect;
    float sepiaFlicker = 1.f;
    float randomValue[5];
    float timeLapse;
    float iGlobalTime;
    float iNoiseThreshold = 1.f;
    float iFade = 1.f;
    float wobbleIntensity = 1.f;
    Color shadows = Color(-0.3f, 0.f, 0.f);
    Color midtones = Color(-0.2f, 0.f, 0.1f);
    Color highlights = Color(0.f, 0.f, 0.2f);

	void updateShader();
};


enum class FadeEffect {
	None,
	In,
	Out,
	Wobble
};

class FadeShader: public Shader {
public:
	FadeShader();
	virtual ~FadeShader() override;

	virtual int getNumTextures() override;
	virtual int getTexture(int index) override;
	virtual int getTextureLoc(int index) override;

private:
	virtual void applyUniforms() final;

public:
	FadeEffect _effect = FadeEffect::None;
	Room *_room = nullptr;
	Texture *_texture1 = nullptr;
	Texture *_texture2 = nullptr;
	Math::Vector2d _cameraPos;
	float _duration = 0.f;
	float _elapsed = 0.f;
	float _movement = 0.f;		// movement for wobble effect
	float _fade = 0.f;			// fade value between [0.f,1.f]
	bool _fadeToSepia = false;	// true to fade to sepia

private:
	int _textureLoc[2];
	int _timerLoc;
	int _fadeLoc;
	int _fadeToSepLoc;
	int _movementLoc;
};

}

#endif
