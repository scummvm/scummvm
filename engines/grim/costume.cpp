/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/actor.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/grim.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/lua.h"

#include "engines/grim/imuse/imuse.h"

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
	Common::String _filename;
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
	void setMatrix(Graphics::Matrix4 matrix) { _matrix = matrix; };
	~ModelComponent();

	Model::HierNode *hierarchy() { return _hier; }
	void draw();

protected:
	Common::String _filename;
	Model *_obj;
	Model::HierNode *_hier;
	Graphics::Matrix4 _matrix;
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
		if (!mc)
			return NULL;
		return mc->cmap();
	}
	void setKey(int val);
	void update();
	void reset();
	~MeshComponent() { }

	void setMatrix(Graphics::Matrix4 matrix) { _matrix = matrix; };

	Model::HierNode *node() { return _node; }

private:
	Common::String _name;
	int _num;
	Model::HierNode *_node;
	Graphics::Matrix4 _matrix;
};

BitmapComponent::BitmapComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename) {
}

void BitmapComponent::setKey(int val) {
	const char *bitmap = _filename.c_str();
	ObjectState *state = g_grim->currScene()->findState(bitmap);

	if (state) {
		state->setNumber(val);
		return;
	}
	// Complain that we couldn't find the bitmap.  This means we probably
	// didn't handle something correctly.  Example: Before the tube-switcher
	// bitmaps were not loading with the scene. This was because they were requested
	// as a different case then they were stored (tu_0_dorcu_door_open versus
	// TU_0_DORCU_door_open), which was causing problems in the string comparison.
	if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("Missing scene bitmap: %s", bitmap);

/* In case you feel like drawing the missing bitmap anyway...
	// Assume that all objects the scene file forgot about are OBJSTATE_STATE class
	state = new ObjectState(0, ObjectState::OBJSTATE_STATE, bitmap, NULL, true);
	if (!state) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Couldn't find bitmap %s in current scene", _filename.c_str());
		return;
	}
	g_grim->currScene()->addObjectState(state);
	state->setNumber(val);
*/
}

ModelComponent::ModelComponent(Costume::Component *p, int parentID, const char *filename, Costume::Component *prevComponent, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename),
		_obj(NULL), _hier(NULL) {
	const char *comma = strchr(filename, ',');

	// Can be called with a comma and a numeric parameter afterward, but
	// the use for this parameter is currently unknown
	// Example: At the "scrimshaw parlor" in Rubacava the object
	// "manny_cafe.3do,1" is requested
	if (comma) {
		_filename = Common::String(filename, comma);
		warning("Comma in model components not supported: %s", filename);
	} else {
		_filename = filename;
	}
	if (prevComponent) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);

		if (mmc)
			_previousCmap = mmc->cmap();
	}
}

void ModelComponent::init() {
	// Skip loading if it was initialized
	// by the sharing MainModelComponent
	// constructor before
	if (!_obj) {
		CMap *cm = this->cmap();

		// Get the default colormap if we haven't found
		// a valid colormap
		if (!cm) {
			if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("No colormap specified for %s, using %s", _filename.c_str(), DEFAULT_COLORMAP);

			cm = g_resourceloader->loadColormap(DEFAULT_COLORMAP);
		}
		_obj = g_resourceloader->loadModel(_filename.c_str(), cm);
		_hier = _obj->copyHierarchy();
		// Use parent availablity to decide whether to default the
		// component to being visible
		if (!_parent || !_parent->visible())
			setKey(1);
		else
			setKey(0);
	}

	// If we're the child of a mesh component, put our nodes in the
	// parent object's tree.
	if (_parent) {
		MeshComponent *mc = dynamic_cast<MeshComponent *>(_parent);

		if (mc)
			mc->node()->addChild(_hier);
		else if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Parent of model %s wasn't a mesh", _filename.c_str());
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
	CMap *cm;

	cm = this->cmap();
	if (_obj && cm)
		_obj->reload(cm);
}

ModelComponent::~ModelComponent() {
	if (_hier && _hier->_parent)
		_hier->_parent->removeChild(_hier);

	delete[] _hier;
}

void translateObject(Model::HierNode *node, bool reset) {
	if (node->_parent)
		translateObject(node->_parent, reset);

	if (reset)
		g_driver->translateViewpointFinish();
	else
		g_driver->translateViewpointStart(node->_animPos / node->_totalWeight, node->_animPitch / node->_totalWeight, node->_animYaw / node->_totalWeight, node->_animRoll / node->_totalWeight);
}

void ModelComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it
	if (_parent && _parent->visible())
			return;
	// Need to translate object to be in accordance
	// with the setup of the parent
	if (_hier->_parent)
		translateObject(_hier->_parent, false);
	_hier->draw();
	// Need to un-translate when done
	if (_hier->_parent)
		translateObject(_hier->_parent, true);
}

MainModelComponent::MainModelComponent(Costume::Component *p, int parentID, const char *filename, Costume::Component *prevComponent, tag32 t) :
		ModelComponent(p, parentID, filename, prevComponent, t), _hierShared(false) {
	if (parentID == -2 && prevComponent) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);

		if (mmc && mmc->_filename == filename) {
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
	Material *_mat;
	Common::String _filename;
	int _num;
};

ColormapComponent::ColormapComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t) {
	_cmap = g_resourceloader->loadColormap(filename);

	if (p)
		p->setColormap(_cmap);
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
	KeyframeAnim *_keyf;
	int _priority1, _priority2;
	Model::HierNode *_hier;
	bool _active;
	int _repeatMode;
	int _currTime;
	Common::String _fname;
};

KeyframeComponent::KeyframeComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _priority1(1), _priority2(5), _hier(NULL), _active(false) {
	_fname = filename;
	const char *comma = strchr(filename, ',');
	if (comma) {
		Common::String realName(filename, comma);
		_keyf = g_resourceloader->loadKeyframe(realName.c_str());
		sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
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
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Unknown key %d for keyframe %s", val, _keyf->filename());
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
		_currTime += g_grim->frameTime();

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
				if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
					warning("Unknown repeat mode %d for keyframe %s", _repeatMode, _keyf->filename());
		}
	}
	_keyf->animate(_hier, _currTime / 1000.0, _priority1, _priority2);
}

void KeyframeComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc)
		_hier = mc->hierarchy();
	else {
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Parent of %s was not a model", _keyf->filename());
		_hier = NULL;
	}
}

MeshComponent::MeshComponent(Costume::Component *p, int parentID, const char *name, tag32 t) :
		Costume::Component(p, parentID, t), _name(name), _node(NULL) {
	if (sscanf(name, "mesh %d", &_num) < 1)
		error("Couldn't parse mesh name %s", name);

}

void MeshComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc)
		_node = mc->hierarchy() + _num;
	else {
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Parent of mesh %d was not a model", _num);
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

MaterialComponent::MaterialComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename),
		_num(0) {

	if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("Constructing MaterialComponent %s", filename);
}

void MaterialComponent::init() {
	CMap *cm = this->cmap();

	if (!cm) {
		// Use the default colormap if we're still drawing a blank
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("MaterialComponent::init on %s", _filename.c_str());

		cm = g_resourceloader->loadColormap(DEFAULT_COLORMAP);
	}
	_mat = g_resourceloader->loadMaterial(_filename.c_str(), cm);
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
	Common::String _name;
};

LuaVarComponent::LuaVarComponent(Costume::Component *p, int parentID, const char *name, tag32 t) :
		Costume::Component(p, parentID, t), _name(name) {
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
	Common::String _soundName;
};

SoundComponent::SoundComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t) {
	const char *comma = strchr(filename, ',');
	if (comma) {
		_soundName = Common::String(filename, comma);
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
		if (g_grim->currScene() && g_currentUpdatedActor) {
			Graphics::Vector3d pos = g_currentUpdatedActor->pos();
			g_grim->currScene()->setSoundPosition(_soundName.c_str(), pos);
		}
		break;
	case 1: // "Stop"
		g_imuse->stopSound(_soundName.c_str());
		break;
	case 2: // "Stop Looping"
		g_imuse->setHookId(_soundName.c_str(), 0x80);
		break;
	default:
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Unknown key %d for sound %s", val, _soundName.c_str());
	}
}

void SoundComponent::reset() {
	// A lot of the sound components this gets called against aren't actually running
	if (g_imuse->getSoundStatus(_soundName.c_str()))
		g_imuse->stopSound(_soundName.c_str());
}

Costume::Costume(const char *fname, const char *data, int len, Costume *prevCost) :
	_fname(fname), _cmap(NULL) {
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
		memcpy(&tags[which], t, sizeof(tag32));
	}

	ts.expectString("section components");
	ts.scanString(" numcomponents %d", 1, &_numComponents);
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; i++) {
		int id, tagID, hash, parentID, namePos;
		const char *line = ts.currentLine();
		Component *prevComponent = NULL;

		if (sscanf(line, " %d %d %d %d %n", &id, &tagID, &hash, &parentID, &namePos) < 4)
			error("Bad component specification line: `%s'", line);
		ts.nextLine();

		// A Parent ID of "-1" indicates that the component should
		// use the properties of the previous costume as a base
		if (parentID == -1 && prevCost) {
			MainModelComponent *mmc;

			// However, only the first item can actually share the
			// node hierarchy with the previous costume, so flag
			// that component so it knows what to do
			if (i == 0)
				parentID = -2;
			prevComponent = prevCost->_components[0];
			mmc = dynamic_cast<MainModelComponent *>(prevComponent);
			// Make sure that the component is valid
			if (!mmc)
				prevComponent = NULL;
		}
		// Actually load the appropriate component
		_components[id] = loadComponent(tags[tagID], parentID < 0 ? NULL : _components[parentID], parentID, line + namePos, prevComponent);
	}

	for (int i = 0; i < _numComponents; i++) {
		if (_components[i])
			_components[i]->setCostume(this);
	}

	delete[] tags;

	for (int i = 0; i < _numComponents; i++)
		if (_components[i])
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
		memcpy(_chores[id]._name, name, 32);
		if (gDebugLevel == DEBUG_ALL || gDebugLevel == DEBUG_CHORES)
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
		if (_components[i])
			delete _components[i];
	}
	delete[] _components;
	delete[] _chores;
}

Costume::Component::Component(Component *p, int parentID, tag32 t) {
	_visible = true;
	_previousCmap = NULL;
	_cmap = NULL;
	_cost = NULL;
	_parent = NULL;
	_parentID = parentID;
	setParent(p);
	_tag = t;
}

void Costume::Component::setColormap(CMap *c) {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(this);

	if (c)
		_cmap = c;
	if (mc && this->cmap())
		mc->resetColormap();
}

bool Costume::Component::visible() {
	if (_visible && _parent)
		return _parent->visible();
	return _visible;
}

CMap *Costume::Component::cmap() {
	if (!_cmap && _previousCmap)
		return _previousCmap;
	else if (!_cmap && _parent)
		return _parent->cmap();
	else if (!_cmap && !_parent && _cost)
		return _cost->_cmap;
	else
		return _cmap;
}

void Costume::Component::setParent(Component *newParent) {
	_parent = newParent;
	_child = NULL;
	_sibling = NULL;
	if (_parent) {
		Component **lastChildPos = &_parent->_child;
		while (*lastChildPos)
			lastChildPos = &((*lastChildPos)->_sibling);
		*lastChildPos = this;
	}
}

// Should initialize the status variables so the chore can't play unexpectedly
Costume::Chore::Chore() : _hasPlayed(false), _playing(false), _looping(false), _currTime(-1),
                          _tracks(NULL) {
}

Costume::Chore::~Chore() {
	if (_tracks) {
		for (int i = 0; i < _numTracks; i++)
			delete[] _tracks[i].keys;

		delete[] _tracks;
		_tracks = NULL;
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
		if (comp)
			comp->reset();
	}
}

void Costume::Chore::setKeys(int startTime, int stopTime) {
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (!comp)
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
		newTime = _currTime + g_grim->frameTime();

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
	if (FROM_BE_32(tag) == MKID_BE('MMDL'))
		return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKID_BE('MODL'))
		return new ModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKID_BE('CMAP'))
		return new ColormapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('KEYF'))
		return new KeyframeComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('MESH'))
		return new MeshComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('LUAV'))
		return new LuaVarComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('IMLS'))
		return new SoundComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('BKND'))
		return new BitmapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('MAT '))
		return new MaterialComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKID_BE('SPRT'))
		return NULL;// new SpriteComponent(parent, parentID, name);

	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);
	return NULL;
}

Model::HierNode *Costume::getModelNodes() {
	for (int i = 0; i < _numComponents; i++) {
		if (!_components[i])
			continue;
		// Needs to handle Main Models (pigeons) and normal Models
		// (when Manny climbs the rope)
		if (FROM_BE_32(_components[i]->tag()) == MKID_BE('MMDL'))
			return dynamic_cast<ModelComponent *>(_components[i])->hierarchy();
	}
	return NULL;
}

void Costume::playChoreLooping(int num) {
	if (num < 0 || num >= _numChores) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num].playLooping();
}

void Costume::playChore(int num) {
	if (num < 0 || num >= _numChores) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num].play();
}

void Costume::setColormap(char *map) {
	// Sometimes setColormap is called on a null costume,
	// see where raoul is gone in hh.set
	if (!map)
		return;
	_cmap = g_resourceloader->loadColormap(map);
	for (int i = 0; i < _numComponents; i++)
		_components[i]->setColormap(NULL);
}

void Costume::stopChores() {
	for (int i = 0; i < _numChores; i++)
		_chores[i].stop();
}

int Costume::isChoring(const char *name, bool excludeLooping) {
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
		if (_components[i])
			_components[i]->setupTexture();
}

void Costume::draw() {
	for (int i = 0; i < _numComponents; i++)
		if (_components[i])
			_components[i]->draw();
}

void Costume::update() {
	for (int i = 0; i < _numChores; i++)
		_chores[i].update();

	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
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

void Costume::setPosRotate(Graphics::Vector3d pos, float pitch, float yaw, float roll) {
	_matrix._pos = pos;
	_matrix._rot.buildFromPitchYawRoll(pitch, yaw, roll);
}

} // end of namespace Grim
