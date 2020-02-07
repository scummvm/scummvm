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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"

#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"

namespace Avalanche {

AvalancheEngine::AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd) : Engine(syst), _gameDescription(gd), _fxHidden(false), _interrogation(0) {
	_system = syst;
	setDebugger(new AvalancheConsole(this));

	_rnd = new Common::RandomSource("avalanche");
	_showDebugLines = false;

	_clock = nullptr;
	_graphics = nullptr;
	_parser = nullptr;
	_dialogs = nullptr;
	_background = nullptr;
	_sequence = nullptr;
	_timer = nullptr;
	_animation = nullptr;
	_dropdown = nullptr;
	_closing = nullptr;
	_sound = nullptr;
	_nim = nullptr;
	_ghostroom = nullptr;
	_help = nullptr;
	_highscore = nullptr;

	initVariables();
}

AvalancheEngine::~AvalancheEngine() {
	delete _rnd;

	delete _graphics;
	delete _parser;

	delete _clock;
	delete _dialogs;
	delete _background;
	delete _sequence;
	delete _timer;
	delete _animation;
	delete _dropdown;
	delete _closing;
	delete _sound;
	delete _nim;
	delete _ghostroom;
	delete _help;
	delete _highscore;

	for (int i = 0; i < 31; i++) {
		for (int j = 0; j < 2; j++) {
			if (_also[i][j] != nullptr)  {
				delete _also[i][j];
				_also[i][j] = nullptr;
			}
		}
	}
}

void AvalancheEngine::initVariables() {
	for (int i = 0; i < 31; i++) {
		_also[i][0] = nullptr;
		_also[i][1] = nullptr;
	}

	memset(_fxPal, 0, 16 * 16 * 3);

	for (int i = 0; i < 15; i++) {
		_peds[i]._direction = kDirNone;
		_peds[i]._x = 0;
		_peds[i]._y = 0;
		_magics[i]._operation = kMagicNothing;
		_magics[i]._data = 0;
	}

	for (int i = 0; i < 7; i++) {
		_portals[i]._operation = kMagicNothing;
		_portals[i]._data = 0;
	}

	for (int i = 0; i < 30; i++) {
		_fields[i]._x1 = 0;
		_fields[i]._y1 = 0;
		_fields[i]._x2 = 0;
		_fields[i]._y2 = 0;
	}

	_fieldNum = 0;
	_cp = 0;
	_ledStatus = 177;
	_alive = false;
	_subjectNum = 0;
	_him = kPeoplePardon;
	_her = kPeoplePardon;
	_it = Parser::kPardon;
	_roomCycles = 0;
	_doingSpriteRun = false;
	_isLoaded = false;
	_soundFx = true;
	_holdTheDawn = false;

	_lineNum = 0;
	for (int i = 0; i < 50; i++)
		_lines[i]._color = kColorWhite;
	_dropsOk = false;
	_cheat = false;
	_letMeOut = false;
	_thinks = 2;
	_thinkThing = true;
	_animationsEnabled = true;
	_currentMouse = 177;
	_holdLeftMouse = false;

	resetVariables();
}

Common::ErrorCode AvalancheEngine::initialize() {
	_graphics = new GraphicManager(this);
	_parser = new Parser(this);

	_clock = new Clock(this);
	_dialogs = new Dialogs(this);
	_background = new Background(this);
	_sequence = new Sequence(this);
	_timer = new Timer(this);
	_animation = new Animation(this);
	_dropdown = new DropDownMenu(this);
	_closing = new Closing(this);
	_sound = new SoundHandler(this);
	_nim = new Nim(this);
	_ghostroom = new GhostRoom(this);
	_help = new Help(this);
	_highscore = new HighScore(this);

	_graphics->init();
	_dialogs->init();
	init();
	_parser->init();

	return Common::kNoError;
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
	_nim->synchronize(sz);
	_sequence->synchronize(sz);
	_background->synchronize(sz);

	sz.syncAsByte(_carryNum);
	for (int i = 0; i < kObjectNum; i++)
		sz.syncAsByte(_objects[i]);
	sz.syncAsSint16LE(_score);
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
		uint16 like2drinkSize = _favoriteDrink.size();
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			char actChr = _favoriteDrink[i];
			sz.syncAsByte(actChr);
		}

		uint16 favoriteSongSize = _favoriteSong.size();
		sz.syncAsUint16LE(favoriteSongSize);
		for (uint16 i = 0; i < favoriteSongSize; i++) {
			char actChr = _favoriteSong[i];
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
		if (!_favoriteDrink.empty())
			_favoriteDrink.clear();
		uint16 like2drinkSize = 0;
		char actChr = ' ';
		sz.syncAsUint16LE(like2drinkSize);
		for (uint16 i = 0; i < like2drinkSize; i++) {
			sz.syncAsByte(actChr);
			_favoriteDrink += actChr;
		}

		if (!_favoriteSong.empty())
			_favoriteSong.clear();
		uint16 favoriteSongSize = 0;
		sz.syncAsUint16LE(favoriteSongSize);
		for (uint16 i = 0; i < favoriteSongSize; i++) {
			sz.syncAsByte(actChr);
			_favoriteSong += actChr;
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

bool AvalancheEngine::canSaveGameStateCurrently() {
	return (_animationsEnabled && _alive);
}

Common::Error AvalancheEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return (saveGame(slot, desc) ? Common::kNoError : Common::kWritingFailed);
}

bool AvalancheEngine::saveGame(const int16 slot, const Common::String &desc) {
	Common::String fileName = getSaveStateName(slot);
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

	_totalTime += getTimeInSeconds() - _startTime;

	Common::Serializer sz(NULL, f);
	synchronize(sz);
	f->finalize();
	delete f;

	return true;
}

bool AvalancheEngine::canLoadGameStateCurrently() {
	return (_animationsEnabled);
}

Common::Error AvalancheEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool AvalancheEngine::loadGame(const int16 slot) {
	Common::String fileName = getSaveStateName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);
	if (!f)
		return false;

	uint32 signature = f->readUint32LE();
	if (signature != MKTAG('A', 'V', 'A', 'L'))
		return false;

	// Check version. We can't restore from obsolete versions.
	byte saveVersion = f->readByte();
	if (saveVersion > kSavegameVersion) {
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

	resetAllVariables();

	Common::Serializer sz(f, NULL);
	synchronize(sz);
	delete f;

	_isLoaded = true;

	_animationsEnabled = false;

	if (_holdTheDawn) {
		_holdTheDawn = false;
		fadeIn();
	}

	_background->release();
	minorRedraw();
	_dropdown->setup();
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

	AnimationType *avvy = _animation->_sprites[0];
	if (avvy->_quick && avvy->_visible)
		_animation->setMoveSpeed(0, _animation->getDirection()); // We push Avvy in the right direction is he was moving.

	return true;
}

Common::String AvalancheEngine::expandDate(int d, int m, int y) {
	static const char months[12][10] = {
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

	Common::String month = Common::String(months[m]);
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

uint32 AvalancheEngine::getTimeInSeconds() {
	TimeDate time;
	_system->getTimeAndDate(time);
	return time.tm_hour * 3600 + time.tm_min * 60 + time.tm_sec;
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
	} while (!_letMeOut && !shouldQuit());

	return Common::kNoError;
}

} // End of namespace Avalanche
