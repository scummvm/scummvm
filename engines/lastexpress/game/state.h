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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef LASTEXPRESS_STATE_H
#define LASTEXPRESS_STATE_H

#include "lastexpress/shared.h"

#include "common/system.h"

namespace LastExpress {

class LastExpressEngine;

class Inventory;
class Objects;
class SavePoints;

class State {
public:
	struct GameProgress {
		uint32 field_0;
		JacketType jacket;
		bool eventCorpseMovedFromFloor;
		uint32 field_C;
		bool eventCorpseFound;
		uint32 field_14;                  ///< EntityIndex (used in Gendarmes)
		uint32 field_18;
		uint32 portrait;
		bool eventCorpseThrown;
		uint32 field_24;
		uint32 field_28;
		ChapterIndex chapter;
		uint32 field_30;
		bool eventMetAugust;
		bool isNightTime;
		uint32 field_3C;
		uint32 field_40;
		uint32 field_44;
		uint32 field_48;
		uint32 field_4C;
		bool isTrainRunning;
		uint32 field_54;
		uint32 field_58;
		uint32 field_5C;
		uint32 field_60;
		uint32 field_64;
		uint32 field_68;
		bool eventMertensAugustWaiting;
		bool eventMertensKronosInvitation;
		bool isEggOpen;
		uint32 field_78;	// time?
		uint32 field_7C;
		uint32 field_80;
		uint32 field_84;
		uint32 field_88;
		uint32 field_8C;
		uint32 field_90;
		uint32 field_94;
		uint32 field_98;
		uint32 field_9C;
		uint32 field_A0;
		uint32 field_A4;
		uint32 field_A8;
		uint32 field_AC;
		uint32 field_B0;
		uint32 field_B4;
		uint32 field_B8;
		uint32 field_BC;
		uint32 field_C0;
		uint32 field_C4;
		uint32 field_C8;
		uint32 field_CC;
		bool eventMetBoutarel;
		bool eventMetHadija;
		bool eventMetYasmin;
		uint32 field_DC;
		uint32 field_E0;
		uint32 field_E4;
		uint32 field_E8;
		uint32 field_EC;
		uint32 field_F0;
		uint32 field_F4;
		uint32 field_F8;
		uint32 field_FC;
		uint32 field_100;
		uint32 field_104;
		uint32 field_108;
		uint32 field_10C;
		uint32 field_110;
		uint32 field_114;
		uint32 field_118;
		uint32 field_11C;
		uint32 field_120;
		uint32 field_124;
		uint32 field_128;
		uint32 field_12C;
		uint32 field_130;
		uint32 field_134;
		uint32 field_138;
		uint32 field_13C;
		uint32 field_140;
		uint32 field_144;
		uint32 field_148;
		uint32 field_14C;
		uint32 field_150;
		uint32 field_154;
		uint32 field_158;
		uint32 field_15C;
		uint32 field_160;
		uint32 field_164;
		uint32 field_168;
		uint32 field_16C;
		uint32 field_170;
		uint32 field_174;
		uint32 field_178;
		uint32 field_17C;
		uint32 field_180;
		uint32 field_184;
		uint32 field_188;
		uint32 field_18C;
		uint32 field_190;
		uint32 field_194;
		uint32 field_198;
		uint32 field_19C;
		uint32 field_1A0;
		uint32 field_1A4;
		uint32 field_1A8;
		uint32 field_1AC;
		uint32 field_1B0;
		uint32 field_1B4;
		uint32 field_1B8;
		uint32 field_1BC;
		uint32 field_1C0;
		uint32 field_1C4;
		uint32 field_1C8;
		uint32 field_1CC;
		uint32 field_1D0;
		uint32 field_1D4;
		uint32 field_1D8;
		uint32 field_1DC;
		uint32 field_1E0;
		uint32 field_1E4;
		uint32 field_1E8;
		uint32 field_1EC;
		uint32 field_1F0;
		uint32 field_1F4;
		uint32 field_1F8;
		uint32 field_1FC;

		GameProgress() {
			field_0 = 0;
			jacket = kJacketOriginal;
			eventCorpseMovedFromFloor = false;
			field_C = 0;
			eventCorpseFound = false;
			field_14 = 0;                             // 5
			field_18 = 0;
			portrait = _defaultPortrait;
			eventCorpseThrown = false;
			field_24 = 0;
			field_28 = 0;                             // 10
			chapter = kChapter1;
			field_30 = 0;
			eventMetAugust = false;
			isNightTime = false;
			field_3C = 0;                             // 15
			field_40 = 0;
			field_44 = 0;
			field_48 = 0;
			field_4C = 0;
			isTrainRunning = false;                   // 20
			field_54 = 0;
			field_58 = 0;
			field_5C = 0;
			field_60 = 0;
			field_64 = 0;                             // 25
			field_68 = 0;
			eventMertensAugustWaiting = false;
			eventMertensKronosInvitation = false;
			isEggOpen = false;
			field_78 = 0;                             // 30
			field_7C = 0;
			field_80 = 0;
			field_84 = 0;
			field_88 = 0;
			field_8C = 0;                             // 35
			field_90 = 0;
			field_94 = 0;
			field_98 = 0;
			field_9C = 0;
			field_A0 = 0;                             // 40
			field_A4 = 0;
			field_A8 = 0;
			field_AC = 0;
			field_B0 = 0;
			field_B4 = 0;                             // 45
			field_B8 = 0;
			field_BC = 0;
			field_C0 = 0;
			field_C4 = 0;
			field_C8 = 0;                             // 50
			field_CC = 0;
			eventMetBoutarel = false;
			eventMetHadija = false;
			eventMetYasmin = false;
			field_DC = 0;                             // 55
			field_E0 = 0;
			field_E4 = 0;
			field_E8 = 0;
			field_EC = 0;
			field_F0 = 0;                             // 60
			field_F4 = 0;
			field_F8 = 0;
			field_FC = 0;
			field_100 = 0;
			field_104 = 0;                            // 65
			field_108 = 0;
			field_10C = 0;
			field_110 = 0;
			field_114 = 0;
			field_118 = 0;                            // 70
			field_11C = 0;
			field_120 = 0;
			field_124 = 0;
			field_128 = 0;
			field_12C = 0;                            // 75
			field_130 = 0;
			field_134 = 0;
			field_138 = 0;
			field_13C = 0;
			field_140 = 0;                            // 80
			field_144 = 0;
			field_148 = 0;
			field_14C = 0;
			field_150 = 0;
			field_154 = 0;                            // 85
			field_158 = 0;
			field_15C = 0;
			field_160 = 0;
			field_164 = 0;
			field_168 = 0;                            // 90
			field_16C = 0;
			field_170 = 0;
			field_174 = 0;
			field_178 = 0;
			field_17C = 0;                            // 95
			field_180 = 0;
			field_184 = 0;
			field_188 = 0;
			field_18C = 0;
			field_190 = 0;                            // 100
			field_194 = 0;
			field_198 = 0;
			field_19C = 0;
			field_1A0 = 0;
			field_1A4 = 0;                            // 105
			field_1A8 = 0;
			field_1AC = 0;
			field_1B0 = 0;
			field_1B4 = 0;
			field_1B8 = 0;                            // 110
			field_1BC = 0;
			field_1C0 = 0;
			field_1C4 = 0;
			field_1C8 = 0;
			field_1CC = 0;                            // 115
			field_1D0 = 0;
			field_1D4 = 0;
			field_1D8 = 0;
			field_1DC = 0;
			field_1E0 = 0;                            // 120
			field_1E4 = 0;
			field_1E8 = 0;
			field_1EC = 0;
			field_1F0 = 0;
			field_1F4 = 0;                            // 125
			field_1F8 = 0;
			field_1FC = 0;
		}

		/**
		 * Query if if a progress value is equal to the specified value.
		 *
		 * Note: This is necessary because we store different types in the progress structure
		 *       and need to test a value based on an index in Action::getCursor()
		 *
		 * @param index Zero-based index of the progress structure entry
		 * @param val   The value.
		 *
		 * @return true if equal, false if not.
		 */
		bool isEqual(uint index, uint val) {
			#define EXPOSE_VALUE(idx, name) case idx: return ((uint)name == val);

			switch (index) {
			default:
				error("GameProgress::isEqual: invalid index value (was: %d, max:127)", index);
				break;

			EXPOSE_VALUE(0, field_0);
			EXPOSE_VALUE(1, jacket);
			EXPOSE_VALUE(2, eventCorpseMovedFromFloor);
			EXPOSE_VALUE(3, field_C);
			EXPOSE_VALUE(4, eventCorpseFound);
			EXPOSE_VALUE(5, field_14);
			EXPOSE_VALUE(6, field_18);
			EXPOSE_VALUE(7, portrait);
			EXPOSE_VALUE(8, eventCorpseThrown);
			EXPOSE_VALUE(9, field_24);
			EXPOSE_VALUE(10, field_28);
			EXPOSE_VALUE(11, chapter);
			EXPOSE_VALUE(12, field_30);
			EXPOSE_VALUE(13, eventMetAugust);
			EXPOSE_VALUE(14, isNightTime);
			EXPOSE_VALUE(15, field_3C);
			EXPOSE_VALUE(16, field_40);
			EXPOSE_VALUE(17, field_44);
			EXPOSE_VALUE(18, field_48);
			EXPOSE_VALUE(19, field_4C);
			EXPOSE_VALUE(20, isTrainRunning);
			EXPOSE_VALUE(21, field_54);
			EXPOSE_VALUE(22, field_58);
			EXPOSE_VALUE(23, field_5C);
			EXPOSE_VALUE(24, field_60);
			EXPOSE_VALUE(25, field_64);
			EXPOSE_VALUE(26, field_68);
			EXPOSE_VALUE(27, eventMertensAugustWaiting);
			EXPOSE_VALUE(28, eventMertensKronosInvitation);
			EXPOSE_VALUE(29, isEggOpen);
			EXPOSE_VALUE(30, field_78);
			EXPOSE_VALUE(31, field_7C);
			EXPOSE_VALUE(32, field_80);
			EXPOSE_VALUE(33, field_84);
			EXPOSE_VALUE(34, field_88);
			EXPOSE_VALUE(35, field_8C);
			EXPOSE_VALUE(36, field_90);
			EXPOSE_VALUE(37, field_94);
			EXPOSE_VALUE(38, field_98);
			EXPOSE_VALUE(39, field_9C);
			EXPOSE_VALUE(40, field_A0);
			EXPOSE_VALUE(41, field_A4);
			EXPOSE_VALUE(42, field_A8);
			EXPOSE_VALUE(43, field_AC);
			EXPOSE_VALUE(44, field_B0);
			EXPOSE_VALUE(45, field_B4);
			EXPOSE_VALUE(46, field_B8);
			EXPOSE_VALUE(47, field_BC);
			EXPOSE_VALUE(48, field_C0);
			EXPOSE_VALUE(49, field_C4);
			EXPOSE_VALUE(50, field_C8);
			EXPOSE_VALUE(51, field_CC);
			EXPOSE_VALUE(52, eventMetBoutarel);
			EXPOSE_VALUE(53, eventMetHadija);
			EXPOSE_VALUE(54, eventMetYasmin);
			EXPOSE_VALUE(55, field_DC);
			EXPOSE_VALUE(56, field_E0);
			EXPOSE_VALUE(57, field_E4);
			EXPOSE_VALUE(58, field_E8);
			EXPOSE_VALUE(59, field_EC);
			EXPOSE_VALUE(60, field_F0);
			EXPOSE_VALUE(61, field_F4);
			EXPOSE_VALUE(62, field_F8);
			EXPOSE_VALUE(63, field_FC);
			EXPOSE_VALUE(64, field_100);
			EXPOSE_VALUE(65, field_104);
			EXPOSE_VALUE(66, field_108);
			EXPOSE_VALUE(67, field_10C);
			EXPOSE_VALUE(68, field_110);
			EXPOSE_VALUE(69, field_114);
			EXPOSE_VALUE(70, field_118);
			EXPOSE_VALUE(71, field_11C);
			EXPOSE_VALUE(72, field_120);
			EXPOSE_VALUE(73, field_124);
			EXPOSE_VALUE(74, field_128);
			EXPOSE_VALUE(75, field_12C);
			EXPOSE_VALUE(76, field_130);
			EXPOSE_VALUE(77, field_134);
			EXPOSE_VALUE(78, field_138);
			EXPOSE_VALUE(79, field_13C);
			EXPOSE_VALUE(80, field_140);
			EXPOSE_VALUE(81, field_144);
			EXPOSE_VALUE(82, field_148);
			EXPOSE_VALUE(83, field_14C);
			EXPOSE_VALUE(84, field_150);
			EXPOSE_VALUE(85, field_154);
			EXPOSE_VALUE(86, field_158);
			EXPOSE_VALUE(87, field_15C);
			EXPOSE_VALUE(88, field_160);
			EXPOSE_VALUE(89, field_164);
			EXPOSE_VALUE(90, field_168);
			EXPOSE_VALUE(91, field_16C);
			EXPOSE_VALUE(92, field_170);
			EXPOSE_VALUE(93, field_174);
			EXPOSE_VALUE(94, field_178);
			EXPOSE_VALUE(95, field_17C);
			EXPOSE_VALUE(96, field_180);
			EXPOSE_VALUE(97, field_184);
			EXPOSE_VALUE(98, field_188);
			EXPOSE_VALUE(99, field_18C);
			EXPOSE_VALUE(100, field_190);
			EXPOSE_VALUE(101, field_194);
			EXPOSE_VALUE(102, field_198);
			EXPOSE_VALUE(103, field_19C);
			EXPOSE_VALUE(104, field_1A0);
			EXPOSE_VALUE(105, field_1A4);
			EXPOSE_VALUE(106, field_1A8);
			EXPOSE_VALUE(107, field_1AC);
			EXPOSE_VALUE(108, field_1B0);
			EXPOSE_VALUE(109, field_1B4);
			EXPOSE_VALUE(110, field_1B8);
			EXPOSE_VALUE(111, field_1BC);
			EXPOSE_VALUE(112, field_1C0);
			EXPOSE_VALUE(113, field_1C4);
			EXPOSE_VALUE(114, field_1C8);
			EXPOSE_VALUE(115, field_1CC);
			EXPOSE_VALUE(116, field_1D0);
			EXPOSE_VALUE(117, field_1D4);
			EXPOSE_VALUE(118, field_1D8);
			EXPOSE_VALUE(119, field_1DC);
			EXPOSE_VALUE(120, field_1E0);
			EXPOSE_VALUE(121, field_1E4);
			EXPOSE_VALUE(122, field_1E8);
			EXPOSE_VALUE(123, field_1EC);
			EXPOSE_VALUE(124, field_1F0);
			EXPOSE_VALUE(125, field_1F4);
			EXPOSE_VALUE(126, field_1F8);
			EXPOSE_VALUE(127, field_1FC);
			}
		}

		Common::String toString() {
#define PRINT_VALUE(idx, name) ret += Common::String::printf("(%03d) " #name " = %d\n", idx, name);

			Common::String ret = "";

			PRINT_VALUE(0, field_0);
			PRINT_VALUE(1, jacket);
			PRINT_VALUE(2, eventCorpseMovedFromFloor);
			PRINT_VALUE(3, field_C);
			PRINT_VALUE(4, eventCorpseFound);
			PRINT_VALUE(5, field_14);
			PRINT_VALUE(6, field_18);
			PRINT_VALUE(7, portrait);
			PRINT_VALUE(8, eventCorpseThrown);
			PRINT_VALUE(9, field_24);
			PRINT_VALUE(10, field_28);
			PRINT_VALUE(11, chapter);
			PRINT_VALUE(12, field_30);
			PRINT_VALUE(13, eventMetAugust);
			PRINT_VALUE(14, isNightTime);
			PRINT_VALUE(15, field_3C);
			PRINT_VALUE(16, field_40);
			PRINT_VALUE(17, field_44);
			PRINT_VALUE(18, field_48);
			PRINT_VALUE(19, field_4C);
			PRINT_VALUE(20, isTrainRunning);
			PRINT_VALUE(21, field_54);
			PRINT_VALUE(22, field_58);
			PRINT_VALUE(23, field_5C);
			PRINT_VALUE(24, field_60);
			PRINT_VALUE(25, field_64);
			PRINT_VALUE(26, field_68);
			PRINT_VALUE(27, eventMertensAugustWaiting);
			PRINT_VALUE(28, eventMertensKronosInvitation);
			PRINT_VALUE(29, isEggOpen);
			PRINT_VALUE(30, field_78);
			PRINT_VALUE(31, field_7C);
			PRINT_VALUE(32, field_80);
			PRINT_VALUE(33, field_84);
			PRINT_VALUE(34, field_88);
			PRINT_VALUE(35, field_8C);
			PRINT_VALUE(36, field_90);
			PRINT_VALUE(37, field_94);
			PRINT_VALUE(38, field_98);
			PRINT_VALUE(39, field_9C);
			PRINT_VALUE(40, field_A0);
			PRINT_VALUE(41, field_A4);
			PRINT_VALUE(42, field_A8);
			PRINT_VALUE(43, field_AC);
			PRINT_VALUE(44, field_B0);
			PRINT_VALUE(45, field_B4);
			PRINT_VALUE(46, field_B8);
			PRINT_VALUE(47, field_BC);
			PRINT_VALUE(48, field_C0);
			PRINT_VALUE(49, field_C4);
			PRINT_VALUE(50, field_C8);
			PRINT_VALUE(51, field_CC);
			PRINT_VALUE(52, eventMetBoutarel);
			PRINT_VALUE(53, eventMetHadija);
			PRINT_VALUE(54, eventMetYasmin);
			PRINT_VALUE(55, field_DC);
			PRINT_VALUE(56, field_E0);
			PRINT_VALUE(57, field_E4);
			PRINT_VALUE(58, field_E8);
			PRINT_VALUE(59, field_EC);
			PRINT_VALUE(60, field_F0);
			PRINT_VALUE(61, field_F4);
			PRINT_VALUE(62, field_F8);
			PRINT_VALUE(63, field_FC);
			PRINT_VALUE(64, field_100);
			PRINT_VALUE(65, field_104);
			PRINT_VALUE(66, field_108);
			PRINT_VALUE(67, field_10C);
			PRINT_VALUE(68, field_110);
			PRINT_VALUE(69, field_114);
			PRINT_VALUE(70, field_118);
			PRINT_VALUE(71, field_11C);
			PRINT_VALUE(72, field_120);
			PRINT_VALUE(73, field_124);
			PRINT_VALUE(74, field_128);
			PRINT_VALUE(75, field_12C);
			PRINT_VALUE(76, field_130);
			PRINT_VALUE(77, field_134);
			PRINT_VALUE(78, field_138);
			PRINT_VALUE(79, field_13C);
			PRINT_VALUE(80, field_140);
			PRINT_VALUE(81, field_144);
			PRINT_VALUE(82, field_148);
			PRINT_VALUE(83, field_14C);
			PRINT_VALUE(84, field_150);
			PRINT_VALUE(85, field_154);
			PRINT_VALUE(86, field_158);
			PRINT_VALUE(87, field_15C);
			PRINT_VALUE(88, field_160);
			PRINT_VALUE(89, field_164);
			PRINT_VALUE(90, field_168);
			PRINT_VALUE(91, field_16C);
			PRINT_VALUE(92, field_170);
			PRINT_VALUE(93, field_174);
			PRINT_VALUE(94, field_178);
			PRINT_VALUE(95, field_17C);
			PRINT_VALUE(96, field_180);
			PRINT_VALUE(97, field_184);
			PRINT_VALUE(98, field_188);
			PRINT_VALUE(99, field_18C);
			PRINT_VALUE(100, field_190);
			PRINT_VALUE(101, field_194);
			PRINT_VALUE(102, field_198);
			PRINT_VALUE(103, field_19C);
			PRINT_VALUE(104, field_1A0);
			PRINT_VALUE(105, field_1A4);
			PRINT_VALUE(106, field_1A8);
			PRINT_VALUE(107, field_1AC);
			PRINT_VALUE(108, field_1B0);
			PRINT_VALUE(109, field_1B4);
			PRINT_VALUE(110, field_1B8);
			PRINT_VALUE(111, field_1BC);
			PRINT_VALUE(112, field_1C0);
			PRINT_VALUE(113, field_1C4);
			PRINT_VALUE(114, field_1C8);
			PRINT_VALUE(115, field_1CC);
			PRINT_VALUE(116, field_1D0);
			PRINT_VALUE(117, field_1D4);
			PRINT_VALUE(118, field_1D8);
			PRINT_VALUE(119, field_1DC);
			PRINT_VALUE(120, field_1E0);
			PRINT_VALUE(121, field_1E4);
			PRINT_VALUE(122, field_1E8);
			PRINT_VALUE(123, field_1EC);
			PRINT_VALUE(124, field_1F0);
			PRINT_VALUE(125, field_1F4);
			PRINT_VALUE(126, field_1F8);
			PRINT_VALUE(127, field_1FC);

			return ret;
		}
	};

	struct GameState {
		// Header
		uint32 brightness;
		uint32 volume;

		// Game data
		uint32 field_0;
		uint32 time;
		uint32 timeDelta;
		uint32 timeTicks;
		bool sceneUseBackup;       // byte
		SceneIndex scene;          // uint32
		SceneIndex sceneBackup;    // uint32
		SceneIndex sceneBackup2;   // uin32

		GameProgress progress;
		byte events[512];

		GameState() {
			brightness = _defaultBrigthness;
			volume = _defaultVolume;

			//Game data
			time = _defaultTime;
			timeDelta = _defaultTimeDelta;
			timeTicks = 0;
			sceneUseBackup = false;
			scene = kSceneDefault;
			sceneBackup = kSceneNone;
			sceneBackup2 = kSceneNone;

			// Clear game events
			memset(events, 0, 512*sizeof(byte));
		}

		/**
		 * Convert this object into a string representation.
		 *
		 * @return A string representation of this object.
		 */
		Common::String toString() {
			Common::String ret = "";

			ret += Common::String::printf("Time: %d    - Time delta: %d    - Ticks: %d\n", time, timeDelta, timeTicks);
			ret += Common::String::printf("Brightness: %d    - Volume: %d    - UseBackup: %d\n", brightness, volume, sceneUseBackup);
			ret += Common::String::printf("Scene: %d    - Scene backup: %d    - Scene backup 2: %d\n", scene, sceneBackup, sceneBackup2);

			return ret;
		}
	};

	struct Flags {
		bool flag_0;
		bool flag_3;
		bool flag_4;
		bool flag_5;

		bool frameInterval;

		bool isGameRunning;

		// Mouse flags
		bool mouseLeftClick;
		bool mouseRightClick;

		bool flag_entities_0;
		bool flag_entities_1;

		bool shouldRedraw;
		bool shouldDrawEggOrHourGlass;

		Flags() {
			flag_0 = false;
			flag_3 = false;
			flag_4 = false;
			flag_5 = false;

			frameInterval = false;

			isGameRunning = false;

			mouseRightClick = false;
			mouseLeftClick = false;

			flag_entities_0 = false;
			flag_entities_1 = false;

			shouldRedraw = false;
			shouldDrawEggOrHourGlass = false;
		}

		/**
		 * Convert this object into a string representation.
		 *
		 * @return A string representation of this object.
		 */
		Common::String toString() {
			Common::String ret = "";

			ret += Common::String::printf("Unknown: 0:%02d  -  3:%02d  -  4:%02d  -  5:%02d\n", flag_0, flag_3, flag_4, flag_5);
			ret += Common::String::printf("FrameInterval: %02d  -  ShouldRedraw:%02d  -  ShouldDrawEggOrHourGlass:%02d\n", frameInterval, shouldRedraw, shouldDrawEggOrHourGlass);
			ret += Common::String::printf("IsGameRunning: %02d\n", isGameRunning);
			ret += Common::String::printf("Mouse: RightClick:%02d  - LeftClick:%02d\n", mouseRightClick, mouseLeftClick);
			ret += Common::String::printf("Entities: 0:%02d  -  1:%02d\n", flag_entities_0, flag_entities_1);

			return ret;
		}
	};

	State(LastExpressEngine *engine);
	~State();

	// Accessors
	Inventory  *getGameInventory() { return _inventory; }
	Objects    *getGameObjects() { return _objects; }
	SavePoints *getGameSavePoints() { return _savepoints; }
	GameState  *getGameState() { return _state; }
	Flags      *getGameFlags() { return _flags; }

	// Time checks
	bool isNightTime() const;

	// Timer
	int getTimer() { return _timer; }
	void setTimer(int val) { _timer = val; }

	// Coordinates
	void setCoordinates(Common::Point coords) { _coords = coords; }
	const Common::Point getCoordinates() { return _coords; }

	// Helpers
	static uint32 getPowerOfTwo(uint32 x);
	static void getHourMinutes(uint32 time, uint8 *hours, uint8 *minutes);

private:
	static const uint32 _defaultBrigthness = 0x3;
	static const uint32 _defaultVolume = 0x7;
	static const uint32 _defaultTime = 1037700;
	static const uint32 _defaultTimeDelta = 3;
	static const uint32 _defaultPortrait = 32;

	LastExpressEngine *_engine;

	// Timer
	int _timer;

	Flags *_flags;				///< Flags
	Inventory *_inventory;  	///< Inventory
	Objects *_objects;			///< Objects
	SavePoints *_savepoints;	///< SavePoints
	GameState *_state;			///< State
	Common::Point _coords;		///< Current coordinates
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_STATE_H
