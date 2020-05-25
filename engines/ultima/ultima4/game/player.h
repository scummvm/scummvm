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

#ifndef ULTIMA4_GAME_PLAYER_H
#define ULTIMA4_GAME_PLAYER_H

#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/core/observable.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/script.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

class Armor;
class Party;
class Weapon;

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

	/**
	 * Notify the party that this player has changed somehow
	 */
	void notifyOfChange();

	/**
	 * Used to translate script values into something useful
	 */
	Common::String translate(Std::vector<Common::String> &parts) override;

	// Accessor methods
	int getHp() const override;
	int getMaxHp() const {
		return _player->_hpMax;
	}
	int getExp() const {
		return _player->_xp;
	}
	int getStr() const {
		return _player->_str;
	}
	int getDex() const {
		return _player->_dex;
	}
	int getInt() const {
		return _player->_intel;
	}
	int getMp() const {
		return _player->_mp;
	}

	/**
	 * Determine the most magic points a character could have
	 * given his class and intelligence.
	 */
	int getMaxMp() const;

	const Weapon *getWeapon() const;
	const Armor *getArmor() const;
	Common::String getName() const override;
	SexType getSex() const;
	ClassType getClass() const;
	CreatureStatus getState() const override;

	/**
	 * Determine what level a character has.
	 */
	int getRealLevel() const;

	/**
	 * Determine the highest level a character could have with the number
	 * of experience points he has.
	 */
	int getMaxLevel() const;

	/**
	 * Adds a status effect to the player
	 */
	void addStatus(StatusType status) override;

	/**
	 * Adjusts the player's mp by 'pts'
	 */
	void adjustMp(int pts);

	/**
	 * Advances the player to the next level if they have enough experience
	 */
	void advanceLevel();

	/**
	 * Apply an effect to the party member
	 */
	void applyEffect(TileEffect effect);

	/**
	 * Award a player experience points.  Maxs out the players xp at 9999.
	 */
	void awardXp(int xp);

	/**
	 * Perform a certain type of healing on the party member
	 */
	bool heal(HealType type);

	/**
	 * Remove status effects from the party member
	 */
	void removeStatus(StatusType status) override;

	void setHp(int hp) override;
	void setMp(int mp);
	EquipError setArmor(const Armor *a);
	EquipError setWeapon(const Weapon *w);

	/**
	 * Applies damage to a player, and changes status to dead if hit
	 * points drop below zero.
	 *
	 * Byplayer is ignored for now, since it should always be false for U4.  (Is
	 * there anything special about being killed by a party member in U5?)  Also
	 * keeps interface consistent for virtual base function Creature::applydamage()
	 */
	bool applyDamage(int damage, bool byplayer = false) override;
	int getAttackBonus() const override;
	int getDefense() const override;
	bool dealDamage(Creature *m, int damage) override;

	/**
	 * Calculate damage for an attack.
	 */
	int getDamage();

	/**
	 * Returns the tile that will be displayed when the party
	 * member's attack hits
	 */
	const Common::String &getHitTile() const override;

	/**
	 * Returns the tile that will be displayed when the party
	 * member's attack fails
	 */
	const Common::String &getMissTile() const override;
	bool isDead();
	bool isDisabled();

	/**
	 * Lose the equipped weapon for the player (flaming oil, ranged daggers, etc.)
	 * Returns the number of weapons left of that type, including the one in
	 * the players hand
	 */
	int  loseWeapon();

	/**
	 * Put the party member to sleep
	 */
	virtual void putToSleep() override;

	/**
	 * Wakes up the party member
	 */
	void wakeUp() override;

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
		INVENTORY_ADDED
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

	/**
	 * Notify the party that something about it has changed
	 */
	void notifyOfChange(PartyMember *partyMember = 0, PartyEvent::Type = PartyEvent::GENERIC);

	// Used to translate script values into something useful
	Common::String translate(Std::vector<Common::String> &parts) override;

	void adjustFood(int food);
	void adjustGold(int gold);

	/**
	 * Adjusts the avatar's karma level for the given action.  Notify
	 * observers with a lost eighth event if the player has lost
	 * avatarhood.
	 */
	void adjustKarma(KarmaAction action);

	/**
	 * Apply effects to the entire party
	 */
	void applyEffect(TileEffect effect);

	/**
	 * Attempt to elevate in the given virtue
	 */
	bool attemptElevation(Virtue virtue);

	/**
	 * Burns a torch's duration down a certain number of turns
	 */
	void burnTorch(int turns = 1);

	/**
	 * Returns true if the party can enter the shrine
	 */
	bool canEnterShrine(Virtue virtue);

	/**
	 * Returns true if the person can join the party
	 */
	bool canPersonJoin(Common::String name, Virtue *v);

	/**
	 * Damages the party's ship
	 */
	void damageShip(uint pts);

	/**
	 * Donates 'quantity' gold. Returns true if the donation succeeded,
	 * or false if there was not enough gold to make the donation
	 */
	bool donate(int quantity);

	/**
	 * Ends the party's turn
	 */
	void endTurn();

	/**
	 * Adds a chest worth of gold to the party's inventory
	 */
	int  getChest();

	/**
	 * Returns the number of turns a currently lit torch will last (or 0 if no torch lit)
	 */
	int  getTorchDuration() const;

	/**
	 * Heals the ship's hull strength by 'pts' points
	 */
	void healShip(uint pts);

	/**
	 * Returns true if the balloon is currently in the air
	 */
	bool isFlying() const;

	/**
	 * Whether or not the party can make an action.
	 */
	bool isImmobilized();

	/**
	 * Whether or not all the party members are dead.
	 */
	bool isDead();

	/**
	 * Returns true if the person with that name
	 * is already in the party
	 */
	bool isPersonJoined(Common::String name);

	/**
	 * Attempts to add the person to the party.
	 * Returns JOIN_SUCCEEDED if successful.
	 */
	CannotJoinError join(Common::String name);

	/**
	 * Lights a torch with a default duration of 100
	 */
	bool lightTorch(int duration = 100, bool loseTorch = true);

	/**
	 * Extinguishes a torch
	 */
	void quenchTorch();

	/**
	 * Revives the party after the entire party has been killed
	 */
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

	/**
	 * Returns the size of the party
	 */
	int size() const;

	/**
	 * Returns a pointer to the party member indicated
	 */
	PartyMember *member(int index) const;

private:
	void syncMembers();
	PartyMemberVector _members;
	SaveGame *_saveGame;
	MapTile _transport;
	int _torchDuration;
	int _activePlayer;
#ifdef IOS_ULTIMA4
	friend void U4IOS::syncPartyMembersWithSaveGame();
#endif
};

bool isPartyMember(Object *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
