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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_ENTITIES_H
#define WAGE_ENTITIES_H

namespace Graphics {
	class ManagedSurface;
	class MacFont;
}

namespace Wage {

class Design;
class Script;

enum StatVariable {
/** The base physical accuracy of the player. */
	PHYS_ACC_BAS = 0,
/** The current physical accuracy of the player. */
	PHYS_ACC_CUR = 1,
/** The base physical armor of the player. */
	PHYS_ARM_BAS = 2,
/** The current physical armor of the player. */
	PHYS_ARM_CUR = 3,
/** The base physical hit points of the player. */
	PHYS_HIT_BAS = 4,
/** The current physical hit points of the player. */
	PHYS_HIT_CUR = 5,
/** The base physical speed of the player. */
	PHYS_SPE_BAS = 6,
/** The current physical speed of the player. */
	PHYS_SPE_CUR = 7,
/** The base physical strength of the player. */
	PHYS_STR_BAS = 8,
/** The current physical strength of the player. */
	PHYS_STR_CUR = 9,
/** The base spiritual accuracy of the player. */
	SPIR_ACC_BAS = 10,
/** The current spiritual accuracy of the player. */
	SPIR_ACC_CUR = 11,
/** The base spiritual armor of the player. */
	SPIR_ARM_BAS = 12,
/** The current spiritual armor of the player. */
	SPIR_ARM_CUR = 13,
/** The base spiritual hit points of the player. */
	SPIR_HIT_BAS = 14,
/** The current spiritual hit points of the player. */
	SPIR_HIT_CUR = 15,
/** The base spiritual strength of the player. */
	SPIR_STR_BAS = 16,
/** The current spiritual strength of the player. */
	SPIR_STR_CUR = 17
};

class Context {
public:
	Context();

	int16 _visits; // Number of scenes visited, including repeated visits
	int16 _kills;  // Number of characters killed
	int16 _experience;
	bool _frozen;
	int16 _userVariables[26 * 9];
	int16 _statVariables[18];
};

class Designed {
public:
	Designed() : _design(NULL), _designBounds(NULL), _classType(UNKNOWN) {}
	~Designed();

	Common::String _name;
	Design *_design;
	Common::Rect *_designBounds;
	OperandType _classType;

	Common::Rect *getDesignBounds() {
		return _designBounds == NULL ? NULL : new Common::Rect(*_designBounds);
	}

	void setDesignBounds(Common::Rect *bounds);

	Common::String toString() const { return _name; }
};

class Chr : public Designed {
public:
	enum ChrDestination {
		RETURN_TO_STORAGE = 0,
		RETURN_TO_RANDOM_SCENE = 1,
		RETURN_TO_INITIAL_SCENE = 2
	};

	enum ChrPart {
		HEAD = 0,
		CHEST = 1,
		SIDE = 2
	};

	enum ChrArmorType {
		HEAD_ARMOR = 0,
		BODY_ARMOR = 1,
		SHIELD_ARMOR = 2,
		MAGIC_ARMOR = 3,
		NUMBER_OF_ARMOR_TYPES = 4
	};

	Chr(Common::String name, Common::SeekableReadStream *data);
	~Chr();

	int _index;
	int _resourceId;
	Common::String _initialScene;
	int _gender;
	bool _nameProperNoun;
	bool _playerCharacter;
	uint _maximumCarriedObjects;
	int _returnTo;

	int _physicalStrength;
	int _physicalHp;
	int _naturalArmor;
	int _physicalAccuracy;
	int _spiritualStength;
	int _spiritialHp;
	int _resistanceToMagic;
	int _spiritualAccuracy;
	int _runningSpeed;
	uint _rejectsOffers;
	int _followsOpponent;

	Common::String _initialSound;
	Common::String _scoresHitSound;
	Common::String _receivesHitSound;
	Common::String _dyingSound;

	Common::String _nativeWeapon1;
	Common::String _operativeVerb1;
	int _weaponDamage1;
	Common::String _weaponSound1;

	Common::String _nativeWeapon2;
	Common::String _operativeVerb2;
	int _weaponDamage2;
	Common::String _weaponSound2;

	int _winningWeapons;
	int _winningMagic;
	int _winningRun;
	int _winningOffer;
	int _losingWeapons;
	int _losingMagic;
	int _losingRun;
	int _losingOffer;

	Common::String _initialComment;
	Common::String _scoresHitComment;
	Common::String _receivesHitComment;
	Common::String _makesOfferComment;
	Common::String _rejectsOfferComment;
	Common::String _acceptsOfferComment;
	Common::String _dyingWords;

	Scene *_currentScene;
	ObjArray _inventory;

	Obj *_armor[NUMBER_OF_ARMOR_TYPES];

	Context _context;

	ObjArray *getWeapons(bool includeMagic);
	ObjArray *getMagicalObjects();
	const char *getDefiniteArticle(bool capitalize);

	Obj *_weapon1;
	Obj *_weapon2;

public:
	int wearObjIfPossible(Obj *obj);
	void wearObjs();

	void resetState();

	bool isWearing(Obj *obj);
};

class Obj : public Designed {
public:
	Obj();
	Obj(Common::String name, Common::SeekableReadStream *data, int resourceId);
	~Obj();

	enum ObjectType {
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

	enum AttackType {
		CAUSES_PHYSICAL_DAMAGE = 0,
		CAUSES_SPIRITUAL_DAMAGE = 1,
		CAUSES_PHYSICAL_AND_SPIRITUAL_DAMAGE = 2,
		HEALS_PHYSICAL_DAMAGE = 3,
		HEALS_SPIRITUAL_DAMAGE = 4,
		HEALS_PHYSICAL_AND_SPIRITUAL_DAMAGE = 5,
		FREEZES_OPPONENT = 6
	};

public:
	int _index;
	int _resourceId;
	bool _namePlural;
	uint _value;
	int _attackType;
	int _numberOfUses;
	bool _returnToRandomScene;
	Common::String _sceneOrOwner;
	Common::String _clickMessage;
	Common::String _failureMessage;
	Common::String _useMessage;

	Scene *_currentScene;
	Chr *_currentOwner;

	int _type;
	uint _accuracy;
	Common::String _operativeVerb;
	int _damage;
	Common::String _sound;

public:
	void setCurrentOwner(Chr *currentOwner) {
		_currentOwner = currentOwner;
		if (currentOwner != NULL)
			_currentScene = NULL;
	}

	void setCurrentScene(Scene *currentScene) {
		_currentScene = currentScene;
		if (currentScene != NULL)
			_currentOwner = NULL;
	}

	Chr *removeFromChr();
	Designed *removeFromCharOrScene();

	void resetState(Chr *owner, Scene *scene);
};

class Scene : public Designed {
public:
	enum SceneTypes {
		PERIODIC = 0,
		RANDOM = 1
	};

	int _resourceId;

	Script *_script;
	Common::String _text;
	Common::Rect *_textBounds;
	Graphics::MacFont *_font;
	bool _blocked[4];
	Common::String _messages[4];
	int _soundFrequency; // times a minute, max 3600
	int _soundType;
	Common::String _soundName;
	int _worldX;
	int _worldY;
	bool _visited;

	ObjList _objs;
	ChrList _chrs;

	Scene();
	Scene(Common::String name, Common::SeekableReadStream *data);
	~Scene();

	Designed *lookUpEntity(int x, int y);

	Common::Rect *getTextBounds() {
		return _textBounds == NULL ? NULL : new Common::Rect(*_textBounds);
	}

	void paint(Graphics::ManagedSurface *screen, int x, int y);

	const Graphics::MacFont *getFont() { return _font; }
};

} // End of namespace Wage

#endif
