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

AvalancheEngine::AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd) : Engine(syst), _gameDescription(gd), _fxHidden(false), _interrogation(0) {
	_system = syst;
	_console = new AvalancheConsole(this);

	_rnd = new Common::RandomSource("avalanche");
	TimeDate time;
	_system->getTimeAndDate(time);
	_rnd->setSeed(time.tm_sec + time.tm_min + time.tm_hour);

	// Needed because of Lucerna::load_also()
	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++)
			_also[i][j] = nullptr;
	}

	_totalTime = 0;
	_showDebugLines = false;
}

AvalancheEngine::~AvalancheEngine() {
	delete _console;
	delete _rnd;

	delete _graphics;
	delete _parser;

	delete _clock;
	delete _pingo;
	delete _dialogs;
	delete _background;
	delete _sequence;
	delete _timer;
	delete _animation;
	delete _menu;
	delete _closing;
	delete _sound;

	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)  {
				delete _also[i][j];
				_also[i][j] = nullptr;
			}
		}
	}
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new GraphicManager(this);
	_parser = new Parser(this);

	_clock = new Clock(this);
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
	init();
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
	_animation->synchronize(sz);
	_parser->synchronize(sz);
	_sequence->synchronize(sz);
	_background->synchronize(sz);

	sz.syncAsByte(_carryNum);
	for (int i = 0; i < kObjectNum; i++)
		sz.syncAsByte(_objects[i]);
	sz.syncAsSint16LE(_dnascore);
	sz.syncAsSint32LE(_money);
	sz.syncAsByte(_room);
	if (sz.isSaving())
		_saveNum++;
	sz.syncAsByte(_saveNum);
	sz.syncBytes(_roomCount, 100);
	sz.syncAsByte(_wonNim);
	sz.syncAsByte(_wineState);
	sz.syncAsByte(_cwytalotGone);
	sz.syncAsByte(_passwordNum);
	sz.syncAsByte(_aylesIsAwake);
	sz.syncAsByte(_drawbridgeOpen);
	sz.syncAsByte(_avariciusTalk);
	sz.syncAsByte(_rottenOnion);
	sz.syncAsByte(_onionInVinegar);
	sz.syncAsByte(_givenToSpludwick);
	sz.syncAsByte(_brummieStairs);
	sz.syncAsByte(_cardiffQuestionNum);
	sz.syncAsByte(_passedCwytalotInHerts);
	sz.syncAsByte(_avvyIsAwake);
	sz.syncAsByte(_avvyInBed);
	sz.syncAsByte(_userMovesAvvy);
	sz.syncAsByte(_npcFacing);
	sz.syncAsByte(_givenBadgeToIby);
	sz.syncAsByte(_friarWillTieYouUp);
	sz.syncAsByte(_tiedUp);
	sz.syncAsByte(_boxContent);
	sz.syncAsByte(_talkedToCrapulus);
	sz.syncAsByte(_jacquesState);
	sz.syncAsByte(_bellsAreRinging);
	sz.syncAsByte(_standingOnDais);
	sz.syncAsByte(_takenPen);
	sz.syncAsByte(_arrowInTheDoor);

	if (sz.isSaving()) {
		uint16 like2drinkSize = _favouriteDrink.size();
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr = _favouriteDrink[i];
			sz.syncAsByte(actChr);
		}

		uint16 favourite_songSize = _favouriteSong.size();
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			char actChr = _favouriteSong[i];
			sz.syncAsByte(actChr);
		}

		uint16 worst_place_on_earthSize = _worstPlaceOnEarth.size();
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			char actChr = _worstPlaceOnEarth[i];
			sz.syncAsByte(actChr);
		}

		uint16 spare_eveningSize = _spareEvening.size();
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			char actChr = _spareEvening[i];
			sz.syncAsByte(actChr);
		}
	} else {
		if (!_favouriteDrink.empty())
			_favouriteDrink.clear();
		uint16 like2drinkSize = 0;
		char actChr = ' ';
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			sz.syncAsByte(actChr);
			_favouriteDrink += actChr;
		}

		if (!_favouriteSong.empty())
			_favouriteSong.clear();
		uint16 favourite_songSize = 0;
		sz.syncAsUint16LE(favourite_songSize);
		for (uint16 i = 0; i < favourite_songSize; i++) {
			sz.syncAsByte(actChr);
			_favouriteSong += actChr;
		}

		if (!_worstPlaceOnEarth.empty())
			_worstPlaceOnEarth.clear();
		uint16 worst_place_on_earthSize = 0;
		sz.syncAsUint16LE(worst_place_on_earthSize);
		for (uint16 i = 0; i < worst_place_on_earthSize; i++) {
			sz.syncAsByte(actChr);
			_worstPlaceOnEarth += actChr;
		}

		if (!_spareEvening.empty())
			_spareEvening.clear();
		uint16 spare_eveningSize = 0;
		sz.syncAsUint16LE(spare_eveningSize);
		for (uint16 i = 0; i < spare_eveningSize; i++) {
			sz.syncAsByte(actChr);
			_spareEvening += actChr;
		}
	}

	sz.syncAsSint32LE(_totalTime);
	sz.syncAsByte(_jumpStatus);
	sz.syncAsByte(_mushroomGrowing);
	sz.syncAsByte(_spludwickAtHome);
	sz.syncAsByte(_lastRoom);
	sz.syncAsByte(_lastRoomNotMap);
	sz.syncAsByte(_crapulusWillTell);
	sz.syncAsByte(_enterCatacombsFromLustiesRoom);
	sz.syncAsByte(_teetotal);
	sz.syncAsByte(_malagauche);
	sz.syncAsByte(_drinking);
	sz.syncAsByte(_enteredLustiesRoomAsMonk);
	sz.syncAsByte(_catacombX);
	sz.syncAsByte(_catacombY);
	sz.syncAsByte(_avvysInTheCupboard);
	sz.syncAsByte(_geidaFollows);
	sz.syncAsByte(_givenPotionToGeida);
	sz.syncAsByte(_lustieIsAsleep);
	sz.syncAsByte(_beenTiedUp);
	sz.syncAsByte(_sittingInPub);
	sz.syncAsByte(_spurgeTalkCount);
	sz.syncAsByte(_metAvaroid);
	sz.syncAsByte(_takenMushroom);
	sz.syncAsByte(_givenPenToAyles);
	sz.syncAsByte(_askedDogfoodAboutNim);

	for (int i = 0; i < 7; i++) {
		sz.syncAsSint32LE(_timer->_times[i]._timeLeft);
		sz.syncAsByte(_timer->_times[i]._action);
		sz.syncAsByte(_timer->_times[i]._reason);
	}
	
}

bool AvalancheEngine::canSaveGameStateCurrently() { // TODO: Refine these!!!
	return (!_seeScroll && _alive);
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

	f->writeUint32LE(MKTAG('A', 'V', 'A', 'L'));

	// Write version. We can't restore from obsolete versions.
	f->writeByte(kSavegameVersion);

	f->writeUint32LE(desc.size());
	f->write(desc.c_str(), desc.size());
	Graphics::saveThumbnail(*f);

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
	return Common::String::format("%s.%03d", _targetName, slot);
}

bool AvalancheEngine::canLoadGameStateCurrently() { // TODO: Refine these!!!
	return (!_seeScroll);
}

Common::Error AvalancheEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool AvalancheEngine::loadGame(const int16 slot) {
	Common::String fileName = getSaveFileName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);
	if (!f)
		return false;

	uint32 signature = f->readUint32LE();
	if (signature != MKTAG('A', 'V', 'A', 'L'))
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
	Graphics::skipThumbnail(*f);

	// Read the time the game was saved.
	TimeDate t;
	t.tm_mday = f->readSint16LE();
	t.tm_mon = f->readSint16LE();
	t.tm_year = f->readSint16LE();

	Common::Serializer sz(f, NULL);
	synchronize(sz);
	delete f;

	_isLoaded = true;
	_seeScroll = true;  // This prevents display of the new sprites before the new picture is loaded.

	if (_holdTheDawn) {
		_holdTheDawn = false;
		dawn();
	}

	_background->release();
	minorRedraw();
	_menu->setup();
	setRoom(kPeopleAvalot, _room);
	_alive = true;
	refreshObjectList();
	_animation->updateSpeed();
	drawDirection();
	_animation->animLink();
	_background->update();

	Common::String tmpStr = Common::String::format("%cLoaded: %c%s.ASG%c%c%c%s%c%csaved on %s.", 
		kControlItalic, kControlRoman, description.c_str(), kControlCenter, kControlNewLine, 
		kControlNewLine, _roomnName.c_str(), kControlNewLine, kControlNewLine, 
		expandDate(t.tm_mday, t.tm_mon, t.tm_year).c_str());
	_dialogs->displayText(tmpStr);

	AnimationType *avvy = &_animation->_sprites[0];
	if (avvy->_quick && avvy->_visible)
		_animation->setMoveSpeed(0, _animation->getDirection()); // We push Avvy in the right direction is he was moving.

	return true;
}

Common::String AvalancheEngine::expandDate(int d, int m, int y) {
	static const Common::String months[12] = {
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

	Common::String month = months[m];
	Common::String day = intToStr(d);

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

	return day + ' ' + month + ' ' + intToStr(y + 1900);
}

void AvalancheEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			_holdLeftMouse = true; // Used in checkclick() and Menu::menu_link().
			break;
		case Common::EVENT_LBUTTONUP:
			_holdLeftMouse = false; // Same as above.
			break;
		case Common::EVENT_KEYDOWN:
			if ((event.kbd.keycode == Common::KEYCODE_d) && (event.kbd.flags & Common::KBD_CTRL)) {
				// Attach to the debugger
				_console->attach();
				_console->onFrame();
			} else
				handleKeyDown(event);
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
		runAvalot();

#if 0
		switch (_storage._operation) {
		case kRunShootemup:
			run("seu.avx", kJsb, kBflight, kNormal);
			break;
		case kRunDosshell:
			dosShell();
			break;
		case kRunGhostroom:
			run("g-room.avx", kJsb, kNoBflight, kNormal);
			break;
		case kRunGolden:
			run("golden.avx", kJsb, kBflight, kMusical);
			break;
		}
#endif

	} while (!_letMeOut && !shouldQuit());

	return Common::kNoError;
}

#if 0
void AvalancheEngine::run(Common::String what, bool withJsb, bool withBflight, Elm how) {
	// Probably there'll be no need of this function, as all *.AVX-es will become classes.
	warning("STUB: run(%s)", what.c_str());
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
