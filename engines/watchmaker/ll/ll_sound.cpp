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

#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

bool mInitMusicSystem(void) {
	warning("STUBBED: mInitMusicSystem");
	return true;
}

bool mCloseMusicSystem(void) {
	warning("STUBBED: mCloseMusicSystem");
	return true;
}

bool mLoadMusic(const char *FileName) {
	warning("STUBBED: mLoadMusic");
	return true;
}

bool mPlayMusic(const char *FileName) {
	warning("STUBBED: mPlayMusic");
	return true;
}

bool mStopMusic(void) {
	warning("STUBBED: mStopMusic");
	return true;
}

bool mRestoreMixerVolume(void) {
	warning("STUBBED: mRestoreMixerVolume");
	return true;
}

bool sSetListener(sListener *NewListener) {
	warning("STUBBED: sSetListener");
	return true;
}

bool sStartSound(sSound *CurSound, bool Reload) {
	warning("STUBBED: sStartSound");
	return true;
}

bool sStopSound(int32 index) {
	warning("STUBBED: sStopSound");
	return true;
}

bool sStopAllSounds(void) {
	warning("STUBBED: sStopAllSounds");
	return true;
}

bool sIsPlaying(sS32 lIndex) {
	warning("STUBBED: sIsPlaying");
	return true;
}

bool mSetAllVolume(unsigned char Volume) {
	warning("STUBBED: mSetAllVolume");
	return true;
}

bool sSetAllSoundsVolume(unsigned char Vol) {
	warning("STUBBED: sSetAllSoundsVolume");
	return true;
}

bool sSetAllSpeechVolume(unsigned char Vol) {
	warning("STUBBED: sSetAllSpeechVolume");
	return true;
}

bool sStartSoundDiffuse(sSound *CurSound) {
	warning("STUBBED: sStartSoundDiffuse");
	return true;
}

} // End of namespace Watchmaker
