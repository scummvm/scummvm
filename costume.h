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

#ifndef COSTUME_H
#define COSTUME_H

#include <string>

#define MAX_TALK_CHORES 10
class TextSplitter;
class Actor;

class Costume {
public:
  Costume(const char *filename, const char *data, int len,
	  Costume *prevCost);

  ~Costume();

  const char *filename() const { return fname_.c_str(); }

  void playChore(int num) { chores_[num].play(); }
  void playChoreLooping(int num) { chores_[num].playLooping(); }
  void setChoreLooping(int num, bool val) { chores_[num].setLooping(val); }
  void stopChore(int num) { chores_[num].stop(); }
  void stopChores();
  int isChoring(int num, bool excludeLooping);
  int isChoring(bool excludeLooping);

  void playTalkChores();
  void stopTalkChores();
  void setTalkChore(int index, int chore);

  void update();
  void draw();

  class Component {
  public:
    Component(Component *parent, int parentID);
    virtual void init() { }
    virtual void setKey(int /* val */) { }
    virtual void update() { }
    virtual void draw() { }
    virtual void reset() { }
    virtual ~Component() { }

  protected:
    int parentID_;
    Component *parent_, *child_, *sibling_;
    void setParent(Component *newParent);

    friend class Costume;
  };

private:
  Component *loadComponent(char tag[4], Component *parent, int parentID,
			   const char *name);

  std::string fname_;

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
  int talkChores_[MAX_TALK_CHORES];
};

#endif
