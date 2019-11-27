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

#include "prince/mob.h"
#include "prince/animation.h"
#include "prince/font.h"

namespace Prince {

bool Mob::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();

	uint16 visible = stream.readUint16LE();

	if (visible == 0xFFFF)
		return false;

	_visible = visible;
	_type = stream.readUint16LE();
	_rect.left = stream.readUint16LE();
	_rect.top = stream.readUint16LE();
	_rect.right = stream.readUint16LE();
	_rect.bottom = stream.readUint16LE();

	_mask = stream.readUint16LE();

	_examPosition.x = stream.readUint16LE();
	_examPosition.y = stream.readUint16LE();
	_examDirection = (Direction)stream.readUint16LE();

	_usePosition.x = stream.readByte();
	_usePosition.y = stream.readByte();
	_useDirection = (Direction)stream.readUint16LE();

	uint32 nameOffset = stream.readUint32LE();
	uint32 examTextOffset = stream.readUint32LE();

	byte c;
	stream.seek(nameOffset);
	_name.clear();
	while ((c = stream.readByte()))
		_name += c;

	stream.seek(examTextOffset);
	_examText.clear();
	c = stream.readByte();
	if (c) {
		_examText += c;
		do {
			c = stream.readByte();
			_examText += c;
		} while (c != 255);
	}
	stream.seek(pos + 32);

	return true;
}

void Mob::setData(AttrId dataId, uint16 value) {
	switch (dataId) {
	case kMobExamDir:
		_examDirection = (Direction)value;
		break;
	case kMobExamX:
		_examPosition.x = value;
		break;
	case kMobExamY:
		_examPosition.y = value;
		break;
	default:
		assert(false);
	}
}

uint16 Mob::getData(AttrId dataId) {
	switch (dataId) {
	case kMobVisible:
		return _visible;
	case kMobExamDir:
		return _examDirection;
	case kMobExamX:
		return _examPosition.x;
	case kMobExamY:
		return _examPosition.y;
	default:
		assert(false);
		return 0;
	}
}

int PrinceEngine::getMob(Common::Array<Mob> &mobList, bool usePriorityList, int posX, int posY) {

	Common::Point pointPos(posX, posY);

	int mobListSize;
	if (usePriorityList) {
		mobListSize = _mobPriorityList.size();
	} else {
		mobListSize = mobList.size();
	}

	for (int mobNumber = 0; mobNumber < mobListSize; mobNumber++) {
		Mob *mob = nullptr;
		if (usePriorityList) {
			mob = &mobList[_mobPriorityList[mobNumber]];
		} else {
			mob = &mobList[mobNumber];
		}

		if (mob->_visible) {
			continue;
		}

		int type = mob->_type & 7;
		switch (type) {
		case 0:
		case 1:
			//normal_mob
			if (!mob->_rect.contains(pointPos)) {
				continue;
			}
			break;
		case 3:
			//mob_obj
			if (mob->_mask < kMaxObjects) {
				int nr = _objSlot[mob->_mask];
				if (nr != 0xFF) {
					Object &obj = *_objList[nr];
					Common::Rect objectRect(obj._x, obj._y, obj._x + obj._width, obj._y + obj._height);
					if (objectRect.contains(pointPos)) {
						Graphics::Surface *objSurface = obj.getSurface();
						byte *pixel = (byte *)objSurface->getBasePtr(posX - obj._x, posY - obj._y);
						if (*pixel != 255) {
							break;
						}
					}
				}
			}
			continue;
			break;
		case 2:
		case 5:
			//check_ba_mob
			if (!_backAnimList[mob->_mask].backAnims.empty()) {
				int currentAnim = _backAnimList[mob->_mask]._seq._currRelative;
				Anim &backAnim = _backAnimList[mob->_mask].backAnims[currentAnim];
				if (backAnim._animData != nullptr) {
					if (!backAnim._state) {
						Common::Rect backAnimRect(backAnim._currX, backAnim._currY, backAnim._currX + backAnim._currW, backAnim._currY + backAnim._currH);
						if (backAnimRect.contains(pointPos)) {
							int phase = backAnim._showFrame;
							int phaseFrameIndex = backAnim._animData->getPhaseFrameIndex(phase);
							Graphics::Surface *backAnimSurface = backAnim._animData->getFrame(phaseFrameIndex);
							byte pixel = *(byte *)backAnimSurface->getBasePtr(posX - backAnim._currX, posY - backAnim._currY);
							if (pixel != 255) {
								if (type == 5) {
									if (mob->_rect.contains(pointPos)) {
										break;
									}
								} else {
									break;
								}
							}
						}
					}
				}
			}
			continue;
			break;
		default:
			//not_part_ba
			continue;
			break;
		}

		if (usePriorityList) {
			return _mobPriorityList[mobNumber];
		} else {
			return mobNumber;
		}
	}
	return -1;
}

int PrinceEngine::checkMob(Graphics::Surface *screen, Common::Array<Mob> &mobList, bool usePriorityList) {
	if (_mouseFlag == 0 || _mouseFlag == 3) {
		return -1;
	}
	Common::Point mousePos = _system->getEventManager()->getMousePos();
	int mobNumber = getMob(mobList, usePriorityList, mousePos.x + _picWindowX, mousePos.y);

	if (mobNumber != -1) {
		Common::String mobName = mobList[mobNumber]._name;

		if (getLanguage() == Common::DE_DEU) {
			for (uint i = 0; i < mobName.size(); i++) {
				switch (mobName[i]) {
				case '\xc4':
					mobName.setChar('\x83', i);
					break;
				case '\xd6':
					mobName.setChar('\x84', i);
					break;
				case '\xdc':
					mobName.setChar('\x85', i);
					break;
				case '\xdf':
					mobName.setChar('\x7f', i);
					break;
				case '\xe4':
					mobName.setChar('\x80', i);
					break;
				case '\xf6':
					mobName.setChar('\x81', i);
					break;
				case '\xfc':
					mobName.setChar('\x82', i);
					break;
				default:
					break;
				}
			}
		}

		uint16 textW = getTextWidth(mobName.c_str());

		uint16 x = mousePos.x - textW / 2;
		if (x > screen->w) {
			x = 0;
		}

		if (x + textW > screen->w) {
			x = screen->w - textW;
		}

		uint16 y = mousePos.y - _font->getFontHeight();
		if (y > screen->h) {
			y = _font->getFontHeight() - 2;
		}

		_font->drawString(screen, mobName, x, y, screen->w, 216);
	}

	return mobNumber;
}

} // End of namespace Prince
