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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags) : KyraEngine(system, flags) {
	memset(&_sceneAnims, 0, sizeof(_sceneAnims));
	memset(&_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));

	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;
}

KyraEngine_v2::~KyraEngine_v2() {
	for (ShapeMap::iterator i = _gameShapes.begin(); i != _gameShapes.end(); ++i) {
		delete [] i->_value;
		i->_value = 0;
	}
	_gameShapes.clear();
}

void KyraEngine_v2::updateInput() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			_quitFlag = true;
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == '.' || event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				_quitFlag = true;
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_MOUSEMOVE:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}
}

void KyraEngine_v2::removeInputTop() {
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v2::skipFlag() const {
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v2::resetSkipFlag(bool removeEvent) {
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}

bool KyraEngine_v2::checkSpecialSceneExit(int num, int x, int y) {
	if (_specialExitTable[0+num] > x || _specialExitTable[5+num] > y ||
		_specialExitTable[10+num] < x || _specialExitTable[15+num] < y)
		return false;
	return true;
}

void KyraEngine_v2::addShapeToPool(const uint8 *data, int realIndex, int shape) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::addShapeToPool(%p, %d, %d)", data, realIndex, shape);
	remShapeFromPool(realIndex);
	_gameShapes[realIndex] = screen_v2()->makeShapeCopy(data, shape);
	assert(_gameShapes[realIndex]);
}

void KyraEngine_v2::addShapeToPool(uint8 *shpData, int index) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::addShapeToPool(%p, %d)", shpData, index);
	remShapeFromPool(index);
	_gameShapes[index] = shpData;
	assert(_gameShapes[index]);
}

void KyraEngine_v2::remShapeFromPool(int idx) {
	ShapeMap::iterator iter = _gameShapes.find(idx);
	if (iter != _gameShapes.end()) {
		delete [] iter->_value;
		iter->_value = 0;
	}
}

uint8 *KyraEngine_v2::getShapePtr(int shape) const {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::getShapePtr(%d)", shape);
	ShapeMap::iterator iter = _gameShapes.find(shape);
	if (iter == _gameShapes.end())
		return 0;
	return iter->_value;
}

} // end of namespace Kyra

