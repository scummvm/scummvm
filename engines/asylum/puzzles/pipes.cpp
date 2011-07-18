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

#include "asylum/puzzles/pipes.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

#include <math.h>

namespace Asylum {

const Common::Point connectorPoints[] = {
	Common::Point(158,  59),
	Common::Point(202,  59),
	Common::Point(271,  60),
	Common::Point(380,  72),
	Common::Point(205, 132),
	Common::Point(272, 131),
	Common::Point(469, 119),
	Common::Point(163, 172),
	Common::Point(206, 172),
	Common::Point(318, 169),
	Common::Point(360, 171),
	Common::Point(428, 172),
	Common::Point(466, 171),
	Common::Point(319, 206),
	Common::Point(360, 206),
	Common::Point(168, 272),
	Common::Point(273, 262),
	Common::Point(318, 261),
	Common::Point(401, 242),
	Common::Point(399, 295),
	Common::Point(460, 294)
};

const Common::Point peepholePoints[] = {
	Common::Point(140,  65),
	Common::Point(311,  44),
	Common::Point(387,  48),
	Common::Point(475,  72),
	Common::Point(189,  67),
	Common::Point(246,  66),
	Common::Point(169, 113),
	Common::Point(215, 106),
	Common::Point(280, 105),
	Common::Point(336,  95),
	Common::Point(434,  80),
	Common::Point(248, 136),
	Common::Point(303, 154),
	Common::Point(407, 125),
	Common::Point(470, 151),
	Common::Point(193, 180),
	Common::Point(347, 176),
	Common::Point(401, 177),
	Common::Point(245, 201),
	Common::Point(325, 196),
	Common::Point(347, 212),
	Common::Point(406, 213),
	Common::Point(431, 218),
	Common::Point(174, 228),
	Common::Point(217, 234),
	Common::Point(280, 227),
	Common::Point(325, 239),
	Common::Point(370, 244),
	Common::Point(467, 239),
	Common::Point(303, 267),
	Common::Point(405, 273),
	Common::Point(356, 293),
	Common::Point(436, 294),
	Common::Point(182, 317),
	Common::Point(277, 299),
	Common::Point(324, 291),
	Common::Point(461, 323)
};

const uint32 peepholeResources[] = {15, 15, 15, 15, 32, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 32, 32, 15,
                                    15, 32, 32, 15, 15, 15, 15,15, 15, 15, 15, 32, 15, 15, 15, 15, 15, 15, 15};

const double LOG2 = 0.6931471;

//////////////////////////////////////////////////////////////////////////
// Peephole
//////////////////////////////////////////////////////////////////////////
bool Peephole::marks[peepholesCount];

void Peephole::startUpWater(bool flag) {
	if (flag)
		memset(marks, false, sizeof(marks));

	marks[_id] = true;

	for (Common::List<Connector *>::iterator iter = _connectors.begin(); iter != _connectors.end(); ++iter) {
		for (Common::List<Peephole *>::iterator iter1 = (*iter)->_connectedNodes.begin(); iter1 != (*iter)->_connectedNodes.end(); ++iter1) {
			if (!marks[(*iter1)->getId()]) {

				for (uint32 i = 0; i < 4; ++i) {
					if (isConnected(i) && (*iter1)->getId() > 3)
						(*iter1)->_flowValues[i] += _flowValues[i];
				}

				(*iter1)->startUpWater();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Connector
//////////////////////////////////////////////////////////////////////////
void Connector::init(Peephole *n, Peephole *e, Peephole *s, Peephole *w, BinNum state, Connector *nextConnector, Direction nextConnectorPosition) {
	_nodes[0] = n;
	_nodes[1] = e;
	_nodes[2] = s;
	_nodes[3] = w;

	_state = state;
	_nextConnector = nextConnector;
	_nextConnectorPosition = nextConnectorPosition;
	_isConnected = false;

	for (uint32 i = 0; i < 4; ++i) {
		if (_state & (1 << i) && _nodes[i]) {
			_nodes[i]->connect(this);
			_connectedNodes.push_back(_nodes[i]);
		}
	}
}

void Connector::initGroup() {
	if (!_isConnected && isReadyForConnection() && _nextConnector->isReadyForConnection())
		connect(_nextConnector);
}

void Connector::turn() {
	BinNum newState = BinNum(_state >> 1 | (_state & 1) << 3);

	uint32 delta = _state ^ newState;
	uint32 newIndex[2], oldIndex[2];

	if (delta == kBinNum1111) {
		if (newState == kBinNum0101) {
			newIndex[0] = 0;
			newIndex[1] = 2;
			oldIndex[0] = 1;
			oldIndex[1] = 3;
		} else {
			newIndex[0] = 1;
			newIndex[1] = 3;
			oldIndex[0] = 0;
			oldIndex[1] = 2;
		}
	} else {
		newIndex[0] = log((double)(newState & delta)) / LOG2;
		oldIndex[0] = log((double)(_state & delta)) / LOG2;
	}

	for (uint32 i = 0; i < (uint32)(delta == kBinNum1111 ? 2 : 1); ++i) {
		if (_nodes[oldIndex[i]]) {
			_nodes[oldIndex[i]]->disconnect(this);
			_connectedNodes.remove(_nodes[oldIndex[i]]);
		}

		if (_nodes[newIndex[i]]) {
			_nodes[newIndex[i]]->connect(this);
			_connectedNodes.push_back(_nodes[newIndex[i]]);
		}
	}

	_state = newState;

	if (_nextConnector) {
		if (_isConnected) {
			if(!(_nextConnectorPosition & _state))
				disconnect(_nextConnector);
		} else if (_nextConnectorPosition & _state && _nextConnector->isReadyForConnection()) {
			connect(_nextConnector);
		}
	}
}

void Connector::connect(Connector *connector) {
	for (Common::List<Peephole *>::iterator iter = _connectedNodes.begin(); iter != _connectedNodes.end(); ++iter) {
		(*iter)->connect(connector);
		connector->_connectedNodes.push_back(*iter);
	}

	for (Common::List<Peephole *>::iterator iter = connector->_connectedNodes.begin(); iter != connector->_connectedNodes.end(); ++iter) {
		(*iter)->connect(this);
		_connectedNodes.push_back(*iter);
	}

	_isConnected = connector->_isConnected = true;

}

void Connector::disconnect(Connector *connector) {
	uint32 i;
	Common::List<Common::List<Peephole *>::iterator> markedForDeletion;
	bool flag;

	for (i = 0; i < 4; ++i)
		if (_nodes[i]) {
			_nodes[i]->disconnect(connector);
			connector->_connectedNodes.remove(_nodes[i]);
		}

	for (Common::List<Peephole *>::iterator iter = _connectedNodes.begin(); iter != _connectedNodes.end(); ++iter) {
		flag = true;

		for (i = 0; i < 4; ++i) {
			if (*iter == _nodes[i]) {
				flag = false;
				break;
			}
		}

		if (flag)
			markedForDeletion.push_back(iter);
	}

	for (Common::List<Common::List<Peephole *>::iterator>::iterator iter1 = markedForDeletion.begin(); iter1 != markedForDeletion.end(); ++iter1) {
		(*(*iter1))->disconnect(this);
		_connectedNodes.remove(*(*iter1));
	}

	_isConnected = connector->_isConnected = false;
}

//////////////////////////////////////////////////////////////////////////
// Spider
//////////////////////////////////////////////////////////////////////////
Spider::Spider(Common::Rect rect, Common::String id) {
	_boundingBox = rect;
	_rnd = new Common::RandomSource(Common::String("pipes_spider") + id);
	_isAlive = true;
	_location.x = _rnd->getRandomNumber(_boundingBox.right - _boundingBox.left) + _boundingBox.left;
	_location.y = _rnd->getRandomNumber(_boundingBox.bottom - _boundingBox.top) + _boundingBox.top;
	_direction = Direction(1 << _rnd->getRandomNumber(3));

	randomize();
}

void Spider::randomize(Direction excluded) {
	if (_rnd->getRandomNumber(5) == 5)
		_delta = Common::Point(0, 0);
	else {
		while (_direction == excluded)
			_direction = Direction(1 << _rnd->getRandomNumber(3));
		_delta = Common::Point((_direction & kBinNum0010 ? 1 : 0) - (_direction & kBinNum1000 ? 1 : 0), (_direction & kBinNum0100 ? 1 : 0) - (_direction & kBinNum0001 ? 1 : 0));
	}

	_stepsNumber = _rnd->getRandomNumber(maxStepsNumber - minStepsNumber) + minStepsNumber;
	_steps = 0;
}

Common::Point Spider::move() {
	Common::Point previousLocation(_location);

	if (_isAlive) {
		if (_steps++ > _stepsNumber)
			randomize();

		if (!_boundingBox.contains(_location + _delta))
			randomize(_direction);
		else
			_location += _delta;
	}

	return previousLocation;
}

//////////////////////////////////////////////////////////////////////////
// PuzzlePipes
//////////////////////////////////////////////////////////////////////////
PuzzlePipes::PuzzlePipes(AsylumEngine *engine) : Puzzle(engine) {
	_previousMusicVolume = 0;
	_rectIndex = -2;

	_frameIndex = _frameIndexLever = 0;
	memset(_previousLevels, 0, sizeof(_previousLevels));
	memset(&_levelFlags, false, sizeof(_levelFlags));
	_levelFlags[4] = true;

	_frameIndexSpider = NULL;
}

PuzzlePipes::~PuzzlePipes() {
	for (uint32 i = 0; i < _spiders.size(); ++i)
		delete _spiders[i];
	delete [] _frameIndexSpider;
}

void PuzzlePipes::reset() {
	warning("[PuzzlePipes::reset] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzlePipes::init(const AsylumEvent &evt) {
	_previousMusicVolume = getSound()->getMusicVolume();

	if (_previousMusicVolume >= -1000)
		getSound()->setMusicVolume(-1000);

	getSound()->playSound(getWorld()->graphicResourceIds[41], true, Config.ambientVolume);
	getScreen()->setPalette(getWorld()->graphicResourceIds[0]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[0], 0);

	_rectIndex = -2;

	initResources();
	setup();

	return true;
}

bool PuzzlePipes::update(const AsylumEvent &evt) {
	getScreen()->clear();
	getScreen()->clearGraphicsInQueue();
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[1], 0, Common::Point(0, 0), kDrawFlagNone, 0, 4);

	for (uint32 i = 0; i < ARRAYSIZE(_connectors); ++i)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[_connectorResources[_connectors[i].getState()]], 0, connectorPoints[i], kDrawFlagNone, 0, 1);

	for (uint32 i = 0; i < 4; ++i) {
		if (fabs(_levelValues[i] - _previousLevels[i]) > 0.005)
			_previousLevels[i] += _levelValues[i] > _previousLevels[i] ? 0.01 : -0.01;
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(210, 444 - uint32(_previousLevels[0] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(276, 455 - uint32(_previousLevels[1] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(376, 448 - uint32(_previousLevels[2] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(458, 442 - uint32(_previousLevels[3] * 52)), kDrawFlagNone, 0, 3);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[33], 0, Common::Point(204, 377), kDrawFlagNone, 0, 1);

	_frameIndex = (_frameIndex + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[15]);
	for (uint32 i = 0; i < ARRAYSIZE(_peepholes); ++i)
		if (_peepholes[i].isConnected())
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[peepholeResources[i]], _frameIndex, peepholePoints[i], kDrawFlagNone, 0, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[2], _frameIndexLever, Common::Point(540, 90), kDrawFlagNone, 0, 1);
	_isLeverReady = false;
	if (_frameIndexLever) {
		_frameIndexLever = (_frameIndexLever + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[2]);
		if (!_frameIndexLever) {
			_isLeverReady = true;
			getCursor()->show();
		}
	}

	// TODO: turn the fountain on
	if (_levelFlags[0])
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], 0, Common::Point(233, 416), kDrawFlagNone, 0, 1);
	else if (_levelFlags[1])
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], 0, Common::Point(299, 431), kDrawFlagNone, 0, 1);
	else if (_levelFlags[2])
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], 0, Common::Point(398, 421), kDrawFlagNone, 0, 1);
	else if (_levelFlags[3])
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[40], 0, Common::Point(481, 417), kDrawFlagNone, 0, 1);
	if (!_levelFlags[4])
		 getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[45], 0, Common::Point(518, 108), kDrawFlagNone, 0, 2);

	for (uint32 i = 0; i < _spiders.size(); ++i) {
		uint32 spiderResourceId = 0;

		switch (_spiders[i]->getDirection()) {
		default:
			error("[PuzzlePipes::update] Invalid spider direction (%d)", _spiders[i]->getDirection());

		case kDirectionNh:
			spiderResourceId = _spiders[i]->isAlive() ? 34 : 37;
			break;

		case kDirectionEt:
			spiderResourceId = _spiders[i]->isAlive() ? 35 : 38;	// FIXME
			break;

		case kDirectionSh:
			spiderResourceId = _spiders[i]->isAlive() ? 36 : 39;
			break;

		case kDirectionWt:
			spiderResourceId = _spiders[i]->isAlive() ? 35 : 38;
			break;
		}

		if (_spiders[i]->isVisible(Common::Rect(-10, -10, 650, 490))) {
			uint32 frameCountSpider = GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[spiderResourceId]);
			_frameIndexSpider[i] = _spiders[i]->isActive() ? (_frameIndexSpider[i] + 1) % frameCountSpider : 0;
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[spiderResourceId], _frameIndexSpider[i], _spiders[i]->move(), kDrawFlagNone, 0, 1);
		}
	}

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();
	updateCursor();

	if (_isLeverReady && !_levelFlags[4]) {
		getScreen()->clear();
		getSound()->stop(getWorld()->graphicResourceIds[41]);
		getSound()->setMusicVolume(_previousMusicVolume);

		_vm->switchEventHandler(getScene());
	}

	return true;
}

bool PuzzlePipes::mouseLeftDown(const AsylumEvent &evt) {
	Common::Point mousePos = getCursor()->position();

	if (Common::Rect(540, 90, 590, 250).contains(mousePos)) {
		if (!_frameIndexLever)
			++_frameIndexLever;
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[43], false, Config.sfxVolume - 10);
	} else {
		if (_rectIndex != -1) {
			if (_rectIndex < ARRAYSIZE(connectorPoints)) {
				getSound()->playSound(getWorld()->graphicResourceIds[42], false, Config.sfxVolume - 10);

				_connectors[_rectIndex].turn();
				startUpWater();
				memset(_levelFlags, false, sizeof(_levelFlags));
				_levelFlags[checkFlags()] = true;
			} else {
				getSound()->playSound(getWorld()->graphicResourceIds[44], false, Config.sfxVolume - 10);
				_spiders[_rectIndex - ARRAYSIZE(connectorPoints)]->smash();
				_frameIndexSpider[_rectIndex - ARRAYSIZE(connectorPoints)] = 0;
			}
		}
	}

	return true;
}

bool PuzzlePipes::mouseRightDown(const AsylumEvent &evt) {
	getScreen()->clear();
	getSound()->stop(getWorld()->graphicResourceIds[41]);
	getSound()->setMusicVolume(_previousMusicVolume);

	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzlePipes::initResources() {
	_connectorResources[kBinNum0011] =  4;
	_connectorResources[kBinNum0110] =  3;
	_connectorResources[kBinNum1100] =  6;
	_connectorResources[kBinNum1001] =  5;

	_connectorResources[kBinNum0111] =  7;
	_connectorResources[kBinNum1110] = 10;
	_connectorResources[kBinNum1101] =  9;
	_connectorResources[kBinNum1011] =  8;

	_connectorResources[kBinNum0101] = 11;
	_connectorResources[kBinNum1010] = 12;
}

void PuzzlePipes::setup() {
	memset(&_levelValues, 0.0, sizeof(_levelValues));

	for (uint32 i = 0; i < peepholesCount; ++i)
		_peepholes[i].setId(i);

	for (uint32 i = 0; i < connectorsCount; ++i)
		_connectors[i].setId(i);

	for (uint32 i = 0; i < 4; ++i) {
		_sinks[i] = &_peepholes[peepholesCount - 4 + i];
		_sources[i] = &_peepholes[i];
		memset(&_sources[i]->_flowValues, 0, sizeof(_sources[i]->_flowValues));
		_sources[i]->_flowValues[i] = 1;
	}

	_connectors[ 0].init(           NULL,  _peepholes + 4,  _peepholes + 6,  _peepholes + 0, kBinNum0110);
	_connectors[ 1].init(           NULL,  _peepholes + 5,  _peepholes + 7,  _peepholes + 4, kBinNum0110);
	_connectors[ 2].init(           NULL,  _peepholes + 1,  _peepholes + 8,  _peepholes + 5, kBinNum0110);
	_connectors[ 3].init(_peepholes +  2, _peepholes + 10,            NULL,  _peepholes + 9, kBinNum0011);
	_connectors[ 4].init(_peepholes +  7, _peepholes + 11,            NULL,            NULL, kBinNum0011,  _connectors + 8, kDirectionSh);
	_connectors[ 5].init(_peepholes +  8, _peepholes + 12, _peepholes + 25, _peepholes + 11, kBinNum0111);
	_connectors[ 6].init(_peepholes +  3,            NULL, _peepholes + 14, _peepholes + 13, kBinNum1100);
	_connectors[ 7].init(_peepholes +  6, _peepholes + 15, _peepholes + 23,            NULL, kBinNum0110);
	_connectors[ 8].init(           NULL, _peepholes + 18, _peepholes + 24, _peepholes + 15, kBinNum0111,  _connectors + 4, kDirectionNh);
	_connectors[ 9].init(_peepholes +  9, _peepholes + 16, _peepholes + 19, _peepholes + 12, kBinNum1110);
	_connectors[10].init(_peepholes + 13, _peepholes + 17,            NULL, _peepholes + 16, kBinNum0111, _connectors + 14, kDirectionSh);
	_connectors[11].init(_peepholes + 10,            NULL, _peepholes + 22, _peepholes + 17, kBinNum0101, _connectors + 12, kDirectionEt);
	_connectors[12].init(_peepholes + 14,            NULL, _peepholes + 28,            NULL, kBinNum1001, _connectors + 11, kDirectionWt);
	_connectors[13].init(_peepholes + 19, _peepholes + 20, _peepholes + 26,            NULL, kBinNum0011);
	_connectors[14].init(           NULL, _peepholes + 21, _peepholes + 27, _peepholes + 20, kBinNum1110, _connectors + 10, kDirectionNh);
	_connectors[15].init(_peepholes + 23, _peepholes + 24, _peepholes + 33,            NULL, kBinNum0011);
	_connectors[16].init(_peepholes + 25, _peepholes + 29, _peepholes + 34, _peepholes + 18, kBinNum1011);
	_connectors[17].init(_peepholes + 26, _peepholes + 31, _peepholes + 35, _peepholes + 29, kBinNum1011);
	_connectors[18].init(_peepholes + 21, _peepholes + 22, _peepholes + 30, _peepholes + 27, kBinNum1011);
	_connectors[19].init(_peepholes + 30, _peepholes + 32,            NULL, _peepholes + 31, kBinNum0011);
	_connectors[20].init(_peepholes + 28,            NULL, _peepholes + 36, _peepholes + 32, kBinNum1001);

	_connectors[ 4].initGroup();
	_connectors[10].initGroup();
	_connectors[11].initGroup();

	uint32 i = rnd(7);
	if (i & kBinNum0001)
		_spiders.push_back(new Spider(Common::Rect(-10, 45, 92, 315), "1"));
	if (i & kBinNum0010)
		_spiders.push_back(new Spider(Common::Rect(-10, 389, 149, 476), "2"));
	if (i & kBinNum0100)
		_spiders.push_back(new Spider(Common::Rect(544, 225, 650, 490), "3"));

	_frameIndexSpider = new uint32[_spiders.size()];
	memset(_frameIndexSpider, 0, sizeof(_frameIndexSpider));

	startUpWater();
}

void PuzzlePipes::updateCursor() {
	int32 index = findRect();

	if (_rectIndex == index)
		return;

	_rectIndex = index;
	// FIXME
	if (index > -1 || Common::Rect(540, 90, 590, 250).contains(getCursor()->position()))
		getCursor()->set(getWorld()->graphicResourceIds[16]);
	else
		getCursor()->set(getWorld()->graphicResourceIds[16], 0, kCursorAnimationNone);
}

int32 PuzzlePipes::findRect() {
	for (uint32 i = 0; i < ARRAYSIZE(connectorPoints); ++i)
		if (Common::Rect(connectorPoints[i].x - 5, connectorPoints[i].y - 5, connectorPoints[i].x + 30, connectorPoints[i].y + 30).contains(getCursor()->position()))
			return i;

	for (uint32 i = 0; i < _spiders.size(); ++i)
		if (_spiders[i]->getPolygon(Common::Rect(10, 10, 30, 30)).contains(getCursor()->position()))
			return ARRAYSIZE(connectorPoints) + i;

	return -1;
}

uint32 PuzzlePipes::checkFlags() {
	uint32 total = _sinks[0]->getLevel1() + _sinks[1]->getLevel1() +_sinks[2]->getLevel1() + _sinks[3]->getLevel1();
	float temp;
	uint32 val = 4;

	if (total)
		for (uint32 i = 0; i < 4; ++i) {
			temp = _sinks[i]->getLevel1() / float(total);
			_levelValues[i] = temp * _sinks[i]->getLevel() / 4;
			if (_levelValues[i] == 1.0)
				val = i;
		}
	else
		memset(_levelValues, 0, sizeof(_levelValues));

	return val;
}

void PuzzlePipes::startUpWater() {
	for (uint32 i = 4; i < peepholesCount; ++i)
		memset(_peepholes[i]._flowValues, 0, sizeof(_peepholes[i]._flowValues));

	_sources[0]->startUpWater(true);
	_sources[1]->startUpWater(true);
	_sources[2]->startUpWater(true);
	_sources[3]->startUpWater(true);
}

} // End of namespace Asylum
