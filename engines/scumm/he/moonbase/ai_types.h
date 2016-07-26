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

#ifndef SCUMM_HE_MOONBASE_AI_TYPES_H
#define SCUMM_HE_MOONBASE_AI_TYPES_H

namespace Scumm {

enum {
	AGI = 1,
	AONE = 2,
	BRUTAKAS = 3,
	CYBALL = 4,
	EL_GATO = 5,
	NEEP = 6,
	ORBNU_LUNATEK = 7,
	PIXELAHT = 8,
	SPANDO = 9,
	WARCUPINE = 10
};

enum {
	CRAWLER_CHUCKER = 11,
	ENERGY_HOG = 12,
	RANGER = 13
};

enum {
	AI_VAR_NONE = -1,
	AI_VAR_SMALL = 0,
	AI_VAR_MEDIUM = 1,
	AI_VAR_LARGE = 2,
	AI_VAR_HUGE = 5
};

enum {
	AI_VAR_BASE_BEHAVIOR = 10,
	AI_VAR_BASE_TARGET = 10,
	AI_VAR_BASE_ANGLE = 2,
	AI_VAR_BASE_POWER = 5
};

class AIEntity {
private:
	int _id;
	char *_nameString;
	int _behaviorVariation;
	int _targetVariation;
	int _angleVariation;
	int _powerVariation;

public:
	AIEntity(int id);
	~AIEntity() {
		if (_nameString) {
			delete[] _nameString;
			_nameString = 0;
		}
	}

	int getID() const { return _id; }
	char *getNameString() const { return _nameString; }
	int getBehaviorVariation() const { return _behaviorVariation; }
	int getTargetVariation() const { return _targetVariation; }
	int getAngleVariation() const { return _angleVariation; }
	int getPowerVariation() const { return _powerVariation; }

	void setID(int id) { _id = id; }
	void setNameString(char *nameString) { _nameString = nameString; }
	void setBehaviorVariation(int behaviorVariation) { _behaviorVariation = behaviorVariation; }
	void setTargetVariation(int targetVariation) { _targetVariation = targetVariation; }
	void setAngleVariation(int angleVariation) { _angleVariation = angleVariation; }
	void setPowerVariation(int powerVariation) { _powerVariation = powerVariation; }
};

} // End of namespace Scumm

#endif
