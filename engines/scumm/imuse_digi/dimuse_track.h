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

#if !defined(SCUMM_IMUSE_DIGI_TRACK_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_TRACK_H

#include "common/scummsys.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Scumm {


// These flag bits correspond exactly to the sound mixer flags of March 2007.
// We don't want to use the mixer flags directly, because then our saved games
// will break in interesting ways if the mixer flags are ever assigned new
// values. Now they should keep working, as long as these flags don't change.

enum {
	kFlagUnsigned = 1 << 0,
	kFlag16Bits = 1 << 1,
	kFlagStereo = 1 << 3
};

struct Track {
	int trackId;		     // used to identify track by value (0-15)
						     
	int8 pan;			     // panning value of sound
	int32 vol;			     // volume level (values 0-127 * 1000)
	int32 volFadeDest;	     // volume level which fading target (values 0-127 * 1000)
	int32 volFadeStep;	     // delta of step while changing volume at each imuse callback
	int32 volFadeDelay;	     // time in ms how long fading volume must be
	bool volFadeUsed;	     // flag if fading is in progress
	int32 gainReduction;     // amount of volume to subtract
	int32 gainRedFadeDest;   // target of fade for gain reduction
	bool gainRedFadeUsed;    // flag if fading is in progress
	bool alreadyCrossfading; // used by COMI to check if this track is already running a crossfade
	int loopShiftType;       // currently used by COMI to check if the loop point for this track has to be shifted
	int32 soundId;		     // sound id used by scumm script
	char soundName[15];      // sound name but also filename of sound in bundle data
	bool used;			     // flag mean that track is used
	bool toBeRemoved;        // flag mean that track need to be free
	bool souStreamUsed;	     // flag mean that track use stream from sou file
	bool sndDataExtComp;     // flag mean that sound data is compressed by scummvm tools
	int32 soundPriority;     // priority level of played sound (0-127)
	int32 regionOffset;      // offset to sound data relative to begining of current region
	int32 dataOffset;	     // offset to sound data relative to begining of 'DATA' chunk
	int32 curRegion;	     // id of current used region
	int32 curHookId;	     // id of current used hook id
	int32 volGroupId;	     // id of volume group (IMUSE_VOLGRP_VOICE, IMUSE_VOLGRP_SFX, IMUSE_VOLGRP_MUSIC)
	int32 soundType;	     // type of sound data (IMUSE_BUNDLE, IMUSE_RESOURCE)
	int32 feedSize;		     // size of sound data needed to be filled at each callback iteration
	int32 dataMod12Bit;	     // value used between all callback to align 12 bit source of data
	int32 mixerFlags;	     // flags for sound mixer's channel (kFlagStereo, kFlag16Bits, kFlagUnsigned)
	bool littleEndian;       // Endianness: default is big for original files and native for recompressed ones
	ImuseDigiSndMgr::SoundDesc *soundDesc;	// sound handle used by iMuse sound manager
	Audio::SoundHandle mixChanHandle;		// sound mixer's channel handle
	Audio::QueuingAudioStream *stream;		// sound mixer's audio stream handle for *.la1 and *.bun
	Actor *speakingActor;					// actor reference for CMI speech

	Track() : soundId(-1), used(false), stream(nullptr) {
	}

	void reset() {
		trackId = 0;
		pan = 64;
		vol = 0;
		volFadeDest = 0;
		volFadeStep = 0;
		volFadeDelay = 0;
		volFadeUsed = false;
		gainReduction = 0;
		gainRedFadeDest = 127 * 290; // About 4 dB of gain reduction
		gainRedFadeUsed = false;
		alreadyCrossfading = false;
		loopShiftType = 0; 
		soundId = 0;
		memset(soundName, 0, sizeof(soundName));
		used = false;
		toBeRemoved = false;
		souStreamUsed = false;
		sndDataExtComp = false;
		soundPriority = 0;
		regionOffset = 0;
		dataOffset = 0;
		curRegion = 0;
		curHookId = 0;
		soundType = 0;
		feedSize = 0;
		dataMod12Bit = 0;
		mixerFlags = 0;
		soundDesc = nullptr;
		stream = nullptr;
		speakingActor = nullptr;
		littleEndian = false;
	}

	int getPan() const { return (pan != 64) ? 2 * pan - 127 : 0; }
	int getVol() const { return vol / 1000; }
	Audio::Mixer::SoundType getType() const {
		Audio::Mixer::SoundType type;
		if (volGroupId == 1)
			type = Audio::Mixer::kSpeechSoundType;
		else if (volGroupId == 2)
			type = Audio::Mixer::kSFXSoundType;
		else if (volGroupId == 3)
			type = Audio::Mixer::kMusicSoundType;
		else
			error("Track::getType(): invalid sound type");
		return type;
	}
};

} // End of namespace Scumm

#endif
