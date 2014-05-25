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

#include "bladerunner/script/script.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

void ScriptRC01::InitializeScene() {
	_vm->outtakePlay(28, true);
	_vm->outtakePlay(41, true);
	_vm->outtakePlay( 0, false);

	_vm->ISez("Blade Runner");
	_vm->ISez("");
	_vm->ISez("From the dark recesses of David Leary's imagination comes a game unlike any");
	_vm->ISez("other. Blade Runner immerses you in the underbelly of future Los Angeles. Right");
	_vm->ISez("from the start, the story pulls you in with graphic descriptions of a");
	_vm->ISez("grandmother doing the shimmy in her underwear, child molestation, brutal");
	_vm->ISez("cold-blooded slaying of innocent animals, vomiting on desks, staring at a");
	_vm->ISez("woman's ass, the list goes on. And when the game starts, the real fun begins -");
	_vm->ISez("shoot down-on-their-luck homeless people and toss them into a dumpster. Watch");
	_vm->ISez("with sadistic glee as a dog gets blown into chunky, bloody, bits by an");
	_vm->ISez("explosive, and even murder a shy little girl who loves you. If you think David");
	_vm->ISez("Leary is sick, and you like sick, this is THE game for you.");
	_vm->ISez("");
	_vm->ISez("JW: Don't forget the wasting of helpless mutated cripples in the underground.");
	_vm->ISez("It's such a beautiful thing!");
	_vm->ISez("");
	_vm->ISez("DL: Go ahead.  Just keep beating that snarling pit bull...ignore the foam");
	_vm->ISez("around his jaws. There's room on the top shelf of my fridge for at least one");
	_vm->ISez("more head... - Psychotic Dave");
	_vm->ISez("");
}

void ScriptRC01::SceneLoaded() {

}

void ScriptRC01::SceneFrameAdvanced(int frame) {

}

} // End of namespace BladeRunner
