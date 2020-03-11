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

#ifndef ULTIMA4_COMBAT_H
#define ULTIMA4_COMBAT_H

#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/controller.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/observer.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

#define AREA_CREATURES   16
#define AREA_PLAYERS    8

class CombatMap;
class Creature;
class MoveEvent;
class Weapon;

typedef enum {
    CA_ATTACK,
    CA_CAST_SLEEP,
    CA_ADVANCE,
    CA_RANGED,
    CA_FLEE,
    CA_TELEPORT    
} CombatAction;

/**
 * CombatController class
 */ 
class CombatController : public Controller, public Observer<Party *, PartyEvent &>, public TurnCompleter {
protected:
    CombatController();
public:
    CombatController(CombatMap *m);
    CombatController(MapId id);
    virtual ~CombatController();

    // Accessor Methods    
    bool          isCombatController() const { return true; }
    bool          isCamping() const;
    bool          isWinOrLose() const;
    Direction     getExitDir() const;
    unsigned char getFocus() const;
    CombatMap *   getMap() const;
    Creature *    getCreature() const;
    PartyMemberVector *getParty();
    PartyMember*  getCurrentPlayer();
    
    void setExitDir(Direction d);
    void setCreature(Creature *);
    void setWinOrLose(bool worl = true);
    void showCombatMessage(bool show = true);

    // Methods
    virtual void init(Creature *m);
    void initDungeonRoom(int room, Direction from);
    
    void applyCreatureTileEffects();
    virtual void begin();
    virtual void end(bool adjustKarma);
    void fillCreatureTable(const Creature *creature);
    int  initialNumberOfCreatures(const Creature *creature) const;
    bool isWon() const;
    bool isLost() const;
    void moveCreatures();
    void placeCreatures();
    void placePartyMembers();
    bool setActivePlayer(int player);
    bool attackHit(Creature *attacker, Creature *defender);
    virtual void awardLoot();

    // attack functions
    void attack();
    bool attackAt(const Coords &coords, PartyMember *attacker, int dir, int range, int distance);
    bool rangedAttack(const Coords &coords, Creature *attacker);
    void rangedMiss(const Coords &coords, Creature *attacker);
    bool returnWeaponToOwner(const Coords &coords, int distance, int dir, const Weapon *weapon);

    /** 
     * Static member functions
     */
    static void attackFlash(const Coords &coords, MapTile tile, int timeFactor);
    static void attackFlash(const Coords &coords, const Common::String &tilename, int timeFactor);
    static void doScreenAnimationsWhilePausing(int timeFactor);

    // Key handlers
    virtual bool keyPressed(int key);

    virtual void finishTurn();
    void movePartyMember(MoveEvent &event);
    virtual void update(Party *party, PartyEvent &event);

    // Properties
protected:
    CombatMap *map;
    
    PartyMemberVector party;
    unsigned char focus;

    const Creature *creatureTable[AREA_CREATURES];
    Creature *_creature;    

    bool camping;
    bool forceStandardEncounterSize;
    bool placePartyOnMap;
    bool placeCreaturesOnMap;
    bool winOrLose;
    bool showMessage;
    Direction exitDir;

private:
    CombatController(const CombatController&);
    const CombatController &operator=(const CombatController&);
};

/**
 * CombatMap class
 */
class CombatMap : public Map {
public:
    CombatMap();
        
    CreatureVector getCreatures();
    PartyMemberVector getPartyMembers();
    PartyMember* partyMemberAt(Coords coords);    
    Creature* creatureAt(Coords coords);    
    
    static MapId mapForTile(const Tile *ground, const Tile *transport, Object *obj);

    // Getters
    bool isDungeonRoom() const      {return dungeonRoom;}
    bool isAltarRoom() const        {return altarRoom != VIRT_NONE;}
    bool isContextual() const       {return contextual;}
    BaseVirtue getAltarRoom() const {return altarRoom;}

    // Setters
    void setAltarRoom(BaseVirtue ar){altarRoom = ar;}
    void setDungeonRoom(bool d)     {dungeonRoom = d;}
    void setContextual(bool c)      {contextual = c;}
    
    // Properties
protected:
    bool dungeonRoom;
    BaseVirtue altarRoom;
    bool contextual;

public:
    Coords creature_start[AREA_CREATURES];
    Coords player_start[AREA_PLAYERS];
};

bool isCombatMap(Map *punknown);
CombatMap *getCombatMap(Map *punknown = NULL);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
