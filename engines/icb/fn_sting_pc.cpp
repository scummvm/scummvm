/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/debug.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound.h"

namespace ICB {

mcodeFunctionReturnCodes fn_preload_sting(int32 &result, int32 *params) { return (MS->fn_preload_sting(result, params)); }

mcodeFunctionReturnCodes fn_play_sting(int32 &result, int32 *params) { return (MS->fn_play_sting(result, params)); }

mcodeFunctionReturnCodes fn_stop_sting(int32 &result, int32 *params) { return (MS->fn_stop_sting(result, params)); }

mcodeFunctionReturnCodes speak_allocate_music(int32 &result, int32 *params) { return (MS->speak_allocate_music(result, params)); }

mcodeFunctionReturnCodes speak_preload_music(int32 &result, int32 *params) { return (MS->speak_preload_music(result, params)); }

mcodeFunctionReturnCodes speak_play_music(int32 &result, int32 *params) { return (MS->speak_play_music(result, params)); }

mcodeFunctionReturnCodes speak_stop_music(int32 &result, int32 *params) { return (MS->speak_stop_music(result, params)); }

mcodeFunctionReturnCodes speak_end_music(int32 &result, int32 *params) { return (MS->speak_end_music(result, params)); }

// Global data for this file
int inSpeechMusicAllocated = 0;

// Useful prototype
bool8 DoesClusterContainFile(const char *clustername, uint32 hash_to_find, uint32 &fileoffset, uint32 &filesize);

void LoadSting(uint32 looking_for_hash) {
#ifdef PC_DEMO
	return;
#endif

	pxString clustername;

	// Just so that the game can be run from non-clustered data
	/*const char* cluster_root = "thegame\\english\\pc\\everything\\cd1";*/

	// Do nothing if parameter zero
	if (looking_for_hash == 0)
		return;

	clustername.Format("g\\music.clu");

	// To store offsets in cluster
	uint32 file_offset, file_size;

	// Take a look
	if (!DoesClusterContainFile(clustername, looking_for_hash, file_offset, file_size)) {
		// Wav not present in global cluster either
		Fatal_error(pxVString("Music file: %d could not be found in the global music cluster", looking_for_hash));
	}

	// If we've allocated some emulated psx memory
	if (inSpeechMusicAllocated != 0) {
		if ((int)file_size > inSpeechMusicAllocated)
			Fatal_error("Cannot load music as it's size exceeds that given to speak_allocate_music()!");
	}

	// Pass parameters to the music manager through this low-level interface
	if (g_theMusicManager) {
		g_theMusicManager->LoadMusic(clustername, file_offset, GetMusicVolume());
	}
}

void PlaySting(uint32 looking_for_hash) {
#ifdef PC_DEMO
	return;
#endif

	pxString clustername;

	// Just so that the game can be run from non-clustered data
	/*const char* cluster_root = "thegame\\english\\pc\\everything\\cd1";*/

	clustername.Format("g\\music.clu");

	// To store offsets in cluster
	uint32 file_offset, file_size;

	// Take a look
	if (!DoesClusterContainFile(clustername, looking_for_hash, file_offset, file_size)) {
		// Wav not present in global cluster either
		Fatal_error(pxVString("Music file: %d could not be found in the global music cluster", looking_for_hash));
	}

	if (g_theMusicManager) {
		// This loads only if not already loaded then starts playback
		g_theMusicManager->StartMusic(clustername, file_offset, GetMusicVolume());
	}
}

mcodeFunctionReturnCodes _game_session::fn_play_sting(int32 &, int32 *params) {
	if (inSpeechMusicAllocated != 0)
		return IR_REPEAT;

	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();
	}

	PlaySting((uint32)params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_preload_sting(int32 &, int32 *params) {
	if (inSpeechMusicAllocated != 0)
		return IR_REPEAT;

	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();
	}

	LoadSting((uint32)params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_stop_sting(int32 &, int32 *) {
	if (inSpeechMusicAllocated != 0)
		return IR_REPEAT;

	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();

		// Wait for fade to finish
		if (g_theMusicManager->IsPlaying())
			return IR_REPEAT;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_allocate_music(int32 &, int32 *params) {
	int seconds = (int)params[0];
	int hertz = (int)params[1];

	// Hardcode this puupy to match the psx for now
	hertz = 16000;

	if (seconds <= 0 || seconds > 20)
		Fatal_error("Invalid parameter to speak_allocate_music(secs, hz): secs = %d", seconds);

	// Calculate the number of bytes requested to check later
	inSpeechMusicAllocated = 2 * seconds * hertz * (16 / 8);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_preload_music(int32 &, int32 *params) {
	if (inSpeechMusicAllocated == 0)
		Fatal_error("speak_preload_music() called before speak_allocate_music()!");

	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();
	}

	LoadSting((uint32)params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_play_music(int32 &, int32 *params) {
	if (inSpeechMusicAllocated == 0)
		Fatal_error("speak_play_music() called before speak_allocate_music()!");

	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();
	}

	PlaySting((uint32)params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_stop_music(int32 &, int32 *) {
	if (g_theMusicManager) {
		// Stop playback
		g_theMusicManager->StopMusic();
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::speak_end_music(int32 &, int32 *) {
	inSpeechMusicAllocated = 0;

	return IR_CONT;
}

} // End of namespace ICB
