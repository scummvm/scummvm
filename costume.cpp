// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "costume.h"
#include "textsplit.h"
#include "debug.h"
#include "engine.h"
#include "colormap.h"
#include "keyframe.h"
#include "material.h"
#include "model.h"
#include "lua.h"
#include "sound.h"
#include <string>
#include <cstdio>
#include <map>

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
// setupTexture() -- sets up animated textures for the object.  This
//                   is a separate stage from update() since all the
//                   costumes on screen need to get updated before any
//                   drawing can start.
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
// For bitmaps, the actual drawing is handled by the Scene class.  The
// bitmaps to be drawn are associated to the needed camera setups
// using NewObjectState; bitmaps marked OBJSTATE_UNDERLAY and
// OBJSTATE_STATE are drawn first, then the 3D objects, then bitmaps
// marked OBJSTATE_OVERLAY.  So the BitmapComponent just needs to pass
// along setKey requests to the actual bitmap object.

class BitmapComponent : public Costume::Component {
public:
	BitmapComponent(Costume::Component *parent, int parentID, const char *filename);

	void setKey(int val);

private:
	std::string _filename;
};

class ModelComponent : public Costume::Component {
public:
	ModelComponent(Costume::Component *parent, int parentID, const char *filename);
	void init();
	void setKey(int val);
	void update();
	void reset();
	void setColormap(CMap *c);
	void setMatrix(Matrix4 matrix) { _matrix = matrix; };
	~ModelComponent();

	Model::HierNode *hierarchy() { return _hier; }
void draw();

protected:
	std::string _filename;
	ResPtr<Model> _obj;
	ResPtr<CMap> _cmap;
	Model::HierNode *_hier;
	Matrix4 _matrix;
};

class MainModelComponent : public ModelComponent {
public:
	MainModelComponent(Costume::Component *parent, int parentID, const char *filename);
	MainModelComponent(const char *filename, Model *prevObj, Model::HierNode *prevHier);
	void init();
	void update();
	void reset();
	~MainModelComponent();

private:
	bool _hierShared;

	friend class Costume;
};

class MeshComponent : public Costume::Component {
public:
	MeshComponent(Costume::Component *parent, int parentID, const char *name);
	void init();
	void setKey(int val);
	void update();
	void reset();
	~MeshComponent() { }

	void setMatrix(Matrix4 matrix) { _matrix = matrix; };

	Model::HierNode *node() { return _node; }

private:
	int _num;
	Model::HierNode *_node;
	Matrix4 _matrix;
};

BitmapComponent::BitmapComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID), _filename(filename) {
}

void BitmapComponent::setKey(int val) {
	ObjectState *state = Engine::instance()->currScene()->findState(_filename.c_str());

	if (state != NULL)
		state->setNumber(val);
	else
		warning("Couldn't find bitmap %s in current scene\n", _filename.c_str());
}

ModelComponent::ModelComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID), _filename(filename), _obj(NULL), _cmap(NULL), _hier(NULL) {
}

void ModelComponent::init() {
	if (_obj == NULL) {
		// Skip loading if it was initialized
		// by the sharing MainModelComponent
		// constructor before
		if (_cmap == NULL) {
			warning("No colormap specified for %s\n", _filename.c_str());
			_cmap = ResourceLoader::instance()->loadColormap("item.cmp");
		}
		_obj = ResourceLoader::instance()->loadModel(_filename.c_str(), *_cmap);
		_hier = _obj->copyHierarchy();
		_hier->_hierVisible = false;
	}

	// If we're the child of a mesh component, put our nodes in the
	// parent object's tree.
	if (_parent != NULL) {
		MeshComponent *mc = dynamic_cast<MeshComponent *>(_parent);
		if (mc != NULL)
			mc->node()->addChild(_hier);
		else
			warning("Parent of model %s wasn't a mesh\n", _filename.c_str());
	}
}

void ModelComponent::setKey(int val) {
	_hier->_hierVisible = (val != 0);
}

// Reset the hierarchy nodes for any keyframe animations (which
// are children of this component and therefore get updated later).
void ModelComponent::update() {
	for (int i = 0; i < _obj->numNodes(); i++) {
		_hier[i]._priority = -1;
		_hier[i]._animPos = _hier[i]._pos;
		_hier[i]._animPitch = _hier[i]._pitch;
		_hier[i]._animYaw = _hier[i]._yaw;
		_hier[i]._animRoll = _hier[i]._roll;
		_hier[i]._totalWeight = 1;
	}
}

void ModelComponent::reset() {
	_hier->_hierVisible = false;
}

void ModelComponent::setColormap(CMap *c) {
	_cmap = c;
}

ModelComponent::~ModelComponent() {
	if (_hier != NULL && _hier->_parent != NULL)
		_hier->_parent->removeChild(_hier);

	delete[] _hier;
}

void ModelComponent::draw() {
	if (_parent == NULL) 
		// Otherwise it was already drawn by
		// being included in the parent's hierarchy
		_hier->draw();
}

MainModelComponent::MainModelComponent(Costume::Component *parent, int parentID, const char *filename) :
		ModelComponent(parent, parentID, filename), _hierShared(false) {
}

// Constructor used if sharing the main model with the previous costume
MainModelComponent::MainModelComponent(const char *filename, Model *prevObj, Model::HierNode *prevHier) :
		ModelComponent(NULL, -1, filename), _hierShared(true) {
	_obj = prevObj;
	_hier = prevHier;
}

void MainModelComponent::init() {
	ModelComponent::init();
	_hier->_hierVisible = true;
}

void MainModelComponent::update() {
	if (!_hierShared)
		// Otherwise, it was already initialized
		// and reinitializing it will destroy work
		// from previous costumes
		ModelComponent::update();
}

void MainModelComponent::reset() {
	_hier->_hierVisible = true;
}

MainModelComponent::~MainModelComponent() {
	if (_hierShared)
		_hier = NULL; // Keep ~ModelComp from deleting it
}

class ColormapComponent : public Costume::Component {
public:
	ColormapComponent(Costume::Component *parent, int parentID, const char *filename);
	ColormapComponent *copy(Costume::Component *newParent);
	~ColormapComponent();

private:
	ResPtr<CMap> _cmap;
};

ColormapComponent::ColormapComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID) {
	_cmap = ResourceLoader::instance()->loadColormap(filename);

	ModelComponent *mc = dynamic_cast<ModelComponent *>(parent);
	if (mc != NULL)
		mc->setColormap(_cmap);
	}

ColormapComponent::~ColormapComponent() {
}

class KeyframeComponent : public Costume::Component {
public:
	KeyframeComponent(Costume::Component *parent, int parentID, const char *filename);
	void init();
	void setKey(int val);
	void update();
	void reset();
	~KeyframeComponent() {}

private:
	ResPtr<KeyframeAnim> _keyf;
	int _priority1, _priority2;
	Model::HierNode *_hier;
	bool _active;
	int _repeatMode;
	int _currTime;
};

KeyframeComponent::KeyframeComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID), _priority1(1), _priority2(5), _hier(NULL), _active(false) {
	const char *comma = std::strchr(filename, ',');
	if (comma != NULL) {
		std::string realName(filename, comma);
		_keyf = ResourceLoader::instance()->loadKeyframe(realName.c_str());
		std::sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
	} else
		_keyf = ResourceLoader::instance()->loadKeyframe(filename);
}

void KeyframeComponent::setKey(int val) {
	switch (val) {
	case 0:
	case 1:
	case 2:
	case 3:
		if (!_active || val != 1) {
			_active = true;
			_currTime = -1;
		}
		_repeatMode = val;
		break;
	case 4:
		_active = false;
		break;
	default:
		warning("Unknown key %d for keyframe %s\n", val, _keyf->filename());
	}
}

void KeyframeComponent::reset() {
	_active = false;
}

void KeyframeComponent::update() {
	if (!_active)
		return;

	if (_currTime < 0)		// For first time through
		_currTime = 0;
	else
		_currTime += Engine::instance()->frameTime();

	int animLength = (int)(_keyf->length() * 1000);

	if (_currTime > animLength) { // What to do at end?
		switch (_repeatMode) {
			case 0: // Stop
			case 3: // Fade at end
				_active = false;
				return;
			case 1: // Loop
				do
					_currTime -= animLength;
				while (_currTime > animLength);
				break;
			case 2: // Hold at end
				_currTime = animLength;
				break;
		}
	}
	_keyf->animate(_hier, _currTime / 1000.0, _priority1, _priority2);
}

void KeyframeComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc != NULL)
		_hier = mc->hierarchy();
	else {
		warning("Parent of %s was not a model\n", _keyf->filename());
		_hier = NULL;
	}
}

MeshComponent::MeshComponent(Costume::Component *parent, int parentID, const char *name) :
		Costume::Component(parent, parentID), _node(NULL) {
	if (std::sscanf(name, "mesh %d", &_num) < 1)
		error("Couldn't parse mesh name %s\n", name);
}

void MeshComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc != NULL)
		_node = mc->hierarchy() + _num;
	else {
		warning("Parent of mesh %d was not a model\n", _num);
		_node = NULL;
	}
}

void MeshComponent::setKey(int val) {
	_node->_meshVisible = (val != 0);
}

void MeshComponent::reset() {
	_node->_meshVisible = true;
}

void MeshComponent::update() {
	_node->setMatrix(_matrix);
	_node->update();
}

class MaterialComponent : public Costume::Component {
public:
	MaterialComponent(Costume::Component *parent, int parentID, const char *filename);
	void init();
	void setKey(int val);
	void setupTexture();
	void reset();
	~MaterialComponent() { }

private:
	ResPtr<Material> _mat;
	std::string _filename;
	int _num;
};

MaterialComponent::MaterialComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID), _filename(filename), _num(0) {
	warning("Constructing MaterialComponent %s\n", filename);
}

void MaterialComponent::init() {
	warning("MaterialComponent::init on %s\n", _filename.c_str());
	// The parent model and thus all its textures should have been
	// loaded by now, so passing an arbitrary colormap here
	// shouldn't cause problems.
	ResPtr<CMap> cmap = ResourceLoader::instance()->loadColormap("item.cmp");
	_mat = ResourceLoader::instance()->loadMaterial(_filename.c_str(), *cmap);
}

void MaterialComponent::setKey(int val) {
	_num = val;
}

void MaterialComponent::setupTexture() {
	_mat->setNumber(_num);
}

void MaterialComponent::reset() {
	_num = 0;
}

class LuaVarComponent : public Costume::Component {
public:
	LuaVarComponent(Costume::Component *parent, int parentID, const char *name);
	void setKey(int val);
	~LuaVarComponent() { }

private:
	std::string _name;
};

LuaVarComponent::LuaVarComponent(Costume::Component *parent, int parentID, const char *name) :
		Costume::Component(parent, parentID), _name(name) {
}

void LuaVarComponent::setKey(int val) {
	lua_pushnumber(val);
	lua_setglobal(const_cast<char *>(_name.c_str()));
}

class SoundComponent : public Costume::Component {
public:
	SoundComponent(Costume::Component *parent, int parentID, const char *name);
	void setKey(int val);
	void reset();
	~SoundComponent() { }

private:
	ResPtr<Sound> _sound;
};

SoundComponent::SoundComponent(Costume::Component *parent, int parentID, const char *filename) :
		Costume::Component(parent, parentID) {
	const char *comma = std::strchr(filename, ',');
	if (comma != NULL) {
		std::string realName(filename, comma);
		_sound = ResourceLoader::instance()->loadSound(realName.c_str());
	} else {
		_sound = ResourceLoader::instance()->loadSound(filename);
	}
}

void SoundComponent::setKey(int val) {
	switch (val) {
	case 0:
		Mixer::instance()->playSfx(_sound);
		break;
	case 2:
		Mixer::instance()->stopSfx(_sound);
		break;
	default:
		warning("Unknown key %d for sound %s\n", val, _sound->filename());
	}
}

void SoundComponent::reset() {
	Mixer::instance()->stopSfx(_sound);
}

Costume::Costume(const char *filename, const char *data, int len, Costume *prevCost) :
		_fname(filename) {
	TextSplitter ts(data, len);
	ts.expectString("costume v0.1");
	ts.expectString("section tags");
	int numTags;
	ts.scanString(" numtags %d", 1, &numTags);
	typedef char tag[4];
	tag *tags = new tag[numTags];
	for (int i = 0; i < numTags; i++) {
		int which;
		tag t;
		ts.scanString(" %d '%c%c%c%c'", 5, &which, &t[0], &t[1], &t[2], &t[3]);
		std::memcpy(tags + which, t, 4);
	}

	ts.expectString("section components");
	ts.scanString(" numcomponents %d", 1, &_numComponents);
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; i++) {
		int id, tagID, hash, parentID;
		int namePos;
		const char *line = ts.currentLine();
		if (std::sscanf(line, " %d %d %d %d %n", &id, &tagID, &hash, &parentID, &namePos) < 4)
			error("Bad component specification line: `%s'\n", line);
		ts.nextLine();

		// Check for sharing a main model with the previous costume
		if (id == 0 && prevCost != NULL && std::memcmp(tags[tagID], "mmdl", 4) == 0) {
			MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevCost->_components[0]);
			if (mmc != NULL && mmc->_filename == std::string(line + namePos)) {
				_components[id] = new MainModelComponent(line + namePos, mmc->_obj, mmc->_hier);
				continue;
			}
		}

		_components[id] = loadComponent(tags[tagID], parentID == -1 ? NULL : _components[parentID], parentID, line + namePos);
	}

	delete[] tags;

	for (int i = 0; i < _numComponents; i++)
		if (_components[i] != NULL)
			_components[i]->init();

	ts.expectString("section chores");
	ts.scanString(" numchores %d", 1, &_numChores);
	_chores = new Chore[_numChores];
	for (int i = 0; i < _numChores; i++) {
		int id, length, tracks;
		char name[32];
		ts.scanString(" %d %d %d %32s", 4, &id, &length, &tracks, name);
		_chores[id]._length = length;
		_chores[id]._numTracks = tracks;
		std::memcpy(_chores[id]._name, name, 32);
		printf("Loaded chore: %s\n", name);
	}

	ts.expectString("section keys");
	for (int i = 0; i < _numChores; i++) {
		int which;
		ts.scanString("chore %d", 1, &which);
		_chores[which].load(this, ts);
	}
}

Costume::~Costume() {
	stopChores();
	for (int i = _numComponents - 1; i >= 0; i--)
		delete _components[i];
	delete[] _chores;
}

Costume::Component::Component(Component *parent, int parentID) {
	_parentID = parentID;
	setParent(parent);
}

void Costume::Component::setParent(Component *newParent) {
	_parent = newParent;
	_child = NULL;
	_sibling = NULL;
	if (_parent != NULL) {
		Component **lastChildPos = &_parent->_child;
		while (*lastChildPos != NULL)
			lastChildPos = &((*lastChildPos)->_sibling);
		*lastChildPos = this;
	}
}

void Costume::Chore::load(Costume *owner, TextSplitter &ts) {
	_owner = owner;
	_tracks = new ChoreTrack[_numTracks];
	_hasPlayed = _playing = false;
	for (int i = 0; i < _numTracks; i++) {
		int compID, numKeys;
		ts.scanString(" %d %d", 2, &compID, &numKeys);
		_tracks[i].compID = compID;
		_tracks[i].numKeys = numKeys;
		_tracks[i].keys = new TrackKey[numKeys];
		for (int j = 0; j < numKeys; j++) {
			ts.scanString(" %d %d", 2, &_tracks[i].keys[j].time, &_tracks[i].keys[j].value);
		}
	}
}

void Costume::Chore::play() {
	_playing = true;
	_hasPlayed = true;
	_looping = false;
	_currTime = -1;
}

void Costume::Chore::playLooping() {
	_playing = true;
	_hasPlayed = true;
	_looping = true;
	_currTime = -1;
}

void Costume::Chore::stop() {
	if (!_hasPlayed)
		return;

	_playing = false;
	_hasPlayed = false;

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (comp != NULL)
			comp->reset();
	}
}

void Costume::Chore::setKeys(int startTime, int stopTime) {
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (comp == NULL)
			continue;

		for (int j = 0; j < _tracks[i].numKeys; j++) {
			if (_tracks[i].keys[j].time > stopTime)
				break;
			if (_tracks[i].keys[j].time > startTime)
				comp->setKey(_tracks[i].keys[j].value);
		}
	}
}

void Costume::Chore::update() {
	if (!_playing)
		return;

	int newTime;
	if (_currTime < 0)
		newTime = 0; // For first time through
	else
		newTime = _currTime + Engine::instance()->frameTime();

	setKeys(_currTime, newTime);

	if (newTime > _length) {
		if (!_looping) {
			_playing = false;
		} else {
			do {
				newTime -= _length;
				setKeys(-1, newTime);
			} while (newTime > _length);
		}
	}
	_currTime = newTime;
}

Costume::Component *Costume::loadComponent (char tag[4], Costume::Component *parent, int parentID, const char *name) {
	if (std::memcmp(tag, "mmdl", 4) == 0)
		return new MainModelComponent(parent, parentID, name);
	else if (std::memcmp(tag, "modl", 4) == 0)
		return new ModelComponent(parent, parentID, name);
	else if (std::memcmp(tag, "cmap", 4) == 0)
		return new ColormapComponent(parent, parentID, name);
	else if (std::memcmp(tag, "keyf", 4) == 0)
		return new KeyframeComponent(parent, parentID, name);
	else if (std::memcmp(tag, "mesh", 4) == 0)
		return new MeshComponent(parent, parentID, name);
	else if (std::memcmp(tag, "luav", 4) == 0)
		return new LuaVarComponent(parent, parentID, name);
	else if (std::memcmp(tag, "imls", 4) == 0)
		return new SoundComponent(parent, parentID, name);
	else if (std::memcmp(tag, "bknd", 4) == 0)
		return new BitmapComponent(parent, parentID, name);
	else if (std::memcmp(tag, "mat ", 4) == 0)
		return new MaterialComponent(parent, parentID, name);

	warning("Unknown tag '%.4s', name '%s'\n", tag, name);
	return NULL;
}

void Costume::stopChores() {
	for (int i = 0; i < _numChores; i++)
		_chores[i].stop();
}

int Costume::isChoring(int num, bool excludeLooping) {
	if (_chores[num]._playing && !(excludeLooping && _chores[num]._looping))
		return num;
	else
		return -1;
}

int Costume::isChoring(bool excludeLooping) {
	for (int i = 0; i < _numChores; i++) {
		if (_chores[i]._playing && !(excludeLooping && _chores[i]._looping))
			return i;
	}
	return -1;
}

void Costume::setupTextures() {
	for (int i = 0; i < _numComponents; i++)
		if (_components[i] != NULL)
			_components[i]->setupTexture();
}

void Costume::draw() {
	for (int i = 0; i < _numComponents; i++)
		if (_components[i] != NULL)
			_components[i]->draw();
}

void Costume::update() {
	for (int i = 0; i < _numChores; i++)
		_chores[i].update();

	for (int i = 0; i < _numComponents; i++) {
		if (_components[i] != NULL) {
			_components[i]->setMatrix(_matrix);
			_components[i]->update();
		}
	}
}

void Costume::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	_head.joint1 = joint1;
	_head.joint2 = joint2;
	_head.joint3 = joint3;
	_head.maxRoll = maxRoll;
	_head.maxPitch = maxPitch;
	_head.maxYaw = maxYaw;
}

void Costume::setPosRotate(Vector3d pos, float pitch, float yaw, float roll) {
	_matrix._pos = pos;
	_matrix._rot.buildFromPitchYawRoll(pitch, yaw, roll);
}
