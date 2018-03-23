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

#ifndef SUPERNOVA_RESOURCES_H
#define SUPERNOVA_RESOURCES_H

#include "common/ptr.h"

#include "supernova/sound.h"

namespace Audio {
class SeekableAudioStream;
}

namespace Supernova {

class ResourceManager {
public:
	ResourceManager(SupernovaEngine *vm);
	~ResourceManager();

	Audio::SeekableAudioStream *getSoundStream(AudioIndex index);
	Audio::AudioStream *getSoundStream(MusicIndex index);

private:
	void initSoundFiles();
	void initGraphics();
	void initCursorGraphics();

private:
	SupernovaEngine *_vm;
	Audio::SeekableAudioStream *_soundSamples[kAudioNumSamples];
	Audio::AudioStream *_musicIntro;
	Audio::AudioStream *_musicOutro;
	int _audioRate;
	byte _cursorNormal[256];
	byte _cursorWait[256];
};

}

#endif
