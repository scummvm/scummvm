/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

class GameDetector {
public:
	int detectMain(int argc, char **argv);
	void parseCommandLine(int argc, char **argv);
	bool detectGame(void);
	char *getGameName(void);

	bool _fullScreen;
	byte _gameId;
	bool _simon;

	bool _use_adlib;

	byte _music_volume;
	byte _sfx_volume;
	bool _amiga;

	uint16 _talkSpeed;
	uint16 _debugMode;
	uint16 _noSubtitles;
	uint16 _bootParam;
	uint16 _soundCardType;

	char *_gameDataPath;
	int _gameTempo;
	int _midi_driver;
	char *_exe_name;
	const char *_gameText;
	uint32 _features;

	int _gfx_driver;
	int _gfx_mode;
	
	int _scummVersion;
	int _cdrom;
	int _save_slot;
	
	bool _saveconfig;

	int parseGraphicsMode(const char *s);

	bool parseMusicDriver(const char *s);
	
	void updateconfig();

public:
	OSystem *createSystem();
	MidiDriver *createMidi();
};
