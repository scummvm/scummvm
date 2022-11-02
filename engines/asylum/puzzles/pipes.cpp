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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/math.h"

#include "asylum/puzzles/pipes.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const int16 connectorPoints[21][2] = {
	{158,  59}, {163, 172}, {168, 272},
	{202,  59}, {205, 132}, {206, 172},
	{271,  60}, {272, 131}, {273, 262},
	{318, 169}, {319, 206}, {318, 261},
	{380,  72}, {360, 171}, {360, 206},
	{428, 172}, {401, 242}, {399, 295},
	{469, 119}, {466, 171}, {460, 294},
};

static const int16 peepholePoints[37][2] = {
	{140,  65}, {311,  44}, {387,  48}, {475,  72},
	{189,  67}, {246,  66}, {169, 113}, {215, 106},
	{280, 105}, {336,  95}, {434,  80}, {248, 136},
	{303, 154}, {407, 125}, {470, 151}, {193, 180},
	{347, 176}, {401, 177}, {245, 201}, {325, 196},
	{347, 212}, {406, 213}, {431, 218}, {174, 228},
	{217, 234}, {280, 227}, {325, 239}, {370, 244},
	{467, 239}, {303, 267}, {405, 273}, {356, 293},
	{436, 294}, {182, 317}, {277, 299}, {324, 291},
	{461, 323}
};

const uint32 peepholeResources[] = {15, 15, 15, 15, 32, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 32, 32, 15,
									15, 32, 32, 15, 15, 15, 15, 15, 15, 15, 15, 32, 15, 15, 15, 15, 15, 15, 15};

static BinNum calcStateFromPosition(ConnectorType type, uint32 position) {
	assert(position);
	position--;
	uint32 shift = !!position + !!(position >> 1) + !!(position >> 2);

	return BinNum((type >> shift | type << (4 - shift)) & 0xF);
}

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
Connector::Connector() :
	_id(0),
	_position(nullptr),
	_state(kBinNum0000),
	_isConnected(false),
	_nextConnector(nullptr),
	_type(kConnectorTypeI),
	_nextConnectorPosition(kDirectionNowhere) {
	memset(_nodes, 0, sizeof(_nodes));
}

void Connector::init(Peephole *n, Peephole *e, Peephole *s, Peephole *w, uint32 pos, ConnectorType type, Connector *nextConnector, Direction nextConnectorPosition) {
	_nodes[0] = n;
	_nodes[1] = e;
	_nodes[2] = s;
	_nodes[3] = w;

	*_position = pos;
	_type = type;
	_state = calcStateFromPosition(_type, *_position);

	_nextConnector = nextConnector;
	_nextConnectorPosition = nextConnectorPosition;
	_isConnected = false;

	for (uint32 i = 0; i < 4; ++i) {
		if (_state & ((uint32)1 << i) && _nodes[i]) {
			_nodes[i]->connect(this);
			_connectedNodes.push_back(_nodes[i]);
		}
	}
}

void Connector::initGroup() {
	if (!_isConnected && isReadyForConnection() && _nextConnector->isReadyForConnection())
		connect(_nextConnector);
}

void Connector::turn(bool updpos) {
	if (updpos)
		*_position = (*_position == 8) ? 1 : *_position << 1;
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
		newIndex[0] = (uint32)Common::intLog2(newState & delta);
		oldIndex[0] = (uint32)Common::intLog2(_state & delta);
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
			if (!(_nextConnectorPosition & _state))
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
Spider::Spider(AsylumEngine *engine, const Common::Rect &rect) : _vm(engine) {
	_boundingBox = rect;
	_isAlive = true;
	_location.x = (int16)rnd((uint16)(_boundingBox.right - _boundingBox.left + 1)) + _boundingBox.left;
	_location.y = (int16)rnd((uint16)(_boundingBox.bottom - _boundingBox.top + 1)) + _boundingBox.top;
	_direction = Direction((uint32)1 << rnd(4));
	_stepsNumber = 0;
	_steps = 0;

	randomize();
}

void Spider::randomize(Direction excluded) {
	if (rnd(6) == 5)
		_delta = Common::Point(0, 0);
	else {
		while (_direction == excluded)
			_direction = Direction((uint32)1 << rnd(4));

		_delta = Common::Point((_direction & kBinNum0010 ? 1 : 0) - (_direction & kBinNum1000 ? 1 : 0), (_direction & kBinNum0100 ? 1 : 0) - (_direction & kBinNum0001 ? 1 : 0));
	}

	_stepsNumber = rnd(maxStepsNumber - minStepsNumber + 1) + minStepsNumber;
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
	memset(&_levelFlags, false, sizeof(_levelFlags));
	_levelFlags[4] = true;
	memset(&_levelValues, 0, sizeof(_levelValues));
	memset(&_previousLevels, 0, sizeof(_previousLevels));
	_isLeverReady = false;
	memset(&_sinks, 0, sizeof(_sinks));
	memset(&_sources, 0, sizeof(_sources));
	_frameIndexSpider = nullptr;

	initResources();
	setup();
}

PuzzlePipes::~PuzzlePipes() {
	for (uint32 i = 0; i < _spiders.size(); ++i)
		delete _spiders[i];
	if (_frameIndexSpider)
		delete[] _frameIndexSpider;
}

void PuzzlePipes::saveLoadWithSerializer(Common::Serializer &s) {
	s.skip(16);
	for (uint32 i = 0; i < connectorsCount; i++) {
		s.syncAsUint32LE(_positions[i]);
	}
	s.skip(16);
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzlePipes::init(const AsylumEvent &) {
	_previousMusicVolume = getSound()->getMusicVolume();

	if (_previousMusicVolume >= -1000)
		getSound()->setMusicVolume(-1000);

	getSound()->playSound(getWorld()->graphicResourceIds[41], true, Config.ambientVolume);
	getScreen()->setPalette(getWorld()->graphicResourceIds[0]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[0]);

	_rectIndex = -2;

	checkConnections();
	startUpWater();
	(void)checkFlags();

	getCursor()->show();

	return true;
}

void PuzzlePipes::updateScreen() {
	getScreen()->clearGraphicsInQueue();
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[1], 0, Common::Point(0, 0), kDrawFlagNone, 0, 4);

	for (uint32 i = 0; i < ARRAYSIZE(_connectors); ++i)
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[_connectorResources[_connectors[i].getState()]], 0, &connectorPoints[i], kDrawFlagNone, 0, 1);

	uint32 filled = 0;
	for (uint32 i = 0; i < 4; ++i) {
		if (fabs(_levelValues[i] - _previousLevels[i]) > 0.005)
			_previousLevels[i] += _levelValues[i] > _previousLevels[i] ? 0.01f : -0.01f;
		else
			++filled;
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(210, 444 - int16(_previousLevels[0] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(276, 455 - int16(_previousLevels[1] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(376, 448 - int16(_previousLevels[2] * 52)), kDrawFlagNone, 0, 3);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[18], 0, Common::Point(458, 442 - int16(_previousLevels[3] * 52)), kDrawFlagNone, 0, 3);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[33], 0, Common::Point(204, 377), kDrawFlagNone, 0, 1);

	_frameIndex = (_frameIndex + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[15]);
	for (uint32 i = 0; i < ARRAYSIZE(_peepholes); ++i)
		if (_peepholes[i].isConnected())
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[peepholeResources[i]], _frameIndex, &peepholePoints[i], kDrawFlagNone, 0, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[2], _frameIndexLever, Common::Point(540, 90), kDrawFlagNone, 0, 1);
	_isLeverReady = false;
	if (_frameIndexLever) {
		_frameIndexLever = (_frameIndexLever + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[2]);
		if (!_frameIndexLever) {
			_isLeverReady = true;
			getCursor()->show();
		}
	}

	if (filled == 4) {
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
	}

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

	if (_isLeverReady) {
		_vm->clearGameFlag(kGameFlagBrokenPipeSpraying);
		_vm->clearGameFlag(kGameFlagSmFtnOverflows);
		_vm->clearGameFlag(kGameFlagFountainFilling);
		_vm->clearGameFlag(kGameFlagSewerExplodes);

		if (!_levelFlags[4])
			_vm->setGameFlag((GameFlag)(96 + checkFlags()));
		getScreen()->clear();
		getSound()->stop(getWorld()->graphicResourceIds[41]);
		getSound()->setMusicVolume(_previousMusicVolume);

		_vm->switchEventHandler(getScene());
	}
}

bool PuzzlePipes::mouseLeftDown(const AsylumEvent &) {
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

bool PuzzlePipes::mouseRightDown(const AsylumEvent &) {
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
	memset(&_levelValues, 0, sizeof(_levelValues));

	for (uint32 i = 0; i < peepholesCount; ++i)
		_peepholes[i].setId(i);

	for (uint32 i = 0; i < connectorsCount; ++i) {
		_connectors[i].setId(i);
		_connectors[i].setPos(&_positions[i]);
	}

	for (uint32 i = 0; i < 4; ++i) {
		_sinks[i] = &_peepholes[(peepholesCount - 4) + i];
		_sources[i] = &_peepholes[i];
		memset(&_sources[i]->_flowValues, 0, sizeof(_sources[i]->_flowValues));
		_sources[i]->_flowValues[i] = 1;
	}

	_connectors[ 0].init(        nullptr,  _peepholes + 4,  _peepholes + 6,  _peepholes + 0, 1, kConnectorTypeL);
	_connectors[ 1].init(_peepholes +  6, _peepholes + 15, _peepholes + 23,         nullptr, 1, kConnectorTypeL);
	_connectors[ 2].init(_peepholes + 23, _peepholes + 24, _peepholes + 33,         nullptr, 2, kConnectorTypeL);
	_connectors[ 3].init(        nullptr,  _peepholes + 5,  _peepholes + 7,  _peepholes + 4, 1, kConnectorTypeL);
	_connectors[ 4].init(_peepholes +  7, _peepholes + 11,         nullptr,         nullptr, 2, kConnectorTypeL,  _connectors + 5, kDirectionSh);
	_connectors[ 5].init(        nullptr, _peepholes + 18, _peepholes + 24, _peepholes + 15, 1, kConnectorTypeT,  _connectors + 4, kDirectionNh);
	_connectors[ 6].init(        nullptr,  _peepholes + 1,  _peepholes + 8,  _peepholes + 5, 1, kConnectorTypeL);
	_connectors[ 7].init(_peepholes +  8, _peepholes + 12, _peepholes + 25, _peepholes + 11, 1, kConnectorTypeT);
	_connectors[ 8].init(_peepholes + 25, _peepholes + 29, _peepholes + 34, _peepholes + 18, 2, kConnectorTypeT);
	_connectors[ 9].init(_peepholes +  9, _peepholes + 16, _peepholes + 19, _peepholes + 12, 8, kConnectorTypeT);
	_connectors[10].init(_peepholes + 19, _peepholes + 20, _peepholes + 26,         nullptr, 2, kConnectorTypeL);
	_connectors[11].init(_peepholes + 26, _peepholes + 31, _peepholes + 35, _peepholes + 29, 2, kConnectorTypeT);
	_connectors[12].init(_peepholes +  2, _peepholes + 10,         nullptr,  _peepholes + 9, 2, kConnectorTypeL);
	_connectors[13].init(_peepholes + 13, _peepholes + 17,         nullptr, _peepholes + 16, 1, kConnectorTypeT, _connectors + 14, kDirectionSh);
	_connectors[14].init(        nullptr, _peepholes + 21, _peepholes + 27, _peepholes + 20, 8, kConnectorTypeT, _connectors + 13, kDirectionNh);
	_connectors[15].init(_peepholes + 10,         nullptr, _peepholes + 22, _peepholes + 17, 1, kConnectorTypeI, _connectors + 19, kDirectionEt);
	_connectors[16].init(_peepholes + 21, _peepholes + 22, _peepholes + 30, _peepholes + 27, 2, kConnectorTypeT);
	_connectors[17].init(_peepholes + 30, _peepholes + 32,         nullptr, _peepholes + 31, 2, kConnectorTypeL);
	_connectors[18].init(_peepholes +  3,         nullptr, _peepholes + 14, _peepholes + 13, 8, kConnectorTypeL);
	_connectors[19].init(_peepholes + 14,         nullptr, _peepholes + 28,         nullptr, 4, kConnectorTypeL, _connectors + 15, kDirectionWt);
	_connectors[20].init(_peepholes + 28,         nullptr, _peepholes + 36, _peepholes + 32, 4, kConnectorTypeL);

	_connectors[ 4].initGroup();
	_connectors[13].initGroup();
	_connectors[15].initGroup();

	uint32 i = rnd(kBinNum1000);
	if (i & kBinNum0001)
		_spiders.push_back(new Spider(_vm, Common::Rect(-10,  45,  92, 315)));
	if (i & kBinNum0010)
		_spiders.push_back(new Spider(_vm, Common::Rect(-10, 389, 149, 476)));
	if (i & kBinNum0100)
		_spiders.push_back(new Spider(_vm, Common::Rect(544, 225, 650, 490)));

	if (i) {
		_frameIndexSpider = new uint32[_spiders.size()]();
	}
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
		if (Common::Rect(connectorPoints[i][0] - 5, connectorPoints[i][1] - 5, connectorPoints[i][0] + 30, connectorPoints[i][1] + 30).contains(getCursor()->position()))
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

void PuzzlePipes::checkConnections() {
	for (uint32 i = 0; i < connectorsCount; i++) {
		BinNum oldState = _connectors[i].getState(),
			newState = calcStateFromPosition(_connectors[i].getType(), _positions[i]);
		if (oldState != newState) {
			do {
				_connectors[i].turn(false);
			} while (_connectors[i].getState() != newState);
		}
	}
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
