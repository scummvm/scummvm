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

#include "stdafx.h"
#include "scene.h"
#include "textsplit.h"
#include "resource.h"
#include "debug.h"
#include "bitmap.h"
#include "colormap.h"
#include "vector3d.h"
#include <SDL.h>
#include <cmath>
#include "screen.h"
#include "driver_gl.h"

Scene::Scene(const char *name, const char *buf, int len) :
  name_(name) {
  TextSplitter ts(buf, len);
  char tempBuf[256];

  ts.expectString("section: colormaps");
  ts.scanString(" numcolormaps %d", 1, &numCmaps_);
  cmaps_ = new ResPtr<CMap>[numCmaps_];
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

  numSectors_ = -1;
  numLights_ = -1;
  lights_ = NULL;
  sectors_ = NULL;
  // Lights are optional
  if (ts.eof())
   return;

  ts.expectString("section: lights");
  ts.scanString(" numlights %d", 1, &numLights_);
  lights_ = new Light[numLights_];
  for (int i = 0; i < numLights_; i++)
    lights_[i].load(ts);

  // Calculate the number of sectors
  ts.expectString("section: sectors");
  if (ts.eof()) 	// Sectors are optional, but section: doesn't seem to be
	return;

 // Sector NAMES can be null, but ts doesn't seem flexible enough to allow this
 if (strlen(ts.currentLine()) > strlen(" sector"))
   ts.scanString(" sector %256s", 1, tempBuf);
  else {
   ts.nextLine();
   strcpy(tempBuf, "");
  }

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
  if (lights_)
   delete [] lights_;
  if (sectors_)
   delete [] sectors_; 
}

void Scene::Setup::load(TextSplitter &ts) {
  char buf[256];

  ts.scanString(" setup %256s", 1, buf);
  name_ = buf;

  ts.scanString(" background %256s", 1, buf);
  bkgnd_bm_ = ResourceLoader::instance()->loadBitmap(buf);

  // ZBuffer is optional
  if (!ts.checkString("zbuffer")) {
    bkgnd_zbm_ = NULL;
  } else {
    ts.scanString(" zbuffer %256s", 1, buf);
    bkgnd_zbm_ = ResourceLoader::instance()->loadBitmap(buf);
  }

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

 // Light names can be null, but ts doesn't seem flexible enough to allow this
 if (strlen(ts.currentLine()) > strlen(" light"))
   ts.scanString(" light %256s", 1, buf);
  else {
   ts.nextLine();
   strcpy(buf, "");
  }
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
  g_driver->setupCamera(fov_, nclip_, fclip_);
  g_driver->positionCamera(roll_, pos_, interest_);
}

void Scene::setSetup(int num)
{
	currSetup_ = setups_ + num;

	if (SCREENBLOCKS_GLOBAL == 0)
		return;
	if(currSetup_->bkgnd_zbm_)
		screenBlocksInit( currSetup_->bkgnd_zbm_->getData() );
	else
		screenBlocksInitEmpty();
}
