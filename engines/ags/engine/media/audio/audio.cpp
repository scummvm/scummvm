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

#include "ags/shared/core/platform.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/shared/ac/audio_clip_type.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/dynobj/cc_audio_clip.h"
#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/audio_channel.h"
#include "ags/engine/ac/audio_clip.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/media/audio/sound.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/file.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/main/game_run.h"
#include "ags/globals.h"
#include "ags/ags.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

//-----------------------
//sound channel management; all access goes through here, which can't be done without a lock

SOUNDCLIP *AudioChans::GetChannel(int index) {
	return _GP(audioChannels)[index];
}

SOUNDCLIP *AudioChans::GetChannelIfPlaying(int index) {
	auto *ch = _GP(audioChannels)[index];
	return (ch != nullptr && ch->is_playing()) ? ch : nullptr;
}

SOUNDCLIP *AudioChans::SetChannel(int index, SOUNDCLIP *ch) {
	SoundClipWaveBase *wavClip = dynamic_cast<SoundClipWaveBase *>(ch);
	if (wavClip) {
		switch (index) {
		case SCHAN_SPEECH:
			wavClip->setType(Audio::Mixer::kSpeechSoundType);
			break;
		case SCHAN_MUSIC:
			wavClip->setType(Audio::Mixer::kMusicSoundType);
			break;
		default:
			wavClip->setType(Audio::Mixer::kSFXSoundType);
			break;
		}
	}

	// TODO: store clips in smart pointers
	if (_GP(audioChannels)[index] != nullptr && _GP(audioChannels)[index] == ch)
		Debug::Printf(kDbgMsg_Warn, "WARNING: channel %d - same clip assigned", index);
	else if (_GP(audioChannels)[index] != nullptr && ch != nullptr)
		Debug::Printf(kDbgMsg_Warn, "WARNING: channel %d - clip overwritten", index);
	_GP(audioChannels)[index] = ch;
	return ch;
}

SOUNDCLIP *AudioChans::MoveChannel(int to, int from) {
	auto from_ch = _GP(audioChannels)[from];
	_GP(audioChannels)[from] = nullptr;
	return SetChannel(to, from_ch);
}

void calculate_reserved_channel_count() {
	int reservedChannels = 0;
	for (size_t i = 0; i < _GP(game).audioClipTypes.size(); i++) {
		reservedChannels += _GP(game).audioClipTypes[i].reservedChannels;
	}
	_G(reserved_channel_count) = reservedChannels;
}

void update_clip_default_volume(ScriptAudioClip *audioClip) {
	if (_GP(play).default_audio_type_volumes[audioClip->type] >= 0) {
		audioClip->defaultVolume = _GP(play).default_audio_type_volumes[audioClip->type];
	}
}

void start_fading_in_new_track_if_applicable(int fadeInChannel, ScriptAudioClip *newSound) {
	int crossfadeSpeed = _GP(game).audioClipTypes[newSound->type].crossfadeSpeed;
	if (crossfadeSpeed > 0) {
		update_clip_default_volume(newSound);
		_GP(play).crossfade_in_volume_per_step = crossfadeSpeed;
		_GP(play).crossfade_final_volume_in = newSound->defaultVolume;
		_GP(play).crossfading_in_channel = fadeInChannel;
	}
}

static void move_track_to_crossfade_channel(int currentChannel, int crossfadeSpeed, int fadeInChannel, ScriptAudioClip *newSound) {
	stop_and_destroy_channel(SPECIAL_CROSSFADE_CHANNEL);
	auto *cfade_clip = AudioChans::MoveChannel(SPECIAL_CROSSFADE_CHANNEL, currentChannel);
	if (!cfade_clip)
		return;

	_GP(play).crossfading_out_channel = SPECIAL_CROSSFADE_CHANNEL;
	_GP(play).crossfade_step = 0;
	_GP(play).crossfade_initial_volume_out = cfade_clip->get_volume100();
	_GP(play).crossfade_out_volume_per_step = crossfadeSpeed;

	_GP(play).crossfading_in_channel = fadeInChannel;
	if (newSound != nullptr) {
		start_fading_in_new_track_if_applicable(fadeInChannel, newSound);
	}
}

// NOTE: this function assumes one of the user channels
void stop_or_fade_out_channel(int fadeOutChannel, int fadeInChannel, ScriptAudioClip *newSound) {
	ScriptAudioClip *sourceClip = AudioChannel_GetPlayingClip(&_G(scrAudioChannel)[fadeOutChannel]);
	if ((_GP(play).fast_forward == 0) && // don't crossfade if skipping a cutscene
		(sourceClip != nullptr) && (_GP(game).audioClipTypes[sourceClip->type].crossfadeSpeed > 0)) {
		move_track_to_crossfade_channel(fadeOutChannel, _GP(game).audioClipTypes[sourceClip->type].crossfadeSpeed, fadeInChannel, newSound);
	} else {
		stop_and_destroy_channel(fadeOutChannel);
	}
}

static int find_free_audio_channel(ScriptAudioClip *clip, int priority, bool interruptEqualPriority, bool for_queue = true) {
	int lowestPrioritySoFar = 9999999;
	int lowestPriorityID = -1;
	int channelToUse = -1;

	if (!interruptEqualPriority)
		priority--;

	int startAtChannel = _G(reserved_channel_count);
	int endBeforeChannel = _GP(game).numGameChannels;

	if (_GP(game).audioClipTypes[clip->type].reservedChannels > 0) {
		startAtChannel = 0;
		for (int i = 0; i < clip->type; i++) {
			startAtChannel += MIN(MAX_SOUND_CHANNELS,
				_GP(game).audioClipTypes[i].reservedChannels);
		}
		// NOTE: we allow to place sound on a crossfade channel for backward compatibility,
		// but ONLY for the case of audio type with reserved channels (weird quirk).
		endBeforeChannel = MIN(_GP(game).numCompatGameChannels,
			startAtChannel + _GP(game).audioClipTypes[clip->type].reservedChannels);
	}

	for (int i = startAtChannel; i < endBeforeChannel; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (ch == nullptr) {
			channelToUse = i;
			stop_and_destroy_channel(i);
			break;
		}
		if ((ch->_priority < lowestPrioritySoFar) &&
		        (ch->_sourceClipType == clip->type)) {
			lowestPrioritySoFar = ch->_priority;
			lowestPriorityID = i;
		}
		// NOTE: This is a "hack" for starting queued clips;
		// since having a new audio system (3.6.0 onwards), the audio timing
		// changed a little, and queued sounds have to start bit earlier
		// if we want them to sound seamless with the previous clips.
		// TODO: investigate better solutions? may require reimplementation of the sound queue.
		if (for_queue && (ch->_sourceClipType == clip->type)) {
			// try to start queued sounds 1 frame earlier
			const float trigger_pos = (1000.f / _G(frames_per_second)) * 1.f;
			if (ch->get_pos_ms() >= (ch->get_length_ms() - trigger_pos)) {
				lowestPrioritySoFar = priority;
				lowestPriorityID = i;
			}
		}
	}

	if ((channelToUse < 0) && (lowestPriorityID >= 0) &&
	        (lowestPrioritySoFar <= priority)) {
		stop_or_fade_out_channel(lowestPriorityID, lowestPriorityID, clip);
		channelToUse = lowestPriorityID;
	} else if ((channelToUse >= 0) && (_GP(play).crossfading_in_channel < 1)) {
		start_fading_in_new_track_if_applicable(channelToUse, clip);
	}
	return channelToUse;
}

bool is_audiotype_allowed_to_play(AudioFileType /*type*/) {
	// TODO: this is a remnant of an old audio logic, think this function over
	return _GP(usetup).audio_enabled;
}

SOUNDCLIP *load_sound_clip(ScriptAudioClip *audioClip, bool repeat) {
	if (!is_audiotype_allowed_to_play((AudioFileType)audioClip->fileType)) {
		return nullptr;
	}

	update_clip_default_volume(audioClip);

	SOUNDCLIP *soundClip = nullptr;
	AssetPath asset_name = get_audio_clip_assetpath(audioClip->bundlingType, audioClip->fileName);
	switch (audioClip->fileType) {
	case eAudioFileOGG:
		soundClip = my_load_static_ogg(asset_name, repeat);
		break;
	case eAudioFileMP3:
		soundClip = my_load_static_mp3(asset_name, repeat);
		break;
	case eAudioFileWAV:
	case eAudioFileVOC:
		soundClip = my_load_wave(asset_name, repeat);
		break;
	case eAudioFileMIDI:
		soundClip = my_load_midi(asset_name, repeat);
		break;
	case eAudioFileMOD:
		soundClip = my_load_mod(asset_name, repeat);
		break;
	default:
		quitprintf("AudioClip.Play: invalid audio file type encountered: %d", audioClip->fileType);
	}
	if (soundClip != nullptr) {
		soundClip->set_volume100(audioClip->defaultVolume);
		soundClip->_sourceClipID = audioClip->id;
		soundClip->_sourceClipType = audioClip->type;
	}
	return soundClip;
}

static void audio_update_polled_stuff() {
	///////////////////////////////////////////////////////////////////////////
	// Do crossfade
	_GP(play).crossfade_step++;

	if (_GP(play).crossfading_out_channel > 0 && !AudioChans::GetChannelIfPlaying(_GP(play).crossfading_out_channel))
		_GP(play).crossfading_out_channel = 0;

	if (_GP(play).crossfading_out_channel > 0) {
		SOUNDCLIP *ch = AudioChans::GetChannel(_GP(play).crossfading_out_channel);
		int newVolume = ch ? ch->get_volume100() - _GP(play).crossfade_out_volume_per_step : 0;
		if (newVolume > 0) {
			ch->set_volume100(newVolume);
		} else {
			stop_and_destroy_channel(_GP(play).crossfading_out_channel);
			_GP(play).crossfading_out_channel = 0;
		}
	}

	if (_GP(play).crossfading_in_channel > 0 && !AudioChans::GetChannelIfPlaying(_GP(play).crossfading_in_channel))
		_GP(play).crossfading_in_channel = 0;

	if (_GP(play).crossfading_in_channel > 0) {
		SOUNDCLIP *ch = AudioChans::GetChannel(_GP(play).crossfading_in_channel);
		int newVolume = ch ? ch->get_volume100() + _GP(play).crossfade_in_volume_per_step : 0;
		if (newVolume > _GP(play).crossfade_final_volume_in) {
			newVolume = _GP(play).crossfade_final_volume_in;
		}

		ch->set_volume100(newVolume);

		if (newVolume >= _GP(play).crossfade_final_volume_in) {
			_GP(play).crossfading_in_channel = 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Do audio queue
	if (_GP(play).new_music_queue_size > 0) {
		for (int i = 0; i < _GP(play).new_music_queue_size; i++) {
			ScriptAudioClip *clip = &_GP(game).audioClips[_GP(play).new_music_queue[i].audioClipIndex];
			int channel = find_free_audio_channel(clip, clip->defaultPriority, false, true);
			if (channel >= 0) {
				QueuedAudioItem itemToPlay = _GP(play).new_music_queue[i];

				_GP(play).new_music_queue_size--;
				for (int j = i; j < _GP(play).new_music_queue_size; j++) {
					_GP(play).new_music_queue[j] = _GP(play).new_music_queue[j + 1];
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
	if (_GP(play).IsNonBlockingVoiceSpeech()) {
		if (!AudioChans::ChannelIsPlaying(SCHAN_SPEECH)) {
			stop_voice_nonblocking();
		}
	}
}

// Applies a volume drop modifier to the clip, in accordance to its audio type
static void apply_volume_drop_to_clip(SOUNDCLIP *clip) {
	int audiotype = clip->_sourceClipType;
	clip->apply_volume_modifier(-(_GP(game).audioClipTypes[audiotype].volume_reduction_while_speech_playing * 255 / 100));
}

static void queue_audio_clip_to_play(ScriptAudioClip *clip, int priority, int repeat) {
	if (_GP(play).new_music_queue_size >= MAX_QUEUED_MUSIC) {
		debug_script_log("Too many queued music, cannot add %s", clip->scriptName.GetCStr());
		return;
	}

	SOUNDCLIP *cachedClip = load_sound_clip(clip, (repeat != 0));
	if (cachedClip != nullptr) {
		_GP(play).new_music_queue[_GP(play).new_music_queue_size].audioClipIndex = clip->id;
		_GP(play).new_music_queue[_GP(play).new_music_queue_size].priority = priority;
		_GP(play).new_music_queue[_GP(play).new_music_queue_size].repeat = (repeat != 0);
		_GP(play).new_music_queue[_GP(play).new_music_queue_size].cachedClip = cachedClip;
		_GP(play).new_music_queue_size++;
	}
}

ScriptAudioChannel *play_audio_clip_on_channel(int channel, ScriptAudioClip *clip, int priority, int repeat, int fromOffset, SOUNDCLIP *soundfx) {
	if (soundfx == nullptr) {
		soundfx = load_sound_clip(clip, (repeat) ? true : false);
	}
	if (soundfx == nullptr) {
		debug_script_log("AudioClip.Play: unable to load sound file");
		if (_GP(play).crossfading_in_channel == channel) {
			_GP(play).crossfading_in_channel = 0;
		}
		return nullptr;
	}
	soundfx->_priority = priority;

	if (_GP(play).crossfading_in_channel == channel) {
		soundfx->set_volume100(0);
	}

	// Mute the audio clip if fast-forwarding the cutscene
	if (_GP(play).fast_forward) {
		soundfx->set_mute(true);

		// CHECKME!!
		// [IKM] According to the 3.2.1 logic the clip will restore
		// its value after cutscene, but only if originalVolAsPercentage
		// is not zeroed. Something I am not sure about: why does it
		// disable the clip under condition that there's more than one
		// channel for this audio type? It does not even check if
		// anything of this type is currently playing.
		if (_GP(game).audioClipTypes[clip->type].reservedChannels != 1)
			soundfx->set_volume100(0);
	}

	if (soundfx->play_from(fromOffset) == 0) {
		// not assigned to a channel, so clean up manually.
		delete soundfx;
		soundfx = nullptr;
		debug_script_log("AudioClip.Play: failed to play sound file");
		return nullptr;
	}

	// Apply volume drop if any speech voice-over is currently playing
	// NOTE: there is a confusing logic in sound clip classes, that they do not use
	// any modifiers when begin playing, therefore we must apply this only after
	// playback was started.
	if (!_GP(play).fast_forward && _GP(play).speech_has_voice)
		apply_volume_drop_to_clip(soundfx);

	AudioChans::SetChannel(channel, soundfx);
	return &_G(scrAudioChannel)[channel];
}

void remove_clips_of_type_from_queue(int audioType) {
	int aa;
	for (aa = 0; aa < _GP(play).new_music_queue_size; aa++) {
		ScriptAudioClip *clip = &_GP(game).audioClips[_GP(play).new_music_queue[aa].audioClipIndex];
		if ((audioType == SCR_NO_VALUE) || (clip->type == audioType)) {
			_GP(play).new_music_queue_size--;
			for (int bb = aa; bb < _GP(play).new_music_queue_size; bb++)
				_GP(play).new_music_queue[bb] = _GP(play).new_music_queue[bb + 1];
			aa--;
		}
	}
}

void update_queued_clips_volume(int audioType, int new_vol) {
	for (int i = 0; i < _GP(play).new_music_queue_size; ++i) {
		// NOTE: if clip is uncached, the volume will be set from defaults when it is loaded
		SOUNDCLIP *sndclip = _GP(play).new_music_queue[i].cachedClip;
		if (sndclip) {
			ScriptAudioClip *clip = &_GP(game).audioClips[_GP(play).new_music_queue[i].audioClipIndex];
			if (clip->type == audioType)
				sndclip->set_volume100(new_vol);
		}
	}
}

ScriptAudioChannel *play_audio_clip(ScriptAudioClip *clip, int priority, int repeat, int fromOffset, bool queueIfNoChannel) {
	if (!queueIfNoChannel)
		remove_clips_of_type_from_queue(clip->type);

	if (priority == SCR_NO_VALUE)
		priority = clip->defaultPriority;
	if (repeat == SCR_NO_VALUE)
		repeat = clip->defaultRepeat;

	int channel = find_free_audio_channel(clip, priority, !queueIfNoChannel, queueIfNoChannel);
	if (channel < 0) {
		if (queueIfNoChannel)
			queue_audio_clip_to_play(clip, priority, repeat);
		else
			debug_script_log("AudioClip.Play: no channels available to interrupt PRI:%d TYPE:%d", priority, clip->type);

		return nullptr;
	}

	return play_audio_clip_on_channel(channel, clip, priority, repeat, fromOffset);
}

ScriptAudioChannel *play_audio_clip_by_index(int audioClipIndex) {
	if ((audioClipIndex >= 0) && ((size_t)audioClipIndex < _GP(game).audioClips.size()))
		return AudioClip_Play(&_GP(game).audioClips[audioClipIndex], SCR_NO_VALUE, SCR_NO_VALUE);
	else
		return nullptr;
}

void stop_and_destroy_channel_ex(int chid, bool resetLegacyMusicSettings) {
	if ((chid < 0) || (chid >= TOTAL_AUDIO_CHANNELS))
		quit("!StopChannel: invalid channel ID");

	SOUNDCLIP *ch = AudioChans::GetChannel(chid);

	if (ch != nullptr) {
		delete ch;
		AudioChans::SetChannel(chid, nullptr);
		ch = nullptr;
	}

	if (_GP(play).crossfading_in_channel == chid)
		_GP(play).crossfading_in_channel = 0;
	if (_GP(play).crossfading_out_channel == chid)
		_GP(play).crossfading_out_channel = 0;
	// don't update '_G(crossFading)' here as it is updated in all the cross-fading functions.

	// destroyed an ambient sound channel
	if (chid < _GP(game).numGameChannels) {
		if (_GP(ambient)[chid].channel > 0)
			_GP(ambient)[chid].channel = 0;
	}

	if ((chid == SCHAN_MUSIC) && (resetLegacyMusicSettings)) {
		_GP(play).cur_music_number = -1;
		_G(current_music_type) = 0;
	}
}

void stop_and_destroy_channel(int chid) {
	stop_and_destroy_channel_ex(chid, true);
}

void export_missing_audiochans() {
	for (int i = 0; i < _GP(game).numCompatGameChannels; ++i) {
		int h = ccGetObjectHandleFromAddress(&_G(scrAudioChannel)[i]);
		if (h <= 0)
			ccRegisterManagedObject(&_G(scrAudioChannel)[i], &_GP(ccDynamicAudio));
	}
}


// ***** BACKWARDS COMPATIBILITY WITH OLD AUDIO SYSTEM ***** //

int get_old_style_number_for_sound(int sound_number) {
	// In the legacy audio system treat sound_number as an old style number
	if (_GP(game).IsLegacyAudioSystem()) {
		return sound_number;
	}

	// Treat sound number as a real clip index
	if (sound_number >= 0) {
		int old_style_number = 0;
		if (sscanf(_GP(game).audioClips[sound_number].scriptName.GetCStr(), "aSound%d", &old_style_number) == 1)
			return old_style_number;
	}

	return 0;
}

SOUNDCLIP *load_sound_clip_from_old_style_number(bool isMusic, int indexNumber, bool repeat) {
	ScriptAudioClip *audioClip = GetAudioClipForOldStyleNumber(_GP(game), isMusic, indexNumber);

	if (audioClip != nullptr) {
		return load_sound_clip(audioClip, repeat);
	}

	return nullptr;
}

//=============================================================================

int get_volume_adjusted_for_distance(int volume, int sndX, int sndY, int sndMaxDist) {
	int distx = _G(playerchar)->x - sndX;
	int disty = _G(playerchar)->y - sndY;
	// it uses Allegro's "fix" sqrt without the ::
	int dist = (int)::sqrt((double)(distx * distx + disty * disty));

	// if they're quite close, full volume
	int wantvol = volume;

	if (dist >= AMBIENCE_FULL_DIST) {
		// get the relative volume
		wantvol = ((dist - AMBIENCE_FULL_DIST) * volume) / sndMaxDist;
		// closer is louder
		wantvol = volume - wantvol;
	}

	return wantvol;
}

void update_directional_sound_vol() {
	for (int chnum = NUM_SPEECH_CHANS; chnum < _GP(game).numGameChannels; chnum++) {
		auto *ch = AudioChans::GetChannelIfPlaying(chnum);
		if ((ch != nullptr) && (ch->_xSource >= 0)) {
			ch->apply_directional_modifier(
				get_volume_adjusted_for_distance(ch->get_volume255(),
					ch->_xSource,
					ch->_ySource,
					ch->_maximumPossibleDistanceAway) -
				ch->get_volume255());
		}
	}
}

void update_ambient_sound_vol() {
	for (int chan = NUM_SPEECH_CHANS; chan < _GP(game).numGameChannels; chan++) {
		AmbientSound *thisSound = &_GP(ambient)[chan];

		if (thisSound->channel == 0)
			continue;

		int sourceVolume = thisSound->vol;

		if (_GP(play).speech_has_voice) {
			// Negative value means set exactly; positive means drop that amount
			if (_GP(play).speech_music_drop < 0)
				sourceVolume = -_GP(play).speech_music_drop;
			else
				sourceVolume -= _GP(play).speech_music_drop;

			if (sourceVolume < 0)
				sourceVolume = 0;
			if (sourceVolume > 255)
				sourceVolume = 255;
		}

		// Adjust ambient volume so it maxes out at overall sound volume
		int ambientvol = (sourceVolume * _GP(play).sound_volume) / 255;

		int wantvol;

		if ((thisSound->x == 0) && (thisSound->y == 0)) {
			wantvol = ambientvol;
		} else {
			wantvol = get_volume_adjusted_for_distance(ambientvol, thisSound->x, thisSound->y, thisSound->maxdist);
		}

		auto *ch = AudioChans::GetChannelIfPlaying(thisSound->channel);
		if (ch)
			ch->set_volume255(wantvol);
	}
}

SOUNDCLIP *load_sound_and_play(ScriptAudioClip *aclip, bool repeat) {
	SOUNDCLIP *soundfx = load_sound_clip(aclip, repeat);
	if (!soundfx) {
		return nullptr;
	}

	if (soundfx->play() == 0) {
		// not assigned to a channel, so clean up manually.
		delete soundfx;
		return nullptr;
	}

	return soundfx;
}

void stop_all_sound_and_music() {
	stopmusic();
	stop_voice_nonblocking();
	// make sure it doesn't start crossfading when it comes back
	_G(crossFading) = 0;
	// any ambient sound will be aborted
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; ++i)
		stop_and_destroy_channel(i);
}

void shutdown_sound() {
	stop_all_sound_and_music(); // game logic
#if !AGS_PLATFORM_SCUMMVM
	audio_core_shutdown(); // audio core system
#endif
	sys_audio_shutdown(); // backend
	_GP(usetup).audio_enabled = false;
}

// the sound will only be played if there is a free channel or
// it has a priority >= an existing sound to override
static int play_sound_priority(int val1, int priority) {
	int lowest_pri = 9999, lowest_pri_id = -1;

	// find a free channel to play it on
	for (int i = SCHAN_NORMAL; i < _GP(game).numGameChannels; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (val1 < 0) {
			// Playing sound -1 means iterate through and stop all sound
			if (ch)
				stop_and_destroy_channel(i);
		} else if (ch == nullptr || !ch->is_playing()) {
			// PlaySoundEx will destroy the previous channel value.
			const int usechan = PlaySoundEx(val1, i);
			if (usechan >= 0) {
				// channel will hold a different clip here
				assert(usechan == i);
				auto *chan = AudioChans::GetChannel(usechan);
				if (chan)
					chan->_priority = priority;
			}
			return usechan;
		} else if (ch->_priority < lowest_pri) {
			lowest_pri = ch->_priority;
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
			auto *ch = AudioChans::GetChannel(usechan);
			if (ch)
				ch->_priority = priority;
			return usechan;
		}
	}

	return -1;
}

int play_sound(int val1) {
	return play_sound_priority(val1, 10);
}

//=============================================================================

void cancel_scheduled_music_update() {
	_G(music_update_scheduled) = false;
}

void schedule_music_update_at(AGS_Clock::time_point at) {
	_G(music_update_scheduled) = true;
	_G(music_update_at) = at;
}

void postpone_scheduled_music_update_by(std::chrono::milliseconds duration) {
	if (!_G(music_update_scheduled)) {
		return;
	}
	_G(music_update_at) += duration;
}

void process_scheduled_music_update() {
	if (!_G(music_update_scheduled)) {
		return;
	}
	if (_G(music_update_at) > AGS_Clock::now()) {
		return;
	}
	cancel_scheduled_music_update();
	update_music_volume();
	apply_volume_drop_modifier(false);
	update_ambient_sound_vol();
}
// end scheduled music update functions
//=============================================================================

void clear_music_cache() {

	if (_G(cachedQueuedMusic) != nullptr) {
		delete _G(cachedQueuedMusic);
		_G(cachedQueuedMusic) = nullptr;
	}

}

static void play_new_music(int mnum, SOUNDCLIP *music);

void play_next_queued() {
	// check if there's a queued one to play
	if (_GP(play).music_queue_size > 0) {

		int tuneToPlay = _GP(play).music_queue[0];

		if (tuneToPlay >= QUEUED_MUSIC_REPEAT) {
			// Loop it!
			_GP(play).music_repeat++;
			play_new_music(tuneToPlay - QUEUED_MUSIC_REPEAT, _G(cachedQueuedMusic));
			_GP(play).music_repeat--;
		} else {
			// Don't loop it!
			int repeatWas = _GP(play).music_repeat;
			_GP(play).music_repeat = 0;
			play_new_music(tuneToPlay, _G(cachedQueuedMusic));
			_GP(play).music_repeat = repeatWas;
		}

		// don't free the memory, as it has been transferred onto the
		// main music channel
		_G(cachedQueuedMusic) = nullptr;

		_GP(play).music_queue_size--;
		for (int i = 0; i < _GP(play).music_queue_size; i++)
			_GP(play).music_queue[i] = _GP(play).music_queue[i + 1];

		if (_GP(play).music_queue_size > 0)
			_G(cachedQueuedMusic) = load_music_from_disk(_GP(play).music_queue[0], 0);
	}

}

int calculate_max_volume() {
	// quieter so that sounds can be heard better
	int newvol = _GP(play).music_master_volume + ((int)_GP(thisroom).Options.MusicVolume) * LegacyRoomVolumeFactor;
	if (newvol > 255) newvol = 255;
	if (newvol < 0) newvol = 0;

	if (_GP(play).fast_forward)
		newvol = 0;

	return newvol;
}

// add/remove the volume drop to the audio channels while speech is playing
void apply_volume_drop_modifier(bool applyModifier) {
	for (int i = NUM_SPEECH_CHANS; i < _GP(game).numGameChannels; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (ch && ch->_sourceClipID >= 0) {
			if (applyModifier)
				apply_volume_drop_to_clip(ch);
			else
				ch->apply_volume_modifier(0); // reset modifier
		}
	}
}

// Checks if speech voice-over is currently playing, and reapply volume drop to all other active clips
void update_volume_drop_if_voiceover() {
	apply_volume_drop_modifier(_GP(play).speech_has_voice);
}

// Sync logical game channels with the audio backend:
// startup new assigned clips, apply changed parameters.
void sync_audio_playback() {
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; ++i) {
		// update the playing channels, and dispose the finished / invalid ones
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (ch && !ch->update()) {
			AudioChans::SetChannel(i, nullptr);
			delete ch;
		}
	}
}

// Update the music, and advance the crossfade on a step
// (this should only be called once per game loop)
void update_audio_system_on_game_loop() {
	update_polled_stuff();

	// Sync logical game channels with the audio backend
	// NOTE: we update twice, first time here - because we need to know
	// which clips are still playing before updating the sound transitions
	// and queues, then second time later - because we need to apply any
	// changes to channels / parameters.
	// TODO: investigate options for optimizing this.
	sync_audio_playback();

	process_scheduled_music_update();

	audio_update_polled_stuff();

	if (_G(crossFading)) {
		_G(crossFadeStep)++;
		update_music_volume();
	}

	// Check if the current music has finished playing
	if ((_GP(play).cur_music_number >= 0) && (_GP(play).fast_forward == 0)) {
		if (IsMusicPlaying() == 0) {
			// The current music has finished
			_GP(play).cur_music_number = -1;
			play_next_queued();
		} else if ((_GP(game).options[OPT_CROSSFADEMUSIC] > 0) &&
		           (_GP(play).music_queue_size > 0) && (!_G(crossFading))) {
			// want to crossfade, and new tune in the queue
			auto *ch = AudioChans::GetChannel(SCHAN_MUSIC);
			if (ch) {
				int curpos = ch->get_pos_ms();
				int muslen = ch->get_length_ms();
				if ((curpos > 0) && (muslen > 0)) {
					// we want to crossfade, and we know how far through
					// the tune we are
					int takesSteps = calculate_max_volume() / _GP(game).options[OPT_CROSSFADEMUSIC];
					int takesMs = ::lround(takesSteps * 1000.0f / get_game_fps());
					if (curpos >= muslen - takesMs)
						play_next_queued();
				}
			}
		}
	}

	if (_G(loopcounter) % 5 == 0) { // TODO: investigate why we do this each 5 frames?
		update_ambient_sound_vol();
		update_directional_sound_vol();
	}

	// Sync logical game channels with the audio backend again
	sync_audio_playback();
}

void stopmusic() {
	if (_G(crossFading) > 0) {
		// stop in the middle of a new track fading in
		// Abort the new track, and let the old one finish fading out
		stop_and_destroy_channel(_G(crossFading));
		_G(crossFading) = -1;
	} else if (_G(crossFading) < 0) {
		// the music is already fading out
		if (_GP(game).options[OPT_CROSSFADEMUSIC] <= 0) {
			// If they have since disabled crossfading, stop the fadeout
			stop_and_destroy_channel(SCHAN_MUSIC);
			_G(crossFading) = 0;
			_G(crossFadeStep) = 0;
			update_music_volume();
		}
	} else if ((_GP(game).options[OPT_CROSSFADEMUSIC] > 0)
	           && (AudioChans::GetChannelIfPlaying(SCHAN_MUSIC) != nullptr)
	           && (_G(current_music_type) != 0)
	           && (_G(current_music_type) != MUS_MIDI)
	           && (_G(current_music_type) != MUS_MOD)) {

		_G(crossFading) = -1;
		_G(crossFadeStep) = 0;
		_G(crossFadeVolumePerStep) = _GP(game).options[OPT_CROSSFADEMUSIC];
		_G(crossFadeVolumeAtStart) = calculate_max_volume();
	} else
		stop_and_destroy_channel(SCHAN_MUSIC);

	_GP(play).cur_music_number = -1;
	_G(current_music_type) = 0;
}

void update_music_volume() {
	if ((_G(current_music_type)) || (_G(crossFading) < 0)) {
		// targetVol is the maximum volume we're fading in to
		// newvol is the starting volume that we faded out from
		int targetVol = calculate_max_volume();
		int newvol;
		if (_G(crossFading))
			newvol = _G(crossFadeVolumeAtStart);
		else
			newvol = targetVol;

		// fading out old track, target volume is silence
		if (_G(crossFading) < 0)
			targetVol = 0;

		if (_G(crossFading)) {
			int curvol = _G(crossFadeVolumePerStep) * _G(crossFadeStep);

			if ((curvol > targetVol) && (curvol > newvol)) {
				// it has fully faded to the new track
				newvol = targetVol;
				stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
				if (_G(crossFading) > 0) {
					AudioChans::MoveChannel(SCHAN_MUSIC, _G(crossFading));
				}
				_G(crossFading) = 0;
			} else {
				if (_G(crossFading) > 0) {
					auto *ch = AudioChans::GetChannel(_G(crossFading));
					if (ch)
						ch->set_volume255((curvol > targetVol) ? targetVol : curvol);
				}

				newvol -= curvol;
				if (newvol < 0)
					newvol = 0;
			}
		}
		auto *ch = AudioChans::GetChannel(SCHAN_MUSIC);
		if (ch)
			ch->set_volume255(newvol);
	}
}

// Ensures crossfader is stable after loading (or failing to load) new music
// NOTE: part of the legacy audio logic
void post_new_music_check() {
	if ((_G(crossFading) > 0) && (AudioChans::GetChannel(_G(crossFading)) == nullptr)) {
		_G(crossFading) = 0;
		// Was fading out but then they played invalid music, continue to fade out
		if (AudioChans::GetChannel(SCHAN_MUSIC) != nullptr)
			_G(crossFading) = -1;
	}
}

int prepare_for_new_music() {
	int useChannel = SCHAN_MUSIC;

	if ((_GP(game).options[OPT_CROSSFADEMUSIC] > 0)
	        && (AudioChans::GetChannelIfPlaying(SCHAN_MUSIC) != nullptr)
	        && (_G(current_music_type) != MUS_MIDI)
	        && (_G(current_music_type) != MUS_MOD)) {

		if (_G(crossFading) > 0) {
			// It's still crossfading to the previous track
			stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
			AudioChans::MoveChannel(SCHAN_MUSIC, _G(crossFading));
			_G(crossFading) = 0;
			update_music_volume();
		} else if (_G(crossFading) < 0) {
			// an old track is still fading out, no new music yet
			// Do nothing, and keep the current crossfade step
		} else {
			// start crossfading
			_G(crossFadeStep) = 0;
			_G(crossFadeVolumePerStep) = _GP(game).options[OPT_CROSSFADEMUSIC];
			_G(crossFadeVolumeAtStart) = calculate_max_volume();
		}
		useChannel = SPECIAL_CROSSFADE_CHANNEL;
		_G(crossFading) = useChannel;
	} else {
		// crossfading is now turned off
		stopmusic();
		// ensure that any traces of old tunes fading are eliminated
		// (otherwise the new track will be faded out)
		_G(crossFading) = 0;
	}

	// Just make sure, because it will be overwritten in a sec
	if (AudioChans::GetChannel(useChannel) != nullptr)
		stop_and_destroy_channel(useChannel);

	return useChannel;
}

ScriptAudioClip *get_audio_clip_for_music(int mnum) {
	if (mnum >= QUEUED_MUSIC_REPEAT)
		mnum -= QUEUED_MUSIC_REPEAT;
	return GetAudioClipForOldStyleNumber(_GP(game), true, mnum);
}

SOUNDCLIP *load_music_from_disk(int mnum, bool doRepeat) {
	if (mnum >= QUEUED_MUSIC_REPEAT) {
		mnum -= QUEUED_MUSIC_REPEAT;
		doRepeat = true;
	}

	SOUNDCLIP *loaded = load_sound_clip_from_old_style_number(true, mnum, doRepeat);

	if ((loaded == nullptr) && (mnum > 0)) {
		debug_script_warn("Music %d not found", mnum);
		debug_script_log("FAILED to load music %d", mnum);
	}

	return loaded;
}

static void play_new_music(int mnum, SOUNDCLIP *music) {
	if (_G(debug_flags) & DBG_NOMUSIC)
		return;

	if ((_GP(play).cur_music_number == mnum) && (music == nullptr)) {
		debug_script_log("PlayMusic %d but already playing", mnum);
		return;  // don't play the music if it's already playing
	}

	ScriptAudioClip *aclip = get_audio_clip_for_music(mnum);
	if (aclip && !is_audiotype_allowed_to_play((AudioFileType)aclip->fileType))
		return;

	int useChannel = SCHAN_MUSIC;
	debug_script_log("Playing music %d", mnum);

	if (mnum < 0) {
		stopmusic();
		return;
	}

	if (_GP(play).fast_forward) {
		// while skipping cutscene, don't change the music
		_GP(play).end_cutscene_music = mnum;
		return;
	}

	useChannel = prepare_for_new_music();
	_GP(play).cur_music_number = mnum;
	_G(current_music_type) = 0;

	_GP(play).current_music_repeating = _GP(play).music_repeat;
	// now that all the previous music is unloaded, load in the new one

	SOUNDCLIP *new_clip;
	if (music != nullptr)
		new_clip = music;
	else
		new_clip = load_music_from_disk(mnum, (_GP(play).music_repeat > 0));

	auto *ch = AudioChans::SetChannel(useChannel, new_clip);
	if (ch != nullptr) {
		if (!ch->play()) {
			// previous behavior was to set channel[] to null on error, so continue to do that here.
			delete ch;
			ch = nullptr;
			AudioChans::SetChannel(useChannel, nullptr);
		} else
			_G(current_music_type) = ch->get_sound_type();
	}

	post_new_music_check();
	update_music_volume();
}

void newmusic(int mnum) {
	play_new_music(mnum, nullptr);
}

} // namespace AGS3
