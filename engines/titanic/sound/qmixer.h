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

#ifndef TITANIC_QMIXER_H
#define TITANIC_QMIXER_H

#include "audio/mixer.h"

namespace Titanic {

enum WaveMixOpenChannel {
	QMIX_OPENSINGLE		= 0,	// Open the single channel specified by iChannel
	QMIX_OPENALL		= 1,	// Opens all the channels, iChannel ignored
	QMIX_OPENCOUNT		= 2,	// Open iChannel Channels (eg. if iChannel = 4 will create channels 0-3)
	QMIX_OPENAVAILABLE	= 3		// Open the first unopened channel, and return channel number
};

/**
 * Mixer configuration structure for qsWaveMixInitEx
 */
struct QMIXCONFIG {
	uint32 dwSize;
	uint32 dwFlags;
	uint32 dwSamplingRate;   // Sampling rate in Hz
	void *lpIDirectSound;
	const void *lpGuid;
	int iChannels;          // Number of channels
	int iOutput;            // if 0, uses best output device
	int iLatency;           // (in ms) if 0, uses default for output device
	int iMath;              // style of math
	uint hwnd;

	QMIXCONFIG() : dwSize(40), dwFlags(0), dwSamplingRate(0), lpIDirectSound(nullptr),
		lpGuid(nullptr), iChannels(0), iOutput(0), iLatency(0), iMath(0), hwnd(0) {}
	QMIXCONFIG(uint32 rate, int channels, int latency) : dwSize(40),  dwFlags(0),
		dwSamplingRate(rate), iChannels(channels), iLatency(latency),
		lpIDirectSound(nullptr), lpGuid(nullptr), iOutput(0), iMath(0), hwnd(0) {}
};

/**
 * Vector positioning in metres
 */
struct QSVECTOR {
	double x;
	double y;
	double z;
};

/**
 * This class represents an interface to the QMixer library developed by
 * QSound Labs, Inc. Which itself is apparently based on Microsoft's
 * WaveMix API.
 *
 * It does not currently have any actual code from the library, 
 * and instead remaps calls to ScummVM's existing mixer where possible.
 * This means that advanced features of the QMixer library, like being
 * able to set up both the player and sounds at different positions are
 * currently ignored, and all sounds play at full volume.
 */
class QMixer {
private:
	Audio::Mixer *_mixer;
public:
	QMixer(Audio::Mixer *mixer);
	virtual ~QMixer() {}

	/**
	 * Initializes the mixer
	 */
	bool qsWaveMixInitEx(const QMIXCONFIG &config);

	/**
	 * Activates the mixer
	 */
	void qsWaveMixActivate(bool fActivate);

	/**
	 * Opens channels in the mixer for access
	 */
	int qsWaveMixOpenChannel(int iChannel, WaveMixOpenChannel mode);

	/**
	 * Closes down the mixer
	 */
	void qsWaveMixCloseSession();

	/**
	 * Stops a sound from playing
	 */
	void qsWaveMixFreeWave(Audio::SoundHandle &handle);
};

} // End of namespace Titanic

#endif /* TITANIC_QMIXER_H */
