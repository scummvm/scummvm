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

#ifndef GNAP_SOUND_H
#define GNAP_SOUND_H

#include "gnap/gnap.h"
#include "gnap/resource.h"
#include "audio/mixer.h"
#include "common/array.h"

namespace Gnap {

struct SoundItem {
	int _resourceId;
	Audio::SoundHandle _handle;
};

class SoundMan {
public:
	SoundMan(GnapEngine *vm);
	~SoundMan();
	void playSound(int resourceId, bool looping);
	void stopSound(int resourceId);
	void setSoundVolume(int resourceId, int volume);
	bool isSoundPlaying(int resourceId);
	void stopAll();
	void update();
protected:
	GnapEngine *_vm;
	Common::Array<SoundItem> _items;

	int find(int resourceId);
};

} // End of namespace Gnap

#endif // GNAP_SOUND_H
