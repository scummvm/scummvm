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

#ifndef AGS_PLUGINS_AGS_WAVES_VARS_H
#define AGS_PLUGINS_AGS_WAVES_VARS_H

#include "audio/audiostream.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

#define texWidth 240
#define texHeight 240
#define screenWidth 640
#define screenHeight 360

// TODO: Dummy definitions that need to be fixed
typedef void *stb_vorbis;
typedef void *Mix_Chunk;
typedef int SDL_AudioSpec;
typedef int SDL_AudioDeviceID;

struct Particle {
	int x;
	int y;
	int transp;
	int life;
	bool active;
	int dx;
	int dy;
	int mlay;
	int timlay;
	int movedport;
	int translay;
	int translayHold;
	int width;
	int height;
	int fx;
	int fy;
	bool doingcircle;
	float angle;
	float radius;
	int doingCircleChance;
	float angleLay;
	int frame;
	float anglespeed;
};

/*---------------------------------------------*/

typedef int (*SCAPI_CHARACTER_GETX)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_GETY)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_ID) (AGSCharacter *ch);

//WAVE SOUNDS FILES
struct SoundEffect {
	Audio::SoundHandle _soundHandle;
	int _repeat = 0;
	int _volume = 0;
	int _allow = 0;
	int _channel = 0;
	int _filter = 0;
	int _playing = 0;
};

struct Aud {
	int NumOfChannels = 0;
	bool Initialized = false;
	bool Disabled = false;
	int FilterFrequency = 0;
	int SoundValue = 0;
};

struct Mus {
	int ID = 0;
	int FadeTime = 0;
	float FadeRate = 0;
	float FadeVolume = 0;
	int Channel = 0;
	bool Switch = 0;
	bool HaltedZero = 0;
	bool HaltedOne = 0;
	Audio::SoundHandle _soundHandle;
};

struct RainParticle {
	int x = 0;
	int y = 0;
	int fx = 0;
	int fy = 0;
	int life = 0;
	int trans = 0;
	bool active = 0;
	int translay = 0;
	int transhold = 0;
};

struct MusicStream {
	int volume = 0;
	const char *Filename = nullptr;
	int repeat = 0;
	stb_vorbis *Vorbis = 0;
	bool fix_click = false;
};

struct DustParticle {
	int x = 0;
	int y = 0;
	int transp = 0;
	int life = 0;
	bool active = false;
	int dx = 0;
	int dy = 0;
	int mlay = 0;
	int timlay = 0;
	int movedport = 0;
	int translay = 0;
	int translayHold = 0;
};

struct Vars {
	int32 screen_width = 640;
	int32 screen_height = 360;
	int32 screen_color_depth = 32;
	AGSCharacter *playerCharacter = nullptr;

	PluginMethod Character_GetX;
	PluginMethod Character_GetY;
	PluginMethod Character_ID;

	SoundEffect SFX[500];
	RainParticle RainParticles[400];
	RainParticle RainParticlesFore[400];
	RainParticle RainParticlesBack[800];
	Aud GeneralAudio;
	Mus MFXStream;

	int currentMusic = -1;
	int currentMusicRepeat = -1;
	int currentMusicFadein = 0;

	double xv[3];
	double yv[3];
	double xvOGG[3];
	double yvOGG[3];

	Particle particles[110];
	Particle particlesF[10];
	Particle particles2[12];
	int WForceX[400];
	int WForceY[400];

	int raysizeF = 4;
	int dsizeF = 0;
	int raysize = 100;
	int dsize = 0;
	int raysize2 = 12;
	int dsize2 = 0;
	int creationdelayf = 0;
	int ww;
	int hh;
	int proom;
	int prevroom;
	bool OGG_Filter = false;

	SDL_AudioSpec spec[2];
	MusicStream globalStream[2];
	SDL_AudioDeviceID getDevice[2];
	bool AudioEnabled = false;
	float ix = 0, iy = 0, ua = 0;
	float b_time[5];
	float d_time = 0;

	// Y-coordinate first because we use horizontal scanlines
	uint32 texture[texHeight][texWidth];
	int distanceTable[screenHeight][screenWidth];
	int angleTable[screenHeight][screenWidth];
	bool generateonce = false;

	DustParticle dusts[200];
	int waitBy = 6;
	int raysizeDust = 200;
	int dsizeDust = 0;
	int creationdelay = 0;
	int Walkbehind[20];
	char *GameDatavalue[40000];
	char *Token[10000];
	int TokenUnUsed[10000];
	int usedTokens = 0;

	int dY[30];
	int tDy[30];
	int direction[30];

	// Warper fields
	int _newWidth = 0, _newHeight = 0;
	int _y2 = 0;
	int _x3 = 0, _y3 = 0;
	int _x4 = 0, _y4 = 0;
};

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3

#endif
