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

#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "color.h"
#include "debug.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

class TextObject {
 public:
  TextObject(const char *text, const int x, const int y, const Color& fgColor);
  void setX(int x);
  void setY(int y);
  void setColor(Color *newColor);

  const char *name() const { return textID_; }
  void draw();

 protected:
   char textID_[10];
   Color fgColor_;
   int x_, y_;
};
#endif
