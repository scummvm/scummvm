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

#ifndef AGDS_SOUND_MANAGER_H
#define AGDS_SOUND_MANAGER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "audio/mixer.h"

namespace Common	{ class SeekableReadStream; }
namespace Audio		{ class Mixer; }

namespace AGDS {
	class AGDSEngine;

	struct Sound {
		int 				id;
		Common::String		process;
		Common::String		name;
		Common::String		phaseVar;
		Audio::SoundHandle	handle;
		int					group;
		Sound(int id_, const Common::String &p, const Common::String & res, const Common::String & var, Audio::SoundHandle h, int g = 0):
			id(id_), process(p), name(res), phaseVar(var), handle(h), group(g) {
		}
	};

	class SoundManager {
		typedef Common::List<Sound> SoundList;
		int							_nextId;
		AGDSEngine *				_engine;
		Audio::Mixer *				_mixer;
		SoundList					_sounds;

	public:
		SoundManager(AGDSEngine *engine, Audio::Mixer *mixer): _nextId(1), _engine(engine), _mixer(mixer) { }
		void tick();
		int play(const Common::String &process, const Common::String &file, const Common::String &phaseVar);
		bool playing(int id) const;
		void stopAll();
	};

} // End of namespace AGDS

#endif /* AGDS_SOUND_MANAGER_H */
