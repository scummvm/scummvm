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

#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/EventManager.h"
#include "qdengine/minigames/adv/RunTime.h"
#include "qdengine/minigames/adv/TextManager.h"

namespace QDEngine {

EventManager::EventPreset::EventPreset() {
	score = 0;
	fontID = -1;
	escapeID = -1;
	triggerEventID = -1;
}

EventManager::EventManager() {
	_score = 0;

	char str_cache[256];

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_trigger_%d", idx);
		if (const char *descr = g_runtime->parameter(str_cache, false))
			_triggerEvents.push_back(g_runtime->getObject(descr));
		else
			break;
	}
	debugC(2, kDebugMinigames, "EventManager(): registered %d trigger objects", _triggerEvents.size());

	_eventPresets.resize(SYSTEM_EVENTS_SIZE);
	for (int idx = 0; idx < SYSTEM_EVENTS_SIZE; ++idx) {
		snprintf(str_cache, 127, "system_event_%d", idx);
		if (const char * descr = g_runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d", &preset.score, &preset.triggerEventID);

			if (read != 2)
				warning("EventManager(): Incorrect description string: %s", str_cache);

			if (read == 2) {
				if (preset.triggerEventID >= (int)_triggerEvents.size())
					error("EventManager(): Reference to an unregistered trigger in %s", str_cache);

				if (preset.triggerEventID < (int)_triggerEvents.size())
					_eventPresets[idx] = preset;
			}
		}
	}

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_event_%d", idx);
		if (const char * descr = g_runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d %d %d", &preset.score, &preset.fontID, &preset.escapeID, &preset.triggerEventID);

			if (read != 4)
				warning("EventManager(): Incorrect event description string: %d", idx);

			if (preset.triggerEventID >= (int)_triggerEvents.size())
				error("EventManager(): Reference to an unregistered trigger in %s", str_cache);

			if (read == 4 && preset.triggerEventID < (int)_triggerEvents.size())
				_eventPresets.push_back(preset);
			else
				_eventPresets.push_back(EventPreset());
		} else
			break;
	}
	debugC(2, kDebugMinigames, "EventManager(): registered %d events", _eventPresets.size());

	if (const char * data = g_runtime->parameter("allow_negative", false)) {
		int tmp;
		sscanf(data, "%d", &tmp);
		_enableNegative = tmp;
	} else
		_enableNegative = false;
}

void EventManager::sysEvent(int eventID) {
	assert(eventID >= 0);
	debugC(6, kDebugMinigames, "EventManager() System event: %d", eventID);

	assert(eventID < SYSTEM_EVENTS_SIZE);

	mgVect2i pos = g_runtime->screenSize() / 2;
	event(eventID - SYSTEM_EVENTS_SIZE, mgVect2f(pos.x, pos.y), 1);
}

void EventManager::event(int eventID, const mgVect2f& pos, int factor) {
	debugC(6, kDebugMinigames, "EventManager() Event: %d, pos=(%5.1f, %5.1f), fartor=%d", eventID, pos.x, pos.y, factor);

	eventID += SYSTEM_EVENTS_SIZE;

	if (eventID >= (int)_eventPresets.size())
		return;

	const EventPreset& pr = _eventPresets[eventID];

	if (pr.triggerEventID >= 0) {
		assert(pr.triggerEventID < (int)_triggerEvents.size());
		_triggerEvents[pr.triggerEventID]->set_state("on");
	}

	if (pr.score) {
		int diff = addScore(pr.score);

		if (pr.fontID >= 0 && pr.escapeID >= 0 && diff != 0)
			g_runtime->textManager().showNumber(diff, pos, pr.fontID, pr.escapeID);
	}
}

int EventManager::addScore(int sc) {
	int diff = _score;

	_score += sc;
	if (_score < 0 && !_enableNegative)
		_score = 0;

	diff = _score - diff;

	if (diff)
		g_runtime->textManager().updateScore(_score);

	return diff;
}

} // namespace QDEngine
