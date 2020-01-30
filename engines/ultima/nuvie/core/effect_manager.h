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

#ifndef NUVIE_CORE_EFFECT_MANAGER_H
#define NUVIE_CORE_EFFECT_MANAGER_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Nuvie {

class Effect;

/* This just keeps a list of Effect pointers, and deletes them when requested.
 */
class EffectManager {
	friend class Effect;
	typedef Std::vector<Effect *>::iterator EffectIterator;
	/* For each EffectWatch, a message will be sent to "watcher" when
	   "effect" is deleted. */
	typedef struct {
		CallBack *watcher;
		Effect *effect;
	} EffectWatch;
	typedef Std::vector<EffectWatch>::iterator WatchIterator;

	Std::vector<Effect *> effects; // the simple list
	Std::vector<EffectWatch> watched;

	void add_effect(Effect *eff); // only effects can add themselves
	void signal_watch(Effect *effect);
	EffectWatch *find_effect_watch(Effect *effect);

public:
	EffectManager();
	~EffectManager();

	void delete_effect(Effect *eff); // anyone may delete an effect
	void update_effects(); // check and delete

	bool has_effects();
	void watch_effect(CallBack *callback_target, Effect *watch);
	void unwatch_effect(CallBack *callback_target, Effect *watch = NULL);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
