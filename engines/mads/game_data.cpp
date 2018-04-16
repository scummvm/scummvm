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
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/nebular/game_nebular.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"

namespace MADS {

void VisitedScenes::add(int sceneId) {
	_sceneRevisited = exists(sceneId);

	if (!_sceneRevisited)
		push_back(sceneId);
}

bool VisitedScenes::exists(int sceneId) {
	for (uint i = 0; i < size(); ++i) {
		if ((*this)[i] == sceneId)
			return true;
	}

	return false;
}

void VisitedScenes::synchronize(Common::Serializer &s, int sceneId) {
	SynchronizedList::synchronize(s);
	s.syncAsByte(_sceneRevisited);

	// If the scene hasn't been visited yet, remove it from the visited
	// scenes list. It'll be readded to the list in add() above, from
	// Game::sectionLoop()
	if (s.isLoading() && !_sceneRevisited) {
		for (uint i = 0; i < size(); ++i) {
			if ((*this)[i] == sceneId) {
				remove_at(i);
				return;
			}
		}
	}
}

} // End of namespace MADS
