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

void GriffonEngine::setChannelVolume(int channel, int volume) {}

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
	_mixer->stopHandle(_handles[channel]);
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

void GriffonEngine::drawProgress(int w, int wm) {
	long ccc = _videobuffer->format.RGBToColor(0, 255, 0);

	rcDest.setWidth(w * 74 / wm);
	_videobuffer->fillRect(rcDest, ccc);

	g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
	g_system->updateScreen();

	g_system->getEventManager()->pollEvent(_event);
}

void GriffonEngine::setupAudio() {
	// FIXME
	//Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);

	//atexit(Mix_Quit);
	//atexit(Mix_CloseAudio);

	const char *stri = "Loading...";
	drawString(_videobuffer, stri, 160 - 4 * strlen(stri), 116, 0);

	Graphics::TransparentSurface *loadimg = loadImage("art/load.bmp", true);

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

	_mboss = cacheSound("music/boss.ogg");
	drawProgress(1, 21);
	_mgardens = cacheSound("music/gardens.ogg");
	drawProgress(2, 21);
	_mgardens2 = cacheSound("music/gardens2.ogg");
	drawProgress(3, 21);
	_mgardens3 = cacheSound("music/gardens3.ogg");
	drawProgress(4, 21);
	_mgardens4 = cacheSound("music/gardens4.ogg");
	drawProgress(5, 21);
	_mendofgame = cacheSound("music/endofgame.ogg");
	drawProgress(6, 21);
	_mmenu = cacheSound("music/menu.ogg");
	drawProgress(7, 21);

	_sfx[0] = cacheSound("sfx/bite.ogg");
	drawProgress(8, 21);
	_sfx[1] = cacheSound("sfx/crystal.ogg");
	drawProgress(9, 21);
	_sfx[2] = cacheSound("sfx/door.ogg");
	drawProgress(10, 21);
	_sfx[3] = cacheSound("sfx/enemyhit.ogg");
	drawProgress(11, 21);
	_sfx[4] = cacheSound("sfx/ice.ogg");
	drawProgress(12, 21);
	_sfx[5] = cacheSound("sfx/lever.ogg");
	drawProgress(13, 21);
	_sfx[6] = cacheSound("sfx/lightning.ogg");
	drawProgress(14, 21);
	_sfx[7] = cacheSound("sfx/metalhit.ogg");
	drawProgress(15, 21);
	_sfx[8] = cacheSound("sfx/powerup.ogg");
	drawProgress(16, 21);
	_sfx[9] = cacheSound("sfx/rocks.ogg");
	drawProgress(17, 21);
	_sfx[10] = cacheSound("sfx/swordhit.ogg");
	drawProgress(18, 21);
	_sfx[11] = cacheSound("sfx/throw.ogg");
	drawProgress(19, 21);
	_sfx[12] = cacheSound("sfx/chest.ogg");
	drawProgress(20, 21);
	_sfx[13] = cacheSound("sfx/fire.ogg");
	drawProgress(21, 21);
	_sfx[14] = cacheSound("sfx/beep.ogg");
}


} // end of namespace Griffon
