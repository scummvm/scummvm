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

#include "common/scummsys.h"

#include "zvision/scripting/effects/region_effect.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"

namespace ZVision {

RegionNode::RegionNode(ZVision *engine, uint32 key, GraphicsEffect *effect, uint32 delay)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_REGION) {
	_effect = effect;
	_delay = delay;
	_timeLeft = 0;
}

RegionNode::~RegionNode() {
	_engine->getRenderManager()->deleteEffect(_key);
}

bool RegionNode::process(uint32 deltaTimeInMillis) {
	_timeLeft -= deltaTimeInMillis;

	if (_timeLeft <= 0) {
		_timeLeft = _delay;
		if (_effect)
			_effect->update();
	}

	return false;
}

} // End of namespace ZVision
