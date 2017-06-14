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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/random.h"
#include "engines/advancedDetector.h"
#include "common/error.h"
#include "engines/engine.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "image/bmp.h"

#include "kingdom/console.h"

namespace Kingdom {
	enum KingdomDebugChannels {
		kDebugGeneral = 1 << 0
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

	private:
		Console *_console;
		Common::RandomSource *_rnd;

		bool _LoopFlag;
		int _GameMode;
		int _StatPlay;
		bool _FstFwd;
		bool _NoIFScreen;
		bool _Sound;
		bool _LastSound;
		int _FrameStop;
		int _DaelonCntr;
		bool _Spell1;
		bool _Spell2;
		bool _Spell3;
		bool _ItemInhibit;
		bool _ASMode;
		bool _ATimerFlag;
		bool _BTimerFlag;
		bool _CTimerFlag;
		bool _SkylarTimerFlag;
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
		int _TreeHGTimer;
		int _TreeHGUPic;
		int _TreeLeftPic;
		int _TreeRightPic;
		int _TreeRightSta;
		bool _TSIconOnly;
		bool _LastObs;
		bool _Pouch;
		bool _NoMusic;
		byte *_ASPtr;
		int _ASMap;
		int _CurrMap;
		int _OldTLS;
		int _TreeLeftSta;
		bool _IconsClosed;
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
		bool _CursorDrawn;
		bool _Wizard;
		int _Zoom;
		int _Pmovie;
		bool _Resurrect;
		bool _SaveFile;
		int _LastObstacle;

		// Game Flags - Will be renames later into _Nodes[]
		int16 word_2D74C;
		int16 word_2D74E;
		int16 word_2D750;
		int16 word_2D752;
		int16 word_2D754;
		int16 word_2D756;
		int16 word_2D758;
		int16 word_2D75A;
		int16 word_2D75E;
		int16 word_2D764;
		int16 word_2D766;
		int16 word_2D768;
		int16 word_2D76A;
		int16 word_2D76E;
		int16 word_2D770;
		int16 word_2D772;
		int16 word_2D776;
		int16 word_2D77A;
		int16 word_2D77C;
		int16 word_2D77E;
		int16 word_2D780;
		int16 word_2D784;
		int16 word_2D786;
		int16 word_2D78A;
		int16 word_2D78C;
		int16 word_2D78E;
		int16 word_2D790;
		int16 word_2D792;
		int16 word_2D796;
		int16 word_2D79A;
		int16 word_2D79C;
		int16 word_2D7A2;
		int16 word_2D7A6;
		int16 word_2D7AA;
		int16 word_2D7BA;
		int16 word_2D7CC;

		Common::SeekableReadStream *_ArtPtr;
		Common::SeekableReadStream *_RezPointers[510];
		int _RezSize[510];
		int8 _Inventory[19];
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
		void LoadAResource(int reznum);
		void ReleaseAResource(int reznum);
		void ShowPic(int reznum);
		void FShowPic(int reznum);
		void InitCursor();
		void SetMouse();
		void InitMPlayer();
		void PlayMovie(int movieNum);
		void EnAll();
		void DsAll();
		void SaveAS();
		void RestoreAS();
		void DrawHelpScreen();
		void DrawRect(int v1, int v2, int v3, int v4, int v5);
		void DrawInventory();
		void GameHelp_Sub43C();
		void SaveGame();
		void PlaySound(int v1);
		void EraseCursor();
		void GetUserInput();
		void EraseCursorAsm();
		void RestoreGame();
		void DrawLocation();
		void ProcessMap(int mapNum, int zoom);
		void ProcessMapInput(int mapNum);
		void InventoryDel(int item);
		void InventoryAdd(int item);
		void DrawPic(int reznum);
		void DisplayIcon(int reznum);
		void SetATimer();
		bool Wound();
		void ReadMouse();
		void RefreshSound();
		void SwitchAS();
		void IncreaseHealth();
		void CheckSaveGame();
		void CheckMainScreen();
		bool ChkDesertObstacles();
	};
} // End of namespace Kingdom

#endif
