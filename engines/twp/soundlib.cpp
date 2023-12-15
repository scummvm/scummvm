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

#include "twp/sqgame.h"
#include "twp/twp.h"
#include "twp/room.h"
#include "twp/object.h"
#include "twp/squtil.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

static SQInteger actorSound(HSQUIRRELVM v) {
	warning("TODO: actorSound not implemented");
	return 0;
}

static SQInteger defineSound(HSQUIRRELVM v) {
	warning("TODO: defineSound not implemented");
	return 0;
}

static SQInteger fadeOutSound(HSQUIRRELVM v) {
	warning("TODO: fadeOutSound not implemented");
	return 0;
}

static SQInteger isSoundPlaying(HSQUIRRELVM v) {
	warning("TODO: isSoundPlaying not implemented");
	return 0;
}

static SQInteger playObjectSound(HSQUIRRELVM v) {
	warning("TODO: playObjectSound not implemented");
	return 0;
}

static SQInteger playSound(HSQUIRRELVM v) {
	warning("TODO: playSound not implemented");
	return 0;
}

static SQInteger playSoundVolume(HSQUIRRELVM v) {
	warning("TODO: playSoundVolume not implemented");
	return 0;
}

static SQInteger loadSound(HSQUIRRELVM v) {
	warning("TODO: loadSound not implemented");
	return 0;
}

static SQInteger loopMusic(HSQUIRRELVM v) {
	warning("TODO: loopMusic not implemented");
	return 0;
}

static SQInteger loopObjectSound(HSQUIRRELVM v) {
	warning("TODO: loopObjectSound not implemented");
	return 0;
}

static SQInteger loopSound(HSQUIRRELVM v) {
	warning("TODO: loopSound not implemented");
	return 0;
}

static SQInteger masterSoundVolume(HSQUIRRELVM v) {
	warning("TODO: masterSoundVolume not implemented");
	return 0;
}

static SQInteger musicMixVolume(HSQUIRRELVM v) {
	warning("TODO: musicMixVolume not implemented");
	return 0;
}

static SQInteger playMusic(HSQUIRRELVM v) {
	warning("TODO: playMusic not implemented");
	return 0;
}

static SQInteger soundMixVolume(HSQUIRRELVM v) {
	warning("TODO: soundMixVolume not implemented");
	return 0;
}

static SQInteger soundVolume(HSQUIRRELVM v) {
	warning("TODO: soundVolume not implemented");
	return 0;
}

static SQInteger stopAllSounds(HSQUIRRELVM v) {
	warning("TODO: stopAllSounds not implemented");
	return 0;
}

static SQInteger stopMusic(HSQUIRRELVM v) {
	warning("TODO: stopMusic not implemented");
	return 0;
}

static SQInteger stopSound(HSQUIRRELVM v) {
	warning("TODO: stopSound not implemented");
	return 0;
}

static SQInteger talkieMixVolume(HSQUIRRELVM v) {
	warning("TODO: talkieMixVolume not implemented");
	return 0;
}

void sqgame_register_soundlib(HSQUIRRELVM v) {
  regFunc(v, actorSound, "actorSound");
  regFunc(v, defineSound, "defineSound");
  regFunc(v, fadeOutSound, "fadeOutSound");
  regFunc(v, isSoundPlaying, "isSoundPlaying");
  regFunc(v, loadSound, "loadSound");
  regFunc(v, loopMusic, "loopMusic");
  regFunc(v, loopObjectSound, "loopObjectSound");
  regFunc(v, loopSound, "loopSound");
  regFunc(v, masterSoundVolume, "masterSoundVolume");
  regFunc(v, musicMixVolume, "musicMixVolume");
  regFunc(v, playMusic, "playMusic");
  regFunc(v, playObjectSound, "playObjectSound");
  regFunc(v, playSound, "playSound");
  regFunc(v, playSoundVolume, "playSoundVolume");
  regFunc(v, soundMixVolume, "soundMixVolume");
  regFunc(v, soundVolume, "soundVolume");
  regFunc(v, stopAllSounds, "stopAllSounds");
  regFunc(v, stopMusic, "stopMusic");
  regFunc(v, stopSound, "stopSound");
  regFunc(v, talkieMixVolume, "talkieMixVolume");
}
} // namespace Twp
