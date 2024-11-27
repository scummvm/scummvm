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

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/vm.h"		// for Object
#include "sci/sound/audio.h"
#ifdef ENABLE_SCI32
#include "sci/sound/audio32.h"
#endif
#include "sci/sound/soundcmd.h"
#include "sci/sound/sync.h"

#include "audio/mixer.h"
#include "common/system.h"

namespace Sci {

/**
 * Used for synthesized music playback
 */
reg_t kDoSound(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, g_sci->_features->detectDoSoundType());
	error("not supposed to call this");
}

#define CREATE_DOSOUND_FORWARD(_name_) reg_t k##_name_(EngineState *s, int argc, reg_t *argv) { return g_sci->_soundCmd->k##_name_(s, argc, argv); }

CREATE_DOSOUND_FORWARD(DoSoundInit)
CREATE_DOSOUND_FORWARD(DoSoundPlay)
CREATE_DOSOUND_FORWARD(DoSoundDispose)
CREATE_DOSOUND_FORWARD(DoSoundMute)
CREATE_DOSOUND_FORWARD(DoSoundStop)
CREATE_DOSOUND_FORWARD(DoSoundStopAll)
CREATE_DOSOUND_FORWARD(DoSoundPause)
CREATE_DOSOUND_FORWARD(DoSoundResumeAfterRestore)
CREATE_DOSOUND_FORWARD(DoSoundMasterVolume)
CREATE_DOSOUND_FORWARD(DoSoundUpdate)
CREATE_DOSOUND_FORWARD(DoSoundFade)
CREATE_DOSOUND_FORWARD(DoSoundGetPolyphony)
CREATE_DOSOUND_FORWARD(DoSoundUpdateCues)
CREATE_DOSOUND_FORWARD(DoSoundSendMidi)
CREATE_DOSOUND_FORWARD(DoSoundGlobalReverb)
CREATE_DOSOUND_FORWARD(DoSoundSetHold)
CREATE_DOSOUND_FORWARD(DoSoundGetAudioCapability)
CREATE_DOSOUND_FORWARD(DoSoundSuspend)
CREATE_DOSOUND_FORWARD(DoSoundSetVolume)
CREATE_DOSOUND_FORWARD(DoSoundSetPriority)
CREATE_DOSOUND_FORWARD(DoSoundSetLoop)

#ifdef ENABLE_SCI32
reg_t kDoSoundMac32(EngineState *s, int argc, reg_t *argv) {
	// Several SCI 2.1 Middle Mac games, but not all, contain a modified kDoSound
	//  in which all but eleven subops were removed, changing their subop values to
	//  zero through ten. PQSWAT and HOYLE5 Solitaire restored all of the subops,
	//  but kept the new values that removing caused in the first place.
	switch (argv[0].toUint16()) {
	case 0:
		return g_sci->_soundCmd->kDoSoundMasterVolume(s, argc - 1, argv + 1);
	case 1:
		return g_sci->_soundCmd->kDoSoundGetAudioCapability(s, argc - 1, argv + 1);
	case 2:
		return g_sci->_soundCmd->kDoSoundInit(s, argc - 1, argv + 1);
	case 3:
		return g_sci->_soundCmd->kDoSoundDispose(s, argc - 1, argv + 1);
	case 4:
		return g_sci->_soundCmd->kDoSoundPlay(s, argc - 1, argv + 1);
	case 5:
		return g_sci->_soundCmd->kDoSoundStop(s, argc - 1, argv + 1);
	case 6:
		return g_sci->_soundCmd->kDoSoundPause(s, argc - 1, argv + 1);
	case 7:
		return g_sci->_soundCmd->kDoSoundFade(s, argc - 1, argv + 1);
	case 8:
		return g_sci->_soundCmd->kDoSoundSetVolume(s, argc - 1, argv + 1);
	case 9:
		return g_sci->_soundCmd->kDoSoundSetLoop(s, argc - 1, argv + 1);
	case 10:
		return g_sci->_soundCmd->kDoSoundUpdateCues(s, argc - 1, argv + 1);
	// PQSWAT, HOYLE5 solitaire
	case 12: // kDoSoundRestore
		return kEmpty(s, argc - 1, argv + 1);
	case 13:
		return g_sci->_soundCmd->kDoSoundGetPolyphony(s, argc - 1, argv + 1);
	case 14:
		return g_sci->_soundCmd->kDoSoundSuspend(s, argc - 1, argv + 1);
	case 15:
		return g_sci->_soundCmd->kDoSoundSetHold(s, argc - 1, argv + 1);
	case 17:
		return g_sci->_soundCmd->kDoSoundSetPriority(s, argc - 1, argv + 1);
	case 18:
		return g_sci->_soundCmd->kDoSoundSendMidi(s, argc - 1, argv + 1);
	default:
		break;
	}

	error("Unknown kDoSoundMac32 subop %d", argv[0].toUint16());
	return s->r_acc;
}
#endif

reg_t kDoCdAudio(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case kSciAudioPlay: {
		if (argc < 2)
			return NULL_REG;

		uint16 track = argv[1].toUint16();
		uint32 startFrame = (argc > 2) ? argv[2].toUint16() * 75 : 0;
		uint32 totalFrames = (argc > 3) ? argv[3].toUint16() * 75 : 0;

		return make_reg(0, g_sci->_audio->audioCdPlay(track, startFrame, totalFrames));
	}
	case kSciAudioStop:
		g_sci->_audio->audioCdStop();

		if (getSciVersion() == SCI_VERSION_1_1)
			return make_reg(0, 1);

		break;
	case kSciAudioPause:
		warning("Can't pause CD Audio");
		break;
	case kSciAudioResume:
		// This seems to be hacked up to update the CD instead of resuming
		// audio like kDoAudio does.
		g_sci->_audio->audioCdUpdate();
		break;
	case kSciAudioPosition:
		return make_reg(0, g_sci->_audio->audioCdPosition());
	case kSciAudioWPlay: // CD Audio can't be preloaded
	case kSciAudioRate: // No need to set the audio rate
	case kSciAudioVolume: // The speech setting isn't used by CD Audio
	case kSciAudioLanguage: // No need to set the language
		break;
	case kSciAudioCD:
		// Init
		return make_reg(0, 1);
	default:
		error("kCdDoAudio: Unhandled case %d", argv[0].toUint16());
	}

	return s->r_acc;
}

/**
 * Used for speech playback and digital soundtracks in CD games.
 * This is the SCI16 version; SCI32 is handled separately.
 */
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv) {
	// JonesCD and Mothergoose256 CD use different functions
	// based on the cdaudio.map file to use red book tracks.
	if (g_sci->_features->usesCdTrack()) {
		if (g_sci->getGameId() == GID_MOTHERGOOSE256) {
			// The CD audio version of Mothergoose256 CD is unique with a
			// custom interpreter. Instead of using AUDIO001 files for English,
			// the interpreter is hard-coded to use CD audio when the language
			// is set to English. Otherwise, it uses the normal kDoAudio code
			// to use AUDIO### files for the other four languages
			// This is transparent to the scripts; they are the same as in the
			// version that uses AUDIO001 for English and not CD Audio.
			int audioLanguage = g_sci->getResMan()->getAudioLanguage();
			bool english = (audioLanguage == K_LANG_NONE) || (audioLanguage == K_LANG_ENGLISH);
			if (english && argv[0].toUint16() != kSciAudioLanguage) {
				return kDoCdAudio(s, argc, argv);
			}
		} else {
			return kDoCdAudio(s, argc, argv);
		}
	}

	Audio::Mixer *mixer = g_system->getMixer();

	switch (argv[0].toUint16()) {
	case kSciAudioWPlay:
	case kSciAudioPlay: {
		uint16 module;
		uint32 number;

		g_sci->_audio->stopAudio();
		// In SSCI, kDoAudio handles all samples, even if started by kDoSound.
		// We manage samples started by kDoSound in SoundCommandParser.
		g_sci->_soundCmd->stopAllSamples();

		if (argc == 2) {
			module = 65535;
			number = argv[1].toUint16();
		} else if (argc == 6 || argc == 8) {
			module = argv[1].toUint16();
			number = ((argv[2].toUint16() & 0xff) << 24) |
			         ((argv[3].toUint16() & 0xff) << 16) |
			         ((argv[4].toUint16() & 0xff) <<  8) |
			          (argv[5].toUint16() & 0xff);
		} else {
			warning("kDoAudio: Play called with an unknown number of parameters (%d)", argc);
			return NULL_REG;
		}

		if (argv[0].toUint16() == kSciAudioPlay) {
			g_sci->_audio->incrementPlayCounter();
		}

		debugC(kDebugLevelSound, "kDoAudio: play sample %d, module %d", number, module);

		// return sample length in ticks
		if (argv[0].toUint16() == kSciAudioWPlay)
			return make_reg(0, g_sci->_audio->wPlayAudio(module, number));
		else
			return make_reg(0, g_sci->_audio->startAudio(module, number));
	}
	case kSciAudioStop:
		debugC(kDebugLevelSound, "kDoAudio: stop");
		g_sci->_audio->stopAudio();
		// In SSCI, kDoAudio handles all samples, even if started by kDoSound.
		// We manage samples started by kDoSound in SoundCommandParser.
		g_sci->_soundCmd->stopAllSamples();
		break;
	case kSciAudioPause:
		debugC(kDebugLevelSound, "kDoAudio: pause");
		g_sci->_audio->pauseAudio();
		break;
	case kSciAudioResume:
		debugC(kDebugLevelSound, "kDoAudio: resume");
		g_sci->_audio->resumeAudio();
		break;
	case kSciAudioPosition: {
		// SSCI queried the audio driver's AudioLoc function and returned the result.
		// The driver returned the location in ticks if audio was playing, otherwise -1.
		// The driver could only play one piece of audio at a time, and it could have
		// been playing either a digital sample from kDoSound or speech from kDoAudio.
		// We have two separate components for these interfaces, so we must check both.
		int audioPosition = g_sci->_audio->getAudioPosition();
		if (audioPosition == -1) {
			// kDoAudio is not playing speech, so now we check the kDoSound interface.
			// SoundCommandParser does not keep track of the audio position in ticks
			// for digital samples, so we can't return the real position. Scripts only
			// care about the exact tick value for speech, otherwise it only matters
			// if the result is -1 or not, so just return 1 if a sample is playing.
			if (g_sci->_soundCmd->isDigitalSamplePlaying()) {
				audioPosition = 1;
			}
		}
		return make_reg(0, audioPosition);
	}
	case kSciAudioRate:
		debugC(kDebugLevelSound, "kDoAudio: set audio rate to %d", argv[1].toUint16());
		g_sci->_audio->setAudioRate(argv[1].toUint16());
		break;
	case kSciAudioVolume: {
		int16 volume = argv[1].toUint16();
		volume = CLIP<int16>(volume, 0, AUDIO_VOLUME_MAX);
		debugC(kDebugLevelSound, "kDoAudio: set volume to %d", volume);
		mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume * 2);
		break;
	}
	case kSciAudioLanguage:
		if (getSciVersion() == SCI_VERSION_1_1) {
			// SCI1.1: tests for digital audio support
			debugC(kDebugLevelSound, "kDoAudio: audio capability test");
			return make_reg(0, 1);
		} else {
			// SCI1: sets audio language, queries current language
			int newLanguage = argv[1].toSint16();
			int previousLanguage = g_sci->getResMan()->getAudioLanguage();

			// Handle the CD audio version of Mothergoose256 CD.
			// See above; when the language is English, kDoCdAudio is used
			// for all subops except this one.
			if (g_sci->_features->usesCdTrack()) {
				// Unload language audio to indicate that the current language is
				// English, and return success. There are no English audio files.
				if (newLanguage == K_LANG_ENGLISH) {
					g_sci->getResMan()->unloadAudioLanguage();
					debugC(kDebugLevelSound, "kDoAudio: set language to %d", newLanguage);
					return make_reg(0, newLanguage);
				}
				if (previousLanguage == K_LANG_NONE) {
					previousLanguage = K_LANG_ENGLISH;
				}
			}

			if (newLanguage == -1) {
				if (previousLanguage == K_LANG_NONE) {
					// Initialize default language, fall back on English.
					// The original interpreter had a language global with a hard-coded initial value.
					// For example, KQ5 PC was set to English, FM-Towns was set Japanese.
					// We initialize from getSciLanguage() to use our own default, or the launcher language
					int initialLanguage = g_sci->getSciLanguage();
					if (!g_sci->getResMan()->setAudioLanguage(initialLanguage) && initialLanguage != K_LANG_ENGLISH) {
						initialLanguage = K_LANG_ENGLISH;
						g_sci->getResMan()->setAudioLanguage(initialLanguage);
					}
					debugC(kDebugLevelSound, "kDoAudio: initialized language to %d", initialLanguage);
					return make_reg(0, initialLanguage);
				} else {
					debugC(kDebugLevelSound, "kDoAudio: current language: %d", previousLanguage);
					return make_reg(0, previousLanguage);
				}
			}

			if (g_sci->getResMan()->setAudioLanguage(newLanguage)) {
				debugC(kDebugLevelSound, "kDoAudio: set language to %d", newLanguage);
				return make_reg(0, newLanguage);
			} else {
				g_sci->getResMan()->setAudioLanguage(previousLanguage);
				debugC(kDebugLevelSound, "kDoAudio: error setting language: %d, using: %d", newLanguage, previousLanguage);
				return make_reg(0, previousLanguage);
			}
		}
		break;
	case kSciAudioCD:
		debugC(kDebugLevelSound, "kDoAudio: CD audio subop");
		return kDoCdAudio(s, argc - 1, argv + 1);

	// 3 new subops in Pharkas CD (including CD demo). kDoAudio in Pharkas sits at seg026:038C
	case 11:
		// Not sure where this is used yet
		warning("kDoAudio: Unhandled case 11, %d extra arguments passed", argc - 1);
		break;
	case 12:
		// SSCI calls this function with no parameters from
		// the TalkRandCycle class and branches on the return
		// value like a boolean. The conjectured purpose of
		// this function is to ensure that the talker's mouth
		// does not move if there is read jitter (slow CD
		// drive, scratched CD). The old behavior here of not
		// doing anything caused a nonzero value to be left in
		// the accumulator by chance. This is equivalent, but
		// more explicit.

		return make_reg(0, 1);
	case 13:
		// SSCI returns a counter that increments each time a sample
		//  is played. This is used by the PointsSound and Narrator
		//  classes to detect if their sound was interrupted by
		//  another sample playing, since only one can play at a time.
		//  The counter is incremented on each kDoAudio(Play) and on
		//  each kDoSound(Play) when the sound is a sample, since SSCI
		//  just passes those on to kDoAudio.
		//
		// When awarding points, the icon bar is often disabled, and
		//  PointsSound:check polls its signal to detect when the
		//  sound is completed so that it can re-enable the icon bar.
		//  If the sound is interrupted by another sample then the icon
		//  bar could remain permanently disabled, so the play counter
		//  is stored before playing and PointsSound:check polls for
		//  a change. The Narrator class also does this to detect
		//  if speech was interrupted so that the game can continue.
		return make_reg(0, g_sci->_audio->getPlayCounter());
	default:
		warning("kDoAudio: Unhandled case %d, %d extra arguments passed", argv[0].toUint16(), argc - 1);
	}

	return s->r_acc;
}

reg_t kDoSync(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case kSciAudioSyncStart: {
		ResourceId id;

		g_sci->_sync->stop();

		if (argc == 3) {
			id = ResourceId(kResourceTypeSync, argv[2].toUint16());
		} else if (argc == 7) {
			id = ResourceId(kResourceTypeSync36, argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(),
							argv[5].toUint16(), argv[6].toUint16());
		} else {
			warning("kDoSync: Start called with an unknown number of parameters (%d)", argc);
			return s->r_acc;
		}

		g_sci->_sync->start(id, argv[1]);
		break;
	}
	case kSciAudioSyncNext:
		g_sci->_sync->next(argv[1]);
		break;
	case kSciAudioSyncStop:
		g_sci->_sync->stop();
		break;
	default:
		error("DoSync: Unhandled subfunction %d", argv[0].toUint16());
	}

	return s->r_acc;
}

#ifdef ENABLE_SCI32
reg_t kDoAudio32(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kDoAudioInit(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, 0);
}

reg_t kDoAudioWaitForPlay(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		if (g_sci->_features->hasSci3Audio()) {
			return make_reg(0, g_sci->_audio32->getNumUnlockedChannels());
		} else {
			return make_reg(0, g_sci->_audio32->getNumActiveChannels());
		}
	}

	return g_sci->_audio32->kernelPlay(false, s, argc, argv);
}

reg_t kDoAudioPlay(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0) {
		return make_reg(0, g_sci->_audio32->getNumActiveChannels());
	}

	return g_sci->_audio32->kernelPlay(true, s, argc, argv);
}

reg_t kDoAudioStop(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelStop(s, argc, argv);
}

reg_t kDoAudioPause(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelPause(s, argc, argv);
}

reg_t kDoAudioResume(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelResume(s, argc, argv);
}

reg_t kDoAudioPosition(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelPosition(s, argc, argv);
}

reg_t kDoAudioRate(EngineState *s, int argc, reg_t *argv) {
	if (argc > 0) {
		const uint16 sampleRate = argv[0].toUint16();
		if (sampleRate != 0) {
			g_sci->_audio32->setSampleRate(sampleRate);
		}
	}

	return make_reg(0, g_sci->_audio32->getSampleRate());
}

reg_t kDoAudioVolume(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelVolume(s, argc, argv);
}

reg_t kDoAudioGetCapability(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, 1);
}

reg_t kDoAudioBitDepth(EngineState *s, int argc, reg_t *argv) {
	if (argc > 0) {
		const uint16 bitDepth = argv[0].toUint16();
		if (bitDepth != 0) {
			g_sci->_audio32->setBitDepth(bitDepth);
		}
	}

	return make_reg(0, g_sci->_audio32->getBitDepth());
}

reg_t kDoAudioMixing(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelMixing(argc, argv);
}

reg_t kDoAudioChannels(EngineState *s, int argc, reg_t *argv) {
	if (argc > 0) {
		const int16 numChannels = argv[0].toSint16();
		if (numChannels != 0) {
			g_sci->_audio32->setNumOutputChannels(numChannels);
		}
	}

	return make_reg(0, g_sci->_audio32->getNumOutputChannels());
}

reg_t kDoAudioPreload(EngineState *s, int argc, reg_t *argv) {
	if (argc > 0) {
		g_sci->_audio32->setPreload(argv[0].toUint16());
	}

	return make_reg(0, g_sci->_audio32->getPreload());
}

reg_t kDoAudioFade(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_audio32->kernelFade(s, argc, argv);
}

reg_t kDoAudioHasSignal(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_audio32->hasSignal());
}

reg_t kDoAudioSetLoop(EngineState *s, int argc, reg_t *argv) {
	g_sci->_audio32->kernelLoop(s, argc, argv);
	return s->r_acc;
}

reg_t kDoAudioPan(EngineState *s, int argc, reg_t *argv) {
	g_sci->_audio32->kernelPan(s, argc, argv);
	return s->r_acc;
}

reg_t kDoAudioPanOff(EngineState *s, int argc, reg_t *argv) {
	g_sci->_audio32->kernelPanOff(s, argc, argv);
	return s->r_acc;
}

reg_t kSetLanguage(EngineState *s, int argc, reg_t *argv) {
	// Used by script 90 of MUMG Deluxe from the main menu to toggle between
	// English and Spanish in some versions and English and Spanish and
	// French and German in others.
	const Common::Path audioDirectory(s->_segMan->getString(argv[0]));
	if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
		g_sci->getResMan()->changeMacAudioDirectory(audioDirectory);
	} else {
		g_sci->getResMan()->changeAudioDirectory(audioDirectory);
	}
	return s->r_acc;
}

#endif

} // End of namespace Sci
