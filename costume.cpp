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
// For bitmaps, the actual drawing is handled by the Scene class.  The
// bitmaps to be drawn are associated to the needed camera setups
// using NewObjectState; bitmaps marked OBJSTATE_UNDERLAY and
// OBJSTATE_STATE are drawn first, then the 3D objects, then bitmaps
// marked OBJSTATE_OVERLAY.  So the BitmapComponent just needs to pass
// along setKey requests to the actual bitmap object.

class BitmapComponent : public Costume::Component {
public:
  BitmapComponent(Costume::Component *parent, int parentID,
		 const char *filename);

  void update();
  void draw();

private:
  std::string filename_;
  std::string zbuf_filename_;
  ResPtr<Bitmap> bitmap_;
  ResPtr<Bitmap> zbuffer_;
};

class ModelComponent : public Costume::Component {
public:
  ModelComponent(Costume::Component *parent, int parentID,
		 const char *filename);
  void init();
  void setKey(int val);
  void update();
  void reset();
  void setColormap(Colormap *c);
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
  MainModelComponent(const char *filename, Model *prevObj,
		     Model::HierNode *prevHier);
  void init();
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
  void init();
  void setKey(int val);
  void reset();
  ~MeshComponent() { }

  Model::HierNode *node() { return node_; }

private:
  int num_;
  Model::HierNode *node_;
};

BitmapComponent::BitmapComponent(Costume::Component *parent, int parentID,
			       const char *filename) :
  Costume::Component(parent, parentID), filename_(filename) {

  bitmap_ = ResourceLoader::instance()->loadBitmap(filename);
  warning("Instanced BitmapComponenet from Costume renderer: NOT IMPLEMENTED YET");
}

void BitmapComponent::draw() {
 ;
}

void BitmapComponent::update() {
 ;
}

ModelComponent::ModelComponent(Costume::Component *parent, int parentID,
			       const char *filename) :
  Costume::Component(parent, parentID), filename_(filename), obj_(NULL),
  cmap_(NULL), hier_(NULL) {
}

void ModelComponent::init() {
  if (obj_ == NULL) {		// Skip loading if it was initialized
				// by the sharing MainModelComponent
				// constructor before
    if (cmap_ == NULL) {
      warning("No colormap specified for %s\n", filename_.c_str());
      cmap_ = ResourceLoader::instance()->loadColormap("item.cmp");
    }
    obj_ = ResourceLoader::instance()->loadModel(filename_.c_str(), *cmap_);
    hier_ = obj_->copyHierarchy();
    hier_->hierVisible_ = false;
  }

  // If we're the child of a mesh component, put our nodes in the
  // parent object's tree.
  if (parent_ != NULL) {
    MeshComponent *mc = dynamic_cast<MeshComponent *>(parent_);
    if (mc != NULL)
      mc->node()->addChild(hier_);
    else
      warning("Parent of model %s wasn't a mesh\n", filename_.c_str());
  }
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
}

ModelComponent::~ModelComponent() {
  if (hier_ != NULL && hier_->parent_ != NULL)
    hier_->parent_->removeChild(hier_);
  delete[] hier_;
}

void ModelComponent::draw() {
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

// Constructor used if sharing the main model with the previous costume
MainModelComponent::MainModelComponent(const char *filename, Model *prevObj,
				       Model::HierNode *prevHier) :
  ModelComponent(NULL, -1, filename), hierShared_(true)
{
  obj_ = prevObj;
  hier_ = prevHier;
}

void MainModelComponent::init() {
  ModelComponent::init();
  hier_->hierVisible_ = true;
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

ColormapComponent::~ColormapComponent() {
}

class KeyframeComponent : public Costume::Component {
public:
  KeyframeComponent(Costume::Component *parent, int parentID,
		    const char *filename);
  void init();
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

void KeyframeComponent::init() {
  ModelComponent *mc = dynamic_cast<ModelComponent *>(parent_);
  if (mc != NULL)
    hier_ = mc->hierarchy();
  else {
    warning("Parent of %s was not a model\n", keyf_->filename());
    hier_ = NULL;
  }
}

MeshComponent::MeshComponent(Costume::Component *parent, int parentID,
			     const char *name) :
  Costume::Component(parent, parentID), node_(NULL) {
  if (std::sscanf(name, "mesh %d", &num_) < 1)
    error("Couldn't parse mesh name %s\n", name);
}

void MeshComponent::init() {
  ModelComponent *mc = dynamic_cast<ModelComponent *>(parent_);
  if (mc != NULL)
    node_ = mc->hierarchy() + num_;
  else {
    warning("Parent of mesh %d was not a model\n", num_);
    node_ = NULL;
  }
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
  void setKey(int val);
  ~LuaVarComponent() { }

private:
  std::string name_;
};

LuaVarComponent::LuaVarComponent(Costume::Component *parent, int parentID,
				 const char *name) :
  Costume::Component(parent, parentID), name_(name) {
}

void LuaVarComponent::setKey(int val) {
  lua_pushnumber(val);
  lua_setglobal(const_cast<char *>(name_.c_str()));
}

class SoundComponent : public Costume::Component {
public:
  SoundComponent(Costume::Component *parent, int parentID,
		 const char *name);
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

Costume::Costume(const char *filename, const char *data, int len,
		 Costume *prevCost) :
  fname_(filename)
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

    // Check for sharing a main model with the previous costume
    if (id == 0 && prevCost != NULL &&
	std::memcmp(tags[tagID], "mmdl", 4) == 0) {
      MainModelComponent *mmc =
	dynamic_cast<MainModelComponent *>(prevCost->components_[0]);
      if (mmc != NULL && mmc->filename_ == std::string(line + namePos)) {
	components_[id] = new MainModelComponent(line + namePos,
						 mmc->obj_,
						 mmc->hier_);
	continue;
      }
    }

    components_[id] =
      loadComponent(tags[tagID],
		    parentID == -1 ? NULL : components_[parentID], parentID,
		    line + namePos);
  }

  delete[] tags;

  for (int i = 0; i < numComponents_; i++)
    if (components_[i] != NULL)
      components_[i]->init();

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
    printf("Loaded chore: %s\n", name);
  }

  ts.expectString("section keys");
  for (int i = 0; i < numChores_; i++) {
    int which;
    ts.scanString("chore %d", 1, &which);
    chores_[which].load(this, ts);
  }

  for (int i=0; i < MAX_TALK_CHORES; i++)
    talkChores_[i] = -1;
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
  else if (std::memcmp(tag, "bknd", 4) == 0)
    return new BitmapComponent(parent, parentID, name);
  
  warning("Unknown tag '%.4s', name '%s'\n", tag, name);
  return NULL;
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

void Costume::playTalkChores() {
 for (int i=0; i<MAX_TALK_CHORES; i++) {
  if (talkChores_[i] > -1) {
   printf("Running talk chore %d (%d)!\n", i, talkChores_[i], chores_[talkChores_[i]].name_);
   chores_[talkChores_[i]].playLooping(); }
   printf("Woo\n");
  }
}

void Costume::stopTalkChores() {
 for (int i=0; i<MAX_TALK_CHORES; i++) {
  if ((talkChores_[i] > -1) && (chores_[i].playing_)) {
   printf("Stopping talk chore %d!\n", i);
   chores_[talkChores_[i]-1].stop();
  }
 }
}

void Costume::setTalkChore(int index, int chore) {
 if (index > MAX_TALK_CHORES)
  return;

 printf("Setting chore %d(+1) to %d - %s\n", index, chore, chores_[chore].name_);
 talkChores_[index-1] = chore;
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

