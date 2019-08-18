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

#include "audio/audiostream.h"
#include "common/ptr.h"
#include "common/sinetables.h"

#include "supernova/graphics.h"
#include "supernova/sound.h"
#include "supernova/supernova.h"


namespace Common {
class MemoryReadStream;
}

namespace Supernova {
class SupernovaEngine;

class ResourceManager {
public:
	enum CursorId {
		kCursorNormal,
		kCursorWait
	};

public:
	static const int kNumImageFiles1 = 45;
	static const int kNumImageFiles2 = 47;

public:
	ResourceManager(SupernovaEngine *vm);
	~ResourceManager();

	Audio::SeekableAudioStream *getSoundStream(AudioId index);
	Audio::AudioStream *getSoundStream(MusicId index);
	Audio::AudioStream *getSirenStream();
	MSNImage *getImage(int filenumber);
	const byte *getCursor(CursorId id) const;
	int getAudioRate();

private:
	void initGraphics();
	void initCursorGraphics();
	void initImages1();
	void initImages2();
	void loadImage(int filenumber);
	void loadSound1(AudioId id);
	void loadSound2(AudioId id);
	void initSiren();
	byte *generateTone(byte *buffer, int frequency, int length, int audioRate, Common::SineTable &table);

private:
	Common::ScopedPtr<Audio::SeekableAudioStream> *_soundSamples;
	Common::ScopedPtr<Common::MemoryReadStream> _musicIntroBuffer;
	Common::ScopedPtr<Common::MemoryReadStream> _musicOutroBuffer;
	Common::ScopedPtr<Audio::AudioStream> _musicIntro;
	Common::ScopedPtr<Audio::AudioStream> _musicOutro;
	Common::ScopedPtr<Audio::AudioStream> _sirenStream;
	SupernovaEngine *_vm;
	int _audioRate;
	MSNImage **_images;
	byte _cursorNormal[256];
	byte _cursorWait[256];
};

}

#endif
