//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// ACSOUND - AGS sound system wrapper
//
//=============================================================================

#ifndef __AC_SOUNDCLIP_H
#define __AC_SOUNDCLIP_H

#include "util/mutex.h"

// JJS: This is needed for the derieved classes
extern volatile int psp_audio_multithreaded;

// TODO: one of the biggest problems with sound clips currently is that it
// provides several methods of applying volume, which may ignore or override
// each other, and does not shape a consistent interface.
// Improving this situation is only possible with massive refactory of
// sound clip use, taking backwards-compatible audio system in account.

enum SoundClipState { SoundClipInitial, SoundClipPlaying, SoundClipPaused, SoundClipStopped };

struct SOUNDCLIP
{
    int priority;
    int sourceClipType;
    // absolute volume, set by implementations only!
    int vol;
    // current relative volume, in percents
    int volAsPercentage;
    // volModifier is used when there's a need to temporarily change and
    // the restore the clip's absolute volume (vol)
    int volModifier;
    int panning;
    int panningAsPercentage;
    int xSource, ySource;
    int maximumPossibleDistanceAway;
    int directionalVolModifier;
    bool repeat;
    void *sourceClip;

    virtual void poll() = 0;
    virtual void destroy() = 0;
    // apply volume directly to playback; volume is given in units of 255
    // NOTE: this completely ignores volAsPercentage and muted property
    virtual void set_volume(int) = 0;
    virtual void seek(int) = 0;
    virtual int get_pos() = 0;    // return 0 to indicate seek not supported
    virtual int get_pos_ms() = 0; // this must always return valid value if poss
    virtual int get_length_ms() = 0; // return total track length in ms (or 0)
    virtual int get_sound_type() = 0;
    virtual int play() = 0;

    virtual int play_from(int position);

    virtual void set_panning(int newPanning);
    virtual void set_speed(int new_speed) { speed = new_speed; }

    virtual void pause();
    virtual void resume();

    inline bool is_playing() const { return state_ == SoundClipPlaying || state_ == SoundClipPaused; }

    inline int get_speed() const
    {
        return speed;
    }

    // Gets clip's volume property, as percentage (0 - 100);
    // note this may not be the real volume of playback (which could e.g. be muted)
    inline int get_volume() const
    {
        return volAsPercentage;
    }

    inline bool is_muted() const
    {
        return muted;
    }

    // Sets the current volume property, as percentage (0 - 100).
    inline void set_volume_percent(int volume)
    {
        volAsPercentage = volume;
        if (!muted)
            set_volume((volume * 255) / 100);
    }

    // Explicitly defines both percentage and absolute volume value,
    // without calculating it from given percentage.
    // NOTE: this overrides the mute
    inline void set_volume_direct(int vol_percent, int vol_absolute)
    {
        muted = false;
        volAsPercentage = vol_percent;
        set_volume(vol_absolute);
    }

    // Mutes sound clip, while preserving current volume property
    // for the future reference; when unmuted, that property is
    // used to restart previous volume.
    inline void set_mute(bool enable)
    {
        muted = enable;
        if (enable)
            set_volume(0);
        else
            set_volume((volAsPercentage * 255) / 100);
    }

    // Apply arbitrary permanent volume modifier, in absolute units (0 - 255);
    // this is distinct value that is used in conjunction with current volume
    // (can be both positive and negative).
    inline void apply_volume_modifier(int mod)
    {
        volModifier = mod;
        adjust_volume();
    }

    // Apply permanent directional volume modifier, in absolute units (0 - 255)
    // this is distinct value that is used in conjunction with current volume
    // (can be both positive and negative).
    inline void apply_directional_modifier(int mod)
    {
        directionalVolModifier = mod;
        adjust_volume();
    }

    virtual void adjust_volume() = 0;

    SOUNDCLIP();
    virtual ~SOUNDCLIP();


protected:

    SoundClipState state_;

    // mute mode overrides the volume; if set, any volume assigned is stored
    // in properties, but not applied to playback itself
    bool muted;

    // speed of playback, in clip ms per real second
    int speed; 

    // Return the allegro voice number (or -1 if none)
    // Used by generic pause/resume functions.
    virtual int get_voice() = 0;

    // helper function for calculating volume with applied modifiers
    inline int get_final_volume() const
    {
        int final_vol = vol + volModifier + directionalVolModifier;
        return final_vol >= 0 ? final_vol : 0;
    }
};

#endif // __AC_SOUNDCLIP_H
