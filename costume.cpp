// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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

#include "costume.h"
#include "textsplit.h"
#include "debug.h"
#include "engine.h"
#include "colormap.h"
#include "keyframe.h"
#include "model.h"
#include "lua.h"
#include "sound.h"
#include "mixer.h"
#include <string>
#include <cstdio>
#include <map>

class ModelComponent : public Costume::Component {
public:
  ModelComponent(Costume::Component *parent, int parentID,
		 const char *filename);
  ModelComponent *copy(Costume::Component *newParent);
  void setKey(int val);
  void update();
  void reset();
  void setColormap(Colormap *c);
  void loadModel();
  ~ModelComponent();

  Model::HierNode *hierarchy() { return hier_; }
  void draw();

protected:
  std::string filename_;
  ResPtr<Model> obj_;
  ResPtr<Colormap> cmap_;
  Model::HierNode *hier_;
};

class MainModelComponent : public ModelComponent {
public:
  MainModelComponent(Costume::Component *parent, int parentID,
		     const char *filename);
  MainModelComponent *copy(Costume::Component *newParent);
  MainModelComponent *copy(Model *prevObj, Model::HierNode *prevHier);
  void update();
  void reset();
  ~MainModelComponent();

private:
  bool hierShared_;

  friend class Costume;
};

class MeshComponent : public Costume::Component {
public:
  MeshComponent(Costume::Component *parent, int parentID,
		const char *name);
  MeshComponent *copy(Costume::Component *newParent);
  void setKey(int val);
  void reset();
  ~MeshComponent() { }

  Model::HierNode *node() { return node_; }

private:
  int num_;
  Model::HierNode *node_;
};

ModelComponent::ModelComponent(Costume::Component *parent, int parentID,
			       const char *filename) :
  Costume::Component(parent, parentID), filename_(filename), obj_(NULL),
  cmap_(NULL), hier_(NULL) {
}

ModelComponent *ModelComponent::copy(Costume::Component *newParent) {
  loadModel();
  ModelComponent *result = new ModelComponent(*this);
  result->setParent(newParent);
  result->hier_ = obj_->copyHierarchy();
  result->hier_->hierVisible_ = false;
  if (newParent != NULL) {
    MeshComponent *mc = dynamic_cast<MeshComponent *>(newParent);
    if (mc != NULL)
      mc->node()->addChild(result->hier_);
    else
      warning("Parent of model %s wasn't a mesh\n", filename_.c_str());
  }
  return result;
}

void ModelComponent::setKey(int val) {
  hier_->hierVisible_ = (val != 0);
}

// Reset the hierarchy nodes for any keyframe animations (which
// are children of this component and therefore get updated later).
void ModelComponent::update() {
  for (int i = 0; i < obj_->numNodes(); i++) {
    hier_[i].priority_ = -1;
    hier_[i].animPos_ = hier_[i].pos_;
    hier_[i].animPitch_ = hier_[i].pitch_;
    hier_[i].animYaw_ = hier_[i].yaw_;
    hier_[i].animRoll_ = hier_[i].roll_;
    hier_[i].totalWeight_ = 1;
  }
}

void ModelComponent::reset() {
  hier_->hierVisible_ = false;
}

void ModelComponent::setColormap(Colormap *c) {
  cmap_ = c;
  obj_ = ResourceLoader::instance()->loadModel(filename_.c_str(), *c);
  hier_ = obj_->copyHierarchy();
}

void ModelComponent::loadModel() {
  if (obj_ == NULL) {
    warning("No colormap specified for %s\n", filename_.c_str());
    setColormap(ResourceLoader::instance()->loadColormap("item.cmp"));
  }
}

ModelComponent::~ModelComponent() {
  if (hier_ != NULL && hier_->parent_ != NULL)
    hier_->parent_->removeChild(hier_);
  delete[] hier_;
}

void ModelComponent::draw() {
  loadModel();
  if (parent_ == NULL)		// Otherwise it was already drawn by
				// being included in the parent's hierarchy
    hier_->draw();
}

MainModelComponent::MainModelComponent(Costume::Component *parent,
				       int parentID,
				       const char *filename) :
  ModelComponent(parent, parentID, filename), hierShared_(false)
{
}

MainModelComponent *MainModelComponent::copy(Costume::Component *newParent) {
  loadModel();
  MainModelComponent *result = new MainModelComponent(*this);
  result->setParent(newParent);
  result->hier_ = obj_->copyHierarchy();
  result->hierShared_ = false;
  return result;
}

MainModelComponent *MainModelComponent::copy(Model *prevObj,
					     Model::HierNode *prevHier) {
  MainModelComponent *result = new MainModelComponent(*this);
  result->setParent(NULL);
  result->obj_ = prevObj;
  result->hier_ = prevHier;
  result->hierShared_ = true;
  return result;
}

void MainModelComponent::update() {
  if (! hierShared_)		// Otherwise, it was already initialized
				// and reinitializing it will destroy work
				// from previous costumes
    ModelComponent::update();
}

void MainModelComponent::reset() {
  hier_->hierVisible_ = true;
}

MainModelComponent::~MainModelComponent() {
  if (hierShared_)
    hier_ = NULL;		// Keep ~ModelComp from deleting it
}

class ColormapComponent : public Costume::Component {
public:
  ColormapComponent(Costume::Component *parent, int parentID,
		    const char *filename);
  ColormapComponent *copy(Costume::Component *newParent);
  ~ColormapComponent();

private:
  ResPtr<Colormap> cmap_;
};

ColormapComponent::ColormapComponent(Costume::Component *parent,
				     int parentID,
				     const char *filename) :
  Costume::Component(parent, parentID)
{
  cmap_ = ResourceLoader::instance()->loadColormap(filename);
  ModelComponent *mc = dynamic_cast<ModelComponent *>(parent);
  if (mc != NULL)
    mc->setColormap(cmap_);
}

ColormapComponent *ColormapComponent::copy(Costume::Component *newParent) {
  ColormapComponent *result = new ColormapComponent(*this);
  result->setParent(newParent);
  return result;
}

ColormapComponent::~ColormapComponent() {
}

class KeyframeComponent : public Costume::Component {
public:
  KeyframeComponent(Costume::Component *parent, int parentID,
		    const char *filename);
  KeyframeComponent *copy(Costume::Component *newParent);
  void setKey(int val);
  void update();
  void reset();
  ~KeyframeComponent() { }

private:
  ResPtr<KeyframeAnim> keyf_;
  int priority1_, priority2_;
  Model::HierNode *hier_;
  bool active_;
  int repeatMode_;
  int currTime_;
};

KeyframeComponent::KeyframeComponent(Costume::Component *parent,
				     int parentID,
				     const char *filename) :
  Costume::Component(parent, parentID), priority1_(1), priority2_(5),
  hier_(NULL), active_(false)
{
  const char *comma = std::strchr(filename, ',');
  if (comma != NULL) {
    std::string realName(filename, comma);
    keyf_ = ResourceLoader::instance()->loadKeyframe(realName.c_str());
    std::sscanf(comma + 1, "%d,%d", &priority1_, &priority2_);
  }
  else
    keyf_ = ResourceLoader::instance()->loadKeyframe(filename);
}

void KeyframeComponent::setKey(int val) {
  switch (val) {
  case 0:
  case 1:
  case 2:
  case 3:
    if (! active_ || val != 1) {
      active_ = true;
      currTime_ = -1;
    }
    repeatMode_ = val;
    break;
  case 4:
    active_ = false;
    break;
  default:
    warning("Unknown key %d for keyframe %s\n", val, keyf_->filename());
  }
}

void KeyframeComponent::reset() {
  active_ = false;
}

void KeyframeComponent::update() {
  if (! active_)
    return;
  if (currTime_ < 0)		// For first time through
    currTime_ = 0;
  else
    currTime_ += Engine::instance()->frameTime();
  int animLength = int(keyf_->length() * 1000);
  if (currTime_ > animLength) { // What to do at end?
    switch (repeatMode_) {
    case 0:			// Stop
    case 3:			// Fade at end
      active_ = false;
      return;
    case 1:			// Loop
      do
	currTime_ -= animLength;
      while (currTime_ > animLength);
      break;
    case 2:			// Hold at end
      currTime_ = animLength;
      break;
    }
  }
  keyf_->animate(hier_, currTime_ / 1000.0, priority1_, priority2_);
}

KeyframeComponent *KeyframeComponent::copy(Costume::Component *newParent) {
  KeyframeComponent *result = new KeyframeComponent(*this);
  result->setParent(newParent);
  ModelComponent *mc = dynamic_cast<ModelComponent *>(newParent);
  if (mc != NULL)
    result->hier_ = mc->hierarchy();
  else {
    warning("Parent of %s was not a model\n", keyf_->filename());
    result->hier_ = NULL;
  }
  return result;
}

MeshComponent::MeshComponent(Costume::Component *parent, int parentID,
			     const char *name) :
  Costume::Component(parent, parentID), node_(NULL) {
  if (std::sscanf(name, "mesh %d", &num_) < 1)
    error("Couldn't parse mesh name %s\n", name);
}

MeshComponent *MeshComponent::copy(Costume::Component *newParent) {
  MeshComponent *result = new MeshComponent(*this);
  result->setParent(newParent);
  ModelComponent *mc = dynamic_cast<ModelComponent *>(newParent);
  if (mc != NULL)
    result->node_ = mc->hierarchy() + num_;
  else {
    warning("Parent of mesh %d was not a model\n", num_);
    result->node_ = NULL;
  }
  return result;
}

void MeshComponent::setKey(int val) {
  node_->meshVisible_ = (val != 0);
}

void MeshComponent::reset() {
  node_->meshVisible_ = true;
}

class LuaVarComponent : public Costume::Component {
public:
  LuaVarComponent(Costume::Component *parent, int parentID,
		  const char *name);
  LuaVarComponent *copy(Costume::Component *newParent);
  void setKey(int val);
  ~LuaVarComponent() { }

private:
  std::string name_;
};

LuaVarComponent::LuaVarComponent(Costume::Component *parent, int parentID,
				 const char *name) :
  Costume::Component(parent, parentID), name_(name) {
}

LuaVarComponent *LuaVarComponent::copy(Costume::Component *newParent) {
  LuaVarComponent *result = new LuaVarComponent(*this);
  result->setParent(newParent);
  return result;
}

void LuaVarComponent::setKey(int val) {
  lua_pushnumber(val);
  lua_setglobal(const_cast<char *>(name_.c_str()));
}

class SoundComponent : public Costume::Component {
public:
  SoundComponent(Costume::Component *parent, int parentID,
		 const char *name);
  SoundComponent *copy(Costume::Component *newParent);
  void setKey(int val);
  void reset();
  ~SoundComponent() { }

private:
  ResPtr<Sound> sound_;
};

SoundComponent::SoundComponent(Costume::Component *parent, int parentID,
			       const char *filename) :
  Costume::Component(parent, parentID) {
  const char *comma = std::strchr(filename, ',');
  if (comma != NULL) {
    std::string realName(filename, comma);
    sound_ = ResourceLoader::instance()->loadSound(realName.c_str());
  }
  else
    sound_ = ResourceLoader::instance()->loadSound(filename);
}

SoundComponent *SoundComponent::copy(Costume::Component *newParent) {
  SoundComponent *result = new SoundComponent(*this);
  result->setParent(newParent);
  return result;
}

void SoundComponent::setKey(int val) {
  switch (val) {
  case 0:
    Mixer::instance()->playSfx(sound_);
    break;
  case 2:
    Mixer::instance()->stopSfx(sound_);
    break;
  default:
    warning("Unknown key %d for sound %s\n", val, sound_->filename());
  }
}

void SoundComponent::reset() {
  Mixer::instance()->stopSfx(sound_);
}

Costume::Costume(const char *filename, const char *data, int len) :
  Resource(filename), orig_(NULL)
{
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
  ts.scanString(" numcomponents %d", 1, &numComponents_);
  components_ = new Component*[numComponents_];
  for (int i = 0; i < numComponents_; i++) {
    int id, tagID, hash, parentID;
    int namePos;
    const char *line = ts.currentLine();
    if (std::sscanf(line, " %d %d %d %d %n", &id, &tagID, &hash, &parentID,
		    &namePos) < 4)
      error("Bad component specification line: `%s'\n", line);
    ts.nextLine();
    components_[id] =
      loadComponent(tags[tagID],
		    parentID == -1 ? NULL : components_[parentID], parentID,
		    line + namePos);
  }

  ts.expectString("section chores");
  ts.scanString(" numchores %d", 1, &numChores_);
  chores_ = new Chore[numChores_];
  for (int i = 0; i < numChores_; i++) {
    int id, length, tracks;
    char name[32];
    ts.scanString(" %d %d %d %32s", 4, &id, &length, &tracks, name);
    chores_[id].length_ = length;
    chores_[id].numTracks_ = tracks;
    std::memcpy(chores_[id].name_, name, 32);
  }

  ts.expectString("section keys");
  for (int i = 0; i < numChores_; i++) {
    int which;
    ts.scanString("chore %d", 1, &which);
    chores_[which].load(this, ts);
  }

delete[] tags;
}

Costume::~Costume() {
  stopChores();
  for (int i = numComponents_ - 1; i >= 0; i--)
    delete components_[i];
  delete[] chores_;
}

Costume::Component::Component(Component *parent, int parentID) {
  parentID_ = parentID;
  setParent(parent);
}

void Costume::Component::setParent(Component *newParent) {
  parent_ = newParent;
  child_ = NULL;
  sibling_ = NULL;
  if (parent_ != NULL) {
    Component **lastChildPos = &parent_->child_;
    while (*lastChildPos != NULL)
      lastChildPos = &((*lastChildPos)->sibling_);
    *lastChildPos = this;
  }
}

void Costume::Chore::load(Costume *owner, TextSplitter &ts) {
  owner_ = owner;
  tracks_ = new ChoreTrack[numTracks_];
  hasPlayed_ = playing_ = false;
  for (int i = 0; i < numTracks_; i++) {
    int compID, numKeys;
    ts.scanString(" %d %d", 2, &compID, &numKeys);
    tracks_[i].compID_ = compID;
    tracks_[i].numKeys_ = numKeys;
    tracks_[i].keys_ = new TrackKey[numKeys];
    for (int j = 0; j < numKeys; j++)
      ts.scanString(" %d %d", 2, &tracks_[i].keys_[j].time_,
		    &tracks_[i].keys_[j].value_);
  }
}

void Costume::Chore::play() {
  playing_ = true;
  hasPlayed_ = true;
  looping_ = false;
  currTime_ = -1;
}

void Costume::Chore::playLooping() {
  playing_ = true;
  hasPlayed_ = true;
  looping_ = true;
  currTime_ = -1;
}

void Costume::Chore::stop() {
  if (! hasPlayed_)
    return;
  playing_ = false;
  hasPlayed_ = false;
  for (int i = 0; i < numTracks_; i++) {
    Component *comp = owner_->components_[tracks_[i].compID_];
    if (comp != NULL)
      comp->reset();
  }
}

void Costume::Chore::setKeys(int startTime, int stopTime) {
  for (int i = 0; i < numTracks_; i++) {
    Component *comp = owner_->components_[tracks_[i].compID_];
    if (comp == NULL)
      continue;
    for (int j = 0; j < tracks_[i].numKeys_; j++) {
      if (tracks_[i].keys_[j].time_ > stopTime)
	break;
      if (tracks_[i].keys_[j].time_ > startTime)
	comp->setKey(tracks_[i].keys_[j].value_);
    }
  }
}

void Costume::Chore::update() {
  if (! playing_)
    return;
  int newTime;
  if (currTime_ < 0)
    newTime = 0;		// For first time through
  else
    newTime = currTime_ + Engine::instance()->frameTime();
  setKeys(currTime_, newTime);
  if (newTime > length_) {
    if (! looping_)
      playing_ = false;
    else {
      do {
	newTime -= length_;
	setKeys(-1, newTime);
      } while (newTime > length_);
    }
  }
  currTime_ = newTime;
}

Costume::Component *Costume::loadComponent
(char tag[4], Costume::Component *parent, int parentID, const char *name) {
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
  warning("Unknown tag '%.4s', name '%s'\n", tag, name);
  return NULL;
}

Costume::Costume(Costume &orig, Costume *prev) : Resource(orig), orig_(&orig) {
  numComponents_ = orig.numComponents_;
  components_ = new Component*[numComponents_];
  int start = 0;

  MainModelComponent *mmc =
    dynamic_cast<MainModelComponent *>(orig.components_[0]);
  MainModelComponent *prev_mmc =
    (prev == NULL ? NULL :
     dynamic_cast<MainModelComponent *>(prev->components_[0]));
  if (mmc != NULL && prev_mmc != NULL &&
      mmc->filename_ == prev_mmc->filename_) {
    prev_mmc->loadModel();
    components_[0] = mmc->copy(prev_mmc->obj_, prev_mmc->hier_);
    start = 1;
  }

  for (int i = start; i < numComponents_; i++) {
    if (orig.components_[i] != NULL) {
      Component *newParent;
      if (orig.components_[i]->parentID_ >= 0)
	newParent = components_[orig.components_[i]->parentID_];
      else
	newParent = NULL;
      components_[i] = orig.components_[i]->copy(newParent);
    }
    else
      components_[i] = NULL;
  }
  numChores_ = orig.numChores_;
  chores_ = new Chore[numChores_];
  std::memcpy(chores_, orig.chores_, numChores_ * sizeof(Chore));
  for (int i = 0; i < numChores_; i++)
    chores_[i].owner_ = this;
}

void Costume::stopChores() {
  for (int i = 0; i < numChores_; i++)
    chores_[i].stop();
}

int Costume::isChoring(int num, bool excludeLooping) {
  if (chores_[num].playing_ && !(excludeLooping && chores_[num].looping_))
    return num;
  else
    return -1;
}

int Costume::isChoring(bool excludeLooping) {
  for (int i = 0; i < numChores_; i++) {
    if (chores_[i].playing_ && !(excludeLooping && chores_[i].looping_))
      return i;
  }
  return -1;
}

void Costume::draw() {
  for (int i = 0; i < numComponents_; i++)
    if (components_[i] != NULL)
      components_[i]->draw();
}

void Costume::update() {
  for (int i = 0; i < numChores_; i++)
    chores_[i].update();
  for (int i = 0; i < numComponents_; i++)
    if (components_[i] != NULL)
      components_[i]->update();
}
