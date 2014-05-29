/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/objectstate.h"
#include "engines/grim/grim.h"
#include "engines/grim/debug.h"
#include "engines/grim/set.h"

#include "engines/grim/costume/anim_component.h"

namespace Grim {


AnimComponent::AnimComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t) {
	_overlay = false;
	_created = false;
	const char *comma = strchr(filename, ',');
	if (comma) {
		_name = Common::String(filename, comma);
		_overlay = atoi(comma + 1) == 1;
	}
}

void AnimComponent::setKey(int val) {
	ObjectState *state = g_grim->getCurrSet()->findState(_name);

	if (!state) {
		Set *set = g_grim->getCurrSet();
		state = set->addObjectState(set->getSetup(), (_overlay ? ObjectState::OBJSTATE_OVERLAY : ObjectState::OBJSTATE_UNDERLAY),
									_name.c_str(), nullptr, false);
	}
	_created = true;

	if (state) {
		state->setActiveImage(val);
		return;
	}
	// Complain that we couldn't find the bitmap.  This means we probably
	// didn't handle something correctly.  Example: Before the tube-switcher
	// bitmaps were not loading with the scene. This was because they were requested
	// as a different case then they were stored (tu_0_dorcu_door_open versus
	// TU_0_DORCU_door_open), which was causing problems in the string comparison.
	Debug::warning(Debug::Bitmaps | Debug::Costumes, "Missing scene bitmap: %s", _name.c_str());

/* In case you feel like drawing the missing bitmap anyway...
	// Assume that all objects the scene file forgot about are OBJSTATE_STATE class
	state = new ObjectState(0, ObjectState::OBJSTATE_STATE, bitmap, NULL, true);
	if (!state) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Couldn't find bitmap %s in current scene", _filename.c_str());
		return;
	}
	g_grim->getCurrSet()->addObjectState(state);
	state->setNumber(val);
*/
}

void AnimComponent::reset() {
	if (_created) {
		setKey(0);
	}
}

} // end of namespace Grim
