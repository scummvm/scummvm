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

#ifndef WAGE_OBJ_H
#define WAGE_OBJ_H

#include "common/rect.h"

#include "wage/designed.h"

namespace Wage {

class Weapon {
public:
	virtual ~Weapon() {}
	virtual String getOperativeVerb() = 0;
	virtual int getType() = 0;
	virtual int getAccuracy() = 0;
	virtual int getDamage() = 0;
	virtual String getSound() = 0;
	virtual void decrementNumberOfUses() = 0;
};

class Design;
class Scene;
class Chr;

class Obj : public Weapon, public Designed {
public:
	Obj() : _currentOwner(NULL), _currentScene(NULL) {}

	enum ObjectTypes {
		REGULAR_WEAPON = 1,
		THROW_WEAPON = 2,
		MAGICAL_OBJECT = 3,
		HELMET = 4,
		SHIELD = 5,
		CHEST_ARMOR = 6,
		SPIRITUAL_ARMOR = 7,
		MOBILE_OBJECT = 8,
		IMMOBILE_OBJECT = 9
	};

	enum AttackTypes {
		CAUSES_PHYSICAL_DAMAGE = 0,
		CAUSES_SPIRITUAL_DAMAGE = 1,
		CAUSES_PHYSICAL_AND_SPIRITUAL_DAMAGE = 2,
		HEALS_PHYSICAL_DAMAGE = 3,
		HEALS_SPIRITUAL_DAMAGE = 4,
		HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE = 5,
		FREEZES_OPPONENT = 6
	};

private:
	int _index;
	bool _namePlural;
	int _type;
	int _value;
	int _damage;
	int _accuracy;
	int _attackType;
	int _numberOfUses;
	bool _returnToRandomScene;
	String _sceneOrOwner;
	String _clickMessage;
	String _operativeVerb;
	String _failureMessage;
	String _useMessage;
	String _sound;
	
	Scene *_currentScene;
	Chr *_currentOwner;

public:
	Chr *getCurrentOwner() { return _currentOwner; }

	void setCurrentOwner(Chr *currentOwner) {
		_currentOwner = currentOwner;
		if (currentOwner != NULL)
			_currentScene = NULL;
	}

	Scene *getCurrentScene() { return _currentScene; }

	void setCurrentScene(Scene *currentScene) {
		_currentScene = currentScene;
		if (currentScene != NULL)
			_currentOwner = NULL;
	}

	int getAccuracy() { return _accuracy; }
	void setAccuracy(int accuracy) { _accuracy = accuracy; }

	int getAttackType() { return _attackType; }
	void setAttackType(int attackType) { _attackType = attackType; }

	String getClickMessage() { return _clickMessage; }
	void setClickMessage(String clickMessage) { _clickMessage = clickMessage; }

	int getDamage() { return _damage; }
	void setDamage(int damage) { _damage = damage; }

	String getFailureMessage() { return _failureMessage; }
	void setFailureMessage(String failureMessage) { _failureMessage = failureMessage; }

	int getNumberOfUses() { return _numberOfUses; }
	void setNumberOfUses(int numberOfUses) { _numberOfUses = numberOfUses; }
	void decrementNumberOfUses() { 
		if (_numberOfUses != -1) {
			_numberOfUses--;
		}
	}

	int getType() { return _type; }
	void setType(int type) { _type = type; }

	String getOperativeVerb() { return _operativeVerb; }
	void setOperativeVerb(String operativeVerb) { _operativeVerb = operativeVerb; }

	bool isReturnToRandomScene() { return _returnToRandomScene; }
	void setReturnToRandomScene(bool returnToRandomScene) { _returnToRandomScene = returnToRandomScene; }

	String getSceneOrOwner() { return _sceneOrOwner; }
	void setSceneOrOwner(String sceneOrOwner) { _sceneOrOwner = sceneOrOwner; }

	String getSound() { return _sound; }
	void setSound(String sound) { _sound = sound; }

	String getUseMessage() { return _useMessage; }
	void setUseMessage(String useMessage) { _useMessage = useMessage; }

	int getValue() { return _value; }
	void setValue(int value) { _value = value; }

	bool isNamePlural() { return _namePlural; }
	void setNamePlural(bool namePlural) { _namePlural = namePlural; }

	int getIndex() { return _index; }
	void setIndex(int index) { _index = index; }
};

} // End of namespace Wage
 
#endif
