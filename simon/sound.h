/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef SIMON_SOUND_H
#define SIMON_SOUND_H

#include "sound/mixer.h"
#include "simon/intern.h"

namespace Simon {

class BaseSound;

class SimonSound {
private:
	byte _game;
	const char *_gameDataPath;

	SoundMixer *_mixer;

	int _voice_index;
	int _ambient_index;

	BaseSound *_voice;
	BaseSound *_effects;

	bool _effects_paused;
	bool _ambient_paused;

	uint16 *_filenums;
	uint32 *_offsets;

public:
	PlayingSoundHandle _voice_handle;
	PlayingSoundHandle _effects_handle;
	PlayingSoundHandle _ambient_handle;

	bool _voice_file;
	uint _ambient_playing;

	SimonSound(const byte game, const GameSpecificSettings *gss, const char *gameDataPath, SoundMixer *mixer);
	~SimonSound();
	
	void readSfxFile(const char *filename, const char *gameDataPath);
	void loadSfxTable(File *gameFile, uint32 base);
	void readVoiceFile(const char *filename, const char *gameDataPath);

	void playVoice(uint sound);
	void playEffects(uint sound);
	void playAmbient(uint sound);

	bool hasVoice();
	void stopVoice();
	void stopAll();
	void effectsPause(bool b);
	void ambientPause(bool b);
};

} // End of namespace Simon

#endif
