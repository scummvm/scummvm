/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/speldefs.h"
#include "saga2/spellbuk.h"
#include "saga2/spelshow.h"
#include "saga2/spelvals.h"
#include "saga2/tilevect.h"
#include "saga2/tileline.h"

namespace Saga2 {

extern WorldMapData                     *mapList;
extern SpellStuff                       *spellBook;
extern PlatformHandle   platformList;       // platform resource hunk

/* ===================================================================== *
   Locals
 * ===================================================================== */

static int16        prevMapNum;
static StaticTilePoint prevCoords = {(int16)minint16, (int16)minint16, (int16)minint16};
static MetaTilePtr  prevMeta;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

blockageType checkNontact(Effectron *obj, const TilePoint &loc, GameObject **blockResultObj = nullptr);
int32 scatterer(int32 i, int32 m, int32 s);

/* ===================================================================== *
   SpellStuff implementation
 * ===================================================================== */

//-----------------------------------------------------------------------
//	ctor

SpellStuff::SpellStuff() {
	_master = kNullSpell;
	_display = kNullSpell;
	_prototype = nullptr;
	_targetableTypes = kSpellTargNone;
	_targetTypes = kSpellApplyNone;
	_effects = nullptr;
	_targets = nullptr;
	_manaType = ksManaIDSkill;
	_manaUse = 0;
	_shape = keAreaInvisible;
	_size = 0;
	_range = 0;
	_sound = 0;

	_debug = false;
}

//-----------------------------------------------------------------------
//	is this spell harmful

bool SpellStuff::isOffensive() {
	return (canTarget(kSpellTargActor) || canTarget(kSpellTargObject)) &&
	       (!canTarget(kSpellTargCaster));
}

//-----------------------------------------------------------------------
//	determine whether an area spell protects the caster

bool SpellStuff::safe() {
	switch (_shape) {
	case keAreaInvisible:
	case keAreaAura:
	case keAreaGlow:
	case keAreaProjectile:
	case keAreaExchange:
	case keAreaMissle:
	case keAreaSquare:
	case keAreaBall:
	case keAreaWall:
	case keAreaStorm:
		return false;
	case keAreaBolt:
	case keAreaBeam:
	case keAreaCone:
	case keAreaWave:
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------
//	add an internal effect to a spell

void SpellStuff::addEffect(ProtoEffect *pe) {
	if (_effects == nullptr)
		_effects = pe;
	else {
		ProtoEffect *_tail;
		for (_tail = _effects; _tail->_next; _tail = _tail->_next) ;
		_tail->_next = pe;
	}
}

//-----------------------------------------------------------------------
//	play the sound associated with a spell

void SpellStuff::playSound(GameObject *go) {
	if (_sound) {
		Location cal = go->notGetWorldLocation(); //Location(go->getLocation(),go->IDParent());
		Saga2::playSoundAt(MKTAG('S', 'P', 'L', _sound), cal);
	}
}

//-----------------------------------------------------------------------
//	cleanup

void SpellStuff::killEffects() {
	if (_effects) {
		delete _effects;
	}
	_effects = nullptr;
}

//-----------------------------------------------------------------------
//	implement a spell - general target type

void SpellStuff::implement(GameObject *enactor, SpellTarget *target) {
	assert(target);
	switch (target->getType()) {
	case SpellTarget::kSpellTargetPoint:
		implement(enactor, Location(target->getPoint(), Nothing));
		break;
	case SpellTarget::kSpellTargetObjectPoint:
		if (_targetTypes == kSpellApplyObject)
			implement(enactor, target->getObject());
		else
			implement(enactor, Location(target->getPoint(), Nothing));
		break;
	case SpellTarget::kSpellTargetObject:
		implement(enactor, target->getObject());
		break;
	case SpellTarget::kSpellTargetTAG:
		implement(enactor, target->getTAG());
		break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------
//	implement a spell - object target type

void SpellStuff::implement(GameObject *enactor, GameObject *target) {
	SpellTarget st = SpellTarget(target);
	if (safe() &&
	        target->thisID() == enactor->thisID() &&
	        !canTarget(kSpellTargCaster))
		return;
	if (_effects) {
		for (ProtoEffect *pe = _effects; pe; pe = pe->_next)
			if (pe->applicable(st))
				pe->implement(enactor, &st);
	}
}

//-----------------------------------------------------------------------
//	implement a spell - TAG target type

void SpellStuff::implement(GameObject *enactor, ActiveItem *target) {
	SpellTarget st = SpellTarget(target);
	if (_effects) {
		for (ProtoEffect *pe = _effects; pe; pe = pe->_next)
			if (pe->applicable(st))
				pe->implement(enactor, &st);
	}
}

//-----------------------------------------------------------------------
//	implement a spell - area target type

void SpellStuff::implement(GameObject *enactor, Location target) {
	SpellTarget st = SpellTarget(target);
	buildTargetList(enactor, st);
	if (_effects && _targets) {
		for (SpellTarget *t = _targets; t; t = t->_next) {
			if (safe() &&
			        t->getObject() != nullptr &&
			        t->getObject()->thisID() == enactor->thisID() &&
			        !canTarget(kSpellTargCaster))
				continue;
			for (ProtoEffect *pe = _effects; pe; pe = pe->_next)
				if (pe->applicable(*t))
					pe->implement(enactor, t);
		}
	}
	removeTargetList();
}

//-----------------------------------------------------------------------
//	determine target list for a spell

void SpellStuff::buildTargetList(GameObject *caster, SpellTarget &trg) {
	int16 radius = _size;
	TilePoint tVect, orth, tBase;
	show(caster, trg);
	switch (_shape) {
	case keAreaInvisible:
	case keAreaAura:
	case keAreaGlow:
	case keAreaProjectile:
	case keAreaExchange:
	case keAreaMissle:
		_targets = &trg;
		break;
	case keAreaSquare: {
		tVect = trg.getPoint();
		orth = TilePoint(kSquareSpellSize / 2, kSquareSpellSize / 2, 0);

		RectangularObjectIterator   it(currentWorld,
		                               tVect - orth,
		                               TilePoint(kSquareSpellSize, 0, 0),
		                               TilePoint(0, kSquareSpellSize, 0));
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}

	case keAreaBolt: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kBoltSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kBoltSpellWidth / 2);
		tBase = caster->getWorldLocation() + (isActor(caster) ? (tVect / 32) : TilePoint(0, 0, 0));
		RectangularObjectIterator   it(currentWorld, tBase - orth, tVect, orth * 2);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}

	case keAreaBeam: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kBeamSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kBeamSpellWidth / 2);
		tBase = caster->getWorldLocation() + (isActor(caster) ? (tVect / 32) : TilePoint(0, 0, 0));
		RectangularObjectIterator   it(currentWorld, tBase - orth, tVect, orth * 2);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}

	case keAreaBall: {
		radius = kBallSpellRadius;
		CircularObjectIterator  it(currentWorld, trg.getPoint(), radius);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}
	case keAreaWall: {
		radius = kWallSpellRadius;
		RingObjectIterator  it(currentWorld, trg.getPoint(), radius, kWallInnerRadius);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}
	case keAreaStorm: {
		radius = kStormSpellRadius;
		CircularObjectIterator  it(currentWorld, trg.getPoint(), radius);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}
	case keAreaCone: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kConeSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kConeSpellWidth / 2);
		tBase = caster->getWorldLocation() + (isActor(caster) ? (tVect / 32) : TilePoint(0, 0, 0));
		TriangularObjectIterator    it(currentWorld, tBase, tBase + tVect - orth, tBase + tVect + orth);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}
	case keAreaWave: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kWaveSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kWaveSpellWidth / 2);
		tBase = caster->getWorldLocation() + (isActor(caster) ? (tVect / 32) : TilePoint(0, 0, 0));
		TriangularObjectIterator    it(currentWorld, tBase, tBase + tVect - orth, tBase + tVect + orth);
		GameObject *go;
		it.first(&go);
		while (go) {
			addTarget(new SpellTarget(go));
			it.next(&go);
		}
		break;
	}
	}

}

//-----------------------------------------------------------------------
//	add a target to the target list

void SpellStuff::addTarget(SpellTarget *trg) {
	if (_targets == nullptr)
		_targets = trg;
	else {
		SpellTarget *t = _targets;
		while (t->_next) t = t->_next;
		t->_next = trg;
	}
}

//-----------------------------------------------------------------------
//	clean the target list

void SpellStuff::removeTargetList() {
	switch (_shape) {
	case keAreaInvisible:
	case keAreaAura:
	case keAreaGlow:
	case keAreaProjectile:
	case keAreaExchange:
	case keAreaMissle:
		_targets = nullptr;
		break;
	case keAreaWall:
	case keAreaCone:
	case keAreaBeam:
	case keAreaBolt:
	case keAreaBall:
	case keAreaStorm:
	case keAreaSquare:
		if (_targets) delete _targets;
		_targets = nullptr;
		break;
	default:
		error("bad spell");
	}
	assert(_targets == nullptr);
}

//-----------------------------------------------------------------------
//	spell debugging

#define DSPELL_AREA_COLOR 3
#define DSPELL_TARGET_COLOR 7

void showTarg(const TilePoint &tp) {
	TPCircle(tp, 8, DSPELL_TARGET_COLOR);
}

void SpellStuff::show(GameObject *caster, SpellTarget &trg) {
	if (!_debug) return;

	int16 radius = _size;
	TilePoint tVect, orth, tBase;
	switch (_shape) {
	case keAreaInvisible:
		showTarg(trg.getPoint());
		break;
	case keAreaAura:
	case keAreaGlow:
	case keAreaProjectile:
	case keAreaExchange:
	case keAreaMissle:
		TPLine(caster->getWorldLocation(), trg.getPoint(), DSPELL_AREA_COLOR);
		showTarg(trg.getPoint());
		break;
	case keAreaSquare: {
		tVect = trg.getPoint();
		orth = TilePoint(kSquareSpellSize / 2, kSquareSpellSize / 2, 0);

		TPRectangle(tVect - orth,
		            tBase + TilePoint(kSquareSpellSize, 0, 0),
		            tBase + TilePoint(0, kSquareSpellSize, 0) + TilePoint(kSquareSpellSize, 0, 0),
		            tBase + TilePoint(0, kSquareSpellSize, 0),
		            DSPELL_AREA_COLOR);
		RectangularObjectIterator   it(currentWorld,
		                               tVect - orth,
		                               TilePoint(kSquareSpellSize, 0, 0),
		                               TilePoint(0, kSquareSpellSize, 0));
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}

	case keAreaBolt: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kBoltSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kBoltSpellWidth / 2);
		tBase = caster->getWorldLocation();
		TPRectangle(tBase - orth, tBase + orth, tBase + tVect + orth, tBase + tVect - orth, DSPELL_AREA_COLOR);
		RectangularObjectIterator   it(currentWorld, tBase - orth, tVect, orth * 2);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}

	case keAreaBeam: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kBeamSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kBeamSpellWidth / 2);
		tBase = caster->getWorldLocation();
		TPRectangle(tBase - orth, tBase + orth, tBase + tVect + orth, tBase + tVect - orth, DSPELL_AREA_COLOR);
		RectangularObjectIterator   it(currentWorld, tBase - orth, tVect, orth * 2);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}

	case keAreaBall: {
		radius = kBallSpellRadius;
		TPCircle(trg.getPoint(), kBallSpellRadius, DSPELL_AREA_COLOR);
		CircularObjectIterator  it(currentWorld, trg.getPoint(), radius);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}
	case keAreaWall: {
		radius = kWallSpellRadius;
		TPCircle(trg.getPoint(), radius, DSPELL_AREA_COLOR);
		TPCircle(trg.getPoint(), radius / 2, DSPELL_AREA_COLOR);
		RingObjectIterator  it(currentWorld, trg.getPoint(), radius, kWallInnerRadius);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}
	case keAreaStorm: {
		radius = kStormSpellRadius;
		TPCircle(trg.getPoint(), kStormSpellRadius, DSPELL_AREA_COLOR);
		CircularObjectIterator  it(currentWorld, trg.getPoint(), radius);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}
	case keAreaCone: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kConeSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kConeSpellWidth / 2);
		tBase = caster->getWorldLocation();
		TPTriangle(tBase, tBase + tVect - orth, tBase + tVect + orth, DSPELL_AREA_COLOR);
		TriangularObjectIterator    it(currentWorld, tBase, tBase + tVect - orth, tBase + tVect + orth);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}
	case keAreaWave: {
		tVect = trg.getPoint() - caster->getWorldLocation();
		while (tVect.magnitude() == 0) {
			tVect = randomVector(TilePoint(-1, -1, 0), TilePoint(1, 1, 0));
		}
		setMagnitude(tVect, kWaveSpellLength);
		orth = rightVector(tVect, 0);
		setMagnitude(orth, kWaveSpellWidth / 2);
		tBase = caster->getWorldLocation();
		TPTriangle(tBase, tBase + tVect - orth, tBase + tVect + orth, DSPELL_AREA_COLOR);
		TriangularObjectIterator    it(currentWorld, tBase, tBase + tVect - orth, tBase + tVect + orth);
		GameObject *go;
		it.first(&go);
		while (go) {
			showTarg(go->getWorldLocation());
			it.next(&go);
		}
		break;
	}
	}
}

/* ===================================================================== *
   SpellInstance implementation
 * ===================================================================== */

// ------------------------------------------------------------------
// ctor

SpellInstance::SpellInstance(SpellCaster *newCaster, SpellTarget *newTarget, SpellID spellNo) {
	assert(newCaster);
	assert(newTarget);
	_caster = newCaster;
	_target = new SpellTarget(*newTarget);
	_world = newCaster->world();
	_spell = spellNo;
	init();
}

// ------------------------------------------------------------------
// ctor

SpellInstance::SpellInstance(SpellCaster *newCaster, GameObject &newTarget, SpellID spellNo) {
	assert(newCaster);
	_target = new SpellTarget(newTarget);
	_caster = newCaster;
	_world = newCaster->world();
	_spell = spellNo;
	init();
}

// ------------------------------------------------------------------
// ctor

SpellInstance::SpellInstance(SpellCaster *newCaster, GameObject *newTarget, SpellID spellNo) {
	assert(newCaster);
	assert(newTarget);
	_target = new SpellTarget(newTarget);
	_caster = newCaster;
	_world = newCaster->world();
	_spell = spellNo;
	init();
}

// ------------------------------------------------------------------
// ctor

SpellInstance::SpellInstance(SpellCaster *newCaster, TilePoint &newTarget, SpellID spellNo) {
	assert(newCaster);
	_target = new SpellTarget(newTarget);
	_caster = newCaster;
	_world = newCaster->world();
	_spell = spellNo;
	init();
}

// ------------------------------------------------------------------
// dtor


SpellInstance::~SpellInstance() {
	if (_age < _implementAge && g_vm->_gameRunning)
		spellBook[_spell].implement(_caster, _target);
	for (int32 i = 0; i < _eList._count; i++) {
		if (_eList._displayList[i]._efx)
			delete _eList._displayList[i]._efx;
		_eList._displayList[i]._efx = nullptr;
	}
	if (_target)
		delete _target;
	_target = nullptr;
}

// ------------------------------------------------------------------
// common initialization code

void SpellInstance::init() {
	_dProto = (*g_vm->_sdpList)[_spell];
	ProtoObj        *proto = _caster->proto();
	TilePoint       sPoint = _caster->getWorldLocation();
	sPoint.z += proto->height / 2;
	_age = 0;
	_implementAge = 0;
	_effSeq = 0;

	assert(_dProto);
	if (!_dProto)
		return;

	_effect = (*g_vm->_edpList)[_dProto->_effect];
	_implementAge = _dProto->_implementAge;
	_maxAge = _dProto->_maxAge;
	initEffect(sPoint);

	if (_implementAge == 0)
		spellBook[_spell].implement(_caster, _target);

}

// ------------------------------------------------------------------
// common cleanup

void SpellInstance::termEffect() {
	if (_eList._count)
		for (int32 i = 0; i < _eList._count; i++) {
			if (_eList._displayList[i]._efx) {
				delete _eList._displayList[i]._efx;
				_eList._displayList[i]._efx = nullptr;
			}
		}
}

// ------------------------------------------------------------------
// visual init

void SpellInstance::initEffect(TilePoint startpoint) {
	_eList._count = _effect->_nodeCount; //sdp->effCount;
	if (_eList._count)
		for (int32 i = 0; i < _eList._count; i++) {
			Effectron *e = new Effectron(0, i);
			_eList._displayList[i]._efx = e;
			e->_parent = this;
			e->_start = startpoint;
			e->_current = startpoint;
			e->_partno = i;
			e->_stepNo = 0;
			e->initCall(i);
		}
}

// ------------------------------------------------------------------
// visual update

bool SpellInstance::buildList() {
	if (_eList.dissipated()) {
		termEffect();
		if (_effect->_next == nullptr)
			return false;
		_effect = _effect->_next;
		_effSeq++;
		//
		initEffect(_target->getPoint());
	}
	_eList.buildEffects(false);
	return true;
}

// ------------------------------------------------------------------
// visual update

bool SpellInstance::updateStates(int32 deltaTime) {

	spellBook[_spell].show(_caster, *_target);
	_age++;
	if (_age == _implementAge || _implementAge == continuouslyImplemented)
		spellBook[_spell].implement(_caster, _target);
	if (_maxAge > 0 && _age > _maxAge)
		termEffect();
	_eList.updateEStates(deltaTime);
	return true;
}


/* ===================================================================== *
   General spell routines
 * ===================================================================== */

//-----------------------------------------------------------------------
// effectron sctterer

int32 scatterer(int32 i, int32 m, int32 s) {
	switch (s) {
	case 0:
		return 0;
	case 1:
		return i % m;
	case 2:
	case 3:
	case 4:
		return (i * s) % m;
	case 5:
	default:
		return g_vm->_rnd->getRandomNumber(m - 1);
	}
}

//-----------------------------------------------------------------------
//	point to point collision detection


TilePoint collideTo(Effectron *e, TilePoint nloc) {
	GameObject *bumpy;
	blockageType bt = checkNontact(e, nloc, &bumpy);

	if (bt == kBlockageTerrain) {
		e->bump();
	}

	return nloc;
}

/* ===================================================================== *
   Effectron implementation
 * ===================================================================== */

//-----------------------------------------------------------------------
// default constructor

Effectron::Effectron() {
	_age = 0;
	_pos = 0;
	_flags = kEffectronDead;
	_parent = nullptr;
	_partno = 0;
	_totalSteps = _stepNo = 0;
	_hgt = 0;
	_brd = 0;
	_spr = 0;
}

//-----------------------------------------------------------------------
// constructor with starting position / velocity

Effectron::Effectron(uint16 newPos, uint16 newDir) {
	_age = 0;
	_pos = (newDir << 16) + newPos;
	_flags = 0;
	_parent = nullptr;
	_partno = 0;
	_totalSteps = _stepNo = 0;
	_hgt = 0;
	_brd = 0;
	_spr = 0;
}

//-----------------------------------------------------------------------
// Effectron display update

void Effectron::updateEffect(int32 deltaTime) {
	_age += deltaTime;
	if (_age > 1) {
		_age = 0;
		_pos++;
		_finish = _parent->_target->getPoint();
		_stepNo++;

		_flags = staCall();
		if (isHidden() || isDead())
			return;
		_spr = sprCall();
		_hgt = hgtCall();
		_brd = brdCall();
		TilePoint oLoc = posCall();

		// at this point we need to detect collisions
		// between current ( or start ) and oLoc

		_current = collideTo(this, oLoc);
		TileToScreenCoords(oLoc, _screenCoords);
	}

}

//-----------------------------------------------------------------------
void Effectron::bump() {
	switch (_parent->_dProto->_elasticity) {
	case kEcFlagBounce:
		_velocity = -_velocity;
		break;
	case kEcFlagDie:
		kill();
		break;
	case kEcFlagStop:
		_velocity = TilePoint(0, 0, 0);
		break;
	case kEcFlagNone:
		break;
	}
}

//-----------------------------------------------------------------------
//	Effectron collision detection clones
//    I don't know how they work, and they're cloned from routines
//    that have long since changed.


//-----------------------------------------------------------------------
// clone of tileSlopeHeight()
int16 tileNopeHeight(
    const TilePoint &pt,
    Effectron *obj,
    StandingTileInfo *sti = nullptr);

//-----------------------------------------------------------------------
// clone of objectCollision()
GameObject *objectNollision(Effectron *obj, const TilePoint &loc);



blockageType checkNontact(
    Effectron *obj,
    const TilePoint &loc,
    GameObject **blockResultObj) {
	int16           mapNum = obj->getMapNum();
	int32           terrain;
	GameObject      *blockObj;

	if (blockResultObj) *blockResultObj = nullptr;

	terrain = volumeTerrain(mapNum,
	                        loc,
	                        obj->brdCall(),
	                        obj->hgtCall());

	//  Check for intersection with a wall or obstacle
	if (terrain & kTerrainRaised)
		return kBlockageTerrain;

	//  Check for intersection with slope of the terrain.
	if (((terrain & kTerrainSurface)
	        || (!(terrain & kTerrainWater) && loc.z <= 0))
	        &&  loc.z < tileNopeHeight(loc, obj))
		return kBlockageTerrain;

	//  See if object collided with an object
	blockObj = objectNollision(obj, loc);
	if (blockObj) {
		if (blockResultObj) *blockResultObj = blockObj;
		return kBlockageObject;
	}

	return kBlockageNone;
}

int16 tileNopeHeight(
    const TilePoint &pt,
    Effectron *obj,
    StandingTileInfo *stiResult) {
	int16           mapNum = obj->getMapNum();

	//  Calculate coordinates of tile, metatile, and subtile
	TilePoint       tileCoords = pt >> kTileUVShift,
	                metaCoords = tileCoords >> kPlatShift,
	                origin = metaCoords << kPlatShift,
	                coords = tileCoords - origin,
	                subTile((pt.u >> kSubTileShift) & kSubTileMask,
	                        (pt.v >> kSubTileShift) & kSubTileMask,
	                        0);

	MetaTilePtr     metaPtr;
	StandingTileInfo highestTile,   //  Represents highest tile which is below
	                 //  object's base
	                 lowestTile;     //  Represents lowest tile at tile position
	int16           supportHeight,
	                highestSupportHeight,
	                lowestSupportHeight;

	{
		//  Look up the metatile on the map.
		metaPtr = prevMeta = mapList[mapNum].lookupMeta(metaCoords);
		prevMapNum = mapNum;
		prevCoords.set(metaCoords.u, metaCoords.v, metaCoords.z);
	}

	if (metaPtr == nullptr) return 0L;

	highestTile.surfaceTile = lowestTile.surfaceTile = nullptr;
	highestSupportHeight = -100;
	lowestSupportHeight = 0x7FFF;

	//  Search each platform until we find a tile which is under
	//  the character.

	for (int i = 0; i < kMaxPlatforms; i++) {
		Platform    *p;

		if ((p = metaPtr->fetchPlatform(mapNum, i)) == nullptr)
			continue;

		if (p->flags & kPlVisible) {
			TileInfo        *ti;
			StandingTileInfo sti;

			//  Get the tile, and its base height
			ti =    p->fetchTAGInstance(
			            mapNum,
			            coords,
			            origin,
			            sti);

			if (ti) {
				int32 subTileTerrain =
				    ti->attrs.testTerrain(calcSubTileMask(subTile.u,
				                          subTile.v));
				if (subTileTerrain & kTerrainInsubstantial)
					continue;
				else if (subTileTerrain & kTerrainRaised)
					// calculate height of raised surface
					supportHeight = sti.surfaceHeight +
					                ti->attrs.terrainHeight;
				else if (subTileTerrain & kTerrainWater)
					// calculate depth of water
					supportHeight = sti.surfaceHeight -
					                ti->attrs.terrainHeight;
				else
					// calculate height of unraised surface
					supportHeight = sti.surfaceHeight +
					                ptHeight(TilePoint(pt.u & kTileUVMask,
					                                   pt.v & kTileUVMask,
					                                   0),
					                         ti->attrs.cornerHeight);

				//  See if the tile is a potential supporting surface
				if (supportHeight <= pt.z + obj->hgtCall()
				        &&  supportHeight >= highestSupportHeight
				        && (ti->combinedTerrainMask() &
				            (kTerrainSurface | kTerrainRaised))) {
					highestTile = sti;
					highestSupportHeight = supportHeight;
				} else if (highestTile.surfaceTile == nullptr &&
				           supportHeight <= lowestSupportHeight &&
				           (ti->combinedTerrainMask() &
				            (kTerrainSurface | kTerrainRaised))) {
					lowestTile = sti;
					lowestSupportHeight = supportHeight;
				}
			}
		}
	}

	if (highestTile.surfaceTile) {
		if (stiResult) *stiResult = highestTile;
		return highestSupportHeight;
	}
	if (lowestTile.surfaceTile) {
		if (stiResult) *stiResult = lowestTile;
		return lowestSupportHeight;
	}
	if (stiResult) {
		stiResult->surfaceTile = nullptr;
		stiResult->surfaceTAG = nullptr;
		stiResult->surfaceHeight = 0;
	}
	return 0;
}

GameObject *objectNollision(Effectron *obj, const TilePoint &loc) {
//    ProtoObj        *proto = obj->proto();
	TileRegion      volume;
	GameObject      *obstacle = nullptr;

	volume.min.u = loc.u - obj->brdCall();
	volume.min.v = loc.v - obj->brdCall();
	volume.max.u = loc.u + obj->brdCall();
	volume.max.v = loc.v + obj->brdCall();
	volume.min.z = loc.z;
	volume.max.z = loc.z + obj->hgtCall();

	//  Adjust MIN Z for the fact that they can step over obstacles.
	//if ( isActor( obj )) volume.min.z += kMaxStepHeight / 2;

	//  Constructor
	CircularObjectIterator  iter(obj->world(), loc, obj->brdCall() + 32);

	for (iter.first(&obstacle);
	        obstacle != nullptr;
	        iter.next(&obstacle)) {
		TilePoint   tp = obstacle->getLocation();
		ProtoObj    *proto = obstacle->proto();

		//if ( obstacle == obj ) continue;

		if (tp.z < volume.max.z
		        &&  tp.z + proto->height > volume.min.z
		        &&  tp.u - proto->crossSection < volume.max.u
		        &&  tp.u + proto->crossSection > volume.min.u
		        &&  tp.v - proto->crossSection < volume.max.v
		        &&  tp.v + proto->crossSection > volume.min.v) {
			return obstacle;
		}
	}
	return nullptr;
}

} // end of namespace Saga2
