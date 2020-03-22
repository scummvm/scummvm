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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/spell.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/dungeon.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/moongate.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/graphics/tile.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

SpellEffectCallback spellEffectCallback = NULL;

CombatController *spellCombatController();
void spellMagicAttack(const Common::String &tilename, Direction dir, int minDamage, int maxDamage);
bool spellMagicAttackAt(const Coords &coords, MapTile attackTile, int attackDamage);

static int spellAwaken(int player);
static int spellBlink(int dir);
static int spellCure(int player);
static int spellDispel(int dir);
static int spellEField(int param);
static int spellFireball(int dir);
static int spellGate(int phase);
static int spellHeal(int player);
static int spellIceball(int dir);
static int spellJinx(int unused);
static int spellKill(int dir);
static int spellLight(int unused);
static int spellMMissle(int dir);
static int spellNegate(int unused);
static int spellOpen(int unused);
static int spellProtect(int unused);
static int spellRez(int player);
static int spellQuick(int unused);
static int spellSleep(int unused);
static int spellTremor(int unused);
static int spellUndead(int unused);
static int spellView(int unsued);
static int spellWinds(int fromdir);
static int spellXit(int unused);
static int spellYup(int unused);
static int spellZdown(int unused);

/* masks for reagents */
#define ASH (1 << REAG_ASH)
#define GINSENG (1 << REAG_GINSENG)
#define GARLIC (1 << REAG_GARLIC)
#define SILK (1 << REAG_SILK)
#define MOSS (1 << REAG_MOSS)
#define PEARL (1 << REAG_PEARL)
#define NIGHTSHADE (1 << REAG_NIGHTSHADE)
#define MANDRAKE (1 << REAG_MANDRAKE)

/* spell error messages */
static const struct {
    SpellCastError err;
    const char *msg;
} spellErrorMsgs[] = {
    { CASTERR_NOMIX, "None Mixed!\n" },        
    { CASTERR_MPTOOLOW, "Not Enough MP!\n" },
    { CASTERR_FAILED, "Failed!\n" },
    { CASTERR_WRONGCONTEXT, "Not here!\n" },
    { CASTERR_COMBATONLY, "Combat only!\nFailed!\n" },
    { CASTERR_DUNGEONONLY, "Dungeon only!\nFailed!\n" },
    { CASTERR_WORLDMAPONLY, "Outdoors only!\nFailed!\n" }
};

static const Spell spells[] = {
    { "Awaken",       GINSENG | GARLIC,         CTX_ANY,        TRANSPORT_ANY,  &spellAwaken,  Spell::PARAM_PLAYER,  5 },
    { "Blink",        SILK | MOSS,              CTX_WORLDMAP,   TRANSPORT_FOOT_OR_HORSE,
                                                                                &spellBlink,   Spell::PARAM_DIR,     15 },
    { "Cure",         GINSENG | GARLIC,         CTX_ANY,        TRANSPORT_ANY,  &spellCure,    Spell::PARAM_PLAYER,  5 },
    { "Dispell",      ASH | GARLIC | PEARL,     CTX_ANY,        TRANSPORT_ANY,  &spellDispel,  Spell::PARAM_DIR,     20 },
    { "Energy Field", ASH | SILK | PEARL,       (LocationContext)(CTX_COMBAT | CTX_DUNGEON),
                                                                TRANSPORT_ANY,  &spellEField,  Spell::PARAM_TYPEDIR, 10 },
    { "Fireball",     ASH | PEARL,              CTX_COMBAT,     TRANSPORT_ANY,  &spellFireball,Spell::PARAM_DIR,     15 },
    { "Gate",         ASH | PEARL | MANDRAKE,   CTX_WORLDMAP,   TRANSPORT_FOOT_OR_HORSE,
                                                                                &spellGate,    Spell::PARAM_PHASE,   40 },
    { "Heal",         GINSENG | SILK,           CTX_ANY,        TRANSPORT_ANY,  &spellHeal,    Spell::PARAM_PLAYER,  10 },
    { "Iceball",      PEARL | MANDRAKE,         CTX_COMBAT,     TRANSPORT_ANY,  &spellIceball, Spell::PARAM_DIR,     20 },
    { "Jinx",         PEARL | NIGHTSHADE | MANDRAKE,
                                                CTX_ANY,        TRANSPORT_ANY,  &spellJinx,    Spell::PARAM_NONE,    30 },
    { "Kill",         PEARL | NIGHTSHADE,       CTX_COMBAT,     TRANSPORT_ANY,  &spellKill,    Spell::PARAM_DIR,     25 },
    { "Light",        ASH,                      CTX_DUNGEON,    TRANSPORT_ANY,  &spellLight,   Spell::PARAM_NONE,    5 },
    { "Magic missile", ASH | PEARL,             CTX_COMBAT,     TRANSPORT_ANY,  &spellMMissle, Spell::PARAM_DIR,     5 },
    { "Negate",       ASH | GARLIC | MANDRAKE,  CTX_ANY,        TRANSPORT_ANY,  &spellNegate,  Spell::PARAM_NONE,    20 },
    { "Open",         ASH | MOSS,               CTX_ANY,        TRANSPORT_ANY,  &spellOpen,    Spell::PARAM_NONE,    5 },
    { "Protection",   ASH | GINSENG | GARLIC,   CTX_ANY,        TRANSPORT_ANY,  &spellProtect, Spell::PARAM_NONE,    15 },
    { "Quickness",    ASH | GINSENG | MOSS,     CTX_ANY,        TRANSPORT_ANY,  &spellQuick,   Spell::PARAM_NONE,    20 },
    { "Resurrect",    ASH | GINSENG | GARLIC | SILK | MOSS | MANDRAKE, 
                                                CTX_NON_COMBAT, TRANSPORT_ANY,  &spellRez,     Spell::PARAM_PLAYER,  45 },
    { "Sleep",        SILK | GINSENG,           CTX_COMBAT,     TRANSPORT_ANY,  &spellSleep,   Spell::PARAM_NONE,    15 },
    { "Tremor",       ASH | MOSS | MANDRAKE,    CTX_COMBAT,     TRANSPORT_ANY,  &spellTremor,  Spell::PARAM_NONE,    30 },
    { "Undead",       ASH | GARLIC,             CTX_COMBAT,     TRANSPORT_ANY,  &spellUndead,  Spell::PARAM_NONE,    15 },
    { "View",         NIGHTSHADE | MANDRAKE,    CTX_NON_COMBAT, TRANSPORT_ANY,  &spellView,    Spell::PARAM_NONE,    15 },
    { "Winds",        ASH | MOSS,               CTX_WORLDMAP,   TRANSPORT_ANY,  &spellWinds,   Spell::PARAM_FROMDIR, 10 },
    { "X-it",         ASH | SILK | MOSS,        CTX_DUNGEON,    TRANSPORT_ANY,  &spellXit,     Spell::PARAM_NONE,    15 },
    { "Y-up",         SILK | MOSS,              CTX_DUNGEON,    TRANSPORT_ANY,  &spellYup,     Spell::PARAM_NONE,    10 },
    { "Z-down",       SILK | MOSS,              CTX_DUNGEON,    TRANSPORT_ANY,  &spellZdown,   Spell::PARAM_NONE,    5 }
};

#define N_SPELLS (sizeof(spells) / sizeof(spells[0]))

void spellSetEffectCallback(SpellEffectCallback callback) {
    spellEffectCallback = callback;    
}

Ingredients::Ingredients() {
    memset(_reagents, 0, sizeof(_reagents));
}

bool Ingredients::addReagent(Reagent reagent) {
    ASSERT(reagent < REAG_MAX, "invalid reagent: %d", reagent);    
    if (g_context->_party->getReagent(reagent) < 1)
        return false;
    g_context->_party->adjustReagent(reagent, -1);    
    _reagents[reagent]++;
    return true;
}

bool Ingredients::removeReagent(Reagent reagent) {
    ASSERT(reagent < REAG_MAX, "invalid reagent: %d", reagent);
    if (_reagents[reagent] == 0)
        return false;
    g_context->_party->adjustReagent(reagent, 1);    
    _reagents[reagent]--;
    return true;
}

int Ingredients::getReagent(Reagent reagent) const {
    ASSERT(reagent < REAG_MAX, "invalid reagent: %d", reagent);
    return _reagents[reagent];
}

void Ingredients::revert() {
    int reg;

    for (reg = 0; reg < REAG_MAX; reg++) {
        g_context->_saveGame->_reagents[reg] += _reagents[reg];
        _reagents[reg] = 0;
    }
}

bool Ingredients::checkMultiple(int batches) const {
    for (int i = 0; i < REAG_MAX; i++) {
        /* see if there's enough reagents to mix (-1 because one is already counted) */
        if (_reagents[i] > 0 && g_context->_saveGame->_reagents[i] < batches - 1) {
            return false;
        }
    }    
    return true;
}

void Ingredients::multiply(int batches) {
    ASSERT(checkMultiple(batches), "not enough reagents to multiply ingredients by %d\n", batches);
    for (int i = 0; i < REAG_MAX; i++) {
        if (_reagents[i] > 0) {
            g_context->_saveGame->_reagents[i] -= batches - 1;
            _reagents[i] += batches - 1;
        }
    }
}

const char *spellGetName(unsigned int spell) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);

    return spells[spell]._name;
}

int spellGetRequiredMP(unsigned int spell) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);
    
    return spells[spell]._mp;
}

LocationContext spellGetContext(unsigned int spell) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);

    return spells[spell]._context;
}

TransportContext spellGetTransportContext(unsigned int spell) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);

    return spells[spell]._transportContext;
}

Common::String spellGetErrorMessage(unsigned int spell, SpellCastError error) {
    unsigned int i;
    SpellCastError err = error;

    /* try to find a more specific error message */
    if (err == CASTERR_WRONGCONTEXT) {
        switch(spells[spell]._context) {
            case CTX_COMBAT: err = CASTERR_COMBATONLY; break;
            case CTX_DUNGEON: err = CASTERR_DUNGEONONLY; break;
            case CTX_WORLDMAP: err = CASTERR_WORLDMAPONLY; break;
            default: break;
        }
    }

    /* find the message that we're looking for and return it! */
    for (i = 0; i < sizeof(spellErrorMsgs) / sizeof(spellErrorMsgs[0]); i++) {
        if (err == spellErrorMsgs[i].err)
            return Common::String(spellErrorMsgs[i].msg);
    }

    return Common::String();
}

/**
 * Mix reagents for a spell.  Fails and returns false if the reagents
 * selected were not correct.
 */
int spellMix(unsigned int spell, const Ingredients *ingredients) {
    int regmask, reg;

    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);

    regmask = 0;
    for (reg = 0; reg < REAG_MAX; reg++) {
        if (ingredients->getReagent((Reagent) reg) > 0)
            regmask |= (1 << reg);
    }

    if (regmask != spells[spell]._components)
        return 0;

    g_context->_saveGame->_mixtures[spell]++;

    return 1;
}

Spell::Param spellGetParamType(unsigned int spell) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);

    return spells[spell]._paramType;
}

/**
 * Checks some basic prerequistes for casting a spell.  Returns an
 * error if no mixture is available, the context is invalid, or the
 * character doesn't have enough magic points.
 */
SpellCastError spellCheckPrerequisites(unsigned int spell, int character) {
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);
    ASSERT(character >= 0 && character < g_context->_saveGame->_members, "character out of range: %d", character);

    if (g_context->_saveGame->_mixtures[spell] == 0)
        return CASTERR_NOMIX;

    if ((g_context->_location->_context & spells[spell]._context) == 0)
        return CASTERR_WRONGCONTEXT;        

    if ((g_context->_transportContext & spells[spell]._transportContext) == 0)
        return CASTERR_FAILED;

    if (g_context->_party->member(character)->getMp() < spells[spell]._mp)
        return CASTERR_MPTOOLOW;

    return CASTERR_NOERROR;
}

/**
 * Casts spell.  Fails and returns false if the spell cannot be cast.
 * The error code is updated with the reason for failure.
 */
bool spellCast(unsigned int spell, int character, int param, SpellCastError *error, bool spellEffect) {
    int subject = (spells[spell]._paramType == Spell::PARAM_PLAYER) ? param : -1;
    PartyMember *p = g_context->_party->member(character);
    
    ASSERT(spell < N_SPELLS, "invalid spell: %d", spell);
    ASSERT(character >= 0 && character < g_context->_saveGame->_members, "character out of range: %d", character);

    *error = spellCheckPrerequisites(spell, character);

    // subtract the mixture for even trying to cast the spell
    AdjustValueMin(g_context->_saveGame->_mixtures[spell], -1, 0);
        
    if (*error != CASTERR_NOERROR)
        return false;

    // If there's a negate magic aura, spells fail!
    if (*g_context->_aura == Aura::NEGATE) {
        *error = CASTERR_FAILED;
        return false;
    }

    // subtract the mp needed for the spell
    p->adjustMp(-spells[spell]._mp);

    if (spellEffect) {
		int time;
		/* recalculate spell speed - based on 5/sec */
		float MP_OF_LARGEST_SPELL = 45;
		int spellMp = spells[spell]._mp;
		time = int(10000.0 / settings._spellEffectSpeed  *  spellMp / MP_OF_LARGEST_SPELL);
		soundPlay(SOUND_PREMAGIC_MANA_JUMBLE, false, time);
		EventHandler::wait_msecs(time);

        (*spellEffectCallback)(spell + 'a', subject, SOUND_MAGIC);
    }
    
    if (!(*spells[spell]._spellFunc)(param)) {
        *error = CASTERR_FAILED;
        return false;
    }

    return true;
}

CombatController *spellCombatController() {
    CombatController *cc = dynamic_cast<CombatController *>(eventHandler->getController());
    return cc;
}

/**
 * Makes a special magic ranged attack in the given direction
 */
void spellMagicAttack(const Common::String &tilename, Direction dir, int minDamage, int maxDamage) {    
    CombatController *controller = spellCombatController();
    PartyMemberVector *party = controller->getParty();

    MapTile tile = g_context->_location->_map->_tileset->getByName(tilename)->getId();

    int attackDamage = ((minDamage >= 0) && (minDamage < maxDamage)) ?
        xu4_random((maxDamage + 1) - minDamage) + minDamage :
        maxDamage;

    Std::vector<Coords> path = gameGetDirectionalActionPath(MASK_DIR(dir), MASK_DIR_ALL, (*party)[controller->getFocus()]->getCoords(), 
                                                       1, 11, Tile::canAttackOverTile, false);
    for (Std::vector<Coords>::iterator i = path.begin(); i != path.end(); i++) {
        if (spellMagicAttackAt(*i, tile, attackDamage))
            return;
    }
}

bool spellMagicAttackAt(const Coords &coords, MapTile attackTile, int attackDamage) {
    bool objectHit = false;
//    int attackdelay = MAX_BATTLE_SPEED - settings.battleSpeed;    
    CombatMap *cm = getCombatMap();
    
    Creature *creature = cm->creatureAt(coords);

    if (!creature) {
    	GameController::flashTile(coords, attackTile,2);
    }
    else {
        objectHit = true;

        /* show the 'hit' tile */
        soundPlay(SOUND_NPC_STRUCK);
        GameController::flashTile(coords, attackTile, 3);


        /* apply the damage to the creature */
        CombatController *controller = spellCombatController();
        controller->getCurrentPlayer()->dealDamage(creature, attackDamage);
        GameController::flashTile(coords, attackTile, 1);
    }

    return objectHit;
}

static int spellAwaken(int player) {
    ASSERT(player < 8, "player out of range: %d", player);
    PartyMember *p = g_context->_party->member(player);

    if ((player < g_context->_party->size()) && (p->getStatus() == STAT_SLEEPING)) {
        p->wakeUp();
        return 1;
    }

    return 0;
}

static int spellBlink(int dir) {
    int i,        
        failed = 0,
        distance,
        diff,
        *var;
    Direction reverseDir = dirReverse((Direction)dir);
    MapCoords coords = g_context->_location->_coords;
    
    /* Blink doesn't work near the mouth of the abyss */
    /* Note: This means you can teleport to Hythloth from the top of the map,
       and that you can teleport to the abyss from the left edge of the map,
       Unfortunately, this matches the bugs in the game. :(  Consider fixing. */
    if (coords.x >= 192 && coords.y >= 192)
        return 0;

    /* figure out what numbers we're working with */
    var = (dir & (DIR_WEST | DIR_EAST)) ? &coords.x : &coords.y;
        
    /* find the distance we are going to move */
    distance = (*var) % 0x10;
    if (dir == DIR_EAST || dir == DIR_SOUTH)
        distance = 0x10 - distance;
    
    /* see if we move another 16 spaces over */
    diff = 0x10 - distance;
    if ((diff > 0) && (xu4_random(diff * diff) > distance))
        distance += 0x10;

    /* test our distance, and see if it works */
    for (i = 0; i < distance; i++)
        coords.move((Direction)dir, g_context->_location->_map);    
    
    i = distance;   
    /* begin walking backward until you find a valid spot */
    while ((i-- > 0) && !g_context->_location->_map->tileTypeAt(coords, WITH_OBJECTS)->isWalkable())
        coords.move(reverseDir, g_context->_location->_map);
    
    if (g_context->_location->_map->tileTypeAt(coords, WITH_OBJECTS)->isWalkable()) {
        /* we didn't move! */
        if (g_context->_location->_coords == coords)
            failed = 1;

        g_context->_location->_coords = coords;
    } else failed = 1;    

    return (failed ? 0 : 1);
}

static int spellCure(int player) {
    ASSERT(player < 8, "player out of range: %d", player);

    GameController::flashTile(g_context->_party->member(player)->getCoords(), "wisp", 1);
    return g_context->_party->member(player)->heal(HT_CURE);
}

static int spellDispel(int dir) {    
    MapTile *tile; 
    MapCoords field;

    /* 
     * get the location of the avatar (or current party member, if in battle)
     */
    g_context->_location->getCurrentPosition(&field);        

    /*
     * find where we want to dispel the field
     */
    field.move((Direction)dir, g_context->_location->_map);    

    GameController::flashTile(field, "wisp", 2);
    /*
     * if there is a field annotation, remove it and replace it with a valid
     * replacement annotation.  We do this because sometimes dungeon triggers
     * create annotations, that, if just removed, leave a wall tile behind
     * (or other unwalkable surface).  So, we need to provide a valid replacement
     * annotation to fill in the gap :)
     */
    Annotation::List a = g_context->_location->_map->_annotations->allAt(field);
    if (a.size() > 0) {
        Annotation::List::iterator i;
        for (i = a.begin(); i != a.end(); i++) {            
            if (i->getTile().getTileType()->canDispel()) {

                /*
                 * get a replacement tile for the field
                 */
                MapTile newTile(g_context->_location->getReplacementTile(field, i->getTile().getTileType()));

                g_context->_location->_map->_annotations->remove(*i);
                g_context->_location->_map->_annotations->add(field, newTile, false, true);
                return 1;
            }                
        }
    }    

    /*
     * if the map tile itself is a field, overlay it with a replacement tile
     */

    tile = g_context->_location->_map->tileAt(field, WITHOUT_OBJECTS);    
    if (!tile->getTileType()->canDispel())
        return 0;

    /*
     * get a replacement tile for the field
     */
    MapTile newTile(g_context->_location->getReplacementTile(field, tile->getTileType()));
    
    g_context->_location->_map->_annotations->add(field, newTile, false, true);

    return 1;
}

static int spellEField(int param) {    
    MapTile fieldTile(0);
    int fieldType;
    int dir;
    MapCoords coords;
    
    /* Unpack fieldType and direction */
    fieldType = param >> 4;
    dir = param & 0xF;
    
    /* Make sure params valid */
    switch (fieldType) {
        case ENERGYFIELD_FIRE: fieldTile = g_context->_location->_map->_tileset->getByName("fire_field")->getId(); break;
        case ENERGYFIELD_LIGHTNING: fieldTile = g_context->_location->_map->_tileset->getByName("energy_field")->getId(); break;
        case ENERGYFIELD_POISON: fieldTile = g_context->_location->_map->_tileset->getByName("poison_field")->getId(); break;
        case ENERGYFIELD_SLEEP: fieldTile = g_context->_location->_map->_tileset->getByName("sleep_field")->getId(); break;
        default: return 0; break;
    }

    g_context->_location->getCurrentPosition(&coords);        
    
    coords.move((Direction)dir, g_context->_location->_map);    
    if (MAP_IS_OOB(g_context->_location->_map, coords))
        return 0;
    else {
        /*
         * Observed behaviour on Amiga version of Ultima IV:
         * Field cast on other field: Works, unless original field is lightning
         * in which case it doesn't.
         * Field cast on creature: Works, creature remains the visible tile
         * Field cast on top of field and then dispel = no fields left
         * The code below seems to produce this behaviour.
         */
        const Tile *tile = g_context->_location->_map->tileTypeAt(coords, WITH_GROUND_OBJECTS);
        if (!tile->isWalkable()) return 0;
        
        /* Get rid of old field, if any */
        Annotation::List a = g_context->_location->_map->_annotations->allAt(coords);
        if (a.size() > 0) {
            Annotation::List::iterator i;
            for (i = a.begin(); i != a.end(); i++) {                
                if (i->getTile().getTileType()->canDispel())
                    g_context->_location->_map->_annotations->remove(*i);
            }
        }     
            
        g_context->_location->_map->_annotations->add(coords, fieldTile);
    }

    return 1;
}

static int spellFireball(int dir) {
    spellMagicAttack("hit_flash", (Direction)dir, 24, 128);    
    return 1;
}

static int spellGate(int phase) {
    const Coords *moongate;

    GameController::flashTile(g_context->_location->_coords, "moongate", 2);

    moongate = moongateGetGateCoordsForPhase(phase);
    if (moongate) 
        g_context->_location->_coords = *moongate;

    return 1;    
}

static int spellHeal(int player) {
    ASSERT(player < 8, "player out of range: %d", player);

    GameController::flashTile(g_context->_party->member(player)->getCoords(), "wisp", 1);
    g_context->_party->member(player)->heal(HT_HEAL);
    return 1;
}

static int spellIceball(int dir) {
    spellMagicAttack("magic_flash", (Direction)dir, 32, 224);    
    return 1;
}

static int spellJinx(int unused) {
    g_context->_aura->set(Aura::JINX, 10);    
    return 1;
}

static int spellKill(int dir) {
    spellMagicAttack("whirlpool", (Direction)dir, -1, 232);
    return 1;
}

static int spellLight(int unused) {
    g_context->_party->lightTorch(100, false);
    return 1;
}

static int spellMMissle(int dir) {
    spellMagicAttack("miss_flash", (Direction)dir, 64, 16);
    return 1;
}

static int spellNegate(int unused) {
    g_context->_aura->set(Aura::NEGATE, 10);    
    return 1;
}

static int spellOpen(int unused) {    
    getChest(-2);   // HACK: -2 will not prompt for opener
    return 1;
}

static int spellProtect(int unused) {
    g_context->_aura->set(Aura::PROTECTION, 10);    
    return 1;
}

static int spellRez(int player) {
    ASSERT(player < 8, "player out of range: %d", player);

    return g_context->_party->member(player)->heal(HT_RESURRECT);
}

static int spellQuick(int unused) {
    g_context->_aura->set(Aura::QUICKNESS, 10);    
    return 1;
}

static int spellSleep(int unused) {    
    CombatMap *cm = getCombatMap();
    CreatureVector creatures = cm->getCreatures();
    CreatureVector::iterator i;

    /* try to put each creature to sleep */

    for (i = creatures.begin(); i != creatures.end(); i++) {         
        Creature *m = *i;
        Coords coords = m->getCoords();
        GameController::flashTile(coords, "wisp", 1);
        if ((m->getResists() != EFFECT_SLEEP) &&
            xu4_random(0xFF) >= m->getHp())
        {
        	soundPlay(SOUND_POISON_EFFECT);
            m->putToSleep();
            GameController::flashTile(coords, "sleep_field", 3);
        }
        else
        	soundPlay(SOUND_EVADE);
    }

    return 1;
}

static int spellTremor(int unused) {
    CombatController *ct = spellCombatController();    
    CreatureVector creatures = ct->getMap()->getCreatures();
    CreatureVector::iterator i;

    for (i = creatures.begin(); i != creatures.end(); i++) {
        Creature *m = *i;


        Coords coords = m->getCoords();
        //GameController::flashTile(coords, "rocks", 1);

        /* creatures with over 192 hp are unaffected */
        if (m->getHp() > 192) {
            soundPlay(SOUND_EVADE);
            continue;
        }
        else {
            /* Deal maximum damage to creature */
            if (xu4_random(2) == 0) {
                soundPlay(SOUND_NPC_STRUCK);
                GameController::flashTile(coords, "hit_flash", 3);
                ct->getCurrentPlayer()->dealDamage(m, 0xFF);
            }
            /* Deal enough damage to creature to make it flee */
            else if (xu4_random(2) == 0) {
                soundPlay(SOUND_NPC_STRUCK);
                GameController::flashTile(coords, "hit_flash", 2);
                if (m->getHp() > 23)
                    ct->getCurrentPlayer()->dealDamage(m, m->getHp()-23);
            }
            else
            {
                soundPlay(SOUND_EVADE);
            }
        }
    }
    
    return 1;
}

static int spellUndead(int unused) {    
    CombatController *ct = spellCombatController();
    CreatureVector creatures = ct->getMap()->getCreatures();
    CreatureVector::iterator i;

    for (i = creatures.begin(); i != creatures.end(); i++) {         
        Creature *m = *i;
        if (m && m->isUndead() && xu4_random(2) == 0)
            m->setHp(23);
    }
    
    return 1;
}

static int spellView(int unsued) {    
    peer(false);
    return 1;
}

static int spellWinds(int fromdir) {
    g_context->_windDirection = fromdir;
    return 1;
}

static int spellXit(int unused) {
    if (!g_context->_location->_map->isWorldMap()) {
        screenMessage("Leaving...\n");
        g_game->exitToParentMap();
        musicMgr->play();
        return 1;
    }
    return 0;
}

static int spellYup(int unused) {
    MapCoords coords = g_context->_location->_coords;
    Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);

    /* can't cast in the Abyss */
    if (g_context->_location->_map->_id == MAP_ABYSS)
        return 0;
    /* staying in the dungeon */
    else if (coords.z > 0) {
        for (int i = 0; i < 0x20; i++) {
            coords = MapCoords(xu4_random(8), xu4_random(8), g_context->_location->_coords.z - 1);
            if (dungeon->validTeleportLocation(coords)) {
                g_context->_location->_coords = coords;
                return 1;
            }
        }
    /* exiting the dungeon */
    } else {
        screenMessage("Leaving...\n");
        g_game->exitToParentMap();
        musicMgr->play();
        return 1;
    }
    
    /* didn't find a place to go, failed! */
    return 0;
}

static int spellZdown(int unused) {
    MapCoords coords = g_context->_location->_coords;
    Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
    
    /* can't cast in the Abyss */
    if (g_context->_location->_map->_id == MAP_ABYSS)
        return 0;
    /* can't go lower than level 8 */
    else if (coords.z >= 7)
        return 0;
    else {
        for (int i = 0; i < 0x20; i++) {
            coords = MapCoords(xu4_random(8), xu4_random(8), g_context->_location->_coords.z + 1);
            if (dungeon->validTeleportLocation(coords)) {
                g_context->_location->_coords = coords;
                return 1;
            }
        }
    }
    
    /* didn't find a place to go, failed! */
    return 0;
}

const Spell* getSpell(int i) {return &spells[i];}

} // End of namespace Ultima4
} // End of namespace Ultima
