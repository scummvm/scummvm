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

#include "keyframe.hh"
#include "debug.hh"
#include "bits.hh"
#include "textsplit.hh"
#include <cstring>

KeyframeAnim::KeyframeAnim(const char *filename, const char *data, int len) :
  Resource(filename) {
  if (len >= 4 && std::memcmp(data, "FYEK", 4) == 0)
    loadBinary(data, len);
  else {
    TextSplitter ts(data, len);
    loadText(ts);
  }
}

void KeyframeAnim::loadBinary(const char *data, int len) {
  flags_ = get_LE_uint32(data + 40);
  type_ = get_LE_uint32(data + 48);
  fps_ = get_float(data + 52);
  numFrames_ = get_LE_uint32(data + 56);
  numJoints_ = get_LE_uint32(data + 60);
  numMarkers_ = get_LE_uint32(data + 68);
  markers_ = new Marker[numMarkers_];
  for (int i = 0; i < numMarkers_; i++) {
    markers_[i].frame_ = get_float(data + 72 + 4 * i);
    markers_[i].val_ = get_LE_uint32(data + 104 + 4 * i);
  }

  nodes_ = new KeyframeNode*[numJoints_];
  for (int i = 0; i < numJoints_; i++)
    nodes_[i] = NULL;
  const char *dataEnd = data + len;
  data += 180;
  while (data < dataEnd) {
    int nodeNum = get_LE_uint32(data + 32);
    nodes_[nodeNum] = new KeyframeNode;
    nodes_[nodeNum]->loadBinary(data);
  }
}

void KeyframeAnim::loadText(TextSplitter &ts) {
  ts.expectString("section: header");
  ts.scanString("flags %i", 1, &flags_);
  ts.scanString("type %i", 1, &type_);
  ts.scanString("frames %d", 1, &numFrames_);
  ts.scanString("fps %f", 1, &fps_);
  ts.scanString("joints %d", 1, &numJoints_);

  if (std::strcmp(ts.currentLine(), "section: markers") == 0) {
    ts.nextLine();
    ts.scanString("markers %d", 1, &numMarkers_);
    markers_ = new Marker[numMarkers_];
    for (int i = 0; i < numMarkers_; i++)
      ts.scanString("%f %d", 2, &markers_[i].frame_, &markers_[i].val_);
  }
  else {
    numMarkers_ = 0;
    markers_ = NULL;
  }

  ts.expectString("section: keyframe nodes");
  int numNodes;
  ts.scanString("nodes %d", 1, &numNodes);
  nodes_ = new KeyframeNode*[numJoints_];
  for (int i = 0; i < numJoints_; i++)
    nodes_[i] = NULL;
  for (int i = 0; i < numNodes; i++) {
    int which;
    ts.scanString("node %d", 1, &which);
    nodes_[which] = new KeyframeNode;
    nodes_[which]->loadText(ts);
  }
}

KeyframeAnim::~KeyframeAnim() {
  for (int i = 0; i < numJoints_; i++)
    delete nodes_[i];
  delete[] markers_;
}

void KeyframeAnim::animate(Model::HierNode *nodes, float time,
			   int priority1, int priority2) const {
  float frame = time * fps_;
  if (frame > numFrames_)
    frame = numFrames_;
  for (int i = 0; i < numJoints_; i++)
    if (nodes_[i] != NULL)
      nodes_[i]->animate(nodes[i], frame,
			 ((type_ & nodes[i].type_) != 0 ?
			  priority2 : priority1));
}

void KeyframeAnim::KeyframeEntry::loadBinary(const char *&data) {
  frame_ = get_float(data);
  flags_ = get_LE_uint32(data + 4);
  pos_ = get_vector3d(data + 8);
  pitch_ = get_float(data + 20);
  yaw_ = get_float(data + 24);
  roll_ = get_float(data + 28);
  dpos_ = get_vector3d(data + 32);
  dpitch_ = get_float(data + 44);
  dyaw_ = get_float(data + 48);
  droll_ = get_float(data + 52);
  data += 56;
}

void KeyframeAnim::KeyframeNode::loadBinary(const char *&data) {
  std::memcpy(meshName_, data, 32);
  numEntries_ = get_LE_uint32(data + 36);
  data += 44;
  entries_ = new KeyframeEntry[numEntries_];
  for (int i = 0; i < numEntries_; i++)
    entries_[i].loadBinary(data);
}

void KeyframeAnim::KeyframeNode::loadText(TextSplitter &ts) {
  ts.scanString("mesh name %s", 1, meshName_);
  ts.scanString("entries %d", 1, &numEntries_);
  entries_ = new KeyframeEntry[numEntries_];
  for (int i = 0; i < numEntries_; i++) {
    int which, flags;
    float frame, x, y, z, p, yaw, r, dx, dy, dz, dp, dyaw, dr;
    ts.scanString(" %d: %f %i %f %f %f %f %f %f", 9, &which, &frame, &flags,
		  &x, &y, &z, &p, &yaw, &r);
    ts.scanString(" %f %f %f %f %f %f", 6, &dx, &dy, &dz, &dp, &dyaw, &dr);
    entries_[which].frame_ = frame;
    entries_[which].flags_ = flags;
    entries_[which].pos_ = Vector3d(x, y, z);
    entries_[which].dpos_ = Vector3d(dx, dy, dz);
    entries_[which].pitch_ = p;
    entries_[which].yaw_ = yaw;
    entries_[which].roll_ = r;
    entries_[which].dpitch_ = dp;
    entries_[which].dyaw_ = dyaw;
    entries_[which].droll_ = dr;
  }
}

KeyframeAnim::KeyframeNode::~KeyframeNode() {
  delete[] entries_;
}

void KeyframeAnim::KeyframeNode::animate(Model::HierNode &node,
					 float frame, int priority) const {
  if (numEntries_ == 0)
    return;
  if (priority < node.priority_)
    return;

  // Do a binary search for the nearest previous frame
  // Loop invariant: entries_[low].frame_ <= frame < entries_[high].frame_
  int low = 0, high = numEntries_;
  while (high > low + 1) {
    int mid = (low + high) / 2;
    if (entries_[mid].frame_ <= frame)
      low = mid;
    else
      high = mid;
  }

  float dt = frame - entries_[low].frame_;
  Vector3d pos = entries_[low].pos_ + dt * entries_[low].dpos_;
  float pitch = entries_[low].pitch_ + dt * entries_[low].dpitch_;
  float yaw = entries_[low].yaw_ + dt * entries_[low].dyaw_;
  float roll = entries_[low].roll_ + dt * entries_[low].droll_;
  if (pitch > 180)
    pitch -= 360;
  if (yaw > 180)
    yaw -= 360;
  if (roll > 180)
    roll -= 360;

  if (priority > node.priority_) {
    node.priority_ = priority;
    node.totalWeight_ = 1;
    node.animPos_ = pos;
    node.animPitch_ = pitch;
    node.animYaw_ = yaw;
    node.animRoll_ = roll;
  }
  else {			// priority == node.priority_
    node.totalWeight_++;
    node.animPos_ += pos;
    node.animPitch_ += pitch;
    node.animYaw_ += yaw;
    node.animRoll_ += roll;
  }
}
