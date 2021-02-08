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

#ifndef AGS_ENGINE_MEDIA_AUDIO_AUDIO_H
#define AGS_ENGINE_MEDIA_AUDIO_AUDIO_H

#include "ags/lib/std/array.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/shared/ac/dynobj/scriptaudioclip.h"
#include "ags/engine/ac/dynobj/scriptaudiochannel.h"
#include "ags/engine/media/audio/ambientsound.h"
#include "ags/engine/util/mutex.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/engine/ac/timer.h"

namespace AGS3 {

struct SOUNDCLIP;

//controls access to the channels, since that's the main point of synchronization between the streaming thread and the user code
//this is going to be dependent on the underlying mutexes being recursive
//yes, we will have more recursive traffic on mutexes than we need
//however this should mostly be happening only when playing sounds, and possibly when sounds numbering only several
//the load should not be high
class AudioChannelsLock : public AGS::Engine::MutexLock {
private:
	AudioChannelsLock(AudioChannelsLock const &); // non-copyable
	AudioChannelsLock &operator=(AudioChannelsLock const &); // not copy-assignable

public:
	AudioChannelsLock();

	// Gets a clip from the channel
	SOUNDCLIP *GetChannel(int index);
	// Gets a clip from the channel but only if it's in playback state
	SOUNDCLIP *GetChannelIfPlaying(int index);
	// Assign new clip to the channel
	SOUNDCLIP *SetChannel(int index, SOUNDCLIP *clip);
	// Move clip from one channel to another, clearing the first channel
	SOUNDCLIP *MoveChannel(int to, int from);
};

//
// Channel helpers, autolock and perform a simple action on a channel.
//
// Tells if channel has got a clip; does not care about its state
bool channel_has_clip(int chanid);
// Tells if channel has got a clip and clip is in playback state
bool channel_is_playing(int chanid);
// Sets new clip to the channel
void set_clip_to_channel(int chanid, SOUNDCLIP *clip);


void        calculate_reserved_channel_count();
void        update_clip_default_volume(ScriptAudioClip *audioClip);
void        start_fading_in_new_track_if_applicable(int fadeInChannel, ScriptAudioClip *newSound);
void        stop_or_fade_out_channel(int fadeOutChannel, int fadeInChannel = -1, ScriptAudioClip *newSound = nullptr);
SOUNDCLIP *load_sound_clip(ScriptAudioClip *audioClip, bool repeat);
ScriptAudioChannel *play_audio_clip_on_channel(int channel, ScriptAudioClip *clip, int priority, int repeat, int fromOffset, SOUNDCLIP *cachedClip = nullptr);
void        remove_clips_of_type_from_queue(int audioType);
void        update_queued_clips_volume(int audioType, int new_vol);
// Checks if speech voice-over is currently playing, and reapply volume drop to all other active clips
void        update_volume_drop_if_voiceover();
ScriptAudioChannel *play_audio_clip(ScriptAudioClip *clip, int priority, int repeat, int fromOffset, bool queueIfNoChannel);
ScriptAudioChannel *play_audio_clip_by_index(int audioClipIndex);
void        stop_and_destroy_channel_ex(int chid, bool resetLegacyMusicSettings);
void        stop_and_destroy_channel(int chid);

// ***** BACKWARDS COMPATIBILITY WITH OLD AUDIO SYSTEM ***** //
int         get_old_style_number_for_sound(int sound_number);
SOUNDCLIP *load_sound_clip_from_old_style_number(bool isMusic, int indexNumber, bool repeat);

//=============================================================================

int         get_volume_adjusted_for_distance(int volume, int sndX, int sndY, int sndMaxDist);
void        update_directional_sound_vol();
void        update_ambient_sound_vol();
// Tells if the audio type is allowed to play with regards to current sound config
bool        is_audiotype_allowed_to_play(AudioFileType type);
// Loads sound data referenced by audio clip item, and starts playback;
// returns NULL on failure
SOUNDCLIP *load_sound_and_play(ScriptAudioClip *aclip, bool repeat);
void        stop_all_sound_and_music();
void        shutdown_sound();
int         play_sound(int val1);

//=============================================================================

// This is an indicator of a music played by an old audio system
// (to distinguish from the new system API); if it is not set, then old API
// should "think" that no music is played regardless of channel state
// TODO: refactor this and hide behind some good interface to prevent misuse!
extern int current_music_type;

void        clear_music_cache();
void        play_next_queued();
int         calculate_max_volume();
// add/remove the volume drop to the audio channels while speech is playing
void        apply_volume_drop_modifier(bool applyModifier);
// Update the music, and advance the crossfade on a step
// (this should only be called once per game loop);
void        update_audio_system_on_game_loop();
void        stopmusic();
void        update_music_volume();
void        post_new_music_check(int newchannel);
// Sets up the crossfading for playing the new music track,
// and returns the channel number to use; the channel is guaranteed to be free
int         prepare_for_new_music();
// Gets audio clip from legacy music number, which also may contain queue flag
ScriptAudioClip *get_audio_clip_for_music(int mnum);
SOUNDCLIP *load_music_from_disk(int mnum, bool doRepeat);
void        newmusic(int mnum);

extern volatile bool _audio_doing_crossfade;

extern void cancel_scheduled_music_update();
extern void schedule_music_update_at(AGS_Clock::time_point);
extern void postpone_scheduled_music_update_by(std::chrono::milliseconds);

// crossFading is >0 (channel number of new track), or -1 (old
// track fading out, no new track)
extern int crossFading, crossFadeVolumePerStep, crossFadeStep;
extern int crossFadeVolumeAtStart;

extern SOUNDCLIP *cachedQueuedMusic;

// TODO: double check that ambient sounds array actually needs +1
extern std::array<AmbientSound> ambient;

} // namespace AGS3

#endif
