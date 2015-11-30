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

#ifndef XEEN_SOUND_H
#define XEEN_SOUND_H

#include "common/scummsys.h"
#include "common/system.h"
#include "xeen/files.h"

namespace Xeen {

class SoundManager;

class VOC: public Common::File {
	friend class SoundManager;
private:
	SoundManager *_sound;
public:
	VOC() : _sound(nullptr) {}
	virtual ~VOC() { stop(); }

	/**
	 * Stop playing the sound
	 */
	void stop();
};

class SoundManager {
private:
	XeenEngine *_vm;
public:
	SoundManager(XeenEngine *vm);

	void proc2(Common::SeekableReadStream &f);

	void loadMusic(const Common::String &name, int v2) {}

	void startMusic(int v1);

	void stopMusic(int id);

	void playSong(Common::SeekableReadStream &f) {}

	void playSound(VOC &voc) {}

	void playSample(const Common::SeekableReadStream *stream, int v2 = 1) {}

	bool playSample(int v1, int v2) { return false; }

	void playFX(int id) {}
};

} // End of namespace Xeen

#endif /* XEEN_SOUND_H */
