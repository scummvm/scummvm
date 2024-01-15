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
#include "twp/audio.h"
#include "twp/squirrel/squirrel.h"
#include "audio/mixer.h"

namespace Twp {

class SoundTrigger : public Trigger {
public:
	SoundTrigger(const Common::Array<SoundDefinition *> sounds, int objId) : _sounds(sounds), _objId(objId) {}
	virtual ~SoundTrigger() {}

	virtual void trig() override {
		int i = g_engine->getRandomSource().getRandomNumber(_sounds.size() - 1);
		g_engine->_audio.play(_sounds[i], Audio::Mixer::SoundType::kPlainSoundType, 0, 0.f, 0.f, _objId);
	}

private:
	const Common::Array<SoundDefinition *> _sounds;
	int _objId;
};

// Plays a sound at the specified actor's location.
// If no sound is given, then it will turn off the trigger.
// If a list of multiple sounds or an array are given, will randomly choose between the sound files.
// The triggerNumber says which trigger in the animation JSON file should be used as a trigger to play the sound.
static SQInteger actorSound(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get actor or object");
	int trigNum = 0;
	if (SQ_FAILED(sqget(v, 3, trigNum)))
		return sq_throwerror(v, "failed to get trigger number");
	SQInteger numSounds = sq_gettop(v) - 3;
	if (numSounds != 0) {
		int tmp = 0;
		if ((numSounds == 1) && (SQ_SUCCEEDED(sqget(v, 4, tmp))) && (tmp == 0)) {
			obj->_triggers.erase(trigNum);
		} else {
			Common::Array<SoundDefinition *> sounds;
			if (sq_gettype(v, 4) == OT_ARRAY) {
				if (SQ_FAILED(sqgetarray(v, 4, sounds)))
					return sq_throwerror(v, "failed to get sounds");
			} else {
				sounds.resize(numSounds);
				for (int i = 0; i < numSounds; i++) {
					sounds[i] = sqsounddef(v, 4 + i);
				}
			}

			Trigger *trigger = new SoundTrigger(sounds, obj->getId());
			obj->_triggers[trigNum] = trigger;
		}
	}
	return 0;
}

// Defines a sound and binds it to an id.
// The defineSound(file) calls should be done at boot and do not load the file.
// Its main use is to keep strings from being created and referenced during game play and providing a way to globally change a sound.
// .. code-block:: Squirrel
// clock_tick <- defineSound("clockTick.wav")
static SQInteger defineSound(HSQUIRRELVM v) {
	Common::String filename;
	if (SQ_FAILED(sqget(v, 2, filename)))
		return sq_throwerror(v, "failed to get filename");
	SoundDefinition *sound = new SoundDefinition(filename);
	g_engine->_audio._soundDefs.push_back(sound);
	debug("defineSound(%s)-> %d", filename.c_str(), sound->getId());
	sqpush(v, sound->getId());
	return 1;
}

// Fades a sound out over a specified fade out duration (in seconds).
// .. code-block:: Squirrel
// fadeOutSound(soundElevatorMusic, 0.5)
static SQInteger fadeOutSound(HSQUIRRELVM v) {
	int sound = 0;
	if (SQ_FAILED(sqget(v, 2, sound)))
		return sq_throwerror(v, "failed to get sound");
	float t;
	if (SQ_FAILED(sqget(v, 3, t)))
		return sq_throwerror(v, "failed to get fadeOut time");
	g_engine->_audio.fadeOut(sound, t);
	return 0;
}

// Returns `TRUE` if sound is currently playing.
// Where sound can be a channel (an integer from 1-32), a sound id (as obtained when sound was created with playSound), an actual sound (ie one that has been defined using defineSound).
// .. code-block:: Squirrel
// if (isSoundPlaying(soundElevatorMusic)) { ...}
static SQInteger isSoundPlaying(HSQUIRRELVM v) {
	int soundId;
	if (SQ_FAILED(sqget(v, 2, soundId)))
		return sq_throwerror(v, "failed to get sound");
	sqpush(v, g_engine->_audio.playing(soundId));
	return 1;
}

static SQInteger playObjectSound(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	SoundDefinition *soundDef = sqsounddef(v, 2);
	if (!soundDef)
		return sq_throwerror(v, "failed to get sound");

	Object *obj = sqobj(v, 3);
	if (!obj)
		return sq_throwerror(v, "failed to get actor or object");
	int loopTimes = 1;
	float fadeInTime = 0.0f;
	if (nArgs >= 4) {
		sqget(v, 4, loopTimes);
		sqget(v, 5, fadeInTime);
	}

	if (obj->_sound) {
		g_engine->_audio.stop(obj->_sound);
	}

	int soundId = g_engine->_audio.play(soundDef, Audio::Mixer::SoundType::kPlainSoundType, loopTimes, fadeInTime, 1.f, obj->getId());
	obj->_sound = soundId;
	sqpush(v, soundId);
	return 1;
}

// Plays a sound that has been loaded with defineSound(file).
// Classifies the audio as "sound" (not "music").
// Returns a sound ID that can be used to reference the sound later on.
// .. code-block:: Squirrel
// playSound(clock_tick)
// objectState(quickiePalFlickerLight, ON)
// _flourescentSoundID = playSound(soundFlourescentOn)
static SQInteger playSound(HSQUIRRELVM v) {
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound) {
		int soundId = 0;
		sqget(v, 2, soundId);
		return sq_throwerror(v, Common::String::format("failed to get sound: %d", soundId).c_str());
	}
	int soundId = g_engine->_audio.play(sound, Audio::Mixer::SoundType::kPlainSoundType);
	sqpush(v, soundId);
	return 1;
}

// Starts playing sound at the specified volume, where volume is a float between 0 and 1.
// Not for use in adjusting the volume of a sound that is already playing.
// Returns a sound ID which can be used when turning off the sound or otherwise manipulating it.
// .. code-block:: Squirrel
// script runAway(bunnyActor) {
//     local soundVolume = 1.0
//     for (local soundVolume = 1.0; x > 0; x -= 0.25) {
//         playSoundVolume(soundHop, soundVolume)
//         objectOffsetTo(bunnyActor, -10, 0, 0.5)
//         breaktime(1.0)
//     }
// }
static SQInteger playSoundVolume(HSQUIRRELVM v) {
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound)
		return sq_throwerror(v, "failed to get sound");
	int soundId = g_engine->_audio.play(sound, Audio::Mixer::SoundType::kPlainSoundType);
	sqpush(v, soundId);
	return 1;
}

static SQInteger loadSound(HSQUIRRELVM v) {
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound)
		return sq_throwerror(v, "failed to get sound");
	sound->load();
	return 0;
}

// Loops music.
// If loopTimes is not defined or is -1, will loop infinitely.
// For the first loop, it will fade the sound in for fadeInTime seconds, if specified.
// See also loopSound, which classifies the audio as being "sound" not "music".
// This is important if we allow separate volume control adjustment.
// .. code-block:: Squirrel
// enter = function()
// {
//     print("Enter StartScreen")
//     exCommand(EX_BUTTON_HOVER_SOUND, soundClockTick)
//     _music = loopMusic(musicTempA)
// }
static SQInteger loopMusic(HSQUIRRELVM v) {
	int loopTimes = -1;
	float fadeInTime = 0.f;
	SQInteger numArgs = sq_gettop(v);
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound)
		return sq_throwerror(v, "failed to get music");
	if (numArgs == 3) {
		sqget(v, 3, loopTimes);
	}
	if (numArgs == 4) {
		sqget(v, 4, fadeInTime);
	}
	int soundId = g_engine->_audio.play(sound, Audio::Mixer::kMusicSoundType, loopTimes, fadeInTime);
	sqpush(v, soundId);
	return 1;
}

static SQInteger loopObjectSound(HSQUIRRELVM v) {
	int loopTimes = -1;
	float fadeInTime = 0.f;
	SQInteger numArgs = sq_gettop(v);
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound)
		return sq_throwerror(v, "failed to get music");
	Object *obj = sqobj(v, 3);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (numArgs == 4) {
		if (SQ_FAILED(sqget(v, 4, loopTimes))) {
			return sq_throwerror(v, "failed to get loopTimes");
		}
	}
	if (numArgs == 5) {
		if (SQ_FAILED(sqget(v, 5, fadeInTime))) {
			return sq_throwerror(v, "failed to get fadeInTime");
		}
	}
	int soundId = g_engine->_audio.play(sound, Audio::Mixer::kPlainSoundType, loopTimes, fadeInTime, 1.f, obj->getId());
	sqpush(v, soundId);
	return 1;
}

// Loops a sound a specified number of times (loopTimes).
// If loopTimes = -1 or not set, then it loops the sound forever.
// You can fade in the sound for the first loop by setting the fadeInTime duration (in seconds).
// If fadeInTime is 0 or not set, it will immediately be at full volume.
// Returns a sound ID which can be used when turning off the sound or otherwise manipulating it.
// See also loopMusic.
// .. code-block:: Squirrel
// local _muzac = loopSound(soundElevatorMusic, -1, 1.0)
//
// script daveCooking() {
//     loopSound(soundSizzleLoop)
//     ...
// }
//
// if (Bank.bankTelephone.inUse) {
//     breaktime(0.5)
//     loopSound(soundPhoneBusy, 3)
// }
static SQInteger loopSound(HSQUIRRELVM v) {
	int loopTimes = -1;
	float fadeInTime = 0.f;
	SQInteger numArgs = sq_gettop(v);
	SoundDefinition *sound = sqsounddef(v, 2);
	if (!sound)
		return sq_throwerror(v, "failed to get music");
	if (numArgs == 3) {
		if (SQ_FAILED(sqget(v, 3, loopTimes))) {
			return sq_throwerror(v, "failed to get loopTimes");
		}
	}
	if (numArgs == 4) {
		if (SQ_FAILED(sqget(v, 4, fadeInTime))) {
			return sq_throwerror(v, "failed to get fadeInTime");
		}
	}
	int soundId = g_engine->_audio.play(sound, Audio::Mixer::kPlainSoundType, loopTimes, fadeInTime);
	sqpush(v, soundId);
	return 1;
}

static SQInteger soundVolume(HSQUIRRELVM v, Audio::Mixer::SoundType soundType) {
	float volume = 0.f;
	if (sq_gettop(v) == 2) {
		if (SQ_FAILED(sqget(v, 2, volume))) {
			return sq_throwerror(v, "failed to get volume");
		}
		int vol = volume * Audio::Mixer::kMaxMixerVolume;
		g_engine->_mixer->setVolumeForSoundType(soundType, vol);
		return 0;
	}
	volume = (float)g_engine->_mixer->getVolumeForSoundType(soundType) / Audio::Mixer::kMaxMixerVolume;
	sqpush(v, volume);
	return 1;
}

static SQInteger masterSoundVolume(HSQUIRRELVM v) {
	float volume = 0.f;
	if (sq_gettop(v) == 2) {
		if (SQ_FAILED(sqget(v, 2, volume))) {
			return sq_throwerror(v, "failed to get volume");
		}
		g_engine->_audio.setMasterVolume(volume);
		return 0;
	}
	volume = g_engine->_audio.getMasterVolume();
	sqpush(v, volume);
	return 1;
}

static SQInteger musicMixVolume(HSQUIRRELVM v) {
	return soundVolume(v, Audio::Mixer::SoundType::kMusicSoundType);
}

static SQInteger playMusic(HSQUIRRELVM v) {
	SoundDefinition *soundDef = sqsounddef(v, 2);
	if (!soundDef)
		return sq_throwerror(v, "failed to get music");
	int soundId = g_engine->_audio.play(soundDef, Audio::Mixer::SoundType::kMusicSoundType);
	sqpush(v, soundId);
	return 1;
}

static SQInteger soundMixVolume(HSQUIRRELVM v) {
	return soundVolume(v, Audio::Mixer::SoundType::kPlainSoundType);
}

// Sets the volume (float from 0 to 1) of an already playing sound.
// Can be used for a channel (integer 1-32), soundId (as obtained when starting the sound playing) or an actual sound (defined by defineSound).
// If _sound is not yet playing, then nothing will happen (if sound is subsequently set to play it will be at full volume).
// .. code-block:: Squirrel
// local _tronSoundTID = loopObjectSound(soundTronRattle_Loop, quickieToilet, -1, 0.25)
// soundVolume(_tronSoundTID, 0.2)
// shakeObject(quickieToilet, 0.25)
// jiggleObject(quickieToilet, 0.25)
// breaktime(0.2)
static SQInteger soundVolume(HSQUIRRELVM v) {
	int soundId;
	if (SQ_FAILED(sqget(v, 2, soundId)))
		return sq_throwerror(v, "failed to get sound");
	float volume = 1.0f;
	if (SQ_FAILED(sqget(v, 3, volume)))
		return sq_throwerror(v, "failed to get volume");
	g_engine->_audio.setVolume(soundId, volume);
	return 0;
}

static SQInteger stopAllSounds(HSQUIRRELVM v) {
	g_engine->_mixer->stopAll();
	return 0;
}

// Immediately stops the indicated sound.
// Abruptly. Silently. No fades. It's dead.
// Can be used for a channel (integer 1-32), _soundId (as obtained when starting the sound playing) or an actual sound (defined by defineSound).
// If using a defined sound, will stop any sound that is named that, eg all cricket sounds (soundCrickets, soundCrickets).
// .. code-block:: Squirrel
// stopSound(soundElevatorMusic)
static SQInteger stopSound(HSQUIRRELVM v) {
	int soundId;
	if (SQ_FAILED(sqget(v, 2, soundId)))
		return sq_throwerror(v, "failed to get sound");
	g_engine->_audio.stop(soundId);
	return 0;
}

static SQInteger talkieMixVolume(HSQUIRRELVM v) {
	return soundVolume(v, Audio::Mixer::SoundType::kSpeechSoundType);
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
	regFunc(v, stopSound, "stopSound");
	regFunc(v, talkieMixVolume, "talkieMixVolume");
}
} // namespace Twp
