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


#include "engines/grim/debug.h"
#include "engines/grim/costume/keyframe_component.h"
#include "engines/grim/costume/model_component.h"

namespace Grim {

KeyframeComponent::KeyframeComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t), _priority1(1), _priority2(5), _anim(nullptr) {
	const char *comma = strchr(filename, ',');
	if (comma) {
		_name = Common::String(filename, comma);
		sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
	}
}

KeyframeComponent::~KeyframeComponent() {
	delete _anim;
}

void KeyframeComponent::fade(Animation::FadeMode fadeMode, int fadeLength) {
	_anim->fade(fadeMode, fadeLength);
}

void KeyframeComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play Once"
		_anim->play(Animation::Once);
		break;
	case 1: // "Play Looping"
		_anim->play(Animation::Looping);
		break;
	case 2: // "Play and Endpause"
		_anim->play(Animation::PauseAtEnd);
		break;
	case 3: // "Play and Endfade"
		_anim->play(Animation::FadeAtEnd);
		break;
	case 4: // "Stop"
		reset();
		break;
	case 5: // "Pause"
		_anim->pause(true);
		break;
	case 6: // "Unpause"
		_anim->pause(false);
		break;
	case 7: // "1.0 Fade in"
		fade(Animation::FadeIn, 1000);
		_anim->activate();
		break;
	case 8: // "0.5 Fade in"
		fade(Animation::FadeIn, 500);
		_anim->activate();
		break;
	case 9: // "0.25 Fade in"
		fade(Animation::FadeIn, 250);
		_anim->activate();
		break;
	case 10: // "0.125 Fade in"
		fade(Animation::FadeIn, 125);
		_anim->activate();
		break;
	case 11: // "1.0 Fade out"
		fade(Animation::FadeOut, 1000);
		break;
	case 12: // "0.5 Fade out
		fade(Animation::FadeOut, 500);
		break;
	case 13: // "0.25 Fade out"
		fade(Animation::FadeOut, 250);
		break;
	case 14: // "0.125 Fade out"
		fade(Animation::FadeOut, 125);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for component %s", val, _name.c_str());
	}
}

void KeyframeComponent::reset() {
	if (_anim->getFadeMode() != Animation::FadeOut) {
		_anim->stop();
	}
}

int KeyframeComponent::update(uint time) {
	if (!_anim->getIsActive())
		return 0;

	return _anim->update((int)time);
}

void KeyframeComponent::init() {
	if (_parent->isComponentType('M','M','D','L') ||
		_parent->isComponentType('M','O','D','L')) {
		ModelComponent *mc = static_cast<ModelComponent *>(_parent);
		_anim = new Animation(_name, mc->getAnimManager(), _priority1, _priority2);
	} else {
		Debug::warning(Debug::Costumes, "Parent of %s was not a model", _name.c_str());
		_anim = nullptr;
	}
}

void KeyframeComponent::saveState(SaveGame *state) {
	_anim->saveState(state);
}

void KeyframeComponent::restoreState(SaveGame *state) {
	_anim->restoreState(state);
}

} // end of namespace Grim
