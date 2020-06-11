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

#ifndef KINGDOM_KINGDOM_H
#define KINGDOM_KINGDOM_H

#include "common/system.h"
#include "common/scummsys.h"
#include "common/random.h"
#include "engines/advancedDetector.h"
#include "common/error.h"
#include "engines/engine.h"

#include "audio/mixer.h"
#include "common/file.h"
#include "graphics/screen.h"
#include "graphics/surface.h"

#include "common/serializer.h"

#include "kingdom/console.h"
#include "kingdom/logic.h"

namespace Kingdom {
	enum KingdomDebugChannels {
		kDebugGeneral = 1 << 0
	};

	struct KingArtEntry {
		uint8 _width;
		uint8 _height;
		byte* _data;
	};

	struct HotSpot {
		int x1, y1, x2, y2;
		int16 _mouseValue;

		bool contains(const Common::Point &p) {
			return (x1 <= p.x) && (p.x < x2) && (y1 <= p.y) && (p.y < y2);
		}

		bool dummy() {
			return !(x1 || x2 || y1 || y2);
		}
	};

	extern byte _finalFrameTable[];
	extern const char *_rezNames[];
	extern const char *_movieNames[];
	extern int _mapExit[];
	extern int _emlTable[];
	extern int _zoomTable[81][9][2];
	extern int _iconActTable[82][7];
	extern int _cursorTable[96];
	extern int _teaSeq[6][2];
	extern int _hgaSeq[4][2];
	extern HotSpot _mouseMapMSFull[51];
	extern HotSpot _mouseMapMSDemo[51];
	extern HotSpot _mouseMapASFull[128][16];
	extern HotSpot _mouseMapASDemo[128][16];

	struct KingdomSavegameHeader {
		uint32 _signature;
		uint8 _version;
		Common::String _saveName;
		Graphics::Surface *_thumbnail;
		int _year, _month, _day;
		int _hour, _minute;
	};

	class KingdomGame : public Engine {
	public:
		KingdomGame(OSystem *syst, const ADGameDescription *gameDesc);
		~KingdomGame();

		virtual Common::Error run();

		// Detection related functions
		const ADGameDescription *_gameDescription;
		const char *getGameId() const;
		Common::Platform getPlatform() const;
		static bool readSavegameHeader(Common::InSaveFile *in, KingdomSavegameHeader &header);
		bool isDemo() const;

	private:
		Console *_console;
		Logic *_logic;

		KingArtEntry *_kingartEntries;
		void displayDebugHotSpots();

	public:
		Common::RandomSource *_rnd;

		bool _showHotspots;
		bool _loopFlag;
		int _gameMode;
		bool _fstFwd;
		bool _noIFScreen;
		bool _sound;
		bool _lastSound;
		bool _fullScreen;
		int _frameStop;
		int _daelonCntr;
		bool _itemInhibit;
		bool _asMode;
		bool _aTimerFlag;
		bool _bTimerFlag;
		bool _cTimerFlag;
		bool _skylarTimerFlag;
		int _aTimer;
		int _bTimer;
		int _cTimer;
		int _skylarTimer;
		bool _mapEx;
		int _healthTmr;
		int _treeEyeTimer;
		int _treeEyePic;
		int _treeEyeSta;
		int _treeHGTimer;
		int _treeHGPic;
		int _treeHGUPic;
		int _treeLeftPic;
		int _treeRightPic;
		int _treeRightSta;
		int _treeHGSta;
		bool _tsIconOnly;
		bool _noMusic; // TODO: Synchronize this flag with the launcher
		byte *_asPtr;
		int _asMap;
		int _oldTLS;
		int _oldTRS;
		int _treeLeftSta;
		bool _iconsClosed;
		bool _oldIconsClosed;
		int _pMovie;
		bool _keyActive;
		bool _iconRedraw;
		bool _quit;
		bool _cursorDrawn; // CHECKME: Useless
		bool _wizard;
		int _zoom;
		int _mouseValue;
		int _cursorDef; // TODO: Could be removed by using the return value of CursorTypeExit()
		int _oldCursorDef; // CHECKME: Unused in our implementation?

		Common::Point _cursorPos;
		Common::Point _oldCursorPos; // CHECKME: Unused in out implementation?

		int _iconSel;
		int _iconSelect;
		bool _mouseDebound;
		int _soundNumber;
		bool _palStepFlag;
		Audio::SoundHandle _soundHandle;
		int _tickCount;
		uint32 _oldTime;

		Common::SeekableReadStream *_rezPointers[510];
		int _rezSize[510];
		int _iconPic[7];
		uint16 _userInput;
		uint16 _mouseButton;

		void drawScreen();
		void setupPics();
		void initTools();
		void titlePage();
		void initPlay();
		void initHelp();
		void fadeToBlack1();
		void fadeToBlack2();
		void loadAResource(int reznum);
		void releaseAResource(int reznum);
		void showPic(int reznum);
		void fShowPic(int reznum);
		void initCursor();
		void initMouse();
		void setMouse();
		void readMouse();
		void initMPlayer();
		void playMovie(int movieNum);
		void saveAS(); // TODO: Rename later as saveVideoBackground
		void restoreAS(); // TODO: Rename later as restoreVideoBackground
		void drawHelpScreen();
		void drawRect(uint minX, uint minY, uint maxX, uint maxY, int color);
		void drawHotSpot(const HotSpot &hs, int color);
		void drawInventory();
		void playSound(int idx);
		void eraseCursor();
		void getUserInput();
		void eraseCursorAsm();
		void drawLocation();
		void processMap(int mapNum, int zoom);
		void processMapInput(int mapNum);
		void drawPic(int reznum);
		void displayIcon(int reznum);
		void setATimer();
		void refreshSound();
		void checkMainScreen();
		void switchAtoM();
		void switchMtoA();
		void drawIcon(int x, int y, int index);
		int waitKey();
		void drawCursor();
		void cursorType();
		void loadKingArt();
		void setCursor(int cursor);
		int getAKey();
		int checkMouseMapAS();
		void cursorTypeExit();
		void saveGame();
		void restoreGame();
		virtual Common::Error loadGameState(int slot);
		virtual Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);
		Common::String getSavegameFilename(int slot);
		void writeSavegameHeader(Common::OutSaveFile *out, KingdomSavegameHeader &header);
		void synchronize(Common::Serializer &s);
		void refreshScreen();
		void checkTimers();
		void initVariables();
	};
} // End of namespace Kingdom

#endif
