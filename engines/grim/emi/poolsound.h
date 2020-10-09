/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_POOLSOUND_H
#define GRIM_POOLSOUND_H

#include "common/endian.h"
#include "engines/grim/pool.h"
#include "math/vector3d.h"

namespace Grim {
class PoolSound : public PoolObject<PoolSound> {
public:
	PoolSound();
	PoolSound(const Common::String &filename);
	~PoolSound();

	void openFile(const Common::String &filename);
	void play(bool looping);
	void playFrom(const Math::Vector3d &pos, bool looping);
	void setVolume(int volume);
	void setBalance(int balance);
	void setPosition(Math::Vector3d &pos);
	void stop();
	int getVolume();
	bool isPlaying();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);

	static int32 getStaticTag() { return MKTAG('A','I','F','F'); }

private:
	Common::String _filename;
	int _soundId;
	bool _loaded;
};
}

#endif
