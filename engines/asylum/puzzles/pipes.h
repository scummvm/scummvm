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

#ifndef ASYLUM_PIPES_H
#define ASYLUM_PIPES_H

#include "asylum/puzzles/puzzle.h"

#include "common/list.h"
#include "common/hashmap.h"

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

enum Direction {
	kDirectionNowhere = kBinNum0000,
	kDirectionNh = kBinNum0001,
	kDirectionEt = kBinNum0010,
	kDirectionSh = kBinNum0100,
	kDirectionWt = kBinNum1000
};

class Peephole {
public:
	Peephole() {}
	~Peephole() {}

	static bool marks[peepholesCount];
	uint32 _flowValues[4];

	uint32 getId() { return _id; }
	void setId(uint32 id) { _id = id; }
	uint32 getLevel() { return (_flowValues[0] > 0) + (_flowValues[1] > 0) + (_flowValues[2]  > 0) + (_flowValues[3]  > 0); }
	uint32 getLevel1() { return _flowValues[0] + _flowValues[1] + _flowValues[2] + _flowValues[3]; }
	bool isConnected() { return isConnected(0) || isConnected(1) || isConnected(2) || isConnected(3); }

	void connect(Connector *connector);
	void disconnect(Connector *connector);
	void startUpWater(bool flag = false);

private:
	uint32 _id;
	Common::List<Connector *> _connectors;

	bool isConnected(uint32 val) { return _flowValues[val]; }
};

class Connector {
public:
	Connector() {}
	~Connector() {}

	uint32 getId() { return _id; }
	void setId(uint32 id) { _id = id; }
	BinNum getState() { return _state; }

	void init(Peephole *n, Peephole *e, Peephole *s, Peephole *w, BinNum state, Connector *nextConnector = NULL, Direction nextConnectorPosition = kDirectionNowhere);
	void initGroup();
	void turn();

private:
	uint32 _id;
	BinNum _state;
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

class PuzzlePipes : public Puzzle {
public:
	PuzzlePipes(AsylumEngine *engine);
	~PuzzlePipes();

	void reset();

private:
	int32 _previousMusicVolume;
	int32 _rectIndex;
	uint32 _frameIndex, _frameIndexLever;
	bool _levelFlags[5];
	float _levelValues[4];
	bool _isLeverReady;
	Common::HashMap<uint32, uint32> _connectorResources;
	Connector _connectors[connectorsCount];
	Peephole _peepholes[peepholesCount];
	Peephole *_sinks[4], *_sources[4];

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	bool update(const AsylumEvent &evt);
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
};

} // End of namespace Asylum

#endif // ASYLUM_PIPES_H
