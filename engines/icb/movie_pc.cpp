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
#include "engines/icb/movie_pc.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/global_objects.h"

#include "common/system.h"
#include "common/keyboard.h"

namespace ICB {

// Instance our global bink handler for movies
MovieManager *g_theSequenceManager;

SequenceManager::SequenceManager() {
	m_binkObject = NULL;
	m_x = 0;
	m_y = 0;
	m_fadeCounter = 255;
	m_fadeRate = 4;
	m_haveFaded = FALSE8;
	m_rater = TRUE8;
	m_haveClearedScreen = FALSE8;
	m_mission1intro = FALSE8;
	// Zero these, as the global construction handled it earlier:
	m_loop = FALSE8;
	m_endAtFrame = FALSE8;
}

SequenceManager::~SequenceManager() {}

bool8 SequenceManager::Register(const char *fileName, bool8 fade, bool8 loop, uint32 flags) {
	// Release any currently held sequence
	Kill();

	if (g_theMusicManager)
		g_theMusicManager->StopMusic();

	// Special case hack
	if (strstr(fileName, "m01intro") != NULL)
		m_mission1intro = TRUE8;
	else
		m_mission1intro = FALSE8;

	// No hack for russian or polish thanks
	if (g_theClusterManager->GetLanguage() == T_RUSSIAN || g_theClusterManager->GetLanguage() == T_POLISH)
		m_mission1intro = FALSE8;

	// Let bink make it's own direct sound interface
	BinkSoundUseDirectSound(0);

	// Open the Bink file
	m_binkObject = BinkOpen(fileName, flags);
	if (!m_binkObject) {
		Fatal_error("[FILE %s]: BINK error: %s", fileName, BinkGetError());
		return FALSE8;
	}

	// Mute the sound if necessary
	if (m_rater)
		SetVolume(GetMusicVolume());
	else
		SetVolume(0);

	// Now we need to centre the movie in the screen so calculate the correct coordinates
	uint32 movieWidth = m_binkObject->Width;
	uint32 movieHeight = m_binkObject->Height;

	if (movieWidth != SCREEN_WIDTH) {
		m_x = (SCREEN_WIDTH / 2) - (movieWidth / 2);
	}
	if (movieHeight != SCREEN_DEPTH) {
		m_y = (SCREEN_DEPTH / 2) - (movieHeight / 2);
	}

	m_flags = BINKSURFACE32;

	// Should we fade the screen out before playing the movie
	if (fade) {
		m_fadeCounter = 1; // Yes we should
	} else {
		m_fadeCounter = 255; // No thanks
	}

	m_haveFaded = FALSE8;
	m_loop = loop;
	m_endAtFrame = 0;

	return TRUE8;
}

bool8 SequenceManager::Busy() {
	if (!m_binkObject)
		return FALSE8;

	return TRUE8;
}

uint32 SequenceManager::GetMovieHeight() {
	if (Busy())
		return m_binkObject->Height;

	return 0;
}

uint32 SequenceManager::GetMovieWidth() {
	if (Busy())
		return m_binkObject->Width;

	return 0;
}

uint32 SequenceManager::GetMovieFrames() {
	if (Busy())
		return m_binkObject->Frames;

	return 0;
}

int SequenceManager::GetFrameNumber() {
	if (Busy())
		return m_binkObject->FrameNum;

	return 0;
}

uint32 SequenceManager::DrawFrame(uint32 surface_id) {
	if (!m_binkObject)
		return NOMOVIE;

	// Get the current time count
	uint32 timeStart = g_system->getMillis();

	// Non-looping movies can be quit using the escape key
	if (Read_DI_once_keys(Common::KEYCODE_ESCAPE) && !m_loop) {
		// Release Bink object and file
		Kill();
		return FINISHED;
	}
	// And also the joystick pause button cos I'm lazy
	if (Read_Joystick_once(pause_button) && !m_loop) {
		// Release Bink object and file
		Kill();
		return FINISHED;
	}

	// Don't do bink, fade the screen
	if (m_fadeCounter < (255 / m_fadeRate)) {
		FadeScreen(surface_id);
		m_haveFaded = TRUE8;
		return FADING;
	}

	// Have we performed a fade
	if (m_haveFaded) {
		// Reassign correct surface id
		surface_id = working_buffer_id;
		m_haveFaded = FALSE8;
	} else {
		// Black out the screen before we play the movie
		if ((m_binkObject->FrameNum == 1) && !m_haveClearedScreen) {
			surface_manager->Clear_surface(working_buffer_id);
			m_haveClearedScreen = TRUE8;
		}
	}

	if (m_rater) {
		// Don't do anything (skipping frames for audio sync and that I think)
		if (BinkWait(m_binkObject)) {
			BinkService(m_binkObject);

			// Mission 1 intro is special case (25fps rather than 12fps)
			if (m_mission1intro) {
				while (g_system->getMillis() - timeStart < 25)
					;
			} else
				return WAITING;
		}
	}

	// Decompress a frame
	BinkDoFrame(m_binkObject);

	// For access to buffer
	uint32 pitch;
	uint8 *surface_address;

	// Lock the directdraw surface
	surface_address = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);

	// Copy the data onto the screen
	BinkCopyToBuffer(m_binkObject, surface_address, pitch, surface_manager->Get_height(surface_id), m_x, m_y, m_flags);

	// Unlock the buffer
	surface_manager->Unlock_surface(surface_id);

	// Advance to next frame or signal end of sequence
	if (m_binkObject->FrameNum == m_binkObject->Frames || (m_endAtFrame != 0 && m_endAtFrame == m_binkObject->FrameNum)) {
		if (m_loop) {
			BinkGoto(m_binkObject, 1, 0);
		} else {
			// Release Bink object and file
			Kill();
			surface_manager->Clear_surface(working_buffer_id);

			return FINISHED;
		}
	} else
		BinkNextFrame(m_binkObject);

	// An extra precaution preventing sound break-up
	BinkService(m_binkObject);

	// No problemo
	return JUSTFINE;
}

void SequenceManager::SetRate() {
	// Toggle
	if (m_rater)
		m_rater = FALSE8;
	else
		m_rater = TRUE8;

	if (m_binkObject) {
		// Mute the sound if necessary
		if (m_rater)
			SetVolume(GetMusicVolume());
		else
			SetVolume(0);
	}
}

void SequenceManager::FadeScreen(uint32 surface_id) {
	// Pointer to subtractive table block
	uint8 subtractive[8];

	// Fade by table
	subtractive[4] = subtractive[0] = (uint8)m_fadeRate;
	subtractive[5] = subtractive[1] = (uint8)m_fadeRate;
	subtractive[6] = subtractive[2] = (uint8)m_fadeRate;
	subtractive[7] = subtractive[3] = 0;

	// Lock the directdraw surface
	uint8 *surface_address = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

#if 1
	for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
		for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
			// 32-bit BGRA pixel
			uint8 *pixel = &surface_address[xPos * 4];
			// Subtract from RGB components
			for (int i = 0; i < 3; i++) {
				pixel[i] = MAX(0, pixel[i] - subtractive[i]);
			}
		}
		// Next line
		surface_address += pitch;
	}
#else
	// Safe surface pointer
	uint32 *safe_ad = (uint32 *)surface_address;
	int pixelPairs = SCREEN_WIDTH / 2;
	for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
		_asm {
			lea  edi, subtractive  ; // Load the address of the blend colour block
			mov  ecx, pixelPairs   ; // Pixel Counter (2 pixels at a time mind)
			mov  esi, safe_ad      ; // Load the address of the pixels
			movq MM0, [edi]        ; // Put address of the blend colour block into MMX register

			subtractive_fade_loop:

			movq    MM1, [esi]             ; // Load 2 pixels
			psubusb MM1, MM0               ; // Do the subtract
			movq    [esi], MM1             ; // Store the result
			add     esi, 8                 ; // Move pixel pointer on
			dec     ecx                    ; // Reduce counter
			jne     subtractive_fade_loop  ; // On to the next 2 pixels

			EMMS   ; // Clear/Set MMX/FPU flag
		}

		safe_ad += (pitch / 4);
	}
#endif

	// Unlock the buffer
	surface_manager->Unlock_surface(surface_id);

	// Increment fade level (pixel shift right)
	m_fadeCounter++;
}

void SequenceManager::SetVolume(int32 vol) {
	if (!m_binkObject)
		return;

	int bink_vol = vol * 256;

	if (bink_vol < 0)
		bink_vol = 0;
	if (bink_vol > 32768)
		bink_vol = 32768;

	BinkSetVolume(m_binkObject, bink_vol);
}

void SequenceManager::Kill() {
	if (m_binkObject) {
		BinkClose(m_binkObject);
		m_binkObject = NULL;
	}

	// Reset blitting coordinates
	m_x = 0;
	m_y = 0;

	m_fadeCounter = 255;

	m_haveClearedScreen = FALSE8;
}

bool MovieManager::registerMovie(const char *fileName, bool8 fade, bool8 loop) {
	// Release any currently held sequence
	kill();
	_x = 0;
	_y = 0;

	if (g_theMusicManager)
		g_theMusicManager->StopMusic();
	/*
	// Special case hack
	if (strstr(fileName, "m01intro") != NULL)
	        m_mission1intro = TRUE8;
	else
	        m_mission1intro = FALSE8;

	// No hack for russian or polish thanks
	if (g_theClusterManager->GetLanguage() == T_RUSSIAN || g_theClusterManager->GetLanguage() == T_POLISH)
	        m_mission1intro = FALSE8;
	*/

	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(fileName);
	if (!stream) {
		return false;
	}
	if (!_binkDecoder->loadStream(stream)) {
		return false;
	}
	if (_binkDecoder->getWidth() != SCREEN_WIDTH) {
		_x = (SCREEN_WIDTH / 2) - (_binkDecoder->getWidth() / 2);
	}
	if (_binkDecoder->getHeight() != SCREEN_DEPTH) {
		_y = (SCREEN_DEPTH / 2) - (_binkDecoder->getHeight() / 2);
	}
	_binkDecoder->start();
	return true;
}

uint32 MovieManager::drawFrame(uint32 surface_id) {
	if (!_binkDecoder->isPlaying()) {
		return FINISHED;
	}
	// Non-looping movies can be quit using the escape key
	if (Read_DI_once_keys(Common::KEYCODE_ESCAPE) /* && !m_loop*/) {
		kill();
		return FINISHED;
	}
	// TODO: The rest.
	const Graphics::Surface *surface = _binkDecoder->decodeNextFrame();
	if (!surface) {
		kill();
		return FINISHED;
	}
	// For access to buffer
	uint16 pitch;
	uint8 *surface_address;

	// Lock the directdraw surface
	surface_address = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);
	uint32 height = surface_manager->Get_height(surface_id);

	for (int i = 0; i < surface->h; i++) {
		if (i + _y >= height) {
			warning("Movie out of bounds");
			break;
		}
		memcpy(surface_address + (i + _y) * pitch, surface->getBasePtr(0, i), MIN(surface->pitch, pitch));
	}

	// Unlock the buffer
	surface_manager->Unlock_surface(surface_id);
	return JUSTFINE;
}

} // End of namespace ICB
