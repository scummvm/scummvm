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

#ifndef QDENGINE_MINIGAMES_ADV_EVENT_MANAGER_H
#define QDENGINE_MINIGAMES_ADV_EVENT_MANAGER_H

namespace QDEngine {

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

class EventManager {
public:
	EventManager();

	void sysEvent(int eventID);
	void event(int eventID, const mgVect2f& pos, int factor);

	int score() const {
		return score_;
	}
	int addScore(int sc);

private:
	int score_;
	bool enableNegative_;

	struct EventPreset {
		EventPreset();
		int score;
		int fontID;
		int escapeID;
		int triggerEventID;
	};
	typedef vector<EventPreset> EventPresets;
	EventPresets eventPresets_;

	QDObjects triggerEvents_;
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_EVENT_MANAGER_H
