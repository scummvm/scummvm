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
		_snprintf(str_cache, 127, "register_trigger_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false))
			triggerEvents_.push_back(runtime->getObject(descr));
		else
			break;
	}
	dprintf("registered %d trigger objects\n", triggerEvents_.size());

	eventPresets_.resize(SYSTEM_EVENTS_SIZE);
	for (int idx = 0; idx < SYSTEM_EVENTS_SIZE; ++idx) {
		_snprintf(str_cache, 127, "system_event_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d", &preset.score, &preset.triggerEventID);
			xxassert(read == 2, (XBuffer() < "Неверная строка для описания" < str_cache).c_str());
			if (read == 2) {
				xxassert(preset.triggerEventID < (int)triggerEvents_.size(), (XBuffer() < "Ссылка на незарегистрированный триггер в " < str_cache).c_str());
				if (preset.triggerEventID < (int)triggerEvents_.size())
					eventPresets_[idx] = preset;
			}
		}
	}

	for (int idx = 0;; ++idx) {
		_snprintf(str_cache, 127, "register_event_%d", idx);
		if (const char * descr = runtime->parameter(str_cache, false)) {
			EventPreset preset;
			int read = sscanf(descr, "%d %d %d %d", &preset.score, &preset.fontID, &preset.escapeID, &preset.triggerEventID);
			xxassert(read == 4, (XBuffer() < "Неверная строка для описания события " < idx).c_str());
			xxassert(preset.triggerEventID < (int)triggerEvents_.size(), (XBuffer() < "Ссылка на незарегистрированный триггер в " < str_cache).c_str());
			if (read == 4 && preset.triggerEventID < (int)triggerEvents_.size())
				eventPresets_.push_back(preset);
			else
				eventPresets_.push_back(EventPreset());
		} else
			break;
	}
	dprintf("registered %d events\n", eventPresets_.size());

	if (const char * data = runtime->parameter("allow_negative", false)) {
		int tmp;
		sscanf(data, "%d", &tmp);
		enableNegative_ = tmp;
	} else
		enableNegative_ = false;
}

void EventManager::sysEvent(int eventID) {
	xassert(eventID >= 0);
	//dprintf("System event: %d\n", eventID);

	xassert(eventID < SYSTEM_EVENTS_SIZE);

	mgVect2i pos = runtime->screenSize() / 2;
	event(eventID - SYSTEM_EVENTS_SIZE, mgVect2f(pos.x, pos.y), 1);
}

void EventManager::event(int eventID, const mgVect2f& pos, int factor) {
	//dprintf("Event: %d, pos=(%5.1f, %5.1f), fartor=%d\n", eventID, pos.x, pos.y, factor);

	eventID += SYSTEM_EVENTS_SIZE;

	if (eventID >= eventPresets_.size())
		return;

	const EventPreset& pr = eventPresets_[eventID];

	if (pr.triggerEventID >= 0) {
		xassert(pr.triggerEventID < triggerEvents_.size());
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
