// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
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
#include "debug.h"
#include "colormap.h"
#include "costume.h"
#include "actor.h"
#include "textsplit.h"
#include "engine.h"
#include "keyframe.h"
#include "material.h"
#include "model.h"
#include "lua.h"

#include "imuse/imuse.h"

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
	BitmapComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	void setKey(int val);

private:
	std::string _filename;
};

class ColormapComponent : public Costume::Component {
public:
	ColormapComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	ColormapComponent *copy(Costume::Component *newParent);
	~ColormapComponent();
};

class ModelComponent : public Costume::Component {
public:
	ModelComponent(Costume::Component *parent, int parentID, const char *filename, Costume::Component *prevComponent, tag32 tag);
	void init();
	void setKey(int val);
	void update();
	void reset();
	void resetColormap();
	void setMatrix(Matrix4 matrix) { _matrix = matrix; };
	~ModelComponent();

	Model::HierNode *hierarchy() { return _hier; }
	void draw();

protected:
	std::string _filename;
	ResPtr<Model> _obj;
	Model::HierNode *_hier;
	Matrix4 _matrix;
};

class MainModelComponent : public ModelComponent {
public:
	MainModelComponent(Costume::Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
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
	MeshComponent(Costume::Component *parent, int parentID, const char *name, tag32 tag);
	void init();
	CMap *cmap() {
		ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
		if (mc == NULL)
			return NULL;
		return mc->cmap();
	}
	void setKey(int val);
	void update();
	void reset();
	~MeshComponent() { }

	void setMatrix(Matrix4 matrix) { _matrix = matrix; };

	Model::HierNode *node() { return _node; }

private:
	std::string _name;
	int _num;
	Model::HierNode *_node;
	Matrix4 _matrix;
};

BitmapComponent::BitmapComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag) :
		Costume::Component(parent, parentID, tag), _filename(filename) {
}

void BitmapComponent::setKey(int val) {
	const char *bitmap = _filename.c_str();
	ObjectState *state = g_engine->currScene()->findState(bitmap);

	if (state != NULL) {
		state->setNumber(val);
		return;
	}
	// Complain that we couldn't find the bitmap.  This means we probably
	// didn't handle something correctly.  Example: Before the tube-switcher 
	// bitmaps were not loading with the scene. This was because they were requested
	// as a different case then they were stored (tu_0_dorcu_door_open versus
	// TU_0_DORCU_door_open), which was causing problems in the string comparison.
	if(debugLevel == DEBUG_BITMAPS || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("Missing scene bitmap: %s\n", bitmap);
	
/* In case you feel like drawing the missing bitmap anyway...
	// Assume that all objects the scene file forgot about are OBJSTATE_STATE class
	state = new ObjectState(0, ObjectState::OBJSTATE_STATE, bitmap, NULL, true);
	if (state == NULL) {
		if (debugLevel == DEBUG_BITMAPS || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Couldn't find bitmap %s in current scene\n", _filename.c_str());
		return;
	}
	g_engine->currScene()->addObjectState(state);
	state->setNumber(val);
*/
}

ModelComponent::ModelComponent(Costume::Component *parent, int parentID, const char *filename, Costume::Component *prevComponent, tag32 tag) :
		Costume::Component(parent, parentID, tag), _filename(filename),
		_obj(NULL), _hier(NULL) {
	const char *comma = std::strchr(filename, ',');
	
	// Can be called with a comma and a numeric parameter afterward, but
	// the use for this parameter is currently unknown
	// Example: At the "scrimshaw parlor" in Rubacava the object
	// "manny_cafe.3do,1" is requested
	if (comma != NULL) {
		_filename = std::string(filename, comma);
		warning("Comma in model components not supported: %s", filename);
	} else {
		_filename = filename;
	}
	if (prevComponent != NULL) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);
		
		if (mmc != NULL)
			_previousCmap = mmc->cmap();
	}
}

void ModelComponent::init() {
	// Skip loading if it was initialized
	// by the sharing MainModelComponent
	// constructor before
	if (_obj == NULL) {
		CMap *cmap = this->cmap();
		
		// Get the default colormap if we haven't found
		// a valid colormap
		if (cmap == NULL) {
			if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("No colormap specified for %s, using %s\n", _filename.c_str(), DEFAULT_COLORMAP);

			cmap = g_resourceloader->loadColormap(DEFAULT_COLORMAP);
		}
		_obj = g_resourceloader->loadModel(_filename.c_str(), *cmap);
		_hier = _obj->copyHierarchy();
		// Use parent availablity to decide whether to default the
		// component to being visible
		if (_parent == NULL || !_parent->visible())
			setKey(1);
		else
			setKey(0);
	}

	// If we're the child of a mesh component, put our nodes in the
	// parent object's tree.
	if (_parent != NULL) {
		MeshComponent *mc = dynamic_cast<MeshComponent *>(_parent);
		
		if (mc != NULL)
			mc->node()->addChild(_hier);
		else if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Parent of model %s wasn't a mesh\n", _filename.c_str());
	}
}

void ModelComponent::setKey(int val) {
	_visible = (val != 0);
	_hier->_hierVisible = _visible;
}

void ModelComponent::reset() {
	_visible = false;
	_hier->_hierVisible = _visible;
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

void ModelComponent::resetColormap() {
	CMap *cmap;
	
	cmap = this->cmap();
	if (_obj != NULL && cmap != NULL)
		_obj->reload(*cmap);
}

ModelComponent::~ModelComponent() {
	if (_hier != NULL && _hier->_parent != NULL)
		_hier->_parent->removeChild(_hier);

	delete[] _hier;
}

void translateObject(Model::HierNode *node, bool reset) {
	if (node->_parent != NULL)
		translateObject(node->_parent, reset);

	if(reset)
		g_driver->translateViewpoint();
	else
		g_driver->translateViewpoint(node->_animPos / node->_totalWeight, node->_animPitch / node->_totalWeight, node->_animYaw / node->_totalWeight, node->_animRoll / node->_totalWeight);
}

void ModelComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it
	if (_parent != NULL && _parent->visible())
			return;
	// Need to translate object to be in accordance
	// with the setup of the parent
	if (_hier->_parent != NULL)
		translateObject(_hier->_parent, false);
	_hier->draw();
	// Need to un-translate when done
	if (_hier->_parent != NULL)
		translateObject(_hier->_parent, true);
}

MainModelComponent::MainModelComponent(Costume::Component *parent, int parentID, const char *filename, Costume::Component *prevComponent, tag32 tag) :
		ModelComponent(parent, parentID, filename, prevComponent, tag), _hierShared(false) {
	if (parentID == -2 && prevComponent != NULL) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);
		
		if (mmc != NULL && mmc->_filename == filename) {
			_obj = mmc->_obj;
			_hier = mmc->_hier;
			_hierShared = true;
		}
	}
}

void MainModelComponent::init() {
	ModelComponent::init();
	_visible = true;
	_hier->_hierVisible = _visible;
}

void MainModelComponent::update() {
	if (!_hierShared)
		// Otherwise, it was already initialized
		// and reinitializing it will destroy work
		// from previous costumes
		ModelComponent::update();
}

void MainModelComponent::reset() {
	_visible = true;
	_hier->_hierVisible = _visible;
}

MainModelComponent::~MainModelComponent() {
	if (_hierShared)
		_hier = NULL; // Keep ~ModelComp from deleting it
}

class MaterialComponent : public Costume::Component {
public:
	MaterialComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	void init();
	void setKey(int val);
	void setColormap(CMap *c);
	void setupTexture();
	void reset();
	~MaterialComponent() { }

private:
	ResPtr<Material> _mat;
	std::string _filename;
	int _num;
};

ColormapComponent::ColormapComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag) :
		Costume::Component(parent, parentID, tag) {
	_cmap = g_resourceloader->loadColormap(filename);
	
	if (parent != NULL)
		parent->setColormap(_cmap);
	else
		warning("No parent to apply colormap object on.");
}

ColormapComponent::~ColormapComponent() {
}

class KeyframeComponent : public Costume::Component {
public:
	KeyframeComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
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

KeyframeComponent::KeyframeComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag) :
		Costume::Component(parent, parentID, tag), _priority1(1), _priority2(5), _hier(NULL), _active(false) {
	const char *comma = std::strchr(filename, ',');
	if (comma != NULL) {
		std::string realName(filename, comma);
		_keyf = g_resourceloader->loadKeyframe(realName.c_str());
		std::sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
	} else
		_keyf = g_resourceloader->loadKeyframe(filename);
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
		if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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
		_currTime += g_engine->frameTime();

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
			default:
				if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
					warning("Unknown repeat mode %d for keyframe %s\n", _repeatMode, _keyf->filename());
		}
	}
	_keyf->animate(_hier, _currTime / 1000.0, _priority1, _priority2);
}

void KeyframeComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc != NULL)
		_hier = mc->hierarchy();
	else {
		if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Parent of %s was not a model\n", _keyf->filename());
		_hier = NULL;
	}
}

MeshComponent::MeshComponent(Costume::Component *parent, int parentID, const char *name, tag32 tag) :
		Costume::Component(parent, parentID, tag), _name(name), _node(NULL) {
	if (std::sscanf(name, "mesh %d", &_num) < 1)
		error("Couldn't parse mesh name %s\n", name);

}

void MeshComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc != NULL)
		_node = mc->hierarchy() + _num;
	else {
		if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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

MaterialComponent::MaterialComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag) :
		Costume::Component(parent, parentID, tag), _filename(filename),
		_num(0) {
	
	if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("Constructing MaterialComponent %s\n", filename);
}

void MaterialComponent::init() {
	CMap *cmap = this->cmap();
	
	if (cmap == NULL) {
		// Use the default colormap if we're still drawing a blank
		if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("MaterialComponent::init on %s\n", _filename.c_str());
		
		cmap = g_resourceloader->loadColormap(DEFAULT_COLORMAP);
	}
	_mat = g_resourceloader->loadMaterial(_filename.c_str(), *cmap);
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
	LuaVarComponent(Costume::Component *parent, int parentID, const char *name, tag32 tag);
	void setKey(int val);
	~LuaVarComponent() { }

private:
	std::string _name;
};

LuaVarComponent::LuaVarComponent(Costume::Component *parent, int parentID, const char *name, tag32 tag) :
		Costume::Component(parent, parentID, tag), _name(name) {
}

void LuaVarComponent::setKey(int val) {
	lua_pushnumber(val);
	lua_setglobal(const_cast<char *>(_name.c_str()));
}

class SoundComponent : public Costume::Component {
public:
	SoundComponent(Costume::Component *parent, int parentID, const char *name, tag32 tag);
	void setKey(int val);
	void reset();
	~SoundComponent() {
	// Stop the sound if it's in progress
	reset();
}

private:
	std::string _soundName;
};

SoundComponent::SoundComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag) :
		Costume::Component(parent, parentID, tag) {
	const char *comma = std::strchr(filename, ',');
	if (comma != NULL) {
		_soundName = std::string(filename, comma);
	} else {
		_soundName = filename;
	}
}

void SoundComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play"
		// No longer a need to check the sound status, if it's already playing
		// then it will just use the existing handle
		g_imuse->startSfx(_soundName.c_str());
		if (g_engine->currScene() && g_currentUpdatedActor) {
			Vector3d pos = g_currentUpdatedActor->pos();
			g_engine->currScene()->setSoundPosition(_soundName.c_str(), pos);
		}
		break;
	case 1: // "Stop"
		g_imuse->stopSound(_soundName.c_str());
		break;
	case 2: // "Stop Looping"
		g_imuse->setHookId(_soundName.c_str(), 0x80);
		break;
	default:
		if (debugLevel == DEBUG_MODEL || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Unknown key %d for sound %s\n", val, _soundName.c_str());
	}
}

void SoundComponent::reset() {
	// A lot of the sound components this gets called against aren't actually running
	if(g_imuse->getSoundStatus(_soundName.c_str()))
		g_imuse->stopSound(_soundName.c_str());
}

Costume::Costume(const char *filename, const char *data, int len, Costume *prevCost) :
	_fname(filename), _cmap(NULL) {
	TextSplitter ts(data, len);
	
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
		std::memcpy(&tags[which], t, sizeof(tag32));
	}

	ts.expectString("section components");
	ts.scanString(" numcomponents %d", 1, &_numComponents);
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; i++) {
		int id, tagID, hash, parentID, namePos;
		const char *line = ts.currentLine();
		Component *prevComponent = NULL;
		
		if (std::sscanf(line, " %d %d %d %d %n", &id, &tagID, &hash, &parentID, &namePos) < 4)
			error("Bad component specification line: `%s'\n", line);
		ts.nextLine();

		// A Parent ID of "-1" indicates that the component should
		// use the properties of the previous costume as a base
		if (parentID == -1 && prevCost != NULL) {
			MainModelComponent *mmc;
			
			// However, only the first item can actually share the
			// node hierarchy with the previous costume, so flag
			// that component so it knows what to do
			if (i == 0)
				parentID = -2;
			prevComponent = prevCost->_components[0];
			mmc = dynamic_cast<MainModelComponent *>(prevComponent);
			// Make sure that the component is valid 
			if (mmc == NULL)
				prevComponent = NULL;
		}
		// Actually load the appropriate component
		_components[id] = loadComponent(tags[tagID], parentID < 0 ? NULL : _components[parentID], parentID, line + namePos, prevComponent);
	}

	for (int i = 0; i < _numComponents; i++) {
		if (_components[i] != NULL)
			_components[i]->setCostume(this);
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
		if(debugLevel == DEBUG_ALL || debugLevel == DEBUG_CHORES)
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
	for (int i = _numComponents - 1; i >= 0; i--) {
		// The "Sprite" component can be NULL
		if (_components[i] != NULL)
			delete _components[i];
	}
	delete[] _chores;
}

Costume::Component::Component(Component *parent, int parentID, tag32 tag) {
	_visible = true;
	_previousCmap = NULL;
	_cmap = NULL;
	_cost = NULL;
	_parent = NULL;
	_parentID = parentID;
	setParent(parent);
	_tag = tag;
}

void Costume::Component::setColormap(CMap *c) {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(this);
	
	if (c != NULL)
		_cmap = c;
	if (mc != NULL && this->cmap() != NULL)
		mc->resetColormap();
}

bool Costume::Component::visible() {
	if (_visible && _parent != NULL)
		return _parent->visible();
	return _visible;
}

CMap *Costume::Component::cmap() {
	if (_cmap == NULL && _previousCmap != NULL)
		return _previousCmap;
	else if (_cmap == NULL && _parent != NULL)
		return _parent->cmap();
	else if (_cmap == NULL && _parent == NULL && _cost != NULL)
		return _cost->_cmap;
	else
		return _cmap;
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

// Should initialize the status variables so the chore can't play unexpectedly
Costume::Chore::Chore() : _hasPlayed(false), _playing(false), _looping(false), _currTime(-1) {
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

void Costume::Chore::setLastFrame() {
	// If the chore has already played then don't set it to the end
	// Example: This executing would result in Glottis being
	// choppy when he hands Manny the work order
	if (_hasPlayed)
		return;
	_currTime = _length;
	_playing = false;
	_hasPlayed = true;
	_looping = false;
	setKeys(-1, _currTime);
	_currTime = -1;
}

void Costume::Chore::update() {
	if (!_playing)
		return;

	int newTime;
	if (_currTime < 0)
		newTime = 0; // For first time through
	else
		newTime = _currTime + g_engine->frameTime();

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

Costume::Component *Costume::loadComponent (tag32 tag, Costume::Component *parent, int parentID, const char *name, Costume::Component *prevComponent) {
	if (tag == MKID('MMDL'))
		return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	else if (tag == MKID('MODL'))
		return new ModelComponent(parent, parentID, name, prevComponent, tag);
	else if (tag == MKID('CMAP'))
		return new ColormapComponent(parent, parentID, name, tag);
	else if (tag == MKID('KEYF'))
		return new KeyframeComponent(parent, parentID, name, tag);
	else if (tag == MKID('MESH'))
		return new MeshComponent(parent, parentID, name, tag);
	else if (tag == MKID('LUAV'))
		return new LuaVarComponent(parent, parentID, name, tag);
	else if (tag == MKID('IMLS'))
		return new SoundComponent(parent, parentID, name, tag);
	else if (tag == MKID('BKND'))
		return new BitmapComponent(parent, parentID, name, tag);
	else if (tag == MKID('MAT '))
		return new MaterialComponent(parent, parentID, name, tag);
	else if (tag == MKID('SPRT'))
		return NULL;// new SpriteComponent(parent, parentID, name);

	char t[4];
	std::memcpy(t, &tag, sizeof(tag32));
	error("Unknown tag '%c%c%c%c', name '%s'\n", t[0], t[1], t[2], t[3], name);
	return NULL;
}

Model::HierNode *Costume::getModelNodes()
{
		for(int i=0;i<_numComponents;i++) {
			if (_components[i] == NULL)
				continue;
			// Needs to handle Main Models (pigeons) and normal Models
			// (when Manny climbs the rope)
			if (_components[i]->tag() == MKID('MMDL'))
				return dynamic_cast<ModelComponent *>(_components[i])->hierarchy();
		}
		return NULL;
}

void Costume::playChoreLooping(int num) {
	if (num < 0 || num >= _numChores) {
		if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)!", num, _numChores);
		return;
	}
	_chores[num].playLooping();
}

void Costume::playChore(int num) {
	if (num < 0 || num >= _numChores) {
		if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)!", num, _numChores);
		return;
	}
	_chores[num].play();
}

void Costume::setColormap(char *map) {
	// Sometimes setColormap is called on a null costume,
	// see where raoul is gone in hh.set
	if (this == NULL || map == NULL)
		return;
	_cmap = g_resourceloader->loadColormap(map);
	for (int i = 0; i < _numComponents; i++)
		_components[i]->setColormap(NULL);
}

void Costume::stopChores() {
	for (int i = 0; i < _numChores; i++)
		_chores[i].stop();
}

int Costume::isChoring(char *name, bool excludeLooping) {
	for (int i = 0; i < _numChores; i++) {
		if (!strcmp(_chores[i]._name, name) && _chores[i]._playing && !(excludeLooping && _chores[i]._looping))
			return i;
	}
	return -1;
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
