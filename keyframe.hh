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

#ifndef KEYFRAME_HH
#define KEYFRAME_HH

#include "vector3d.hh"
#include "resource.hh"
#include "model.hh"

class TextSplitter;

class KeyframeAnim : public Resource {
public:
  KeyframeAnim(const char *filename, const char *data, int len);
  ~KeyframeAnim();

  void loadBinary(const char *data, int len);
  void loadText(TextSplitter &ts);
  void animate(Model::HierNode *nodes, float time,
	       int priority1 = 1, int priority2 = 5) const;

  float length() const { return numFrames_ / fps_; }

private:
  int flags_, type_, numFrames_, numJoints_;
  float fps_;
  int numMarkers_;

  struct Marker {
    float frame_;
    int val_;
  };
  Marker *markers_;

  struct KeyframeEntry {
    void loadBinary(const char *&data);

    float frame_;
    int flags_;
    Vector3d pos_, dpos_;
    float pitch_, yaw_, roll_, dpitch_, dyaw_, droll_;
  };

  struct KeyframeNode {
    void loadBinary(const char *&data);
    void loadText(TextSplitter &ts);
    ~KeyframeNode();

    void animate(Model::HierNode &node, float frame, int priority) const;
    char meshName_[32];
    int numEntries_;
    KeyframeEntry *entries_;
  };

  KeyframeNode **nodes_;
};

#endif
