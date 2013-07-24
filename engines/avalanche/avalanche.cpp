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

#include "avalanche/avalanche.h"

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "engines/util.h"



namespace Avalanche {

	AvalancheEngine *AvalancheEngine::s_Engine = 0;

	AvalancheEngine::AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd) : Engine(syst), _gameDescription(gd) {
		_system = syst;
		_console = new AvalancheConsole(this);

		_rnd = new Common::RandomSource("avalanche");
		_rnd->setSeed(42);     

		_graph.setParent(this);

		_gyro.setParent(this);
		_enhanced.setParent(this);
		_logger.setParent(this);
		_pingo.setParent(this);
		_scrolls.setParent(this);
		_visa.setParent(this);
		_lucerna.setParent(this);
		_enid.setParent(this);
		_celer.setParent(this);
		_sequence.setParent(this);
		_timeout.setParent(this);
		_trip.setParent(this);
		_acci.setParent(this);
		_basher.setParent(this);
		_dropdown.setParent(this);
		_closing.setParent(this);
		_avalot.setParent(this);
	}

	AvalancheEngine::~AvalancheEngine() {
		delete _console;
		delete _rnd;
	}

	GUI::Debugger *AvalancheEngine::getDebugger() {
		return _console;
	}

	Common::Platform AvalancheEngine::getPlatform() const {
		return _platform;
	}

	bool AvalancheEngine::hasFeature(EngineFeature f) const {
		return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
	}

	const char *AvalancheEngine::getCopyrightString() const {
		return "Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.";
	}

	Common::String AvalancheEngine::getSavegameFilename(int slot) {
		return _targetName + Common::String::format("-%02d.SAV", slot);
	}

	void AvalancheEngine::syncSoundSettings() {
		Engine::syncSoundSettings();

		//	_sound->syncVolume();
	}

	void AvalancheEngine::updateEvents() {
		Common::Event event;

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_avalot.handleKeyDown(event);
			}
		}
	}







	// From Bootstrp:

	const char AvalancheEngine::runcodes[2][3] = {"et", "Go"};



	// The original ones were all commented out, so porbably there's no need
	// of these two cursor functions at all. TODO: Remove later.
	void AvalancheEngine::cursor_off() {
		warning("STUB: cursor_off()");
	}

	void AvalancheEngine::cursor_on() {
		warning("STUB: cursor_on()");
	}

	// Needed later.
	void AvalancheEngine::quit() {
		cursor_on();
	}

	Common::String AvalancheEngine::strf(int32 x) {
		Common::String q = Common::String::format("%d", x);
		return q;
	}

	// Needed in dos_shell(). TODO: Remove later.
	Common::String AvalancheEngine::command_com() {
		warning("STUB: command_com()");
		return ("STUB: command_com()");
	}

	// Needed for run_avalot()'s errors. TODO: Remove later.
	void AvalancheEngine::explain(byte error) {
		warning("STUB: explain()");
	}



	//TODO: Remove these (b_flight) functions later ( https://github.com/tthurman/avalot/wiki/B-Flight )

	void AvalancheEngine::b_flight() {   /*interrupt;*/
		_storage.skellern++;
	}

	void AvalancheEngine::bflight_on() {
		_storage.skellern = _reset_;
		// setintvec(0x1c, &b_flight);
	}

	void AvalancheEngine::bflight_off() {
		// setintvec(0x1c, old_1c);
	}



	Common::String AvalancheEngine::elm2str(elm how) {
		Common::String elm2str_result;
		switch (how) {
		case Normal:
		case Musical:
			elm2str_result = "jsb";
			break;
		case Regi:
			elm2str_result = "REGI";
			break;
		case Elmpoyten:
			elm2str_result = "ELMPOYTEN";
			break;
		}
		return elm2str_result;
	}

	void AvalancheEngine::run(Common::String what, bool with_jsb, bool with_bflight, elm how) {
		warning("STUB: run(%s)", what.c_str()); 
		// Probably there'll be no need of this function, as all *.AVX-es will become classes.
	}

	void AvalancheEngine::get_arguments() {
		// This function should mess around with command line arguments,
		// but I am not sure if there'll be use of these arguments at all...
		warning("STUB: get_arguments()"); 
	}

	void AvalancheEngine::get_slope() {
		// Same as get_arguments()
		warning("STUB: get_slope()");
	}

	void AvalancheEngine::call_menu() {
		warning("STUB: call_menu()");
	}

	void AvalancheEngine::run_the_demo() {
		warning("STUB: run_the_demo()");
	}

	void AvalancheEngine::dos_shell() {
		warning("STUB: dos_shell()");
	}

	// Getting used only in demo() / call_menu(). Going to be implemented at the same time with these.
	bool AvalancheEngine::keypressed1() {	
		warning("STUB: keypressed1()");
		return false;
	}

	// Same as keypressed1().
	void AvalancheEngine::flush_buffer() {
		warning("STUB: flush_buffer()");
	}

	// Same as keypressed1().
	void AvalancheEngine::demo() {
		warning("STUB: demo()");
	}



	
	void AvalancheEngine::run_avalot() {
		bflight_on();

		_avalot.run(Common::String(runcodes[first_time]) + arguments);
		// TODO: Check if parameteres are ever used (probably not) and eventually remove them.
		// If there's an error initalizing avalot, i'll handle it in there, not here

		first_time = false;
	}



	Common::Error AvalancheEngine::run() {
		s_Engine = this;

		_console = new AvalancheConsole(this);
		
		_scrolls.init();
		_lucerna.init();
		_acci.init();
		_basher.init();
		_graph.init();

		
		

		// From bootstrp:

		first_time = true;

		get_arguments();
		get_slope();

		zoomy = true; 
		// Don't call the menu by default. Might be modified later, if get_slope() gets implemented,
		// becouse zoomy's value is given there. Not sure yet what "zoomy" stands for.
		if (!zoomy)
			call_menu();    /* Not run when zoomy. */



		do {
			run_avalot();

			//if (dosexitcode != 77)  quit(); /* Didn't stop for us. */

			switch (_storage.operation) {
			case _runShootemup:
				run("seu.avx", _jsb, _bflight, Normal);
				break;
			case _runDosshell:
				dos_shell();
				break;
			case _runGhostroom:
				run("g-room.avx", _jsb, _no_bflight, Normal);
				break;
			case _runGolden:
				run("golden.avx", _jsb, _bflight, Musical);
				break;
			}

		} while (true);



		return Common::kNoError;
	}

	

} // End of namespace Avalanche
