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

#ifndef WINTERMUTE_PLUGIN_EVENT_H
#define WINTERMUTE_PLUGIN_EVENT_H

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
// WME events
typedef enum {
	WME_EVENT_UPDATE = 0,
	WME_EVENT_SCENE_DRAW_BEGIN,
	WME_EVENT_SCENE_DRAW_END,
	WME_EVENT_SCENE_INIT,
	WME_EVENT_SCENE_SHUTDOWN,
	WME_EVENT_GAME_BEFORE_SAVE,
	WME_EVENT_GAME_AFTER_LOAD,
	WME_EVENT_MAX
} EWmeEvent;

typedef void (*PluginApplyEvent)(void *, void *);

typedef struct _PluginEventEntry {
	EWmeEvent        _type;
	PluginApplyEvent _callback;
	void             *_plugin;
} PluginEventEntry;

class PluginEvent {
public:
	PluginEvent() {};
	~PluginEvent() {};

	void subscribeEvent(PluginEventEntry &event) {
		for (auto it = _entries.begin(); it != _entries.end(); ++it) {
			if (event._type == (*it)._type && event._callback == (*it)._callback) {
				break;
			}
		}
		_entries.push_back(event);
	}

	void unsubscribeEvent(PluginEventEntry &event) {
		for (auto it = _entries.begin(); it != _entries.end(); ++it) {
			if (event._type == (*it)._type && event._callback == (*it)._callback) {
				_entries.erase(it);
				break;
			}
		}
	}

	void applyEvent(EWmeEvent type, void *eventData) {
		for (auto it = _entries.begin(); it != _entries.end(); ++it) {
			if (type == (*it)._type && (*it)._callback != nullptr) {
				(*it)._callback(eventData, (*it)._plugin);
			}
		}
	}

	void clearEvents() {
		_entries.clear();
	}

private:
	Common::List<PluginEventEntry> _entries;
};

} // End of namespace Wintermute

#endif
