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

#ifndef QUEEN_QUEEN_H
#define QUEEN_QUEEN_H

#include "engines/engine.h"
#include "common/random.h"

namespace Common {
class SeekableReadStream;
}

/**
 * This is the namespace of the Queen engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Flight of the Amazon Queen
 */
namespace Queen {

struct GameStateHeader {
	uint32 version;
	uint32 flags;
	uint32 dataSize;
	char description[32];
};

class BamScene;
class BankManager;
class Command;
class Debugger;
class Display;
class Graphics;
class Grid;
class Input;
class Logic;
class Resource;
class Sound;
class Walk;

class QueenEngine : public Engine {
public:

	QueenEngine(OSystem *syst);
	~QueenEngine() override;

	BamScene *bam() const { return _bam; }
	BankManager *bankMan() const { return _bankMan; }
	Command *command() const { return _command; }
	Debugger *debugger() const { return _debugger; }
	Display *display() const { return _display; }
	Graphics *graphics() const { return _graphics; }
	Grid *grid() const { return _grid; }
	Input *input() const { return _input; }
	Logic *logic() const { return _logic; }
	Resource *resource() const { return _resource; }
	Sound *sound() const { return _sound; }
	Walk *walk() const { return _walk; }

	Common::RandomSource randomizer;

	void registerDefaultSettings();
	void checkOptionSettings();
	void readOptionSettings();
	void writeOptionSettings();

	int talkSpeed() const { return _talkSpeed; }
	void talkSpeed(int speed) { _talkSpeed = speed; }
	bool subtitles() const { return _subtitles; }
	void subtitles(bool enable) { _subtitles = enable; }

	void update(bool checkPlayerInput = false);

	bool canLoadOrSave() const;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;
	virtual int getAutosaveSlot() const override { return 99; }
	virtual Common::String getSaveStateName(int slot) const override;
	void makeGameStateName(int slot, char *buf) const;
	int getGameStateSlot(const char *filename) const;
	void findGameStateDescriptions(char descriptions[100][32]);
	Common::SeekableReadStream *readGameStateHeader(int slot, GameStateHeader *gsh);

	enum {
		SAVESTATE_CUR_VER  = 1,
		SAVESTATE_MAX_NUM  = 100,
		SAVESTATE_MAX_SIZE = 30000,

		SLOT_LISTPREFIX    = -2,
		SLOT_AUTOSAVE      = -1,
		SLOT_QUICKSAVE     = 0,

		MIN_TEXT_SPEED     = 4,
		MAX_TEXT_SPEED     = 100
	};

protected:

	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	void syncSoundSettings() override;


	int _talkSpeed;
	bool _subtitles;
	uint32 _lastUpdateTime;
	bool _gameStarted;

	BamScene *_bam;
	BankManager *_bankMan;
	Command *_command;
	Debugger *_debugger;
	Display *_display;
	Graphics *_graphics;
	Grid *_grid;
	Input *_input;
	Logic *_logic;
	Resource *_resource;
	Sound *_sound;
	Walk *_walk;
};

} // End of namespace Queen

#endif
