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

#ifndef ASYLUM_PUZZLES_PIPES_H
#define ASYLUM_PUZZLES_PIPES_H

#include "common/list.h"
#include "common/hashmap.h"
#include "common/array.h"
#include "common/random.h"
#include "common/str.h"

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;
class Connector;

static const uint32 connectorsCount = 21, peepholesCount = 37;

enum BinNum {
	kBinNum0000,
	kBinNum0001,
	kBinNum0010,
	kBinNum0011,
	kBinNum0100,
	kBinNum0101,
	kBinNum0110,
	kBinNum0111,
	kBinNum1000,
	kBinNum1001,
	kBinNum1010,
	kBinNum1011,
	kBinNum1100,
	kBinNum1101,
	kBinNum1110,
	kBinNum1111
};

enum ConnectorType {
	kConnectorTypeI = kBinNum0101,
	kConnectorTypeL = kBinNum0110,
	kConnectorTypeT = kBinNum0111
};

enum Direction {
	kDirectionNowhere = kBinNum0000,
	kDirectionNh = kBinNum0001,
	kDirectionEt = kBinNum0010,
	kDirectionSh = kBinNum0100,
	kDirectionWt = kBinNum1000
};

class Peephole {
public:
	Peephole() : _id(0) {}
	~Peephole() {}

	static bool marks[peepholesCount];
	uint32 _flowValues[4];

	uint32 getId() { return _id; }
	void setId(uint32 id) { _id = id; }
	uint32 getLevel() { return (_flowValues[0] > 0) + (_flowValues[1] > 0) + (_flowValues[2]  > 0) + (_flowValues[3]  > 0); }
	uint32 getLevel1() { return _flowValues[0] + _flowValues[1] + _flowValues[2] + _flowValues[3]; }
	bool isConnected() { return isConnected(0) || isConnected(1) || isConnected(2) || isConnected(3); }

	void connect(Connector *connector) { _connectors.push_back(connector); }
	void disconnect(Connector *connector) { _connectors.remove(connector); }
	void startUpWater(bool flag = false);

private:
	uint32 _id;
	Common::List<Connector *> _connectors;

	bool isConnected(uint32 val) { return _flowValues[val]; }
};

class Connector {
public:
	Connector();
	~Connector() {}

	uint32 getId() { return _id; }
	void setId(uint32 id) { _id = id; }
	void setPos(uint32 *pos) { _position = pos; }
	BinNum getState() { return _state; }
	ConnectorType getType() { return _type; }

	void init(Peephole *n, Peephole *e, Peephole *s, Peephole *w, uint32 pos, ConnectorType type, Connector *nextConnector = NULL, Direction nextConnectorPosition = kDirectionNowhere);
	void initGroup();
	void turn(bool updpos = true);

private:
	uint32 _id;
	BinNum _state;
	ConnectorType _type;
	uint32 *_position;
	Peephole *_nodes[4];
	Common::List<Peephole *> _connectedNodes;

	Connector *_nextConnector;
	Direction _nextConnectorPosition;
	bool _isConnected;

	void connect(Connector *connector);
	void disconnect(Connector *connector);

	bool isReadyForConnection() { return _state & _nextConnectorPosition; }

	friend void Peephole::startUpWater(bool);
};

class Spider {
public:
	Spider(AsylumEngine *engine, const Common::Rect &rect);
	~Spider() {}

	bool isAlive() const { return _isAlive; }
	bool isActive() const { return _delta != Common::Point(0, 0); }
	bool isVisible(Common::Rect rect) const { return rect.contains(_location); }

	Direction getDirection() const { return _direction; }
	Common::Rect getPolygon(Common::Rect frame) const { return Common::Rect(_location.x - frame.left, _location.y - frame.top, _location.x + frame.right, _location.y + frame.bottom); }

	Common::Point move();
	void smash() { _isAlive = false; }
private:
	static const uint32 minStepsNumber = 20, maxStepsNumber = 200;
	AsylumEngine *_vm;
	bool _isAlive;
	Common::Point _location;
	Common::Point _delta;
	Common::Rect _boundingBox;
	Direction _direction;
	uint32 _stepsNumber;
	uint32 _steps;

	void randomize(Direction excluded = kDirectionNowhere);
};

class PuzzlePipes : public Puzzle {
public:
	PuzzlePipes(AsylumEngine *engine);
	~PuzzlePipes();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	int32 _previousMusicVolume;
	int32 _rectIndex;
	uint32 _frameIndex, _frameIndexLever;
	bool _levelFlags[5];
	float _levelValues[4], _previousLevels[4];
	bool _isLeverReady;
	Common::HashMap<uint32, uint32> _connectorResources;
	Connector _connectors[connectorsCount];
	uint32  _positions[connectorsCount];
	Peephole _peepholes[peepholesCount];
	Peephole *_sinks[4], *_sources[4];
	Common::Array<Spider *> _spiders;
	uint32 *_frameIndexSpider;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void initResources();
	void setup();
	void updateCursor();
	int32 findRect();
	uint32 checkFlags();
	void startUpWater();
	void checkConnections();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_PIPES_H
