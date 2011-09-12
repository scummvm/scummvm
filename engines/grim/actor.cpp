/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink

#include "graphics/line3d.h"
#include "graphics/rect2d.h"

#include "engines/grim/debug.h"
#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/movie/movie.h"
#include "engines/grim/imuse/imuse.h"
#include "engines/grim/lua.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"
#include "engines/grim/scene.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/model.h"

namespace Grim {

int g_winX1, g_winY1, g_winX2, g_winY2;

Actor::Actor(const Common::String &actorName) :
		PoolObject<Actor, MKTAG('A', 'C', 'T', 'R')>(), _name(actorName), _setName(""),
		_talkColor(PoolColor::getPool()->getObject(2)), _pos(0, 0, 0),
		// Some actors don't set walk and turn rates, so we default the
		// _turnRate so Doug at the cat races can turn and we set the
		// _walkRate so Glottis at the demon beaver entrance can walk and
		// so Chepito in su.set
		_pitch(0), _yaw(0), _roll(0), _walkRate(0.3f), _turnRate(100.0f),
		_reflectionAngle(80),
		_visible(true), _lipSync(NULL), _turning(false), _walking(false),
		_restCostume(NULL), _restChore(-1),
		_walkCostume(NULL), _walkChore(-1), _walkedLast(false), _walkedCur(false),
		_turnCostume(NULL), _leftTurnChore(-1), _rightTurnChore(-1),
		_lastTurnDir(0), _currTurnDir(0),
		_mumbleCostume(NULL), _mumbleChore(-1), _sayLineText(0) {
	_lookingMode = false;
	_lookAtRate = 200;
	_constrain = false;
	_talkSoundName = "";
	_activeShadowSlot = -1;
	_shadowArray = new Shadow[5];
	_winX1 = _winY1 = 1000;
	_winX2 = _winY2 = -1000;
	_toClean = false;
	_running = false;
	_scale = 1.f;
	_timeScale = 1.f;
	_mustPlaceText = false;
	_collisionMode = CollisionOff;
	_collisionScale = 1.f;

	for (int i = 0; i < 5; i++) {
		_shadowArray[i].active = false;
		_shadowArray[i].dontNegate = false;
		_shadowArray[i].shadowMask = NULL;
		_shadowArray[i].shadowMaskSize = 0;
	}

	for (int i = 0; i < 10; i++) {
		_talkCostume[i] = NULL;
		_talkChore[i] = -1;
	}
}

Actor::Actor() :
	PoolObject<Actor, MKTAG('A', 'C', 'T', 'R')>() {

	_shadowArray = new Shadow[5];
	_winX1 = _winY1 = 1000;
	_winX2 = _winY2 = -1000;
	_toClean = false;
	_running = false;
	_scale = 1.f;
	_timeScale = 1.f;
	_mustPlaceText = false;
	_collisionMode = CollisionOff;
	_collisionScale = 1.f;

	for (int i = 0; i < 5; i++) {
		_shadowArray[i].active = false;
		_shadowArray[i].dontNegate = false;
		_shadowArray[i].shadowMask = NULL;
		_shadowArray[i].shadowMaskSize = 0;
	}
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
}

void Actor::saveState(SaveGame *savedState) const {
	// store actor name
	savedState->writeString(_name);
	savedState->writeString(_setName);

	if (_talkColor) {
		savedState->writeLEUint32(_talkColor->getId());
	} else {
		savedState->writeLEUint32(0);
	}

	savedState->writeVector3d(_pos);

	savedState->writeFloat(_pitch);
	savedState->writeFloat(_yaw);
	savedState->writeFloat(_roll);
	savedState->writeFloat(_walkRate);
	savedState->writeFloat(_turnRate);
	savedState->writeLESint32(_constrain);
	savedState->writeFloat(_reflectionAngle);
	savedState->writeLESint32(_visible);
	savedState->writeLESint32(_lookingMode),
	savedState->writeFloat(_scale);
	savedState->writeFloat(_timeScale);

	savedState->writeString(_talkSoundName);

	if (_lipSync) {
		savedState->writeLEUint32(1);
		savedState->writeString(_lipSync->getFilename());
	} else {
		savedState->writeLEUint32(0);
	}

	savedState->writeLESint32(_costumeStack.size());
	for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		savedState->writeString(c->getFilename());
		Costume *pc = c->getPreviousCostume();
		int depth = 0;
		while (pc) {
			++depth;
			pc = pc->getPreviousCostume();
		}
		savedState->writeLEUint32(depth);
		pc = c->getPreviousCostume();
		for (int j = 0; j < depth; ++j) { //save the previousCostume hierarchy
			savedState->writeString(pc->getFilename());
			pc = pc->getPreviousCostume();
		}
		c->saveState(savedState);
	}

	savedState->writeLESint32(_turning);
	savedState->writeFloat(_destYaw);

	savedState->writeLESint32(_walking);
	savedState->writeVector3d(_destPos);

	if (_restCostume) {
		savedState->writeLEUint32(1);
		savedState->writeString(_restCostume->getFilename());
	} else {
		savedState->writeLEUint32(0);
	}
	savedState->writeLESint32(_restChore);

	if (_walkCostume) {
		savedState->writeLEUint32(1);
		savedState->writeString(_walkCostume->getFilename());
	} else {
		savedState->writeLEUint32(0);
	}
	savedState->writeLESint32(_walkChore);
	savedState->writeLESint32(_walkedLast);
	savedState->writeLESint32(_walkedCur);

	if (_turnCostume) {
		savedState->writeLEUint32(1);
		savedState->writeString(_turnCostume->getFilename());
	} else {
		savedState->writeLEUint32(0);
	}
	savedState->writeLESint32(_leftTurnChore);
	savedState->writeLESint32(_rightTurnChore);
	savedState->writeLESint32(_lastTurnDir);
	savedState->writeLESint32(_currTurnDir);

	for (int i = 0; i < 10; ++i) {
		if (_talkCostume[i]) {
			savedState->writeLEUint32(1);
			savedState->writeString(_talkCostume[i]->getFilename());
		} else {
			savedState->writeLEUint32(0);
		}
		savedState->writeLESint32(_talkChore[i]);
	}
	savedState->writeLESint32(_talkAnim);

	if (_mumbleCostume) {
		savedState->writeLEUint32(1);
		savedState->writeString(_mumbleCostume->getFilename());
	} else {
		savedState->writeLEUint32(0);
	}
	savedState->writeLESint32(_mumbleChore);

	for (int i = 0; i < 5; ++i) {
		Shadow &shadow = _shadowArray[i];
		savedState->writeString(shadow.name);

		savedState->writeVector3d(shadow.pos);

		savedState->writeLESint32(shadow.planeList.size());
		// Cannot use g_grim->getCurrScene() here because an actor can have walk planes
		// from other scenes. It happens e.g. when Membrillo calls Velasco to tell him
		// Naranja is dead.
		for (SectorListType::iterator j = shadow.planeList.begin(); j != shadow.planeList.end(); ++j) {
			Plane &p = *j;
			savedState->writeString(p.setName);
			savedState->writeString(p.sector->getName());
		}

		savedState->writeLESint32(shadow.shadowMaskSize);
		savedState->write(shadow.shadowMask, shadow.shadowMaskSize);
		savedState->writeLESint32(shadow.active);
		savedState->writeLESint32(shadow.dontNegate);
	}
	savedState->writeLESint32(_activeShadowSlot);

	savedState->writeLEUint32(_sayLineText);

	savedState->writeVector3d(_lookAtVector);
	savedState->writeFloat(_lookAtRate);

	savedState->writeLESint32(_winX1);
	savedState->writeLESint32(_winY1);
	savedState->writeLESint32(_winX2);
	savedState->writeLESint32(_winY2);

	savedState->writeLESint32(_path.size());
	for (Common::List<Graphics::Vector3d>::const_iterator i = _path.begin(); i != _path.end(); ++i) {
		savedState->writeVector3d(*i);
	}
}

bool Actor::restoreState(SaveGame *savedState) {
	for (Common::List<Costume *>::const_iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		delete *i;
	}
	_costumeStack.clear();

	// load actor name
	_name = savedState->readString();
	_setName = savedState->readString();

	_talkColor = PoolColor::getPool()->getObject(savedState->readLEUint32());

	_pos                = savedState->readVector3d();
	_pitch              = savedState->readFloat();
	_yaw                = savedState->readFloat();
	_roll               = savedState->readFloat();
	_walkRate           = savedState->readFloat();
	_turnRate           = savedState->readFloat();
	_constrain          = savedState->readLESint32();
	_reflectionAngle    = savedState->readFloat();
	_visible            = savedState->readLESint32();
	_lookingMode        = savedState->readLESint32();
	_scale              = savedState->readFloat();
	_timeScale          = savedState->readFloat();

	_talkSoundName 		= savedState->readString();

	if (savedState->readLEUint32()) {
		Common::String fn = savedState->readString();
		_lipSync = g_resourceloader->getLipSync(fn);
	} else {
		_lipSync = NULL;
	}

	int32 size = savedState->readLESint32();
	for (int32 i = 0; i < size; ++i) {
		Common::String fname = savedState->readString();
		const int depth = savedState->readLEUint32();
		Costume *pc = NULL;
		if (depth > 0) {	//build all the previousCostume hierarchy
			Common::String *names = new Common::String[depth];
			for (int j = 0; j < depth; ++j) {
				names[j] = savedState->readString();
			}
			for (int j = depth - 1; j >= 0; --j) {
				pc = findCostume(names[j]);
				if (!pc) {
					pc = g_resourceloader->loadCostume(names[j], pc);
				}
			}
			delete[] names;
		}

		Costume *c = g_resourceloader->loadCostume(fname, pc);
		c->restoreState(savedState);
		_costumeStack.push_back(c);
	}

	_turning = savedState->readLESint32();
	_destYaw = savedState->readFloat();

	_walking = savedState->readLESint32();
	_destPos = savedState->readVector3d();

	if (savedState->readLEUint32()) {
		Common::String fname = savedState->readString();
		_restCostume = findCostume(fname);
	} else {
		_restCostume = NULL;
	}
	_restChore = savedState->readLESint32();

	if (savedState->readLEUint32()) {
		Common::String fname = savedState->readString();
		_walkCostume = findCostume(fname);
	} else {
		_walkCostume = NULL;
	}

	_walkChore = savedState->readLESint32();
	_walkedLast = savedState->readLESint32();
	_walkedCur = savedState->readLESint32();

	if (savedState->readLEUint32()) {
		Common::String fname = savedState->readString();
		_turnCostume = findCostume(fname);
	} else {
		_turnCostume = NULL;
	}
	_leftTurnChore = savedState->readLESint32();
	_rightTurnChore = savedState->readLESint32();
	_lastTurnDir = savedState->readLESint32();
	_currTurnDir = savedState->readLESint32();

	for (int i = 0; i < 10; ++i) {
		if (savedState->readLEUint32()) {
			Common::String fname = savedState->readString();
			_talkCostume[i] = findCostume(fname);
		} else {
			_talkCostume[i] = NULL;
		}
		_talkChore[i] = savedState->readLESint32();
	}
	_talkAnim = savedState->readLESint32();

	if (savedState->readLEUint32()) {
		Common::String fname = savedState->readString();
		_mumbleCostume = findCostume(fname);
	} else {
		_mumbleCostume = NULL;
	}
	_mumbleChore = savedState->readLESint32();

	for (int i = 0; i < 5; ++i) {
		Shadow &shadow = _shadowArray[i];
		shadow.name = savedState->readString();

		shadow.pos = savedState->readVector3d();

		size = savedState->readLESint32();
		shadow.planeList.clear();
		Scene *scene = NULL;
		for (int j = 0; j < size; ++j) {
			Common::String setName = savedState->readString();
			Common::String secName = savedState->readString();
			if (!scene || scene->getName() != setName) {
				scene = g_grim->findScene(setName);
			}
			if (scene) {
				addShadowPlane(secName.c_str(), scene, i);
			} else {
				warning("%s: No scene \"%s\" found, cannot restore shadow on sector \"%s\"", getName().c_str(), setName.c_str(), secName.c_str());
			}
		}

		shadow.shadowMaskSize = savedState->readLESint32();
		delete[] shadow.shadowMask;
		if (shadow.shadowMaskSize > 0) {
			shadow.shadowMask = new byte[shadow.shadowMaskSize];
			savedState->read(shadow.shadowMask, shadow.shadowMaskSize);
		} else {
			shadow.shadowMask = NULL;
		}
		shadow.active = savedState->readLESint32();
		shadow.dontNegate = savedState->readLESint32();
	}
	_activeShadowSlot = savedState->readLESint32();

	_sayLineText = savedState->readLEUint32();

	_lookAtVector = savedState->readVector3d();
	_lookAtRate = savedState->readFloat();

	_winX1 = savedState->readLESint32();
	_winY1 = savedState->readLESint32();
	_winX2 = savedState->readLESint32();
	_winY2 = savedState->readLESint32();

	size = savedState->readLESint32();
	for (int i = 0; i < size; ++i) {
		_path.push_back(savedState->readVector3d());
	}

	return true;
}

void Actor::setYaw(float yawParam) {
	// While the program correctly handle yaw angles outside
	// of the range [0, 360), proper convention is to roll
	// these values over correctly
	if (yawParam >= 360.0)
		_yaw = yawParam - 360;
	else if (yawParam < 0.0)
		_yaw = yawParam + 360;
	else
		_yaw = yawParam;
}

void Actor::setRot(float pitchParam, float yawParam, float rollParam) {
	_pitch = pitchParam;
	setYaw(yawParam);
	_roll = rollParam;
	_turning = false;
}

void Actor::setPos(Graphics::Vector3d position) {
	_walking = false;
	_pos = position;

	// Don't allow positions outside the sectors.
	// This is necessary after solving the tree pump puzzle, when the bone
	// wagon returns to the signopost set.
	if (_constrain && !_walking) {
		g_grim->getCurrScene()->findClosestSector(_pos, NULL, &_pos);
	}
}

void Actor::turnTo(float pitchParam, float yawParam, float rollParam) {
	_pitch = pitchParam;
	_roll = rollParam;
	if (_yaw != yawParam) {
		_turning = true;
		_destYaw = yawParam;
	} else
		_turning = false;
}

void Actor::walkTo(const Graphics::Vector3d &p) {
	if (p == _pos)
		_walking = false;
	else {
		_walking = true;
		_destPos = p;
		_path.clear();

		if (_constrain) {
			g_grim->getCurrScene()->findClosestSector(p, NULL, &_destPos);

			Common::List<PathNode *> openList;
			Common::List<PathNode *> closedList;

			PathNode *start = new PathNode;
			start->parent = NULL;
			start->pos = _pos;
			start->dist = 0.f;
			start->cost = 0.f;
			openList.push_back(start);
			g_grim->getCurrScene()->findClosestSector(_pos, &start->sect, NULL);

			Common::List<Sector *> sectors;
			for (int i = 0; i < g_grim->getCurrScene()->getSectorCount(); ++i) {
				Sector *s = g_grim->getCurrScene()->getSectorBase(i);
				int type = s->getType();
				if ((type == Sector::WalkType || type == Sector::HotType || type == Sector::FunnelType) && s->isVisible()) {
					sectors.push_back(s);
				}
			}

			Sector *endSec = NULL;
			g_grim->getCurrScene()->findClosestSector(_destPos, &endSec, NULL);

			do {
				PathNode *node = NULL;
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

				if (sector == endSec) {
					PathNode *n = closedList.back();
					while (n) {
						// Don't put the start position in the list, or else
						// the first angle calculated in updateWalk() will be
						// meaningless.
						if (n->pos == _pos) {
							break;
						}
						_path.push_back(n->pos);
						n = n->parent;
					}

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

					Common::List<Graphics::Line3d> bridges = sector->getBridgesTo(s);
					if (bridges.empty())
						continue; // The sectors are not adjacent.

					Graphics::Vector3d closestPoint = s->getClosestPoint(_destPos);
					Graphics::Vector3d best;
					float bestDist = 1e6f;
					Graphics::Line3d l(node->pos, closestPoint);
					while (!bridges.empty()) {
						Graphics::Line3d bridge = bridges.back();
						Graphics::Vector3d pos;
						if (!bridge.intersectLine2d(l, &pos)) {
							pos = bridge.middle();
						}
						float dist = (pos - closestPoint).magnitude();
						if (dist < bestDist) {
							bestDist = dist;
							best = pos;
						}
						bridges.pop_back();
					}

					PathNode *n = NULL;
					for (Common::List<PathNode *>::iterator j = openList.begin(); j != openList.end(); ++j) {
						if ((*j)->sect == s) {
							n = *j;
							break;
						}
					}
					if (n) {
						float newCost = node->cost + (best - node->pos).magnitude();
						if (newCost < n->cost) {
							n->cost = newCost;
							n->parent = node;
							n->pos = best;
							n->dist = (n->pos - _destPos).magnitude();
						}
					} else {
						n = new PathNode;
						n->parent = node;
						n->sect = s;
						n->pos = best;
						n->dist = (n->pos - _destPos).magnitude();
						n->cost = node->cost + (n->pos - node->pos).magnitude();
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
		}

		_path.push_front(_destPos);
	}
}

bool Actor::isWalking() const {
	return _walkedLast || _walkedCur || _walking;
}

bool Actor::isTurning() const {
	if (_turning)
		return true;

	if (_lastTurnDir != 0 || _currTurnDir != 0)
		return true;

	return false;
}

void Actor::moveTo(const Graphics::Vector3d &pos) {
	// This is necessary for collisions in set hl to work, since
	// Manny's collision mode isn't set.
	if (_collisionMode == CollisionOff) {
		_collisionMode = CollisionSphere;
	}

	Graphics::Vector3d v = pos - _pos;
	for (Actor::Pool::Iterator i = getPool()->getBegin(); i != getPool()->getEnd(); ++i) {
		Actor *a = i->_value;
		if (a != this && a->isInSet(_setName) && a->isVisible()) {
			collidesWith(a, &v);
		}
	}
	_pos += v;
}

void Actor::walkForward() {
	float dist = g_grim->getPerSecond(_walkRate);
	_walking = false;

	float yaw_rad = _yaw * (LOCAL_PI / 180.f), pitch_rad = _pitch * (LOCAL_PI / 180.f);
	//float yaw;
	Graphics::Vector3d forwardVec(-sin(yaw_rad) * cos(pitch_rad),
		cos(yaw_rad) * cos(pitch_rad), sin(pitch_rad));

	if (! _constrain) {
		_pos += forwardVec * dist;
		_walkedCur = true;
		return;
	}

	if (dist < 0) {
		dist = -dist;
		forwardVec = -forwardVec;
	}

	Sector *currSector = NULL, *prevSector = NULL;
	Sector::ExitInfo ei;

	g_grim->getCurrScene()->findClosestSector(_pos, &currSector, &_pos);
	if (!currSector) { // Shouldn't happen...
		moveTo(_pos + forwardVec * dist);
		_walkedCur = true;
		return;
	}

	while (currSector) {
		prevSector = currSector;
		Graphics::Vector3d puckVec = currSector->getProjectionToPuckVector(forwardVec);
		puckVec /= puckVec.magnitude();
		currSector->getExitInfo(_pos, puckVec, &ei);
		float exitDist = (ei.exitPoint - _pos).magnitude();
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
		currSector = g_grim->getCurrScene()->findPointSector(ei.exitPoint + (float)0.0001 * puckVec, Sector::WalkType);
		if (currSector == prevSector)
			break;
	}

	ei.angleWithEdge *= (float)(180.f / LOCAL_PI);
	int turnDir = 1;
	if (ei.angleWithEdge > 90) {
		ei.angleWithEdge = 180 - ei.angleWithEdge;
		ei.edgeDir = -ei.edgeDir;
		turnDir = -1;
	}
	if (ei.angleWithEdge > _reflectionAngle)
		return;

	ei.angleWithEdge += (float)0.1;
	float turnAmt = g_grim->getPerSecond(_turnRate) * 5.;
	if (turnAmt > ei.angleWithEdge)
		turnAmt = ei.angleWithEdge;
	setYaw(_yaw + turnAmt * turnDir);
}

Graphics::Vector3d Actor::getPuckVector() const {
	float yaw_rad = _yaw * (LOCAL_PI / 180.f);
	Graphics::Vector3d forwardVec(-sin(yaw_rad), cos(yaw_rad), 0);

	Sector *sector = g_grim->getCurrScene()->findPointSector(_pos, Sector::WalkType);
	if (!sector)
		return forwardVec;
	else
		return sector->getProjectionToPuckVector(forwardVec);
}

void Actor::setRestChore(int chore, Costume *cost) {
	if (_restCostume == cost && _restChore == chore)
		return;

	if (_restChore >= 0)
		_restCostume->stopChore(_restChore);

	_restCostume = cost;
	_restChore = chore;

	if (_restChore >= 0)
		_restCostume->playChoreLooping(_restChore);
}

void Actor::setWalkChore(int chore, Costume *cost) {
	if (_walkCostume == cost && _walkChore == chore)
		return;

	if (_walkChore >= 0 && _walkCostume->isChoring(_walkChore, false) >= 0) {
		_walkCostume->fadeChoreOut(_walkChore, 150);
		_walkCostume->stopChore(_walkChore);
		if (_restChore >= 0) {
			_restCostume->playChoreLooping(_restChore);
			_restCostume->fadeChoreIn(_restChore, 150);
		}
	}

	_walkCostume = cost;
	_walkChore = chore;
}

void Actor::setTurnChores(int left_chore, int right_chore, Costume *cost) {
	if (_turnCostume == cost && _leftTurnChore == left_chore &&
		_rightTurnChore == right_chore)
		return;

	if (_leftTurnChore >= 0) {
		_turnCostume->stopChore(_leftTurnChore);
		_turnCostume->stopChore(_rightTurnChore);
	}

	_turnCostume = cost;
	_leftTurnChore = left_chore;
	_rightTurnChore = right_chore;

	if ((left_chore >= 0 && right_chore < 0) || (left_chore < 0 && right_chore >= 0))
		error("Unexpectedly got only one turn chore");
}

void Actor::setTalkChore(int index, int chore, Costume *cost) {
	if (index < 1 || index > 10)
		error("Got talk chore index out of range (%d)", index);

	index--;

	if (_talkCostume[index] == cost && _talkChore[index] == chore)
		return;

	if (_talkChore[index] >= 0)
		_talkCostume[index]->stopChore(_talkChore[index]);

	_talkCostume[index] = cost;
	_talkChore[index] = chore;
}

void Actor::setMumbleChore(int chore, Costume *cost) {
	if (_mumbleChore >= 0)
		_mumbleCostume->stopChore(_mumbleChore);

	_mumbleCostume = cost;
	_mumbleChore = chore;
}

void Actor::turn(int dir) {
	_walking = false;
	float delta = g_grim->getPerSecond(_turnRate) * dir;
	setYaw(_yaw + delta);
	_currTurnDir = dir;
}

float Actor::getYawTo(const Actor &a) const {
	float yaw_rad = _yaw * (LOCAL_PI / 180.f);
	Graphics::Vector3d forwardVec(-sin(yaw_rad), cos(yaw_rad), 0);
	Graphics::Vector3d delta = a.getPos() - _pos;
	delta.z() = 0;

	return angle(forwardVec, delta) * (180.f / LOCAL_PI);
}

float Actor::getYawTo(Graphics::Vector3d p) const {
	Graphics::Vector3d dpos = p - _pos;

	if (dpos.x() == 0 && dpos.y() == 0)
		return 0;
	else
		return atan2(-dpos.x(), dpos.y()) * (180.f / LOCAL_PI);
}

void Actor::sayLine(const char *msgId, bool background) {
	assert(msgId);

	char id[50];
	Common::String msg = parseMsgText(msgId, id);

	if (msgId[0] == 0) {
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
	soundName += ".wav";

	if (_talkSoundName == soundName)
		return;

	if (g_imuse->getSoundStatus(_talkSoundName.c_str()) || msg.empty())
		shutUp();

	_talkSoundName = soundName;
	if (g_grim->getSpeechMode() != GrimEngine::TextOnly) {
		if (g_imuse->startVoice(_talkSoundName.c_str()) && g_grim->getCurrScene()) {
			g_grim->getCurrScene()->setSoundPosition(_talkSoundName.c_str(), _pos);
		}
	}

	// If the actor is clearly not visible then don't try to play the lip sync
	if (_visible && (!g_movie->isPlaying() || g_grim->getMode() == ENGINE_MODE_NORMAL)) {
		Common::String soundLip = id;
		soundLip += ".lip";

		if (_talkChore[0] >= 0 && _talkCostume[0]->isChoring(_talkChore[0], true) < 0) {
			// _talkChore[0] is *_stop_talk
			_talkCostume[0]->setChoreLastFrame(_talkChore[0]);
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
		if (!_lipSync && _mumbleChore != -1)
			_mumbleCostume->playChoreLooping(_mumbleChore);

		_talkAnim = -1;
	}

	g_grim->setTalkingActor(this);

	if (_sayLineText) {
		delete TextObject::getPool()->getObject(_sayLineText);
		_sayLineText = 0;
	}

	if (!msg.empty()) {
		GrimEngine::SpeechMode m = g_grim->getSpeechMode();
		if (!g_grim->_sayLineDefaults.getFont() || m == GrimEngine::VoiceOnly)
			return;

		if (g_grim->getMode() == ENGINE_MODE_SMUSH)
			TextObject::getPool()->deleteObjects();

		TextObject *textObject = new TextObject(false, true);
		textObject->setDefaults(&g_grim->_sayLineDefaults);
		textObject->setFGColor(_talkColor);
		if (m == GrimEngine::TextOnly || g_grim->getMode() == ENGINE_MODE_SMUSH) {
			textObject->setDuration(500 + msg.size() * 15 * (11 - g_grim->getTextSpeed()));
		}
		if (g_grim->getMode() == ENGINE_MODE_SMUSH) {
			textObject->setX(640 / 2);
			textObject->setY(456);
		} else {
			if (_visible && isInSet(g_grim->getCurrScene()->getName())) {
				_mustPlaceText = true;
			} else {
				_mustPlaceText = false;
				textObject->setX(640 / 2);
				textObject->setY(463);
			}
		}
		textObject->setText(msgId);
		if (g_grim->getMode() != ENGINE_MODE_SMUSH)
			_sayLineText = textObject->getId();
	}
}

bool Actor::isTalking() {
	// If there's no sound file then we're obviously not talking
	GrimEngine::SpeechMode m = g_grim->getSpeechMode();
	TextObject *textObject = NULL;
	if (_sayLineText)
		textObject = TextObject::getPool()->getObject(_sayLineText);
	if ((m == GrimEngine::TextOnly && (!textObject || textObject->getDisabled())) ||
			(m != GrimEngine::TextOnly && (strlen(_talkSoundName.c_str()) == 0 || !g_imuse->getSoundStatus(_talkSoundName.c_str())))) {
		return false;
	}

	return true;
}

void Actor::shutUp() {
	// While the call to stop the sound is usually made by the game,
	// we also need to handle when the user terminates the dialog.
	// Some warning messages will occur when the user terminates the
	// actor dialog but the game will continue alright.
	if (_talkSoundName != "") {
		g_imuse->stopSound(_talkSoundName.c_str());
		_talkSoundName = "";
	}
	if (_lipSync) {
		if (_talkAnim != -1 && _talkChore[_talkAnim] >= 0)
			_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);
		_lipSync = NULL;
		stopTalking();
	} else if (stopMumbleChore()) {
		stopTalking();
	}

	if (_sayLineText) {
		delete TextObject::getPool()->getObject(_sayLineText);
		_sayLineText = 0;
	}
	if (g_grim->getTalkingActor() == this) {
		g_grim->setTalkingActor(NULL);
	}
}

void Actor::pushCostume(const char *n) {
	Costume *newCost = g_resourceloader->loadCostume(n, getCurrentCostume());

	newCost->setColormap(NULL);
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
		freeCostumeChore(_costumeStack.back(), _restCostume, _restChore);
		freeCostumeChore(_costumeStack.back(), _walkCostume, _walkChore);

		if (_turnCostume == _costumeStack.back()) {
			_turnCostume = NULL;
			_leftTurnChore = -1;
			_rightTurnChore = -1;
		}

		freeCostumeChore(_costumeStack.back(), _mumbleCostume, _mumbleChore);
		for (int i = 0; i < 10; i++)
			freeCostumeChore(_costumeStack.back(), _talkCostume[i], _talkChore[i]);
		delete _costumeStack.back();
		_costumeStack.pop_back();

		if (_costumeStack.empty()) {
			if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
				printf("Popped (freed) the last costume for an actor.\n");
		}
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Attempted to pop (free) a costume when the stack is empty!");
	}
}

void Actor::clearCostumes() {
	// Make sure to destroy costume copies in reverse order
	while (!_costumeStack.empty())
		popCostume();
}

void Actor::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	if (!_costumeStack.empty()) {
		_costumeStack.back()->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
	}
}

Costume *Actor::findCostume(const Common::String &n) {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		if ((*i)->getFilename().compareToIgnoreCase(n) == 0)
			return *i;
	}

	return NULL;
}

void Actor::updateWalk() {
	if (_path.empty()) {
		return;
	}

	Graphics::Vector3d destPos = _path.back();
	float y = getYawTo(destPos);
	if (y < 0.f) {
		y += 360.f;
	}
	turnTo(_pitch, y, _roll);

	Graphics::Vector3d dir = destPos - _pos;
	float dist = dir.magnitude();

	if (dist > 0)
		dir /= dist;

	float walkAmt = g_grim->getPerSecond(_walkRate);

	if (walkAmt >= dist) {
		_pos = destPos;
		_path.pop_back();
		if (_path.empty()) {
			_walking = false;
// It seems that we need to allow an already active turning motion to
// continue or else turning actors away from barriers won't work right
			_turning = false;
		}
	} else {
		_pos += dir * walkAmt;
	}

	_walkedCur = true;
}

void Actor::update(float frameTime) {
	// Snap actor to walkboxes if following them.  This might be
	// necessary for example after activating/deactivating
	// walkboxes, etc.
	if (_constrain && !_walking) {
		g_grim->getCurrScene()->findClosestSector(_pos, NULL, &_pos);
	}

	if (_turning) {
		float turnAmt = g_grim->getPerSecond(_turnRate) * 5.f;
		float dyaw = _destYaw - _yaw;
		while (dyaw > 180)
			dyaw -= 360;
		while (dyaw < -180)
			dyaw += 360;
		// If the actor won't turn because the rate is set to zero then
		// have the actor turn all the way to the destination yaw.
		// Without this some actors will lock the interface on changing
		// scenes, this affects the Bone Wagon in particular.
		if (turnAmt == 0 || turnAmt >= fabs(dyaw)) {
			setYaw(_destYaw);
			_turning = false;
		} else if (dyaw > 0) {
			setYaw(_yaw + turnAmt);
		} else {
			setYaw(_yaw - turnAmt);
		}
		_currTurnDir = (dyaw > 0 ? 1 : -1);
	}

	if (_walking) {
		updateWalk();
	}

	if (_walkChore >= 0) {
		if (_walkedCur) {
			if (_walkCostume->isChoring(_walkChore, false) < 0) {
				_walkCostume->stopChore(_walkChore);
				_walkCostume->playChoreLooping(_walkChore);
				_walkCostume->fadeChoreIn(_walkChore, 150);
			}

			if (_restChore >= 0 && _restCostume->isChoring(_restChore, false) >= 0) {
				_restCostume->fadeChoreOut(_restChore, 150);
				_restCostume->stopChore(_restChore);
			}
		} else {
			if (_walkedLast && _walkCostume->isChoring(_walkChore, false) >= 0) {
				_walkCostume->fadeChoreOut(_walkChore, 150);
				_walkCostume->stopChore(_walkChore);

				if (_restChore >= 0 && _restCostume->isChoring(_restChore, false) < 0) {
					_restCostume->playChoreLooping(_restChore);
					_restCostume->fadeChoreIn(_restChore, 150);
				}
			}
		}
	}

	if (_leftTurnChore >= 0) {
		if (_walkedCur || _walkedLast)
			_currTurnDir = 0;

		if (_restChore >= 0) {
			if (_currTurnDir != 0) {
				if (_turnCostume->isChoring(getTurnChore(_currTurnDir), false) >= 0 &&
					_restChore >= 0 && _restCostume->isChoring(_restChore, false) >= 0) {
					_restCostume->fadeChoreOut(_restChore, 500);
					_restCostume->stopChore(_restChore);
				}
			}
			else if (_lastTurnDir != 0) {
				if (!_walkedCur && _turnCostume->isChoring(getTurnChore(_lastTurnDir), false) >= 0) {
					_restCostume->playChoreLooping(_restChore);
					_restCostume->fadeChoreIn(_restChore, 150);
				}
			}
		}

		if (_lastTurnDir != 0 && _lastTurnDir != _currTurnDir) {
			_turnCostume->fadeChoreOut(getTurnChore(_lastTurnDir), 150);
			_turnCostume->stopChore(getTurnChore(_lastTurnDir));
		}
		if (_currTurnDir != 0 && _currTurnDir != _lastTurnDir) {
			_turnCostume->playChoreLooping(getTurnChore(_currTurnDir));
			_turnCostume->fadeChoreIn(getTurnChore(_currTurnDir), 500);
		}
	} else
		_currTurnDir = 0;

	// The rest chore might have been stopped because of a
	// StopActorChore(nil).  Restart it if so.
	if (!_walkedCur && _currTurnDir == 0 && _restChore >= 0 && _restCostume->isChoring(_restChore, false) < 0)
		_restCostume->playChoreLooping(_restChore);

	_walkedLast = _walkedCur;
	_walkedCur = false;
	_lastTurnDir = _currTurnDir;
	_currTurnDir = 0;

	// Update lip syncing
	if (_lipSync) {
		int posSound;

		// While getPosIn60HzTicks will return "-1" to indicate that the
		// sound is no longer playing, it is more appropriate to check first
		if (g_grim->getSpeechMode() != GrimEngine::TextOnly && g_imuse->getSoundStatus(_talkSoundName.c_str()))
			posSound = g_imuse->getPosIn60HzTicks(_talkSoundName.c_str());
		else
			posSound = -1;
		if (posSound != -1) {
			int anim = _lipSync->getAnim(posSound);
			if (_talkAnim != anim) {
				if (anim != -1) {
					if (_talkChore[anim] >= 0) {
						stopMumbleChore();
						if (_talkAnim != -1 && _talkChore[_talkAnim] >= 0)
							_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);

						// Run the stop_talk chore so that it resets the components
						// to the right visibility.
						stopTalking();
						_talkAnim = anim;
						_talkCostume[_talkAnim]->playChore(_talkChore[_talkAnim]);
					} else if (_mumbleChore != -1 && _mumbleCostume->isChoring(_mumbleChore, false) < 0) {
						_mumbleCostume->playChoreLooping(_mumbleChore);
						_talkAnim = -1;
					}
				} else {
					stopMumbleChore();
					if (_talkAnim != -1 && _talkChore[_talkAnim] >= 0)
						_talkCostume[_talkAnim]->stopChore(_talkChore[_talkAnim]);

					_talkAnim = 0;
					stopTalking();
				}
			}
		}
	}

	frameTime *= _timeScale;
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		c->setPosRotate(_pos, _pitch, _yaw, _roll);
		int marker = c->update(frameTime);
		if (marker > 0) {
			costumeMarkerCallback(marker);
		}
	}

	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		c->animate();
	}

	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		c->moveHead(_lookingMode, _lookAtVector, _lookAtRate);
	}
}

void Actor::draw() {
	for (Common::List<Costume *>::iterator i = _costumeStack.begin(); i != _costumeStack.end(); ++i) {
		Costume *c = *i;
		c->setupTextures();
	}

	if (!g_driver->isHardwareAccelerated() && g_grim->getFlagRefreshShadowMask()) {
		for (int l = 0; l < 5; l++) {
			if (!_shadowArray[l].active)
				continue;
			g_driver->setShadow(&_shadowArray[l]);
			g_driver->drawShadowPlanes();
			g_driver->setShadow(NULL);
		}
	}

	int x1, y1, x2, y2;
	int *px1, *py1, *px2, *py2;
	if (_mustPlaceText) {
		px1 = &x1;
		py1 = &y1;
		px2 = &x2;
		py2 = &y2;

		x1 = y1 = 1000;
		x2 = y2 = -1000;
	} else {
		px1 = py1 = px2 = py2 = NULL;
	}

	if (!_costumeStack.empty()) {
		Costume *costume = _costumeStack.back();
		if (!g_driver->isHardwareAccelerated()) {
			for (int l = 0; l < 5; l++) {
				if (!shouldDrawShadow(l))
					continue;
				g_driver->setShadow(&_shadowArray[l]);
				g_driver->setShadowMode();
				g_driver->startActorDraw(_pos, _scale, _yaw, _pitch, _roll);
				costume->draw();
				g_driver->finishActorDraw();
				g_driver->clearShadowMode();
				g_driver->setShadow(NULL);
			}
			// normal draw actor
			g_driver->startActorDraw(_pos, _scale, _yaw, _pitch, _roll);
			costume->draw(px1, py1, px2, py2);
			g_driver->finishActorDraw();
		} else {
			// normal draw actor
			g_driver->startActorDraw(_pos, _scale, _yaw, _pitch, _roll);
			costume->draw(px1, py1, px2, py2);
			g_driver->finishActorDraw();

			for (int l = 0; l < 5; l++) {
				if (!shouldDrawShadow(l))
					continue;
				g_driver->setShadow(&_shadowArray[l]);
				g_driver->setShadowMode();
				g_driver->drawShadowPlanes();
				g_driver->startActorDraw(_pos, _scale, _yaw, _pitch, _roll);
				costume->draw();
				g_driver->finishActorDraw();
				g_driver->clearShadowMode();
				g_driver->setShadow(NULL);
			}
		}
	}

	if (_mustPlaceText) {
		TextObject *textObject = TextObject::getPool()->getObject(_sayLineText);
		if (textObject) {
			if (x1 == 1000 || x2 == -1000 || y2 == -1000) {
				textObject->setX(640 / 2);
				textObject->setY(463);
			} else {
				textObject->setX((x1 + x2) / 2);
				textObject->setY(y1);
			}
			textObject->reset();
		}
		_mustPlaceText = false;
	}
}

// "Undraw objects" (handle objects for actors that may not be on screen)
void Actor::undraw(bool /*visible*/) {
	if (!isTalking())
		shutUp();
}

void Actor::setShadowPlane(const char *n) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].name = n;
}

void Actor::addShadowPlane(const char *n, Scene *scene, int shadowId) {
	assert(shadowId != -1);

	int numSectors = scene->getSectorCount();

	for (int i = 0; i < numSectors; i++) {
		// Create a copy so we are sure it will not be deleted by the Scene destructor
		// behind our back. This is important when Membrillo phones Velasco to tell him
		// Naranja is dead, because the scene changes back and forth few times and so
		// the scenes' sectors are deleted while they are still keeped by the actors.
		Sector *sector = scene->getSectorBase(i);
		if (!strcmp(sector->getName(), n)) {
			Plane p = { scene->getName(), new Sector(*sector) };
			_shadowArray[shadowId].planeList.push_back(p);
			g_grim->flagRefreshShadowMask(true);
			return;
		}
	}
}

bool Actor::shouldDrawShadow(int shadowId) {
	Shadow *shadow = &_shadowArray[shadowId];
	if (!shadow->active)
		return false;

	// Don't draw a shadow if the actor is behind the shadow plane.
	Sector *sector = shadow->planeList.front().sector;
	Graphics::Vector3d n = sector->getNormal();
	Graphics::Vector3d p = sector->getVertices()[0];
	float d = -(n.x() * p.x() + n.y() * p.y() + n.z() * p.z());

	p = getPos();
	if (n.x() * p.x() + n.y() * p.y() + n.z() * p.z() + d < 0.f)
		return true;
	else
		return false;
}

void Actor::addShadowPlane(const char *n) {
	addShadowPlane(n, g_grim->getCurrScene(), _activeShadowSlot);
}

void Actor::setActiveShadow(int shadowId) {
	assert(shadowId >= 0 && shadowId <= 4);

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
	assert(shadowId >= 0 && shadowId <= 4);

	_shadowArray[shadowId].active = state;
}

void Actor::setShadowPoint(Graphics::Vector3d p) {
	assert(_activeShadowSlot != -1);

	_shadowArray[_activeShadowSlot].pos = p;
}

void Actor::clearShadowPlanes() {
	for (int i = 0; i < 5; i++) {
		Shadow *shadow = &_shadowArray[i];
		while (!shadow->planeList.empty()) {
			delete shadow->planeList.back().sector;
			shadow->planeList.pop_back();
		}
		delete[] shadow->shadowMask;
		shadow->shadowMaskSize = 0;
		shadow->shadowMask = NULL;
		shadow->active = false;
		shadow->dontNegate = false;
	}
}

void Actor::putInSet(const Common::String &setName) {
	// The set should change immediately, otherwise a very rapid set change
	// for an actor will be recognized incorrectly and the actor will be lost.
	_setName = setName;
}

bool Actor::isInSet(const Common::String &setName) const {
	return _setName == setName;
}

void Actor::freeCostumeChore(Costume *toFree, Costume *&cost, int &chore) {
	if (cost == toFree) {
		cost = NULL;
		chore = -1;
	}
}

void Actor::stopTalking() {
	if (_talkChore[0] >= 0) {
		// _talkChore[0] is *_stop_talk
		// Don't playLooping it, or else manny's mouth will flicker when he smokes.
		_talkCostume[0]->playChore(_talkChore[0]);
	}
}

bool Actor::stopMumbleChore() {
	if (_mumbleChore >= 0 && _mumbleCostume->isChoring(_mumbleChore, false) >= 0) {
		_mumbleCostume->stopChore(_mumbleChore);
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

bool Actor::collidesWith(Actor *actor, Graphics::Vector3d *vec) const {
	if (actor->_collisionMode == CollisionOff) {
		return false;
	}

	Model *model1 = getCurrentCostume()->getModel();
	Model *model2 = actor->getCurrentCostume()->getModel();

	Graphics::Vector3d p1 = _pos + model1->_insertOffset;
	Graphics::Vector3d p2 = actor->_pos + model2->_insertOffset;

	float size1 = model1->_radius * _collisionScale;
	float size2 = model2->_radius * actor->_collisionScale;

	CollisionMode mode1 = _collisionMode;
	CollisionMode mode2 = actor->_collisionMode;

	if (mode1 == CollisionSphere && mode2 == CollisionSphere) {
		Graphics::Vector3d pos = p1 + *vec;
		float distance = (pos - p2).magnitude();
		if (distance < size1 + size2) {
			// Move the destination point so that its distance from the
			// center of the circle is size1+size2.
			Graphics::Vector3d v = pos - p2;
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
		Graphics::Rect2d rect;

		Graphics::Vector3d bboxPos;
		Graphics::Vector3d size;
		Graphics::Vector3d pos;
		Graphics::Vector3d circlePos;
		float yaw;

		Graphics::Vector2d circle;
		float radius;

		if (mode1 == CollisionBox) {
			pos = p1 + *vec;
			bboxPos = pos + model1->_bboxPos;
			size = model1->_bboxSize * _collisionScale;
			yaw = _yaw;

			circle.setX(p2.x());
			circle.setY(p2.y());
			circlePos = p2;
			radius = size2;
		} else {
			pos = p2;
			bboxPos = p2 + model2->_bboxPos;
			size = model2->_bboxSize * actor->_collisionScale;
			yaw = actor->_yaw;

			circle.setX(p1.x() + vec->x());
			circle.setY(p1.y() + vec->y());
			circlePos = p1;
			radius = size1;
		}

		rect._topLeft = Graphics::Vector2d(bboxPos.x(), bboxPos.y() + size.y());
		rect._topRight = Graphics::Vector2d(bboxPos.x() + size.x(), bboxPos.y() + size.y());
		rect._bottomLeft = Graphics::Vector2d(bboxPos.x(), bboxPos.y());
		rect._bottomRight = Graphics::Vector2d(bboxPos.x() + size.x(), bboxPos.y());
		rect.rotateAround(Graphics::Vector2d(pos.x(), pos.y()), yaw);

		if (rect.intersectsCircle(circle, radius)) {
			Graphics::Vector2d center = rect.getCenter();
			// Draw a line from the center of the rect to the place the character
			// would go to.
			Graphics::Vector2d v = circle - center;
			v.normalize();

			Graphics::Segment2d edge;
			// That line intersects (usually) an edge
			rect.getIntersection(center, v, &edge);
			// Take the perpendicular of that edge
			Graphics::Line2d perpendicular = edge.getPerpendicular(circle);

			Graphics::Vector3d point;
			Graphics::Vector2d p;
			// If that perpendicular intersects the edge
			if (edge.intersectsLine(perpendicular, &p)) {
				Graphics::Vector2d direction = perpendicular.getDirection();
				direction.normalize();

				// Move from the intersection until we are at a safe distance
				Graphics::Vector2d point1(p - direction * radius);
				Graphics::Vector2d point2(p + direction * radius);

				if (center.getDistanceTo(point1) < center.getDistanceTo(point2)) {
					point = point2.toVector3d();
				} else {
					point = point1.toVector3d();
				}
			} else { //if not we're around a corner
				// Find the nearest vertex of the rect
				Graphics::Vector2d vertex = rect.getTopLeft();
				float distance = vertex.getDistanceTo(circle);

				Graphics::Vector2d other = rect.getTopRight();
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
				Graphics::Vector2d dst = circle - vertex;
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

extern int refSystemTable;

void Actor::costumeMarkerCallback(int marker) {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("costumeMarkerHandler");
	lua_Object table = lua_gettable();

	if (lua_istable(table)) {
		lua_pushobject(table);
		lua_pushstring("costumeMarkerHandler");
		lua_Object func = lua_gettable();
		if (lua_isfunction(func)) {
			lua_pushobject(func);
			lua_pushusertag(getId(), MKTAG('A','C','T','R'));
			lua_pushnumber(marker);
			lua_callfunction(func);
		}
	} else if (lua_isfunction(table)) {
		lua_pushusertag(getId(), MKTAG('A','C','T','R'));
		lua_pushnumber(marker);
		lua_callfunction(table);
	}

	lua_endblock();
}

void Actor::collisionHandlerCallback(Actor *other) const {
	lua_beginblock();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("collisionHandler");
	lua_Object table = lua_gettable();

	if (lua_istable(table)) {
		lua_pushobject(table);
		lua_pushstring("collisionHandler");
		lua_Object func = lua_gettable();
		if (lua_isfunction(func)) {
			lua_pushobject(func);
			lua_pushusertag(getId(), MKTAG('A','C','T','R'));
			lua_pushusertag(other->getId(), MKTAG('A','C','T','R'));
			lua_callfunction(func);
		}
	} else if (lua_isfunction(table)) {
		lua_pushusertag(getId(), MKTAG('A','C','T','R'));
		lua_pushusertag(other->getId(), MKTAG('A','C','T','R'));
		lua_callfunction(table);
	}

	lua_endblock();
}

} // end of namespace Grim
