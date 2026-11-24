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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dune/dune.h"
#include "common/system.h"

namespace Dune {

uint32 DuneEngine::animationClockMillis() const {
	return _animationClock.active() ? _animationClock.millis() : _system->getMillis();
}

uint32 DuneEngine::animationClockTicks() const {
	return animationClockMillis() / 5;
}

uint16 DuneEngine::animationElapsedTicks(uint32 previous, uint32 now) const {
	return _animationClock.active() ? _animationClock.elapsed() : (uint16)(now - previous);
}

void DuneEngine::updateRegisteredAnimations() {
	// ACE6 has already run. Rendering by an early callback must not add time
	// to a later one in this same D9D2, or alter its saved elapsed interval.
	const AnimationClock::Poll poll(_animationClock, _system->getMillis());
	updateBookCredits();
	updateMapAnimation();
	updateCabinMarker();
	updateCabinInstruction();
	updateGlobeAnimation();
	updateResultsGauges();
	updateBattleAnimation();
	updateDesertHarvester();
	updateMapSpeech();
	updateLocalSpeaker();
	updateEventMenu();
	updateSkyFade();
}

} // namespace Dune
