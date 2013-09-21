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
	delete _dialogs;
	delete _background;
	delete _sequence;
	delete _timer;
	delete _animation;
	delete _menu;
	delete _closing;
	delete _sound;
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new Graphics(this);
	_parser = new Parser(this);

	_avalot = new Avalot(this);
	_pingo = new Pingo(this);
	_dialogs = new Dialogs(this);
	_background = new Background(this);
	_sequence = new Sequence(this);
	_timer = new Timer(this);
	_animation = new Animation(this);
	_menu = new Menu(this);
	_closing = new Closing(this);
	_sound = new SoundHandler(this);

	_graphics->init();
	_dialogs->init();
	_avalot->init();
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
	sz.syncAsByte(_avalot->_carryNum);
	for (int i = 0; i < kObjectNum; i++)
		sz.syncAsByte(_avalot->_objects[i]);
	sz.syncAsSint16LE(_avalot->_dnascore);
	sz.syncAsSint32LE(_avalot->_money);
	sz.syncAsByte(_avalot->_room);
	sz.syncAsByte(_avalot->_wearing);
	sz.syncAsByte(_avalot->_sworeNum);
	sz.syncAsByte(_avalot->_saveNum);
	sz.syncBytes(_avalot->_roomCount, 100);
	sz.syncAsByte(_avalot->_alcoholLevel);
	sz.syncAsByte(_avalot->_playedNim);
	sz.syncAsByte(_avalot->_wonNim);
	sz.syncAsByte(_avalot->_wineState);
	sz.syncAsByte(_avalot->_cwytalotGone);
	sz.syncAsByte(_avalot->_passwordNum);
	sz.syncAsByte(_avalot->_aylesIsAwake);
	sz.syncAsByte(_avalot->_drawbridgeOpen);
	sz.syncAsByte(_avalot->_avariciusTalk);
	sz.syncAsByte(_avalot->_boughtOnion);
	sz.syncAsByte(_avalot->_rottenOnion);
	sz.syncAsByte(_avalot->_onionInVinegar);
	sz.syncAsByte(_avalot->_givenToSpludwick);
	sz.syncAsByte(_avalot->_brummieStairs);
	sz.syncAsByte(_avalot->_cardiffQuestionNum);
	sz.syncAsByte(_avalot->_passedCwytalotInHerts);
	sz.syncAsByte(_avalot->_avvyIsAwake);
	sz.syncAsByte(_avalot->_avvyInBed);
	sz.syncAsByte(_avalot->_userMovesAvvy);
	sz.syncAsByte(_avalot->_npcFacing);
	sz.syncAsByte(_avalot->_givenBadgeToIby);
	sz.syncAsByte(_avalot->_friarWillTieYouUp);
	sz.syncAsByte(_avalot->_tiedUp);
	sz.syncAsByte(_avalot->_boxContent);
	sz.syncAsByte(_avalot->_talkedToCrapulus);
	sz.syncAsByte(_avalot->_jacquesState);
	sz.syncAsByte(_avalot->_bellsAreRinging);
	sz.syncAsByte(_avalot->_standingOnDais);
	sz.syncAsByte(_avalot->_takenPen);
	sz.syncAsByte(_avalot->_arrowTriggered);
	sz.syncAsByte(_avalot->_arrowInTheDoor);

	if (sz.isSaving()) {
		uint16 like2drinkSize = _avalot->_favouriteDrink.size();
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr = _avalot->_favouriteDrink[i];
			sz.syncAsByte(actChr);
		}

		uint16 favourite_songSize = _avalot->_favouriteSong.size();
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			char actChr = _avalot->_favouriteSong[i];
			sz.syncAsByte(actChr);
		}

		uint16 worst_place_on_earthSize = _avalot->_worstPlaceOnEarth.size();
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			char actChr = _avalot->_worstPlaceOnEarth[i];
			sz.syncAsByte(actChr);
		}

		uint16 spare_eveningSize = _avalot->_spareEvening.size();
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			char actChr = _avalot->_spareEvening[i];
			sz.syncAsByte(actChr);
		}
	} else {
		if (!_avalot->_favouriteDrink.empty())
			_avalot->_favouriteDrink.clear();
		uint16 like2drinkSize = 0;
		char actChr = ' ';
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			sz.syncAsByte(actChr);
			_avalot->_favouriteDrink += actChr;
		}

		if (!_avalot->_favouriteSong.empty())
			_avalot->_favouriteSong.clear();
		uint16 favourite_songSize = 0;
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			sz.syncAsByte(actChr);
			_avalot->_favouriteSong += actChr;
		}

		if (!_avalot->_worstPlaceOnEarth.empty())
			_avalot->_worstPlaceOnEarth.clear();
		uint16 worst_place_on_earthSize = 0;
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			sz.syncAsByte(actChr);
			_avalot->_worstPlaceOnEarth += actChr;
		}

		if (!_avalot->_spareEvening.empty())
			_avalot->_spareEvening.clear();
		uint16 spare_eveningSize = 0;
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			sz.syncAsByte(actChr);
			_avalot->_spareEvening += actChr;
		}
	}

	sz.syncAsSint32LE(_avalot->_totalTime);
	sz.syncAsByte(_avalot->_jumpStatus);
	sz.syncAsByte(_avalot->_mushroomGrowing);
	sz.syncAsByte(_avalot->_spludwickAtHome);
	sz.syncAsByte(_avalot->_lastRoom);
	sz.syncAsByte(_avalot->_lastRoomNotMap);
	sz.syncAsByte(_avalot->_crapulusWillTell);
	sz.syncAsByte(_avalot->_enterCatacombsFromLustiesRoom);
	sz.syncAsByte(_avalot->_teetotal);
	sz.syncAsByte(_avalot->_malagauche);
	sz.syncAsByte(_avalot->_drinking);
	sz.syncAsByte(_avalot->_enteredLustiesRoomAsMonk);
	sz.syncAsByte(_avalot->_catacombX);
	sz.syncAsByte(_avalot->_catacombY);
	sz.syncAsByte(_avalot->_avvysInTheCupboard);
	sz.syncAsByte(_avalot->_geidaFollows);
	sz.syncAsByte(_avalot->_geidaSpin);
	sz.syncAsByte(_avalot->_geidaTime);
	sz.syncAsByte(_avalot->_nextBell);
	sz.syncAsByte(_avalot->_givenPotionToGeida);
	sz.syncAsByte(_avalot->_lustieIsAsleep);
	sz.syncAsByte(_avalot->_flipToWhere);
	sz.syncAsByte(_avalot->_flipToPed);
	sz.syncAsByte(_avalot->_beenTiedUp);
	sz.syncAsByte(_avalot->_sittingInPub);
	sz.syncAsByte(_avalot->_spurgeTalkCount);
	sz.syncAsByte(_avalot->_metAvaroid);
	sz.syncAsByte(_avalot->_takenMushroom);
	sz.syncAsByte(_avalot->_givenPenToAyles);
	sz.syncAsByte(_avalot->_askedDogfoodAboutNim);


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
	return (!_avalot->_seeScroll && _avalot->_alive);
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
	return (!_avalot->_seeScroll);
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

	_avalot->_isLoaded = true;
	_avalot->_seeScroll = true;  // This prevents display of the new sprites before the new picture is loaded.

	if (_avalot->_holdTheDawn) {
		_avalot->_holdTheDawn = false;
		_avalot->dawn();
	}

	_background->forgetBackgroundSprites();
	_avalot->minorRedraw();
	_menu->setup();
	_avalot->_whereIs[Avalot::kPeopleAvalot - 150] = _avalot->_room;
	_avalot->_alive = true;
	_avalot->refreshObjectList();
	_animation->updateSpeed();
	_avalot->drawDirection();
	_avalot->_onToolbar = false;
	_animation->animLink();
	_background->updateBackgroundSprites();

	Common::String tmpStr = Common::String::format("%cLoaded: %c%s.ASG%c%c%c%s%c%csaved on %s.", 
		Dialogs::kControlItalic, Dialogs::kControlRoman, description.c_str(), Dialogs::kControlCenter, 
		Dialogs::kControlNewLine, Dialogs::kControlNewLine, _avalot->_roomnName.c_str(), Dialogs::kControlNewLine, 
		Dialogs::kControlNewLine, expandDate(t.tm_mday, t.tm_mon, t.tm_year).c_str());
	_dialogs->displayText(tmpStr);

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
	Common::String day = _avalot->intToStr(d);

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

	return day + ' ' + month + ' ' + _avalot->intToStr(y + 1900);
}

void AvalancheEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_avalot->_holdLeftMouse = true; // Used in Lucerna::checkclick() and Dropdown::menu_link().
			break;
		case Common::EVENT_LBUTTONUP:
			_avalot->_holdLeftMouse = false; // Same as above.
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

Common::Error AvalancheEngine::run() {
	Common::ErrorCode err = initialize();
	if (err != Common::kNoError)
		return err;

	do {
		_avalot->runAvalot();

#if 0
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
#endif

	} while (!_avalot->_letMeOut && !shouldQuit());

	return Common::kNoError;
}

#if 0
void AvalancheEngine::run(Common::String what, bool withJsb, bool withBflight, Elm how) {
	warning("STUB: run(%s)", what.c_str());
	// Probably there'll be no need of this function, as all *.AVX-es will become classes.
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

// Same as keypressed1().
void AvalancheEngine::flushBuffer() {
	warning("STUB: flushBuffer()");
}

void AvalancheEngine::dosShell() {
	warning("STUB: dosShell()");
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

// Needed later.
void AvalancheEngine::quit() {
	cursorOn();
}

#endif

} // End of namespace Avalanche
