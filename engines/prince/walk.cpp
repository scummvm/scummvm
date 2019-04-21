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

#include "prince/prince.h"
#include "prince/hero.h"
#include "prince/script.h"

namespace Prince {

void PrinceEngine::walkTo() {
	if (_mainHero->_visible) {
		_mainHero->freeHeroAnim();
		_mainHero->freeOldMove();
		_interpreter->storeNewPC(_script->_scriptInfo.usdCode);
		int destX, destY;
		if (_optionsMob != -1) {
			destX = _mobList[_optionsMob]._examPosition.x;
			destY = _mobList[_optionsMob]._examPosition.y;
			_mainHero->_destDirection = _mobList[_optionsMob]._examDirection;
		} else {
			Common::Point mousePos = _system->getEventManager()->getMousePos();
			destX = mousePos.x + _picWindowX;
			destY = mousePos.y + _picWindowY;
			_mainHero->_destDirection = 0;
		}
		_mainHero->_coords = makePath(kMainHero, _mainHero->_middleX, _mainHero->_middleY, destX, destY);
		if (_mainHero->_coords != nullptr) {
			_mainHero->_currCoords = _mainHero->_coords;
			_mainHero->_dirTab = _directionTable;
			_mainHero->_currDirTab = _directionTable;
			_directionTable = nullptr;
			_mainHero->_state = Hero::kHeroStateMove;
			moveShandria();
		}
	}
}

void PrinceEngine::moveRunHero(int heroId, int x, int y, int dir, bool runHeroFlag) {
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _mainHero;
	} else if (heroId == 1) {
		hero = _secondHero;
	}

	if (hero != nullptr) {
		if (dir) {
			hero->_destDirection = dir;
		}
		if (x || y) {
			hero->freeOldMove();
			hero->_coords = makePath(heroId, hero->_middleX, hero->_middleY, x, y);
			if (hero->_coords != nullptr) {
				hero->_currCoords = hero->_coords;
				hero->_dirTab = _directionTable;
				hero->_currDirTab = _directionTable;
				_directionTable = nullptr;
				if (runHeroFlag) {
					hero->_state = Hero::kHeroStateRun;
				} else {
					hero->_state = Hero::kHeroStateMove;
				}
				if (heroId == kMainHero && _mouseFlag) {
					moveShandria();
				}
			}
		} else {
			hero->freeOldMove();
			hero->_state = Hero::kHeroStateTurn;
		}
		hero->freeHeroAnim();
		hero->_visible = 1;
	}
}

// Modified version of Graphics::drawLine() to allow breaking the loop and return value
int PrinceEngine::drawLine(int x0, int y0, int x1, int y1, int (*plotProc)(int, int, void *), void *data) {
	// Bresenham's line algorithm, as described by Wikipedia
	const bool steep = ABS(y1 - y0) > ABS(x1 - x0);

	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}

	const int delta_x = ABS(x1 - x0);
	const int delta_y = ABS(y1 - y0);
	const int delta_err = delta_y;
	int x = x0;
	int y = y0;
	int err = 0;

	const int x_step = (x0 < x1) ? 1 : -1;
	const int y_step = (y0 < y1) ? 1 : -1;

	int stopFlag = 0;
	if (steep)
		stopFlag = (*plotProc)(y, x, data);
	else
		stopFlag = (*plotProc)(x, y, data);

	while (x != x1 && !stopFlag) {
		x += x_step;
		err += delta_err;
		if (2 * err > delta_x) {
			y += y_step;
			err -= delta_x;
		}
		if (steep)
			stopFlag = (*plotProc)(y, x, data);
		else
			stopFlag = (*plotProc)(x, y, data);
	}
	return stopFlag;
}

int PrinceEngine::getPixelAddr(byte *pathBitmap, int x, int y) {
	int mask = 128 >> (x & 7);
	byte value = pathBitmap[x / 8 + y * 80];
	return (mask & value);
}

void PrinceEngine::findPoint(int x, int y) {
	_fpX = x;
	_fpY = y;

	if (getPixelAddr(_roomPathBitmap, x, y)) {
		return;
	}

	int fpL = x;
	int fpU = y;
	int fpR = x;
	int fpD = y;

	while (1) {
		if (fpD != kMaxPicHeight) {
			if (getPixelAddr(_roomPathBitmap, x, fpD)) {
				_fpX = x;
				_fpY = fpD;
				break;
			}
			fpD++;
		}
		if (fpU) {
			if (getPixelAddr(_roomPathBitmap, x, fpU)) {
				_fpX = x;
				_fpY = fpU;
				break;
			}
			fpU--;
		}
		if (fpL) {
			if (getPixelAddr(_roomPathBitmap, fpL, y)) {
				_fpX = fpL;
				_fpY = y;
				break;
			}
			fpL--;
		}
		if (fpR != _sceneWidth) {
			if (getPixelAddr(_roomPathBitmap, fpR, y)) {
				_fpX = fpR;
				_fpY = y;
				break;
			}
			fpR++;
		}
		if (!fpU && (fpD == kMaxPicHeight)) {
			if (!fpL && (fpR == _sceneWidth)) {
				break;
			}
		}
	}
}

Direction PrinceEngine::makeDirection(int x1, int y1, int x2, int y2) {
	if (x1 != x2) {
		if (y1 != y2) {
			if (x1 > x2) {
				if (y1 > y2) {
					if (x1 - x2 >= y1 - y2) {
						return kDirLU;
					} else {
						return kDirUL;
					}
				} else {
					if (x1 - x2 >= y2 - y1) {
						return kDirLD;
					} else {
						return kDirDL;
					}
				}
			} else {
				if (y1 > y2) {
					if (x2 - x1 >= y1 - y2) {
						return kDirRU;
					} else {
						return kDirUR;
					}
				} else {
					if (x2 - x1 >= y2 - y1) {
						return kDirRD;
					} else {
						return kDirDR;
					}
				}
			}
		} else {
			if (x1 >= x2) {
				return kDirL;
			} else {
				return kDirR;
			}
		}
	} else {
		if (y1 >= y2) {
			return kDirU;
		} else {
			return kDirD;
		}
	}
}

void PrinceEngine::specialPlot(int x, int y) {
	if (_coords < _coordsBufEnd) {
		WRITE_LE_UINT16(_coords, x);
		_coords += 2;
		WRITE_LE_UINT16(_coords, y);
		_coords += 2;
		specialPlot2(x, y);
	}
}

void PrinceEngine::specialPlot2(int x, int y) {
	int mask = 128 >> (x & 7);
	_roomPathBitmapTemp[x / 8 + y * 80] |= mask;
}

void PrinceEngine::specialPlotInside(int x, int y) {
	if (_coords < _coordsBufEnd) {
		WRITE_LE_UINT16(_coords, x);
		_coords += 2;
		WRITE_LE_UINT16(_coords, y);
		_coords += 2;
	}
}

int PrinceEngine::plotTraceLine(int x, int y, void *data) {
	PrinceEngine *traceLine = (PrinceEngine *)data;
	if (!traceLine->_traceLineFirstPointFlag) {
		if (!traceLine->getPixelAddr(traceLine->_roomPathBitmapTemp, x, y)) {
			if (traceLine->getPixelAddr(traceLine->_roomPathBitmap, x, y)) {
				traceLine->specialPlotInside(x, y);
				traceLine->_traceLineLen++;
				return 0;
			} else {
				return -1;
			}
		} else {
			return 1;
		}
	} else {
		traceLine->_traceLineFirstPointFlag = false;
		return 0;
	}
}

int PrinceEngine::leftDownDir() {
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::leftDir() {
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::leftUpDir() {
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightDownDir() {
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightDir() {
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::rightUpDir() {
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upLeftDir() {
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upDir() {
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::upRightDir() {
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downLeftDir() {
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downDir() {
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::downRightDir() {
	if (!checkRightDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDownDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkRightUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	if (!checkLeftUpDir()) {
		specialPlot(_checkX, _checkY);
		return 0;
	}
	return -1;
}

int PrinceEngine::cpe() {
	if ((*(_checkBitmap - kPBW) & _checkMask)) {
		if ((*(_checkBitmap + kPBW) & _checkMask)) {
			int value;
			switch (_checkMask) {
			case 128:
				value = READ_LE_UINT16(_checkBitmap - 1);
				value &= 0x4001;
				if (value != 0x4001) {
					return 0;
				}
				break;
			case 64:
				value = *_checkBitmap;
				value &= 0xA0;
				if (value != 0xA0) {
					return 0;
				}
				break;
			case 32:
				value = *_checkBitmap;
				value &= 0x50;
				if (value != 0x50) {
					return 0;
				}
				break;
			case 16:
				value = *_checkBitmap;
				value &= 0x28;
				if (value != 0x28) {
					return 0;
				}
				break;
			case 8:
				value = *_checkBitmap;
				value &= 0x14;
				if (value != 0x14) {
					return 0;
				}
				break;
			case 4:
				value = *_checkBitmap;
				value &= 0xA;
				if (value != 0xA) {
					return 0;
				}
				break;
			case 2:
				value = *_checkBitmap;
				value &= 0x5;
				if (value != 0x5) {
					return 0;
				}
				break;
			case 1:
				value = READ_LE_UINT16(_checkBitmap);
				value &= 0x8002;
				if (value != 0x8002) {
					return 0;
				}
				break;
			default:
				error("Wrong _checkMask value - cpe()");
				break;
			}
			_checkX = _rembX;
			_checkY = _rembY;
			_checkBitmapTemp = _rembBitmapTemp;
			_checkBitmap = _rembBitmap;
			_checkMask = _rembMask;
			return -1;
		}
		return 0;
	}
	return 0;
}

int PrinceEngine::checkLeftDownDir() {
	if (_checkX && _checkY != (kMaxPicHeight / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap + kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp + kPBW) & tempMask)) {
					_checkBitmap += kPBW;
					_checkBitmapTemp += kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + kPBW - 1) & 1)) {
				if (!(*(_checkBitmapTemp + kPBW - 1) & 1)) {
					_checkBitmap += (kPBW - 1);
					_checkBitmapTemp += (kPBW - 1);
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		_checkY++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkLeftDir() {
	if (_checkX) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap) & tempMask)) {
				if (!(*(_checkBitmapTemp) & tempMask)) {
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - 1) & 1)) {
				if (!(*(_checkBitmapTemp - 1) & 1)) {
					_checkBitmap--;
					_checkBitmapTemp--;
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkDownDir() {
	if (_checkY != (kMaxPicHeight / 2 - 1)) {
		if ((*(_checkBitmap + kPBW) & _checkMask)) {
			if (!(*(_checkBitmapTemp + kPBW) & _checkMask)) {
				_checkBitmap += kPBW;
				_checkBitmapTemp += kPBW;
				_checkY++;
				return cpe();
			} else {
				return 1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int PrinceEngine::checkUpDir() {
	if (_checkY) {
		if ((*(_checkBitmap - kPBW) & _checkMask)) {
			if (!(*(_checkBitmapTemp - kPBW) & _checkMask)) {
				_checkBitmap -= kPBW;
				_checkBitmapTemp -= kPBW;
				_checkY--;
				return cpe();
			} else {
				return 1;
			}
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap) & tempMask)) {
				if (!(*(_checkBitmapTemp) & tempMask)) {
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + 1) & 128)) {
				if (!(*(_checkBitmapTemp + 1) & 128)) {
					_checkBitmap++;
					_checkBitmapTemp++;
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkLeftUpDir() {
	if (_checkX && _checkY) {
		int tempMask = _checkMask;
		if (tempMask != 128) {
			tempMask <<= 1;
			if ((*(_checkBitmap - kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp - kPBW) & tempMask)) {
					_checkBitmap -= kPBW;
					_checkBitmapTemp -= kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - (kPBW + 1)) & 1)) {
				if (!(*(_checkBitmapTemp - (kPBW + 1)) & 1)) {
					_checkBitmap -= (kPBW + 1);
					_checkBitmapTemp -= (kPBW + 1);
					_checkMask = 1;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX--;
		_checkY--;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightDownDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1) && _checkY != (kMaxPicHeight / 2 - 1)) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap + kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp + kPBW) & tempMask)) {
					_checkBitmap += kPBW;
					_checkBitmapTemp += kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap + kPBW + 1) & 128)) {
				if (!(*(_checkBitmapTemp + kPBW + 1) & 128)) {
					_checkBitmap += kPBW + 1;
					_checkBitmapTemp += kPBW + 1;
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		_checkY++;
		return cpe();
	} else {
		return -1;
	}
}

int PrinceEngine::checkRightUpDir() {
	if (_checkX != (kMaxPicWidth / 2 - 1) && _checkY) {
		int tempMask = _checkMask;
		if (tempMask != 1) {
			tempMask >>= 1;
			if ((*(_checkBitmap - kPBW) & tempMask)) {
				if (!(*(_checkBitmapTemp - kPBW) & tempMask)) {
					_checkBitmap -= kPBW;
					_checkBitmapTemp -= kPBW;
					_checkMask = tempMask;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		} else {
			if ((*(_checkBitmap - kPBW + 1) & 128)) {
				if (!(*(_checkBitmapTemp - kPBW + 1) & 128)) {
					_checkBitmap -= (kPBW - 1);
					_checkBitmapTemp -= (kPBW - 1);
					_checkMask = 128;
				} else {
					return 1;
				}
			} else {
				return -1;
			}
		}
		_checkX++;
		_checkY--;
		return cpe();
	} else {
		return -1;
	}
}

bool PrinceEngine::tracePath(int x1, int y1, int x2, int y2) {
	for (uint i = 0; i < kPathBitmapLen; i++) {
		_roomPathBitmapTemp[i] = 0;
	}
	if (x1 != x2 || y1 != y2) {
		if (getPixelAddr(_roomPathBitmap, x1, y1)) {
			if (getPixelAddr(_roomPathBitmap, x2, y2)) {
				_coords = _coordsBuf;
				specialPlot(x1, y1);

				int x = x1;
				int y = y1;

				while (1) {
					int btx = x;
					int bty = y;
					byte *bcad = _coords;

					_traceLineLen = 0;
					_traceLineFirstPointFlag = true;
					int drawLineFlag = drawLine(x, y, x2, y2, &this->plotTraceLine, this);

					if (!drawLineFlag) {
						return true;
					} else if (drawLineFlag == -1 && _traceLineLen >= 2) {
						byte *tempCorrds = bcad;
						while (tempCorrds != _coords) {
							x = READ_LE_UINT16(tempCorrds);
							y = READ_LE_UINT16(tempCorrds + 2);
							tempCorrds += 4;
							specialPlot2(x, y);
						}
					} else {
						_coords = bcad;
						x = btx;
						y = bty;
					}

					Direction dir = makeDirection(x, y, x2, y2);

					_rembBitmapTemp = &_roomPathBitmapTemp[x / 8 + y * 80];
					_rembBitmap = &_roomPathBitmap[x / 8 + y * 80];
					_rembMask = 128 >> (x & 7);
					_rembX = x;
					_rembY = y;

					_checkBitmapTemp = _rembBitmapTemp;
					_checkBitmap = _rembBitmap;
					_checkMask = _rembMask;
					_checkX = _rembX;
					_checkY = _rembY;

					int result;
					switch (dir) {
					case kDirLD:
						result = leftDownDir();
						break;
					case kDirL:
						result = leftDir();
						break;
					case kDirLU:
						result = leftUpDir();
						break;
					case kDirRD:
						result = rightDownDir();
						break;
					case kDirR:
						result = rightDir();
						break;
					case kDirRU:
						result = rightUpDir();
						break;
					case kDirUL:
						result = upLeftDir();
						break;
					case kDirU:
						result = upDir();
						break;
					case kDirUR:
						result = upRightDir();
						break;
					case kDirDL:
						result = downLeftDir();
						break;
					case kDirD:
						result = downDir();
						break;
					case kDirDR:
						result = downRightDir();
						break;
					default:
						result = -1;
						error("tracePath: wrong direction %d", dir);
						break;
					}

					if (result) {
						byte *tempCoords = _coords;
						tempCoords -= 4;
						if (tempCoords > _coordsBuf) {
							int tempX = READ_LE_UINT16(tempCoords);
							int tempY = READ_LE_UINT16(tempCoords + 2);
							if (_checkX == tempX && _checkY == tempY) {
								_coords = tempCoords;
							}
							x = READ_LE_UINT16(tempCoords);
							y = READ_LE_UINT16(tempCoords + 2);
						} else {
							return false;
						}
					} else {
						x = _checkX;
						y = _checkY;
					}
				}
				return true;
			} else {
				error("tracePath: wrong destination point");
			}
		} else {
			error("tracePath: wrong start point");
		}
	} else {
		error("tracePath: same point");
	}
}

void PrinceEngine::specialPlotInside2(int x, int y) {
	WRITE_LE_UINT16(_coords2, x);
	_coords2 += 2;
	WRITE_LE_UINT16(_coords2, y);
	_coords2 += 2;
}

int PrinceEngine::plotTracePoint(int x, int y, void *data) {
	PrinceEngine *tracePoint = (PrinceEngine *)data;
	if (!tracePoint->_tracePointFirstPointFlag) {
		if (tracePoint->getPixelAddr(tracePoint->_roomPathBitmap, x, y)) {
			tracePoint->specialPlotInside2(x, y);
			return 0;
		} else {
			return -1;
		}
	} else {
		tracePoint->_tracePointFirstPointFlag = false;
		return 0;
	}
}

void PrinceEngine::approxPath() {
	byte *oldCoords;
	_coords2 = _coordsBuf2;
	byte *tempCoordsBuf = _coordsBuf; // first point on path
	byte *tempCoords = _coords;
	if (tempCoordsBuf != tempCoords) {
		tempCoords -= 4; // last point on path
		while (tempCoordsBuf != tempCoords) {
			int x1 = READ_LE_UINT16(tempCoords);
			int y1 = READ_LE_UINT16(tempCoords + 2);
			int x2 = READ_LE_UINT16(tempCoordsBuf);
			int y2 = READ_LE_UINT16(tempCoordsBuf + 2);
			tempCoordsBuf += 4;
			//TracePoint
			oldCoords = _coords2;
			if (_coords2 == _coordsBuf2) {
				WRITE_LE_UINT16(_coords2, x1);
				WRITE_LE_UINT16(_coords2 + 2, y1);
				_coords2 += 4;
			} else {
				int testX = READ_LE_UINT16(_coords2 - 4);
				int testY = READ_LE_UINT16(_coords2 - 2);
				if (testX != x1 || testY != y1) {
					WRITE_LE_UINT16(_coords2, x1);
					WRITE_LE_UINT16(_coords2 + 2, y1);
					_coords2 += 4;
				}
			}
			_tracePointFirstPointFlag = true;
			bool drawLineFlag = drawLine(x1, y1, x2, y2, &this->plotTracePoint, this);
			if (!drawLineFlag) {
				tempCoords = tempCoordsBuf - 4;
				tempCoordsBuf = _coordsBuf;
			} else {
				_coords2 = oldCoords;
			}
		}
	}
}

void PrinceEngine::freeDirectionTable() {
	if (_directionTable != nullptr) {
		free(_directionTable);
		_directionTable = nullptr;
	}
}

int PrinceEngine::scanDirectionsFindNext(byte *tempCoordsBuf, int xDiff, int yDiff) {

	int tempX, tempY, direction;

	tempX = Hero::kHeroDirLeft;
	if (xDiff < 0) {
		tempX = Hero::kHeroDirRight;
	}

	tempY = Hero::kHeroDirUp;
	if (yDiff < 0) {
		tempY = Hero::kHeroDirDown;
	}

	while (1) {
		int againPointX1 = READ_LE_UINT16(tempCoordsBuf);
		int againPointY1 = READ_LE_UINT16(tempCoordsBuf + 2);
		tempCoordsBuf += 4;

		if (tempCoordsBuf == _coords) {
			direction = tempX;
			break;
		}

		int dX = againPointX1 - READ_LE_UINT16(tempCoordsBuf);
		int dY = againPointY1 - READ_LE_UINT16(tempCoordsBuf + 2);

		if (dX != xDiff) {
			direction = tempY;
			break;
		}

		if (dY != yDiff) {
			direction = tempX;
			break;
		}
	}
	return direction;
}

void PrinceEngine::scanDirections() {
	freeDirectionTable();
	byte *tempCoordsBuf = _coordsBuf;
	if (tempCoordsBuf != _coords) {
		int size = (_coords - tempCoordsBuf) / 4 + 1; // number of coord points plus one for end marker
		_directionTable = (byte *)malloc(size);
		byte *tempDirTab = _directionTable;
		int direction = -1;
		int lastDirection = -1;

		while (1) {
			int x1 = READ_LE_UINT16(tempCoordsBuf);
			int y1 = READ_LE_UINT16(tempCoordsBuf + 2);
			tempCoordsBuf += 4;
			if (tempCoordsBuf == _coords) {
				break;
			}
			int x2 = READ_LE_UINT16(tempCoordsBuf);
			int y2 = READ_LE_UINT16(tempCoordsBuf + 2);

			int xDiff = x1 - x2;
			int yDiff = y1 - y2;

			if (xDiff) {
				if (yDiff) {
					if (lastDirection != -1) {
						direction = lastDirection;
						if (direction == Hero::kHeroDirLeft) {
							if (xDiff < 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else if (direction == Hero::kHeroDirRight) {
							if (xDiff >= 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else if (direction == Hero::kHeroDirUp) {
							if (yDiff < 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						} else {
							if (yDiff >= 0) {
								direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
							}
						}
					} else {
						direction = scanDirectionsFindNext(tempCoordsBuf, xDiff, yDiff);
					}
				} else {
					direction = Hero::kHeroDirLeft;
					if (xDiff < 0) {
						direction = Hero::kHeroDirRight;
					}
				}
			} else {
				if (yDiff) {
					direction = Hero::kHeroDirUp;
					if (yDiff < 0) {
						direction = Hero::kHeroDirDown;
					}
				} else {
					direction = lastDirection;
				}
			}
			lastDirection = direction;
			*tempDirTab = direction;
			tempDirTab++;
		}
		*tempDirTab = *(tempDirTab - 1);
		tempDirTab++;
		*tempDirTab = 0;
	}
}

void PrinceEngine::moveShandria() {
	int shanLen1 = _shanLen;
	if (_flags->getFlagValue(Flags::SHANDOG)) {
		_secondHero->freeHeroAnim();
		_secondHero->freeOldMove();
		byte *shanCoords = _mainHero->_currCoords + shanLen1 * 4 - 4;
		int shanX = READ_LE_UINT16(shanCoords - 4);
		int shanY = READ_LE_UINT16(shanCoords - 2);
		int xDiff = shanX - _secondHero->_middleX;
		if (xDiff < 0) {
			xDiff *= -1;
		}
		int yDiff = shanY - _secondHero->_middleY;
		if (yDiff < 0) {
			yDiff *= -1;
		}
		shanCoords -= 4;
		if (shanCoords != _mainHero->_currCoords) {
			yDiff *= 1.5;
			int shanDis =  xDiff * xDiff + yDiff * yDiff;
			if (shanDis >= kMinDistance) {
				while (1) {
					shanCoords -= 4;
					if (shanCoords == _mainHero->_currCoords) {
						break;
					}
					int x = READ_LE_UINT16(shanCoords);
					int y = READ_LE_UINT16(shanCoords + 2);
					int pointDiffX = x - shanX;
					if (pointDiffX < 0) {
						pointDiffX *= -1;
					}
					int pointDiffY = y - shanY;
					if (pointDiffY < 0) {
						pointDiffY *= -1;
					}
					pointDiffY *= 1.5;
					int distance = pointDiffX * pointDiffX + pointDiffY * pointDiffY;
					if (distance >= kMinDistance) {
						break;
					}
				}
				int pathSizeDiff = (shanCoords - _mainHero->_currCoords) / 4;
				int destDir = *(_mainHero->_currDirTab + pathSizeDiff);
				_secondHero->_destDirection = destDir;
				int destX = READ_LE_UINT16(shanCoords);
				int destY = READ_LE_UINT16(shanCoords + 2);
				_secondHero->_coords = makePath(kSecondHero, _secondHero->_middleX, _secondHero->_middleY, destX, destY);
				if (_secondHero->_coords != nullptr) {
					_secondHero->_currCoords = _secondHero->_coords;
					int delay = shanLen1 - _shanLen;
					if (delay < 6) {
						delay = 6;
					}
					_secondHero->_moveDelay = delay / 2;
					_secondHero->_state = Hero::kHeroStateDelayMove;
					_secondHero->_dirTab = _directionTable;
					_secondHero->_currDirTab = _directionTable;
					_directionTable = nullptr;
				}
			}
		}
	}
}

byte *PrinceEngine::makePath(int heroId, int currX, int currY, int destX, int destY) {
	int realDestX = destX;
	int realDestY = destY;
	_flags->setFlagValue(Flags::MOVEDESTX, destX);
	_flags->setFlagValue(Flags::MOVEDESTY, destY);

	int x1 = currX / 2;
	int y1 = currY / 2;
	int x2 = destX / 2;
	int y2 = destY / 2;

	if ((x1 != x2) || (y1 != y2)) {
		findPoint(x1, y1);
		if (!getPixelAddr(_roomPathBitmap, _fpX, _fpY)) {
			return nullptr;
		}
		if ((x1 != _fpX) || (y1 != _fpY)) {
			x1 = _fpX;
			y1 = _fpY;
		}
		findPoint(x2, y2);
		if (!getPixelAddr(_roomPathBitmap, _fpX, _fpY)) {
			return nullptr;
		}
		if ((x2 != _fpX) || (y2 != _fpY)) {
			x2 = _fpX;
			y2 = _fpY;
			if (!_flags->getFlagValue(Flags::EXACTMOVE)) {
				realDestX = x2 * 2;
				realDestY = y2 * 2;
				_flags->setFlagValue(Flags::MOVEDESTX, realDestX);
				_flags->setFlagValue(Flags::MOVEDESTY, realDestY);
			} else {
				return nullptr;
			}
		}

		if ((x1 == x2) && (y1 == y2)) {
			if (!heroId) {
				_mainHero->freeOldMove();
				_mainHero->_state = Hero::kHeroStateTurn;
			} else if (heroId == 1) {
				_secondHero->freeOldMove();
				_secondHero->_state = Hero::kHeroStateTurn;
			}
			return nullptr;
		}

		int pathLen1 = 0;
		int pathLen2 = 0;
		int stX = x1;
		int stY = y1;
		int sizeCoords2 = 0;

		if (tracePath(x1, y1, x2, y2)) {
			allocCoords2();
			approxPath();
			sizeCoords2 = _coords2 - _coordsBuf2;
			for (int i = 0; i < sizeCoords2; i++) {
				_coordsBuf[i] = _coordsBuf2[i];
			}
			_coords = _coordsBuf + sizeCoords2;
			approxPath();
			_coordsBuf3 = _coordsBuf2;
			_coordsBuf2 = nullptr;
			_coords3 = _coords2;
			_coords2 = nullptr;
			pathLen1 = _coords3 - _coordsBuf3;
		}
		if (tracePath(x2, y2, x1, y1)) {
			allocCoords2();
			approxPath();
			sizeCoords2 = _coords2 - _coordsBuf2;
			for (int i = 0; i < sizeCoords2; i++) {
				_coordsBuf[i] = _coordsBuf2[i];
			}
			_coords = _coordsBuf + sizeCoords2;
			approxPath();
			pathLen2 = _coords2 - _coordsBuf2;
		}

		byte *chosenCoordsBuf = _coordsBuf2;
		byte *choosenCoords = _coords2;
		int choosenLength = pathLen1;
		if (pathLen1 < pathLen2) {
			chosenCoordsBuf = _coordsBuf3;
			choosenCoords = _coords3;
			choosenLength = pathLen2;
		}

		if (choosenLength) {
			if (chosenCoordsBuf != nullptr) {
				int tempXBegin = READ_LE_UINT16(chosenCoordsBuf);
				int tempYBegin = READ_LE_UINT16(chosenCoordsBuf + 2);
				if (stX != tempXBegin || stY != tempYBegin) {
					SWAP(chosenCoordsBuf, choosenCoords);
					chosenCoordsBuf -= 4;
					byte *tempCoordsBuf = _coordsBuf;
					while (1) {
						int cord = READ_LE_UINT32(chosenCoordsBuf);
						WRITE_LE_UINT32(tempCoordsBuf, cord);
						tempCoordsBuf += 4;
						if (chosenCoordsBuf == choosenCoords) {
							break;
						}
						chosenCoordsBuf -= 4;
					}
					_coords = tempCoordsBuf;
				} else {
					int sizeChoosen = choosenCoords - chosenCoordsBuf;
					for (int i = 0; i < sizeChoosen; i++) {
						_coordsBuf[i] = chosenCoordsBuf[i];
					}
					_coords = _coordsBuf + sizeChoosen;
				}
				WRITE_LE_UINT32(_coords, 0xFFFFFFFF);
				freeCoords2();
				freeCoords3();
				scanDirections();

				byte *tempCoordsBuf = _coordsBuf;
				byte *tempCoords = _coords;
				byte *newCoords;
				if (tempCoordsBuf != tempCoords) {
					int normCoordsSize = _coords - _coordsBuf + 4;
					newCoords = (byte *)malloc(normCoordsSize);
					byte *newCoordsBegin = newCoords;
					while (tempCoordsBuf != tempCoords) {
						int newValueX = READ_LE_UINT16(tempCoordsBuf);
						WRITE_LE_UINT16(newCoords, newValueX * 2);
						newCoords += 2;
						int newValueY = READ_LE_UINT16(tempCoordsBuf + 2);
						WRITE_LE_UINT16(newCoords, newValueY * 2);
						newCoords += 2;
						tempCoordsBuf += 4;
					}
					WRITE_LE_UINT16(newCoords - 4, realDestX);
					WRITE_LE_UINT16(newCoords - 2, realDestY);
					WRITE_LE_UINT32(newCoords, 0xFFFFFFFF);
					newCoords += 4;
					_shanLen = (newCoords - newCoordsBegin);
					_shanLen /= 4;
					return newCoordsBegin;
				}
			}
		}
		_coords = _coordsBuf;
		freeCoords2();
		freeCoords3();
		return nullptr;
	} else {
		if (!heroId) {
			_mainHero->freeOldMove();
			_mainHero->_state = Hero::kHeroStateTurn;
		} else if (heroId == 1) {
			_secondHero->freeOldMove();
			_secondHero->_state = Hero::kHeroStateTurn;
		}
		return nullptr;
	}
}

void PrinceEngine::allocCoords2() {
	if (_coordsBuf2 == nullptr) {
		_coordsBuf2 = (byte *)malloc(kTracePts * 4);
		_coords2 = _coordsBuf2;
	}
}

void PrinceEngine::freeCoords2() {
	if (_coordsBuf2 != nullptr) {
		free(_coordsBuf2);
		_coordsBuf2 = nullptr;
		_coords2 = nullptr;
	}
}

void PrinceEngine::freeCoords3() {
	if (_coordsBuf3 != nullptr) {
		free(_coordsBuf3);
		_coordsBuf3 = nullptr;
		_coords3 = nullptr;
	}
}

} // End of namespace Prince
