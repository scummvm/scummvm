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

#include "material.h"
#include "colormap.h"
#include "bits.h"
#include "debug.h"

Material::Material(const char *filename, const char *data, int len,
		   const Colormap &cmap) :
  Resource(filename)
{
  if (len < 4 || memcmp(data, "MAT ", 4) != 0)
    error("invalid magic loading texture\n");

  num_images_ = get_LE_uint32(data + 12);
  curr_image_ = 0;
  textures_ = new GLuint[num_images_];
  glGenTextures(num_images_, textures_);
  width_ = get_LE_uint32(data + 76 + num_images_ * 40);
  height_ = get_LE_uint32(data + 80 + num_images_ * 40);

  if ((width_ == 0) || (height_ == 0)) {
    warning("bad texture size (%dx%d) for texture %s\n", width_, height_, filename);
  }

  data += 100 + num_images_ * 40;
  char *texdata = new char[width_ * height_ * 4];
  for (int i = 0; i < num_images_; i++) {
    char *texdatapos = texdata;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
	int col = get_uint8(data);
	if (col == 0)
	  memset(texdatapos, 0, 4); // transparent
	else {
	  memcpy(texdatapos, cmap.colors + 3 * get_uint8(data), 3);
	  texdatapos[3] = '\xff'; // fully opaque
	}
	texdatapos += 4;
	data++;
      }
    }
    glBindTexture(GL_TEXTURE_2D, textures_[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, texdata);
    data += 24;
  }

  delete[] texdata;
}

void Material::select() const {
  glBindTexture(GL_TEXTURE_2D, textures_[curr_image_]);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(1.0f / width_, 1.0f / height_, 1);
}

Material::~Material() {
  glDeleteTextures(num_images_, textures_);
  delete[] textures_;
}
