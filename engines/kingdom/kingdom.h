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
#include "common/config-manager.h"
#include "common/random.h"
#include "engines/advancedDetector.h"
#include "common/error.h"
#include "engines/engine.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/memstream.h"
#include "common/events.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "image/bmp.h"

#include "common/savefile.h"
#include "gui/saveload.h"
#include "graphics/thumbnail.h"
#include "common/serializer.h"

#include "kingdom/console.h"

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
		int16 _minX;
		int16 _maxX;
		int16 _minY;
		int16 _maxY;
		int16 _mouseValue;
	};

	extern const char *_rezNames[];
	extern const char *_movieNames[];
	extern int _mapExit[];
	extern int _emlTable[];
	extern int _zoomTable[81][9][2];
	extern int _iconActTable[82][7];
	extern int _cursorTable[96];
	extern int _teaSeq[6][2];
	extern int _hgaSeq[4][2];
	extern HotSpot _mouseMapMS[51];
	extern HotSpot _mouseMapAS[128][16];

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

	private:
		Console *_console;
		Common::RandomSource *_rnd;

		KingArtEntry *_kingartEntries;

		bool _loopFlag;
		int _gameMode;
		int _statPlay;
		int _oldStatPlay;
		bool _fstFwd;
		bool _noIFScreen;
		bool _sound;
		bool _lastSound;
		bool _fullScreen;
		int _frameStop;
		int _daelonCntr;
		bool _spell1;
		bool _spell2;
		bool _spell3;
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
		bool _tideCntl;
		bool _mapEx;
		int _health;
		int _healthOld;
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
		bool _lastObs;
		bool _Pouch;
		bool _noMusic; // TODO: Synchronize this flag with the launcher
		byte *_asPtr;
		int _asMap;
		int _currMap;
		int _oldTLS;
		int _oldTRS;
		int _treeLeftSta;
		bool _iconsClosed;
		bool _oldIconsClosed;
		bool _oldEye;
		bool _eye;
		bool _oldHelp;
		bool _help;
		bool _oldPouch;
		int _pMovie;
		bool _keyActive;
		bool _iconRedraw;
		bool _replay;
		bool _quit;
		int _nodeNum;
		int _oldNode; // CHECKME: Useless? If so, to be removed
		int _nextNode;
		int _robberyNode;
		bool _cursorDrawn; // CHECKME: Useless
		bool _wizard;
		int _zoom;
		bool _resurrect;
		int _lastObstacle;
		int _mapStat; // CHECKME: Useless? If so, to be removed
		int _mouseValue;
		int _cursorDef; // TODO: Could be removed by using the return value of CursorTypeExit()
		int _oldCursorDef; // CHECKME: Unused in our implementation?
		int _cursorX;
		int _cursorY;
		int _oldCursorX; // CHECKME: Unused in out implementation?
		int _oldCursorY; // CHECKME: Unused in out implementation?
		int _iconSel;
		int _iconSelect;
		bool _mouseDebound;
		int _rtnNode;
		int _soundNumber;
		bool _palStepFlag;
		Audio::SoundHandle _soundHandle;
		int _tickCount;
		uint32 _oldTime;

		int16 _nodes[99];

		Common::SeekableReadStream *_rezPointers[510];
		int _rezSize[510];
		int _inventory[19];
		int _iconPic[7];
		uint16 _userInput;
		uint16 _mouseButton;

		void drawScreen();
		void SetupPics();
		void InitTools();
		void TitlePage();
		void InitPlay();
		void InitHelp();
		void FadeToBlack1();
		void FadeToBlack2();
		void GameHelp();
		void GPLogic1();
		void GPLogic1_SubSP10();
		void GPLogic1_SubSP121();
		void GPLogic2();
		void GPLogic2_SubSP361();
		void GPLogic3();
		void GPLogic4();
		void GPLogic4_SP901();
		void LoadAResource(int reznum);
		void ReleaseAResource(int reznum);
		void ShowPic(int reznum);
		void FShowPic(int reznum);
		void InitCursor();
		void InitMouse();
		void SetMouse();
		void ReadMouse();
		void InitMPlayer();
		void PlayMovie(int movieNum);
		void EnAll();
		void DsAll();
		void SaveAS();
		void RestoreAS();
		void DrawHelpScreen();
		void DrawRect(uint v1, uint v2, uint v3, uint v4, int v5);
		void DrawInventory();
		void GameHelp_Sub43C();
		void PlaySound(int v1);
		void EraseCursor();
		void GetUserInput();
		void EraseCursorAsm();
		void DrawLocation();
		void ProcessMap(int mapNum, int zoom);
		void ProcessMapInput(int mapNum);
		void InventoryDel(int item);
		void InventoryAdd(int item);
		void DrawPic(int reznum);
		void DisplayIcon(int reznum);
		void SetATimer();
		bool Wound();
		void RefreshSound();
		void SwitchAS();
		void IncreaseHealth();
		void CheckMainScreen();
		bool ChkDesertObstacles();
		void SwitchAtoM();
		void SwitchMtoA();
		void DrawIcon(int x, int y, int index);
		int WaitKey();
		void DrawCursor();
		void CursorType();
		void LoadKingArt();
		void SetCursor(int cursor);
		int GetAKey();
		int checkMouseMapAS();
		void CursorTypeExit();
		void EndCredits();
		void saveGame();
		void restoreGame();
		virtual Common::Error loadGameState(int slot);
		virtual Common::Error saveGameState(int slot, const Common::String &desc);
		Common::String getSavegameFilename(int slot);
		void writeSavegameHeader(Common::OutSaveFile *out, KingdomSavegameHeader &header);
		void synchronize(Common::Serializer &s);
		void refreshScreen();
		void checkTimers();
		void initVariables();
	};
} // End of namespace Kingdom

#endif
