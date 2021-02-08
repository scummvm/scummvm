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
	int _priority;
	int _sourceClipType;
	int _vol;
	int _volAsPercentage;
	int _volModifier;
	int _panning;
	int _panningAsPercentage;
	int _xSource, _ySource;		// Used for positioning sounds in game rooms
	int _maximumPossibleDistanceAway;
	int _directionalVolModifier;
	bool _repeat;
	void *_sourceClip;		// Pointer to source object that spawned the clip

	virtual void poll() = 0;
	virtual void destroy() = 0;
	// apply volume directly to playback; volume is given in units of 255
	// NOTE: this completely ignores volAsPercentage and muted property
	virtual void set_volume(int volume) = 0;
	virtual void seek(int offset) = 0;
	virtual int get_pos() = 0; // return 0 to indicate seek not supported
	virtual int get_pos_ms() = 0; // this must always return valid value if poss
	virtual int get_length_ms() = 0; // return total track length in ms (or 0)
	virtual int get_sound_type() const = 0;
	virtual int play() = 0;
	virtual int play_from(int position) = 0;

	virtual void set_panning(int newPanning) = 0;
	virtual void set_speed(int new_speed) = 0;

	virtual void pause() = 0;
	virtual void resume() = 0;

	virtual bool is_playing() const = 0; // true if playing or paused. false if never played or stopped.

	inline int get_speed() const {
		return _speed;
	}

	// Gets clip's volume property, as percentage (0 - 100);
	// note this may not be the real volume of playback (which could e.g. be muted)
	inline int get_volume() const {
		return _volAsPercentage;
	}

	inline bool is_muted() const {
		return _muted;
	}

	// Sets the current volume property, as percentage (0 - 100).
	inline void set_volume_percent(int volume) {
		_volAsPercentage = volume;
		if (!_muted)
			set_volume((volume * 255) / 100);
	}

	/**
	 * Explicitly defines both percentage and absolute volume value,
	 * without calculating it from given percentage.
	 * NOTE: this overrides the mute
	 */
	inline void set_volume_direct(int vol_percent, int vol_absolute) {
		_muted = false;
		_volAsPercentage = vol_percent;
		set_volume(vol_absolute);
	}

	// Mutes sound clip, while preserving current volume property
	// for the future reference; when unmuted, that property is
	// used to restart previous volume.
	inline void set_mute(bool enable) {
		_muted = enable;
		if (enable)
			set_volume(0);
		else
			set_volume((_volAsPercentage * 255) / 100);
	}

	// Apply arbitrary permanent volume modifier, in absolute units (0 - 255);
	// this is distinct value that is used in conjunction with current volume
	// (can be both positive and negative).
	inline void apply_volume_modifier(int mod) {
		_volModifier = mod;
		adjust_volume();
	}

	/**
	 * Apply permanent directional volume modifier, in absolute units (0 - 255)
	 * this is distinct value that is used in conjunction with current volume
	 * (can be both positive and negative).
	 */
	inline void apply_directional_modifier(int mod) {
		_directionalVolModifier = mod;
		adjust_volume();
	}

	virtual void adjust_volume() = 0;

	SOUNDCLIP();
	virtual ~SOUNDCLIP() {}

protected:
	// mute mode overrides the volume; if set, any volume assigned is stored
	// in properties, but not applied to playback itself
	bool _muted;

	// speed of playback, in clip ms per real second
	int _speed;

	// helper function for calculating volume with applied modifiers
	inline int get_final_volume() const {
		int final_vol = _vol + _volModifier + _directionalVolModifier;
		return final_vol >= 0 ? final_vol : 0;
	}
};

struct SoundClipWaveBase : public SOUNDCLIP {
	Audio::Mixer *_mixer;
	Audio::AudioStream *_stream;
	Audio::SoundHandle _soundHandle;
	SoundClipState _state;

	SoundClipWaveBase(Audio::AudioStream *stream, int volume, bool repeat = false);
	~SoundClipWaveBase() override { destroy(); }

	void destroy() override;
	void poll() override;
	int play() override;
	int play_from(int position) override;
	void pause() override;
	void resume() override;
	bool is_playing() const override;
	void seek(int offset) override;
	int get_pos() override;
	int get_pos_ms() override;
	int get_length_ms() override;
	void set_volume(int volume) override;
	void set_panning(int newPanning) override;
	void set_speed(int new_speed) override;
	void adjust_volume() override;
};

template<int SOUND_TYPE>
struct SoundClipWave : public SoundClipWaveBase {
	SoundClipWave(Audio::AudioStream *stream, int volume, bool repeat = false) :
		SoundClipWaveBase(stream, volume, repeat) {}
	int get_sound_type() const { return SOUND_TYPE; }
};

} // namespace AGS3

#endif
