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

#ifndef COSTUME_HH
#define COSTUME_HH

#include "resource.hh"

class TextSplitter;
class Actor;

class Costume : public Resource {
public:
  Costume(const char *filename, const char *data, int len);

  // Create a copy of orig, sharing the model data from prev if
  // appropriate
  Costume(Costume &orig, Costume *prev);

  ~Costume();

  void playChore(int num) { chores_[num].play(); }
  void playChoreLooping(int num) { chores_[num].playLooping(); }
  void setChoreLooping(int num, bool val) { chores_[num].setLooping(val); }
  void stopChore(int num) { chores_[num].stop(); }
  void stopChores();
  int isChoring(int num, bool excludeLooping);
  int isChoring(bool excludeLooping);

  void update();
  void draw();

  class Component {
  public:
    Component(Component *parent, int parentID);
    virtual Component *copy(Component *newParent) = 0;
    virtual void setKey(int /* val */) { }
    virtual void update() { }
    virtual void draw() { }
    virtual void reset() { }
    virtual ~Component() { }

    void setParent(Component *newParent);

  protected:
    int parentID_;
    Component *parent_, *child_, *sibling_;

    friend class Costume;
  };

private:
  // Reference the original copy of the costume in the cache
  ResPtr<Costume> orig_;

  Component *loadComponent(char tag[4], Component *parent, int parentID,
			   const char *name);

  int numComponents_;
  Component **components_;

  struct TrackKey {
    int time_, value_;
  };

  struct ChoreTrack {
    int compID_;
    int numKeys_;
    TrackKey *keys_;
  };

  class Chore {
  public:
    void load(Costume *owner, TextSplitter &ts);
    void play();
    void playLooping();
    void setLooping(bool val) { looping_ = val; }
    void stop();
    void update();

  private:
    Costume *owner_;

    int length_;
    int numTracks_;
    ChoreTrack *tracks_;
    char name_[32];

    bool hasPlayed_, playing_, looping_;
    int currTime_;

    void setKeys(int startTime, int stopTime);

    friend class Costume;
  };

  int numChores_;
  Chore *chores_;
};

#endif
