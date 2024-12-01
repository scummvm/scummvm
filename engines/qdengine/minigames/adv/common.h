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

#ifndef QDENGINE_MINIGAMES_ADV_COMMON_H
#define QDENGINE_MINIGAMES_ADV_COMMON_H

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_minigame_interface.h"

namespace QDEngine {

class MinigameManager;

typedef mgVect3<int> mgVect3i;

class QDObject {
	qdMinigameObjectInterface *_obj;

#ifdef _DEBUG
	Common::String name_;
#endif

public:
	QDObject(qdMinigameObjectInterface* obj = 0, const char* name = "") : _obj(obj) {
#ifdef _DEBUG
		name_ = name;
#endif
	}

	const char *getName() const; // DEBUG ONLY
	bool hit(const mgVect2f& point) const;
	float depth(MinigameManager *runtime) const;

	void setState(const char* name);

	bool operator==(const QDObject& obj) const {
		return _obj == obj._obj;
	}
	bool operator==(const qdMinigameObjectInterface* obj) const {
		return _obj == obj;
	}

	operator qdMinigameObjectInterface* () const {
		return _obj;
	}
	qdMinigameObjectInterface* operator->() const {
		return _obj;
	}
};

typedef qdMinigameCounterInterface *QDCounter;

typedef Std::vector<QDObject> QDObjects;
typedef Std::vector<int> Indexes;
typedef Std::vector<mgVect3f> Coords;

class MinigameManager;
extern MinigameManager *g_runtime;

enum SystemEvent {
	EVENT_TIME_1_SECOND_TICK,
	EVENT_TIME_10_SECOND_TICK,
	EVENT_TIME_60_SECOND_TICK,
	EVENT_TIME_10_SECOND_LEFT,
	EVENT_TIME_LESS_10_SECOND_LEFT_SECOND_TICK,
	EVENT_TIME_OUT,
	EVENT_GAME_LOSE,
	EVENT_GAME_WIN,
	SYSTEM_EVENTS_SIZE
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_COMMON_H
