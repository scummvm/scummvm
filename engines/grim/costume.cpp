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

#include "engines/grim/debug.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/actor.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/grim.h"
#include "engines/grim/keyframe.h"
#include "engines/grim/material.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"
#include "engines/grim/model.h"
#include "engines/grim/set.h"
#include "engines/grim/objectstate.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/animation.h"

#include "engines/grim/lua/lua.h"

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
// For bitmaps, the actual drawing is handled by the Set class.  The
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

class ModelComponent : public Costume::Component {
public:
	ModelComponent(Costume::Component *parent, int parentID, const char *filename, Costume::Component *prevComponent, tag32 tag);
	void init();
	void setKey(int val);
	void animate();
	void reset();
	void resetColormap();
	void setMatrix(Math::Matrix4 matrix) { _matrix = matrix; };
	void restoreState(SaveGame *state);
	void translateObject(bool reset);
	static void translateObject(ModelNode *node, bool reset);
	AnimManager *getAnimManager() const;
	~ModelComponent();

	ModelNode *getHierarchy() { return _hier; }
	int getNumNodes() { return _obj->getNumNodes(); }
	Model *getModel() { return _obj; }
	void draw();
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2);

protected:
	Common::String _filename;
	Model *_obj;
	ModelNode *_hier;
	Math::Matrix4 _matrix;
	AnimManager *_animation;
	Component *_prevComp;
};

class MainModelComponent : public ModelComponent {
public:
	MainModelComponent(Costume::Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag);
	void init();
	int update(float time);
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
	int update(float time);
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	~MeshComponent() { }

	void setMatrix(Math::Matrix4 matrix) { _matrix = matrix; };

	ModelNode *getNode() { return _node; }
	Model *getModel() { return _model; }

private:
	Common::String _name;
	int _num;
	Model *_model;
	ModelNode *_node;
	Math::Matrix4 _matrix;
};

BitmapComponent::BitmapComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename) {
}

void BitmapComponent::setKey(int val) {
	const char *bitmap = _filename.c_str();
	ObjectState *state = g_grim->getCurrSet()->findState(bitmap);

	if (state) {
		state->setActiveImage(val);
		return;
	}
	// Complain that we couldn't find the bitmap.  This means we probably
	// didn't handle something correctly.  Example: Before the tube-switcher
	// bitmaps were not loading with the scene. This was because they were requested
	// as a different case then they were stored (tu_0_dorcu_door_open versus
	// TU_0_DORCU_door_open), which was causing problems in the string comparison.
	Debug::warning(Debug::Bitmaps | Debug::Costumes, "Missing scene bitmap: %s", bitmap);

/* In case you feel like drawing the missing bitmap anyway...
	// Assume that all objects the scene file forgot about are OBJSTATE_STATE class
	state = new ObjectState(0, ObjectState::OBJSTATE_STATE, bitmap, NULL, true);
	if (!state) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Couldn't find bitmap %s in current scene", _filename.c_str());
		return;
	}
	g_grim->getCurrSet()->addObjectState(state);
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
			else
				Debug::warning(Debug::Costumes, "Parent of sprite %s wasn't a mesh", _filename.c_str());
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
		_sprite->_material->setActiveTexture(val - 1);
	}
}

void SpriteComponent::reset() {
	if (_sprite)
		_sprite->_visible = false;
}

void SpriteComponent::saveState(SaveGame *state) {
	state->writeLEBool(_sprite->_visible);
	state->writeLESint32(_sprite->_material->getActiveTexture());
}

void SpriteComponent::restoreState(SaveGame *state) {
	_sprite->_visible = state->readLEBool();
	_sprite->_material->setActiveTexture(state->readLESint32());
}

ModelComponent::ModelComponent(Costume::Component *p, int parentID, const char *filename, Costume::Component *prevComponent, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename),
		_obj(NULL), _hier(NULL), _animation(NULL) {
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
	_prevComp = prevComponent;
}

void ModelComponent::init() {
	if (_prevComp) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(_prevComp);

		if (mmc) {
			_previousCmap = mmc->getCMap();
		}
	}
	// Skip loading if it was initialized
	// by the sharing MainModelComponent
	// constructor before
	if (!_obj) {
		CMapPtr cm = getCMap();

		// Get the default colormap if we haven't found
		// a valid colormap
		if (!cm && g_grim->getCurrSet())
			cm = g_grim->getCurrSet()->getCMap();
		if (!cm) {
			Debug::warning(Debug::Costumes, "No colormap specified for %s, using %s", _filename.c_str(), DEFAULT_COLORMAP);

			cm = g_resourceloader->getColormap(DEFAULT_COLORMAP);
		}

		// If we're the child of a mesh component, put our nodes in the
		// parent object's tree.
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			_obj = g_resourceloader->loadModel(_filename, cm, mc->getModel());
			_hier = _obj->getHierarchy();
			mc->getNode()->addChild(_hier);
		} else {
			_obj = g_resourceloader->loadModel(_filename, cm);
			_hier = _obj->getHierarchy();
			Debug::warning(Debug::Costumes, "Parent of model %s wasn't a mesh", _filename.c_str());
		}

		// Use parent availablity to decide whether to default the
		// component to being visible
		if (_parent)
			setKey(0);
		else
			setKey(1);
	}

	if (!_animation) {
		_animation = new AnimManager();
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

AnimManager *ModelComponent::getAnimManager() const {
	return _animation;
}

void ModelComponent::animate() {
	// First reset the current animation.
	for (int i = 0; i < getNumNodes(); i++) {
		_hier[i]._animPos.set(0,0,0);
		_hier[i]._animPitch = 0;
		_hier[i]._animYaw = 0;
		_hier[i]._animRoll = 0;
	}

	_animation->animate(_hier, getNumNodes());
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

	delete _obj;
	delete _animation;
}

void ModelComponent::translateObject(ModelNode *node, bool reset) {
	if (node->_parent)
		translateObject(node->_parent, reset);

	if (reset) {
		node->translateViewpointBack();
	} else {
		node->translateViewpoint();
	}
}

void ModelComponent::translateObject(bool res) {
	ModelNode *node = _hier->_parent;
	if (node) {
		translateObject(node, res);
	}
}

void ModelComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it

	if (_parent && _parent->isVisible())
			return;
	// Need to translate object to be in accordance
	// with the setup of the parent
	translateObject(false);

	_hier->draw();

	// Need to un-translate when done
	translateObject(true);
}

void ModelComponent::getBoundingBox(int *x1, int *y1, int *x2, int *y2) {
	// If the object was drawn by being a component
	// of it's parent then don't draw it

	if (_parent && _parent->isVisible())
		return;
	// Need to translate object to be in accordance
		// with the setup of the parent
	translateObject(false);

	_hier->getBoundingBox(x1, y1, x2, y2);

	// Need to un-translate when done
	translateObject(true);
}

MainModelComponent::MainModelComponent(Costume::Component *p, int parentID, const char *filename, Costume::Component *prevComponent, tag32 t) :
		ModelComponent(p, parentID, filename, prevComponent, t), _hierShared(false), _parentModel(NULL) {
	if (parentID == -2 && prevComponent) {
		MainModelComponent *mmc = dynamic_cast<MainModelComponent *>(prevComponent);

		if (mmc && mmc->_filename == filename) {
			_animation = mmc->_animation;
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

int MainModelComponent::update(float time) {
	if (!_hierShared)
		// Otherwise, it was already initialized
		// and reinitializing it will destroy work
		// from previous costumes
		ModelComponent::update(time);

	return 0;
}

void MainModelComponent::setColormap(CMap *cmap) {
	Component::setColormap(cmap);
	if (_parentModel) {
		_parentModel->setColormap(cmap);
	}
}

void MainModelComponent::reset() {
	_visible = true;
	// Can be NULL if this was attached to another costume which
	// was deleted.
	if (_hier) {
		_hier->_hierVisible = _visible;
	}
}

MainModelComponent::~MainModelComponent() {
	if (_hierShared) {
		_obj = NULL; // Keep ~ModelComp from deleting it
		_animation = NULL;
	}

	for (Common::List<MainModelComponent*>::iterator i = _children.begin(); i != _children.end(); ++i) {
		(*i)->_obj = NULL;
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
	void reset();
	void resetColormap();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	~MaterialComponent() { }

private:
	Material *_mat;
	Common::String _filename;
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
	KeyframeComponent(Costume::Component *parent, int parentID, const char *filename, tag32 tag);
	~KeyframeComponent();
	void init();
	void fade(Animation::FadeMode, int fadeLength);
	void setKey(int val);
	int update(float time);
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);

private:
	Animation *_anim;
	int _priority1, _priority2;
	Common::String _fname;

	friend class Costume;
};

KeyframeComponent::KeyframeComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _priority1(1), _priority2(5) {
	_fname = filename;
	const char *comma = strchr(filename, ',');
	if (comma) {
		_fname = Common::String(filename, comma);
		sscanf(comma + 1, "%d,%d", &_priority1, &_priority2);
	}
}

KeyframeComponent::~KeyframeComponent() {
	delete _anim;
}

void KeyframeComponent::fade(Animation::FadeMode fadeMode, int fadeLength) {
	_anim->fade(fadeMode, fadeLength);
}

void KeyframeComponent::setKey(int val) {
	switch (val) {
	case 0: // "Play Once"
		_anim->play(Animation::Once);
		break;
	case 1: // "Play Looping"
		_anim->play(Animation::Looping);
		break;
	case 2: // "Play and Endpause"
		_anim->play(Animation::PauseAtEnd);
		break;
	case 3: // "Play and Endfade"
		_anim->play(Animation::FadeAtEnd);
		break;
	case 4: // "Stop"
		reset();
		break;
	case 5: // "Pause"
		_anim->pause(true);
		break;
	case 6: // "Unpause"
		_anim->pause(false);
		break;
	case 7: // "1.0 Fade in"
		fade(Animation::FadeIn, 1000);
		_anim->activate();
		break;
	case 8: // "0.5 Fade in"
		fade(Animation::FadeIn, 500);
		_anim->activate();
		break;
	case 9: // "0.25 Fade in"
		fade(Animation::FadeIn, 250);
		_anim->activate();
		break;
	case 10: // "0.125 Fade in"
		fade(Animation::FadeIn, 125);
		_anim->activate();
		break;
	case 11: // "1.0 Fade out"
		fade(Animation::FadeOut, 1000);
		break;
	case 12: // "0.5 Fade out
		fade(Animation::FadeOut, 500);
		break;
	case 13: // "0.25 Fade out"
		fade(Animation::FadeOut, 250);
		break;
	case 14: // "0.125 Fade out"
		fade(Animation::FadeOut, 125);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for component %s", val, _fname.c_str());
	}
}

void KeyframeComponent::reset() {
	if (_anim->getFadeMode() != Animation::FadeOut) {
		_anim->stop();
	}
}

int KeyframeComponent::update(float time) {
	if (!_anim->getIsActive())
		return 0;

	return _anim->update(time);
}

void KeyframeComponent::init() {
	ModelComponent *mc = dynamic_cast<ModelComponent *>(_parent);
	if (mc) {
		_anim = new Animation(_fname, mc->getAnimManager(), _priority1, _priority2);
	} else {
		Debug::warning(Debug::Costumes, "Parent of %s was not a model", _fname.c_str());
		_anim = NULL;
	}
}

void KeyframeComponent::saveState(SaveGame *state) {
	_anim->saveState(state);
}

void KeyframeComponent::restoreState(SaveGame *state) {
	_anim->restoreState(state);
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
		Debug::warning(Debug::Costumes, "Parent of mesh %d was not a model", _num);
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

int MeshComponent::update(float /*time*/) {
	_node->setMatrix(_matrix);
	return 0;
}

void MeshComponent::saveState(SaveGame *state) {
	state->writeLESint32(_node->_meshVisible);
}

void MeshComponent::restoreState(SaveGame *state) {
	_node->_meshVisible = state->readLESint32();
}

MaterialComponent::MaterialComponent(Costume::Component *p, int parentID, const char *filename, tag32 t) :
		Costume::Component(p, parentID, t), _filename(filename) {

	Debug::debug(Debug::Costumes, "Constructing MaterialComponent %s", filename);
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
	_mat->setActiveTexture(val);
}

void MaterialComponent::reset() {
	_mat->setActiveTexture(0);
}

void MaterialComponent::resetColormap() {
	init();
}

void MaterialComponent::saveState(SaveGame *state) {
	state->writeLESint32(_mat->getActiveTexture());
}

void MaterialComponent::restoreState(SaveGame *state) {
	_mat->setActiveTexture(state->readLESint32());
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
		if (g_grim->getCurrSet()) {
			Math::Vector3d pos = _cost->getMatrix().getPosition();
			g_grim->getCurrSet()->setSoundPosition(_soundName.c_str(), pos);
		}
		break;
	case 1: // "Stop"
		g_imuse->stopSound(_soundName.c_str());
		break;
	case 2: // "Stop Looping"
		g_imuse->setHookId(_soundName.c_str(), 0x80);
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for sound %s", val, _soundName.c_str());
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
	_lookAtRate = 200;
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
		if (parentID == -1) {
			if (prevCost) {
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
			} else if (id > 0) {
				// Use the MainModelComponent of this costume as prevComponent,
				// so that the component can use its colormap.
				prevComponent = _components[0];
			}
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
		Debug::debug(Debug::Chores, "Loaded chore: %s\n", name);
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

	fade(Animation::None, 0);
}

void Costume::Chore::playLooping() {
	_playing = true;
	_hasPlayed = true;
	_looping = true;
	_currTime = -1;

	fade(Animation::None, 0);
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

void Costume::Chore::update(float time) {
	if (!_playing)
		return;

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

void Costume::Chore::fade(Animation::FadeMode mode, int msecs) {
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = _owner->_components[_tracks[i].compID];
		if (FROM_BE_32(comp->getTag()) == MKTAG('K','E','Y','F')) {
			KeyframeComponent *kf = static_cast<KeyframeComponent *>(comp);
			kf->fade(mode, msecs);
		}
	}
}

void Costume::Chore::fadeIn(int msecs) {
	if (!_playing) {
		_playing = true;
		_hasPlayed = true;
		_currTime = -1;
	}

	fade(Animation::FadeIn, msecs);
}

void Costume::Chore::fadeOut(int msecs) {
	// Note: It doesn't matter whether the chore is playing or not. The keyframe
	// components should fade out in either case.
	fade(Animation::FadeOut, msecs);
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
	else if (FROM_BE_32(tag) == MKTAG('A','N','I','M')) //Used  in the demo
		return new BitmapComponent(parent, parentID, name, tag);

	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);
	return NULL;
}

Costume::Component *Costume::loadComponentEMI(Costume::Component *parent, int parentID, const char *name, Costume::Component *prevComponent) {
	// some have an exclimation mark, this could mean something.
	// for now, return 0 otherwise it will just crash in some other part.
	//return 0;

	assert(name[0] == '!');
	++name;

	char type[5];
	tag32 tag = 0;
	memcpy(&tag, name, 4);
	memcpy(&type, name, 4);
	type[4] = 0;

	name += 4;

	if (FROM_BE_32(tag) == MKTAG('m','e','s','h')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement MESH-handling: %s" , name);
		//return new MainModelComponent(parent, parentID, name, prevComponent, tag);
	} else if (FROM_BE_32(tag) == MKTAG('s','k','e','l')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SKEL-handling: %s" , name);
		//return new ModelComponent(parent, parentID, name, prevComponent, tag);
	} else if (FROM_BE_32(tag) == MKTAG('t','e','x','i')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement TEXI-handling: %s" , name);
		//return new MaterialComponent(parent, parentID, name, tag);
	} else if (FROM_BE_32(tag) == MKTAG('a','n','i','m')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement ANIM-handling: %s" , name);
		//return new KeyframeComponent(parent, parentID, name, tag);
	} else if (FROM_BE_32(tag) == MKTAG('l','u','a','c')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement LUAC-handling: %s" , name);
	} else if (FROM_BE_32(tag) == MKTAG('l','u','a','v')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement LUAV-handling: %s" , name);
		//return new LuaVarComponent(parent, parentID, name, tag);
	} else if (FROM_BE_32(tag) == MKTAG('s','p','r','t')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SPRT-handling: %s" , name);
		//return new SpriteComponent(parent, parentID, name, tag);
	} else {
		error("Actor::loadComponentEMI missing tag: %s for %s", name, type);
	}
	/*
	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);*/
	return NULL;
}

ModelComponent *Costume::getMainModelComponent() const {
	for (int i = 0; i < _numComponents; i++) {
		// Needs to handle Main Models (pigeons) and normal Models
		// (when Manny climbs the rope)
		if (FROM_BE_32(_components[i]->getTag()) == MKTAG('M','M','D','L'))
			return static_cast<ModelComponent *>(_components[i]);
	}
	return NULL;
}

ModelNode *Costume::getModelNodes() {
	ModelComponent *comp = getMainModelComponent();
	if (comp) {
		return comp->getHierarchy();
	}
	return NULL;
}

Model *Costume::getModel() {
	ModelComponent *comp = getMainModelComponent();
	if (comp) {
		return comp->getModel();
	}
	return NULL;
}

void Costume::playChoreLooping(int num) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
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
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
		return;
	}
	_chores[num].play();
	if (Common::find(_playingChores.begin(), _playingChores.end(), &_chores[num]) == _playingChores.end())
		_playingChores.push_back(&_chores[num]);
}

void Costume::stopChore(int num) {
	if (num < 0 || num >= _numChores) {
		Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", num, _numChores);
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
			Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
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
			Debug::warning(Debug::Chores, "Requested chore number %d is outside the range of chores (0-%d)", chore, _numChores);
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

void Costume::getBoundingBox(int *x1, int *y1, int *x2, int *y2) {
	for (int i = 0; i < _numComponents; i++) {
		ModelComponent *c = dynamic_cast<ModelComponent *>(_components[i]);
		if (c) {
			c->getBoundingBox(x1, y1, x2, y2);
		}
	}
}

int Costume::update(float time) {
	for (Common::List<Chore*>::iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		(*i)->update(time);
		if (!(*i)->_playing) {
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
	_head.lookAt(entering, lookAt, _lookAtRate, _matrix);
}

void Costume::setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw) {
	_head.setJoints(joint1, joint2, joint3);
	_head.loadJoints(getModelNodes());
	_head.setMaxAngles(maxPitch, maxYaw, maxRoll);
}

void Costume::setLookAtRate(float rate) {
	_lookAtRate = rate;
}

float Costume::getLookAtRate() const {
	return _lookAtRate;
}

void Costume::setPosRotate(Math::Vector3d pos, const Math::Angle &pitch,
						   const Math::Angle &yaw, const Math::Angle &roll) {
	_matrix.setPosition(pos);
	_matrix.buildFromPitchYawRoll(pitch, yaw, roll);
}

Math::Matrix4 Costume::getMatrix() const {
	return _matrix;
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
	}

	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			state->writeLESint32(c->_visible);
			state->writeVector3d(c->_matrix.getPosition());
			c->saveState(state);
		}
	}

	state->writeLEUint32(_playingChores.size());
	for (Common::List<Chore*>::const_iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		state->writeLESint32((*i)->_id);
	}

	// FIXME: Decomment this!!
// 	state.writeFloat(_lookAtRate);
	_head.saveState(state);
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
	}
	for (int i = 0; i < _numComponents; ++i) {
		Component *c = _components[i];

		if (c) {
			c->_visible = state->readLESint32();
			c->_matrix.setPosition(state->readVector3d());
			c->restoreState(state);
		}
	}

	int numPlayingChores = state->readLEUint32();
	for (int i = 0; i < numPlayingChores; ++i) {
		int id = state->readLESint32();
		_playingChores.push_back(&_chores[id]);
	}

	// FIXME: Decomment this!!
// 	_lookAtRate = state->readFloat();
	_head.restoreState(state);
	_head.loadJoints(getModelNodes());

	return true;
}

Costume::Head::Head() :
	_maxPitch(0),
	_joint1(-1), _joint2(-1), _joint3(-1),
	_joint1Node(NULL), _joint2Node(NULL), _joint3Node(NULL),
	_headYaw(0), _headPitch(0) {

}

void Costume::Head::setJoints(int joint1, int joint2, int joint3) {
	_joint1 = joint1;
	_joint2 = joint2;
	_joint3 = joint3;
}

void Costume::Head::loadJoints(ModelNode *nodes) {
	if (_joint1 >= 0 && _joint2 >= 0 && _joint3 >= 0 && nodes) {
		_joint1Node = nodes + _joint1;
		_joint2Node = nodes + _joint2;
		_joint3Node = nodes + _joint3;
	}
}

void Costume::Head::setMaxAngles(float maxPitch, float maxYaw, float maxRoll) {
	_maxRoll = maxRoll;
	_maxPitch = maxPitch;
	_maxYaw = maxYaw;
}

void Costume::Head::lookAt(bool entering, const Math::Vector3d &point, float rate, const Math::Matrix4 &matrix) {
	if (_joint1Node) {
		float step = g_grim->getPerSecond(rate);
		float yawStep = step;
		float pitchStep = step / 3.f;
		if (!entering) {
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
			Math::Angle pi = _headPitch / 3.f;
			_joint1Node->_animPitch += pi;
			_joint2Node->_animPitch += pi;
			_joint3Node->_animPitch += pi;
			_joint1Node->_animRoll = (_joint1Node->_animYaw.getDegrees() / 20.f) *
			_headPitch.getDegrees() / -5.f;

			if (_joint1Node->_animRoll > _maxRoll)
				_joint1Node->_animRoll = _maxRoll;
			if (_joint1Node->_animRoll < -_maxRoll)
				_joint1Node->_animRoll = -_maxRoll;
			return;
		}

		ModelNode *p = _joint3Node;
		while (p->_parent) {
			p = p->_parent;
		}
		p->setMatrix(matrix);
		p->update();

		Math::Vector3d v = point - _joint3Node->_matrix.getPosition();
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

		Math::Angle bodyYaw = matrix.getYaw();
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

		if (_joint1Node->_animYaw > _maxYaw)
			_joint1Node->_animYaw = _maxYaw;
		if (_joint1Node->_animYaw < -_maxYaw)
			_joint1Node->_animYaw = -_maxYaw;

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
		Math::Angle pitch;
		pitch = acos(a) * (180.0f / LOCAL_PI);

		if (b < 0.0f)
			pitch = 360.0f - pitch;

		if (pitch > 180)
			pitch -= 360;

		if (pitch > _maxPitch)
			pitch = _maxPitch;
		if (pitch < -_maxPitch)
			pitch = -_maxPitch;

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

		Math::Angle pi = pitch / 3.f;
		_joint1Node->_animPitch += pi;
		_joint2Node->_animPitch += pi;
		_joint3Node->_animPitch += pi;

		//animate yaw
		if (_joint1Node->_animYaw - _headYaw > yawStep)
			_joint1Node->_animYaw = _headYaw + yawStep;
		if (_headYaw - _joint1Node->_animYaw > yawStep)
			_joint1Node->_animYaw = _headYaw - yawStep;

		_joint1Node->_animRoll = (_joint1Node->_animYaw.getDegrees() / 20.f) *
		pitch.getDegrees() / -5.f;

		if (_joint1Node->_animRoll > _maxRoll)
			_joint1Node->_animRoll = _maxRoll;
		if (_joint1Node->_animRoll < -_maxRoll)
			_joint1Node->_animRoll = -_maxRoll;

		_headPitch = pitch;
		_headYaw = _joint1Node->_animYaw;
	}
}

void Costume::Head::saveState(SaveGame *state) const {
	state->writeLESint32(_joint1);
	state->writeLESint32(_joint2);
	state->writeLESint32(_joint3);
	state->writeFloat(_maxPitch);
	state->writeFloat(_maxYaw);
	state->writeFloat(_maxRoll);
	state->writeFloat(_headPitch.getDegrees());
	state->writeFloat(_headYaw.getDegrees());
}

void Costume::Head::restoreState(SaveGame *state) {
	_joint1 = state->readLESint32();
	_joint2 = state->readLESint32();
	_joint3 = state->readLESint32();
	_maxPitch = state->readFloat();
	_maxYaw = state->readFloat();
	_maxRoll = state->readFloat();

	_headPitch = state->readFloat();
	_headYaw = state->readFloat();
}

} // end of namespace Grim
