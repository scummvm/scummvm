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

#include "graphics/palette.h"

#include "prince/prince.h"

#include "prince/animation.h"
#include "prince/graphics.h"
#include "prince/hero.h"
#include "prince/script.h"

namespace Prince {

bool PrinceEngine::spriteCheck(int sprWidth, int sprHeight, int destX, int destY) {
	destX -= _picWindowX;
	destY -= _picWindowY;

	 // if x1 is on visible part of screen
	if (destX < 0) {
		if (destX + sprWidth < 1) {
			//x2 is negative - out of window
			return false;
		}
	}
	 // if x1 is outside of screen on right side
	if (destX >= kNormalWidth) {
		return false;
	}

	if (destY < 0) {
		if (destY + sprHeight < 1) {
			//y2 is negative - out of window
			return false;
		}
	}
	if (destY >= kNormalHeight) {
		return false;
	}

	return true;
}

// CheckNak
void PrinceEngine::checkMasks(int x1, int y1, int sprWidth, int sprHeight, int z) {
	int x2 = x1 + sprWidth - 1;
	int y2 = y1 + sprHeight - 1;
	if (x1 < 0) {
		x1 = 0;
	}
	for (uint i = 0; i < _maskList.size(); i++) {
		if (!_maskList[i]._state && !_maskList[i]._flags) {
			if (_maskList[i]._z > z) {
				if (_maskList[i]._x1 <= x2 && _maskList[i]._x2 >= x1) {
					if (_maskList[i]._y1 <= y2 && _maskList[i]._y2 >= y1) {
						_maskList[i]._state = 1;
					}
				}
			}
		}
	}
}

// ClsNak
void PrinceEngine::clsMasks() {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state) {
			_maskList[i]._state = 0;
		}
	}
}

// InsertNakladki
void PrinceEngine::insertMasks(Graphics::Surface *originalRoomSurface) {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state) {
			if (_maskList[i]._data != nullptr) {
				showMask(i, originalRoomSurface);
			} else {
				error("insertMasks() - Wrong mask data- nr %d", i);
			}
		}
	}
}

// ShowNak
void PrinceEngine::showMask(int maskNr, Graphics::Surface *originalRoomSurface) {
	if (!_maskList[maskNr]._flags) {
		if (spriteCheck(_maskList[maskNr]._width, _maskList[maskNr]._height, _maskList[maskNr]._x1, _maskList[maskNr]._y1)) {
			int destX = _maskList[maskNr]._x1 - _picWindowX;
			int destY = _maskList[maskNr]._y1 - _picWindowY;
			DrawNode newDrawNode;
			newDrawNode.posX = destX;
			newDrawNode.posY = destY;
			newDrawNode.posZ = _maskList[maskNr]._z;
			newDrawNode.width = _maskList[maskNr]._width;
			newDrawNode.height = _maskList[maskNr]._height;
			newDrawNode.s = nullptr;
			newDrawNode.originalRoomSurface = originalRoomSurface;
			newDrawNode.data = _maskList[maskNr].getMask();
			newDrawNode.drawFunction = &_graph->drawMaskDrawNode;
			_drawNodeList.push_back(newDrawNode);
		}
	}
}

void PrinceEngine::showSprite(Graphics::Surface *spriteSurface, int destX, int destY, int destZ) {
	if (spriteCheck(spriteSurface->w, spriteSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = spriteSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = _transTable;
		newDrawNode.drawFunction = &_graph->drawTransparentWithTransDrawNode;
		_drawNodeList.push_back(newDrawNode);
	}
}

void PrinceEngine::showSpriteShadow(Graphics::Surface *shadowSurface, int destX, int destY, int destZ) {
	if (spriteCheck(shadowSurface->w, shadowSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = shadowSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = _graph->_shadowTable70;
		newDrawNode.drawFunction = &_graph->drawAsShadowDrawNode;
		_drawNodeList.push_back(newDrawNode);
	}
}

void PrinceEngine::showAnim(Anim &anim) {
	//ShowFrameCode
	//ShowAnimFrame
	int phase = anim._showFrame;
	int phaseFrameIndex = anim._animData->getPhaseFrameIndex(phase);
	int x = anim._x + anim._animData->getPhaseOffsetX(phase);
	int y = anim._y + anim._animData->getPhaseOffsetY(phase);
	int animFlag = anim._flags;
	int checkMaskFlag = (animFlag & 1);
	int maxFrontFlag = (animFlag & 2);
	int specialZFlag = anim._nextAnim;
	int z = anim._nextAnim;
	Graphics::Surface *animSurface = anim._animData->getFrame(phaseFrameIndex);
	int frameWidth = animSurface->w;
	int frameHeight = animSurface->h;
	int shadowZ = 0;

	if (checkMaskFlag) {
		if (!anim._nextAnim) {
			z = y + frameHeight - 1;
		}
		checkMasks(x, y, frameWidth, frameHeight, z);
	}

	if (specialZFlag) {
		z = specialZFlag;
	} else if (maxFrontFlag) {
		z = kMaxPicHeight + 1;
	} else {
		z = y + frameHeight - 1;
	}
	shadowZ = z;

	anim._currX = x;
	anim._currY = y;
	anim._currW = frameWidth;
	anim._currH = frameHeight;
	showSprite(animSurface, x, y, z);

	// make_special_shadow
	if ((anim._flags & 0x80)) {
		DrawNode newDrawNode;
		newDrawNode.posX = x;
		newDrawNode.posY = y + animSurface->h - anim._shadowBack;
		newDrawNode.posZ = Hero::kHeroShadowZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.scaleValue = _scaleValue;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = this;
		newDrawNode.drawFunction = &Hero::showHeroShadow;
		newDrawNode.s = animSurface;
		_drawNodeList.push_back(newDrawNode);
	}

	//ShowFrameCodeShadow
	//ShowAnimFrameShadow
	if (anim._shadowData != nullptr) {
		int shadowPhaseFrameIndex = anim._shadowData->getPhaseFrameIndex(phase);
		int shadowX = anim._shadowData->getBaseX() + anim._shadowData->getPhaseOffsetX(phase);
		int shadowY = anim._shadowData->getBaseY() + anim._shadowData->getPhaseOffsetY(phase);
		Graphics::Surface *shadowSurface = anim._shadowData->getFrame(shadowPhaseFrameIndex);
		int shadowFrameWidth = shadowSurface->w;
		int shadowFrameHeight = shadowSurface->h;

		if (checkMaskFlag) {
			checkMasks(shadowX, shadowY, shadowFrameWidth, shadowFrameHeight, shadowY + shadowFrameWidth - 1);
		}

		if (!shadowZ) {
			if (maxFrontFlag) {
				shadowZ = kMaxPicHeight + 1;
			} else {
				shadowZ = shadowY + shadowFrameWidth - 1;
			}
		}
		showSpriteShadow(shadowSurface, shadowX, shadowY, shadowZ);
	}
}

void PrinceEngine::showNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		Anim &anim = _normAnimList[i];
		if (anim._animData != nullptr) {
			int phaseCount = anim._animData->getPhaseCount();
			if (!anim._state) {
				if (anim._frame == anim._lastFrame - 1) {
					if (anim._loopType) {
						if (anim._loopType == 1) {
							anim._frame = anim._loopFrame;
						} else {
							continue;
						}
					}
				} else {
					anim._frame++;
				}
				anim._showFrame = anim._frame;
				if (anim._showFrame >= phaseCount) {
					anim._showFrame = phaseCount - 1;
				}
				showAnim(anim);
			}
		}
	}
}

void PrinceEngine::setBackAnim(Anim &backAnim) {
	int start = backAnim._basaData._start;
	if (start != -1) {
		backAnim._frame = start;
		backAnim._showFrame = start;
		backAnim._loopFrame = start;
	}
	int end = backAnim._basaData._end;
	if (end != -1) {
		backAnim._lastFrame = end;
	}
	backAnim._state = 0;
}

void PrinceEngine::showBackAnims() {
	for (int i = 0; i < kMaxBackAnims; i++) {
		BAS &seq = _backAnimList[i]._seq;
		int activeSubAnim = seq._currRelative;
		if (!_backAnimList[i].backAnims.empty()) {
			if (_backAnimList[i].backAnims[activeSubAnim]._animData != nullptr) {
				if (!_backAnimList[i].backAnims[activeSubAnim]._state) {
					seq._counter++;
					if (seq._type == 2) {
						if (!seq._currRelative) {
							if (seq._counter >= seq._data) {
								if (seq._anims > 2) {
									seq._currRelative = _randomSource.getRandomNumber(seq._anims - 2) + 1;
									activeSubAnim = seq._currRelative;
									seq._current = _backAnimList[i].backAnims[activeSubAnim]._basaData._num;
								}
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
						}
					}

					if (seq._type == 3) {
						if (!seq._currRelative) {
							if (seq._counter < seq._data2) {
								continue;
							} else {
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
							}
						}
					}

					if (_backAnimList[i].backAnims[activeSubAnim]._frame == _backAnimList[i].backAnims[activeSubAnim]._lastFrame - 1) {
						_backAnimList[i].backAnims[activeSubAnim]._frame = _backAnimList[i].backAnims[activeSubAnim]._loopFrame;
						switch (seq._type) {
						case 1:
							if (seq._anims > 1) {
								int rnd;
								do {
									rnd = _randomSource.getRandomNumber(seq._anims - 1);
								} while (rnd == seq._currRelative);
								seq._currRelative = rnd;
								seq._current = _backAnimList[i].backAnims[rnd]._basaData._num;
								activeSubAnim = rnd;
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
							break;
						case 2:
							if (seq._currRelative) {
								seq._currRelative = 0;
								seq._current = _backAnimList[i].backAnims[0]._basaData._num;
								activeSubAnim = 0;
								setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
								seq._counter = 0;
							}
							break;
						case 3:
							seq._currRelative = 0;
							seq._current = _backAnimList[i].backAnims[0]._basaData._num;
							seq._counter = 0;
							seq._data2 = _randomSource.getRandomNumber(seq._data - 1);
							continue; // for bug in original game
							break;
						default:
							break;
						}
					} else {
						_backAnimList[i].backAnims[activeSubAnim]._frame++;
					}
					_backAnimList[i].backAnims[activeSubAnim]._showFrame = _backAnimList[i].backAnims[activeSubAnim]._frame;
					showAnim(_backAnimList[i].backAnims[activeSubAnim]);
				}
			}
		}
	}
}

void PrinceEngine::removeSingleBackAnim(int slot) {
	if (!_backAnimList[slot].backAnims.empty()) {
		for (uint j = 0; j < _backAnimList[slot].backAnims.size(); j++) {
			if (_backAnimList[slot].backAnims[j]._animData != nullptr) {
				delete _backAnimList[slot].backAnims[j]._animData;
				_backAnimList[slot].backAnims[j]._animData = nullptr;
			}
			if (_backAnimList[slot].backAnims[j]._shadowData != nullptr) {
				delete _backAnimList[slot].backAnims[j]._shadowData;
				_backAnimList[slot].backAnims[j]._shadowData = nullptr;
			}
		}
		_backAnimList[slot].backAnims.clear();
		_backAnimList[slot]._seq._currRelative = 0;
	}
}

void PrinceEngine::clearBackAnimList() {
	for (int i = 0; i < kMaxBackAnims; i++) {
		removeSingleBackAnim(i);
	}
}

void PrinceEngine::grabMap() {
	_graph->_frontScreen->copyFrom(*_roomBmp->getSurface());
	showObjects();
	runDrawNodes();
	_graph->_mapScreen->copyFrom(*_graph->_frontScreen);
}

void PrinceEngine::initZoomIn(int slot) {
	freeZoomObject(slot);
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *zoomSource = object->getSurface();
		if (zoomSource != nullptr) {
			object->_flags |= 0x8000;
			object->_zoomSurface = new Graphics::Surface();
			object->_zoomSurface->create(zoomSource->w, zoomSource->h, Graphics::PixelFormat::createFormatCLUT8());
			object->_zoomSurface->fillRect(Common::Rect(zoomSource->w, zoomSource->h), 0xFF);
			object->_zoomTime = 20;
		}
	}
}

void PrinceEngine::initZoomOut(int slot) {
	freeZoomObject(slot);
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *zoomSource = object->getSurface();
		if (zoomSource != nullptr) {
			object->_flags |= 0x4000;
			object->_zoomSurface = new Graphics::Surface();
			object->_zoomSurface->copyFrom(*zoomSource);
			object->_zoomTime = 10;
		}
	}
}

void PrinceEngine::doZoomIn(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *orgSurface = object->getSurface();
		if (orgSurface != nullptr) {
			byte *src1 = (byte *)orgSurface->getBasePtr(0, 0);
			byte *dst1 = (byte *)object->_zoomSurface->getBasePtr(0, 0);
			int x = 0;
			int surfaceHeight = orgSurface->h;
			for (int y = 0; y < surfaceHeight; y++) {
				byte *src2 = src1;
				byte *dst2 = dst1;
				int w = orgSurface->w - x;
				src2 += x;
				dst2 += x;
				while (w > 0) {
					int randVal = _randomSource.getRandomNumber(zoomInStep - 1);
					if (randVal < w) {
						*(dst2 + randVal) = *(src2 + randVal);
						src2 += zoomInStep;
						dst2 += zoomInStep;
					} else if (y + 1 != surfaceHeight) {
						*(dst1 + orgSurface->pitch + randVal - w) = *(src1 + orgSurface->pitch + randVal - w);
					}
					w -= zoomInStep;
				}
				x = -1 * w;
				src1 += orgSurface->pitch;
				dst1 += orgSurface->pitch;
			}
		}
	}
}

void PrinceEngine::doZoomOut(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		Graphics::Surface *orgSurface = object->getSurface();
		if (orgSurface != nullptr) {
			byte *dst1 = (byte *)object->_zoomSurface->getBasePtr(0, 0);
			int x = 0;
			int surfaceHeight = orgSurface->h;
			for (int y = 0; y < surfaceHeight; y++) {
				byte *dst2 = dst1;
				int w = orgSurface->w - x;
				dst2 += x;
				while (w > 0) {
					int randVal = _randomSource.getRandomNumber(zoomInStep - 1);
					if (randVal < w) {
						*(dst2 + randVal) = 255;
						dst2 += zoomInStep;
					} else if (y + 1 != surfaceHeight) {
						*(dst1 + orgSurface->pitch + randVal - w) = 255;
					}
					w -= zoomInStep;
				}
				x = -1 * w;
				dst1 += orgSurface->pitch;
			}
		}
	}
}

void PrinceEngine::freeZoomObject(int slot) {
	Object *object = _objList[slot];
	if (object != nullptr) {
		if (object->_zoomSurface != nullptr) {
			object->_zoomSurface->free();
			delete object->_zoomSurface;
			object->_zoomSurface = nullptr;
		}
	}
}

void PrinceEngine::showObjects() {
	for (int i = 0; i < kMaxObjects; i++) {
		int nr = _objSlot[i];
		if (nr != 0xFF) {
			Graphics::Surface *objSurface = nullptr;
			if ((_objList[nr]->_flags & 0x8000)) {
				_objList[nr]->_zoomTime--;
				if (!_objList[nr]->_zoomTime) {
					freeZoomObject(nr);
					_objList[nr]->_flags &= 0x7FFF;
					objSurface = _objList[nr]->getSurface();
				} else {
					doZoomIn(nr);
					objSurface = _objList[nr]->_zoomSurface;
				}
			} else if ((_objList[nr]->_flags & 0x4000)) {
				_objList[nr]->_zoomTime--;
				if (!_objList[nr]->_zoomTime) {
					freeZoomObject(nr);
					_objList[nr]->_flags &= 0xBFFF;
					objSurface = _objList[nr]->getSurface();
				} else {
					doZoomOut(nr);
					objSurface = _objList[nr]->_zoomSurface;
				}
			} else {
				objSurface = _objList[nr]->getSurface();
			}

			if (objSurface != nullptr) {
				if (spriteCheck(objSurface->w, objSurface->h, _objList[nr]->_x, _objList[nr]->_y)) {
					int destX = _objList[nr]->_x - _picWindowX;
					int destY = _objList[nr]->_y - _picWindowY;
					DrawNode newDrawNode;
					newDrawNode.posX = destX;
					newDrawNode.posY = destY;
					newDrawNode.posZ = _objList[nr]->_z;
					newDrawNode.width = 0;
					newDrawNode.height = 0;
					newDrawNode.s = objSurface;
					newDrawNode.originalRoomSurface = nullptr;
					if ((_objList[nr]->_flags & 0x2000)) {
						newDrawNode.data = nullptr;
						newDrawNode.drawFunction = &_graph->drawBackSpriteDrawNode;
					} else {
						newDrawNode.data = _transTable;
						if (_flags->getFlagValue(Flags::NOANTIALIAS)) {
							newDrawNode.drawFunction = &_graph->drawTransparentDrawNode;
						} else {
							newDrawNode.drawFunction = &_graph->drawTransparentWithTransDrawNode;
						}
					}
					_drawNodeList.push_back(newDrawNode);
				}

				if ((_objList[nr]->_flags & 1)) {
					checkMasks(_objList[nr]->_x, _objList[nr]->_y, objSurface->w, objSurface->h, _objList[nr]->_z);
				}
			}
		}
	}
}

void PrinceEngine::showParallax() {
	if (!_pscrList.empty()) {
		for (uint i = 0; i < _pscrList.size(); i++) {
			Graphics::Surface *pscrSurface = _pscrList[i]->getSurface();
			if (pscrSurface != nullptr) {
				int x = _pscrList[i]->_x - (_pscrList[i]->_step * _picWindowX / 4);
				int y = _pscrList[i]->_y;
				int z = PScr::kPScrZ;
				if (spriteCheck(pscrSurface->w, pscrSurface->h, x, y)) {
					showSprite(pscrSurface, x, y, z);
				}
			}
		}
	}
}

bool PrinceEngine::compareDrawNodes(DrawNode d1, DrawNode d2) {
	if (d1.posZ < d2.posZ) {
		return true;
	}
	return false;
}

void PrinceEngine::runDrawNodes() {
	Common::sort(_drawNodeList.begin(), _drawNodeList.end(), compareDrawNodes);

	for (uint i = 0; i < _drawNodeList.size(); i++) {
		(*_drawNodeList[i].drawFunction)(_graph->_frontScreen, &_drawNodeList[i]);
	}
	_graph->change();
}

void PrinceEngine::drawScreen() {
	if (!_showInventoryFlag || _inventoryBackgroundRemember) {
		clsMasks();

		_mainHero->showHero();
		_mainHero->scrollHero();
		_mainHero->drawHero();

		_secondHero->showHero();
		_secondHero->_drawX -= _picWindowX;
		_secondHero->drawHero();

		const Graphics::Surface *roomSurface;
		if (_locationNr != 50) {
			roomSurface = _roomBmp->getSurface();
		} else {
			roomSurface = _graph->_mapScreen;
		}
		Graphics::Surface visiblePart;
		if (roomSurface) {
			visiblePart = roomSurface->getSubArea(Common::Rect(_picWindowX, 0, roomSurface->w, roomSurface->h));
			_graph->draw(_graph->_frontScreen, &visiblePart);
		}

		showBackAnims();

		showNormAnims();

		playNextFLCFrame();

		showObjects();

		if (roomSurface) {
			insertMasks(&visiblePart);
		}

		showParallax();

		runDrawNodes();

		_drawNodeList.clear();

		if (!_inventoryBackgroundRemember && !_dialogFlag) {
			if (!_optionsFlag) {
				_selectedMob = checkMob(_graph->_frontScreen, _mobList, true);
			}
			showTexts(_graph->_frontScreen);
			checkOptions();
		} else {
			_inventoryBackgroundRemember = false;
		}

		showPower();

	} else {
		displayInventory();
	}
}

void PrinceEngine::blackPalette() {
	byte *paletteBackup = (byte *)malloc(256 * 3);
	byte *blackPalette1 = (byte *)malloc(256 * 3);

	int fadeStep = kFadeStep - 1;
	for (int i = 0; i < kFadeStep; i++) {
		_system->getPaletteManager()->grabPalette(paletteBackup, 0, 256);
		for (int j = 0; j < 256; j++) {
			blackPalette1[3 * j] = paletteBackup[3 * j] * fadeStep / 4;
			blackPalette1[3 * j + 1] = paletteBackup[3 * j + 1] * fadeStep / 4;
			blackPalette1[3 * j + 2] = paletteBackup[3 * j + 2] * fadeStep / 4;
		}
		fadeStep--;
		_graph->setPalette(blackPalette1);
		_system->updateScreen();
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		eventMan->pollEvent(event);
		if (shouldQuit()) {
			free(paletteBackup);
			free(blackPalette1);
			return;
		}
		pausePrinceEngine();
	}
	free(paletteBackup);
	free(blackPalette1);
}

void PrinceEngine::setPalette(const byte *palette) {
	if (palette != nullptr) {
		byte *blackPalette_ = (byte *)malloc(256 * 3);
		int fadeStep = 0;
		for (int i = 0; i <= kFadeStep; i++) {
			for (int j = 0; j < 256; j++) {
				blackPalette_[3 * j] = palette[3 * j] * fadeStep / 4;
				blackPalette_[3 * j + 1] = palette[3 * j + 1] * fadeStep / 4;
				blackPalette_[3 * j + 2] = palette[3 * j + 2] * fadeStep / 4;
			}
			fadeStep++;
			_graph->setPalette(blackPalette_);
			_system->updateScreen();
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				_graph->setPalette(palette);
				free(blackPalette_);
				return;
			}
			pausePrinceEngine();
		}
		_graph->setPalette(palette);
		free(blackPalette_);
	}
}

void PrinceEngine::doTalkAnim(int animNumber, int slot, AnimType animType) {
	Text &text = _textSlots[slot];
	int lines = calcTextLines((const char *)_interpreter->getString());
	int time = lines * 30;
	if (animType == kNormalAnimation) {
		Anim &normAnim = _normAnimList[animNumber];
		if (normAnim._animData != nullptr) {
			if (!normAnim._state) {
				if (normAnim._currW && normAnim._currH) {
					text._color = _flags->getFlagValue(Flags::KOLOR);
					text._x = normAnim._currX + normAnim._currW / 2;
					text._y = normAnim._currY - 10;
				}
			}
		}
	} else if (animType == kBackgroundAnimation) {
		if (!_backAnimList[animNumber].backAnims.empty()) {
			int currAnim = _backAnimList[animNumber]._seq._currRelative;
			Anim &backAnim = _backAnimList[animNumber].backAnims[currAnim];
			if (backAnim._animData != nullptr) {
				if (!backAnim._state) {
					if (backAnim._currW && backAnim._currH) {
						text._color = _flags->getFlagValue(Flags::KOLOR);
						text._x = backAnim._currX + backAnim._currW / 2;
						text._y = backAnim._currY - 10;
					}
				}
			}
		}
	} else {
		error("doTalkAnim() - wrong animType: %d", animType);
	}
	text._time = time;
	if (getLanguage() == Common::DE_DEU) {
		correctStringDEU((char *)_interpreter->getString());
	}
	text._str = (const char *)_interpreter->getString();
	_interpreter->increaseString();
}

void PrinceEngine::freeNormAnim(int slot) {
	if (!_normAnimList.empty()) {
		_normAnimList[slot]._state = 1;
		if (_normAnimList[slot]._animData != nullptr) {
			delete _normAnimList[slot]._animData;
			_normAnimList[slot]._animData = nullptr;
		}
		if (_normAnimList[slot]._shadowData != nullptr) {
			delete _normAnimList[slot]._shadowData;
			_normAnimList[slot]._shadowData = nullptr;
		}
	}
}

void PrinceEngine::freeAllNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		freeNormAnim(i);
	}
}

} // End of namespace Prince
