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

#ifndef BITMAP_H
#define BITMAP_H

#include "resource.h"
#include <cstring>

class Bitmap : public Resource {
public:
  // Construct a bitmap from the given data.
  Bitmap(const char *filename, const char *data, int len);

  // Set up GL for drawing bitmaps
  static void prepareGL();
  void draw() const;

  // Set which image in an animated bitmap to use
  void setNumber(int n) { curr_image_ = n; }

  int numImages() const { return num_images_; }
  int currentImage() const { return curr_image_; }

  int width() const { return width_; }
  int height() const { return height_; }
  int x() const { return x_; }
  int y() const { return y_; }

  ~Bitmap();

private:
  char **data_;
  int num_images_, curr_image_;
  int width_, height_, x_, y_;
  int format_;
};

#endif
