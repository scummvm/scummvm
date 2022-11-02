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

#ifndef KYRA_SOUND_MACRES_H
#define KYRA_SOUND_MACRES_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/mutex.h"

namespace Common {
	class Archive;
	class MacResManager;
	class SeekableReadStream;
}

namespace Kyra {

class KyraEngine_v1;

class SoundMacRes {
public:
	SoundMacRes(KyraEngine_v1 *vm);
	~SoundMacRes();
	bool init();
	bool setQuality(bool hi);
	Common::SeekableReadStream *getResource(uint16 id, uint32 type);

private:
	Common::String _kyraMacExe;
	Common::MacResManager *_resMan;
	Common::Archive *_stuffItArchive;
	Common::Mutex _mutex;
	const bool _isTalkie;
};

} // End of namespace Kyra

#endif
