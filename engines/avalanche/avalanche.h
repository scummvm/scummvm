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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_H
#define AVALANCHE_H

#include "engines/engine.h"
#include "avalanche/console.h"
#include "engines/advancedDetector.h"

namespace Common {
class RandomSource;
}

namespace Avalanche {

struct AvalancheGameDescription;

static const int kSavegameVersion = 1;

class AvalancheEngine : public Engine {
public:
	AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd);
	~AvalancheEngine();

	OSystem *_system;

	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;
	
	const AvalancheGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const AvalancheGameDescription *gd);

	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings();

protected:
	// Engine APIs
	Common::Error run();

private:
	static AvalancheEngine *s_Engine;

	AvalancheConsole *_console;
	Common::Platform _platform;

	void initialize();

	// From bootstrp:

	enum elm {Normal, Musical, Elmpoyten, Regi};

	static const int16 _runShootemup = 1, _runDosshell = 2, _runGhostroom = 3, _runGolden = 4;

	static const int16 _reset_ = 0;

	static const bool _jsb = true, _no_jsb = false, _bflight = true, _no_bflight = false;

	struct {
		byte operation;
		uint16 skellern;
		byte contents[1000];
	} _storage;

	Common::String arguments, demo_args, args_with_no_filename;

	bool first_time;

	byte original_mode;
	void *old_1c;

	Common::String segofs;

	bool zoomy;

	int32 soundcard, speed, baseaddr, irq, dma;

	void cursor_off();

	void cursor_on();

	void quit();

	Common::String strf(int32 x);

	Common::String command_com();

	void explain(byte error);

	void b_flight();

	void bflight_on();

	void bflight_off();

	Common::String elm2str(elm how);

	void run(Common::String what, bool with_jsb, bool with_bflight, elm how);

	void run_avalot();

	void run_the_demo();

	void get_arguments();

	void dos_shell();

	bool keypressed1();

	void flush_buffer();

	void demo();

	void call_menu();

	void get_slope();

};

} // End of namespace Avalanche

#endif // AVALANCHE_H
