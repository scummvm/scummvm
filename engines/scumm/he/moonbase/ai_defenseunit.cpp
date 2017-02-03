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

#include "common/rect.h"
#include "common/util.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_defenseunit.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

DefenseUnit::DefenseUnit(AI *ai) : _ai(ai) {
	_state = DUS_ON;

	_id = -1;
	_distanceTo = 0;
	_state = 0;
	_radius = 0;
	_armor = 0;
	_cost = 0;
}

DefenseUnit::DefenseUnit(DefenseUnit *inUnit, AI *ai) : _ai(ai) {
	_id = inUnit->getID();
	_pos.x = inUnit->getPosX();
	_pos.y = inUnit->getPosY();
	_distanceTo = inUnit->getDistanceTo();
	_state = inUnit->getState();
	_radius = inUnit->getRadius();
	_armor = inUnit->getArmor();
	_cost = inUnit->getCost();
}

DefenseUnit::~DefenseUnit() {
}

Common::Point *AntiAirUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	float ratio;
	int radius;
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CRAWLER:
		radius = getRadius();

		if ((distance < radius) || (getState() == DUS_OFF)) {
			targetPos->x = getPosX();
			targetPos->y = getPosY();
		} else {
			ratio = MAX(0, (getRadius() / distance));
			targetPos->x = (int16)(getPosX() - ratio * (getPosX() - sourceX));
			targetPos->y = (int16)(getPosY() - ratio * (getPosY() - sourceY));
		}

		break;

	case ITEM_EMP:
		if (getRadius() + 215 > distance) { // Emp radius
			double x1 = static_cast<double>(sourceX);
			double y1 = static_cast<double>(sourceY);
			double x2 = static_cast<double>(getPosX());
			double y2 = static_cast<double>(getPosY());
			double r1 = static_cast<double>(215);
			double r2 = static_cast<double>(getRadius() + 3);
			double d = static_cast<double>(distance);

			// Formulae for calculating one point of intersection of two circles
			float root = sqrt((((r1 + r2) * (r1 + r2)) - (d * d)) * ((d * d) - ((r2 - r1) * (r2 - r1))));
			int x = (int)(((x1 + x2) / 2) + ((x2 - x1) * (r1 * r1 - r2 * r2)) / (2 * d * d) + ((y2 - y1) / (2 * d * d)) * root);
			int y = (int)(((y1 + y2) / 2) + ((y2 - y1) * (r1 * r1 - r2 * r2)) / (2 * d * d) - ((x2 - x1) / (2 * d * d)) * root);

			targetPos->x = x;
			targetPos->y = y;
		} else {
			ratio = 1 - (getRadius() / static_cast<float>(distance - 20));
			targetPos->x = (int16)(sourceX + ratio * (getPosX() - sourceX));
			targetPos->y = (int16)(sourceY + ratio * (getPosY() - sourceY));
		}

		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int AntiAirUnit::selectWeapon(int index) {
	switch (index) {
	case 0:
		return ITEM_CLUSTER;
		break;

	case 1:
		return ITEM_EMP;
		break;

	case 2:
		if (getState() == DUS_OFF) {
			if (_ai->getPlayerEnergy() > 6) {
				if (!_ai->_vm->_rnd.getRandomNumber(3)) {
					return ITEM_VIRUS;
				}
			}

			if (_ai->getPlayerEnergy() > 2) {
				if (!_ai->_vm->_rnd.getRandomNumber(1)) {
					return ITEM_SPIKE;
				}
			}

			return ITEM_BOMB;
		}

		return ITEM_CLUSTER;
		break;

	default:
		return ITEM_CLUSTER;
		break;
	}
}

Common::Point *ShieldUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	float ratio;
	Common::Point *targetPos = new Common::Point;

	if (getState() == DUS_OFF) {
		targetPos->x = getPosX();
		targetPos->y = getPosY();
	} else {
		switch (weaponType) {
		case ITEM_BOMB:
			targetPos->x = getPosX();
			targetPos->y = getPosY();
			break;

		case ITEM_CLUSTER:
			targetPos->x = getPosX();
			targetPos->y = getPosY();
			break;

		case ITEM_CRAWLER:
			ratio = MAX(0.0, 1.0 - (static_cast<float>(getRadius()) / static_cast<float>(distance - 20)));
			{
				int maxX = _ai->getMaxX();
				int maxY = _ai->getMaxY();
				int thisX = (static_cast<int>(sourceX + ratio * (getPosX() - sourceX)) + maxX) % maxX;
				int thisY = (static_cast<int>(sourceY + ratio * (getPosY() - sourceY)) + maxY) % maxY;
				targetPos->x = thisX;
				targetPos->y = thisY;
			}
			break;

		case ITEM_EMP:
			if (getRadius() + 215 > distance) { // Emp radius
				double x1 = static_cast<double>(sourceX);
				double y1 = static_cast<double>(sourceY);
				double x2 = static_cast<double>(getPosX());
				double y2 = static_cast<double>(getPosY());
				double r1 = static_cast<double>(215);
				double r2 = static_cast<double>(getRadius() + 10);
				double d = static_cast<double>(distance);

				// Formulae for calculating one point of intersection of two circles
				float root = sqrt((((r1 + r2) * (r1 + r2)) - (d * d)) * ((d * d) - ((r2 - r1) * (r2 - r1))));
				int x = (int)(((x1 + x2) / 2) + ((x2 - x1) * (r1 * r1 - r2 * r2)) / (2 * d * d) + ((y2 - y1) / (2 * d * d)) * root);
				int y = (int)(((y1 + y2) / 2) + ((y2 - y1) * (r1 * r1 - r2 * r2)) / (2 * d * d) - ((x2 - x1) / (2 * d * d)) * root);

				targetPos->x = x;
				targetPos->y = y;
			} else {
				ratio = 1 - (getRadius() / static_cast<float>(distance - 20));
				targetPos->x = (int16)(sourceX + ratio * (getPosX() - sourceX));
				targetPos->y = (int16)(sourceY + ratio * (getPosY() - sourceY));
			}

			if (distance < getRadius()) {
				targetPos->x = getPosX();
				targetPos->y = getPosY();
			}

			break;

		default:
			targetPos->x = getPosX();
			targetPos->y = getPosY();
			break;
		}
	}

	return targetPos;
}

int ShieldUnit::selectWeapon(int index) {
	debugC(DEBUG_MOONBASE_AI, "Shield weapon select");

	int myUnit = _ai->getClosestUnit(getPosX(), getPosY(), _ai->getMaxX(), _ai->getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 0);
	int dist = _ai->getDistance(getPosX(), getPosY(), _ai->getHubX(myUnit), _ai->getHubY(myUnit));

	if ((dist < (getRadius() - 20)) && (dist > 90)) {
		return ITEM_SPIKE;
	}

	switch (index) {
	case 0:
		if (getState() == DUS_OFF)  {
			if (_ai->getPlayerEnergy() < 3) {
				return ITEM_BOMB;
			} else {
				return ITEM_SPIKE;
			}
		}

		return ITEM_EMP;
		break;

	case 1:
		if (dist < getRadius() + 150) {
			return ITEM_EMP;
		} else {
			return ITEM_CRAWLER;
		}

		break;

	default:
		return ITEM_EMP;
		break;
	}
}

Common::Point *MineUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	float ratio;
	Common::Point *targetPos = new Common::Point;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_EMP:
		ratio = 1 - (getRadius() / static_cast<float>(distance - 20));
		targetPos->x = (int16)(sourceX + ratio * (getPosX() - sourceX));
		targetPos->y = (int16)(sourceY + ratio * (getPosY() - sourceY));
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int MineUnit::selectWeapon(int index) {
	int myUnit = _ai->getClosestUnit(getPosX(), getPosY(), _ai->getMaxX(), _ai->getCurrentPlayer(), 1, 0, 0, 0);
	int x = getPosX();
	int y = getPosY();

	int dist = _ai->getDistance(x, y, _ai->getHubX(myUnit), _ai->getHubY(myUnit));

	if ((getState() == DUS_ON) && (dist < 110)) {
		return ITEM_EMP;
	} else {
		return ITEM_BOMB;
	}
}


Common::Point *HubUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CRAWLER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int HubUnit::selectWeapon(int index) {
	debugC(DEBUG_MOONBASE_AI, "Hub weapon select");

	int energy = _ai->getPlayerEnergy();

	if (energy > 6) {
		//possibly choose crawler
		if (_ai->getBuildingWorth(getID()) > 21) {
			return ITEM_CRAWLER;
		}
	}

	//choose betw/ bomb and cluster
	if (_ai->getBuildingArmor(getID()) < 1.5) {
		return ITEM_CLUSTER;
	}

	if (energy > 2) {
		if (!_ai->_vm->_rnd.getRandomNumber(3)) {
			return ITEM_SPIKE;
		}

		if (!_ai->_vm->_rnd.getRandomNumber(4)) {
			return ITEM_GUIDED;
		}

		if (!_ai->_vm->_rnd.getRandomNumber(4)) {
			return ITEM_MINE;
		}

		if (!_ai->_vm->_rnd.getRandomNumber(9)) {
			return ITEM_EMP;
		}
	}

	return ITEM_BOMB;
}


Common::Point *TowerUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_SPIKE:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int TowerUnit::selectWeapon(int index) {
	switch (index) {
	case 0:
		return ITEM_SPIKE;
		break;

	default:
		return ITEM_SPIKE;
		break;
	}
}


Common::Point *BridgeUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int BridgeUnit::selectWeapon(int index) {
	switch (index) {
	case 0:
		return ITEM_BOMB;
		break;

	case 1:
		return ITEM_CLUSTER;
		break;

	default:
		return ITEM_BOMB;
		break;
	}
}

Common::Point *EnergyUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CRAWLER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int EnergyUnit::selectWeapon(int index) {
	debugC(DEBUG_MOONBASE_AI, "Energy weapon select");

	int energy = _ai->getPlayerEnergy();

	if (energy > 6) {
		//possibly choose crawler
		if (_ai->getBuildingWorth(getID()) > 21) {
			return ITEM_CRAWLER;
		}
	}

	//choose betw/ bomb and cluster
	if (_ai->getBuildingArmor(getID()) < 1.5) {
		return ITEM_CLUSTER;
	}

	if (energy > 2) {
		if (!_ai->_vm->_rnd.getRandomNumber(3)) {
			return ITEM_EMP;
		}
	}

	return ITEM_BOMB;
}

Common::Point *OffenseUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance) distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CRAWLER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int OffenseUnit::selectWeapon(int index) {
	debugC(DEBUG_MOONBASE_AI, "Offense weapon select");

	int energy = _ai->getPlayerEnergy();

	if (energy > 6) {
		//possibly choose crawler
		if (_ai->getBuildingWorth(getID()) > 21) {
			return ITEM_CRAWLER;
		}
	}

	//choose betw/ bomb and cluster
	if (_ai->getBuildingArmor(getID()) < 1.5) {
		return ITEM_CLUSTER;
	}

	return ITEM_BOMB;
}

Common::Point *CrawlerUnit::createTargetPos(int index, int distance, int weaponType, int sourceX, int sourceY) {
	Common::Point *targetPos = new Common::Point;

	if (!distance)
		distance = 1;

	switch (weaponType) {
	case ITEM_BOMB:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CLUSTER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	case ITEM_CRAWLER:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;

	default:
		targetPos->x = getPosX();
		targetPos->y = getPosY();
		break;
	}

	return targetPos;
}

int CrawlerUnit::selectWeapon(int index) {
	debugC(DEBUG_MOONBASE_AI, "Crawler weapon select");
	int myUnit = _ai->getClosestUnit(getPosX(), getPosY(), _ai->getMaxX(), _ai->getCurrentPlayer(), 1, 0, 0, 0);
	int dist = _ai->getDistance(_ai->getHubX(myUnit), _ai->getHubY(myUnit), getPosX(), getPosY());

	int x = getPosX();
	int y = getPosY();
	int energy = _ai->getPlayerEnergy();
	int terrain = _ai->getTerrain(x, y);

	if (terrain != TERRAIN_TYPE_WATER) {
		if ((energy > 2) && (dist < 220)) {
			return ITEM_RECLAIMER;
		} else {
			return ITEM_BOMB;
		}
	} else {
		if (energy > 6) {
			return ITEM_CRAWLER;
		}

		if (energy > 2) {
			if (_ai->_vm->_rnd.getRandomNumber(1)) {
				return ITEM_MINE;
			} else {
				return ITEM_TIME_EXPIRED;
			}
		}
	}

	return SKIP_TURN;
}

AntiAirUnit::AntiAirUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(190);
	setArmor(3);
	setCost(1);
}

ShieldUnit::ShieldUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(170);
	setArmor(3);
	setCost(7);
}

MineUnit::MineUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(80);
	setArmor(1);
	setCost(3);
}

HubUnit::HubUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(5);
	setCost(7);
}

TowerUnit::TowerUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(3);
	setCost(1);
}

BridgeUnit::BridgeUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(3);
	setCost(1);
}

EnergyUnit::EnergyUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(5);
	setCost(7);
}

OffenseUnit::OffenseUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(3);
	setCost(7);
}

CrawlerUnit::CrawlerUnit(AI *ai) : DefenseUnit(ai) {
	setRadius(1);
	setArmor(3);
	setCost(7);
}

AntiAirUnit::AntiAirUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());

}

ShieldUnit::ShieldUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

MineUnit::MineUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

HubUnit::HubUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

TowerUnit::TowerUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

BridgeUnit::BridgeUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

EnergyUnit::EnergyUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

OffenseUnit::OffenseUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

CrawlerUnit::CrawlerUnit(DefenseUnit *inUnit, AI *ai) : DefenseUnit(inUnit, ai) {
	setID(inUnit->getID());
	setPos(inUnit->getPosX(), inUnit->getPosY());
	setDistanceTo(inUnit->getDistanceTo());
	setState(inUnit->getState());
	setRadius(inUnit->getRadius());
	setArmor(inUnit->getArmor());
}

} // End of namespace Scumm
