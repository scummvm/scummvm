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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/util/string_compat.h"

namespace AGS3 {

using namespace AGS::Shared;

void StopAmbientSound(int channel) {
	if ((channel < NUM_SPEECH_CHANS) || (channel >= _GP(game).numGameChannels))
		quitprintf("!StopAmbientSound: invalid channel %d, supported %d - %d",
			channel, NUM_SPEECH_CHANS, _GP(game).numGameChannels - 1);

	if (_GP(ambient)[channel].channel == 0)
		return;

	stop_and_destroy_channel(channel);
	_GP(ambient)[channel].channel = 0;
}

void PlayAmbientSound(int channel, int sndnum, int vol, int x, int y) {
	// the channel parameter is to allow multiple ambient sounds in future
	if ((channel < 1) || (channel == SCHAN_SPEECH) || (channel >= _GP(game).numGameChannels))
		quit("!PlayAmbientSound: invalid channel number");
	if ((vol < 1) || (vol > 255))
		quit("!PlayAmbientSound: volume must be 1 to 255");

	ScriptAudioClip *aclip = GetAudioClipForOldStyleNumber(_GP(game), false, sndnum);
	if (aclip && !is_audiotype_allowed_to_play((AudioFileType)aclip->fileType))
		return;

	// only play the sound if it's not already playing
	if ((_GP(ambient)[channel].channel < 1) || (!AudioChans::ChannelIsPlaying(_GP(ambient)[channel].channel)) ||
			(_GP(ambient)[channel].num != sndnum)) {

		StopAmbientSound(channel);
		// in case a normal non-ambient sound was playing, stop it too
		stop_and_destroy_channel(channel);

		SOUNDCLIP *asound = aclip ? load_sound_and_play(aclip, true) : nullptr;
		if (asound == nullptr) {
			debug_script_warn("Cannot load ambient sound %d", sndnum);
			debug_script_log("FAILED to load ambient sound %d", sndnum);
			return;
		}

		debug_script_log("Playing ambient sound %d on channel %d", sndnum, channel);
		_GP(ambient)[channel].channel = channel;
		asound->_priority = 15;  // ambient sound higher priority than normal sfx
		AudioChans::SetChannel(channel, asound);
	}
	// calculate the maximum distance away the player can be, using X
	// only (since X centred is still more-or-less total Y)
	_GP(ambient)[channel].maxdist = ((x > _GP(thisroom).Width / 2) ? x : (_GP(thisroom).Width - x)) - AMBIENCE_FULL_DIST;
	_GP(ambient)[channel].num = sndnum;
	_GP(ambient)[channel].x = x;
	_GP(ambient)[channel].y = y;
	_GP(ambient)[channel].vol = vol;
	update_ambient_sound_vol();
}

int IsChannelPlaying(int chan) {
	if (_GP(play).fast_forward)
		return 0;

	if ((chan < 0) || (chan >= _GP(game).numGameChannels))
		quit("!IsChannelPlaying: invalid sound channel");

	if (AudioChans::ChannelIsPlaying(chan))
		return 1;

	return 0;
}

int IsSoundPlaying() {
	if (_GP(play).fast_forward)
		return 0;

	// find if there's a sound playing
	for (int i = SCHAN_NORMAL; i < _GP(game).numGameChannels; i++) {
		if (AudioChans::GetChannelIfPlaying(i))
			return 1;
	}

	return 0;
}

// returns -1 on failure, channel number on success
int PlaySoundEx(int val1, int channel) {

	if (_G(debug_flags) & DBG_NOSFX)
		return -1;

	ScriptAudioClip *aclip = GetAudioClipForOldStyleNumber(_GP(game), false, val1);
	if (aclip && !is_audiotype_allowed_to_play((AudioFileType)aclip->fileType))
		return -1; // if sound is off, ignore it

	if ((channel < SCHAN_NORMAL) || (channel >= _GP(game).numGameChannels))
		quitprintf("!PlaySoundEx: invalid channel specified, must be %d-%d", SCHAN_NORMAL, _GP(game).numGameChannels - 1);

	// if an ambient sound is playing on this channel, abort it
	StopAmbientSound(channel);

	if (val1 < 0) {
		stop_and_destroy_channel(channel);
		return -1;
	}
	// if skipping a cutscene, don't try and play the sound
	if (_GP(play).fast_forward)
		return -1;

	// free the old sound
	stop_and_destroy_channel(channel);
	debug_script_log("Playing sound %d on channel %d", val1, channel);

	SOUNDCLIP *soundfx = aclip ? load_sound_and_play(aclip, false) : nullptr;
	if (soundfx == nullptr) {
		debug_script_warn("Sound sample load failure: cannot load sound %d", val1);
		debug_script_log("FAILED to load sound %d", val1);
		return -1;
	}

	soundfx->_priority = 10;
	soundfx->set_volume255(_GP(play).sound_volume);
	AudioChans::SetChannel(channel, soundfx);
	return channel;
}

void StopAllSounds(int evenAmbient) {
	// backwards-compatible hack -- stop Type 3 (default Sound Type)
	Game_StopAudio(3);

	if (evenAmbient)
		Game_StopAudio(1);
}

void PlayMusicResetQueue(int newmus) {
	_GP(play).music_queue_size = 0;
	newmusic(newmus);
}

void SeekMIDIPosition(int position) {
	if (_GP(play).silent_midi == 0 && _G(current_music_type) != MUS_MIDI)
		return;

	auto *ch = AudioChans::GetChannel(SCHAN_MUSIC);
	ch->seek(position);
	debug_script_log("Seek MIDI position to %d", position);
}

int GetMIDIPosition() {
	if (_GP(play).fast_forward)
		return 99999;
	if (_GP(play).silent_midi == 0 && _G(current_music_type) != MUS_MIDI)
		return -1; // returns -1 on failure according to old manuals

	auto *ch = AudioChans::GetChannelIfPlaying(SCHAN_MUSIC);
	if (ch) {
		return ch->get_pos();
	}

	return -1;
}

int IsMusicPlaying() {
	// in case they have a "while (IsMusicPlaying())" loop
	if ((_GP(play).fast_forward) && (_GP(play).skip_until_char_stops < 0))
		return 0;

	// This only returns positive if there was a music started by old audio API
	if (_G(current_music_type) == 0)
		return 0;

	auto *ch = AudioChans::GetChannel(SCHAN_MUSIC);
	if (ch == nullptr) { // This was probably a hacky fix in case it was not reset by game update; TODO: find out if needed
		_G(current_music_type) = 0;
		return 0;
	}

	bool result = (ch->is_playing()) || (_G(crossFading) > 0 && (AudioChans::GetChannelIfPlaying(_G(crossFading)) != nullptr));
	return result ? 1 : 0;
}

int PlayMusicQueued(int musnum) {

	// Just get the queue size
	if (musnum < 0)
		return _GP(play).music_queue_size;

	if ((IsMusicPlaying() == 0) && (_GP(play).music_queue_size == 0)) {
		newmusic(musnum);
		return 0;
	}

	if (_GP(play).music_queue_size >= MAX_QUEUED_MUSIC) {
		debug_script_log("Too many queued music, cannot add %d", musnum);
		return 0;
	}

	if ((_GP(play).music_queue_size > 0) &&
	        (_GP(play).music_queue[_GP(play).music_queue_size - 1] >= QUEUED_MUSIC_REPEAT)) {
		debug_script_warn("PlayMusicQueued: cannot queue music after a repeating tune has been queued");
		return 0;
	}

	if (_GP(play).music_repeat) {
		debug_script_log("Queuing music %d to loop", musnum);
		musnum += QUEUED_MUSIC_REPEAT;
	} else {
		debug_script_log("Queuing music %d", musnum);
	}

	_GP(play).music_queue[_GP(play).music_queue_size] = musnum;
	_GP(play).music_queue_size++;

	if (_GP(play).music_queue_size == 1) {

		clear_music_cache();

		_G(cachedQueuedMusic) = load_music_from_disk(musnum, (_GP(play).music_repeat > 0));
	}

	return _GP(play).music_queue_size;
}

void scr_StopMusic() {
	_GP(play).music_queue_size = 0;
	stopmusic();
}

void SeekMODPattern(int patnum) {
	if (_G(current_music_type) != MUS_MOD)
		return;

	auto *ch = AudioChans::GetChannelIfPlaying(SCHAN_MUSIC);
	if (ch) {
		ch->seek(patnum);
		debug_script_log("Seek MOD/XM to pattern %d", patnum);
	}
}

void SeekMP3PosMillis(int posn) {
	if (_G(current_music_type) != MUS_MP3 && _G(current_music_type) != MUS_OGG)
		return;

	auto *mus_ch = AudioChans::GetChannel(SCHAN_MUSIC);
	auto *cf_ch = (_G(crossFading) > 0) ? AudioChans::GetChannel(_G(crossFading)) : nullptr;
	if (cf_ch)
		cf_ch->seek(posn);
	else if (mus_ch)
		mus_ch->seek(posn);
}

int GetMP3PosMillis() {
	// in case they have "while (GetMP3PosMillis() < 5000) "
	if (_GP(play).fast_forward)
		return 999999;
	if (_G(current_music_type) != MUS_MP3 && _G(current_music_type) != MUS_OGG)
		return 0;  // returns 0 on failure according to old manuals

	auto *ch = AudioChans::GetChannelIfPlaying(SCHAN_MUSIC);
	if (ch) {
		int result = ch->get_pos_ms();
		if (result >= 0)
			return result;

		return ch->get_pos();
	}

	return 0;
}

void SetMusicVolume(int newvol) {
	if ((newvol < kRoomVolumeMin) || (newvol > kRoomVolumeMax))
		quitprintf("!SetMusicVolume: invalid volume number. Must be from %d to %d.", kRoomVolumeMin, kRoomVolumeMax);
	_GP(thisroom).Options.MusicVolume = (RoomVolumeMod)newvol;
	update_music_volume();
}

void SetMusicMasterVolume(int newvol) {
	const int min_volume = _G(loaded_game_file_version) < kGameVersion_330 ? 0 :
	                       -LegacyMusicMasterVolumeAdjustment - (kRoomVolumeMax * LegacyRoomVolumeFactor);
	if ((newvol < min_volume) | (newvol > 100))
		quitprintf("!SetMusicMasterVolume: invalid volume - must be from %d to %d", min_volume, 100);
	_GP(play).music_master_volume = newvol + LegacyMusicMasterVolumeAdjustment;
	update_music_volume();
}

void SetSoundVolume(int newvol) {
	if ((newvol < 0) | (newvol > 255))
		quit("!SetSoundVolume: invalid volume - must be from 0-255");
	_GP(play).sound_volume = newvol;
	Game_SetAudioTypeVolume(AUDIOTYPE_LEGACY_AMBIENT_SOUND, (newvol * 100) / 255, VOL_BOTH);
	Game_SetAudioTypeVolume(AUDIOTYPE_LEGACY_SOUND, (newvol * 100) / 255, VOL_BOTH);
	update_ambient_sound_vol();
}

void SetChannelVolume(int chan, int newvol) {
	if ((newvol < 0) || (newvol > 255))
		quit("!SetChannelVolume: invalid volume - must be from 0-255");
	if ((chan < 0) || (chan >= _GP(game).numGameChannels))
		quit("!SetChannelVolume: invalid channel id");

	auto *ch = AudioChans::GetChannelIfPlaying(chan);

	if (ch) {
		if (chan == _GP(ambient)[chan].channel) {
			_GP(ambient)[chan].vol = newvol;
			update_ambient_sound_vol();
		} else
			ch->set_volume255(newvol);
	}
}

void SetDigitalMasterVolume(int newvol) {
	if ((newvol < 0) | (newvol > 100))
		quit("!SetDigitalMasterVolume: invalid volume - must be from 0-100");
	_GP(play).digital_master_volume = newvol;
#if !AGS_PLATFORM_SCUMMVM
	auto newvol_f = static_cast<float>(newvol) / 100.0;
	audio_core_set_master_volume(newvol_f);
#endif
}

int GetCurrentMusic() {
	return _GP(play).cur_music_number;
}

void SetMusicRepeat(int loopflag) {
	_GP(play).music_repeat = loopflag;
}

void PlayMP3File(const char *filename) {
	debug_script_log("PlayMP3File %s", filename);

	AssetPath asset_name(filename, "audio");
	const bool doLoop = (_GP(play).music_repeat > 0);

	SOUNDCLIP *clip = my_load_ogg(asset_name, doLoop);
	int sound_type = 0;

	if (clip)
		sound_type = MUS_OGG;

	if (!clip) {
		clip = my_load_mp3(asset_name, doLoop);
		sound_type = MUS_MP3;
	}

	if (!clip) {
		debug_script_warn("PlayMP3File: music file '%s' not found or be read", filename);
		return;
	}

	const int use_chan = prepare_for_new_music();
	_G(current_music_type) = sound_type;
	_GP(play).cur_music_number = 1000;
	_GP(play).playmp3file_name = filename;
	clip->set_volume255(150);
	AudioChans::SetChannel(use_chan, clip);
	post_new_music_check();
	update_music_volume();
}

void PlaySilentMIDI(int mnum) {
	if (_G(current_music_type) == MUS_MIDI)
		quit("!PlaySilentMIDI: proper midi music is in progress");

	_GP(play).silent_midi = mnum;
	_GP(play).silent_midi_channel = SCHAN_SPEECH;
	stop_and_destroy_channel(_GP(play).silent_midi_channel);
	// No idea why it uses speech voice channel, but since it does (and until this is changed)
	// we have to correctly reset speech voice in case there was a nonblocking speech
	if (_GP(play).IsNonBlockingVoiceSpeech())
		stop_voice_nonblocking();

	SOUNDCLIP *clip = load_sound_clip_from_old_style_number(true, mnum, false);
	if (clip == nullptr) {
		quitprintf("!PlaySilentMIDI: failed to load aMusic%d", mnum);
	}
	AudioChans::SetChannel(_GP(play).silent_midi_channel, clip);

	if (!clip->play()) {
		delete clip;
		clip = nullptr;
		quitprintf("!PlaySilentMIDI: failed to play aMusic%d", mnum);
	}
	clip->set_volume100(0);
}

void SetSpeechVolume(int newvol) {
	if ((newvol < 0) | (newvol > 255))
		quit("!SetSpeechVolume: invalid volume - must be from 0-255");

	auto *ch = AudioChans::GetChannel(SCHAN_SPEECH);
	if (ch)
		ch->set_volume255(newvol);
	_GP(play).speech_volume = newvol;
}

void SetVoiceMode(int newmod) {
	if ((newmod < kSpeech_First) | (newmod > kSpeech_Last))
		quitprintf("!SetVoiceMode: invalid mode number %d", newmod);
	_GP(play).speech_mode = (SpeechMode)newmod;
}

int GetVoiceMode() {
	return (int)_GP(play).speech_mode;
}

int IsVoxAvailable() {
	return _GP(play).voice_avail ? 1 : 0;
}

int IsMusicVoxAvailable() {
	return _GP(play).separate_music_lib ? 1 : 0;
}

ScriptAudioChannel *PlayVoiceClip(CharacterInfo *ch, int sndid, bool as_speech) {
	if (!play_voice_nonblocking(ch->index_id, sndid, as_speech))
		return nullptr;
	return &_G(scrAudioChannel)[SCHAN_SPEECH];
}

// Construct an asset name for the voice-over clip for the given character and cue id
String get_cue_filename(int charid, int sndid) {
	String asset_path = get_voice_assetpath();
	String script_name;
	if (charid >= 0) {
		// append the first 4 characters of the script name to the filename
		if (_GP(game).chars2[charid].scrname_new.GetAt(0) == 'c')
			script_name.SetString(_GP(game).chars2[charid].scrname_new.GetCStr() + 1, 4);
		else
			script_name.SetString(_GP(game).chars2[charid].scrname_new.GetCStr(), 4);
	} else {
		script_name = "NARR";
	}
	return String::FromFormat("%s%s%d", asset_path.GetCStr(), script_name.GetCStr(), sndid);
}

// Play voice-over clip on the common channel;
// voice_name should be bare clip name without extension
static bool play_voice_clip_on_channel(const String &voice_name) {
	stop_and_destroy_channel(SCHAN_SPEECH);

	String asset_name = voice_name;
	asset_name.Append(".wav");
	SOUNDCLIP *speechmp3 = my_load_wave(get_voice_over_assetpath(asset_name), false);

	if (speechmp3 == nullptr) {
		asset_name.ReplaceMid(asset_name.GetLength() - 3, 3, "ogg");
		speechmp3 = my_load_ogg(get_voice_over_assetpath(asset_name), false);
	}

	if (speechmp3 == nullptr) {
		asset_name.ReplaceMid(asset_name.GetLength() - 3, 3, "mp3");
		speechmp3 = my_load_mp3(get_voice_over_assetpath(asset_name), false);
	}

	if (speechmp3 != nullptr) {
		speechmp3->set_volume255(_GP(play).speech_volume);
		if (!speechmp3->play()) {
			// not assigned to a channel, so clean up manually.
			delete speechmp3;
			speechmp3 = nullptr;
		}
	}

	if (speechmp3 == nullptr) {
		debug_script_warn("Speech load failure: '%s'", voice_name.GetCStr());
		return false;
	}

	AudioChans::SetChannel(SCHAN_SPEECH, speechmp3);
	return true;
}

// Play voice-over clip and adjust audio volumes;
// voice_name should be bare clip name without extension
static bool play_voice_clip_impl(const String &voice_name, bool as_speech, bool is_blocking) {
	if (!play_voice_clip_on_channel(voice_name))
		return false;
	if (!as_speech)
		return true;

	_GP(play).speech_has_voice = true;
	_GP(play).speech_voice_blocking = is_blocking;

	cancel_scheduled_music_update();
	_GP(play).music_vol_was = _GP(play).music_master_volume;
	// Negative value means set exactly; positive means drop that amount
	if (_GP(play).speech_music_drop < 0)
		_GP(play).music_master_volume = -_GP(play).speech_music_drop;
	else
		_GP(play).music_master_volume -= _GP(play).speech_music_drop;
	apply_volume_drop_modifier(true);
	update_music_volume();
	update_ambient_sound_vol();
	return true;
}

// Stop voice-over clip and schedule audio volume reset
static void stop_voice_clip_impl() {
	_GP(play).music_master_volume = _GP(play).music_vol_was;
	// update the music in a bit (fixes two speeches follow each other
	// and music going up-then-down)
	schedule_music_update_at(AGS_Clock::now() + std::chrono::milliseconds(500));
	stop_and_destroy_channel(SCHAN_SPEECH);
}

bool play_voice_speech(int charid, int sndid) {
	// don't play speech if we're skipping a cutscene
	if (!_GP(play).ShouldPlayVoiceSpeech())
		return false;

	String voice_file = get_cue_filename(charid, sndid);
	if (!play_voice_clip_impl(voice_file, true, true))
		return false;

	int ii;  // Compare the base file name to the .pam file name
	_G(curLipLine) = -1;  // See if we have voice lip sync for this line
	_G(curLipLinePhoneme) = -1;
	for (ii = 0; ii < _G(numLipLines); ii++) {
		if (voice_file.CompareNoCase(_GP(splipsync)[ii].filename) == 0) {
			_G(curLipLine) = ii;
			break;
		}
	}
	// if the lip-sync is being used for voice sync, disable
	// the text-related lipsync
	if (_G(numLipLines) > 0)
		_GP(game).options[OPT_LIPSYNCTEXT] = 0;

	// change Sierra w/bgrnd  to Sierra without background when voice
	// is available (for Tierra)
	if ((_GP(game).options[OPT_SPEECHTYPE] == 2) && (_GP(play).no_textbg_when_voice > 0)) {
		_GP(game).options[OPT_SPEECHTYPE] = 1;
		_GP(play).no_textbg_when_voice = 2;
	}
	return true;
}

bool play_voice_nonblocking(int charid, int sndid, bool as_speech) {
	// don't play voice if we're skipping a cutscene
	if (!_GP(play).ShouldPlayVoiceSpeech())
		return false;
	// don't play voice if there's a blocking speech with voice-over already
	if (_GP(play).IsBlockingVoiceSpeech())
		return false;

	String voice_file = get_cue_filename(charid, sndid);
	return play_voice_clip_impl(voice_file, as_speech, false);
}

void stop_voice_speech() {
	if (!_GP(play).speech_has_voice)
		return;

	stop_voice_clip_impl();

	// Reset lipsync
	_G(curLipLine) = -1;
	// Set back to Sierra w/bgrnd
	if (_GP(play).no_textbg_when_voice == 2) {
		_GP(play).no_textbg_when_voice = 1;
		_GP(game).options[OPT_SPEECHTYPE] = 2;
	}
	_GP(play).speech_has_voice = false;
	_GP(play).speech_voice_blocking = false;
}

void stop_voice_nonblocking() {
	if (!_GP(play).speech_has_voice)
		return;
	stop_voice_clip_impl();
	// Only reset speech flags if we are truly playing a non-blocking voice;
	// otherwise we might be inside blocking speech function and should let
	// it keep these flags to be able to finalize properly.
	// This is an imperfection of current speech implementation.
	if (!_GP(play).speech_voice_blocking) {
		_GP(play).speech_has_voice = false;
		_GP(play).speech_voice_blocking = false;
	}
}

} // namespace AGS3
