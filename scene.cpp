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

#include "scene.h"
#include "textsplit.h"
#include "resource.h"
#include "debug.h"
#include "bitmap.h"
#include "colormap.h"
#include "vector3d.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>

Scene::Scene(const char *name, const char *buf, int len) :
  name_(name) {
  TextSplitter ts(buf, len);
  char tempBuf[256];

  ts.expectString("section: colormaps");
  ts.scanString(" numcolormaps %d", 1, &numCmaps_);
  cmaps_ = new ResPtr<Colormap>[numCmaps_];
  char cmap_name[256];
  for (int i = 0; i < numCmaps_; i++) {
    ts.scanString(" colormap %256s", 1, cmap_name);
    cmaps_[i] = ResourceLoader::instance()->loadColormap(cmap_name);
  }

  ts.expectString("section: setups");
  ts.scanString(" numsetups %d", 1, &numSetups_);
  setups_ = new Setup[numSetups_];
  for (int i = 0; i < numSetups_; i++)
    setups_[i].load(ts);
  currSetup_ = setups_;

  ts.expectString("section: lights");
  ts.scanString(" numlights %d", 1, &numLights_);
  lights_ = new Light[numLights_];
  for (int i = 0; i < numLights_; i++)
    lights_[i].load(ts);

  // Calculate the number of sectors
  ts.expectString("section: sectors");
  if (ts.eof()) 	// Sometimes there ARE no sectors (eg, inv room)
	return;
  ts.scanString(" sector %256s", 1, tempBuf);
  ts.scanString(" id %d", 1, &numSectors_);
  sectors_ = new Sector[numSectors_];
  // FIXME: This would be nicer if we could rewind the textsplitter
  // stream...
  sectors_[0].load0(ts, tempBuf, numSectors_);
  for (int i = 1; i < numSectors_; i++)
    sectors_[i].load(ts);
}

Scene::~Scene() {
  delete [] cmaps_;
  delete [] setups_;
  delete [] lights_;
  delete [] sectors_;
}

void Scene::Sector::load(TextSplitter &ts) {
  char buf[256];
  int id = 0;
  ts.scanString(" sector %256s", 1, buf);
  ts.scanString(" id %d", 1, &id);
  load0(ts, buf, id);
}

void Scene::Sector::load0(TextSplitter &ts, char *name, int id) {
  char buf[256];
  int i = 0;
  Vector3d tempVert;

  name_ = name;
  id_ = id;
  ts.scanString(" type %256s", 1, buf);
  type_ = buf;
  ts.scanString(" default visibility %256s", 1, buf);
  visibility_ = buf;
  ts.scanString(" height %f", 1, &height_);
  ts.scanString(" numvertices %d", 1, &numVertices_);
  vertices_ = new Vector3d[numVertices_];

  ts.scanString(" vertices: %f %f %f", 3, &vertices_[0].x(), &vertices_[0].y(), &vertices_[0].z());
  for (i=1;i<numVertices_;i++)
    ts.scanString(" %f %f %f", 3, &vertices_[i].x(), &vertices_[i].y(), &vertices_[i].z());
}

void Scene::Setup::load(TextSplitter &ts) {
  char buf[256];
  ts.scanString(" setup %256s", 1, buf);
  name_ = buf;

  ts.scanString(" background %256s", 1, buf);
  bkgnd_bm_ = ResourceLoader::instance()->loadBitmap(buf);
  ts.scanString(" zbuffer %256s", 1, buf);
  bkgnd_zbm_ = ResourceLoader::instance()->loadBitmap(buf);

  ts.scanString(" position %f %f %f", 3, &pos_.x(), &pos_.y(), &pos_.z());
  ts.scanString(" interest %f %f %f", 3, &interest_.x(), &interest_.y(),
		&interest_.z());
  ts.scanString(" roll %f", 1, &roll_);
  ts.scanString(" fov %f", 1, &fov_);
  ts.scanString(" nclip %f", 1, &nclip_);
  ts.scanString(" fclip %f", 1, &fclip_);
}

void Scene::Light::load(TextSplitter &ts) {
  char buf[256];
  ts.scanString(" light %256s", 1, buf);
  name_ = buf;

  ts.scanString(" type %256s", 1, buf);
  type_ = buf;

  ts.scanString(" position %f %f %f", 3, &pos_.x(), &pos_.y(), &pos_.z());
  ts.scanString(" direction %f %f %f", 3, &dir_.x(), &dir_.y(), &dir_.z());
  ts.scanString(" intensity %f", 1, &intensity_);
  ts.scanString(" umbraangle %f", 1, &umbraangle_);
  ts.scanString(" penumbraangle %f", 1, &penumbraangle_);
  ts.scanString(" color %d %d %d", 3, &color_.red(), &color_.green(), &color_.blue());
}

void Scene::Setup::setupCamera() const {
  // Set perspective transformation
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //  gluPerspective(std::atan(std::tan(fov_ / 2 * (M_PI/180)) * 0.75) * 2 * (180/M_PI), 4.0f / 3, nclip_, fclip_);
  float right = nclip_ * std::tan(fov_ / 2 * (M_PI/180));
  glFrustum(-right, right, -right * 0.75, right * 0.75, nclip_, fclip_);

  // Apply camera roll
  glRotatef(roll_, 0, 0, -1);

  // Set camera position and direction
  Vector3d up_vec(0, 0, 1);
  if (pos_.x() == interest_.x() && pos_.y() == interest_.y())
    up_vec = Vector3d(0, 1, 0);
  gluLookAt(pos_.x(), pos_.y(), pos_.z(),
	    interest_.x(), interest_.y(), interest_.z(),
	    up_vec.x(), up_vec.y(), up_vec.z());

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
