/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "common/memstream.h"

#include "griffon/griffon.h"
#include "griffon/config.h"
#include "griffon/console.h"

#include "common/file.h"
#include "common/system.h"

#include "image/bmp.h"

namespace Griffon {

// memo
/*
 in game scripts
   2 - find master key
   3 - find crystal
   4 - find shield - obj 8
   5 - find sword - obj 9
   6 - regular key chest
   7 - blue flask
   8 - garden's master key
   9 - lightning bomb
  10 - blue flask chest
  11 - lightning chest
  12 - armour chest
  13 - citadel master key
  14 - end of game
  15 - get sword3
  16 - shield3
  17 - armour3
  20 key chest 1
  60-lever

 monsters
 1 - baby dragon
 2 - one wing
 3 - boss 1
 4 - black knight
 5 - fire hydra
 6 - red dragon
 7 - priest
 8 - yellow fire dragon
 9 - two wing
10 - dragon2
11 - final boss
12 - bat kitty

 chests
  0 - regular flask
 11 - key chest
 14 - blue flask chest
 15 - lightning chest
 16 - armour chest
 17 - citadel master key
 18 - sword3
 19 - shield3
 20 - armour3

*/

// stubs
void GriffonEngine::Mix_Volume(int channel, int volume) {}

int GriffonEngine::Mix_getHandle() {
	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (!_mixer->isSoundHandleActive(_handles[i])) {
			return i;
		}
	}

	error("Mix_getHandle(): Too many sound handles");

	return -1;
}

int GriffonEngine::Mix_PlayChannel(DataChunk *chunk, bool looped) {
	int ch = Mix_getHandle();

#ifdef USE_VORBIS
	Audio::SeekableAudioStream *audioStream = Audio::makeVorbisStream(new Common::MemoryReadStream(chunk->data, chunk->size), DisposeAfterUse::YES);


	if (looped) {
		Audio::AudioStream *loopingStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles[ch], loopingStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES, false, false);

	} else {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handles[ch], audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES, false, false);
	}
#endif // USE_VORBIS

	return ch;
}

void GriffonEngine::Mix_Pause(int channel) {
	_mixer->pauseHandle(_handles[channel], true);
}

void GriffonEngine::Mix_HaltChannel(int channel) {
	_mixer->stopHandle(_handles[channel]);
}

void GriffonEngine::Mix_Resume(int channel) {
	_mixer->pauseHandle(_handles[channel], false);
}

bool GriffonEngine::Mix_Playing(int channel) {
	return _mixer->isSoundHandleActive(_handles[channel]);
}

DataChunk *Mix_LoadWAV(const char *name) {
	Common::File file;
	DataChunk *res = new DataChunk;

	file.open(name);
	if (!file.isOpen()) {
		error("Cannot open file %s", name);
	}

	res->size = file.size();
	res->data = (byte *)malloc(res->size);

	file.read(res->data, res->size);

	return res;
}


void GriffonEngine::griffon_main() {
	sys_initialize();
	game_showlogos();

	if (_shouldQuit)
		return;

	game_main();
}

// element tile locations
const int elementmap[15][20] = {
	{  2, 2, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, 2, -1, -1, -1, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1 },
	{  2, -1, 2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{  2, 2, 2, 2, 2, -1, -1, -1, 2, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, 2, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 0, 0, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};

const char *story[48] = {
	"The Griffon Legend",
	"http://syn9.thehideoutgames.com/",
	"",
	"Programming/Graphics: Daniel Kennedy",
	"Music/Sound effects: David Turner",
	"",
	"Porting to GCW-Zero: Dmitry Smagin",
	"",
	"",
	"Story",
	"Ever since I was a child",
	"I remember being told the",
	"Legend of the Griffon Knights,",
	"who rid the world of the",
	"Dragon Empire.  These great",
	"heroes inspired us to become",
	"knights as well.",
	" ",
	"Now, 500 years after the war",
	"ended, the Dragons have",
	"returned.  Cities are falling",
	"from the lack of knights to",
	"protect them.",
	" ",
	"We never saw it coming.",
	" ",
	"And now, here I am, making",
	"my way into the lower town",
	"of Fidelis, a small city on",
	"the main continent. The rest",
	"of my men have died over",
	"the last couple days from",
	"aerial attacks.",
	" ",
	"We believed we could find",
	"shelter here, only to find",
	"every last griffon dead,",
	"the town burned to the ground,",
	"and transformed into a garrison",
	"for the Dragon forces.",
	" ",
	"In these dark times, I try to",
	"draw strength from the stories",
	"of those knights that risked",
	"everything to protect their homeland,",
	" ",
	"and hope that I can die",
	"with that honor as well."
};

const char *story2[27] = {
	"After the fall of Margrave Gradius,",
	"All the dragons, struck with panic,",
	"evacuated the city immediately.",
	" ",
	"It\'s funny how without a leader",
	"everyone is so weak.",
	" ",
	" ",
	"But yet another leader will rise,",
	"and another city will fall.",
	" ",
	" ",
	"I should return home to Asherton",
	"It\'s time to leave this place",
	"and cleanse this blood stained",
	"life of mine.",
	" ",
	"No one should have to see as much",
	"death as I have.",
	" ",
	" ",
	"Before, I said that I wanted",
	"to die an honorable death.",
	" ",
	"Now I say that I have lived an",
	"honorable life,",
	"and I am free to die as I please."
};

// map in inventory menu
const int invmap[4][7][13] = {
	// map 0
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 43, 44, 45, 46, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 3, 2, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
	// map 1
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 19, 20, 21, 22, 0, 0, 0, 27, 0, 0, 0},
		{0, 0, 16, 17, 18, 0, 0, 0, 29, 30, 31, 0, 0},
		{0, 0, 12, 0, 13, 14, 0, 32, 33, 34, 35, 36, 0},
		{0, 8, 7, 6, 9, 10, 0, 37, 38, 39, 40, 41, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
	// map 2
	{
		{0, 0, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 63, 64, 65, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 58, 59, 60, 61, 62, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 55, 56, 57, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 50, 51, 52, 53, 54, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 48, 47, 49, 0, 0, 0, 0, 0}
	},

	// map 3
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 79, 80, 81, 0, 74, 72, 0, 0, 0, 0},
		{0, 0, 0, 78, 0, 0, 0, 73, 70, 69, 68, 0, 0},
		{0, 0, 77, 76, 75, 0, 0, 0, 71, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	}
};

// HELPER MACRO ---------------------------------------------------------------
#define INPUT(A, B)                 \
	do {                            \
		Common::String line;        \
		line = file.readLine();     \
		sscanf(line.c_str(), A, B); \
	} while(0)

// CODE GOES HERE -------------------------------------------------------------

void game_fillrect(Graphics::TransparentSurface *surface, int x, int y, int w, int h, int color) {
	surface->fillRect(Common::Rect(x, y, x + w, y + h), color);
}

Graphics::TransparentSurface *IMG_Load(const char *name, bool colorkey = false) {
	Common::File file;

	file.open(name);
	if (!file.isOpen()) {
		error("Cannot open file %s", name);
	}

	debug(1, "Loading: %s", name);

	Image::BitmapDecoder bitmapDecoder;
	bitmapDecoder.loadStream(file);
	file.close();

	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface(*bitmapDecoder.getSurface()->convertTo(g_system->getScreenFormat()));

	if (colorkey)
		surface->applyColorKey(255, 0, 255);

	return surface;
}

// copypaste from hRnd_CRT()
float GriffonEngine::RND() {
	/* return between 0 and 1 (but never 1) */
	return (float)_rnd->getRandomNumber(32767) * (1.0f / 32768.0f);
}

void GriffonEngine::game_addFloatIcon(int ico, float xloc, float yloc) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(floaticon[i][0]) < kEpsilon) {
			floaticon[i][0] = 32;
			floaticon[i][1] = xloc;
			floaticon[i][2] = yloc;
			floaticon[i][3] = ico;
			return;
		}
	}
}

void GriffonEngine::game_addFloatText(const char *stri, float xloc, float yloc, int col) {
	for (int i = 0; i < kMaxFloat; i++) {
		if (ABS(floattext[i][0]) < kEpsilon) {
			floattext[i][0] = 32;
			floattext[i][1] = xloc;
			floattext[i][2] = yloc;
			floattext[i][3] = col;
			strcpy(floatstri[i], stri);
			return;
		}
	}
}

void GriffonEngine::game_attack() {
	float npx = _player.px + 12;
	float npy = _player.py + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	// if facing up
	if (_player.walkdir == 0) {
		if (ly > 0) {
			int o2 = 0; // ??
			int o = objmap[lx][ly - 1];
			if (ly > 1 && _curmap == 58)
				o2 = objmap[lx][ly - 2];
			if (ly > 1 && _curmap == 54)
				o2 = objmap[lx][ly - 2];

			// cst
			if ((objectinfo[o][4] == 1 && (o == 0 || o > 4)) || (objectinfo[o2][4] == 0 && o2 == 10)) {
				if (o2 == 10)
					o = 10;

				int oscript = objectinfo[o][5];
				if (oscript == 0 && _player.inventory[INV_FLASK] < 9) {
					_player.inventory[INV_FLASK]++;
					game_addFloatIcon(6, lx * 16, (ly - 1) * 16);

					objmapf[_curmap][lx][ly - 1] = 1;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found Flask!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 0 && _player.inventory[INV_FLASK] == 9) {
					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndchest]);
						Mix_Volume(snd, config.effectsvol);
					}

					game_eventtext("Cannot Carry any more Flasks!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 2) {
					_player.inventory[INV_MASTERKEY]++;

					game_addFloatIcon(14, lx * 16, (ly - 1) * 16);

					_itemticks = _ticks + 215;

					if (_curmap == 34)
						_scriptflag[2][0] = 2;
					if (_curmap == 62)
						_scriptflag[8][0] = 2;
					if (_curmap == 81)
						_scriptflag[13][0] = 2;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;
					game_eventtext("Found the Temple Key!");
					return;
				}

				if (oscript == 3) {
					_player.foundspell[0] = 1;
					_player.spellcharge[0] = 0;

					game_addFloatIcon(7, lx * 16, (ly - 1) * 16);

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found the Infinite Crystal!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 4 && _player.shield == 1) {
					_player.shield = 2;

					game_addFloatIcon(4, lx * 16, (ly - 1) * 16);

					_itemticks = _ticks + 215;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found the Obsidian Shield!");
					objmapf[4][1][2] = 1;
					return;
				}

				if (oscript == 5 && _player.sword == 1) {
					_player.sword = 2;

					game_addFloatIcon(3, lx * 16, (ly - 1) * 16);

					_itemticks = _ticks + 215;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;
					game_eventtext("Found the Fidelis Sword!");
					return;
				}

				if (oscript == 6) {
					if (_player.inventory[INV_NORMALKEY] < 9) {
						_player.inventory[INV_NORMALKEY]++;

						for (int s = 20; s <= 23; s++) {
							if (_scriptflag[s][0] == 1) {
								_scriptflag[s][0] = 2;
							}
						}

						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[sndpowerup]);
							Mix_Volume(snd, config.effectsvol);
						}

						objmapf[_curmap][lx][ly - 1] = 1;

						if (objectinfo[o][4] == 1)
							objmap[lx][ly - 1] = 3;

						game_eventtext("Found Key");
						game_addFloatIcon(16, lx * 16, (ly - 1) * 16);
					} else {
						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[sndchest]);
							Mix_Volume(snd, config.effectsvol);
						}

						game_eventtext("Cannot Carry Any More Keys");
					}
				}

				if (oscript == 7 && _player.inventory[INV_DOUBLEFLASK] < 9) {
					_player.inventory[INV_DOUBLEFLASK]++;
					game_addFloatIcon(12, lx * 16, (ly - 1) * 16);

					objmapf[_curmap][lx][ly - 1] = 1;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found Mega Flask!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 7 && _player.inventory[INV_DOUBLEFLASK] == 9) {
					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndchest]);
						Mix_Volume(snd, config.effectsvol);
					}

					game_eventtext("Cannot Carry any more Mega Flasks!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 10 && _player.inventory[INV_DOUBLEFLASK] < 9) {
					_player.inventory[INV_DOUBLEFLASK]++;
					game_addFloatIcon(12, lx * 16, (ly - 1) * 16);

					objmapf[_curmap][lx][ly - 1] = 1;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found Mega Flask!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 10 && _player.inventory[INV_DOUBLEFLASK] == 9) {
					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndchest]);
						Mix_Volume(snd, config.effectsvol);
					}

					game_eventtext("Cannot Carry any more Mega Flasks!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 11 && _player.inventory[INV_SHOCK] < 9) {
					_player.inventory[INV_SHOCK]++;
					game_addFloatIcon(17, lx * 16, (ly - 1) * 16);

					objmapf[_curmap][lx][ly - 1] = 1;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found Lightning Bomb!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 11 && _player.inventory[INV_SHOCK] == 9) {
					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndchest]);
						Mix_Volume(snd, config.effectsvol);
					}

					game_eventtext("Cannot Carry any more Lightning Bombs!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 12 && _player.armour == 1) {
					_player.armour = 2;

					game_addFloatIcon(5, lx * 16, (ly - 1) * 16);

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;

					game_eventtext("Found the Fidelis Mail!");
					_itemticks = _ticks + 215;
					return;
				}

				if (oscript == 60) {
					if (_curmap == 58 && _scriptflag[60][0] == 0) {
						_scriptflag[60][0] = 1;

						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[sndlever]);
							Mix_Volume(snd, config.effectsvol);
						}

					} else if (_curmap == 58 && _scriptflag[60][0] > 0) {
						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[snddoor]);
							Mix_Volume(snd, config.effectsvol);
						}

						game_eventtext("It's stuck!");
					}

					if (_curmap == 54 && _scriptflag[60][0] == 1) {
						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[sndlever]);
							Mix_Volume(snd, config.effectsvol);
						}

						_scriptflag[60][0] = 2;
					} else if (_curmap == 54 && _scriptflag[60][0] > 1) {
						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[snddoor]);
							Mix_Volume(snd, config.effectsvol);
						}

						game_eventtext("It's stuck!");
					}

				}

				if (oscript == 15 && _player.sword < 3) {
					_player.sword = 3;

					game_addFloatIcon(18, lx * 16, (ly - 1) * 16);

					_itemticks = _ticks + 215;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;
					game_eventtext("Found the Blood Sword!");
					objmapf[4][1][2] = 1;
					return;
				}

				if (oscript == 16 && _player.shield < 3) {
					_player.shield = 3;
					game_addFloatIcon(19, lx * 16, (ly - 1) * 16);
					_itemticks = _ticks + 215;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;
					game_eventtext("Found the Entropy Shield!");
					objmapf[4][1][2] = 1;
					return;
				}

				if (oscript == 17 && _player.armour < 3) {
					_player.armour = 3;
					game_addFloatIcon(20, lx * 16, (ly - 1) * 16);
					_itemticks = _ticks + 215;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					if (objectinfo[o][4] == 1)
						objmap[lx][ly - 1] = 3;
					game_eventtext("Found the Rubyscale Armour!");
					objmapf[4][1][2] = 1;
					return;
				}

			}
		}
	}

	attacking = true;
	_player.attackframe = 0;
	movingup = 0;
	movingdown = 0;
	movingleft = 0;
	movingright = 0;

	for (int i = 0; i <= 15; i++) {
		for (int a = 0; a <= 3; a++) {
			_playerattackofs[a][i][2] = 0;
		}
	}
}

void GriffonEngine::game_castspell(int spellnum, float homex, float homey, float enemyx, float enemyy, int damagewho) {
	// spellnum 7 = sprite 6 spitfire

	for (int i = 0; i < kMaxSpell; i++) {
		if (ABS(spellinfo[i].frame) < kEpsilon) {
			spellinfo[i].homex = homex;
			spellinfo[i].homey = homey;
			spellinfo[i].enemyx = enemyx;
			spellinfo[i].enemyy = enemyy;
			spellinfo[i].spellnum = spellnum;
			int dw = 0;
			int npc = 0;
			if (damagewho > 0) {
				dw = 1;
				npc = damagewho;
			}

			spellinfo[i].damagewho = dw;
			spellinfo[i].npc = npc;

			spellinfo[i].frame = 32.0f;
			if (damagewho == 0) {
				spellinfo[i].strength = _player.spellstrength / 100;
				if (ABS(_player.spellstrength - 100) < kEpsilon)
					spellinfo[i].strength = 1.5f;
			}

			// set earthslide vars
			if (spellnum == 2) {
				for (int f = 0; f <= 8; f++) {
					spellinfo[i].rocky[f] = 0;
					spellinfo[i].rockimg[f] = (int)(RND() * 4);
					spellinfo[i].rockdeflect[f] = ((int)(RND() * 128) - 64) * 1.5;
				}
			}

			// set fire vars
			if (spellnum == 3) {
				for (int f = 0; f <= 4; f++) {
					spellinfo[i].legalive[f] = 32;
				}
			}


			// room fireball vars
			if (spellnum == 6) {
				int nballs = 0;
				for (int x = 0; x <= 19; x++) {
					for (int y = 0; y <= 14; y++) {
						if ((objmap[x][y] == 1 || objmap[x][y] == 2) && nballs < 5 && (int)(RND() * 4) == 0) {
							int ax = x * 16;
							int ay = y * 16;

							spellinfo[i].fireballs[nballs][0] = ax;
							spellinfo[i].fireballs[nballs][1] = ay;
							spellinfo[i].fireballs[nballs][2] = 0;
							spellinfo[i].fireballs[nballs][3] = 0;

							spellinfo[i].ballon[nballs] = 1;
							nballs = nballs + 1;
						}
					}
				}
				spellinfo[i].nfballs = nballs;
			}

			if (menabled && config.effects) {
				if (spellnum == 1) {
					int snd = Mix_PlayChannel(sfx[sndthrow]);
					Mix_Volume(snd, config.effectsvol);
				} else if (spellnum == 5) {
					int snd = Mix_PlayChannel(sfx[sndcrystal]);
					Mix_Volume(snd, config.effectsvol);
				} else if (spellnum == 8 || spellnum == 9) {
					int snd = Mix_PlayChannel(sfx[sndlightning]);
					Mix_Volume(snd, config.effectsvol);
				}
			}

			return;
		}
	}
}

void GriffonEngine::game_checkhit() {
	if (attacking) {
		for (int i = 1; i <= lastnpc; i++) {
			if (npcinfo[i].hp > 0 && npcinfo[i].pause < _ticks && (int)(RND() * 2) == 0) {
				float npx = npcinfo[i].x;
				float npy = npcinfo[i].y;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				float ps = _player.sword;
				if (ps > 1)
					ps = ps * 0.75;
				float damage = (float)_player.sworddamage * (1.0 + RND() * 1.0) * _player.attackstrength / 100.0 * ps;

				if (_console->_godMode)
					damage = 1000;

				if (ABS(_player.attackstrength - 100) < kEpsilon)
					damage *= 1.5;

				bool hit = false;
				if (_player.walkdir == 0) {
					if (abs(xdif) <= 8 && ydif >= 0 && ydif < 8)
						hit = true;
				} else if (_player.walkdir == 1) {
					if (abs(xdif) <= 8 && ydif <= 0 && ydif > -8)
						hit = true;
				} else if (_player.walkdir == 2) {
					if (abs(ydif) <= 8 && xdif >= -8 && xdif < 8)
						hit = true;
				} else if (_player.walkdir == 3) {
					if (abs(ydif) <= 8 && xdif <= 8 && xdif > -8)
						hit = true;
				}

				if (hit) {
					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndswordhit]);
						Mix_Volume(snd, config.effectsvol);
					}

					game_damagenpc(i, damage, 0);
				}
			}
		}
	}
}

void GriffonEngine::game_checkinputs() {
	int ntickdelay = 175;

	g_system->getEventManager()->pollEvent(_event);

	nposts = 0;

	for (int i = 0; i <= 20; i++) {
		postinfo[i][0] = 0;
		postinfo[i][1] = 0;
	}

	for (int x = 0; x <= 19; x++) {
		for (int y = 0; y <= 14; y++) {
			int o = objmap[x][y];
			if (objectinfo[o][4] == 3) {
				postinfo[nposts][0] = x * 16;
				postinfo[nposts][1] = y * 16;
				nposts = nposts + 1;
			}
		}
	}

	if (attacking || (_forcepause && _itemselon == 0))
		return;

	if (_event.type == Common::EVENT_QUIT) {
		_shouldQuit = true;
		return;
	}

	if (_event.type == Common::EVENT_KEYDOWN) {
		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			if (_itemticks < _ticks)
				game_title(1);
		} else if (_event.kbd.keycode == Common::KEYCODE_d && _event.kbd.hasFlags(Common::KBD_CTRL)) {
			_console->attach();
			_event.type = Common::EVENT_INVALID;
		} else if (_event.kbd.hasFlags(Common::KBD_CTRL)) {
			if (_itemselon == 0 && _itemticks < _ticks)
				game_attack();

			if (_itemselon == 1 && _itemticks < _ticks) {
				if (_curitem == 0 && _player.inventory[INV_FLASK] > 0) {
					_itemticks = _ticks + ntickdelay;

					int heal = 50;
					int maxh = _player.maxhp - _player.hp;

					if (heal > maxh)
						heal = maxh;

					_player.hp = _player.hp + heal;

					char text[256];
					sprintf(text, "+%i", heal);
					game_addFloatText(text, _player.px + 16 - 4 * strlen(text), _player.py + 16, 5);

					_player.inventory[INV_FLASK]--;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem == 1 && _player.inventory[INV_DOUBLEFLASK] > 0) {
					_itemticks = _ticks + ntickdelay;

					int heal = 200;
					int maxh = _player.maxhp - _player.hp;

					if (heal > maxh)
						heal = maxh;

					_player.hp = _player.hp + heal;

					char text[256];
					sprintf(text, "+%i", heal);
					game_addFloatText(text, _player.px + 16 - 4 * strlen(text), _player.py + 16, 5);

					_player.inventory[INV_DOUBLEFLASK]--;

					if (menabled && config.effects) {
						int snd = Mix_PlayChannel(sfx[sndpowerup]);
						Mix_Volume(snd, config.effectsvol);
					}

					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem == 2 && _player.inventory[INV_SHOCK] > 0) {
					game_castspell(8, _player.px, _player.py, npcinfo[_curenemy].x, npcinfo[_curenemy].y, 0);

					_forcepause = true;

					_player.inventory[INV_SHOCK]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;

				}

				if (_curitem == 3 && _player.inventory[INV_NORMALKEY] > 0 && canusekey == 1 && locktype == 1) {
					_roomlocks[roomtounlock] = 0;
					game_eventtext("UnLocked!");

					_player.inventory[INV_NORMALKEY]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					return;
				}

				if (_curitem == 4 && _player.inventory[INV_MASTERKEY] > 0 && canusekey == 1 && locktype == 2) {
					_roomlocks[roomtounlock] = 0;
					game_eventtext("UnLocked!");

					_player.inventory[INV_MASTERKEY]--;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					return;
				}

				if (_curitem == 5 && _player.spellcharge[0] == 100) {
					game_castspell(5, _player.px, _player.py, npcinfo[_curenemy].x, npcinfo[_curenemy].y, 0);

					_player.spellcharge[0] = 0;

					_forcepause = true;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
				}

				if (_curitem > 5 && _selenemyon == 1) {
					if (_curenemy <= lastnpc) {
						game_castspell(_curitem - 6, _player.px, _player.py, npcinfo[_curenemy].x, npcinfo[_curenemy].y, 0);
					} else {
						int pst = _curenemy - lastnpc - 1;
						game_castspell(_curitem - 6, _player.px, _player.py, postinfo[pst][0], postinfo[pst][1], 0);
					}

					_player.spellcharge[_curitem - 5] = 0;

					_player.spellstrength = 0;

					_itemticks = _ticks + ntickdelay;
					_selenemyon = 0;
					_itemselon = 0;
					_forcepause = false;
				}

				if (_curitem > 5 && _selenemyon == 0 && _itemselon == 1) {
					if (ABS(_player.spellcharge[_curitem - 5] - 100) < kEpsilon) {
						_itemticks = _ticks + ntickdelay;

						_selenemyon = 1;

						int i = 0;
						do {
							if (npcinfo[i].hp > 0) {
								_curenemy = i;
								goto __exit_do;
							}
							i = i + 1;
							if (i == lastnpc + 1) {
								_selenemyon = 0;
								goto __exit_do;
							}
						} while (1);
__exit_do:

						if (nposts > 0 && _selenemyon == 0) {
							_selenemyon = 1;
							_curenemy = lastnpc + 1;
						}
					}

				}
			}
		} else if (_event.kbd.hasFlags(Common::KBD_ALT)) {
			if (_itemticks < _ticks) {
				_selenemyon = 0;
				if (_itemselon == 1) {
					_itemselon = 0;
					_itemticks = _ticks + 220;
					_forcepause = false;
				} else {
					_itemselon = 1;
					_itemticks = _ticks + 220;
					_forcepause = true;
					_player.itemselshade = 0;
				}
			}
		}
	}

	if (_itemselon == 0) {
		movingup = 0;
		movingdown = 0;
		movingleft = 0;
		movingright = 0;
		if (_event.kbd.keycode == Common::KEYCODE_UP)
			movingup = 1;
		if (_event.kbd.keycode == Common::KEYCODE_DOWN)
			movingdown = 1;
		if (_event.kbd.keycode == Common::KEYCODE_LEFT)
			movingleft = 1;
		if (_event.kbd.keycode == Common::KEYCODE_RIGHT)
			movingright = 1;
	} else {
		movingup = 0;
		movingdown = 0;
		movingleft = 0;
		movingright = 0;

		if (_selenemyon == 1) {
			if (_itemticks < _ticks) {
				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					int origin = _curenemy;
					do {
						_curenemy = _curenemy - 1;
						if (_curenemy < 1)
							_curenemy = lastnpc + nposts;
						if (_curenemy == origin)
							break;
						if (_curenemy <= lastnpc && npcinfo[_curenemy].hp > 0)
							break;
						if (_curenemy > lastnpc)
							break;
					} while (1);
					_itemticks = _ticks + ntickdelay;
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					int origin = _curenemy;
					do {
						_curenemy = _curenemy + 1;
						if (_curenemy > lastnpc + nposts)
							_curenemy = 1;
						if (_curenemy == origin)
							break;
						if (_curenemy <= lastnpc && npcinfo[_curenemy].hp > 0)
							break;
						if (_curenemy > lastnpc)
							break;
					} while (1);
					_itemticks = _ticks + ntickdelay;
				}


				if (_curenemy > lastnpc + nposts)
					_curenemy = 1;
				if (_curenemy < 1)
					_curenemy = lastnpc + nposts;
			}
		} else {
			if (_itemticks < _ticks) {
				if (_event.kbd.keycode == Common::KEYCODE_UP) {
					_curitem = _curitem - 1;
					_itemticks = _ticks + ntickdelay;
					if (_curitem == 4)
						_curitem = 9;
					if (_curitem == -1)
						_curitem = 4;
				}
				if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					_curitem = _curitem + 1;
					_itemticks = _ticks + ntickdelay;
					if (_curitem == 5)
						_curitem = 0;
					if (_curitem == 10)
						_curitem = 5;
				}
				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					_curitem = _curitem - 5;
					_itemticks = _ticks + ntickdelay;
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					_curitem = _curitem + 5;
					_itemticks = _ticks + ntickdelay;
				}

				if (_curitem > 9)
					_curitem = _curitem - 10;
				if (_curitem < 0)
					_curitem = _curitem + 10;
			}
		}
	}
}

void GriffonEngine::game_checktrigger() {
	int npx = _player.px + 12;
	int npy = _player.py + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	canusekey = 0;

	if (triggerloc[lx][ly] > -1)
		game_processtrigger(triggerloc[lx][ly]);
}

#ifdef OPENDINGUX
#define MINCURSEL 7
#define MAXCURSEL 14
#define SY 22
#else
#define MINCURSEL 0
#define MAXCURSEL 14
#define SY (38 + (240 - 38) / 2 - 88)
#endif

void GriffonEngine::game_configmenu() {
	int cursel = MINCURSEL;

	_ticks = g_system->getMillis();
	int tickwait = 100;
	int keypause = _ticks + tickwait;

	Graphics::TransparentSurface *configwindow = IMG_Load("art/configwindow.bmp", true);
	configwindow->setAlpha(160, true);

	int _ticks1 = _ticks;
	do {
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		rcDest.left = 256 + 256 * cos(3.141592 / 180 * clouddeg * 40);
		rcDest.top = 192 + 192 * sin(3.141592 / 180 * clouddeg * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		_videobuffer->copyRectToSurface(configwindow->getPixels(), configwindow->pitch, 0, 0, configwindow->w, configwindow->h);

		int sy = SY;

		for (int i = 0; i <= 21; i++) {
			static const char *vr[22] = {
#ifdef OPENDINGUX
				"", "",
				"", "", "", "",
				"", "", "",
#else
				"Resolution:", "",
				"Bit Depth:", "", "", "",
				"Start Fullscreen:", "", "",
#endif
				"Music:", "", "",
				"Sound Effects:", "", "",
				"Music Volume:", "",
				"Effects Volume:", "", "", "", ""
			};
			static const char *vl[22] = {
#ifdef OPENDINGUX
				"", "",
				"", "", "", "",
				"", "", "",
#else
				"320x240", "640x480",
				"16", "24", "32", "",
				"Yes", "No", "",
#endif
				"On", "Off", "",
				"On", "Off", "",
				"[----------]", "",
				"[----------]", "",
				"Exit + Save", "",
				"Exit"
			};
			static char line[24];

			if (i == 15 || i == 17) {
				int vol = (i == 15 ? config.musicvol : config.effectsvol) * 9 / 255;
				if (vol < 0)
					vol = 0;
				if (vol > 9)
					vol = 9;

				strcpy(line, "[----------]");
				line[vol + 1] = 'X';
				vl[i] = line;
			}

			int cl = 3;
			if (i == 0 && config.scr_width == 320)
				cl = 0;
			if (i == 1 && config.scr_width == 640)
				cl = 0;
			if (i == 2 && config.scr_bpp == 16)
				cl = 0;
			if (i == 3 && config.scr_bpp == 24)
				cl = 0;
			if (i == 4 && config.scr_bpp == 32)
				cl = 0;
			if (i == 6 && config.fullscreen)
				cl = 0;
			if (i == 7 && !config.fullscreen)
				cl = 0;
			if (i == 9 && config.music)
				cl = 0;
			if (i == 10 && !config.music)
				cl = 0;
			if (i == 12 && config.effects)
				cl = 0;
			if (i == 13 && !config.effects)
				cl = 0;

			if (i > 18)
				cl = 0;

			sys_print(_videobuffer, vr[i], 156 - 8 * strlen(vr[i]), sy + i * 8, 0);
			sys_print(_videobuffer, vl[i], 164, sy + i * 8, cl);
		}

		int curselt = cursel;
		if (cursel > 4)
			curselt += 1;
		if (cursel > 6)
			curselt += 1;
		if (cursel > 8)
			curselt += 1;
		if (cursel > 10)
			curselt += 1;
		if (cursel > 11)
			curselt += 1;
		if (cursel > 12)
			curselt += 1;
		if (cursel > 13)
			curselt += 1;

		Common::Rect rc;
		rc.left = 148 + 3 * cos(3.14159 * 2 * _itemyloc / 16.0);
		rc.top = sy + 8 * curselt - 4;

		_itemimg[15]->blit(*_videobuffer, rc.left, rc.top);

		float yy = 255.0;
		if (_ticks < _ticks1 + 1000) {
			yy = 255.0 * ((float)(_ticks - _ticks1) / 1000.0);
			if (yy < 0.0)
				yy = 0.0;
			if (yy > 255.0)
				yy = 255.0;
		}

		_videobuffer->setAlpha((int)yy);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		_itemyloc += 0.75 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		if (keypause < _ticks) {
			g_system->getEventManager()->pollEvent(_event);

			if (_event.type == Common::EVENT_QUIT)
				_shouldQuit = true;

			if (_event.type == Common::EVENT_KEYDOWN) {
				keypause = _ticks + tickwait;

				if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
					break;

				if (_event.kbd.keycode == Common::KEYCODE_LEFT) {
					if (cursel == 11) {
						config.musicvol = config.musicvol - 25;
						if (config.musicvol < 0)
							config.musicvol = 0;

						Mix_Volume(musicchannel, config.musicvol);
						Mix_Volume(menuchannel, config.musicvol);
					} else if (cursel == 12) {
						config.effectsvol = config.effectsvol - 25;
						if (config.effectsvol < 0)
							config.effectsvol = 0;

						Mix_Volume(-1, config.effectsvol);
						Mix_Volume(musicchannel, config.musicvol);
						Mix_Volume(menuchannel, config.musicvol);

						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[snddoor]);
							Mix_Volume(snd, config.effectsvol);
						}
					}
				}
				if (_event.kbd.keycode == Common::KEYCODE_RIGHT) {
					if (cursel == 11) {
						config.musicvol = config.musicvol + 25;
						if (config.musicvol > 255)
							config.musicvol = 255;

						Mix_Volume(musicchannel, config.musicvol);
						Mix_Volume(menuchannel, config.musicvol);
					} else if (cursel == 12) {
						config.effectsvol = config.effectsvol + 25;
						if (config.effectsvol > 255)
							config.effectsvol = 255;

						Mix_Volume(-1, config.effectsvol);
						Mix_Volume(musicchannel, config.musicvol);
						Mix_Volume(menuchannel, config.musicvol);

						if (menabled && config.effects) {
							int snd = Mix_PlayChannel(sfx[snddoor]);
							Mix_Volume(snd, config.effectsvol);
						}
					}
				}

				if (_event.kbd.keycode == Common::KEYCODE_UP) {
					cursel--;
					if (cursel < MINCURSEL)
						cursel = 14;
				}
				if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					cursel++;
					if (cursel > 14)
						cursel = MINCURSEL;
				}

				if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
#if 0
					if (cursel == 0) {
						fullscreen = config.fullscreen | config.hwaccel | config.hwsurface;

						_video = SDL_SetVideoMode(320, 240, config.scr_bpp, fullscreen);
						if (_video == 0) {
							_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, fullscreen);
						} else {
							config.scr_width = 320;
							config.scr_height = 240;
						}

						SDL_UpdateRect(_video, 0, 0, config.scr_width, config.scr_height);
					}
					if (cursel == 1) {
						fullscreen = config.fullscreen | config.hwaccel | config.hwsurface;

						_video = SDL_SetVideoMode(640, 480, config.scr_bpp, fullscreen);
						if (_video == 0) {
							_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, fullscreen);
						} else {
							config.scr_width = 640;
							config.scr_height = 480;
						}

						SDL_UpdateRect(_video, 0, 0, config.scr_width, config.scr_height);
					}
					if (cursel == 2 || cursel == 3 || cursel == 4) {
						fullscreen = config.fullscreen | config.hwaccel | config.hwsurface;

						int b = 16;
						if (cursel == 3)
							b = 24;
						if (cursel == 4)
							b = 32;
						_video = SDL_SetVideoMode(config.scr_width, config.scr_height, b, fullscreen);
						if (_video == 0) {
							_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, fullscreen);
						} else {
							config.scr_bpp = b;
						}

						SDL_UpdateRect(_video, 0, 0, config.scr_width, config.scr_height);
					}
					if (cursel == 5) {
						ofullscreen = config.fullscreen | config.hwaccel | config.hwsurface;
						fullscreen = SDL_FULLSCREEN | config.hwaccel | config.hwsurface;

						_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, fullscreen);
						if (_video == 0) {
							_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, ofullscreen);
						} else {
							config.fullscreen = SDL_FULLSCREEN;
						}

						SDL_UpdateRect(_video, 0, 0, config.scr_width, config.scr_height);
					}
					if (cursel == 6) {
						ofullscreen = config.fullscreen | config.hwaccel | config.hwsurface;
						fullscreen = 0 | config.hwaccel | config.hwsurface;

						_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, fullscreen);
						if (_video == 0) {
							_video = SDL_SetVideoMode(config.scr_width, config.scr_height, config.scr_bpp, ofullscreen);
						} else {
							config.fullscreen = 0;
						}

						SDL_UpdateRect(_video, 0, 0, config.scr_width, config.scr_height);
					}
					if (cursel == 7 && config.music == 0) {
						config.music = 1;
						if (menabled == 1) {
							menuchannel = Mix_PlayChannel(_mmenu, true);
							Mix_Volume(menuchannel, config.musicvol);
						}
					}
					if (cursel == 8 && config.music == 1) {
						config.music = 0;
						Mix_HaltChannel(musicchannel);
						Mix_HaltChannel(menuchannel);
					}
					if (cursel == 9 && config.effects == 0) {
						config.effects = 1;
						if (menabled == 1) {
							int snd = Mix_PlayChannel(sfx[snddoor]);
							Mix_Volume(snd, config.effectsvol);
						}
					}

					if (cursel == 10 && config.effects)
						config.effects = 0;

					if (cursel == 13) {
						config_save(&config);
						break;
					}
#endif

					if (cursel == 14) {
						// reset keys to avoid returning
						// keys[SDLK_SPACE] = keys[SDLK_RETURN] = 0; // FIXME
						break;
					}
				}
			}
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg = clouddeg - 360;

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	configwindow->free();
	_itemticks = _ticks + 210;

	cloudimg->setAlpha(64, true);
}

void GriffonEngine::game_damagenpc(int npcnum, int damage, int spell) {
	char line[256];
	int fcol;

	if (damage == 0) {
		strcpy(line, "miss!");
		fcol = 2;
	} else {
		int ratio = 0;
		int heal = 0;
		if (damage < 0)
			heal = 1;
		damage = abs(damage);

		if (heal == 0) {
			if (damage > npcinfo[npcnum].hp) {
				ratio = (damage - npcinfo[npcnum].hp) * 100 / damage;
				damage = npcinfo[npcnum].hp;
			}

			npcinfo[npcnum].hp -= damage;
			if (npcinfo[npcnum].hp < 0)
				npcinfo[npcnum].hp = 0;

			sprintf(line, "-%i", damage);
			fcol = 1;
		} else {
			npcinfo[npcnum].hp += damage;
			if (npcinfo[npcnum].hp > npcinfo[npcnum].maxhp)
				npcinfo[npcnum].hp = npcinfo[npcnum].maxhp;

			sprintf(line, "+%i", damage);
			fcol = 5;
		}

		npcinfo[npcnum].pause = _ticks + 900;

		if (spell == 0)
			_player.attackstrength = ratio;
	}

	game_addFloatText(line, npcinfo[npcnum].x + 12 - 4 * strlen(line), npcinfo[npcnum].y + 16, fcol);

	if (npcinfo[npcnum].spriteset == 12)
		game_castspell(9, npcinfo[npcnum].x, npcinfo[npcnum].y, _player.px, _player.py, npcnum);

	// if enemy is killed
	if (npcinfo[npcnum].hp == 0) {
		_player.exp = _player.exp + npcinfo[npcnum].maxhp;

		if (npcinfo[npcnum].spriteset == 1 || npcinfo[npcnum].spriteset == 7 || npcinfo[npcnum].spriteset == 6) {
			int ff = (int)(RND() * _player.level * 3);
			if (ff == 0) {
				float npx = npcinfo[npcnum].x + 12;
				float npy = npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (objmap[lx][ly] == -1)
					objmap[lx][ly] = 4;
			}
		}

		if (npcinfo[npcnum].spriteset == 2 || npcinfo[npcnum].spriteset == 9 || npcinfo[npcnum].spriteset == 4 || npcinfo[npcnum].spriteset == 5) {
			int ff = (int)(RND() * _player.level);
			if (ff == 0) {
				float npx = npcinfo[npcnum].x + 12;
				float npy = npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (objmap[lx][ly] == -1)
					objmap[lx][ly] = 12;
			}
		}

		if (npcinfo[npcnum].spriteset == 9 || npcinfo[npcnum].spriteset == 10 || npcinfo[npcnum].spriteset == 5) {
			int ff = (int)(RND() * _player.level * 2);
			if (ff == 0) {
				float npx = npcinfo[npcnum].x + 12;
				float npy = npcinfo[npcnum].y + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (objmap[lx][ly] == -1)
					objmap[lx][ly] = 13;
			}
		}

		// academy master key chest script
		if (npcinfo[npcnum].script == 2) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				objmap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[2][0] = 1;
			}
		}

		// academy crystal chest script
		if (npcinfo[npcnum].script == 3) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				objmap[cx][cy] = 6;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_scriptflag[3][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}
		}

		// tower shield chest script
		if (npcinfo[npcnum].script == 4 && _scriptflag[4][0] == 0) {
			triggerloc[9][7] = 5004;

			int curtile = 40;
			int curtilel = 0;
			int curtilex = curtile % 20;
			int curtiley = (curtile - curtilex) / 20;

			int l = 0; // ?? not defined in original code
			tileinfo[l][9][7][0] = curtile + 1;
			tileinfo[l][9][7][1] = 0;

			rcSrc.left = curtilex * 16;
			rcSrc.top = curtiley * 16;
			rcSrc.setWidth(16);
			rcSrc.setHeight(16);

			rcDest.left = 9 * 16;
			rcDest.top = 7 * 16;
			rcDest.setWidth(16);
			rcDest.setHeight(16);

			tiles[curtilel]->blit(*_mapbg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

		// firehydra sword chest
		if (npcinfo[npcnum].script == 5) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 6;

				objmap[cx][cy] = 9;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_scriptflag[5][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}

		}

		// gardens master key script
		if (npcinfo[npcnum].script == 8 && _scriptflag[6][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 13;
				int cy = 7;

				objmap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[8][0] = 1;
			}
		}

		// regular key chest 1
		for (int s = 20; s <= 23; s++) {
			if (npcinfo[npcnum].script == s && _scriptflag[s][0] < 2) {
				bool alive = false;
				for (int i = 1; i <= lastnpc; i++) {
					if (npcinfo[i].hp > 0)
						alive = true;
				}

				if (!alive) {
					int cx = 9;
					int cy = 7;

					objmap[cx][cy] = 11;

					rcDest.left = cx * 8;
					rcDest.top = cy * 8;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					float npx = _player.px + 12;
					float npy = _player.py + 20;

					int lx = (int)npx / 16;
					int ly = (int)npy / 16;

					if (lx == cx && ly == cy)
						_player.py = _player.py + 16;
					_scriptflag[s][0] = 1;
					_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				}
			}
		}

		// pickup lightning bomb
		if (npcinfo[npcnum].script == 9 && (_curmap == 41 && _scriptflag[9][1] == 0)) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 9;
				int cy = 7;

				objmap[cx][cy] = 13;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
			}
		}

		// citadel armour chest
		if (npcinfo[npcnum].script == 12) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 8;
				int cy = 7;

				objmap[cx][cy] = 16;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_scriptflag[12][0] = 1;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
			}
		}

		// citadel master key script
		if (npcinfo[npcnum].script == 13 && _scriptflag[13][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 11;
				int cy = 10;

				objmap[cx][cy] = 5;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[13][0] = 1;
			}
		}

		// max ups
		if (npcinfo[npcnum].script == 15 && _scriptflag[15][0] == 0) {
			bool alive = false;
			for (int i = 1; i <= lastnpc; i++) {
				if (npcinfo[i].hp > 0)
					alive = true;
			}

			if (!alive) {
				int cx = 6;
				int cy = 8;

				objmap[cx][cy] = 18;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				float npx = _player.px + 12;
				float npy = _player.py + 20;

				int lx = (int)npx / 16;
				int ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[15][0] = 1;

				cx = 9;
				cy = 8;

				objmap[cx][cy] = 19;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				npx = _player.px + 12;
				npy = _player.py + 20;

				lx = (int)npx / 16;
				ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));

				_scriptflag[16][0] = 1;

				cx = 12;
				cy = 8;

				objmap[cx][cy] = 20;

				rcDest.left = cx * 8;
				rcDest.top = cy * 8;
				rcDest.setWidth(8);
				rcDest.setHeight(8);

				npx = _player.px + 12;
				npy = _player.py + 20;

				lx = (int)npx / 16;
				ly = (int)npy / 16;

				if (lx == cx && ly == cy)
					_player.py = _player.py + 16;
				_clipbg2->fillRect(rcDest, _clipbg->format.RGBToColor(255, 255, 255));
				_scriptflag[17][0] = 1;
			}
		}

		if (npcinfo[npcnum].script == 14)
			game_endofgame();
	}
}

void GriffonEngine::game_damageplayer(int damage) {
	char line[256];

	if (!_console->_godMode)
		_player.hp -= damage;

	if (_player.hp < 0)
		_player.hp = 0;

	sprintf(line, "-%i", damage);
	if (damage == 0)
		strcpy(line, "miss!");

	game_addFloatText(line, _player.px + 12 - 4 * strlen(line), _player.py + 16, 4);

	_player.pause = _ticks + 1000;
}

void GriffonEngine::game_drawanims(int Layer) {
	for (int sx = 0; sx <= 19; sx++) {
		for (int sy = 0; sy <= 14; sy++) {
			int o = objmap[sx][sy];

			if (o > -1) {
				int xtiles = objectinfo[o][1];
				int ytiles = objectinfo[o][2];
				int cframe = objectframe[o][1];

				for (int x = 0; x <= xtiles - 1; x++) {
					for (int y = 0; y <= ytiles - 1; y++) {
						int x1 = (sx + x) * 16;
						int y1 = (sy + y) * 16;

						if (objecttile[o][cframe][x][y][1] == Layer) {
							int c = objecttile[o][cframe][x][y][0];
							c = c - 1;
							int curtilel = 3;
							int curtilex = c % 20;
							int curtiley = (c - curtilex) / 20;

							if (_curmap == 58 && _scriptflag[60][0] > 0)
								curtilex = 1;
							if (_curmap == 54 && _scriptflag[60][0] > 1)
								curtilex = 1;
							rcSrc.left = curtilex * 16;
							rcSrc.top = curtiley * 16;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							rcDest.left = x1;
							rcDest.top = y1;
							rcDest.setWidth(16);
							rcDest.setHeight(16);

							tiles[curtilel]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
						}

						if (Layer == 1) {
							for (int l = 1; l <= 2; l++) {
								int c = tileinfo[l][sx + x][sy + y][0];
								if (c > 0) {
									int cl = tileinfo[l][sx + x][sy + y][1];

									c = c - 1;
									int curtile = c;
									int curtilel = cl;
									int curtilex = c % 20;
									int curtiley = (c - curtilex) / 20;

									rcSrc.left = curtilex * 16;
									rcSrc.top = curtiley * 16;
									rcSrc.setWidth(16);
									rcSrc.setHeight(16);

									rcDest.left = (sx + x) * 16;
									rcDest.top = (sy + y) * 16;
									rcDest.setWidth(16);
									rcDest.setHeight(16);

									int pass = 1;
									if (curtilel == 1) {
										for (int ff = 0; ff <= 5; ff++) {
											int ffa = 20 * 5 - 1 + ff * 20;
											int ffb = 20 * 5 + 4 + ff * 20;
											if (curtile > ffa && curtile < ffb)
												pass = 0;
										}
									}

									if (pass == 1)
										tiles[curtilel]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
								}
							}
						}
					}
				}
			}
		}
	}
}

int hud_recalc(int a, int b, int c) {
	int result = a * b / c;

	return result > b ? b : result;
}

#define RGB(R, G, B) (_videobuffer->format.RGBToColor((R), (G), (B)))

void GriffonEngine::game_drawhud() {
	//sprintf(line, "_fps: %i, map: %i, exp: %i/%i", (int)_fps, _curmap, _player.exp, _player.nextlevel);
	//sys_print(_videobuffer, line, 0, 0, 0);

	game_fillrect(_videobuffer2, 0, 0, 320, 240, 0);

	for (int i = 0; i < kMaxFloat; i++) {
		if (floattext[i][0] > 0) {
			int fc = (int)floattext[i][3];
			int c = fc, c2 = 3;

			if (fc == 4)
				c = 1;
			else if (fc == 5)
				c = 0;

			if (fc == 1 || fc == 3)
				c2 = 2;

			if (fc != 0) {
				sys_print(_videobuffer, floatstri[i], (int)(floattext[i][1]) + 0, (int)(floattext[i][2]) - 1, c2);
				sys_print(_videobuffer, floatstri[i], (int)(floattext[i][1]) + 0, (int)(floattext[i][2]) + 1, c2);
				sys_print(_videobuffer, floatstri[i], (int)(floattext[i][1]) - 1, (int)(floattext[i][2]) + 0, c2);
				sys_print(_videobuffer, floatstri[i], (int)(floattext[i][1]) + 1, (int)(floattext[i][2]) + 0, c2);
			}

			sys_print(_videobuffer, floatstri[i], (int)(floattext[i][1]), (int)(floattext[i][2]), c);
		}

		if (floaticon[i][0] > 0) {
			int ico = floaticon[i][3];
			int ix = floaticon[i][1];
			int iy = floaticon[i][2];

			rcDest.left = ix;
			rcDest.top = iy;

			if (ico != 99)
				_itemimg[ico]->blit(*_videobuffer, rcDest.left, rcDest.top);
			if (ico == 99) {
				_spellimg->setAlpha((int)(RND() * 96) + 96, true);

				rcSrc.left = 16 * (int)(RND() * 2);
				rcSrc.top = 80;
				rcSrc.setWidth(16);
				rcSrc.setHeight(16);

				rcDest.left = ix;
				rcDest.top = iy;

				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

				_spellimg->setAlpha(255, true);
			}
		}
	}

	if (_itemselon == 0) {
		int sy = 211;


		int nx = 19 * 8 + 13;
		rcSrc.left = nx - 17 + 48;
		rcSrc.top = sy;

		// spells in game
		if (_player.foundspell[0] == 1) {
			for (int i = 0; i < 5; i++) {
				rcSrc.left = rcSrc.left + 17;

				if (_player.foundspell[i] == 1) {
					_itemimg[7 + i]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

					game_fillrect(_videobuffer, rcSrc.left, sy + 16, 16, 4, RGB(0, 32, 32));
					game_fillrect(_videobuffer, rcSrc.left + 1, sy + 17,
					              hud_recalc(_player.spellcharge[i], 14, 100), 2,
					              ABS(_player.spellcharge[i] - 100) < kEpsilon ? RGB(255, 128, 32) : RGB(0, 224, 64));
				}
			}
		}
		return;
	}

	if (_selenemyon == 0) {
		rcDest.left = 0;
		rcDest.top = 0;
		rcDest.right = 320;
		rcDest.bottom = 240;
		_videobuffer2->fillRect(rcDest, 0);
		_videobuffer2->setAlpha((int)(_player.itemselshade * 4)); // FIXME
		_videobuffer2->blit(*_videobuffer);

		int sy = 202;
		rcSrc.left = 46;
		rcSrc.top = 46;

		_inventoryimg->setAlpha(160, true); // 128
		_inventoryimg->blit(*_videobuffer, rcSrc.left, rcSrc.top);
		_inventoryimg->setAlpha(255, true);

		int sx = 54;
		sy = 55;

		// draw map 9,77
		rcDest.left = 46 + 9;
		rcDest.top = 46 + 77;

		int amap = 0;
		if (_curmap > 46)
			amap = 2;
		if (_curmap > 67)
			amap = 3;
		if (_curmap > 5 && _curmap < 42)
			amap = 1;
		mapimg[amap]->blit(*_videobuffer, rcDest.left, rcDest.top);

		long ccc = _videobuffer->format.RGBToColor(128 + 127 * sin(3.141592 * 2 * _itemyloc / 16), 0, 0);

		for (int b = 0; b <= 6; b++) {
			for (int a = 0; a <= 12; a++) {
				if (invmap[amap][b][a] == _curmap) {
					game_fillrect(_videobuffer, 46 + 9 + a * 9 + 2, 46 + 77 + b * 9 + 1, 6, 6, ccc);
				}
			}
		}

		if (amap == 1) {
			sys_print(_videobuffer, "L1", 46 + 9, 46 + 77, 0);
			sys_print(_videobuffer, "L2", 46 + 9 + 7 * 9, 46 + 77, 0);
		}

		char line[128];
		sprintf(line, "Health: %i/%i", _player.hp, _player.maxhp);
		sys_print(_videobuffer, line, sx, sy, _player.hp <= _player.maxhp * 0.25 ? (int)_player.hpflash : 0);

		sprintf(line, "Level : %i", _player.level);
		if (_player.level == _player.maxlevel)
			strcpy(line, "Level : MAX");
		sys_print(_videobuffer, line, sx, sy + 8, 0);

		// experience
		game_fillrect(_videobuffer, sx + 64, sy + 16, 16, 4, RGB(0, 32, 32));
		game_fillrect(_videobuffer, sx + 65, sy + 17,
		              hud_recalc(_player.exp, 14, _player.nextlevel), 2, RGB(0, 224, 64));

		// attack strength
		game_fillrect(_videobuffer, sx + 1, sy + 16, 56, 6, RGB(0, 32, 32));
		game_fillrect(_videobuffer, sx + 1, sy + 17,
		              hud_recalc(_player.attackstrength, 54, 100), 2,
		              ABS(_player.attackstrength - 100) < kEpsilon ? RGB(255, 128, 32) : RGB(0, 64, 224));

		// spell strength
		game_fillrect(_videobuffer, sx + 1, sy + 19,
		              hud_recalc(_player.spellstrength, 54, 100), 2,
		              ABS(_player.spellstrength - 100) < kEpsilon ? RGB(224, 0, 0) : RGB(128, 0, 224));

		// time
		int ase = secstart + secsingame;
		int h = ((ase - (ase % 3600)) / 3600);
		ase = (ase - h * 3600);
		int m = ((ase - (ase % 60)) / 60);
		int s = (ase - m * 60);

		sprintf(line, "%02i:%02i:%02i", h, m, s);
		sys_print(_videobuffer, line, 46 + 38 - strlen(line) * 4, 46 + 49, 0);

		sys_print(_videobuffer, "Use", 193, 55, 0);
		sys_print(_videobuffer, "Cast", 236, 55, 0);

		rcSrc.left = 128;
		rcSrc.top = 91;

		int ss = (_player.sword - 1) * 3;
		if (_player.sword == 3)
			ss = 18;
		_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

		rcSrc.left = rcSrc.left + 16;
		ss = (_player.shield - 1) * 3 + 1;
		if (_player.shield == 3)
			ss = 19;
		_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

		rcSrc.left = rcSrc.left + 16;
		ss = (_player.armour - 1) * 3 + 2;
		if (_player.armour == 3)
			ss = 20;
		_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

		for (int i = 0; i <= 4; i++) {
			sx = 188;
			sy = 70 + i * 24;
			rcSrc.left = sx;
			rcSrc.top = sy;
			if (i == 0)
				_itemimg[6]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
			if (i == 1)
				_itemimg[12]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
			if (i == 2)
				_itemimg[17]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
			if (i == 3)
				_itemimg[16]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
			if (i == 4)
				_itemimg[14]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

			sprintf(line, "x%i", _player.inventory[i]);
			sys_print(_videobuffer, line, sx + 17, sy + 7, 0);
		}

		// spells in menu
		if (_player.foundspell[0] == 1) {
			for (int i = 0; i < 5; i++) {
				rcSrc.left = 243;
				rcSrc.top = 67 + i * 24;
				sy = rcSrc.top;

				if (_player.foundspell[i] == 1) {
					_itemimg[7 + i]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

					game_fillrect(_videobuffer, rcSrc.left, sy + 16, 16, 4, RGB(0, 32, 32));
					game_fillrect(_videobuffer, rcSrc.left + 1, sy + 17,
					              hud_recalc(_player.spellcharge[i], 14, 100), 2,
					              ABS(_player.spellcharge[i] - 100) < kEpsilon ? RGB(255, 128, 32) : RGB(0, 224, 64));
				}
			}
		}

		if (_itemselon == 1) {
			for (int i = 0; i <= 4; i++) {
				if (_curitem == 5 + i) {
					rcDest.left = (float)(243 - 12 + 3 * sin(3.141592 * 2 * _itemyloc / 16));
					rcDest.top = 67 + 24 * i;
					_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);
				}

				if (_curitem == i) {
					rcDest.left = (float)(189 - 12 + 3 * sin(3.141592 * 2 * _itemyloc / 16));
					rcDest.top = 70 + 24 * i;
					_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);
				}
			}
		}
	}

	if (_selenemyon == 1) {
		if (_curenemy > lastnpc) {
			int pst = _curenemy - lastnpc - 1;
			rcDest.left = postinfo[pst][0];
			rcDest.top = (float)(postinfo[pst][1] - 4 - sin(3.141592 / 8 * _itemyloc));
		} else {
			rcDest.left = npcinfo[_curenemy].x + 4;
			rcDest.top = (float)(npcinfo[_curenemy].y + 4 - 16 - sin(3.141592 / 8 * _itemyloc));
		}

		_itemimg[13]->blit(*_videobuffer, rcDest.left, rcDest.top);
	}
}

void GriffonEngine::game_drawnpcs(int mode) {
	unsigned int ccc = _videobuffer->format.RGBToColor(255, 128, 32);
	int fst = firsty;
	int lst = lasty;

	if (mode == 0)
		lst = _player.ysort;
	if (mode == 1)
		fst = _player.ysort;

	for (int yy = fst; yy <= lst; yy++) {

		if (ysort[yy] > 0) {
			int i = ysort[yy];

			if (npcinfo[i].hp > 0) {
				int npx = (int)(npcinfo[i].x);
				int npy = (int)(npcinfo[i].y);

				int sprite = npcinfo[i].spriteset;

				int wdir = npcinfo[i].walkdir;

				// spriteset1 specific
				if (npcinfo[i].spriteset == 1) {

					if (npcinfo[i].attacking == 0) {

						int cframe = npcinfo[i].cframe;

						rcSrc.left = (int)(cframe / 4) * 24;
						rcSrc.top = wdir * 24;
						rcSrc.setWidth(24);
						rcSrc.setHeight(24);

						rcDest.left = npx;
						rcDest.top = npy;
						rcDest.setWidth(24);
						rcDest.setHeight(24);

						if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
							npcinfo[i].shake = _ticks + 50;
							rcDest.left += (int)(RND() * 3) - 1;
							rcDest.top += (int)(RND() * 3) - 1;
						}

						anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					} else {
						int cframe = npcinfo[i].cattackframe;

						rcSrc.left = (int)(cframe / 4) * 24;
						rcSrc.top = wdir * 24;
						rcSrc.setWidth(24);
						rcSrc.setHeight(24);

						rcDest.left = npx;
						rcDest.top = npy;
						rcDest.setWidth(24);
						rcDest.setHeight(24);

						animsa[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

				}

				// onewing
				if (npcinfo[i].spriteset == 2) {
					for (int f = 0; f <= 7; f++) {
						int s = npcinfo[i].bodysection[f].sprite;
						rcSrc.left = animset2[s].x;
						rcSrc.top = animset2[s].y;
						rcSrc.setWidth(animset2[s].w);
						rcSrc.setHeight(animset2[s].h);

						rcDest.left = npcinfo[i].bodysection[f].x - animset2[s].xofs;
						rcDest.top = npcinfo[i].bodysection[f].y - animset2[s].yofs + 2;

						anims[2]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

				}

				// twowing
				if (npcinfo[i].spriteset == 9) {
					for (int f = 0; f <= 7; f++) {
						int yp = 0;

						if (f == 0 && (_curmap == 53 || _curmap == 57 || _curmap == 61 || _curmap == 65 || _curmap == 56 || _curmap > 66) && _scriptflag[60][0] > 0)
							yp = 16;
						int s = npcinfo[i].bodysection[f].sprite;
						rcSrc.left = animset9[s].x;
						rcSrc.top = animset9[s].y + yp;
						rcSrc.setWidth(animset9[s].w);
						rcSrc.setHeight(animset9[s].h);

						rcDest.left = npcinfo[i].bodysection[f].x - animset9[s].xofs;
						rcDest.top = npcinfo[i].bodysection[f].y - animset9[s].yofs + 2;

						anims[9]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

				}


				//  boss 1
				if (npcinfo[i].spriteset == 3) {
					if (npcinfo[i].attacking == 0) {
						int cframe = npcinfo[i].cframe;

						rcSrc.left = (int)(cframe / 4) * 24;
						rcSrc.top = 0;
						rcSrc.setWidth(24);
						rcSrc.setHeight(48);

						rcDest.left = npx - 2;
						rcDest.top = npy - 24;

						anims[3]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					} else {
						rcSrc.left = 4 * 24;
						rcSrc.top = 0;
						rcSrc.setWidth(24);
						rcSrc.setHeight(48);

						rcDest.left = npx - 2;
						rcDest.top = npy - 24;

						anims[3]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

				}

				// black knight
				if (npcinfo[i].spriteset == 4) {
					if (npcinfo[i].attacking == 0) {
						int cframe = npcinfo[i].cframe;

						rcSrc.left = (int)(cframe / 4) * 24;
						rcSrc.top = 0;
						rcSrc.setWidth(24);
						rcSrc.setHeight(48);

						rcDest.left = npx - 2;
						rcDest.top = npy - 24;

						anims[4]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					} else {
						rcSrc.left = 4 * 24;
						rcSrc.top = 0;
						rcSrc.setWidth(24);
						rcSrc.setHeight(48);

						rcDest.left = npx - 2;
						rcDest.top = npy - 24;

						anims[4]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}
				}


				// firehydra
				if (npcinfo[i].spriteset == 5) {
					for (int ff = 0; ff <= 2; ff++) {
						if (npcinfo[i].hp > 10 * ff * 20) {
							rcSrc.left = 16 * (int)(RND() * 2);
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							rcDest.left = npcinfo[i].bodysection[10 * ff].x - 8;
							rcDest.top = npcinfo[i].bodysection[10 * ff].y - 8;

							int x = 192 + ((int)(_itemyloc + ff * 5) % 3) * 64;
							if (x > 255)
								x = 255;
							_spellimg->setAlpha(x, true);
							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
							_spellimg->setAlpha(255, true);

							for (int f = 1; f <= 8; f++) {
								rcSrc.left = 16 * (int)(RND() * 2);
								rcSrc.top = 80;
								rcSrc.setWidth(16);
								rcSrc.setHeight(16);

								rcDest.left = npcinfo[i].bodysection[ff * 10 + f].x - 8 + (int)(RND() * 3) - 1;
								rcDest.top = npcinfo[i].bodysection[ff * 10 + f].y - 8 + (int)(RND() * 3) - 1;

								x = 192 + f % 3 * 64;
								if (x > 255)
									x = 255;
								_spellimg->setAlpha(x, true);
								_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
								_spellimg->setAlpha(255, true);
							}

							rcSrc.left = 0;
							rcSrc.top = 0;
							rcSrc.setWidth(42);
							rcSrc.setHeight(36);

							rcDest.left = npcinfo[i].bodysection[10 * ff + 9].x - 21;
							rcDest.top = npcinfo[i].bodysection[10 * ff + 9].y - 21;

							_spellimg->setAlpha(192, true);
							anims[5]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
							_spellimg->setAlpha(255, true);
						}

					}

				}

				// red dragon
				if (npcinfo[i].spriteset == 6) {
					int cframe = npcinfo[i].cframe;

					rcSrc.left = (int)(cframe / 4) * 24;
					rcSrc.top = wdir * 24;
					rcSrc.setWidth(24);
					rcSrc.setHeight(24);

					rcDest.left = npx;
					rcDest.top = npy;
					rcDest.setWidth(24);
					rcDest.setHeight(24);

					if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
						npcinfo[i].shake = _ticks + 50;
						rcDest.left = rcDest.left + (int)(RND() * 3) - 1;
						rcDest.top = rcDest.top + (int)(RND() * 3) - 1;
					}

					anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				}

				// wizard
				if (npcinfo[i].spriteset == 7) {
					// if(npcinfo[i].attacking == 0) {
					int cframe = npcinfo[i].cframe;

					rcSrc.left = (int)(cframe / 4) * 24;
					rcSrc.top = wdir * 24;
					rcSrc.setWidth(24);
					rcSrc.setHeight(24);

					rcDest.left = npx;
					rcDest.top = npy;
					rcDest.setWidth(24);
					rcDest.setHeight(24);

					if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
						npcinfo[i].shake = _ticks + 50;
						rcDest.left = rcDest.left + (int)(RND() * 3) - 1;
						rcDest.top = rcDest.top + (int)(RND() * 3) - 1;
					}
					anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					// } else {
					//cframe = npcinfo[i].cattackframe;

					//rcSrc.left = (int)(cframe / 4) * 24;
					//rcSrc.top = wdir * 24;
					//rcSrc.setWidth(24);
					//rcSrc.setHeight(24);

					//rcDest.left = npx;
					//rcDest.top = npy;
					//rcDest.setWidth(24);
					//rcDest.setHeight(24);
					// animsa(sprite)->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					// }
				}


				// yellow dragon
				if (npcinfo[i].spriteset == 8) {
					int cframe = npcinfo[i].cframe;

					rcSrc.left = (int)(cframe / 4) * 24;
					rcSrc.top = wdir * 24;
					rcSrc.setWidth(24);
					rcSrc.setHeight(24);

					rcDest.left = npx;
					rcDest.top = npy;
					rcDest.setWidth(24);
					rcDest.setHeight(24);

					if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
						npcinfo[i].shake = _ticks + 50;
						rcDest.left = rcDest.left + (int)(RND() * 3) - 1;
						rcDest.top = rcDest.top + (int)(RND() * 3) - 1;
					}
					anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				}


				// dragon2
				if (npcinfo[i].spriteset == 10) {
					if (npcinfo[i].attacking == 0) {
						npcinfo[i].floating = npcinfo[i].floating + 0.25 * _fpsr;
						while (npcinfo[i].floating >= 16)
							npcinfo[i].floating = npcinfo[i].floating - 16;

						float frame = npcinfo[i].frame;
						int cframe = npcinfo[i].cframe;

						frame = frame + 0.5 * _fpsr;
						while (frame >= 16)
							frame = frame - 16;

						cframe = (int)(frame);
						if (cframe > 16)
							cframe = 16 - 1;
						if (cframe < 0)
							cframe = 0;

						npcinfo[i].frame = frame;
						npcinfo[i].cframe = cframe;

						cframe = npcinfo[i].cframe;

						rcSrc.left = 74 * wdir;
						rcSrc.top = (int)(cframe / 4) * 48;
						rcSrc.setWidth(74);
						rcSrc.setHeight(48);

						rcDest.left = npx + 12 - 37;
						rcDest.top = (float)(npy + 12 - 32 - 3 * sin(3.141592 * 2 * npcinfo[i].floating / 16));
						rcDest.setWidth(24);
						rcDest.setHeight(24);

						if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
							npcinfo[i].shake = _ticks + 50;
							rcDest.left = rcDest.left + (int)(RND() * 3) - 1;
							rcDest.top = rcDest.top + (int)(RND() * 3) - 1;
						}

						anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					} else {
						npcinfo[i].floating = npcinfo[i].floating + 0.25 * _fpsr;
						while (npcinfo[i].floating >= 16)
							npcinfo[i].floating = npcinfo[i].floating - 16;

						int cframe = npcinfo[i].cattackframe;

						rcSrc.left = 74 * wdir;
						rcSrc.top = (int)(cframe / 4) * 48;
						rcSrc.setWidth(74);
						rcSrc.setHeight(48);

						rcDest.left = npx + 12 - 37;
						rcDest.top = (float)(npy + 12 - 32 - 3 * sin(3.141592 * 2 * npcinfo[i].floating / 16));
						rcDest.setWidth(24);
						rcDest.setHeight(24);

						animsa[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}
				}

				// end boss
				if (npcinfo[i].spriteset == 11) {

					npcinfo[i].floating = npcinfo[i].floating + .3 * _fpsr;
					while (npcinfo[i].floating >= 16)
						npcinfo[i].floating = npcinfo[i].floating - 16;


					float frame = npcinfo[i].frame2;

					frame = frame + 0.5 * _fpsr;
					while (frame >= 16)
						frame = frame - 16;

					npcinfo[i].frame2 = frame;

					int sx = npx + 12 - 40;
					int sy = (float)(npy + 12 - 50 - 3 * sin(3.141592 * 2 * npcinfo[i].floating / 16));

					for (int fr = 0; fr <= 3; fr++) {
						_spellimg->setAlpha(128 + (int)(RND() * 96), true);

						rcSrc.left = 16 * (int)(RND() * 2);
						rcSrc.top = 80;
						rcSrc.setWidth(16);
						rcSrc.setHeight(16);

						rcDest.left = sx + 32 + (int)(RND() * 3) - 1;
						rcDest.top = sy - (int)(RND() * 6);

						_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

					for (int ii = 0; ii <= 8; ii++) {
						for (int i2 = 0; i2 <= 3; i2++) {
							rcSrc.left = 16 * (int)(RND() * 2);
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							float fr3 = frame - 3 + i2;
							if (fr3 < 0)
								fr3 = fr3 + 16;

							rcDest.left = (float)(sx + 36 + ii * 8 - ii * cos(3.14159 * 2 * (fr3 - ii) / 16) * 2);
							rcDest.top = (float)(sy + 16 + ii * sin(3.14159 * 2 * (fr3 - ii) / 16) * 3 - ii); //  * 4

							_spellimg->setAlpha(i2 / 3 * 224, true);

							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

							int xloc = rcDest.left;
							int yloc = rcDest.top;
							int xdif = (xloc + 8) - (_player.px + 12);
							int ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = (float)npcinfo[i].spelldamage * (1.0 + RND() * 0.5);

								if (_player.hp > 0) {
									game_damageplayer(damage);
									if (menabled && config.effects) {
										int snd = Mix_PlayChannel(sfx[sndfire]);
										Mix_Volume(snd, config.effectsvol);
									}
								}

							}


							rcDest.left = (float)(sx + 36 - ii * 8 + ii * cos(3.14159 * 2 * (fr3 - ii) / 16) * 2);
							rcDest.top = (float)(sy + 16 + ii * sin(3.14159 * 2 * (fr3 - ii) / 16) * 3 - ii); //  * 4

							_spellimg->setAlpha(i2 / 3 * 224, true);

							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

							xloc = rcDest.left;
							yloc = rcDest.top;
							xdif = (xloc + 8) - (_player.px + 12);
							ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = (float)npcinfo[i].spelldamage * (1.0 + RND() * 0.5);

								if (_player.hp > 0) {
									game_damageplayer(damage);
									if (menabled && config.effects) {
										int snd = Mix_PlayChannel(sfx[sndfire]);
										Mix_Volume(snd, config.effectsvol);
									}
								}
							}
						}
					}

					_spellimg->setAlpha(255, true);

					if (npcinfo[i].attacking == 0) {
						int cframe = (int)(frame);
						rcSrc.left = 0;
						rcSrc.top = 72 * (int)(cframe / 4);
						rcSrc.setWidth(80);
						rcSrc.setHeight(72);

						rcDest.left = sx;
						rcDest.top = sy;

						if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
							npcinfo[i].shake = _ticks + 50;
							rcDest.left = rcDest.top + (int)(RND() * 3) - 1;
							rcDest.left = rcDest.top + (int)(RND() * 3) - 1;
						}

						anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					} else {
						int cframe = (int)(npcinfo[i].cattackframe);

						rcSrc.left = 0;
						rcSrc.top = 72 * (int)(cframe / 4);
						rcSrc.setWidth(80);
						rcSrc.setHeight(72);

						rcDest.left = sx;
						rcDest.top = sy;

						animsa[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}
				}

				// bat kitty
				if (npcinfo[i].spriteset == 12) {
					npcinfo[i].floating = npcinfo[i].floating + 1 * _fpsr;
					while (npcinfo[i].floating >= 16)
						npcinfo[i].floating = npcinfo[i].floating - 16;

					float frame = npcinfo[i].frame;
					int cframe = npcinfo[i].cframe;

					frame = frame + 0.5 * _fpsr;
					while (frame >= 16)
						frame = frame - 16;

					cframe = (int)(frame);
					if (cframe > 16)
						cframe = 16 - 1;
					if (cframe < 0)
						cframe = 0;

					npcinfo[i].frame = frame;
					npcinfo[i].cframe = cframe;

					cframe = npcinfo[i].cframe;

					rcSrc.left = 0;
					rcSrc.top = 0;
					rcSrc.setWidth(99);
					rcSrc.setHeight(80);

					rcDest.left = npx + 12 - 50;
					rcDest.top = (float)(npy + 12 - 64 + 2 * sin(3.141592 * 2 * npcinfo[i].floating / 16));
					rcDest.setWidth(99);
					rcDest.setHeight(80);

					if (npcinfo[i].pause > _ticks && npcinfo[i].shake < _ticks) {
						npcinfo[i].shake = _ticks + 50;
						rcDest.left = rcDest.left + (int)(RND() * 3) - 1;
						rcDest.top = rcDest.top + (int)(RND() * 3) - 1;
					}

					anims[sprite]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				}

				rcDest.left = npx + 4;
				rcDest.top = npy + 22;
				rcDest.setWidth(16);
				rcDest.setHeight(4);

				_videobuffer->fillRect(rcDest, 0);

				rcDest.left = npx + 5;
				rcDest.top = npy + 23;


				int ww = 14 * npcinfo[i].hp / npcinfo[i].maxhp;
				if (ww > 14)
					ww = 14;
				if (ww < 1)
					ww = 1;

				rcDest.setWidth(ww);
				rcDest.setHeight(2);


				_videobuffer->fillRect(rcDest, ccc);

				int pass = 1;

				if (npcinfo[i].spriteset == 3)
					pass = 0;
				if (pass == 1)
					game_drawover(npx, npy);

			}
		}
	}
}

void GriffonEngine::game_drawover(int modx, int mody) {
	int npx = modx + 12;
	int npy = mody + 20;

	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	for (int xo = -1; xo <= 1; xo++) {
		for (int yo = -1; yo <= 1; yo++) {

			int sx = lx + xo;
			int sy = ly + yo;

			int sx2 = sx * 16;
			int sy2 = sy * 16;

			if (sx > -1 && sx < 40 && sy > -1 && sy < 24) {

				int curtile = tileinfo[2][sx][sy][0];
				int curtilel = tileinfo[2][sx][sy][1];

				if (curtile > 0) {
					curtile = curtile - 1;
					int curtilex = curtile % 20;
					int curtiley = (curtile - curtilex) / 20;

					rcSrc.left = curtilex * 16;
					rcSrc.top = curtiley * 16;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					rcDest.left = sx2;
					rcDest.top = sy2;
					rcDest.setWidth(16);
					rcDest.setHeight(16);

					int pass = 1;
					if (curtilel == 1) {
						for (int ff = 0; ff <= 5; ff++) {
							int ffa = 20 * 5 - 1 + ff * 20;
							int ffb = 20 * 5 + 4 + ff * 20;
							if (curtile > ffa && curtile < ffb)
								pass = 0;
						}
					}

					if (pass == 1)
						tiles[curtilel]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				}
			}
		}
	}
}

void GriffonEngine::game_drawplayer() {
	int f = 0;
	if (_player.armour == 3)
		f = 13;

	if (!attacking) {
		rcSrc.left = (int)(_player.walkframe / 4) * 24;
		rcSrc.top = _player.walkdir * 24;
		rcSrc.setWidth(24);
		rcSrc.setHeight(24);

		rcDest.left = (int)(_player.px);
		rcDest.top = (int)(_player.py);
		rcDest.setWidth(24);
		rcDest.setHeight(24);

		anims[f]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
	} else {
		rcSrc.left = (int)(_player.attackframe / 4) * 24;
		rcSrc.top = _player.walkdir * 24;
		rcSrc.setWidth(24);
		rcSrc.setHeight(24);

		rcDest.left = (int)(_player.px);
		rcDest.top = (int)(_player.py);
		rcDest.setWidth(24);
		rcDest.setHeight(24);

		animsa[f]->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

	}

	long ccc = _videobuffer->format.RGBToColor(224, 224, 64);

	int pass = 0;
	if (_player.hp <= _player.maxhp * 0.25)
		pass = 1;

	if (pass == 1) {
		ccc = _videobuffer->format.RGBToColor(255, 255, 255);
		if ((int)(_player.hpflash) == 1)
			ccc = _videobuffer->format.RGBToColor(255, 0, 0);
	}

	int sss = 6;
	if (_player.foundspell[0])
		sss = 8;
	int npx = _player.px;
	int npy = _player.py;
	rcDest.left = npx + 4;
	rcDest.top = npy + 22;
	rcDest.setWidth(16);
	rcDest.setHeight(sss);

	_videobuffer->fillRect(rcDest, 0);

	rcDest.left = npx + 5;
	rcDest.top = npy + 23;


	int ww = 14 * _player.hp / _player.maxhp;
	if (ww > 14)
		ww = 14;
	if (ww < 1)
		ww = 1;

	rcDest.setWidth(ww);
	rcDest.setHeight(2);

	_videobuffer->fillRect(rcDest, ccc);

	ccc = _videobuffer->format.RGBToColor(0, 224, 64);
	if (ABS(_player.attackstrength - 100) < kEpsilon)
		ccc = _videobuffer->format.RGBToColor(255, 128, 32);

	ww = 14 * _player.attackstrength / 100;
	if (ww > 14)
		ww = 14;

	int ww2 = 14 * _player.spellstrength / 100;
	if (ww2 > 14)
		ww2 = 14;

	rcDest.top = rcDest.top + 2;
	rcDest.setWidth(ww);
	rcDest.setHeight(2);

	_videobuffer->fillRect(rcDest, ccc);

	ccc = _videobuffer->format.RGBToColor(128, 0, 224);
	if (ABS(_player.spellstrength - 100) < kEpsilon)
		ccc = _videobuffer->format.RGBToColor(224, 0, 0);

	rcDest.top = rcDest.top + 2;
	rcDest.setWidth(ww2);
	rcDest.setHeight(2);

	_videobuffer->fillRect(rcDest, ccc);
}

void GriffonEngine::game_drawview() {
	_videobuffer->copyRectToSurface(_mapbg->getPixels(), _mapbg->pitch, 0, 0, _mapbg->w, _mapbg->h);

	game_updspellsunder();

	game_drawanims(0);

	// ------dontdrawover = special case to make boss work right in room 24
	if (dontdrawover == 1)
		game_drawanims(1);
	game_drawnpcs(0);

	game_drawplayer();

	game_drawnpcs(1);
	if (dontdrawover == 0)
		game_drawanims(1);

	game_drawover((int)_player.px, (int)_player.py);

	game_updspells();

	if (cloudson == 1) {
		Common::Rect rc;
		rc.left = (float)(256 + 256 * cos(3.141592 / 180 * clouddeg));
		rc.top = (float)(192 + 192 * sin(3.141592 / 180 * clouddeg));
		rc.setWidth(320);
		rc.setHeight(240);

		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rc);
	}

	game_drawhud();

	g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
}

void GriffonEngine::game_endofgame() {
	float xofs = 0;
	_ticks = g_system->getMillis();

	float spd = 0.2f;

	if (menabled && config.music) {
		Mix_HaltChannel(-1);
		musicchannel = Mix_PlayChannel(_mendofgame, true);
		Mix_Volume(musicchannel, 0);
	}

	int _ticks1 = _ticks;
	int ya = 0;

	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);
	_videobuffer2->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

	float ld = 0;
	int ldstop = 0;

	do {
		ld = ld + 4 * _fpsr;
		if (ld > config.musicvol)
			ld = config.musicvol;
		if (menabled && ldstop == 0) {
			Mix_Volume(musicchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = 1;
		}

		ya = 0;
		if (_ticks < _ticks1 + 1500) {
			ya = (255 * (_ticks - _ticks1)) / 1500;
			if (ya < 0)
				ya = 0;
			if (ya > 255)
				ya = 255;
		} else {
			break;
		}

		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		_videobuffer->setAlpha(ya);
		_videobuffer3->copyRectToSurface(_videobuffer2->getPixels(), _videobuffer2->pitch, 0, 0, _videobuffer2->w, _videobuffer2->h);
		_videobuffer3->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

		g_system->copyRectToScreen(_videobuffer3->getPixels(), _videobuffer3->pitch, 0, 0, _videobuffer3->w, _videobuffer3->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	} while (1);

	_ticks1 = _ticks;
	ya = 0;
	float y = 140;

	do {
		Common::Rect rc;

		rc.left = -xofs;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		y = y - spd * _fpsr;
		for (int i = 0; i <= 26; i++) {
			int yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				int x = 160 - strlen(story2[i]) * 4;
				sys_print(_videobuffer, story2[i], x, yy, 4);
			}

			if (yy < 10 && i == 25)
				break;
		}

		ya = 255;
		if (_ticks < _ticks1 + 1000) {
			ya = 255 * (_ticks - _ticks1) / 1000;
			if (ya < 0)
				ya = 0;
			if (ya > 255)
				ya = 255;
		}

		_videobuffer->setAlpha(ya);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs = xofs + add;
		if (xofs >= 320)
			xofs = xofs - 320;

		if (_event.type == Common::EVENT_KEYDOWN)
			spd = 1.0f;
		if (_event.type == Common::EVENT_KEYUP)
			spd = 0.2f;

		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
			break;
	} while (1);


	_ticks1 = _ticks;
	int y1 = 0;

	_videobuffer2->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

	do {
		if (_ticks < _ticks1 + 1500) {
			y1 = 255 * (_ticks - _ticks1) / 1500;
			if (y1 < 0)
				y1 = 0;
			if (y1 > 255)
				y1 = 255;
			else
				break;
		}

		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		_videobuffer->setAlpha(y1);
		_videobuffer2->blit(*_videobuffer3);
		_videobuffer->blit(*_videobuffer3);

		g_system->copyRectToScreen(_videobuffer3->getPixels(), _videobuffer3->pitch, 0, 0, _videobuffer3->w, _videobuffer3->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	} while (1);


	int keywait = 2000 + _ticks;

	_ticks1 = _ticks;
	y1 = 0;
	do {

		_videobuffer->copyRectToSurface(_theendimg->getPixels(), _theendimg->pitch, 0, 0, _theendimg->w, _theendimg->h);

		y1 = 255;
		if (_ticks < _ticks1 + 1000) {
			y1 = 255 * (_ticks - _ticks1) / 1000;
			if (y1 < 0)
				y1 = 0;
			if (y1 > 255)
				y1 = 255;
		}

		_videobuffer->setAlpha(y1);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN && keywait < _ticks)
			break;

	} while (1);

	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	game_theend();

}

void GriffonEngine::game_eventtext(const char *stri) {
	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	int x = 160 - 4 * strlen(stri);

	_ticks = g_system->getMillis();
	int pause_ticks = _ticks + 500;
	int b_ticks = _ticks;

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	do {
		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN && pause_ticks < _ticks)
			break;
		_videobuffer2->blit(*_videobuffer);

		int fr = 192;

		if (pause_ticks > _ticks)
			fr = 192 * (_ticks - b_ticks) / 500;
		if (fr > 192)
			fr = 192;

		_windowimg->setAlpha(fr, true);

		_windowimg->blit(*_videobuffer);
		if (pause_ticks < _ticks)
			sys_print(_videobuffer, stri, x, 15, 0);

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
	} while (1);

	_videobuffer3->blit(*_videobuffer);

	_itemticks = _ticks + 210;
}


void GriffonEngine::game_handlewalking() {
	unsigned int *temp/*, c*/, bgc;
	float spd, /*ppx, ppy,*/ px, py, opx, opy;
	float nx, ny, npx, npy;

	int xmax = 20 * 16 - 25;
	int ymax = 15 * 16 - 25;

	px = _player.px;
	py = _player.py;
	opx = px;
	opy = py;

	spd = _player.walkspd * _fpsr;

	nx = (px / 2 + 6);
	ny = (py / 2 + 10);

	npx = px + 12;
	npy = py + 20;
	int lx = (int)npx / 16;
	int ly = (int)npy / 16;

	int ramp = _rampdata[lx][ly];
	if (ramp == 1 && movingup)
		spd = spd * 2;
	if (ramp == 1 && movingdown)
		spd = spd * 2;

	if (ramp == 2 && movingleft)
		movingup = 1;
	if (ramp == 2 && movingright)
		movingdown = 1;

	if (ramp == 3 && movingright)
		movingup = 1;
	if (ramp == 3 && movingleft)
		movingdown = 1;

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			int sx = nx + x;
			int sy = ny + y;

			_clipsurround[x + 1][y + 1] = 0;
			if (sx > -1 && sx < 320 && sy > -1 && sy < 192) {
				temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				_clipsurround[x + 1][y + 1] = *temp;
			}
		}
	}

	if (movingup)
		_player.walkdir = 0;
	if (movingdown)
		_player.walkdir = 1;
	if (movingleft)
		_player.walkdir = 2;
	if (movingright)
		_player.walkdir = 3;

	if (movingup && _clipsurround[1][0] == 0) {
		py = py - spd;
		_player.walkdir = 0;
	} else if (movingup && _clipsurround[1][0] > 0) {
		// move upleft
		if (movingright == 0 && _clipsurround[0][0] == 0) {
			py = py - spd;
			px = px - spd;
		}

		// move upright
		if (movingleft == 0 && _clipsurround[2][0] == 0) {
			py = py - spd;
			px = px + spd;
		}
	}
	if (movingdown && _clipsurround[1][2] == 0) {
		py = py + spd;
		_player.walkdir = 1;
	} else if (movingdown && _clipsurround[1][2] > 0) {
		// move downleft
		if (movingright == 0 && _clipsurround[0][2] == 0) {
			py = py + spd;
			px = px - spd;
		}

		// move downright
		if (movingleft == 0 && _clipsurround[2][2] == 0) {
			py = py + spd;
			px = px + spd;
		}
	}
	if (movingleft && _clipsurround[0][1] == 0) {
		px = px - spd;
		_player.walkdir = 2;
	} else if (movingleft && _clipsurround[0][1] > 0) {
		// move leftup
		if (movingdown == 0 && _clipsurround[0][0] == 0) {
			py = py - spd;
			px = px - spd;
		}

		// move leftdown
		if (movingup == 0 && _clipsurround[0][2] == 0) {
			py = py + spd;
			px = px - spd;
		}
	}
	if (movingright && _clipsurround[2][1] == 0) {
		px = px + spd;
		_player.walkdir = 3;
	} else if (movingright && _clipsurround[2][1] > 0) {
		// move rightup
		if (movingdown == 0 && _clipsurround[2][0] == 0) {
			px = px + spd;
			py = py - spd;
		}

		// move rightdown
		if (movingup == 0 && _clipsurround[2][2] == 0) {
			py = py + spd;
			px = px + spd;
		}
	}

	if (px < -8)
		px = -8;
	if (px > xmax)
		px = xmax;
	if (py < -8)
		py = -8;
	if (py > ymax)
		py = ymax;

	int pass = 1;

	int sx = (px / 2 + 6);
	int sy = (py / 2 + 10);
	temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
	bgc = *temp;
	if (bgc > 0 && bgc != 1000) {
		px = opx;
		py = opy;
		pass = 0;
	}

	// push npc
	if (pass == 1) {
		for (int i = 1; i <= lastnpc; i++) {
			if (npcinfo[i].hp > 0) {
				npx = npcinfo[i].x;
				npy = npcinfo[i].y;

				opx = npx;
				opy = npy;

				int xdif = _player.px - npx;
				int ydif = _player.py - npy;

				if (_player.walkdir == 0) {
					if (abs(xdif) <= 8 && ydif > 0 && ydif < 8)
						npcinfo[i].y = npcinfo[i].y - spd;
				} else if (_player.walkdir == 1) {
					if (abs(xdif) <= 8 && ydif < 0 && ydif > -8)
						npcinfo[i].y = npcinfo[i].y + spd;
				} else if (_player.walkdir == 2) {
					if (abs(ydif) <= 8 && xdif > 0 && xdif < 8)
						npcinfo[i].x = npcinfo[i].x - spd;
				} else if (_player.walkdir == 3) {
					if (abs(ydif) <= 8 && xdif < 0 && xdif > -8)
						npcinfo[i].x = npcinfo[i].x + spd;
				}

				npx = npcinfo[i].x;
				npy = npcinfo[i].y;

				sx = (int)(npx / 2 + 6);
				sy = (int)(npy / 2 + 10);
				temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				bgc = *temp;

				if (bgc > 0) {
					npcinfo[i].x = opx;
					npcinfo[i].y = opy;
				}
			}
		}
	}

	_player.opx = _player.px;
	_player.opy = _player.py;
	_player.px = px;
	_player.py = py;

	if (_player.px != _player.opx || _player.py != _player.opy)
		_player.walkframe = _player.walkframe + _animspd * _fpsr;
	if (_player.walkframe >= 16)
		_player.walkframe = _player.walkframe - 16;

	// walking over items to pickup :::
	int o = objmap[lx][ly];

	if (o > -1) {
		// fsk
		if (objectinfo[o][4] == 2 && _player.inventory[INV_FLASK] < 9) {
			objmap[lx][ly] = -1;

			_player.inventory[INV_FLASK]++;
			game_addFloatIcon(6, lx * 16, ly * 16);

			objmapf[_curmap][lx][ly] = 1;

			if (menabled && config.effects) {
				int snd = Mix_PlayChannel(sfx[sndpowerup]);
				Mix_Volume(snd, config.effectsvol);
			}
		}

		if (objectinfo[o][5] == 7 && _player.inventory[INV_DOUBLEFLASK] < 9) {
			objmap[lx][ly] = -1;

			_player.inventory[INV_DOUBLEFLASK]++;
			game_addFloatIcon(12, lx * 16, ly * 16);

			objmapf[_curmap][lx][ly] = 1;

			if (menabled && config.effects) {
				int snd = Mix_PlayChannel(sfx[sndpowerup]);
				Mix_Volume(snd, config.effectsvol);
			}
		}

		if (objectinfo[o][5] == 9 && _player.inventory[INV_SHOCK] < 9 && (_curmap == 41 && _scriptflag[9][1] == 0)) {
			objmap[lx][ly] = -1;

			_player.inventory[INV_SHOCK]++;
			game_addFloatIcon(17, lx * 16, ly * 16);

			objmapf[_curmap][lx][ly] = 1;
			if (_curmap == 41)
				_scriptflag[9][1] = 1;

			if (menabled && config.effects) {
				int snd = Mix_PlayChannel(sfx[sndpowerup]);
				Mix_Volume(snd, config.effectsvol);
			}

		}

		if (objectinfo[o][5] == 9 && _player.inventory[INV_SHOCK] < 9) {
			objmap[lx][ly] = -1;

			_player.inventory[INV_SHOCK]++;
			game_addFloatIcon(17, lx * 16, ly * 16);

			objmapf[_curmap][lx][ly] = 1;

			if (menabled && config.effects) {
				int snd = Mix_PlayChannel(sfx[sndpowerup]);
				Mix_Volume(snd, config.effectsvol);
			}

		}
	}
}

void GriffonEngine::game_loadmap(int mapnum) {
	unsigned int ccc;
	Common::Rect trect(320, 240);
	char name[256];
	int tempmap[320][200];

	debug(2, "Loaded map %d", mapnum);

	ccc = _clipbg->format.RGBToColor(255, 255, 255);

	_curmap = mapnum;

	_mapbg->fillRect(trect, 0);
	_clipbg->fillRect(trect, ccc);
	_clipbg2->fillRect(trect, ccc);

	_forcepause = false;
	cloudson = 0;
	if (mapnum < 6)
		cloudson = 1;
	if (mapnum > 41)
		cloudson = 1;
	if (mapnum > 47)
		cloudson = 0;
	if (mapnum == 52)
		cloudson = 1;
	if (mapnum == 60)
		cloudson = 1;
	if (mapnum == 50)
		cloudson = 1;
	if (mapnum == 54)
		cloudson = 1;
	if (mapnum == 58)
		cloudson = 1;
	if (mapnum == 62)
		cloudson = 1;
	if (mapnum == 83)
		cloudson = 1;

	// -----------special case
	dontdrawover = 0;
	if (mapnum == 24)
		dontdrawover = 1;

	if ((mapnum == 53 || mapnum == 57 || mapnum == 61 || mapnum == 65 || mapnum == 62) && _scriptflag[60][0] > 0)
		mapnum = mapnum + 100;
	if ((mapnum == 161 || mapnum == 162) && _scriptflag[60][0] == 2)
		mapnum = mapnum + 100;

	for (int i = 0; i < kMaxSpell; i++)
		spellinfo[i].frame = 0;

	_roomlock = 0;

	// read *.map file
	sprintf(name, "mapdb/%04i.map", mapnum);
	debug(1, "Reading %s", name);

	Common::File file;
	file.open(name);
	for (int x = 0; x <= 319; x++)
		for (int y = 0; y <= 199; y++) {
			INPUT("%i", &tempmap[x][y]);
		}
	file.close();

	for (int x = 0; x <= 319; x++)
		for (int y = 0; y <= 239; y++) {
			triggerloc[x][y] = -1;
		}

	// read *.trg file
	sprintf(name, "mapdb/%04i.trg", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	INPUT("%i", &ntriggers);

	for (int i = 0; i < ntriggers; i++) {
		int mapx, mapy, trig;

		INPUT("%i", &mapx);
		INPUT("%i", &mapy);
		INPUT("%i", &trig);

		triggerloc[mapx][mapy] = trig;
	}
	file.close();


	for (int y = 0; y <= 23; y++)
		for (int x = 0; x <= 39; x++)
			_rampdata[x][y] = tempmap[3 * 40 + x][y + 40];


	for (int y = 0; y <= 23; y++)
		for (int x = 0; x <= 39; x++)
			for (int l = 0; l <= 2; l++)
				for (int a = 0; a <= 2; a++)
					tileinfo[l][x][y][a] = 0;

	if (_scriptflag[4][0] == 1 && _curmap == 4) {
		triggerloc[9][7] = 5004;
		tempmap[9][7] = 41;
		tempmap[9][7 + 40] = 0;
	}

	for (int y = 0; y <= 23; y++) {
		for (int x = 0; x <= 39; x++) {
			for (int l = 0; l <= 2; l++) {
				int ly = y;
				int lx = x + l * 40;

				// tile
				int curtile = tempmap[lx][ly];
				int curtilelayer = tempmap[lx][ly + 40];

				if (curtile > 0) {
					curtile = curtile - 1;
					int curtilel = curtilelayer;
					int curtilex = curtile % 20;
					int curtiley = (curtile - curtilex) / 20;

					tileinfo[l][x][y][0] = curtile + 1;
					tileinfo[l][x][y][1] = curtilelayer;

					rcSrc.left = curtilex * 16;
					rcSrc.top = curtiley * 16;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					rcDest.left = x * 16;
					rcDest.top = y * 16;
					rcDest.setWidth(16);
					rcDest.setHeight(16);

					if (l == 2 && curtilel == 1) {
						for (int ff = 0; ff <= 5; ff++) {
							int ffa = 20 * 5 - 1 + ff * 20;
							int ffb = 20 * 5 + 4 + ff * 20;
							if (curtile > ffa && curtile < ffb) {
								tiles[curtilel]->setAlpha(128, true);
							}
						}
					}
					if (l == 1 && curtilel == 2) {
						for (int ff = 0; ff <= 4; ff++) {
							int ffa = 20 * (5 + ff) + 3;
							if (curtile == ffa) {
								tiles[curtilel]->setAlpha(192, true);
							}
						}
					}

					tiles[curtilel]->blit(*_mapbg, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					tiles[curtilel]->setAlpha(255, true);

					rcDest.left = x * 8;
					rcDest.top = y * 8;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					_clipbg->fillRect(rcDest, 0);
				}

			}
		}
	}


	for (int x = 0; x <= 39; x++) {
		for (int y = 0; y <= 23; y++) {
			int d = tempmap[3 * 40 + x][y];

			if (_scriptflag[4][0] == 1 && x == 9 && y == 7)
				d = 99;

			if (d > 0) {
				int clip = d % 2;
				d = (d - clip) / 2;
				int npc = d % 2;
				d = (d - npc) / 2;

				if (d == 99 && x == 9 && y == 7)
					clip = 1;

				if (clip) {
					if (d != 99)
						d = tempmap[6 * 40 + x][y];
					if (d == 99)
						d = 1;

					int x1 = x * 8;
					int y1 = y * 8;

					if (d == 1) {
						for (int i = 0; i <= 7; i++) {
							sys_line(_clipbg, x1, y1 + i, x1 + 7 - i, y1 + i, ccc);
						}
					} else if (d == 2) {
						sys_line(_clipbg, x1, y1, x1 + 7, y1, ccc);
						sys_line(_clipbg, x1, y1 + 1, x1 + 7, y1 + 1, ccc);
					} else if (d == 3) {
						for (int i = 0; i <= 7; i++) {
							sys_line(_clipbg, x1 + i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					} else if (d == 4) {
						sys_line(_clipbg, x1, y1, x1, y1 + 7, ccc);
						sys_line(_clipbg, x1 + 1, y1, x1 + 1, y1 + 7, ccc);
					} else if (d == 5) {
						rcDest.left = x1;
						rcDest.top = y1;
						rcDest.setWidth(8);
						rcDest.setHeight(8);
						_clipbg->fillRect(rcDest, ccc);
					} else if (d == 6) {
						sys_line(_clipbg, x1 + 7, y1, x1 + 7, y1 + 7, ccc);
						sys_line(_clipbg, x1 + 6, y1, x1 + 6, y1 + 7, ccc);
					} else if (d == 7) {
						for (int i = 0; i <= 7; i++) {
							sys_line(_clipbg, x1, y1 + i, x1 + i, y1 + i, ccc);
						}
					} else if (d == 8) {
						sys_line(_clipbg, x1, y1 + 7, x1 + 7, y1 + 7, ccc);
						sys_line(_clipbg, x1, y1 + 7, x1 + 6, y1 + 6, ccc);
					} else if (d == 9) {
						for (int i = 0; i <= 7; i++) {
							sys_line(_clipbg, x1 + 7 - i, y1 + i, x1 + 7, y1 + i, ccc);
						}
					}
				}
			}
		}
	}

	lastobj = 0;
	lastnpc = 0;

	for (int i = 0; i < kMaxNPC; i++)
		npcinfo[i].onmap = 0;

	for (int x = 0; x <= 19; x++) {
		for (int y = 0; y <= 19; y++) {
			int d = tempmap[3 * 40 + x][y];

			int npc = 0;
			int obj = 0;
			if (d > 0) {
				int clip = d % 2;
				d = (d - clip) / 2;
				npc = d % 2;
				d = (d - npc) / 2;
				obj = d % 2;
			}

			objmap[x][y] = -1;

			if (obj == 1) {

				int o = tempmap[5 * 40 + x][y];

				if (objmapf[_curmap][x][y] == 0) {
					objmap[x][y] = o;

					if (objectinfo[o][0] > 1) {
						if (o > lastobj)
							lastobj = o;
					}

					int x1 = x * 8;
					int y1 = y * 8;

					rcDest.left = x1;
					rcDest.top = y1;
					rcDest.setWidth(8);
					rcDest.setHeight(8);

					if (objectinfo[o][4] == 1)
						_clipbg->fillRect(rcDest, ccc);
					if (objectinfo[o][4] == 3)
						_clipbg->fillRect(rcDest, ccc);
				}
			}
			if (npc == 1) {
				int o = tempmap[4 * 40 + x][y];

				if (o > lastnpc)
					lastnpc = o;

				npcinfo[o].x = x * 16 - 4;
				npcinfo[o].y = y * 16 - 5;

				npcinfo[o].walkdir = 1;
				npcinfo[o].onmap = 1;
			}
		}
	}


	if (_curmap == 62 && _scriptflag[8][0] > 0)
		lastnpc = 0;
	if (_curmap == 73 && _scriptflag[12][0] > 0)
		lastnpc = 0;
	if (_curmap == 81 && _scriptflag[13][0] > 0)
		lastnpc = 0;

	if (_curmap == 73 && _scriptflag[12][0] == 0)
		_roomlock = 1;
	if (_curmap == 81 && _scriptflag[13][0] == 0)
		_roomlock = 1;
	if (_curmap == 83 && _scriptflag[15][0] == 0)
		_roomlock = 1;
	if (_curmap == 82)
		_roomlock = 1;

	// read *.npc file
	sprintf(name, "mapdb/%04i.npc", mapnum);
	debug(1, "Reading %s", name);
	file.open(name);

	for (int i = 0; i < kMaxNPC; i++) {
		INPUT("%i", &npcinfo[i].spriteset);
		INPUT("%i", &npcinfo[i].x1);
		INPUT("%i", &npcinfo[i].y1);
		INPUT("%i", &npcinfo[i].x2);
		INPUT("%i", &npcinfo[i].y2);
		INPUT("%i", &npcinfo[i].movementmode);
		INPUT("%i", &npcinfo[i].hp);
		INPUT("%i", &npcinfo[i].item1);
		INPUT("%i", &npcinfo[i].item2);
		INPUT("%i", &npcinfo[i].item3);
		INPUT("%i", &npcinfo[i].script);

		// baby dragon
		if (npcinfo[i].spriteset == 1) {
			npcinfo[i].hp = 12;
			npcinfo[i].attackdelay = 2000;

			npcinfo[i].attackdamage = 2;
			npcinfo[i].spelldamage = 0;

			npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				npcinfo[i].hp = 0;
		}

		// onewing
		if (npcinfo[i].spriteset == 2) {
			npcinfo[i].hp = 200;
			npcinfo[i].attackdelay = 2000;
			npcinfo[i].swayspd = 1;

			npcinfo[i].attackdamage = 24;
			npcinfo[i].spelldamage = 30;

			npcinfo[i].walkspd = 1.4f;
			npcinfo[i].castpause = _ticks;
		}

		// boss1
		if (npcinfo[i].spriteset == 3) {
			npcinfo[i].hp = 300;
			npcinfo[i].attackdelay = 2200;

			npcinfo[i].attackdamage = 0;
			npcinfo[i].spelldamage = 30;

			npcinfo[i].walkspd = 1.2f;
		}

		// black knights
		if (npcinfo[i].spriteset == 4) {
			npcinfo[i].hp = 200;
			npcinfo[i].attackdelay = 2800;

			npcinfo[i].attackdamage = 0;
			npcinfo[i].spelldamage = 30;

			npcinfo[i].walkspd = 1;
		}

		// boss2 firehydra
		if (npcinfo[i].spriteset == 5) {
			npcinfo[i].hp = 600;
			npcinfo[i].attackdelay = 2200;

			npcinfo[i].attackdamage = 50;
			npcinfo[i].spelldamage = 30;

			npcinfo[i].walkspd = 1.3f;

			npcinfo[i].swayangle = 0;
		}

		// baby fire dragon
		if (npcinfo[i].spriteset == 6) {
			npcinfo[i].hp = 20;
			npcinfo[i].attackdelay = 1500;

			npcinfo[i].attackdamage = 0;
			npcinfo[i].spelldamage = 12;

			npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				npcinfo[i].hp = 0;
		}

		// priest1
		if (npcinfo[i].spriteset == 7) {
			npcinfo[i].hp = 40;
			npcinfo[i].attackdelay = 5000;

			npcinfo[i].attackdamage = 0;
			npcinfo[i].spelldamage = 8;

			npcinfo[i].walkspd = 1;

			if (RND() * 8 == 0)
				npcinfo[i].hp = 0;
		}

		// yellow fire dragon
		if (npcinfo[i].spriteset == 8) {
			npcinfo[i].hp = 100;
			npcinfo[i].attackdelay = 1500;

			npcinfo[i].attackdamage = 0;
			npcinfo[i].spelldamage = 24;

			npcinfo[i].walkspd = 1;

			if (RND() * 5 == 0)
				npcinfo[i].hp = 0;
		}

		// twowing
		if (npcinfo[i].spriteset == 9) {
			npcinfo[i].hp = 140;
			npcinfo[i].attackdelay = 2000;
			npcinfo[i].swayspd = 1;

			npcinfo[i].attackdamage = 30;
			npcinfo[i].spelldamage = 0;

			npcinfo[i].walkspd = 1;

			npcinfo[i].castpause = 0;
		}

		// dragon2
		if (npcinfo[i].spriteset == 10) {
			npcinfo[i].hp = 80;
			npcinfo[i].attackdelay = 1500;

			npcinfo[i].attackdamage = 24;
			npcinfo[i].spelldamage = 0;

			npcinfo[i].walkspd = 1;

			npcinfo[i].floating = RND() * 16;
		}

		// end boss
		if (npcinfo[i].spriteset == 11) {
			npcinfo[i].hp = 1200;
			npcinfo[i].attackdelay = 2000;

			npcinfo[i].attackdamage = 100;
			npcinfo[i].spelldamage = 60;

			npcinfo[i].walkspd = 1;

			npcinfo[i].floating = RND() * 16;
		}

		// bat kitty
		if (npcinfo[i].spriteset == 12) {
			npcinfo[i].hp = 800;
			npcinfo[i].attackdelay = 2000;

			npcinfo[i].attackdamage = 100;
			npcinfo[i].spelldamage = 50;

			npcinfo[i].walkspd = 1;

			npcinfo[i].floating = RND() * 16;
		}

		if (npcinfo[i].onmap == 0)
			npcinfo[i].hp = 0;

		npcinfo[i].maxhp = npcinfo[i].hp;

		npcinfo[i].attacking = 0;
		npcinfo[i].attackframe = 0;
		npcinfo[i].cattackframe = 0;
		npcinfo[i].attackspd = 1.5;
		npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay * (1 + RND() * 2);

		if (npcinfo[i].spriteset == 2 || npcinfo[i].spriteset == 9) {
			npcinfo[i].bodysection[0].sprite = 0;
			npcinfo[i].bodysection[1].sprite = 1;
			npcinfo[i].bodysection[2].sprite = 2;
			npcinfo[i].bodysection[3].sprite = 3;
			npcinfo[i].bodysection[4].sprite = 4;
			npcinfo[i].bodysection[5].sprite = 3;
			npcinfo[i].bodysection[6].sprite = 3;
			npcinfo[i].bodysection[7].sprite = 5;

			npcinfo[i].bodysection[0].bonelength = 8;
			npcinfo[i].bodysection[1].bonelength = 7;
			npcinfo[i].bodysection[2].bonelength = 6;
			npcinfo[i].bodysection[3].bonelength = 4;
			npcinfo[i].bodysection[4].bonelength = 4;
			npcinfo[i].bodysection[5].bonelength = 4;
			npcinfo[i].bodysection[6].bonelength = 4;

			for (int f = 0; f <= 7; f++) {
				npcinfo[i].bodysection[f].x = npcinfo[i].x + 12;
				npcinfo[i].bodysection[f].y = npcinfo[i].y + 14;
			}

			npcinfo[i].headtargetx[0] = npcinfo[i].x + 12;
			npcinfo[i].headtargety[0] = npcinfo[i].y + 14;

		}

		if (npcinfo[i].spriteset == 5) {
			for (int f = 0; f <= 29; f++) {
				npcinfo[i].bodysection[f].x = npcinfo[i].x + 12;
				npcinfo[i].bodysection[f].y = npcinfo[i].y + 14;
			}

			for (int f = 0; f <= 2; f++) {
				npcinfo[i].headtargetx[f] = npcinfo[i].x + 12;
				npcinfo[i].headtargety[f] = npcinfo[i].y + 14;

				npcinfo[i].attacking2[f] = 0;
				npcinfo[i].attackframe2[f] = 0;
			}
		}

		if (npcinfo[i].script == 2) {
			_roomlock = 1;
			if (_scriptflag[2][0] > 0) {
				_roomlock = 0;
				npcinfo[i].hp = 0;
			}
		}

		if (npcinfo[i].script == 3) {
			_roomlock = 1;
			if (_scriptflag[3][0] > 0) {
				_roomlock = 0;
				npcinfo[i].hp = 0;
			}
		}

		if (npcinfo[i].script == 5) {
			_roomlock = 1;
			if (_scriptflag[5][0] > 0) {
				_roomlock = 0;
				npcinfo[i].hp = 0;
			}
		}

		if (npcinfo[i].script == 15) {
			_roomlock = 1;
			if (_scriptflag[15][0] > 0) {
				_roomlock = 0;
				npcinfo[i].hp = 0;
			}
		}

		npcinfo[i].pause = _ticks;
	}

	file.close();


	int cx, cy, npx, npy, lx, ly;

	// academy master key
	if (_curmap == 34 && _scriptflag[2][0] == 1) {
		cx = 9;
		cy = 7;

		objmap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// academy crystal
	if (_curmap == 24 && _player.foundspell[0] == 0 && _scriptflag[3][0] == 1) {
		cx = 9;
		cy = 7;

		objmap[cx][cy] = 6;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// gardens master key
	if (_curmap == 62 && _scriptflag[8][0] == 1) {
		cx = 13;
		cy = 7;

		objmap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// gardens fidelis sword
	if (_curmap == 66 && _scriptflag[5][0] == 1 && _player.sword == 1) {
		cx = 9;
		cy = 6;

		objmap[cx][cy] = 9;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// citadel armour
	if (_curmap == 73 && _scriptflag[12][0] == 1 && _player.armour == 1) {
		cx = 8;
		cy = 7;

		objmap[cx][cy] = 16;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	// citadel master key
	if (_curmap == 81 && _scriptflag[13][0] == 1) {
		cx = 11;
		cy = 10;

		objmap[cx][cy] = 5;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}


	// max ups
	if (_curmap == 83 && _scriptflag[15][0] == 1 && _player.sword < 3) {
		cx = 6;
		cy = 8;

		objmap[cx][cy] = 18;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	if (_curmap == 83 && _scriptflag[16][0] == 1 && _player.shield < 3) {
		cx = 9;
		cy = 8;

		objmap[cx][cy] = 19;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	if (_curmap == 83 && _scriptflag[17][0] == 1 && _player.armour < 3) {
		cx = 12;
		cy = 8;

		objmap[cx][cy] = 20;

		rcDest.left = cx * 8;
		rcDest.top = cy * 8;
		rcDest.setWidth(8);
		rcDest.setHeight(8);

		npx = _player.px + 12;
		npy = _player.py + 20;

		lx = (int)npx / 16;
		ly = (int)npy / 16;

		if (lx == cx && ly == cy)
			_player.py = _player.py + 16;

		_clipbg->fillRect(rcDest, ccc);
	}

	_clipbg2->copyRectToSurface(_clipbg->getPixels(), _clipbg->pitch, 0, 0, _clipbg->w, _clipbg->h);
}

void GriffonEngine::game_main() {
	game_title(0);

	if (_shouldQuit)
		return;

	game_saveloadnew();
}

void GriffonEngine::game_newgame() {
	float xofs = 0;
	float ld = 0, add;
	int cnt = 0;

	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	_ticks = g_system->getMillis();

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	_fpsr = 0.0;
	int y = 140;

	if (menabled && config.music) {
		Mix_HaltChannel(-1);
		musicchannel = Mix_PlayChannel(_mendofgame, true);
		Mix_Volume(musicchannel, 0);
	}

	secsingame = 0;
	secstart = 0;

	int ldstop = 0;

	do {
		Common::Rect rc;

		ld += 4 * _fpsr;
		if ((int)ld > config.musicvol)
			ld = config.musicvol;
		if (menabled && ldstop == 0) {
			Mix_Volume(musicchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = 1;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		if (++cnt >= 6) {
			cnt = 0;
			y--;
		}

		for (int i = 0; i <= 37; i++) {
			int yy, x;

			yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				x = 160 - strlen(story[i]) * 4;
				sys_print(_videobuffer, story[i], x, yy, 4);
			}

			if (yy < 10 && i == 47)
				goto __exit_do;
		}

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs += add;
		if (xofs >= 320)
			xofs = xofs - 320;

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN)
			cnt = 6;
		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
			goto __exit_do;

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		g_system->delayMillis(10);
	} while (!_shouldQuit);
__exit_do:

	if (_shouldQuit)
		return;

	_player.px = 0;
	_player.py = 0;
	_player.opx = 0;
	_player.opy = 0;
	_player.walkdir = 0;
	_player.walkframe = 0;
	_player.walkspd = 0;
	_player.attackframe = 0;
	_player.attackspd = 0;
	_player.hp = 0;
	_player.maxhp = 0;
	_player.hpflash = 0;
	_player.level = 0;
	_player.maxlevel = 0;
	_player.sword = 0;
	_player.shield = 0;
	_player.armour = 0;
	for (int i = 0; i < 5; i++) {
		_player.foundspell[i] = 0;
		_player.spellcharge[i] = 0;
		_player.inventory[i] = 0;
	}
	_player.attackstrength = 0;
	_player.spelldamage = 0;
	_player.sworddamage = 0;
	_player.exp = 0;
	_player.nextlevel = 0;

	memset(_scriptflag, 0, sizeof(_scriptflag));
	memset(objmapf, 0, sizeof(objmapf));
	memset(_roomlocks, 0, sizeof(_roomlocks));

	_roomlocks[66] = 2;
	_roomlocks[24] = 2;
	_roomlocks[17] = 1;
	_roomlocks[34] = 1;
	_roomlocks[50] = 1;
	_roomlocks[73] = 1;
	_roomlocks[82] = 2;

	_player.walkspd = 1.1f;
	_animspd = 0.5f;
	attacking = false;
	_player.attackspd = 1.5f;

	_player.sword = 1;
	_player.level = 1;
	_player.maxlevel = 22;
	_player.nextlevel = 50;
	_player.shield = 1;
	_player.armour = 1;
	_player.hp = 14;
	_player.maxhp = _player.hp;

	_player.sworddamage = _player.level * 2;
	_player.spelldamage = _player.level * 3 / 2;

	_player.px = 15 * 16 - 4;
	_player.py = 6 * 16 - 4;
	_player.walkdir = 1;

	pgardens = 0;
	ptown = 0;
	pboss = 0;
	pacademy = 0;
	pcitadel = 0;

	game_loadmap(2);

	game_playgame();
}

void GriffonEngine::game_playgame() {
	game_swash();

	if (pmenu == 1 && menabled) {
		Mix_HaltChannel(menuchannel);
		pmenu = 0;
	}

	do {
		if (!_forcepause) {
			game_updanims();
			game_updnpcs();
		}

		game_checktrigger();
		game_checkinputs();

		if (!_forcepause)
			game_handlewalking();

		game_updatey();
		game_drawview();

		game_updmusic();

		 _console->onFrame();

		sys_update();
	} while (!_shouldQuit);
}

void GriffonEngine::game_processtrigger(int trignum) {
	int trigtype = triggers[trignum][0];

	if (_roomlock == 1)
		return;
	// map jump------------------------------
	if (trigtype == 0) {
		int tx = triggers[trignum][1];
		int ty = triggers[trignum][2];
		int tmap = triggers[trignum][3];
		int tjumpstyle = triggers[trignum][4];

		if (_roomlocks[tmap] > 0) {
			if (saidlocked == 0)
				game_eventtext("Locked");
			saidlocked = 1;
			canusekey = 1;
			locktype = _roomlocks[tmap];
			roomtounlock = tmap;
			return;
		}

		if (tmap == 1) {
			if (saidjammed == 0)
				game_eventtext("Door Jammed!");
			saidjammed = 1;
			return;
		}

		saidlocked = 0;
		saidjammed = 0;

		// loc-sxy+oldmaploc
		if (tjumpstyle == 0) {

			int tsx = triggers[trignum][5];
			int tsy = triggers[trignum][6];

			_player.px += (tx - tsx) * 16;
			_player.py += (ty - tsy) * 16;

			// HACKFIX
			if (_player.px < 0)
				_player.px = 0;
			if (_player.py < 0)
				_player.py = 0;

			if (tmap > 0) {
				if (menabled && config.effects) {
					int snd = Mix_PlayChannel(sfx[snddoor]);
					Mix_Volume(snd, config.effectsvol);
				}

				game_loadmap(tmap);
				game_swash();
			}
		}
	}

	for (int i = 0; i < kMaxFloat; i++) {
		floattext[i][0] = 0;
		floaticon[i][0] = 0;
	}
}

void GriffonEngine::game_saveloadnew() {
	float y = 0.0;

	clouddeg = 0;

	_videobuffer->setAlpha(255);
	saveloadimg->setAlpha(192, true);

	int currow = 0;
	int curcol = 0;
	int lowerlock = 0;

	_ticks = g_system->getMillis();
	int _ticks1 = _ticks;
	int tickpause = _ticks + 150;

	do {
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		y = y + 1 * _fpsr;

		rcDest.left = 256 + 256 * cos(3.141592 / 180 * clouddeg * 40);
		rcDest.top = 192 + 192 * sin(3.141592 / 180 * clouddeg * 40);
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		rcDest.left = 256;
		rcDest.top = 192;
		rcDest.setWidth(320);
		rcDest.setHeight(240);

		cloudimg->setAlpha(128, true);
		cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		cloudimg->setAlpha(64, true);

		saveloadimg->blit(*_videobuffer);

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT) {
			_shouldQuit = true;
			return;
		}

		if (tickpause < _ticks) {
			if (_event.type == Common::EVENT_KEYDOWN) {
				_itemticks = _ticks + 220;

				if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
					// QUIT - non existent :)
					if (currow == 0 && curcol == 4) {
						_shouldQuit = true;
						return;
					}
					// RETURN
					if (currow == 0 && curcol == 3) {
						// reset keys to avoid returning
						// keys[SDLK_RETURN] = keys[SDLK_SPACE] = 0; // FIXME
						return;
					}
					// NEW GAME
					if (currow == 0 && curcol == 0)
						game_newgame();

					// LOAD GAME
					if (currow == 0 && curcol == 1) {
						lowerlock = 1;
						currow = 1 + _saveslot;
						tickpause = _ticks + 125;
					}
					// SAVE GAME
					if (currow == 0 && curcol == 2) {
						lowerlock = 1;
						currow = 1;
						tickpause = _ticks + 125;
					}


					if (lowerlock == 1 && curcol == 1 && tickpause < _ticks) {
						if (state_save(currow - 1)) {
							secstart = secstart + secsingame;
							secsingame = 0;
							lowerlock = 0;
							_saveslot = currow - 1;
							currow = 0;
						}
					}

					if (lowerlock == 1 && curcol == 2 && tickpause < _ticks) {
						if (state_load(currow - 1)) {
							_player.walkspd = 1.1f;
							_animspd = 0.5f;
							attacking = false;
							_player.attackspd = 1.5f;

							pgardens = 0;
							ptown = 0;
							pboss = 0;
							pacademy = 0;
							pcitadel = 0;

							Mix_HaltChannel(-1);

							secsingame = 0;
							_saveslot = currow - 1;
							game_loadmap(_curmap);
							game_playgame();
						}
					}

					tickpause = _ticks + 125;
				}

				switch (_event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (lowerlock == 0)
						return;
					lowerlock = 0;
					currow = 0;
					tickpause = _ticks + 125;
					break;
				case Common::KEYCODE_DOWN:
					if (lowerlock == 1) {
						currow = currow + 1;
						if (currow == 5)
							currow = 1;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_UP:
					if (lowerlock == 1) {
						currow = currow - 1;
						if (currow == 0)
							currow = 4;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_LEFT:
					if (lowerlock == 0) {
						curcol = curcol - 1;
						if (curcol == -1)
							curcol = 3;
						tickpause = _ticks + 125;
					}
					break;

				case Common::KEYCODE_RIGHT:
					if (lowerlock == 0) {
						curcol = curcol + 1;
						if (curcol == 4)
							curcol = 0;
						tickpause = _ticks + 125;
					}
					break;
				default:
					;
				}
			}
		}


		// savestates---------------------------------
		// read it only when needed!

		for (int ff = 0; ff <= 3; ff++) {
			state_load_player(ff);

			if (_playera.level > 0) {
				char line[256];
				int sx, sy, cc, ss, nx;

				sx = 8;
				sy = 57 + ff * 48;

				// time
				int ase = asecstart;
				int h = ((ase - (ase % 3600)) / 3600);
				ase = (ase - h * 3600);
				int m = ((ase - (ase % 60)) / 60);
				int s = (ase - m * 60);

				sprintf(line, "Game Time: %02i:%02i:%02i", h, m, s);
				sys_print(_videobuffer, line, 160 - strlen(line) * 4, sy, 0);

				sx  = 12;
				sy = sy + 11;
				cc = 0;

				sprintf(line, "Health: %i/%i", _playera.hp, _playera.maxhp);
				sys_print(_videobuffer, line, sx, sy, cc);

				if (_playera.level == 22)
					strcpy(line, "Level: MAX");
				else
					sprintf(line, "Level: %i", _playera.level);

				sys_print(_videobuffer, line, sx, sy + 11, 0);

				rcSrc.left = sx + 15 * 8 + 24;
				rcSrc.top = sy + 1;

				ss = (_playera.sword - 1) * 3;
				if (_playera.sword == 3)
					ss = 18;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				rcSrc.left = rcSrc.left + 16;
				ss = (_playera.shield - 1) * 3 + 1;
				if (_playera.shield == 3)
					ss = 19;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				rcSrc.left = rcSrc.left + 16;
				ss = (_playera.armour - 1) * 3 + 2;
				if (_playera.armour == 3)
					ss = 20;
				_itemimg[ss]->blit(*_videobuffer, rcSrc.left, rcSrc.top);

				nx = rcSrc.left + 13 + 3 * 8;
				rcSrc.left = nx - 17;

				if (_playera.foundspell[0] == 1) {
					for (int i = 0; i < 5; i++) {
						rcSrc.left = rcSrc.left + 17;
						if (_playera.foundspell[i] == 1)
							_itemimg[7 + i]->blit(*_videobuffer, rcSrc.left, rcSrc.top);
					}
				}
			} else {
				int sy = 57 + ff * 48;
				sys_print(_videobuffer, "Empty", 160 - 5 * 4, sy, 0);
			}
		}
		// ------------------------------------------


		if (currow == 0) {
			rcDest.top = 18;
			if (curcol == 0)
				rcDest.left = 10;
			if (curcol == 1)
				rcDest.left = 108;
			if (curcol == 2)
				rcDest.left = 170;
			if (curcol == 3)
				rcDest.left = 230;
			rcDest.left = (float)(rcDest.left + 2 + 2 * sin(3.14159 * 2 * _itemyloc / 16));
		}

		if (currow > 0) {
			rcDest.left = (float)(0 + 2 * sin(3.14159 * 2 * _itemyloc / 16));
			rcDest.top = (float)(53 + (currow - 1) * 48);
		}

		_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);


		if (lowerlock == 1) {
			rcDest.top = 18;
			if (curcol == 1)
				rcDest.left = 108;
			if (curcol == 2)
				rcDest.left = 170;

			// CHECKME: Useless code? or temporary commented?
			// rcDest.left = rcDest.left; // + 2 + 2 * sin(-3.14159 * 2 * _itemyloc / 16)

			_itemimg[15]->blit(*_videobuffer, rcDest.left, rcDest.top);
		}

		int yy = 255;
		if (_ticks < _ticks1 + 1000) {
			yy = 255 * (_ticks - _ticks1) / 1000;
			if (yy < 0)
				yy = 0;
			if (yy > 255)
				yy = 255;
		}

		_videobuffer->setAlpha((int)yy);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg -= 360;

		_itemyloc += 0.6 * _fpsr;
		while (_itemyloc >= 16)
			_itemyloc -= 16;

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	cloudimg->setAlpha(64, true);
}

void GriffonEngine::game_showlogos() {
	float y;
	int _ticks1;

	_ticks = g_system->getMillis();
	_ticks1 = _ticks;

	y = 0.0;

	do {
		y = 255.0;
		if (_ticks < _ticks1 + 1000) {
			y = 255.0 * ((float)(_ticks - _ticks1) / 1000.0);
			if (y < 0.0)
				y = 0.0;
			if (y > 255.0)
				y = 255.0;
		}

		if (_ticks > _ticks1 + 3000) {
			y = 255.0 - 255.0 * ((float)(_ticks - _ticks1 - 3000.0) / 1000.0);
			if (y < 0.0)
				y = 0.0;
			if (y > 255.0)
				y = 255.0;
		}

		_videobuffer->fillRect(Common::Rect(0, 0, 320, 240), 0);
		_logosimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB((int)y, (int)y, (int)y, (int)y));

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
		if (_ticks > _ticks1 + 4000)
			break;
	} while (!_shouldQuit);
}


void GriffonEngine::game_swash() {
	float y;

	y = 0;

	do {
		y = y + 1 * _fpsr;

		_videobuffer->setAlpha((int)y);
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg = clouddeg - 360;

		if (y > 10)
			break;
	} while (1);

	y = 0;
	do {
		y = y + 1 * _fpsr;

		_videobuffer->setAlpha((int)(y * 25));
		_mapbg->blit(*_videobuffer);

		if (cloudson == 1) {
			rcDest.left = (float)(256 + 256 * cos(3.141592 / 180 * clouddeg));
			rcDest.top = (float)(192 + 192 * sin(3.141592 / 180 * clouddeg));
			rcDest.setWidth(320);
			rcDest.setHeight(240);

			cloudimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, &rcDest);
		}

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		clouddeg += 0.01 * _fpsr;
		while (clouddeg >= 360)
			clouddeg = clouddeg - 360;

		if (y > 10)
			break;
	} while (1);


	_videobuffer->setAlpha(255);
}

void GriffonEngine::game_theend() {
	for (int i = 0; i < kMaxFloat; i++) {
		floattext[i][0] = 0;
		floaticon[i][0] = 0;
	}

	for (float y = 0; y < 100; y += _fpsr) {
		_videobuffer->setAlpha((int)y);
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	}

	game_title(0);
}

void GriffonEngine::game_title(int mode) {
	float xofs = 0;

	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.setWidth(320);
	rcSrc.setHeight(240);

	_videobuffer2->fillRect(rcSrc, 0);
	_videobuffer3->fillRect(rcSrc, 0);

	_ticks = g_system->getMillis();

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	int cursel = 0;
	int keypause = _ticks + 220;
	int _ticks1 = _ticks;

	if (menabled && config.music) {
		Mix_Volume(musicchannel, 0);
		Mix_Pause(musicchannel);

		menuchannel = Mix_PlayChannel(_mmenu, true);
		Mix_Volume(menuchannel, config.musicvol);
		pmenu = 1;
	}

	int ldstop = 0;

	float ld = 0;
	do {
		Common::Rect rc;

		ld += 4.0 * _fpsr;
		if (ld > config.musicvol)
			ld = config.musicvol;
		if (menabled && ldstop == 0) {
			Mix_Volume(menuchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = 1;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320.0;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = 0;
		rc.top = 0;

		_titleimg2->blit(*_videobuffer, rc.left, rc.top);

		int y = 172;
		int x = 160 - 14 * 4;

		sys_print(_videobuffer, "new game/save/load", x, y, 4);
		sys_print(_videobuffer, "options", x, y + 16, 4);
		sys_print(_videobuffer, "quit game", x, y + 32, 4);

		if (mode == 1)
			sys_print(_videobuffer, "return", x, y + 48, 4);
		else
			sys_print(_videobuffer, "(c) 2005 by Daniel 'Syn9' Kennedy", 28, 224, 4);

		rc.left = (float)(x - 16 - 4 * cos(3.14159 * 2 * _itemyloc / 16));
		rc.top = (float)(y - 4 + 16 * cursel);

		_itemimg[15]->blit(*_videobuffer, rc.left, rc.top);

		float yf = 255.0;
		if (_ticks < _ticks1 + 1000) {
			yf = 255.0 * ((float)(_ticks - _ticks1) / 1000.0);
			if (y < 0.0)
				yf = 0.0;
			if (y > 255.0)
				yf = 255.0;
		}

		_videobuffer->setAlpha((int)yf);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;

		xofs = xofs + add;
		if (xofs >= 320.0)
			xofs -= 320.0;

		_itemyloc += 0.75 * _fpsr;

		while (_itemyloc >= 16)
			_itemyloc = _itemyloc - 16;

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		if (keypause < _ticks) {
			if (_event.type == Common::EVENT_KEYDOWN) {
				keypause = _ticks + 150;

				if (_event.kbd.keycode == Common::KEYCODE_ESCAPE && mode == 1)
					break;
				else if (_event.kbd.keycode == Common::KEYCODE_UP) {
					cursel--;
					if (cursel < 0)
						cursel = (mode == 1 ? 3 : 2);
				} else if (_event.kbd.keycode == Common::KEYCODE_DOWN) {
					cursel++;
					if (cursel >= (mode == 1 ? 4 : 3))
						cursel = 0;
				} else if (_event.kbd.keycode == Common::KEYCODE_RETURN) {
					if (cursel == 0) {
						game_saveloadnew();
						_ticks = g_system->getMillis();
						keypause = _ticks + 150;
						_ticks1 = _ticks;
					} else if (cursel == 1) {
						game_configmenu();
						_ticks = g_system->getMillis();
						keypause = _ticks + 150;
						_ticks1 = _ticks;
					} else if (cursel == 2) {
						_shouldQuit = true;
					} else if (cursel == 3) {
						break;
					}
				}
			}
		}

		g_system->delayMillis(10);
	} while (!_shouldQuit);

	_itemticks = _ticks + 210;

	if (menabled && config.music) {
		Mix_HaltChannel(menuchannel);
		Mix_Resume(musicchannel);
		Mix_Volume(musicchannel, config.musicvol);
		pmenu = 0;
	}
}

void GriffonEngine::game_updanims() {
	for (int i = 0; i <= lastobj; i++) {
		int nframes = objectinfo[i][0];
		int o_animspd = objectinfo[i][3];
		float frame = objectframe[i][0];
		int cframe = objectframe[i][1];
		// objectinfo[i][6] = 0; // ?? out of bounds

		if (nframes > 1) {
			frame = frame + o_animspd / 50 * _fpsr;
			while (frame >= nframes)
				frame = frame - nframes;

			cframe = (int)frame; // truncate fractional part
			if (cframe > nframes)
				cframe = nframes - 1;
			if (cframe < 0)
				cframe = 0;

			objectframe[i][0] = frame;
			objectframe[i][1] = cframe;
		}
	}
}

void GriffonEngine::game_updatey() {
	for (int i = 0; i <= 2400; i++)
		ysort[i] = -1;

	int ff = (int)(_player.py * 10);
	if (ff < 0) // HACKFIX or ysort[yy] may go out of bounds
		ff = 0;
	_player.ysort = ff;
	ysort[ff] = 0;

	firsty = 2400;
	lasty = 0;

	for (int i = 1; i <= lastnpc; i++) {
		int yy = (int)(npcinfo[i].y * 10);

		do {
			if (ysort[yy] == -1 || yy == 2400)
				break;
			yy = yy + 1;
		} while (1);

		ysort[yy] = i;
		if (yy < firsty)
			firsty = yy;
		if (yy > lasty)
			lasty = yy;
	}
}

void GriffonEngine::game_updmusic() {
	DataChunk *iplaysound = NULL;

	if (menabled && config.music) {

		// if(_curmap > 5 && _curmap < 42) iplaysound = macademy;
		// if(_curmap > 47) iplaysound = _mgardens;
		iplaysound = _mgardens;
		if (_roomlock == 1)
			iplaysound = _mboss;

		if (iplaysound == _mboss && pboss)
			iplaysound = NULL;
		if (iplaysound == _mgardens && pgardens)
			iplaysound = NULL;

		if (iplaysound != NULL) {
			Mix_HaltChannel(musicchannel);

			pboss = 0;
			pgardens = 0;
			ptown = 0;
			pacademy = 0;
			pcitadel = 0;

			if (iplaysound == _mboss)
				pboss = 1;
			if (iplaysound == _mgardens)
				pgardens = 1;

			musicchannel = Mix_PlayChannel(iplaysound, true);
			Mix_Volume(musicchannel, config.musicvol);
		} else {
			if (!Mix_Playing(musicchannel)) {
				loopseta = loopseta + 1;
				if (loopseta == 4)
					loopseta = 0;

				if (pgardens == 1) {
					Mix_HaltChannel(musicchannel);
					if (pgardens == 1 && loopseta == 0)
						musicchannel = Mix_PlayChannel(_mgardens);
					if (pgardens == 1 && loopseta == 1)
						musicchannel = Mix_PlayChannel(_mgardens2);
					if (pgardens == 1 && loopseta == 2)
						musicchannel = Mix_PlayChannel(_mgardens3);
					if (pgardens == 1 && loopseta == 3)
						musicchannel = Mix_PlayChannel(_mgardens4);
				}

				Mix_Volume(musicchannel, config.musicvol);
			}
		}
	}
}

void GriffonEngine::game_updnpcs() {
	for (int i = 1; i <= lastnpc; i++) {
		if (npcinfo[i].hp > 0) {
			//  is npc walking
			int pass = 0;
			if (npcinfo[i].attacking == 0)
				pass = 1;
			if (npcinfo[i].spriteset == 5)
				pass = 1;
			if (pass == 1) {
				int moveup = 0;
				int movedown = 0;
				int moveleft = 0;
				int moveright = 0;

				float npx = npcinfo[i].x;
				float npy = npcinfo[i].y;

				float onpx = npx;
				float onpy = npy;

				float wspd = npcinfo[i].walkspd / 4;

				if (npcinfo[i].spriteset == 10)
					wspd = wspd * 2;
				int wdir = npcinfo[i].walkdir;

				int mode = npcinfo[i].movementmode;

				float xdif = _player.px - npx;
				float ydif = _player.py - npy;

				if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16 && mode < 3)
					mode = 0;
				if (npcinfo[i].hp < npcinfo[i].maxhp * 0.25)
					mode = 3;

				if (npcinfo[i].pause > _ticks)
					mode = -1;
				if (npcinfo[i].spriteset == 2 && npcinfo[i].castpause > _ticks)
					mode = -1;

				if (mode == 3) {
					mode = 1;
					if (abs(xdif) < 4 * 16 && abs(ydif) < 4 * 16)
						mode = 3;
				}

				bool checkpass = false;

				// npc  AI CODE
				// --------------

				// *** aggressive
				if (mode == 0) {
					wspd = npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 2;
						if (xdif > -4)
							wdir = 3;
					} else {
						if (ydif < 4)
							wdir = 0;
						if (ydif > -4)
							wdir = 1;
					}

					if (xdif < 4)
						moveleft = 1;
					if (xdif > -4)
						moveright = 1;
					if (ydif < 4)
						moveup = 1;
					if (ydif > -4)
						movedown = 1;
				}
				// *******************

				// *** defensive
				if (mode == 1) {

					int movingdir = npcinfo[i].movingdir;

					if (npcinfo[i].ticks > _ticks + 100000)
						npcinfo[i].ticks = _ticks;

					if (npcinfo[i].ticks < _ticks) {
						npcinfo[i].ticks = _ticks + 2000;
						movingdir = (int)(RND() * 8);
						npcinfo[i].movingdir = movingdir;
					}

					if (movingdir == 0) {
						wdir = 2; // left
						moveup = 1;
						moveleft = 1;
					} else if (movingdir == 1) {
						wdir = 0; // up
						moveup = 1;
					} else if (movingdir == 2) {
						wdir = 3; // right
						moveup = 1;
						moveright = 1;
					} else if (movingdir == 3) {
						wdir = 3; // right
						moveright = 1;
					} else if (movingdir == 4) {
						wdir = 3; // right
						moveright = 1;
						movedown = 1;
					} else if (movingdir == 5) {
						wdir = 1; // down
						movedown = 1;
					} else if (movingdir == 6) {
						wdir = 2; // left
						movedown = 1;
						moveleft = 1;
					} else if (movingdir == 7) {
						wdir = 2; // left
						moveleft = 1;
					}

					checkpass = true;
				}
				// *******************

				// *** run away
				if (mode == 3) {
					wspd = npcinfo[i].walkspd / 2;

					xdif = _player.px - npx;
					ydif = _player.py - npy;

					if (abs(xdif) > abs(ydif)) {
						if (xdif < 4)
							wdir = 3;
						if (xdif > -4)
							wdir = 2;
					} else {
						if (ydif < 4)
							wdir = 1;
						if (ydif > -4)
							wdir = 0;
					}

					if (xdif < 4)
						moveright = 1;
					if (xdif > -4)
						moveleft = 1;
					if (ydif < 4)
						movedown = 1;
					if (ydif > -4)
						moveup = 1;
				}
				// *******************

				// -------------- ?? move*** vs movin***
				int movinup = 0;
				int movindown = 0;
				int movinleft = 0;
				int movinright = 0;

				float xp = (npx / 2 + 6);
				float yp = (npy / 2 + 10);

				if (npcinfo[i].spriteset == 10)
					wspd = wspd * 2;

				float ii = wspd * _fpsr;
				if (ii < 1)
					ii = 1;

				if (moveup) {
					int sx = xp;
					int sy = yp - ii;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (npcinfo[i].spriteset == 11)
						dq = 0;

					if (dq == 0)
						movinup = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinup = 1;
							movinleft = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinup = 1;
							movinright = 1;
						}
					}
				}

				if (movedown) {
					int sx = xp;
					int sy = yp + ii;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movindown = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movindown = 1;
							movinleft = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movindown = 1;
							movinright = 1;
						}
					}
				}

				if (moveleft) {
					int sx = xp - ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movinleft = 1;
					if (dq > 0) {
						sx = xp - ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinleft = 1;
							movinup = 1;
						}
					}
					if (dq > 0) {
						sx = xp - ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinleft = 1;
							movindown = 1;
						}
					}
				}

				if (moveright) {
					int sx = xp + ii;
					int sy = yp;
					uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
					uint32 dq = *temp;
					if (npcinfo[i].spriteset == 11)
						dq = 0;
					if (dq == 0)
						movinright = 1;
					if (dq > 0) {
						sx = xp + ii;
						sy = yp - ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinright = 1;
							movinup = 1;
						}
					}
					if (dq > 0) {
						sx = xp + ii;
						sy = yp + ii;
						temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						dq = *temp;
						if (npcinfo[i].spriteset == 11)
							dq = 0;
						if (dq == 0) {
							movinright = 1;
							movindown = 1;
						}
					}
				}

				if (movinup)
					npy = npy - wspd * _fpsr;
				if (movindown)
					npy = npy + wspd * _fpsr;
				if (movinleft)
					npx = npx - wspd * _fpsr;
				if (movinright)
					npx = npx + wspd * _fpsr;

				if (checkpass) {
					pass = 0;
					if (npx >= npcinfo[i].x1 * 16 - 8 && npx <= npcinfo[i].x2 * 16 + 8 && npy >= npcinfo[i].y1 * 16 - 8 && npy <= npcinfo[i].y2 * 16 + 8)
						pass = 1;
					if (pass == 0) {
						npx = onpx;
						npy = onpy;
						npcinfo[i].ticks = _ticks;
					}
				}

				float aspd = wspd;

				if (npcinfo[i].spriteset == 10)
					aspd = wspd / 2;

				xp = (npx / 2 + 6);
				yp = (npy / 2 + 10);

				int sx = xp;
				int sy = yp;
				uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				uint32 bgc = *temp;

				float anpx = npx + 12;
				float anpy = npy + 20;

				int lx = (int)anpx / 16;
				int ly = (int)anpy / 16;

				if (triggerloc[lx][ly] > -1)
					bgc = 1;
				if (npcinfo[i].spriteset == 11)
					bgc = 0;

				int rst = 0;

				if (npcinfo[i].spriteset == 11) {
					if (npx < 40 || npx > 280 || npy < 36 || npy > 204)
						rst = 1;
				}

				if (bgc > 0 || rst == 1) {
					npx = onpx;
					npy = onpy;
				}

				npcinfo[i].x = npx;
				npcinfo[i].y = npy;

				npcinfo[i].walkdir = wdir;
				npcinfo[i].moving = 0;

				if (npx != onpx || npy != onpy)
					npcinfo[i].moving = 1;

				if (npcinfo[i].moving == 1) {
					float frame = npcinfo[i].frame;
					int cframe = npcinfo[i].cframe;

					frame = frame + aspd * _fpsr;
					while (frame >= 16)
						frame = frame - 16;

					cframe = (int)(frame);
					if (cframe > 16)
						cframe = 16 - 1;
					if (cframe < 0)
						cframe = 0;

					npcinfo[i].frame = frame;
					npcinfo[i].cframe = cframe;
				}

				// spriteset1 specific
				if (npcinfo[i].spriteset == 1 && npcinfo[i].attackattempt < _ticks) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 20 && abs(ydif) < 20) {
							npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndenemyhit]);
									Mix_Volume(snd, config.effectsvol);
								}

								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;
							}
						}
					}
				}

				bool dospell = false;

				// onewing specific
				if (npcinfo[i].spriteset == 2) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndbite]);
									Mix_Volume(snd, config.effectsvol);
								}

								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;

								npcinfo[i].headtargetx[0] = _player.px + 12;
								npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					dospell = false;

					if (npcinfo[i].attacking == 0 && npcinfo[i].castpause < _ticks) {
						npcinfo[i].swayspd = npcinfo[i].swayspd + npcinfo[i].swayspd / 200 * _fpsr;
						if (npcinfo[i].swayspd > 15) {
							dospell = true;
							npcinfo[i].swayspd = 1;
						}

						// sway code
						npcinfo[i].swayangle = npcinfo[i].swayangle + npcinfo[i].swayspd * _fpsr;
						if (npcinfo[i].swayangle >= 360)
							npcinfo[i].swayangle = npcinfo[i].swayangle - 360;

						npcinfo[i].headtargetx[0] = npcinfo[i].x + (24 - npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * npcinfo[i].swayangle) + 12;
						npcinfo[i].headtargety[0] = npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * npcinfo[i].swayangle);
					}

					if (dospell) {
						npcinfo[i].pause = _ticks + 3000;
						npcinfo[i].attacknext = _ticks + 4500;
						npcinfo[i].castpause = _ticks + 4500;

						game_castspell(3, npcinfo[i].x, npcinfo[i].y, npcinfo[i].x, npcinfo[i].y, i);

						npcinfo[i].headtargetx[0] = npcinfo[i].x;
						npcinfo[i].headtargety[0] = npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = npcinfo[i].bodysection[7].x - npcinfo[i].headtargetx[0];
					ydif = npcinfo[i].bodysection[7].y - npcinfo[i].headtargety[0];


					npcinfo[i].bodysection[7].x = npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					npcinfo[i].bodysection[7].y = npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;


					npcinfo[i].bodysection[0].x = npcinfo[i].x + 12;
					npcinfo[i].bodysection[0].y = npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = npcinfo[i].bodysection[f + 1].x - npcinfo[i].bodysection[f - 1].x;
						ydif = npcinfo[i].bodysection[f + 1].y - npcinfo[i].bodysection[f - 1].y;

						float tx = npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = npcinfo[i].bodysection[f - 1].y + ydif / 2;

						npcinfo[i].bodysection[f].x = npcinfo[i].bodysection[f].x - (npcinfo[i].bodysection[f].x - tx) / 3;
						npcinfo[i].bodysection[f].y = npcinfo[i].bodysection[f].y - (npcinfo[i].bodysection[f].y - ty) / 3;
					}
				}

				// boss1 specific and blackknight
				if (npcinfo[i].spriteset == 3 || npcinfo[i].spriteset == 4) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npcinfo[i].attacking = 1;
						npcinfo[i].attackframe = 0;

						game_castspell(1, npcinfo[i].x, npcinfo[i].y, _player.px, _player.py, i);
					}

					if (npcinfo[i].castpause < _ticks) {
						game_castspell(6, npcinfo[i].x, npcinfo[i].y, _player.px, _player.py, i);
						npcinfo[i].castpause = _ticks + 12000;
					}
				}


				// firehydra specific
				if (npcinfo[i].spriteset == 5) {
					npcinfo[i].swayspd = 4;

					// sway code
					npcinfo[i].swayangle = npcinfo[i].swayangle + npcinfo[i].swayspd * _fpsr;
					if (npcinfo[i].swayangle >= 360)
						npcinfo[i].swayangle = npcinfo[i].swayangle - 360;

					for (int ff = 0; ff <= 2; ff++) {
						if (npcinfo[i].hp > 10 * ff * 20) {
							if (npcinfo[i].pause < _ticks && npcinfo[i].attacking2[ff] == 0 && npcinfo[i].attacknext2[ff] < _ticks) {
								npx = npcinfo[i].x;
								npy = npcinfo[i].y;

								xdif = _player.px - npx;
								ydif = _player.py - npy;

								if (abs(xdif) < 48 && abs(ydif) < 48) {
									float dist = sqrt(xdif * xdif + ydif * ydif);

									if ((dist) < 36) {
										if (menabled && config.effects) {
											int snd = Mix_PlayChannel(sfx[sndbite]);
											Mix_Volume(snd, config.effectsvol);
										}

										npcinfo[i].attacking = 1;
										npcinfo[i].attacking2[ff] = 1;
										npcinfo[i].attackframe2[ff] = 0;

										npcinfo[i].headtargetx[ff] = _player.px + 12;
										npcinfo[i].headtargety[ff] = _player.py - 4;

										npcinfo[i].swayangle = 0;
									}
								}

							}

							if (npcinfo[i].attacking2[ff] == 0) {
								npcinfo[i].headtargetx[ff] = npcinfo[i].x + 38 * sin(3.14159 / 180 * (npcinfo[i].swayangle + 120 * ff)) + 12;
								npcinfo[i].headtargety[ff] = npcinfo[i].y - 46 + 16 + 16 * sin(3.14159 * 2 / 180 * (npcinfo[i].swayangle + 120 * ff));
							}

							// targethead code
							xdif = npcinfo[i].bodysection[10 * ff + 9].x - npcinfo[i].headtargetx[ff];
							ydif = npcinfo[i].bodysection[10 * ff + 9].y - npcinfo[i].headtargety[ff];

							npcinfo[i].bodysection[10 * ff + 9].x = npcinfo[i].bodysection[10 * ff + 9].x  - xdif * 0.4 * _fpsr;
							npcinfo[i].bodysection[10 * ff + 9].y = npcinfo[i].bodysection[10 * ff + 9].y  - ydif * 0.4 * _fpsr;

							npcinfo[i].bodysection[10 * ff].x = npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							npcinfo[i].bodysection[10 * ff].y = npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = npcinfo[i].bodysection[ff * 10 + f + 1].x - npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = npcinfo[i].bodysection[ff * 10 + f + 1].y - npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								npcinfo[i].bodysection[ff * 10 + f].x = npcinfo[i].bodysection[ff * 10 + f].x - (npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								npcinfo[i].bodysection[ff * 10 + f].y = npcinfo[i].bodysection[ff * 10 + f].y - (npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}
						}
					}
				}

				// spriteset6 specific
				if (npcinfo[i].spriteset == 6 && npcinfo[i].attackattempt < _ticks) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 48 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 48 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;
								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;

								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 48 - 8;
									nnyb = npy + 4;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 48;
									nnyb = npy + 4;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 48;
									nnxb = npx + 4;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 48;
									nnxb = npx + 4;
								}

								game_castspell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// wizard1 specific
				if (npcinfo[i].spriteset == 7) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npcinfo[i].attacking = 1;
						npcinfo[i].attackframe = 0;

						game_castspell(9, npcinfo[i].x, npcinfo[i].y, _player.px, _player.py, i);
					}

					if (npcinfo[i].castpause < _ticks) {
						// game_castspell 6, npcinfo[i].x, npcinfo[i].y, _player.px, _player.py, i
						// npcinfo[i].castpause = _ticks + 12000
					}

				}

				// spriteset6 specific
				if (npcinfo[i].spriteset == 8 && npcinfo[i].attackattempt < _ticks) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						pass = 0;
						if (abs(xdif) < 56 && abs(ydif) < 6)
							pass = 1;
						if (abs(ydif) < 56 && abs(xdif) < 6)
							pass = 2;

						if (pass > 0) {
							npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;

								float nnxa = 0, nnya = 0, nnxb = 0, nnyb = 0;
								if (pass == 1 && xdif < 0) {
									nnxa = npx - 8;
									nnya = npy + 4;
									nnxb = npx - 56 - 8;
									nnyb = npy + 4;
									npcinfo[i].walkdir = 2;
								} else if (pass == 1 && xdif > 0) {
									nnxa = npx + 16;
									nnya = npy + 4;
									nnxb = npx + 16 + 56;
									nnyb = npy + 4;
									npcinfo[i].walkdir = 3;
								} else if (pass == 2 && ydif < 0) {
									nnya = npy;
									nnxa = npx + 4;
									nnyb = npy - 56;
									nnxb = npx + 4;
									npcinfo[i].walkdir = 0;
								} else if (pass == 2 && ydif > 0) {
									nnya = npy + 20;
									nnxa = npx + 4;
									nnyb = npy + 20 + 56;
									nnxb = npx + 4;
									npcinfo[i].walkdir = 1;
								}

								game_castspell(7, nnxa, nnya, nnxb, nnyb, i);
							}
						}
					}
				}

				// twowing specific
				if (npcinfo[i].spriteset == 9) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].bodysection[7].x;
						npy = npcinfo[i].bodysection[7].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 24 && abs(ydif) < 24) {
							float dist = sqrt(xdif * xdif + ydif * ydif);

							if ((dist) < 24) {
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndbite]);
									Mix_Volume(snd, config.effectsvol);
								}

								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;

								npcinfo[i].headtargetx[0] = _player.px + 12;
								npcinfo[i].headtargety[0] = _player.py - 4;
							}
						}

					}

					if (npcinfo[i].attacking == 0 && npcinfo[i].castpause < _ticks) {
						npcinfo[i].swayspd = 4;

						// sway code
						npcinfo[i].swayangle = npcinfo[i].swayangle + npcinfo[i].swayspd * _fpsr;
						if (npcinfo[i].swayangle >= 360)
							npcinfo[i].swayangle = npcinfo[i].swayangle - 360;

						npcinfo[i].headtargetx[0] = npcinfo[i].x + (24 - npcinfo[i].swayspd / 2) * sin(3.14159 / 180 * npcinfo[i].swayangle) + 12;
						npcinfo[i].headtargety[0] = npcinfo[i].y - 36 + 16 + 8 * sin(3.14159 * 2 / 180 * npcinfo[i].swayangle);
					}

					if (dospell) {
						npcinfo[i].pause = _ticks + 3000;
						npcinfo[i].attacknext = _ticks + 5000;
						npcinfo[i].castpause = _ticks + 3000;

						game_castspell(3, npcinfo[i].x, npcinfo[i].y, npcinfo[i].x, npcinfo[i].y, i);

						npcinfo[i].headtargetx[0] = npcinfo[i].x;
						npcinfo[i].headtargety[0] = npcinfo[i].y - 36 + 16;
					}

					// targethead code
					xdif = npcinfo[i].bodysection[7].x - npcinfo[i].headtargetx[0];
					ydif = npcinfo[i].bodysection[7].y - npcinfo[i].headtargety[0];


					npcinfo[i].bodysection[7].x = npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					npcinfo[i].bodysection[7].y = npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					npcinfo[i].bodysection[0].x = npcinfo[i].x + 12;
					npcinfo[i].bodysection[0].y = npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = npcinfo[i].bodysection[f + 1].x - npcinfo[i].bodysection[f - 1].x;
						ydif = npcinfo[i].bodysection[f + 1].y - npcinfo[i].bodysection[f - 1].y;

						float tx = npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = npcinfo[i].bodysection[f - 1].y + ydif / 2;

						npcinfo[i].bodysection[f].x = npcinfo[i].bodysection[f].x - (npcinfo[i].bodysection[f].x - tx) / 3;
						npcinfo[i].bodysection[f].y = npcinfo[i].bodysection[f].y - (npcinfo[i].bodysection[f].y - ty) / 3;
					}

				}

				// dragon2 specific
				if (npcinfo[i].spriteset == 10 && npcinfo[i].attackattempt < _ticks) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 32 && abs(ydif) < 32) {
							npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndenemyhit]);
									Mix_Volume(snd, config.effectsvol);
								}

								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;
							}
						}
					}
				}


				// endboss specific
				if (npcinfo[i].spriteset == 11 && npcinfo[i].attackattempt < _ticks) {
					if (npcinfo[i].attacknext < _ticks && npcinfo[i].pause < _ticks && npcinfo[i].attacking == 0) {
						npx = npcinfo[i].x;
						npy = npcinfo[i].y;

						xdif = _player.px - npx;
						ydif = _player.py - npy;

						if (abs(xdif) < 38 && abs(ydif) < 38) {
							npcinfo[i].attackattempt = _ticks + 100;
							if ((int)(RND() * 2) == 0) {
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndice]);
									Mix_Volume(snd, config.effectsvol);
								}
								npcinfo[i].attacking = 1;
								npcinfo[i].attackframe = 0;
							}
						}
					}
				}
			}


			float npx = npcinfo[i].x;
			float npy = npcinfo[i].y;

			int xp = (npx / 2 + 6);
			int yp = (npy / 2 + 10);

			rcSrc.left = xp - 1;
			rcSrc.top = yp - 1;
			rcSrc.setWidth(3);
			rcSrc.setHeight(3);

			if (npcinfo[i].pause < _ticks)
				_clipbg->fillRect(rcSrc, i);


			pass = 0;
			if (npcinfo[i].attacking == 1)
				pass = 1;
			if (npcinfo[i].spriteset == 5) {
				if (npcinfo[i].attacking2[0] == 1)
					pass = 1;
				if (npcinfo[i].attacking2[1] == 1)
					pass = 1;
				if (npcinfo[i].attacking2[2] == 1)
					pass = 1;
			}

			if (pass == 1) {
				int dist;
				float damage;
				// spriteset1 specific
				if (npcinfo[i].spriteset == 1) {
					npcinfo[i].attackframe = npcinfo[i].attackframe + npcinfo[i].attackspd * _fpsr;
					if (npcinfo[i].attackframe >= 16) {
						npcinfo[i].attackframe = 0;
						npcinfo[i].attacking = 0;
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
					}

					npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

					npx = npcinfo[i].x;
					npy = npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 10;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
						// npcinfo[i].attackframe = 0;
						// npcinfo[i].attacking = 0;

						damage = (float)npcinfo[i].attackdamage * (0.5 + RND() * 1.0);

						if (_player.hp > 0)
							game_damageplayer(damage);
					}
				}

				if (npcinfo[i].spriteset == 2) {
					// targethead code
					float xdif = npcinfo[i].bodysection[7].x - npcinfo[i].headtargetx[0];
					float ydif = npcinfo[i].bodysection[7].y - npcinfo[i].headtargety[0];

					npcinfo[i].bodysection[7].x = npcinfo[i].bodysection[7].x  - xdif * 0.4 * _fpsr;
					npcinfo[i].bodysection[7].y = npcinfo[i].bodysection[7].y  - ydif * 0.4 * _fpsr;

					npcinfo[i].bodysection[0].x = npcinfo[i].x + 12;
					npcinfo[i].bodysection[0].y = npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = npcinfo[i].bodysection[f + 1].x - npcinfo[i].bodysection[f - 1].x;
						ydif = npcinfo[i].bodysection[f + 1].y - npcinfo[i].bodysection[f - 1].y;

						float tx = npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = npcinfo[i].bodysection[f - 1].y + ydif / 2;

						npcinfo[i].bodysection[f].x = npcinfo[i].bodysection[f].x - (npcinfo[i].bodysection[f].x - tx);
						npcinfo[i].bodysection[f].y = npcinfo[i].bodysection[f].y - (npcinfo[i].bodysection[f].y - ty);
					}

					npcinfo[i].attackframe = npcinfo[i].attackframe + npcinfo[i].attackspd * _fpsr;
					if (npcinfo[i].attackframe >= 16) {
						npcinfo[i].attackframe = 0;
						npcinfo[i].attacking = 0;
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
					}

					npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

					npx = npcinfo[i].bodysection[7].x;
					npy = (npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
						// npcinfo[i].attackframe = 0
						// npcinfo[i].attacking = 0
						damage = (float)npcinfo[i].attackdamage * (1.0 + (RND() * 0.5));
						if (_player.hp > 0)
							game_damageplayer(damage);
					}
				}


				// firehydra
				if (npcinfo[i].spriteset == 5) {
					for (int ff = 0; ff <= 2; ff++) {
						if (npcinfo[i].attacking2[ff] == 1) {
							float xdif = npcinfo[i].bodysection[10 * ff + 9].x - npcinfo[i].headtargetx[ff];
							float ydif = npcinfo[i].bodysection[10 * ff + 9].y - npcinfo[i].headtargety[ff];

							npcinfo[i].bodysection[10 * ff + 9].x = npcinfo[i].bodysection[10 * ff + 9].x  - xdif * .2 * _fpsr;
							npcinfo[i].bodysection[10 * ff + 9].y = npcinfo[i].bodysection[10 * ff + 9].y  - ydif * .2 * _fpsr;

							npcinfo[i].bodysection[10 * ff].x = npcinfo[i].x + 12 + 8 * cos(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));
							npcinfo[i].bodysection[10 * ff].y = npcinfo[i].y + 12 + 8 * sin(3.141592 * 2 * (_itemyloc / 16 + ff * 120 / 360));

							for (int f = 8; f >= 1; f--) {
								xdif = npcinfo[i].bodysection[ff * 10 + f + 1].x - npcinfo[i].bodysection[ff * 10 + f - 1].x;
								ydif = npcinfo[i].bodysection[ff * 10 + f + 1].y - npcinfo[i].bodysection[ff * 10 + f - 1].y;

								float tx = npcinfo[i].bodysection[ff * 10 + f - 1].x + xdif / 2;
								float ty = npcinfo[i].bodysection[ff * 10 + f - 1].y + ydif / 2;

								npcinfo[i].bodysection[ff * 10 + f].x = npcinfo[i].bodysection[ff * 10 + f].x - (npcinfo[i].bodysection[ff * 10 + f].x - tx) / 3;
								npcinfo[i].bodysection[ff * 10 + f].y = npcinfo[i].bodysection[ff * 10 + f].y - (npcinfo[i].bodysection[ff * 10 + f].y - ty) / 3;
							}

							npcinfo[i].attackframe2[ff] = npcinfo[i].attackframe2[ff] + npcinfo[i].attackspd * _fpsr;
							if (npcinfo[i].attackframe2[ff] >= 16) {
								npcinfo[i].attackframe2[ff] = 0;
								npcinfo[i].attacking2[ff] = 0;
								npcinfo[i].attacknext2[ff] = _ticks + npcinfo[i].attackdelay;
							}

							npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

							npx = npcinfo[i].bodysection[10 * ff + 9].x;
							npy = (npcinfo[i].bodysection[10 * ff + 9].y + 16);

							xdif = (_player.px + 12) - npx;
							ydif = (_player.py + 12) - npy;

							dist = 8;

							if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
								npcinfo[i].attacknext2[ff] = _ticks + npcinfo[i].attackdelay;
								// npcinfo[i].attackframe2(ff) = 0
								// npcinfo[i].attacking2(ff) = 0
								damage = (float)npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
								if (_player.hp > 0)
									game_damageplayer(damage);
							}
						}
					}

				}

				// twowing specific
				if (npcinfo[i].spriteset == 9) {
					// targethead code
					float xdif = npcinfo[i].bodysection[7].x - npcinfo[i].headtargetx[0];
					float ydif = npcinfo[i].bodysection[7].y - npcinfo[i].headtargety[0];

					npcinfo[i].bodysection[7].x = npcinfo[i].bodysection[7].x - xdif * 0.4 * _fpsr;
					npcinfo[i].bodysection[7].y = npcinfo[i].bodysection[7].y - ydif * 0.4 * _fpsr;

					npcinfo[i].bodysection[0].x = npcinfo[i].x + 12;
					npcinfo[i].bodysection[0].y = npcinfo[i].y + 12;

					for (int f = 6; f >= 1; f--) {
						xdif = npcinfo[i].bodysection[f + 1].x - npcinfo[i].bodysection[f - 1].x;
						ydif = npcinfo[i].bodysection[f + 1].y - npcinfo[i].bodysection[f - 1].y;

						float tx = npcinfo[i].bodysection[f - 1].x + xdif / 2;
						float ty = npcinfo[i].bodysection[f - 1].y + ydif / 2;

						npcinfo[i].bodysection[f].x = npcinfo[i].bodysection[f].x - (npcinfo[i].bodysection[f].x - tx);
						npcinfo[i].bodysection[f].y = npcinfo[i].bodysection[f].y - (npcinfo[i].bodysection[f].y - ty);
					}

					npcinfo[i].attackframe = npcinfo[i].attackframe + npcinfo[i].attackspd * _fpsr;
					if (npcinfo[i].attackframe >= 16) {
						npcinfo[i].attackframe = 0;
						npcinfo[i].attacking = 0;
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
					}

					npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

					npx = npcinfo[i].bodysection[7].x;
					npy = (npcinfo[i].bodysection[7].y + 16);

					xdif = (_player.px + 12) - npx;
					ydif = (_player.py + 12) - npy;

					dist = 8;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
						// npcinfo[i].attackframe = 0
						// npcinfo[i].attacking = 0
						damage = (float)npcinfo[i].attackdamage * (1.0 + RND() * 0.5);
						if (_player.hp > 0)
							game_damageplayer(damage);
					}
				}

				// dragon 2 specific
				if (npcinfo[i].spriteset == 10) {

					npcinfo[i].attackframe = npcinfo[i].attackframe + npcinfo[i].attackspd * _fpsr;
					if (npcinfo[i].attackframe >= 16) {
						npcinfo[i].attackframe = 0;
						npcinfo[i].attacking = 0;
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
					}

					npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

					npx = npcinfo[i].x;
					npy = npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 16 + npcinfo[i].attackframe;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
						// npcinfo[i].attackframe = 0
						// npcinfo[i].attacking = 0
						damage = (float)npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							game_damageplayer(damage);
					}
				}

				// endboss specific
				if (npcinfo[i].spriteset == 11) {
					npcinfo[i].attackframe = npcinfo[i].attackframe + npcinfo[i].attackspd * _fpsr;
					if (npcinfo[i].attackframe >= 16) {
						npcinfo[i].attackframe = 0;
						npcinfo[i].attacking = 0;
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
					}

					npcinfo[i].cattackframe = (int)(npcinfo[i].attackframe);

					npx = npcinfo[i].x;
					npy = npcinfo[i].y;

					float xdif = _player.px - npx;
					float ydif = _player.py - npy;

					dist = 36;

					if (abs(xdif) < dist && abs(ydif) < dist && _player.pause < _ticks) {
						npcinfo[i].attacknext = _ticks + npcinfo[i].attackdelay;
						// npcinfo[i].attackframe = 0
						// npcinfo[i].attacking = 0
						damage = (float)npcinfo[i].attackdamage * (0.5 + RND() * 1.0);
						if (_player.hp > 0)
							game_damageplayer(damage);
					}
				}
			}
			// -------end fight code
		}
	}
}

void GriffonEngine::game_updspells() {
	int foundel[5];
	float npx, npy;
	long cl1, cl2, cl3;
	int ll[4][2];

	float xloc = 0, yloc = 0, xst, yst, xdif, ydif;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0 && !_forcepause) {
				float fr = (32 - spellinfo[i].frame);

				ll[0][0] = -2;
				ll[0][1] = -3;
				ll[1][0] = 2;
				ll[1][1] = -3;
				ll[2][0] = -4;
				ll[2][1] = -2;
				ll[3][0] = 4;
				ll[3][1] = -2;

				for (int f = 0; f <= 3; f++) {
					if (fr > f * 4 && fr < f * 4 + 16) {
						float alf = 255;

						if (fr < f * 4 + 8) {
							int fi = (int)((fr - f * 4) * 3) % 4;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							xloc = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							yloc = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255 * ((fr - f * 4) / 8);
						}

						if (fr >= f * 4 + 8) {
							int fi = 0; // ??

							if (f == 0 || f == 2)
								fi = 0;
							if (f == 1 || f == 3)
								fi = 1;
							rcSrc.left = 32 + fi * 16;
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							xst = spellinfo[i].enemyx + 12 + ll[f][0] * 16;
							yst = spellinfo[i].enemyy + 16 + ll[f][1] * 16;

							float xi = (spellinfo[i].enemyx - xst) * 2 / 8;
							float yi = (spellinfo[i].enemyy - yst) * 2 / 8;

							float fl = (fr - f * 4 - 8) / 2;
							xloc = xst + xi * fl * fl;
							yloc = yst + yi * fl * fl;

							rcDest.left = xloc;
							rcDest.top = yloc;

							alf = 255;
						}

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellimg->setAlpha(alf, true);
							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
							_spellimg->setAlpha(255, true);

							if (spellinfo[i].damagewho == 0) {
								for (int e = 1; e <= lastnpc; e++) {

									xdif = (xloc + 16) - (npcinfo[e].x + 12);
									ydif = (yloc + 16) - (npcinfo[e].y + 12);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

										if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks) {
											game_damagenpc(e, damage, 1);
											if (menabled && config.effects) {
												int snd = Mix_PlayChannel(sfx[sndice]);
												Mix_Volume(snd, config.effectsvol);
											}
										}
									}
								}
							}

							// check for post damage
							if (nposts > 0) {
								for (int e = 0; e <= nposts - 1; e++) {
									xdif = (xloc + 16) - (postinfo[e][0] + 8);
									ydif = (yloc + 16) - (postinfo[e][1] + 8);

									if ((abs(xdif) < 16 && abs(ydif) < 16)) {
										objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
										objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

										rcSrc.left = postinfo[e][0] / 2;
										rcSrc.top = postinfo[e][1] / 2;
										rcSrc.setWidth(8);
										rcSrc.setHeight(8);

										_clipbg2->fillRect(rcSrc, 0);

										game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

										if (menabled && config.effects) {
											int snd = Mix_PlayChannel(sfx[sndice]);
											Mix_Volume(snd, config.effectsvol);
										}
									}
								}
							}
						}
					}
				}
			}

			// metal
			if (spellnum == 1 && !_forcepause) {
				int fr = (int)((32 - spellinfo[i].frame) * 4) % 3;

				rcSrc.left = fr * 48;
				rcSrc.top = 0;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				float c1 = (32 - spellinfo[i].frame) / 16;

				float halfx = (spellinfo[i].homex - 12) + ((spellinfo[i].enemyx - 12) - (spellinfo[i].homex - 12)) / 2;
				float halfy = (spellinfo[i].homey - 12) + ((spellinfo[i].enemyy - 12) - (spellinfo[i].homey - 12)) / 2;

				float wdth = (halfx - spellinfo[i].homex) * 1.2;
				float hight = (halfy - spellinfo[i].homey) * 1.2;

				xloc = halfx + wdth * cos(3.14159 + 3.14159 * 2 * c1);
				yloc = halfy + hight * sin(3.14159 + 3.14159 * 2 * c1);

				rcDest.left = xloc;
				rcDest.top = yloc;

				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 0) {
					for (int e = 1; e <= lastnpc; e++) {
						xdif = (xloc + 24) - (npcinfo[e].x + 12);
						ydif = (yloc + 24) - (npcinfo[e].y + 12);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

							if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks) {
								game_damagenpc(e, damage, 1);
								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndmetalhit]);
									Mix_Volume(snd, config.effectsvol);
								}
							}
						}
					}
				}

				if (spellinfo[i].damagewho == 1) {
					// --------- boss 1 specific
					if (ABS(spellinfo[i].frame) < 0 && npcinfo[spellinfo[i].npc].spriteset == 3) {
						int npc = spellinfo[i].npc;
						npcinfo[npc].attackframe = 0;
						npcinfo[npc].attacking = 0;

						npcinfo[npc].pause = _ticks + 1000;
						npcinfo[npc].attacknext = _ticks + 4000;
					}
					// ---------------

					// --------- blackknight specific
					if (ABS(spellinfo[i].frame) < 0 && npcinfo[spellinfo[i].npc].spriteset == 4) {
						int npc = spellinfo[i].npc;
						npcinfo[npc].attackframe = 0;
						npcinfo[npc].attacking = 0;

						npcinfo[npc].pause = _ticks + 1000;
						npcinfo[npc].attacknext = _ticks + 3500;
					}
					// ---------------

					xdif = (xloc + 24) - (_player.px + 12);
					ydif = (yloc + 24) - (_player.py + 12);

					if ((abs(xdif) < 24 && abs(ydif) < 24) && _player.pause < _ticks) {
						npx = _player.px;
						npy = _player.py;

						float damage = (float)npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

						if (_player.hp > 0) {
							game_damageplayer(damage);
							if (menabled && config.effects) {
								int snd = Mix_PlayChannel(sfx[sndmetalhit]);
								Mix_Volume(snd, config.effectsvol);
							}
						}
					}
				}


				// check for(int post damage
				if (nposts > 0) {
					for (int e = 0; e <= nposts - 1; e++) {
						xdif = (xloc + 24) - (postinfo[e][0] + 8);
						ydif = (yloc + 24) - (postinfo[e][1] + 8);

						if ((abs(xdif) < 24 && abs(ydif) < 24)) {
							objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
							objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

							rcSrc.left = postinfo[e][0] / 2;
							rcSrc.top = postinfo[e][1] / 2;
							rcSrc.setWidth(8);
							rcSrc.setHeight(8);

							_clipbg2->fillRect(rcSrc, 0);

							game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

							if (menabled && config.effects) {
								int snd = Mix_PlayChannel(sfx[sndmetalhit]);
								Mix_Volume(snd, config.effectsvol);
							}
						}
					}
				}
			}

			// earth
			if (spellnum == 2 && !_forcepause) {
				float hght = 240 - spellinfo[i].enemyy;

				for (int f = 8; f >= 0; f--) {

					float fr = (32 - spellinfo[i].frame);

					if (fr > f && fr < f + 16) {
						rcSrc.left = 32 * spellinfo[i].rockimg[f];
						rcSrc.top = 48;
						rcSrc.setWidth(32);
						rcSrc.setHeight(32);

						int scatter = 0;
						if (fr < 8 + f) {
							xloc = spellinfo[i].enemyx - 4;
							yloc = spellinfo[i].enemyy * (1 - cos(3.14159 / 2 * (fr - f) / 8)); // ^ 2;
							yloc *= yloc;
						} else {
							scatter = 1;
							xloc = spellinfo[i].enemyx - 4 - spellinfo[i].rockdeflect[f] * sin(3.14159 / 2 * ((fr - f) - 8) / 8);
							yloc = spellinfo[i].enemyy + hght * (1 - cos(3.14159 / 2 * ((fr - f) - 8) / 8));
						}

						rcDest.left = xloc;
						rcDest.top = yloc;

						if (xloc > -16 && xloc < 304 && yloc > -16 && yloc < 224) {
							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

							if (scatter == 1) {
								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= lastnpc; e++) {
										xdif = (xloc + 16) - (npcinfo[e].x + 12);
										ydif = (yloc + 16) - (npcinfo[e].y + 12);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks) {
												game_damagenpc(e, damage, 1);
												if (menabled && config.effects) {
													int snd = Mix_PlayChannel(sfx[sndrocks]);
													Mix_Volume(snd, config.effectsvol);
												}
											}
										}
									}
								}


								// check for(int post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										xdif = (xloc + 16) - (postinfo[e][0] + 8);
										ydif = (yloc + 16) - (postinfo[e][1] + 8);

										if ((abs(xdif) < 16 && abs(ydif) < 16)) {
											objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
											objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

											rcSrc.left = postinfo[e][0] / 2;
											rcSrc.top = postinfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipbg2->fillRect(rcSrc, 0);

											game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);

											if (menabled && config.effects) {
												int snd = Mix_PlayChannel(sfx[sndrocks]);
												Mix_Volume(snd, config.effectsvol);
											}
										}
									}
								}
							}
						}
					}
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;
			}

			// crystal
			if (spellnum == 5) {

				float fra = (32 - spellinfo[i].frame);
				int fr = (int)((spellinfo[i].frame) * 2) % 8;

				rcSrc.left = fr * 32;
				rcSrc.top = 96 + 48;
				rcSrc.setWidth(32);
				rcSrc.setHeight(64);

				rcDest.left = _player.px - 4;
				rcDest.top = _player.py + 16 - 48;

				int f = 160;
				if (fra < 8)
					f = 192 * fra / 8;
				if (fra > 24)
					f = 192 * (1 - (fra - 24) / 8);

				_spellimg->setAlpha(f, true);
				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.3 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcepause = false;

					npx = _player.px + 12;
					npy = _player.py + 20;

					int lx = (int)npx / 16;
					int ly = (int)npy / 16;

					for (int f1 = 0; f1 < 5; f1++) { // !! f < 5
						foundel[f1] = 0;
					}

					for (int xo = -2; xo <= 2; xo++) {
						for (int yo = -2; yo <= 2; yo++) {

							int sx = lx + xo;
							int sy = ly + yo;

							if (sx > -1 && sx < 20 && sy > -1 && sy < 15) {
								for (int l = 0; l <= 2; l++) {
									int curtile = tileinfo[l][sx][sy][0];
									int curtilel = tileinfo[l][sx][sy][1];

									if (curtile > 0) {
										curtile = curtile - 1;
										int curtilex = curtile % 20;
										int curtiley = (curtile - curtilex) / 20;

										int element = elementmap[curtiley][curtilex];
										if (element > -1 && curtilel == 0)
											foundel[element + 1] = 1;
									}
								}

								int o = objmap[sx][sy];
								if (o > -1) {
									if (objectinfo[o][4] == 1)
										foundel[2] = 1;
									if (o == 1 || o == 2) {
										foundel[2] = 1;
										foundel[4] = 1;
									}
								}
							}
						}
					}

					char line[256];
					strcpy(line, "Found... nothing...");

					for (int f1 = 0; f1 < 5; f1++) {
						if (foundel[f1] == 1 && _player.foundspell[f1] == 0) {
							_player.foundspell[f1] = 1;
							_player.spellcharge[f1] = 0;
							if (f1 == 1)
								strcpy(line, "Found... Water Essence");
							if (f1 == 2)
								strcpy(line, "Found... Metal Essence");
							if (f1 == 3)
								strcpy(line, "Found... Earth Essence");
							if (f1 == 4)
								strcpy(line, "Found... Fire Essence");
							break;
						}
					}

					game_eventtext(line);
				}
			}

			// room fireballs
			if (spellnum == 6 && !_forcepause) {

				if (spellinfo[i].frame > 16) {
					float fr = (32 - spellinfo[i].frame);

					_spellimg->setAlpha(192 * sin(3.14159 * fr / 4), true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {

						xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
						yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

						rcDest.left = xloc;
						rcDest.top = yloc;

						_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
					}

					_spellimg->setAlpha(255, true);
				} else {
					_spellimg->setAlpha(192, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						float ax = spellinfo[i].fireballs[ff][0];
						float ay = spellinfo[i].fireballs[ff][1];
						float bx = _player.px + 4;
						float by = _player.py + 4;
						float d = sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));

						float tx = (bx - ax) / d;
						float ty = (by - ay) / d;

						spellinfo[i].fireballs[ff][2] += tx * 1.2 * _fpsr;
						spellinfo[i].fireballs[ff][3] += ty * 1.2 * _fpsr;

						if (spellinfo[i].ballon[ff] == 1) {
							spellinfo[i].fireballs[ff][0] = ax + spellinfo[i].fireballs[ff][2] * 0.2 * _fpsr;
							spellinfo[i].fireballs[ff][1] = ay + spellinfo[i].fireballs[ff][3] * 0.2 * _fpsr;

							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							rcDest.left = xloc;
							rcDest.top = yloc;

							_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
						}

						if (xloc < -1 || yloc < -1 || xloc > 304 || yloc > 224)
							spellinfo[i].ballon[ff] = 0;
					}

					_spellimg->setAlpha(255, true);
				}

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (spellinfo[i].damagewho == 1) {
					for (int ff = 0; ff <= spellinfo[i].nfballs - 1; ff++) {
						if (spellinfo[i].ballon[ff] == 1) {
							xloc = spellinfo[i].fireballs[ff][0] + (int)(RND() * 3) - 1;
							yloc = spellinfo[i].fireballs[ff][1] + (int)(RND() * 3) - 1;

							xdif = (xloc + 8) - (_player.px + 12);
							ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = npcinfo[spellinfo[i].npc].spelldamage * (1 + RND() * 0.5) / 3;

								if (_player.hp > 0)
									game_damageplayer(damage);

								if (menabled && config.effects) {
									int snd = Mix_PlayChannel(sfx[sndfire]);
									Mix_Volume(snd, config.effectsvol);
								}
							}
						}
					}
				}
			}

			// lightning bomb
			if (spellnum == 8) {

				cl1 = _videobuffer->format.RGBToColor(0, 32, 204);
				cl2 = _videobuffer->format.RGBToColor(142, 173, 191);
				cl3 = _videobuffer->format.RGBToColor(240, 240, 240);

				float px = _player.px + 12;
				float py = _player.py + 12;

				int apx = px + (int)(RND() * 5 - 2);
				int apy = py + (int)(RND() * 5 - 2);

				for (int f = 0; f <= 0; f++) { // ??
					int y = apy;
					int orn = 0;
					for (int x = apx; x <= 319; x++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							y = y - 1;
						if (orn == 2)
							y = y + 1;

						sys_line(_videobuffer, x, y - 1, x, y + 2, cl1);
						sys_line(_videobuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							sys_line(_videobuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							sys_line(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= lastnpc; e++) {

								xdif = (x + 16) - (npcinfo[e].x + 12);
								ydif = (y + 16) - (npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks)
										game_damagenpc(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					y = apy;
					orn = 0;
					for (int x = apx; x >= 0; x--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							y = y - 1;
						if (orn == 2)
							y = y + 1;

						sys_line(_videobuffer, x, y - 1, x, y + 2, cl1);
						sys_line(_videobuffer, x, y, x, y + 1, cl3);

						if (rn == 0)
							sys_line(_videobuffer, x, y + 1, x, y + 1, cl2);
						if (rn == 2)
							sys_line(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= lastnpc; e++) {

								xdif = (x + 16) - (npcinfo[e].x + 12);
								ydif = (y + 16) - (npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks)
										game_damagenpc(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					int x = apx;
					orn = 0;
					for (y = apy; y <= 239; y++) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						sys_line(_videobuffer, x - 1, y, x + 2, y, cl1);
						sys_line(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							sys_line(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							sys_line(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= lastnpc; e++) {
								xdif = (x + 16) - (npcinfo[e].x + 12);
								ydif = (y + 16) - (npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks)
										game_damagenpc(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}

					x = apx;
					orn = 0;
					for (y = apy; y >= 0; y--) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						sys_line(_videobuffer, x - 1, y, x + 2, y, cl1);
						sys_line(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							sys_line(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							sys_line(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 0) {
							for (int e = 1; e <= lastnpc; e++) {
								xdif = (x + 16) - (npcinfo[e].x + 12);
								ydif = (y + 16) - (npcinfo[e].y + 12);

								if ((abs(xdif) < 8 && abs(ydif) < 8)) {
									float damage = 30 * (1 + RND() * 0.5);

									if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks)
										game_damagenpc(e, damage, 1);
								}
							}
						}

						// check for post damage
						if (nposts > 0) {
							for (int e = 0; e <= nposts - 1; e++) {
								xdif = (xloc + 16) - (postinfo[e][0] + 8);
								ydif = (yloc + 16) - (postinfo[e][1] + 8);

								if ((abs(xdif) < 16 && abs(ydif) < 16)) {
									objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
									objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

									rcSrc.left = postinfo[e][0] / 2;
									rcSrc.top = postinfo[e][1] / 2;
									rcSrc.setWidth(8);
									rcSrc.setHeight(8);

									_clipbg2->fillRect(rcSrc, 0);

									game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
								}
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;
					_forcepause = false;
				}
			}

			// wizard 1 lightning
			if (spellnum == 9) {

				cl1 = _videobuffer->format.RGBToColor(0, 32, 204);
				cl2 = _videobuffer->format.RGBToColor(142, 173, 191);
				cl3 = _videobuffer->format.RGBToColor(240, 240, 240);

				int px = spellinfo[i].enemyx + 12;
				int py = spellinfo[i].enemyy + 24;

				int apx = px + (int)(RND() * 20 - 10);
				int apy = py + (int)(RND() * 20 - 10);

				int x = apx;
				int orn = 0;
				for (int y = 0; y <= apy; y++) {
					if (y < 240) {
						int rn = (int)(RND() * 3);

						if (orn == 0)
							x = x - 1;
						if (orn == 2)
							x = x + 1;

						sys_line(_videobuffer, x - 1, y, x + 2, y, cl1);
						sys_line(_videobuffer, x, y, x + 1, y, cl3);

						if (rn == 0)
							sys_line(_videobuffer, x + 1, y, x + 1, y, cl2);
						if (rn == 2)
							sys_line(_videobuffer, x, y, x, y, cl2);

						orn = rn;

						if (spellinfo[i].damagewho == 1) {

							xdif = (x + 8) - (_player.px + 12);
							ydif = (y + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
								float damage = ((float)_player.hp * 0.75) * (RND() * 0.5 + 0.5);
								if (damage < 5)
									damage = 5;

								if (npcinfo[spellinfo[i].npc].spriteset == 12) {
									if (damage < 50)
										damage = 40 + (int)(RND() * 40);
								}

								if (_player.hp > 0)
									game_damageplayer(damage);
							}
						}
					}
				}

				spellinfo[i].frame -= 0.5 * _fpsr;
				if (spellinfo[i].frame < 0) {
					spellinfo[i].frame = 0;

					npcinfo[spellinfo[i].npc].attacking = 0;
					npcinfo[spellinfo[i].npc].attacknext = _ticks + npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}



void GriffonEngine::game_updspellsunder() {
	if (_forcepause)
		return;

	for (int i = 0; i < kMaxSpell; i++) {
		if (spellinfo[i].frame > 0) {
			int spellnum = spellinfo[i].spellnum;

			// water
			if (spellnum == 0) {
				int fra = (32 - spellinfo[i].frame);
				int fr = (int)((32 - spellinfo[i].frame) * 2) % 4;

				rcSrc.left = fr * 48;
				rcSrc.top = 96;
				rcSrc.setWidth(48);
				rcSrc.setHeight(48);

				rcDest.left = spellinfo[i].enemyx - 12;
				rcDest.top = spellinfo[i].enemyy - 8;

				int f = 160;
				if (fra < 8)
					f = 160 * fra / 8;
				if (fra > 24)
					f = 160 * (1 - (fra - 24) / 8);

				_spellimg->setAlpha(f, true);
				_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


				for (f = 1; f <= lastnpc; f++) {
					int xdif = spellinfo[i].enemyx - npcinfo[f].x;
					int ydif = spellinfo[i].enemyy - npcinfo[f].y;

					float dist = sqrt((float)(xdif * xdif + ydif * ydif));

					if (dist > 20)
						dist = 20;

					if (dist > 5) {
						float ratio = (1 - dist / 25);

						float newx = npcinfo[f].x + ratio * xdif / 3 * _fpsr;
						float newy = npcinfo[f].y + ratio * ydif / 3 * _fpsr;

						int sx = (newx / 2 + 6);
						int sy = (newy / 2 + 10);

						unsigned int *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
						unsigned int dq = *temp;

						if (dq == 0) {
							npcinfo[f].x = newx;
							npcinfo[f].y = newy;
							// npcinfo[f].castpause = _ticks + 200;
						} else {
							int xpass = 0;
							int ypass = 0;

							sx = (newx / 2 + 6);
							sy = (npcinfo[f].y / 2 + 10);
							temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								xpass = 1;


							sx = (npcinfo[f].x / 2 + 6);
							sy = (newy / 2 + 10);
							temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
							dq = *temp;

							if (dq == 0)
								ypass = 1;

							if (ypass == 1) {
								newx = npcinfo[f].x;
							} else if (xpass == 1) {
								newy = npcinfo[f].y;
							}

							if (xpass == 1 || ypass == 1) {
								npcinfo[f].x = newx;
								npcinfo[f].y = newy;
								// npcinfo[f].castpause = _ticks + 200;
							}
						}
					}
				}
			}

			// fire
			if (spellnum == 3) {
				float fr = (32 - spellinfo[i].frame);

				fr = fr * fr * (1 - cos(3.14159 / 4 + 3.14159 / 4 * fr / 32));

				if (fr > 32)
					fr = 32;

				float s = 8;
				if (spellinfo[i].frame < 8)
					s = spellinfo[i].frame;

				int fra = (int)fr;

				for (int f = 0; f <= 4; f++) {
					for (int x = 0; x <= fra; x += 2) {
						if (spellinfo[i].legalive[f] >= x) {
							_spellimg->setAlpha(192 * sin(3.14159 * x / 32) * s / 8, true);

							float an = 360 / 5 * f + x / 32 * 180;

							rcSrc.left = 16 * (int)(RND() * 2);
							rcSrc.top = 80;
							rcSrc.setWidth(16);
							rcSrc.setHeight(16);

							float xloc = (float)(spellinfo[i].enemyx + 4 + x * 2 * cos(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							float yloc = (float)(spellinfo[i].enemyy + 4 + x * 2 * sin(3.14159 / 180 * an) + (int)(RND() * 3) - 1);
							rcDest.left = (int)xloc;
							rcDest.top = (int)yloc;

							if (xloc > -1 && xloc < 304 && yloc > -1 && yloc < 224) {
								_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

								int sx = (xloc / 2 + 4);
								int sy = (yloc / 2 + 8);

								unsigned int *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
								unsigned int dq = *temp;

								if (dq > 1000 && x > 4)
									spellinfo[i].legalive[f] = x;

								if (spellinfo[i].damagewho == 0) {
									for (int e = 1; e <= lastnpc; e++) {
										float xdif = (xloc + 8) - (npcinfo[e].x + 12);
										float ydif = (yloc + 8) - (npcinfo[e].y + 12);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											float damage = (float)_player.spelldamage * (1.0 + RND() * 0.5) * (float)spellinfo[i].strength;

											if (npcinfo[e].spriteset == 5)
												damage = -damage;
											if (npcinfo[e].spriteset == 11)
												damage = -damage;
											if (npcinfo[e].hp > 0 && npcinfo[e].pause < _ticks) {
												game_damagenpc(e, damage, 1);
												if (menabled && config.effects) {
													int snd = Mix_PlayChannel(sfx[sndfire]);
													Mix_Volume(snd, config.effectsvol);
												}
											}
										}
									}
								}

								if (spellinfo[i].damagewho == 1) {
									float xdif = (xloc + 8) - (_player.px + 12);
									float ydif = (yloc + 8) - (_player.py + 12);

									if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks) {
										float damage = (float)npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

										if (_player.hp > 0) {
											game_damageplayer(damage);

											if (menabled && config.effects) {
												int snd = Mix_PlayChannel(sfx[sndfire]);
												Mix_Volume(snd, config.effectsvol);
											}
										}
									}
								}

								// check for post damage
								if (nposts > 0) {
									for (int e = 0; e <= nposts - 1; e++) {
										float xdif = (xloc + 8) - (postinfo[e][0] + 8);
										float ydif = (yloc + 8) - (postinfo[e][1] + 8);

										if ((abs(xdif) < 8 && abs(ydif) < 8)) {
											objmapf[_curmap][(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = 1;
											objmap[(int)postinfo[e][0] / 16][(int)postinfo[e][1] / 16] = -1;

											rcSrc.left = postinfo[e][0] / 2;
											rcSrc.top = postinfo[e][1] / 2;
											rcSrc.setWidth(8);
											rcSrc.setHeight(8);

											_clipbg2->fillRect(rcSrc, 0);

											if (menabled && config.effects) {
												int snd = Mix_PlayChannel(sfx[sndfire]);
												Mix_Volume(snd, config.effectsvol);
											}

											game_addFloatIcon(99, postinfo[e][0], postinfo[e][1]);
										}
									}
								}
							}
						}
					}
				}

				_spellimg->setAlpha(255, true);

				spellinfo[i].frame = spellinfo[i].frame - 0.2 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;


			}


			// sprite 6 spitfire
			if (spellnum == 7) {
				float xspan = spellinfo[i].enemyx - spellinfo[i].homex;
				float yspan = spellinfo[i].enemyy - spellinfo[i].homey;
				float fr = (32 - spellinfo[i].frame);

				for (int f = 0; f <= 7; f++) {
					int alpha = 0;
					float xx = 0;
					if (fr > f * 2 && fr < f * 2 + 16)
						xx = fr - f * 2;
					if (xx < 8)
						alpha = 255 * xx / 8;
					if (xx > 8)
						alpha = 255 * (1 - (xx - 8) / 8);
					float yy = 16 * sin(3.141592 / 2 * xx / 16) - 8;

					if (alpha < 0)
						alpha = 0;
					if (alpha > 255)
						alpha = 255;

					_spellimg->setAlpha(alpha, true);

					rcSrc.left = 16 * (int)(RND() * 2);
					rcSrc.top = 80;
					rcSrc.setWidth(16);
					rcSrc.setHeight(16);

					float xloc = spellinfo[i].homex + xspan / 7 * f;
					float yloc = spellinfo[i].homey + yspan / 7 * f - yy;

					rcDest.left = xloc;
					rcDest.top = yloc;

					if (xloc > -16 && xloc < 320 && yloc > -16 && yloc < 240) {
						_spellimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

						if (spellinfo[i].damagewho == 1) {
							float xdif = (xloc + 8) - (_player.px + 12);
							float ydif = (yloc + 8) - (_player.py + 12);

							if ((abs(xdif) < 8 && abs(ydif) < 8) && _player.pause < _ticks && alpha > 64) {
								float damage = (float)npcinfo[spellinfo[i].npc].spelldamage * (1.0 + RND() * 0.5);

								if (_player.hp > 0) {
									game_damageplayer(damage);
									if (menabled && config.effects) {
										int snd = Mix_PlayChannel(sfx[sndfire]);
										Mix_Volume(snd, config.effectsvol);
									}
								}

							}
						}

					}

				}

				_spellimg->setAlpha(255, true);
				spellinfo[i].frame = spellinfo[i].frame - 0.5 * _fpsr;
				if (spellinfo[i].frame < 0)
					spellinfo[i].frame = 0;

				if (ABS(spellinfo[i].frame) < 0) {
					npcinfo[spellinfo[i].npc].attacking = 0;
					npcinfo[spellinfo[i].npc].attacknext = _ticks + npcinfo[spellinfo[i].npc].attackdelay;
				}
			}
		}
	}
}

void GriffonEngine::sys_initialize() {
	// init char *floatstri[kMaxFloat]
	for (int i = 0; i < kMaxFloat; i++)
		floatstri[i] = (char *)malloc(64); // 64 bytes each string (should be enough)

	_video = new Graphics::TransparentSurface;
	_video->create(320, 240, g_system->getScreenFormat());
	_videobuffer = new Graphics::TransparentSurface;
	_videobuffer->create(320, 240, g_system->getScreenFormat());
	_videobuffer2 = new Graphics::TransparentSurface;
	_videobuffer2->create(320, 240, g_system->getScreenFormat());
	_videobuffer3 = new Graphics::TransparentSurface;
	_videobuffer3->create(320, 240, g_system->getScreenFormat());
	_mapbg = new Graphics::TransparentSurface;
	_mapbg->create(320, 240, g_system->getScreenFormat());
	_clipbg = new Graphics::TransparentSurface;
	_clipbg->create(320, 240, g_system->getScreenFormat());
	_clipbg2 = new Graphics::TransparentSurface;
	_clipbg2->create(320, 240, g_system->getScreenFormat());

	for (int i = 0; i <= 3; i++) {
		char name[128];

		sprintf(name, "art/map%i.bmp", i + 1);
		mapimg[i] = IMG_Load(name, true);
	}

	cloudimg = IMG_Load("art/clouds.bmp", true);
	cloudimg->setAlpha(96, true);


	saveloadimg = IMG_Load("art/saveloadnew.bmp", true);
	saveloadimg->setAlpha(160, true);

	_titleimg = IMG_Load("art/titleb.bmp");
	_titleimg2 = IMG_Load("art/titlea.bmp", true);
	//_titleimg2->setAlpha(204, true);

	_inventoryimg = IMG_Load("art/inventory.bmp", true);

	_logosimg = IMG_Load("art/logos.bmp");
	_theendimg = IMG_Load("art/theend.bmp");


	sys_LoadTiles();
	sys_LoadTriggers();
	sys_LoadObjectDB();
	sys_LoadAnims();
	sys_LoadFont();
	sys_LoadItemImgs();

	_fpsr = 1.0f;
	_nextticks = _ticks + 1000;

	for (int i = 0; i <= 15; i++) {
		_playerattackofs[0][i][0] = 0; // -1// -(i + 1)
		_playerattackofs[0][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2 - 1;

		_playerattackofs[1][i][0] = 0; // i + 1
		_playerattackofs[1][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2 + 1;

		_playerattackofs[2][i][0] = -1; // -(i + 1)
		_playerattackofs[2][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2;

		_playerattackofs[3][i][0] = 1; // i + 1
		_playerattackofs[3][i][1] = -sin(3.14159 * 2 * (i + 1) / 16) * 2;
	}

	sys_setupAudio();
}

void GriffonEngine::sys_line(Graphics::TransparentSurface *buffer, int x1, int y1, int x2, int y2, int col) {
	int xdif = x2 - x1;
	int ydif = y2 - y1;

	if (xdif == 0) {
		for (int y = y1; y <= y2; y++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x1, y);
			*temp = col;
		}
	}

	if (ydif == 0) {
		for (int x = x1; x <= x2; x++) {
			uint32 *temp = (uint32 *)buffer->getBasePtr(x, y1);
			*temp = col;
		}
	}
}

void GriffonEngine::sys_LoadAnims() {
	_spellimg = IMG_Load("art/spells.bmp", true);
	anims[0] = IMG_Load("art/anims0.bmp", true);
	animsa[0] = IMG_Load("art/anims0a.bmp", true);
	anims[13] = IMG_Load("art/anims0x.bmp", true);
	animsa[13] = IMG_Load("art/anims0xa.bmp", true);
	anims[1] = IMG_Load("art/anims1.bmp", true);
	animsa[1] = IMG_Load("art/anims1a.bmp", true);
	anims[2] = IMG_Load("art/anims2.bmp", true);

	// huge
	animset2[0].xofs = 8;
	animset2[0].yofs = 7;
	animset2[0].x = 123;
	animset2[0].y = 0;
	animset2[0].w = 18;
	animset2[0].h = 16;
	// big
	animset2[1].xofs = 7;
	animset2[1].yofs = 7;
	animset2[1].x = 107;
	animset2[1].y = 0;
	animset2[1].w = 16;
	animset2[1].h = 14;
	// med
	animset2[2].xofs = 6;
	animset2[2].yofs = 6;
	animset2[2].x = 93;
	animset2[2].y = 0;
	animset2[2].w = 14;
	animset2[2].h = 13;
	// small
	animset2[3].xofs = 4;
	animset2[3].yofs = 4;
	animset2[3].x = 83;
	animset2[3].y = 0;
	animset2[3].w = 10;
	animset2[3].h = 10;
	// wing
	animset2[4].xofs = 4;
	animset2[4].yofs = 20;
	animset2[4].x = 42;
	animset2[4].y = 0;
	animset2[4].w = 41;
	animset2[4].h = 33;
	// head
	animset2[5].xofs = 20;
	animset2[5].yofs = 18;
	animset2[5].x = 0;
	animset2[5].y = 0;
	animset2[5].w = 42;
	animset2[5].h = 36;

	anims[9] = IMG_Load("art/anims9.bmp", true);

	// huge
	animset9[0].xofs = 8;
	animset9[0].yofs = 7;
	animset9[0].x = 154;
	animset9[0].y = 0;
	animset9[0].w = 18;
	animset9[0].h = 16;
	// big
	animset9[1].xofs = 7;
	animset9[1].yofs = 7;
	animset9[1].x = 138;
	animset9[1].y = 0;
	animset9[1].w = 16;
	animset9[1].h = 14;
	// med
	animset9[2].xofs = 6;
	animset9[2].yofs = 6;
	animset9[2].x = 93 + 31;
	animset9[2].y = 0;
	animset9[2].w = 14;
	animset9[2].h = 13;
	// small
	animset9[3].xofs = 4;
	animset9[3].yofs = 4;
	animset9[3].x = 83 + 31;
	animset9[3].y = 0;
	animset9[3].w = 10;
	animset9[3].h = 10;
	// wing
	animset9[4].xofs = 36;
	animset9[4].yofs = 20;
	animset9[4].x = 42;
	animset9[4].y = 0;
	animset9[4].w = 72;
	animset9[4].h = 33;
	// head
	animset9[5].xofs = 20;
	animset9[5].yofs = 18;
	animset9[5].x = 0;
	animset9[5].y = 0;
	animset9[5].w = 42;
	animset9[5].h = 36;

	anims[3] = IMG_Load("art/anims3.bmp", true);
	anims[4] = IMG_Load("art/anims4.bmp", true);
	anims[5] = IMG_Load("art/anims5.bmp", true);
	anims[6] = IMG_Load("art/anims6.bmp", true);
	anims[7] = IMG_Load("art/anims7.bmp", true);
	anims[8] = IMG_Load("art/anims8.bmp", true);
	anims[10] = IMG_Load("art/anims10.bmp", true);
	animsa[10] = IMG_Load("art/anims10a.bmp", true);
	anims[11] = IMG_Load("art/anims11.bmp", true);
	animsa[11] = IMG_Load("art/anims11a.bmp", true);
	anims[12] = IMG_Load("art/anims12.bmp", true);
}

void GriffonEngine::sys_LoadItemImgs() {
	Graphics::TransparentSurface *temp = IMG_Load("art/icons.bmp", true);

	for (int i = 0; i <= 20; i++) {
		_itemimg[i] = new Graphics::TransparentSurface;
		_itemimg[i]->create(16, 16, g_system->getScreenFormat());

		rcSrc.left = i * 16;
		rcSrc.top = 0;
		rcSrc.setWidth(16);
		rcSrc.setHeight(16);

		temp->blit(*_itemimg[i], 0, 0, Graphics::FLIP_NONE, &rcSrc);
	}

	temp->free();
}

void GriffonEngine::sys_LoadFont() {
	Graphics::TransparentSurface *font = IMG_Load("art/font.bmp", true);

	for (int i = 32; i <= 255; i++)
		for (int f = 0; f <= 4; f++) {
			int i2 = i - 32;

			_fontchr[i2][f] = new Graphics::TransparentSurface;
			_fontchr[i2][f]->create(8, 8, g_system->getScreenFormat());

			int col = i2 % 40;

			int row = (i2 - col) / 40;

			rcSrc.left = col * 8;
			rcSrc.top = row * 8 + f * 48;
			rcSrc.setWidth(8);
			rcSrc.setHeight(8);

			rcDest.left = 0;
			rcDest.top = 0;
			font->blit(*_fontchr[i2][f], rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);
		}

	font->free();
}

void GriffonEngine::sys_LoadTiles() {
	tiles[0] = IMG_Load("art/tx.bmp", true);
	tiles[1] = IMG_Load("art/tx1.bmp", true);
	tiles[2] = IMG_Load("art/tx2.bmp", true);
	tiles[3] = IMG_Load("art/tx3.bmp", true);

	_windowimg = IMG_Load("art/window.bmp", true);
}

void GriffonEngine::sys_LoadTriggers() {
	Common::File file;
	file.open("data/triggers.dat");

	for (int i = 0; i <= 9999; i++) {
		for (int a = 0; a <= 8; a++) {
			INPUT("%i", &triggers[i][a]);
		}
	}

	file.close();
}

void GriffonEngine::sys_LoadObjectDB() {
	Common::File file;

	file.open("objectdb.dat");

	for (int a = 0; a <= 32; a++) {
		for (int b = 0; b <= 5; b++) {
			INPUT("%i", &objectinfo[a][b]);
		}

		for (int b = 0; b <= 8; b++) {
			for (int c = 0; c <= 2; c++) {
				for (int d = 0; d <= 2; d++) {
					for (int e = 0; e <= 1; e++) {
						INPUT("%i", &objecttile[a][b][c][d][e]);
					}
				}
			}
		}
	}

	file.close();
}

void GriffonEngine::sys_print(Graphics::TransparentSurface *buffer, const char *stri, int xloc, int yloc, int col) {
	int l = strlen(stri);

	for (int i = 0; i < l; i++) {
		rcDest.left = xloc + i * 8;
		rcDest.top = yloc;

		_fontchr[stri[i] - 32][col]->blit(*buffer, rcDest.left, rcDest.top);
	}
}

void GriffonEngine::sys_progress(int w, int wm) {
	long ccc = _videobuffer->format.RGBToColor(0, 255, 0);

	rcDest.setWidth(w * 74 / wm);
	_videobuffer->fillRect(rcDest, ccc);

	g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
	g_system->updateScreen();

	g_system->getEventManager()->pollEvent(_event);
}

void GriffonEngine::sys_setupAudio() {
	menabled = true;

	// FIXME
	//Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);

	//atexit(Mix_Quit);
	//atexit(Mix_CloseAudio);

	const char *stri = "Loading...";
	sys_print(_videobuffer, stri, 160 - 4 * strlen(stri), 116, 0);

	Graphics::TransparentSurface *loadimg = IMG_Load("art/load.bmp", true);

	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.setWidth(88);
	rcSrc.setHeight(32);

	rcDest.left = 160 - 44;
	rcDest.top = 116 + 12;

	loadimg->setAlpha(160, true); // 128
	loadimg->blit(*_videobuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc);

	g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
	g_system->updateScreen();

	rcDest.left = 160 - 44 + 7;
	rcDest.top = 116 + 12 + 12;
	rcDest.setHeight(8);

	if (menabled) {
		_mboss = Mix_LoadWAV("music/boss.ogg");
		sys_progress(1, 21);
		_mgardens = Mix_LoadWAV("music/gardens.ogg");
		sys_progress(2, 21);
		_mgardens2 = Mix_LoadWAV("music/gardens2.ogg");
		sys_progress(3, 21);
		_mgardens3 = Mix_LoadWAV("music/gardens3.ogg");
		sys_progress(4, 21);
		_mgardens4 = Mix_LoadWAV("music/gardens4.ogg");
		sys_progress(5, 21);
		_mendofgame = Mix_LoadWAV("music/endofgame.ogg");
		sys_progress(6, 21);
		_mmenu = Mix_LoadWAV("music/menu.ogg");
		sys_progress(7, 21);

		sfx[0] = Mix_LoadWAV("sfx/bite.ogg");
		sys_progress(8, 21);
		sfx[1] = Mix_LoadWAV("sfx/crystal.ogg");
		sys_progress(9, 21);
		sfx[2] = Mix_LoadWAV("sfx/door.ogg");
		sys_progress(10, 21);
		sfx[3] = Mix_LoadWAV("sfx/enemyhit.ogg");
		sys_progress(11, 21);
		sfx[4] = Mix_LoadWAV("sfx/ice.ogg");
		sys_progress(12, 21);
		sfx[5] = Mix_LoadWAV("sfx/lever.ogg");
		sys_progress(13, 21);
		sfx[6] = Mix_LoadWAV("sfx/lightning.ogg");
		sys_progress(14, 21);
		sfx[7] = Mix_LoadWAV("sfx/metalhit.ogg");
		sys_progress(15, 21);
		sfx[8] = Mix_LoadWAV("sfx/powerup.ogg");
		sys_progress(16, 21);
		sfx[9] = Mix_LoadWAV("sfx/rocks.ogg");
		sys_progress(17, 21);
		sfx[10] = Mix_LoadWAV("sfx/swordhit.ogg");
		sys_progress(18, 21);
		sfx[11] = Mix_LoadWAV("sfx/throw.ogg");
		sys_progress(19, 21);
		sfx[12] = Mix_LoadWAV("sfx/chest.ogg");
		sys_progress(20, 21);
		sfx[13] = Mix_LoadWAV("sfx/fire.ogg");
		sys_progress(21, 21);
		sfx[14] = Mix_LoadWAV("sfx/beep.ogg");
	}
}

void GriffonEngine::sys_update() {
	g_system->updateScreen();
	g_system->getEventManager()->pollEvent(_event);

	_tickspassed = _ticks;
	_ticks = g_system->getMillis();

	_tickspassed = _ticks - _tickspassed;
	_fpsr = (float)_tickspassed / 24.0;

	_fp++;
	if (_ticks > _nextticks) {
		_nextticks = _ticks + 1000;
		_fps = _fp;
		_fp = 0;
		secsingame = secsingame + 1;
	}

	if (attacking) {
		_player.attackframe += _player.attackspd * _fpsr;
		if (_player.attackframe >= 16) {
			attacking = false;
			_player.attackframe = 0;
			_player.walkframe = 0;
		}

		int pa = (int)(_player.attackframe);

		for (int i = 0; i <= pa; i++) {
			if (ABS(_playerattackofs[_player.walkdir][i][2]) < kEpsilon) {
				_playerattackofs[_player.walkdir][i][2] = 1;

				float opx = _player.px;
				float opy = _player.py;

				_player.px = _player.px + _playerattackofs[_player.walkdir][i][0];
				_player.py = _player.py + _playerattackofs[_player.walkdir][i][1];

				int sx = (int)(_player.px / 2 + 6);
				int sy = (int)(_player.py / 2 + 10);
				uint32 *temp = (uint32 *)_clipbg->getBasePtr(sx, sy);
				uint32 bgc = *temp;
				if (bgc > 0) {
					_player.px = opx;
					_player.py = opy;
				}
			}
		}

		_player.opx = _player.px;
		_player.opy = _player.py;

		game_checkhit();
	}

	for (int i = 0; i < kMaxFloat; i++) {
		if (floattext[i][0] > 0) {
			float spd = 0.5 * _fpsr;
			floattext[i][0] = floattext[i][0] - spd;
			floattext[i][2] = floattext[i][2] - spd;
			if (floattext[i][0] < 0)
				floattext[i][0] = 0;
		}

		if (floaticon[i][0] > 0) {
			float spd = 0.5 * _fpsr;
			floaticon[i][0] = floaticon[i][0] - spd;
			floaticon[i][2] = floaticon[i][2] - spd;
			if (floaticon[i][0] < 0)
				floaticon[i][0] = 0;
		}
	}

	if (_player.level == _player.maxlevel)
		_player.exp = 0;

	if (_player.exp >= _player.nextlevel) {
		_player.level = _player.level + 1;
		game_addFloatText("LEVEL UP!", _player.px + 16 - 36, _player.py + 16, 3);
		_player.exp = _player.exp - _player.nextlevel;
		_player.nextlevel = _player.nextlevel * 3 / 2; // 1.5
		_player.maxhp = _player.maxhp + _player.level * 3;
		if (_player.maxhp > 999)
			_player.maxhp = 999;
		_player.hp = _player.maxhp;

		_player.sworddamage = _player.level * 14 / 10;
		_player.spelldamage = _player.level * 13 / 10;

		if (menabled && config.effects) {
			int snd = Mix_PlayChannel(sfx[sndpowerup]);
			Mix_Volume(snd, config.effectsvol);
		}
	}

	_clipbg->copyRectToSurface(_clipbg2->getPixels(), _clipbg2->pitch, 0, 0, _clipbg2->w, _clipbg2->h);

	Common::Rect rc;

	rc.left = _player.px - 2;
	rc.top = _player.py - 2;
	rc.setWidth(5);
	rc.setHeight(5);

	_clipbg->fillRect(rc, 1000);

	if (!_forcepause) {
		for (int i = 0; i < 5; i++) {
			if (_player.foundspell[i] == 1)
				_player.spellcharge[i] += 1 * _player.level * 0.01 * _fpsr;
			if (_player.spellcharge[i] > 100)
				_player.spellcharge[i] = 100;
		}

		if (_player.foundspell[0]) {
			_player.spellstrength += 3 * _player.level * .01 * _fpsr;
		}

		_player.attackstrength += (30 + 3 * (float)_player.level) / 50 * _fpsr;
	}

	if (_player.attackstrength > 100)
		_player.attackstrength = 100;

	if (_player.spellstrength > 100)
		_player.spellstrength = 100;

	_itemyloc += 0.75 * _fpsr;
	while (_itemyloc >= 16)
		_itemyloc -= 16;

	if (_player.hp <= 0)
		game_theend();

	if (_roomlock == 1) {
		_roomlock = 0;
		for (int i = 1; i <= lastnpc; i++)
			if (npcinfo[i].hp > 0)
				_roomlock = 1;
	}

	clouddeg += 0.1 * _fpsr;
	while (clouddeg >= 360)
		clouddeg = clouddeg - 360;

	_player.hpflash = _player.hpflash + 0.1 * _fpsr;
	if (_player.hpflash >= 2) {
		_player.hpflash = 0;
		_player.hpflashb = _player.hpflashb + 1;
		if (_player.hpflashb == 2)
			_player.hpflashb = 0;
		if (menabled && config.effects && _player.hpflashb == 0 && _player.hp < _player.maxhp / 4) {
			int snd = Mix_PlayChannel(sfx[sndbeep]);
			Mix_Volume(snd, config.effectsvol);
		}
	}

	// cloudson = 0

	if (_itemselon == 1)
		_player.itemselshade = _player.itemselshade + 2 * _fpsr;
	if (_player.itemselshade > 24)
		_player.itemselshade = 24;

	for (int i = 0; i <= 4; i++)
		if (_player.inventory[i] > 9)
			_player.inventory[i] = 9;
}

} // end of namespace Griffon
