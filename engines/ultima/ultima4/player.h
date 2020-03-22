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

#ifndef ULTIMA4_PLAYER_H
#define ULTIMA4_PLAYER_H

#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/observable.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/script.h"
#include "ultima/ultima4/graphics/tile.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

class Armor;
class Party;
class Weapon;

using Common::String;

typedef Std::vector<class PartyMember *> PartyMemberVector;

#define ALL_PLAYERS -1

enum KarmaAction {
	KA_FOUND_ITEM,
	KA_STOLE_CHEST,
	KA_GAVE_TO_BEGGAR,
	KA_GAVE_ALL_TO_BEGGAR,
	KA_BRAGGED,
	KA_HUMBLE,
	KA_HAWKWIND,
	KA_MEDITATION,
	KA_BAD_MANTRA,
	KA_ATTACKED_GOOD,
	KA_FLED_EVIL,
	KA_FLED_GOOD,
	KA_HEALTHY_FLED_EVIL,
	KA_KILLED_EVIL,
	KA_SPARED_GOOD,
	KA_DONATED_BLOOD,
	KA_DIDNT_DONATE_BLOOD,
	KA_CHEAT_REAGENTS,
	KA_DIDNT_CHEAT_REAGENTS,
	KA_USED_SKULL,
	KA_DESTROYED_SKULL
};

enum HealType {
	HT_NONE,
	HT_CURE,
	HT_FULLHEAL,
	HT_RESURRECT,
	HT_HEAL,
	HT_CAMPHEAL,
	HT_INNHEAL
};

enum InventoryItem {
	INV_NONE,
	INV_WEAPON,
	INV_ARMOR,
	INV_FOOD,
	INV_REAGENT,
	INV_GUILDITEM,
	INV_HORSE
};

enum CannotJoinError {
	JOIN_SUCCEEDED,
	JOIN_NOT_EXPERIENCED,
	JOIN_NOT_VIRTUOUS
};

enum EquipError {
	EQUIP_SUCCEEDED,
	EQUIP_NONE_LEFT,
	EQUIP_CLASS_RESTRICTED
};

/**
 * PartyMember class
 */
class PartyMember : public Creature, public Script::Provider {
public:
	PartyMember(Party *p, SaveGamePlayerRecord *pr);
	virtual ~PartyMember();

	void notifyOfChange();

	// Used to translate script values into something useful
	virtual Common::String translate(Std::vector<Common::String> &parts);

	// Accessor methods
	virtual int getHp() const;
	int getMaxHp() const   {
		return _player->_hpMax;
	}
	int getExp() const     {
		return _player->_xp;
	}
	int getStr() const     {
		return _player->_str;
	}
	int getDex() const     {
		return _player->_dex;
	}
	int getInt() const     {
		return _player->_intel;
	}
	int getMp() const      {
		return _player->_mp;
	}
	int getMaxMp() const;
	const Weapon *getWeapon() const;
	const Armor *getArmor() const;
	virtual Common::String getName() const;
	SexType getSex() const;
	ClassType getClass() const;
	virtual CreatureStatus getState() const;
	int getRealLevel() const;
	int getMaxLevel() const;

	virtual void addStatus(StatusType status);
	void adjustMp(int pts);
	void advanceLevel();
	void applyEffect(TileEffect effect);
	void awardXp(int xp);
	bool heal(HealType type);
	virtual void removeStatus(StatusType status);
	virtual void setHp(int hp);
	void setMp(int mp);
	EquipError setArmor(const Armor *a);
	EquipError setWeapon(const Weapon *w);

	virtual bool applyDamage(int damage, bool byplayer = false);
	virtual int getAttackBonus() const;
	virtual int getDefense() const;
	virtual bool dealDamage(Creature *m, int damage);
	int getDamage();
	virtual const Common::String &getHitTile() const;
	virtual const Common::String &getMissTile() const;
	bool isDead();
	bool isDisabled();
	int  loseWeapon();
	virtual void putToSleep();
	virtual void wakeUp();

protected:
	static MapTile tileForClass(int klass);

	SaveGamePlayerRecord *_player;
	class Party *_party;
};

/**
 * Party class
 */
class PartyEvent {
public:
	enum Type {
		GENERIC,
		LOST_EIGHTH,
		ADVANCED_LEVEL,
		STARVING,
		TRANSPORT_CHANGED,
		PLAYER_KILLED,
		ACTIVE_PLAYER_CHANGED,
		MEMBER_JOINED,
		PARTY_REVIVED,
		INVENTORY_ADDED,
	};

	PartyEvent(Type type, PartyMember *partyMember) : _type(type), _player(partyMember) { }

	Type _type;
	PartyMember *_player;
};

typedef Std::vector<PartyMember *> PartyMemberVector;

class Party : public Observable<Party *, PartyEvent &>, public Script::Provider {
	friend class PartyMember;
public:
	Party(SaveGame *saveGame);
	virtual ~Party();

	void notifyOfChange(PartyMember *partyMember = 0, PartyEvent::Type = PartyEvent::GENERIC);

	// Used to translate script values into something useful
	virtual Common::String translate(Std::vector<Common::String> &parts);

	void adjustFood(int food);
	void adjustGold(int gold);
	void adjustKarma(KarmaAction action);
	void applyEffect(TileEffect effect);
	bool attemptElevation(Virtue virtue);
	void burnTorch(int turns = 1);
	bool canEnterShrine(Virtue virtue);
	bool canPersonJoin(Common::String name, Virtue *v);
	void damageShip(unsigned int pts);
	bool donate(int quantity);
	void endTurn();
	int  getChest();
	int  getTorchDuration() const;
	void healShip(unsigned int pts);
	bool isFlying() const;
	bool isImmobilized();
	bool isDead();
	bool isPersonJoined(Common::String name);
	CannotJoinError join(Common::String name);
	bool lightTorch(int duration = 100, bool loseTorch = true);
	void quenchTorch();
	void reviveParty();
	MapTile getTransport() const;
	void setTransport(MapTile transport);
	void setShipHull(int str);

	Direction getDirection() const;
	void setDirection(Direction dir);

	void adjustReagent(int reagent, int amt);
	int getReagent(int reagent) const;
	short *getReagentPtr(int reagent) const;

	void setActivePlayer(int p);
	int getActivePlayer() const;

	void swapPlayers(int p1, int p2);

	int size() const;
	PartyMember *member(int index) const;

private:
	void syncMembers();
	PartyMemberVector _members;
	SaveGame *_saveGame;
	MapTile _transport;
	int _torchDuration;
	int _activePlayer;
#ifdef IOS
	friend void U4IOS::syncPartyMembersWithSaveGame();
#endif
};

bool isPartyMember(Object *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
