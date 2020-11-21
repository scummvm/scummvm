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

#include <math.h>

#include "core/platform.h"
#include "util/wgt2allg.h"
#include "media/audio/audio.h"
#include "ac/audiocliptype.h"
#include "ac/gamesetupstruct.h"
#include "ac/dynobj/cc_audioclip.h"
#include "ac/dynobj/cc_audiochannel.h"
#include "ac/gamestate.h"
#include "script/script_runtime.h"
#include "ac/audiochannel.h"
#include "ac/audioclip.h"
#include "ac/gamesetup.h"
#include "ac/path_helper.h"
#include "media/audio/sound.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "ac/common.h"
#include "ac/file.h"
#include "ac/global_audio.h"
#include <math.h>
#include "util/stream.h"
#include "core/assetmanager.h"
#include "ac/timer.h"
#include "main/game_run.h"

using namespace AGS::Common;
using namespace AGS::Engine;

//-----------------------
//sound channel management; all access goes through here, which can't be done without a lock

static std::array<SOUNDCLIP *,MAX_SOUND_CHANNELS+1> _channels;
AGS::Engine::Mutex AudioChannelsLock::s_mutex;

SOUNDCLIP *AudioChannelsLock::GetChannel(int index)
{
    return _channels[index];
}

SOUNDCLIP *AudioChannelsLock::GetChannelIfPlaying(int index)
{
    auto *ch = _channels[index];
    return (ch != nullptr && ch->is_playing()) ? ch : nullptr;
}

SOUNDCLIP *AudioChannelsLock::SetChannel(int index, SOUNDCLIP* ch)
{
    // TODO: store clips in smart pointers
    if (_channels[index] == ch)
        Debug::Printf(kDbgMsg_Warn, "WARNING: channel %d - same clip assigned", index);
    else if (_channels[index] != nullptr && ch != nullptr)
        Debug::Printf(kDbgMsg_Warn, "WARNING: channel %d - clip overwritten", index);
    _channels[index] = ch;
    return ch;
}

SOUNDCLIP *AudioChannelsLock::MoveChannel(int to, int from)
{
    auto from_ch = _channels[from];
    _channels[from] = nullptr;
    return SetChannel(to, from_ch);
}

//-----------------------
// Channel helpers

bool channel_has_clip(int chanid)
{
    AudioChannelsLock lock;
    return lock.GetChannel(chanid) != nullptr;
}

bool channel_is_playing(int chanid)
{
    AudioChannelsLock lock;
    return lock.GetChannelIfPlaying(chanid) != nullptr;
}

void set_clip_to_channel(int chanid, SOUNDCLIP *clip)
{
    AudioChannelsLock lock;
    lock.SetChannel(chanid, clip);
}
//-----------------------

volatile bool _audio_doing_crossfade;

extern GameSetupStruct game;
extern GameSetup usetup;
extern GameState play;
extern RoomStruct thisroom;
extern CharacterInfo*playerchar;

extern volatile int switching_away_from_game;

#if ! AGS_PLATFORM_OS_IOS && ! AGS_PLATFORM_OS_ANDROID
volatile int psp_audio_multithreaded = 0;
#endif

ScriptAudioChannel scrAudioChannel[MAX_SOUND_CHANNELS + 1];
char acaudio_buffer[256];
int reserved_channel_count = 0;

AGS::Engine::Thread audioThread;

void calculate_reserved_channel_count()
{
    int reservedChannels = 0;
    for (size_t i = 0; i < game.audioClipTypes.size(); i++)
    {
        reservedChannels += game.audioClipTypes[i].reservedChannels;
    }
    reserved_channel_count = reservedChannels;
}

void update_clip_default_volume(ScriptAudioClip *audioClip)
{
    if (play.default_audio_type_volumes[audioClip->type] >= 0) 
    {
        audioClip->defaultVolume = play.default_audio_type_volumes[audioClip->type];
    }
}

void start_fading_in_new_track_if_applicable(int fadeInChannel, ScriptAudioClip *newSound)
{
    int crossfadeSpeed = game.audioClipTypes[newSound->type].crossfadeSpeed;
    if (crossfadeSpeed > 0)
    {
        update_clip_default_volume(newSound);
        play.crossfade_in_volume_per_step = crossfadeSpeed;
        play.crossfade_final_volume_in = newSound->defaultVolume;
        play.crossfading_in_channel = fadeInChannel;
    }
}

static void move_track_to_crossfade_channel(int currentChannel, int crossfadeSpeed, int fadeInChannel, ScriptAudioClip *newSound)
{
    AudioChannelsLock lock;
    stop_and_destroy_channel(SPECIAL_CROSSFADE_CHANNEL);
    auto *cfade_clip = lock.MoveChannel(SPECIAL_CROSSFADE_CHANNEL, currentChannel);
    if (!cfade_clip)
        return;

    play.crossfading_out_channel = SPECIAL_CROSSFADE_CHANNEL;
    play.crossfade_step = 0;
    play.crossfade_initial_volume_out = cfade_clip->get_volume();
    play.crossfade_out_volume_per_step = crossfadeSpeed;

    play.crossfading_in_channel = fadeInChannel;
    if (newSound != nullptr)
    {
        start_fading_in_new_track_if_applicable(fadeInChannel, newSound);
    }
}

void stop_or_fade_out_channel(int fadeOutChannel, int fadeInChannel, ScriptAudioClip *newSound)
{
    ScriptAudioClip *sourceClip = AudioChannel_GetPlayingClip(&scrAudioChannel[fadeOutChannel]);
    if ((sourceClip != nullptr) && (game.audioClipTypes[sourceClip->type].crossfadeSpeed > 0))
    {
        move_track_to_crossfade_channel(fadeOutChannel, game.audioClipTypes[sourceClip->type].crossfadeSpeed, fadeInChannel, newSound);
    }
    else
    {
        stop_and_destroy_channel(fadeOutChannel);
    }
}

static int find_free_audio_channel(ScriptAudioClip *clip, int priority, bool interruptEqualPriority)
{
    AudioChannelsLock lock;

    int lowestPrioritySoFar = 9999999;
    int lowestPriorityID = -1;
    int channelToUse = -1;

    if (!interruptEqualPriority)
        priority--;

    int startAtChannel = reserved_channel_count;
    int endBeforeChannel = MAX_SOUND_CHANNELS;

    if (game.audioClipTypes[clip->type].reservedChannels > 0)
    {
        startAtChannel = 0;
        for (int i = 0; i < clip->type; i++)
        {
            startAtChannel += game.audioClipTypes[i].reservedChannels;
        }
        endBeforeChannel = startAtChannel + game.audioClipTypes[clip->type].reservedChannels;
    }

    for (int i = startAtChannel; i < endBeforeChannel; i++)
    {
        auto* ch = lock.GetChannelIfPlaying(i);
        if (ch == nullptr)
        {
            channelToUse = i;
            stop_and_destroy_channel(i);
            break;
        }
        if ((ch->priority < lowestPrioritySoFar) &&
            (ch->sourceClipType == clip->type))
        {
            lowestPrioritySoFar = ch->priority;
            lowestPriorityID = i;
        }
    }

    if ((channelToUse < 0) && (lowestPriorityID >= 0) &&
        (lowestPrioritySoFar <= priority))
    {
        stop_or_fade_out_channel(lowestPriorityID, lowestPriorityID, clip);
        channelToUse = lowestPriorityID;
    }
    else if ((channelToUse >= 0) && (play.crossfading_in_channel < 1))
    {
        start_fading_in_new_track_if_applicable(channelToUse, clip);
    }
    return channelToUse;
}

bool is_audiotype_allowed_to_play(AudioFileType type)
{
    return (type == eAudioFileMIDI && usetup.midicard != MIDI_NONE) ||
           (type != eAudioFileMIDI && usetup.digicard != DIGI_NONE);
}

SOUNDCLIP *load_sound_clip(ScriptAudioClip *audioClip, bool repeat)
{
    if (!is_audiotype_allowed_to_play((AudioFileType)audioClip->fileType))
    {
        return nullptr;
    }

    update_clip_default_volume(audioClip);

    SOUNDCLIP *soundClip = nullptr;
    AssetPath asset_name = get_audio_clip_assetpath(audioClip->bundlingType, audioClip->fileName);
    switch (audioClip->fileType)
    {
    case eAudioFileOGG:
        soundClip = my_load_static_ogg(asset_name, audioClip->defaultVolume, repeat);
        break;
    case eAudioFileMP3:
        soundClip = my_load_static_mp3(asset_name, audioClip->defaultVolume, repeat);
        break;
    case eAudioFileWAV:
    case eAudioFileVOC:
        soundClip = my_load_wave(asset_name, audioClip->defaultVolume, repeat);
        break;
    case eAudioFileMIDI:
        soundClip = my_load_midi(asset_name, repeat);
        break;
    case eAudioFileMOD:
#ifndef PSP_NO_MOD_PLAYBACK
        soundClip = my_load_mod(asset_name, repeat);
#else
        soundClip = NULL;
#endif
        break;
    default:
        quitprintf("AudioClip.Play: invalid audio file type encountered: %d", audioClip->fileType);
    }
    if (soundClip != nullptr)
    {
        soundClip->set_volume_percent(audioClip->defaultVolume);
        soundClip->sourceClip = audioClip;
        soundClip->sourceClipType = audioClip->type;
    }
    return soundClip;
}

static void audio_update_polled_stuff()
{
    ///////////////////////////////////////////////////////////////////////////
    // Do crossfade
    play.crossfade_step++;

    AudioChannelsLock lock;

    if (play.crossfading_out_channel > 0 && !lock.GetChannelIfPlaying(play.crossfading_out_channel))
        play.crossfading_out_channel = 0;

    if (play.crossfading_out_channel > 0)
    {
        SOUNDCLIP* ch = lock.GetChannel(play.crossfading_out_channel);
        int newVolume = ch ? ch->get_volume() - play.crossfade_out_volume_per_step : 0;
        if (newVolume > 0)
        {
            AudioChannel_SetVolume(&scrAudioChannel[play.crossfading_out_channel], newVolume);
        }
        else
        {
            stop_and_destroy_channel(play.crossfading_out_channel);
            play.crossfading_out_channel = 0;
        }
    }

    if (play.crossfading_in_channel > 0 && !lock.GetChannelIfPlaying(play.crossfading_in_channel))
        play.crossfading_in_channel = 0;

    if (play.crossfading_in_channel > 0)
    {
        SOUNDCLIP* ch = lock.GetChannel(play.crossfading_in_channel);
        int newVolume = ch ? ch->get_volume() + play.crossfade_in_volume_per_step : 0;
        if (newVolume > play.crossfade_final_volume_in)
        {
            newVolume = play.crossfade_final_volume_in;
        }

        AudioChannel_SetVolume(&scrAudioChannel[play.crossfading_in_channel], newVolume);

        if (newVolume >= play.crossfade_final_volume_in)
        {
            play.crossfading_in_channel = 0;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Do audio queue
    if (play.new_music_queue_size > 0)
    {
        for (int i = 0; i < play.new_music_queue_size; i++)
        {
            ScriptAudioClip *clip = &game.audioClips[play.new_music_queue[i].audioClipIndex];
            int channel = find_free_audio_channel(clip, clip->defaultPriority, false);
            if (channel >= 0)
            {
                QueuedAudioItem itemToPlay = play.new_music_queue[i];

                play.new_music_queue_size--;
                for (int j = i; j < play.new_music_queue_size; j++)
                {
                    play.new_music_queue[j] = play.new_music_queue[j + 1];
                }

                play_audio_clip_on_channel(channel, clip, itemToPlay.priority, itemToPlay.repeat, 0, itemToPlay.cachedClip);
                i--;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Do non-blocking voice speech
    // NOTE: there's only one speech channel, therefore it's either blocking
    // or non-blocking at any given time. If it's changed, we'd need to keep
    // record of every channel, or keep a count of active channels.
    if (play.IsNonBlockingVoiceSpeech())
    {
        if (!channel_is_playing(SCHAN_SPEECH))
        {
            stop_voice_nonblocking();
        }
    }
}

// Applies a volume drop modifier to the clip, in accordance to its audio type
static void apply_volume_drop_to_clip(SOUNDCLIP *clip)
{
    int audiotype = clip->sourceClipType;
    clip->apply_volume_modifier(-(game.audioClipTypes[audiotype].volume_reduction_while_speech_playing * 255 / 100));
}

static void queue_audio_clip_to_play(ScriptAudioClip *clip, int priority, int repeat)
{
    if (play.new_music_queue_size >= MAX_QUEUED_MUSIC) {
        debug_script_log("Too many queued music, cannot add %s", clip->scriptName.GetCStr());
        return;
    }

    SOUNDCLIP *cachedClip = load_sound_clip(clip, (repeat != 0));
    if (cachedClip != nullptr) 
    {
        play.new_music_queue[play.new_music_queue_size].audioClipIndex = clip->id;
        play.new_music_queue[play.new_music_queue_size].priority = priority;
        play.new_music_queue[play.new_music_queue_size].repeat = (repeat != 0);
        play.new_music_queue[play.new_music_queue_size].cachedClip = cachedClip;
        play.new_music_queue_size++;
    }
    
    update_polled_mp3();
}

ScriptAudioChannel* play_audio_clip_on_channel(int channel, ScriptAudioClip *clip, int priority, int repeat, int fromOffset, SOUNDCLIP *soundfx)
{
    if (soundfx == nullptr)
    {
        soundfx = load_sound_clip(clip, (repeat) ? true : false);
    }
    if (soundfx == nullptr)
    {
        debug_script_log("AudioClip.Play: unable to load sound file");
        if (play.crossfading_in_channel == channel)
        {
            play.crossfading_in_channel = 0;
        }
        return nullptr;
    }
    soundfx->priority = priority;

    if (play.crossfading_in_channel == channel)
    {
        soundfx->set_volume_percent(0);
    }

    // Mute the audio clip if fast-forwarding the cutscene
    if (play.fast_forward) 
    {
        soundfx->set_mute(true);

        // CHECKME!!
        // [IKM] According to the 3.2.1 logic the clip will restore
        // its value after cutscene, but only if originalVolAsPercentage
        // is not zeroed. Something I am not sure about: why does it
        // disable the clip under condition that there's more than one
        // channel for this audio type? It does not even check if
        // anything of this type is currently playing.
        if (game.audioClipTypes[clip->type].reservedChannels != 1)
            soundfx->set_volume_percent(0);
    }

    if (soundfx->play_from(fromOffset) == 0)
    {
        // not assigned to a channel, so clean up manually.
        soundfx->destroy();
        delete soundfx;
        soundfx = nullptr;
        debug_script_log("AudioClip.Play: failed to play sound file");
        return nullptr;
    }

    // Apply volume drop if any speech voice-over is currently playing
    // NOTE: there is a confusing logic in sound clip classes, that they do not use
    // any modifiers when begin playing, therefore we must apply this only after
    // playback was started.
    if (!play.fast_forward && play.speech_has_voice)
        apply_volume_drop_to_clip(soundfx);

    set_clip_to_channel(channel, soundfx);
    return &scrAudioChannel[channel];
}

void remove_clips_of_type_from_queue(int audioType) 
{
    int aa;
    for (aa = 0; aa < play.new_music_queue_size; aa++)
    {
        ScriptAudioClip *clip = &game.audioClips[play.new_music_queue[aa].audioClipIndex];
        if (clip->type == audioType)
        {
            play.new_music_queue_size--;
            for (int bb = aa; bb < play.new_music_queue_size; bb++)
                play.new_music_queue[bb] = play.new_music_queue[bb + 1];
            aa--;
        }
    }
}

void update_queued_clips_volume(int audioType, int new_vol)
{
    for (int i = 0; i < play.new_music_queue_size; ++i)
    {
        // NOTE: if clip is uncached, the volume will be set from defaults when it is loaded
        SOUNDCLIP *sndclip = play.new_music_queue[i].cachedClip;
        if (sndclip)
        {
            ScriptAudioClip *clip = &game.audioClips[play.new_music_queue[i].audioClipIndex];
            if (clip->type == audioType)
                sndclip->set_volume_percent(new_vol);
        }
    }
}

ScriptAudioChannel* play_audio_clip(ScriptAudioClip *clip, int priority, int repeat, int fromOffset, bool queueIfNoChannel)
{
    if (!queueIfNoChannel)
        remove_clips_of_type_from_queue(clip->type);

    if (priority == SCR_NO_VALUE)
        priority = clip->defaultPriority;
    if (repeat == SCR_NO_VALUE)
        repeat = clip->defaultRepeat;

    int channel = find_free_audio_channel(clip, priority, !queueIfNoChannel);
    if (channel < 0)
    {
        if (queueIfNoChannel)
            queue_audio_clip_to_play(clip, priority, repeat);
        else
            debug_script_log("AudioClip.Play: no channels available to interrupt PRI:%d TYPE:%d", priority, clip->type);

        return nullptr;
    }

    return play_audio_clip_on_channel(channel, clip, priority, repeat, fromOffset);
}

ScriptAudioChannel* play_audio_clip_by_index(int audioClipIndex)
{
    if ((audioClipIndex >= 0) && ((size_t)audioClipIndex < game.audioClips.size()))
        return AudioClip_Play(&game.audioClips[audioClipIndex], SCR_NO_VALUE, SCR_NO_VALUE);
    else 
        return nullptr;
}

void stop_and_destroy_channel_ex(int chid, bool resetLegacyMusicSettings)
{
    if ((chid < 0) || (chid > MAX_SOUND_CHANNELS))
        quit("!StopChannel: invalid channel ID");

    AudioChannelsLock lock;
    SOUNDCLIP* ch = lock.GetChannel(chid);

    if (ch != nullptr) {
        ch->destroy();
        delete ch;
        lock.SetChannel(chid, nullptr);
        ch = nullptr;
    }

    if (play.crossfading_in_channel == chid)
        play.crossfading_in_channel = 0;
    if (play.crossfading_out_channel == chid)
        play.crossfading_out_channel = 0;
    // don't update 'crossFading' here as it is updated in all the cross-fading functions.

    // destroyed an ambient sound channel
    if (ambient[chid].channel > 0)
        ambient[chid].channel = 0;

    if ((chid == SCHAN_MUSIC) && (resetLegacyMusicSettings))
    {
        play.cur_music_number = -1;
        current_music_type = 0;
    }
}

void stop_and_destroy_channel(int chid)
{
    stop_and_destroy_channel_ex(chid, true);
}



// ***** BACKWARDS COMPATIBILITY WITH OLD AUDIO SYSTEM ***** //

int get_old_style_number_for_sound(int sound_number)
{
    int audio_clip_id = 0;

    if (game.IsLegacyAudioSystem())
    {
        // No sound assigned.
        if (sound_number < 1)
            return 0;

        // Sound number is not yet updated to audio clip id.
        if (sound_number <= 0x10000000)
            return sound_number;

        // Remove audio clip id flag.
        audio_clip_id = sound_number - 0x10000000;
    }
    else
        audio_clip_id = sound_number;

    if (audio_clip_id >= 0)
    {
        int old_style_number = 0;
        if (sscanf(game.audioClips[audio_clip_id].scriptName.GetCStr(), "aSound%d", &old_style_number) == 1)
            return old_style_number;    
    }
    return 0;
}

SOUNDCLIP *load_sound_clip_from_old_style_number(bool isMusic, int indexNumber, bool repeat)
{
    ScriptAudioClip* audioClip = GetAudioClipForOldStyleNumber(game, isMusic, indexNumber);

    if (audioClip != nullptr)
    {
        return load_sound_clip(audioClip, repeat);
    }

    return nullptr;
}

//=============================================================================

void force_audiostream_include() {
    // This should never happen, but the call is here to make it
    // link the audiostream libraries
    stop_audio_stream(nullptr);
}

// TODO: double check that ambient sounds array actually needs +1
std::array<AmbientSound,MAX_SOUND_CHANNELS+1> ambient;

int get_volume_adjusted_for_distance(int volume, int sndX, int sndY, int sndMaxDist)
{
    int distx = playerchar->x - sndX;
    int disty = playerchar->y - sndY;
    // it uses Allegro's "fix" sqrt without the ::
    int dist = (int)::sqrt((double)(distx*distx + disty*disty));

    // if they're quite close, full volume
    int wantvol = volume;

    if (dist >= AMBIENCE_FULL_DIST)
    {
        // get the relative volume
        wantvol = ((dist - AMBIENCE_FULL_DIST) * volume) / sndMaxDist;
        // closer is louder
        wantvol = volume - wantvol;
    }

    return wantvol;
}

void update_directional_sound_vol()
{
    AudioChannelsLock lock;

    for (int chnum = 1; chnum < MAX_SOUND_CHANNELS; chnum++) 
    {
        auto* ch = lock.GetChannelIfPlaying(chnum);
        if ((ch != nullptr) && (ch->xSource >= 0)) 
        {
            ch->apply_directional_modifier(
                get_volume_adjusted_for_distance(ch->vol, 
                    ch->xSource,
                    ch->ySource,
                    ch->maximumPossibleDistanceAway) -
                ch->vol);
        }
    }
}

void update_ambient_sound_vol ()
{
    AudioChannelsLock lock;

    for (int chan = 1; chan < MAX_SOUND_CHANNELS; chan++) {

        AmbientSound *thisSound = &ambient[chan];

        if (thisSound->channel == 0)
            continue;

        int sourceVolume = thisSound->vol;

        if (play.speech_has_voice) {
            // Negative value means set exactly; positive means drop that amount
            if (play.speech_music_drop < 0)
                sourceVolume = -play.speech_music_drop;
            else
                sourceVolume -= play.speech_music_drop;

            if (sourceVolume < 0)
                sourceVolume = 0;
            if (sourceVolume > 255)
                sourceVolume = 255;
        }

        // Adjust ambient volume so it maxes out at overall sound volume
        int ambientvol = (sourceVolume * play.sound_volume) / 255;

        int wantvol;

        if ((thisSound->x == 0) && (thisSound->y == 0)) {
            wantvol = ambientvol;
        }
        else {
            wantvol = get_volume_adjusted_for_distance(ambientvol, thisSound->x, thisSound->y, thisSound->maxdist);
        }

        auto *ch = lock.GetChannelIfPlaying(thisSound->channel);
        if (ch)
            ch->set_volume(wantvol);
    }
}

SOUNDCLIP *load_sound_and_play(ScriptAudioClip *aclip, bool repeat)
{
    SOUNDCLIP *soundfx = load_sound_clip(aclip, repeat);
    if (!soundfx) { return nullptr; }

    if (soundfx->play() == 0) {
        // not assigned to a channel, so clean up manually.
        soundfx->destroy();
        delete soundfx;
        return nullptr;
    }

    return soundfx;
}

void stop_all_sound_and_music() 
{
    int a;
    stopmusic();
    stop_voice_nonblocking();
    // make sure it doesn't start crossfading when it comes back
    crossFading = 0;
    // any ambient sound will be aborted
    for (a = 0; a <= MAX_SOUND_CHANNELS; a++)
        stop_and_destroy_channel(a);
}

void shutdown_sound() 
{
    stop_all_sound_and_music();

#ifndef PSP_NO_MOD_PLAYBACK
    if (usetup.mod_player)
        remove_mod_player();
#endif
    remove_sound();
}

// the sound will only be played if there is a free channel or
// it has a priority >= an existing sound to override
static int play_sound_priority (int val1, int priority) {
    int lowest_pri = 9999, lowest_pri_id = -1;

    AudioChannelsLock lock;

    // find a free channel to play it on
    for (int i = SCHAN_NORMAL; i < MAX_SOUND_CHANNELS; i++) {
        auto* ch = lock.GetChannelIfPlaying(i);
        if (val1 < 0) {
            // Playing sound -1 means iterate through and stop all sound
            if (ch)
                stop_and_destroy_channel (i);
        }
        else if (ch == nullptr || !ch->is_playing()) {
            // PlaySoundEx will destroy the previous channel value.
            const int usechan = PlaySoundEx(val1, i);
            if (usechan >= 0)
            { // channel will hold a different clip here
                assert(usechan == i);
                auto *ch = lock.GetChannel(usechan);
                if (ch)
                    ch->priority = priority;
            }
            return usechan;
        }
        else if (ch->priority < lowest_pri) {
            lowest_pri = ch->priority;
            lowest_pri_id = i;
        }

    }
    if (val1 < 0)
        return -1;

    // no free channels, see if we have a high enough priority
    // to override one
    if (priority >= lowest_pri) {
        const int usechan = PlaySoundEx(val1, lowest_pri_id);
        if (usechan >= 0) {
            assert(usechan == lowest_pri_id);
            auto *ch = lock.GetChannel(usechan);
            if (ch)
                ch->priority = priority;
            return usechan;
        }
    }

    return -1;
}

int play_sound(int val1) {
    return play_sound_priority(val1, 10);
}


//=============================================================================


// This is an indicator of a music played by an old audio system
// (to distinguish from the new system API)
int current_music_type = 0;
// crossFading is >0 (channel number of new track), or -1 (old
// track fading out, no new track)
int crossFading = 0, crossFadeVolumePerStep = 0, crossFadeStep = 0;
int crossFadeVolumeAtStart = 0;
SOUNDCLIP *cachedQueuedMusic = nullptr;

//=============================================================================
// Music update is scheduled when the voice speech stops;
// we do a small delay before reverting any volume adjustments
static bool music_update_scheduled = false;
static auto music_update_at = AGS_Clock::now();

void cancel_scheduled_music_update() {
    music_update_scheduled = false;
}

void schedule_music_update_at(AGS_Clock::time_point at) {
    music_update_scheduled = true;
    music_update_at = at;
}

void postpone_scheduled_music_update_by(std::chrono::milliseconds duration) {
    if (!music_update_scheduled) { return; }
    music_update_at += duration;
}

void process_scheduled_music_update() {
    if (!music_update_scheduled) { return; }
    if (music_update_at > AGS_Clock::now()) { return; }
    cancel_scheduled_music_update();
    update_music_volume();
    apply_volume_drop_modifier(false);
    update_ambient_sound_vol();
}
// end scheduled music update functions
//=============================================================================

void clear_music_cache() {

    if (cachedQueuedMusic != nullptr) {
        cachedQueuedMusic->destroy();
        delete cachedQueuedMusic;
        cachedQueuedMusic = nullptr;
    }

}

static void play_new_music(int mnum, SOUNDCLIP *music);

void play_next_queued() {
    // check if there's a queued one to play
    if (play.music_queue_size > 0) {

        int tuneToPlay = play.music_queue[0];

        if (tuneToPlay >= QUEUED_MUSIC_REPEAT) {
            // Loop it!
            play.music_repeat++;
            play_new_music(tuneToPlay - QUEUED_MUSIC_REPEAT, cachedQueuedMusic);
            play.music_repeat--;
        }
        else {
            // Don't loop it!
            int repeatWas = play.music_repeat;
            play.music_repeat = 0;
            play_new_music(tuneToPlay, cachedQueuedMusic);
            play.music_repeat = repeatWas;
        }

        // don't free the memory, as it has been transferred onto the
        // main music channel
        cachedQueuedMusic = nullptr;

        play.music_queue_size--;
        for (int i = 0; i < play.music_queue_size; i++)
            play.music_queue[i] = play.music_queue[i + 1];

        if (play.music_queue_size > 0)
            cachedQueuedMusic = load_music_from_disk(play.music_queue[0], 0);
    }

}

int calculate_max_volume() {
    // quieter so that sounds can be heard better
    int newvol=play.music_master_volume + ((int)thisroom.Options.MusicVolume) * LegacyRoomVolumeFactor;
    if (newvol>255) newvol=255;
    if (newvol<0) newvol=0;

    if (play.fast_forward)
        newvol = 0;

    return newvol;
}

// add/remove the volume drop to the audio channels while speech is playing
void apply_volume_drop_modifier(bool applyModifier)
{
    AudioChannelsLock lock;

    for (int i = 0; i < MAX_SOUND_CHANNELS; i++) 
    {
        auto* ch = lock.GetChannelIfPlaying(i);
        if (ch && ch->sourceClip != nullptr)
        {
            if (applyModifier)
                apply_volume_drop_to_clip(ch);
            else
                ch->apply_volume_modifier(0); // reset modifier
        }
    }
}

// Checks if speech voice-over is currently playing, and reapply volume drop to all other active clips
void update_volume_drop_if_voiceover()
{
    apply_volume_drop_modifier(play.speech_has_voice);
}

extern volatile char want_exit;

void update_mp3_thread()
{
	if (switching_away_from_game) { return; }

    AudioChannelsLock lock;

    for(int i = 0; i <= MAX_SOUND_CHANNELS; ++i)
    {
        auto* ch = lock.GetChannel(i);
        if (ch)
            ch->poll();
    }
}

//this is called at various points to give streaming logic a chance to update
//it seems those calls have been littered around and points where it ameliorated skipping
//a better solution would be to forcibly thread the streaming logic
void update_polled_mp3()
{
	if (psp_audio_multithreaded) { return; }
    update_mp3_thread();
}

// Update the music, and advance the crossfade on a step
// (this should only be called once per game loop)
void update_audio_system_on_game_loop ()
{
	update_polled_stuff_if_runtime ();

    AudioChannelsLock lock;

    process_scheduled_music_update();

    _audio_doing_crossfade = true;

    audio_update_polled_stuff();

    if (crossFading) {
        crossFadeStep++;
        update_music_volume();
    }

    // Check if the current music has finished playing
    if ((play.cur_music_number >= 0) && (play.fast_forward == 0)) {
        if (IsMusicPlaying() == 0) {
            // The current music has finished
            play.cur_music_number = -1;
            play_next_queued();
        }
        else if ((game.options[OPT_CROSSFADEMUSIC] > 0) &&
            (play.music_queue_size > 0) && (!crossFading)) {
                // want to crossfade, and new tune in the queue
                auto *ch = lock.GetChannel(SCHAN_MUSIC);
                if (ch) {
                    int curpos = ch->get_pos_ms();
                    int muslen = ch->get_length_ms();
                    if ((curpos > 0) && (muslen > 0)) {
                        // we want to crossfade, and we know how far through
                        // the tune we are
                        int takesSteps = calculate_max_volume() / game.options[OPT_CROSSFADEMUSIC];
                        int takesMs = ::lround(takesSteps * 1000.0f / get_current_fps());
                        if (curpos >= muslen - takesMs)
                            play_next_queued();
                    }
                }
        }
    }

    _audio_doing_crossfade = false;

}

void stopmusic()
{
    AudioChannelsLock lock;

    if (crossFading > 0) {
        // stop in the middle of a new track fading in
        // Abort the new track, and let the old one finish fading out
        stop_and_destroy_channel (crossFading);
        crossFading = -1;
    }
    else if (crossFading < 0) {
        // the music is already fading out
        if (game.options[OPT_CROSSFADEMUSIC] <= 0) {
            // If they have since disabled crossfading, stop the fadeout
            stop_and_destroy_channel(SCHAN_MUSIC);
            crossFading = 0;
            crossFadeStep = 0;
            update_music_volume();
        }
    }
    else if ((game.options[OPT_CROSSFADEMUSIC] > 0)
        && (lock.GetChannelIfPlaying(SCHAN_MUSIC) != nullptr)
        && (current_music_type != 0)
        && (current_music_type != MUS_MIDI)
        && (current_music_type != MUS_MOD)) {

        crossFading = -1;
        crossFadeStep = 0;
        crossFadeVolumePerStep = game.options[OPT_CROSSFADEMUSIC];
        crossFadeVolumeAtStart = calculate_max_volume();
    }
    else
        stop_and_destroy_channel (SCHAN_MUSIC);

    play.cur_music_number = -1;
    current_music_type = 0;
}

void update_music_volume()
{
    AudioChannelsLock lock;

    if ((current_music_type) || (crossFading < 0)) 
    {
        // targetVol is the maximum volume we're fading in to
        // newvol is the starting volume that we faded out from
        int targetVol = calculate_max_volume();
        int newvol;
        if (crossFading)
            newvol = crossFadeVolumeAtStart;
        else
            newvol = targetVol;

        // fading out old track, target volume is silence
        if (crossFading < 0)
            targetVol = 0;

        if (crossFading) {
            int curvol = crossFadeVolumePerStep * crossFadeStep;

            if ((curvol > targetVol) && (curvol > newvol)) {
                // it has fully faded to the new track
                newvol = targetVol;
                stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
                if (crossFading > 0) {
                    lock.MoveChannel(SCHAN_MUSIC, crossFading);
                }
                crossFading = 0;
            }
            else {
                if (crossFading > 0)
                {
                    auto *ch = lock.GetChannel(crossFading);
                    if (ch)
                        ch->set_volume((curvol > targetVol) ? targetVol : curvol);
                }

                newvol -= curvol;
                if (newvol < 0)
                    newvol = 0;
            }
        }
        auto *ch = lock.GetChannel(SCHAN_MUSIC);
        if (ch)
            ch->set_volume(newvol);
    }
}

// Ensures crossfader is stable after loading (or failing to load)
// new music
void post_new_music_check (int newchannel)
{
    AudioChannelsLock lock;
    if ((crossFading > 0) && (lock.GetChannel(crossFading) == nullptr)) {
        crossFading = 0;
        // Was fading out but then they played invalid music, continue to fade out
        if (lock.GetChannel(SCHAN_MUSIC) != nullptr)
            crossFading = -1;
    }

}

int prepare_for_new_music ()
{
    AudioChannelsLock lock;

    int useChannel = SCHAN_MUSIC;

    if ((game.options[OPT_CROSSFADEMUSIC] > 0)
        && (lock.GetChannelIfPlaying(SCHAN_MUSIC) != nullptr)
        && (current_music_type != MUS_MIDI)
        && (current_music_type != MUS_MOD)) {

            if (crossFading > 0) {
                // It's still crossfading to the previous track
                stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
                lock.MoveChannel(SCHAN_MUSIC, crossFading);
                crossFading = 0;
                update_music_volume();
            }
            else if (crossFading < 0) {
                // an old track is still fading out, no new music yet
                // Do nothing, and keep the current crossfade step
            }
            else {
                // start crossfading
                crossFadeStep = 0;
                crossFadeVolumePerStep = game.options[OPT_CROSSFADEMUSIC];
                crossFadeVolumeAtStart = calculate_max_volume();
            }
            useChannel = SPECIAL_CROSSFADE_CHANNEL;
            crossFading = useChannel;
    }
    else {
        // crossfading is now turned off
        stopmusic();
        // ensure that any traces of old tunes fading are eliminated
        // (otherwise the new track will be faded out)
        crossFading = 0;
    }

    // Just make sure, because it will be overwritten in a sec
    if (lock.GetChannel(useChannel) != nullptr)
        stop_and_destroy_channel (useChannel);

    return useChannel;
}

ScriptAudioClip *get_audio_clip_for_music(int mnum)
{
    if (mnum >= QUEUED_MUSIC_REPEAT)
        mnum -= QUEUED_MUSIC_REPEAT;
    return GetAudioClipForOldStyleNumber(game, true, mnum);
}

SOUNDCLIP *load_music_from_disk(int mnum, bool doRepeat) {

    if (mnum >= QUEUED_MUSIC_REPEAT) {
        mnum -= QUEUED_MUSIC_REPEAT;
        doRepeat = true;
    }

    SOUNDCLIP *loaded = load_sound_clip_from_old_style_number(true, mnum, doRepeat);

    if ((loaded == nullptr) && (mnum > 0)) 
    {
        debug_script_warn("Music %d not found",mnum);
        debug_script_log("FAILED to load music %d", mnum);
    }

    return loaded;
}

static void play_new_music(int mnum, SOUNDCLIP *music)
{
    if (debug_flags & DBG_NOMUSIC)
        return;

    if ((play.cur_music_number == mnum) && (music == nullptr)) {
        debug_script_log("PlayMusic %d but already playing", mnum);
        return;  // don't play the music if it's already playing
    }

    ScriptAudioClip *aclip = get_audio_clip_for_music(mnum);
    if (aclip && !is_audiotype_allowed_to_play((AudioFileType)aclip->fileType))
        return;

    int useChannel = SCHAN_MUSIC;
    debug_script_log("Playing music %d", mnum);

    if (mnum<0) {
        stopmusic();
        return;
    }

    if (play.fast_forward) {
        // while skipping cutscene, don't change the music
        play.end_cutscene_music = mnum;
        return;
    }

    useChannel = prepare_for_new_music();
    play.cur_music_number = mnum;
    current_music_type = 0;

    play.current_music_repeating = play.music_repeat;
    // now that all the previous music is unloaded, load in the new one

    SOUNDCLIP *new_clip;
    if (music != nullptr)
        new_clip = music;
    else
        new_clip = load_music_from_disk(mnum, (play.music_repeat > 0));

    AudioChannelsLock lock;
    auto* ch = lock.SetChannel(useChannel, new_clip);
    if (ch != nullptr) {
        if (!ch->play()) {
            // previous behavior was to set channel[] to null on error, so continue to do that here.
            ch->destroy();
            delete ch;
            ch = nullptr;
            lock.SetChannel(useChannel, nullptr);
        } else
            current_music_type = ch->get_sound_type();
    }

    post_new_music_check(useChannel);
    update_music_volume();
}

void newmusic(int mnum)
{
    play_new_music(mnum, nullptr);
}
