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

#include "avalanche/console.h"

#include "avalanche/graphics.h"
#include "avalanche/parser.h"

#include "avalanche/avalot.h"
#include "avalanche/gyro2.h"
#include "avalanche/pingo2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/visa2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/enid2.h"
#include "avalanche/celer2.h"
#include "avalanche/sequence2.h"
#include "avalanche/timeout2.h"
#include "avalanche/trip6.h"
#include "avalanche/acci2.h"
#include "avalanche/dropdown2.h"
#include "avalanche/closing2.h"

#include "common/serializer.h"

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "graphics/cursorman.h"

namespace Common {
class RandomSource;
}

namespace Avalanche {

struct AvalancheGameDescription;

static const int kSavegameVersion = 1;

class AvalancheEngine : public Engine {
public:
	Graphics *_graphics;
	Parser *_parser;

	Avalot *_avalot;
	Gyro *_gyro;
	Pingo *_pingo;
	Scrolls *_scrolls;
	Visa *_visa;
	Lucerna *_lucerna;
	Enid *_enid;
	Celer *_celer;
	Sequence *_sequence;
	Timeout *_timeout;
	Trip *_trip;
	Acci *_acci;
	Dropdown *_dropdown;
	Closing *_closing;

	OSystem *_system;

	AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd);
	~AvalancheEngine();

	Common::ErrorCode initialize(); 
	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;
	
	const AvalancheGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	void synchronize(Common::Serializer &sz);
	virtual bool canSaveGameStateCurrently();
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool saveGame(const int16 slot, const Common::String &desc);
	Common::String getSaveFileName(const int slot);
	virtual bool canLoadGameStateCurrently();
	Common::Error loadGameState(int slot);
	bool loadGame(const int16 slot);
	Common::String expandDate(int d, int m, int y);

	void updateEvents();
	bool getEvent(Common::Event &event); // A wrapper around _eventMan->pollEvent(), so we can use it in Scrolls::normscroll() for example.
	Common::Point getMousePos();

protected:
	// Engine APIs
	Common::Error run();

private:
	static AvalancheEngine *s_Engine;

	AvalancheConsole *_console;
	Common::Platform _platform;

	// From bootstrp:

	enum Elm {kNormal, kMusical, kElmpoyten, kRegi};

	static const int16 kRunShootemup = 1, kRunDosshell = 2, kRunGhostroom = 3, kRunGolden = 4;
	static const char kRuncodes[2][3];
	static const int16 kReset = 0;
	static const bool kJsb = true, kNoJsb = false, kBflight = true, kNoBflight = false;

	struct {
		byte _operation;
		uint16 _skellern;
		byte _contents[1000];
	} _storage;

	Common::String _arguments, _demoArgs, _argsWithNoFilename;
	bool _firstTime;
	byte _originalMode;
	byte *_old1c;
	Common::String _segofs;
	bool _zoomy;
	int32 _soundcard, _speed, _baseaddr, _irq, _dma;

	void cursorOff();
	void cursorOn();
	void quit();
	Common::String commandCom();
	void explain(byte error);
	void bFlight();
	void bFlightOn();
	void bFlightOff();
	Common::String elmToStr(Elm how);
	void run(Common::String what, bool withJsb, bool withBflight, Elm how);
	void getArguments();
	void getSlope();
	void callMenu();
	void runAvalot();
	void runDemo();
	void dosShell();
	bool keyPressed();
	void flushBuffer();
	void demo();
};

} // End of namespace Avalanche

#endif // AVALANCHE_H
