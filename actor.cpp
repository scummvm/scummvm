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

#include "actor.h"
#include "engine.h"
#include "costume.h"
#include "sound.h"
#include "mixer.h"
#include <cmath>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>

Actor::Actor(const char *name) :
  name_(name), talkColor_(255, 255, 255), pos_(0, 0, 0),
  pitch_(0), yaw_(0), roll_(0), walkRate_(0), turnRate_(0),
  visible_(true), talkSound_(NULL)
{
  Engine::instance()->registerActor(this);
}

void Actor::walkForward() {
  float dist = Engine::instance()->perSecond(walkRate_);
  float yaw_deg = yaw_ * (M_PI / 180), pitch_deg = pitch_ * (M_PI / 180);
  Vector3d forwardVec(-std::sin(yaw_deg) * std::cos(pitch_deg),
		      std::cos(yaw_deg) * std::cos(pitch_deg),
		      std::sin(pitch_deg));
  pos_ += forwardVec * dist;
}

void Actor::turn(int dir) {
  float delta = Engine::instance()->perSecond(turnRate_) * dir;
  yaw_ += delta;
}

float Actor::angleTo(const Actor &a) const {
  float yaw_deg = yaw_ * (M_PI / 180);
  Vector3d forwardVec(-std::sin(yaw_deg), std::cos(yaw_deg), 0);
  Vector3d delta = a.pos() - pos_;
  delta.z() = 0;
  return angle(forwardVec, delta) * (180 / M_PI);
}

void Actor::sayLine(const char *msg) {
  // For now, just play the appropriate sound if found.  Eventually,
  // this needs to handle possibly displaying text, starting up
  // appropriate talking chores, etc.

  // Find the message identifier
  if (msg[0] != '/')
    return;
  const char *secondSlash = std::strchr(msg + 1, '/');
  if (secondSlash == NULL)
    return;
  std::string msgId(msg + 1, secondSlash);
  talkSound_ = ResourceLoader::instance()->loadSound((msgId + ".wav").c_str());
  if (talkSound_ != NULL)
    Mixer::instance()->playVoice(talkSound_);

// FIXME: Ender - Disabled until I work out why the wrong Chores play
//  if (!costumeStack_.empty()) {
//    printf("Requesting talk chore\n");
//    costumeStack_.back()->playTalkChores();
//  }
}

bool Actor::talking() {
  if (talkSound_ == NULL)
    return false;
  if (talkSound_->done()) {
// FIXME: Ender - Disabled until I work out why the wrong Chores play
//    if (!costumeStack_.empty())
//      costumeStack_.back()->stopTalkChores();
    talkSound_ = NULL;
    return false;
  }
  return true;
}

void Actor::pushCostume(Costume *c) {
  Costume *copy = new Costume(*c, currentCostume());
  costumeStack_.push_back(copy);
}

void Actor::setCostume(Costume *c) {
  if (! costumeStack_.empty())
    popCostume();
  pushCostume(c);
}

void Actor::popCostume() {
  costumeStack_.pop_back();
}

void Actor::clearCostumes() {
  // Make sure to destroy costume copies in reverse order
  while (! costumeStack_.empty())
    costumeStack_.pop_back();
}

Costume *Actor::findCostume(const char *name) {
  for (std::list<ResPtr<Costume> >::iterator i = costumeStack_.begin();
       i != costumeStack_.end(); i++)
    if (std::strcmp((*i)->filename(), name) == 0)
      return *i;
  return NULL;
}

void Actor::update() {
  for (std::list<ResPtr<Costume> >::iterator i = costumeStack_.begin();
       i != costumeStack_.end(); i++)
    (*i)->update();
}

void Actor::draw() {
  if (! costumeStack_.empty()) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(pos_.x(), pos_.y(), pos_.z());
    glRotatef(yaw_, 0, 0, 1);
    glRotatef(pitch_, 1, 0, 0);
    glRotatef(roll_, 0, 1, 0);
    costumeStack_.back()->draw();
    glPopMatrix();
  }
}
