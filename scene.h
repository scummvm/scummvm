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

#ifndef SCENE_H
#define SCENE_H

#include "vector3d.h"
#include "bitmap.h"
#include "color.h"
#include "debug.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

class Colormap;
class TextSplitter;

// The Lua code calls this a "set".

class Scene {
public:
  Scene(const char *name, const char *buf, int len);
  ~Scene();

  void drawBackground() const {
    if (currSetup_->bkgnd_zbm_ == NULL) {
      error("Null background for setup %s in %s", currSetup_->name_.c_str(), name_.c_str());
      return;
    }
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    currSetup_->bkgnd_zbm_->draw();
    glDepthMask(GL_TRUE);
  }
  void setupCamera() {
    currSetup_->setupCamera();
  }

  const char *name() const { return name_.c_str(); }

  void setSetup(int num) { currSetup_ = setups_ + num; }
  int setup() const { return currSetup_ - setups_; }

  // Sector access functions
  #define validSector(id) ((numSectors_ >= 0) && (id < numSectors_))
  int getSectorCount() { return numSectors_; }
  const char *getSectorName(int id) const {
   if (validSector(id)) return sectors_[id].name_.c_str(); else return NULL;
  }
  int getSectorType(int id) { if (validSector(id)) return sectors_[id].type_; else return -1; }
  int getSectorID(int id) { if (validSector(id)) return sectors_[id].id_; else return -1; }
  bool isPointInSector(int id, Vector3d point) { return false; } // FIXME: Need pointInPoly func

private:
  struct Setup {		// Camera setup data
    void load(TextSplitter &ts);
    void setupCamera() const;
    std::string name_;
    ResPtr<Bitmap> bkgnd_bm_, bkgnd_zbm_;
    Vector3d pos_, interest_;
    float roll_, fov_, nclip_, fclip_;
  };

  struct Light {		// Scene lighting data
    void load(TextSplitter &ts);
    std::string name_;
    std::string type_;
    Vector3d pos_, dir_;
    Color color_;
    float intensity_, umbraangle_, penumbraangle_;
  };

  struct Sector {		// Walkarea 'sectors'
    void load(TextSplitter &ts);
    void load0(TextSplitter &ts, char *name, int id);
    int numVertices_, id_;
    std::string name_;
    int type_;
    std::string visibility_;
    Vector3d *vertices_;
    float height_;
  };

  std::string name_;
  int numCmaps_;
  ResPtr<Colormap> *cmaps_;
  int numSetups_, numLights_, numSectors_;
  Sector *sectors_;
  Light *lights_;
  Setup *setups_;
  Setup *currSetup_;
};

#endif
