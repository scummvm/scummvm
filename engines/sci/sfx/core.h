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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* Sound engine */
#ifndef SCI_SFX_CORE_H
#define SCI_SFX_CORE_H

#include "common/error.h"
#include "sci/sfx/songlib.h"
#include "sci/resource.h"

namespace Sci {

class SfxPlayer;
class SongIterator;
struct fade_params_t;

#define SFX_TICKS_PER_SEC 60 /* MIDI ticks per second */


#define SFX_STATE_FLAG_MULTIPLAY (1 << 0) /* More than one song playable
** simultaneously ? */
#define SFX_STATE_FLAG_NOSOUND	 (1 << 1) /* Completely disable sound playing */

class SfxState {
private:
	SfxPlayer *_player;

public:	// FIXME, make private
	SongIterator *_it; /**< The song iterator at the heart of things */
	uint _flags; /**< SFX_STATE_FLAG_* */
	SongLibrary _songlib; /**< Song library */
	Song *_song; /**< Active song, or start of active song chain */
	bool _suspended; /**< Whether we are suspended */
	Resource *_syncResource; /**< Used by kDoSync for speech syncing in CD talkie games */
	uint _syncOffset;
	ResourceManager *_resMan;

public:
	SfxState();
	~SfxState();

	/***********/
	/* General */
	/***********/

	/* Initializes the sound engine
	** Parameters: (ResourceManager *) resMan: Resource manager for initialization
	**             (int) flags: SFX_STATE_FLAG_*
	*/
	void sfx_init(ResourceManager *resMan, int flags);

	/** Deinitializes the sound subsystem. */
	void sfx_exit();

	/* Suspends/unsuspends the sound sybsystem
	** Parameters: (int) suspend: Whether to suspend (non-null) or to unsuspend
	*/
	void sfx_suspend(bool suspend);

	/* Polls the sound server for cues etc.
	** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
	**             (SongHandle) *handle: The affected handle
	**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
	*/
	int sfx_poll(SongHandle *handle, int *cue);

	/* Polls the sound server for cues etc.
	** Parameters: (SongHandle) handle: The handle to poll
	** Returns   : (int) 0 if the cue queue is empty, SI_LOOP, SI_CUE, or SI_FINISHED otherwise
	**             (int) *cue: The sound cue number (if SI_CUE), or the loop number (if SI_LOOP)
	*/
	int sfx_poll_specific(SongHandle handle, int *cue);

	/* Determines the current global volume settings
	** Returns   : (int) The global volume, between 0 (silent) and 127 (max. volume)
	*/
	int sfx_get_volume();

	/* Determines the current global volume settings
	** Parameters: (int) volume: The new global volume, between 0 and 127 (see above)
	*/
	void sfx_set_volume(int volume);

	/* Stops all songs currently playing, purges song library
	*/
	void sfx_all_stop();


	/*****************/
	/*  Song basics  */
	/*****************/

	/* Adds a song to the internal sound library
	** Parameters: (SongIterator *) it: The iterator describing the song
	**             (int) priority: Initial song priority (higher <-> more important)
	**             (SongHandle) handle: The handle to associate with the song
	*/
	void sfx_add_song(SongIterator *it, int priority, SongHandle handle, int resnum);


	/* Deletes a song and its associated song iterator from the song queue
	** Parameters: (SongHandle) handle: The song to remove
	*/
	void sfx_remove_song(SongHandle handle);


	/**********************/
	/* Song modifications */
	/**********************/


	/* Sets the song status, i.e. whether it is playing, suspended, or stopped.
	** Parameters: (SongHandle) handle: Handle of the song to modify
	**             (int) status: The song status the song should assume
	** WAITING and PLAYING are set implicitly and essentially describe the same state
	** as far as this function is concerned.
	*/
	void sfx_song_set_status(SongHandle handle, int status);

	/* Sets the new song priority
	** Parameters: (SongHandle) handle: The handle to modify
	**             (int) priority: The priority to set
	*/
	void sfx_song_renice(SongHandle handle, int priority);

	/* Sets the number of loops for the specified song
	** Parameters: (SongHandle) handle: The song handle to reference
	**             (int) loops: Number of loops to set
	*/
	void sfx_song_set_loops(SongHandle handle, int loops);

	/* Sets the number of loops for the specified song
	** Parameters: (SongHandle) handle: The song handle to reference
	**             (int) hold: Number of loops to setn
	*/
	void sfx_song_set_hold(SongHandle handle, int hold);

	/* Instructs a song to be faded out
	** Parameters: (SongHandle) handle: The song handle to reference
	**             (fade_params_t *) fade_setup: The precise fade-out configuration to use
	*/
	void sfx_song_set_fade(SongHandle handle, fade_params_t *fade_setup);


	// Previously undocumented:
	Common::Error sfx_send_midi(SongHandle handle, int channel,
		int command, int arg1, int arg2);

	// Functions for digital sound
	void setAudioRate(uint16 rate) { _audioRate = rate; }
	Audio::SoundHandle* getAudioHandle() { return &_audioHandle; }
	int getAudioPosition();
	int startAudio(uint16 module, uint32 tuple);
	void stopAudio() { g_system->getMixer()->stopHandle(_audioHandle); }
	void pauseAudio() { g_system->getMixer()->pauseHandle(_audioHandle, true); }
	void resumeAudio() { g_system->getMixer()->pauseHandle(_audioHandle, false); }

	// misc

	/**
	 * Determines the polyphony of the player in use.
	 * @return Number of voices the active player can emit
	 */
	int sfx_get_player_polyphony();

	/**
	 * Tells the player to stop its internal iterator.
	 */
	void sfx_reset_player();

	/**
	 * Pass a raw MIDI event to the synth of the player.
	 * @param	argc	Length of buffer holding the midi event
	 * @param	argv	The buffer itself
	 */
	void sfx_player_tell_synth(int buf_nr, byte *buf);

protected:
	void freezeTime();
	void thawTime();

	bool isPlaying(Song *song);
	void setSongStatus(Song *song, int status);
	void updateSingleSong();
	void updateMultiSong();
	void update();

private:
	uint16 _audioRate;
	Audio::SoundHandle _audioHandle;
	Audio::AudioStream* getAudioStream(uint32 number, uint32 volume, int *sampleLen);
};

} // End of namespace Sci

#endif // SCI_SFX_CORE_H
