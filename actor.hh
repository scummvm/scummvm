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

#ifndef ACTOR_HH
#define ACTOR_HH

#include "color.hh"
#include "vector3d.hh"
#include "resource.hh"
#include <string>
#include <list>

class Sound;
class Costume;

class Actor {
public:
  Actor(const char *name);

  const char *name() const { return name_.c_str(); }

  void setTalkColor(const Color& c) { talkColor_ = c; }
  Color talkColor() const { return talkColor_; }
  void setPos(Vector3d pos) { pos_ = pos; }
  Vector3d pos() const { return pos_; }
  void setRot(float pitch, float yaw, float roll) {
    pitch_ = pitch; yaw_ = yaw; roll_ = roll;
  }
  float pitch() const { return pitch_; }
  float yaw() const { return yaw_; }
  float roll() const { return roll_; }
  void setVisibility(bool val) { visible_ = val; }
  bool visible() const { return visible_; }
  void putInSet(const char *name) { setName_ = name; }
  void setTurnRate(float rate) { turnRate_ = rate; }
  float turnRate() const { return turnRate_; }
  void setWalkRate(float rate) { walkRate_ = rate; }
  float walkRate() const { return walkRate_; }

  float angleTo(const Actor &a) const;

  bool inSet(const char *name) const {
    return setName_ == name;
  }
  void walkForward();
  void turn(int dir);

  void sayLine(const char *msg);
  bool talking();

  void pushCostume(Costume *c);
  void setCostume(Costume *c);
  void popCostume();
  void clearCostumes();
  Costume *currentCostume() {
    if (costumeStack_.empty())
      return NULL;
    else
      return costumeStack_.back();
  }
  Costume *findCostume(const char *name);
  int costumeStackDepth() const {
    return costumeStack_.size();
  }

  void update();
  void draw();

private:
  std::string name_;
  std::string setName_;
  Color talkColor_;
  Vector3d pos_;
  float pitch_, yaw_, roll_;
  float walkRate_, turnRate_;
  bool visible_;
  ResPtr<Sound> talkSound_;
  std::list<ResPtr<Costume> > costumeStack_;

  friend class Engine;
};

#endif
