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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/timing.h"
#include "lab/mouse.h"
#include "lab/vga.h"
#include "lab/lab.h"

namespace Lab {

#define MUSICBUFSIZE   (2 * 65536L)

#define SAMPLESPEED    15000L

extern bool EffectPlaying;
Music *g_music;

Music::Music() {
	_file = 0;
	_tFile = 0;
	_musicPaused = false;

	_tMusicOn = false;
	_tLeftInFile = 0;

	_leftinfile = 0;

	_musicOn = false;
	_turnMusicOn = false;
	_winmusic = false;
	_doNotFileFlushAudio = false;
}

/*****************************************************************************/
/* Figures out which buffer is currently playing based on messages sent to   */
/* it from the Audio device.                                                 */
/*****************************************************************************/
void Music::updateMusic() {
	WSDL_ProcessInput(0);

	updateMouse();

	if (EffectPlaying)
		updateSoundBuffers();
	
	if (_musicOn && getPlayingBufferCount() < MAXBUFFERS) {
		// NOTE: We need to use malloc(), cause this will be freed with free()
		// by the music code
		byte *musicBuffer = (byte *)malloc(MUSICBUFSIZE);
		fillbuffer(musicBuffer);
		playMusicBlock(musicBuffer, MUSICBUFSIZE, 0, SAMPLESPEED);
	}
}


void Music::fillbuffer(byte *musicBuffer) {
	if (MUSICBUFSIZE < _leftinfile) {
		_file->read(musicBuffer, MUSICBUFSIZE);
		_leftinfile -= MUSICBUFSIZE;
	} else {
		_file->read(musicBuffer, _leftinfile);

		memset((char *)musicBuffer + _leftinfile, 0, MUSICBUFSIZE - _leftinfile);

		_file->seek(0);
		_leftinfile = _file->size();
	}
}


/*****************************************************************************/
/* Fills up the buffers that have already been played if necessary; if doit  */
/* is set to TRUE then it will fill up all empty buffers.  Otherwise, it     */
/* Check if there are MINBUFFERS or less buffers that are playing.           */
/*****************************************************************************/
void Music::fillUpMusic(bool doit) {
	updateMusic();
}


/*****************************************************************************/
/* Starts up the music initially.                                            */
/*****************************************************************************/
void Music::startMusic(bool startatbegin) {
	if (!_musicOn)
		return;

	flushAudio();

	if (startatbegin) {
		_file->seek(0);
		_leftinfile  = _file->size();
	}

	_musicOn = true;
	updateMusic();
}


/*****************************************************************************/
/* Initializes the music buffers.                                            */
/*****************************************************************************/
bool Music::initMusic() {

	if (!_turnMusicOn)
		return true;

	_musicOn = true;
	_musicPaused = false;

	const char *filename;

	if (_winmusic)
		filename = "Music:WinGame";
	else
		filename = "Music:BackGrou";

	_file = openPartial(filename);

	if (_file) {
		startMusic(true);
		return true;
	}

	_musicOn = false;
	return false;
}



/*****************************************************************************/
/* Frees up the music buffers and closes the file.                           */
/*****************************************************************************/
void Music::freeMusic() {
	_musicOn = false;

	if (_file->isOpen())
		_file->close();

	_file = 0;
}


/*****************************************************************************/
/* Pauses the background music.                                              */
/*****************************************************************************/
void Music::pauseBackMusic() {
	if (!_musicPaused && _musicOn) {
		updateMusic();
		_musicOn = false;
		flushAudio();

		// TODO: Pause

		_musicPaused = true;
	}
}



/*****************************************************************************/
/* Restarts the paused background music.                                     */
/*****************************************************************************/
void Music::restartBackMusic() {
	if (_musicPaused) {
		flushAudio();
		_musicOn = true;
		updateMusic();
		_musicPaused = false;
	}
}





/*****************************************************************************/
/* Checks to see if need to fill buffers fill of music.                      */
/*****************************************************************************/
void Music::checkMusic() {
	updateMusic();

	if (!_musicOn)
		return;

	fillUpMusic(false);
}



/*****************************************************************************/
/* Checks to see if need to fill buffers fill of music.                      */
/*****************************************************************************/
void Music::newCheckMusic() {
	checkMusic();
}





/*****************************************************************************/
/* Turns the music on and off.                                               */
/*****************************************************************************/
void Music::setMusic(bool on) {
	flushAudio();

	if (on && !_musicOn) {
		_musicOn = true;
		startMusic(true);
	} else if (!on && _musicOn) {
		_musicOn = false;
		updateMusic();
	} else
		_musicOn = on;
}

/*****************************************************************************/
/* Changes the background music to something else.                           */
/*****************************************************************************/
void Music::changeMusic(const char *newmusic) {
	if (!_tFile) {
		_tFile = _file;
		_tMusicOn = _musicOn;
#if defined(DOSCODE)
		_tLeftInFile = _leftinfile;
#else
		_tLeftInFile = _leftinfile + 65536L;

		if (_tLeftInFile > (uint32)_tFile->size())
			_tLeftInFile = _leftinfile;

#endif
	}

	_file = openPartial(newmusic);

	if (_file) {
		_musicOn = true;   /* turn it off */
		setMusic(false);
		_musicOn = false;  /* turn it back on */
		setMusic(true);
	} else {
		_file  = _tFile;
		_tFile = 0;
	}
}



/*****************************************************************************/
/* Changes the background music to the original piece playing.               */
/*****************************************************************************/
void Music::resetMusic() {
	if (!_tFile)
		return;

	if (_file->isOpen())
		_file->close();

	_file      = _tFile;
	_leftinfile = _tLeftInFile;

	_file->seek(_file->size() - _leftinfile);

	_musicOn = true;
	setMusic(false);
	updateMusic();

	if (!_tMusicOn) {
		_tFile = 0;
		return;
	}

	_musicOn = _tMusicOn;
	startMusic(false);

	_tFile = 0;
}


/*****************************************************************************/
/* Checks whether or note enough memory in music buffer before loading any   */
/* files.  Fills it if not.  Does not take into account the current buffer   */
/* playing; a built in fudge factor.  We've also got another FUDGEFACTOR     */
/* defined above in case things go wrong.                                    */
/*                                                                           */
/* Here, the seconds are multipled by 10.                                    */
/*****************************************************************************/
byte **Music::newOpen(const char *name) {
	byte **file;

	if (name == NULL) {
		return NULL;
	}

	if ((strcmp(name, "") == 0) || (strcmp(name, " ") == 0)) {
		return NULL;
	}

	if ((file = isBuffered(name))) {
		return file;
	}

	if (_musicOn) {
		updateMusic();
		fillUpMusic(true);
	}

	if (!_doNotFileFlushAudio && EffectPlaying)
		flushAudio();

	file = openFile(name);
	checkMusic();
	return file;
}

} // End of namespace Lab
