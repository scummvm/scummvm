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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/misc.h"
#include "sci/sfx/soundcmd.h"
#include "sci/engine/kernel.h"
#include "sci/engine/vm.h"		// for Object

#include "sound/mixer.h"

namespace Sci {

/**
 * Used for synthesized music playback
 */
reg_t kDoSound(EngineState *s, int argc, reg_t *argv) {
	return s->_soundCmd->parseCommand(argc, argv, s->r_acc);
}

reg_t kDoCdAudio(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case kSciAudioWPlay:
	case kSciAudioPlay: {
		if (argc < 2)
			return NULL_REG;

		uint16 track = argv[1].toUint16();
		uint32 startFrame = (argc > 2) ? argv[2].toUint16() * 75 : 0;
		uint32 totalFrames = (argc > 3) ? argv[3].toUint16() * 75 : 0;

		return make_reg(0, s->_audio->audioCdPlay(track, startFrame, totalFrames));
	}
	case kSciAudioStop:
		s->_audio->audioCdStop();
		
		if (getSciVersion() == SCI_VERSION_1_1)
			return make_reg(0, 1);
			
		break;
	case kSciAudioPause:
		warning("Can't pause CD Audio");
		break;
	case kSciAudioResume:
		// This seems to be hacked up to update the CD instead of resuming
		// audio like kDoAudio does.
		s->_audio->audioCdUpdate();
		break;
	case kSciAudioPosition:
		return make_reg(0, s->_audio->audioCdPosition());
	case kSciAudioRate: // No need to set the audio rate
	case kSciAudioVolume: // The speech setting isn't used by CD Audio
	case kSciAudioLanguage: // No need to set the language
		break;
	case kSciAudioCD:
		// Init
		return make_reg(0, 1);
	default:
		warning("kCdDoAudio: Unhandled case %d", argv[0].toUint16());
	}

	return s->r_acc;
}

/**
 * Used for speech playback and digital soundtracks in CD games
 */
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv) {
	// JonesCD uses different functions based on the cdaudio.map file
	// to use red book tracks.
	if (s->usesCdTrack())
		return kDoCdAudio(s, argc, argv);

	Audio::Mixer *mixer = g_system->getMixer();

	switch (argv[0].toUint16()) {
	case kSciAudioWPlay:
	case kSciAudioPlay: {
		uint16 module;
		uint32 number;

		s->_audio->stopAudio();

		if (argc == 2) {
			module = 65535;
			number = argv[1].toUint16();
		} else if (argc == 6) {
			module = argv[1].toUint16();
			number = ((argv[2].toUint16() & 0xff) << 24) | ((argv[3].toUint16() & 0xff) << 16) |
					 ((argv[4].toUint16() & 0xff) <<  8) | (argv[5].toUint16() & 0xff);
		} else {
			warning("kDoAudio: Play called with an unknown number of parameters (%d)", argc);
			return NULL_REG;
		}

		return make_reg(0, s->_audio->startAudio(module, number)); // return sample length in ticks
	}
	case kSciAudioStop:
		s->_audio->stopAudio();
		break;
	case kSciAudioPause:
		s->_audio->pauseAudio();
		break;
	case kSciAudioResume:
		s->_audio->resumeAudio();
		break;
	case kSciAudioPosition:
		return make_reg(0, s->_audio->getAudioPosition());
	case kSciAudioRate:
		s->_audio->setAudioRate(argv[1].toUint16());
		break;
	case kSciAudioVolume:
		mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, argv[1].toUint16());
		break;
	case kSciAudioLanguage:
		if (argc == 1) {
			// In SCI1.1: tests for digital audio support
			return make_reg(0, 1);
		} else {
			s->resMan->setAudioLanguage(argv[1].toSint16());
		}
		break;
	case kSciAudioCD:
		return kDoCdAudio(s, argc - 1, argv + 1);
	default:
		warning("kDoAudio: Unhandled case %d", argv[0].toUint16());
	}

	return s->r_acc;
}

reg_t kDoSync(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	switch (argv[0].toUint16()) {
	case kSciAudioSyncStart: {
		ResourceId id;

		s->_audio->stopSoundSync();

		// Load sound sync resource and lock it
		if (argc == 3) {
			id = ResourceId(kResourceTypeSync, argv[2].toUint16());
		} else if (argc == 7) {
			id = ResourceId(kResourceTypeSync36, argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(),
							argv[5].toUint16(), argv[6].toUint16());
		} else {
			warning("kDoSync: Start called with an unknown number of parameters (%d)", argc);
			return s->r_acc;
		}

		s->_audio->setSoundSync(id, argv[1], segMan);
		break;
	}
	case kSciAudioSyncNext: 
		s->_audio->doSoundSync(argv[1], segMan);
		break;
	case kSciAudioSyncStop:
		s->_audio->stopSoundSync();
		break;
	default:
		warning("DoSync: Unhandled subfunction %d", argv[0].toUint16());
	}

	return s->r_acc;
}

} // End of namespace Sci
