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
	delete _lucerna;
	delete _celer;
	delete _sequence;
	delete _timer;
	delete _animation;
	delete _acci;
	delete _menu;
	delete _closing;
	delete _gyro;
	delete _sound;
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new Graphics(this);
	_parser = new Parser(this);

	_avalot = new Avalot(this);
	_gyro = new Gyro(this);
	_pingo = new Pingo(this);
	_scrolls = new Scrolls(this);
	_lucerna = new Lucerna(this);
	_celer = new Celer(this);
	_sequence = new Sequence(this);
	_timer = new Timer(this);
	_animation = new Animation(this);
	_acci = new Acci(this);
	_menu = new Menu(this);
	_closing = new Closing(this);
	_sound = new SoundHandler(this);

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
	sz.syncAsByte(_animation->_direction);
	sz.syncAsByte(_gyro->_carryNum);
	for (int i = 0; i < kObjectNum; i++)
		sz.syncAsByte(_gyro->_objects[i]);
	sz.syncAsSint16LE(_gyro->_dnascore);
	sz.syncAsSint32LE(_gyro->_money);
	sz.syncAsByte(_gyro->_room);
	sz.syncAsByte(_gyro->_wearing);
	sz.syncAsByte(_gyro->_sworeNum);
	sz.syncAsByte(_gyro->_saveNum);
	sz.syncBytes(_gyro->_roomCount, 100);
	sz.syncAsByte(_gyro->_alcoholLevel);
	sz.syncAsByte(_gyro->_playedNim);
	sz.syncAsByte(_gyro->_wonNim);
	sz.syncAsByte(_gyro->_wineState);
	sz.syncAsByte(_gyro->_cwytalotGone);
	sz.syncAsByte(_gyro->_passwordNum);
	sz.syncAsByte(_gyro->_aylesIsAwake);
	sz.syncAsByte(_gyro->_drawbridgeOpen);
	sz.syncAsByte(_gyro->_avariciusTalk);
	sz.syncAsByte(_gyro->_boughtOnion);
	sz.syncAsByte(_gyro->_rottenOnion);
	sz.syncAsByte(_gyro->_onionInVinegar);
	sz.syncAsByte(_gyro->_givenToSpludwick);
	sz.syncAsByte(_gyro->_brummieStairs);
	sz.syncAsByte(_gyro->_cardiffQuestionNum);
	sz.syncAsByte(_gyro->_passedCwytalotInHerts);
	sz.syncAsByte(_gyro->_avvyIsAwake);
	sz.syncAsByte(_gyro->_avvyInBed);
	sz.syncAsByte(_gyro->_userMovesAvvy);
	sz.syncAsByte(_gyro->_npcFacing);
	sz.syncAsByte(_gyro->_givenBadgeToIby);
	sz.syncAsByte(_gyro->_friarWillTieYouUp);
	sz.syncAsByte(_gyro->_tiedUp);
	sz.syncAsByte(_gyro->_boxContent);
	sz.syncAsByte(_gyro->_talkedToCrapulus);
	sz.syncAsByte(_gyro->_jacquesState);
	sz.syncAsByte(_gyro->_bellsAreRinging);
	sz.syncAsByte(_gyro->_standingOnDais);
	sz.syncAsByte(_gyro->_takenPen);
	sz.syncAsByte(_gyro->_arrowTriggered);
	sz.syncAsByte(_gyro->_arrowInTheDoor);

	if (sz.isSaving()) {
		uint16 like2drinkSize = _gyro->_favouriteDrink.size();
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr = _gyro->_favouriteDrink[i];
			sz.syncAsByte(actChr);
		}

		uint16 favourite_songSize = _gyro->_favouriteSong.size();
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			char actChr = _gyro->_favouriteSong[i];
			sz.syncAsByte(actChr);
		}

		uint16 worst_place_on_earthSize = _gyro->_worstPlaceOnEarth.size();
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			char actChr = _gyro->_worstPlaceOnEarth[i];
			sz.syncAsByte(actChr);
		}

		uint16 spare_eveningSize = _gyro->_spareEvening.size();
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			char actChr = _gyro->_spareEvening[i];
			sz.syncAsByte(actChr);
		}
	} else {
		if (!_gyro->_favouriteDrink.empty())
			_gyro->_favouriteDrink.clear();
		uint16 like2drinkSize = 0;
		char actChr = ' ';
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->_favouriteDrink += actChr;
		}

		if (!_gyro->_favouriteSong.empty())
			_gyro->_favouriteSong.clear();
		uint16 favourite_songSize = 0;
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->_favouriteSong += actChr;
		}

		if (!_gyro->_worstPlaceOnEarth.empty())
			_gyro->_worstPlaceOnEarth.clear();
		uint16 worst_place_on_earthSize = 0;
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->_worstPlaceOnEarth += actChr;
		}

		if (!_gyro->_spareEvening.empty())
			_gyro->_spareEvening.clear();
		uint16 spare_eveningSize = 0;
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			sz.syncAsByte(actChr);
			_gyro->_spareEvening += actChr;
		}
	}

	sz.syncAsSint32LE(_gyro->_totalTime);
	sz.syncAsByte(_gyro->_jumpStatus);
	sz.syncAsByte(_gyro->_mushroomGrowing);
	sz.syncAsByte(_gyro->_spludwickAtHome);
	sz.syncAsByte(_gyro->_lastRoom);
	sz.syncAsByte(_gyro->_lastRoomNotMap);
	sz.syncAsByte(_gyro->_crapulusWillTell);
	sz.syncAsByte(_gyro->_enterCatacombsFromLustiesRoom);
	sz.syncAsByte(_gyro->_teetotal);
	sz.syncAsByte(_gyro->_malagauche);
	sz.syncAsByte(_gyro->_drinking);
	sz.syncAsByte(_gyro->_enteredLustiesRoomAsMonk);
	sz.syncAsByte(_gyro->_catacombX);
	sz.syncAsByte(_gyro->_catacombY);
	sz.syncAsByte(_gyro->_avvysInTheCupboard);
	sz.syncAsByte(_gyro->_geidaFollows);
	sz.syncAsByte(_gyro->_geidaSpin);
	sz.syncAsByte(_gyro->_geidaTime);
	sz.syncAsByte(_gyro->_nextBell);
	sz.syncAsByte(_gyro->_givenPotionToGeida);
	sz.syncAsByte(_gyro->_lustieIsAsleep);
	sz.syncAsByte(_gyro->_flipToWhere);
	sz.syncAsByte(_gyro->_flipToPed);
	sz.syncAsByte(_gyro->_beenTiedUp);
	sz.syncAsByte(_gyro->_sittingInPub);
	sz.syncAsByte(_gyro->_spurgeTalkCount);
	sz.syncAsByte(_gyro->_metAvaroid);
	sz.syncAsByte(_gyro->_takenMushroom);
	sz.syncAsByte(_gyro->_givenPenToAyles);
	sz.syncAsByte(_gyro->_askedDogfoodAboutNim);


#if 0
	for (int groi = 0; groi < numtr; groi++) {
		if (tr[groi].quick) {
			blockwrite(f, groi, 1);
			tr[groi].savedata(f);
		}
	}
#endif

	byte spriteNum = 0;
	if (sz.isSaving()) {
		for (int i = 0; i < _animation->kSpriteNumbMax; i++) {
			if (_animation->_sprites[i]._quick)
				spriteNum++;
		}
	}
	sz.syncAsByte(spriteNum);

	if (sz.isLoading()) {
		for (int i = 0; i < _animation->kSpriteNumbMax; i++) { // Deallocate sprites.
			AnimationType *spr = &_animation->_sprites[i];
			if (spr->_quick)
				spr->remove();
		}
	}

	for (int i = 0; i < spriteNum; i++) {
		AnimationType *spr = &_animation->_sprites[i];
		sz.syncAsByte(spr->_id);
		sz.syncAsByte(spr->_doCheck);

		if (sz.isLoading()) {
			spr->_quick = true;
			spr->init(spr->_id, spr->_doCheck, _animation);
		}

		sz.syncAsByte(spr->_moveX);
		sz.syncAsByte(spr->_moveY);
		sz.syncAsByte(spr->_facingDir);
		sz.syncAsByte(spr->_stepNum);
		sz.syncAsByte(spr->_visible);
		sz.syncAsByte(spr->_homing);
		sz.syncAsByte(spr->_count);
		sz.syncAsByte(spr->_info._xWidth);
		sz.syncAsByte(spr->_speedX);
		sz.syncAsByte(spr->_speedY);
		sz.syncAsByte(spr->_animCount);
		sz.syncAsSint16LE(spr->_homingX);
		sz.syncAsSint16LE(spr->_homingY);
		sz.syncAsByte(spr->_callEachStepFl);
		sz.syncAsByte(spr->_eachStepProc);
		sz.syncAsByte(spr->_vanishIfStill);
		sz.syncAsSint16LE(spr->_x);
		sz.syncAsSint16LE(spr->_y);

		if (sz.isLoading() && spr->_visible)
			spr->appear(spr->_x, spr->_y, spr->_facingDir);
	}

	//groi = 177;
	//blockwrite(f, groi, 1);

	//blockwrite(f, times, sizeof(times)); // Timeout.times: Timers.
	for (int i = 0; i < 7; i++) {
		sz.syncAsSint32LE(_timer->_times[i]._timeLeft);
		sz.syncAsByte(_timer->_times[i]._action);
		sz.syncAsByte(_timer->_times[i]._reason);
	}

	//blockwrite(f, seq, sizeof(seq)); // Sequencer information.
	sz.syncBytes(_sequence->_seq, _sequence->kSeqLength);
}

bool AvalancheEngine::canSaveGameStateCurrently() { // TODO: Refine these!!!
	return (!_gyro->_seeScroll && _gyro->_alive);
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
	return (!_gyro->_seeScroll);
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
	for (int i = 0; i < 4; i++)
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

	_gyro->_isLoaded = true;
	_gyro->_seeScroll = true;  // This prevents display of the new sprites before the new picture is loaded.

	if (_gyro->_holdTheDawn) {
		_gyro->_holdTheDawn = false;
		_lucerna->dawn();
	}

	_celer->forgetBackgroundSprites();
	_lucerna->minorRedraw();
	_menu->setup();
	_gyro->_whereIs[Gyro::kPeopleAvalot - 150] = _gyro->_room;
	_gyro->_alive = true;
	_lucerna->refreshObjectList();
	_animation->updateSpeed();
	_lucerna->drawDirection();
	_gyro->_onToolbar = false;
	_animation->animLink();
	_celer->updateBackgroundSprites();

	Common::String tmpStr = Common::String::format("%cLoaded: %c%s.ASG%c%c%c%s%c%csaved on %s.", 
		Scrolls::kControlItalic, Scrolls::kControlRoman, description.c_str(), Scrolls::kControlCenter, 
		Scrolls::kControlNewLine, Scrolls::kControlNewLine, _gyro->_roomnName.c_str(), Scrolls::kControlNewLine, 
		Scrolls::kControlNewLine, expandDate(t.tm_mday, t.tm_mon, t.tm_year).c_str());
	_scrolls->displayText(tmpStr);

	AnimationType *avvy = &_animation->_sprites[0];
	if (avvy->_quick && avvy->_visible)
		_animation->changeDirection(0, _animation->_direction); // We push Avvy in the right direction is he was moving.

	return true;
}

Common::String AvalancheEngine::expandDate(int d, int m, int y) {
	static const Common::String months[12] = {
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

	Common::String month = months[m];
	Common::String day = _gyro->intToStr(d);

	if (((1 <= d) && (d <= 9)) || ((21 <= d) && (d <= 31)))
		switch (d % 10) {
		case 1:
			day += "st";
			break;
		case 2:
			day += "nd";
			break;
		case 3:
			day += "rd";
			break;
		default:
			day += "th";
		}

	return day + ' ' + month + ' ' + _gyro->intToStr(y + 1900);
}

void AvalancheEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_lucerna->_holdLeftMouse = true; // Used in Lucerna::checkclick() and Dropdown::menu_link().
			break;
		case Common::EVENT_LBUTTONUP:
			_lucerna->_holdLeftMouse = false; // Same as above.
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

void AvalancheEngine::bFlightOn() {
	_storage._skellern = kReset;
	// setintvec(0x1c, &b_flight);
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

void AvalancheEngine::runAvalot() {
	bFlightOn();

	_avalot->run(Common::String(kRuncodes[_firstTime]) + _arguments);
	// TODO: Check if parameters are ever used (probably not) and eventually remove them.
	// If there's an error initializing avalot, i'll handle it in there, not here

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

	} while (!_gyro->_letMeOut && !shouldQuit());

	return Common::kNoError;
}

#if 0
void AvalancheEngine::run(Common::String what, bool withJsb, bool withBflight, Elm how) {
	warning("STUB: run(%s)", what.c_str());
	// Probably there'll be no need of this function, as all *.AVX-es will become classes.
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

void AvalancheEngine::runDemo() {
	warning("STUB: runDemo()");
}

void AvalancheEngine::dosShell() {
	warning("STUB: dosShell()");
}

//TODO: Remove these (b_flight) functions later ( https://github.com/tthurman/avalot/wiki/B-Flight )
void AvalancheEngine::bFlight() {   //interrupt;
	_storage._skellern++;
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

// The original ones were all commented out, so probably there's no need
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

#endif

} // End of namespace Avalanche
