/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "math/line3d.h"
#include "math/rect2d.h"

#include "engines/grim/debug.h"
#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/costume.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/movie/movie.h"
#include "engines/grim/sound.h"
#include "engines/grim/lua.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/model.h"

#include "engines/grim/emi/emi.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/emi/modelemi.h"

#include "common/foreach.h"

namespace Grim {

Shadow::Shadow() :
		shadowMask(nullptr), shadowMaskSize(0), active(false), dontNegate(false), userData(nullptr) {
}

static int animTurn(float turnAmt, const Math::Angle &dest, Math::Angle *cur) {
	Math::Angle d = dest - *cur;
	d.normalize(-180);
	// If the actor won't turn because the rate is set to zero then
	// have the actor turn all the way to the destination yaw.
	// Without this some actors will lock the interface on changing
	// scenes, this affects the Bone Wagon in particular.
	if (turnAmt == 0 || turnAmt >= fabsf(d.getDegrees())) {
		*cur = dest;
	} else if (d > 0) {
		*cur += turnAmt;
	} else {
		*cur -= turnAmt;
	}
	if (d != 0) {
		return (d > 0 ? 1 : -1);
	}
	return 0;
}

bool Actor::_isTalkingBackground = false;

void Actor::saveStaticState(SaveGame *state) {
	state->writeBool(_isTalkingBackground);
}

void Actor::restoreStaticState(SaveGame *state) {
	_isTalkingBackground = state->readBool();
}

Actor::Actor() :
		_talkColor(255, 255, 255), _pos(0, 0, 0),
		_lookingMode(false), _followBoxes(false), _running(false), 
		_pitch(0), _yaw(0), _roll(0), _walkRate(0.3f),
		_turnRateMultiplier(0.f), _talkAnim(0),
		_reflectionAngle(80), _scale(1.f), _timeScale(1.f),
		_visible(true), _lipSync(nullptr), _turning(false), _singleTurning(false), _walking(false),
		_walkedLast(false), _walkedCur(false),
		_collisionMode(CollisionOff), _collisionScale(1.f),
		_lastTurnDir(0), _currTurnDir(0),
		_sayLineText(0), _talkDelay(0),
		_attachedActor(0), _attachedJoint(""),
		_globalAlpha(1.f), _alphaMode(AlphaOff),
		 _mustPlaceText(false), 
		_puckOrient(false), _talking(false), 
		_inOverworld(false), _drawnToClean(false), _backgroundTalk(false),
		_sortOrder(0), _useParentSortOrder(false),
		_sectorSortOrder(-1), _fakeUnbound(false), _lightMode(LightFastDyn),
		_hasFollowedBoxes(false), _lookAtActor(0) {

	// Some actors don't set walk and turn rates, so we default the
	// _turnRate so Doug at the cat races can turn and we set the
	// _walkRate so Glottis at the demon beaver entrance can walk and
	// so Chepito in su.set
	_turnRate = 100.0f;

	_activeShadowSlot = -1;
	_shadowArray = new Shadow[MAX_SHADOWS];
}

Actor::~Actor() {
	if (_shadowArray) {
		clearShadowPlanes();
		delete[] _shadowArray;
	}
	while (!_costumeStack.empty()) {
		delete _costumeStack.back();
		_costumeStack.pop_back();
	}
	g_grim->immediatelyRemoveActor(this);
}

void Actor::saveState(SaveGame *savedState) const {
	// store actor name
	savedState->writeString(_name);
	savedState->writeString(_setName);

	savedState->writeColor(_talkColor);

	savedState->writeVector3d(_pos);

	savedState->writeFloat(_pitch.getDegrees());
	savedState->writeFloat(_yaw.getDegrees());
	savedState->writeFloat(_roll.getDegrees());
	savedState->writeFloat(_walkRate);
	savedState->writeFloat(_turnRate);
	savedState->writeFloat(_turnRateMultiplier);
	savedState->writeBool(_followBoxes);
	savedState->writeFloat(_reflectionAngle);
	savedState->writeBool(_visible);
	savedState->writeBool(_lookingMode);
	savedState->writeFloat(_scale);
	savedState->writeFloat(_timeScale);
	savedState->writeBool(_puckOrient);

	savedState->writeString(_talkSoundName);
	savedState->writeBool(_talking);
	savedState->writeBool(_backgroundTalk);

	savedState->writeLEUint32((uint32)_collisionMode);
	savedState->writeFloat(_collisionScale);

	if (_lipSync) {
		savedState->writeBool(true);
		savedState->writeString(_lipSync->getFilename());
	} else {
		savedState->writeBool(false);
	}

	savedState->writeLEUint32(_costumeStack.size());
	for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		savedState->writeString(c->getFilename());
		Costume *pc = c->getPreviousCostume();
		int depth = 0;
		while (pc) {
			++depth;
			pc = pc->getPreviousCostume();
		}
		savedState->writeLESint32(depth);
		pc = c->getPreviousCostume();
		for (int j = 0; j < depth; ++j) { //save the previousCostume hierarchy
			savedState->writeString(pc->getFilename());
			pc = pc->getPreviousCostume();
		}
		c->saveState(savedState);
	}

	savedState->writeBool(_turning);
	savedState->writeBool(_singleTurning);
	savedState->writeFloat(_moveYaw.getDegrees());
	savedState->writeFloat(_movePitch.getDegrees());
	savedState->writeFloat(_moveRoll.getDegrees());

	savedState->writeBool(_walking);
	savedState->writeVector3d(_destPos);

	_restChore.saveState(savedState);

	_walkChore.saveState(savedState);
	savedState->writeBool(_walkedLast);
	savedState->writeBool(_walkedCur);

	_leftTurnChore.saveState(savedState);
	_rightTurnChore.saveState(savedState);
	savedState->writeLESint32(_lastTurnDir);
	savedState->writeLESint32(_currTurnDir);

	for (int i = 0; i < 10; ++i) {
		_talkChore[i].saveState(savedState);
	}
	savedState->writeLESint32(_talkAnim);

	_mumbleChore.saveState(savedState);

	for (int i = 0; i < MAX_SHADOWS; ++i) {
		Shadow &shadow = _shadowArray[i];
		savedState->writeString(shadow.name);

		savedState->writeVector3d(shadow.pos);

		savedState->writeLEUint32(shadow.planeList.size());
		// Cannot use g_grim->getCurrSet() here because an actor can have walk planes
		// from other scenes. It happens e.g. when Membrillo calls Velasco to tell him
		// Naranja is dead.
		for (SectorListType::iterator j = shadow.planeList.begin(); j != shadow.planeList.end(); ++j) {
			Plane &p = *j;
			savedState->writeString(p.setName);
			savedState->writeString(p.sector->getName());
		}

		savedState->writeLESint32(shadow.shadowMaskSize);
		savedState->write(shadow.shadowMask, shadow.shadowMaskSize);
		savedState->writeBool(shadow.active);
		savedState->writeBool(shadow.dontNegate);
	}
	savedState->writeLESint32(_activeShadowSlot);

	savedState->writeLESint32(_sayLineText);

	savedState->writeVector3d(_lookAtVector);

	savedState->writeLEUint32(_path.size());
	for (Common::List<Math::Vector3d>::const_iterator i = _path.begin(); i != _path.end(); ++i) {
		savedState->writeVector3d(*i);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		savedState->writeLEUint32(_alphaMode);
		savedState->writeFloat(_globalAlpha);

		savedState->writeBool(_inOverworld);
		savedState->writeLESint32(_sortOrder);
		savedState->writeBool(_useParentSortOrder);

		savedState->writeLESint32(_attachedActor);
		savedState->writeString(_attachedJoint);

		Common::List<MaterialPtr>::const_iterator it = _materials.begin();
		for (; it != _materials.end(); ++it) {
			if (*it) {
				warning("%s", (*it)->getFilename().c_str());
				savedState->writeLESint32((*it)->getActiveTexture());
			}
		}

		savedState->writeLESint32(_lookAtActor);

		savedState->writeLEUint32(_localAlpha.size());
		for (unsigned int i = 0; i < _localAlpha.size(); i++) {
			savedState->writeFloat(_localAlpha[i]);
		}
		savedState->writeLEUint32(_localAlphaMode.size());
		for (unsigned int i = 0; i < _localAlphaMode.size(); i++) {
			savedState->writeLESint32(_localAlphaMode[i]);
		}
	}

	savedState->writeBool(_drawnToClean);
	savedState->writeBool(_fakeUnbound);
}

bool Actor::restoreState(SaveGame *savedState) {
	for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		delete *i;
	}
	_costumeStack.clear();
	_materials.clear();

	// load actor name
	_name = savedState->readString();
	_setName = savedState->readString();

	_talkColor = savedState->readColor();

	_pos                = savedState->readVector3d();
	_pitch              = savedState->readFloat();
	_yaw                = savedState->readFloat();
	_roll               = savedState->readFloat();
	_walkRate           = savedState->readFloat();
	_turnRate           = savedState->readFloat();
	if (savedState->saveMinorVersion() > 6) {
		_turnRateMultiplier = savedState->readFloat();
	} else {
		_turnRateMultiplier = 1.f;
	}
	_followBoxes        = savedState->readBool();
	_reflectionAngle    = savedState->readFloat();
	_visible            = savedState->readBool();
	_lookingMode        = savedState->readBool();
	_scale              = savedState->readFloat();
	_timeScale          = savedState->readFloat();
	_puckOrient         = savedState->readBool();

	_talkSoundName      = savedState->readString();
	_talking = savedState->readBool();
	_backgroundTalk = savedState->readBool();
	if (isTalking()) {
		g_grim->addTalkingActor(this);
	}

	_collisionMode      = (CollisionMode)savedState->readLEUint32();
	_collisionScale     = savedState->readFloat();

	if (savedState->readBool()) {
		Common::String fn = savedState->readString();
		_lipSync = g_resourceloader->getLipSync(fn);
	} else {
		_lipSync = nullptr;
	}

	uint32 size = savedState->readLEUint32();
	for (uint32 i = 0; i < size; ++i) {
		Common::String fname = savedState->readString();
		const int depth = savedState->readLESint32();
		Costume *pc = nullptr;
		if (depth > 0) {    //build all the previousCostume hierarchy
			Common::String *names = new Common::String[depth];
			for (int j = 0; j < depth; ++j) {
				names[j] = savedState->readString();
			}
			for (int j = depth - 1; j >= 0; --j) {
				pc = findCostume(names[j]);
				if (!pc) {
					pc = g_resourceloader->loadCostume(names[j], this, pc);
				}
			}
			delete[] names;
		}

		Costume *c = g_resourceloader->loadCostume(fname, this, pc);
		c->restoreState(savedState);
		_costumeStack.push_back(c);
	}

	_turning = savedState->readBool();
	if (savedState->saveMinorVersion() > 25) {
		_singleTurning = savedState->readBool();
	}
	_moveYaw = savedState->readFloat();
	if (savedState->saveMinorVersion() > 6) {
		_movePitch = savedState->readFloat();
		_moveRoll = savedState->readFloat();
	} else {
		_movePitch = _pitch;
		_moveRoll = _roll;
	}

	_walking = savedState->readBool();
	_destPos = savedState->readVector3d();

	_restChore.restoreState(savedState, this);

	_walkChore.restoreState(savedState, this);
	_walkedLast = savedState->readBool();
	_walkedCur = savedState->readBool();

	_leftTurnChore.restoreState(savedState, this);
	_rightTurnChore.restoreState(savedState, this);
	_lastTurnDir = savedState->readLESint32();
	_currTurnDir = savedState->readLESint32();

	for (int i = 0; i < 10; ++i) {
		_talkChore[i].restoreState(savedState, this);
	}
	_talkAnim = savedState->readLESint32();

	_mumbleChore.restoreState(savedState, this);

	clearShadowPlanes();

	int maxShadows = MAX_SHADOWS;
	if (savedState->saveMinorVersion() < 24)
		maxShadows = 5;
	for (int i = 0; i < maxShadows; ++i) {
		Shadow &shadow = _shadowArray[i];
		shadow.name = savedState->readString();

		shadow.pos = savedState->readVector3d();

		size = savedState->readLEUint32();
		Set *scene = nullptr;
		for (uint32 j = 0; j < size; ++j) {
			Common::String actorSetName = savedState->readString();
			Common::String secName = savedState->readString();
			if (!scene || scene->getName() != actorSetName) {
				scene = g_grim->findSet(actorSetName);
			}
			if (scene) {
				addShadowPlane(secName.c_str(), scene, i);
			} else {
				warning("%s: No scene \"%s\" found, cannot restore shadow on sector \"%s\"", getName().c_str(), actorSetName.c_str(), secName.c_str());
			}
		}

		shadow.shadowMaskSize = savedState->readLESint32();
		delete[] shadow.shadowMask;
		if (shadow.shadowMaskSize > 0) {
			shadow.shadowMask = new byte[shadow.shadowMaskSize];
			savedState->read(shadow.shadowMask, shadow.shadowMaskSize);
		} else {
			shadow.shadowMask = nullptr;
		}
		shadow.active = savedState->readBool();
		shadow.dontNegate = savedState->readBool();
	}
	_activeShadowSlot = savedState->readLESint32();

	_sayLineText = savedState->readLESint32();

	_lookAtVector = savedState->readVector3d();

	size = savedState->readLEUint32();
	_path.clear();
	for (uint32 i = 0; i < size; ++i) {
		_path.push_back(savedState->readVector3d());
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		_alphaMode = (AlphaMode) savedState->readLEUint32();
		_globalAlpha = savedState->readFloat();

		_inOverworld  = savedState->readBool();
		_sortOrder    = savedState->readLESint32();
		if (savedState->saveMinorVersion() >= 22)
			_useParentSortOrder = savedState->readBool();

		if (savedState->saveMinorVersion() < 18)
			savedState->readBool(); // Used to be _shadowActive.

		_attachedActor = savedState->readLESint32();
		_attachedJoint = savedState->readString();

		// will be recalculated in next update()
		_sectorSortOrder = -1;

		if (savedState->saveMinorVersion() < 24) {
			// Used to be the wear chore.
			if (savedState->readBool()) {
				savedState->readString();
			}
			savedState->readLESint32();
		}

		if (savedState->saveMinorVersion() >= 13) {
			Common::List<MaterialPtr>::const_iterator it = _materials.begin();
			for (; it != _materials.end(); ++it) {
				if (*it) {
					(*it)->setActiveTexture(savedState->readLESint32());
				}
			}
		}

		if (savedState->saveMinorVersion() >= 17)
			_lookAtActor = savedState->readLESint32();

		if (savedState->saveMinorVersion() >= 25) {
			_localAlpha.resize(savedState->readLEUint32());
			for (unsigned int i = 0; i < _localAlpha.size(); i++) {
				_localAlpha[i] = savedState->readFloat();
			}

			_localAlphaMode.resize(savedState->readLEUint32());
			for (unsigned int i = 0; i < _localAlphaMode.size(); i++) {
				_localAlphaMode[i] = savedState->readLESint32();
			}
		}
	}

	if (savedState->saveMinorVersion() >= 4) {
		_drawnToClean = savedState->readBool();
	} else {
		_drawnToClean = false;
	}
	if (savedState->saveMinorVersion() >= 27) {
		_fakeUnbound = savedState->readBool();
	} else {
		// Note: actor will stay invisible until re-bound to set
		// by game scripts.
		_fakeUnbound = false;
	}

	return true;
}

void Actor::setRot(const Math::Vector3d &pos) {
	Math::Angle y, p, r;
	calculateOrientation(pos, &p, &y, &r);
	setRot(p, y, r);
}

void Actor::setRot(const Math::Angle &pitchParam, const Math::Angle &yawParam, const Math::Angle &rollParam) {
	_pitch = pitchParam;
	_yaw = yawParam;
	_moveYaw = _yaw;
	_roll = rollParam;
	_turning = false;
}

void Actor::setPos(const Math::Vector3d &position) {
	_walking = false;
	_pos = position;

	// Don't allow positions outside the sectors.
	// This is necessary after solving the tree pump puzzle, when the bone
	// wagon returns to the signopost set.
	if (_followBoxes) {
		g_grim->getCurrSet()->findClosestSector(_pos, nullptr, &_pos);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		Math::Vector3d moveVec = position - _pos;
		foreach (Actor *a, g_grim->getActiveActors()) {
			handleCollisionWith(a, _collisionMode, &moveVec);
		}
	}
}

void Actor::calculateOrientation(const Math::Vector3d &pos, Math::Angle *pitch, Math::Angle *yaw, Math::Angle *roll) {
	Math::Vector3d actorForward(0.f, 1.f, 0.f);
	Math::Vector3d actorUp(0.f, 0.f, 1.f);
	Math::Vector3d lookVector = pos - _pos;
	lookVector.normalize();

	// EMI: Y is up-down, actors use an X-Z plane for movement
	if (g_grim->getGameType() == GType_MONKEY4) {
		float temp = lookVector.z();
		lookVector.x() = -lookVector.x();
		lookVector.z() = lookVector.y();
		lookVector.y() = temp;
	}

	Math::Vector3d up = actorUp;
	if (_puckOrient) {
		Sector *s = nullptr;
		g_grim->getCurrSet()->findClosestSector(_pos, &s, nullptr);
		if (s) {
			up = s->getNormal();
		}
	}

	Math::Matrix3 m;
	m.buildFromTargetDir(actorForward, lookVector, actorUp, up);

	if (_puckOrient) {
		m.getEuler(yaw, pitch, roll, Math::EO_ZXY);
	} else {
		*pitch = _movePitch;
		m.getEuler(yaw, nullptr, nullptr, Math::EO_ZXY);
		*roll = _moveRoll;
	}
}

void Actor::turnTo(const Math::Vector3d &pos, bool snap) {
	Math::Angle y, p, r;
	calculateOrientation(pos, &p, &y, &r);
	turnTo(p, y, r, snap);
}

bool Actor::singleTurnTo(const Math::Vector3d &pos) {
	Math::Angle y, p, r;
	calculateOrientation(pos, &p, &y, &r);

	float turnAmt = g_grim->getPerSecond(_turnRate);
	bool done = animTurn(turnAmt, y, &_yaw) == 0;
	done = animTurn(turnAmt, p, &_pitch) == 0 && done;
	done = animTurn(turnAmt, r, &_roll) == 0 && done;
	_moveYaw = _yaw;
	_movePitch = _pitch;
	_moveRoll = _roll;
	_singleTurning = !done;

	return done;
}

void Actor::turnTo(const Math::Angle &pitchParam, const Math::Angle &yawParam, const Math::Angle &rollParam, bool snap) {
	_movePitch = pitchParam;
	_moveRoll = rollParam;
	_moveYaw = yawParam;
	_turnRateMultiplier = (snap ? 5.f : 1.f);
	if (_yaw != yawParam || _pitch != pitchParam || _roll != rollParam) {
		_turning = true;
	} else
		_turning = false;
}

void Actor::walkTo(const Math::Vector3d &p) {
	if (p == _pos)
		_walking = false;
	else {
		_walking = true;
		_destPos = p;
		_path.clear();

		if (_followBoxes) {
			bool pathFound = false;

			Set *currSet = g_grim->getCurrSet();
			currSet->findClosestSector(p, nullptr, &_destPos);

			Common::List<PathNode *> openList;
			Common::List<PathNode *> closedList;

			PathNode *start = new PathNode;
			start->parent = nullptr;
			start->pos = _pos;
			start->dist = 0.f;
			start->cost = 0.f;
			openList.push_back(start);
			currSet->findClosestSector(_pos, &start->sect, nullptr);

			Common::List<Sector *> sectors;
			for (int i = 0; i < currSet->getSectorCount(); ++i) {
				Sector *s = currSet->getSectorBase(i);
				int type = s->getType();
				if ((type == Sector::WalkType || type == Sector::HotType || type == Sector::FunnelType) && s->isVisible()) {
					sectors.push_back(s);
				}
			}

			do {
				PathNode *node = nullptr;
				float cost = -1.f;
				for (Common::List<PathNode *>::iterator j = openList.begin(); j != openList.end(); ++j) {
					PathNode *n = *j;
					float c = n->dist + n->cost;
					if (cost < 0.f || c < cost) {
						cost = c;
						node = n;
					}
				}
				closedList.push_back(node);
				openList.remove(node);
				Sector *sector = node->sect;

				if (sector && sector->isPointInSector(_destPos)) {
					PathNode *n = closedList.back();
					// Don't put the start position in the list, or else
					// the first angle calculated in updateWalk() will be
					// meaningless. The only node without parent is the start
					// one.
					while (n->parent) {
						_path.push_back(n->pos);
						n = n->parent;
					}

					pathFound = true;
					break;
				}

				for (Common::List<Sector *>::iterator i = sectors.begin(); i != sectors.end(); ++i) {
					Sector *s = *i;
					bool inClosed = false;
					for (Common::List<PathNode *>::iterator j = closedList.begin(); j != closedList.end(); ++j) {
						if ((*j)->sect == s) {
							inClosed = true;
							break;
						}
					}
					if (inClosed)
						continue;

					// Get "bridges" from the current sector to the other.
					Common::List<Math::Line3d> bridges = sector->getBridgesTo(s);
					if (bridges.empty())
						continue; // The sectors are not adjacent.

					Math::Vector3d closestPoint;
					if (g_grim->getGameType() == GType_GRIM)
						closestPoint = s->getClosestPoint(_destPos);
					else
						closestPoint = _destPos;
					Math::Vector3d best;
					float bestDist = 1e6f;
					Math::Line3d l(node->pos, closestPoint);

					// Pick a point on the boundary of the two sectors to walk towards.
					while (!bridges.empty()) {
						Math::Line3d bridge = bridges.back();
						Math::Vector3d pos;
						const bool useXZ = (g_grim->getGameType() == GType_MONKEY4);

						// Prefer points on the straight line from this node towards
						// the destination. Otherwise pick the middle point of a bridge
						// that is closest to the destination.
						if (!bridge.intersectLine2d(l, &pos, useXZ)) {
							pos = bridge.middle();
						} else {
							best = pos;
							break;
						}
						float dist = (pos - closestPoint).getMagnitude();
						if (dist < bestDist) {
							bestDist = dist;
							best = pos;
						}
						bridges.pop_back();
					}
					best = handleCollisionTo(node->pos, best);

					PathNode *n = nullptr;
					for (Common::List<PathNode *>::iterator j = openList.begin(); j != openList.end(); ++j) {
						if ((*j)->sect == s) {
							n = *j;
							break;
						}
					}
					if (n) {
						float newCost = node->cost + (best - node->pos).getMagnitude();
						if (newCost < n->cost) {
							n->cost = newCost;
							n->parent = node;
							n->pos = best;
							n->dist = (n->pos - _destPos).getMagnitude();
						}
					} else {
						n = new PathNode;
						n->parent = node;
						n->sect = s;
						n->pos = best;
						n->dist = (n->pos - _destPos).getMagnitude();
						n->cost = node->cost + (n->pos - node->pos).getMagnitude();
						openList.push_back(n);
					}
				}
			} while (!openList.empty());

			for (Common::List<PathNode *>::iterator j = closedList.begin(); j != closedList.end(); ++j) {
				delete *j;
			}
			for (Common::List<PathNode *>::iterator j = openList.begin(); j != openList.end(); ++j) {
				delete *j;
			}

			if (!pathFound) {
				warning("Actor::walkTo(): No path found for %s", _name.c_str());
				if (g_grim->getGameType() == GType_MONKEY4) {
					_walking = false;
					return;
				}
			}
		}

		_path.push_front(_destPos);
	}
}

bool Actor::isWalking() const {
	return _walkedLast || _walkedCur || _walking;
}

bool Actor::isTurning() const {
	if (g_grim->getGameType() == GType_MONKEY4)
		if (_singleTurning)
			return true;
	if (_turning)
		return true;

	if (_lastTurnDir != 0 || _currTurnDir != 0)
		return true;

	return false;
}

void Actor::stopTurning() {
	_turning = false;
	if (_lastTurnDir != 0)
		getTurnChore(_lastTurnDir)->stop(true);

	_lastTurnDir = 0;
	_currTurnDir = 0;
}

void Actor::moveTo(const Math::Vector3d &pos) {
	// The walking actor doesn't always have the collision mode set, but it must however check
	// the collisions. E.g. the set hl doesn't set Manny's mode, but it must check for
	// collisions with Raoul.
	// On the other hand, it must not *set* the sphere mode, it must just use it for this call:
	// the set xb sets Manny's collision scale as 1 and mode as Off. If you then go to set tb
	// and talk to Doug at the photo window, Manny's mode *must be* Off, otherwise Doug will
	// collide, miss the target point and will walk away, leaving Manny waiting for him at
	// the window forever.
	CollisionMode mode = _collisionMode;
	if (_collisionMode == CollisionOff) {
		mode = CollisionSphere;
	}

	Math::Vector3d moveVec = pos - _pos;
	foreach (Actor *a, g_grim->getActiveActors()) {
		handleCollisionWith(a, mode, &moveVec);
	}
	_pos += moveVec;
}

void Actor::walkForward() {
	float dist = g_grim->getPerSecond(_walkRate);
	// Limit the amount of the movement per frame, otherwise with low fps
	// scripts that use WalkActorForward and proximity may break.
	if ((dist > 0 && dist > _walkRate / 5.f) || (dist < 0 && dist < _walkRate / 5.f))
		dist = _walkRate / 5.f;

	// Handle special case where actor is trying to walk but _walkRate is
	// currently set to 0.0f by _walkChore to simulate roboter-like walk style:
	// set _walkedCur to true to keep _walkChore playing in Actor::update()
	if (_walkRate == 0.0f) {
		_walkedCur = true;
	}

	_walking = false;

	if (!_followBoxes) {
		Math::Vector3d forwardVec(-_moveYaw.getSine() * _pitch.getCosine(),
								  _moveYaw.getCosine() * _pitch.getCosine(), _pitch.getSine());

		// EMI: Y is up-down, actors use an X-Z plane for movement
		if (g_grim->getGameType() == GType_MONKEY4) {
			float temp = forwardVec.z();
			forwardVec.x() = -forwardVec.x();
			forwardVec.z() = forwardVec.y();
			forwardVec.y() = temp;
		}

		_pos += forwardVec * dist;
		_walkedCur = true;
		return;
	}

	bool backwards = false;
	if (dist < 0) {
		dist = -dist;
		backwards = true;
	}

	int tries = 0;
	while (dist > 0.0f) {
		Sector *currSector = nullptr, *prevSector = nullptr, *startSector = nullptr;
		Sector::ExitInfo ei;

		g_grim->getCurrSet()->findClosestSector(_pos, &currSector, &_pos);
		if (!currSector) { // Shouldn't happen...
			Math::Vector3d forwardVec(-_moveYaw.getSine() * _pitch.getCosine(),
									  _moveYaw.getCosine() * _pitch.getCosine(), _pitch.getSine());

			// EMI: Y is up-down, actors use an X-Z plane for movement
			if (g_grim->getGameType() == GType_MONKEY4) {
				float temp = forwardVec.z();
				forwardVec.x() = -forwardVec.x();
				forwardVec.z() = forwardVec.y();
				forwardVec.y() = temp;
			}

			if (backwards)
				forwardVec = -forwardVec;

			moveTo(_pos + forwardVec * dist);
			_walkedCur = true;
			return;
		}
		startSector = currSector;

		float oldDist = dist;
		while (currSector) {
			prevSector = currSector;
			Math::Vector3d forwardVec;
			const Math::Vector3d &normal = currSector->getNormal();
			if (g_grim->getGameType() == GType_GRIM) {
				Math::Angle ax = Math::Vector2d(normal.x(), normal.z()).getAngle();
				Math::Angle ay = Math::Vector2d(normal.y(), normal.z()).getAngle();

				float z1 = -_moveYaw.getCosine() * (ay - _pitch).getCosine();
				float z2 = _moveYaw.getSine() * (ax - _pitch).getCosine();
				forwardVec = Math::Vector3d(-_moveYaw.getSine() * ax.getSine() * _pitch.getCosine(),
											_moveYaw.getCosine() * ay.getSine() * _pitch.getCosine(), z1 + z2);
			} else {
				Math::Angle ax = Math::Vector2d(normal.x(), normal.y()).getAngle();
				Math::Angle az = Math::Vector2d(normal.z(), normal.y()).getAngle();

				float y1 = _moveYaw.getCosine() * (az - _pitch).getCosine();
				float y2 = _moveYaw.getSine() * (ax - _pitch).getCosine();
				forwardVec = Math::Vector3d(-_moveYaw.getSine() * ax.getSine() * _pitch.getCosine(), y1 + y2,
											-_moveYaw.getCosine() * az.getSine() * _pitch.getCosine());
			}

			if (backwards)
				forwardVec = -forwardVec;

			Math::Vector3d puckVec = currSector->getProjectionToPuckVector(forwardVec);
			puckVec /= puckVec.getMagnitude();
			currSector->getExitInfo(_pos, puckVec, &ei);
			float exitDist = (ei.exitPoint - _pos).getMagnitude();
			if (dist < exitDist) {
				moveTo(_pos + puckVec * dist);
				_walkedCur = true;
				return;
			}
			_pos = ei.exitPoint;
			dist -= exitDist;
			if (exitDist > 0.0001)
				_walkedCur = true;

			// Check for an adjacent sector which can continue
			// the path
			currSector = g_grim->getCurrSet()->findPointSector(ei.exitPoint + (float)0.0001 * puckVec, Sector::WalkType);

			// EMI: some sectors are significantly higher/lower than others.
			if (currSector && g_grim->getGameType() == GType_MONKEY4) {
				float planeDist = currSector->distanceToPoint(_pos);
				if (fabs(planeDist) < 1.f)
					_pos -= planeDist * currSector->getNormal();
			}

			if (currSector == prevSector || currSector == startSector)
				break;
		}

		int turnDir = 1;
		if (ei.angleWithEdge > 90) {
			ei.angleWithEdge = 180 - ei.angleWithEdge;
			ei.edgeDir = -ei.edgeDir;
			turnDir = -1;
		}
		if (ei.angleWithEdge > _reflectionAngle)
			return;

		ei.angleWithEdge += (float)1.0f;

		if (g_grim->getGameType() == GType_MONKEY4) {
			ei.angleWithEdge = -ei.angleWithEdge;
		}
		turnTo(0, _moveYaw + ei.angleWithEdge * turnDir, 0, true);

		if (oldDist <= dist + 0.001f) {
			// If we didn't move at all, keep trying a couple more times
			// in case we can move in the new direction.
			tries++;
			if (tries > 3)
				break;
		}
	}
}

Math::Vector3d Actor::getSimplePuckVector() const {
	if (g_grim->getGameType() == GType_MONKEY4) {
		Math::Angle yaw = 0;
		const Actor *a = this;
		while (a) {
			yaw += a->_yaw;
			if (!a->isAttached())
				break;
			a = Actor::getPool().getObject(a->_attachedActor);
		}
		return Math::Vector3d(yaw.getSine(), 0, yaw.getCosine());
	} else {
		return Math::Vector3d(-_yaw.getSine(), _yaw.getCosine(), 0);
	}
}

Math::Vector3d Actor::getPuckVector() const {
	Math::Vector3d forwardVec = getSimplePuckVector();

	Set *currSet = g_grim->getCurrSet();
	if (!currSet)
		return forwardVec;

	Sector *sector = currSet->findPointSector(_pos, Sector::WalkType);
	if (!sector)
		return forwardVec;
	else
		return sector->getProjectionToPuckVector(forwardVec);
}

void Actor::setPuckOrient(bool orient) {
	_puckOrient = orient;
	warning("Actor::setPuckOrient() not implemented");
}

void Actor::setRestChore(int chore, Costume *cost) {
	if (_restChore.equals(cost, chore))
		return;

	_restChore.stop(g_grim->getGameType() == GType_GRIM);

	if (!cost) {
		cost = _restChore._costume;
	}
	if (!cost) {
		cost = getCurrentCostume();
	}

	_restChore = ActionChore(cost, chore);

	_restChore.playLooping(g_grim->getGameType() == GType_GRIM);
}

int Actor::getRestChore() const {
	return _restChore._chore;
}

void Actor::setWalkChore(int chore, Costume *cost) {
	if (_walkChore.equals(cost, chore))
		return;

	if (_walkedLast && _walkChore.isPlaying()) {
		_walkChore.stop(true);

		if (g_grim->getGameType() == GType_GRIM) {
			_restChore.playLooping(true);
		}
	}

	if (!cost) {
		cost = _walkChore._costume;
	}
	if (!cost) {
		cost = getCurrentCostume();
	}

	_walkChore = ActionChore(cost, chore);
}

void Actor::setTurnChores(int left_chore, int right_chore, Costume *cost) {
	if (_leftTurnChore.equals(cost, left_chore) && _rightTurnChore.equals(cost, right_chore))
		return;

	if (!cost) {
		cost = _leftTurnChore._costume;
	}
	if (!cost) {
		cost = getCurrentCostume();
	}

	_leftTurnChore.stop(true);
	_rightTurnChore.stop(true);
	_lastTurnDir = 0;

	_leftTurnChore = ActionChore(cost, left_chore);
	_rightTurnChore = ActionChore(cost, right_chore);

	if ((left_chore >= 0 && right_chore < 0) || (left_chore < 0 && right_chore >= 0))
		error("Unexpectedly got only one turn chore");
}

void Actor::setTalkChore(int index, int chore, Costume *cost) {
	if (index < 1 || index > 10)
		error("Got talk chore index out of range (%d)", index);

	index--;

	if (!cost) {
		cost = _talkChore[index]._costume;
	}
	if (!cost) {
		cost = getCurrentCostume();
	}

	if (_talkChore[index].equals(cost, chore))
		return;

	_talkChore[index].stop();

	_talkChore[index] = ActionChore(cost, chore);
}

int Actor::getTalkChore(int index) const {
	return _talkChore[index]._chore;
}

Costume *Actor::getTalkCostume(int index) const {
	return _talkChore[index]._costume;
}

void Actor::setMumbleChore(int chore, Costume *cost) {
	if (_mumbleChore.isPlaying()) {
		_mumbleChore.stop();
	}

	if (!cost) {
		cost = _mumbleChore._costume;
	}
	if (!cost) {
		cost = getCurrentCostume();
	}

	_mumbleChore = ActionChore(cost, chore);
}

void Actor::stopAllChores(bool ignoreLoopingChores) {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *costume = *i;
		costume->stopChores(ignoreLoopingChores);
		if (costume->isChoring(false) == -1) {
			freeCostume(costume);
			i = _costumeStack.erase(i);
			--i;
		}
	}
}

void Actor::turn(int dir) {
	_walking = false;
	float delta = g_grim->getPerSecond(_turnRate) * dir;
	if (g_grim->getGameType() == GType_MONKEY4) {
		delta = -delta;
	}
	_moveYaw = _moveYaw + delta;
	_turning = true;
	_turnRateMultiplier = 5.f;
	_currTurnDir = dir;
}

Math::Angle Actor::getYawTo(const Actor *a) const {
	Math::Vector3d forwardVec = getSimplePuckVector();
	Math::Vector3d delta = a->getWorldPos() - getWorldPos();

	if (g_grim->getGameType() == GType_MONKEY4) {
		delta.y() = 0;
	} else {
		delta.z() = 0;
	}
	if (delta.getMagnitude() < Math::epsilon)
		return Math::Angle(0);
	return Math::Vector3d::angle(forwardVec, delta);
}

Math::Angle Actor::getYawTo(const Math::Vector3d &p) const {
	Math::Vector3d dpos = p - _pos;

	if (g_grim->getGameType() == GType_MONKEY4) {
		dpos.y() = dpos.z();
	}
	if (dpos.x() == 0 && dpos.y() == 0)
		return 0;
	else
		return Math::Angle::arcTangent2(-dpos.x(), dpos.y());
}

void Actor::sayLine(const char *msgId, bool background, float x, float y) {
	assert(msgId);

	if (msgId[0] == 0) {
		warning("Actor::sayLine: Empty message");
		return;
	}

	char id[50];
	Common::String msg = LuaBase::instance()->parseMsgText(msgId, id);

	if (id[0] == 0) {
		error("Actor::sayLine: No message ID for text");
		return;
	}

	// During Fullscreen movies SayLine is usually called for text display only.
	// The movie with Charlie screaming after Manny put the sheet on him instead
	// uses sayLine for the voice too.
	// However, normal SMUSH movies may call SayLine, for example:
	// When Domino yells at Manny (a SMUSH movie) he does it with
	// a SayLine request rather than as part of the movie!

	Common::String soundName = id;

	if (g_grim->getGameType() == GType_GRIM) {
		if (g_grim->getGameFlags() & ADGF_REMASTERED) {
			soundName = g_grim->getLanguagePrefix() + "_" + soundName;
		}
		soundName += ".wav";
	} else if (g_grim->getGameType() == GType_MONKEY4 && g_grim->getGamePlatform() == Common::kPlatformPS2) {
		soundName += ".scx";
	} else {
		soundName += ".wVC";
	}
	if (_talkSoundName == soundName)
		return;

	if (_talking || msg.empty())
		shutUp();

	_talkSoundName = soundName;

	Set *currSet = g_grim->getCurrSet();

	if (g_grim->getSpeechMode() != GrimEngine::TextOnly) {
		// if there is no costume probably the character is drawn by a smush movie, so
		// we don't want to go out of sync with it.
		if (g_grim->getGameType() == GType_GRIM && getCurrentCostume()) {
			_talkDelay = 500;
		}
		g_sound->startVoice(_talkSoundName.c_str());
	}

	// If the actor is clearly not visible then don't try to play the lip sync
	if (_visible && (!g_movie->isPlaying() || g_grim->getMode() == GrimEngine::NormalMode)) {
		Common::String soundLip = id;
		soundLip += ".lip";

		if (!_talkChore[0].isPlaying()) {
			// _talkChore[0] is *_stop_talk
			_talkChore[0].setLastFrame();
		}
		// Sometimes actors speak offscreen before they, including their
		// talk chores are initialized.
		// For example, when reading the work order (a LIP file exists for no reason).
		// Also, some lip sync files have no entries
		// In these cases, revert to using the mumble chore.
		if (g_grim->getSpeechMode() != GrimEngine::TextOnly)
			_lipSync = g_resourceloader->getLipSync(soundLip);
		// If there's no lip sync file then load the mumble chore if it exists
		// (the mumble chore doesn't exist with the cat races announcer)
		if (!_lipSync)
			_mumbleChore.playLooping();

		_talkAnim = -1;
	}

	_talking = true;
	g_grim->addTalkingActor(this);

	_backgroundTalk = background;
	if (background)
		_isTalkingBackground = true;

	if (_sayLineText && g_grim->getMode() != GrimEngine::SmushMode) {
		delete TextObject::getPool().getObject(_sayLineText);
		_sayLineText = 0;
	}

	if (!msg.empty()) {
		GrimEngine::SpeechMode m = g_grim->getSpeechMode();
		if (!g_grim->_sayLineDefaults.getFont() || m == GrimEngine::VoiceOnly)
			return;

		if (background) {
			// if we're talking background draw the text object only if there are no no-background
			// talking actors. This prevents glottis and nick subtitles overlapping in the high roller lounge,
			// where glottis is background and nick isn't. (https://github.com/residualvm/residualvm/issues/685)
			foreach (Actor *a, g_grim->getTalkingActors()) {
				if (!a->_backgroundTalk && a->_sayLineText) {
					return;
				}
			}
		} else {
			// if we're not background then delete the TextObject of any talking background actor.
			foreach (Actor *a, g_grim->getTalkingActors()) {
				if (a->_backgroundTalk && a->_sayLineText) {
					delete TextObject::getPool().getObject(a->_sayLineText);
					a->_sayLineText = 0;
				}
			}
		}

		TextObject *textObject = new TextObject();
		textObject->setDefaults(&g_grim->_sayLineDefaults);
		textObject->setFGColor(_talkColor);
		textObject->setIsSpeech();
		if (m == GrimEngine::TextOnly || g_grim->getMode() == GrimEngine::SmushMode) {
			textObject->setDuration(500 + msg.size() * 15 * (11 - g_grim->getTextSpeed()));
		}
		if (g_grim->getGameType() == GType_MONKEY4 && (x != -1 || y != -1)) {
			textObject->setX(320 * (x + 1));
			textObject->setY(240 * (y + 1));
		} else if (g_grim->getMode() == GrimEngine::SmushMode) {
			textObject->setX(640 / 2);
			textObject->setY(456);
			g_grim->setMovieSubtitle(textObject);
		} else {
			if (_visible && isInSet(currSet->getName())) {
				_mustPlaceText = true;
			} else {
				_mustPlaceText = false;
				textObject->setX(640 / 2);
				textObject->setY(463);
			}
		}
		textObject->setText(msgId, _mustPlaceText);
		if (g_grim->getMode() != GrimEngine::SmushMode)
			_sayLineText = textObject->getId();
	}
}

void Actor::lineCleanup() {
	if (_sayLineText) {
		delete TextObject::getPool().getObject(_sayLineText);
		_sayLineText = 0;
	}
}

bool Actor::isTalking() {
	// This should return if the actor is actually talking, disregarding of the background status,
	// otherwise when Naranja is asleep Toto's lines will be cut sometimes. Naranja and Toto both
	// talk in background, and when Naranja lines stop toto:wait_for_message() should not return
	// until he actually stops talking.
	return _talking;
}

bool Actor::isTalkingForeground() const {
	if (!_talking) {
		return false;
	}

	if (_backgroundTalk)
		return _isTalkingBackground;

	return true;
}

void Actor::shutUp() {
	// While the call to stop the sound is usually made by the game,
	// we also need to handle when the user terminates the dialog.
	// Some warning messages will occur when the user terminates the
	// actor dialog but the game will continue alright.
	if (_talkSoundName != "") {
		g_sound->stopSound(_talkSoundName.c_str());
		_talkSoundName = "";
	}

	if (_lipSync) {
		if (_talkAnim != -1)
			_talkChore[_talkAnim].stop(g_grim->getGameType() == GType_MONKEY4, ActionChore::talkFadeTime);
		_lipSync = nullptr;
	}
	// having a lipsync is no guarantee the mumble chore is no running. the talk chores may be -1 (domino in do)
	stopMumbleChore();
	stopTalking();

	if (_sayLineText) {
		delete TextObject::getPool().getObject(_sayLineText);
		_sayLineText = 0;
	}

	// The actors talking in background have a special behaviour: if there are two or more of them
	// talking at the same time, after one of them finishes talking calling isTalking() an *all*
	// of them must return false. This is necessary for the angelitos in set fo: when they start crying
	// they talk in background, and the lua script that hangs on IsMessageGoing() must return before they
	// stop, since they can go on forever.
	if (_backgroundTalk)
		_isTalkingBackground = false;

	_talking = false;
}

void Actor::pushCostume(const char *n) {
	Costume *c = findCostume(n);
	if (c) {
		Debug::debug(Debug::Actors, "Trying to push a costume already in the stack. %s, %s", getName().c_str(), n);
		return;
	}

	Costume *newCost = g_resourceloader->loadCostume(n, this, getCurrentCostume());

	_costumeStack.push_back(newCost);
}

void Actor::setColormap(const char *map) {
	if (!_costumeStack.empty()) {
		Costume *cost = _costumeStack.back();
		cost->setColormap(map);
	} else {
		warning("Actor::setColormap: No costumes");
	}
}

void Actor::setCostume(const char *n) {
	if (!_costumeStack.empty())
		popCostume();

	pushCostume(n);
}

void Actor::popCostume() {
	if (!_costumeStack.empty()) {
		freeCostume(_costumeStack.back());
		_costumeStack.pop_back();

		if (_costumeStack.empty()) {
			Debug::debug(Debug::Actors, "Popped (freed) the last costume for an actor.\n");
		}
	} else {
		Debug::warning(Debug::Actors, "Attempted to pop (free) a costume when the stack is empty!");
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (!_costumeStack.empty())
		popCostume();
}

Costume *Actor::getCurrentCostume() const {
	if (g_grim->getGameType() == GType_MONKEY4) {
		// Return the first costume that has a model component.
		for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
			EMICostume *costume = static_cast<EMICostume *>(*i);
			if (costume->getEMIModel()) {
				return costume;
			}
		}
		return nullptr;
	} else {
		if (_costumeStack.empty())
			return nullptr;
		else
			return _costumeStack.back();
	}
}

void Actor::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	Costume *curCostume = getCurrentCostume();
	if (curCostume) {
		curCostume->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
	}
}

void Actor::setHead(const char *joint, const Math::Vector3d &offset) {
	Costume *curCostume = getCurrentCostume();
	if (curCostume) {
		EMICostume *costume = static_cast<EMICostume *>(curCostume);
		costume->setHead(joint, offset);
	}
}

void Actor::setHeadLimits(float yawRange, float maxPitch, float minPitch) {
	Costume *curCostume = getCurrentCostume();
	if (curCostume) {
		EMICostume *costume = static_cast<EMICostume *>(curCostume);
		costume->setHeadLimits(yawRange, maxPitch, minPitch);
	}
}

void Actor::setLookAtRate(float rate) {
	getCurrentCostume()->setLookAtRate(rate);
}

float Actor::getLookAtRate() const {
	return getCurrentCostume()->getLookAtRate();
}

EMIModel *Actor::findModelWithMesh(const Common::String &mesh) {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		EMICostume *costume = static_cast<EMICostume *>(*i);
		if (!costume) {
			continue;
		}
		for (int j = 0; j < costume->getNumChores(); j++) {
			EMIModel *model = costume->getEMIModel(j);
			if (!model) {
				continue;
			}
			if (mesh == model->_meshName) {
				return model;
			}
		}
	}
	return nullptr;
}

void Actor::setGlobalAlpha(float alpha, const Common::String &mesh) {
	if (mesh.empty()) {
		_globalAlpha = alpha;
	} else {
		EMIModel *model = findModelWithMesh(mesh);
		if (model != nullptr) {
			model->_meshAlpha = alpha;
		}
	}
}

void Actor::setAlphaMode(AlphaMode mode, const Common::String &mesh) {
	if (mesh.empty()) {
		_alphaMode = mode;
	} else {
		EMIModel *model = findModelWithMesh(mesh);
		if (model != nullptr) {
			model->_meshAlphaMode = mode;
		}
	}
}

Costume *Actor::findCostume(const Common::String &n) {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		if ((*i)->getFilename().compareToIgnoreCase(n) == 0)
			return *i;
	}

	return nullptr;
}

void Actor::setFollowBoxes(bool follow) {
	_followBoxes = follow;
	if (follow)
		_hasFollowedBoxes = true;
}

void Actor::updateWalk() {
	if (_path.empty()) {
		return;
	}

	Math::Vector3d destPos = _path.back();
	Math::Vector3d dir = destPos - _pos;
	float dist = dir.getMagnitude();

	_walkedCur = true;
	float walkAmt = g_grim->getPerSecond(_walkRate);
	if (walkAmt >= dist) {
		walkAmt = dist;
		_path.pop_back();
		if (_path.empty()) {
			_walking = false;
			_pos = destPos;
// It seems that we need to allow an already active turning motion to
// continue or else turning actors away from barriers won't work right
			_turning = false;
			return;
		}
	}

	turnTo(destPos, true);

	dir = destPos - _pos;
	dir.normalize();
	_pos += dir * walkAmt;
}

void Actor::update(uint frameTime) {
	Set *set = g_grim->getCurrSet();
	// Snap actor to walkboxes if following them.  This might be
	// necessary for example after activating/deactivating
	// walkboxes, etc.
	if (_followBoxes && !_walking) {
		set->findClosestSector(_pos, nullptr, &_pos);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		if (_followBoxes) {
			// Check for sort order information in the current sector
			int oldSortOrder = getEffectiveSortOrder();
			_sectorSortOrder = set->findSectorSortOrder(_pos, Sector::WalkType);

			if (oldSortOrder != getEffectiveSortOrder())
				g_emi->invalidateSortOrder();
		} else if (_sectorSortOrder >= 0) {
			_sectorSortOrder = -1;
			g_emi->invalidateSortOrder();
		}
	}

	if (_turning) {
		float turnAmt = g_grim->getPerSecond(_turnRate) * _turnRateMultiplier;
		_currTurnDir = animTurn(turnAmt, _moveYaw, &_yaw);
		if (g_grim->getGameType() == GType_MONKEY4) {
			_currTurnDir = -_currTurnDir;
		}
		int p = animTurn(turnAmt, _movePitch, &_pitch);
		int r = animTurn(turnAmt, _moveRoll, &_roll);
		if (_currTurnDir == 0 && p == 0 && r == 0) {
			_turning = false;
			_turnRateMultiplier = 1.f;
		}
	}

	if (_walking) {
		updateWalk();
	}

	if (_walkChore.isValid()) {
		if (_walkedCur) {
			if (!_walkChore.isPlaying()) {
				_walkChore.playLooping(true);
			}
			if (g_grim->getGameType() == GType_GRIM && _restChore.isPlaying()) {
				_restChore.stop(true);
			}
		} else {
			if (_walkedLast && _walkChore.isPlaying()) {
				_walkChore.stop(true);
				if (!_restChore.isPlaying()) {
					_restChore.playLooping(true);
				}
			}
		}
	}

	if (_leftTurnChore.isValid()) {
		if (_walkedCur || _walkedLast)
			_currTurnDir = 0;

		if (g_grim->getGameType() == GType_GRIM && _restChore.isValid()) {
			if (_currTurnDir != 0) {
				if (getTurnChore(_currTurnDir)->isPlaying() && _restChore.isPlaying()) {
					_restChore.stop(true, 500);
				}
			} else if (_lastTurnDir != 0) {
				if (!_walkedCur && getTurnChore(_lastTurnDir)->isPlaying()) {
					_restChore.playLooping(true);
				}
			}
		}

		if (_lastTurnDir != 0 && _lastTurnDir != _currTurnDir) {
			getTurnChore(_lastTurnDir)->stop(true);
		}
		if (_currTurnDir != 0 && _currTurnDir != _lastTurnDir) {
			getTurnChore(_currTurnDir)->playLooping(true, 500);
			if (_currTurnDir == 1) {
				// When turning to the left, ensure that the components of the right turn chore
				// are fading out (or stopped).
				// This is necessary because the left turn chore typically contains both the
				// left turn and right turn keyframe components. The above call to playLooping
				// will thus start fading in both of the components, overriding the right turn's
				// fade out that was started before.
				// The left turn chore's keys will eventually stop the right turn keyframe from
				// playing, but the stopping will be instantaneous. To get a smooth transition,
				// we want to keep fading out the right turn. The chore's "stop" key will be
				// ignored when the keyframe is fading out (see KeyframeComponent::stop()).
				_rightTurnChore.stop(true);
			}
		}
	} else {
		_currTurnDir = 0;
	}

	// The rest chore might have been stopped because of a
	// StopActorChore(nil).  Restart it if so.
	if (!_walkedCur && _currTurnDir == 0 && !_restChore.isPlaying()) {
		_restChore.playLooping(g_grim->getGameType() == GType_GRIM);
	}

	_walkedLast = _walkedCur;
	_walkedCur = false;
	_lastTurnDir = _currTurnDir;
	_currTurnDir = 0;

	// Update lip syncing
	if (_lipSync) {
		int posSound;

		// While getPosIn16msTicks will return "-1" to indicate that the
		// sound is no longer playing, it is more appropriate to check first
		if (g_grim->getSpeechMode() != GrimEngine::TextOnly && g_sound->getSoundStatus(_talkSoundName.c_str()))
			posSound = g_sound->getPosIn16msTicks(_talkSoundName.c_str());
		else
			posSound = -1;
		if (posSound != -1) {
			int anim = _lipSync->getAnim(posSound);
			if (_talkAnim != anim) {
				if (anim != -1) {
					if (_talkChore[anim].isValid()) {
						stopMumbleChore();
						if (_talkAnim != -1) {
							_talkChore[_talkAnim].stop(g_grim->getGameType() == GType_MONKEY4, ActionChore::talkFadeTime);
						}
						if (g_grim->getGameType() == GType_GRIM) {
							// Run the stop_talk chore so that it resets the components
							// to the right visibility.
							stopTalking();
						} else {
							// Make sure the talk rest chore isn't playing.
							_talkChore[0].stop();
						}
						_talkAnim = anim;
						_talkChore[_talkAnim].play(g_grim->getGameType() == GType_MONKEY4, ActionChore::talkFadeTime);
					} else if (_mumbleChore.isValid() && !_mumbleChore.isPlaying()) {
						_mumbleChore.playLooping();
						_talkAnim = -1;
					}
				} else {
					stopMumbleChore();
					if (_talkAnim != -1)
						_talkChore[_talkAnim].stop(true, ActionChore::talkFadeTime);

					_talkAnim = 0;
					stopTalking();
				}
			}
		}
	}

	frameTime = (uint)(frameTime * _timeScale);
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		c->setPosRotate(_pos, _pitch, _yaw, _roll);
		int marker = c->update(frameTime);
		if (marker > 0) {
			costumeMarkerCallback(marker);
		}
		if (g_grim->getGameType() == GType_MONKEY4 && c->isChoring(false) == -1) {
			freeCostume(c);
			i = _costumeStack.erase(i);
			--i;
		}
	}

	Costume *c = getCurrentCostume();
	if (c) {
		c->animate();
	}

	if (_lookingMode && _lookAtActor != 0) {
		Actor *actor = Actor::getPool().getObject(_lookAtActor);
		if (actor)
			_lookAtVector = actor->getHeadPos();
	}

	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		(*i)->moveHead(_lookingMode, _lookAtVector);
	}
}

// Not all the talking actors are in the current set, and so on not all the talking actors
// update() is called. For example, Don when he comes out of his office after reaping Meche.
bool Actor::updateTalk(uint frameTime) {
	if (_talking) {
		// If there's no sound file then we're obviously not talking
		GrimEngine::SpeechMode m = g_grim->getSpeechMode();
		TextObject *textObject = nullptr;
		if (_sayLineText)
			textObject = TextObject::getPool().getObject(_sayLineText);
		if (m == GrimEngine::TextOnly && !textObject) {
			shutUp();
			return false;
		} else if (m != GrimEngine::TextOnly && (_talkSoundName.empty() || !g_sound->getSoundStatus(_talkSoundName.c_str()))) {
			_talkDelay -= frameTime;
			if (_talkDelay <= 0) {
				_talkDelay = 0;
				shutUp();
				return false;
			}
		}
		return true;
	}

	return false;
}

void Actor::draw() {
	if (!g_driver->isHardwareAccelerated() && g_grim->getFlagRefreshShadowMask()) {
		for (int l = 0; l < MAX_SHADOWS; l++) {
			if (!_shadowArray[l].active)
				continue;
			g_driver->setShadow(&_shadowArray[l]);
			g_driver->drawShadowPlanes();
			g_driver->setShadow(nullptr);
		}
	}

	// FIXME: if isAttached(), factor in the joint rotation as well.
	const Math::Vector3d &absPos = getWorldPos();
	if (!_costumeStack.empty()) {
		g_grim->getCurrSet()->setupLights(absPos, _inOverworld);
		if (g_grim->getGameType() == GType_GRIM) {
			Costume *costume = _costumeStack.back();
			drawCostume(costume);
		} else {
			for (Common::List<Costume *>::iterator it = _costumeStack.begin(); it != _costumeStack.end(); ++it) {
				Costume *costume = *it;
				drawCostume(costume);
			}
		}
	}

	if (_mustPlaceText) {
		Common::Point p1, p2;
		if (g_grim->getGameType() == GType_GRIM) {
			if (!_costumeStack.empty()) {
				int x1 = 1000, y1 = 1000, x2 = -1000, y2 = -1000;
				g_driver->startActorDraw(this);
				_costumeStack.back()->getBoundingBox(&x1, &y1, &x2, &y2);
				g_driver->finishActorDraw();
				p1.x = x1;
				p1.y = y1;
				p2.x = x2;
				p2.y = y2;
			}
		} else {
			g_driver->getActorScreenBBox(this, p1, p2);
		}

		TextObject *textObject = TextObject::getPool().getObject(_sayLineText);
		if (textObject) {
			if (p1.x == 1000 || p2.x == -1000 || p2.x == -1000) {
				textObject->setX(640 / 2);
				textObject->setY(463);
			} else {
				textObject->setX((p1.x + p2.x) / 2);
				textObject->setY(p1.y);
			}
			// Deletes the original text and rebuilds it with the newly placed text
			textObject->reset();
		}
		_mustPlaceText = false;
	}
}

void Actor::drawCostume(Costume *costume) {
	for (int l = 0; l < MAX_SHADOWS; l++) {
		if (!shouldDrawShadow(l))
			continue;
		g_driver->setShadow(&_shadowArray[l]);
		g_driver->setShadowMode();
		if (g_driver->isHardwareAccelerated())
			g_driver->drawShadowPlanes();
		g_driver->startActorDraw(this);
		costume->draw();
		g_driver->finishActorDraw();
		g_driver->clearShadowMode();
		g_driver->setShadow(nullptr);
	}

	// normal draw actor
	g_driver->startActorDraw(this);
	costume->draw();
	g_driver->finishActorDraw();
}

void Actor::setShadowPlane(const char *n) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].name = n;
}

void Actor::addShadowPlane(const char *n, Set *scene, int shadowId) {
	assert(shadowId != -1);

	// This needs to be an exact match, because with a substring search a search for sector
	// "shadow1" would return sector "shadow10" in set st, causing shadows not to be cast
	// on the street.
	Sector *sector = scene->getSectorByName(n);
	if (sector) {
		// Create a copy so we are sure it will not be deleted by the Set destructor
		// behind our back. This is important when Membrillo phones Velasco to tell him
		// Naranja is dead, because the scene changes back and forth few times and so
		// the scenes' sectors are deleted while they are still keeped by the actors.
		Plane p = { scene->getName(), new Sector(*sector) };
		_shadowArray[shadowId].planeList.push_back(p);
		g_grim->flagRefreshShadowMask(true);
	}
}

bool Actor::shouldDrawShadow(int shadowId) {
	Shadow *shadow = &_shadowArray[shadowId];
	if (!shadow->active)
		return false;

	// Don't draw a shadow if the shadow caster and the actor are on different sides
	// of the the shadow plane.
	Sector *sector = shadow->planeList.front().sector;
	Math::Vector3d n = sector->getNormal();
	Math::Vector3d p = sector->getVertices()[0];
	float d = -(n.x() * p.x() + n.y() * p.y() + n.z() * p.z());

	Math::Vector3d bboxPos, bboxSize;
	getBBoxInfo(bboxPos, bboxSize);
	Math::Vector3d centerOffset = bboxPos + bboxSize * 0.5f;
	p = getPos() + centerOffset;

	bool actorSide = n.x() * p.x() + n.y() * p.y() + n.z() * p.z() + d < 0.f;
	p = shadow->pos;
	bool shadowSide = n.x() * p.x() + n.y() * p.y() + n.z() * p.z() + d < 0.f;

	if (actorSide == shadowSide)
		return true;
	return false;
}

void Actor::addShadowPlane(const char *n) {
	addShadowPlane(n, g_grim->getCurrSet(), _activeShadowSlot);
}

void Actor::setActiveShadow(int shadowId) {
	assert(shadowId >= 0 && shadowId < MAX_SHADOWS);

	_activeShadowSlot = shadowId;
	_shadowArray[_activeShadowSlot].active = true;
}

void Actor::setShadowValid(int valid) {
	if (valid == -1)
		_shadowArray[_activeShadowSlot].dontNegate = true;
	else
		_shadowArray[_activeShadowSlot].dontNegate = false;
}

void Actor::setActivateShadow(int shadowId, bool state) {
	assert(shadowId >= 0 && shadowId < MAX_SHADOWS);

	_shadowArray[shadowId].active = state;
}

void Actor::setShadowPoint(const Math::Vector3d &p) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].pos = p;
}

void Actor::setShadowColor(const Color &color) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].color = color;
}

void Actor::clearShadowPlanes() {
	for (int i = 0; i < MAX_SHADOWS; i++) {
		clearShadowPlane(i);
	}
}

void Actor::clearShadowPlane(int i) {
	Shadow *shadow = &_shadowArray[i];
	while (!shadow->planeList.empty()) {
		delete shadow->planeList.back().sector;
		shadow->planeList.pop_back();
	}
	delete[] shadow->shadowMask;
	shadow->shadowMaskSize = 0;
	shadow->shadowMask = nullptr;
	shadow->active = false;
	shadow->dontNegate = false;

	g_driver->destroyShadow(shadow);
}

void Actor::putInSet(const Common::String &set) {
	if (_drawnToClean) {
		// actor was frozen and...
		if (set == "") {
			// ...is getting unbound from set.
			// This is done in game scripts to reduce the drawing
			// workload in original engine, and we should not need
			// it here. And implementing off-screen buffers is
			// highly non-trivial. Disobey so it is still drawn.
			_fakeUnbound = true;
			return;
		} else {
			// ...is getting bound to a set. Thaw and continue.
			_drawnToClean = false;
		}
	}
	_fakeUnbound = false;
	// The set should change immediately, otherwise a very rapid set change
	// for an actor will be recognized incorrectly and the actor will be lost.
	_setName = set;

	g_grim->invalidateActiveActorsList();
}

bool Actor::isDrawableInSet(const Common::String &set) const {
	return _setName == set;
}

bool Actor::isInSet(const Common::String &set) const {
	return !_fakeUnbound && _setName == set;
}

void Actor::freeCostume(Costume *costume) {
	Debug::debug(Debug::Actors, "Freeing costume %s", costume->getFilename().c_str());
	freeCostumeChore(costume, &_restChore);
	freeCostumeChore(costume, &_walkChore);
	freeCostumeChore(costume, &_leftTurnChore);
	freeCostumeChore(costume, &_rightTurnChore);
	freeCostumeChore(costume, &_mumbleChore);
	for (int i = 0; i < 10; i++)
		freeCostumeChore(costume, &_talkChore[i]);
	delete costume;
}

void Actor::freeCostumeChore(const Costume *toFree, ActionChore *chore) {
	if (chore->_costume == toFree) {
		*chore = ActionChore();
	}
}

void Actor::stopTalking() {
	// _talkChore[0] is *_stop_talk
	// Don't playLooping it, or else manny's mouth will flicker when he smokes.
	_talkChore[0].setLastFrame();
}

bool Actor::stopMumbleChore() {
	if (_mumbleChore.isPlaying()) {
		_mumbleChore.stop();
		return true;
	}

	return false;
}

void Actor::setCollisionMode(CollisionMode mode) {
	_collisionMode = mode;
}

void Actor::setCollisionScale(float scale) {
	_collisionScale = scale;
}

Math::Vector3d Actor::handleCollisionTo(const Math::Vector3d &from, const Math::Vector3d &pos) const {
	if (_collisionMode == CollisionOff) {
		return pos;
	}

	Math::Vector3d p = pos;
	Math::Vector3d moveVec = pos - _pos;
	foreach (Actor *a, Actor::getPool()) {
		if (a != this && a->isInSet(_setName) && a->isVisible()) {
			p = a->getTangentPos(from, p);
			handleCollisionWith(a, _collisionMode, &moveVec);
		}
	}
	return p;
}

Math::Vector3d Actor::getTangentPos(const Math::Vector3d &pos, const Math::Vector3d &dest) const {
	if (_collisionMode == CollisionOff) {
		return dest;
	}
	if (pos.getDistanceTo(dest) < Math::epsilon) {
		return dest;
	}
	Math::Vector3d p;
	float size;
	if (!getSphereInfo(false, size, p))
		return dest;
	Math::Vector2d p1(pos.x(), pos.y());
	Math::Vector2d p2(dest.x(), dest.y());
	if (p1.getDistanceTo(p2) < Math::epsilon) {
		return dest;
	}
	Math::Segment2d segment(p1, p2);

	// TODO: collision with Box
//  if (_collisionMode == CollisionSphere) {
	Math::Vector2d center(p.x(), p.y());

	Math::Vector2d inter;
	float distance = segment.getLine().getDistanceTo(center, &inter);

	if (distance < size && segment.containsPoint(inter)) {
		Math::Vector2d v(inter - center);
		v.normalize();
		v *= size;
		v += center;

		return Math::Vector3d(v.getX(), v.getY(), dest.z());
	}
//  } else {

//  }

	return dest;
}

void Actor::setLocalAlpha(unsigned int vertex, float alpha) {
	if (vertex >= _localAlpha.size()) {
		_localAlpha.resize(MAX(MAX_LOCAL_ALPHA_VERTICES, vertex + 1));
	}
	_localAlpha[vertex] = alpha;
}

void Actor::setLocalAlphaMode(unsigned int vertex, AlphaMode alphaMode) {
	if (vertex >= _localAlphaMode.size()) {
		_localAlphaMode.resize(MAX(MAX_LOCAL_ALPHA_VERTICES, vertex + 1));
	}
	_localAlphaMode[vertex] = alphaMode;
}

bool Actor::hasLocalAlpha() const {
	return !_localAlphaMode.empty();
}

float Actor::getLocalAlpha(unsigned int vertex) const {
	if (vertex < _localAlphaMode.size() && vertex < _localAlpha.size() && _localAlphaMode[vertex] == Actor::AlphaReplace) {
		return _localAlpha[vertex];
	} else {
		return 1.0f;
	}
}

void Actor::getBBoxInfo(Math::Vector3d &bboxPos, Math::Vector3d &bboxSize) const {
	if (g_grim->getGameType() == GType_MONKEY4) {
		EMICostume *costume = static_cast<EMICostume *>(getCurrentCostume());
		if (!costume) {
			bboxPos = Math::Vector3d(0, 0, 0);
			bboxSize = Math::Vector3d(0, 0, 0);
			return;
		}
		EMIModel *model = costume->getEMIModel();
		bboxPos = *model->_center;
		bboxSize = *model->_boxData2 - *model->_boxData;
	} else {
		Model *model = getCurrentCostume()->getModel();
		bboxPos = model->_bboxPos;
		bboxSize = model->_bboxSize;
	}
}

bool Actor::getSphereInfo(bool adjustZ, float &size, Math::Vector3d &p) const {
	if (g_grim->getGameType() == GType_MONKEY4) {
		EMICostume *costume = static_cast<EMICostume *>(getCurrentCostume());
		if (!costume) {
			Debug::warning(Debug::Actors, "Actor::getSphereInfo: actor \"%s\" has no costume", getName().c_str());
			return false;
		}
		EMIModel *model = costume->getEMIModel();
		assert(model);
		p = _pos + *(model->_center);
		// pre-scale factor of 0.8 was guessed by comparing with the original game
		size = model->_radius * _collisionScale * 0.8f;
	} else {
		Model *model = getCurrentCostume()->getModel();
		assert(model);

		p = _pos + model->_insertOffset;
		// center the sphere on the model center.
		if (adjustZ) {
			p.z() += model->_bboxSize.z() / 2.f;
		}
		size = model->_radius * _collisionScale;
	}
	return true;
}

bool Actor::handleCollisionWith(Actor *actor, CollisionMode mode, Math::Vector3d *vec) const {
	if (actor->_collisionMode == CollisionOff || actor == this) {
		return false;
	}

	if (!actor->getCurrentCostume()) {
		return false;
	}

	Math::Vector3d p1, p2;
	float size1, size2;
	// you may ask: why center the sphere of the first actor only (by setting adjustZ to true)?
	// because it seems the original does so.
	// if you change this code test this places: the rocks in lb and bv (both when booting directly in the
	// set and when coming in from another one) and the poles in xb.
	if (!this->getSphereInfo(true, size1, p1) || 
	    !actor->getSphereInfo(false, size2, p2)) {
		return false;
	}

	CollisionMode mode1 = mode;
	CollisionMode mode2 = actor->_collisionMode;

	if (mode1 == CollisionSphere && mode2 == CollisionSphere) {
		Math::Vector3d pos = p1 + *vec;
		float distance = (pos - p2).getMagnitude();
		if (distance < size1 + size2) {
			// Move the destination point so that its distance from the
			// center of the circle is size1+size2.
			Math::Vector3d v = pos - p2;
			v.normalize();
			v *= size1 + size2;
			*vec = v + p2 - p1;

			collisionHandlerCallback(actor);
			return true;
		}
	} else if (mode1 == CollisionBox && mode2 == CollisionBox) {
		warning("Collision between box and box not implemented!");
		return false;
	} else {
		Math::Vector3d bboxSize1, bboxSize2;
		Math::Vector3d bboxPos1, bboxPos2;

		// get bboxSize and bboxPos for the current and the colliding actor
		this->getBBoxInfo(bboxPos1, bboxSize1);
		actor->getBBoxInfo(bboxPos2, bboxSize2);

		Math::Rect2d rect;

		Math::Vector3d bboxPos;
		Math::Vector3d size;
		float scale;
		Math::Vector3d pos;
		Math::Vector3d circlePos;
		Math::Angle yaw;

		Math::Vector2d circle;
		float radius;

		if (mode1 == CollisionBox) {
			pos = p1 + *vec;
			bboxPos = pos + bboxPos1;
			size =  bboxSize1;
			scale = _collisionScale;
			yaw = _yaw;

			circle.setX(p2.x());
			circle.setY(p2.y());
			circlePos = p2;
			radius = size2;
		} else {
			pos = p2;
			bboxPos = p2  + bboxPos2;
			size = bboxSize2;
			scale = actor->_collisionScale;
			yaw = actor->_yaw;

			circle.setX(p1.x() + vec->x());
			circle.setY(p1.y() + vec->y());
			circlePos = p1;
			radius = size1;
		}

		rect._topLeft = Math::Vector2d(bboxPos.x(), bboxPos.y() + size.y());
		rect._topRight = Math::Vector2d(bboxPos.x() + size.x(), bboxPos.y() + size.y());
		rect._bottomLeft = Math::Vector2d(bboxPos.x(), bboxPos.y());
		rect._bottomRight = Math::Vector2d(bboxPos.x() + size.x(), bboxPos.y());

		rect.scale(scale);
		rect.rotateAround(Math::Vector2d(pos.x(), pos.y()), yaw);

		if (rect.intersectsCircle(circle, radius)) {
			Math::Vector2d center = rect.getCenter();
			// Draw a line from the center of the rect to the place the character
			// would go to.
			Math::Vector2d v = circle - center;
			v.normalize();

			Math::Segment2d edge;
			// That line intersects (usually) an edge
			rect.getIntersection(center, v, &edge);
			// Take the perpendicular of that edge
			Math::Line2d perpendicular = edge.getPerpendicular(circle);

			Math::Vector3d point;
			Math::Vector2d p;
			// If that perpendicular intersects the edge
			if (edge.intersectsLine(perpendicular, &p)) {
				Math::Vector2d direction = perpendicular.getDirection();
				direction.normalize();

				// Move from the intersection until we are at a safe distance
				Math::Vector2d point1(p - direction * radius);
				Math::Vector2d point2(p + direction * radius);

				if (center.getDistanceTo(point1) < center.getDistanceTo(point2)) {
					point = point2.toVector3d();
				} else {
					point = point1.toVector3d();
				}
			} else { //if not we're around a corner
				// Find the nearest vertex of the rect
				Math::Vector2d vertex = rect.getTopLeft();
				float distance = vertex.getDistanceTo(circle);

				Math::Vector2d other = rect.getTopRight();
				float otherDist = other.getDistanceTo(circle);
				if (otherDist < distance) {
					distance = otherDist;
					vertex = other;
				}

				other = rect.getBottomLeft();
				otherDist = other.getDistanceTo(circle);
				if (otherDist < distance) {
					distance = otherDist;
					vertex = other;
				}

				other = rect.getBottomRight();
				if (other.getDistanceTo(circle) < distance) {
					vertex = other;
				}

				// and move on a circle around it
				Math::Vector2d dst = circle - vertex;
				dst.normalize();
				dst = dst * radius;
				point = (vertex + dst).toVector3d();
			}

			float z = vec->z();
			*vec = point - circlePos;
			vec->z() = z;
			collisionHandlerCallback(actor);
			return true;
		}
	}

	return false;
}

void Actor::costumeMarkerCallback(int marker) {
	LuaObjects objects;
	objects.add(this);
	objects.add(marker);

	LuaBase::instance()->callback("costumeMarkerHandler", objects);
}

void Actor::collisionHandlerCallback(Actor *other) const {
	LuaObjects objects;
	objects.add(this);
	objects.add(other);

	LuaBase::instance()->callback("collisionHandler", objects);

	LuaObjects objects2;
	objects2.add(other);
	objects2.add(this);
	LuaBase::instance()->callback("collisionHandler", objects2);
}

const Math::Matrix4 Actor::getFinalMatrix() const {
	// Defaults to identity (no rotation)
	Math::Matrix4 m;

	// Add the rotation and translation from the parent actor, if this actor is attached
	if (isAttached()) {
		Actor *attachedActor = Actor::getPool().getObject(_attachedActor);
		m = attachedActor->getFinalMatrix();

		// If this actor is attached to a joint, add that rotation
		EMICostume *cost = static_cast<EMICostume *>(attachedActor->getCurrentCostume());
		if (cost && cost->_emiSkel && cost->_emiSkel->_obj) {
			Joint *j = cost->_emiSkel->_obj->getJointNamed(_attachedJoint);
			m = m * j->_finalMatrix;
		}
	}

	// Translate by this actor's position
	Math::Vector3d pp = getPos();
	Math::Matrix4 t;
	t.setToIdentity();
	t.setPosition(pp);
	m = m * t;

	// Scaling could be applied here as follows:
	//
	// const float &scale = getScale();
	// t.setToIdentity();
	// t.setValue(3, 3, 1.0 / scale);
	// m = m * t;
	//
	// However, actor's _scale is only changed via the lua call SetActorScale
	// which is not used in EMI. Actor::getFinalMatrix() is only used for EMI
	// so the additional scaling can be omitted.

	// Finish with this actor's rotation
	Math::Matrix4 rotMat(getRoll(), getYaw(), getPitch(), Math::EO_ZYX);
	m = m * rotMat;
	return  m;
}

Math::Vector3d Actor::getWorldPos() const {
	if (! isAttached())
		return getPos();

	return getFinalMatrix().getPosition();
}

Math::Quaternion Actor::getRotationQuat() const {
	if (g_grim->getGameType() == GType_MONKEY4) {
		const Math::Matrix4 m = getFinalMatrix();
		return Math::Quaternion(m).inverse();
	} else {
		return Math::Quaternion::fromEuler(_yaw, _pitch, _roll, Math::EO_ZXY).inverse();
	}
}

Math::Vector3d Actor::getHeadPos() const {
	if (g_grim->getGameType() == GType_GRIM) {
		for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
			int headJoint = (*i)->getHeadJoint();
			if (headJoint == -1)
				continue;

			ModelNode *allNodes = (*i)->getModelNodes();
			ModelNode *node = allNodes + headJoint;

			node->_needsUpdate = true;
			ModelNode *root = node;
			while (root->_parent) {
				root = root->_parent;
				root->_needsUpdate = true;
			}

			Math::Matrix4 matrix;
			matrix.setPosition(_pos);
			matrix.buildFromEuler(_yaw, _pitch, _roll, Math::EO_ZXY);
			root->setMatrix(matrix);
			root->update();

			return node->_pivotMatrix.getPosition();
		}
	}

	return getWorldPos();
}

void Actor::setSortOrder(const int order) {
	_sortOrder = order;

	// If this actor is attached to another actor, we'll use the
	// explicitly specified sort order instead of the parent actor's
	// sort order from now on.
	if (_useParentSortOrder)
		_useParentSortOrder = false;
}

int Actor::getSortOrder() const {
	return _sortOrder;
}

int Actor::getEffectiveSortOrder() const {
	if (_useParentSortOrder && _attachedActor != 0) {
		Actor *attachedActor = Actor::getPool().getObject(_attachedActor);
		return attachedActor->getEffectiveSortOrder();
	}
	return _sectorSortOrder >= 0 ? _sectorSortOrder : getSortOrder();
}

void Actor::activateShadow(bool active, const char *shadowName) {
	Set *set = g_grim->getCurrSet();
	if (!set) {
		warning("Actor %s trying to activate shadow to null Set", getName().c_str());
		return;
	}
	if (!shadowName) {
		for (int i = 0; i < set->getShadowCount(); ++i) {
			activateShadow(active, set->getShadow(i));
		}
	} else {
		SetShadow *shadow = set->getShadowByName(shadowName);
		if (shadow)
			activateShadow(active, shadow);
	}
}

void Actor::activateShadow(bool active, SetShadow *setShadow) {
	int shadowId = -1;
	for (int i = 0; i < MAX_SHADOWS; i++) {
		if (setShadow->_name.equals(_shadowArray[i].name)) {
			shadowId = i;
			break;
		}
	}

	if (shadowId == -1) {
		for (int i = 0; i < MAX_SHADOWS; i++) {
			if (!_shadowArray[i].active) {
				shadowId = i;
				break;
			}
		}
	}

	if (shadowId == -1) {
		warning("Actor %s trying to activate shadow %s, but all shadow slots are in use", getName().c_str(), setShadow->_name.c_str());
		return;
	}

	clearShadowPlane(shadowId);
	setActivateShadow(shadowId, active);

	if (active) {
		setActiveShadow(shadowId);
		setShadowPoint(setShadow->_shadowPoint);
		setShadowPlane(setShadow->_name.c_str());
		setShadowColor(setShadow->_color);
		setShadowValid(-1); // Don't negate the normal.

		Common::List<Common::String>::iterator it;
		for (it = setShadow->_sectorNames.begin(); it != setShadow->_sectorNames.end(); ++it) {
			addShadowPlane((*it).c_str(), g_grim->getCurrSet(), shadowId);
		}
	}
}

void Actor::attachToActor(Actor *parent, const char *joint) {
	assert(parent != nullptr);
	// No need to attach if we're already attached to this parent
	if (parent->getId() == _attachedActor)
		return;
	// If we're attached to a different parent, detach first
	if (_attachedActor != 0)
		detach();

	// Find the new rotation relative to the parent actor's rotation
	// Note: Any joint rotation is a part of the parent actor's rotation Quat
	Math::Quaternion newRot = getRotationQuat().inverse() * parent->getRotationQuat();

	// Find the new position coordinates
	Math::Matrix4 parentMatrix = parent->getFinalMatrix();

	// If the parent has a skeleton, check if it has the requested joint
	// Some models (pile o' boulders) don't have a skeleton
	Common::String jointStr = joint ? joint : "";
	EMICostume *cost = static_cast<EMICostume *>(parent->getCurrentCostume());
	if (cost && cost->_emiSkel && cost->_emiSkel->_obj) {
		assert(cost->_emiSkel->_obj->hasJoint(jointStr));

		// Add the rotation from the attached actor's joint
		Joint *j = cost->_emiSkel->_obj->getJointNamed(_attachedJoint);
		newRot = newRot.inverse() * j->_finalQuat;

		// Get the final position coordinates
		_pos = _pos - j->_finalMatrix.getPosition();
		j->_finalMatrix.transpose();
		j->_finalMatrix.transform(&_pos, true);
	}

	// Get the final rotation euler coordinates
	newRot.getEuler(&_roll, &_yaw, &_pitch, Math::EO_ZYX);

	// Get the final position coordinates
	_pos = _pos - parentMatrix.getPosition();
	parentMatrix.transpose();
	parentMatrix.transform(&_pos, true);

	// Save the attachement info
	_attachedActor = parent->getId();
	_attachedJoint = jointStr;

	// Use the parent actor's sort order.
	_useParentSortOrder = true;
}

void Actor::detach() {
	if (!isAttached())
		return;

	// Replace our sort order with the parent actor's sort order. Note
	// that we do this even if a sort order was explicitly specified for
	// the actor during the time it was attached (in which case the actor
	// doesn't respect the parent actor's sort order). This seems weird,
	// but matches the behavior of the original engine.
	Actor *attachedActor = Actor::getPool().getObject(_attachedActor);
	_sortOrder = attachedActor->getEffectiveSortOrder();
	_useParentSortOrder = false;

	// Position and rotate the actor in relation to the world coords
	setPos(getWorldPos());
	Math::Quaternion q = getRotationQuat();
	q.inverse().getEuler(&_roll, &_yaw, &_pitch, Math::EO_ZYX);

	// Remove the attached actor
	_attachedActor = 0;
	_attachedJoint = "";
}

void Actor::drawToCleanBuffer() {
	_drawnToClean = true;
}

MaterialPtr Actor::findMaterial(const Common::String &name) {
	Common::String fixedName = g_resourceloader->fixFilename(name, false);
	Common::List<MaterialPtr>::iterator it = _materials.begin();
	for (; it != _materials.end(); ++it) {
		if (*it) {
			if ((*it)->getFilename() == fixedName) {
				return *it;
			}
		} else {
			it = _materials.erase(it);
			--it;
		}
	}
	return (MaterialPtr)nullptr;
}

MaterialPtr Actor::loadMaterial(const Common::String &name, bool clamp) {
	MaterialPtr mat = findMaterial(name);
	if (!mat) {
		mat = g_resourceloader->loadMaterial(name.c_str(), nullptr, clamp);
		// Note: We store a weak reference.
		_materials.push_back(mat);
		mat->dereference();
	}
	return mat;
}

unsigned const int Actor::ActionChore::fadeTime = 150;
unsigned const int Actor::ActionChore::talkFadeTime = 50;

Actor::ActionChore::ActionChore() :
	_costume(nullptr),
	_chore(-1) {

}

Actor::ActionChore::ActionChore(Costume *cost, int chore) :
	_costume(cost),
	_chore(chore) {

}

void Actor::ActionChore::play(bool fade, unsigned int time) {
	if (isValid()) {
		if (fade) {
			_costume->playChore(_chore, time);
		} else {
			_costume->playChore(_chore);
		}
	}
}

void Actor::ActionChore::playLooping(bool fade, unsigned int time) {
	if (isValid()) {
		if (fade) {
			_costume->playChoreLooping(_chore, time);
		} else {
			_costume->playChoreLooping(_chore);
		}
	}
}

void Actor::ActionChore::stop(bool fade, unsigned int time) {
	if (isValid()) {
		if (fade) {
			_costume->stopChore(_chore, time);
		} else {
			_costume->stopChore(_chore);
		}
	}
}

void Actor::ActionChore::setLastFrame() {
	if (isValid()) {
		_costume->setChoreLastFrame(_chore);
	}
}

bool Actor::ActionChore::isPlaying() const {
	return (isValid() && _costume->isChoring(_chore, false) >= 0);
}

void Actor::ActionChore::saveState(SaveGame *savedState) const {
	if (_costume) {
		savedState->writeBool(true);
		savedState->writeString(_costume->getFilename());
	} else {
		savedState->writeBool(false);
	}
	savedState->writeLESint32(_chore);
}

void Actor::ActionChore::restoreState(SaveGame *savedState, Actor *actor) {
	if (savedState->readBool()) {
		Common::String fname = savedState->readString();
		_costume = actor->findCostume(fname);
	} else {
		_costume = nullptr;
	}
	_chore = savedState->readLESint32();
}

} // end of namespace Grim
