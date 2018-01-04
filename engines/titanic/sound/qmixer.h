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
 * aint with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_QMIXER_H
#define TITANIC_QMIXER_H

#include "audio/mixer.h"
#include "titanic/sound/wave_file.h"

namespace Titanic {

enum QMixFlag {
	QMIX_OPENSINGLE		= 0,	// Open the single channel specified by iChannel
	QMIX_OPENALL		= 1,	// Opens all the channels, iChannel ignored
	QMIX_OPENCOUNT		= 2,	// Open iChannel Channels (eg. if iChannel = 4 will create channels 0-3)
	QMIX_OPENAVAILABLE	= 3,	// Open the first unopened channel, and return channel number

	// Channel function flags
	QMIX_ALL				= 0x01,	// apply to all channels
	QMIX_NOREMIX			= 0x02,	// don't remix
	QMIX_CONTROL_NOREMIX	= 0x04,	// don't remix
	QMIX_USEONCE			= 0x10	// settings are temporary
};

// qsWaveMixEnableChannel flags:  if mode==0, use conventional, high-performance
// stereo mixer.  Non-zero modes imply some form of additional processing.
enum QMixChannelFlag {
	QMIX_CHANNEL_STEREO			= 0x0000,	// Perform stereo mixing
	QMIX_CHANNEL_QSOUND			= 0x0001,	// Perform QSound localization (default)
	QMIX_CHANNEL_DOPPLER		= 0x0002,	// Calculate velocity using position updates
	QMIX_CHANNEL_RANGE			= 0x0004,	// Do range effects
	QMIX_CHANNEL_ELEVATION		= 0x0008,	// Do elevation effects
	QMIX_CHANNEL_NODOPPLERPITCH	= 0x0010,	// Disable Doppler pitch shift for this channel
	QMIX_CHANNEL_PITCH_COPY		= 0x0000,	// Pitch shifting using copying (fastest)
	QMIX_CHANNEL_PITCH_LINEAR	= 0x0100,	// Pitch shifting using linear interpolation (better but slower)
	QMIX_CHANNEL_PITCH_SPLINE	= 0x0200,	// Pitch shifting using spline interpolation (better yet, but much slower)
	QMIX_CHANNEL_PITCH_FILTER	= 0x0300,	// Pitch shifting using FIR filter (best, but slowest)
	QMIX_CHANNEL_PITCH_MASK		= 0x0700	// Bits reserved for pitch types
};

/**
 * Options for dwFlags parameter in QSWaveMixPlayEx.
 *
 * Notes: The QMIX_USELRUCHANNEL flag has two roles.  When QMIX_CLEARQUEUE is also set,
 * the channel that has been playing the longest (least-recently-used) is cleared and
 * the buffer played.  When QMIX_QUEUEWAVE is set, the channel that  will first finish
 * playing will be selected and the buffer queued to play. Of course, if an unused
 * channel is found, it will be selected instead.
 * If QMIX_WAIT hasn't been specified, then the channel number will be returned
 * in the iChannel field.
 */
enum QMixPlayFlag {
	QMIX_QUEUEWAVE			= 0x0000,	// Queue on channel
	QMIX_CLEARQUEUE			= 0x0001,	// Clear queue on channel
	QMIX_USELRUCHANNEL		= 0x0002,	// See notes above
	QMIX_HIGHPRIORITY		= 0x0004,
	QMIX_WAIT				= 0x0008,	// Queue to be played with other sounds
	QMIX_IMMEDIATE			= 0x0020,	// Apply volume/pan changes without interpolation

	QMIX_PLAY_SETEVENT		= 0x0100,	// Calls SetEvent in the original library when done
	QMIX_PLAY_PULSEEVENT	= 0x0200,	// Calls PulseEvent in the original library when done
	QMIX_PLAY_NOTIFYSTOP	= 0x0400	// Do callback even when stopping or flushing sound
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

	QSVECTOR() : x(0.0), y(0.0), z(0.0) {}
	QSVECTOR(double xp, double yp, double zp) : x(xp), y(yp), z(zp) {}
};

/**
 * Polar positioning
 */
struct QSPOLAR {
	double azimuth;		// degrees
	double range;		// meters
	double elevation;	// degrees

	QSPOLAR() : azimuth(0.0), range(0.0), elevation(0.0) {}
	QSPOLAR(double azimuth_, double range_, double elevation_) :
		azimuth(azimuth_), range(range_), elevation(elevation_) {}
};

struct QMIX_DISTANCES {
	int cbSize;			// Structure size
	double minDistance;	// sounds are at full volume if closer than this
	double maxDistance;	// sounds are muted if further away than this
	double scale;		// relative amount to adjust rolloff by

	QMIX_DISTANCES() : cbSize(16), minDistance(0.0), maxDistance(0.0), scale(0.0) {}
	QMIX_DISTANCES(double minDistance_, double maxDistance_, double scale_) :
		cbSize(16), minDistance(minDistance_), maxDistance(maxDistance_), scale(scale_) {}
};

typedef void (*LPQMIXDONECALLBACK)(int iChannel, CWaveFile *lpWave, void *dwUser);

struct QMIXPLAYPARAMS {
	uint dwSize;		// Size of the play structure
	void *lpImage;		// Additional preprocessed audio for high performance
	uint hwndNotify;	// if set, WOM_OPEN and WOM_DONE messages sent to that window
	LPQMIXDONECALLBACK callback;	// Callback function
	void *dwUser;					// User data accompanying callback
	int lStart;
	int lStartLoop;
	int lEndLoop;
	int lEnd;
	const void *lpChannelParams;	// initialize with these parameters
	// Properties introduced by ScummVM
	Audio::Mixer::SoundType _soundType;

	QMIXPLAYPARAMS() : dwSize(36), lpImage(nullptr), hwndNotify(0), callback(nullptr),
		dwUser(nullptr), lStart(0), lStartLoop(0), lEndLoop(0), lEnd(0),
		lpChannelParams(nullptr), _soundType(Audio::Mixer::kPlainSoundType)  {}
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
	struct SoundEntry {
		bool _started;
		CWaveFile *_waveFile;
		Audio::SoundHandle _soundHandle;
		LPQMIXDONECALLBACK _callback;
		int _loops;
		void *_userData;
		SoundEntry() : _started(false), _waveFile(nullptr), _callback(nullptr),
			_loops(0), _userData(nullptr) {}

		SoundEntry(CWaveFile *waveFile, LPQMIXDONECALLBACK callback, int loops, void *userData) :
			_started(false), _waveFile(waveFile), _callback(callback), _loops(loops), _userData(userData) {}
	};
	struct ChannelEntry {
		// Currently playing and any following queued sounds for the channel
		Common::List<SoundEntry> _sounds;
		// Current channel volume
		byte _volume;
		// Current time in milliseconds for paning (volume) changes
		uint _panRate;
		// Fields used to transition between volume levels
		uint _volumeChangeStart;
		uint _volumeChangeEnd;
		byte _volumeStart;
		byte _volumeEnd;
		// Distance of source
		double _distance;
		bool _resetDistance;

		ChannelEntry() : _volume(0), _panRate(0), _volumeChangeStart(0),
			_volumeChangeEnd(0), _volumeStart(0), _volumeEnd(0),
			_distance(0.0), _resetDistance(true) {}

		/**
		 * Calculates the raw volume level to pass to ScummVM playStream, taking
		 * into the sound's volume level and distance from origin
		 */
		byte getRawVolume() const;
	};
private:
	Common::Array<ChannelEntry> _channels;
protected:
	Audio::Mixer *_mixer;
public:
	QMixer(Audio::Mixer *mixer);
	virtual ~QMixer();

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
	int qsWaveMixOpenChannel(int iChannel, QMixFlag mode);

	/**
	 * Enables a given channel
	 */
	int qsWaveMixEnableChannel(int iChannel, uint flags, bool enabled);

	/**
	 * Closes down the mixer
	 */
	void qsWaveMixCloseSession();

	/**
	 * Stops a sound from playing
	 */
	void qsWaveMixFreeWave(Audio::SoundHandle &handle);

	/**
	 * Flushes a channel
	 */
	void qsWaveMixFlushChannel(int iChannel, uint flags = 0);

	/**
	 * Sets the amount of time, in milliseconds, to effect a change in
	 * a channel property (e.g. volume, position).  Non-zero values
	 * smooth out changes
	 * @param iChannel		Channel to change
	 * @param flags			Flags
	 * @param rate			Pan rate in milliseconds
	*/
	void qsWaveMixSetPanRate(int iChannel, uint flags, uint rate);

	/**
	* Sets the volume for a channel
	*/
	void qsWaveMixSetVolume(int iChannel, uint flags, uint volume);

	/**
	 * Sets the relative position of a channel
	 * @param iChannel		Channel number
	 * @param Flags			Flags
	 * @param position		Vector position for channel
	 */
	void qsWaveMixSetSourcePosition(int iChannel, uint flags, const QSVECTOR &position);

	/**
	 * Sets the relative position of a channel using polar co-ordinates
	 * @param iChannel		Channel number
	 * @param Flags			Flags
	 * @param position		Polar position for channel
	 */
	void qsWaveMixSetPolarPosition(int iChannel, uint flags, const QSPOLAR &position);

	/**
	 * Sets the listener position
	 */
	void qsWaveMixSetListenerPosition(const QSVECTOR &position, uint flags = 0);

	/**
	 * Sets the listener orientation
	 */
	void qsWaveMixSetListenerOrientation(const QSVECTOR &direction, const QSVECTOR &up, uint flags = 0);

	/**
	 * Sets the mapping ditance range
	 */
	void qsWaveMixSetDistanceMapping(int iChannel, uint flags, const QMIX_DISTANCES &distances);

	/**
	 * Sets the frequency/rate of sound playback
	 */
	void qsWaveMixSetFrequency(int iChannel, uint flags, uint frequency);

	/**
	 * Sets the velocity of the source (listener)
	 */
	void qsWaveMixSetSourceVelocity(int iChannel, uint flags, const QSVECTOR &velocity);

	/**
	 * Plays sound
	 * @param iChannel		The channel number to be played on
	 * @param flags			Play flags
	 * @param mixWave		Data for the sound to be played
	 * @param loops			Number of loops to play (-1 for forever)
	 * @param params		Playback parameter data
	 */
	int qsWaveMixPlayEx(int iChannel, uint flags, CWaveFile *waveFile, int loops, const QMIXPLAYPARAMS &params);

	/**
	 * Returns true if there are no more buffers playing or queued on the channel
	 */
	bool qsWaveMixIsChannelDone(int iChannel) const;

	/**
	 * Handles regularly updating the mixer
	 */
	void qsWaveMixPump();
};

} // End of namespace Titanic

#endif /* TITANIC_QMIXER_H */
