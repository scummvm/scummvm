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

#include "common/endian.h"

#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/actor.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/grim.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/lua.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"

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

class SpriteComponent : public Costume::Component {
public:
	SpriteComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	~SpriteComponent();

	void init();
	void setKey(int val);
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);

private:
	Common::String _filename;
	Sprite *_sprite;
};

class ColormapComponent : public Costume::Component {
public:
	ColormapComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	ColormapComponent *copy(Costume::Component *newParent);
	~ColormapComponent();

	void init();
};

struct AnimationState {
	KeyframeAnimPtr _keyf;
	int _time;
	float _fade;
};

class ModelComponent : public Costume::Component {
public:
	ModelComponent(Costume::Component *parent, int parentID, const char *filename, Costume::Component *prevComponent, tag32 tag);
	void init();
	void setKey(int val);
	void animate();
	void reset();
	void resetColormap();
	void setMatrix(Graphics::Matrix4 matrix) { _matrix = matrix; };
	void restoreState(SaveGame *state);
	void addActiveAnimation(AnimationState *anim, int priority1, int priority2);
	void removeActiveAnimation(AnimationState *anim);
	~ModelComponent();

	Model::HierNode *getHierarchy() { return _hier; }
	int getNumNodes() { return _obj->getNumNodes(); }
	Model *getModel() { return _obj; }
	void draw();

protected:
	struct AnimationEntry {
		AnimationState *_anim;
		int _priority;
		bool _tagged;
	};

	Common::String _filename;
	ObjectPtr<Model> _obj;
	Model::HierNode *_hier;
	Graphics::Matrix4 _matrix;
	Common::List<AnimationEntry> *_activeAnims;
};

class MainModelComponent : public ModelComponent {
public:
	MainModelComponent(Costume::Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
	void init();
	void update();
	void setColormap(CMap *cmap);
	void reset();
	~MainModelComponent();

private:
	bool _hierShared;
	Common::List<MainModelComponent*> _children;
	MainModelComponent *_parentModel;
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
		return mc->getCMap();
	}
	void setKey(int val);
	void update();
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	~MeshComponent() { }

	void setMatrix(Graphics::Matrix4 matrix) { _matrix = matrix; };

	Model::HierNode *getNode() { return _node; }
	Model *getModel() { return _model; }

private:
	Common::String _name;
	int _num;
	Model *_model;
	Model::HierNode *_node;
	Graphics::Matrix4 _matrix;
};

BitmapComponent::BitmapComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename) {
}

void BitmapComponent::setKey(int val) {
	const char *bitmap = _filename.c_str();
	ObjectState *state = g_grim->getCurrScene()->findState(bitmap);

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
	g_grim->getCurrScene()->addObjectState(state);
	state->setNumber(val);
*/
}

SpriteComponent::SpriteComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
	Costume::Component(p, parentID, t), _filename(filename), _sprite(NULL) {

}

SpriteComponent::~SpriteComponent() {
	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			if (mc) {
				ModelComponent *mdlc = dynamic_cast<ModelComponent *>(mc->getParent());
				if (mdlc && mdlc->getHierarchy())
					mc->getNode()->removeSprite(_sprite);
			}
		}
		delete _sprite->_material;
		delete _sprite;
	}
}

void SpriteComponent::init() {
	const char *comma = strchr(_filename.c_str(), ',');

	Common::String name(_filename.c_str(), comma);

	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			mc->getNode()->removeSprite(_sprite);
		}
		delete _sprite;
		_sprite = NULL;
	}

	if (comma) {
		int width, height, x, y, z;
		sscanf(comma, ",%d,%d,%d,%d,%d", &width, &height, &x, &y, &z);

		_sprite = new Sprite;
		_sprite->_material = g_resourceloader->loadMaterial(name, getCMap());
		_sprite->_width = (float)width / 100.0f;
		_sprite->_height = (float)height / 100.0f;
		_sprite->_pos.set((float)x / 100.0f, (float)y / 100.0f, (float)z / 100.0f);
		_sprite->_visible = false;
		_sprite->_next = NULL;

		if (_parent) {
			MeshComponent *mc = dynamic_cast<MeshComponent *>(_parent);
			if (mc)
				mc->getNode()->addSprite(_sprite);
			else if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("Parent of sprite %s wasn't a mesh", _filename.c_str());
		}
	}
}

void SpriteComponent::setKey(int val) {
	if (!_sprite)
		return;

	if (val == 0) {
		_sprite->_visible = false;
	} else {
		_sprite->_visible = true;
		_sprite->_material->setNumber(val - 1);
	}
}

void SpriteComponent::reset() {
	if (_sprite)
		_sprite->_visible = false;
}

void SpriteComponent::saveState(SaveGame *state) {
	state->writeLEBool(_sprite->_visible);
	state->writeLESint32(_sprite->_material->getCurrentImage());
}

void SpriteComponent::restoreState(SaveGame *state) {
	_sprite->_visible = state->readLEBool();
	_sprite->_material->setNumber(state->readLESint32());
}

ModelComponent::ModelComponent(Costume::Component *p, int parentID, const char *filename, Costume::Component *prevComponent, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename),
		_obj(NULL), _hier(NULL), _activeAnims(NULL) {
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
			_previousCmap = mmc->getCMap();
	}
}

void ModelComponent::init() {
	// Skip loading if it was initialized
	// by the sharing MainModelComponent
	// constructor before
	if (!_obj) {
		CMapPtr cm = getCMap();

		// Get the default colormap if we haven't found
		// a valid colormap
		if (!cm && g_grim->getCurrScene())
			cm = g_grim->getCurrScene()->getCMap();
		if (!cm) {
			if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("No colormap specified for %s, using %s", _filename.c_str(), DEFAULT_COLORMAP);

			cm = g_resourceloader->getColormap(DEFAULT_COLORMAP);
		}

		// If we're the child of a mesh component, put our nodes in the
		// parent object's tree.
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			_obj = g_resourceloader->loadModel(_filename, cm, mc->getModel());
			_hier = _obj->copyHierarchy();
			mc->getNode()->addChild(_hier);
		} else {
			_obj = g_resourceloader->loadModel(_filename, cm);
			_hier = _obj->copyHierarchy();
			if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("Parent of model %s wasn't a mesh", _filename.c_str());
		}

		// Use parent availablity to decide whether to default the
		// component to being visible
		if (_parent)
			setKey(0);
		else
			setKey(1);
	}

	if (!_activeAnims) {
		_activeAnims = new Common::List<AnimationEntry>();
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

void ModelComponent::addActiveAnimation(AnimationState *anim, int priority1, int priority2) {
	// Keep the list of animations sorted by priorities in descending order. Because
	// the animations have two different priorities, we add the animation to the list
	// with both priorities.
	Common::List<AnimationEntry>::iterator i;
	AnimationEntry entry;
	entry._anim = anim;
	entry._priority = priority1;
	entry._tagged = false;
	for (i = _activeAnims->begin(); i != _activeAnims->end(); ++i) {
		if (i->_priority < entry._priority) {
			_activeAnims->insert(i, entry);
			break;
		}
	}
	if (i == _activeAnims->end())
		_activeAnims->push_back(entry);

	entry._priority = priority2;
	entry._tagged = true;
	for (i = _activeAnims->begin(); i != _activeAnims->end(); ++i) {
		if (i->_priority < entry._priority) {
			_activeAnims->insert(i, entry);
			break;
		}
	}
	if (i == _activeAnims->end())
		_activeAnims->push_back(entry);
}

void ModelComponent::removeActiveAnimation(AnimationState *anim) {
	Common::List<AnimationEntry>::iterator i;
	for (i = _activeAnims->begin(); i != _activeAnims->end(); ++i) {
		if (i->_anim == anim) {
			i = _activeAnims->erase(i);
			i--;
		}
	}
}

void ModelComponent::animate() {
	// First reset the current animation.
	for (int i = 0; i < getNumNodes(); i++) {
		_hier[i]._animPos.set(0,0,0);
		_hier[i]._animPitch = 0;
		_hier[i]._animYaw = 0;
		_hier[i]._animRoll = 0;
	}

	// Apply animation to each hierarchy node separately.
	for (int i = 0; i < getNumNodes(); i++) {
		Graphics::Vector3d tempPos;
		float tempYaw = 0.0f, tempPitch = 0.0f, tempRoll = 0.0f;
		float totalWeight = 0.0f;
		float remainingWeight = 1.0f;
		int currPriority = -1;

		// The animations are layered so that animations with a higher priority
		// are played regardless of the blend weights of lower priority animations.
		// The highest priority layer gets as much weight as it wants, while the
		// next layer gets the remaining amount and so on.
		for (Common::List<AnimationEntry>::iterator j = _activeAnims->begin(); j != _activeAnims->end(); ++j) {
			if (currPriority != j->_priority) {
				currPriority = j->_priority;
				remainingWeight *= 1 - totalWeight;
				if (remainingWeight <= 0.0f)
					break;

				float weightFactor = 1.0f;
				if (totalWeight > 1.0f) {
					weightFactor = 1.0f / totalWeight;
				}
				tempPos += _hier[i]._animPos * weightFactor;
				tempYaw += _hier[i]._animYaw * weightFactor;
				tempPitch += _hier[i]._animPitch * weightFactor;
				tempRoll += _hier[i]._animRoll * weightFactor;
				_hier[i]._animPos.set(0,0,0);
				_hier[i]._animYaw = 0.0f;
				_hier[i]._animPitch = 0.0f;
				_hier[i]._animRoll = 0.0f;
				totalWeight = 0.0f;
			}

			float time = j->_anim->_time / 1000.0f;
			float weight = j->_anim->_fade * remainingWeight;
			if (j->_anim->_keyf->animate(_hier, i, time, weight, j->_tagged))
				totalWeight += j->_anim->_fade;
		}

		float weightFactor = 1.0f;
		if (totalWeight > 1.0f) {
			weightFactor = 1.0f / totalWeight;
		}
		_hier[i]._animPos = _hier[i]._animPos * weightFactor + tempPos;
		_hier[i]._animYaw = _hier[i]._animYaw * weightFactor + tempYaw;
		_hier[i]._animPitch = _hier[i]._animPitch * weightFactor + tempPitch;
		_hier[i]._animRoll = _hier[i]._animRoll * weightFactor + tempRoll;
	}
}

void ModelComponent::resetColormap() {
	CMap *cm;

	cm = getCMap();
	if (_obj && cm)
		_obj->reload(cm);
}

void ModelComponent::restoreState(SaveGame *state) {
	_hier->_hierVisible = _visible;
}

ModelComponent::~ModelComponent() {
	if (_hier && _hier->_parent) {
		_hier->_parent->removeChild(_hier);
	}

	delete[] _hier;
}

void translateObject(Model::HierNode *node, bool reset) {
	if (node->_parent)
		translateObject(node->_parent, reset);

	if (reset) {
		g_driver->translateViewpointFinish();
	} else {
		Graphics::Vector3d animPos = node->_pos + node->_animPos;
		float animPitch = node->_pitch + node->_animPitch;
		float animYaw = node->_yaw + node->_animYaw;
		float animRoll = node->_roll + node->_animRoll;
		g_driver->translateViewpointStart(animPos, animPitch, animYaw, animRoll);
	}
}

void ModelComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it

	if (_parent && _parent->isVisible())
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
		ModelComponent(p, parentID, filename, prevComponent, t), _hierShared(false), _parentModel(NULL) {
	if (parentID == -2 && prevComponent) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);

		if (mmc && mmc->_filename == filename) {
			_activeAnims = mmc->_activeAnims;
			_obj = mmc->_obj;
			_hier = mmc->_hier;
			_hierShared = true;
			mmc->_children.push_back(this);
			_parentModel = mmc;
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

void MainModelComponent::setColormap(CMap *cmap) {
	Component::setColormap(cmap);
	if (_parentModel) {
		_parentModel->setColormap(cmap);
	}
}

void MainModelComponent::reset() {
	_visible = true;
	_hier->_hierVisible = _visible;
}

MainModelComponent::~MainModelComponent() {
	if (_hierShared)
		_hier = NULL; // Keep ~ModelComp from deleting it

	for (Common::List<MainModelComponent*>::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->_hier = NULL;
		(*i)->_parentModel = NULL;
	}

	if (_parentModel) {
		_parentModel->_children.remove(this);
	}
}

class MaterialComponent : public Costume::Component {
public:
	MaterialComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	void init();
	void setKey(int val);
	void setupTexture();
	void reset();
	void resetColormap();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	~MaterialComponent() { }

private:
	Material *_mat;
	Common::String _filename;
	int _num;
};

ColormapComponent::ColormapComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t) {
	_cmap = g_resourceloader->getColormap(filename);

	// Set the colormap here in the ctor and not in init()!
	if (p)
		p->setColormap(_cmap);
}

ColormapComponent::~ColormapComponent() {
}

void ColormapComponent::init() {
	if (!_parent)
		warning("No parent to apply colormap object on. CMap: %s, Costume: %s",
			_cmap->getFilename().c_str(), _cost->getFilename().c_str());
}

class KeyframeComponent : public Costume::Component {
public:
	enum RepeatMode {
		Once = 0,
		Looping = 1,
		PauseAtEnd = 2,
		FadeAtEnd = 3
	};
	enum FadeMode {
		None = 0,
		FadeIn = 1,
		FadeOut = 2
	};

	KeyframeComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	~KeyframeComponent();
	void init();
	void play(RepeatMode repeatMode);
	void fade(FadeMode, int fadeLength);
	void setKey(int val);
	void update();
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	void activate();
	void deactivate();

private:
	AnimationState _anim;
	int _priority1, _priority2;
	Model::HierNode *_hier;
	int _numNodes;
	bool _active;
	bool _paused;
	RepeatMode _repeatMode;
	FadeMode _fadeMode;
	int _fadeLength;
	Common::String _fname;

	friend class Costume;
};

KeyframeComponent::KeyframeComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _priority1(1), _priority2(5), _hier(NULL), _active(false),
		_paused(false), _repeatMode(Once), _fadeMode(None) {
	_fname = filename;
	const char *comma = strchr(filename, ',');
	if (comma) {
		Common::String realName(filename, comma);
		_anim._keyf = g_resourceloader->getKeyframe(realName.c_str());
		sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
	} else
		_anim._keyf = g_resourceloader->getKeyframe(filename);
}

KeyframeComponent::~KeyframeComponent() {
	deactivate();
}

void KeyframeComponent::play(RepeatMode repeatMode) {
	_repeatMode = repeatMode;
	if (_repeatMode != Looping)
		_anim._time = -1;
	_paused = false;
	activate();
}

void KeyframeComponent::fade(FadeMode fadeMode, int fadeLength) {
	if (!_active) {
		if (fadeMode == FadeIn) {
			_repeatMode = Once;
			_anim._time = -1;
			_anim._fade = 0.f;
			_paused = false;
			activate();
		} else {
			return;
		}
	}
	_fadeMode = fadeMode;
	_fadeLength = fadeLength;
}

void KeyframeComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play Once"
		play(Once);
		break;
	case 1: // "Play Looping"
		play(Looping);
		break;
	case 2: // "Play and Endpause"
		play(PauseAtEnd);
		break;
	case 3: // "Play and Endfade"
		play(FadeAtEnd);
		break;
	case 4: // "Stop"
		reset();
		break;
	case 5: // "Pause"
		_paused = true;
		break;
	case 6: // "Unpause"
		_paused = false;
		break;
	case 7: // "1.0 Fade in"
		fade(FadeIn, 1000);
		break;
	case 8: // "0.5 Fade in"
		fade(FadeIn, 500);
		break;
	case 9: // "0.25 Fade in"
		fade(FadeIn, 250);
		break;
	case 10: // "0.125 Fade in"
		fade(FadeIn, 125);
		break;
	case 11: // "1.0 Fade out"
		fade(FadeOut, 1000);
		break;
	case 12: // "0.5 Fade out
		fade(FadeOut, 500);
		break;
	case 13: // "0.25 Fade out"
		fade(FadeOut, 250);
		break;
	case 14: // "0.125 Fade out"
		fade(FadeOut, 125);
		break;
	default:
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Unknown key %d for keyframe %s", val, _anim._keyf->getFilename().c_str());
	}
}

void KeyframeComponent::reset() {
	if (_fadeMode != FadeOut) {
		_fadeMode = None;
		_anim._time = -1;
		_anim._fade = 1.f;
		_paused = false;
		deactivate();
	}
}

void KeyframeComponent::update() {
	if (!_active)
		return;

	int time = g_grim->getFrameTime() * g_currentUpdatedActor->getTimeScale();

	if (_anim._time < 0)		// For first time through
		_anim._time = 0;
	else if (!_paused)
		_anim._time += time;

	int animLength = (int)(_anim._keyf->getLength() * 1000);

	if (_fadeMode != None) {
		if (_fadeMode == FadeIn) {
			_anim._fade += (float)time / (float)_fadeLength;
			if (_anim._fade >= 1.f) {
				_anim._fade = 1.f;
				_fadeMode = None;
			}
		} else {
			_anim._fade -= (float)time / (float)_fadeLength;
			if (_anim._fade <= 0.f) {
				_anim._fade = 1.f;
				_fadeMode = None;
				deactivate();
				return;
			}
		}
	} else {
		_anim._fade = 1.f;
	}

	if (_anim._time > animLength) { // What to do at end?
		switch (_repeatMode) {
			case Once:
				if (_fadeMode == None)
					deactivate();
				else
					_anim._time = animLength;
				break;
			case Looping:
				do
					_anim._time -= animLength;
				while (_anim._time > animLength);
				break;
			case PauseAtEnd:
				_anim._time = animLength;
				_paused = true;
				break;
			case FadeAtEnd:
				if (_fadeMode != FadeOut) {
					_fadeMode = FadeOut;
					_fadeLength = 250;
				}
				_anim._time = animLength;
				break;
			default:
				if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
					warning("Unknown repeat mode %d for keyframe %s", _repeatMode, _anim._keyf->getFilename().c_str());
		}
	}
}

void KeyframeComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc) {
		_hier = mc->getHierarchy();
		_numNodes = mc->getNumNodes();
	} else {
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Parent of %s was not a model", _anim._keyf->getFilename().c_str());
		_hier = NULL;
		_numNodes = 0;
	}

	_anim._time = -1;
	_anim._fade = 1.f;
}

void KeyframeComponent::saveState(SaveGame *state) {
	state->writeLESint32(_active);
	state->writeLESint32((int)_repeatMode);
	state->writeLESint32(_anim._time);
	state->writeLESint32((int)_fadeMode);
#if 1
	// Remove on next save format change.
	state->writeLESint32(0);
#else
	// Enable on next save format change.
	state->writeFloat(_anim._fade);
#endif
	state->writeLESint32(_fadeLength);
	state->writeLESint32(_paused);
}

void KeyframeComponent::restoreState(SaveGame *state) {
	bool active = state->readLESint32();
	_repeatMode = (RepeatMode)state->readLESint32();
	_anim._time = state->readLESint32();
	_fadeMode = (FadeMode)state->readLESint32();
#if 1
	// Remove on next save format change.
	state->readLESint32();
#else
	// Enable on next save format change.
	_anim._fade = state->readFloat();
#endif
	_fadeLength = state->readLESint32();
	_paused = state->readLESint32();

	if (active)
		activate();
}

void KeyframeComponent::activate() {
	if (!_active) {
		_active = true;
		ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
		if (mc)
			 mc->addActiveAnimation(&_anim, _priority1, _priority2);
	}
}

void KeyframeComponent::deactivate() {
	if (_active) {
		_active = false;
		ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
		if (mc)
			 mc->removeActiveAnimation(&_anim);
	}
}

MeshComponent::MeshComponent(Costume::Component *p, int parentID, const char *name, tag32 t) :
		Costume::Component(p, parentID, t), _name(name), _node(NULL) {
	if (sscanf(name, "mesh %d", &_num) < 1)
		error("Couldn't parse mesh name %s", name);

}

void MeshComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc) {
		_node = mc->getHierarchy() + _num;
		_model = mc->getModel();
	} else {
		if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Parent of mesh %d was not a model", _num);
		_node = NULL;
		_model = NULL;
	}
}

void MeshComponent::setKey(int val) {
	_node->_meshVisible = (val != 0);
}

void MeshComponent::reset() {
	// NOTE: Setting the visibility to true here causes a bug with the thunderboy costume:
	// closing the inventory causes the hat to appear, while it shouldn't.
	// This could however introduce regressions somewhere else, so if there is something
	// disappearing or not behaving properly in a costume the cause might be here.
// 	_node->_meshVisible = true;
}

void MeshComponent::update() {
	_node->setMatrix(_matrix);
	_node->update();
}

void MeshComponent::saveState(SaveGame *state) {
	state->writeLESint32(_node->_meshVisible);
}

void MeshComponent::restoreState(SaveGame *state) {
	_node->_meshVisible = state->readLESint32();
}

MaterialComponent::MaterialComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename),
		_num(0) {

	if (gDebugLevel == DEBUG_MODEL || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("Constructing MaterialComponent %s", filename);
}

void MaterialComponent::init() {
	_mat = NULL;
	if (FROM_BE_32(_parent->getTag()) == MKTAG('M','M','D','L') ||
		FROM_BE_32(_parent->getTag()) == MKTAG('M','O','D','L')) {
		ModelComponent *p = static_cast<ModelComponent *>(_parent);
		Model *model = p->getModel();
		if (model) {
			for (int i = 0; i < model->_numMaterials; ++i) {
				if (_filename.compareToIgnoreCase(model->_materials[i]->getFilename()) == 0) {
					_mat = model->_materials[i];
					return;
				}
			}
		}
	} else {
		warning("Parent of a MaterialComponent not a ModelComponent. %s %s", _filename.c_str(), _cost->getFilename().c_str());
	}
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

void MaterialComponent::resetColormap() {
	init();
}

void MaterialComponent::saveState(SaveGame *state) {
	state->writeLESint32(_num);
}

void MaterialComponent::restoreState(SaveGame *state) {
	_num = state->readLESint32();
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
	lua_setglobal(_name.c_str());
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
		if (g_grim->getCurrScene() && g_currentUpdatedActor) {
			Graphics::Vector3d pos = g_currentUpdatedActor->getPos();
			g_grim->getCurrScene()->setSoundPosition(_soundName.c_str(), pos);
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
	if (g_imuse && g_imuse->getSoundStatus(_soundName.c_str()))
		g_imuse->stopSound(_soundName.c_str());
}

Costume::Costume(const Common::String &fname, const char *data, int len, Costume *prevCost) :
		Object() {

	_fname = fname;
	_head.maxPitch = 0;
	_head.joint1 = -1;
	_head.joint2 = -1;
	_head.joint3 = -1;
	_joint1Node = NULL;
	_joint2Node = NULL;
	_joint3Node = NULL;
	_headYaw = 0;
	_headPitch = 0;
	_prevCostume = prevCost;
	if (g_grim->getGameType() == GType_MONKEY4) {
		Common::MemoryReadStream ms((const byte *)data, len);
		loadEMI(ms, prevCost);
	} else {
		TextSplitter ts(data, len);
		loadGRIM(ts, prevCost);
	}
}

void Costume::loadGRIM(TextSplitter &ts, Costume *prevCost) {
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
		const char *line = ts.getCurrentLine();
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
		_components[id]->setCostume(this);
	}

	delete[] tags;

	for (int i = 0; i < _numComponents; i++)
		if (_components[i]) {
			_components[i]->init();
		}

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
		_chores[which].load(i, this, ts);
	}
}

void Costume::loadEMI(Common::MemoryReadStream &ms, Costume *prevCost) {
	Common::List<Component *>components;

	_numChores = ms.readUint32LE();
	_chores = new Chore[_numChores];
	for (int i = 0; i < _numChores; i++) {
		uint32 nameLength;
		Component *prevComponent = NULL;
		nameLength = ms.readUint32LE();
		ms.read(_chores[i]._name, nameLength);
		float length;
		ms.read(&length, 4);
		_chores[i]._length = (int)length;

		_chores[i]._owner = this;
		_chores[i]._numTracks = ms.readUint32LE();
		_chores[i]._tracks = new ChoreTrack[_chores[i]._numTracks];

		for (int k = 0; k < _chores[i]._numTracks; k++) {
			int componentNameLength = ms.readUint32LE();
			assert(componentNameLength < 64);

			char name[64];
			ms.read(name, componentNameLength);

			ms.readUint32LE();
			int parentID = ms.readUint32LE();
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
			Component *component = loadComponentEMI(parentID < 0 ? NULL : _components[parentID], parentID, name, prevComponent);


			//Component *component = loadComponentEMI(name, parent);

			components.push_back(component);

			ChoreTrack &track = _chores[i]._tracks[k];
			track.numKeys = ms.readUint32LE();
			track.keys = new TrackKey[track.numKeys];

			// this is probably wrong
			track.compID = 0;
			for (int j = 0; j < track.numKeys; j++) {
				float time, value;
				ms.read(&time, 4);
				ms.read(&value, 4);
				track.keys[j].time = (int)time;
				track.keys[j].value = (int)value;
			}
		}
		//_chores[i]._tracks->compID;
	}

	_numComponents = components.size();
	_components = new Component *[_numComponents];
	int i = 0;
	for (Common::List<Component *>::iterator it = components.begin(); it != components.end(); ++it, ++i) {
		_components[i] = *it;
		if (!_components[i])
			continue;
		_components[i]->setCostume(this);
		_components[i]->init();
	}
}

Costume::~Costume() {
	if (_chores) {
		stopChores();
		for (int i = _numComponents - 1; i >= 0; i--) {
			// The "Sprite" component can be NULL
			if (_components[i])
				delete _components[i];
		}
		delete[] _components;
		delete[] _chores;
	}
}

Costume::Component::Component(Component *p, int parentID, tag32 t)  {
	_visible = true;
	_previousCmap = NULL;
	_cmap = NULL;
	_cost = NULL;
	_parent = NULL;
	_parentID = parentID;
	setParent(p);
	_tag = t;
}

Costume::Component::~Component()
{
	if (_parent)
		_parent->removeChild(this);

	Component *child = _child;
	while (child) {
		child->_parent = NULL;
		child = child->_sibling;
	}
}

void Costume::Component::setColormap(CMap *c) {
	if (c)
		_cmap = c;
	if (getCMap()) {
		resetColormap();
		resetHierCMap();
	}
}

bool Costume::Component::isVisible() {
	if (_visible && _parent)
		return _parent->isVisible();
	return _visible;
}

CMap *Costume::Component::getCMap() {
	if (!_cmap && _previousCmap)
		return _previousCmap;
	else if (!_cmap && _parent)
		return _parent->getCMap();
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
		_sibling = _parent->_child;
		_parent->_child = this;
	}
}

void Costume::Component::removeChild(Component *child) {
	Component **childPos = &_child;
	while (*childPos && *childPos != child)
		childPos = &(*childPos)->_sibling;
	if (*childPos) {
		*childPos = child->_sibling;
		child->_parent = NULL;
	}
}

void Costume::Component::resetHierCMap() {
	resetColormap();

	Component *child = _child;
	while (child) {
		child->resetHierCMap();
		child = child->_sibling;
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

void Costume::Chore::load(int id, Costume *owner, TextSplitter &ts) {
	_owner = owner;
	_tracks = new ChoreTrack[_numTracks];
	_hasPlayed = _playing = false;
	_id = id;
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
	// 	if (_hasPlayed)
	// 		return;

	// This comment above is perfectly right, but unfortunately doing that
	// breaks glottis movements when he answers to "i'm calavera, manny calavera".
	// Moreover, the choppy behaviour stated above happens with grim original too,
	// meaning the bug is not in Residual but in the scripts or in GrimE design.

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

	int time = g_grim->getFrameTime() * g_currentUpdatedActor->getTimeScale();

	int newTime;
	if (_currTime < 0)
		newTime = 0; // For first time through
	else
		newTime = _currTime + time;

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

void Costume::Chore::fadeIn(int msecs) {
	if (!_playing)
	{
		_playing = true;
		_hasPlayed = true;
		_currTime = -1;
	}

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (!comp)
			continue;

		if (FROM_BE_32(comp->getTag()) == MKTAG('K','E','Y','F')) {
			KeyframeComponent* kf = static_cast<KeyframeComponent*>(comp);
			kf->fade(KeyframeComponent::FadeIn, msecs);
		}
	}
}

void Costume::Chore::fadeOut(int msecs) {
	// Note: It doesn't matter whether the chore is playing or not. The keyframe
	// components should fade out in either case.
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (!comp)
			continue;

		if (FROM_BE_32(comp->getTag()) == MKTAG('K','E','Y','F')) {
			KeyframeComponent* kf = static_cast<KeyframeComponent*>(comp);
			kf->fade(KeyframeComponent::FadeOut, msecs);
		}
	}
}

Costume::Component *Costume::loadComponent (tag32 tag, Costume::Component *parent, int parentID, const char *name, Costume::Component *prevComponent) {
	if (FROM_BE_32(tag) == MKTAG('M','M','D','L'))
		return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','O','D','L'))
		return new ModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKTAG('C','M','A','P'))
		return new ColormapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('K','E','Y','F'))
		return new KeyframeComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','E','S','H'))
		return new MeshComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('L','U','A','V'))
		return new LuaVarComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('I','M','L','S'))
		return new SoundComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('B','K','N','D'))
		return new BitmapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','A','T',' '))
		return new MaterialComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('S','P','R','T'))
		return new SpriteComponent(parent, parentID, name, tag);

	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);
	return NULL;
}

Costume::Component *Costume::loadComponentEMI(Costume::Component *parent, int parentID, const char *name, Costume::Component *prevComponent) {
	// some have an exclimation mark, this could mean something.
	// for now, return 0 otherwise it will just crash in some other part.
	return 0;

	assert(name[0] == '!');
	++name;

	char type[5];
	memcpy(type, name, 4);
	type[4] = 0;

	name += 4;

	if (strcmp(type, "mesh") == 0)
		return new MainModelComponent(0, parentID, name, 0, 0);
	/*if (FROM_BE_32(tag) == MKTAG('M','M','D','L'))
	return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','O','D','L'))
	return new ModelComponent(parent, parentID, name, prevComponent, tag);
	else if (FROM_BE_32(tag) == MKTAG('C','M','A','P'))
	return new ColormapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('K','E','Y','F'))
	return new KeyframeComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','E','S','H'))
	return new MeshComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('L','U','A','V'))
	return new LuaVarComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('I','M','L','S'))
	return new SoundComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('B','K','N','D'))
	return new BitmapComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('M','A','T',' '))
	return new MaterialComponent(parent, parentID, name, tag);
	else if (FROM_BE_32(tag) == MKTAG('S','P','R','T'))
	return NULL;// new SpriteComponent(parent, parentID, name);

	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);*/
	return NULL;
}

Model::HierNode *Costume::getModelNodes() {
	for (int i = 0; i < _numComponents; i++) {
		if (!_components[i])
			continue;
		// Needs to handle Main Models (pigeons) and normal Models
		// (when Manny climbs the rope)
		if (FROM_BE_32(_components[i]->getTag()) == MKTAG('M','M','D','L'))
			return dynamic_cast<ModelComponent *>(_components[i])->getHierarchy();
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
	if (Common::find(_playingChores.begin(), _playingChores.end(), &_chores[num]) == _playingChores.end())
		_playingChores.push_back(&_chores[num]);
}

void Costume::playChore(const char *name) {
	for (int i = 0; i < _numChores; ++i) {
			if (strcmp(_chores[i]._name, name) == 0) {
			playChore(i);
			return;
		}
	}
	warning("Costume::playChore: Could not find chore: %s", name);
	return;
}

void Costume::playChore(int num) {
	if (num < 0 || num >= _numChores) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num].play();
	if (Common::find(_playingChores.begin(), _playingChores.end(), &_chores[num]) == _playingChores.end())
		_playingChores.push_back(&_chores[num]);
}

void Costume::stopChore(int num) {
	if (num < 0 || num >= _numChores) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num].stop();
	_playingChores.remove(&_chores[num]);
}

void Costume::setColormap(const Common::String &map) {
	// Sometimes setColormap is called on a null costume,
	// see where raoul is gone in hh.set
	if (!map.size())
		return;
	_cmap = g_resourceloader->getColormap(map);
	for (int i = 0; i < _numComponents; i++)
		_components[i]->setColormap(NULL);
}

void Costume::stopChores() {
	for (int i = 0; i < _numChores; i++) {
		_chores[i].stop();
		_playingChores.remove(&_chores[i]);
	}
}

void Costume::fadeChoreIn(int chore, int msecs) {
	if (chore >= _numChores) {
		if (chore < 0 || chore >= _numChores) {
			if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
			return;
		}
	}
	_chores[chore].fadeIn(msecs);
	if (Common::find(_playingChores.begin(), _playingChores.end(), &_chores[chore]) == _playingChores.end())
		_playingChores.push_back(&_chores[chore]);
}

void Costume::fadeChoreOut(int chore, int msecs) {
	if (chore >= _numChores) {
		if (chore < 0 || chore >= _numChores) {
			if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
			return;
		}
	}
	_chores[chore].fadeOut(msecs);
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
	for (Common::List<Chore*>::iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		(*i)->update();
		if (!(*i)->_playing) {
			i = _playingChores.erase(i);
			--i;
		}
	}

	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
			_components[i]->setMatrix(_matrix);
			_components[i]->update();
		}
	}
}

void Costume::animate() {
	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
			_components[i]->animate();
		}
	}
}

void Costume::moveHead(bool lookingMode, const Graphics::Vector3d &lookAt, float rate) {
	if (_joint1Node) {
		float step = g_grim->getPerSecond(rate);
		float yawStep = step;
		float pitchStep = step / 3.f;
		if (!lookingMode) {
			//animate yaw
			if (_headYaw > yawStep) {
				_headYaw -= yawStep;
			} else if (_headYaw < -yawStep) {
				_headYaw += yawStep;
			} else {
				_headYaw = 0;
			}
			//animate pitch
			if (_headPitch > pitchStep) {
				_headPitch -= pitchStep;
			} else if (_headPitch < -pitchStep) {
				_headPitch += pitchStep;
			} else {
				_headPitch = 0;
			}
			_joint1Node->_animYaw = _headYaw;
			float pi = _headPitch / 3.f;
			_joint1Node->_animPitch += pi;
			_joint2Node->_animPitch += pi;
			_joint3Node->_animPitch += pi;
			_joint1Node->_animRoll = (_joint1Node->_animYaw / 20.f) * -_headPitch / 5.f;

			if (_joint1Node->_animRoll > _head.maxRoll)
				_joint1Node->_animRoll = _head.maxRoll;
			if (_joint1Node->_animRoll < -_head.maxRoll)
				_joint1Node->_animRoll = -_head.maxRoll;
			return;
		}

		Model::HierNode *p = _joint3Node;
		while (p->_parent) {
			p = p->_parent;
		}
		p->setMatrix(_matrix);
		p->update();

		Graphics::Vector3d v =  lookAt - _joint3Node->_matrix._pos;
		if (v.isZero()) {
			return;
		}

		float magnitude = sqrt(v.x() * v.x() + v.y() * v.y());
		float a = v.x() / magnitude;
		float b = v.y() / magnitude;
		float yaw;
		yaw = acos(a) * (180.0f / LOCAL_PI);
		if (b < 0.0f)
			yaw = 360.0f - yaw;

		float bodyYaw = _matrix._rot.getYaw();
		p = _joint1Node->_parent;
		while (p) {
			bodyYaw += p->_yaw + p->_animYaw;
			p = p->_parent;
		}

		_joint1Node->_animYaw = (- 90 + yaw - bodyYaw);
		if (_joint1Node->_animYaw < -180.) {
			_joint1Node->_animYaw += 360;
		}
		if (_joint1Node->_animYaw > 180.) {
			_joint1Node->_animYaw -= 360;
		}

		if (_joint1Node->_animYaw > _head.maxYaw)
			_joint1Node->_animYaw = _head.maxYaw;
		if (_joint1Node->_animYaw < -_head.maxYaw)
			_joint1Node->_animYaw = -_head.maxYaw;

		float sqLenght = v.x() * v.x() + v.y() * v.y();
		float h;
		if (sqLenght > 0) {
			h = sqrt(sqLenght);
		} else {
			h = -sqrt(sqLenght);
		}
		magnitude = sqrt(v.z() * v.z() + h * h);
		a = h / magnitude;
		b = v.z() / magnitude;
		float pitch;
		pitch = acos(a) * (180.0f / LOCAL_PI);

		if (b < 0.0f)
			pitch = 360.0f - pitch;

		if (pitch > 180)
			pitch -= 360;

		if (pitch > _head.maxPitch)
			pitch = _head.maxPitch;
		if (pitch < -_head.maxPitch)
			pitch = -_head.maxPitch;

		if ((_joint1Node->_animYaw > 0 && pitch < 0) || (_joint1Node->_animYaw < 0 && pitch > 0)) {
			pitch += _joint1Node->_animYaw / 10.f;
		} else {
			pitch -= _joint1Node->_animYaw / 10.f;
		}

		//animate pitch
		if (pitch - _headPitch > pitchStep)
			pitch = _headPitch + pitchStep;
		if (_headPitch - pitch > pitchStep)
			pitch = _headPitch - pitchStep;

		float pi = pitch / 3.f;
		_joint1Node->_animPitch += pi;
		_joint2Node->_animPitch += pi;
		_joint3Node->_animPitch += pi;

		//animate yaw
		if (_joint1Node->_animYaw - _headYaw > yawStep)
			_joint1Node->_animYaw = _headYaw + yawStep;
		if (_headYaw - _joint1Node->_animYaw > yawStep)
			_joint1Node->_animYaw = _headYaw - yawStep;

		_joint1Node->_animRoll = (_joint1Node->_animYaw / 20.f) * -pitch / 5.f;

		if (_joint1Node->_animRoll > _head.maxRoll)
			_joint1Node->_animRoll = _head.maxRoll;
		if (_joint1Node->_animRoll < -_head.maxRoll)
			_joint1Node->_animRoll = -_head.maxRoll;

		_headPitch = pitch;
		_headYaw = _joint1Node->_animYaw;
	}
}

void Costume::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	_head.joint1 = joint1;
	_head.joint2 = joint2;
	_head.joint3 = joint3;
	_head.maxRoll = maxRoll;
	_head.maxPitch = maxPitch;
	_head.maxYaw = maxYaw;

	if (joint1 >= 0 && joint2 >= 0 && joint3 >= 0) {
		Model::HierNode *nodes = getModelNodes();
		if (nodes) {
			_joint1Node = nodes + joint1;
			_joint2Node = nodes + joint2;
			_joint3Node = nodes + joint3;
		}
	}
}

void Costume::setPosRotate(Graphics::Vector3d pos, float pitch, float yaw, float roll) {
	_matrix._pos = pos;
	_matrix._rot.buildFromPitchYawRoll(pitch, yaw, roll);
}

Costume *Costume::getPreviousCostume() const {
	return _prevCostume;
}

void Costume::saveState(SaveGame *state) const {
	if (_cmap) {
		state->writeLEUint32(1);
		state->writeString(_cmap->getFilename());
	} else {
		state->writeLEUint32(0);
	}

	for (int i = 0; i < _numChores; ++i) {
		Chore &c = _chores[i];

		state->writeLESint32(c._hasPlayed);
		state->writeLESint32(c._playing);
		state->writeLESint32(c._looping);
		state->writeLESint32(c._currTime);
#if 1
		// Remove on next save format change.
		state->writeLESint32(0);
		state->writeLESint32(0);
		state->writeFloat(0.f);
#endif
	}

	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			state->writeLESint32(c->_visible);
			state->writeVector3d(c->_matrix._pos);
#if 1
			// Remove on next save format change.
			state->writeFloat(0.f);
#endif
			c->saveState(state);
		}
	}

	state->writeLEUint32(_playingChores.size());
	for (Common::List<Chore*>::const_iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		state->writeLESint32((*i)->_id);
	}

	state->writeLESint32(_head.joint1);
	state->writeLESint32(_head.joint2);
	state->writeLESint32(_head.joint3);
	state->writeFloat(_head.maxPitch);
	state->writeFloat(_head.maxYaw);
	state->writeFloat(_head.maxRoll);
}

bool Costume::restoreState(SaveGame *state) {
	if (state->readLEUint32()) {
		Common::String str = state->readString();
		setColormap(str);
	}

	for (int i = 0; i < _numChores; ++i) {
		Chore &c = _chores[i];

		c._hasPlayed = state->readLESint32();
		c._playing = state->readLESint32();
		c._looping = state->readLESint32();
		c._currTime = state->readLESint32();
#if 1
		// Remove on next save format change.
		state->readLESint32();
		state->readLESint32();
		state->readFloat();
#endif
	}
	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			c->_visible = state->readLESint32();
			c->_matrix._pos = state->readVector3d();
#if 1
			// Remove on next save format change.
			state->readFloat();
#endif

			c->restoreState(state);
		}
	}

	int numPlayingChores = state->readLEUint32();
	for (int i = 0; i < numPlayingChores; ++i) {
		int id = state->readLESint32();
		_playingChores.push_back(&_chores[id]);
	}

	int j1 = state->readLESint32();
	int j2 = state->readLESint32();
	int j3 = state->readLESint32();
	float mP = state->readFloat();
	float mY = state->readFloat();
	float mR = state->readFloat();
	setHead(j1, j2, j3, mR, mP, mY);

	return true;
}

} // end of namespace Grim
