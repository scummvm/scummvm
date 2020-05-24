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

#include "common/file.h"
#include "common/system.h"

#include "griffon/griffon.h"

namespace Griffon {

void GriffonEngine::setChannelVolume(int channel, int volume) {
	_mixer->setChannelVolume(_handles[channel], volume);
}

int GriffonEngine::getSoundHandle() {
	for (uint i = 0; i < kSoundHandles; i++) {
		if (!_mixer->isSoundHandleActive(_handles[i])) {
			return i;
		}
	}

	error("getSoundHandle(): Too many sound handles");

	return -1;
}

int GriffonEngine::playSound(DataChunk *chunk, bool looped) {
	int ch = getSoundHandle();

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

void GriffonEngine::pauseSoundChannel(int channel) {
	_mixer->pauseHandle(_handles[channel], true);
}

void GriffonEngine::haltSoundChannel(int channel) {
	if (channel == -1) {
		for (int i = 0; i < kSoundHandles; i++)
			_mixer->stopHandle(_handles[i]);
	} else {
		_mixer->stopHandle(_handles[channel]);
	}
}

void GriffonEngine::resumeSoundChannel(int channel) {
	_mixer->pauseHandle(_handles[channel], false);
}

bool GriffonEngine::isSoundChannelPlaying(int channel) {
	return _mixer->isSoundHandleActive(_handles[channel]);
}

DataChunk *cacheSound(const char *name) {
	Common::File file;
	DataChunk *res = new DataChunk;

	file.open(name);
	if (!file.isOpen())
		error("Cannot open file %s", name);

	res->size = file.size();
	res->data = (byte *)malloc(res->size);

	file.read(res->data, res->size);

	return res;
}

void GriffonEngine::setupAudio() {
	const char *stri = "Loading...";
	drawString(_videoBuffer, stri, 160 - 4 * strlen(stri), 116, 0);

	Graphics::TransparentSurface *loadimg = loadImage("art/load.bmp", true);

	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.setWidth(88);
	rcSrc.setHeight(32);

	rcDest.left = 160 - 44;
	rcDest.top = 116 + 12;

	loadimg->blit(*_videoBuffer, rcDest.left, rcDest.top, Graphics::FLIP_NONE, &rcSrc, TS_ARGB(160, 255, 255, 255));

	g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
	g_system->updateScreen();

	rcDest.left = 160 - 44 + 7;
	rcDest.top = 116 + 12 + 12;
	rcDest.setHeight(8);

	_musicBoss = cacheSound("music/boss.ogg");
	drawProgress(1, 21);
	_musicGardens1 = cacheSound("music/gardens.ogg");
	drawProgress(2, 21);
	_musicGardens2 = cacheSound("music/gardens2.ogg");
	drawProgress(3, 21);
	_musicGardens3 = cacheSound("music/gardens3.ogg");
	drawProgress(4, 21);
	_musicGardens4 = cacheSound("music/gardens4.ogg");
	drawProgress(5, 21);
	_musicEndOfGame = cacheSound("music/endofgame.ogg");
	drawProgress(6, 21);
	_musicMenu = cacheSound("music/menu.ogg");
	drawProgress(7, 21);

	_sfx[kSndBite] = cacheSound("sfx/bite.ogg");
	drawProgress(8, 21);
	_sfx[kSndCrystal] = cacheSound("sfx/crystal.ogg");
	drawProgress(9, 21);
	_sfx[kSndDoor] = cacheSound("sfx/door.ogg");
	drawProgress(10, 21);
	_sfx[kSndEnemyHit] = cacheSound("sfx/enemyhit.ogg");
	drawProgress(11, 21);
	_sfx[kSndIce] = cacheSound("sfx/ice.ogg");
	drawProgress(12, 21);
	_sfx[kSndLever] = cacheSound("sfx/lever.ogg");
	drawProgress(13, 21);
	_sfx[kSndLightning] = cacheSound("sfx/lightning.ogg");
	drawProgress(14, 21);
	_sfx[kSndMetalHit] = cacheSound("sfx/metalhit.ogg");
	drawProgress(15, 21);
	_sfx[kSndPowerUp] = cacheSound("sfx/powerup.ogg");
	drawProgress(16, 21);
	_sfx[kSndRocks] = cacheSound("sfx/rocks.ogg");
	drawProgress(17, 21);
	_sfx[kSndSwordHit] = cacheSound("sfx/swordhit.ogg");
	drawProgress(18, 21);
	_sfx[kSndThrow] = cacheSound("sfx/throw.ogg");
	drawProgress(19, 21);
	_sfx[kSndChest] = cacheSound("sfx/chest.ogg");
	drawProgress(20, 21);
	_sfx[kSndFire] = cacheSound("sfx/fire.ogg");
	drawProgress(21, 21);
	_sfx[kSndBeep] = cacheSound("sfx/beep.ogg");
}

void GriffonEngine::updateMusic() {
	static int loopseta = 0;

	DataChunk *iplaysound = NULL;

	if (config.music) {
		// if(_curmap > 5 && _curmap < 42) iplaysound = macademy;
		// if(_curmap > 47) iplaysound = _mgardens;
		iplaysound = _musicGardens1;
		if (_roomLock)
			iplaysound = _musicBoss;

		if (iplaysound == _musicBoss && _playingBoss)
			iplaysound = NULL;
		if (iplaysound == _musicGardens1 && _playingGardens)
			iplaysound = NULL;

		if (iplaysound != NULL) {
			haltSoundChannel(_musicChannel);

			_playingBoss = (iplaysound == _musicBoss);
			_playingGardens = (iplaysound == _musicGardens1);

			_musicChannel = playSound(iplaysound, true);
			setChannelVolume(_musicChannel, config.musicVol);
		} else {
			if (!isSoundChannelPlaying(_musicChannel)) {
				loopseta += 1;
				if (loopseta == 4)
					loopseta = 0;

				if (_playingGardens) {
					haltSoundChannel(_musicChannel);
					if (_playingGardens) {
						switch (loopseta) {
						case 0:
							playSound(_musicGardens1);
							break;
						case 1:
							playSound(_musicGardens2);
							break;
						case 2:
							playSound(_musicGardens3);
							break;
						case 3:
							playSound(_musicGardens4);
						default:
							break;
						}
					}
				}

				setChannelVolume(_musicChannel, config.musicVol);
			}
		}
	}
}


} // end of namespace Griffon
