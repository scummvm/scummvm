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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common/debug.h"

#include "prince/hero.h"
#include "prince/hero_set.h"
#include "prince/animation.h"
#include "prince/resource.h"


namespace Prince {

static const uint32 kMoveSetSize = 26;

Hero::Hero() : _number(0), _visible(false), _state(STAY), _middleX(0), _middleY(0)
    , _boreNum(0), _currHeight(0), _moveDelay(0), _shadMinus(1), _moveSetType(0), _frame(0) {
} 

bool Hero::loadAnimSet(uint32 animSetNr) {
	if (animSetNr > sizeof(heroSetTable)) {
		return false;
	}

	for (uint32 i = 0; i < _moveSet.size(); ++i) {
		delete _moveSet[i];
	}

	const HeroSetAnimNames &animSet = *heroSetTable[animSetNr];

	_moveSet.resize(kMoveSetSize);
	for (uint32 i = 0; i < kMoveSetSize; ++i) {
		debug("Anim set item %d %s", i, animSet[i]);
		Animation *anim = NULL;
		if (animSet[i] != NULL) {
			anim = new Animation();
			Resource::loadResource(anim, animSet[i]);
		}
		_moveSet[i] = anim;
	}


	return true;
}

const Graphics::Surface * Hero::getSurface() {
	if (_moveSet[_moveSetType]) {
		int16 phaseFrameIndex = _moveSet[_moveSetType]->getPhaseFrameIndex(_frame);
		return _moveSet[_moveSetType]->getFrame(phaseFrameIndex);
	}
	return NULL;
}

}

/* vim: set tabstop=4 noexpandtab: */
