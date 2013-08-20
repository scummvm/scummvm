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
#include "common/savefile.h"

#include "engines/util.h"

#include "gui/saveload.h"


namespace Avalanche {

AvalancheEngine *AvalancheEngine::s_Engine = 0;

AvalancheEngine::AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	_system = syst;
	s_Engine = this;
	_console = new AvalancheConsole(this);

	_rnd = new Common::RandomSource("avalanche");
	_rnd->setSeed(42);     
}

AvalancheEngine::~AvalancheEngine() {
	delete _console;
	delete _rnd;

	delete _graphics;
	delete _parser;

	delete _avalot;
	delete _gyro;
	delete _enhanced;
	delete _logger;
	delete _pingo;
	delete _scrolls;
	delete _visa;
	delete _lucerna;
	delete _enid;
	delete _celer;
	delete _sequence;
	delete _timeout;
	delete _trip;
	delete _acci;
	delete _basher;
	delete _dropdown;
	delete _closing;
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new Graphics(this);
	_parser = new Parser(this);

	_avalot = new Avalot(this);
	_gyro = new Gyro(this);
	_enhanced = new Enhanced(this);
	_logger = new Logger(this);
	_pingo = new Pingo(this);
	_scrolls = new Scrolls(this);
	_visa = new Visa(this);
	_lucerna = new Lucerna(this);
	_enid = new Enid(this);
	_celer = new Celer(this);
	_sequence = new Sequence(this);
	_timeout = new Timeout(this);
	_trip = new Trip(this);
	_acci = new Acci(this);
	_basher = new Basher(this);
	_dropdown = new Dropdown(this);
	_closing = new Closing(this);

	_graphics->init();

	_scrolls->init();
	_lucerna->init();
	_acci->init();
	_basher->init();
	

	return Common::kNoError;
}


GUI::Debugger *AvalancheEngine::getDebugger() {
	return _console;
}

Common::Platform AvalancheEngine::getPlatform() const {
	return _platform;
}



bool AvalancheEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSavingDuringRuntime) || (f == kSupportsLoadingDuringRuntime);
}

const char *AvalancheEngine::getCopyrightString() const {
	return "Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.";
}



void AvalancheEngine::synchronize(Common::Serializer &sz) {
	//blockwrite(f, dna, sizeof(dna));
	sz.syncAsByte(_gyro->dna.rw);
	sz.syncAsByte(_gyro->dna.carrying);
	for (byte i = 0; i < numobjs; i++)
		sz.syncAsByte(_gyro->dna.obj[i]);
	sz.syncAsSint16LE(_gyro->dna.score);
	sz.syncAsSint32LE(_gyro->dna.pence);
	sz.syncAsByte(_gyro->dna.room);
	sz.syncAsByte(_gyro->dna.wearing);
	sz.syncAsByte(_gyro->dna.swore);
	sz.syncAsByte(_gyro->dna.saves);
	sz.syncBytes(_gyro->dna.rooms, 100);
	sz.syncAsByte(_gyro->dna.alcohol);
	sz.syncAsByte(_gyro->dna.playednim);
	sz.syncAsByte(_gyro->dna.wonnim);
	sz.syncAsByte(_gyro->dna.winestate);
	sz.syncAsByte(_gyro->dna.cwytalot_gone);
	sz.syncAsByte(_gyro->dna.pass_num);
	sz.syncAsByte(_gyro->dna.ayles_is_awake);
	sz.syncAsByte(_gyro->dna.drawbridge_open);
	sz.syncAsByte(_gyro->dna.avaricius_talk);
	sz.syncAsByte(_gyro->dna.bought_onion);
	sz.syncAsByte(_gyro->dna.rotten_onion);
	sz.syncAsByte(_gyro->dna.onion_in_vinegar);
	sz.syncAsByte(_gyro->dna.given2spludwick);
	sz.syncAsByte(_gyro->dna.brummie_stairs);
	sz.syncAsByte(_gyro->dna.cardiff_things);
	sz.syncAsByte(_gyro->dna.cwytalot_in_herts);
	sz.syncAsByte(_gyro->dna.avvy_is_awake);
	sz.syncAsByte(_gyro->dna.avvy_in_bed);
	sz.syncAsByte(_gyro->dna.user_moves_avvy);
	sz.syncAsByte(_gyro->dna.dogfoodpos);
	sz.syncAsByte(_gyro->dna.givenbadgetoiby);
	sz.syncAsByte(_gyro->dna.friar_will_tie_you_up);
	sz.syncAsByte(_gyro->dna.tied_up);
	sz.syncAsByte(_gyro->dna.box_contents);
	sz.syncAsByte(_gyro->dna.talked_to_crapulus);
	sz.syncAsByte(_gyro->dna.jacques_awake);
	sz.syncAsByte(_gyro->dna.ringing_bells);
	sz.syncAsByte(_gyro->dna.standing_on_dais);
	sz.syncAsByte(_gyro->dna.taken_pen);
	sz.syncAsByte(_gyro->dna.arrow_triggered);
	sz.syncAsByte(_gyro->dna.arrow_in_the_door);

	if (sz.isSaving()) {
		uint16 like2drinkSize = _gyro->dna.like2drink.size();
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr = _gyro->dna.like2drink[i];
			sz.syncAsByte(actChr);
		}

		uint16 favourite_songSize = _gyro->dna.favourite_song.size();
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			char actChr = _gyro->dna.favourite_song[i];
			sz.syncAsByte(actChr);
		}

		uint16 worst_place_on_earthSize = _gyro->dna.worst_place_on_earth.size();
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			char actChr = _gyro->dna.worst_place_on_earth[i];
			sz.syncAsByte(actChr);
		}

		uint16 spare_eveningSize = _gyro->dna.spare_evening.size();
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			char actChr = _gyro->dna.spare_evening[i];
			sz.syncAsByte(actChr);
		}
	} else {
		if (!_gyro->dna.like2drink.empty())
			_gyro->dna.like2drink.clear();
		uint16 like2drinkSize;
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr;
			sz.syncAsByte(actChr);
			_gyro->dna.like2drink += actChr;
		}

		if (!_gyro->dna.favourite_song.empty())
			_gyro->dna.favourite_song.clear();
		uint16 favourite_songSize;
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			char actChr;
			sz.syncAsByte(actChr);
			_gyro->dna.favourite_song += actChr;
		}

		if (!_gyro->dna.worst_place_on_earth.empty())
			_gyro->dna.worst_place_on_earth.clear();
		uint16 worst_place_on_earthSize;
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			char actChr;
			sz.syncAsByte(actChr);
			_gyro->dna.worst_place_on_earth += actChr;
		}

		if (!_gyro->dna.spare_evening.empty())
			_gyro->dna.spare_evening.clear();
		uint16 spare_eveningSize;
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			char actChr;
			sz.syncAsByte(actChr);
			_gyro->dna.spare_evening += actChr;
		}
	}
	
	sz.syncAsSint32LE(_gyro->dna.total_time);
	sz.syncAsByte(_gyro->dna.jumpstatus);
	sz.syncAsByte(_gyro->dna.mushroom_growing);
	sz.syncAsByte(_gyro->dna.spludwicks_here);
	sz.syncAsByte(_gyro->dna.last_room);
	sz.syncAsByte(_gyro->dna.last_room_not_map);
	sz.syncAsByte(_gyro->dna.crapulus_will_tell);
	sz.syncAsByte(_gyro->dna.enter_catacombs_from_lusties_room);
	sz.syncAsByte(_gyro->dna.teetotal);
	sz.syncAsByte(_gyro->dna.malagauche);
	sz.syncAsByte(_gyro->dna.drinking);
	sz.syncAsByte(_gyro->dna.entered_lusties_room_as_monk);
	sz.syncAsByte(_gyro->dna.cat_x);
	sz.syncAsByte(_gyro->dna.cat_y);
	sz.syncAsByte(_gyro->dna.avvys_in_the_cupboard);
	sz.syncAsByte(_gyro->dna.geida_follows);
	sz.syncAsByte(_gyro->dna.geida_spin);
	sz.syncAsByte(_gyro->dna.geida_time);
	sz.syncAsByte(_gyro->dna.nextbell);
	sz.syncAsByte(_gyro->dna.geida_given_potion);
	sz.syncAsByte(_gyro->dna.lustie_is_asleep);
	sz.syncAsByte(_gyro->dna.flip_to_where);
	sz.syncAsByte(_gyro->dna.flip_to_ped);
	sz.syncAsByte(_gyro->dna.been_tied_up);
	sz.syncAsByte(_gyro->dna.sitting_in_pub);
	sz.syncAsByte(_gyro->dna.spurge_talk);
	sz.syncAsByte(_gyro->dna.met_avaroid);
	sz.syncAsByte(_gyro->dna.taken_mushroom);
	sz.syncAsByte(_gyro->dna.given_pen_to_ayles);
	sz.syncAsByte(_gyro->dna.asked_dogfood_about_nim);


	//for (byte groi = 0; groi < numtr; groi ++) {
	//	if (tr[groi].quick) {
	//		blockwrite(f, groi, 1);
	//		tr[groi].savedata(f);
	//	}
	//}
	
	byte spriteNum;
	if (sz.isSaving()) {
		spriteNum = 0;
		for (byte i = 0; i < _trip->numtr; i++)
			if (_trip->tr[i].quick)
				spriteNum++;
	}
	sz.syncAsByte(spriteNum);
	
	if (sz.isLoading())
		for (byte i = 0; i < _trip->numtr; i++) { // Deallocate sprites.
			if (_trip->tr[i].quick)
				_trip->tr[i].done();
		}

	for (byte i = 0; i < spriteNum; i++) {
		sz.syncAsByte(_trip->tr[i].whichsprite);
		sz.syncAsByte(_trip->tr[i].check_me);
		
	
		if (sz.isLoading()) {
			_trip->tr[i].quick = true;
			_trip->tr[i].init(_trip->tr[i].whichsprite, _trip->tr[i].check_me, _trip);
		}

		sz.syncAsByte(_trip->tr[i].ix);
		sz.syncAsByte(_trip->tr[i].iy);
		sz.syncAsByte(_trip->tr[i].face);
		sz.syncAsByte(_trip->tr[i].step);
		sz.syncAsByte(_trip->tr[i].visible);
		sz.syncAsByte(_trip->tr[i].homing);
		sz.syncAsByte(_trip->tr[i].count);
		sz.syncAsByte(_trip->tr[i]._info.xw);
		sz.syncAsByte(_trip->tr[i].xs);
		sz.syncAsByte(_trip->tr[i].ys);
		sz.syncAsByte(_trip->tr[i].totalnum);
		sz.syncAsSint16LE(_trip->tr[i].hx);
		sz.syncAsSint16LE(_trip->tr[i].hy);
		sz.syncAsByte(_trip->tr[i].call_eachstep);
		sz.syncAsByte(_trip->tr[i].eachstep);
		sz.syncAsByte(_trip->tr[i].vanishifstill);

		sz.syncAsSint16LE(_trip->tr[i].x);
		sz.syncAsSint16LE(_trip->tr[i].y);

		if (sz.isLoading() && _trip->tr[i].visible)
			_trip->tr[i].appear(_trip->tr[i].x, _trip->tr[i].y, _trip->tr[i].face);
	}

	



	//groi = 177;
	//blockwrite(f, groi, 1);

	//blockwrite(f, times, sizeof(times)); // Timeout.times: Timers.
	for (byte i = 0; i < 7; i++) {
		sz.syncAsSint32LE(_timeout->times[i].time_left);
		sz.syncAsByte(_timeout->times[i].then_where);
		sz.syncAsByte(_timeout->times[i].what_for);
	}

	//blockwrite(f, seq, sizeof(seq)); // Sequencer information.
	sz.syncBytes(_sequence->seq, _sequence->seq_length);
}

bool AvalancheEngine::canSaveGameStateCurrently() { // TODO: Refine these!!!
	return (!_gyro->seescroll);
}

Common::Error AvalancheEngine::saveGameState(int slot, const Common::String &desc) {
	return (saveGame(slot, desc) ? Common::kNoError : Common::kWritingFailed);
}

bool AvalancheEngine::saveGame(const int16 slot, const Common::String &desc) {
	Common::String fileName = getSaveFileName(slot);
	Common::OutSaveFile *f = g_system->getSavefileManager()->openForSaving(fileName);

	if (!f) {
		warning("Can't create file '%s', game not saved.", fileName.c_str());
		return false;
	}

	char *signature = "AVAL";

	f->write(signature, 4);

	f->writeUint32LE(desc.size());

	f->write(desc.c_str(), desc.size());

	Common::Serializer sz(NULL, f);

	synchronize(sz);
	
	f->finalize();

	delete f;

	return true;
}



Common::String AvalancheEngine::getSaveFileName(const int slot) {
	Common::String upperName = _targetName;
	upperName.toUppercase();
	return upperName+ Common::String::format("-%02d.SAV", slot);
}



bool AvalancheEngine::canLoadGameStateCurrently() { // TODO: Refine these!!!
	return (!_gyro->seescroll);
}

Common::Error AvalancheEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool AvalancheEngine::loadGame(const int16 slot) {
	Common::String fileName = getSaveFileName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (!f)
		return false;
	
	// Check for our signature.
	Common::String signature;
	for (byte i = 0; i < 4; i++)
		signature += f->readByte();
	if (signature != "AVAL")
		return false;

	// We dont care about the description here.
	uint32 descSize = f->readUint32LE();
	f->skip(descSize);

	Common::Serializer sz(f, NULL);

	synchronize(sz);

	delete f;

	_gyro->seescroll = true;  // This prevents display of the new sprites before the new picture is loaded.

	if (_gyro->holdthedawn) {
		_gyro->holdthedawn = false;
		_lucerna->dawn();
	}

	_celer->forget_chunks();

	_lucerna->minor_redraw();

	_gyro->whereis[0] = _gyro->dna.room;
	
	_gyro->alive = true;

	_lucerna->objectlist();

	_trip->newspeed();

	_lucerna->showrw();

	return true;
}



void AvalancheEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_lucerna->holdLeftMouse = true; // Used in Lucerna::checkclick().
			break;
		case Common::EVENT_LBUTTONUP:
			_lucerna->holdLeftMouse = false; // Same as above.
			break;
		case Common::EVENT_KEYDOWN:
			_avalot->handleKeyDown(event);
		}
	}
}

bool AvalancheEngine::getEvent(Common::Event &event) {
	return _eventMan->pollEvent(event);
}

Common::Point AvalancheEngine::getMousePos() {
	return _eventMan->getMousePos();
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

	_avalot->run(Common::String(runcodes[first_time]) + arguments);
	// TODO: Check if parameteres are ever used (probably not) and eventually remove them.
	// If there's an error initalizing avalot, i'll handle it in there, not here

	first_time = false;
}



Common::Error AvalancheEngine::run() {
	Common::ErrorCode err = initialize();
	if (err != Common::kNoError)
		return err;

		

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
