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
#include "graphics/thumbnail.h"

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
	delete _dropdown;
	delete _closing;
	delete _gyro;
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new Graphics(this);
	_parser = new Parser(this);

	_avalot = new Avalot(this);
	_gyro = new Gyro(this);
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
	_dropdown = new Dropdown(this);
	_closing = new Closing(this);

	_graphics->init();

	_scrolls->init();
	_lucerna->init();
	_acci->init();
	_parser->init();

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
		uint16 like2drinkSize = 0;
		char actChr = ' ';
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->dna.like2drink += actChr;
		}

		if (!_gyro->dna.favourite_song.empty())
			_gyro->dna.favourite_song.clear();
		uint16 favourite_songSize = 0;
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->dna.favourite_song += actChr;
		}

		if (!_gyro->dna.worst_place_on_earth.empty())
			_gyro->dna.worst_place_on_earth.clear();
		uint16 worst_place_on_earthSize = 0;
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->dna.worst_place_on_earth += actChr;
		}

		if (!_gyro->dna.spare_evening.empty())
			_gyro->dna.spare_evening.clear();
		uint16 spare_eveningSize = 0;
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
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


#if 0
	for (byte groi = 0; groi < numtr; groi++) {
		if (tr[groi].quick) {
			blockwrite(f, groi, 1);
			tr[groi].savedata(f);
		}
	}
#endif

	byte spriteNum = 0;
	if (sz.isSaving()) {
		for (byte i = 0; i < _trip->numtr; i++) {
			if (_trip->tr[i].quick)
				spriteNum++;
		}
	}
	sz.syncAsByte(spriteNum);
	
	if (sz.isLoading()) {
		for (byte i = 0; i < _trip->numtr; i++) { // Deallocate sprites.
			if (_trip->tr[i].quick)
				_trip->tr[i].done();
		}
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
		sz.syncAsByte(_trip->tr[i]._info._xWidth);
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
	return (!_gyro->seescroll && _gyro->alive);
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

	const char *signature = "AVAL";
	f->write(signature, 4);

	// Write version. We can't restore from obsolete versions.
	f->writeByte(kSavegameVersion);

	f->writeUint32LE(desc.size());
	f->write(desc.c_str(), desc.size());
	::Graphics::saveThumbnail(*f);

	TimeDate t;
	_system->getTimeAndDate(t);
	f->writeSint16LE(t.tm_mday);
	f->writeSint16LE(t.tm_mon);
	f->writeSint16LE(t.tm_year);

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

	// Check version. We can't restore from obsolete versions.
	byte saveVersion = f->readByte();
	if (saveVersion != kSavegameVersion) {
		warning("Savegame of incompatible version!");
		delete f;
		return false;
	}

	// Read the description.
	uint32 descSize = f->readUint32LE();
	Common::String description;
	for (uint32 i = 0; i < descSize; i++) {
		char actChar = f->readByte();
		description += actChar;
	}
	description.toUppercase();

	::Graphics::skipThumbnail(*f);

	// Read the time the game was saved.
	TimeDate t;
	t.tm_mday = f->readSint16LE();
	t.tm_mon = f->readSint16LE();
	t.tm_year = f->readSint16LE();

	Common::Serializer sz(f, NULL);

	synchronize(sz);

	delete f;

	_gyro->isLoaded = true;

	_gyro->seescroll = true;  // This prevents display of the new sprites before the new picture is loaded.

	if (_gyro->holdthedawn) {
		_gyro->holdthedawn = false;
		_lucerna->dawn();
	}

	_celer->forgetBackgroundSprites();

	_lucerna->minor_redraw();

	_dropdown->setupMenu();

	_gyro->whereis[0] = _gyro->dna.room;
	
	_gyro->alive = true;

	_lucerna->objectlist();

	_trip->newspeed();

	_lucerna->showrw();

	_gyro->ontoolbar = false;
	_trip->trippancy_link();

	_celer->updateBackgroundSprites();
	
	_scrolls->display(Common::String(_scrolls->kControlItalic) + "Loaded: " + _scrolls->kControlRoman + description + ".ASG"
		+ _scrolls->kControlCenter + _scrolls->kControlNewLine + _scrolls->kControlNewLine
		+ _gyro->roomname + _scrolls->kControlNewLine + _scrolls->kControlNewLine
		+ "saved on " + expandDate(t.tm_mday, t.tm_mon, t.tm_year) + '.');

	if (_trip->tr[0].quick && _trip->tr[0].visible)
		_trip->rwsp(0, _gyro->dna.rw); // We push Avvy in the right direction is he was moving.

	return true;
}

Common::String AvalancheEngine::expandDate(int d, int m, int y) {
	const Common::String months[12] = {
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

	Common::String month = months[m];

	Common::String day = _gyro->strf(d);

	if (((1 <= d) && (d <= 9)) || ((21 <= d) && (d <= 31)))
		switch (d % 10) {
		case 1:
			day = day + "st";
			break;
		case 2:
			day = day + "nd";
			break;
		case 3:
			day = day + "rd";
			break;
		default:
			day = day + "th";
		}

	return day + ' ' + month + ' ' + _gyro->strf(y + 1900);
}



void AvalancheEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_lucerna->holdLeftMouse = true; // Used in Lucerna::checkclick() and Dropdown::menu_link().
			break;
		case Common::EVENT_LBUTTONUP:
			_lucerna->holdLeftMouse = false; // Same as above.
			break;
		case Common::EVENT_KEYDOWN:
			_avalot->handleKeyDown(event);
			break;
		default:
			break;
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

const char AvalancheEngine::kRuncodes[2][3] = {"et", "Go"};



// The original ones were all commented out, so porbably there's no need
// of these two cursor functions at all. TODO: Remove later.
void AvalancheEngine::cursorOff() {
	warning("STUB: cursorOff()");
}

void AvalancheEngine::cursorOn() {
	warning("STUB: cursorOn()");
}

// Needed later.
void AvalancheEngine::quit() {
	cursorOn();
}

// Needed in dos_shell(). TODO: Remove later.
Common::String AvalancheEngine::commandCom() {
	warning("STUB: commandCom()");
	return ("STUB: commandCom()");
}

// Needed for run_avalot()'s errors. TODO: Remove later.
void AvalancheEngine::explain(byte error) {
	warning("STUB: explain()");
}



//TODO: Remove these (b_flight) functions later ( https://github.com/tthurman/avalot/wiki/B-Flight )

void AvalancheEngine::bFlight() {   //interrupt;
	_storage._skellern++;
}

void AvalancheEngine::bFlightOn() {
	_storage._skellern = kReset;
	// setintvec(0x1c, &b_flight);
}

void AvalancheEngine::bFlightOff() {
	// setintvec(0x1c, old_1c);
}



Common::String AvalancheEngine::elmToStr(Elm how) {
	switch (how) {
	case kNormal:
	case kMusical:
		return Common::String("jsb");
	case kRegi:
		return Common::String("REGI");
	case kElmpoyten:
		return Common::String("ELMPOYTEN");
	// Useless, but silent a warning
	default:
		return Common::String("");
	}
}

void AvalancheEngine::run(Common::String what, bool withJsb, bool withBflight, Elm how) {
	warning("STUB: run(%s)", what.c_str()); 
	// Probably there'll be no need of this function, as all *.AVX-es will become classes.
}

void AvalancheEngine::getArguments() {
	// This function should mess around with command line arguments,
	// but I am not sure if there'll be use of these arguments at all...
	warning("STUB: getArguments()"); 
}

void AvalancheEngine::getSlope() {
	// Same as get_arguments()
	warning("STUB: getSlope()");
}

void AvalancheEngine::callMenu() {
	warning("STUB: callMenu()");
}

void AvalancheEngine::runDemo() {
	warning("STUB: runDemo()");
}

void AvalancheEngine::dosShell() {
	warning("STUB: dosShell()");
}

// Getting used only in demo() / call_menu(). Going to be implemented at the same time with these.
bool AvalancheEngine::keyPressed() {	
	warning("STUB: keyPressed()");
	return false;
}

// Same as keypressed1().
void AvalancheEngine::flushBuffer() {
	warning("STUB: flushBuffer()");
}

// Same as keypressed1().
void AvalancheEngine::demo() {
	warning("STUB: demo()");
}



	
void AvalancheEngine::runAvalot() {
	bFlightOn();

	_avalot->run(Common::String(kRuncodes[_firstTime]) + _arguments);
	// TODO: Check if parameteres are ever used (probably not) and eventually remove them.
	// If there's an error initalizing avalot, i'll handle it in there, not here

	_firstTime = false;
}



Common::Error AvalancheEngine::run() {
	Common::ErrorCode err = initialize();
	if (err != Common::kNoError)
		return err;

	

	// From bootstrp:

	_firstTime = true;

	getArguments();
	getSlope();

	_zoomy = true; 
	// Don't call the menu by default. Might be modified later, if get_slope() gets implemented,
	// because zoomy's value is given there. Not sure yet what "zoomy" stands for.
	if (!_zoomy)
		callMenu();    // Not run when zoomy.



	do {
		runAvalot();

		// Needed for later implementation!!! Don't remove these comments!!!
		
		//if (dosexitcode != 77)  quit(); // Didn't stop for us.

		//switch (_storage._operation) {
		//case kRunShootemup:
		//	run("seu.avx", kJsb, kBflight, kNormal);
		//	break;
		//case kRunDosshell:
		//	dosShell();
		//	break;
		//case kRunGhostroom:
		//	run("g-room.avx", kJsb, kNoBflight, kNormal);
		//	break;
		//case kRunGolden:
		//	run("golden.avx", kJsb, kBflight, kMusical);
		//	break;
		//}

	} while (false);



	return Common::kNoError;
}

	

} // End of namespace Avalanche
