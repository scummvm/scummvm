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

#ifndef ENGINE_H
#define ENGINE_H

#include "scene.h"
#include "textobject.h"
#include <cstdlib>
#include <list>
#include <SDL_keysym.h>

class Actor;

// Fake SDLK_* values for joystick and mouse events
enum {
  SDLK_JOY1_B1 = SDLK_LAST,
  SDLK_JOY1_B2,
  SDLK_JOY1_B3,
  SDLK_JOY1_B4,
  SDLK_JOY1_B5,
  SDLK_JOY1_B6,
  SDLK_JOY1_B7,
  SDLK_JOY1_B8,
  SDLK_JOY1_B9,
  SDLK_JOY1_B10,
  SDLK_JOY1_HLEFT,
  SDLK_JOY1_HUP,
  SDLK_JOY1_HRIGHT,
  SDLK_JOY1_HDOWN,
  SDLK_JOY2_B1,
  SDLK_JOY2_B2,
  SDLK_JOY2_B3,
  SDLK_JOY2_B4,
  SDLK_JOY2_B5,
  SDLK_JOY2_B6,
  SDLK_JOY2_B7,
  SDLK_JOY2_B8,
  SDLK_JOY2_B9,
  SDLK_JOY2_B10,
  SDLK_JOY2_HLEFT,
  SDLK_JOY2_HUP,
  SDLK_JOY2_HRIGHT,
  SDLK_JOY2_HDOWN,
  SDLK_MOUSE_B1,
  SDLK_MOUSE_B2,
  SDLK_MOUSE_B3,
  SDLK_MOUSE_B4,
  SDLK_AXIS_JOY1_X,
  SDLK_AXIS_JOY1_Y,
  SDLK_AXIS_JOY1_Z,
  SDLK_AXIS_JOY1_R,
  SDLK_AXIS_JOY1_U,
  SDLK_AXIS_JOY1_V,
  SDLK_AXIS_JOY2_X,
  SDLK_AXIS_JOY2_Y,
  SDLK_AXIS_JOY2_Z,
  SDLK_AXIS_JOY2_R,
  SDLK_AXIS_JOY2_U,
  SDLK_AXIS_JOY2_V,
  SDLK_AXIS_MOUSE_X,
  SDLK_AXIS_MOUSE_Y,
  SDLK_AXIS_MOUSE_Z,
  SDLK_EXTRA_LAST
};

class Engine {
public:
  GLuint font;	// FIXME: Temporary font drawing hack

  static Engine *instance() {
    if (instance_ == NULL)
      instance_ = new Engine;
    return instance_;
  }

  void mainLoop();
  unsigned frameStart() const { return frameStart_; }
  unsigned frameTime() const { return frameTime_; }

  float perSecond(float rate) const { return rate * frameTime_ / 1000; }

  void enableControl(int num) { controlsEnabled_[num] = true; }
  void disableControl(int num) { controlsEnabled_[num] = false; }

  void registerActor(Actor *a) { actors_.push_back(a); }

  void setScene(const char *name);
  Scene *currScene() { return currScene_; }
  const char *sceneName() const { return currScene_->name(); }

  typedef std::list<Actor *> actor_list_type;
  actor_list_type::const_iterator actorsBegin() const {
    return actors_.begin();
  }
  actor_list_type::const_iterator actorsEnd() const {
    return actors_.end();
  }

  void setSelectedActor(Actor *a) { selectedActor_ = a; }
  Actor *selectedActor() { return selectedActor_; }

  typedef std::list<TextObject *> text_list_type;
  text_list_type::const_iterator textsBegin() const {
    return textObjects_.begin();
  }
  text_list_type::const_iterator textsEnd() const {
    return textObjects_.end();
  }
  void registerTextObject(TextObject *a) { textObjects_.push_back(a); }
  void killTextObject(TextObject *a) {
	textObjects_.remove(a);
  }
  void killTextObjects() {
   while (!textObjects_.empty()) {
    delete textObjects_.back();
    textObjects_.pop_back();
   }
  }


private:
  static Engine *instance_;

  Engine();
  ~Engine() { }

  Scene *currScene_;

  unsigned frameStart_, frameTime_;

  bool controlsEnabled_[SDLK_EXTRA_LAST];

  actor_list_type actors_;
  Actor *selectedActor_;
  text_list_type textObjects_;
};

#endif
