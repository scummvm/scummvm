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

#include "darkseed/darkseed.h"
#include "darkseed/animation.h"

namespace Darkseed {

void Animation::setupOtherNspAnimation(int nspAnimIdx, int animId) {
	assert(nspAnimIdx < 20);
	_player->_playerIsChangingDirection = false;
	_nsp_sprite_scaling_y_position = 0;
	_scaleSequence = false;

	_animIndexTbl[nspAnimIdx] = 0;
	if (_player->_animations.getTotalAnim() > nspAnimIdx) {
		_spriteAnimCountdownTimer[nspAnimIdx] = _player->_animations.getAnimAt(nspAnimIdx)._frameDuration[0];
		_player->_frameIdx = _player->_animations.getAnimAt(nspAnimIdx)._frameNo[0];
	} else {
		_spriteAnimCountdownTimer[nspAnimIdx] = 0;
		_player->_frameIdx = 0;
	}
	_isPlayingAnimation_maybe = true;
	_otherNspAnimationType_maybe = animId;

	g_engine->_player->_flipSprite = false;
	_objRestarted = false;

	if (_otherNspAnimationType_maybe < 2) {
		return;
	}

	// TODO big switch here to init the different animation types.
	switch (_otherNspAnimationType_maybe) {
	case 2:
		_player->_position.x = 188;
		_player->_position.y = 94;
		break;
	case 3:
		if (g_engine->_currentDay == 3) {
			g_engine->_sound->playMusic(MusicId::kDth);
			g_engine->playSound(0,6,-1);
		}
		_scaleSequence = true;
		break;
	case 4:
	case 14:
	case 15:
	case 18:
	case 21:
	case 22:
	case 24:
	case 25:
	case 65:
		_scaleSequence = true;
		break;
	case 6: // stairs
		_nsp_sprite_scaling_y_position = 0xbe;
		_scaleSequence = true;
		break;
	case 7: // stairs down
		_nsp_sprite_scaling_y_position = 0xbe;
		_scaleSequence = true;
		break;
	case 8:
		_player->_position.x = 249;
		_player->_position.y = 92;
		break;
	case 16:
		_player->_position.x = 324;
		_player->_position.y = 50;
		break;
	case 19:
	case 23:
		_scaleSequence = true;
		_player->_frameIdx = 24;
		break;
	case 20:
		_spriteAnimCountdownTimer[3] = 3;
		break;
	case 26 :
		_player->_position.x = 324;
		_player->_position.y = 135;
		break;
	case 27 :
		_player->_position.x = 397;
		_player->_position.y = 84;
		break;
	case 28 :
		_player->_position.x = 397;
		_player->_position.y = 77;
		break;
	case 39:
		_scaleSequence = true;
		_player->_frameIdx = g_engine->_room->_locationSprites.getAnimAt(nspAnimIdx)._frameNo[0];
		break;
	case 41 :
	case 42 :
		_player->_position.x = 432;
		_player->_position.y = 78;
		break;
	case 44:
	case 46:
		g_engine->playSound(30, 5, -1);
		break;
	case 47:
		_scaleSequence = true;
		_player->_frameIdx = g_engine->_room->_locationSprites.getAnimAt(nspAnimIdx)._frameNo[0];
		break;
	case 53 :
	case 54 :
		_player->_position.x = 308;
		_player->_position.y = 160;
		break;
	case 55 :
		_player->_position.x = 150;
		_player->_position.y = 104;
		break;
	case 56 :
		_player->_position.x = 142;
		_player->_position.y = 104;
		break;
	case 57:
	case 58:
		g_engine->playSound(48, 5, -1);
		break;
	case 63:
		_player->_position.x = 249;
		_player->_position.y = 92;
		g_engine->_phoneStatus = 1;
		break;
	default:
		break;
	}
}

void Animation::updateAnimation() {
	int currentRoomNumber = g_engine->_room->_roomNumber;
	switch (_otherNspAnimationType_maybe) {
	case 0:
		break;
	case 1: // sleep wake anim
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		} else {
			_player->_position.x = 0xdf;
			_player->_position.y = 0xbe;
			_player->_walkTarget.x = 0xdf;
			_player->_walkTarget.y = 0xbe;
			_player->updateSprite();
		}
		break;
	case 2:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		} else {
			_player->_position.x = 218;
			_player->_position.y = 198;
			_player->_direction = 2;
			_player->updateSprite();
			_player->_walkTarget.x = 218;
			_player->_walkTarget.y = 198;
			_objectVar[52] = 0;
			//			StopVOC(); TODO
		}
		break;
	case 3:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			if (g_engine->_room->isGiger()) {
				stuffPlayer();
			} else {
				if (g_engine->_room->isOutside() && g_engine->_currentTimeInSeconds > 61200) {
					g_engine->_room->restorePalette();
				}
				g_engine->gotoNextMorning();
				g_engine->playDayChangeCutscene();
			}
		}
		break;
	case 4:
	case 21:
	case 22: // dig up grave
		advanceAnimationFrame(0);
		if (_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]] % 5 == 3 &&
			_player->_animations.getAnimAt(0)._frameDuration[_animIndexTbl[0]] == _spriteAnimCountdownTimer[0]) {
			g_engine->playSound(14, 5, -1);
		}
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			_isPlayingAnimation_maybe = false;
			_objectVar[52] = 1;
			if (_otherNspAnimationType_maybe == 21 && _objectVar[151] != 0) {
				if (_objectVar[87] == 0) {
					_objectVar[88] = 1;
					_objectVar[87] = 1;
					g_engine->_console->printTosText(581);
					g_engine->_inventory.addItem(29);
				} else {
					g_engine->_console->printTosText(582);
				}
			} else {
				_objectVar[88] = 1;
				g_engine->_console->printTosText(580);
			}
		}
		break;
	case 5: // goto sleep animation
		_player->_position.x = 135;
		_player->_position.y = 91;
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]]];
		} else {
			g_engine->gotoNextMorning(); // TODO there might be some extra logic required from original function
			_player->_position.x = 242;
			_player->_position.y = 187;
			g_engine->playDayChangeCutscene();
		}
		_player->_position.x = 242;
		_player->_position.y = 187;
		break;
	case 6: // stairs up
		if (currentRoomNumber == 6) {
			advanceAnimationFrame(1);
		} else {
			advanceAnimationFrame(2);
		}
		if (_frameAdvanced && ((currentRoomNumber == 6 && _animIndexTbl[1] == 1) || (currentRoomNumber == 5 && _animIndexTbl[2] == 1))) {
			//			FUN_1208_0dacg_engine->_sound_related(0xd,CONCAT11(uVar4,5));
		}
		if (!_objRestarted) {
			if (currentRoomNumber == 6) {
				_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]]];
			} else {
				_player->_frameIdx = _player->_animations.getAnimAt(2)._frameNo[_player->_animations.getAnimAt(2)._frameNo[_animIndexTbl[2]]];
			}
		} else {
			if (currentRoomNumber == 6) {
				g_engine->_previousRoomNumber = 6;
				g_engine->changeToRoom(5);
			}
			if (_player->_isAutoWalkingToBed) {
				_player->setPlayerTowardsBedroom();
			}
		}
		break;
	case 7: // stairs down
		if (currentRoomNumber == 5) {
			advanceAnimationFrame(3);
			if (_frameAdvanced && _animIndexTbl[3] == 1) {
				// FUN_1208_0dacg_engine->_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!_objRestarted) {
				_player->_frameIdx = _player->_animations.getAnimAt(3)._frameNo[_player->_animations.getAnimAt(3)._frameNo[_animIndexTbl[3]]];
			} else {
				g_engine->_previousRoomNumber = 5;
				g_engine->changeToRoom(6);
			}
		} else {
			advanceAnimationFrame(0);
			if (_frameAdvanced && _animIndexTbl[0] == 1) {
				// FUN_1208_0dacg_engine->_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			}
			if (!_objRestarted) {
				_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
			}
		}
		break;
	case 8: //phone call
	case 63:
		if (_otherNspAnimationType_maybe == 8) {
			advanceAnimationFrame(0);
		} else {
			advanceAnimationFrame(2);
			if (g_engine->_phoneStatus == 1) {
				g_engine->_phoneStatus = 2;
			}
		}
		if (!_objRestarted) {
			if (_otherNspAnimationType_maybe == 8) {
				_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
			} else {
				_player->_frameIdx = _player->_animations.getAnimAt(2)._frameNo[_player->_animations.getAnimAt(2)._frameNo[_animIndexTbl[2]]];
			}
		} else {
			if (_otherNspAnimationType_maybe == 63) {
				g_engine->_console->printTosText(44);
				setupOtherNspAnimation(1, 64);
			} else {
				_objectVar.setObjectRunningCode(47, 0);
				if (g_engine->_currentDay == 1) {
					g_engine->_console->printTosText(904);
					_objectVar[47] = 1;
					_objectVar[46] = 1;
				} else {
					_objectVar[51] = 2;
					_objectVar[47] = 1;
					g_engine->_console->printTosText(922);
				}
				g_engine->waitForSpeech();
				setupOtherNspAnimation(1, 9);
			}
		}
		break;
	case 9: // hang up phone
	case 64:
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]]];
		} else {
			_player->_position.x = 300;
			_player->_position.y = 183;
			_player->_walkTarget.x = 300;
			_player->_walkTarget.y = 183;
			_player->_direction = 3;
			_player->updateSprite();
		}
		break;
	case 11:
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			_player->_position.x = 175;
			_player->_position.y = 200;
			_player->_walkTarget.x = 175;
			_player->_walkTarget.y = 200;
			_player->updateSprite();
			if (_player->_isAutoWalkingToBed) {
				_player->setPlayerTowardsBedroom();
			}
		}
		break;
	case 12:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		} else {
			_player->_position.x = 360;
			_player->_position.y = 238;
			_player->_walkTarget.x = 360;
			_player->_walkTarget.y = 238;
			_player->_direction = 3;
			g_engine->changeToRoom(2);
		}
		break;
	case 13:
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			_player->_position.x = 450;
			_player->_position.y = 217;
			_player->_walkTarget.x = 450;
			_player->_walkTarget.y = 217;
			_player->_direction = 3;
			_player->updateSprite();
		}
		break;
	case 16: // climb down rope
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		} else {
			_player->_position.x = 336;
			_player->_position.y = 195;
			_player->_walkTarget.x = 336;
			_player->_walkTarget.y = 195;
			_player->_direction = 1;
			_player->updateSprite();
		}
		break;
	case 17: // open trunk with crowbar
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		}
		if (_animIndexTbl[0] == 5 && _frameAdvanced) {
			g_engine->playSound(31, 5, -1);
			g_engine->_console->printTosText(666);
			if (_objectVar[42] == 0) {
				_objectVar[42] = 1;
			} else {
				_objectVar[42] = 3;
			}
		}
		break;
	case 18: // push trunk
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		}
		if (_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]] == 3 && _frameAdvanced) {
			g_engine->_console->printTosText(_objectVar[22] + 662);
			_objectVar[22] = _objectVar[22] + 1;
			if (_objectVar[22] == 3) {
				g_engine->_room->loadRoom61AWalkableLocations();
			}
		}
		break;
	case 19: // pickup book from library
		_player->_frameIdx = 24;
		if (_objRestarted) {
			_isPlayingAnimation_maybe = true;
			_objectVar[46] = 2;
			libAnim(true);
		}
		break;
	case 20: // receive card from delbert
		advanceAnimationFrame(3);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(3)._frameNo[_animIndexTbl[3]];
		} else {
			g_engine->_inventory.addItem(18);
		}
		break;
	case 23:
		_player->_frameIdx = 24;
		if (_objRestarted != 0) {
			_isPlayingAnimation_maybe = false;
			_objectVar[99] = 1;
			g_engine->_console->printTosText(468);
		}
		break;
	case 10:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]];
		} else {
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			g_engine->changeToRoom(61);
		}
		break;
	case 14:
	case 15:
	case 24:
	case 25:
		// Open doors
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			_player->updateSprite();
			if (_otherNspAnimationType_maybe == 14) {
				// TODO
				if (_objectVar.getObjectRunningCode(140) == 0 || g_engine->_room->_roomNumber != 6) {
					g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
					int newRoomNumber;
					if (g_engine->_room->_roomNumber == 6) {
						if (_player->_isAutoWalkingToBed && _objectVar[137] == 2) {
							g_engine->wonGame();
						}
						newRoomNumber = 10;
					} else if (g_engine->_room->_roomNumber == 10) {
						newRoomNumber = 6;
					} else if (g_engine->_room->_roomNumber == 11) {
						if (_player->_position.x < 250) {
							newRoomNumber = 15;
						} else {
							newRoomNumber = 16;
						}
					} else {
						newRoomNumber = 17;
					}
					g_engine->changeToRoom(newRoomNumber);
				} else {
					_objectVar.setObjectRunningCode(140, 0);
					g_engine->getPackageObj(g_engine->_currentDay);
				}
			}
			if (_otherNspAnimationType_maybe == 25) {
				g_engine->_previousRoomNumber = 33;
				g_engine->changeToRoom(34);
			}
		}
		break;
	case 26: // climb up rope.
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			g_engine->changeToRoom(13);
		}
		break;
	case 27:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]]; // TODO check if this is correct.
		} else {
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			g_engine->changeToRoom(38);
		}
		break;
	case 28: // step out of mirror normal world
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			_player->_position.x = 418;
			_player->_position.y = 170;
			_player->_direction = 3;
			_player->updateSprite();
			_player->_walkTarget = _player->_position;
		}
		break;
	case 30:
	case 31: {
		int animIdx = _otherNspAnimationType_maybe - 30;
		advanceAnimationFrame(animIdx);
		if (_isPlayingAnimation_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(animIdx)._frameNo[_animIndexTbl[animIdx]];
		}
		if (_objRestarted && _otherNspAnimationType_maybe == 30) {
			setupOtherNspAnimation(1, 31);
		}
		break;
	}
	case 32:
	case 33:
	case 34: {
		_objectVar[112] = 1;
		int animIdx = _otherNspAnimationType_maybe - 30;
		advanceAnimationFrame(animIdx);
		// TODO play sfx.
		if (_isPlayingAnimation_maybe) {
			_player->_frameIdx = _player->_animations.getAnimAt(animIdx)._frameNo[_animIndexTbl[animIdx]];
		}
		if (_objRestarted && (_otherNspAnimationType_maybe == 32 || _otherNspAnimationType_maybe == 33)) {
			setupOtherNspAnimation(_otherNspAnimationType_maybe - 29, _otherNspAnimationType_maybe + 1);
		}
		break;
	}
	case 35: // pay shopkeeper
		advanceAnimationFrame(6);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(6)._frameNo[_animIndexTbl[6]];
		} else {
			_objectVar[8]++;
			if (_objectVar[8] > 2) {
				g_engine->_inventory.removeItem(8);
			}
			_objectVar[138]++;
			g_engine->_console->printTosText(927);
		}
		break;
	case 36:
	case 37: // smash mirror
		advanceAnimationFrame(_otherNspAnimationType_maybe - 36);
		_player->_frameIdx = _player->_animations.getAnimAt(_otherNspAnimationType_maybe - 36)._frameNo[_animIndexTbl[_otherNspAnimationType_maybe - 36]];
		if (_frameAdvanced && _otherNspAnimationType_maybe == 36 && _player->_frameIdx == 4) {
			g_engine->playSound(28, 5, -1);
		}
		if (_objRestarted) {
			if (_otherNspAnimationType_maybe == 36) {
				setupOtherNspAnimation(1, 37);
			} else {
				_objectVar[137] = 2;
				_player->_isAutoWalkingToBed = true;
				_player->setPlayerTowardsBedroom();
			}
		}
		break;
	case 38:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			stuffPlayer();
		}
		break;
	case 39: // Arrest Mike.
		g_engine->_room->advanceFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = g_engine->_room->_locationSprites.getAnimAt(1)._frameNo[g_engine->_room->_locObjFrame[1]];
		} else {
			g_engine->throwmikeinjail();
		}
		break;
	case 40: // give Delbert's card to cop.
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			g_engine->_console->printTosText(61);
			g_engine->_console->draw();
			g_engine->_screen->updateScreen();
			g_engine->waitxticks(60);
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			_player->_position.x = 240;
			_player->_position.y = 200;
			_player->updateSprite();
			_isPlayingAnimation_maybe = false;
			g_engine->changeToRoom(15, true);
			g_engine->_inventory.removeItem(41);
			g_engine->_inventory.removeItem(18);
			_objectVar.setMoveObjectRoom(41, 255);
		}
		break;
	case 41:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			g_engine->changeToRoom(7);
		}
		break;
	case 42: // step out of mirror giger world
		advanceAnimationFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(1)._frameNo[_animIndexTbl[1]];
		} else {
			_player->_position.x = 456;
			_player->_position.y = 173;
			_player->_direction = 3;
			_player->updateSprite();
			_player->_walkTarget = _player->_position;
		}
		break;
	case 43:
	case 44:
	case 45:
	case 46: {
		int iVar4 = 0;
		if ((_otherNspAnimationType_maybe == 44) || (_otherNspAnimationType_maybe == 46)) {
			iVar4 = 1;
		}
		advanceAnimationFrame(iVar4);
		_player->_frameIdx = _player->_animations.getAnimAt(iVar4)._frameNo[_player->_animations.getAnimAt(iVar4)._frameNo[_animIndexTbl[iVar4]]];
		//		_HeroSpr = (uint) * (byte *)((int)&DAT_1060_7eb8 + *(int *)((int)&_ObjFrame + iVar4 * 2) + iVar4 * 202);
		if (!_objRestarted || (_otherNspAnimationType_maybe != 46 && _otherNspAnimationType_maybe != 44)) {
			if (_otherNspAnimationType_maybe == 45) {
				_objectVar[117] = 1;
			} else if (_objRestarted) {
				if ((_objectVar[71] == 2) && (_objectVar[44] != 0)) {
					g_engine->_console->printTosText(896);
					_objectVar[57] = 1;
				} else {
					g_engine->_console->printTosText(897);
				}
			}
		} else {
			g_engine->_sound->playMusic(MusicId::kDth);
			g_engine->playSound(0, 6, -1);
			stuffPlayer();
		}
		break;
	}
	case 47:
		g_engine->_room->mikeStickThrowAnim();
		break;
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
		// open/close cupboards, oven
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			if (_otherNspAnimationType_maybe == 52) {
				_objectVar[108] = (_objectVar[108] == 0) ? 1 : 0;
			} else {
				_objectVar[56 + _otherNspAnimationType_maybe] = _objectVar[56 + _otherNspAnimationType_maybe] == 0 ? 1 : 0;
			}
			g_engine->playSound(42, 5, -1);
		}
		break;
	case 53:
	case 54:
	case 55:
	case 56: {
		int stairsIdx = (_otherNspAnimationType_maybe - 53) & 1;
		advanceAnimationFrame(stairsIdx);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(stairsIdx)._frameNo[_animIndexTbl[stairsIdx]];
		} else {
			switch (_otherNspAnimationType_maybe) {
			case 53:
				g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
				g_engine->changeToRoom(6);
				break;
			case 54:
				_player->_position.x = 322;
				_player->_position.y = 220;
				_player->_walkTarget = _player->_position;
				_player->_direction = 2;
				_player->updateSprite();
				break;
			case 55:
				g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
				g_engine->changeToRoom(15);
				break;
			case 56:
				_player->_position.x = 162;
				_player->_position.y = 206;
				_player->_walkTarget = _player->_position;
				_player->_direction = 2;
				_player->updateSprite();
				break;
			default:
				break;
			}
		}
		if (_frameAdvanced && _animIndexTbl[0] == 1) {
			// FUN_1208_0dacg_engine->_sound_related(0xd,CONCAT11(extraout_AH_05,5));
			// PlaySound(1,5,-1);
		}
		break;
	}
	case 57:
	case 58:
		advanceAnimationFrame((_otherNspAnimationType_maybe - 57) & 1);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]; //_player->_animations.getAnimAt(0).frameNo[animIndexTbl[0]]];
		} else if (_otherNspAnimationType_maybe == 57) {
			g_engine->_previousRoomNumber = g_engine->_room->_roomNumber;
			g_engine->changeToRoom(g_engine->_room->_roomNumber == 41 ? 44 : 41);
		} else {
			_player->_direction = 2;
		}
		break;
	case 59: // use shard on mirror
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			_player->_direction = 1;
			_player->updateSprite();
		}
		if (_frameAdvanced && _player->_frameIdx == 3) {
			g_engine->playSound(26, 5, -1);
		}
		break;
	case 60: // turn on alien computer
		g_engine->_room->advanceFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = g_engine->_room->_locationSprites.getAnimAt(0)._frameNo[g_engine->_room->_locObjFrame[0]];
		} else {
			_isPlayingAnimation_maybe = false;
			_objectVar[187] = 1;
		}
		break;
	case 61: // turn off alien computer
		g_engine->_room->advanceFrame(1);
		if (!_objRestarted) {
			_player->_frameIdx = g_engine->_room->_locationSprites.getAnimAt(1)._frameNo[g_engine->_room->_locObjFrame[1]];
		} else {
			_isPlayingAnimation_maybe = false;
		}
		_objectVar[187] = 0;
		break;
	case 62: // bang cup against bars
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			bool bVar5 = _objectVar.getMoveObjectRoom(18) == 250;
			if (!bVar5) {
				if (g_engine->_inventory.hasObject(18)) {
					bVar5 = true;
				}
			}
			if (bVar5) {
				_isPlayingAnimation_maybe = false;
				_objectVar.setObjectRunningCode(53, 1);
				_player->loadAnimations("copcard.nsp");
				_animIndexTbl[0] = 0;
				_spriteAnimCountdownTimer[0] = 3;
				_objectVar[1] = 2000;
			} else {
				g_engine->_console->addTextLine("The cops ignore your demands for attention.");
			}
		}
		break;
	case 65:
		advanceAnimationFrame(0);
		if (!_objRestarted) {
			_player->_frameIdx = _player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]];
		} else {
			_player->updateSprite();
		}
		break;
	default:
		error("Unhandled animation type! %d", _otherNspAnimationType_maybe);
	}
}

void Animation::advanceAnimationFrame(int nspAminIdx) {
	if (!_scaleSequence) {
		g_engine->_scaledWalkSpeed_maybe = 1000;
	} else {
		g_engine->_room->calculateScaledSpriteDimensions(10, 10, _player->_position.y);
	}
	_objRestarted = false;
	_frameAdvanced = false;
	const Obt &anim = _player->_animations.getAnimAt(nspAminIdx);
	_spriteAnimCountdownTimer[nspAminIdx] = _spriteAnimCountdownTimer[nspAminIdx] - 1;
	if (_spriteAnimCountdownTimer[nspAminIdx] < 1) {
		_frameAdvanced = true;
		_animIndexTbl[nspAminIdx] = _animIndexTbl[nspAminIdx] + 1;
		_player->_position.x += ((int16)anim._deltaX[_animIndexTbl[nspAminIdx]] * g_engine->_scaledWalkSpeed_maybe) / 1000;
		_player->_position.y += ((int16)anim._deltaY[_animIndexTbl[nspAminIdx]] * g_engine->_scaledWalkSpeed_maybe) / 1000;
		if (_animIndexTbl[nspAminIdx] == anim._numFrames) {
			_animIndexTbl[nspAminIdx] = 0;
			_objRestarted = true;
			_isPlayingAnimation_maybe = false;
			_player->_walkTarget.x = _player->_position.x;
			_player->_walkTarget.y = _player->_position.y;
			_player->updateSprite();
		}
		_spriteAnimCountdownTimer[nspAminIdx] = anim._frameDuration[_animIndexTbl[nspAminIdx]];
	}
}

static constexpr uint8 dcopList[100] = {
	0, 1, 2, 3,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 1,
	2, 3, 2, 1,
	0, 1, 2, 3,
	2, 3, 2, 3,
	2, 1, 0, 1,
	2, 1, 2, 1,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 1,
	2, 1, 2, 3,
	2, 1, 0, 1,
	2, 1, 2, 1,
	2, 3, 2, 1
};

void Animation::dCopAnim() {
	_player->loadAnimations("dcopb.nsp");
	g_engine->showFullscreenPic("dcopb.pic");
	_animIndexTbl[0] = 0;
	_spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0)._frameDuration[0];

	g_engine->_sprites.clearSpriteDrawList();
	g_engine->_console->printTosText(923);

	uint8 lipsIdx = 0;
	while (g_engine->_sound->isPlayingSpeech()) {
		g_engine->_sprites.clearSpriteDrawList();

		g_engine->drawFullscreenPic();

		advanceAnimationFrame(0);
		const Sprite &dcopSprite = _player->_animations.getSpriteAt(dcopList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(310, 180, &dcopSprite, 255, dcopSprite._width, dcopSprite._height, false);
		g_engine->_sprites.drawSprites();

		g_engine->_console->draw();
		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		for (int i = 0; i < 6; i++) {
			g_engine->wait();
		}
	}
	g_engine->removeFullscreenPic();
}

static constexpr uint8 sargoList[100] = {
	0, 1, 2, 3,
	4, 3, 2, 1,
	0, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	0, 1, 2, 3,
	4, 3, 4, 3,
	2, 1, 0, 1,
	2, 3, 4, 3,
	2, 1, 0, 1,
	0, 1, 2, 3,
	2, 3, 2, 3,
	2, 1, 0, 1,
	2, 3, 2, 1,
	0, 4, 3, 2,
	1, 1, 2, 3,
	4, 3, 2, 1,
	0, 1, 0, 1,
	2, 3, 2, 3,
	2, 3, 2, 1,
	2, 1, 0, 0,
	1, 2, 3, 2,
	1, 0, 1, 2,
	3, 4, 3, 2,
	3, 2, 1, 1
};

void Animation::sargoAnim() {
	g_engine->_cursor.showCursor(false);
	_player->loadAnimations("sargo.nsp");
	g_engine->showFullscreenPic("sargo.pic");
	_animIndexTbl[0] = 0;
	_spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0)._frameDuration[0];

	g_engine->_console->printTosText(916);

	uint8 dialogIdx = 79;
	uint8 lipsIdx = 0;
	while (g_engine->_sound->isPlayingSpeech() || dialogIdx < 81) {
		g_engine->_sprites.clearSpriteDrawList();

		g_engine->drawFullscreenPic();

		advanceAnimationFrame(0);
		const Sprite &sargoSprite = _player->_animations.getSpriteAt(sargoList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(334, 160, &sargoSprite, 255, sargoSprite._width, sargoSprite._height, false);
		g_engine->_sprites.drawSprites();

		g_engine->_console->draw();
		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		if (!g_engine->_sound->isPlayingSpeech()) {
			dialogIdx++;
			if (dialogIdx == 80) {
				g_engine->_console->printTosText(917);
			}
		}
		g_engine->waitxticks(1);
	}
	g_engine->removeFullscreenPic();
	g_engine->_cursor.showCursor(true);
}

void Animation::gancAnim() {
	g_engine->_cursor.showCursor(false);
	_player->loadAnimations("ganc.nsp");
	g_engine->showFullscreenPic("ganc.pic");
	_animIndexTbl[0] = 0;
	_spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0)._frameDuration[0];

	g_engine->_sound->playMusic(MusicId::kDth);
	g_engine->playSound(0, 6, -1);
	g_engine->_console->printTosText(0);
	g_engine->_console->draw();

	while (!_objRestarted) {
		g_engine->_sprites.clearSpriteDrawList();

		g_engine->drawFullscreenPic();

		advanceAnimationFrame(0);
		const Sprite &sargoSprite = _player->_animations.getSpriteAt(_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]);
		g_engine->_sprites.addSpriteToDrawList(303, 122, &sargoSprite, 255, sargoSprite._width, sargoSprite._height, false);
		g_engine->_sprites.drawSprites();


		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		g_system->delayMillis(20);
	}
	g_engine->removeFullscreenPic();
	g_engine->_cursor.showCursor(true);
	stuffPlayer();
}

static constexpr uint8 keeperList[250] = {
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 12,
	13, 12, 11, 10,
	11, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	 0,  0,  1,  1,
	 2,  2,  3,  3,
	 4,  4,  5,  5,
	 6,  6,  7,  7,
	 8,  8,  9,  9,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 10,
	11, 11, 12, 12,
	11, 12, 12, 13,
	12, 12, 12, 13,
	13, 12, 13, 12,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 11, 12, 13,
	12, 12, 13, 10,
	11, 10, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	10, 10, 11, 11,
	12, 12, 11, 12,
	12, 13, 12, 12,
	12, 13, 13, 12,
	13, 12, 11, 10,
	 0,  1,  2,  3,
	 4,  5,  6,  7,
	 8,  9, 10, 11,
	12, 13, 12, 12,
	13, 10, 11, 10,
	 0,  0,  0,  0,
	 0,  0,  0,  0,
	 0,  0
};

void Animation::keeperAdmin() {
	g_engine->_cursor.showCursor(false);
	_player->loadAnimations("keeper.nsp");
	g_engine->showFullscreenPic("keeper.pic");
	_animIndexTbl[0] = 0;
	_spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0)._frameDuration[0];

	g_engine->_console->printTosText(913);

	uint8 dialogIdx = 73;
	uint8 lipsIdx = 0;
	while (g_engine->_sound->isPlayingSpeech() || dialogIdx < 76) {
		g_engine->_sprites.clearSpriteDrawList();

		g_engine->drawFullscreenPic();

		advanceAnimationFrame(0);
		const Sprite &keeperSprite = _player->_animations.getSpriteAt(keeperList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(254, 117, &keeperSprite, 255, keeperSprite._width, keeperSprite._height, false);
		g_engine->_sprites.drawSprites();

		g_engine->_console->draw();
		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		lipsIdx++;
		if (lipsIdx == 250) {
			lipsIdx = 0;
		}

		if (!g_engine->_sound->isPlayingSpeech()) {
			dialogIdx++;
			if (dialogIdx == 74) {
				g_engine->_console->printTosText(914);
			} else if (dialogIdx == 75) {
				g_engine->_console->printTosText(915);
			}
		}
		g_engine->waitxticks(1);
	}
	g_engine->removeFullscreenPic();
	g_engine->_cursor.showCursor(true);
}

void Animation::stuffPlayer() {
	g_engine->_cursor.showCursor(false);
	_player->loadAnimations("labparts.nsp");
	g_engine->showFullscreenPic("lab.pic");
	const Sprite &alienSprite = _player->_animations.getSpriteAt(8);

	bool updateCounter = false;
	int counter = 0;
	while (counter < 8) {
		g_engine->_sprites.clearSpriteDrawList();

		g_engine->drawFullscreenPic();

		const Sprite &mikeSprite = _player->_animations.getSpriteAt(counter);
		g_engine->_sprites.addSpriteToDrawList(103, 93, &mikeSprite, 255, mikeSprite._width, mikeSprite._height, false);
		g_engine->_sprites.addSpriteToDrawList(226, 100, &alienSprite, 255, alienSprite._width, alienSprite._height, false);

		g_engine->_sprites.drawSprites();

		g_engine->_console->draw();
		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		updateCounter = !updateCounter;
		if (updateCounter) {
			counter++;
		}
		g_engine->waitxticks(1);
	}
	g_engine->waitxticks(3);
	g_engine->removeFullscreenPic();
	g_engine->_sprites.clearSpriteDrawList();
	g_engine->_cursor.showCursor(true);
	g_engine->_cutscene.play('Z');
	g_engine->_sound->stopMusic();
}

void Animation::runDrekethSequence() {
	bool updateCounter = false;
	int counter = 0;
	if (!g_engine->isCdVersion()) {
		g_engine->_sound->playMusic(MusicId::kDth);
	}

	g_engine->_console->printTosText(2);
	g_engine->_console->draw();
	while (counter < 6) {
		g_engine->_sprites.clearSpriteDrawList();

		const Sprite &drekethBaseSprite = g_engine->_room->_locationSprites.getSpriteAt(15);
		const Sprite &openingAnimation = g_engine->_room->_locationSprites.getSpriteAt(counter);
		g_engine->_sprites.addSpriteToDrawList(346, 133, &drekethBaseSprite, 255, drekethBaseSprite._width, drekethBaseSprite._height, false);
		g_engine->_sprites.addSpriteToDrawList(395, 133, &openingAnimation, 255, openingAnimation._width, openingAnimation._height, false);
		adddrekbutt();

		g_engine->_sprites.drawSprites();

		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		updateCounter = !updateCounter;
		if (updateCounter) {
			counter++;
		}
		g_engine->waitxticks(1);
	}

	while (counter < 9) {
		g_engine->_sprites.clearSpriteDrawList();

		const Sprite &grabMikeSprite = g_engine->_room->_locationSprites.getSpriteAt(counter);
		g_engine->_sprites.addSpriteToDrawList(346, 133, &grabMikeSprite, 255, grabMikeSprite._width, grabMikeSprite._height, false);
		adddrekbutt();

		g_engine->_sprites.drawSprites();

		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		updateCounter = !updateCounter;
		if (updateCounter) {
			counter++;
			if (counter == 7) {
				g_engine->playSound(19, 5, -1);
			}
		}
		g_engine->waitxticks(1);
	}

	while (counter < 13) {
		g_engine->_sprites.clearSpriteDrawList();

		const Sprite &drekethBaseSprite = g_engine->_room->_locationSprites.getSpriteAt(14);
		const Sprite &closingAnimation = g_engine->_room->_locationSprites.getSpriteAt(counter);
		g_engine->_sprites.addSpriteToDrawList(346, 175, &drekethBaseSprite, 255, drekethBaseSprite._width, drekethBaseSprite._height, false);
		g_engine->_sprites.addSpriteToDrawList(346, 133, &closingAnimation, 255, closingAnimation._width, closingAnimation._height, false);
		adddrekbutt();

		g_engine->_sprites.drawSprites();

		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		updateCounter = !updateCounter;
		if (updateCounter) {
			counter++;
		}
		g_engine->waitxticks(1);
	}
	stuffPlayer();
}

void Animation::adddrekbutt() {
	const Sprite &drekButt = g_engine->_room->_locationSprites.getSpriteAt(16);
	g_engine->_sprites.addSpriteToDrawList(466, 133, &drekButt, 255, drekButt._width, drekButt._height, false);

}

static constexpr uint8 libList[100] = {
	5,  6, 7,  8,
	9, 10, 9,  8,
	7,  6, 5,  6,
	7,  6, 7,  8,
	7,  6, 5,  6,
	5,  6, 7,  8,
	9, 10, 9,  8,
	7,  6, 5,  6,
	7,  6, 7,  8,
	7,  6, 5,  6,
	5,  6, 7,  8,
	9, 10, 9,  8,
	7,  6, 5,  6,
	5,  6, 7,  6,
	7,  8, 7,  6,
	5,  6, 7,  6,
	7,  8, 7,  6,
	5,  6, 5,  6,
	5,  6, 7,  6,
	7,  8, 7,  6,
	5,  6, 7,  8,
	9, 10, 9,  8,
	7,  6, 5,  6,
	7,  8, 9, 10,
	9,  8, 7,  6
};

void Animation::libAnim(bool pickingUpReservedBook) {
	_player->loadAnimations("libparts.nsp");
	g_engine->showFullscreenPic("libinlib.pic");

	g_engine->_console->printTosText(pickingUpReservedBook ? 928 : 924);

	_spriteAnimCountdownTimer[0] = _player->_animations.getAnimAt(0)._frameDuration[0];
	uint8 lipsIdx = 0;
	while (g_engine->_sound->isPlayingSpeech()) {
		g_engine->_sprites.clearSpriteDrawList();
		g_engine->_frame.draw();
		g_engine->drawFullscreenPic();
		g_engine->_console->draw();

		advanceAnimationFrame(0);
		const Sprite &eyesSprite = _player->_animations.getSpriteAt(_player->_animations.getAnimAt(0)._frameNo[_animIndexTbl[0]]);
		g_engine->_sprites.addSpriteToDrawList(255, 114, &eyesSprite, 255, eyesSprite._width, eyesSprite._height, false);
		advanceAnimationFrame(1);

		const Sprite &mouthSprite = _player->_animations.getSpriteAt(libList[lipsIdx]);
		g_engine->_sprites.addSpriteToDrawList(255, 154, &mouthSprite, 255, mouthSprite._width, mouthSprite._height, false);

		g_engine->_sprites.drawSprites();

		g_engine->_screen->makeAllDirty();
		g_engine->_screen->update();

		lipsIdx++;
		if (lipsIdx == 100) {
			lipsIdx = 0;
		}

		for (int i = 0; i < 6; i++) {
			g_engine->wait();
		}
	}

	g_engine->removeFullscreenPic();

	if (pickingUpReservedBook) {
		_objectVar[49] = 1;
		_objectVar[62] = 0;
		g_engine->_cutscene.play('G');
	}
}

} // End of namespace Darkseed
