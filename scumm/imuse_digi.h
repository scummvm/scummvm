/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef IMUSE_DIGI_H
#define IMUSE_DIGI_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

#define MAX_DIGITAL_CHANNELS 8
#define MAX_IMUSE_JUMPS 50
#define MAX_IMUSE_REGIONS 50
#define MAX_IMUSE_MARKERS 50

class ScummEngine;
class Bundle;

/**
 * iMuse Digital Implementation for SCUMM v7 and higher.
 */
class IMuseDigital : public MusicEngine {
private:

	struct _region {
		uint32 start;		// begin of region
		uint32 length;		// lenght of region
	};

	struct _jump {
		uint32 start;		// jump start position
		uint32 dest;		// jump to dest position
		uint32 hookId;		// id of hook
		uint32 fadeDelay;	// fade delay in ms
	};

	struct _marker {
		char name[256];		// name of marker
	};

	struct Channel {
		int8 pan;			// pan
		int32 vol;			// volume
		int32 volFadeDest;	//
		int32 volFadeStep;	//
		int32 volFadeDelay;	//
		bool volFadeUsed;	//

		_region region[MAX_IMUSE_REGIONS];
		_marker marker[MAX_IMUSE_MARKERS];
		_jump jump[MAX_IMUSE_JUMPS];
		int32 offsetStop;
		int32 numJumps;
		int32 numRegions;
		int32 numMarkers;

		int32 offset;
		byte *data;
		int32 size;
		int idSound;
		int32 mixerSize;
		bool used;
		bool toBeRemoved;
		PlayingSoundHandle handle;
		WrappedAudioInputStream *stream;
		
		Channel();
	};

	Channel _channel[MAX_DIGITAL_CHANNELS];

	ScummEngine *_scumm;
	bool _pause;

	static void timer_handler(void *refConf);
	void callback();

	//
	// Bundle music
	//
	const char *_nameBundleMusic;
	const char *_newNameBundleMusic;
	byte _musicDisk;
	byte _voiceDisk;
	int32 _currentSampleBundleMusic;
	int32 _numberSamplesBundleMusic;
	int32 _offsetSampleBundleMusic;
	int32 _offsetBufBundleMusic;
	byte *_musicBundleBufFinal;
	byte *_musicBundleBufOutput;
	bool _pauseBundleMusic;
	PlayingSoundHandle _bundleMusicTrack;
	bool _musicBundleToBeChanged;
	int32 _bundleMusicSampleBits;
	int32 _outputMixerSize;
	int32 _bundleSampleChannels;
	int32 _bundleMusicPosition;

	static void music_handler(void *refCon);
	void bundleMusicHandler();

	void playBundleMusic(const char *song);

public:
	int32 _bundleSongPosInMs;
	Bundle *_bundle;	// FIXME: should be protected but is used by ScummEngine::askForDisk

	void pauseBundleMusic(bool state);
	void stopBundleMusic();
	void playBundleSound(const char *sound);

	void startSound(int sound, byte *src, int size = 0, int rate = 0);

public:
	IMuseDigital(ScummEngine *scumm);
	~IMuseDigital();

	void setMasterVolume(int vol) {}

	void startSound(int sound) { startSound(sound, 0, 0, 0); }
	void stopSound(int sound);
	void stopAllSounds();
	void pause(bool pause);
	void parseScriptCmds(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound) const;
};

} // End of namespace Scumm

#endif
