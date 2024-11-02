/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "common/debug.h"
#include "darkseed/darkseed.h"
#include "darkseed/usecode.h"
#include "darkseed/usecode_tos_tables.h"

namespace Darkseed {

UseCode::UseCode(Console *console, Player *player, Objects &objectVar, Inventory &inventory) : _console(console), _player(player), _objectVar(objectVar), _inventory(inventory) {}


int Darkseed::UseCode::getHandDescriptionTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getHandDescriptionTosIdx: Object Index out of range! %d", objNum);
	}
	return handDescriptionsTbl[objNum];
}

int16 Darkseed::UseCode::getUseGlovesTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseGlovesTosIdx: Object Index out of range! %d", objNum);
	}
	return glovesTextTbl[objNum];
}

int16 Darkseed::UseCode::getUseMoneyTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseMoneyTosIdx: Object Index out of range! %d", objNum);
	}
	return moneyTextTbl[objNum];
}

int16 Darkseed::UseCode::getUseCrowbarTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseCrowbarTosIdx: Object Index out of range! %d", objNum);
	}
	return crowBarTextTbl[objNum];
}

int16 Darkseed::UseCode::getUseJournalTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseJournalTosIdx: Object Index out of range! %d", objNum);
	}
	return journalTextTbl[objNum];
}

int16 Darkseed::UseCode::getUseLibraryCardTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseLibraryCardTosIdx: Object Index out of range! %d", objNum);
	}
	return libraryCardTextTbl[objNum];
}

int16 UseCode::getUseBobbyPinTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseBobbyPinTosIdx: Object Index out of range! %d", objNum);
	}
	return bobbyPinTextTbl[objNum];
}

int16 UseCode::getUseKeysTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseKeysTosIdx: Object Index out of range! %d", objNum);
	}
	return keysTextTbl[objNum];
}

int16 UseCode::getUseBinocularsTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseBinocularsTosIdx: Object Index out of range! %d", objNum);
	}
	return binocularsTextTbl[objNum];
}

int16 UseCode::getUseShovelTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseShovelTosIdx: Object Index out of range! %d", objNum);
	}
	return shovelTextTbl[objNum];
}

int16 UseCode::getUseDelbertsCardTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseDelbertsCardTosIdx: Object Index out of range! %d", objNum);
	}
	return delbertsCardTextTbl[objNum];
}

int16 UseCode::getUseStickTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseStickTosIdx: Object Index out of range! %d", objNum);
	}
	return stickTextTbl[objNum];
}

int16 UseCode::getUseAxeHandleTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseAxeHandleTosIdx: Object Index out of range! %d", objNum);
	}
	return axeHandleTextTbl[objNum];
}

int16 UseCode::getUseRopeTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseRopeTosIdx: Object Index out of range! %d", objNum);
	}
	return ropeTextTbl[objNum];
}

int16 UseCode::getUseMicroFilmTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseMicroFilmTosIdx: Object Index out of range! %d", objNum);
	}
	return microFilmTextTbl[objNum];
}

int16 UseCode::getUseSpecialHammerTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseSpecialHammerTosIdx: Object Index out of range! %d", objNum);
	}
	return specialHammerTextTbl[objNum];
}

int16 UseCode::getUseGunTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseGunTosIdx: Object Index out of range! %d", objNum);
	}
	return gunTextTbl[objNum];
}

int16 UseCode::getUseWatchTosIdx(uint16 objNum) {
	if (objNum >= Objects::MAX_OBJECTS) {
		error("getUseWatchTosIdx: Object Index out of range! %d", objNum);
	}
	return watchTextTbl[objNum];
}

void Darkseed::UseCode::useCode(int objNum) {
	debug("useCode: objNum = %d", objNum);

	if (objNum == 141) {
		_console->addTextLine("You touch Delbert...");
		return;
	}
	if (objNum == 189) {
		_console->addTextLine("You touch the cold iron bars.");
		return;
	}
	if (objNum == 42) {
		switch (_objectVar[42]) {
		case 0:
			_console->printTosText(653);
			break;
		case 1:
			_console->printTosText(655);
			_objectVar[42] = 0;
			break;
		case 2:
			_console->printTosText(660);
			_inventory.addItem(6);
			_objectVar[42] = 3;
			break;
		case 3:
			_console->printTosText(655);
			_objectVar[42] = 4;
			break;
		case 4:
			_console->printTosText(653);
			break;
		default:
			break;
		}
		return;
	}
	if (objNum == 9) {
		g_engine->_room->_collisionType = 0;
		g_engine->_room->removeObjectFromRoom(9);
		_objectVar.setMoveObjectRoom(9, 100);
		g_engine->showFullscreenPic(g_engine->isCdVersion() ? "paper_c.pic" : "paper-c.pic");
		return;
	}
	if (objNum == 35) {
		_objectVar[35] = 3600; // wind watch
	}
	if (objNum == 47) {
		_player->loadAnimations("phone.nsp");
		if (_objectVar.getObjectRunningCode(47) == 0) {
			g_engine->_animation->setupOtherNspAnimation(2, 63);
		} else {
			g_engine->_animation->setupOtherNspAnimation(0, 8);
		}
		_player->_direction = 3;
	}
	if (objNum == 194) {
		if (_objectVar[53] == 2) {
			_console->printTosText(948);
			for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
				if (_objectVar.getMoveObjectRoom(i) == 252) {
					_objectVar.setMoveObjectRoom(i, 254);
					_inventory.addItem(i);
				}
			}
			_objectVar[53] = 3;
		} else {
			_console->printTosText(566);
		}
		return;
	}
	if (objNum == 113) {
		bool foundMatch = false;
		for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
			if (_objectVar.getMoveObjectRoom(i) == 250) {
				_objectVar.setMoveObjectRoom(i, 254);
				_inventory.addItem(i);
				foundMatch = true;
			}
		}
		if (foundMatch) {
			_objectVar[113] = 0;
			_console->printTosText(948);
		} else {
			_console->printTosText(751);
		}
	}
	if (objNum == 123) {
		if (g_engine->_currentTimeInSeconds < 61200) {
			_console->printTosText(119);
		} else {
			_player->loadAnimations("bedsleep.nsp");
			g_engine->_animation->setupOtherNspAnimation(1, 5);
		}
		return;
	}
	if (objNum == 71 && _objectVar[71] == 2) { // car ignition
		_objectVar[71] = 0;
		_console->printTosText(949);
		_inventory.addItem(26);
		return;
	}
	if ((162 < objNum) && (objNum < 169)) {
		g_engine->playSound(47, 5, -1);
	}
	if (objNum == 175) {
		g_engine->playSound(39, 5, -1);
		//		while (iVar2 = VOCPlaying(), iVar2 != 0) {
		//			VOCPoll();
		//		}
		_console->printTosText(719);
		return;
	}
	if (objNum == 187) { //keeper computer screen
		if (_objectVar[187] == 0) {
			_console->printTosText(856);
			g_engine->_animation->setupOtherNspAnimation(0, 60);
		} else {
			g_engine->_animation->setupOtherNspAnimation(1, 61);
		}
		return;
	}
	if (objNum == 137 || objNum == 30) {
		if (_objectVar[30] == 0) {
			_objectVar[30] = 1;
			g_engine->_room->_collisionType = 0;
			g_engine->_room->removeObjectFromRoom(30);
			_objectVar.setMoveObjectRoom(30, 100);
			g_engine->showFullscreenPic(g_engine->isCdVersion() ? "note_c.pic" : "note-c.pic");
		} else {
			_console->printTosText(296);
		}
		return;
	}
	if (g_engine->_room->_roomNumber == 21 && objNum == 12) {
		if (_objectVar[12] == 0 && _objectVar[10] == 1) {
			_objectVar[12] = 1;
			_inventory.addItem(12);
			_console->printTosText(568);
		} else {
			_console->printTosText(566);
		}
		return;
	}
	if (objNum == 46) {
		if (_objectVar[46] == 1) {
			g_engine->_animation->setupOtherNspAnimation(0, 19);
		} else {
			_console->printTosText(539);
		}
		return;
	}
	if (objNum != 7 && objNum != 36 && objNum != 37 && objNum != 38 && objNum != 39 && objNum != 40) {
		int handTosIdx = getHandDescriptionTosIdx(objNum);
		if (handTosIdx != 0 && handTosIdx < 979) {
			_console->printTosText(handTosIdx);
		} else if (handTosIdx > 978) {
			genericResponse(2, objNum, handTosIdx);
		}
		if (objNum == 80) {
			_console->printTosText(553 + (_objectVar[80] & 1));
			_objectVar[80] = (_objectVar[80] & 2) + ((_objectVar[80] & 1) == 0 ? 1 : 0);
			return;
		}
		if ((objNum == 59 || objNum == 78) && _objectVar[34] == 0) {
			return;
		}
		if (objNum == 100) {
			if (_objectVar[100] == 0 || _objectVar[100] == 1 || _objectVar[100] == 3) {
				_console->printTosText(140);
			} else if (_objectVar[100] == 2) {
				_objectVar[100] = 3;
				_inventory.addItem(10);
				_objectVar[10] = 254;
				_console->printTosText(142);
			}
			return;
		}
		if (objNum == 114) {
			g_engine->_animation->gancAnim();
			return;
		}
		if ((objNum == 28) && (_objectVar[28] == 2)) {
			_console->addTextLine("The sergeant says 'Nice gun eh? It's a Browning'");
			return;
		}
		if (objNum > 103 && objNum < 111) {
			if (objNum < 110) {
				_player->loadAnimations("opendrwr.nsp");
				if (objNum == 108) {
					if (_objectVar[108] == 0) {
						_console->printTosText(383);
					} else {
						_console->printTosText(385);
					}
					g_engine->_animation->setupOtherNspAnimation(0, 52);
				} else {
					if (_objectVar[objNum] == 0) {
						_console->printTosText(371);
					} else {
						_console->printTosText(373);
					}
					g_engine->_animation->setupOtherNspAnimation(0, objNum - 56);
				}
			} else {
				if (_objectVar[objNum] == 0) {
					_console->printTosText(389);
				} else {
					_console->printTosText(391);
				}
				_objectVar[objNum] = _objectVar[objNum] == 0 ? 1 : 0;
			}
			return;
		}
		if (objNum == 111) { // tap
			_player->loadAnimations("aspirin.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 30);
			_console->printTosText(242);
			return;
		}
		if (objNum == 112) { // mirror cabinet
			_player->loadAnimations("aspirin.nsp");
			g_engine->_animation->setupOtherNspAnimation(2, 32);
			return;
		}
		if (objNum > 30 && objNum < 34) {
			g_engine->_room->removeObjectFromRoom(objNum);
			g_engine->getPackageObj(objNum - 30);
			return;
		}
		if ((objNum == 14) && (_objectVar[86] == 0)) {
			_objectVar[86] = 1;
			return;
		}
		if ((objNum == 102) && (_objectVar[23] == 1)) {
			_player->loadAnimations("slide.nsp");
			g_engine->_animation->setupOtherNspAnimation(1, 26);
			return;
		}
		if (objNum == 101) {
			switch (_objectVar[101]) {
			case 0:
				_player->loadAnimations("opendoor.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 65);
				g_engine->playSound(31, 5, -1);
				_objectVar[101] = 1;
				_console->printTosText(733);
				break;
			case 1:
				_player->loadAnimations("opendoor.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 65);
				g_engine->playSound(31, 5, -1);
				_objectVar[101] = 0;
				_console->printTosText(737);
				break;
			case 2:
				_inventory.addItem(5);
				_console->printTosText(950);
				_objectVar[101] = 3;
				return;
			case 3:
				_player->loadAnimations("opendoor.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 65);
				g_engine->playSound(31, 5, -1);
				_objectVar[101] = 4;
				_console->printTosText(737);
				break;
			case 4:
				_player->loadAnimations("opendoor.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 65);
				g_engine->playSound(31, 5, -1);
				_objectVar[101] = 3;
				_console->printTosText(733);
				break;
			default:
				break;
			}
		}
		if (objNum == 81) {
			if (_objectVar[81] == 0) {
				_console->printTosText(951);
				for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
					if (_objectVar.getMoveObjectRoom(i) == 251) {
						_objectVar.setMoveObjectRoom(i, 254);
						_inventory.addItem(i);
					}
				}
				_objectVar[81] = 1;
			} else {
				_console->printTosText(952);
			}
			return;
		}
		if (objNum == 25) {
			if (1 < _objectVar[80]) {
				if (_objectVar[25] == 0) {
					_objectVar[25] = 1;
					_console->printTosText(953);
					_inventory.addItem(25);
				} else if (_objectVar[25] == 2) {
					_objectVar[26] = 1;
					_console->printTosText(954);
					_inventory.addItem(26);
					_objectVar[25] = _objectVar[25] + 1;
				} else if (_objectVar[25] == 1 || _objectVar[25] == 3 ||
						   _objectVar[25] == 101 || _objectVar[25] == 103) {
					_console->printTosText(694);
				}
			}
			return;
		}
		if (objNum < 42 && objNum != 22 && (objNum != 7 || _objectVar[7] == 1) &&
			objNum != 31 && objNum != 33 && objNum != 32 && objNum != 86) {
			_console->printTosText(955); // "You pick up the "
			_console->addToCurrentLine(Common::String::format("%s.", _objectVar.getObjectName(objNum)));
			_inventory.addItem(objNum);
			g_engine->_room->_collisionType = 0;
			if (((objNum != 25) || (_objectVar[25] == 1)) || (_objectVar[25] == 101)) {
				g_engine->_room->removeObjectFromRoom(objNum);
			}
			_objectVar.setMoveObjectRoom(objNum, 254);
			if (objNum == 28) {
				_objectVar[28] = 1;
			}
		} else if (objNum == 86) {
			_objectVar[86] = _objectVar[86] == 0 ? 1 : 0;
			g_engine->playSound(42, 5, -1);
		} else if (objNum == 22) {
			if (_objectVar[22] < 4) {
				_player->loadAnimations("push.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 18);
			} else {
				_console->printTosText(665);
			}
		} else if (objNum == 165) {
			if (_objectVar[165] == 0) {
				_objectVar[165] = 1;
				_console->printTosText(639);
				_inventory.addItem(13);
			} else {
				_console->printTosText(640);
			}
		} else if (objNum == 57) {
			_player->loadAnimations("instrshp.nsp");
			if (_objectVar[71] == 2) {
				_console->printTosText(4);
				g_engine->_animation->setupOtherNspAnimation(1, 44);
			} else {
				g_engine->_animation->setupOtherNspAnimation(0, 43);
			}
		} else if (objNum == 117) {
			_player->loadAnimations("obslev.nsp");
			g_engine->_animation->setupOtherNspAnimation(1, 46);
		} else if (objNum == 62) {
			if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (g_engine->_currentDay != 1)) {
				if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (g_engine->_currentDay != 2)) {
					if (((_objectVar[49] == 0) || (1 < _objectVar[62])) || (g_engine->_currentDay != 3)) {
						if (_objectVar[62] < 101) {
							g_engine->_sound->playMusic(MusicId::kRadio);
							g_engine->playSound(0, 6, -1);
							_objectVar[62] = 101;
							_console->addTextLine("You turn on the music.");
						} else if (_objectVar[62] == 101) {
							g_engine->_sound->killAllSound();
							g_engine->_sound->stopMusic();
							_objectVar[62] = 100;
							_console->addTextLine("You turn off the music.");
						}
					} else {
						_objectVar[62] = _objectVar[62] + 1;
						_console->printTosText(921);
					}
				} else {
					_objectVar[62] = _objectVar[62] + 1;
					_console->printTosText(920);
				}
			} else {
				_objectVar[62] = _objectVar[62] + 1;
				_console->printTosText(919);
			}
		} else if (objNum == 63) {
			//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)sg_engine->_room33.nnn_1060_319f);
			//			_Room = 33;
			//			InitRoom();
			//			LoadRoomAndDumpPic();
			//			SetHeroSpriteNumber();
		} else if (objNum == 83) {
			//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)sg_engine->_room28.nnn_1060_31ab);
			//			_Room = 28;
			//			InitRoom();
			//			LoadRoomAndDumpPic();
			//			SetHeroSpriteNumber();
		} else if (objNum == 82) {
			if (g_engine->_room->_roomNumber == 41) {
				g_engine->_room->_roomNumber = 44;
				_console->printTosText(956);
			} else {
				g_engine->_room->_roomNumber = 41;
				_console->printTosText(957);
			}
			//			LoadRoomAndDumpPic();
			//			InitRoom();
			//			SetHeroSpriteNumber();
		} else if (objNum == 52) {
			//			_NoScroll = 0;
			if (_objectVar[52] == 1) {
				_player->loadAnimations("shower.nsp");
				g_engine->_animation->setupOtherNspAnimation(0, 2);
				_console->printTosText(193);
				g_engine->playSound(2, 4, -1);
			} else {
				_console->printTosText(194);
			}
		} else if (objNum == 59) {
			if (_objectVar[59] == 2) {
				_objectVar[59] = 1;
				if (g_engine->_room->_roomNumber == 0 || g_engine->_room->_roomNumber == 9) {
					_console->printTosText(409);
				}
			} else {
				if (g_engine->_room->_roomNumber == 0 || g_engine->_room->_roomNumber == 9) {
					_console->printTosText(407);
				}
				_objectVar[59] = 2;
			}
		} else if (objNum == 78) {
			if (_objectVar[78] == 2) {
				_objectVar[78] = 1;
				if (g_engine->_room->_roomNumber == 0 || g_engine->_room->_roomNumber == 9) {
					_console->printTosText(409);
				}
			} else {
				g_engine->playSound(41, 5, -1);
				if (g_engine->_room->_roomNumber == 0 || g_engine->_room->_roomNumber == 9) {
					_console->printTosText(32);
				}
				_objectVar[78] = 2;
			}
		} else if (objNum == 139) {
			_player->loadAnimations("ltladder.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 10);
		} else if (objNum == 76) {
			//			_NoScroll = 0;
			//			Ordinal_184(unaff_CS,(int)&_file_name,4192,(int)sg_engine->_room3.nnn_1060_31e1);
			//			_Room = 3;
			//			LoadRoomAndDumpPic();
			//			InitRoom();
			//			SetHeroSpriteNumber();
		} else if (objNum == 77) {
			_player->loadAnimations("rtladder.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 12);
		} else if (objNum == 66 && _objectVar[68] == 0) {
			if (_objectVar[12] == 2) {
				g_engine->playSound(5, 5, -1);
				if (_objectVar[66] == 0) {
					if (_objectVar[67] == 0 && _objectVar[68] == 0) {
						_objectVar[66] = 1;
					} else {
						_objectVar[66] = 2;
					}
				} else {
					_objectVar[66] = 0;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		} else if (objNum == 67 && _objectVar[68] == 0) {
			if (_objectVar[12] == 2) {
				g_engine->playSound(5, 5, -1);
				if (_objectVar[67] == 0) {
					if (_objectVar[66] == 1 && _objectVar[68] == 0) {
						_objectVar[67] = 1;
					} else {
						_objectVar[67] = 2;
					}
				} else {
					_objectVar[67] = 0;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		} else if ((objNum == 68) && (_objectVar[68] == 0)) {
			if (_objectVar[12] == 2) {
				if ((_objectVar[66] == 1) && (_objectVar[67] == 1)) {
					g_engine->playSound(13, 5, -1);
					_objectVar[68] = 1;
					g_engine->_animation->setupOtherNspAnimation(0, 23);
				} else {
					_objectVar[68] = 2;
				}
			} else {
				_console->addTextLine("You touch the surface of the ornate sigil.");
			}
		} else if (objNum == 84) {
			_console->printTosText(566);
		}
		//		_NoScroll = 0;
		return;
	}
	if (_objectVar[138] == 0) {
		_console->printTosText(906);
	} else {
		_inventory.addItem(objNum);
		g_engine->_room->removeObjectFromRoom(objNum);
		_objectVar[138] = 0;
		g_engine->updateDisplay();
		if ((objNum == 7) && g_engine->_currentDay == 1) {
			_console->printTosText(905);
			_objectVar[141] = 1;
			_player->loadAnimations("delstore.nsp");
			_objectVar.setMoveObjectPosition(141, {60, 221});
			_objectVar.setMoveObjectRoom(7, 100);
			_objectVar[7] = 1;
		}
	}
}

void Darkseed::UseCode::useCodeGloves(int16 targetObjNum) {
	if (targetObjNum == 113) {
		putObjUnderPillow(14);
		return;
	}

	int16 tosIdx = getUseGlovesTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(14, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 57) {
		_player->loadAnimations("instrshp.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 43);
	} else if (targetObjNum == 117) {
		_player->loadAnimations("obslev.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 45);
	}
}

void Darkseed::UseCode::useCodeMoney(int16 targetObjNum) {
	if ((targetObjNum != 138) && (targetObjNum != 152)) {
		int16 tosIdx = getUseMoneyTosIdx(targetObjNum);
		if (tosIdx == 0) {
			if (targetObjNum == 7) {
				_console->printTosText(961);
			} else if (targetObjNum == 113) {
				putObjUnderPillow(8);
			}
		} else if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(8, targetObjNum, tosIdx);
		}
		return;
	}
	if (_objectVar[138] == 0) {
		_player->loadAnimations("givclerk.nsp");
		g_engine->_animation->setupOtherNspAnimation(6, 35);
	} else {
		_console->addTextLine("Choose an item before giving clerk more money.");
	}
}

void Darkseed::UseCode::useCodeJournal(int16 actionObjNum, int16 targetObjNum) {
	if (targetObjNum == 113) {
		putObjUnderPillow(actionObjNum);
	} else if ((actionObjNum == 6) && (targetObjNum == 47)) {
		_console->printTosText(46);
	} else if ((actionObjNum == 6) && (targetObjNum == 100)) {
		_console->printTosText(145);
	} else if ((actionObjNum == 6) && (targetObjNum == 136)) {
		_console->printTosText(999);
	} else {
		int16 tosIdx = getUseJournalTosIdx(targetObjNum);
		if (tosIdx != 0) {
			if (tosIdx < 979) {
				_console->printTosText(tosIdx);
			} else {
				genericResponse(6, targetObjNum, tosIdx);
			}
		}
	}
}

void Darkseed::UseCode::useCodeShopItems(int16 actionObjNum, int16 targetObjNum) {
	if (actionObjNum == 7) {
		if (targetObjNum == 44) {
			_player->loadAnimations("opendoor.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 24);
			_objectVar[44] = 3600;
			g_engine->playSound(32, 5, -1);
			_inventory.removeItem(7);
			_console->printTosText(730);
		} else if ((targetObjNum == 141) && (_objectVar[141] == 7)) {
			_objectVar[141] = 8;
		} else if (targetObjNum == 47) {
			_console->printTosText(54);
		} else if (targetObjNum == 127) {
			_console->printTosText(106);
		} else if (targetObjNum == 123) {
			_console->printTosText(129);
		} else if (targetObjNum == 100) {
			_console->printTosText(153);
		} else if (targetObjNum == 143) {
			_console->printTosText(189);
		} else if (targetObjNum == 52) {
			_console->printTosText(202);
		} else if (targetObjNum == 145) {
			_console->printTosText(226);
		} else if (targetObjNum == 137) {
			_console->printTosText(300);
		} else if (targetObjNum == 126) {
			_console->printTosText(312);
		} else if (targetObjNum == 103 || targetObjNum == 151) {
			_console->printTosText(362);
		} else if (targetObjNum == 109) {
			_console->printTosText(965);
		} else if (targetObjNum == 61) {
			_console->printTosText(460);
		} else if (targetObjNum == 53) {
			_console->printTosText(489);
			g_engine->throwmikeinjail();
		} else if (targetObjNum == 46) {
			_console->printTosText(542);
		} else if (targetObjNum < 87 || 98 < targetObjNum) {
			if (targetObjNum < 163 || 168 < targetObjNum) {
				if (targetObjNum == 171) {
					_console->printTosText(679);
				} else if (targetObjNum == 129) {
					genericResponse(7, 129, 998);
				} else if (targetObjNum == 108) {
					_console->printTosText(386);
				} else if (targetObjNum == 110) {
					_console->printTosText(392);
				} else if (targetObjNum == 85) {
					_console->printTosText(434);
				} else if (targetObjNum == 194) {
					genericResponse(7, 194, 989);
				} else if (targetObjNum == 156) {
					_console->printTosText(550);
				} else if (targetObjNum == 172) {
					genericResponse(7, 172, 988);
				} else if (targetObjNum == 71) {
					genericResponse(7, 71, 987);
				} else if (targetObjNum == 101) {
					genericResponse(7, 101, 986);
				} else if (targetObjNum == 120) {
					_console->printTosText(804);
				} else if (targetObjNum == 184) {
					genericResponse(7, 184, 980);
				} else if (targetObjNum == 185) {
					genericResponse(7, 185, 982);
				} else if (targetObjNum == 121) {
					genericResponse(7, 121, 990);
				} else {
					genericResponse(7, targetObjNum, 999);
				}
			} else {
				_console->printTosText(632);
			}
		} else {
			_console->printTosText(585);
		}
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 78) {
		_console->printTosText(42);
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 47) {
		_console->printTosText(60);
		return;
	}
	if (actionObjNum == 36 && targetObjNum == 125) {
		_console->printTosText(77);
		return;
	}
	if (actionObjNum == 38 && targetObjNum == 125) {
		_console->printTosText(78);
		return;
	}
	if (actionObjNum == 38 &&
		(targetObjNum == 126 || targetObjNum == 197 ||
		  targetObjNum == 131 || targetObjNum == 89 ||
		  targetObjNum == 171)) {
		_console->printTosText(89);
		return;
	}
	if (actionObjNum == 37 && targetObjNum == 124) {
		_console->printTosText(28);
	} else if (actionObjNum == 38 && targetObjNum == 127) {
		_console->printTosText(116);
	} else if ((actionObjNum == 36 || actionObjNum == 37) && targetObjNum == 123) {
		_console->printTosText(135);
	} else if (actionObjNum == 38 && (targetObjNum == 123 || targetObjNum == 318)) {
		_console->printTosText(320);
	} else if (targetObjNum == 129) {
		genericResponse(actionObjNum, 129, 998);
	} else if ((actionObjNum == 37) &&
			   ((((((targetObjNum == 142 || (targetObjNum == 124)) ||
					((targetObjNum == 137 || (((targetObjNum == 148 || (targetObjNum == 109)) || (targetObjNum == 59)))))) ||
				   ((targetObjNum == 66 || (targetObjNum == 67)))) ||
				  (targetObjNum == 68)) ||
				 ((((targetObjNum == 133 || (targetObjNum == 194)) ||
					((targetObjNum == 46 ||
					  (((((targetObjNum == 156 || (targetObjNum == 158)) || (targetObjNum == 159)) ||
						 ((targetObjNum == 161 || (targetObjNum == 162)))) ||
						(targetObjNum == 22)))))) ||
				   (((targetObjNum == 42 || (targetObjNum == 35)) ||
					 ((targetObjNum == 196 ||
					   ((((targetObjNum == 170 || (targetObjNum == 172)) || (targetObjNum == 25)) ||
						 (((targetObjNum == 71 || (targetObjNum == 101)) || (targetObjNum == 179)))))))))))))) {
		_console->printTosText(180);
	} else if ((actionObjNum == 37) && (targetObjNum == 52)) {
		_console->printTosText(211);
	} else if ((actionObjNum == 37) && (targetObjNum == 144)) {
		_console->printTosText(218);
	} else if ((actionObjNum == 37) && (targetObjNum == 145)) {
		_console->printTosText(229);
	} else if ((actionObjNum == 38) && (targetObjNum == 145)) {
		_console->printTosText(230);
	} else if (targetObjNum == 112) {
		genericResponse(actionObjNum, 112, 996);
	} else if ((actionObjNum == 38) && (targetObjNum == 49)) {
		_console->printTosText(251);
	} else if ((targetObjNum == 147) && (actionObjNum != 7)) {
		_console->printTosText(319);
	} else if (targetObjNum == 197) {
		genericResponse(actionObjNum, 197, 993);
	} else if ((actionObjNum == 38) && (targetObjNum == 150)) {
		_console->printTosText(354);
	} else if (targetObjNum == 109) {
		if (actionObjNum == 38) {
			_console->printTosText(375);
		} else {
			_console->printTosText(374);
		}
	} else if (targetObjNum == 130) {
		genericResponse(actionObjNum, 130, 991);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else if ((targetObjNum == 131) && (actionObjNum != 7)) {
		_console->printTosText(405);
	} else if (targetObjNum == 85) {
		_console->printTosText(434);
	} else if (targetObjNum == 121) {
		if (actionObjNum == 38) {
			_console->printTosText(445);
		} else if (actionObjNum == 40) {
			_console->printTosText(446);
		} else {
			genericResponse(actionObjNum, 121, 990);
		}
	} else if ((actionObjNum == 37) && (targetObjNum == 122)) {
		_console->printTosText(451);
	} else if ((actionObjNum == 38) || (targetObjNum != 61)) {
		if ((actionObjNum == 36) && (targetObjNum == 133)) {
			_console->printTosText(476);
		} else if (((actionObjNum == 36) || (actionObjNum == 40)) && (targetObjNum == 53)) {
			_console->printTosText(491);
		} else if ((actionObjNum == 37) && (targetObjNum == 53)) {
			_console->printTosText(492);
		} else if (targetObjNum == 194) {
			genericResponse(actionObjNum, 194, 989);
		} else if (targetObjNum == 153) {
			_console->printTosText(405);
		} else if ((actionObjNum == 37) && (targetObjNum == 154)) {
			_console->printTosText(535);
		} else if ((actionObjNum == 36) && (targetObjNum == 46)) {
			_console->printTosText(405);
		} else if ((actionObjNum == 38) && (targetObjNum == 46)) {
			_console->printTosText(545);
		} else if ((actionObjNum == 40) && (targetObjNum == 46)) {
			_console->printTosText(405);
		} else if (targetObjNum == 156) {
			_console->printTosText(550);
		} else if (targetObjNum == 159) {
			_console->printTosText(577);
		} else if ((targetObjNum < 87) || (98 < targetObjNum)) {
			if ((actionObjNum == 37) && ((162 < targetObjNum && (targetObjNum < 169)))) {
				_console->printTosText(634);
			} else if ((actionObjNum == 38) && ((162 < targetObjNum && (targetObjNum < 169)))) {
				_console->printTosText(635);
			} else if (targetObjNum == 172) {
				genericResponse(actionObjNum, 172, 988);
			} else if ((actionObjNum == 37) && (targetObjNum == 174)) {
				_console->printTosText(690);
			} else if ((actionObjNum == 38) && (targetObjNum == 25)) {
				_console->printTosText(696);
			} else if ((actionObjNum == 40) && (targetObjNum == 26)) {
				_console->printTosText(700);
			} else if (targetObjNum == 71) {
				genericResponse(actionObjNum, 71, 987);
			} else if ((actionObjNum == 39) && (targetObjNum == 44)) {
				_console->printTosText(731);
			} else if (targetObjNum == 101) {
				genericResponse(actionObjNum, 101, 986);
			} else if (targetObjNum == 113) {
				_console->printTosText(753);
			} else if (targetObjNum == 41) {
				_console->printTosText(757);
			} else if ((actionObjNum == 36) && (targetObjNum == 179)) {
				_console->printTosText(764);
			} else if ((actionObjNum == 38) && (targetObjNum == 179)) {
				_console->printTosText(762);
			} else if ((actionObjNum == 38) && (targetObjNum == 180)) {
				_console->printTosText(769);
			} else if (targetObjNum == 120) {
				_console->printTosText(804);
			} else if ((actionObjNum == 38) && (targetObjNum == 119)) {
				_console->printTosText(808);
			} else if ((actionObjNum == 38) && (targetObjNum == 114)) {
				genericResponse(38, 114, 984);
			} else if (targetObjNum == 184) {
				genericResponse(actionObjNum, 184, 980);
			} else if (targetObjNum == 185) {
				genericResponse(actionObjNum, 185, 982);
			} else if ((actionObjNum == 38) && (targetObjNum == 79)) {
				_console->printTosText(833);
			} else if ((targetObjNum == 79) && (actionObjNum != 7)) {
				_console->printTosText(836);
			} else if ((actionObjNum == 38) && (targetObjNum == 58)) {
				_console->printTosText(849);
			} else {
				genericResponse(actionObjNum, targetObjNum, 999);
			}
		} else {
			_console->printTosText(587);
		}
	} else {
		_console->printTosText(462);
	}
}

void Darkseed::UseCode::useCrowBar(int16 targetObjNum) {
	int16 tosIdx = getUseCrowbarTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(5, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 42) {
		if ((_objectVar[42] == 0) || (_objectVar[42] == 4)) {
			_player->loadAnimations("crowbar.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 17);
		} else {
			_console->printTosText(962);
		}
	}
}

// All this code is unused as you cannot add the newspaper to your inventory. :( I wonder why they didn't add it. It is also missing the inventory icon image.
void UseCode::useCodeNewspaper(int16 targetObjNum) {
	if (targetObjNum == 124) {
		_console->printTosText(26); // The original logic also had this if statement repeated further down the chain
									        // but calling _console->printTosText(268);
	} else if (targetObjNum == 126) {
		_console->printTosText(82);
	} else if (targetObjNum == 127) {
		_console->printTosText(112);
	} else if (targetObjNum == 123) {
		_console->printTosText(133);
	} else if (targetObjNum == 100) {
		_console->printTosText(157);
	} else if (targetObjNum == 52) {
		_console->printTosText(207);
	} else if (targetObjNum == 145) {
		_console->printTosText(222);
	} else if (targetObjNum == 137) {
		_console->printTosText(299);
	} else if (targetObjNum == 147) {
		_console->printTosText(309);
	} else if (targetObjNum == 197) {
		_console->printTosText(993);
	} else if (targetObjNum == 150) {
		_console->printTosText(352);
	} else if ((targetObjNum == 103) || (targetObjNum == 151)) {
		_console->printTosText(367);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else if (targetObjNum == 61) {
		_console->printTosText(455);
	} else if (targetObjNum == 53) {
		_console->printTosText(490);
	} else if (targetObjNum == 156) {
		_console->printTosText(550);
	} else if (targetObjNum == 159) {
		_console->printTosText(577);
	} else if (targetObjNum == 113) {
		_console->printTosText(753);
	} else if (targetObjNum == 120) {
		_console->printTosText(804);
	} else if (targetObjNum == 116) {
		_console->printTosText(832);
	} else if (targetObjNum == 79) {
		_console->printTosText(834);
	} else if (targetObjNum == 185) {
		genericResponse(9, 185, 982);
	} else if (targetObjNum == 184) {
		genericResponse(9, 184, 980);
	} else if (targetObjNum == 71) {
		genericResponse(9, 71, 987);
	} else if (targetObjNum == 101) {
		genericResponse(9, 101, 986);
	} else if (targetObjNum == 172) {
		genericResponse(9, 172, 988);
	} else if (targetObjNum == 194) {
		genericResponse(9, 194, 989);
	} else if (targetObjNum == 121) {
		genericResponse(9, 121, 990);
	} else if (targetObjNum == 109) {
		genericResponse(9, 109, 992);
	} else if (targetObjNum == 130) {
		genericResponse(9, 130, 991);
	} else if (targetObjNum == 129) {
		genericResponse(9, 129, 998);
	} else if (targetObjNum == 112) {
		genericResponse(9, 112, 996);
	} else {
		genericResponse(9, targetObjNum, 999);
	}
}

void UseCode::useCodeLibraryCard(int16 targetObjNum) {
	int16 tosIdx = getUseLibraryCardTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(10, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 46) {
		g_engine->_cursor.setCursorType(Pointer);
		_inventory.removeItem(10);
		g_engine->_animation->libAnim(false);
		_objectVar[10] = 1;
	} else if (targetObjNum == 113) {
		putObjUnderPillow(10);
	}
}

void UseCode::useCodeBobbyPin(int16 targetObjNum) {
	int16 tosIdx = getUseBobbyPinTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(11, targetObjNum, tosIdx);
		}
	}

	if (targetObjNum == 190) {
		if (_objectVar[190] == 0) {
			_console->printTosText(882);
			g_engine->playSound(18, 5, -1);
			_objectVar[190] = 1;
		} else if (_objectVar[190] == 1) {
			_console->printTosText(883);
			g_engine->playSound(18, 5, -1);
			_objectVar[190] = 2;
		} else {
			_console->printTosText(963);
		}
	} else if (targetObjNum == 48) {
		if (_objectVar[48] == 0) {
			_objectVar[48] = 1;
			_console->printTosText(869);
			g_engine->_room->removeObjectFromRoom(48);
			_inventory.addItem(21);
			_inventory.removeItem(11);
		}
	} else if (targetObjNum == 113) {
		putObjUnderPillow(11);
	}
}

void UseCode::useCodeKeys(int16 actionObjNum, int16 targetObjNum) {
	if ((actionObjNum == 13) && (targetObjNum == 151)) {
		_objectVar[151] = 1;
		g_engine->showFullscreenPic("cclock02.pic");
	} else if (targetObjNum == 71) {
		if (actionObjNum == 26) {
			if (_objectVar[44] == 0) {
				g_engine->playSound(16, 5, -1);
				_console->printTosText(708);
			} else if (_objectVar[71] == 0) {
				g_engine->playSound(11, 5, -1);
				_objectVar[71] = 2;
				_console->printTosText(709);
				_inventory.removeItem(26);
			}
		} else {
			_console->printTosText(707);
		}
	} else if (targetObjNum == 113) {
		putObjUnderPillow(actionObjNum);
	} else if ((actionObjNum == 26) &&
			   ((((targetObjNum == 175 || (targetObjNum == 62)) || (targetObjNum == 176)) || (targetObjNum == 44)))) {
		_console->printTosText(720);
	} else if ((actionObjNum == 26) && (targetObjNum == 191)) {
		_console->printTosText(890);
	} else {
		int16 tosIdx = getUseKeysTosIdx(targetObjNum);
		if (tosIdx != 0) {
			if (tosIdx < 979) {
				_console->printTosText(tosIdx);
			} else {
				genericResponse(actionObjNum, targetObjNum, tosIdx);
			}
		}
	}
}

void UseCode::useCodeMirrorShard(int16 targetObjNum) {
	if (targetObjNum == 137) {
		_console->printTosText(298);
		_inventory.removeItem(15);
		_objectVar[137] = 1;
		_player->loadAnimations("mglow.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 59);
	} else if (targetObjNum == 124) {
		_console->printTosText(20);
	} else if (targetObjNum == 129) {
		genericResponse(15, 129, 998);
	} else if (targetObjNum == 23 || targetObjNum == 103 || targetObjNum == 170) {
		genericResponse(15, targetObjNum, 999);
	} else if (targetObjNum == 52) {
		_console->printTosText(197);
	} else if (targetObjNum == 112) {
		_console->printTosText(235);
	} else if (targetObjNum == 130) {
		genericResponse(15, 130, 991);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else {
		genericResponse(15, targetObjNum, 997);
	}
}

void UseCode::useCodeBinoculars(int16 targetObjNum) {
	int16 tosIdx = getUseBinocularsTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(16, targetObjNum, tosIdx);
		}
	}
	if ((targetObjNum == 162) && (g_engine->_room->_roomNumber == 36)) {
		_objectVar[162] = 1;
		g_engine->showFullscreenPic("bnoc.pic");
	}
	if ((targetObjNum == 118) && (g_engine->_room->_roomNumber == 43)) {
		g_engine->showFullscreenPic("darkbnoc.pic");
		_console->printTosText(800);
	}
}

void UseCode::useCodeShovel(int16 targetObjNum) {
	int16 tosIdx = getUseShovelTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(17, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum > 86 && targetObjNum < 99) {
		startDigging(targetObjNum + -87);
	}
}

void UseCode::useCodeDelbertsCard(int16 targetObjNum) {
	int16 tosIdx = getUseDelbertsCardTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(18, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 64) {
		if (g_engine->_room->_roomNumber == 30) {
			g_engine->_animation->setupOtherNspAnimation(1, 40);
		} else {
			_console->printTosText(488);
		}
	}
}

void UseCode::useCodeStick(int16 targetObjNum) {
	int16 tosIdx = getUseStickTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(19, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 116) {
		g_engine->_room->loadLocationSprites("mthrow.nsp");
		g_engine->_animation->setupOtherNspAnimation(2, 47);
	} else if (targetObjNum == 53) {
		g_engine->throwmikeinjail();
	}
}

void UseCode::useCodeAxeHandle(int16 targetObjNum) {
	int16 tosIdx = getUseAxeHandleTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(20, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 53) {
		g_engine->throwmikeinjail();
	}
}

void UseCode::useCodeRope(int16 targetObjNum) {
	int16 tosIdx = getUseRopeTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(23, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 61) {
		_objectVar[23] = 1;
		_player->loadAnimations("opendoor.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 15);
		_inventory.removeItem(23);
	}
	if (targetObjNum == 53) {
		g_engine->throwmikeinjail();
	}
}

void UseCode::useCodeMicroFilm(int16 targetObjNum) {
	int16 tosIdx = getUseMicroFilmTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(24, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 80) {
		if (_objectVar[80] == 1 || _objectVar[80] == 3) {
			_objectVar[80] = 3;
			g_engine->showFullscreenPic("cfilm01.pic");
			_console->printTosText(557);
		} else {
			_console->printTosText(964);
		}
	} else if (targetObjNum == 113) {
		putObjUnderPillow(24);
	}
}

void UseCode::useCodeSpecialHammer(int16 actionObjNum, int16 targetObjNum) {
	if ((actionObjNum == 27) && (targetObjNum == 124)) {
		_console->printTosText(24);
	} else if ((actionObjNum == 27) && (targetObjNum == 49)) {
		_console->printTosText(250);
	} else if ((actionObjNum == 25) && (targetObjNum == 78)) {
		_console->printTosText(41);
	} else if ((actionObjNum == 27) && (targetObjNum == 126)) {
		_console->printTosText(85);
	} else if ((actionObjNum == 25) && (targetObjNum == 127)) {
		_console->printTosText(109);
	} else if ((actionObjNum == 25) && (targetObjNum == 123)) {
		_console->printTosText(131);
	} else if ((actionObjNum == 25) && (targetObjNum == 100)) {
		_console->printTosText(155);
	} else if ((actionObjNum == 25) && (targetObjNum == 142)) {
		_console->printTosText(178);
	} else if ((actionObjNum == 25) && (targetObjNum == 52)) {
		_console->printTosText(204);
	} else if ((actionObjNum == 25) && (targetObjNum == 108)) {
		_console->printTosText(386);
	} else if ((actionObjNum == 25) && (targetObjNum == 11)) {
		_console->printTosText(524);
	} else if ((actionObjNum == 25) && (targetObjNum == 173)) {
		_console->printTosText(686);
	} else if ((actionObjNum == 25) && (targetObjNum == 79)) {
		_console->printTosText(833);
	} else if (actionObjNum == 25) {
		genericResponse(25, targetObjNum, 999);
	} else if (actionObjNum == 27) {
		int16 tosIdx = getUseSpecialHammerTosIdx(targetObjNum);
		if (tosIdx != 0) {
			if (tosIdx < 979) {
				_console->printTosText(tosIdx);
			} else {
				genericResponse(27, targetObjNum, tosIdx);
			}
		}
	}
	if ((actionObjNum == 27) && (targetObjNum == 137)) {
		if (_objectVar[57] == 0) {
			_console->printTosText(301);
		} else {
			_player->loadAnimations("smash.nsp");
			g_engine->_animation->setupOtherNspAnimation(0, 36);
		}
	} else if (targetObjNum == 53) {
		g_engine->throwmikeinjail();
	}
}

void UseCode::useCodeGun(int16 targetObjNum) {
	int16 tosIdx = getUseGunTosIdx(targetObjNum);
	if (tosIdx != 0) {
		if (tosIdx < 979) {
			_console->printTosText(tosIdx);
		} else {
			genericResponse(28, targetObjNum, tosIdx);
		}
	}
	if (targetObjNum == 53) {
		g_engine->throwmikeinjail();
	}
}

void UseCode::useCodeMoversNote(int16 targetObjNum) {
	if (targetObjNum == 47) {
		_console->printTosText(57);
	} else if (targetObjNum == 172) { // original logic also duplicated this if statement and called a generic response
		_console->printTosText(93);
	} else if (targetObjNum == 129) {
		genericResponse(30, 129, 998);
	} else if (targetObjNum == 52) {
		_console->printTosText(205);
	} else if (targetObjNum == 145) {
		_console->printTosText(222);
	} else if (targetObjNum == 112) {
		genericResponse(30, 112, 996);
	} else if (targetObjNum == 146) {
		_console->printTosText(289);
	} else if (targetObjNum == 137) {
		_console->printTosText(302);
	} else if (targetObjNum == 147) {
		_console->printTosText(309);
	} else if (targetObjNum == 197) {
		genericResponse(30, 197, 993);
	} else if (targetObjNum == 109) {
		genericResponse(30, 109, 992);
	} else if (targetObjNum == 130) {
		genericResponse(30, 130, 991);
	} else if (targetObjNum == 121) {
		genericResponse(30, 121, 990);
	} else if (targetObjNum == 194) {
		genericResponse(30, 194, 989);
	} else if (targetObjNum == 71) {
		genericResponse(30, 71, 987);
	} else if (targetObjNum == 101) {
		genericResponse(30, 101, 986);
	} else if (targetObjNum == 184) {
		genericResponse(30, 184, 980);
	} else if (targetObjNum == 185) {
		genericResponse(30, 185, 982);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else if (targetObjNum == 61) {
		_console->printTosText(455);
	} else if (targetObjNum == 156) {
		_console->printTosText(550);
	} else if (targetObjNum == 159) {
		_console->printTosText(577);
	} else if (targetObjNum == 113) {
		putObjUnderPillow(113);
	} else if (targetObjNum == 120) {
		_console->printTosText(804);
	} else if (targetObjNum == 188) {
		_console->printTosText(864);
	} else if (targetObjNum == 48) {
		_console->printTosText(860);
	} else {
		genericResponse(30, targetObjNum, 999);
	}
}

void UseCode::useCodeBluePrints(int16 targetObjNum) {
	if (targetObjNum == 126) {
		_console->printTosText(82);
	} else if (targetObjNum == 127) {
		_console->printTosText(113);
	} else if (targetObjNum == 123) {
		_console->printTosText(133);
	} else if (targetObjNum == 52) {
		_console->printTosText(208);
	} else if (targetObjNum == 100) {
		_console->printTosText(158);
	} else if (targetObjNum == 109) {
		_console->printTosText(992);
	} else if (targetObjNum == 108) {
		_console->printTosText(386);
	} else if (targetObjNum == 110) {
		_console->printTosText(392);
	} else if (targetObjNum == 61) {
		_console->printTosText(455);
	} else if (targetObjNum == 156) {
		_console->printTosText(550);
	} else if (targetObjNum == 159) {
		_console->printTosText(577);
	} else if (targetObjNum == 41) {
		_console->printTosText(757);
	} else if (targetObjNum == 120) {
		_console->printTosText(804);
	} else if (targetObjNum == 114) {
		_console->printTosText(817);
	} else if (targetObjNum == 188) {
		_console->printTosText(864);
	} else if (targetObjNum == 48) {
		_console->printTosText(868);
	} else if (targetObjNum == 185) {
		genericResponse(34, 185, 982);
	} else if (targetObjNum == 184) {
		genericResponse(34, 184, 980);
	} else if (targetObjNum == 130) {
		genericResponse(34, 130, 991);
	} else if (targetObjNum == 197) {
		genericResponse(34, 197, 993);
	} else if (targetObjNum == 112) {
		genericResponse(34, 112, 996);
	} else if (targetObjNum == 129) {
		genericResponse(34, 129, 998);
	} else if (targetObjNum == 121) {
		genericResponse(34, 121, 990);
	} else if (targetObjNum == 194) {
		genericResponse(34, 194, 989);
	} else if (targetObjNum == 172) {
		genericResponse(34, 172, 988);
	} else if (targetObjNum == 71) {
		genericResponse(34, 71, 987);
	} else if (targetObjNum == 101) {
		genericResponse(34, 101, 986);
	} else if (targetObjNum == 113) {
		putObjUnderPillow(34);
	} else {
		genericResponse(34, targetObjNum, 999);
	}
}

void UseCode::useCodeWatch(int16 targetObjNum) {
	if (((targetObjNum == 61) || (targetObjNum == 194)) || (targetObjNum == 156)) {
		g_engine->printTime();
	} else if (targetObjNum == 113) {
		putObjUnderPillow(35);
	} else {
		int16 tosIdx = getUseWatchTosIdx(targetObjNum);
		if (tosIdx != 0) {
			if (tosIdx < 979) {
				_console->printTosText(tosIdx);
			} else {
				genericResponse(35, targetObjNum, tosIdx);
			}
		}
	}
}

void UseCode::useCodeTinCup(int16 targetObjNum) {
	if (targetObjNum == 189) {
		_player->loadAnimations("cuphit.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 62);
		_player->_frameIdx = 3;
	} else if (targetObjNum == 113) {
		_console->printTosText(753);
	}
}

void UseCode::useCodeEmptyUrn(int16 targetObjNum) {
	if ((targetObjNum == 2) || (targetObjNum == 3)) {
		_console->printTosText(964);
	}
}

void UseCode::genericResponse(int16 useObjNum, int16 targetObjNum, int16 tosIdx) {
	_genericResponseCounter++;
	if (_genericResponseCounter > 4) {
		_genericResponseCounter = 0;
	}
	switch (tosIdx) {
	case 979:
		_console->addTextLine(
			Common::String::format(
				"This sentry cannot be stopped with a %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 980:
		_console->addTextLine(
			Common::String::format(
				"The %s has no effect on the alien tubes.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 981:
		_console->addTextLine(
			Common::String::format(
				"You incinerate the %s inside the power nexus!.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 982:
		_console->addTextLine(
			Common::String::format(
				"The %s has no effect on this alien monstrosity.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 983:
		_console->addTextLine(
			Common::String::format(
				"The %s is not strong enough to cut the cables.\nPerhaps if you had some wire cutters?",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 984:
		_console->addTextLine(
			Common::String::format(
				"The %s has no effect on the protected ancient.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 985:
		_console->addTextLine(
			Common::String::format(
				"You hide the %s under the pillow.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 986:
		_console->addTextLine(
			Common::String::format(
				"Don't put the %s in the trunk, you'll forget it.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 987:
		_console->addTextLine(
			Common::String::format(
				"The car won't start with the %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 988:
		_console->addTextLine(
			Common::String::format(
				"If you put the %s in the trunk, you'll forget it.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 989:
		_console->addTextLine(
			Common::String::format(
				"The %s is yours, you haven't lost it.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 990:
		_console->addTextLine(
			Common::String::format(
				"Not a good place to hide the %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 991:
		_console->addTextLine(
			Common::String::format(
				"You try to put the %s inside, but the door won't open",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 992:
		_console->addTextLine(
			Common::String::format(
				"The kitchen is no place to keep the %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 993:
		_console->addTextLine(
			Common::String::format(
				"You'll forget the %s here.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 994:
		_console->addTextLine(
			Common::String::format(
				"You'd rather have the %s with you.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 995:
		_console->addTextLine(
			Common::String::format(
				"The %s has no effect.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 996:
		_console->addTextLine(
			Common::String::format(
				"This is not a good place for the %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 997:
		_console->addTextLine(
			Common::String::format(
				"You see a reflection of the %s.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 998:
		_console->addTextLine(
			Common::String::format(
				"You don't want to leave the %s under the bed.",
				_objectVar.getObjectName(useObjNum)));
		break;
	case 999:
		switch (_genericResponseCounter) {
		case 0:
			_console->addTextLine(
				Common::String::format(
					"Using the %s on the %s doesn't make any sense, it must be your headaches!",
					_objectVar.getObjectName(useObjNum),
					_objectVar.getObjectName(targetObjNum)
				)
			);
			break;
		case 1:
			_console->addTextLine(
				Common::String::format(
					"The %s will do nothing to the %s.",
					_objectVar.getObjectName(useObjNum),
					_objectVar.getObjectName(targetObjNum)
				)
			);
			break;
		case 2:
			_console->addTextLine(
				Common::String::format(
					"The %s doesn't have any effect on the %s.",
					_objectVar.getObjectName(useObjNum),
					_objectVar.getObjectName(targetObjNum)
				)
			);
			break;
		case 3:
			_console->addTextLine(
				Common::String::format(
					"The %s has nothing to do with %s.",
					_objectVar.getObjectName(useObjNum),
					_objectVar.getObjectName(targetObjNum)
				)
			);
			break;
		case 4:
			_console->addTextLine("Are you feeling alright?");
			break;
		default:
			error("Unhandled _genericResponseCounter value");
		}
		break;
	default:
		break;
	}
}

void UseCode::putObjUnderPillow(int objNum) {
	_objectVar[113] = 1;
	_inventory.removeItem(objNum);
	_objectVar.setMoveObjectRoom(objNum, 250);
	g_engine->_cursor.setCursorType(Pointer);
	_console->printTosText(946);
	_console->addToCurrentLine(Common::String::format("%s", g_engine->_objectVar.getObjectName(objNum)));
	_console->printTosText(947);
}

static constexpr bool diggingxflipTbl[12] = {
	true, true, true, true,
	true, false, false, true,
	true, true, true, false
};

void UseCode::startDigging(int16 targetObjNum) {
	if (targetObjNum == 0) {
		_player->loadAnimations("lgravedg.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 21);
	} else if (diggingxflipTbl[targetObjNum]) {
		_player->loadAnimations("lgravedg.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 4);
	} else {
		_player->loadAnimations("rgravedg.nsp");
		g_engine->_animation->setupOtherNspAnimation(0, 22);
	}
	g_engine->playSound(14, 5, -1);
}

} // End of namespace Darkseed
