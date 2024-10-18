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
	score_ = 0;

	char str_cache[256];

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_trigger_%d", idx);
		if (const char *descr = runtime->parameter(str_cache, false))
			triggerEvents_.push_back(runtime->getObject(descr));
		else
			break;
	}
	debugC(2, kDebugMinigames, "EventManager(): registered %d trigger objects", triggerEvents_.size());

	eventPresets_.resize(SYSTEM_EVENTS_SIZE);
	for (int idx = 0; idx < SYSTEM_EVENTS_SIZE; ++idx) {
		snprintf(str_cache, 127, "system_event_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d", &preset.score, &preset.triggerEventID);

			if (read != 2)
				error("EventManager(): Incorrect description string: %s", str_cache);

			if (read == 2) {
				if (preset.triggerEventID >= (int)triggerEvents_.size())
					error("EventManager(): Reference to an unregistered trigger in %s", str_cache);

				if (preset.triggerEventID < (int)triggerEvents_.size())
					eventPresets_[idx] = preset;
			}
		}
	}

	for (int idx = 0;; ++idx) {
		snprintf(str_cache, 127, "register_event_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d %d %d", &preset.score, &preset.fontID, &preset.escapeID, &preset.triggerEventID);

			if (read != 4)
				error("EventManager(): Incorrect event description string: %d", idx);

			if (preset.triggerEventID >= (int)triggerEvents_.size())
				error("EventManager(): Reference to an unregistered trigger in %s", str_cache);

			if (read == 4 && preset.triggerEventID < (int)triggerEvents_.size())
				eventPresets_.push_back(preset);
			else
				eventPresets_.push_back(EventPreset());
		} else
			break;
	}
	debugC(2, kDebugMinigames, "EventManager(): registered %d events", eventPresets_.size());

	if (const char * data = runtime->parameter("allow_negative", false)) {
		int tmp;
		sscanf(data, "%d", &tmp);
		enableNegative_ = tmp;
	} else
		enableNegative_ = false;
}

void EventManager::sysEvent(int eventID) {
	assert(eventID >= 0);
	debugC(6, kDebugMinigames, "EventManager() System event: %d", eventID);

	assert(eventID < SYSTEM_EVENTS_SIZE);

	mgVect2i pos = runtime->screenSize() / 2;
	event(eventID - SYSTEM_EVENTS_SIZE, mgVect2f(pos.x, pos.y), 1);
}

void EventManager::event(int eventID, const mgVect2f& pos, int factor) {
	debugC(6, kDebugMinigames, "EventManager() Event: %d, pos=(%5.1f, %5.1f), fartor=%d", eventID, pos.x, pos.y, factor);

	eventID += SYSTEM_EVENTS_SIZE;

	if (eventID >= eventPresets_.size())
		return;

	const EventPreset& pr = eventPresets_[eventID];

	if (pr.triggerEventID >= 0) {
		assert(pr.triggerEventID < triggerEvents_.size());
		triggerEvents_[pr.triggerEventID]->set_state("on");
	}

	if (pr.score) {
		int diff = addScore(pr.score);

		if (pr.fontID >= 0 && pr.escapeID >= 0 && diff != 0)
			runtime->textManager().showNumber(diff, pos, pr.fontID, pr.escapeID);
	}
}

int EventManager::addScore(int sc) {
	int diff = score_;

	score_ += sc;
	if (score_ < 0 && !enableNegative_)
		score_ = 0;

	diff = score_ - diff;

	if (diff)
		runtime->textManager().updateScore(score_);

	return diff;
}

} // namespace QDEngine
