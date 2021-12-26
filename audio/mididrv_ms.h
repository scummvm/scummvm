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

#ifndef AUDIO_MIDIDRV_MS_H
#define AUDIO_MIDIDRV_MS_H

#include "common/mutex.h"

#include "audio/mididrv.h"

/**
 * Abstract base class for MIDI drivers supporting multiple simultaneous
 * sources of MIDI data.
 *
 * These drivers support the following features:
 * 
 * - Multiple MIDI sources
 *   If the game plays multiple streams of MIDI data at the same time, each
 *   stream can be marked with a source number. When a source has finished
 *   playing, it must be deinitialized to release any resources allocated to
 *   it. This is done automatically when an End Of Track MIDI meta event is
 *   received, or manually by calling deinitSource.
 *   Using source numbers enables the following features:
 *	 - Music/SFX volume
 *	   Using setSourceType a MIDI source can be designated as music or sound
 *	   effects. The driver will then apply the appropriate user volume setting
 *	   to the MIDI channel volume. This setting sticks after deinitializing a
 *	   source, so if you use the same source numbers for the same types of MIDI
 *	   data, you don't need to set the source type repeatedly. The default setup
 *	   is music for source 0 and SFX for sources 1 and higher.
 *	 - Source volume
 *	   If the game changes the volume of the MIDI playback, you can use
 *	   setSourceVolume to set the volume level for a source. The driver will
 *	   then adjust the current MIDI channel volume and any received MIDI volume
 *	   controller messages. Use setSourceNeutralVolume to set the neutral volume
 *	   for a source (MIDI volume is not changed when source volume is at this
 *	   level; if it is lower or higher, MIDI volume is reduced or increased).
 *	 - Volume fading
 *	   If the game needs to gradually change the volume of the MIDI playback
 *	   (typically for a fade-out), you can use the startFade function. You can
 *	   check the status of the fade using isFading, and abort a fade using
 *	   abortFade. An active fade is automatically aborted when the fading source
 *	   is deinitialized.
 *	   The fading functionality uses the source volume, so you should not set
 *	   this while a fade is active. After the fade the source volume will remain
 *	   at the target level, so if you perform f.e. a fade-out, the source volume
 *	   will remain at 0. If you want to start playback again using this source,
 *	   use setSourceVolume to set the correct playback volume.
 *	   Note that when you stop MIDI playback, notes will not be immediately
 *	   silent but will gradually die out ("release"). So if you fade out a
 *	   source, stop playback, and immediately reset the source volume, the
 *	   note release will be audible. It is recommended to wait about 0.5s
 *	   before resetting the source volume.
 * 
 * - User volume settings
 *   The driver can scale the MIDI channel volume using the user specified
 *   volume settings. Just call syncSoundSettings when the user has changed the
 *   volume settings. Set the USER_VOLUME_SCALING property to true to enable
 *   this functionality.
 *
 * A driver extending this class must implement the following functions:
 * - send(source, data): process a MIDI event for a specific source.
 * - stopAllNotes(source, channel): stop all active notes for a source and/or
 *   MIDI channel (called when a source is deinitialized).
 * - applySourceVolume(source): set the current source volume on active notes
 *   and/or MIDI channels.
 */
class MidiDriver_Multisource : public MidiDriver {
public:
	/**
	 * The maximum number of sources supported. This can be increased if
	 * necessary, but this will consume more memory and processing time.
	 */
	static const uint8 MAXIMUM_SOURCES = 10;
	/**
	 * The default neutral volume level for a source. If the source volume is
	 * set to this level, the volume levels in the MIDI data are used directly;
	 * if source volume is lower or higher, output volume is decreased or
	 * increased, respectively. Use @see setSourceNeutralVolume to change the
	 * default neutral volume.
	 */
	static const uint16 DEFAULT_SOURCE_NEUTRAL_VOLUME = 255;

protected:
	// Timeout between updates of the channel volume for fades (25ms)
	static const uint16 FADING_DELAY = 25 * 1000;

public:
	/**
	 * The type of audio produced by a MIDI source (music or sound effects).
	 */
	enum SourceType {
		/**
		 * Source type not specified (generally treated as music).
		 */
		SOURCE_TYPE_UNDEFINED,
		/**
		 * Source produces music.
		 */
		SOURCE_TYPE_MUSIC,
		/**
		 * Source produces sound effects.
		 */
		SOURCE_TYPE_SFX
	};

	/**
	 * Specifies what happens to the volume when a fade is aborted.
	 */
	enum FadeAbortType {
		/**
		 * The volume is set to the fade's end volume level.
		 */
		FADE_ABORT_TYPE_END_VOLUME,
		/**
		 * The volume remains at the current level.
		 */
		FADE_ABORT_TYPE_CURRENT_VOLUME,
		/**
		 * The volume is reset to the fade's start volume level.
		 */
		FADE_ABORT_TYPE_START_VOLUME
	};

	/**
	 * The controllers and parameters for which a default value can be set
	 * using setControllerDefault.
	 */
	enum ControllerDefaultType {
		CONTROLLER_DEFAULT_PROGRAM,
		CONTROLLER_DEFAULT_INSTRUMENT_BANK,
		CONTROLLER_DEFAULT_DRUMKIT,
		CONTROLLER_DEFAULT_CHANNEL_PRESSURE,
		CONTROLLER_DEFAULT_PITCH_BEND,
		CONTROLLER_DEFAULT_MODULATION,
		CONTROLLER_DEFAULT_VOLUME,
		CONTROLLER_DEFAULT_PANNING,
		CONTROLLER_DEFAULT_EXPRESSION,
		CONTROLLER_DEFAULT_SUSTAIN,
		CONTROLLER_DEFAULT_RPN,
		CONTROLLER_DEFAULT_PITCH_BEND_SENSITIVITY
	};

protected:
	// This stores data about a specific source of MIDI data.
	struct MidiSource {
		// Whether this source sends music or SFX MIDI data.
		SourceType type;
		// The source volume (relative volume for this source as defined by the
		// game). Default is the default neutral value (255).
		uint16 volume;
		// The source volume level at which no scaling is performed (volume as
		// defined in the MIDI data is used directly). Volume values below this
		// decrease volume, values above increase volume (up to the maximum MIDI
		// channel volume). Set this to match the volume values used by the game
		// engine to avoid having to convert them. Default value is 255; minimum
		// value is 1.
		uint16 neutralVolume;
		// The volume level at which the fade started.
		uint16 fadeStartVolume;
		// The target volume level for the fade.
		uint16 fadeEndVolume;
		// How much time (microseconds) has passed since the start of the fade.
		int32 fadePassedTime;
		// The total duration of the fade (microseconds).
		int32 fadeDuration;

		MidiSource();
	};

	// Stores the default values that should be set for each controller.
	// -1 means no explicit default should be set for that controller.
	struct ControllerDefaults {
		int8 program;
		int8 instrumentBank;
		int8 drumkit;

		int8 channelPressure;
		int16 pitchBend;

		int8 modulation;
		int8 volume;
		int8 panning;
		int8 expression;
		int8 sustain;
		int16 rpn;

		int8 pitchBendSensitivity;

		ControllerDefaults();
	};

public:
	MidiDriver_Multisource();

	// MidiDriver functions
	using MidiDriver_BASE::send;
	void send(uint32 b) override;
	void send(int8 source, uint32 b) override = 0;

	uint32 property(int prop, uint32 param) override;

	/**
	 * Deinitializes a source. This will abort active fades and stop any active
	 * notes.
	 *
	 * @param source The source to deinitialize.
	 */
	virtual void deinitSource(uint8 source);
	/**
	 * Sets the type for all sources (music or SFX).
	 *
	 * @param type The new type for all sources.
	 */
	void setSourceType(SourceType type);
	/**
	 * Sets the type for a specific sources (music or SFX).
	 *
	 * @param source The source for which the type should be set.
	 * @param type The new type for the specified source.
	 */
	void setSourceType(uint8 source, SourceType type);
	/**
	 * Sets the source volume for all sources.
	 *
	 * @param volume The new source volume for all sources.
	 */
	void setSourceVolume(uint16 volume);
	/**
	 * Sets the volume for this source. The volume values in the MIDI data sent
	 * by this source will be scaled by the source volume.
	 *
	 * @param source The source for which the source volume should be set.
	 * @param volume The new source volume for the specified source.
	 */
	void setSourceVolume(uint8 source, uint16 volume);
	/**
	 * Resets the source volume for all sources to each source's neutral volume.
	 */
	void resetSourceVolume();
	/**
	 * Resets the volume for this source to its neutral volume.
	 */
	void resetSourceVolume(uint8 source);
	/**
	 * Sets the neutral volume for all sources. See the source-specific
	 * setSourceNeutralVolume function for details.
	 * 
	 * @param volume The new neutral volume for all sources.
	 */
	void setSourceNeutralVolume(uint16 volume);
	/**
	 * Sets the neutral volume for this source. If the source volume is at this
	 * level, the volume values in the MIDI data sent by this source will not
	 * be changed. At source volumes below or above this value, the MIDI volume
	 * values will be decreased or increased accordingly.
	 *
	 * @param source The source for which the neutral volume should be set.
	 * @param volume The new neutral volume for the specified source.
	 */
	void setSourceNeutralVolume(uint8 source, uint16 volume);

	/**
	 * Starts a fade for all sources.
	 * See the source-specific startFade function for more information.
	 * 
	 * @param duration The fade duration in milliseconds
	 * @param targetVolume The volume at the end of the fade
	 */
	void startFade(uint16 duration, uint16 targetVolume);
	/**
	 * Starts a fade for a source. This will linearly increase or decrease the
	 * volume of the MIDI channels used by the source to the specified target
	 * value over the specified length of time.
	 *
	 * @param source The source to fade
	 * @param duration The fade duration in milliseconds
	 * @param targetVolume The volume at the end of the fade
	 */
	void startFade(uint8 source, uint16 duration, uint16 targetVolume);
	/**
	 * Aborts any active fades for all sources.
	 * See the source-specific abortFade function for more information.
	 * 
	 * @param abortType How to set the volume when aborting the fade (default:
	 * set to the target fade volume).
	 */
	void abortFade(FadeAbortType abortType = FADE_ABORT_TYPE_END_VOLUME);
	/**
	 * Aborts an active fade for a source. Depending on the abort type, the
	 * volume will remain at the current value or be set to the start or end
	 * volume. If there is no active fade for the specified source, this
	 * function does nothing.
	 *
	 * @param source The source that should have its fade aborted
	 * @param abortType How to set the volume when aborting the fade (default:
	 * set to the target fade volume).
	 */
	void abortFade(uint8 source, FadeAbortType abortType = FADE_ABORT_TYPE_END_VOLUME);
	/**
	 * Check if any source has an active fade.
	 *
	 * @return True if any source has an active fade.
	 */
	bool isFading();
	/**
	 * Check if the specified source has an active fade.
	 *
	 * @return True if the specified source has an active fade.
	 */
	bool isFading(uint8 source);

	/**
	 * Specify a controller which should be reset to its General MIDI default
	 * value when a new track is started. See the overload for more details.
	 * 
	 * @param type The controller which should be reset.
	 */
	void setControllerDefault(ControllerDefaultType type);
	/**
	 * Specify a default value for a controller which should be set when a new
	 * track is started. Use this if a game uses a MIDI controller, but does
	 * not consistently set it to a value at the start of every track, causing
	 * incorrect playback. Do not use this if a game depends on controller
	 * values carrying over to the next track for correct playback.
	 *
	 * This functionality will not work if the fallback MIDI source -1 is used.
	 * It is also necessary to call deinitSource whenever playback of a track
	 * is stopped, as this sets up the contoller reset.
	 *
	 * Use the setControllerDefault(ControllerDefaultType) overload if the
	 * General MIDI default value for the controller should be used.
	 * 
	 * @param type The controller which should be reset.
	 * @param value The default value which should be set.
	 */
	void setControllerDefault(ControllerDefaultType type, int16 value);
	/**
	 * Clears a previously set default value for the specified controller.
	 * 
	 * @param type The controller for which the default value should be cleared.
	 */
	void clearControllerDefault(ControllerDefaultType type);

	/**
	 * Applies the user volume settings to the MIDI driver. MIDI channel
	 * volumes will be scaled using the user volume.
	 * This function must be called by the engine when the user has changed the
	 * volume settings.
	 */
	void syncSoundSettings();

	using MidiDriver::stopAllNotes;
	/**
	 * Stops all active notes (including sustained notes) for the specified
	 * source and MIDI channel. For both source and channel the value 0xFF can
	 * be specified, in which case active notes will be stopped for all sources
	 * and/or MIDI channels.
	 * 
	 * @param source The source for which all notes should be stopped, or all
	 * sources if 0xFF is specified.
	 * @param channel The MIDI channel on which all notes should be stopped, or
	 * all channels if 0xFF is specified.
	 */
	virtual void stopAllNotes(uint8 source, uint8 channel) = 0;

	/**
	 * Sets a callback which will be called whenever the driver's timer
	 * callback is called by the underlying emulator or hardware driver. The
	 * callback will only be called when the driver is open. Use
	 * @see getBaseTempo to get the delay between each callback invocation.
	 * 
	 * @param timer_param A parameter that will be passed to the callback
	 * function. Optional.
	 * @param timer_proc The function that should be called.
	 */
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override {
		_timer_param = timer_param;
		_timer_proc = timer_proc;
	}

protected:
	/**
	 * Applies the current source volume to the active notes and/or MIDI
	 * channels of the specified source. 0xFF can be specified to apply the
	 * source volume for all sources.
	 *
	 * @param source The source for which the source volume should be applied,
	 * or all sources if 0xFF is specified.
	 */
	virtual void applySourceVolume(uint8 source) = 0;
	/**
	 * Processes active fades and sets new volume values if necessary.
	 */
	void updateFading();
	/**
	 * Runs the MIDI driver's timer related functionality. Will update volume
	 * fades and calls the timer callback if necessary.
	 */
	virtual void onTimer();

	// MIDI source data
	MidiSource _sources[MAXIMUM_SOURCES];

	// Default values for each controller
	ControllerDefaults _controllerDefaults;

	// True if the driver should scale MIDI channel volume to the user
	// specified volume settings.
	bool _userVolumeScaling;

	// User volume settings
	uint16 _userMusicVolume;
	uint16 _userSfxVolume;
	bool _userMute;

	Common::Mutex _fadingMutex; // For operations on fades

	// The number of microseconds to wait before the next fading step.
	uint16 _fadeDelay;

	// The number of microseconds between timer callback invocations.
	uint32 _timerRate;

	// External timer callback
	void *_timer_param;
	Common::TimerManager::TimerProc _timer_proc;
};

class MidiDriver_NULL_Multisource : public MidiDriver_Multisource {
public:
	~MidiDriver_NULL_Multisource();

	int open() override;
	bool isOpen() const override { return true; }
	void close() override { }
	uint32 getBaseTempo() override { return 10000; }
	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }

	using MidiDriver_Multisource::send;
	void send(int8 source, uint32 b) override { }
	using MidiDriver_Multisource::stopAllNotes;
	void stopAllNotes(uint8 source, uint8 channel) override { }

	static void timerCallback(void *data);

protected:
	void applySourceVolume(uint8 source) override { }
};

#endif
