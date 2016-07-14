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

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/resource.h"
#include "engines/grim/model.h"
#include "engines/grim/savegame.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/costume/chore.h"
#include "engines/grim/costume/head.h"
#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/costume/main_model_component.h"
#include "engines/grim/costume/colormap_component.h"
#include "engines/grim/costume/keyframe_component.h"
#include "engines/grim/costume/mesh_component.h"
#include "engines/grim/costume/lua_var_component.h"
#include "engines/grim/costume/sound_component.h"
#include "engines/grim/costume/bitmap_component.h"
#include "engines/grim/costume/material_component.h"
#include "engines/grim/costume/sprite_component.h"
#include "engines/grim/costume/anim_component.h"

namespace Grim {

// A costume in the Residual/GrimE engine consists of a set of
// components, and a set of chores.  Each component represents an
// on-screen object, or a keyframe animation, or a sound effect; each
// chore gives a set of instructions for how to move and/or activate
// or deactivate each component at certain times.
//
// Each actor contains a stack of costumes, on which a new costume can
// be pushed or from which an old costume can be popped at any time.
// For the most part, these costumes are independent.  The exception
// is the main model component ('MMDL'), for which multiple costumes
// share the same base 3D object (if they refer to the same file).
//
// This is complicated by the fact that multiple keyframe animations
// can have an effect on the positions of the 3D objects.  Each
// keyframe animation has certain nodes internally "tagged", and the
// keyframe components specify precedences for the tagged nodes and
// for the non-tagged nodes.  If the highest precedence for a given
// node is given by multiple keyframe animations, their contributions
// are averaged.
//
// Each component can implement several virtual methods which are
// called by the costume:
//
// init() -- allows the component to initialize itself.  This is
//           separate from the constructor since there are cases where
//           information from child components may be needed before
//           the object can be fully constructed.  This is particularly
//           the case with colormaps, which are needed before even
//           starting to load a 3D model.
// setKey(val) -- notifies the component of a change in the "state"
//                given by a playing chore
// update() -- gives the component a chance to update its internal
//             state once every frame
// draw() -- actually draws the component onto the screen
// reset() -- notifies the component that a chore controlling it
//            has stopped
//
// For the 3D objects, a model's component first initializes internal
// state for the model's nodes in its update() method.  Then the
// keyframes' update() methods work with this data to implement the
// precedence and add up all contributions for the highest precedence.
// Then the model's draw() method does the averaging and draws the
// polygons accordingly.  (Actually, this is a lie -- the top-level
// 3D objects draw themselves and all their children.  This makes it
// easier to move objects Manny is holding when his hands move, for
// example.)
//
// For bitmaps, the actual drawing is handled by the Set class.  The
// bitmaps to be drawn are associated to the needed camera setups
// using NewObjectState; bitmaps marked OBJSTATE_UNDERLAY and
// OBJSTATE_STATE are drawn first, then the 3D objects, then bitmaps
// marked OBJSTATE_OVERLAY.  So the BitmapComponent just needs to pass
// along setKey requests to the actual bitmap object.

Costume::Costume(const Common::String &fname, Actor *owner, Costume *prevCost) :
		Object(), _head(nullptr), _chores(nullptr), _components(nullptr),
		_numComponents(0), _numChores(0), _fname(fname), _owner(owner) {
	_lookAtRate = 200;
	_prevCostume = prevCost;
}

void Costume::load(Common::SeekableReadStream *data) {
	TextSplitter ts(_fname, data);
	ts.expectString("costume v0.1");
	ts.expectString("section tags");
	int numTags;
	ts.scanString(" numtags %d", 1, &numTags);
	tag32 *tags = new tag32[numTags];
	for (int i = 0; i < numTags; i++) {
		unsigned char t[4];
		int which;

		// Obtain a tag ID from the file
		ts.scanString(" %d '%c%c%c%c'", 5, &which, &t[0], &t[1], &t[2], &t[3]);
		// Force characters to upper case
		for (int j = 0; j < 4; j++)
			t[j] = toupper(t[j]);
		memcpy(&tags[which], t, sizeof(tag32));
		tags[which] = FROM_BE_32(tags[which]);
	}

	ts.expectString("section components");
	ts.scanString(" numcomponents %d", 1, &_numComponents);
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; i++) {
		int id, tagID, hash, parentID, namePos;
		const char *line = ts.getCurrentLine();
		Component *prevComponent = nullptr;

		if (sscanf(line, " %d %d %d %d %n", &id, &tagID, &hash, &parentID, &namePos) < 4)
			error("Bad component specification line: `%s'", line);
		ts.nextLine();

		// A Parent ID of "-1" indicates that the component should
		// use the properties of the previous costume as a base
		if (parentID == -1) {
			if (_prevCostume) {
				// However, only the first item can actually share the
				// node hierarchy with the previous costume, so flag
				// that component so it knows what to do
				if (i == 0)
					parentID = -2;
				prevComponent = _prevCostume->_components[0];
				// Make sure that the component is valid
				if (!prevComponent->isComponentType('M','M','D','L'))
					prevComponent = nullptr;
			} else if (id > 0) {
				// Use the MainModelComponent of this costume as prevComponent,
				// so that the component can use its colormap.
				prevComponent = _components[0];
			}
		}
		// Actually load the appropriate component
		_components[id] = loadComponent(tags[tagID], parentID < 0 ? nullptr : _components[parentID], parentID, line + namePos, prevComponent);
		_components[id]->setCostume(this);
	}

	delete[] tags;

	for (int i = 0; i < _numComponents; i++)
		if (_components[i]) {
			_components[i]->init();
		}

	ts.expectString("section chores");
	ts.scanString(" numchores %d", 1, &_numChores);
	_chores = new Chore *[_numChores];
	for (int i = 0; i < _numChores; i++) {
		int id, length, tracks;
		char name[32];
		ts.scanString(" %d %d %d %32s", 4, &id, &length, &tracks, name);
		_chores[id] = new Chore(name, i, this, length, tracks);
		Debug::debug(Debug::Chores, "Loaded chore: %s\n", name);
	}

	ts.expectString("section keys");
	for (int i = 0; i < _numChores; i++) {
		int which;
		ts.scanString("chore %d", 1, &which);
		_chores[which]->load(ts);
	}

	_head = new Head();
}

Costume::~Costume() {
	stopChores();
	for (int i = _numComponents - 1; i >= 0; i--) {
		delete _components[i];
	}
	delete[] _components;

	for (int i = 0; i < _numChores; ++i) {
		delete _chores[i];
	}
	delete[] _chores;
	delete _head;
}

Component *Costume::loadComponent (tag32 tag, Component *parent, int parentID, const char *name, Component *prevComponent) {
	if (tag == MKTAG('M','M','D','L'))
		return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	else if (tag == MKTAG('M','O','D','L'))
		return new ModelComponent(parent, parentID, name, prevComponent, tag);
	else if (tag == MKTAG('C','M','A','P'))
		return new ColormapComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('K','E','Y','F'))
		return new KeyframeComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('M','E','S','H'))
		return new MeshComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('L','U','A','V'))
		return new LuaVarComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('I','M','L','S'))
		return new SoundComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('B','K','N','D'))
		return new BitmapComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('M','A','T',' '))
		return new MaterialComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('S','P','R','T'))
		return new SpriteComponent(parent, parentID, name, tag);
	else if (tag == MKTAG('A','N','I','M')) //Used  in the demo
		return new AnimComponent(parent, parentID, name, tag);

	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);
	return nullptr;
}

ModelComponent *Costume::getMainModelComponent() const {
	for (int i = 0; i < _numComponents; i++) {
		// Needs to handle Main Models (pigeons) and normal Models
		// (when Manny climbs the rope)
		if (_components[i] && _components[i]->isComponentType('M','M','D','L'))
			return static_cast<ModelComponent *>(_components[i]);
	}
	return nullptr;
}

ModelNode *Costume::getModelNodes() {
	ModelComponent *comp = getMainModelComponent();
	if (comp) {
		return comp->getHierarchy();
	}
	return nullptr;
}

Model *Costume::getModel() {
	ModelComponent *comp = getMainModelComponent();
	if (comp) {
		return comp->getModel();
	}
	return nullptr;
}

void Costume::setChoreLastFrame(int num) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num]->setLastFrame();
}

void Costume::setChoreLooping(int num, bool val) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num]->setLooping(val);
}

void Costume::playChoreLooping(const char *name, uint msecs) {
	for (int i = 0; i < _numChores; ++i) {
		if (strcmp(_chores[i]->getName(), name) == 0) {
			playChoreLooping(i, msecs);
			return;
		}
	}
	warning("Costume::playChoreLooping: Could not find chore: %s", name);
	return;
}

void Costume::playChoreLooping(int num, uint msecs) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}

	_chores[num]->playLooping(msecs);
	if (Common::find(_playingChores.begin(), _playingChores.end(), _chores[num]) == _playingChores.end())
		_playingChores.push_back(_chores[num]);
}

Chore *Costume::getChore(const char *name) {
	for (int i = 0; i < _numChores; ++i) {
		if (strcmp(_chores[i]->getName(), name) == 0) {
			return _chores[i];
		}
	}
	return nullptr;
}

int Costume::getChoreId(const char *name) {
	if (name == nullptr) {
		return -1;
	}
	for (int i = 0; i < _numChores; ++i) {
		if (strcmp(_chores[i]->getName(), name) == 0) {
			return i;
		}
	}
	return -1;
}

void Costume::playChore(const char *name, uint msecs) {
	for (int i = 0; i < _numChores; ++i) {
		if (strcmp(_chores[i]->getName(), name) == 0) {
			playChore(i, msecs);
			return;
		}
	}
	warning("Costume::playChore: Could not find chore: %s", name);
	return;
}

void Costume::playChore(int num, uint msecs) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num]->play(msecs);
	if (Common::find(_playingChores.begin(), _playingChores.end(), _chores[num]) == _playingChores.end())
		_playingChores.push_back(_chores[num]);
}

void Costume::stopChore(int num, uint msecs) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num]->stop(msecs);
}

void Costume::setColormap(const Common::String &map) {
	// Sometimes setColormap is called on a null costume,
	// see where raoul is gone in hh.set
	if (!map.size())
		return;
	_cmap = g_resourceloader->getColormap(map);
	for (int i = 0; i < _numComponents; i++)
		if (_components[i])
			_components[i]->setColormap(nullptr);
}

void Costume::stopChores(bool ignoreLoopingChores, int msecs) {
	for (int i = 0; i < _numChores; i++) {
		if (ignoreLoopingChores && _chores[i]->isLooping()) {
			continue;
		}
		_chores[i]->stop(msecs);
	}
}

void Costume::fadeChoreIn(int chore, uint msecs) {
	if (chore < 0 || chore >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
		return;
	}
	_chores[chore]->fadeIn(msecs);
	if (Common::find(_playingChores.begin(), _playingChores.end(), _chores[chore]) == _playingChores.end())
		_playingChores.push_back(_chores[chore]);
}

void Costume::fadeChoreOut(int chore, uint msecs) {
	if (chore < 0 || chore >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
		return;
	}
	_chores[chore]->fadeOut(msecs);
}

int Costume::isChoring(const char *name, bool excludeLooping) {
	for (int i = 0; i < _numChores; i++) {
		if (!strcmp(_chores[i]->getName(), name) && _chores[i]->isPlaying() && !(excludeLooping && _chores[i]->isLooping()))
			return i;
	}
	return -1;
}

int Costume::isChoring(int num, bool excludeLooping) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return -1;
	}
	if (_chores[num]->isPlaying() && !(excludeLooping && _chores[num]->isLooping()))
		return num;
	else
		return -1;
}

int Costume::isChoring(bool excludeLooping) {
	for (int i = 0; i < _numChores; i++) {
		if (_chores[i]->isPlaying() && !(excludeLooping && _chores[i]->isLooping()))
			return i;
	}
	return -1;
}

void Costume::draw() {
	for (int i = 0; i < _numComponents; i++)
		if (_components[i])
			_components[i]->draw();
}

void Costume::getBoundingBox(int *x1, int *y1, int *x2, int *y2) {
	for (int i = 0; i < _numComponents; i++) {
		if (_components[i] &&(_components[i]->isComponentType('M','M','D','L') ||
							  _components[i]->isComponentType('M','O','D','L'))) {
			ModelComponent *c = static_cast<ModelComponent *>(_components[i]);
			c->getBoundingBox(x1, y1, x2, y2);
		}

		if (_components[i] &&(_components[i]->isComponentType('m','e','s','h'))) {
			EMIMeshComponent *c = static_cast<EMIMeshComponent *>(_components[i]);
			c->getBoundingBox(x1, y1, x2, y2);
		}
	}
}

int Costume::update(uint time) {
	for (Common::List<Chore*>::iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		(*i)->update(time);
		if (!(*i)->isPlaying()) {
			i = _playingChores.erase(i);
			--i;
		}
	}

	int marker = 0;
	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
			_components[i]->setMatrix(_matrix);
			int m = _components[i]->update(time);
			if (m > 0) {
				marker = m;
			}
		}
	}

	return marker;
}

void Costume::animate() {
	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
			_components[i]->animate();
		}
	}
}

void Costume::moveHead(bool entering, const Math::Vector3d &lookAt) {
	_head->lookAt(entering, lookAt, _lookAtRate, _matrix);
}

int Costume::getHeadJoint() const {
	return static_cast<Head *>(_head)->getJoint3();
}

void Costume::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	Head *head = static_cast<Head *>(_head);
	head->setJoints(joint1, joint2, joint3);
	head->loadJoints(getModelNodes());
	head->setMaxAngles(maxPitch, maxYaw, maxRoll);
}

void Costume::setLookAtRate(float rate) {
	_lookAtRate = rate;
}

float Costume::getLookAtRate() const {
	return _lookAtRate;
}

void Costume::setPosRotate(const Math::Vector3d &pos, const Math::Angle &pitch,
						   const Math::Angle &yaw, const Math::Angle &roll) {
	_matrix.setPosition(pos);
	_matrix.buildFromEuler(yaw, pitch, roll, Math::EO_ZXY);
}

Math::Matrix4 Costume::getMatrix() const {
	return _matrix;
}

Costume *Costume::getPreviousCostume() const {
	return _prevCostume;
}

void Costume::saveState(SaveGame *state) const {
	if (_cmap) {
		state->writeBool(true);
		state->writeString(_cmap->getFilename());
	} else {
		state->writeBool(false);
	}

	for (int i = 0; i < _numChores; ++i) {
		_chores[i]->saveState(state);
	}

	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			state->writeBool(c->_visible);
			c->saveState(state);
		}
	}

	state->writeLEUint32(_playingChores.size());
	for (Common::List<Chore*>::const_iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		state->writeLESint32((*i)->getChoreId());
	}

	state->writeFloat(_lookAtRate);
	_head->saveState(state);
}

bool Costume::restoreState(SaveGame *state) {
	if (state->readBool()) {
		Common::String str = state->readString();
		setColormap(str);
	}

	for (int i = 0; i < _numChores; ++i) {
		_chores[i]->restoreState(state);
	}

	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			c->_visible = state->readBool();
			if (state->saveMinorVersion() < 14) {
				// skip the old _matrix vector
				state->readVector3d();
			}
			c->restoreState(state);
		}
	}

	int numPlayingChores = state->readLEUint32();
	for (int i = 0; i < numPlayingChores; ++i) {
		int id = state->readLESint32();
		_playingChores.push_back(_chores[id]);
	}

	_lookAtRate = state->readFloat();
	_head->restoreState(state);
	_head->loadJoints(getModelNodes());

	return true;
}

} // end of namespace Grim
