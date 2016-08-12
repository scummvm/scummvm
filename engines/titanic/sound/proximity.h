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

#ifndef TITANIC_PROXIMITY_H
#define TITANIC_PROXIMITY_H

#include "common/scummsys.h"

namespace Titanic {

enum PositioningMode { POSMODE_NONE = 0, POSMODE_POLAR = 1, POSMODE_VECTOR = 2 };

class TTtalker;

typedef void (*CEndTalkerFn)(TTtalker *talker);

class CProximity {
public:
	int _field4;
	int _channelVolume;
	int _fieldC;
	int _priorSoundHandle;
	int _field14;
	double _frequencyMultiplier;
	double _field1C;
	bool _repeated;
	int _channel;
	PositioningMode _positioningMode;
	double _azimuth;
	double _range;
	double _elevation;
	double _posX;
	double _posY;
	double _posZ;
	bool _hasVelocity;
	double _velocityX;
	double _velocityY;
	double _velocityZ;
	int _field54;
	int _field58;
	int _field5C;
	bool _freeSoundFlag;
	CEndTalkerFn _endTalkerFn;
	TTtalker *_talker;
	int _field6C;
public:
	CProximity();
};

} // End of namespace Titanic

#endif /* TITANIC_PROXIMITY_H */
