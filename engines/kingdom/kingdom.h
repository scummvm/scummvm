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
		int _FrameStop;
		int _DaelonCntr;
		bool _Spell1;
		bool _Spell2;
		bool _Spell3;
		bool _ItemInhibit;
		bool _ASMode;
		bool _ATimerFlag;
		bool _BTimerFlag; // TODO: this flag is always false: to be refactored
		bool _CTimerFlag; // TODO: this flag is always false: to be refactored
		bool _SkylarTimerFlag; // TODO: this flag is always false: to be refactored
		int _ATimer;
		int _BTimer;
		int _CTimer;
		int _SkylarTimer;
		bool _TideCntl;
		bool _MapEx;
		int _Health;
		int _HealthOld;
		int _HealthTmr;
		int _TreeEyeTimer;
		int _TreeEyePic;
		int _TreeEyeSta;
		int _TreeHGTimer;
		int _TreeHGPic;
		int _TreeHGUPic;
		int _TreeLeftPic;
		int _TreeRightPic;
		int _TreeRightSta;
		int _TreeHGSta;
		bool _TSIconOnly;
		bool _LastObs;
		bool _Pouch;
		bool _NoMusic;
		byte *_ASPtr;
		int _ASMap;
		int _CurrMap;
		int _OldTLS;
		int _OldTRS;
		int _TreeLeftSta;
		bool _IconsClosed;
		bool _OldIconsClosed;
		bool _OldEye;
		bool _Eye;
		bool _OldHelp;
		bool _Help;
		bool _OldPouch;
		int _PMovie;
		bool _KeyActive;
		bool _IconRedraw;
		bool _Replay;
		bool _quit;
		int _NodeNum;
		int _OldNode;
		int _NextNode;
		int _RobberyNode;
		bool _CursorDrawn; // CHECKME: Useless
		bool _Wizard;
		int _Zoom;
		bool _Resurrect;
		int _LastObstacle;
		int _MapStat;
		int _MouseValue;
		int _CursorDef;
		int _OldCursorDef;
		int _CursorX;
		int _CursorY;
		int _OldCursorX;
		int _OldCursorY;
		int _IconSel;
		int _IconSelect;
		bool _MouseDebound;
		int _RtnNode;
		int _SoundNumber;
		bool _PalStepFlag;
		Audio::SoundHandle _soundHandle;
		int _tickCount;
		uint32 _oldTime;

		// Game Flags - Will be renames later into _Nodes[]
		int16 _Nodes[99];

		Common::SeekableReadStream *_RezPointers[510];
		int _RezSize[510];
		int _Inventory[19];
		int _IconPic[7];
		uint16 _UserInput;
		uint16 _MouseButton;

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
	};
} // End of namespace Kingdom

#endif
