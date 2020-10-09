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

#include "common/foreach.h"

#include "engines/grim/debug.h"
#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/animationemi.h"

namespace Grim {

EMIAnimComponent::EMIAnimComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) :
		Component(p, parentID, filename, t), _animState(nullptr) {
}

EMIAnimComponent::~EMIAnimComponent() {
	delete _animState;
}

void EMIAnimComponent::init() {
	_visible = true;
	_animState = new AnimationStateEmi(_name);
}

int EMIAnimComponent::update(uint time) {
	EMISkelComponent *skel = ((EMICostume *)_cost)->_emiSkel;
	if (skel) {
		_animState->setSkeleton(skel->_obj);
		_animState->update(time);
	}
	return 0;
}

void EMIAnimComponent::setKey(int f) {
	switch (f) {
	case 0: // Stop
		_animState->stop();
		break;
	case 1: // Play
		_animState->play();
		break;
	case 2: // Pause
		_animState->setPaused(true);
		break;
	case 3: // Loop
		_animState->setLooping(true);
		_animState->play();
		break;
	case 4: // No loop
		_animState->setLooping(false);
		break;
	case 5: // Fade in 1
		_animState->fade(Animation::FadeIn, 1000);
		break;
	case 6: // Fade in 3/4
		_animState->fade(Animation::FadeIn, 750);
		break;
	case 7: // Fade in 1/2
		_animState->fade(Animation::FadeIn, 500);
		break;
	case 8: // Fade in 1/4
		_animState->fade(Animation::FadeIn, 250);
		break;
	case 9: // Fade in 1/8
		_animState->fade(Animation::FadeIn, 125);
		break;
	case 10: // Fade out 1
		_animState->fade(Animation::FadeOut, 1000);
		break;
	case 11: // Fade out 3/4
		_animState->fade(Animation::FadeOut, 750);
		break;
	case 12: // Fade out 1/2
		_animState->fade(Animation::FadeOut, 500);
		break;
	case 13: // Fade out 1/4
		_animState->fade(Animation::FadeOut, 250);
		break;
	case 14: // Fade out 1/8
		_animState->fade(Animation::FadeOut, 125);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for component %s", f, _name.c_str());
		break;
	}
}

void EMIAnimComponent::reset() {
	_visible = true;
	_animState->stop();
}

void EMIAnimComponent::fade(Animation::FadeMode mode, int fadeLength) {
	_animState->fade(mode, fadeLength);
}

void EMIAnimComponent::advance(uint msecs) {
	_animState->advance(msecs);
}

void EMIAnimComponent::setPaused(bool paused) {
	_animState->setPaused(paused);
}

void EMIAnimComponent::draw() {
}

void EMIAnimComponent::saveState(SaveGame *state) {
	_animState->saveState(state);
}

void EMIAnimComponent::restoreState(SaveGame *state) {
	_animState->restoreState(state);
}

} // end of namespace Grim
