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

#ifndef SCUMM_HE_MOONBASE_AI_PATTERN_H
#define SCUMM_HE_MOONBASE_AI_PATTERN_H

namespace Scumm {

const int NO_PATTERN = 0;
const int PATTERN_FOUND = 1;

class patternInstance {
private:
	int _sourceHub;
	int _unit;
	int _power;
	int _angle;

public:
	patternInstance() {
		_sourceHub = 0;
		_unit = 0;
		_power = 0;
		_angle = 0;
	}

	patternInstance(int sh, int unit, int power, int angle) {
		setSourceHub(sh);
		setUnit(unit);
		setPower(power);
		setAngle(angle);
	}

	void setSourceHub(int sh) { _sourceHub = sh; }
	void setUnit(int unit) { _unit = unit; }

	void setPower(int power) {
		if (power < 300)
			_power = 1;
		else if (power < 480)
			_power = 2;
		else
			_power = 3;
	}

	void setAngle(int angle) {
		int tempAngle = angle % 360;

		if ((tempAngle >= 0) && (tempAngle < 90))
			_angle = 1;

		if ((tempAngle >= 90) && (tempAngle < 180))
			_angle = 2;

		if ((tempAngle >= 180) && (tempAngle < 270))
			_angle = 3;

		if ((tempAngle >= 270))
			_angle = 4;
	}

	int getSourceHub() const { return _sourceHub; }
	int getUnit() const { return _unit; }
	int getPowerIndex() const { return _power; }
	int getAngleIndex() const { return _angle; }

	static int comparePatterns(patternInstance *p1, patternInstance *p2) {
		if (p1->getSourceHub() != p2->getSourceHub())
			return 0;

		if (p1->getUnit() != p2->getUnit())
			return 0;

		if (p1->getUnit() == -999)
			return 0;

		int temp = abs(p1->getPowerIndex() - p2->getPowerIndex());

		if (temp > 1)
			return 0;

		temp = abs(p1->getAngleIndex() - p2->getAngleIndex());

		if (temp > 1 && temp < 3)
			return 0;

		return 1;
	}
};

class patternList {
private:
	patternInstance *theList[10];
	int listIndex;

public:
	patternList() {
		for (int i = 0; i < 10; i++) {
			theList[i] = new patternInstance();
		}

		listIndex = 0;
	}
	~patternList() {
		for (int i = 0; i < 10; i++) {
			delete theList[i];
		}
	}

	void addPattern(int sh, int unit, int power, int angle) {
		theList[listIndex]->setSourceHub(sh);
		theList[listIndex]->setUnit(unit);
		theList[listIndex]->setPower(power);
		theList[listIndex]->setAngle(angle);

		listIndex++;

		if (listIndex > 9)
			listIndex = 0;
	}

	int evaluatePattern(int sh, int unit, int power, int angle) {
		patternInstance *patternToMatch = new patternInstance(sh, unit, power, angle);
		int matchCount = 0;

		for (int i = 0; i < 9; i++) {
			if (patternInstance::comparePatterns(theList[i], patternToMatch)) {
				matchCount++;
			}
		}

		delete patternToMatch;

		if (matchCount > 2)
			return PATTERN_FOUND;

		return NO_PATTERN;
	}
};

} // End of namespace Scumm

#endif
