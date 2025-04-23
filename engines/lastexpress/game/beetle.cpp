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

#include "lastexpress/data/archive.h"

#include "lastexpress/game/beetle.h"

#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

Beetle::Beetle(LastExpressEngine *engine) : _engine(engine), _data(nullptr) {}

Beetle::~Beetle() {
	SAFE_DELETE(_data);

	// Free passed pointers
	_engine = nullptr;
}

void Beetle::load() {
	// Only load in chapter 2 & 3
	if (getProgress().chapter != kChapter2 &&  getProgress().chapter != kChapter3)
		return;

	// Already loaded
	if (_data)
		return;

	// Do not load if beetle is in the wrong location
	if (getInventory()->get(kItemBeetle)->location != kObjectLocation3)
		return;

	///////////////////////
	// Load Beetle data
	_data = new BeetleData();

	// Load sequences
	_data->sequences.push_back(loadSequence("BW000.seq"));        // 0
	_data->sequences.push_back(loadSequence("BT000045.seq"));
	_data->sequences.push_back(loadSequence("BT045000.seq"));
	_data->sequences.push_back(loadSequence("BW045.seq"));
	_data->sequences.push_back(loadSequence("BT045090.seq"));
	_data->sequences.push_back(loadSequence("BT090045.seq"));     // 5
	_data->sequences.push_back(loadSequence("BW090.seq"));
	_data->sequences.push_back(loadSequence("BT090135.seq"));
	_data->sequences.push_back(loadSequence("BT135090.seq"));
	_data->sequences.push_back(loadSequence("BW135.seq"));
	_data->sequences.push_back(loadSequence("BT135180.seq"));     // 10
	_data->sequences.push_back(loadSequence("BT180135.seq"));
	_data->sequences.push_back(loadSequence("BW180.seq"));
	_data->sequences.push_back(loadSequence("BT180225.seq"));
	_data->sequences.push_back(loadSequence("BT225180.seq"));
	_data->sequences.push_back(loadSequence("BW225.seq"));        // 15
	_data->sequences.push_back(loadSequence("BT225270.seq"));
	_data->sequences.push_back(loadSequence("BT270225.seq"));
	_data->sequences.push_back(loadSequence("BW270.seq"));
	_data->sequences.push_back(loadSequence("BT270315.seq"));
	_data->sequences.push_back(loadSequence("BT315270.seq"));     // 20
	_data->sequences.push_back(loadSequence("BW315.seq"));
	_data->sequences.push_back(loadSequence("BT315000.seq"));
	_data->sequences.push_back(loadSequence("BT000315.seq"));
	_data->sequences.push_back(loadSequence("BA135.seq"));
	_data->sequences.push_back(loadSequence("BL045.seq"));        // 25
	_data->sequences.push_back(loadSequence("BL000.seq"));
	_data->sequences.push_back(loadSequence("BL315.seq"));
	_data->sequences.push_back(loadSequence("BL180.seq"));

	// Init fields
	_data->field_74 = 0;

	// Check that all sequences are loaded properly
	_data->isLoaded = true;
	for (uint i = 0; i < _data->sequences.size(); i++) {
		if (!_data->sequences[i]->isLoaded()) {
			_data->isLoaded = false;
			break;
		}
	}

	_data->field_D9 = 10;
	_data->coordOffset = 5;
	_data->coordY = 178;
	_data->currentSequence = nullptr;
	_data->offset = 0;
	_data->frame = nullptr;
	_data->field_D5 = 0;
	_data->indexes[0] = 29;
	_data->field_DD = 0;
}

void Beetle::unload() {
	// Remove sequences from display list
	if (_data)
		getScenes()->removeFromQueue(_data->frame);

	// Delete all loaded sequences
	SAFE_DELETE(_data);
}

bool Beetle::isLoaded() const {
	if (!_data)
		return false;

	return _data->isLoaded;
}

bool Beetle::catchBeetle() {
	if (!_data)
		error("[Beetle::catchBeetle] Sequences have not been loaded");

	if (getInventory()->getSelectedItem() == kItemMatchBox
	 && getInventory()->hasItem(kItemMatch)
	 && ABS((int16)(getCoords().x - _data->coordX)) < 10
	 && ABS((int16)(getCoords().y - _data->coordY)) < 10) {
		return true;
	}

	_data->field_D5 = 0;
	move();

	return false;
}

bool Beetle::isCatchable() const {
	if (!_data)
		error("[Beetle::isCatchable] Sequences have not been loaded");

	return (_data->indexes[_data->offset] >= 30);
}

void Beetle::update() {
	if (!_data)
		error("[Beetle::update] Sequences have not been loaded");

	if (!_data->isLoaded)
		return;

	move();

	if (_data->field_D5)
		_data->field_D5--;

	if (_data->currentSequence && _data->indexes[_data->offset] != 29) {
		drawUpdate();
		return;
	}

	if (getInventory()->get(kItemBeetle)->location == kObjectLocation3) {
		if ((!_data->field_DD && rnd(10) < 1)
		  || (_data->field_DD && rnd(30) < 1)
		  || rnd(100) < 1) {

			_data->field_DD++;
			if (_data->field_DD > 3)
				_data->field_DD = 0;

			updateData(24);

			_data->coordX = (int16)(rnd(250) + 190);
			_data->coordOffset = (int16)(rnd(5) + 5);

			if (_data->field_D9 > 1)
				_data->field_D9--;

			drawUpdate();
		}
	}
}

void Beetle::drawUpdate() {
	if (!_data)
		error("[Beetle::drawUpdate] Sequences have not been loaded");

	if (_data->frame != nullptr) {
		getScenes()->setCoordinates(_data->frame);
		getScenes()->removeFromQueue(_data->frame);
	}

	// Update current frame
	switch (_data->indexes[_data->offset]) {
	default:
		_data->currentFrame += 10;
		break;

	case 3:
	case 6:
	case 9:
	case 12:
	case 15:
	case 18:
	case 21:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		_data->currentFrame++;
		break;
	}

	// Update current sequence
	if (_data->currentSequence->count() <= _data->currentFrame) {
		switch (_data->indexes[_data->offset]) {
		default:
			_data->offset++;
			_data->currentSequence = _data->sequences[_data->indexes[_data->offset]];
			break;

		case 3:
		case 6:
		case 9:
		case 12:
		case 15:
		case 18:
		case 21:
			break;
		}

		_data->currentFrame = 0;
		if (_data->indexes[_data->offset] == 29) {
			SAFE_DELETE(_data->frame);
			_data->currentSequence = nullptr; // pointer to existing sequence
			return;
		}
	}

	// Update coordinates
	switch (_data->indexes[_data->offset]) {
	default:
		break;

	case 0:
		_data->coordY -= _data->coordOffset;
		break;

	case 3:
		_data->coordX += _data->coordOffset;
		_data->coordY -= _data->coordOffset;
		break;

	case 6:
		_data->coordX += _data->coordOffset;
		break;

	case 9:
		_data->coordX += _data->coordOffset;
		_data->coordY += _data->coordOffset;
		break;

	case 12:
		_data->coordY += _data->coordOffset;
		break;

	case 15:
		_data->coordX -= _data->coordOffset;
		_data->coordY += _data->coordOffset;
		break;

	case 18:
		_data->coordX -= _data->coordOffset;
		break;

	case 21:
		_data->coordX -= _data->coordOffset;
		_data->coordY -= _data->coordOffset;
		break;
	}

	// Update beetle data
	int rnd = rnd(100);
	if (_data->coordX < 165 || _data->coordX > 465) {
		uint index = 0;

		if (rnd >= 30) {
			if (rnd >= 70)
				index = (_data->coordX < 165) ? 9 : 15;
			else
				index = (_data->coordX < 165) ? 6 : 18;
		} else {
			index = (_data->coordX < 165) ? 3 : 21;
		}

		updateData(index);
	}

	if (_data->coordY < 178) {
		switch (_data->indexes[_data->offset]) {
		default:
			updateData(26);
			break;

		case 3:
			updateData(25);
			break;

		case 21:
			updateData(27);
			break;
		}
	}

	if (_data->coordY > 354) {
		switch (_data->indexes[_data->offset]) {
		default:
			break;

		case 9:
		case 12:
		case 15:
			updateData(28);
			break;
		}
	}

	// Invert direction
	invertDirection();

	SequenceFrame *frame = new SequenceFrame(_data->currentSequence, (uint16)_data->currentFrame);
	updateFrame(frame);

	invertDirection();

	getScenes()->addToQueue(frame);

	SAFE_DELETE(_data->frame);
	_data->frame = frame;
}

void Beetle::invertDirection() {
	if (!_data)
		error("[Beetle::invertDirection] Sequences have not been loaded");

	switch (_data->indexes[_data->offset]) {
	default:
		break;

	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		_data->coordY = -_data->coordY;
		break;
	}
}

void Beetle::move() {
	if (!_data)
		error("[Beetle::move] Sequences have not been loaded");

	if (_data->indexes[_data->offset] >= 24 && _data->indexes[_data->offset] <= 29)
		return;

	if (_data->field_D5)
		return;

	if (ABS((int)(getCoords().x - _data->coordX)) > 35)
		return;

	if (ABS((int)(getCoords().y - _data->coordY)) > 35)
		return;

	int32 deltaX = getCoords().x - _data->coordX;
	int32 deltaY = -getCoords().y - _data->coordY;
	uint32 index = 0;

	// FIXME: check code path
	if (deltaX >= 0) {
		if (deltaY > 0) {
			if (100 * deltaY - 241 * deltaX <= 0) {
				if (100 * deltaY  - 41 * deltaX <= 0)
					index = 18;
				else
					index = 15;
			} else {
				index = 12;
			}

			goto update_data;
		}
	}

	if (deltaX < 0) {

		if (deltaY > 0) {
			if (100 * deltaY + 241 * deltaX <= 0) {
				if (100 * deltaY + 41 * deltaX <= 0)
					index = 6;
				else
					index = 9;
			} else {
				index = 12;
			}

			goto update_data;
		}

		if (deltaY <= 0) {
			if (100 * deltaY - 41 * deltaX <= 0) {
				if (100 * deltaY - 241 * deltaX <= 0)
					index = 0;
				else
					index = 3;
			} else {
				index = 6;
			}

			goto update_data;
		}
	}

update_data:
	updateData(index);

	if (_data->coordOffset >= 15) {
		_data->field_D5 = 0;
		return;
	}

	_data->coordOffset = _data->coordOffset + (int16)(4 * rnd(100)/100 + _data->field_D9);
	_data->field_D5 = 0;
}

// Update the beetle sequence to show the correct frames in the correct place
void Beetle::updateFrame(SequenceFrame *frame) const {
	if (!_data)
		error("[Beetle::updateFrame] Sequences have not been loaded");

	if (!frame)
		return;

	// Update coordinates
	if (_data->coordX > 0)
		frame->getInfo()->xPos1 = (uint16)_data->coordX;

	if (_data->coordY > 0)
		frame->getInfo()->yPos1 = (uint16)_data->coordY;
}

void Beetle::updateData(uint32 index) {
	if (!_data)
		error("[Beetle::updateData] Sequences have not been loaded");

	if (!_data->isLoaded)
		return;

	if (index == 25 || index == 26 || index == 27 || index == 28) {
		_data->indexes[0] = index;
		_data->indexes[1] = 29;
		_data->offset = 0;

		_data->currentSequence = _data->sequences[index];
		_data->currentFrame = 0;
		_data->index = index;
	} else {
		if (!_data->sequences[index])
			return;

		if (_data->index == index)
			return;

		_data->offset = 0;

		// Special case for sequence 24
		if (index == 24) {
			_data->indexes[0] = index;
			_data->coordY = 178;
			_data->index = _data->indexes[1];
			_data->indexes[1] = (_data->coordX >= 265) ? 15 : 9;
			_data->currentFrame = 0;
			_data->currentSequence = _data->sequences[index];
		} else {
			if (index <= _data->index) {
				for (uint32 i = _data->index - 1; i > index; ++_data->offset) {
					_data->indexes[_data->offset] = i;
					i -= 3;
				}
			} else {
				for (uint32 i = _data->index + 1; i < index; ++_data->offset) {
					_data->indexes[_data->offset] = i;
					i += 3;
				}
			}

			_data->index = index;
			_data->indexes[_data->offset] = index;
			_data->currentFrame = 0;
			_data->offset = 0;
			_data->currentSequence = _data->sequences[_data->indexes[0]];
		}
	}
}

CBeetle::CBeetle(LastExpressEngine *engine) {
	_engine = engine;

	// Walk sequences
	_sequences[0] = _engine->getArchiveManager()->loadSeq("BW000.seq", 15, 0);
	_sequences[3] = _engine->getArchiveManager()->loadSeq("BW045.seq", 15, 0);
	_sequences[6] = _engine->getArchiveManager()->loadSeq("BW090.seq", 15, 0);
	_sequences[9] = _engine->getArchiveManager()->loadSeq("BW135.seq", 15, 0);
	_sequences[12] = _engine->getArchiveManager()->loadSeq("BW180.seq", 15, 0);
	_sequences[15] = _engine->getArchiveManager()->loadSeq("BW225.seq", 15, 0);
	_sequences[18] = _engine->getArchiveManager()->loadSeq("BW270.seq", 15, 0);
	_sequences[21] = _engine->getArchiveManager()->loadSeq("BW315.seq", 15, 0);

	// Angle turn sequences
	_sequences[1] = _engine->getArchiveManager()->loadSeq("BT000045.seq", 15, 0);
	_sequences[4] = _engine->getArchiveManager()->loadSeq("BT045090.seq", 15, 0);
	_sequences[7] = _engine->getArchiveManager()->loadSeq("BT090135.seq", 15, 0);
	_sequences[10] = _engine->getArchiveManager()->loadSeq("BT135180.seq", 15, 0);
	_sequences[13] = _engine->getArchiveManager()->loadSeq("BT180225.seq", 15, 0);
	_sequences[16] = _engine->getArchiveManager()->loadSeq("BT225270.seq", 15, 0);
	_sequences[19] = _engine->getArchiveManager()->loadSeq("BT270315.seq", 15, 0);
	_sequences[22] = _engine->getArchiveManager()->loadSeq("BT315000.seq", 15, 0);

	// Inverse angle turn sequences
	_sequences[2] = _engine->getArchiveManager()->loadSeq("BT045000.seq", 15, 0);
	_sequences[5] = _engine->getArchiveManager()->loadSeq("BT090045.seq", 15, 0);
	_sequences[8] = _engine->getArchiveManager()->loadSeq("BT135090.seq", 15, 0);
	_sequences[11] = _engine->getArchiveManager()->loadSeq("BT180135.seq", 15, 0);
	_sequences[14] = _engine->getArchiveManager()->loadSeq("BT225180.seq", 15, 0);
	_sequences[17] = _engine->getArchiveManager()->loadSeq("BT270225.seq", 15, 0);
	_sequences[20] = _engine->getArchiveManager()->loadSeq("BT315270.seq", 15, 0);
	_sequences[23] = _engine->getArchiveManager()->loadSeq("BT000315.seq", 15, 0);

	// Other sequences
	_sequences[24] = _engine->getArchiveManager()->loadSeq("BA135.seq", 15, 0);
	_sequences[25] = _engine->getArchiveManager()->loadSeq("BL045.seq", 15, 0);
	_sequences[26] = _engine->getArchiveManager()->loadSeq("BL000.seq", 15, 0);
	_sequences[27] = _engine->getArchiveManager()->loadSeq("BL315.seq", 15, 0);
	_sequences[28] = _engine->getArchiveManager()->loadSeq("BL180.seq", 15, 0);
	_sequences[29] = nullptr;
	_loaded = true;

	for (int i = 0; i < 29; i++) {
		if (!_sequences[i]) {
			_loaded = false;
			break;
		}
	}
	
	_fleeSpeed = 10;
	_coordOffset = 5;
	_coords.y = 178;
	_currentSequence = nullptr;
	_currentDirectionIndex = 0;
	_frame = nullptr;
	_mouseCooldown = 0;
	_directions[0] = 29;
	_spawnCounter = 0;
}

CBeetle::~CBeetle() {
	if (_currentSequence)
		_engine->getSpriteManager()->removeSprite(&_currentSequence->sprites[_currentFrame]);

	for (int i = 0; i < ARRAYSIZE(_sequences); i++) {
		if (_sequences[i]) {
			_engine->getMemoryManager()->freeMem(_sequences[i]->rawSeqData);
			delete _sequences[i];
			_sequences[i] = nullptr;
		}
	}

	_currentSequence = nullptr;
}

void CBeetle::tick() {
	if (!_loaded)
		return;

	checkMouse();

	if (_mouseCooldown)
		_mouseCooldown--;

	if (!_currentSequence || _directions[_currentDirectionIndex] == 29) {
		if (_engine->getLogicManager()->_gameInventory[kItemBeetle].location != 3 ||
			((_spawnCounter || rnd(10)) && (_spawnCounter >= 3 || rnd(30)) && rnd(100))) {
			return;
		}

		_spawnCounter++;
		if (_spawnCounter > 3)
			_spawnCounter = 0;

		_engine->_beetle->setDirection(24);
		_coords.x = rnd(250) + 190;
		_coordOffset = rnd(5) + 5;

		if (_fleeSpeed > 1)
			_fleeSpeed--;
	}

	if (_frame) {
		_engine->getSpriteManager()->queueErase(_frame);
		_engine->getSpriteManager()->removeSprite(_frame);
	}

	int curDir = _directions[_currentDirectionIndex];
	if (curDir == 0 || curDir == 3 || curDir == 6 || curDir == 9 || curDir == 12 || curDir == 15 || curDir == 18 || curDir == 21 || curDir == 24 || curDir == 27 || curDir == 26 || curDir == 25 || curDir == 28) {
		_currentFrame++;
	} else {
		_currentFrame += 10;
	}

	bool sequenceEnded = false;
	bool terminate = false;

	if (_currentSequence->numFrames <= _currentFrame) {
		curDir = _directions[_currentDirectionIndex];
		if (curDir != 0 && curDir != 3 && curDir != 6 && curDir != 9 && curDir != 12 && curDir != 15 && curDir != 18 && curDir != 21) {
			_currentDirectionIndex++;
			_currentSequence = _sequences[_directions[_currentDirectionIndex]];
		}

		_currentFrame = 0;
		if (_directions[_currentDirectionIndex] == 29) {
			_frame = nullptr;
			_currentSequence = nullptr;
			terminate = true;
		}

		sequenceEnded = true;
	}

	if (terminate) {
		return;
	}

	curDir = _directions[_currentDirectionIndex];
	if (!curDir || curDir == 3 || curDir == 6 || curDir == 9 || curDir == 12 || curDir == 15 || curDir == 18 || curDir == 21) {
		switch (curDir) {
		case 0:
			_coords.y -= _coordOffset;
			break;
		case 3:
			_coords.x += _coordOffset;
			_coords.y -= _coordOffset;
			break;
		case 6:
			_coords.x += _coordOffset;
			break;
		case 9:
			_coords.x += _coordOffset;
			_coords.y += _coordOffset;
			break;
		case 12:
			_coords.y += _coordOffset;
			break;
		case 15:
			_coords.x -= _coordOffset;
			_coords.y += _coordOffset;
			break;
		case 18:
			_coords.x -= _coordOffset;
			break;
		case 21:
			_coords.x -= _coordOffset;
			_coords.y -= _coordOffset;
			break;
		default:
			break;
		}
	}

	uint randNum = rnd(100);
	if (_coords.x > 465) {
		if (randNum >= 30) {
			if (randNum >= 70)
				setDirection(15);
			else
				setDirection(18);
		} else {
			setDirection(21);
		}
	}

	if (_coords.x < 165) {
		if (randNum >= 30) {
			if (randNum >= 70)
				setDirection(9);
			else
				setDirection(6);
		} else {
			setDirection(3);
		}
	}

	if (_coords.y < 178) {
		curDir = _directions[_currentDirectionIndex];
		if (curDir) {
			if (curDir == 3) {
				setDirection(25);
			} else if (curDir == 21) {
				setDirection(27);
			}
		} else {
			setDirection(26);
		}
	}

	if (_coords.y > 354) {
		curDir = _directions[_currentDirectionIndex];
		if (curDir == 9 || curDir == 12 || curDir == 15)
			setDirection(28);
	}

	curDir = _directions[_currentDirectionIndex];
	if (curDir == 24 || curDir == 27 || curDir == 26 || curDir == 25 || curDir == 28)
		_coords.y = -_coords.y;

	_engine->positionSprite(&_currentSequence->sprites[_currentFrame], _coords);

	curDir = _directions[_currentDirectionIndex];
	if (curDir == 24 || curDir == 27 || curDir == 26 || curDir == 25 || curDir == 28)
		_coords.y = -_coords.y;

	_engine->getSpriteManager()->drawSprite(&_currentSequence->sprites[_currentFrame]);

	_frame = &_currentSequence->sprites[_currentFrame];
}

void CBeetle::checkMouse() {
	int16 cursorX;
	int16 cursorY;
	int curDir;
	int scaledDiffY;
	int16 diffX;
	int16 diffY;

	cursorX = _engine->_cursorX;
	cursorY = _engine->_cursorY;
	curDir = _directions[_currentDirectionIndex];

	if (curDir != 29 && curDir != 27 && curDir != 26 && curDir != 25 && curDir != 28 && curDir != 24 &&
		!_mouseCooldown && ABS<int16>(cursorX - _coords.x) <= 35) {

		if (ABS<int16>(cursorY - _coords.y) <= 35) {
			diffX = cursorX - _coords.x;
			diffY = _coords.y - cursorY;

			if (diffX < 0) {
				if (diffY <= 0) {
					scaledDiffY = 100 * diffY;
					if (scaledDiffY - 41 * diffX <= 0) {
						if (scaledDiffY - 241 * diffX <= 0) {
							setDirection(0);
						} else {
							setDirection(3);
						}
					} else {
						setDirection(6);
					}

					if (_coordOffset >= 15) {
						_mouseCooldown = 10;
						return;
					}
				} else {
					scaledDiffY = 100 * diffY;
					if (scaledDiffY + 241 * diffX <= 0) {
						if (scaledDiffY + 41 * diffX <= 0) {
							setDirection(6);
						} else {
							setDirection(9);
						}
					} else {
						setDirection(12);
					}

					if (_coordOffset >= 15) {
						_mouseCooldown = 10;
						return;
					}
				}
			} else if (diffY <= 0) {
				scaledDiffY = 100 * diffY;
				if (scaledDiffY + 41 * diffX <= 0) {
					if (scaledDiffY + 241 * diffX <= 0) {
						setDirection(0);
					} else {
						setDirection(21);
					}
				} else {
					setDirection(18);
				}

				if (_coordOffset >= 15) {
					_mouseCooldown = 10;
					return;
				}
			} else {
				scaledDiffY = 100 * diffY;
				if (scaledDiffY - 241 * diffX <= 0) {
					if (scaledDiffY - 41 * diffX <= 0) {
						setDirection(18);
					} else {
						setDirection(15);
					}
				} else {
					setDirection(12);
				}

				if (_coordOffset >= 15) {
					_mouseCooldown = 10;
					return;
				}
			}

			_coordOffset = _coordOffset + 4 * (rnd(100)) / 100 + _fleeSpeed;
			_mouseCooldown = 10;
		}
	}
}

void CBeetle::setDirection(int direction) {
	if (_loaded) {
		if (direction == 27 || direction == 26 || direction == 25 || direction == 28) {
			_directions[0] = direction;
			_directions[1] = 29;
			_currentDirectionIndex = 0;
			_currentSequence = _sequences[direction];
			_currentFrame = 0;
			_index = direction;
		} else if (_sequences[direction]) {
			if (direction != _index) {
				_currentDirectionIndex = 0;

				if (direction == 24) {
					_directions[0] = 24;
					_coords.y = 178;

					if (_coords.x >= 265) {
						_directions[1] = 15;
					} else {
						_directions[1] = 9;
					}

					_currentSequence = _sequences[24];
					_currentFrame = 0;
					_index = _directions[1];
				} else {
					if (direction <= _index) {
						for (int i = _index - 1; i > direction; ++_currentDirectionIndex) {
							_directions[_currentDirectionIndex] = i;
							i -= 3;
						}
					} else {
						for (int j = _index + 1; j < direction; ++_currentDirectionIndex) {
							_directions[_currentDirectionIndex] = j;
							j += 3;
						}
					}

					_index = direction;
					_directions[_currentDirectionIndex] = direction;
					_currentFrame = 0;
					_currentDirectionIndex = 0;
					_currentSequence = _sequences[_directions[0]];
				}
			}
		}
	}
}

bool CBeetle::onTable() {
	return _directions[_currentDirectionIndex] != 29;
}

bool CBeetle::click() {
	if (_engine->getLogicManager()->_inventorySelectedItemIdx == kItemMatchBox &&
		_engine->getLogicManager()->cathHasItem(12) &&
		ABS<int16>(_engine->_cursorX - _coords.x) < 10 &&
		ABS<int16>(_engine->_cursorY - _coords.y) < 10) {
		return true;
	}

	_mouseCooldown = 0;
	checkMouse();

	return false;
}

void LastExpressEngine::doBeetle() {
	int32 chapter = getLogicManager()->_gameProgress[11];
	if (chapter >= 2 && chapter <= 3 && !_beetle && getLogicManager()->_gameInventory[kItemBeetle].location == 3) {
		_beetle = new CBeetle(this);
	}
}

void LastExpressEngine::endBeetle() {
	if (_beetle) {
		delete _beetle;
		_beetle = nullptr;
	}
}

void LastExpressEngine::positionSprite(Sprite *sprite, Common::Point coord) {
	if (sprite) {
		int spriteWidth = sprite->rect.right - sprite->rect.left + 1;
		int spriteHeight = sprite->rect.bottom - sprite->rect.top + 1;
		int widthFactor = sprite->rect.width + 2 * (-640 * sprite->rect.top - sprite->rect.left);
		int heightFactor = 2 * (sprite->rect.right + 640 * sprite->rect.bottom) - sprite->rect.height;

		if (coord.x > 0)
			sprite->rect.left = coord.x;

		if (coord.y > 0)
			sprite->rect.top = coord.y;


		sprite->rect.right = sprite->rect.left + spriteWidth - 1;
		sprite->rect.bottom = sprite->rect.top + spriteHeight - 1;
		sprite->rect.width = widthFactor + 2 * (sprite->rect.left + 640 * sprite->rect.top);
		sprite->rect.height = 2 * (sprite->rect.right + 640 * sprite->rect.bottom) - heightFactor;
	}
}

} // End of namespace LastExpress
