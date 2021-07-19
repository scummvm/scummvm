/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_PLUGINS_AGS_WAVES_VARS_H
#define AGS_PLUGINS_AGS_WAVES_VARS_H

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
	int x, y;
};

/*---------------------------------------------*/

typedef int (*SCAPI_CHARACTER_GETX)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_GETY)(AGSCharacter *ch);
typedef int (*SCAPI_CHARACTER_ID) (AGSCharacter *ch);

struct getMus {
	char musicPath[1024];
};

//WAVE SOUNDS FILES
struct Soundeffect {
	Mix_Chunk *chunk;
	int repeat;
	int volume;
	int playing;
	int allow;
	int channel;
	int filter;
};

struct Aud {
	int NumOfChannels;
	bool Initialized;
	bool Disabled;
	int FilterFrequency;
	int SoundValue;
};

struct Mus {
	int ID;
	int FadeTime;
	float FadeRate;
	float FadeVolume;
	int Channel;
	bool Switch;
	bool HaltedZero;
	bool HaltedOne;
};

struct RainParticle {
	int x;
	int y;
	int fx;
	int fy;
	int life;
	int trans;
	bool active;
	int translay;
	int transhold;
};

struct MusicStream {
	int volume;
	const char *Filename;
	int repeat;
	stb_vorbis *Vorbis;
	bool fix_click;
};

struct DustParticle {
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
};

struct Vars {
	int screen_width = 640;
	int screen_height = 360;
	int screen_color_depth = 32;
	AGSCharacter *playerCharacter = nullptr;

	SCAPI_CHARACTER_GETX Character_GetX = nullptr;
	SCAPI_CHARACTER_GETY Character_GetY = nullptr;
	SCAPI_CHARACTER_ID   Character_ID = nullptr;

	getMus MusicLoads[80];
	Soundeffect SFX[500];
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

	int _x;
	int _y;
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
	float ix, iy, ua;
	float b_time[5];
	float d_time;

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
};

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3

#endif
