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

//=============================================================================
//
// ACSOUND - AGS sound system wrapper
//
//=============================================================================

#ifndef AGS_ENGINE_MEDIA_AUDIO_SOUNDCLIP_H
#define AGS_ENGINE_MEDIA_AUDIO_SOUNDCLIP_H

#include "ags/engine/util/mutex.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "common/stream.h"

namespace AGS3 {

// TODO: one of the biggest problems with sound clips currently is that it
// provides several methods of applying volume, which may ignore or override
// each other, and does not shape a consistent interface.
// Improving this situation is only possible with massive refactory of
// sound clip use, taking backwards-compatible audio system in account.

enum SoundClipState {
	SoundClipInitial, SoundClipPlaying, SoundClipPaused, SoundClipStopped
};

struct SOUNDCLIP {
	SOUNDCLIP();
	virtual ~SOUNDCLIP() {}

	// TODO: move these to private
	int _sourceClipID;
	int _sourceClipType;
	bool _repeat;
	int _priority;
	int _xSource, _ySource;     // Used for positioning sounds in game rooms
	int _maximumPossibleDistanceAway;

	int _vol255;
	int _vol100;
	int _volModifier;
	int _panning;
	int _panningAsPercentage;
	int _directionalVolModifier;

	virtual int play() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void seek(int offset) = 0;
	virtual int play_from(int position) = 0;
	virtual bool is_playing() = 0; // true if playing or paused. false if never played or stopped.
	virtual bool is_paused() = 0; // true if paused

	/**
	 * Get legacy sound format type
	 */
	virtual int get_sound_type() const = 0;

	/**
	 * Return current position in frames
	 */
	virtual int get_pos() = 0;

	/**
	 * Return the position in milliseconds
	 */
	virtual int get_pos_ms() = 0;

	/**
	 * Return total track length in ms (or 0)
	 */
	virtual int get_length_ms() = 0;

	virtual void set_panning(int newPanning) = 0;
	virtual void set_speed(int new_speed) = 0;
	virtual void poll() = 0;

	/**
	 * Gets clip's volume property, as percentage (0 - 100);
	 * note this may not be the real volume of playback (which could e.g. be muted)
	 */
	inline int get_volume100() const { return _vol100; }

	/**
	 * Gets clip's volume measured in 255 units
	 */
    inline int get_volume255() const { return _vol255; }

	/**
	 * Gets clip's panning (-100 - +100)
	 */
    inline int get_panning() const { return _panning; }

	/**
	 * Gets clip's playback speed in clip ms per real second
	 */
    inline int get_speed() const { return _speed; }

	/**
	 * Gets if clip is muted (without changing the volume setting)
	 */
    inline bool is_muted() const { return _muted; }

	/**
	 * Sets the current volume property, as percentage (0 - 100)
	 */
	void set_volume100(int volume);

	/**
	 * Sets the current volume property, as a level (0 - 255)
	 */
	void set_volume255(int volume);

	/**
	 * Explicitly defines both percentage and absolute volume value,
	 * without calculating it from given percentage.
	 * NOTE: this overrides the mute
	 */
	void set_volume_direct(int vol_percent, int vol_absolute);

	/**
	 * Mutes sound clip, while preserving current volume property
	 * for the future reference; when unmuted, that property is
	 * used to restart previous volume.
	 */
	void set_mute(bool mute);

	/**
	 * Apply arbitrary permanent volume modifier, in absolute units (0 - 255);
	 * this is distinct value that is used in conjunction with current volume
	 * (can be both positive and negative).
	 */
	void apply_volume_modifier(int mod);

	/**
	 * Apply permanent directional volume modifier, in absolute units (0 - 255)
	 * this is distinct value that is used in conjunction with current volume
	 * (can be both positive and negative).
	 */
	void apply_directional_modifier(int mod);

	inline bool is_ready() { return is_playing(); }

	/**
	 * Returns if the clip is still playing, otherwise it's finished
	 */
	bool update();

protected:
	virtual void adjust_volume() = 0;

	// mute mode overrides the volume; if set, any volume assigned is stored
	// in properties, but not applied to playback itself
	bool _muted = false;

	// speed of playback, in clip ms per real second
	int _speed = 0;

	bool _paramsChanged = false;

	// helper function for calculating volume with applied modifiers
	inline int get_final_volume() const {
		int final_vol = _vol255 + _volModifier + _directionalVolModifier;
		return final_vol >= 0 ? final_vol : 0;
	}
};

struct SoundClipWaveBase : public SOUNDCLIP {
private:
	Audio::Mixer::SoundType _soundType = Audio::Mixer::kPlainSoundType;
public:
	Audio::Mixer *_mixer;
	Audio::AudioStream *_stream;
	Audio::SoundHandle _soundHandle;
	SoundClipState _state;
	bool _waitingToPlay = false;

	SoundClipWaveBase(Audio::AudioStream *stream, bool repeat = false);
	~SoundClipWaveBase() override;

	void setType(Audio::Mixer::SoundType type);

	void poll() override;
	int play() override;
	int play_from(int position) override;
	void pause() override;
	void resume() override;
	bool is_playing() override;
	bool is_paused() override;
	void seek(int offset) override;
	int get_pos() override;
	int get_pos_ms() override;
	int get_length_ms() override;
	void set_panning(int newPanning) override;
	void set_speed(int new_speed) override;
	void adjust_volume() override;
};

template<int SOUND_TYPE>
struct SoundClipWave : public SoundClipWaveBase {
	SoundClipWave(Audio::AudioStream *stream, bool repeat = false) :
		SoundClipWaveBase(stream, repeat) {}
	int get_sound_type() const {
		return SOUND_TYPE;
	}
};

} // namespace AGS3

#endif
