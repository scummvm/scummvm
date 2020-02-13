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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/effect_manager.h"

namespace Ultima {
namespace Nuvie {

EffectManager::EffectManager() {
}

EffectManager::~EffectManager() {
	while (!effects.empty())
		delete_effect(effects.front());
}

/* Delete an effect and remove it from the list.
 */
void EffectManager::delete_effect(Effect *eff) {
	EffectIterator ei = effects.begin();
	while (ei != effects.end()) {
		if (*ei == eff) {
			signal_watch(eff);
			delete eff;
			effects.erase(ei);
			return;
		}
		++ei;
	}
}

/* Add an (already existant) effect to the list.
 */
void EffectManager::add_effect(Effect *eff) {
	effects.push_back(eff);
}

/* Delete completed effects.
 */
void EffectManager::update_effects() {
	EffectIterator ei = effects.begin();
	while (ei != effects.end()) {
		if ((*ei)->is_defunct()/* && !has_message(*ei)*/) {
			signal_watch(*ei);
			if ((*ei)->is_retained() == false) { //if no longer needed by other objects we can delete.
				delete(*ei);
				ei = effects.erase(ei);
			} else ++ei;
		} else ++ei;
	}
}

/* Returns true if there are any effects still active.
 */
bool EffectManager::has_effects() {
	if (!effects.empty()) {
		EffectIterator i = effects.begin();
		while (i != effects.end())
			if (!(*i)->is_defunct()) // effect is still active
				return (true);
	}
	return (false); // no effects, or all effects are complete
}

/* Add a watched effect. This will send effect completion message to the
 * target when the effect is deleted.
 */
void EffectManager::watch_effect(CallBack *callback_target, Effect *watch) {
	EffectWatch new_watch;
	new_watch.watcher = callback_target;
	new_watch.effect = watch;
	watched.push_back(new_watch);
}

/* Remove a watched effect, or all watched effects for target.
 */
void EffectManager::unwatch_effect(CallBack *callback_target, Effect *watch) {
	if (!watched.empty()) {
		WatchIterator i = watched.begin();
		while (i != watched.end())
			if ((*i).watcher == callback_target
			        && ((*i).effect == watch || watch == NULL)) {
				i = watched.erase(i); // resume from next element
			} else ++i;
	}
}

/* Signal effect completion if it is being watched, and stop watching it.
 */
void EffectManager::signal_watch(Effect *effect) {
	EffectWatch *watch = find_effect_watch(effect);
	if (watch) {
		if (watch->watcher)
			watch->watcher->callback(EFFECT_CB_COMPLETE, NULL, effect);
		unwatch_effect(watch->watcher, effect);
	}
}

/* Returns watch for an effect. (or NULL)
 */
EffectManager::EffectWatch *EffectManager::find_effect_watch(Effect *effect) {
	if (!watched.empty()) {
		WatchIterator i = watched.begin();
		while (i != watched.end())
			if ((*i).effect == effect)
				return (&(*i));
			else ++i;
	}
	return (NULL);
}

} // End of namespace Nuvie
} // End of namespace Ultima
