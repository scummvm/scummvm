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
 */

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/audiocd/sdl/sdl-audiocd.h"

#if !SDL_VERSION_ATLEAST(2, 0, 0)

#include "common/textconsole.h"

SdlAudioCDManager::SdlAudioCDManager()
	:
	_cdrom(0),
	_cdTrack(0),
	_cdNumLoops(0),
	_cdStartFrame(0),
	_cdDuration(0),
	_cdEndTime(0),
	_cdStopTime(0) {

}

SdlAudioCDManager::~SdlAudioCDManager() {
	close();
}

bool SdlAudioCDManager::open() {
	close();

	if (openRealCD())
		return true;

	return DefaultAudioCDManager::open();
}

bool SdlAudioCDManager::openCD(int drive) {
	if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
		_cdrom = NULL;
	else {
		_cdrom = SDL_CDOpen(drive);
		// Did it open? Check if _cdrom is NULL
		if (!_cdrom) {
			warning("Couldn't open drive: %s", SDL_GetError());
		} else {
			_cdNumLoops = 0;
			_cdStopTime = 0;
			_cdEndTime = 0;
		}
	}

	return (_cdrom != NULL);
}

void SdlAudioCDManager::close() {
	DefaultAudioCDManager::close();

	if (_cdrom) {
                SDL_CDStop(_cdrom);
                SDL_CDClose(_cdrom);
		_cdrom = 0;
        }
}

void SdlAudioCDManager::stop() {
	DefaultAudioCDManager::stop();

	// Stop CD Audio in 1/10th of a second
	_cdStopTime = SDL_GetTicks() + 100;
	_cdNumLoops = 0;
}

bool SdlAudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're set to only emulate, or have no CD, return here
	if (onlyEmulate || !_cdrom)
		return false;

	if (!numLoops && !startFrame)
		return false;

	// FIXME: Explain this.
	if (duration > 0)
		duration += 5;

	_cdTrack = track;
	_cdNumLoops = numLoops;
	_cdStartFrame = startFrame;

	SDL_CDStatus(_cdrom);
	if (startFrame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, startFrame, 0, duration);
	_cdDuration = duration;
	_cdStopTime = 0;
	_cdEndTime = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;

	return true;
}

bool SdlAudioCDManager::isPlaying() const {
	if (DefaultAudioCDManager::isPlaying())
		return true;

	if (!_cdrom)
		return false;

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) == CD_PLAYING));
}

void SdlAudioCDManager::update() {
	DefaultAudioCDManager::update();

	if (!_cdrom)
		return;

	if (_cdStopTime != 0 && SDL_GetTicks() >= _cdStopTime) {
		SDL_CDStop(_cdrom);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || SDL_GetTicks() < _cdEndTime)
		return;

	if (_cdNumLoops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		_cdNumLoops--;

	if (_cdNumLoops != 0) {
		if (_cdStartFrame == 0 && _cdDuration == 0)
			SDL_CDPlayTracks(_cdrom, _cdTrack, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, _cdTrack, _cdStartFrame, 0, _cdDuration);
		_cdEndTime = SDL_GetTicks() + _cdrom->track[_cdTrack].length * 1000 / CD_FPS;
	}
}

#endif // !SDL_VERSION_ATLEAST(2, 0, 0)

#endif
