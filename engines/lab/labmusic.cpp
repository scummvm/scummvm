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
	_tFileLength = 0;
	_tLeftInFile = 0;

	_manyBuffers = MANYBUFFERS;

	_musicFilledTo = 0;
	_musicPlaying  = 0;
	_musicOnBuffer = 0;

	_filelength = 0;
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
void Music::updateMusic(void) {
	uint16 i;

	WSDL_ProcessInput(0);

	updateMouse();

	if (EffectPlaying)
		updateSoundBuffers();
	else if (_musicOn) {
		for (i = 0; i < 2; i++) {
			if (musicBufferEmpty(i)) {
				playMusicBlock(_musicBuffer[_musicPlaying], MUSICBUFSIZE, i, SAMPLESPEED);

				if (_musicPlaying)
					_musicOnBuffer = _musicPlaying - 1;
				else
					_musicOnBuffer = _manyBuffers - 1;

				_musicPlaying++;

				if (_musicPlaying >= _manyBuffers)
					_musicPlaying = 0;
			}
		}
	}
}


void Music::fillbuffer(uint16 unit) {
	return;

	warning("STUB: Music::fillbuffer");
	uint32 Size = MUSICBUFSIZE;
	void *ptr  = _musicBuffer[unit];

	if (Size < _leftinfile) {
		_file->read(ptr, Size);
		_leftinfile -= Size;
	} else {
		_file->read(ptr, _leftinfile);

		memset((char *)ptr + _leftinfile, 0, Size - _leftinfile);

		_file->seek(0);
		_leftinfile = _filelength;
	}
}



/*****************************************************************************/
/* Figures out how many *complete* buffers of music left to play.            */
/*****************************************************************************/
uint16 Music::getManyBuffersLeft(void) {
	uint16 mp = _musicOnBuffer;

	if (mp == _musicFilledTo) /* Already filled */
		return _manyBuffers;
	else if (mp > _musicFilledTo)
		return _manyBuffers - (mp - _musicFilledTo);
	else
		return _musicFilledTo - mp;
}



/*****************************************************************************/
/* Fills up the buffers that have already been played if necessary; if doit  */
/* is set to TRUE then it will fill up all empty buffers.  Otherwise, it     */
/* Check if there are MINBUFFERS or less buffers that are playing.           */
/*****************************************************************************/
void Music::fillUpMusic(bool doit) {
	int16 ManyLeft, ManyFill;

	updateMusic();

	if (!_musicOn)
		return;

	ManyLeft = getManyBuffersLeft();

	if (ManyLeft < MINBUFFERS)
		doit = true;
	else if (ManyLeft == _manyBuffers)  /* All the buffers are already full */
		doit = false;

	if (doit && (ManyLeft < _manyBuffers) && ManyLeft) {
		ManyFill = _manyBuffers - ManyLeft - 1;

		while (ManyFill > 0) {
			_musicFilledTo++;

			if (_musicFilledTo >= _manyBuffers)
				_musicFilledTo = 0;

			fillbuffer(_musicFilledTo);
			updateMusic();

			ManyFill--;
		}

		updateMusic();

		ManyLeft = getManyBuffersLeft();

		if (ManyLeft < _manyBuffers) {
			ManyFill = _manyBuffers - ManyLeft - 1;

			while (ManyFill > 0) {
				_musicFilledTo++;

				if (_musicFilledTo >= _manyBuffers)
					_musicFilledTo = 0;

				fillbuffer(_musicFilledTo);
				updateMusic();

				ManyFill--;
			}
		}
	}

	updateMusic();

	/* NYI: A check for dirty cds; for instance, if lots of buffers already
	   played */
}




/*****************************************************************************/
/* Starts up the music initially.                                            */
/*****************************************************************************/
void Music::startMusic(bool startatbegin) {
	uint16 counter;

	if (!_musicOn)
		return;

	flushAudio();

	if (startatbegin) {
		_file->seek(0);
		_leftinfile  = _filelength;
	}

	initSampleRate(SAMPLESPEED);

	_musicPlaying  = 0;
	_musicOnBuffer = 0;
	_musicFilledTo = _manyBuffers - 1;

	_musicOn = false;

	for (counter = 0; counter < _manyBuffers; counter++)
		fillbuffer(counter);

	_musicOn = true;
	updateMusic();
}






/*****************************************************************************/
/* Initializes the music buffers.                                            */
/*****************************************************************************/
bool Music::initMusic(void) {
	uint16 counter;
	bool mem;

	if (!_turnMusicOn)
		return true;

	_musicOn = true;
	_musicPaused = false;

	const char *filename;

	if (_winmusic)
		filename = "Music:WinGame";
	else
		filename = "Music:BackGrou";

	if (_musicBuffer[0] == NULL) {
		for (counter = 0; counter < _manyBuffers; counter++)
			_musicBuffer[counter] = malloc(MUSICBUFSIZE);
	}

	_filelength = sizeOfFile(filename);
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
void Music::freeMusic(void) {
	_musicOn = false;

	if (_file->isOpen())
		_file->close();

	_file = 0;
}


/*****************************************************************************/
/* Pauses the background music.                                              */
/*****************************************************************************/
void Music::pauseBackMusic(void) {
	if (!_musicPaused && _musicOn) {
		updateMusic();
		_musicOn = false;
		flushAudio();

		if (_musicPlaying)
			_musicPlaying--;
		else
			_musicPlaying = _manyBuffers - 1;

		_musicPaused = true;
	}
}



/*****************************************************************************/
/* Restarts the paused background music.                                     */
/*****************************************************************************/
void Music::restartBackMusic(void) {
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
void Music::checkMusic(void) {
	updateMusic();

	if (!_musicOn)
		return;

	fillUpMusic(false);
}



/*****************************************************************************/
/* Checks to see if need to fill buffers fill of music.                      */
/*****************************************************************************/
void Music::newCheckMusic(void) {
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
		_tFileLength = _filelength;
#if defined(DOSCODE)
		_tLeftInFile = _leftinfile;
#else
		_tLeftInFile = _leftinfile + 65536L;

		if (_tLeftInFile > _tFileLength)
			_tLeftInFile = _leftinfile;

#endif
	}

	_file = openPartial(newmusic);

	if (_file) {
		_musicOn = true;   /* turn it off */
		setMusic(false);

		_filelength = sizeOfFile(newmusic);

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
void Music::resetMusic(void) {
	if (!_tFile)
		return;

	if (_file->isOpen())
		_file->close();

	_file      = _tFile;
	_filelength = _tFileLength;
	_leftinfile = _tLeftInFile;

	_file->seek(_filelength - _leftinfile);

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





#define FUDGEFACTOR  5L
#define READSPEED    (2 * 130000L)



/*****************************************************************************/
/* Checks whether or note enough memory in music buffer before loading any   */
/* files.  Fills it if not.  Does not take into account the current buffer   */
/* playing; a built in fudge factor.  We've also got another FUDGEFACTOR     */
/* defined above in case things go wrong.                                    */
/*                                                                           */
/* Here, the seconds are multipled by 10.                                    */
/*****************************************************************************/
byte **Music::newOpen(const char *name) {
	uint32 filelength, LeftSecs, Time;

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

#if defined(DOSCODE)
		LeftSecs = (getManyBuffersLeft() * MUSICBUFSIZE * 10) / SAMPLESPEED;
#else
		LeftSecs = (getManyBuffersLeft() * MUSICBUFSIZE * 10) / (2 * SAMPLESPEED);
#endif

		filelength = sizeOfFile(name) * 10;
		Time = 10 +                           /* Seek time for the music and the file */
		       (filelength / READSPEED) +     /* Read time for the file */
		       FUDGEFACTOR;

		if (Time >= LeftSecs)
			fillUpMusic(true);
	}

	if (!_doNotFileFlushAudio && EffectPlaying)
		flushAudio();

	file = openFile(name);
	checkMusic();
	return file;
}



/*****************************************************************************/
/* Checks whether or note enough memory in music buffer to continue loading  */
/* in a file.   Fills the music buffer if not.  Does not take into account   */
/* the current buffer playing; a built in fudge factor.  We've also got      */
/* another FUDGEFACTOR defined above in case things go wrong.                */
/*                                                                           */
/* Here, the seconds are multipled by 10.                                    */
/*****************************************************************************/
void Music::fileCheckMusic(uint32 filelength) {
	uint32 LeftSecs, Time;

	if (_musicOn) {
		updateMusic();

#if defined(DOSCODE)
		LeftSecs = (getManyBuffersLeft() * MUSICBUFSIZE * 10) / SAMPLESPEED;
#else
		LeftSecs = (getManyBuffersLeft() * MUSICBUFSIZE * 10) / (2 * SAMPLESPEED);
#endif

		filelength *= 10;
		Time = 5 +                            /* Seek time for the music */
		       (filelength / READSPEED) +     /* Read time for the file  */
		       FUDGEFACTOR;

		if (Time >= LeftSecs)
			fillUpMusic(true);
	}
}

} // End of namespace Lab
