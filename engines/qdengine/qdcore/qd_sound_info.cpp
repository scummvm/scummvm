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


#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_sound_info.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

qdSound *qdSoundInfo::sound() const {
	if (name()) {
		if (qdGameScene * p = static_cast<qdGameScene * >(owner(QD_NAMED_OBJECT_SCENE))) {
			if (qdSound * snd = p->get_sound(name()))
				return snd;
		}

		if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
			return p->get_sound(name());
	}

	return NULL;
}
} // namespace QDEngine
