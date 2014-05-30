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
 */
#include "common/debug.h"
#include "common/random.h"

#include "prince/hero.h"
#include "prince/hero_set.h"
#include "prince/animation.h"
#include "prince/resource.h"
#include "prince/prince.h"
#include "prince/graphics.h"

namespace Prince {

Hero::Hero(PrinceEngine *vm, GraphicsMan *graph) : _vm(vm), _graph(graph)
	, _number(0), _visible(false), _state(MOVE), _middleX(0), _middleY(0)
	, _boreNum(1), _currHeight(0), _moveDelay(0), _shadMinus(0), _moveSetType(0)
	, _lastDirection(DOWN), _destDirection(DOWN), _talkTime(0), _boredomTime(0), _phase(0)
	, _specAnim(0), _drawX(0), _drawY(0), _zoomFactor(0), _scaleValue(0)
	, _shadZoomFactor(0), _shadScaleValue(0), _shadLineLen(0), _shadDrawX(0), _shadDrawY(0)
	, _frameXSize(0), _frameYSize(0), _scaledFrameXSize(0), _scaledFrameYSize(0)
{
	_zoomBitmap = (byte *)malloc(kZoomBitmapLen);
	_shadowBitmap = (byte *)malloc(2 * kShadowBitmapSize);
	_shadowLine = new byte[kShadowLineArraySize];
}

Hero::~Hero() {
	free(_zoomBitmap);
	free(_shadowBitmap);
	delete[] _shadowLine;
}

bool Hero::loadAnimSet(uint32 animSetNr) {
	if (animSetNr > sizeof(heroSetTable)) {
		return false;
	}

	_shadMinus = heroSetBack[animSetNr];

	for (uint32 i = 0; i < _moveSet.size(); i++) {
		delete _moveSet[i];
	}

	const HeroSetAnimNames &animSet = *heroSetTable[animSetNr];

	_moveSet.resize(kMoveSetSize);
	for (uint32 i = 0; i < kMoveSetSize; ++i) {
		debug("Anim set item %d %s", i, animSet[i]);
		Animation *anim = NULL;
		if (animSet[i] != NULL) {
			anim = new Animation();
			Resource::loadResource(anim, animSet[i], true);
		}
		_moveSet[i] = anim;
	}

	return true;
}

Graphics::Surface *Hero::getSurface() {
	if (_moveSet[_moveSetType]) {
		int16 phaseFrameIndex = _moveSet[_moveSetType]->getPhaseFrameIndex(_phase);
		Graphics::Surface *heroFrame = _moveSet[_moveSetType]->getFrame(phaseFrameIndex);
		return heroFrame;
	}
	return NULL;
}

//TEMP
void Hero::getState() {
	switch (_state) {
	case STAY:
		debug("STAY");
		break;
	case TURN:
		debug("TURN");
		break;
	case MOVE:
		debug("MOVE");
		break;
	case BORE:
		debug("BORE");
		break;
	case SPEC:
		debug("SPEC");
		break;
	case TALK:
		debug("TALK");
		break;
	case MVAN:
		debug("MVAN");
		break;
	case TRAN:
		debug("TRAN");
		break;
	case RUN:
		debug("RUN");
		break;
	case DMOVE:
		debug("DMOVE");
		break;
	}
}

int Hero::getScaledValue(int size) {
	int newSize = 0;
	int16 initScaleValue = _scaleValue;
	if (_scaleValue != 10000) {
		for(int i = 0; i < size; i++) {
			initScaleValue -= 100;
			if(initScaleValue >= 0) {
				newSize++;
			} else {
				initScaleValue += _scaleValue;
			}
		}
		return newSize;
	} else {
		return size;
	}
}

Graphics::Surface *Hero::zoomSprite(Graphics::Surface *heroFrame) {
	Graphics::Surface *zoomedFrame = new Graphics::Surface();
	zoomedFrame->create(_scaledFrameXSize, _scaledFrameYSize, Graphics::PixelFormat::createFormatCLUT8());
	
	int sprZoomX;
	int sprZoomY = _scaleValue;
	uint xSource = 0;
	uint ySource = 0;
	uint xDest = 0;
	uint yDest = 0;

	for (int i = 0; i < _scaledFrameYSize; i++) {
		// linear_loop:
		while(1) {
			sprZoomY -= 100;
			if (sprZoomY >= 0 || _scaleValue == 10000) {
				// all_r_y
				sprZoomX = _scaleValue;
				break; // to loop_lin
			} else {
				sprZoomY += _scaleValue;
				xSource = 0;
				ySource++;
			}
		}
		// loop_lin:
		for (int j = 0; j < _scaledFrameXSize; j++) {
			sprZoomX -= 100;
			if (sprZoomX >= 0) {
				// its_all_r
				memcpy(zoomedFrame->getBasePtr(xDest, yDest), heroFrame->getBasePtr(xSource, ySource), 1);
				xDest++;
			} else {
				sprZoomX += _scaleValue;
				j--;
			}
			xSource++;
		}
		xDest = 0;
		yDest++;
		xSource = 0;
		ySource++;
	}
	return zoomedFrame;
}

void Hero::countDrawPosition() {
	int16 tempMiddleX;
	int16 tempMiddleY;
	int16 baseX = _moveSet[_moveSetType]->getBaseX();
	int16 baseY = _moveSet[_moveSetType]->getBaseY();
	// any chance?
	if (baseX == 320) {
		tempMiddleY = _middleY - (baseY - 240);
	} else {
		tempMiddleY = _middleY;
	}
	_frameXSize = _moveSet[_moveSetType]->getFrameWidth(_phase);
	_frameYSize = _moveSet[_moveSetType]->getFrameHeight(_phase);
	_scaledFrameXSize = getScaledValue(_frameXSize);
	_scaledFrameYSize = getScaledValue(_frameYSize);
	
	//TODO
	int tempHeroHeight = _scaledFrameYSize; // not used? global?
	int width = _frameXSize / 2;
	tempMiddleX = _middleX - width; //eax
	int z = _middleY; //ebp
	int y = _middleY - _scaledFrameYSize; //ecx
	_vm->checkMasks(tempMiddleX, y, _scaledFrameXSize, _scaledFrameYSize, z);

	if (_zoomFactor != 0) {
		//notfullSize
		_drawX = _middleX - _scaledFrameXSize / 2;
		_drawY = tempMiddleY + 1 - _scaledFrameYSize;
	} else {
		//fullSize
		_drawX = _middleX - _frameXSize / 2;
		_drawY = tempMiddleY + 1 - _frameYSize;
	}
}

void Hero::plotPoint(int x, int y) {
	WRITE_UINT16(&_shadowLine[_shadLineLen * 4], x);
	WRITE_UINT16(&_shadowLine[_shadLineLen * 4 + 2], y);
}

static void plot(int x, int y, int color, void *data) {
	Hero *shadowLine = (Hero *)data;
	shadowLine->plotPoint(x, y);
	shadowLine->_shadLineLen++;
}

void Hero::showHeroShadow(Graphics::Surface *heroFrame) {
	Graphics::Surface *makeShadow = new Graphics::Surface();
	makeShadow->create(_frameXSize, _frameYSize, Graphics::PixelFormat::createFormatCLUT8());

	for (int y = 0; y < _frameYSize; y++) {
		byte *src = (byte *)heroFrame->getBasePtr(0, y);
		byte *dst = (byte *)makeShadow->getBasePtr(0, y);

		for (int x = 0; x < _frameXSize; x++, dst++, src++) {
			if (*src != 0xFF) {
				*dst = _graph->kShadowColor;
			} else {
				*dst = *src;
			}
		}
	}

	int destX = _middleX - _scaledFrameXSize / 2;
	int destY = _middleY - _shadMinus - 1;

	if (destY > 1 && destY < kMaxPicHeight) {
		int shadDirection;
		if (_lightY > destY) {
			shadDirection = 1;
		} else {
			shadDirection = 0;
		}

		_shadLineLen = 0;
		Graphics::drawLine(_lightX, _lightY, destX, destY, 0, &plot, this);

		byte *sprShadow = (byte *)_graph->_shadowTable70;

		_shadDrawX = destX - _vm->_picWindowX;
		_shadDrawY = destY - _vm->_picWindowY;

		int shadPosX = _shadDrawX;
		int shadPosY = _shadDrawY;
		int shadBitAddr = destY * kMaxPicWidth / 8 + destX / 8;
		int shadBitMask = 128 >> (destX % 8);

		int shadZoomY2 = _shadScaleValue;
		int shadZoomY = _scaleValue;

		int diffX = 0;
		int diffY = 0;

		int shadowHeroX = 0;
		int shadowHeroY = _frameYSize - 1;

		int shadLastY = 0;

		byte *shadowHero = (byte *)makeShadow->getBasePtr(shadowHeroX, shadowHeroY); // first pixel from last row of shadow hero
		byte *background = (byte *)_graph->_frontScreen->getBasePtr(_shadDrawX, _shadDrawY); // pixel of background where shadow sprite starts

		// banked2
		byte *shadowLineStart = _shadowLine + 8;

		int shadWallDown = 0;
		int shadWallBitAddr = 0;
		int shadWallBitMask = 0;
		byte *shadWallDestAddr = 0;
		int shadWallPosY = 0;
		int shadWallSkipX = 0;
		int shadWallModulo = 0;

		// linear_loop
		for (int i = 0; i < _frameYSize; i++) {
			int shadSkipX = 0;
			int ct_loop = 0;
			int sprModulo = 0;

			int j;
			//retry_line:
			for (j = _frameYSize - i; j > 0; j--) {
				shadZoomY -= 100;
				if (shadZoomY < 0 && _scaleValue != 10000) {
					shadZoomY += _scaleValue;
					shadowHeroY--;
					if (shadowHeroY < 0) {
						break;
					}
					shadowHero = (byte *)makeShadow->getBasePtr(shadowHeroX, shadowHeroY);
				} else {
					break;
				}
			}
			if (j == 0) {
				break;
			}
			if (shadowHeroY < 0) {
				break;
			}
			//line_y_ok
			if (shadLastY != shadPosY && shadPosY >= 0 && shadPosY < 480 && shadPosX < 640) {
				shadLastY = shadPosY;
				if (shadPosX < 0) {
					shadSkipX = -1 * shadPosX;
					background += shadSkipX;
					if (_frameXSize > shadSkipX) {
						shadowHero += shadSkipX;
						shadBitAddr += shadSkipX / 8;
						if ((shadSkipX % 8) != 0) {
							//loop_rotate:
							for (int a = 0; a < (shadSkipX % 8); a++) {
								if (shadBitMask == 1) {
									shadBitMask = 128;
									shadBitAddr++;
								} else {
									shadBitMask >>= 1;
								}
							}
						}
					} else {
						//skip_line
						//test it
					}
				} else {
					//x1_ok
					if (shadPosX + _frameXSize > 640) {
						ct_loop = 640 - shadPosX; // test it
						sprModulo = shadPosX + _frameXSize - 640;
					} else {
						//draw_line
						ct_loop = _frameXSize;
					}
				}
				//draw_line1
				//retry_line2
				int k;
				for (k = j; k > 0; k--) {
					shadZoomY2 -= 100;
					if (shadZoomY2 < 0 && _shadScaleValue != 10000) {
						shadZoomY2 += _shadScaleValue;
						shadowHeroY--;
						if (shadowHeroY < 0) {
							break;
						}
						shadowHero = (byte *)makeShadow->getBasePtr(shadowHeroX, shadowHeroY);
					} else {
						break;
					}
				}
				if (shadowHeroY < 0) {
					break;
				}
				if (k == 0) {
					break;
				}
				//line_y_ok_2:
				//copy_trans
				bool shadWDFlag = false;
				int shadZoomX = _scaleValue;
				int backgroundDiff = 0;
				int shadBitMaskCopyTrans = shadBitMask;
				int shadBitAddrCopyTrans = shadBitAddr;
				//ct_loop:
				for (int l = 0; l < ct_loop; l++) {
					shadZoomX -= 100;
					if (shadZoomX < 0 && _scaleValue != 10000) {
						shadZoomX += _scaleValue;
					} else {
						if (*shadowHero == _graph->kShadowColor) {
							if ((shadBitMaskCopyTrans & _shadowBitmap[shadBitAddrCopyTrans]) != 0) {
								if (shadWallDown == 0) {
									if ((shadBitMaskCopyTrans & _shadowBitmap[shadBitAddrCopyTrans + kShadowBitmapSize]) != 0) {
										shadWDFlag = true;
										//shadow
										*background = *(sprShadow + *background);
									}
								}
							} else {
								//shadow
								*background = *(sprShadow + *background);
							}
						}
						//ct_next
						if (shadBitMaskCopyTrans == 1) {
							shadBitMaskCopyTrans = 128;
							shadBitAddrCopyTrans++;
						} else {
							shadBitMaskCopyTrans >>= 1;
						}
						//okok
						backgroundDiff++;
						background = (byte *)_graph->_frontScreen->getBasePtr(_shadDrawX + diffX + backgroundDiff, _shadDrawY + diffY);
					}
					shadowHeroX++;
					shadowHero = (byte *)makeShadow->getBasePtr(shadowHeroX, shadowHeroY);
				}
				//byebyebye
				if (!shadWallDown && shadWDFlag) {
					shadWallDown = shadPosX;
					shadWallBitAddr = shadBitAddr;
					shadWallDestAddr = (byte *)_graph->_frontScreen->getBasePtr(_shadDrawX + diffX, _shadDrawY + diffY);
					shadWallBitMask = shadBitMask;
					shadWallPosY = shadPosY;
					shadWallSkipX = shadSkipX;
					shadWallModulo = sprModulo;
				}
				//byebye
				if (shadDirection != 0 && shadWallDown != 0) {
					int shadBitMaskWallCopyTrans = shadWallBitMask;
					int shadBitAddrWallCopyTrans = shadWallBitAddr;
					background = shadWallDestAddr;
					shadowHero = (byte *)makeShadow->getBasePtr(shadWallSkipX, shadowHeroY);

					if (ct_loop > shadWallSkipX && ct_loop - shadWallSkipX > shadWallModulo) {
						//WALL_copy_trans
						shadWDFlag = false;
						int shadZoomXWall = _scaleValue;
						int backgroundDiffWall = 0;
						int shadowHeroXWall = 0;
						//ct_loop:
						for (int m = 0; m < ct_loop; m++) {
							shadZoomXWall -= 100;
							if (shadZoomXWall < 0 && _scaleValue != 10000) {
								shadZoomXWall += _scaleValue;
							} else {
								//point_ok:
								if (*shadowHero == _graph->kShadowColor) {
									if ((shadBitMaskWallCopyTrans & _shadowBitmap[shadBitAddrWallCopyTrans + kShadowBitmapSize]) != 0) {
										*background = *(sprShadow + *background);
									}
								}
								//ct_next
								if (shadBitMaskWallCopyTrans == 1) {
									shadBitMaskWallCopyTrans = 128;
									shadBitAddrWallCopyTrans++;
								} else {
									shadBitMaskWallCopyTrans >>= 1;
								}
								//okok
								backgroundDiffWall++;
								background = shadWallDestAddr + backgroundDiffWall;
							}
							shadowHeroXWall++;
							shadowHero = (byte *)makeShadow->getBasePtr(shadWallSkipX + shadowHeroXWall, shadowHeroY);
						}
					}
					//krap2
					shadWallDestAddr -= kScreenWidth;
					shadWallBitAddr -= kMaxPicWidth / 8;
					shadWallPosY--;
				}
			}
			//skip_line
			//next_line
			if (*(shadowLineStart + 2) < *(shadowLineStart - 2)) {
				//minus_y
				shadBitAddr -= kMaxPicWidth / 8;
				shadPosY--;
				diffY--;
			} else if (*(shadowLineStart + 2) > *(shadowLineStart - 2)) {
				shadBitAddr += kMaxPicWidth / 8;
				shadPosY++;
				diffY++;
			}
			//no_change_y
			if (*shadowLineStart < *(shadowLineStart - 4)) {
				//minus_x
				shadPosX--;
				//rol
				if (shadBitMask == 128) {
					shadBitMask = 1;
					shadBitAddr--;
				} else {
					shadBitMask <<= 1;
				}
				diffX--;
			} else if (*shadowLineStart > *(shadowLineStart - 4)) {
				shadPosX++;
				//ror
				if (shadBitMask == 1) {
					shadBitMask = 128;
					shadBitAddr++;
				} else {
					shadBitMask >>= 1;
				}
				diffX++;
			}
			//no_change_x
			shadowLineStart += 4;
			shadowHeroY--;
			if (shadowHeroY < 0) {
				break;
			}
			shadowHeroX = 0;
			background = (byte *)_graph->_frontScreen->getBasePtr(_shadDrawX + diffX, _shadDrawY + diffY);
			shadowHero = (byte *)makeShadow->getBasePtr(shadowHeroX, shadowHeroY);
		}
		//koniec_bajki - end_of_a_story
	}
	makeShadow->free();
	delete makeShadow;
}

void Hero::showHeroAnimFrame() {
	if (_phase < _moveSet[_moveSetType]->getFrameCount() - 1) {
		_phase++;
	} else {
		_phase = 0;
	}
	countDrawPosition();
}

void Hero::setScale(int8 zoomBitmapValue) {
	if (zoomBitmapValue == 0) {
		_zoomFactor = 0;
		_scaleValue = 10000;
	} else {
		_zoomFactor = zoomBitmapValue;
		_scaleValue = 10000 / _zoomFactor;
	}
}

void Hero::selectZoom() {
	int8 zoomBitmapValue = *(_zoomBitmap + _middleY / 4 * kZoomBitmapWidth + _middleX / 4);
	setScale(zoomBitmapValue);
}

void Hero::setShadowScale(int32 shadowScale) {
	shadowScale = 100 - shadowScale;
	if (shadowScale == 0) {
		_shadZoomFactor = 0;
		_shadScaleValue = 10000;
	} else {
		_shadZoomFactor = shadowScale;
		_shadScaleValue = 10000 / _shadZoomFactor;
	}
}

void Hero::specialAnim() {
}

void Hero::rotateHero() {
	switch (_lastDirection) {
	case LEFT:
		switch (_destDirection) {
		case RIGHT:
			_moveSetType = Move_MLR;
			break;
		case UP:
			_moveSetType = Move_MLU;
			break;
		case DOWN:
			_moveSetType = Move_MLD;
			break;
		}
		break;
	case RIGHT:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MRL;
			break;
		case UP:
			_moveSetType = Move_MRU;
			break;
		case DOWN:
			_moveSetType = Move_MRD;
			break;
		}
		break;
	case UP:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MUL;
			break;
		case RIGHT:
			_moveSetType = Move_MUR;
			break;
		case DOWN:
			_moveSetType = Move_MUD;
			break;
		}
		break;
	case DOWN:
		switch (_destDirection) {
		case LEFT:
			_moveSetType = Move_MDL;
			break;
		case RIGHT:
			_moveSetType = Move_MDR;
			break;
		case UP:
			_moveSetType = Move_MDU;
			break;
		}
		break;
	}
}

void Hero::showHero() {
	if (_visible) {
		// Is he talking?
		if (_talkTime == 0) { //?
			// Scale of hero
			selectZoom();
			switch (_state) {
			case STAY:
				//if(OptionsFlag == false) {
				//if(OpcodePC == null) {
				_boredomTime++;
				if (_boredomTime == 200) { // 140 for second hero
					_boredomTime = 0;
					_state = BORE;
				}
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_SL;
					break;
				case RIGHT:
					_moveSetType = Move_SR;
					break;
				case UP:
					_moveSetType = Move_SU;
					break;
				case DOWN:
					_moveSetType = Move_SD;
					break;
				}
				break;
			case TURN:
				/*
				if(_lastDirection == _destDirection) {
					_state = STAY;
				} else {
					_frame = 0;
					rotateHero();
					_lastDirection = _destDirection;
				}
				*/
				break;
			case MOVE:
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_ML;
					break;
				case RIGHT:
					_moveSetType = Move_MR;
					break;
				case UP:
					_moveSetType = Move_MU;
					break;
				case DOWN:
					_moveSetType = Move_MD;
					break;
				}
				break;
			case BORE:
				//if (_direction == UP) {
				switch (_boreNum) {
				case 0:
					_moveSetType = Move_BORED1;
					break;
				case 1:
					_moveSetType = Move_BORED2;
					break;
				}
				if (_phase == _moveSet[_moveSetType]->getFrameCount() - 1) {
					_boreNum = _vm->_randomSource.getRandomNumber(1); // rand one of two 'bored' animation
					_lastDirection = DOWN;
					_state = STAY;
				}
				break;
			case SPEC:
				//specialAnim();
				break;
			case TALK:
				switch (_lastDirection) {
				case LEFT:
					_moveSetType = Move_TL;
					break;
				case RIGHT:
					_moveSetType = Move_TR;
					break;
				case UP:
					_moveSetType = Move_TU;
					break;
				case DOWN:
					_moveSetType = Move_TD;
					break;
				}
				break;
			case TRAN:
				break;
			case RUN:
				break;
			case DMOVE:
				break;
			}
		} else {
			_talkTime--; // o ile?
		}
		showHeroAnimFrame();
	} else {
		// no hero visible
		return;
	}
}

void Hero::scrollHero() {
	//FLAGI+SCROLLTYPE ??
	//int scrollType = 0;
	int position = _middleX;

	/*
	switch (scrollType) {
	case 0:
		position = _middleX;
		break;
	case 1:
		break;
	case 2:
		break;
	}
	*/

	int locationWidth = _vm->_sceneWidth;
	int difference = locationWidth - _vm->kNormalWidth / 2;

	int destValue = 0;
	if (position > _vm->kNormalWidth / 2) {
		destValue = difference - _vm->kNormalWidth / 2;
	}
	if (position < difference) {
		destValue = position - _vm->kNormalWidth / 2;
	}
	if(destValue < 0) {
		destValue = 0;
	}
	_vm->_picWindowX = destValue;
	_drawX -= destValue;
}

}

/* vim: set tabstop=4 noexpandtab: */
